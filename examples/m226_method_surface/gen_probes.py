#!/usr/bin/env python3
# ============================================================
# M226 方法面探针生成器 + 清单判据（[S10-c]）
# ------------------------------------------------------------
# 为什么需要（本轮立判据的由来）：
#   M190 量过「方法实参个数」、M199 量过「native **函数**面 的逐位置 × 错类型」，
#   但**方法面**从来没人做过清单级 / 全量度量。本轮实测定量：
#   222 例 → **32 例分叉**，落在 6 个族（越界崩溃 / 静默多余实参 / 缺参口径 /
#   清单不对称 ×2 / 文案 / 缺键语义）。
#
# 判据：
#   [a] 从**源码派生**方法清单（runtime.c `px_method` 的 strcmp 链 + icall.px 的 name== 链）
#       —— 判据的期望集不许手抄（M212 缺陷 288）。
#   [b] spec.tsv ⇄ 派生清单：双向相等（少登记 = 探针面静默变窄；多登记 = 悬空条目）。
#   [c] UNPAIRED.tsv ⇄ 实测差集：精确相等（否则该表退化成「把红当绿记下来」）。
#   [d] 生成聚合驱动器（按 args()[1] 分派）⇒ 两次编译覆盖全量用例。
# 输出：<out>/drv.px · <out>/cases.tsv · 结论行供 verify.sh 判据
# ============================================================
import json, os, re, sys, argparse

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
ap.add_argument('--out', default='/tmp/m226')
ap.add_argument('--here', default=os.path.dirname(os.path.abspath(__file__)))
a = ap.parse_args()
ROOT, OUT = a.root, a.out
os.makedirs(OUT, exist_ok=True)

TYPES = {'PX_STR': 'str', 'PX_LIST': 'list', 'PX_DICT': 'dict', 'PX_CHAN': 'chan',
         'PX_MUTEX': 'mutex', 'PX_RWLOCK': 'rwlock', 'PX_RESULT': 'result',
         'PX_TUPLE': 'tuple', 'PX_STRUCT': 'struct'}
# 解释轨按类型分派的方法族（i_call_method 的 t == "…" 分支）
I_FUNCS = {'str': 'i_str_method', 'list': 'i_list_method', 'dict': 'i_dict_method',
           'result': 'i_result_method'}


def strip_c(src):
    """去注释 + 去 // 行注释；**保留字符串字面量**（方法名就在里面）"""
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
    """PuXian 是缩进语法：`def name(...):` 起，到首个「非空且缩进 <= def 缩进」的行止"""
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


# ── [a] 源码派生 ──
rt = strip_c(open(os.path.join(ROOT, 'runtime/runtime.c'), encoding='utf-8').read())
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

print(f'[派生] 编译轨（runtime.c px_method）：'
      + ' · '.join(f'{t}={len(v)}' for t, v in sorted(C.items()) if v))
print(f'[派生] 解释轨（icall.px）：'
      + ' · '.join(f'{t}={len(v)}' for t, v in sorted(I.items())))

# ── [c] 实测差集 ⇄ UNPAIRED.tsv ──
diff = {}
for t in sorted(set(C) | set(I)):
    if t in ('chan', 'mutex', 'rwlock', 'tuple', 'struct'):     # 解释轨不按类型分派（设计性，见 MINI_SUBSET.md）
        continue
    only_c, only_i = sorted(C.get(t, set()) - I.get(t, set())), sorted(I.get(t, set()) - C.get(t, set()))
    if only_c:
        diff.setdefault(t, {})['c'] = only_c
    if only_i:
        diff.setdefault(t, {})['i'] = only_i

unp = {}
for ln in open(os.path.join(a.here, 'UNPAIRED.tsv'), encoding='utf-8'):
    ln = ln.rstrip('\n')
    if not ln.strip() or ln.lstrip().startswith('#'):
        continue
    f = ln.split('\t')
    if len(f) < 4 or f[0].strip() == '类型':
        continue
    unp.setdefault(f[0], {'c': [], 'i': []})['c' if f[2] == '编译轨' else 'i'].append(f[1])

if unp != diff:
    print('❌ 三轨方法清单差集 ⇄ UNPAIRED.tsv 不相等：')
    print('   实测：', json.dumps(diff, ensure_ascii=False))
    print('   登记：', json.dumps(unp, ensure_ascii=False))
    sys.exit(2)
