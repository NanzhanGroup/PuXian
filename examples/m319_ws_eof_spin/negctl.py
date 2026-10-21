#!/usr/bin/env python3
"""m319 打桩器 —— 把 ws_read_exact 的读失败分支在三种形态之间切换。

用法：
  negctl.py --snapshot <快照文件>     # 备份 runtime_ws.c（门开头一次）
  negctl.py --orig     <快照文件>     # 忠实撤回缺陷 552 的修复（恢复原三行）
  negctl.py --inject   <快照文件>     # 等价放大：失败分支改成无条件 continue
  negctl.py --restore  <快照文件>     # 从快照逐字节还原

三种形态一一对应：
  fixed（仓库里）= `if (r < 0) return -1;`  —— 不重试
  orig（缺陷态）= `if (r < 0) { if (errno == EINTR) continue; return -1; }`
  inject（放大） = `if (r < 0) continue;`    —— 等价于「errno 判据恒真」，确定性自旋

为什么需要 inject：orig 的自旋依赖「该线程 errno 残留 EINTR」，是**概率性**的
（实测 3/4）。判据层需要一条**确定性**的红来做灵敏度证明，否则「负控没红」
既可能是修好了、也可能是这次没抽中（M289 的教训：不要把概率当结论）。
"""
import sys

NEW_BLOCK = """        if (r == 0) return -1;   // EOF（对端正常关闭）
        if (r < 0) return -1;    // 错误（含已关闭/已释放）—— 不重试，见上方 M319 说明"""

ORIG_BLOCK = """        if (r == 0) return -1;
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }"""

INJECT_BLOCK = """        if (r == 0) return -1;
        if (r < 0) continue;"""

# ── 层 1（errno 契约）的四处在位形态 → 修前形态 ─────────────────────────
# ⚠️ 为什么负控必须**同时**撤回层 1：实测发现单撤层 2 时自旋不复现 ——
#   因为 px_conn_read 已显式把 errno 设成 ECONNRESET，`errno == EINTR` 自然为假。
#   这说明两层是**纵深防御**（各自独立生效），但也意味着「忠实撤回」必须回到修前的**组合**，
#   否则负控测的就不是缺陷本身。M209 的「判据回放」同款纪律。
ERRNO_CASES = [
    ("""    if (!px_conn_acquire(c)) {
        errno = ECONNRESET;   // M319（缺陷 552）：连接已关闭/已释放 —— 显式、不留残留值
        return -1;
    }""", "    if (!px_conn_acquire(c)) return -1;"),
    ("""    if (!px_conn_acquire(c)) {
        errno = EPIPE;        // M319（缺陷 552 同族）：失败出口同样显式给原因
        return -1;
    }""", "    if (!px_conn_acquire(c)) return -1;"),
    ("    if (c->closed) { errno = ECONNRESET; return -1; }", "    if (c->closed) return -1;"),
    ("    if (c->closed) { errno = EPIPE; return -1; }   // M319（缺陷 552 同族）", "    if (c->closed) return -1;"),
]


