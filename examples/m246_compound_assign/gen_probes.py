#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M246 门 · 探针与期望表生成器（「复合赋值 × 逐类型组合」全量矩阵 · **Var 目标**）

为什么是这个面
--------------
M231 量过 `(二元运算符 × 左类型 × 右类型)` 的**表达式**矩阵；M204 量过**复合赋值**
在 `Index` / `Field` 目标上的三轨同一真相（缺陷 245/246）。
而 **`x op= y`（Var 目标）的同一矩阵从来没有被清单级度量过** ——
M204 的门头只写了一句口头断言：「`Var` 目标那一支走 `cg_assign_op_local`，是对的」。
本轮把**这句断言变成判据**。

面 = 「13 个复合赋值运算符 × 14 个类型对」× **两个形态**：
  · 形态 A（被测）：`var x = <L>;  x <cmpop>= <R>;  print(x)`
  · 形态 B（对照）：`var x = <L>;  x = x <op> <R>;  print(x)`
⇒ 判据②是**独立真值**（不依赖三轨）：两个形态必须给出**同一个结果**。
   本条能把「复合赋值是二元运算的语法糖」这个**语义承诺**变成可判定的东西。

派生纪律（M212 缺陷 288）
------------------------
运算符清单**不手抄**：从 `selfhost/parser.px` 的复合赋值分支
（`elif k == "+=": op = "Plus"` …）**派生**（运算符文本）。
派生集合 ⇄ 本文件的 `CMP_OPS` **双向一致**由 verify.sh 第 [1] 层断言。

用法
----
  gen_probes.py --root R            # 只校验派生（不写文件）
  gen_probes.py --root R --gen      # 生成 cases.tsv + drv.px + MODEL.tsv