print(f'[清单] 差集 ⇄ UNPAIRED.tsv 精确相等（差集 {sum(len(v.get("c", [])) + len(v.get("i", [])) for v in diff.values())} 条）')

# ── [b] spec.tsv ⇄ 派生清单 ──
SPEC, spec_err = [], []
for ln in open(os.path.join(a.here, 'spec.tsv'), encoding='utf-8'):
    ln = ln.rstrip('\n')
    if not ln.strip() or ln.lstrip().startswith('#'):
        continue
    f = (ln.split('\t') + ['', '', ''])[:4]
    if f[0].strip() == '类型':
        continue
    t, meth, argstr = f[0].strip(), f[1].strip(), f[2].strip()
    if t not in C and t not in I:
        spec_err.append(f'{t}.{meth}：两侧源码都没有这个类型')
        continue
    valid = [x for x in argstr.split('|') if x != ''] if argstr else []
    SPEC.append((t, meth, valid))
    in_c, in_i = meth in C.get(t, set()), meth in I.get(t, set())
    known = meth in unp.get(t, {}).get('c', []) or meth in unp.get(t, {}).get('i', [])
    if not (in_c or in_i):
        spec_err.append(f'{t}.{meth}：两侧源码都没有（悬空条目）')
    elif (not in_c or not in_i) and not known:
        spec_err.append(f'{t}.{meth}：只在一侧且未登记到 UNPAIRED.tsv')

spec_set = {(t, m) for t, m, _ in SPEC}
src_set = {(t, m) for t in ('str', 'list', 'dict') for m in (C.get(t, set()) | I.get(t, set()))}
for t, m in sorted(src_set - spec_set):
    spec_err.append(f'{t}.{m}：源码有、spec.tsv 未登记（**探针面会静默变窄**）')
if spec_err:
    print('❌ spec.tsv 漂移：')
    for e in spec_err:
        print('   ·', e)
    sys.exit(3)
print(f'[清单] spec.tsv ⇄ 源码派生清单双向一致（{len(SPEC)} 方法）')

# ── [d] 生成探针 ──
POISONS = ['{}', '7', '"s"', '[1]', 'null', 'true']
R = {'str': '"abc"', 'list': '[1, 2]', 'dict': '{"a": 1}'}


def lbl(*ps):
    """标签只允许 [A-Za-z0-9_] —— 毒值里的 `"` / `{}` 直接进标签会截断字符串字面量"""
    return '_'.join(re.sub(r'[^0-9A-Za-z]+', '', str(x)) or 'z' for x in ps)


cases = []
for t, m, valid in SPEC:
    rcv = R[t]
    for p in range(len(valid)):
        for pz in POISONS:
            args = list(valid); args[p] = pz
            cases.append((lbl(t, m, 'p%d' % p, pz), f'print({rcv}.{m}({", ".join(args)}))'))
    cases.append((lbl(t, m, 'ok'), f'print({rcv}.{m}({", ".join(valid)}))'))
    for k in (0, 3):
        if k == len(valid):
            continue
        cases.append((lbl(t, m, 'n%d' % k),
                      f'print({rcv}.{m}({", ".join((list(valid) + ["1", "2", "3"])[:k])}))'))
for t in ('str', 'list', 'dict'):
    cases.append((lbl(t, 'nosuch'), f'print({R[t]}.nosuch())'))

L = ['let a = args()', 'if len(a) < 2:', '    print("NOCASE")', 'else:',
     '    let c = a[1]', '    var hit = false']
for lb, src in cases:
    L += ['    if c == "%s":' % lb, '        hit = true', '        ' + src]
L += ['    if hit == false:', '        print("NOCASE")']
open(os.path.join(OUT, 'drv.px'), 'w').write('\n'.join(L) + '\n')
with open(os.path.join(OUT, 'cases.tsv'), 'w') as f:
    for lb, src in cases:
        f.write(f'{lb}@@{src}\n')
json.dump({'c_runtime': {k: sorted(v) for k, v in C.items()},
           'i_interp': {k: sorted(v) for k, v in I.items()}},
          open(os.path.join(OUT, 'inventory.json'), 'w'), ensure_ascii=False, indent=1)
print(f'[探针] 生成 {len(cases)} 例（方法 {len(SPEC)} 个）→ {OUT}/drv.px（{len(L)} 行）')
