#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M302 用例生成器 —— stdlib 第三批（semver + html）
# ------------------------------------------------------------
# 四类用例（与 M199/M226/M279/M302 同一范式）：
#   legal  —— 合法侧，**期望值由 ref.py 独立给出**（不由三轨自证）
#   neg    —— 非法输入，**期望响亮**（R 码必须等于 expect；消息体由 verify.sh 另判）
#   poison —— 逐位置 × 错类型毒值（LOCK：三轨自证一致）
#   arity  —— 错 arity（LOCK）
# 输出：<work>/drv.px（聚合驱动器 · args()[1] 选例）+ CASES.tsv
# ============================================================
import argparse, os, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ref  # noqa: E402


def pystr(v):
    if v is None:
        return 'null'
    if v is True:
        return 'true'
    if v is False:
        return 'false'
    return str(v)


ap = argparse.ArgumentParser()
ap.add_argument('--work', required=True)
a = ap.parse_args()
os.makedirs(a.work, exist_ok=True)

cases = []


def add(lb, expr, want, kind='legal'):
    assert '\t' not in expr and '\n' not in expr, lb
    cases.append((lb, kind, expr, want))


# ─────────────────────────── A 合法比较（semver.org §11 优先级链）
CHAIN = ["1.0.0-alpha", "1.0.0-alpha.1", "1.0.0-alpha.beta", "1.0.0-beta",
         "1.0.0-beta.2", "1.0.0-beta.11", "1.0.0-rc.1", "1.0.0", "2.0.0"]
for i in range(len(CHAIN) - 1):
    x, y = CHAIN[i], CHAIN[i + 1]
    add(f"C{i}", f'sv_cmp("{x}", "{y}")', pystr(ref.cmp_versions(x, y)))
    add(f"Cr{i}", f'sv_cmp("{y}", "{x}")', pystr(ref.cmp_versions(y, x)))
for x, y in [("1.0.0", "1.0.0+b"), ("1.0.0+a", "1.0.0+b"), ("1.0.0-1", "1.0.0-alpha"),
             ("1.0.0-alpha.1", "1.0.0-alpha.1.1"), ("0.9.9", "1.0.0"), ("1.2.3", "1.2.3")]:
    add("Cx_" + x.replace(".", "_").replace("+", "p").replace("-", "m") + "_" +
        y.replace(".", "_").replace("+", "p").replace("-", "m"), f'sv_cmp("{x}", "{y}")',
        pystr(ref.cmp_versions(x, y)))

# ─────────────────────────── B sv_parse 字段
for i, (v, k) in enumerate([("1.2.3-alpha.1+build5", "major"), ("1.2.3-alpha.1+build5", "pre"),
                            ("1.2.3-alpha.1+build5", "build"), ("10.20.30", "minor"),
                            ("0.0.0", "patch"), ("1.0.0-rc.1", "pre"), ("1.0.0", "pre"),
                            ("1.0.0", "build")]):
    p = ref.parse(v)
    m = {'major': p[0], 'minor': p[1], 'patch': p[2], 'pre': p[3], 'build': p[4]}[k]
    if k == 'pre':
        m = '.'.join(str(t[1]) for t in p[3])
    want = pystr(m) if not isinstance(m, str) else m
    add(f"P{i}", f'sv_parse("{v}")["{k}"]', want)
for i, v in enumerate(["1.2", "01.2.3", "1.2.3-01", "1.2.3.4", "v1.2.3", "1..2", "1.2.3-", "1.2.3+", "", "abc"]):
    add(f"Pb{i}", f'str(sv_parse("{v}"))', pystr(ref.parse(v)))

# ─────────────────────────── C satisfies 矩阵（真值 = ref 的 range 展开）
RANGES = ["^1.2.3", "^0.2.3", "^0.0.3", "^0", "^0.0", "^0.x", "^1.x", "^1",
          "~1.2.3", "~1.2", "~1", "~0.0", "1.2.x", "1.x", "1", "*", "", "1.2.3"]
