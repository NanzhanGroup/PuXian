#!/usr/bin/env python3
# ============================================================
# M227 「同名两门」对拍：同一操作 · 两个入口 · 三轨各自
# ------------------------------------------------------------
# 面 = 函数面（`len(x)`）/ 方法面（`x.len()`），**逻辑参数一一对应**（见 gen_probes.py PAIR_TPL）。
#
# 判据（修后必须全绿）：
#   [Ⅰ 跨轨] 同一面下三轨一致（rc 是否 0 / R 码 / 消息体 / 程序输出）—— 沿用 M226 口径。
#   [Ⅱ 跨面]
#     H1 **响亮性一致**  rc==0 ⇄ rc!=0   ← 最硬：静默错值 vs 响亮报错
#     H2 **合法调用输出一致**（`ok` 例两面输出逐字节相同）
#     H3 **词条归属**   函数面调用不得拿到**方法面词条**（`方法 X …` / `类型 X 没有方法 'Y'`），
#                       反之亦然 —— 这正是 **M199 缺陷 238** 的精确形状
#                        （`replace(d,…)` 从函数面进却报 `R1007 类型 dict 没有方法 'replace'`）。
#   [Ⅲ 登记] **R 码分歧集合 ⇄ `RCODE.tsv` 精确相等**（双向）——
#     两面各自的「参数错」(R1002) / 「个数错」(R1005) / 「方法不存在」(R1007) 是**不同检查层**的
#     既定语义，**不硬判**；但要**逐一登记**，且**表里有的必须实测还在**（否则该表退化成
#     「把红当绿记下来」—— M226 UNPAIRED.tsv 同款纪律）。
# 退出码：0 = 全绿；1 = 有分叉；2 = 环境错误
# ============================================================
import argparse, json, os, re, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--work', required=True)
ap.add_argument('--pxi', default=None)
ap.add_argument('--rcode', default=None, help='R 码分歧登记表（默认 <here>/RCODE.tsv）')
ap.add_argument('--xallow', default=None, help='硬判据豁免表（默认 <here>/XALLOW.tsv）')
ap.add_argument('--timeout', type=int, default=20)
ap.add_argument('--dump', default=None)
ap.add_argument('--json', default=None)
ap.add_argument('--harm', action='store_true', help='判据自伤：恒报 0 分叉（负控 C 用）')
ap.add_argument('--here', default=os.path.dirname(os.path.abspath(__file__)))
a = ap.parse_args()

PXI = a.pxi or os.path.join(a.root, 'bootstrap/pxi')
RCODE = a.rcode or os.path.join(a.here, 'RCODE.tsv')
XALLOW = a.xallow or os.path.join(a.here, 'XALLOW.tsv')


def load_tsv(path, ncol, nkey):
    """读登记表 → {前 nkey 列 join('\t'): 最后一列(理由)}；跳过注释/表头/空行。
    ⚠️ 理由取**最后一列**而不是第 nkey 列 —— XALLOW.tsv 是 4 列而键只用前 2 列
      （首版取 `f[nkey]` ⇒ 拿到「种类」列，打印出来是「H1…」这种假理由）。"""
    out = {}
    if not os.path.exists(path):
        return out
    for ln in open(path, encoding='utf-8'):
        ln = ln.rstrip('\n')
        if not ln.strip() or ln.lstrip().startswith('#'):
            continue
        f = (ln.split('\t') + [''] * ncol)[:ncol]
        if f[0].strip() in ('变体', '名字'):
            continue
        out['\t'.join(x.strip() for x in f[:nkey])] = f[-1].strip()
    return out


def nv(lb):
    """标签 → `名字\\t变体`（与 XALLOW.tsv 的键同形；名字可含 `_`，故用类型列作锚定）"""
    m = re.match(r'^m227_(.+)_(str|list|dict)_(.+)$', lb)
    return f'{m.group(1)}\t{m.group(3)}' if m else lb


for f in (PXI, f'{a.work}/build/drv_vm', f'{a.work}/build/drv_c', f'{a.work}/cases.tsv'):
    if not os.path.exists(f):
        print(f'❌ 缺少 {f}')
        sys.exit(2)

# ── M244（门内并行）：逐例 spawn 彼此**无依赖**（同一驱动器、不同 case 选择，
#    独立进程、不写共享文件）⇒ 用 gate_par.pmap 吃满整机核数。
#    ⚠️ 门**之间**仍然串行（run_gates.sh 的 PID 锁）—— 53 个门的负控会改源码，
#       门间并行必然互相踩；门内并行与那条护栏正交。
sys.path.insert(0, os.path.join(a.root, 'selfhost'))
from gate_par import pmap, pmap_records   # noqa: E402
METHOD_ENTRY = re.compile(r'^(方法\s|类型\s\S+\s没有方法\s)')


