/* ============================================================
 * M258 门 · 现场探针：「px_root_restore 的返回值窗口」（缺陷 460）
 * ------------------------------------------------------------
 * 复刻 native 桥的**深度式登记 + 出口归一**惯用法（runtime_h3_qpack*.c 的三个
 * 解码函数即此形态）：
 *
 *     rd = px_root_depth(&rm);     // 进函数记深度
 *     box = px_list(3);  PX_KEEP(box);   // 本帧要交回调用方的容器
 *     ...
 *     px_root_restore(rd, rm);     // ← **本帧登记到此为止**
 *     return box;                  // ← 交回调用方；调用方随后自己 PX_KEEP
 *
 * 病灶：**立即收缩**（修前语义）在 `px_root_restore` 那一瞬就把 `box` 从根面摘掉，
 *   而「返回值 → 调用方 PX_KEEP」这个窗口里它只由 **C 局部**持有 ——
 *   VM 轨产物默认 precise GC（**不扫 C 栈**）⇒ 窗口内任何一次分配都可能回收它
 *   ⇒ 调用方拿到**已回收对象**（M257/缺陷 459 的实测形态）。
 * 与 M183（缺陷 197）**同族**：那次修的是 `px_root_pop`，本处是另一个入口。
 *
 * 本探针在窗口内**故意制造分配**（pad 循环）并按 **VM 轨默认档**（precise）运行：
 *   · 修后（延迟收缩）：box 仍在根面 ⇒ LEN/SUM 正确、无任何检测器报告
 *   · 修前/注入退化：box 已被回收、其槽被 pad 串复用 ⇒ `px_list_push` 触发
 *     M257 的「容器存储已失效」响亮报告（stderr）⇒ 与修后**可区分**
 * ⚠️ 必须 `px_gc_set_precise(1)`：conservative 档会扫 C 栈把 box 捞回来，
 *   那样**窗口根本不暴露**（这正是它长期没被发现的原因）。
 * ============================================================ */
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_gc_set_precise(1);   /* VM 轨产物默认档 —— 现场复刻的必要条件 */

    /* ── native 桥体：深度式登记 ── */
    int rm = 0;
    int rd = px_root_depth(&rm);
    LXValue box = px_list(3);
    PX_KEEP(box);
    px_list_push(box, px_int(11));
    px_list_push(box, px_int(22));
    px_list_push(box, px_int(33));

    /* ── 出口归一：把本帧登记还给调用方（**被测的那一行**） ── */
    px_root_restore(rd, rm);

    /* ── 窗口：调用方在 PX_KEEP 之前的任何分配 ── */
    for (int i = 0; i < 400; i++) {
        LXValue t = px_str("padding-string-0123456789abcdef");
        (void)t;
    }

    /* ── 调用方「接住」返回值：此前的写操作即现场取证 ── */
    px_list_push(box, px_int(44));
    PX_KEEP(box);

    printf("LEN=%d\n", box.as.obj ? box.as.obj->as.list.len : -1);
    printf("SUM=%lld\n", box.as.obj
        ? (long long)(box.as.obj->as.list.items[0].as.i
                    + box.as.obj->as.list.items[1].as.i
                    + box.as.obj->as.list.items[2].as.i)
        : -1LL);
    printf("M258-PROBE-END\n");
    return 0;
}