"""
import argparse, os, re, sys

# 运算符顺序基准（由 parser 派生后按此序排列 ⇒ 可复现）
ORDER = ["+=", "-=", "*=", "/=", "//=", "%=", "**=",
         "&=", "|=", "^=", "<<=", ">>=", ">>>="]

# 复合赋值 → 对应二元运算符（对照形态用）
BIN_OF = {"+=": "+", "-=": "-", "*=": "*", "/=": "/", "//=": "//", "%=": "%",
          "**=": "**", "&=": "&", "|=": "|", "^=": "^",
          "<<=": "<<", ">>=": ">>", ">>>=": ">>>"}

# op 名（tag 用）—— 与 parser.px 的 op 字符串一致
OPNAME = {"+=": "Plus", "-=": "Minus", "*=": "Star", "/=": "Slash",
          "//=": "IntDiv", "%=": "Mod", "**=": "Pow",
          "&=": "BitAnd", "|=": "BitOr", "^=": "BitXor",
          "<<=": "Shl", ">>=": "Shr", ">>>=": "ShrU"}

TYPES = ["int", "float", "str", "bool", "list", "dict", "null"]
LITERAL = {
    "int": "1", "float": "1.5", "str": '"s"', "bool": "true",
    "list": "[1]", "dict": '{"a": 1}', "null": "null",
}

# 类型对：与 M231 同口径（同类型对 + 与 int 的异类型对 + 与 str 的异类型对）
PAIRS = [("int", "int"), ("float", "float"), ("str", "str"), ("bool", "bool"),
         ("list", "list"), ("dict", "dict"), ("null", "null"),
         ("int", "float"), ("float", "int"),
         ("str", "int"), ("int", "str"),
         ("null", "int"), ("int", "null"),
         ("dict", "int")]


def derive_cmp_ops(root):
    """从 parser.px 派生复合赋值运算符集合（源码派生 · 不手抄）"""
    src = open(os.path.join(root, "selfhost", "parser.px"), encoding="utf-8").read()
    got = []
    # 形如：  elif k == "+=":        /   elif k == "<<=":
    for m in re.finditer(r'elif\s+k\s*==\s*"([^"]+)"\s*:', src):
        op = m.group(1)
        if op.endswith("=") and op != "==" and op != "<-" and op not in got:
            got.append(op)
    return got


def check_order(derived):
    a, b = set(derived), set(ORDER)
    return sorted(a - b), sorted(b - a)


def tag_of(cmpop, lt, rt, ref=False):
    t = "c_%s_%s_%s" % (OPNAME[cmpop], lt, rt)
    return t + "_ref" if ref else t


def expect_kind(cmpop, lt, rt):
    """按语言语义给出「响亮性」期望 + 依据名。

    ⚠️ 复合赋值 `x op= y` 的语义**定义为** `x = x <op> y`（本轮要判定的正是这条）⇒
    期望直接**复用 M231 的二元语义规则**（同一份语言定义，不另写第二份）。
    """
    op = BIN_OF[cmpop]
    NUM = ("int", "float")
    if op in ("&", "|", "^", "<<", ">>", ">>>"):
        return ("VAL", "bit-int-only") if (lt, rt) == ("int", "int") else ("ERR", "bit-int-only")
    if op in ("+", "-", "*", "/", "//", "%", "**"):
        if lt in NUM and rt in NUM:
            return "VAL", "arith-numeric"
        if op == "+" and lt == "str" and rt == "str":
            return "VAL", "arith-str-concat"
        if op == "+" and lt == "list" and rt == "list":
            return "VAL", "arith-list-concat"
        if op == "*" and lt == "str" and rt == "int":
            return "VAL", "arith-str-repeat"
        return "ERR", "arith-undefined-pair"
    raise SystemExit("unknown cmp op: %s" % cmpop)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--gen", action="store_true")
    ap.add_argument("--out", default=None)
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    d = a.out or os.path.join(root, "examples", "m246_compound_assign")

    derived = derive_cmp_ops(root)
    only_derived, only_order = check_order(derived)
    if not a.gen:
        print("派生复合赋值运算符 %d 个：%s" % (len(derived), " ".join(derived)))
        if only_derived or only_order:
            print("⚠️ 派生 ⇄ ORDER 不一致：派生独有 %s · ORDER 独有 %s" % (only_derived, only_order))
            return 2
        print("✅ 派生 ⇄ ORDER 双向一致")
        return 0

    cases = []   # (tag, lines(list), kind, basis)
    for op in ORDER:
        for lt, rt in PAIRS:
            k, b = expect_kind(op, lt, rt)
            cases.append((tag_of(op, lt, rt),
                          ["var x = %s" % LITERAL[lt], "x %s %s" % (op, LITERAL[rt])],
                          k, b))
            cases.append((tag_of(op, lt, rt, ref=True),
                          ["var x = %s" % LITERAL[lt],
                           "x = x %s %s" % (BIN_OF[op], LITERAL[rt])],
                          k, b + "-ref"))

    os.makedirs(d, exist_ok=True)
    # ⚠️ 元素**不含尾部换行**，统一由 join 负责 —— 否则 join 会再插一个 ⇒ 每个
    #    `def`/`elif` 之间多出空行（缩进语言里虽可容忍，但与 M231 的产物形状不一致，
    #    且会让「产物逐字节一致」类比对失效）。
    src = ["# M246 复合赋值矩阵探针驱动器（自动生成 · 勿手改）"]
    for i, (tag, lines, k, b) in enumerate(cases):
        body = "\n".join("    " + L for L in lines)
        src.append('def t%d():\n%s\n    print("%s=" + str(x))' % (i, body, tag))
    # ⚠️ 参数**经环境变量**传（`env("M246_CASE")`），不是 `args()[1]` —— 与 M231 的
    #    runner 口径一致（`subprocess.run([pxi, drv], env={...,"M231_CASE": idx})`）。
    #    首版写成 `args()[1]` ⇒ 实测 `pxi run drv.px 0` 报 `io: 读取文件失败 0`
    #    （解释器把 `0` 当脚本路径）⇒ 整个门会跑错。
    src.append("def main():\n    let c = env(\"M246_CASE\")\n"
               "    if c == null:\n        print(\"usage: M246_CASE=<idx>\")\n        return\n    let i = c")
    for i in range(len(cases)):
        src.append('    %s i == "%d":\n        t%d()' % ("if" if i == 0 else "elif", i, i))
    src.append('    else:\n        print("bad idx")')
    open(os.path.join(d, "drv.px"), "w", encoding="utf-8").write("\n".join(src) + "\n")

    with open(os.path.join(d, "cases.tsv"), "w", encoding="utf-8") as f:
        f.write("idx\ttag\texpr\tkind\tbasis\n")
        for i, (tag, lines, k, b) in enumerate(cases):
            f.write("%d\t%s\t%s\t%s\t%s\n" % (i, tag, "; ".join(lines), k, b))

    with open(os.path.join(d, "MODEL.tsv"), "w", encoding="utf-8") as f:
        f.write("tag\tkind\tbasis\n")
        for tag, lines, k, b in cases:
            f.write("%s\t%s\t%s\n" % (tag, k, b))

    print("生成 %d 例（含 %d 例对照形态）" % (len(cases), len(cases) // 2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
