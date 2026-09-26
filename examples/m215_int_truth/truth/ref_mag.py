#!/usr/bin/env python3
# ============================================================
# M215 门 · 真值参考之二（**与 ref.py 算法不同构**，且**不需要 Go**）
# ------------------------------------------------------------
# 为什么还要一份（CI 无 Go 步骤 —— 见 .github/workflows/ci.yml 里 m136/m138 的同款处置）：
#   ref.py 是"定义式"（`n % abs(d)` 得非负余数，再精确除）—— 最直白，但与被测实现
#   **同属"取模再除"的思路**。本文件改成**量级域 + 符号分支**推导（与 truth/main.go
#   的 Go 实现同算法、跨语言互证）：
#       ad = |d| ;  qt = |n| // ad ;  rm = |n| % ad
#       r  = rm ;  若 n<0 且 rm != 0  ⇒  r = ad - rm        （保证 0 <= r < ad）
#       q  由 4 个符号分支给出（|q| 单独承载，最后判可表示性）
#   两套算法都对 ⇒ 真值源可信；只有一套对 ⇒ 立刻暴露（判据自身不可信 = 比没门更糟）。
#
# 输出与 ref.py **逐字节相同**（同一份期望 stdout）。
# 用法：python3 ref_mag.py domain.txt
# ============================================================
import sys

MIN64 = -(2 ** 63)
MAX64 = 2 ** 63 - 1
C_MAXPOS = 9223372036854775807     # 2^63-1
C_MAXNEG = 9223372036854775808     # 2^63（= |INT64_MIN|）


def euclid_mag(n, d):
    """量级域推导；repOK=False 表示真商不在 int64 内（唯一：MIN / -1）"""
    an, ad = abs(n), abs(d)
    qt, rm = an // ad, an % ad
    r = rm
    if n < 0 and rm != 0:
        r = ad - rm
    if d > 0:
        if n >= 0:
            qu, qpos = qt, True
        else:
            qpos = False
            qu = qt if rm == 0 else qt + 1
    else:
        if n >= 0:
            qu, qpos = qt, False
        else:
            qpos = True
            qu = qt if rm == 0 else qt + 1
    if qpos:
        if qu > C_MAXPOS:
            return 0, r, False
        return qu, r, True
    if qu > C_MAXNEG:
        return 0, r, False
    if qu == C_MAXNEG:
        return MIN64, r, True
    return -qu, r, True


def euclid_doc(n, d):
    q, r, ok = euclid_mag(n, d)
    if not ok:
        return MIN64, 0
    return q, r


def b(x):
    return "true" if x else "false"


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "domain.txt"
    pairs = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            n, d = (int(x) for x in line.split())
            assert d != 0, line
            pairs.append((n, d))

    out = []
    out.append("== A-BEGIN ==")
    for n, d in pairs:
        q, r = euclid_doc(n, d)
        out.append("n=%d d=%d q=%d r=%d" % (n, d, q, r))
    out.append("== A-END ==")

    out.append("== B-BEGIN ==")
    out.append("trap1=%d" % euclid_doc(MIN64, -1)[0])
    out.append("trap2=%d" % euclid_doc(MIN64, -1)[1])
    out.append("trap3=%d" % euclid_doc(-7, MIN64)[0])
    out.append("trap4=%d" % euclid_doc(-7, MIN64)[1])
    out.append("trap5=%d" % euclid_doc(MAX64, -1)[0])
    out.append("== B-END ==")

    out.append("== D-BEGIN ==")
    for n, d in pairs:
        out.append("n=%d d=%d lt=%s le=%s gt=%s ge=%s eq=%s ne=%s" % (
            n, d, b(n < d), b(n <= d), b(n > d), b(n >= d), b(n == d), b(n != d)))
    out.append("== D-END ==")

    bad = 0
    for n, d in pairs:
        _, r = euclid_doc(n, d)
        if not (0 <= r < abs(d)):
            bad += 1
    out.append("== C-BEGIN ==")
    out.append("== C-END ==")
    out.append("M215-INV-BAD=%d CNT=%d" % (bad, len(pairs)))
    out.append("M215-DONE")
    sys.stdout.write("\n".join(out) + "\n")
    print("ref_mag.py 行数=%d 对=%d 不变量违规=%d" % (len(out), len(pairs), bad), file=sys.stderr)


if __name__ == "__main__":
    main()
