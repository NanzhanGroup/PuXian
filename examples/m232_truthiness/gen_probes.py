#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M232 门 · 探针与期望表生成器（「真值性表 + 短路族」全量对拍）

为什么是这个面
--------------
M231 把 **`(运算符 × 左类型 × 右类型)`** 矩阵做完之后，短路族
（`and` / `or` / `??` / `|>`）被**显式排除**在矩阵外（理由写在 m231 的 gen_probes.py 头注：
「and/or 对任意类型做真值性转换，矩阵判据对这些运算符退化为恒 VAL ⇒ 无判别力」）。

本轮换一个判据来量同一个面：**真值性本身**。
`if` / `while` / `not` / `and` / `or` / `bool()` / `??` 全都建在同一个原语上 ——
「这个值算真还是算假」。它是**整个语言里最公共的判据**（一处错，处处错），
而它从来没有被清单级度量过。

面
--
① **真值性表**：(类型 × 空/非空) × 5 个观察点（`bool()` / `not` / `if` / `and` / `or`）
   —— 5 个观察点是**同一语义的 5 个门**（M227 的立论：同一个操作的不同门必须口径一致）。
② **短路族返回值语义**：`and` / `or` 返回**操作数**（不是布尔）；`??` 判据是 **null**、不是真值。
③ **短路性**：右侧操作数是否被求值 —— 用**副作用日志**直接观测（不是靠返回值猜）。
④ **`|>` 管道**：左侧作右侧函数的**首个实参**。

缺陷 347（本轮立论）
-------------------
真值性表在**三个实现之间有两处分叉**：
  · `bytes("")`  解释轨 **TRUTHY** ⇄ 编译两轨 **FALSY**
  · `()`         解释轨 **FALSY**  ⇄ 编译两轨 **TRUTHY**
根因 = 两份真值性实现**各漏一个分支**：
  · `selfhost/ival.px` 的 `i_truthy` 缺 `bytes` ⇒ 落到兜底 `return true`
  · `runtime/runtime.c` 的 `px_is_truthy` 缺 `PX_TUPLE` ⇒ 落到 `default: return true`
⇒ 「一条公共路径漏一个分支」的**第二次**（M231 缺陷 346 是同族第一次）。

期望表的来历（如实登记）
------------------------
MODEL.tsv 的「真/假」**由语言自身的容器一致性写定**（`"" ` / `[]` / `{}` 已是 FALSY
⇒ `bytes("")` / `()` 必须同为 FALSY），**不是**从实测抄出来的；
verify.sh 第 [4] 层做**双向**核对（漏登记 / 过期都判红）。

用法
----
  gen_probes.py --root R --gen        生成 cases.tsv + drv.px + MODEL.tsv
