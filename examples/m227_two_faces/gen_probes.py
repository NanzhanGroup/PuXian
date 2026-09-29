#!/usr/bin/env python3
# ============================================================
# M227 探针生成器：「同名两门」（函数面 ⇄ 方法面）全量对拍
# ------------------------------------------------------------
# 为什么需要（本轮立判据的由来）：
#   M199 量过 **native 函数面** 的「逐位置 × 错类型」，M226 量过 **方法面** 的同款 ——
#   但**没人量过「同一个操作的两个门」**：`len(x)` 与 `x.len()` 是同一个操作，
#   从哪个门进应该看到同一套判据。M199 缺陷 238（`replace` 从函数面进却拿到方法面词条
#   `R1007 类型 dict 没有方法 'replace'`）正是这个形状的**第一例**，但那是**偶然撞上**的。
#
# 判据（期望集**从源码派生** · M212 缺陷 288 的纪律）：
#   [a] `xface.tsv` 的名字集合 ⇄ `G ∩ ∪M` **精确相等**（双向）
#         G = runtime.c 的 `px_set_global("N", px_native("N", …))` 名字集
#         M = runtime.c `px_method` 的 strcmp 链 ∪ selfhost/icall.px 的 `i_*_method` name== 链
#   [b] 每一行必须分类：`pair`（同名同义 ⇒ 参与对拍）/ `homonym`（假朋友 ⇒ 必须写明理由）。
#   [c] 规模锚点（G ≥ 300 · 方法 ≥ 30 · 交集 = 12）—— 防「派生失败 ⇒ 空集 ⊇ 任意集」的假绿。
#   [d] 生成聚合驱动器（`args()[1]` 选例 · `args()[2]` 选面）⇒ **一次编译**覆盖两面全量用例。
# 输出：<out>/drv.px · <out>/cases.tsv · <out>/derive.json
# ============================================================
import json, os, re, sys, argparse

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
ap.add_argument('--out', default='/tmp/m227')
ap.add_argument('--here', default=os.path.dirname(os.path.abspath(__file__)))
ap.add_argument('--spec', default=None, help='m226 spec.tsv（合法实参的单一事实源）')
a = ap.parse_args()
ROOT, OUT = a.root, a.out
HERE = a.here
SPEC_PATH = a.spec or os.path.join(ROOT, 'examples/m226_method_surface/spec.tsv')
os.makedirs(OUT, exist_ok=True)

TYPES = {'PX_STR': 'str', 'PX_LIST': 'list', 'PX_DICT': 'dict', 'PX_CHAN': 'chan',
         'PX_MUTEX': 'mutex', 'PX_RWLOCK': 'rwlock', 'PX_RESULT': 'result',
         'PX_TUPLE': 'tuple', 'PX_STRUCT': 'struct'}
I_FUNCS = {'str': 'i_str_method', 'list': 'i_list_method', 'dict': 'i_dict_method',
           'result': 'i_result_method'}

# ── 「同名同义」两门的**逻辑参数**模型 ──
# 逻辑参数 = [接收者, 实参1, 实参2, …]（两面**同一套逻辑参数**）。
# `func_order` = 函数面把逻辑参数**按什么语法顺序**摆（`join` 就是顺序相反的那个）；
# 方法面固定为「接收者在前、其余按逻辑序」，不需要另写。
# ⚠️ 逻辑参数必须一一对应，否则对拍比的是两个不同的操作（那是判据自己的 bug）。
#    `n` = 逻辑参数个数（= 1 + 实参个数）。
PAIR_TPL = {                     # 名字: (func_order, n)
    'len':         ([0],          1),
    'contains':    ([0, 1],       2),
    'starts_with': ([0, 1],       2),
    'ends_with':   ([0, 1],       2),
    'split':       ([0, 1],       2),
    'join':        ([1, 0],       2),   # ⚠ 函数面 `join(分隔符, 序列)` ⇄ 方法面 `序列.join(分隔符)`
    'trim':        ([0],          1),
    'to_upper':    ([0],          1),
    'to_lower':    ([0],          1),
    'replace':     ([0, 1, 2],    3),
}


