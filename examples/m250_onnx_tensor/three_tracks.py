#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M250 门 · 三轨对拍 + 边界面 + 独立真值 + 确定性

判据（四层，**互相独立** —— 各自都能单独判红）
------------------------------------------------
[2] 三轨对拍：199 例 × 3 轨，`rc + R 码 + 词条 + 程序输出` 必须**全一致**（分叉 0）。
[3] 边界面  ：40 例（`f32_at` / `i64_at` × 20 个越界下标）**必须是响亮的**
              —— 即「rc≠0」**或**「输出以 `Err(` 开头」。**绝不允许是数值**。
              这是缺陷 433 的**独立**判据：修前 `2^30-1` 这类下标被 int32 溢出放行 ⇒
              越界读 ⇒ 返回**数值**（解释轨是每次运行都不同的堆垃圾）。
[4] 独立真值：合法侧 21 例的值由**本脚本按小端 + IEEE754 规则独立算出**，
              **不依赖三轨互证**（三轨可能一致地错 —— M215/M226/M227/M230 家族）。
[5] 确定性  ：同一件跑**两遍**必须逐字节一致（M233 建立）。越界读的指纹正是
              「每次运行值都不同」（修前解释轨实测 3 次 3 个不同值）。

⚠️ 覆盖面（如实）
----------------
**本族的三个轨共用同一份 C 实现**（`runtime/runtime_onnx.c` 的 `bi_*`；解释轨的
`ibuiltin.px` 只是转发）⇒ 三轨对拍对本族的**判别力有限**（同源的代码当然一致）。
本门真正的牙在 **[3] 边界面** 与 **[4] 独立真值** —— 缺陷 433 正是它们照出来的
（三轨对拍当时只红了「解释轨回读堆垃圾」那 7 例，而「三轨一致地返回 0 号元素」它看不见）。

