#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M300 · stdlib 第二批边界面：探针生成器（**源码派生** + 独立参考 + 双向核对）
# ------------------------------------------------------------
# 纪律（M212 缺陷 288 / M199 / M215 / M279 的教训）：
#   · 函数清单**不手抄** —— 从 `stdlib/<mod>.px` 的 `def` 行派生（含 arity）；
#     规格表与源码派生不一致 ⇒ **响亮退出 rc=3**（不静默生成 0 条）
#   · **合法侧期望值来自独立参考实现** `ref.py`（不同语言/不同算法），
#     不由三轨自证、不由手算（M215：「三轨一致」是必要条件不是充分条件）
#   · 输出 `drv.px` 是**生成物** ⇒ 门里比对「重生成 == 入库件」（防漂移）
#
# 面 = 合法侧（独立期望）∪ 逐位置 × 错类型毒值 ∪ 错 arity
# 输出：<work>/drv.px / <work>/CASES.tsv
# ============================================================
import argparse, json, os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ref as R

POISON = ['null', 'true', '7', '"7"', '[]', '{}']


def q(s):
    """Python 字符串 → 普贤字面量"""
    out = []
    for ch in s:
        if ch == '\\':
            out.append('\\\\')
        elif ch == '"':
            out.append('\\"')
        elif ch == '\n':
            out.append('\\n')
        elif ch == '\r':
            out.append('\\r')
        elif ch == '\t':
            out.append('\\t')
        else:
            out.append(ch)
    return '"' + ''.join(out) + '"'


def lit(v):
    if isinstance(v, bool):
        return 'true' if v else 'false'
    if isinstance(v, str):
        return q(v)
    if isinstance(v, int):
        return str(v)
    if isinstance(v, list):
        return '[' + ', '.join(lit(x) for x in v) + ']'
    if isinstance(v, dict):
        return '{' + ', '.join('%s: %s' % (q(k), lit(x)) for k, x in v.items()) + '}'
    if v is None:
        return 'null'
    raise SystemExit('无法渲染字面量：%r' % (v,))