def render(name, logical):
    """(名字, 逻辑参数) → (函数面源码, 方法面源码)。

    逻辑参数 = [接收者, 实参…]；`logical` 允许**多于**声明个数（错 arity 探针补尾参），
    此时**两面都要把尾参摆进语法实参表** —— 首版只给方法面补了尾参、函数面被静默丢掉，
    于是伪造出「函数面静默忽略多余实参」11 例。**那是判据自己的 bug，不是语言的缺陷**
    （M220/M223 同款教训：负控/探针不忠实 ⇒ 结论不可信）。
    """
    order, n = PAIR_TPL[name]
    core, extra = logical[:n], list(logical[n:])
    f = '%s(%s)' % (name, ', '.join([core[i] for i in order if i < len(core)] + extra))
    m = '(%s).%s(%s)' % (logical[0], name, ', '.join(list(logical[1:n]) + extra)) if logical else name
    return f, m


def strip_c(src):
    """去注释；**保留字符串字面量**（全局名/方法名就在里面）"""
    out, i, n = [], 0, len(src)
    while i < n:
        if src[i:i+2] == '/*':
            j = src.find('*/', i + 2); j = n if j < 0 else j + 2
            out.append('\n' * src.count('\n', i, j)); i = j
        elif src[i:i+2] == '//':
            j = src.find('\n', i); j = n if j < 0 else j
            out.append('\n' * src.count('\n', i, j)); i = j
        elif src[i] == '"':
            j = i + 1
            while j < n and src[j] != '"':
                j += 2 if src[j] == '\\' else 1
            out.append(src[i:j+1]); i = j + 1
        else:
            out.append(src[i]); i += 1
    return ''.join(out)


def braced(src, sig):
    m = re.search(sig, src)
    if not m:
        return None
    i = src.find('{', m.end() - 1)
    if i < 0:
        return None
    d, j = 0, i
    while j < len(src):
        if src[j] == '{':
            d += 1
        elif src[j] == '}':
            d -= 1
            if d == 0:
                return src[i:j+1]
        j += 1
    return None


def indent_block(src, name):
    """PuXian 是缩进语法：`def name(...)` 起，到首个「非空且缩进 <= def 缩进」的行止"""
    lines = src.splitlines()
    for k, ln in enumerate(lines):
        if re.match(rf'\s*def\s+{re.escape(name)}\s*\(', ln):
            ind = len(ln) - len(ln.lstrip())
            body = [ln]
            for ln2 in lines[k+1:]:
                if not ln2.strip():
                    body.append(ln2); continue
                if (len(ln2) - len(ln2.lstrip())) <= ind:
                    break
                body.append(ln2)
            return '\n'.join(body)
    return None


# ── [a] 源码派生：G（函数面）与 M（方法面） ──
rt = strip_c(open(os.path.join(ROOT, 'runtime/runtime.c'), encoding='utf-8').read())
G = set(re.findall(r'px_set_global\(\s*"([A-Za-z_][A-Za-z0-9_]*)"\s*,\s*px_native\(\s*"', rt))
if len(G) < 50:
    sys.exit(f'❌ 派生失败：函数面全局名只派生出 {len(G)} 条（源码形态可能变了）')

body = braced(rt, r'LXValue\s+px_method\s*\(')
if not body:
    sys.exit('❌ 派生失败：runtime.c 里找不到 px_method 的函数体')
C = {}
for m in re.finditer(r'if\s*\(\s*obj\.type\s*==\s*(PX_[A-Z]+)\s*\)\s*\{', body):
    t = TYPES.get(m.group(1), m.group(1))
    i = m.end() - 1
    d, j = 0, i
    while j < len(body):
        if body[j] == '{':
            d += 1
        elif body[j] == '}':
            d -= 1
            if d == 0:
                break
        j += 1
    C.setdefault(t, set()).update(re.findall(r'strcmp\s*\(\s*name\s*,\s*"([a-z_0-9]+)"\s*\)', body[i:j+1]))