def norm(txt):
    for ln in txt.splitlines():
        if not re.search(r'错误|error|Error', ln):
            continue
        m = re.search(r'(R\d{4})', ln)
        if m:
            rest = re.sub(r'^[^0-9A-Za-z\u4e00-\u9fff]+', '', ln[m.end():])
            return m.group(1), re.sub(r'^\d+:\d+:\s*', '', rest).strip()
        b = ln.split(']: ')[-1].strip() if ']: ' in ln else ln.strip()
        return '', re.sub(r'^运行时错误\s*[:：]?\s*', '', b)
    return '', ''


def run(cmd):
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=a.timeout,
                           cwd=a.work, errors='replace')
    except subprocess.TimeoutExpired:
        return 999, '', ''
    return p.returncode, p.stdout, p.stderr


cases = [ln.rstrip('\n').split('\t') for ln in open(f'{a.work}/cases.tsv', encoding='utf-8')]
cases = [c for c in cases if len(c) >= 3]

def _cmd227(lb, face, t):
    return {'interp': [PXI, lb, face, f'{a.work}/drv.px'],
            'vm': [f'{a.work}/build/drv_vm', lb, face],
            'c': [f'{a.work}/build/drv_c', lb, face]}[t]


def _one227(k):
    """单例执行体（面 × 轨）—— 与串行版逐字节同逻辑。

    ⚠️ `dump` 的行序**必须与串行版相同**（`--dump` 的产物是可 `cmp` 的判据载体）
    ⇒ 这里把 dump 行随结果一起带回，调用方按**输入序** append。
    """
    lb, face, t = k
    rc, out, err = run(_cmd227(lb, face, t))
    code, body = norm(err + '\n' + out)
    so = next((ln.strip() for ln in out.splitlines()
               if not ln.startswith(('/*', ' *', '*/'))), '')
    return k, {'rc': rc, 'code': code, 'body': body, 'out': so,
               '_dump': '\t'.join([lb, face, t, str(rc), code, body, so])}


_GOT227 = pmap_records(_one227, [(lb, face, t) for lb, _f, _m in cases
                                 for face in ('f', 'm') for t in ('interp', 'vm', 'c')])
rows, h1, h2, h3, tri, rset = [], [], [], [], [], {}
dump = []
for lb, fsrc, msrc in cases:
    variant = lb.split('_')[-1] if not re.match(r'^p\d+_', lb.split('_')[-1]) else lb.split('_')[-2] + '_x'
    variant = re.sub(r'^m227_\w+?_(str|list|dict)_', '', lb)
    variant = re.sub(r'^p\d+_.*$', 'poison', variant)
    rec = {}
    for face in ('f', 'm'):
        for t in ('interp', 'vm', 'c'):
            d = _GOT227[(lb, face, t)]
            rec[(face, t)] = {k: v for k, v in d.items() if not k.startswith('_')}
            if a.dump:
                dump.append(d['_dump'])
    rows.append({'label': lb, 'func': fsrc, 'meth': msrc,
                 **{'%s|%s' % k: v for k, v in rec.items()}})

    for face in ('f', 'm'):
        s = {t: (rec[(face, t)]['rc'], rec[(face, t)]['code'], rec[(face, t)]['body'],
                 rec[(face, t)]['out']) for t in ('interp', 'vm', 'c')}
        if len(set(s.values())) > 1:
            tri.append({'label': lb, 'face': face,
                        'src': fsrc if face == 'f' else msrc,
                        **{t: rec[(face, t)] for t in ('interp', 'vm', 'c')}})

    for t in ('interp', 'vm', 'c'):
        F, M = rec[('f', t)], rec[('m', t)]
        lf, lm = F['rc'] == 0, M['rc'] == 0
        if lf != lm:
            h1.append({'label': lb, 'track': t, 'src_f': fsrc, 'src_m': msrc, 'F': F, 'M': M})
            continue
        if lf and lm:
            if F['out'] != M['out']:
                h2.append({'label': lb, 'track': t, 'src_f': fsrc, 'src_m': msrc, 'F': F, 'M': M})
            continue
        # 两面都响亮
        sf = not bool(METHOD_ENTRY.match(F['body']))
        sm = bool(METHOD_ENTRY.match(M['body']))
        if not (sf and sm):
            h3.append({'label': lb, 'track': t, 'src_f': fsrc, 'src_m': msrc,
                       'why': '函数面拿到方法面词条' if not sf else '方法面拿到函数面词条',
                       'F': F, 'M': M})
        if F['code'] != M['code']:
            k = f'{variant}\t{F["code"]}\t{M["code"]}'
            rset.setdefault(k, []).append(f'{lb}/{t}')

