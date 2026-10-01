#!/usr/bin/env python3
# M239 门 · 三轨对拍（聚合驱动器范式：一次编译，80 例 × 3 轨 = 240 次执行）
# 用法: three_tracks.py --root <仓库根> --work <工作目录> [--dev]
import os, re, subprocess, sys, argparse

def norm(txt):
    out = []
    for L in txt.split('\n'):
        L = L.rstrip()
        if not L: continue
        L = re.sub(r'^运行时错误\s*\[[^\]]*\]:\s*', '', L)
        L = re.sub(r'^运行时错误:\s*', '', L)
        L = re.sub(r'^错误\s*\[R([0-9]+)\]\s*[0-9]+:[0-9]+:\s*', r'R\1: ', L)
        L = re.sub(r'^R([0-9]+):\s*', r'R\1: ', L)
        out.append(L)
    return out

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--work', required=True)
ap.add_argument('--drv', default=None)
ap.add_argument('--dev', action='store_true', help='用 dev 件（/tmp/pxcdev 等）而非入库 bootstrap')
a = ap.parse_args()
ROOT = a.root
D = os.path.join(ROOT, 'examples/m239_match_case')
DRV = a.drv or os.path.join(D, 'drv.px')
W = a.work
os.makedirs(W, exist_ok=True)

if a.dev:
    PXC = os.environ.get('M239_PXC', '/tmp/pxcdev')
    PXCV = os.environ.get('M239_PXCV', '/tmp/pxcdev_vm')
    PXI = os.environ.get('M239_PXI', '/tmp/pxidev')
else:
    PXC = os.environ.get('M239_PXC', os.path.join(ROOT, 'bootstrap/pxc'))
    PXCV = os.environ.get('M239_PXCV', os.path.join(ROOT, 'bootstrap/pxc_vm'))
    PXI = os.environ.get('M239_PXI', os.path.join(ROOT, 'bootstrap/pxi'))

def build(tag, env_extra, flags):
    d = os.path.join(W, 'b_' + tag)
    subprocess.run(['rm', '-rf', d]); os.makedirs(d)
    import shutil; shutil.copy(DRV, os.path.join(d, 'drv.px'))
    env = dict(os.environ); env.update(env_extra)
    p = subprocess.run([os.path.join(ROOT, 'tools/px'), 'build'] + flags + ['drv.px'],
                       cwd=d, capture_output=True, text=True, env=env, timeout=1800)
    out = os.path.join(d, 'build/drv')
    if p.returncode != 0 or not os.path.exists(out):
        print('BUILD-FAIL', tag); print(p.stdout[-2000:]); print(p.stderr[-2000:]); sys.exit(1)
    dst = os.path.join(W, 'drv_' + tag)
    shutil.copy(out, dst)
    return dst

cbin = build('c', {'PX_PXC_BIN': PXC, 'PX_BUILD_ENGINE': 'c'}, ['--c'])
vbin = build('vm', {'PXC_VM_BIN': PXCV}, [])

cases = [L.strip() for L in open(os.path.join(D, 'cases.txt')) if L.strip()]
env = os.environ
nfork = 0; nok = 0; det = []
for c in cases:
    e = dict(env); e['M239C'] = c
    r = []
    for cmd in ([PXI, DRV], [cbin], [vbin]):
        p = subprocess.run(cmd, capture_output=True, text=True, env=e, cwd=W, timeout=30)
        r.append((p.returncode, norm(p.stdout + p.stderr)))
    if r[0] == r[1] == r[2]:
        nok += 1
    else:
        nfork += 1
        det.append((c, r))
open(os.path.join(W, 'res.txt'), 'w').write(
    'OK=%d FORK=%d\n' % (nok, nfork) +
    '\n'.join('%s\n  int %s\n  c   %s\n  vm  %s' % (c, x[0], x[1], x[2]) for c, x in det))
print('M239-THREE-TRACKS OK=%d FORK=%d' % (nok, nfork))
for c, x in det[:12]:
    print('FORK %s: int=%s c=%s vm=%s' % (c, x[0], x[1], x[2]))
sys.exit(1 if nfork else 0)