SPEC = {
 'strings': {
  'index_of': {"legal": [("hello", "l"), ("中文ab", "b"), ("", ""), ("aaa", "a")],
               "pos": [("hello", "l")]},
  'index_of_from': {"legal": [("abcabc", "bc", 2), ("abc", "a", -5), ("abc", "z", 1), ("abc", "a", 99)],
                    "pos": [("abcabc", "bc", 2)]},
  'last_index_of': {"legal": [("abcabc", "bc"), ("abc", ""), ("中文中文", "文")],
                    "pos": [("abcabc", "bc")]},
  'count_of': {"legal": [("abcabc", "bc"), ("aaa", "aa"), ("abc", ""), ("aaaa", "aa")],
               "pos": [("abcabc", "bc")]},
  'split_n': {"legal": [("a,b,c", ",", 2), ("abc", "", 3), ("a,b,c", ",", 1), ("a,b,c", ",", 9)],
              "pos": [("a,b,c", ",", 2)]},
  'split_lines': {"legal": [("a\r\nb\nc",), ("",), ("a\n",)],
                  "pos": [("a\nb",)]},
  'repeat_str': {"legal": [("ab", 3), ("ab", 0), ("ab", -1), ("", 5)],
                 "pos": [("ab", 3)]},
  'pad_left': {"legal": [("7", 3, "0"), ("777", 2, "0"), ("7", 3, ""), ("", 3, "0")],
               "pos": [("7", 3, "0")]},
  'pad_right': {"legal": [("7", 3, "."), ("7", 1, "."), ("7", 3, ""), ("", 3, "0")],
                "pos": [("7", 3, ".")]},
  'str_reverse': {"legal": [("中文a",), ("",), ("ab",)],
                  "pos": [("abc",)]},
  'trim_chars': {"legal": [("aa", "a"), ("ab", ""), ("\"x\"", "\"")],
                 "pos": [("aaxaa", "a")]},
  'trim_left_chars': {"legal": [("xxay", "x"), ("ab", "b"), ("", "x")],
                      "pos": [("xxay", "x")]},
  'trim_right_chars': {"legal": [("ayxx", "x"), ("ab", "a"), ("", "x")],
                       "pos": [("ayxx", "x")]},
  'trim_prefix': {"legal": [("wsa-id", "wsa-"), ("abc", "z"), ("abc", ""), ("abc", "abcd")],
                  "pos": [("wsa-id", "wsa-")]},
  'trim_suffix': {"legal": [("a/", "/"), ("abc", "z"), ("abc", ""), ("ab", "abcd")],
                  "pos": [("ab/", "/")]},
  'is_hex': {"legal": [("4aF0",), ("",), ("abc",), ("00",)],
             "pos": [("4aF0",)]},
 },
 'url': {
  'url_query_escape': {"legal": [("a b~!",), ("",), ("中",), ("a/b?c=d&e",)],
                       "pos": [("a b",)]},
  'url_path_escape': {"legal": [("a/b c",), ("a~b",), ("中",), ("a+b",)],
                      "pos": [("a/b c",)]},
  'url_query_unescape': {"legal": [("%41+b",), ("%",), ("%zz",), ("%4",)],
                         "pos": [("%41+b",)]},
  'url_query_unescape_keep_q': {"legal": [("%41+b",), ("%",), ("a%2Fb",), ("%4",)],
                                "pos": [("%41+b",)]},
  'url_parse': {"legal": [],
                "legal_custom": [
                    ('url_parse("https://u@h:8080/p?q=1#f")["scheme"]', lambda M: M.url_parse("https://u@h:8080/p?q=1#f")["scheme"]),
                    ('url_parse("https://u@h:8080/p?q=1#f")["user"]', lambda M: M.url_parse("https://u@h:8080/p?q=1#f")["user"]),
                    ('url_parse("https://u@h:8080/p?q=1#f")["host"]', lambda M: M.url_parse("https://u@h:8080/p?q=1#f")["host"]),
                    ('url_parse("https://u@h:8080/p?q=1#f")["path"]', lambda M: M.url_parse("https://u@h:8080/p?q=1#f")["path"]),
                    ('url_parse("https://u@h:8080/p?q=1#f")["rawquery"]', lambda M: M.url_parse("https://u@h:8080/p?q=1#f")["rawquery"]),
                    ('url_parse("https://u@h:8080/p?q=1#f")["fragment"]', lambda M: M.url_parse("https://u@h:8080/p?q=1#f")["fragment"]),
                    ('url_parse("/a%20b")["path"]', lambda M: M.url_parse("/a%20b")["path"]),
                    ('url_parse("")["path"]', lambda M: M.url_parse("")["path"]),
                ],
                "pos": ["url_parse(\"https://h/p\")"]},
  'url_parse_query': {"legal": [],
                      "legal_custom": [
                          ('url_parse_query("a=1&a=2&b=%41")[0]["a"]', lambda M: M.url_parse_query("a=1&a=2&b=%41")[0]["a"]),
                          ('url_parse_query("a=1")[1]', lambda M: M.url_parse_query("a=1")[1]),
                          ('url_parse_query("a=1;b=2")[1]', lambda M: M.url_parse_query("a=1;b=2")[1]),
                          ('url_parse_query("a=1&a=2")[0]["a"]', lambda M: M.url_parse_query("a=1&a=2")[0]["a"]),
                      ],
                      "pos": ['url_parse_query("a=1")']},
  'url_values_encode': {"legal": [({"a": "1"},), ({"a": ["1", "2"]},), ({},)],
                        "pos": [({"a": "1"},)]},
 },
}


def derive(root, mods):
    out = {}
    for m in mods:
        p = os.path.join(root, 'stdlib', m + '.px')
        if not os.path.exists(p):
            print('❌ 缺源码 %s' % p, file=sys.stderr); sys.exit(3)
        d = {}
        for i, ln in enumerate(open(p, encoding='utf-8'), 1):
            mm = re.match(r'^def ([A-Za-z_][A-Za-z0-9_]*)\((.*)\)\s*:', ln)
            if mm:
                a = [x.strip() for x in mm.group(2).split(',') if x.strip()]
                d[mm.group(1)] = (len(a), a, i)
        out[m] = d
    return out


def split_args(args):
    parts, depth, cur, qq = [], 0, '', False
    for ch in args:
        if ch == '"':
            qq = not qq
        if not qq:
            if ch in '([{':
                depth += 1
            elif ch in ')]}':
                depth -= 1
            elif ch == ',' and depth == 0:
                parts.append(cur); cur = ''; continue
        cur += ch
    parts.append(cur)
    return [p.strip() for p in parts]


