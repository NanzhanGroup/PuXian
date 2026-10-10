#!/usr/bin/env python3
# ============================================================
# examples/m294_h3_qpack_share/negctl.py
# ------------------------------------------------------------
# M294 门 · 负控打桩器（**忠实撤回**缺陷 507 的修复）
#
# 为什么单独成文件（M247「新建门的检查清单」· M285/M286 的门层守卫）：
#   · 门层守卫 `check_gate_assertions.py`（A1 锚点在位 / A4 锚点唯一性）要求
#     打桩锚点是**可解析、可校验**的 —— 写在 python 里比写在 shell heredoc 里更容易被守卫看见；
#   · `--selftest` 让「锚点还有没有牙」变成**可执行判据**（不是靠记性）。
#
# 撤回的 5 处（每一处都要求**恰好命中 1 次** —— 命中 0 = 锚点过期，命中 >1 = 打桩范围超出意图）：
#   ① `qd_sess` 结构体内的**警示注释** → 还原为 `int used;` 字段
#   ② `qd_get` 的独立占用位判断 → 还原为 `g_qds[id - 1].used`
#   ③ `g_qds_used` 独立数组 + 说明注释 → 删除
#   ④ `bi_qs_open` 的原子交换 → 还原为 `if (!g_qds[i].used) { memset; s->used = 1; }`
#   ⑤ `bi_qs_close` 的原子释放 → 删除
#   （另：`#include <stdatomic.h>` 一并撤回）
#   ⚠️ M295s1：M295 又改了同一文件（`g_qds` 静态数组 → 指针数组 + 按需分配）
#   ⇒ ① 三处锚点按**当前文本**同步；② 撤回形态**适配指针布局**（`g_qds[i]->used`）；
#      ③ 「逐字节回到修复前」已不可能 ⇒ S3 改为「撤回不是空操作」判据，
#         忠实性由 S2 的结构判据承担。
#
# 用法：
#   negctl.py --revert <file>            撤回（rc=1 = 锚点不唯一/找不到 ⇒ 不许静默放行）
#   negctl.py --revert-struct-only <file> 只撤回 ①（**不重建** ⇒ 用来证明[1]静态层有牙）
#   negctl.py --selftest <file>          自证 6 条
# ============================================================
import hashlib
import re
import sys

# 修复前的文件指纹（`git show 1e62beb:runtime/runtime_h3_qpack_dyn.c | sha256sum`）
PRE_FIX_SHA = "38abb9d807df4592937d15234a5a12c5b30c280ad01d94c37e826be445081a59"
# 修复后的文件指纹 —— **M295s1 更新为含按需分配的形态**（当前文件）。
POST_FIX_SHA = "4cde1b1d20724ceb2b554693acd801db1b3ffcd738212d65b9dccc39484d5063"

P_S2 = "    int used;\n"
P_S1 = """    /* M294（缺陷 507 修复）：占用位**已移出本结构**（见 g_qds_used）。
     * 别再往这里加 `int used;` —— 它的存在曾让 `memset(s, 0, sizeof(*s))` 把刚占上的
     * 位置又清回 0，于是并发连接在窗口里**二次认领**同一个会话槽。 */
"""

# ⚠️ M295s1：锚点是**当前**源码文本（M295 在 g_qds_used 之前插入了指针数组与说明）。
P_QDGET = """/* M294（缺陷 507）：**占用位与数据分离**。
 * 为什么必须分离见 bi_qs_open 的说明；一句话：`memset(s)` 不得触碰占用位，
 * 否则「占用」这个动作就不是原子的（它由两步组成，中间有可被并发观察到的空档）。 */
static char    g_qds_used[QD_MAX_SESS];
static qd_sess* qd_get(int64_t id) {
    if (!(id > 0 && id <= QD_MAX_SESS)) return NULL;
    if (!g_qds_used[id - 1]) return NULL;      // 占用位（M294：唯一仲裁者）
    return g_qds[id - 1];                      // 可能为 NULL（占位成功但 calloc 失败）
}
"""
P_QDGET_R = """static qd_sess* qd_get(int64_t id) {
    return (id > 0 && id <= QD_MAX_SESS && g_qds[id - 1] && g_qds[id - 1]->used) ? g_qds[id - 1] : NULL;
}
"""