VERS = ["0.0.0", "0.0.3", "0.0.4", "0.1.0", "0.2.3", "0.2.5", "0.3.0",
        "1.0.0", "1.2.3", "1.2.9", "1.3.0", "1.9.9", "2.0.0", "1.0.0-alpha"]
for i, r in enumerate(RANGES):
    for j, v in enumerate(VERS):
        add(f"S{i}_{j}", f'sv_satisfies("{v}", "{r}")', pystr(ref.satisfies(v, r)))

# ─────────────────────────── D sv_best
for i, (vs, r) in enumerate([(["1.2.0", "1.2.5", "2.0.0"], "^1.2.0"),
                             (["1.2.0", "1.2.5", "2.0.0"], "^2.0.0"),
                             (["1.0.0", "0.9.0"], "^1.0.0"),
                             (["3.0.0"], "^1.0.0"),
                             (["1.2.0", "1.2.9", "1.3.0"], "~1.2"),
                             (["0.0.1", "0.0.2"], "^0.0.1"),
                             (["1.0.0", "1.0.0-alpha"], "^1.0.0"),
                             (["2.0.0", "1.0.0"], "*")]):
    lit = '[' + ', '.join('"%s"' % x for x in vs) + ']'
    add(f"B{i}", f'str(sv_best({lit}, "{r}"))', pystr(ref.best(vs, r)))

# ─────────────────────────── E html_escape
ESC_IN = ["", "abc", "<a>", "&", "\"", "'", "a&b", "<p class='x'>", "\u4e2d\u6587",
          "&&", "><", "it's", "<script>alert(1)</script>", "a&amp;b", "\u00a0"]
for i, s in enumerate(ESC_IN):
    q = s.replace('\\', '\\\\').replace('"', '\\"')
    add(f"E{i}", f'html_escape("{q}")', ref.escape(s))

# ─────────────────────────── F hx_decode
DEC_IN = ["", "abc", "&amp;", "&lt;", "&gt;", "&quot;", "&apos;", "&nbsp;", "&copy;",
          "&#65;", "&AMP;", "a&amp;b", "&amp", "&nosuch;", "&", "&&amp;", "&amp;&amp;",
          "x&nbsp;y", "&#x41;", "&amp;;"]
for i, s in enumerate(DEC_IN):
    q = s.replace('\\', '\\\\').replace('"', '\\"')
    add(f"D{i}", f'hx_decode("{q}")', ref.decode(s))

# ─────────────────────────── G DOM 族（真值 = 人写的契约）
def html_tag_of_first_child(h):
    """G{i}c 的期望：根元素第一个子元素的 tag（人写契约）"""
    tbl = {'<div id=\\"x\\">a<b>b</b></div>': '#text',
           '<p>x</p><!--c-->': '#text',
           '<div><script>1<2</script>y</div>': 'script',
           '<span class=\\"k\\">v</span>': '#text',
           '<ul><li>1</li><li>2</li></ul>': 'li'}
    return tbl.get(h, '')


DOM = [
    ('<div id=\\"x\\">a<b>b</b></div>', "ab", "div", "div", "id", "x", 1, 2),
    ('<p>x</p><!--c-->', "x", "p", "p", "class", "", 1, 1),
    ('<div><script>1<2</script>y</div>', "y", "div", "div", "class", "", 1, 2),
    ('<span class=\\"k\\">v</span>', "v", "span", "span", "class", "k", 1, 1),
    ('<ul><li>1</li><li>2</li></ul>', "12", "ul", "ul", "class", "", 1, 2),
]
for i, (h, txt, root_tag, want_root, an, av, nq, nkids) in enumerate(DOM):
    add(f"G{i}t", f'html_text(html_parse("{h}"))', txt)
    add(f"G{i}g", f'html_tag(html_parse("{h}")["children"][0])', want_root)
    add(f"G{i}q", f'str(len(html_query(html_parse("{h}"), "{root_tag}")))', pystr(nq))
    add(f"G{i}a", f'html_attr(html_parse("{h}")["children"][0], "{an}")', av)
    add(f"G{i}k", f'str(len(html_parse("{h}")["children"][0]["children"]))', pystr(nkids))
    add(f"G{i}c", f'html_tag(html_children(html_parse("{h}")["children"][0])[0])',
        html_tag_of_first_child(h))
