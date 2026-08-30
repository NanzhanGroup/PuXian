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
        LXValue s = lx_str("garbage-object");
        (void)s;
    }
}

// 创建一对循环引用（函数返回后外部无引用）
static void create_cycle(void) {
    LXValue a = lx_list(1);
    LXValue b = lx_list(1);
    lx_list_push(a, b);
    lx_list_push(b, a);
    (void)a; (void)b;
}

// 深链：n 层嵌套列表
static LXValue make_deep(int n) {
    LXValue root = lx_list(1);
    LXValue cur = root;
    for (int i = 0; i < n; i++) {
        LXValue nx = lx_list(1);
        lx_list_push(cur, nx);
        cur = nx;
    }
    return root;
}

int main(void) {
    setenv("PX_GC_THRESHOLD", "1000", 1);  // 小阈值：验证自动触发
    int live = 0, total = 0;

    // 1. 无引用对象回收（手动 GC）
    create_garbage(10000);
    lx_gc_collect();
    int runs = lx_gc_stats(&live, &total);
    printf("[1] runs=%d live=%d freed=%d\n", runs, live, total);
    assert(total > 5000);

    // 2. 循环引用回收（mark-sweep 天然优势）
    int before_free = total;
    for (int i = 0; i < 5000; i++) create_cycle();
    lx_gc_collect();
    lx_gc_stats(&live, &total);
    printf("[2] 循环引用 freed=%d live=%d\n", total - before_free, live);
    assert(total - before_free > 9000);  // 10000 个循环对象绝大部分回收

    // 3. 全局根保留（全局表可达对象不可回收，子对象完整）
    LXValue g = lx_list(8);
    for (int i = 0; i < 1000; i++) lx_list_push(g, lx_str("item"));
    lx_set_global("keep", g);
    create_garbage(10000);
    lx_gc_collect();
    assert(lx_len(g) == 1000);
    LXValue first = lx_index(g, lx_int(0));
    assert(first.type == LX_STR);
    assert(strcmp(first.as.obj->as.str.data, "item") == 0);
    printf("[3] 全局根保留 OK (len=%d)\n", lx_len(g));

    // 4. 自动触发：创建大量对象，GC 应自动运行（runs 增加）
    //    （放在深链之前：避免深链把阈值推高导致不触发）
    int runs_before = lx_gc_stats(&live, &total);
    create_garbage(50000);
    int runs_after = lx_gc_stats(&live, &total);
    printf("[4] 自动触发 runs: %d -> %d, live=%d\n", runs_before, runs_after, live);
    assert(runs_after > runs_before);

    // 5. 深链标记不栈溢出（显式栈 DFS）
    LXValue root = make_deep(200000);
    lx_set_global("deep", root);
    create_garbage(10000);
    lx_gc_collect();
    LXValue cur = root;
    int depth = 0;
    while (lx_len(cur) > 0) {
        cur = lx_index(cur, lx_int(0));
        depth++;
    }
    fprintf(stderr, "[5] 深链 depth=%d (期望 200000)\n", depth);
    assert(depth == 200000);

    printf("ALL GC TESTS PASSED\n");
    return 0;
}