P_OPEN = """        if (__atomic_exchange_n(&g_qds_used[i], 1, __ATOMIC_SEQ_CST) == 0) {
            qd_sess* s = g_qds[i];
            if (!s) {                      /* M295：首次用到该槽 ⇒ 按需分配 */
                s = (qd_sess*)calloc(1, sizeof(*s));
                if (!s) {
                    /* 分配失败：**必须**把刚占上的位放回（否则槽被永久占住且 qd_get 返回 NULL） */
                    fprintf(stderr, "[h3] QPACK 会话分配失败（%zu 字节/会话，槽 %d）\\n",
                            sizeof(qd_sess), i + 1);
                    __atomic_store_n(&g_qds_used[i], 0, __ATOMIC_SEQ_CST);
                    return px_int(-1);
                }
                g_qds[i] = s;
            }
            memset(s, 0, sizeof(*s));      /* 只清数据，不触碰占用位 */
            s->max_cap = cap;
            s->enc_cap = cap;      // 会话创建即允许该容量（SETTINGS 已收）
            return px_int(i + 1);
        }
"""
P_OPEN_R = """        if (!(g_qds[i] && g_qds[i]->used)) {   /* ← 无锁读（窗口开始） */
            qd_sess* s = g_qds[i];
            if (!s) { s = (qd_sess*)calloc(1, sizeof(*s)); if (!s) return px_int(-1); g_qds[i] = s; }
            memset(s, 0, sizeof(*s));          /* ← 清 0 后再置位：两步之间就是缺陷 507 的窗口 */
            s->used = 1;
            s->max_cap = cap;
            s->enc_cap = cap;
            return px_int(i + 1);
        }
"""

P_CLOSE = """    memset(s, 0, sizeof(*s));
    /* M295：按需分配 ⇒ 归还内存（内存占用跟随**当前并发**，不跟随上限） */
    g_qds[args[0].as.i - 1] = NULL;
    free(s);
    /* M294（缺陷 507）：占用位显式释放（它不在结构体里，memset 清不到它）。
     * 顺序刻意是「先清数据、后放占用位」—— 反之会让别的线程拿到一个尚未清空的会话。
     * M295 补充：`free` 也必须**先于**放位 —— 放位后别的线程可能立刻 calloc 新会话。 */
    __atomic_store_n(&g_qds_used[args[0].as.i - 1], 0, __ATOMIC_SEQ_CST);
    return px_bool(true);
"""
P_CLOSE_R = """    memset(s, 0, sizeof(*s));
    /* 撤回档（M295s1）：**不**归还内存 —— 避免 UAF 掩盖 507 的竞态形状。
     * 占用位已回到结构体里 ⇒ 上面那句 memset 就是释放动作。 */
    return px_bool(true);
"""

# (修复后文本, 撤回文本, 说明)
EDITS = [
    (P_S1, P_S2, "qd_sess 结构体：警示注释 → int used 字段"),
    (P_QDGET, P_QDGET_R, "qd_get：独立占用位 → 结构体字段"),
    (P_OPEN, P_OPEN_R, "bi_qs_open：原子交换 → 读—清—置位"),
    (P_CLOSE, P_CLOSE_R, "bi_qs_close：原子释放 → 删除（memset 即释放）"),
    ("#include <string.h>\n#include <stdatomic.h>\n", "#include <string.h>\n", "撤回 stdatomic 头（修复后才引入）"),
]


def cstrip(t):
    """去掉 C 风格注释（判据要贴着**代码**写 —— 注释里的提及不是「还在用」）。"""
    t = re.sub(r"/\*.*?\*/", "", t, flags=re.S)
    t = re.sub(r"//[^\n]*", "", t)
    return t


def sha_of(path):
    return hashlib.sha256(open(path, "rb").read()).hexdigest()


def do_edits(path, edits, dry=False, quiet=False):
    s = open(path, encoding="utf-8").read()
    for old, new, desc in edits:
        n = s.count(old)
        if n != 1:
            if not quiet:      # S4/S5 的「预期失败」不往 stderr 泼噪声（否则读者以为脚本坏了）
                print("❌ 锚点命中 %d 次（期望恰好 1）：%s" % (n, desc), file=sys.stderr)
            return 1
        s = s.replace(old, new, 1)
    if not dry:
        open(path, "w", encoding="utf-8").write(s)
    return 0