if a.harm:
    # 判据自伤（负控 C）：**把全部判据（含跨轨与两张登记表）都设成恒绿**。
    #   若此时「负控 A/B 的红」消失，就证明那道红确实来自**本脚本的比对**，
    #   而不是来自别的偶然因素（M213/M214/M226 同款手法）。
    h1, h2, h3, tri, rset = [], [], [], [], {}
if a.dump:
    open(a.dump, 'w').write('\n'.join(dump) + '\n')
if a.json:
    json.dump({'rows': rows, 'h1': h1, 'h2': h2, 'h3': h3, 'tri': tri, 'rcode': rset},
              open(a.json, 'w'), ensure_ascii=False, indent=1)

# ── 登记表 / 豁免表比对 ──
reg = load_tsv(RCODE, 4, 3)      # (变体, F码, M码) → 理由
allow = load_tsv(XALLOW, 4, 2)   # (名字|变体)      → 理由

hard = {'H1': [], 'H2': [], 'H3': []}
info = []
for nm, L in (('H1', h1), ('H2', h2), ('H3', h3)):
    for x in L:
        k = nv(x['label'])
        if k in allow:
            info.append((nm, x, allow[k]))
        else:
            hard[nm].append(x)
seen_allow = {nv(x['label']) for L in (h1, h2, h3) for x in L}
allow_stale = sorted(set(allow) - seen_allow)

extra = sorted(set(rset) - set(reg))     # 实测有、表里没有 ⇒ 红（漏登记）
stale = sorted(set(reg) - set(rset))     # 表里有、实测没有 ⇒ 红（登记过期）
if a.harm:
    info, extra, stale, allow_stale = [], [], [], []

print(f'M227 同名两门对拍：{len(cases)} 例 × 两面 × 三轨 = {len(cases)*6} 次执行')
print(f'  [Ⅰ 跨轨] 同一面三轨分叉：{len(tri)}')
print(f'  [Ⅱ 跨面] H1 响亮性 {len(hard["H1"])} · H2 合法输出 {len(hard["H2"])}'
      f' · H3 词条归属 {len(hard["H3"])}　（已豁免 {len(info)}）')
print(f'  [Ⅲ 登记] R 码分歧形状：实测 {len(rset)} ⇄ 登记 {len(reg)}'
      f'（漏登记 {len(extra)} · 登记过期 {len(stale)}）')
for x in info:
    nm, d, why = x
    print(f'  ℹ {nm} [豁免] {d["label"]} — {why[:60]}…')
for nm in ('H1', 'H2', 'H3'):
    for x in hard[nm]:
        print(f'  ✖ {nm} [{x["track"]}] {x["label"]}')
        print(f'       函数面 {x["src_f"]:<32} rc={x["F"]["rc"]:<4} {x["F"]["code"]} {x["F"]["body"][:56]} out=[{x["F"]["out"][:14]}]')
        print(f'       方法面 {x["src_m"]:<32} rc={x["M"]["rc"]:<4} {x["M"]["code"]} {x["M"]["body"][:56]} out=[{x["M"]["out"][:14]}]')
for x in tri:
    print(f'  ✖ [跨轨·{x["face"]}面] {x["label"]}  {x["src"]}')
    for t in ('interp', 'vm', 'c'):
        d = x[t]
        print(f'       {t:6s} rc={d["rc"]:<4} {d["code"]:6s} out=[{d["out"][:20]}] {d["body"][:56]}')
for k in extra:
    print(f'  ✖ [登记] 实测有、RCODE.tsv 未登记：{k.replace(chr(9), " / ")}   例：{rset[k][0]}')
for k in stale:
    print(f'  ✖ [登记] RCODE.tsv 有、实测已消失（登记过期）：{k.replace(chr(9), " / ")}')
for k in allow_stale:
    print(f'  ✖ [豁免] XALLOW.tsv 有、实测已无此分叉（登记过期）：{k}')

nh = sum(len(hard[n]) for n in hard)
bad = nh + len(tri) + len(extra) + len(stale) + len(allow_stale)
for nm in ('H1', 'H2', 'H3'):
    print(f'{nm}标签：' + (','.join(sorted({x["label"] for x in hard[nm]})) if hard[nm] else '(无)'))
print(f'跨面硬分叉合计 {nh} · 跨轨 {len(tri)} · 登记异常 {len(extra)+len(stale)+len(allow_stale)}')
sys.exit(1 if bad else 0)