def subst(expr, pos, pv):
    m = re.match(r'^([A-Za-z_][A-Za-z0-9_]*)\((.*)\)$', expr)
    if not m:
        raise SystemExit('无法解析表达式：' + expr)
    parts = split_args(m.group(2))
    if pos >= len(parts):
        raise SystemExit('位置越界 %s pos=%d' % (expr, pos))
    parts[pos] = pv
    return m.group(1) + '(' + ', '.join(parts) + ')'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', required=True)
    ap.add_argument('--work', required=True)
    ap.add_argument('--mods', default='strings,url')
    a = ap.parse_args()
    mods = [m.strip() for m in a.mods.split(',') if m.strip()]
    src = derive(a.root, mods)

    src_fns = {}
    for m in mods:
        for fn, (ar, args, ln) in src[m].items():
            if fn.startswith('_'):
                continue
            src_fns[fn] = (m, ar, args)
    spec_fns = set()
    for m in mods:
        spec_fns |= set(SPEC.get(m, {}))
    missing = sorted(set(src_fns) - spec_fns)
    extra = sorted(spec_fns - set(src_fns))
    if missing or extra:
        print('❌ 规格表与源码不一致：源码有而表缺 %s · 表有而源码无 %s' % (missing, extra), file=sys.stderr)
        sys.exit(3)

    cases = []   # (label, kind, expr, expect)
    for m in mods:
        for fn in sorted(SPEC[m]):
            sp = SPEC[m][fn]
            mm, ar, argnames = src_fns[fn]
            legal = []
            for at in sp.get('legal', []):
                if len(at) != ar:
                    print('❌ %s 的合法侧实参个数 %d ≠ 源码 arity %d' % (fn, len(at), ar), file=sys.stderr)
                    sys.exit(3)
                legal.append(('%s(%s)' % (fn, ', '.join(lit(x) for x in at)),
                              R.compute(fn, at)))
            for e, f in sp.get('legal_custom', []):
                legal.append((e, R.render(f(R))))
            for i, (e, want) in enumerate(legal):
                cases.append(('L%s_%s_%d' % (m[0], fn, i + 1), 'legal', e, want))
            for i, e in enumerate(sp.get('bound', [])):
                cases.append(('B%s_%s_%d' % (m[0], fn, i + 1), 'bound', e, 'LOCK'))
            for bi, base in enumerate(sp.get('pos', [])):
                if isinstance(base, tuple):
                    base = '%s(%s)' % (fn, ', '.join(lit(x) for x in base))
                mm2 = re.match(r'^([A-Za-z_][A-Za-z0-9_]*)\((.*)\)$', base)
                if not mm2:
                    print('❌ 无法解析 pos base：%s（应写成 `fn(...)` 或参数元组）' % base, file=sys.stderr)
                    sys.exit(3)
                na = len(split_args(mm2.group(2)))
                for pi in range(na):
                    for qi, pv in enumerate(POISON):
                        cases.append(('P%s_%s_b%dp%dq%d' % (m[0], fn, bi, pi, qi), 'poison',
                                      subst(base, pi, pv), 'LOCK'))
            if ar >= 1:
                cases.append(('A%s_%s_less' % (m[0], fn), 'arity',
                              fn + '(' + ', '.join(['null'] * (ar - 1)) + ')', 'LOCK'))
            cases.append(('A%s_%s_more' % (m[0], fn), 'arity',
                          fn + '(' + ', '.join(['null'] * (ar + 1)) + ')', 'LOCK'))

    lines = ['import std.strings', 'import std.url', '', 'def main():',
             '    var a = args()',
             '    var c = a[1]',
             '    var hit = 0']
    for lb, kind, expr, want in cases:
        lines.append('    if c == "%s":' % lb)
        lines.append('        hit = 1')
        lines.append('        print(%s)' % expr)
    lines.append('    if hit == 0:')
    lines.append('        print("NOCASE")')
    open(os.path.join(a.work, 'drv.px'), 'w', encoding='utf-8').write('\n'.join(lines) + '\n')
    with open(os.path.join(a.work, 'CASES.tsv'), 'w', encoding='utf-8') as f:
        f.write('# label\tkind\texpr\texpect\n')
        for lb, kind, expr, want in cases:
            f.write('%s\t%s\t%s\t%s\n' % (lb, kind, expr, want))

    by = {}
    for _, k, _, _ in cases:
        by[k] = by.get(k, 0) + 1
    print('函数 %d 个（%s）· 用例 %d 条 %s' % (len(src_fns), '+'.join(mods), len(cases), by))
    if len(src_fns) < 20 or len(cases) < 300:
        print('❌ 规模低于下限（函数≥20 · 用例≥300）', file=sys.stderr); sys.exit(3)
    return 0


if __name__ == '__main__':
    sys.exit(main())
