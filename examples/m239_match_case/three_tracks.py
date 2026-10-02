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
# ── M244（门内并行）：逐例 spawn 彼此**无依赖**（同一驱动器、不同 case 选择，
#    独立进程、不写共享文件）⇒ 用 gate_par.pmap 吃满整机核数。
#    ⚠️ 门**之间**仍然串行（run_gates.sh 的 PID 锁）—— 53 个门的负控会改源码，
#       门间并行必然互相踩；门内并行与那条护栏正交。
sys.path.insert(0, os.path.join(ROOT, 'selfhost'))
from gate_par import pmap, pmap_records   # noqa: E402

nfork = 0; nok = 0; det = []
def _one239(c):
    """单例执行体（三轨各 spawn 一次）—— 与串行版逐字节同逻辑。

    ⚠️ 下面第 65 行的 `if r[0] == r[1] == r[2]:` 是 **m239 负控 D 的打桩锚点**，
       必须原样保留（改它 = 让旧负控静默失效，本仓撞过 4 次）。
    """
    e = dict(env); e['M239C'] = c
    out = []
    for cmd in ([PXI, DRV], [cbin], [vbin]):
        p = subprocess.run(cmd, capture_output=True, text=True, env=e, cwd=W, timeout=30)
        out.append((p.returncode, norm(p.stdout + p.stderr)))
    return c, out


_GOT239 = dict(pmap(_one239, cases))
for c in cases:
    r = _GOT239[c]
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
