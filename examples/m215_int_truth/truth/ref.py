#!/usr/bin/env python3
# ============================================================
# M215 门 · 真值参考之一：**Python 任意精度**（定义式，最直白）
# ------------------------------------------------------------
# 输出 = corpus.px 的**完整期望 stdout**（含段标记）—— 门直接 `cmp` 即可。
# 于是本文件同时是"真值"与"期望输出契约"的唯一定义之一（另一份是 truth/main.go，
# 两者必须逐字节一致：verify.sh 会先互校、再拿去比三轨）。
#
# 欧几里得除法的定义（docs/spec.md §算术 / 速查表事实 239）：
#     a = q*b + r,   0 <= r < |b|
# Python 的 `n % abs(d)` 直接给出**非负模** ⇒ 就是欧几里得余数本身；
# 再由 q = (n - r) / d（任意精度）求商。这些**都不依赖任何 int64 溢出行为**。
#
# 唯一特判：`n == INT64_MIN 且 d == -1` 的真商 = +2^63 **超出 int64**
#   ⇒ 按文档口径回绕（q = INT64_MIN, r = 0）—— 与一元负号在 INT64_MIN 的回绕同族。
#
# 用法：python3 ref.py domain.txt
# ============================================================
import sys

MIN64 = -(2 ** 63)
MAX64 = 2 ** 63 - 1


def euclid(n, d):
    if n == MIN64 and d == -1:
        return MIN64, 0, False          # 文档口径：回绕（真商不可表示）
    r = n % abs(d)                      # 非负模 ⇒ 欧几里得余数
    q = (n - r) // d                    # 精确整除
    assert 0 <= r < abs(d), (n, d, q, r)
    assert q * d + r == n, (n, d, q, r)  # 公理自证（任意精度，不会溢出）
    assert MIN64 <= q <= MAX64, ("真商不可表示（不应发生）", n, d, q)
    return q, r, True


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
    # ---------- 段 A：`//` 与 `%` ----------
    out.append("== A-BEGIN ==")
    for n, d in pairs:
        q, r, _ = euclid(n, d)
        out.append("n=%d d=%d q=%d r=%d" % (n, d, q, r))
    out.append("== A-END ==")

    # ---------- 段 B：陷阱回归 ----------
    mn, mx = MIN64, MAX64
    t = []
    t.append("trap1=%d" % euclid(mn, -1)[0])
    t.append("trap2=%d" % euclid(mn, -1)[1])
    t.append("trap3=%d" % euclid(-7, mn)[0])
    t.append("trap4=%d" % euclid(-7, mn)[1])
    t.append("trap5=%d" % euclid(mx, -1)[0])
    out.append("== B-BEGIN ==")
    out.extend(t)
    out.append("== B-END ==")

    # ---------- 段 D：比较真值（6 个运算符 × 702 对） ----------
    # 整数比较是**精确**的（不存在浮点舍入位）—— 这正是缺陷 307 的判据面。
    out.append("== D-BEGIN ==")
    for n, d in pairs:
        out.append("n=%d d=%d lt=%s le=%s gt=%s ge=%s eq=%s ne=%s" % (
            n, d, b(n < d), b(n <= d), b(n > d), b(n >= d), b(n == d), b(n != d)))
    out.append("== D-END ==")

    # ---------- 段 C：不变量自断言 ----------
    # 判据（欧几里得余数定义）：0 <= r < |d|。边界上 q*b 会溢出，故代数公理不在此断言。
    bad = 0
    for n, d in pairs:
        q, r, _ = euclid(n, d)
        if not (0 <= r < abs(d)):
            bad += 1
    out.append("== C-BEGIN ==")
    out.append("== C-END ==")
    out.append("M215-INV-BAD=%d CNT=%d" % (bad, len(pairs)))
    out.append("M215-DONE")
    sys.stdout.write("\n".join(out) + "\n")
    print("ref.py 行数=%d 对=%d 公理自证=OK" % (len(out), len(pairs)), file=sys.stderr)


if __name__ == "__main__":
    main()
