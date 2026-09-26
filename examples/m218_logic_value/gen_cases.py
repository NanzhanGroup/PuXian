#!/usr/bin/env python3
# ============================================================
# M218 语料生成器（**单一事实源**）—— `and` / `or` 的返回值语义（缺陷 310）
# ------------------------------------------------------------
# 产出：cases.px（按 args()[1] 分派的驱动器）+ CASES.tsv（label/kind/期望）
# 语义（三轨同一真相）：`and` / `or` 返回**操作数本身**（对齐 runtime 的
#   `px_and(a,b) = truthy(a) ? b : a` / `px_or(a,b) = truthy(a) ? a : b`），
#   且**短路**不被破坏（右操作数只在需要时求值）。
# 真值来源：本文件的 `py_and/py_or` —— 按定义式用 Python 算（不同构）。
#   ⚠️ Python 的 `and`/`or` **就是**这个语义 ⇒ 天然是独立参照。
# ============================================================
import argparse, json, os, sys

def py_and(a, b):
    return b if bool(a) else a

def py_or(a, b):
    return a if bool(a) else b

def repr_px(v):
    """**源码字面量**：字符串要带引号（写进 cases.px 用）。"""
    if v is True:
        return 'true'
    if v is False:
        return 'false'
    if v is None:
        return 'null'
    if isinstance(v, str):
        return '"%s"' % v
    return str(v)


def out_px(v):
    """**程序输出形式**（期望 stdout 用）。

    ⚠️ 与 `repr_px` **必须分开**：`print("a")` 在 PuXian 里输出 `a`（**不带引号**）。
    本门首版两者共用一个函数 ⇒ 期望值成了 `"a"` ⇒ 三轨一致地「对不上期望」
    ⇒ 门报 30 例 EXPECT 失败（而三轨其实**完全一致**）。
    """
    if v is True:
        return 'true'
    if v is False:
        return 'false'
    if v is None:
        return 'null'
    return str(v)

# (label, .px 源码, 期望 stdout)
LEGAL = []
# ① 基本：非布尔操作数 ⇒ 必须返回**操作数**
CASES = [
    ('int_int', 1, 2), ('int0_int', 0, 3), ('int_int0', 5, 0),
    ('str_str', 'a', 'b'), ('empty_str', '', 'x'), ('str_empty', 'x', ''),
    ('float_int', 1.5, 7), ('zero_float', 0.0, 9),
    ('bool_int', True, 42), ('int_bool', 7, False),
]
for label, a, b in CASES:
    body = 'var a = %s\nvar b = %s\nprint(a and b)' % (repr_px(a), repr_px(b))
    exp = out_px(py_and(a, b))
    LEGAL.append(('ok_and_%s' % label, body, exp))
    body = 'var a = %s\nvar b = %s\nprint(a or b)' % (repr_px(a), repr_px(b))
    exp = out_px(py_or(a, b))
    LEGAL.append(('ok_or_%s' % label, body, exp))
# ② 容器/游离值（null）
for label, a, b in (('list_list', '[]', '[1]'), ('null_int', None, 5),
                    ('int_null', 1, None)):
    sa = 'null' if a is None else str(a)
    sb = 'null' if b is None else str(b)
    if a == '[]':
        sa = '[]'
    if b == '[1]':
        sb = '[1]'
    va, vb = (None if a is None else ([] if a == '[]' else a),
              None if b is None else ([1] if b == '[1]' else b))
    body = 'var a = %s\nvar b = %s\nprint(a and b)\nprint(a or b)' % (sa, sb)
    exp = '%s\n%s' % (out_px(py_and(va, vb)), out_px(py_or(va, vb)))
    LEGAL.append(('ok_mix_%s' % label, body, exp))
# ③ **短路**必须保持（用副作用日志观测右操作数是否被求值）
LEGAL.append(('ok_short_and',
    'def f():\n    print("SIDE")\n    return 1\n'
    'var a = 0\nprint(a and f())',
    '0'))
LEGAL.append(('ok_short_or',
    'def f():\n    print("SIDE")\n    return 1\n'
    'var a = 1\nprint(a or f())',
    '1'))
LEGAL.append(('ok_eval_and',
    'def f():\n    print("SIDE")\n    return 2\n'
    'var a = 1\nprint(a and f())',
    'SIDE\n2'))
LEGAL.append(('ok_eval_or',
    'def f():\n    print("SIDE")\n    return 3\n'
    'var a = 0\nprint(a or f())',
    'SIDE\n3'))
# ④ 链式 / 常见惯用法
# ⚠️ `print(a, b, c, d)` 是**一行**（空格分隔）；首版误写成 4 行 ⇒ 判据对不上。
LEGAL.append(('ok_default',
    'def g(x):\n    return x or "default"\n'
    'print(g(""), g("v"), g(0), g(7))',
    'default v default 7'))
LEGAL.append(('ok_chain',
    'var a = 0\nvar b = 0\nvar c = "z"\nprint(a or b or c)',
    'z'))
LEGAL.append(('ok_not_bool',
    'var a = 5\nprint(not a, not 0, not "")',
    'false true true'))

# 拒绝侧：本缺陷面**不应**产生错误（`and`/`or` 对任意类型都合法）
REJECT = []

HEAD = [
 '# ⚠️ 本文件由 gen_cases.py 生成（单一事实源）—— 手改会被第 1 层的漂移检测判红',
 '# M218 驱动器：按 args()[1] 分派（一次编译覆盖全量用例）',
 'a = args()', 'sel = ""', 'if len(a) >= 2:', '    sel = a[1]', '',
 'def run_case(sel):',
]

def emit(outdir):
    lines = list(HEAD); rows = []
    for label, body, exp in LEGAL:
        lines.append('    if sel == "%s":' % label)
        lines += ['        ' + ln for ln in body.splitlines()]
        lines.append('        return 0')
        rows.append((label, 'ok', exp))
    for label, body, exp in REJECT:
        lines.append('    if sel == "%s":' % label)
        lines += ['        ' + ln for ln in body.splitlines()]
        lines.append('        return 0')
        rows.append((label, 'rej', exp))
    lines += ['    print("NO-SUCH-CASE: " + sel)', '    return 2', '', 'exit(run_case(sel))']
    with open(os.path.join(outdir, 'cases.px'), 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines) + '\n')
    with open(os.path.join(outdir, 'CASES.tsv'), 'w', encoding='utf-8') as f:
        for label, kind, exp in rows:
            # ⚠️ 期望值一律用 **JSON** 编码（单行、无歧义）—— 本门首版自己搓的
            #    `\n` / `\e` 转义有两处会坏：① 多行期望被 `splitlines` 拆开；
            #    ② **空期望**行尾是 `\t` ⇒ 读取端 `strip()` 把它吃掉 ⇒ 字段数 2。
            #    JSON 一行搞定（`"a\nb"` / `""`），读取端 `json.loads` 还原。
            f.write('%s\t%s\t%s\n' % (label, kind, json.dumps(exp, ensure_ascii=False)))
    return len(LEGAL), len(REJECT)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--out', default=os.path.dirname(os.path.abspath(__file__)))
    a = ap.parse_args()
    nok, nrej = emit(a.out)
    print('生成：合法侧 %d · 拒绝侧 %d ⇒ %s' % (nok, nrej, a.out))
    return 0

if __name__ == '__main__':
    sys.exit(main())