def target(root):
    return root + "/runtime/runtime_ws.c"


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    mode, snap = sys.argv[1], sys.argv[2]
    root = sys.argv[3] if len(sys.argv) > 3 else "."
    p = target(root)
    if mode == "--snapshot":
        open(snap, "wb").write(open(p, "rb").read())
        open(snap + ".c", "wb").write(open(root + "/runtime/runtime.c", "rb").read())
        print("SNAPSHOT %s (+runtime.c)" % p)
        return 0
    if mode == "--restore":
        open(p, "wb").write(open(snap, "rb").read())
        import os
        if os.path.exists(snap + ".c"):
            open(root + "/runtime/runtime.c", "wb").write(open(snap + ".c", "rb").read())
        print("RESTORED")
        return 0
    cur = open(p).read()
    if mode == "--orig-instr":
        # 「忠实撤回 + 还原现场 I/O 活动」
        #   先撤两层（orig-full），再在 acquire 失败分支加一条**有界诊断打印**
        #   （前 5 次 + 每 100 万次 —— 与首次定位缺陷时的形态逐字相同）。
        #   为什么必须要这一条：实测**不加诊断时 0/8 复现、加了 3/4** ——
        #   因为 errno 的残留值只能由**系统调用被打断**产生，而打印本身就是那次
        #   write；晨曦现场（px-access 日志、GC 信号、大量小 write）正好处在这个状态。
        #   这是**还原现场**，不是人为制造缺陷：判据要证的是「errno 判据 + 残留 EINTR ⇒ 自旋」
        #   这条因果链，而不是某个特定 errno 值。
        c = open(root + "/runtime/runtime.c").read()
        n1 = 0
        for new, old in ERRNO_CASES:
            if c.count(new) == 1:
                c = c.replace(new, old)
                n1 += 1
        anchor = "ssize_t px_conn_read(PxConn* c, void* buf, size_t n) {\n    if (!px_conn_acquire(c)) return -1;"
        instr = ("ssize_t px_conn_read(PxConn* c, void* buf, size_t n) {\n"
                 "    if (!px_conn_acquire(c)) {\n"
                 "        static __thread long m319_diag = 0;\n"
                 "        m319_diag++;\n"
                 "        if (m319_diag <= 5 || (m319_diag % 1000000) == 0)\n"
                 "            fprintf(stderr, \"[M319-DIAG] acquire-fail #%ld errno=%d\\n\", m319_diag, errno);\n"
                 "        return -1;\n"
                 "    }")
        if c.count(anchor) != 1:
            print("ANCHOR-MISS read-anchor=%d" % c.count(anchor))
            return 3
        c = c.replace(anchor, instr)
        open(root + "/runtime/runtime.c", "w").write(c)
        cur2 = open(p).read()
        n2 = 0
        if cur2.count(ORIG_BLOCK) != 1 and cur2.count(NEW_BLOCK) == 1:
            open(p, "w").write(cur2.replace(NEW_BLOCK, ORIG_BLOCK))
            n2 = 1
        print("ORIG-INSTR layer1=%d layer2=%d" % (n1, n2))
        return 0 if (n1 == 4 and n2 == 1) else 4
    if mode == "--orig-full":
        # 忠实撤回：层 2（ws_read_exact）+ 层 1（四处 errno 设置）
        c = open(root + "/runtime/runtime.c").read()
        n1 = 0
        for new, old in ERRNO_CASES:
            if c.count(new) == 1:
                c = c.replace(new, old)
                n1 += 1
        open(root + "/runtime/runtime.c", "w").write(c)
        cur2 = open(p).read()
        n2 = 0
        if cur2.count(ORIG_BLOCK) != 1 and cur2.count(NEW_BLOCK) == 1:
            open(p, "w").write(cur2.replace(NEW_BLOCK, ORIG_BLOCK))
            n2 = 1
        print("ORIG-FULL layer1=%d layer2=%d" % (n1, n2))
        return 0 if n1 == 4 else 4
    if mode == "--orig":
        if cur.count(ORIG_BLOCK) == 1 and cur.count(NEW_BLOCK) == 0:
            print("ALREADY-ORIG")
            return 0
        if cur.count(NEW_BLOCK) != 1:
            print("ANCHOR-MISS new_block=%d" % cur.count(NEW_BLOCK))
            return 3
        open(p, "w").write(cur.replace(NEW_BLOCK, ORIG_BLOCK))
        print("ORIG")
        return 0
    if mode == "--inject":
        if cur.count(INJECT_BLOCK) == 1 and cur.count(NEW_BLOCK) == 0:
            print("ALREADY-INJECT")
            return 0
        if cur.count(NEW_BLOCK) != 1:
            print("ANCHOR-MISS new_block=%d" % cur.count(NEW_BLOCK))
            return 3
        open(p, "w").write(cur.replace(NEW_BLOCK, INJECT_BLOCK))
        print("INJECT")
        return 0
    print("unknown mode: %s" % mode)
    return 2


if __name__ == "__main__":
    sys.exit(main())