def main():
    if len(sys.argv) < 3:
        print(__doc__ or "用法见文件头", file=sys.stderr)
        return 2
    mode, path = sys.argv[1], sys.argv[2]

    if mode == "--revert":
        return do_edits(path, EDITS)

    if mode == "--revert-struct-only":
        return do_edits(path, EDITS[:1])

    if mode == "--selftest":
        rc = 0
        s = open(path, encoding="utf-8").read()
        # S1：5 处锚点各恰好 1 次
        for old, _n, desc in EDITS:
            n = s.count(old)
            if n == 1:
                print("  ✅ S1 锚点唯一：%s" % desc)
            else:
                print("  ❌ S1 锚点命中 %d 次（期望 1）：%s" % (n, desc))
                rc = 1
        # S2：撤回后结构（在内存里做，不落盘）
        t = s
        okall = True
        for old, new, _d in EDITS:
            if t.count(old) != 1:
                okall = False
                break
            t = t.replace(old, new, 1)
        tc = cstrip(t)
        def _c(x):
            return tc.count(x)
        if (okall and _c("g_qds_used") == 0 and _c("int used;") == 1
                and _c("g_qds[id - 1]->used") == 1 and _c("__atomic_exchange_n") == 0):
            print("  ✅ S2 撤回后结构（**代码行**）：g_qds_used=0 · int used;=1 · g_qds[id-1]->used=1 · 无原子交换")
        else:
            print("  ❌ S2 撤回后结构不符（代码行：g_qds_used=%d · int used;=%d · ->used=%d · 原子交换=%d）"
                  % (_c("g_qds_used"), _c("int used;"),
                     _c("g_qds[id - 1]->used"), _c("__atomic_exchange_n")))
            rc = 1
        # S3：撤回后与修复前**逐字节相同**（仅当当前文件仍是修复后原样时；否则记 ℹ️ 不判红）
        cur = sha_of(path)
        tsha = hashlib.sha256(t.encode("utf-8")).hexdigest()
        # ⚠️ M295s1：逐字节回到「修复前」已**不可能** —— M295 又改了同一文件的数据布局
        #   （`g_qds` 静态数组 → 指针数组 + 按需分配）⇒ 本项改为「撤回**不是空操作**」判据，
        #   忠实性由 S2 的**结构**判据承担（g_qds_used=0 · int used;=1 · 无原子交换）。
        if cur != POST_FIX_SHA:
            print("  ℹ️ S3 跳过（文件已演进：当前 %s… ≠ 记录 %s…）—— 只做结构判据" % (cur[:16], POST_FIX_SHA[:16]))
        elif tsha != cur:
            print("  ✅ S3 撤回**不是空操作**（撤回后 %s… ≠ 修复后 %s…）" % (tsha[:16], cur[:16]))
        else:
            print("  ❌ S3 撤回后与修复后逐字节相同 ⇒ 撤回是空操作")
            rc = 1
        # S4：判据自伤 —— 锚点被改坏时 **必须** rc≠0（不许静默放行）
        import tempfile
        import os
        fd, tp = tempfile.mkstemp(suffix=".c")
        os.close(fd)
        open(tp, "w", encoding="utf-8").write(s.replace("if (__atomic_exchange_n(&g_qds_used[i], 1, __ATOMIC_SEQ_CST) == 0) {",
                                                        "if (__atomic_exchange_n(&g_qds_used[i], 1, __ATOMIC_SEQ_CST) == 0) { /*x*/"))
        r = do_edits(tp, EDITS, quiet=True)
        if r != 0:
            print("  ✅ S4 判据自伤：锚点被改坏 ⇒ rc=%d（响亮，不静默）" % r)
        else:
            print("  ❌ S4 锚点被改坏却仍 rc=0 ⇒ 判据无牙")
            rc = 1
        # S5：撤回后**再撤回必须失败**（证明撤回到位、不是空操作）
        fd, tp2 = tempfile.mkstemp(suffix=".c")
        os.close(fd)
        open(tp2, "w", encoding="utf-8").write(t)
        r2 = do_edits(tp2, EDITS, quiet=True)
        if r2 != 0:
            print("  ✅ S5 已撤回的文件上再撤回 ⇒ rc=%d（不是空操作）" % r2)
        else:
            print("  ❌ 已撤回的文件上再撤回竟 rc=0 ⇒ 撤回是空操作")
            rc = 1
        os.unlink(tp)
        os.unlink(tp2)
        # S6：规模锚点（防「文件被换掉 ⇒ 锚点全过期但脚本说 OK」）
        nl = s.count("\n") + 1
        if nl >= 1000 and s.count("bi_qs_open") >= 2:
            print("  ✅ S6 规模锚点：%d 行（≥1000）· bi_qs_open 出现 %d 次" % (nl, s.count("bi_qs_open")))
        else:
            print("  ❌ S6 规模锚点不符（%d 行 · bi_qs_open %d 次）" % (nl, s.count("bi_qs_open")))
            rc = 1
        print("SELFTEST-%s" % ("OK" if rc == 0 else "FAIL"))
        return rc

    print("未知模式：%s" % mode, file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main())
