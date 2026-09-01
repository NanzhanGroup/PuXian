// 普贤 C 运行时 GC 单元测试（M8）
// 编译：gcc -std=c99 -O2 -Iruntime tests/gc/test_gc.c runtime/runtime.c -o /tmp/test_gc -lpthread -lm
// 运行：/tmp/test_gc
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// 创建 n 个无引用垃圾对象（每次迭代 s 只保留最后一个）
static void create_garbage(int n) {
    for (int i = 0; i < n; i++) {
        LXValue s = px_str("garbage-object");
        (void)s;
    }
}

// 创建一对循环引用（函数返回后外部无引用）
static void create_cycle(void) {
    LXValue a = px_list(1);
    LXValue b = px_list(1);
    px_list_push(a, b);
    px_list_push(b, a);
    (void)a; (void)b;
}

// 深链：n 层嵌套列表
static LXValue make_deep(int n) {
    LXValue root = px_list(1);
    LXValue cur = root;
    for (int i = 0; i < n; i++) {
        LXValue nx = px_list(1);
        px_list_push(cur, nx);
        cur = nx;
    }
    return root;
}

int main(void) {
    setenv("PX_GC_THRESHOLD", "1000", 1);  // 小阈值：验证自动触发
    int live = 0, total = 0;

    // 1. 无引用对象回收（手动 GC）
    create_garbage(10000);
    px_gc_collect();
    int runs = px_gc_stats(&live, &total);
    printf("[1] runs=%d live=%d freed=%d\n", runs, live, total);
    assert(total > 5000);

    // 2. 循环引用回收（mark-sweep 天然优势）
    int before_free = total;
    for (int i = 0; i < 5000; i++) create_cycle();
    px_gc_collect();
    px_gc_stats(&live, &total);
    printf("[2] 循环引用 freed=%d live=%d\n", total - before_free, live);
    assert(total - before_free > 9000);  // 10000 个循环对象绝大部分回收

    // 3. 全局根保留（全局表可达对象不可回收，子对象完整）
    LXValue g = px_list(8);
    for (int i = 0; i < 1000; i++) px_list_push(g, px_str("item"));
    px_set_global("keep", g);
    create_garbage(10000);
    px_gc_collect();
    assert(px_len(g) == 1000);
    LXValue first = px_index(g, px_int(0));
    assert(first.type == PX_STR);
    assert(strcmp(first.as.obj->as.str.data, "item") == 0);
    printf("[3] 全局根保留 OK (len=%d)\n", px_len(g));

    // 4. 自动触发：创建大量对象，GC 应自动运行（runs 增加）
    //    （放在深链之前：避免深链把阈值推高导致不触发）
    int runs_before = px_gc_stats(&live, &total);
    create_garbage(50000);
    int runs_after = px_gc_stats(&live, &total);
    printf("[4] 自动触发 runs: %d -> %d, live=%d\n", runs_before, runs_after, live);
    assert(runs_after > runs_before);

    // 5. 深链标记不栈溢出（显式栈 DFS）
    //    （M11 分配器为 mmap 每对象一页 ≈4KB；200000 层 ≈ 800MB 超小内存服务器
    //     → 降为 50000 层 ≈ 200MB，测试目的"深链不栈溢出"不变）
    LXValue root = make_deep(50000);
    px_set_global("deep", root);
    create_garbage(10000);
    px_gc_collect();
    LXValue cur = root;
    int depth = 0;
    while (px_len(cur) > 0) {
        cur = px_index(cur, px_int(0));
        depth++;
    }
    fprintf(stderr, "[5] 深链 depth=%d (期望 50000)\n", depth);
    assert(depth == 50000);

    printf("ALL GC TESTS PASSED\n");
    return 0;
}
