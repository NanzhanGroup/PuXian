#!/usr/bin/env python3
# ============================================================
# M199 [S10-b] 三轨对拍：对每条自动探针比较 解释轨 / VM 轨 / C 轨 的 (rc, R 码, 消息体, stdout)
# ------------------------------------------------------------
# 判据：**分叉必须为 0**（同一操作的同一位置 × 错类型 ⇒ 三轨同码 · 同文 · 同 rc）。
# 归一化：三轨的错误前缀不同（`运行时错误: 错误 [R####] 行:列:` / `运行时错误 [fn 行N]: R####:`），
#   所以只比「R 码 + 剥掉前缀的消息体」——与 M196 门同口径（别按整行文本对拍）。
# ============================================================
import argparse, json, re, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
ap.add_argument('--sweep', default='/tmp/m199sweep')
ap.add_argument('--build', default='/tmp/m199sweep/build')
ap.add_argument('--timeout', type=int, default=20)
a = ap.parse_args()


def norm(txt):
    for ln in txt.splitlines():
        if '错误' not in ln:
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


def run(cmd, cwd):
    p = subprocess.run(cmd, capture_output=True, text=True, timeout=a.timeout, cwd=cwd, errors='replace')
    return p.returncode, p.stdout, p.stderr


labels = [l.strip().split('@@')[0] for l in open(f'{a.sweep}/autocases.tsv') if l.strip()]
rows, div = [], []
for label in labels:
    rec = {'label': label}
    cmds = {'interp': [f'{a.root}/bootstrap/pxi', label, f'{a.sweep}/drv.px'],
            'vm': [f'{a.build}/drv_vm', label],
            'c': [f'{a.build}/drv_c', label]}
    for name, cmd in cmds.items():
        rc, out, err = run(cmd, a.sweep)
        code, body = norm(err + '\n' + out)
        rec[name] = {'rc': rc, 'code': code, 'body': body, 'out': (out.strip().splitlines() or [''])[0]}
    rows.append(rec)
    sig = {(rec[t]['rc'], rec[t]['code'], rec[t]['body'], rec[t]['out']) for t in ('interp', 'vm', 'c')}
    if len(sig) > 1:
        div.append((label, rec))

json.dump(rows, open(f'{a.sweep}/three_tracks.json', 'w'), ensure_ascii=False, indent=1)
print(f'对拍 {len(labels)} 例（函数位置）· 一致 {len(labels) - len(div)} · 分叉 {len(div)}')
for label, r in div:
    print(f'▲ {label}')
    for t in ('interp', 'vm', 'c'):
        print(f'    {t:6s} rc={r[t]["rc"]:<3} {r[t]["code"]:6s} out=[{r[t]["out"][:26]}] {r[t]["body"][:78]}')
sys.exit(1 if div else 0)