add("Gq_multi", 'str(len(html_query(html_parse("<p>1</p><p>2</p>"), "p")))', "2")
add("Gq_cls", 'str(len(html_query(html_parse("<p class=\'a\'>1</p><p>2</p>"), "p.a")))', "1")
add("Gent", 'str(len(hx_entities()))', "6")
add("Gvoid", 'str(len(hx_void_tags()))', "14")
add("Graw", 'str(len(hx_raw_tags()))', "4")
add("Groot", 'html_tag(html_parse("<p>x</p>"))', "#root")
add("Gempty", 'html_text(html_parse(""))', "")
add("Gtext_node", 'html_text(html_parse("<p>a</p>")["children"][0]["children"][0])', "a")

# ─────────────────────────── H 非法输入（响亮）
NEG_VER = ["abc", "1.2", "1.2.3.4", "01.2.3", "v1.2.3", "1..2", "1.2.3-", "1.2.3+",
           "-1.2.3", "1.2.x", "", "a.b.c", "1.2.3-01", "+1.2.3", "1.2.3-alpha..1"]
for i, v in enumerate(NEG_VER):
    add(f"N{i}", f'sv_cmp("{v}", "1.2.3")', "R2001", "neg")
    add(f"N{i}b", f'sv_satisfies("{v}", "^1.0.0")', "false")     # 谓词：false 是正当口径
for i, r in enumerate(["abc", "^", "~", "^^1.2.3", "1.2.3.4", "01.2.3", "*x", "v1.2.3"]):
    add(f"Nr{i}", f'sv_satisfies("1.5.0", "{r}")', "false")

# ─────────────────────────── I 毒值 / arity（LOCK）
POISON = ["null", "true", "7", '"s"', "([])"]   # 5 个毒值（规模控制：38 个位置 × 5 = 190）
SIG = {}
for line in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'SIGS.tsv'), encoding='utf-8'):
    if line.startswith('#') or not line.strip():
        continue
    p = line.rstrip('\n').split('\t')
    SIG[p[0]] = (int(p[1]), [x for x in p[2].split('|') if x != ''])
for fn, (ar, base) in SIG.items():
    assert len(base) == ar, fn
    for pi in range(ar):
        for qi, pv in enumerate(POISON):
            args = list(base)
            args[pi] = pv
            add(f"Z{fn}_b{pi}q{qi}", '%s(%s)' % (fn, ', '.join(args)), "LOCK", "poison")
    if ar >= 1:
        add(f"A{fn}_less", '%s(%s)' % (fn, ', '.join(['null'] * (ar - 1))), "LOCK", "arity")
    add(f"A{fn}_more", '%s(%s)' % (fn, ', '.join(['null'] * (ar + 1))), "LOCK", "arity")

# ─────────────────────────── 生成
L = ["import std.semver", "import std.html", "", "def main():",
     "    var a = args()", "    var c = a[1]", "    var hit = 0"]
for lb, _k, expr, _w in cases:
    L += ['    if c == "%s":' % lb, '        hit = 1', '        print(%s)' % expr]
L += ['    if hit == 0:', '        print("NOCASE")']
open(os.path.join(a.work, 'drv.px'), 'w', encoding='utf-8').write("\n".join(L) + "\n")

out = os.path.join(a.work, 'CASES.tsv')
with open(out, 'w', encoding='utf-8') as f:
    f.write('# label\tkind\texpr\texpect\n')
    for lb, k, expr, w in cases:
        f.write('%s\t%s\t%s\t%s\n' % (lb, k, expr, w))

by = {}
for _, k, _, _ in cases:
    by[k] = by.get(k, 0) + 1
print('用例 %d 条 %s（函数 %d 个）' % (len(cases), by, len(SIG)))
if len(SIG) < 20 or len(cases) < 260:
    print('❌ 规模低于下限（函数≥20 · 用例≥260）', file=sys.stderr)
    sys.exit(3)
sys.exit(0)
