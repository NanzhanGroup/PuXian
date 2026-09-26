#!/usr/bin/env python3
# ============================================================
# M220 语料生成器（**单一事实源**）—— `?` / `!` / `?.` 三运算符的语义收口
# ------------------------------------------------------------
# 产出：`<out>/cases/NN_<label>.px`（**每例一个独立小程序**）+ `<out>/CASES.tsv`
#
# ⚠️ 为什么不学 M218/M216 用「一个驱动器 + args()[1] 分派」：
#   本轮的**病根就在「模块体 vs 函数体」的上下文差别**上 ——
#     · 顶层 `?` 是用法错误（R1004）；
#     · 函数内 `?` 才传播；
#     · 顶层闭包体是**帧**（应传播），而修前 C 轨把它当成模块体。
#   驱动器把每个用例都塞进 `def run_case(sel):` ⇒ 那些用例**全部跑在函数体里**
#   ⇒ 恰好把本轮要测的上下文差别**测没了**（会假绿）。
#   ⇒ 每例一个独立文件，上下文忠实。
#
# CASES.tsv 字段：label \t kind \t expected
#   kind=ok  ⇒ expected = 期望 stdout（三轨逐字节一致，且 rc=0）
#   kind=err ⇒ expected = "R码|核心文案"（三轨 rc≠0、R 码与**剥掉通道前缀**的文案全一致）
# ============================================================
import argparse, io, os, sys

# ---------------- 合法侧（三轨 stdout 逐字节一致） ----------------
LEGAL = [
    # ① `?` 正常（Ok 解包）
    ('ok_try_ok', '''
def f() -> int?:
    return 42
print("A", f()?)
''', 'A 42'),
    ('ok_try_arith', '''
def f() -> int?:
    return 42
print("B", f()? + 1)
''', 'B 43'),
    # ② `!` 正常（三态解包：Result-Ok / 普通值）
    ('ok_force_ok', '''
def g() -> int?:
    return 7
print("C", g()! * 2)
''', 'C 14'),
    ('ok_force_plain_int', 'print("D", 5!)\n', 'D 5'),
    ('ok_force_plain_str', 'print("E", "x"!)\n', 'E x'),
    ('ok_force_plain_list', 'print("F", [1, 2]!)\n', 'F [1, 2]'),
    # ③ `?.` 可选成员
    ('ok_opt_null', '''
var d = null
print("G", d?.a)
''', 'G null'),
    ('ok_opt_field', '''
var d = {"a": 7}
print("H", d?.a)
''', 'H 7'),
    ('ok_opt_chain', '''
var d = {"a": {"b": 5}}
print("I", d?.a?.b)
''', 'I 5'),
    ('ok_opt_chain_null', '''
var d = {"a": null}
print("J", d?.a?.b)
''', 'J null'),
    # ④ `??` 与优先级
    ('ok_coalesce', '''
var x = null
print("K", x ?? 5)
''', 'K 5'),
    ('ok_coalesce_prec', 'print("L", 5 ?? 0 == 5)\n', 'L 5'),
    # ⑤ 函数内 `?` 传播（**不是**用法错误）
    ('ok_try_prop_err', '''
def bad() -> int?:
    return Err("kaboom")
def wrap() -> int?:
    return bad()?
print("M", wrap())
''', 'M Err(kaboom)'),
    ('ok_try_prop_null', '''
def n() -> int?:
    return null
def wrap() -> int?:
    return n()?
print("N", wrap())
''', 'N null'),
    # ⑥ 顶层闭包体 = **帧**（缺陷 320-a）：应传播，不是「顶层 ?」
    ('ok_clos_toplevel_err', '''
def bad() -> int?:
    return Err("inner")
var cl = fn() -> int? { return bad()? }
print("O", cl())
''', 'O Err(inner)'),
    ('ok_clos_toplevel_null', '''
def n() -> int?:
    return null
var cl = fn() -> int? { return n()? }
print("P", cl())
''', 'P null'),
    # ⑦ 函数内闭包用 `?`（缺陷 320-b）：修前 C 轨 **编译失败**（goto 到外层标签）
    ('ok_clos_in_fn', '''
def bad() -> int?:
    return Err("deep")
def outer() -> int?:
    var cl = fn() -> int? { return bad()? }
    return cl()
print("Q", outer())
''', 'Q Err(deep)'),
    # ⑧ 嵌套闭包
    ('ok_clos_nested', '''
var outer = fn() {
    var inner = fn() -> int? { return Err("nested")? }
    return inner()
}
print("R", outer())
''', 'R Err(nested)'),
    # ⑨ 缺陷 319 专项：`?.` 接收者的**副作用次数**必须为 1
    ('ok_opt_side_effect', '''
var n = 0
def f():
    n = n + 1
    return {"a": 1}
var r = f()?.a
print("S", r, "calls=", n)
''', 'S 1 calls= 1'),
    ('ok_opt_side_effect_null', '''
var n = 0
def f():
    n = n + 1
    return null
var r = f()?.a
print("T", r, "calls=", n)
''', 'T null calls= 1'),
    ('ok_opt_side_effect_chain', '''
var n = 0
def mk(t):
    n = n + 1
    return {"v": t}
print("U", mk("x")?.v, "calls=", n)
''', 'U x calls= 1'),
    ('ok_force_side_effect', '''
var n = 0
def f() -> int?:
    n = n + 1
    return n
print("V", f()!, "calls=", n)
''', 'V 1 calls= 1'),
]

