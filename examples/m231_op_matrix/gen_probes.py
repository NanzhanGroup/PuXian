#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M231 门 · 探针与期望表生成器（「运算符 × 逐类型组合」全量矩阵）

为什么是这个面
--------------
M199 量过 native **函数面**的「逐位置 × 错类型」，M226 量过**方法面**的同款，
M227 量过「同名两门」（函数 ⇄ 方法），M228 量过「三个门」（运算符 ⇄ 函数 ⇄ 方法，只针对 in），
M229 量过 tuple/result 接收者，M230 量过索引/切片族 ——
而 **`(运算符 × 左类型 × 右类型)` 这一整个矩阵从来没有被清单级度量过**。
本轮把它做成可全量度量。实测照出 **缺陷 346**（见 verify.sh 头注）。

派生纪律（M212 缺陷 288）
------------------------
运算符清单**不手抄**：从 `selfhost/parser.px` 的 `chk_op("…")` **派生**，
`not in` 由 `chk_op("not") and chk2("in")` 合成。派生集合 ⇄ 本文件使用的清单
**双向一致**由 verify.sh 第 [1] 层断言（少一个 = 探针面静默变窄；多一个 = 悬空条目）。

期望表 MODEL.tsv 的来历（如实登记）
----------------------------------
规则由 M231 定稿时**按语言语义**写定（每条带依据名），**不是**从实测抄出来的；
verify.sh 第 [4] 层做**双向**核对（实测有、表没有 ⇒ 漏登记判红；表有、实测没有 ⇒ 过期判红）。
它的价值是**回归守卫**：日后任何一次行为漂移都会让它变红。

用法
----
  gen_probes.py --root R --gen        生成 cases.tsv + drv.px + MODEL.tsv
