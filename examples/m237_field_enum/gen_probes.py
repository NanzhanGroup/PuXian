#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M237 门 · 字段访问 / 构造 / 变体族 —— 探针生成器（单一事实源）

面 = (接收者类型 × 操作 × 形状)

  read       x.f              存在 / 不存在 / 各种接收者
  write      x.f = v          存在 / 新增 / 只读接收者 / 回读验证
  optread    x?.f             null 接收者 / 非 null
  variant    E.V              存在 / 不存在 / 小写 / 相等比较
  ctor       T(...)           struct 构造（存在 / 类型宽松 / 嵌套）
  ector      E("V") / E(V)    枚举构造（**组 B**：C 轨生成野指针 ⇒ 单独文件）

分组理由：组 A 的案例**编译期都能过**（可聚合进一个驱动器，一次编译覆盖全量）；
组 B 的案例会触发**编译期诊断 → 代码生成逃逸**（生产非法 C）或**运行时野指针**
⇒ 一例失败会毁掉整个驱动器 ⇒ 必须逐例单独编译。

用法：
  gen_probes.py --gen PROBE_DIR     # 写 drv.px + sep/*.px + cases.tsv
  gen_probes.py --selftest          # 自检（id 唯一 / 规模下限 / 组 B 齐备）
"""
import os
import sys

# ── 组 A：聚合驱动器 ──────────────────────────────────────────────
# (id, 接收者, 操作, 形状, 期望种类, 期望, [代码行])
A = [
    # ── read：读字段 ──
    ("read_struct_ok",   "struct", "read", "ok",     "VAL", "1",  ["let p = Point(1, 2)", "print(p.x)"]),
    ("read_struct_ok2",  "struct", "read", "ok",     "VAL", "2",  ["let p = Point(1, 2)", "print(p.y)"]),
    ("read_struct_miss", "struct", "read", "miss",   "ERR", "R1008|结构体没有字段 'z'", ["let p = Point(1, 2)", "print(p.z)"]),
    ("read_dict_ok",     "dict",   "read", "ok",     "VAL", "1",  ['let d = {"a": 1}', "print(d.a)"]),
    ("read_dict_miss",   "dict",   "read", "miss",   "ERR", "R1008|字典没有键 'z'", ['let d = {"a": 1}', "print(d.z)"]),
    ("read_enum_recv",   "enum",   "read", "recv",   "ERR", "R1007|没有字段 'x'", ["print(Color.Red.x)"]),
    ("read_int",         "int",    "read", "recv",   "ERR", "R1007|类型 int 没有字段 'x'", ["let v = 7", "print(v.x)"]),
    ("read_str",         "string", "read", "recv",   "ERR", "R1007|类型 string 没有字段 'x'", ['let v = "s"', "print(v.x)"]),
    ("read_list",        "list",   "read", "recv",   "ERR", "R1007|类型 list 没有字段 'x'", ["let v = [1, 2]", "print(v.x)"]),
    ("read_bool",        "bool",   "read", "recv",   "ERR", "R1007|类型 bool 没有字段 'x'", ["let v = true", "print(v.x)"]),
    ("read_float",       "float",  "read", "recv",   "ERR", "R1007|类型 float 没有字段 'x'", ["let v = 1.5", "print(v.x)"]),
    ("read_tuple",       "tuple",  "read", "recv",   "ERR", "R1007|类型 tuple 没有字段 'x'", ["let v = (1, 2)", "print(v.x)"]),
    ("read_bytes",       "bytes",  "read", "recv",   "ERR", "R1007|类型 bytes 没有字段 'x'", ['let v = bytes("ab")', "print(v.x)"]),
    ("read_result",      "result", "read", "recv",   "ERR", "R1007|没有字段 'x'", ["let v = Ok(1)", "print(v.x)"]),
    ("read_null",        "null",   "read", "recv",   "ERR", "R1007|没有字段 'x'", ["let v = null", "print(v.x)"]),

    # ── write：写字段 ──
    ("write_struct_ok",  "struct", "write", "ok",   "VAL", "9",  ["var p = Point(1, 2)", "p.x = 9", "print(p.x)"]),
    ("write_struct_ob",  "struct", "write", "ok",   "VAL", "2",  ["var p = Point(1, 2)", "p.x = 9", "print(p.y)"]),
    ("write_struct_new", "struct", "write", "new",  "ERR", "R1008|结构体没有字段 'z'", ["var p = Point(1, 2)", "p.z = 9", "print(p.x)"]),
    ("write_dict_ok",    "dict",   "write", "ok",   "VAL", "9",  ['var d = {"a": 1}', "d.a = 9", "print(d.a)"]),
    ("write_dict_new",   "dict",   "write", "new",  "VAL", "7",  ['var d = {"a": 1}', "d.b = 7", "print(d.b)"]),
    ("write_dict_ob",    "dict",   "write", "ok",   "VAL", "1",  ['var d = {"a": 1}', "d.a = 9", 'print(d["a"] == 9)']),
    ("write_enum",       "enum",   "write", "recv", "ERR", "R1002|类型 enum 不支持字段赋值", ["Color.Red.x = 1"]),
    ("write_int",        "int",    "write", "recv", "ERR", "R1002|不支持字段赋值", ["var v = 7", "v.x = 1"]),
    ("write_str",        "string", "write", "recv", "ERR", "R1002|不支持字段赋值", ['var v = "s"', "v.x = 1"]),
    ("write_list",       "list",   "write", "recv", "ERR", "R1002|不支持字段赋值", ["var v = [1]", "v.x = 1"]),
    ("write_null",       "null",   "write", "recv", "ERR", "R1002|不支持字段赋值", ["var v = null", "v.x = 1"]),

    # ── optread：可选链 ──
    ("optread_struct_ok",   "struct", "optread", "ok",   "VAL", "1",    ["var p = Point(1, 2)", "print(p?.x)"]),
    ("optread_struct_miss", "struct", "optread", "miss", "ERR", "R1008|结构体没有字段 'z'", ["var p = Point(1, 2)", "print(p?.z)"]),
    ("optread_dict_ok",     "dict",   "optread", "ok",   "VAL", "1",    ['var d = {"a": 1}', "print(d?.a)"]),
    ("optread_dict_miss",   "dict",   "optread", "miss", "ERR", "R1008|字典没有键 'z'", ['var d = {"a": 1}', "print(d?.z)"]),
    ("optread_null",        "null",   "optread", "recv", "VAL", "null", ["var v = null", "print(v?.x)"]),

    # ── variant：变体访问 ──
    ("variant_ok",      "enum", "variant", "ok",   "VAL", "Color.Red", ["print(Color.Red)"]),
    ("variant_ok2",     "enum", "variant", "ok",   "VAL", "Color.Green", ["print(Color.Green)"]),
    ("variant_miss",    "enum", "variant", "miss", "ERR", "R1008|枚举 Color 没有变体 'Nope'", ["print(Color.Nope)"]),
    ("variant_lower",   "enum", "variant", "case", "ERR", "R1008|没有变体 'red'", ["print(Color.red)"]),
    ("variant_eq",      "enum", "variant", "ok",   "VAL", "true", ["print(Color.Red == Color.Red)"]),
    ("variant_ne",      "enum", "variant", "ok",   "VAL", "false", ["print(Color.Red == Color.Green)"]),
    ("variant_str",     "enum", "variant", "ok",   "VAL", "Red", ["print(str(Color.Red) == \"Red\")"]),

    # ── ctor：struct 构造 ──
    ("ctor_struct_ok",    "struct", "ctor", "ok",      "VAL", "<struct Point>", ["print(Point(1, 2))"]),
    ("ctor_struct_lenient", "struct", "ctor", "typeerr", "VAL", "<struct Point>", ['print(Point("a", "b"))']),
    ("ctor_struct_field", "struct", "ctor", "ok",      "VAL", "1", ["print(Point(1, 2).x)"]),
    ("ctor_struct_nested", "struct", "ctor", "ok",     "VAL", "1", ["print(Point(Point(1, 2).x, 3).x)"]),
]

# ── 组 B：逐例单独编译（会触发「诊断逃逸」或「野指针」） ────────────
B = [
    ("sep_struct_arity_less", ["let p = Point(1)", "print(p)"]),
    ("sep_struct_arity_more", ["let p = Point(1, 2, 3)", "print(p)"]),
    ("sep_enum_call_str",     ['let c = Color("Red")', "print(c)"]),
    ("sep_enum_call_miss",    ['let c = Color("Nope")', "print(c)"]),
    ("sep_enum_call_var",     ["let c = Color(Red)", "print(c)"]),
    ("sep_enum_call_var_miss", ["let c = Color(Nope)", "print(c)"]),
    ("sep_enum_call_arity0",  ["let c = Color()", "print(c)"]),
    ("sep_enum_call_arity2",  ['let c = Color("Red", "Green")', "print(c)"]),
]

# ── 组 C：data enum（文档承诺，parser 半实现） ─────────────────────
C = [
    ("sep_dataenum_def", ["enum Shape:", "    Circle(radius: float)", "    Rect(w: float, h: float)"]),
]

PRELUDE = [
    "# M237 门 · 聚合驱动器（gen_probes.py 生成，勿手改）",
    "#   env M237_CASE 选例；一次编译覆盖全量（M199/M226/M230 范式）",
    "struct Point:",
    "    x: int",
    "    y: int",
    "",
    "enum Color:",
    "    Red",
    "    Green",
    "",
]


def gen(outdir):
    os.makedirs(os.path.join(outdir, "sep"), exist_ok=True)
    # 驱动器
    L = list(PRELUDE)
    for cid, _recv, _op, _shape, _kind, _exp, code in A:
        L.append("def c_%s():" % cid)
        for line in code:
            L.append("    " + line)
        L.append("")
    L.append("def main():")
    L.append('    let c = env("M237_CASE")')
    first = True
    for cid, _recv, _op, _shape, _kind, _exp, _code in A:
        L.append('    %s c == "%s":' % ("if" if first else "elif", cid))
        L.append("        c_%s()" % cid)
        first = False
    L.append("    else:")
    L.append('        print("NOCASE")')
    L.append("")
    L.append("main()")
    with open(os.path.join(outdir, "drv.px"), "w") as f:
        f.write("\n".join(L))

    # 组 B / C 单文件
    for cid, code in B + C:
        L2 = ["# M237 组 B/C（单独编译）· " + cid, "struct Point:", "    x: int", "    y: int", "",
              "enum Color:", "    Red", "    Green", "", "def main():"]
        for line in code:
            L2.append("    " + line)
        L2.append("")
        L2.append("main()")
        with open(os.path.join(outdir, "sep", cid + ".px"), "w") as f:
            f.write("\n".join(L2))

    # cases.tsv
    with open(os.path.join(outdir, "cases.tsv"), "w") as f:
        f.write("例号\t接收者\t操作\t形状\t期望种类\t期望\t组\n")
        for cid, recv, op, shape, kind, exp, _code in A:
            f.write("%s\t%s\t%s\t%s\t%s\t%s\tA\n" % (cid, recv, op, shape, kind, exp))
        for cid, _code in B:
            f.write("%s\t-\tsep\t-\t?\t?\tB\n" % cid)
        for cid, _code in C:
            f.write("%s\t-\tsep\t-\t?\t?\tC\n" % cid)
    return len(A), len(B), len(C)


def selftest():
    bad = []
    ids = [c[0] for c in A] + [c[0] for c in B] + [c[0] for c in C]
    if len(ids) != len(set(ids)):
        bad.append("id 重复")
    if len(A) < 40:
        bad.append("组 A 规模下限 40（实测 %d）" % len(A))
    if len(B) < 6:
        bad.append("组 B 规模下限 6（实测 %d）" % len(B))
    for cid, _r, _o, _s, kind, exp, _c in A:
        if kind not in ("VAL", "ERR"):
            bad.append("%s: 期望种类非法 %s" % (cid, kind))
        if exp is None or exp == "":
            bad.append("%s: 期望为空" % cid)
    for cid, _r, op, _s, _k, _e, _c in A:
        if op not in ("read", "write", "optread", "variant", "ctor"):
            bad.append("%s: 操作非法 %s" % (cid, op))
    if bad:
        print("M237-GEN-SELFTEST-FAIL")
        for b in bad:
            print("  - " + b)
        return 1
    print("M237-GEN-SELFTEST-OK  A=%d B=%d C=%d" % (len(A), len(B), len(C)))
    return 0


def gen_model(out):
    """MODEL.tsv —— **定稿口径表**（去重后的 操作×接收者×形状 → 行为 + R码）。
    与实测量**双向核对**：实测有表里没有 ⇒ 漏登记判红；表里有实测没有 ⇒ 过期判红。"""
    import re
    seen = {}
    order = []
    for cid, recv, op, shape, kind, exp, _code in A:
        k = (op, recv, shape)
        if k in seen:
            continue
        if kind == "VAL":
            beh, rc = "OK", "OK"
        else:
            m = re.search(r"R[0-9]{4}", exp)
            beh, rc = "ERR", (m.group(0) if m else "?")
        seen[k] = (beh, rc)
        order.append(k)
    order.sort()
    with open(out, "w") as f:
        f.write("操作\t接收者\t形状\t行为\tR码\n")
        for k in order:
            beh, rc = seen[k]
            f.write("%s\t%s\t%s\t%s\t%s\n" % (k[0], k[1], k[2], beh, rc))
    return len(order)


if __name__ == "__main__":
    if "--selftest" in sys.argv:
        sys.exit(selftest())
    if "--model" in sys.argv:
        i = sys.argv.index("--model")
        out = sys.argv[i + 1] if len(sys.argv) > i + 1 else "MODEL.tsv"
        n = gen_model(out)
        print("MODEL-OK %d 条 -> %s" % (n, out))
        sys.exit(0)
    if "--gen" in sys.argv:
        i = sys.argv.index("--gen")
        outdir = sys.argv[i + 1] if len(sys.argv) > i + 1 else "."
        na, nb, nc = gen(outdir)
        print("GEN-OK A=%d B=%d C=%d -> %s" % (na, nb, nc, outdir))
        sys.exit(0)
    print("用法: gen_probes.py --gen PROBE_DIR | --selftest")
    sys.exit(2)