# ---------------- 严格侧（三轨 rc≠0 + 同 R 码 + 同核心文案） ----------------
STRICT = [
    # 缺陷 316：顶层 `?` 遇 null ⇒ 修前 VM 轨 **rc=0 且零输出**（静默）
    ('err_try_toplevel_null', '''
def n() -> int?:
    return null
print("W", n()?)
''', 'R1004|顶层不能使用错误传播 ?（仅函数内可用）'),
    # 缺陷 317：顶层 `?` 遇 Err ⇒ 修前 VM 只印 `错误: <payload>`（丢措辞）
    ('err_try_toplevel_err', '''
def bad() -> int?:
    return Err("boom")
print("X", bad()?)
''', 'R1004|顶层不能使用错误传播 ?（仅函数内可用）'),
    # 缺陷 315：`!` 遇 null
    ('err_force_null', '''
def n() -> int?:
    return null
print("Y", n()!)
''', 'R1004|强制解包 !: 值为 null'),
    # 缺陷 315：`!` 遇 Err（C 轨修前**连载荷都丢**）
    ('err_force_err', '''
def bad():
    return Err("boom")
print("Z", bad()!)
''', 'R1004|强制解包 !: 值为 Err(boom)'),
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--out', required=True, help='输出目录')
    ap.add_argument('--tsv', default=None, help='只写 TSV 的路径（默认 <out>/CASES.tsv）')
    a = ap.parse_args()
    out = a.out
    cdir = os.path.join(out, 'cases')
    if not os.path.isdir(cdir):
        os.makedirs(cdir)
    rows = []
    idx = 0
    for group in (LEGAL, STRICT):
        for item in group:
            if len(item) == 3:
                label, src, exp = item
                kind = 'ok' if group is LEGAL else 'err'
            else:
                label, kind, src, exp = item
            idx += 1
            fn = '%02d_%s.px' % (idx, label)
            with io.open(os.path.join(cdir, fn), 'w', encoding='utf-8') as f:
                f.write(src.lstrip('\n'))
            rows.append((label, kind, exp))
    tsv = a.tsv or os.path.join(out, 'CASES.tsv')
    with io.open(tsv, 'w', encoding='utf-8') as f:
        for label, kind, exp in rows:
            f.write('%s\t%s\t%s\n' % (label, kind, exp))
    # 规模下限 + 关键覆盖（防判据被架空）
    assert len(rows) >= 24, '语料规模低于下限：%d < 24' % len(rows)
    labels = [r[0] for r in rows]
    for key in ('ok_opt_side_effect', 'ok_clos_toplevel_err', 'ok_clos_in_fn',
                'err_try_toplevel_null', 'err_force_err'):
        assert key in labels, '语料缺关键用例：%s' % key
    print('cases=%d ok=%d err=%d -> %s'
          % (len(rows), len(LEGAL), len(STRICT), cdir))


main()