for t in C:
    C[t].discard('')

ic = '\n'.join(re.sub(r'#.*$', '', ln) for ln in
               open(os.path.join(ROOT, 'selfhost/icall.px'), encoding='utf-8').read().splitlines())
I = {}
for t, fn in I_FUNCS.items():
    b = indent_block(ic, fn)
    if not b:
        sys.exit(f'❌ 派生失败：icall.px 里找不到 {fn}')
    I[t] = set(re.findall(r'name\s*==\s*"([a-z_0-9]+)"', b))
    I[t].discard('')

MALL = set()
for t in set(C) | set(I):
    MALL |= C.get(t, set()) | I.get(t, set())

print(f'[派生] 函数面（runtime.c `px_set_global(…, px_native(…)`）：{len(G)} 个全局名')
print(f'[派生] 方法面（runtime.c px_method ∪ icall.px i_*_method）：{len(MALL)} 个方法名'
      f'（' + ' · '.join(f'{t}={len(C.get(t, set()) | I.get(t, set()))}'
                          for t in sorted(set(C) | set(I))) + '）')
inter = G & MALL
print(f'[交集] 同名两门 {len(inter)} 个：' + ' '.join(sorted(inter)))
print(f'[锚点] 规模 ok（函数面全局名 {len(G)} ≥ 300 · 方法面方法名 {len(MALL)} ≥ 30 · 同名两门 {len(inter)} = 12）')

# ── [c] 规模锚点（防「派生失败 ⇒ 空集 ⊇ 任意集」）──
errs = []
if len(G) < 300:
    errs.append(f'函数面全局名 {len(G)} < 300（规模锚点，派生可能失效）')
if len(MALL) < 30:
    errs.append(f'方法面方法名 {len(MALL)} < 30（规模锚点）')
if len(inter) != 12:
    errs.append(f'同名两门 {len(inter)} ≠ 12（新增/删除同名操作必须同步 xface.tsv 与本判据）')

# ── [a][b] xface.tsv ⇄ 派生交集 ──
rows, seen, bad = [], {}, []
for ln in open(os.path.join(HERE, 'xface.tsv'), encoding='utf-8'):
    ln = ln.rstrip('\n')
    if not ln.strip() or ln.lstrip().startswith('#'):
        continue
    f = (ln.split('\t') + ['', ''])[:3]
    if f[0].strip() == '名字':
        continue
    nm, cls, note = f[0].strip(), f[1].strip(), f[2].strip()
    if cls not in ('pair', 'homonym'):
        bad.append(f'{nm}：类别 `{cls}` 不是 pair/homonym')
        continue
    if cls == 'homonym' and len(note) < 20:
        bad.append(f'{nm}：假朋友必须写明理由（现在只有 {len(note)} 字）')
    if cls == 'pair' and nm not in PAIR_TPL:
        bad.append(f'{nm}：类别是 pair 但 PAIR_TPL 里没有模板')
    seen[nm] = cls
    rows.append((nm, cls))

if set(seen) != inter:
    errs.append('xface.tsv ⇄ 派生交集 不相等')
    miss = sorted(inter - set(seen)); extra = sorted(set(seen) - inter)
    if miss:
        errs.append(f'   源码有、xface.tsv 未登记（**探针面会静默变窄**）：{miss}')
    if extra:
        errs.append(f'   xface.tsv 有、源码已经不是同名（悬空条目）：{extra}')
tpl_names = {n for n, c in seen.items() if c == 'pair'}
if tpl_names != set(PAIR_TPL):
    errs.append(f'PAIR_TPL ⇄ pair 行不相等：模板多 {sorted(set(PAIR_TPL) - tpl_names)} · 少 {sorted(tpl_names - set(PAIR_TPL))}')
if bad:
    errs += ['xface.tsv 格式问题：'] + ['   · ' + b for b in bad]
if errs:
    for e in errs:
        print('❌ ' + e)
    sys.exit(2)
