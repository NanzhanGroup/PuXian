#!/usr/bin/env python3
# ============================================================
# M199 静态判据 [S10-a]：native 侧「逐参类型守卫的声明」全量提取
#                         + 「三元兜底」（`args[P].type == T ? A : <默认>`）清单 ⇄ 登记表 对拍
# ------------------------------------------------------------
# 为什么需要（M199 立判据的由来）：
#   M190 量「实参**个数**」，M194/M195 量「**单个**实参的类型守卫」，M197 量「0 参 ⇄ arity 文案」。
#   都没有量 **「同一操作的每一个位置 × 错类型」** 这一面 —— 而这一面最典型的缺口是
#   **三元兜底**：`(nargs >= 2 && args[1].type == PX_STR) ? args[1]... : " "` 把
#   「**不是**字符串」与「**缺省**」混为一谈 ⇒ 编译轨静默（`split("s", {})` → `[s]`）。
#   实测（M199 自动探针）：解释轨 rc=1 R1002 / VM·C 轨 rc=0 ⇒ **三轨分叉**。
#
# 判据：
#   [a] 解析所有**已注册** native 的逐参守卫声明（`args[P].type != T` / `type == T` / helper）
#   [b] 扫描「三元兜底」站点（去注释、去字符串字面量后）—— 每一处必须在 FALLBACK_ALLOW.tsv 登记
#       （判定 guard=后续必有响亮分支 / allow=有意兜底 + 理由），否则判红
#   [c] 表里每条必须在源码中存在（表漂移判红）
# 输出：/tmp/m199sweep/*.json + 结论行（供 verify.sh 判据）
# ============================================================
import json, os, re, sys, glob, argparse

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
ap.add_argument('--out', default='/tmp/m199sweep')
a = ap.parse_args()
os.chdir(a.root)
os.makedirs(a.out, exist_ok=True)

GUARD_HELPERS = ['px_arg_int', 'px_arg_str', 'px_arg_strbytes', 'px_arg_dur_ms', 'px_arg_dur_ns',
                 'px_arg_bytes', 'px_arg_list', 'px_arg_dict',
                 'px_req_int_idx', 'px_req_str', 'px_req_int', 'px_req_bytes']

def strip_lit(src):
    """去注释 + 去字符串字面量（保留换行以便行号可对）"""
    out = []
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        if c == '/' and i + 1 < n and src[i+1] == '*':
            j = src.find('*/', i + 2)
            seg = src[i:(n if j < 0 else j + 2)]
            out.append('\n' * seg.count('\n')); i = n if j < 0 else j + 2
        elif c == '/' and i + 1 < n and src[i+1] == '/':
            j = src.find('\n', i); out.append(' '); i = n if j < 0 else j
        elif c == '"':
            j = i + 1
            while j < n:
                if src[j] == '\\': j += 2; continue
                if src[j] == '"': break
                j += 1
            out.append('""'); i = j + 1
        else:
            out.append(c); i += 1
    return ''.join(out)

def brace_body(src, start):
    d, i = 0, start
    while i < len(src):
        if src[i] == '{': d += 1
        elif src[i] == '}':
            d -= 1
            if d == 0: return src[start:i+1]
        i += 1
    return src[start:]

# ⚠️ 定义可能是 `static LXValue bi_x(...)` 也 可能是 `LXValue bi_x(...)`（runtime_ws.c 等 9 处）——
#    首版只收 static ⇒ `ws_heartbeat` 整个函数**没进**声明表（本轮实测踩到：静默跳过 = 假绿）
FUNC_RE = re.compile(r'^(?:static\s+)?LXValue (bi_\w+)\s*\(\s*LXValue\s*\*\s*args,\s*int\s*nargs,\s*void\s*\*\s*ctx\s*\)\s*\{', re.M)
TRI_RE = re.compile(r'args\[(\d+)\]\.type\s*==\s*(PX_\w+)')

