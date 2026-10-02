#!/usr/bin/env python3
# ============================================================
# M226 三轨对拍：对每条探针比较 解释轨 / VM 轨 / C 轨 的 (rc, R 码, 消息体, 程序输出)
# ------------------------------------------------------------
# 判据：**分叉必须为 0**（同一方法 × 同一位置 × 同一错类型 ⇒ 三轨同码 · 同文 · 同 rc）。
# 归一化口径沿用 M196 / M199：三轨的错误前缀各不相同
#   （`运行时错误: 错误 [R####] 行:列:` / `运行时错误 [fn 行N]: R####:`），
#   所以只比「R 码 + 剥掉前缀的消息体」——**别按整行文本对拍**（那是已登记缺陷 186 族）。
# 退出码：0 = 无分叉；1 = 有分叉（打印全部）；2 = 环境错误（件缺失）
# ============================================================
import argparse, json, os, re, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--work', required=True, help='含 cases.tsv 与 build/{drv_vm,drv_c}')
ap.add_argument('--pxi', default=None, help='解释轨件（默认 <root>/bootstrap/pxi）')
ap.add_argument('--timeout', type=int, default=20)
ap.add_argument('--json', default=None)
# M226 负控 C（**判据自伤**）：把「分叉」判据改成恒真 ⇒ 若此时 NC-A 的红**消失**，
#   就证明那道红确实来自本判据的比对，而不是来自别的偶然因素（M213/M214 同款手法）。
ap.add_argument('--harm', action='store_true', help='判据自伤：总是报告 0 分叉（负控 C 用）')
a = ap.parse_args()

PXI = a.pxi or os.path.join(a.root, 'bootstrap/pxi')
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

def norm(txt):
    """→ (R码, 消息体)。只取**第一行像错误**的行；前缀一律剥掉（缺陷 186 族不判）。"""
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
        b = re.sub(r'^运行时错误\s*', '', b)
        return '', re.sub(r'^:\s*', '', b)
    return '', ''


def run(cmd):
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=a.timeout,
                           cwd=a.work, errors='replace')
    except subprocess.TimeoutExpired:
        return 999, '', ''
    return p.returncode, p.stdout, p.stderr


labels, srcs = [], {}
for ln in open(f'{a.work}/cases.tsv', encoding='utf-8'):
    ln = ln.strip()
    if not ln:
        continue
    lb, src = ln.split('@@', 1)
    labels.append(lb); srcs[lb] = src

rows, div = [], []


def _cmd226(lb, t):
    return {'interp': [PXI, lb, f'{a.work}/drv.px'],
            'vm': [f'{a.work}/build/drv_vm', lb],
            'c': [f'{a.work}/build/drv_c', lb]}[t]


def _one226(k):
    """单例执行体 —— 与串行版**逐字节同逻辑**，只是由 pmap 调度。"""
    lb, t = k
    rc, out, err = run(_cmd226(lb, t))
    code, body = norm(err + '\n' + out)
    so = ''
    for ln in out.splitlines():
        if ln.startswith('/*') or ln.startswith(' *') or ln.startswith('*/'):
            continue
        so = ln.strip()
        break
    return k, {'rc': rc, 'code': code, 'body': body, 'out': so}


_GOT226 = pmap_records(_one226, [(lb, t) for lb in labels for t in ('interp', 'vm', 'c')])
for lb in labels:
    rec = {t: _GOT226[(lb, t)] for t in ('interp', 'vm', 'c')}
    rows.append({'label': lb, 'src': srcs[lb], **rec})
    if len({(rec[t]['rc'], rec[t]['code'], rec[t]['body'], rec[t]['out']) for t in rec}) > 1:
        div.append(rows[-1])
if a.harm:
    div = []

if a.json:
    json.dump(rows, open(a.json, 'w'), ensure_ascii=False, indent=1)

print(f'M226 方法面对拍：{len(labels)} 例 · 一致 {len(labels)-len(div)} · **分叉 {len(div)}**')
for r in div:
    print(f'  ▲ {r["label"]}   {r["src"][:60]}')
    for t in ('interp', 'vm', 'c'):
        d = r[t]
        print(f'      {t:6s} rc={d["rc"]:<4d} {d["code"]:6s} out=[{d["out"][:22]}] {d["body"][:64]}')
print('分叉标签：' + (','.join(r['label'] for r in div) if div else '(无)'))
sys.exit(1 if div else 0)
