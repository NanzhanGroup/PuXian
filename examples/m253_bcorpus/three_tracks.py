#!/usr/bin/env python3
# ============================================================
# M253 三层判据（对 10 个「从未被任何门语料触碰过」的 [B 类] native）
# ------------------------------------------------------------
#   [1] 三轨一致 —— 同一例 × 解释轨/VM 轨/C 轨 ⇒ (rc, R 码, 消息体, stdout) 必须四元组相等
#   [2] 期望值   —— mode=v：stdout 必须**逐字节**等于真值（真值由 Go 独立算出，见 truth/gen_truth.go）
#                   mode=e：必须**响亮**（rc≠0 且含 R1002）
#   [3] 覆盖面   —— 10 个 API 必须真的出现在语料里（防「门在跑、面在缩」）
#
# ⚠️ 为什么 [2] 不可省：这 10 个 API 的实现**只有一份**（runtime 的 C 函数）⇒ 三轨跑的是同一段代码
#    ⇒ 任何「三轨对拍」按定义看不见「三轨一致地错」（M215 缺陷 305 / M226 缺陷 329 /
#    M230 缺陷 345 / M250 缺陷 433 同族）。本轮两个缺陷（GCM 空明文、errno int 截断）**正是这一类**。
#
# 退出码：0 = 全绿；1 = 有分叉 / 期望不符 / 覆盖面缺口；2 = 环境错误；3 = 越界
# --harm：判据自伤（[1][2] 全部恒真）—— 负控 C 用，证明红来自比对本身
# ============================================================
import argparse, json, os, re, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--work', required=True, help='含 cases.tsv / drv.px 与 build/{drv_vm,drv_c}')
ap.add_argument('--pxi', default=None)
ap.add_argument('--timeout', type=int, default=20)
ap.add_argument('--json', default=None)
ap.add_argument('--harm', action='store_true')
a = ap.parse_args()

PXI = a.pxi or os.path.join(a.root, 'bootstrap/pxi')
ART = ['drv.px', 'cases.tsv', 'build/drv_vm', 'build/drv_c']
miss = [f for f in ART if not os.path.exists(os.path.join(a.work, f))]
if not os.path.exists(PXI):
    miss.append(PXI)
if miss:
    print('❌ 缺少 ' + ' '.join(miss))
    sys.exit(2)

sys.path.insert(0, os.path.join(a.root, 'selfhost'))
from gate_par import pmap_records   # noqa: E402

APIS = ['aes_encrypt_bytes', 'aes_decrypt_bytes', 'aes_gcm_encrypt_bytes', 'aes_gcm_decrypt_bytes',
        'aes_decrypt_ecb', 'go_errno_string', 'print_err', 'session_id', 'session_destroy', 'tz_local']


def norm(txt):
    """→ (R码, 消息体)。只取第一行像错误的行；三轨前缀各不相同（缺陷 186 族）⇒ 一律剥掉。"""
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
        return '', re.sub(r'^运行时错误\s*', '', b)
    return '', ''


def run(cmd):
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=a.timeout,
                           cwd=a.work, errors='replace')
    except subprocess.TimeoutExpired:
        return 999, '', ''
    return p.returncode, p.stdout, p.stderr


CASES = []
for ln in open(os.path.join(a.work, 'cases.tsv'), encoding='utf-8'):
    ln = ln.rstrip('\n')
    if not ln:
        continue
    lb, mode, expect = (ln.split('\t') + ['', ''])[:3]
    CASES.append((lb, mode, expect.replace('\\n', '\n').replace('\\t', '\t')))


def cmd_for(lb, t):
    return {'interp': [PXI, lb, os.path.join(a.work, 'drv.px')],
            'vm': [os.path.join(a.work, 'build/drv_vm'), lb],
            'c': [os.path.join(a.work, 'build/drv_c'), lb]}[t]


def one(k):
    lb, t = k
    rc, out, err = run(cmd_for(lb, t))
    code, body = norm(err + '\n' + out)
    return k, {'rc': rc, 'code': code, 'body': body, 'out': out.rstrip('\n')}


GOT = pmap_records(one, [(lb, t) for lb, _m, _e in CASES for t in ('interp', 'vm', 'c')])

div, badv, bade = [], [], []
rows = []
for lb, mode, expect in CASES:
    rec = {t: GOT[(lb, t)] for t in ('interp', 'vm', 'c')}
    rows.append({'label': lb, 'mode': mode, 'expect': expect, **rec})
    sig = {(r['rc'], r['code'], r['body'], r['out']) for r in rec.values()}
    if len(sig) > 1:
        div.append(rows[-1])
    ref = rec['c']
    # 驱动器约定：值例的 stdout 以 'V=' 打头（多条值例则逐行 `N=v`，无前缀）
    out_chk = ref['out'][2:] if ref['out'].startswith('V=') else ref['out']
    if mode == 'v' and out_chk != expect:
        badv.append((lb, expect, ref['out'], ref['rc']))
    if mode == 'e' and not (ref['rc'] != 0 and ref['code'] == 'R1002'):
        bade.append((lb, ref['rc'], ref['code'], ref['body']))

drv = open(os.path.join(a.work, 'drv.px'), encoding='utf-8').read()
covmiss = [n for n in APIS if n not in drv]

if a.harm:
    div, badv, bade, covmiss = [], [], [], []

print(f'M253 [B 类] native 三轨+期望：{len(CASES)} 例 × 3 轨 = {len(CASES) * 3} 次执行')
print(f'  三轨分叉 {len(div)} · 期望不符 {len(badv)} · 边界面未响亮 {len(bade)} · 覆盖面缺口 {len(covmiss)}')
for r in div[:8]:
    print(f'  ▲ {r["label"]}')
    for t in ('interp', 'vm', 'c'):
        d = r[t]
        print(f'      {t:6s} rc={d["rc"]:<4d} {d["code"]:6s} out=[{d["out"][:28]}] {d["body"][:52]}')
for lb, ex, got, rc in badv[:8]:
    print(f'  ✗ {lb} 期望[{ex[:44]}] 实得[{got[:44]}] rc={rc}')
for lb, rc, code, body in bade[:8]:
    print(f'  ✗ {lb} 未响亮 rc={rc} code={code} {body[:44]}')
for n in covmiss:
    print(f'  ✗ 覆盖面：{n} 未出现在语料里')

if a.json:
    json.dump(rows, open(a.json, 'w'), ensure_ascii=False, indent=1)
print('分叉/不符标签：' + (','.join([r['label'] for r in div] + [x[0] for x in badv] +
                                   [x[0] for x in bade] + covmiss) or '(无)'))
sys.exit(1 if (div or badv or bade or covmiss) else 0)
