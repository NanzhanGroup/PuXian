#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M244 动态判据

[3] **合成负载**（证明并行机制本身有效，且**判据不依赖真实门的规模**）
     64 个任务（各 `sleep 60ms`）· `n=1` ⇄ `n=8`
     ⇒ ① 结果**逐字节相同**（保序）② 加速比 ≥3.0（8 核，理想 8x；留足余量）

[4] **真实驱动**（证明并行不只对合成负载有效）
     仓库内 `examples/m231_op_matrix/drv.px`（**真实语料**）× 120 例 · 解释轨（`bootstrap/pxi`）
     ⇒ ① 两次输出**逐字节相同** ② 加速比 ≥1.8
     ⚠️ 用**真实驱动**而不是自造小程序：`pxi` 每次都要重新解析 29KB 的驱动器，
        这正是全量门里占大头的那段开销。

`--harm`（判据自伤，负控 C 用）：把**等价类**判据（[3] 保序 + [4] 等价）**全部**改成恒真
  —— 负控 C 的语义是「让 A 的红消失」，只关 [4] 而 [3] 仍红 ⇒ C 自己会红（实测撞过）。
"""
import argparse
import os
import subprocess
import sys
import time

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--drv', default=None)
ap.add_argument('--pxi', default=None)
ap.add_argument('--n', type=int, default=120)
ap.add_argument('--harm', action='store_true', help='判据自伤：[4] 等价比较恒真')
a = ap.parse_args()
ROOT = a.root
DRV = a.drv or os.path.join(ROOT, 'examples/m231_op_matrix/drv.px')
PXI = a.pxi or os.path.join(ROOT, 'bootstrap/pxi')

sys.path.insert(0, os.path.join(ROOT, 'selfhost'))
from gate_par import pmap, jobs        # noqa: E402

ok, bad = 0, 0


def chk(name, cond, extra=''):
    global ok, bad
    if cond:
        print('  PASS %s' % name)
        ok += 1
    else:
        print('  FAIL %s' % (name + (' ' + extra if extra else '')))
        bad += 1


# ── [3] 合成负载 ────────────────────────────────────────────────
TASKS, DELAY = 64, 0.06


def synth(i):
    time.sleep(DELAY)
    return 'T%03d|%d' % (i, i * i)


t = time.time(); r1 = pmap(synth, range(TASKS), n=1); s1 = time.time() - t
# ⚠️ 并行档**不写死 n**，用默认（= `jobs()` = 本机核数）—— 否则负控 B
#    （把 `jobs()` 打成恒 1）**没有牙**（M209 的 `LEGACY_GROW` 老坑：判据的开关没接上）。
t = time.time(); r8 = pmap(synth, range(TASKS)); s8 = time.time() - t
sp = s1 / s8 if s8 > 0 else 0
if r1 != r8 and a.harm:
    print('  ℹ [3] --harm：保序比较被强制为真（负控 C）')
chk('[3] 合成负载：n=1 与 n=8 结果逐字节相同（保序）', (r1 == r8) or a.harm)
chk('[3] 合成负载：加速比 ≥3.0（实测 %.2fx：n=1 %.2fs → n=8 %.2fs）' % (sp, s1, s8), sp >= 3.0)

# ── [4] 真实驱动（m231 的 drv.px · 解释轨）─────────────────────
if not (os.path.exists(DRV) and os.path.exists(PXI)):
    chk('[4] 真实驱动存在（%s）' % DRV, False)
else:
    idx = []
    for ln in open(os.path.join(ROOT, 'examples/m231_op_matrix/cases.tsv'), encoding='utf-8'):
        f0 = ln.split('\t')[0]
        if f0.isdigit():
            idx.append(int(f0))
    idx = idx[:a.n]
    chk('[4] 语料规模 ≥ 60（实测 %d）' % len(idx), len(idx) >= 60)

    def one(i):
        env = dict(os.environ)
        env['M231_CASE'] = str(i)
        p = subprocess.run([PXI, DRV], capture_output=True, text=True,
                           env=env, errors='replace', timeout=30)
        return '%d|%s|%s' % (i, (p.stdout or '').strip(), (p.stderr or '').strip()[:100])

    t = time.time(); o1 = '\n'.join(pmap(one, idx, n=1)); s1 = time.time() - t
    t = time.time(); o8 = '\n'.join(pmap(one, idx)); s8 = time.time() - t
    sp = s1 / s8 if s8 > 0 else 0
    eq = (o1 == o8) or a.harm
    if o1 != o8 and a.harm:
        print('  ℹ [4] --harm：等价比较被强制为真（负控 C）')
    chk('[4] 真实驱动：n=1 与 n=8 输出逐字节相同（%d 例）' % len(idx), eq)
    chk('[4] 真实驱动：加速比 ≥1.8（实测 %.2fx：n=1 %.1fs → n=8 %.1fs）' % (sp, s1, s8), sp >= 1.8)

print('M244-BENCH-%s pass=%d fail=%d（jobs()=%d）'
      % ('OK' if not bad else 'FAIL', ok, bad, jobs()))
sys.exit(1 if bad else 0)
