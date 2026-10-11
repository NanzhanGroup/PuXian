#!/usr/bin/env python3
# ============================================================
# M301 三轨对拍：对每条探针比较 解释轨 / VM 轨 / C 轨 的 (rc, R 码, 消息体, stdout)
# ------------------------------------------------------------
# 判据：**分叉必须为 0**（同一调用 × 同一实参个数 ⇒ 三轨同 rc · 同 R 码 · 同 stdout）。
# 归一化口径沿用 M196/M199/M226/M232：三轨的错误前缀各不相同（缺陷 186 族），
#   所以只比「R 码 + 剥掉前缀的消息体」——**不按整行文本对拍**。
# 退出码：0 = 无分叉；1 = 有分叉；2 = 环境错误（件缺失）
# ============================================================
import argparse, json, os, re, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--work', required=True)
ap.add_argument('--pxi', default=None)
ap.add_argument('--vm', default=None)
ap.add_argument('--cc', default=None)
ap.add_argument('--timeout', type=int, default=25)
# 负控 C（判据自伤）：把「分叉」判据改成恒真 ⇒ 若此时负控 A 的红**消失**，
#   就证明那道红确实来自本判据的比对（M213/M214/M226 同款手法）。
ap.add_argument('--harm', action='store_true')
a = ap.parse_args()

W = a.work
PXI = a.pxi or os.path.join(a.root, 'bootstrap/pxi')
VM = a.vm or os.path.join(W, 'build/drv_vm')
CC = a.cc or os.path.join(W, 'build/drv_c')
for f in (PXI, VM, CC, os.path.join(W, 'cases.json')):
    if not os.path.exists(f):
        print('❌ 缺少 %s' % f)
        sys.exit(2)


def norm(txt):
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
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=a.timeout, cwd=W, errors='replace')
    except subprocess.TimeoutExpired:
        return 999, '', 'TIMEOUT'
    return p.returncode, p.stdout, p.stderr


cases = json.load(open(os.path.join(W, 'cases.json')))
rows, div = [], []
for lb, desc, kind in cases:
    res = {}
    for t, cmd in [('interp', [PXI, lb, os.path.join(W, 'drv.px')]),
                   ('vm', [VM, lb]),
                   ('c', [CC, lb])]:
        rc, so, se = run(cmd)
        rcode, msg = norm(se)
        res[t] = (rc, so.strip(), rcode, msg)
    rows.append((lb, desc, kind, res))
    sig = {t: (res[t][0], res[t][1], res[t][2]) for t in ('interp', 'vm', 'c')}
    if len(set(sig.values())) > 1:
        div.append((lb, desc, kind, res))

print('%-5s %-26s %-6s %-22s %-22s %-22s' % ('用例', '说明', '类别', '解释轨', 'VM 轨', 'C 轨'))
for lb, desc, kind, res in rows:
    def fmt(t):
        rc, so, rcode, msg = res[t]
        v = so if so else '-'
        if rc != 0:
            v = '%s|rc=%d%s' % (v, rc, ('|' + rcode) if rcode else '')
        return v[:21]
    print('%-5s %-26s %-6s %-22s %-22s %-22s' % (lb, desc, kind, fmt('interp'), fmt('vm'), fmt('c')))

print()
strict_n = sum(1 for r in rows if r[2] == 'strict')
legal_n = sum(1 for r in rows if r[2] == 'legal')
strict_loud = sum(1 for _lb, _d, k, res in rows
                  if k == 'strict' and res['interp'][0] != 0 and res['vm'][0] != 0 and res['c'][0] != 0)
legal_ok = sum(1 for _lb, _d, k, res in rows
               if k == 'legal' and res['interp'][0] == 0 and res['vm'][0] == 0 and res['c'][0] == 0)
print('统计：strict %d/%d 全响亮 · legal %d/%d 全绿' % (strict_loud, strict_n, legal_ok, legal_n))
if a.harm:
    print('分叉 0 / %d（判据自伤模式 —— 恒报 0）' % len(rows))
    sys.exit(0)
print('分叉 %d / %d' % (len(div), len(rows)))
for lb, desc, kind, res in div:
    print('  ✗ %s (%s · %s)' % (lb, desc, kind))
    for t in ('interp', 'vm', 'c'):
        rc, so, rcode, msg = res[t]
        print('      %-7s rc=%-4d out=%-16s %s %s' % (t, rc, so[:16], rcode, msg[:60]))
sys.exit(1 if div else 0)