"""
import argparse, os, re, sys

# ---------- 面 ①：真值性表（类型 × 空/非空） ----------
# (tag, 值表达式, 期望真值, 依据名)
TRUTH_CASES = [
    ("int_0",      "0",           False, "scalar-zero"),
    ("int_1",      "1",           True,  "scalar-nonzero"),
    ("int_neg1",   "0 - 1",       True,  "scalar-nonzero"),
    ("float_00",   "0.0",         False, "scalar-zero"),
    ("float_15",   "1.5",         True,  "scalar-nonzero"),
    ("str_empty",  '""',          False, "container-empty"),
    ("str_x",      '"x"',         True,  "container-nonempty"),
    ("bytes_empty", 'bytes("")',  False, "container-empty"),
    ("bytes_x",    'bytes("x")',  True,  "container-nonempty"),
    ("list_empty", "[]",          False, "container-empty"),
    ("list_1",     "[1]",         True,  "container-nonempty"),
    ("dict_empty", "{}",          False, "container-empty"),
    ("dict_a1",    '{"a": 1}',    True,  "container-nonempty"),
    ("tuple_empty", "()",         False, "container-empty"),
    ("tuple_1",    "(1,)",        True,  "container-nonempty"),
    ("null_v",     "null",        False, "null"),
    ("bool_false", "false",       False, "scalar-zero"),
    ("bool_true",  "true",        True,  "scalar-nonzero"),
    ("fn_v",       "userfn",      True,  "opaque-always"),
    ("struct_v",   "S1(1)",       True,  "opaque-always"),
    ("enum_v",     "E1.A",        True,  "opaque-always"),
    ("result_ok",  "Ok(1)",       True,  "opaque-always"),
    ("result_err", 'Err("e")',    True,  "opaque-always"),
]

# ---------- 面 ②③：短路族（返回值 + 短路性） ----------
# (tag, 运算符, 左操作数, 期望返回值, 期望副作用日志, 依据名)
SC_CASES = [
    # `and`：左 FALSY ⇒ 右不求值，返回**左操作数**
    ("and_false",  "and", "false",      "false",       "[]", "rhs-unevaluated-falsy-lhs"),
    ("and_true",   "and", "true",       "true",        '[A]', "rhs-evaluated-truthy-lhs"),
    ("and_zero",   "and", "0",          "0",           "[]", "rhs-unevaluated-falsy-lhs"),
    ("and_estr",   "and", '""',         "",            "[]", "rhs-unevaluated-falsy-lhs"),
    ("and_null",   "and", "null",       "null",        "[]", "rhs-unevaluated-falsy-lhs"),
    ("and_ebytes", "and", 'bytes("")',  '<bytes 0>',   "[]", "rhs-unevaluated-falsy-lhs"),
    ("and_etuple", "and", "()",         "()",          "[]", "rhs-unevaluated-falsy-lhs"),
    # `or`：左 TRUTHY ⇒ 右不求值，返回**左操作数**
    ("or_true",    "or",  "true",       "true",        "[]", "rhs-unevaluated-truthy-lhs"),
    ("or_false",   "or",  "false",      "true",        '[A]', "rhs-evaluated-falsy-lhs"),
    ("or_one",     "or",  "1",          "1",           "[]", "rhs-unevaluated-truthy-lhs"),
    ("or_zero",    "or",  "0",          "true",        '[A]', "rhs-evaluated-falsy-lhs"),
    ("or_ebytes",  "or",  'bytes("")',  "true",        '[A]', "rhs-evaluated-falsy-lhs"),
    ("or_etuple",  "or",  "()",         "true",        '[A]', "rhs-evaluated-falsy-lhs"),
    # `??`：判据是 **null**（不是真值）⇒ `0` / `""` / `false` 都**不**走右侧
    ("nc_one",     "??",  "1",          "1",           "[]", "coalesce-null-only"),
    ("nc_null",    "??",  "null",       "true",        '[A]', "coalesce-null-only"),
    ("nc_zero",    "??",  "0",          "0",           "[]", "coalesce-null-only"),
    ("nc_estr",    "??",  '""',         "",            "[]", "coalesce-null-only"),
    ("nc_false",   "??",  "false",      "false",       "[]", "coalesce-null-only"),
    ("nc_ebytes",  "??",  'bytes("")',  '<bytes 0>',   "[]", "coalesce-null-only"),
    ("nc_etuple",  "??",  "()",         "()",          "[]", "coalesce-null-only"),
]

# ---------- 面 ④：`|>` 管道（左作右侧函数的**首个实参**） ----------
# (tag, 表达式, 期望输出, 依据名)
PIPE_CASES = [
    ("pipe_inc",   "3 |> inc",        "4", "pipe-first-arg"),
    ("pipe_len",   '"ab" |> len',     "2", "pipe-first-arg"),
    ("pipe_chain", "3 |> inc |> inc", "5", "pipe-left-fold"),
]

HEADER = '''# M232 探针驱动器（自动生成 · 勿手改）
# 面 ① 真值性表（5 个观察点：bool() / not / if / and / or）
# 面 ②③ 短路族（返回值 + 副作用日志）
# 面 ④ |> 管道
var LOG = []

def rhs(tag):
    LOG.append(tag)
    return true

def inc(x):
    return x + 1

def userfn():
    return 1

struct S1:
    a: int

enum E1:
    A
    B

'''


def emit_truth(i, tag, expr):
    """面 ①：一行 5 个观察点（同一语义的 5 个门）"""
    return (
        'def t%d():\n'
        '    let v = %s\n'
        '    var s = "%s"\n'
        '    s += "|" + str(bool(v))\n'
        '    s += "|" + str(not v)\n'
        '    if v:\n'
        '        s += "|T"\n'
        '    else:\n'
        '        s += "|F"\n'
        '    s += "|" + str(v and 1)\n'
        '    s += "|" + str(v or 1)\n'
        '    print(s)\n'
    ) % (i, expr, tag)


def emit_sc(i, tag, op, lhs):
    """面 ②③：短路族 —— 返回**操作数** + 右侧是否求值（看 LOG）"""
    return (
        'def t%d():\n'
        '    LOG = []\n'
        '    let r = %s %s rhs("A")\n'
        '    print("%s|" + str(r) + "|" + str(LOG))\n'
    ) % (i, lhs, op, tag)


def emit_pipe(i, tag, expr):
    return (
        'def t%d():\n'
        '    print("%s|" + str(%s))\n'
    ) % (i, tag, expr)


def audited_specialized(root):
    """**源码派生**两份真值性实现里「有专门分支」的类型集

    判据（本门的立论）：两份实现是**同一语义的两个实现** ⇒
    「有专门分支的类型集」必须**逐个相等**。差集非空 = 有一侧漏了分支
    ——这正是缺陷 347 的形状（`ival.px` 缺 `bytes`、`runtime.c` 缺 `PX_TUPLE`），
    而且这个判据**只看源码就能判**（不需要跑起来）。

    为什么必须源码派生（M212 缺陷 288 的纪律）：手抄的清单必然与实现脱节，
    而「脱节」的表现是**判据静默变窄**（漏掉的类型不再被检查）。
    """
    rt_src = open(os.path.join(root, "runtime", "runtime.c"), encoding="utf-8").read()
    iv_src = open(os.path.join(root, "selfhost", "ival.px"), encoding="utf-8").read()

    m = re.search(r'bool px_is_truthy\(LXValue v\) \{(.*?)\n\}', rt_src, re.S)
    if not m:
        raise SystemExit("❌ 未能定位 px_is_truthy（锚点失效）")
    rt_raw = re.findall(r'case\s+(PX_[A-Z_]+)\s*:', m.group(1))

    m2 = re.search(r'def i_truthy\(v\):(.*?)\n# ', iv_src, re.S)
    if not m2:
        raise SystemExit("❌ 未能定位 i_truthy（锚点失效）")
    iv = re.findall(r'if t == "([a-z_]+)":', m2.group(1))

    # C 侧名字 → 普贤类型名（`PX_STR` 的普贤名是 `string`，不是 `str`）
    CMAP = {"PX_NULL": "null", "PX_BOOL": "bool", "PX_INT": "int",
            "PX_FLOAT": "float", "PX_STR": "string", "PX_BYTES": "bytes",
            "PX_LIST": "list", "PX_DICT": "dict", "PX_TUPLE": "tuple"}
    return {CMAP.get(c, c[3:].lower()) for c in rt_raw}, set(iv)


def audit(root):
    rt, iv = audited_specialized(root)
    print("派生 · runtime/runtime.c 专门分支 %d 个：%s" % (len(rt), " ".join(sorted(rt))))
    print("派生 · selfhost/ival.px  专门分支 %d 个：%s" % (len(iv), " ".join(sorted(iv))))
    only_rt, only_iv = sorted(rt - iv), sorted(iv - rt)
    if only_rt or only_iv:
        print("❌ 两份实现的「专门分支集」不相等：")
        print("   仅编译轨有：%s（解释轨会落到兜底 return true ⇒ 判真）" % (only_rt or "—"))
        print("   仅解释轨有：%s（编译轨会落到 default: return true ⇒ 判真）" % (only_iv or "—"))
        return 1
    print("✅ 两份实现的专门分支集**逐个相等**（%d 个）" % len(rt))
    # 覆盖面：两份实现覆盖到的类型，探针语料必须逐个覆盖（否则判据静默变窄）
    # ⚠️ 标签用 `str_*`（普贤类型名是 `string`）⇒ 归一化后再比，否则**假红**
    covered = {("string" if c[0].split("_")[0] == "str" else c[0].split("_")[0])
               for c in TRUTH_CASES}
    missing = sorted(rt - covered)
    if missing:
        print("❌ 探针未覆盖的类型：%s" % missing)
        return 1
    print("✅ 探针覆盖两份实现的**全部**类型（%d 个）" % len(rt))
    print("M232-AUDIT-OK")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--gen", action="store_true")
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--out", default=None)
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    d = a.out or os.path.join(root, "examples", "m232_truthiness")

    if a.audit:
        return audit(root)

    cases = []
    for tag, expr, truthy, basis in TRUTH_CASES:
        cases.append(("T", tag, expr, "T" if truthy else "F", basis))
    for tag, op, lhs, rv, log, basis in SC_CASES:
        cases.append(("S", tag, "%s %s rhs(\"A\")" % (lhs, op), rv + "|" + log, basis))
    for tag, expr, out, basis in PIPE_CASES:
        cases.append(("P", tag, expr, out, basis))

    if not a.gen:
        print("用例 %d（真值性 %d · 短路族 %d · 管道 %d）"
              % (len(cases), len(TRUTH_CASES), len(SC_CASES), len(PIPE_CASES)))
        return 0

    os.makedirs(d, exist_ok=True)
    src = [HEADER]
    idx = 0
    truth_range, sc_range, pipe_range = {}, {}, {}
    for kind, tag, expr, _exp, _b in cases:
        if kind == "T":
            src.append(emit_truth(idx, tag, expr)); truth_range[tag] = idx
        elif kind == "S":
            op = [c[1] for c in SC_CASES if c[0] == tag][0]
            lhs = [c[2] for c in SC_CASES if c[0] == tag][0]
            src.append(emit_sc(idx, tag, op, lhs)); sc_range[tag] = idx
        else:
            src.append(emit_pipe(idx, tag, expr)); pipe_range[tag] = idx
        idx += 1

    src.append("def main():\n")
    src.append('    let c = env("M232_CASE")\n')
    src.append('    if c == null:\n')
    src.append('        print("usage: M232_CASE=<idx>")\n')
    src.append("        return\n")
    src.append("    let i = c\n")
    for j in range(len(cases)):
        kw = "if" if j == 0 else "elif"
        src.append('    %s i == "%d":\n        t%d()\n' % (kw, j, j))
    src.append('    else:\n        print("bad idx")\n')
    open(os.path.join(d, "drv.px"), "w").write("".join(src))

    with open(os.path.join(d, "cases.tsv"), "w") as f:
        f.write("idx\tkind\ttag\texpr\texpect\tbasis\n")
        for i, (kind, tag, expr, exp, b) in enumerate(cases):
            f.write("%d\t%s\t%s\t%s\t%s\t%s\n" % (i, kind, tag, expr, exp, b))

    with open(os.path.join(d, "MODEL.tsv"), "w") as f:
        f.write("tag\tkind\texpect\tbasis\n")
        for kind, tag, expr, exp, b in cases:
            f.write("%s\t%s\t%s\t%s\n" % (tag, kind, exp, b))

    print("M232-GEN-OK cases=%d（真值性 %d · 短路族 %d · 管道 %d）"
          % (len(cases), len(TRUTH_CASES), len(SC_CASES), len(PIPE_CASES)))
    return 0


sys.exit(main())
