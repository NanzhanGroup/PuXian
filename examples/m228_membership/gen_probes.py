#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M228 门 · 探针生成器（单一事实源）

从「集合 × 元素」矩阵生成：
  · drv.px      —— 三门聚合驱动器（`env("M228_CASE")` / `env("M228_DOOR")` 分派）
                   ⇒ **一次编译**覆盖全量（M199 范式；一例一 build 不可行）
  · cases.tsv   —— 例号 / 集合类型 / 元素类型 / 三门的源码形态 / **Python 独立真值**
  · RCODE.tsv   —— 期望的「门 × 错误形状 ⇒ R 码」表（判据双向核对用）

真值由**独立实现**给出（不在普贤里算）：集合语义按速查表口径
  · str  ⇒ **UTF-8 字节**子串（M83-S1；`"。" in "中文测试"` 为 false）
  · dict ⇒ 键（元素必须是 string）
  · list/tuple ⇒ 按**值相等**逐项；相等口径实测为
      int/float 互通（`2.0 in [1,2,3]` = true）、bool 不与 int 互通（`true in [1,2]` = false）、
      null 只与 null 相等、容器深比较。
"""
import json
import os
import sys

D = os.path.dirname(os.path.abspath(__file__))

# ---- 集合：(id, 普贤源码, Python 值, 类型名, method 门可用的接收者源码) ----
COLLS = [
    ("list3", "[1, 2, 3]",                 [1, 2, 3],           "list",   "[1, 2, 3]"),
    ("listE", "[]",                        [],                  "list",   "[]"),
    ("str4",  '"abcd"',                    "abcd",              "string", '"abcd"'),
    ("strC",  '"中文测试"',                 "中文测试",           "string", '"中文测试"'),
    ("strE",  '""',                        "",                  "string", '""'),
    ("dict2", '{"a": 1, "b": 2}',          {"a": 1, "b": 2},    "dict",   '{"a": 1, "b": 2}'),
    ("dictE", "{}",                        {},                  "dict",   "{}"),
    ("tuple3", "(1, 2, 3)",                (1, 2, 3),           "tuple",  "(1, 2, 3)"),
]

# ---- 元素：(id, 普贤源码, Python 值, 类型名) ----
ELEMS = [
    ("i1",   "1",     1,      "int"),
    ("i9",   "9",     9,      "int"),
    ("sa",   '"a"',   "a",    "string"),
    ("sbc",  '"bc"',  "bc",   "string"),
    ("scjk", '"中"',  "中",    "string"),
    ("btrue", "true", True,   "bool"),
    ("nul",  "null",  None,   "null"),
    ("l12",  "[1, 2]", [1, 2], "list"),
    ("f1",   "1.0",   1.0,    "float"),
]

# ---- 不受支持的集合（三门都必须**响亮**）----
UNSUP_COLLS = [("int5", "5", 5, "int"), ("nul", "null", None, "null"), ("f25", "2.5", 2.5, "float")]
UNSUP_ELEMS = [("i1", "1", 1, "int"), ("sa", '"a"', "a", "string"), ("nul", "null", None, "null")]

STR_ELEM_OK = {"string"}          # str 集合只收 string
DICT_ELEM_OK = {"string"}         # dict 集合只收 string


def pk_eq(a, b):
    """值相等 —— 按实测口径（见模块 docstring）"""
    if isinstance(a, bool) or isinstance(b, bool):
        return isinstance(a, bool) and isinstance(b, bool) and a == b
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return a == b
    if isinstance(a, str) and isinstance(b, str):
        return a == b
    if isinstance(a, list) and isinstance(b, list):
        return len(a) == len(b) and all(pk_eq(x, y) for x, y in zip(a, b))
    if isinstance(a, tuple) and isinstance(b, tuple):
        return len(a) == len(b) and all(pk_eq(x, y) for x, y in zip(a, b))
    if isinstance(a, dict) and isinstance(b, dict):
        return set(a) == set(b) and all(pk_eq(a[k], b[k]) for k in a)
    return False


def truth(cval, ckind, eval_, ekind):
    """返回 ('VAL', bool) 或 ('ERR', 期望的 R 码 per 门) 的**形状**描述"""
    if ckind == "string":
        if ekind not in STR_ELEM_OK:
            return ("ERR", "elem")
        return ("VAL", eval_.encode("utf-8") in cval.encode("utf-8"))
    if ckind == "dict":
        if ekind not in DICT_ELEM_OK:
            return ("ERR", "elem")
        return ("VAL", eval_ in cval)
    if ckind in ("list", "tuple"):
        return ("VAL", any(pk_eq(x, eval_) for x in cval))
    return ("ERR", "coll")


def main():
    cases = []
    for cid, csrc, cval, ckind, msrc in COLLS:
        for eid, esrc, eval_, ekind in ELEMS:
            cases.append((f"{cid}__{eid}", csrc, cval, ckind, msrc, esrc, eval_, ekind, True))
    for cid, csrc, cval, ckind in UNSUP_COLLS:
        for eid, esrc, eval_, ekind in UNSUP_ELEMS:
            cases.append((f"{cid}__{eid}", csrc, cval, ckind, csrc, esrc, eval_, ekind, False))

    rows = []
    for cid, csrc, cval, ckind, msrc, esrc, eval_, ekind, supported in cases:
        kind, shape = truth(cval, ckind, eval_, ekind)
        rows.append(dict(case=cid, csrc=csrc, msrc=msrc, esrc=esrc,
                         ckind=ckind, ekind=ekind, kind=kind, shape=shape,
                         supported=supported))

    # ---- drv.px ----
    L = []
    L.append("# M228 门 · 三门聚合驱动器（由 gen_probes.py 生成，勿手改）")
    L.append("#   env M228_CASE = 例号 · env M228_DOOR = op|notop|func|method")
    L.append("#   为什么用 env 而不是 args()：三轨取 argv 的通道不一致（M205 tools 面），")
    L.append("#   env() 是 native，三轨同一条路。")
    L.append("def main():")
    L.append('    let c = env("M228_CASE")')
    L.append('    let d = env("M228_DOOR")')
    for door in ("op", "notop", "func", "method"):
        L.append(f'    if d == "{door}":')
        first = True
        for r in rows:
            if door == "op":
                src = f"{r['esrc']} in {r['csrc']}"
            elif door == "notop":
                src = f"{r['esrc']} not in {r['csrc']}"
            elif door == "func":
                src = f"contains({r['csrc']}, {r['esrc']})"
            else:
                src = f"({r['msrc']}).contains({r['esrc']})"
            # ⚠️ 必须 elif：全写 if 会让**所有**分支依次执行（每个都 print 一次）
            L.append(f'        {"if" if first else "elif"} c == "{r["case"]}":')
            L.append(f"            print({src})")
            first = False
        L.append("        return")
    L.append('    print("BAD-DOOR")')
    open(os.path.join(D, "drv.px"), "w", encoding="utf-8").write("\n".join(L) + "\n")

    # ---- cases.tsv ----
    with open(os.path.join(D, "cases.tsv"), "w", encoding="utf-8") as fh:
        fh.write("例号\t集合类型\t元素类型\t真值形状\t期望值\t受支持\n")
        for r in rows:
            fh.write("%s\t%s\t%s\t%s\t%s\t%s\n" % (
                r["case"], r["ckind"], r["ekind"], r["kind"],
                ("true" if r["shape"] else "false") if r["kind"] == "VAL" else r["shape"],
                "yes" if r["supported"] else "no"))

    # ---- RCODE.tsv：期望的「门 × 错误形状 ⇒ R 码」----
    #   形状 elem = 元素/键类型不符（str/dict 集合）；coll = 集合类型不支持
    with open(os.path.join(D, "RCODE.tsv"), "w", encoding="utf-8") as fh:
        fh.write("# M228 门 · 期望「门 × 错误形状 ⇒ R 码」表（判据双向核对：实测 ⇄ 本表 精确相等）\n")
        fh.write("# 为什么 op/func 是 R1002 而 method 的 coll 形状是 R1007：方法面在**取方法**这一层\n")
        fh.write("#   就失败了（M227 RCODE.tsv 同款登记）—— 条数上限 6，多一条都必须有理由。\n")
        fh.write("门\t形状\tR码\n")
        for door in ("op", "notop", "func", "method"):
            fh.write("%s\telem\tR1002\n" % door)
            fh.write("%s\tcoll\t%s\n" % (door, "R1007" if door == "method" else "R1002"))

    print("OK: %d 例 · 驱动 %d 行" % (len(rows), len(L)))


if __name__ == "__main__":
    main()
