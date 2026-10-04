/* ============================================================
 * M259 门 · 探针：**桥级**隔离点落点是否归还 marks（缺陷 463 / 决策项 A）
 * ------------------------------------------------------------
 * 复刻 native 桥内隔离点的形态（`px_native_call_capture`：runtime.c:14896）：

 *     px_root_iso_mark_deep();     // setjmp 之前：记 roots **与** marks
 *     px_root_push();              // 桥体内开作用域
 *     LXValue v = px_str(…); PX_KEEP(v);
 *     …  px_error(…) ⇒ longjmp     // 落点：px_root_restore_iso()
 *
 * 病灶（修前）：`px_root_iso_mark` 只记 **roots**，`px_root_restore_iso` 也只归还 roots
 *   ⇒ 被 longjmp **跳过** 的那个 `px_root_push` 的 **marks 条目永久留在栈里**。
 *   每次隔离错误 +1 层 ⇒ ① 栈无界增长（`g_px_root_marks` 反复 xrealloc）；
 *   ② 后续 `px_root_pop` 会弹到**别的层**（该帧自己的条目已被跳过）
 *      ⇒ 记下的待收缩深度指向错误的帧 ⇒ 收缩行为不可预期。
 *
 * ⚠️ 为什么只对**桥级**成立（与 M170「不得收缩 marks」的实测结论不冲突）：
 *   `px_root_iso_mark` 共 5 个调用点，分两类 ——
 *     · 协程/信号级（**跨让出**）：`px_spawn_isolate_begin` · `spawn_thread` ·
 *       `px_sig_isolate_begin` ⇒ 作用域内别的执行流会 push 条目 ⇒ 不得收缩（7/20 SIGSEGV）
 *     · **桥级**（不让出）：`px_native_call_capture` · `bi_json_parse_opt`
 *       ⇒ native 桥执行期间 `yield_ok=0`（`px_vm_run_func`）⇒ 无他人条目 ⇒ 可收缩
 *   ⇒ 修法新增 `px_root_iso_mark_deep`，**只给桥级两处用**（本探针即复刻其中一处）。
 *
 * 判据（本探针 + PX_GC_DEBUG=1）：
 *   `px_root_restore_iso` 打印 `root 还原 marks=N→M roots=R→S`。
 *   · 修前：每次 `marks=k→k`（累积）⇒ 末次 `marks=200→200`
 *   · 修后：每次 `marks=1→0`      ⇒ **M 恒为 0**
 * ============================================================ */
#include "runtime.h"
#include <stdio.h>

/* `_deep` 是 M259 新增的桥级入口（不在 runtime.h 的公开面里） */
extern void px_root_iso_mark_deep(void);

/* ⚠️ 刻意**不调** `px_root_pop()` —— 等价于 longjmp 跳过了它 */
static void sim_iso_skip(void) {
    px_root_iso_mark_deep();
    px_root_push();
    LXValue v = px_str("skipped-frame-value-0123456789abcdef");
    px_root_keep(&v);
    px_root_restore_iso();
}

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_gc_set_precise(1);   /* VM 轨产物默认档 */

    for (int i = 0; i < 200; i++) sim_iso_skip();

    /* 观察点：此后一个**正常配对**的 push/pop 序列仍须正确（不越界、不弹错层） */
    px_root_push();
    LXValue y = px_int(7);
    px_root_keep(&y);
    px_root_pop();

    printf("M259-PROBE-END\n");
    return 0;
}
