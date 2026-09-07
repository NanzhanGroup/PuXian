// ============================================================
// M89-S3-A1: vm_selftest.c —— VM 引擎 A1 自测（GETG/SETG/Top 运行）
// ------------------------------------------------------------
// 手写小型 PxBCModule（与 bc_emit.px A1 输出形态一致：K/G/funcs/top），
// 验证 vm.c A1 扩展：
//   ① SETG/GETG v1 语义（经 px_set_global/px_get_global）
//   ② px_vm_run_module 完整 Top 运行（注册非 top 函数 D2 trampoline + 跑 Top）
//   ③ 注册后的函数可经 px_get_global + px_call 调用（px_vm_entry 入口）
// 运行：
//   gcc -c -I<rtcache> vm.c && gcc -c selftest && 链接 runtime .o 全家
// 断言失败 exit 1；全过打印 "VM selftest A1: ALL PASS"。
// ============================================================
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail = 0;
#define CK(cond, msg) do { if (!(cond)) { \
    fprintf(stderr, "❌ %s (行 %d)\n", msg, __LINE__); g_fail = 1; } \
  } while (0)

// ---- BCModule A1 冒烟：read() 读全局 answer；Top SETG answer=42 ----
extern const PxBCModule m1;

static const PxInst read_bc[] = {
    {PXOP_SRCLINE, 0, 0, 5, 0},   // 行号
    {PXOP_GETG,    0, 0, 0, 0},   // 槽0 = 全局 G[0]("answer")
    {PXOP_RET,     0, 0, 0, 0},   // return 槽0
};
static const PxInst top_bc[] = {
    {PXOP_SRCLINE, 0, 0, 1, 0},
    {PXOP_LOADK,   0, 0, 0, 0},   // 槽0 = K[0] = int 42
    {PXOP_SETG,    0, 0, 0, 0},   // G[0]("answer") = 槽0
    {PXOP_HALT,    0, 0, 0, 0},
};
static const PxVMFunc funcs1[] = {
    { "read", 0, 0, 1, read_bc, 3, &m1, NULL },
    { "<top>", 0, 0, 1, top_bc, 4, &m1, NULL },
};
static const PxK k_pool1[] = {
    { PXK_INT, 42, 0.0, NULL },
};
static const char* g_pool1[] = { "answer" };
const PxBCModule m1 = {
    "selftest", k_pool1, 1, NULL, 0, g_pool1, 1, funcs1, 2, 1,
};

static int64_t as_int(LXValue v) {
    if (v.type != PX_INT) return -999999;
    return v.as.i;
}

int main(void) {
    px_register_builtins();
    PxVmState* st = px_vm_state();

    // ① Top 运行：SETG answer = 42（LOADK K0）
    LXValue r = px_vm_run_module(st, &m1);
    (void)r;
    LXValue ans = px_get_global("answer");
    CK(ans.type == PX_INT && as_int(ans) == 42, "Top SETG answer=42 后全局可取");
    if (g_fail == 0) printf("✓ Top 运行: answer = %lld\n", (long long)as_int(ans));

    // ② 注册函数经 px_get_global + px_call（D2 px_vm_entry trampoline）可调
    LXValue f = px_get_global("read");
    CK(f.type == PX_FUNC || f.type == PX_NATIVE, "run_module 已注册 read 函数");
    if (g_fail == 0) {
        LXValue d = px_null();
        LXValue v = px_call(f, &d, 0);
        CK(v.type == PX_INT && as_int(v) == 42, "px_call(read) 经 GETG 返回 42");
        if (g_fail == 0) printf("✓ px_call(read) = %lld\n", (long long)as_int(v));
    }

    // ③ GETG 走 g_globals：外部改全局 → read() 读新值（v1 语义 = px_get_global）
    px_set_global("answer", px_int(7));
    if (g_fail == 0) {
        LXValue d = px_null();
        LXValue v = px_call(f, &d, 0);
        CK(v.type == PX_INT && as_int(v) == 7, "GETG v1 读 g_globals 最新值(7)");
        if (g_fail == 0) printf("✓ GETG v1: px_set_global(7) 后 read() = %lld\n", (long long)as_int(v));
    }

    if (g_fail) { printf("VM selftest A1: FAIL\n"); return 1; }
    printf("VM selftest A1: ALL PASS\n");
    return 0;
}