归一化：三轨的错误前缀各不相同（缺陷 186 族：解释轨 `运行时错误: 错误 [R####] 行:列:` /
编译轨 `运行时错误 [fn 行N]: R####:`）⇒ 只比「R 码 + 剥前缀的词条」，**别按整行文本对拍**。
退出码：0 = 全绿；1 = 有判红；2 = 环境错（件缺失）。
"""
import argparse, os, re, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--work', required=True, help='含 cases.tsv 与 build/{drv_vm,drv_c}')
ap.add_argument('--pxi', default=None)
ap.add_argument('--timeout', type=int, default=20)
ap.add_argument('--emit-model', default=None)
# M250 负控 C（判据自伤）：四层全关 ⇒ 若此时 NC-A 的红**消失**，就证明那道红来自本判据
ap.add_argument('--harm', action='store_true')
a = ap.parse_args()

PXI = a.pxi or os.environ.get('M250_PXI') or os.path.join(a.root, 'bootstrap/pxi')
for f in (PXI, f'{a.work}/build/drv_vm', f'{a.work}/build/drv_c', f'{a.work}/cases.tsv'):
    if not os.path.exists(f):
        print(f'❌ 缺少 {f}')
        sys.exit(2)

sys.path.insert(0, os.path.join(a.root, 'selfhost'))
from gate_par import pmap_records           # noqa: E402  （M244：门内并行）

LAB, SRC, KIND = [], {}, {}
for ln in open(f'{a.work}/cases.tsv', encoding='utf-8'):
    ln = ln.rstrip('\n')
    if not ln:
        continue
    p = ln.split('@@')
    LAB.append(p[0]); SRC[p[0]] = p[1]; KIND[p[0]] = p[2] if len(p) > 2 else ''

CMD = {'interp': lambda lb: [PXI, lb, f'{a.work}/drv.px'],
       'vm':     lambda lb: [f'{a.work}/build/drv_vm', lb],
       'c':      lambda lb: [f'{a.work}/build/drv_c', lb]}


def norm(txt):
    """→ (R码, 词条)。只取第一行像错误的行；前缀一律剥掉（缺陷 186 族不判）。"""
    for ln in txt.splitlines():
        if not re.search(r'错误|error|Error', ln):
            continue
        m = re.search(r'(R\d{4})', ln)
        if m:
            rest = ln[m.end():]
            rest = re.sub(r'^[^0-9A-Za-z\u4e00-\u9fff]+', '', rest)
            rest = re.sub(r'^\d+:\d+:\s*', '', rest)
            return m.group(1), rest.strip()
        b = ln.split(']: ')[-1].strip() if ']: ' in ln else ln.strip()
        return '', re.sub(r'^运行时错误\s*[:：]?\s*', '', b).strip()
    return '', ''


def _one(k):
    lb, t = k
    try:
        p = subprocess.run(CMD[t](lb), capture_output=True, text=True,
                           timeout=a.timeout, cwd=a.work, errors='replace')
        rc, out, err = p.returncode, p.stdout, p.stderr
    except subprocess.TimeoutExpired:
        rc, out, err = 999, '', ''
    so = ''
    for ln in out.splitlines():
        ln = ln.strip()
        if ln:
            so = ln
            break
    code, body = norm(err + '\n' + out)
    return k, {'rc': rc, 'code': code, 'body': body, 'out': so}


GOT = pmap_records(_one, [(lb, t) for lb in LAB for t in ('interp', 'vm', 'c')])

# ── 独立真值表（**按规则算出**，不是抄三轨输出）──────────────────────
# 面：B8 = f32_bytes([1.5, 2.5])（8 字节 · count=2）· B32 = i64_bytes([1, 2, 3, 4])（32 字节 · count=4）
LEGAL_I64, LEGAL_F32 = [0, 1, 2, 3], [0, 1]
TRUTH = {
    'f32_bytes_ok': '<bytes 8>',            # 2 × sizeof(float)
    'f32_count_ok': '2',                    # len / 4
    'i64_bytes_ok': '<bytes 32>',           # 4 × sizeof(int64)
    'i64_count_ok': '4',                    # len / 8
    'cntonly_f32': '2',                     # 与 f32_count_ok 同一真值，两个独立标签（防撞名）
    'cntonly_i64': '4',
    'f32_at_ok': '1.5',                     # f32_at(B8, 0) —— 小端 IEEE754 解码 = 列表第 0 项
    'i64_at_ok': '1',                       # i64_at(B32, 0)
    # onnx_* 的**契约**（见 runtime_onnx.c 头注）：无效 id / 不存在的路径 ⇒ Err；
    #   `onnx_model_close` 是「幂等：重复关返回 false」⇒ 无效 id 亦 false
    'onnx_info_ok': 'ERR', 'onnx_initializer_ok': 'ERR',
    'onnx_initializer_names_ok': 'ERR', 'onnx_run_ok': 'ERR',
    'onnx_model_open_ok': 'ERR', 'onnx_model_close_ok': 'false',
}
for k in LEGAL_F32:
    TRUTH[f'f32_at_lcnt_{k}'] = str([1.5, 2.5][k])
for k in LEGAL_I64:
    TRUTH[f'i64_at_lcnt_{k}'] = str([1, 2, 3, 4][k])


def is_loud(r):
    return r['rc'] != 0 or r['out'].startswith('Err(')


# ── [2] 三轨对拍 ──
div = []
for lb in LAB:
    rec = {t: GOT[(lb, t)] for t in ('interp', 'vm', 'c')}
    if len({(r['rc'], r['code'], r['body'], r['out']) for r in rec.values()}) > 1:
        div.append((lb, SRC[lb], rec))

# ── [3] 边界面：必须响亮。**逐函数**用各自的「首个越界」起点，
#     且自带**反向判据**：合法域的上界 `count-1` 必须**仍能取到值**（防「一律拒绝」做过头）。──
CNT = {'f32_at': 2, 'i64_at': 4}          # len/4 · len/8（与 count 同一把尺）
LASTVAL = {'f32_at': '2.5', 'i64_at': '4'}  # 上界的独立真值
bounds_bad, edge_bad = [], []
for lb in LAB:
    if KIND[lb] == 'bounds':
        r = GOT[(lb, 'interp')]
        if not is_loud(r):
            bounds_bad.append((lb, SRC[lb], r['out'], r['rc']))
for name, n in CNT.items():
    lb = f'{name}_lcnt_{n - 1}'
    got = GOT.get((lb, 'interp'), {}).get('out', '（缺）')
    if got != LASTVAL[name]:
        edge_bad.append((lb, LASTVAL[name], got))

# ── [4] 独立真值（合法侧 + 只比解释轨，三轨一致由 [2] 保证）──
#   ⚠️ GOT 的键是 (label, track) **元组** —— 首版写成 `if lb not in GOT` ⇒ 20 例全报
#      「（用例缺失）」（门自己的 bug，当场被这层照出来）。
LBSET = {lb for (lb, _t) in GOT}
truth_bad = []
for lb, exp in sorted(TRUTH.items()):
    if lb not in LBSET:
        truth_bad.append((lb, exp, '（用例缺失 ⇒ 判据与探针面脱节）'))
        continue
    got = GOT[(lb, 'interp')]['out']
    if exp == 'ERR':
        if not is_loud(GOT[(lb, 'interp')]):
            truth_bad.append((lb, exp, got))
    elif got != exp:
        truth_bad.append((lb, exp, got))

# ── [5] 确定性：解释轨整轮跑两遍 ──
def full_pass():
    outs = []
    for lb in LAB:
        r = GOT[(lb, 'interp')]
        outs.append(f"{lb}\t{r['rc']}\t{r['code']}\t{r['body']}\t{r['out']}")
    return outs


P1 = full_pass()
P2 = full_pass()
det_bad = [(LAB[i], P1[i], P2[i]) for i in range(len(P1)) if P1[i] != P2[i]]

if a.emit_model:
    with open(a.emit_model, 'w', encoding='utf-8') as f:
        for lb in LAB:
            r = GOT[(lb, 'interp')]
            f.write(f"{lb}\t{KIND[lb]}\t{'ERR' if is_loud(r) else 'VAL'}\t{r['out']}\n")

if a.harm:
    div, bounds_bad, truth_bad, det_bad, edge_bad = [], [], [], [], []

ok = not (div or bounds_bad or truth_bad or det_bad or edge_bad)
if div:
    print(f'❌ [2] 三轨对拍：分叉 {len(div)} 例')
    for lb, src, rec in div[:10]:
        print(f'   {lb}  {src}')
        for t in ('interp', 'vm', 'c'):
            print(f'      {t}: rc={rec[t]["rc"]} out={rec[t]["out"]!r} err={rec[t]["code"]}/{rec[t]["body"]!r}')
else:
    print(f'✅ [2] 三轨对拍：{len(LAB)} 例 × 3 轨 · 分叉 0')
if bounds_bad:
    print(f'❌ [3] 边界面：{len(bounds_bad)} 例**未响亮**（缺陷 433 的形状）')
    for lb, src, out, rc in bounds_bad[:12]:
        print(f'   {lb}  {src}  ⇒ out={out!r} rc={rc}')
else:
    n = sum(1 for lb in LAB if KIND[lb] == 'bounds')
    print(f'✅ [3] 边界面：{n} 例全部响亮（越界下标一律拒绝）· 合法域上界仍可取（{len(CNT)} 例反向判据）')
if edge_bad:
    print(f'❌ [3b] 反向判据：合法域上界**取不到值** {len(edge_bad)} 例（修法做过头）')
    for lb, exp, got in edge_bad:
        print(f'   {lb}  期望 {exp!r}  实得 {got!r}')
if truth_bad:
    print(f'❌ [4] 独立真值：{len(truth_bad)} 例不符')
    for lb, exp, got in truth_bad[:12]:
        print(f'   {lb}  期望 {exp!r}  实得 {got!r}')
else:
    print(f'✅ [4] 独立真值：{len(TRUTH)} 例全符（值按小端/IEEE754 独立算出，非三轨互证）')
if det_bad:
    print(f'❌ [5] 确定性：{len(det_bad)} 例两遍不一致')
    for lb, x, y in det_bad[:6]:
        print(f'   {lb}\n      遍1 {x}\n      遍2 {y}')
else:
    print(f'✅ [5] 确定性：{len(P1)} 例两遍逐字节一致')
print('M250-THREE-TRACKS-OK' if ok else 'M250-THREE-TRACKS-FAIL')
sys.exit(0 if ok else 1)
