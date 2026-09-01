// 普贤 C 运行时并发 GC 单元测试（M11）
// 编译：gcc -std=c99 -O2 -Iruntime -I$MB/include tests/gc/test_gc_concurrent.c runtime/runtime.c \
//          $MB/lib/libmbedtls.a $MB/lib/libmbedx509.a $MB/lib/libmbedcrypto.a -o /tmp/test_gc_conc -lpthread -lm
// 运行：/tmp/test_gc_conc
//
// 验证点：
//   1) spawn 线程活跃期间 GC 真正执行（runs 增加）——不再降级跳过（M11 核心）
//   2) 并发分配 + 循环引用 + 通道通信全程无崩溃、无死锁
//   3) 被引用对象（chan 中传输）不被误回收（正确性）
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// 结果通道（全局根保护，同时验证全局根在并发 GC 下保留）
static LXValue g_ch;

// worker：分配大量无引用垃圾 + 周期性循环引用 + 最后通过 chan 发回校验和
static LXValue worker_garbage(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    (void)nargs;
    int n = (int)args[0].as.i;
    int64_t sum = 0;
    for (int i = 0; i < n; i++) {
        LXValue s = px_str("concurrent-garbage");
        LXValue l = px_list(4);
        px_list_push(l, px_int(i));
        px_list_push(l, s);   // l 引用 s；函数返回后 l/s 均无外部引用 → 垃圾
        (void)l;
        if ((i % 100) == 0) {  // 周期性循环引用（mark-sweep 可回收）
            LXValue a = px_list(1);
            LXValue b = px_list(1);
            px_list_push(a, b);
            px_list_push(b, a);
            (void)a; (void)b;
        }
        sum += i;
    }
    px_chan_send(g_ch, px_int(sum));   // 阻塞发送：同时验证 chan 在并发 GC 下完好
    return px_null();
}

// 验证 chan 中传输的对象（跨线程引用）不被误回收
static LXValue worker_share(LXValue* args, int nargs, void* ctx) {
    (void)ctx; (void)nargs;
    int n = (int)args[0].as.i;
    LXValue lst = px_list(n);
    for (int i = 0; i < n; i++) px_list_push(lst, px_str("shared"));
    px_chan_send(g_ch, lst);   // 把大列表传给主线程（跨线程可达 → 必须存活）
    return px_null();
}

int main(void) {
    setenv("PX_GC_THRESHOLD", "2000", 1);   // 小阈值：频繁触发，高强度并发 GC
    int live = 0, total = 0;
    int runs0 = px_gc_stats(&live, &total);

    // --- 场景 A：4 worker 并发分配垃圾 + 主线程同时分配，验证并发 GC 真正执行 ---
    g_ch = px_chan_create(8);
    px_set_global("__ch", g_ch);
    LXValue arg = px_int(20000);
    px_spawn(worker_garbage, &arg, 1);
    px_spawn(worker_garbage, &arg, 1);
    px_spawn(worker_garbage, &arg, 1);
    px_spawn(worker_garbage, &arg, 1);
    // 主线程同时大量分配（触发 GC → 必须暂停 4 个 worker 后回收）
    for (int i = 0; i < 30000; i++) {
        LXValue s = px_str("main-garbage");
        LXValue l = px_list(4);
        px_list_push(l, px_int(i));
        (void)s; (void)l;
    }
    // 接收 4 个 worker 的校验和（会等待全部完成；能返回即无死锁）
    int64_t worker_sum = 0;
    for (int i = 0; i < 4; i++) {
        LXValue v = px_chan_recv(g_ch);
        assert(v.type == PX_INT);
        worker_sum += v.as.i;
    }
    int runs1 = px_gc_stats(&live, &total);
    int64_t expect = 4LL * 20000LL * 19999LL / 2LL;   // 4 * sum(0..19999)
    printf("[A] 并发 GC runs: %d -> %d, live=%d, 累计回收=%d\n", runs0, runs1, live, total);
    assert(runs1 > runs0);                 // M11 核心：并发期间 GC 真正执行（不再跳过）
    assert(worker_sum == expect);          // 校验和正确（线程通信正确）
    assert(total > 10000);                 // 并发期间实际回收了大量对象

    // --- 场景 B：跨线程对象（chan 传输的大列表）在并发 GC 下不被误回收 ---
    int runs2 = px_gc_stats(&live, &total);
    int nshare = 8;
    for (int i = 0; i < nshare; i++) px_spawn(worker_share, &arg, 1);
    // 主线程继续分配垃圾（触发并发 GC，压力跨线程引用保护）
    for (int i = 0; i < 20000; i++) { LXValue s = px_str("junk"); (void)s; }
    for (int i = 0; i < nshare; i++) {
        LXValue v = px_chan_recv(g_ch);
        assert(v.type == PX_LIST);
        assert(px_len(v) == 20000);                    // 列表完整存活
        LXValue e = px_index(v, px_int(0));
        assert(e.type == PX_STR && strcmp(e.as.obj->as.str.data, "shared") == 0);
    }
    int runs3 = px_gc_stats(&live, &total);
    printf("[B] 跨线程引用保护: runs %d -> %d, live=%d\n", runs2, runs3, live);
    assert(runs3 > runs2);

    printf("ALL CONCURRENT GC TESTS PASSED\n");
    return 0;
}
