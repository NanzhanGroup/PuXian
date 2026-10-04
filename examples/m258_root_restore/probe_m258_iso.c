/* ============================================================
 * M258 门 · **窗口正判据**：「立即收缩」形态 ⇒ 容器必丢
 * ------------------------------------------------------------
 * 本探针是 probe_m258.c 的**等价形态**，唯一差别是把出口归一换成**立即收缩**：
 *     px_root_iso_mark();  ...  PX_KEEP(box); ...  px_root_restore_iso();
 * `px_root_restore_iso` 的语义（M170）就是「把 g_px_roots_n 直接收缩回记录深度」
 * —— 也就是 `px_root_restore` **修前**的语义。
 *
 * 它存在的理由（两条，互相独立）
 *   ① **证明窗口真实存在**：若这一档也活得好好的，说明「窗口内被回收」这个命题
 *      在本环境根本不可观测 ⇒ 那么 probe_m258.c 的通过**什么都没证明**。
 *   ② **免改源码的负控**：本仓以往要证「判据有牙」都得**改产品代码再重编 runtime**
 *      （M257 实测 6~8 分钟/次）。本档用**公开 API 的等价语义**造出同一窗口
 *      ⇒ 不碰产品代码、不重编 ⇒ 判据的灵敏度**零成本**可验。
 * ⚠️ 本探针**不主张** iso 路径有缺陷：隔离点落点（longjmp 后）本就该立即作废被跳过
 *    的登记（M170 硬约束②）。这里只借用它的**语义形态**来造窗口。
 * ============================================================ */
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    px_args_init(argc, argv);
    px_register_builtins();
    px_gc_set_precise(1);

    px_root_iso_mark();              /* 记录深度（此刻 0） */
    LXValue box = px_list(3);
    PX_KEEP(box);
    px_list_push(box, px_int(11));
    px_list_push(box, px_int(22));
    px_list_push(box, px_int(33));
    px_root_restore_iso();           /* ← **立即收缩**（= px_root_restore 修前语义） */

    for (int i = 0; i < 400; i++) {
        LXValue t = px_str("padding-string-0123456789abcdef");
        (void)t;
    }

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
