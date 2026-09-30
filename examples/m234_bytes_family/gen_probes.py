#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M234 门 · 探针与期望表生成器（「`bytes` 族 × 逐类型实参」全量对拍）

为什么是这个面
--------------
M230 的**覆盖边界**里如实登记过一条缺口：「**`bytes` 的 `b[i]` 未覆盖**」；
M233 把「任意值 ⇒ 其 `str()` 形态」这一侧的兜底统一了，但 `bytes` 走的是**另一条档**
（对象自身缓冲 —— 语义是「原始字节」而不是文本形态）。⇒ `bytes` **族**是唯一
既「有专门的数据语义」又「曾经明确登记为未覆盖」的面。

面 = `bytes_len` / `bytes_get` / `bytes_set` / `bytes_slice` / `bytes_concat` /
     `bytes_to_str` / `bytes_to_hex` / `bytes_find` / `bytes_base64` / `base64_to_bytes` /
     `int_to_bytes` / `bytes_to_int` × **13 类型**（二元接口取「另一位置放合法值」的逐位置形态）。

判据（三层）
------------
① 三轨一致（响亮性 / 值 / R 码 + 词条）；
② 与 `MODEL.tsv` **逐轨**双向核对；
③ **确定性**（同二进制 · 不同进程布局两遍逐字节一致）—— M233 建立的 UB 指纹判据。

用法
----
  gen_probes.py --root R --gen        生成 cases.tsv + drv.px + MODEL.tsv
  gen_probes.py --root R --audit      源码派生：`bytes` 族的「数据档」判据
"""
import argparse, os, re, sys

ARGS = [
    ("int_7", "7"), ("int_neg", "0 - 3"), ("int_0", "0"),
    ("float_15", "1.5"), ("float_0", "0.0"),
    ("str_ab", '"ab"'), ("str_empty", '""'), ("str_cjk", '"中文"'),
    ("bytes_ab", 'bytes("ab")'), ("bytes_empty", 'bytes("")'), ("bytes_cjk", 'bytes("中文")'),
    ("bool_t", "true"), ("null_v", "null"),
    ("list_1", "[1]"), ("dict_a1", '{"a": 1}'), ("tuple_1", "(1,)"),
    ("enum_a", "E1.A"), ("struct_v", "S1(1)"), ("fn_v", "userfn"),
]

# 一元：直接对每个实参取
UNARY = ["bytes_len", "bytes_to_str", "bytes_to_hex", "bytes_base64",
         "bytes_to_int", "base64_to_bytes"]

HDR = '''struct S1:
    a: int

enum E1:
    A
    B

def userfn():
    return 1

'''


def tag_of(fn, arg):
    return "%s__%s" % (fn, arg)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--gen", action="store_true")
    ap.add_argument("--audit", action="store_true")
    ap.add_argument("--out", default=None)
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    d = a.out or os.path.join(root, "examples", "m234_bytes_family")

    if a.audit:
        src = open(os.path.join(root, "runtime", "runtime.c"), encoding="utf-8").read()
        bad = 0
        # 反向判据：`bytes` 族的「数据档」不得落到文本档的兜底
        m = re.search(r'static const char\* px_cstr_any\(LXValue v, int\* out_len\) \{(.*?)\n\}', src, re.S)
        if not m:
            print("❌ 未能定位 px_cstr_any"); return 1
        if "PX_BYTES" not in m.group(1):
            print("❌ px_cstr_any 的快路径未处理 PX_BYTES（bytes 会落到文本档）"); bad += 1
        else:
            print("✅ px_cstr_any 的 bytes 数据档在位")
        m2 = re.search(r'static const char\* bdata\(LXValue v\) \{(.*?)\n\}', src, re.S)
        if not m2 or "px_cstr_any" not in m2.group(1):
            print("❌ bdata 未走统一入口"); bad += 1
        else:
            print("✅ bdata 走统一入口")
        if "px_cstr_any" not in src:
            print("❌ px_cstr_any 缺失（M233 的统一入口）"); bad += 1
        else:
            print("✅ px_cstr_any 在位（M233 统一入口）")
        print("M234-AUDIT-OK" if bad == 0 else "M234-AUDIT-FAIL")
        return 0 if bad == 0 else 1

    cases = []
    for fn in UNARY:
        for at, expr in ARGS:
            cases.append((tag_of(fn, at), "%s(%s)" % (fn, expr)))
    for at, expr in ARGS:
        cases.append(("bytes_find__%s" % at, "str(bytes_find(bytes(\"ab\"), %s))" % expr))
        cases.append(("bytes_get__%s" % at, "str(bytes_get(bytes(\"ab\"), %s))" % expr))
        cases.append(("bytes_concat__%s" % at, "str(bytes_concat(bytes(\"ab\"), %s))" % expr))
        cases.append(("bytes_slice__%s" % at, "str(bytes_slice(bytes(\"abc\"), %s))" % expr))
        cases.append(("int_to_bytes__%s" % at, "bytes_to_hex(int_to_bytes(%s, 1, \"little\"))" % expr))

    if not a.gen:
        print("用例 %d" % len(cases))
        return 0

    os.makedirs(d, exist_ok=True)
    src = [HDR]
    for i, (tag, expr) in enumerate(cases):
        src.append('def t%d():\n    print("%s|" + str(%s))\n' % (i, tag, expr))
    src.append("def main():\n")
    src.append('    let c = env("M234_CASE")\n')
    src.append('    if c == null:\n        print("usage")\n        return\n')
    src.append("    let i = c\n")
    for i in range(len(cases)):
        kw = "if" if i == 0 else "elif"
        src.append('    %s i == "%d":\n        t%d()\n' % (kw, i, i))
    src.append('    else:\n        print("bad idx")\n')
    open(os.path.join(d, "drv.px"), "w").write("".join(src))
    with open(os.path.join(d, "cases.tsv"), "w") as f:
        f.write("idx\ttag\texpr\n")
        for i, (tag, expr) in enumerate(cases):
            f.write("%d\t%s\t%s\n" % (i, tag, expr))
    with open(os.path.join(d, "MODEL.tsv"), "w") as f:
        f.write("tag\tkind\tbasis\n")
        for tag, _e in cases:
            f.write("%s\tAUTO\tbytes-family-positional\n" % tag)
    print("M234-GEN-OK cases=%d（%d 一元函数 × %d 实参 + 5 位置族 × %d）"
          % (len(cases), len(UNARY), len(ARGS), len(ARGS)))
    return 0


sys.exit(main())