"""
import argparse, os, re, sys

# 短路族：`and` / `or` / `??` / `|>` —— **不在本轮矩阵内**，理由：
#   · `and`/`or` 对任意类型做**真值性转换**（不报类型错），矩阵判据（响亮性）对这些
#     运算符退化为恒 VAL ⇒ 无判别力；
#   · `??` 只对 `null` 走右侧，同样是真值性语义；
#   · `|>` 右侧必须是函数（另一个面）。
#   ⇒ 四者单列在 verify.sh 的「覆盖边界」里如实登记，本文件不生成它们的探针。
SHORT_CIRCUIT = {"and", "or", "??", "|>"}

# 参与矩阵的二元运算符的**顺序基准**（由 parser 派生后按此顺序排列，保证可复现）
ORDER = ["+", "-", "*", "/", "//", "%", "**",
         "==", "!=", "<", "<=", ">", ">=",
         "&", "|", "^", "<<", ">>", ">>>",
         "in", "not in"]

TYPES = ["int", "float", "str", "bool", "list", "dict", "null"]
LITERAL = {
    "int": "1", "float": "1.5", "str": '"s"', "bool": "true",
    "list": "[1]", "dict": '{"a": 1}', "null": "null",
}

# 每运算符的探针类型对：全 7×7 会到 21×49=1029 例（三轨 3087 次执行 ⇒ 门太重）。
# 取「够发现模式」的 14 对：含全部同类型对 + 全部与 int 的异类型对 + 与 str 的异类型对。
PAIRS = [("int", "int"), ("float", "float"), ("str", "str"), ("bool", "bool"),
         ("list", "list"), ("dict", "dict"), ("null", "null"),
         ("int", "float"), ("float", "int"),
         ("str", "int"), ("int", "str"),
         ("null", "int"), ("int", "null"),
         ("dict", "int")]

UN_OPS = [("-", "neg"), ("not", "not"), ("~", "bnot")]


def derive_bin_ops(root):
    """从 parser.px 派生中缀运算符集合（M212 纪律：判据的期望集必须源码派生）"""
    src = open(os.path.join(root, "selfhost", "parser.px"), encoding="utf-8").read()
    got = []
    for m in re.finditer(r'chk_op\("([^"]+)"\)', src):
        op = m.group(1)
        if op in SHORT_CIRCUIT or op == "not":   # `not` 中缀只作 `not in` 的一部分
            continue
        if op not in got:
            got.append(op)
    # `not in` 由 chk_op("not") and chk2("in") 合成（parser.px 的 parse_comparison）
    if 'chk_op("not") and chk2("in")' in src:
        got.append("not in")
    return got


def check_order(derived):
    """派生集合 ⇄ ORDER 双向一致"""
    a, b = set(derived), set(ORDER)
    return sorted(a - b), sorted(b - a)


def tag_of(op, lt, rt):
    m = {"+": "P", "-": "M", "*": "S", "/": "D", "//": "DD", "%": "PC", "**": "SS",
         "==": "EE", "!=": "NE", "<": "L", "<=": "LE", ">": "G", ">=": "GE",
         "&": "A", "|": "O", "^": "X", "<<": "LL", ">>": "GG", ">>>": "GGG",
         "in": "in", "not in": "notin"}
    return "b_%s_%s_%s" % (m.get(op, op), lt, rt)


def expect_kind(op, lt, rt):
    """按语言语义给出「响亮性」期望 + 依据名（MODEL.tsv 的第三列）"""
    NUM = ("int", "float")
    # 相等族：任意类型对都给值（跨类型 = 不相等）—— 这是语言定义（`1 == "s"` 是 false 而非错误）
    if op in ("==", "!="):
        return "VAL", "eq-cross-type-value"
    # 序比较族：**同类型**或**两侧皆为数值**才给值，跨类型响亮。
    # ⚠️ 现状登记（本轮**不改**，见 verify.sh 覆盖边界）：`list`/`dict`/`bool`/`null`
    #   的**同类型** `</<=/>/>=` 静默给 `false`（Python 是 TypeError）—— 三轨一致、
    #   不属本轮「分叉」族；锁进基线以免日后静默漂移，口径分歧另立候选。
    if op in ("<", "<=", ">", ">="):
        if lt == rt or (lt in NUM and rt in NUM):
            return "VAL", "ord-same-type-or-numeric"
        return "ERR", "ord-cross-type"
    # 成员族：仅「左侧是元素、右侧是容器」的形状给值
    if op in ("in", "not in"):
        if (lt, rt) in (("str", "str"), ("list", "list")):
            return "VAL", "member-iterable"
        return "ERR", "member-noniterable"
    # 位运算族：仅 int × int
    if op in ("&", "|", "^", "<<", ">>", ">>>"):
        return ("VAL", "bit-int-only") if (lt, rt) == ("int", "int") else ("ERR", "bit-int-only")
    # 算术族
    if op in ("+", "-", "*", "/", "//", "%", "**"):
        if lt in NUM and rt in NUM:
            return "VAL", "arith-numeric"
        if op == "+" and lt == "str" and rt == "str":
            return "VAL", "arith-str-concat"
        if op == "+" and lt == "list" and rt == "list":
            return "VAL", "arith-list-concat"
        # ⚠️ 缺陷 346 的位置：`str * int` 是**唯一**合法「非数值」算术 ⇒ 必须静默给重复
        if op == "*" and lt == "str" and rt == "int":
            return "VAL", "arith-str-repeat"
        return "ERR", "arith-undefined-pair"
    raise SystemExit("unknown op: %s" % op)


def expect_kind_un(op, t):
    if op == "-":
        return ("VAL", "neg-numeric") if t in ("int", "float") else ("ERR", "neg-numeric")
    if op == "not":
        return "VAL", "not-truthiness-any"
    if op == "~":
        return ("VAL", "bnot-int-only") if t == "int" else ("ERR", "bnot-int-only")
    raise SystemExit("unknown unary op: %s" % op)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--gen", action="store_true")
    ap.add_argument("--out", default=None)
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    d = a.out or os.path.join(root, "examples", "m231_op_matrix")

    derived = derive_bin_ops(root)
    only_derived, only_order = check_order(derived)
    if not a.gen:
        print("派生二元运算符 %d 个：%s" % (len(derived), " ".join(derived)))
        if only_derived or only_order:
            print("⚠️ 派生 ⇄ ORDER 不一致：派生独有 %s · ORDER 独有 %s" % (only_derived, only_order))
            return 2
        print("✅ 派生 ⇄ ORDER 双向一致")
        return 0

    cases = []      # (tag, expr, kind, basis)
    for op in ORDER:
        for lt, rt in PAIRS:
            e = "%s %s %s" % (LITERAL[lt], op, LITERAL[rt])
            k, b = expect_kind(op, lt, rt)
            cases.append((tag_of(op, lt, rt), e, k, b))
    for op, en in UN_OPS:
        for t in TYPES:
            e = "%s %s" % (op, LITERAL[t])
            k, b = expect_kind_un(op, t)
            cases.append(("u_%s_%s" % (en, t), e, k, b))

    os.makedirs(d, exist_ok=True)
    src = ["# M231 矩阵探针驱动器（自动生成 · 勿手改）\n"]
    for i, (tag, e, k, b) in enumerate(cases):
        src.append('def t%d():\n    print("%s=" + str(%s))\n' % (i, tag, e))
    src.append('def main():\n')
    src.append('    let c = env("M231_CASE")\n')
    src.append('    if c == null:\n        print("usage: M231_CASE=<idx>")\n        return\n')
    src.append('    let i = c\n')
    for i, (tag, e, k, b) in enumerate(cases):
        kw = "if" if i == 0 else "elif"
        src.append('    %s i == "%d":\n        t%d()\n' % (kw, i, i))
    src.append('    else:\n        print("bad idx")\n')
    open(os.path.join(d, "drv.px"), "w").write("".join(src))
    with open(os.path.join(d, "cases.tsv"), "w") as f:
        f.write("idx\ttag\texpr\tkind\tbasis\n")
        for i, (tag, e, k, b) in enumerate(cases):
            f.write("%d\t%s\t%s\t%s\t%s\n" % (i, tag, e, k, b))
    # MODEL.tsv 只留 tag/kind/basis（expr 已在 cases.tsv）
    with open(os.path.join(d, "MODEL.tsv"), "w") as f:
        f.write("tag\tkind\tbasis\n")
        for tag, e, k, b in cases:
            f.write("%s\t%s\t%s\n" % (tag, k, b))
    print("M231-GEN-OK cases=%d（%d 二元 × %d 对 + %d 一元）二元运算符 %d 个"
          % (len(cases), len(ORDER), len(PAIRS), len(UN_OPS) * len(TYPES), len(ORDER)))
    return 0


sys.exit(main())