print(f'[清单] xface.tsv ⇄ 派生交集精确相等（{len(rows)} 行：pair {len(tpl_names)} · homonym {len(rows)-len(tpl_names)}）')

# ── [d] 合法实参：复用 m226 spec.tsv（单一事实源，不抄第二份）──
SPEC = []
for ln in open(SPEC_PATH, encoding='utf-8'):
    ln = ln.rstrip('\n')
    if not ln.strip() or ln.lstrip().startswith('#'):
        continue
    f = (ln.split('\t') + ['', '', ''])[:4]
    if f[0].strip() == '类型':
        continue
    t, meth, argstr = f[0].strip(), f[1].strip(), f[2].strip()
    if meth not in tpl_names:
        continue
    valid = [x for x in argstr.split('|') if x != ''] if argstr else []
    SPEC.append((t, meth, valid))
spec_names = {m for _, m, _ in SPEC}
if spec_names != tpl_names:
    sys.exit(f'❌ m226 spec.tsv 里缺 pair 方法（{sorted(tpl_names - spec_names)}）⇒ 合法实参无来源')
R = {'str': '"abc"', 'list': '[1, 2]', 'dict': '{"a": 1}'}
POISONS = ['{}', '7', '"s"', '[1]', 'null', 'true', '(1, 2)']


def lbl(*ps):
    return '_'.join(re.sub(r'[^0-9A-Za-z]+', '', str(x)) or 'z' for x in ps)


cases = []   # (label, func_src, meth_src)
for t, m, valid in SPEC:
    rcv = R[t]

    def build(args):
        return render(m, [rcv] + list(args))

    for p in range(len(valid) + 1):          # p=0 ⇒ **接收者**位置
        for pz in POISONS:
            args = list(valid)
            if p == 0:
                f, mm = render(m, [pz] + args)
            else:
                args[p - 1] = pz
                f, mm = build(args)
            cases.append((lbl('m227', m, t, 'p%d' % p, pz), f, mm))
    cases.append((lbl('m227', m, t, 'ok'), *build(list(valid))))
    # 错 arity：⚠️ 两面的**语法**实参个数天生差 1（接收者是不是实参）⇒ 探针必须按**逻辑**参数算：
    #   缺 = 丢掉**全部非接收者**实参（`contains(x)` ⇄ `(x).contains()`）；多 = 两面各补两个尾参。
    #   首版按「语法 0 参」造 ⇒ `len()`（函数面缺参）⇄ `("abc").len()`（方法面合法）=
    #   拿两个不同的操作对拍 ⇒ 又是探针自己造的红。
    if valid:
        cases.append((lbl('m227', m, t, 'nmiss'), *render(m, [rcv])))
    cases.append((lbl('m227', m, t, 'nextra'), *render(m, [rcv] + list(valid) + ['1', '2'])))

L = ['let a = args()', 'if len(a) < 3:', '    print("NOCASE")', 'else:',
     '    let c = a[1]', '    let fc = a[2]', '    var hit = false']
for lb, f, mm in cases:
    L += ['    if c == "%s":' % lb, '        hit = true', '        if fc == "f":',
          '            print(%s)' % f, '        else:', '            print(%s)' % mm]
L += ['    if hit == false:', '        print("NOCASE")']
open(os.path.join(OUT, 'drv.px'), 'w').write('\n'.join(L) + '\n')
with open(os.path.join(OUT, 'cases.tsv'), 'w') as fh:
    for lb, f, mm in cases:
        fh.write(f'{lb}\t{f}\t{mm}\n')
json.dump({'globals': sorted(G), 'methods': {k: sorted(v) for k, v in C.items()},
           'i_methods': {k: sorted(v) for k, v in I.items()}, 'inter': sorted(inter)},
          open(os.path.join(OUT, 'derive.json'), 'w'), ensure_ascii=False, indent=1)
print(f'[探针] 生成 {len(cases)} 例（{len(SPEC)} 个 (类型, 方法) 行 · 两面各一）→ {OUT}/drv.px（{len(L)} 行）')