guard_decl, fallbacks, nfn = {}, {}, 0
for path in sorted(glob.glob('runtime/*.c')):
    raw = open(path, encoding='utf-8', errors='replace').read()
    clean = strip_lit(raw)
    for m in FUNC_RE.finditer(clean):
        nfn += 1
        name = m.group(1)[3:]
        body = brace_body(clean, m.end() - 1)
        g = guard_decl.setdefault(name, {})
        for gm in re.finditer(r'args\[(\d+)\]\.type\s*!=\s*(PX_\w+)', body):
            g.setdefault(int(gm.group(1)), set()).add(gm.group(2)[3:])
        for gm in re.finditer(r'args\[(\d+)\]\.type\s*==\s*(PX_\w+)', body):
            g.setdefault(int(gm.group(1)), set()).add(gm.group(2)[3:])
        for h in GUARD_HELPERS:
            for gm in re.finditer(re.escape(h) + r'\(\s*args\[(\d+)\]', body):
                g.setdefault(int(gm.group(1)), set()).add(h)
    # ── 三元兜底：`args[P].type == T` 之后（同语句内）先见 `?` ──
    lines = clean.split('\n')
    for lm, lm_m in enumerate(re.finditer(r'args\[(\d+)\]\.type\s*==\s*(PX_\w+)', clean)):
        seg = clean[lm_m.end(): lm_m.end() + 220]
        seg = seg.split(';')[0]
        if '?' not in seg:
            continue
        line_no = clean[:lm_m.start()].count('\n') + 1
        expr = re.sub(r'\s+', ' ', clean[lm_m.start(): lm_m.start() + 90]).strip()
        expr = expr.split(';')[0][:80]
        fallbacks[f'{(path.split("/")[-1])}:{line_no}|{expr}'] = {
            'file': path.split('/')[-1], 'line': line_no, 'expr': expr,
            'pos': int(lm_m.group(1)), 'type': lm_m.group(2)[3:]}

reg = set(json.load(open('docs/native_index.json'))['names']) if os.path.exists('docs/native_index.json') else set()
decl = {k: {str(p): sorted(v) for p, v in sorted(g.items())} for k, g in sorted(guard_decl.items()) if g}
json.dump({'functions': nfn, 'registered': sorted(reg & set(guard_decl)),
           'guards': decl, 'fallbacks': fallbacks},
          open(f'{a.out}/sweep.json', 'w'), ensure_ascii=False, indent=1)

# ── 登记表对拍 ──
tbl_path = 'examples/m199_argtype/FALLBACK_ALLOW.tsv'
rows = []
if os.path.exists(tbl_path):
    for ln in open(tbl_path, encoding='utf-8'):
        ln = ln.rstrip('\n')
        if not ln or ln.startswith('#'): continue
        f = ln.split('\t')
        if len(f) < 4: continue
        rows.append({'file': f[0], 'expr': f[1], 'verdict': f[2], 'reason': f[3]})

def key_of(fb):
    return '%s|%s' % (fb['file'], fb['expr'])

src_keys = {key_of(v) for v in fallbacks.values()}
tbl_keys = {'%s|%s' % (r['file'], r['expr']) for r in rows}
undeclared = sorted(src_keys - tbl_keys)
stale = sorted(tbl_keys - src_keys)

print('=== [S10-a] native 逐参类型守卫的「声明」全量 ===')
print(f'解析 native 实现（bi_*）        : {nfn}')
print(f'其中在 native_index 注册        : {len(reg & set(guard_decl))}')
print(f'有逐参守卫声明的函数            : {len(decl)}')
print(f'守卫声明位置总数                : {sum(len(v) for v in decl.values())}')
print()
print('=== [S10-a] 三元兜底清单 ⇄ 登记表 ===')
print(f'源码中的三元兜底站点            : {len(fallbacks)}')
print(f'登记表条目                      : {len(rows)}')
print(f'未登记（必须为 0）              : {len(undeclared)}')
for k in undeclared: print('    ✗ 未登记:', k)
print(f'表漂移（必须为 0）              : {len(stale)}')
for k in stale: print('    ✗ 表内条目在源码中找不到:', k)
nbad = len(undeclared) + len(stale)
print(f'结论：{"不一致 0" if nbad == 0 else "不一致 %d" % nbad}')
sys.exit(1 if nbad else 0)
