#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M244 静态判据：`targets.tsv`（清单）⇄ **源码派生**（哪些执行器真的用了 gate_par）

判据（全绿 rc=0）
-----------------
[A] 派生集合 ⇄ 清单 **双向精确相等**
      漏登记 ⇒ 某个门的内联并行"悄悄没接上"（而它看起来还在跑，只是慢）
      悬空   ⇒ 清单过期（文件改名/删除后没人发现）
[B] 每个目标：`from gate_par import` + 至少一处 `pmap(` / `pmap_records(` 调用
[C] 每个目标：插入了 `sys.path.insert(0, os.path.join(<root>, 'selfhost'))`
      —— 否则运行期 **ModuleNotFoundError**（而且是"跑起来才炸"）
[D] 规模锚点：目标 ≥7 + 每个目标都在磁盘上
[E] **负控锚点自证**：negctl 要 patch 的文本在 `selfhost/gate_par.py` 里**各命中恰 1 次**
      —— 锚点漂了 ⇒ 负控会**静默失效**（本仓撞过 4 次：M161/M164/M227/M230）
[F] 缺陷 412 静态侧：`run_gates.sh` 里 `: > "$GATE_TSV"` 必须出现在 `--list` 短路**之后**
"""
import argparse
import os
import re
import sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--here', default=os.path.dirname(os.path.abspath(__file__)))
a = ap.parse_args()
ROOT, HERE = a.root, a.here

ok = [0]
bad = [0]


def chk(name, cond, extra=''):
    if cond:
        print('  PASS %s' % name)
        ok[0] += 1
    else:
        print('  FAIL %s %s' % (name, extra))
        bad[0] += 1


# ── 派生：examples/*/*.py 里 import 了 gate_par 的 ──────────────
# ⚠️ **显式豁免**（本仓纪律：豁免必须带理由 + 计数 + 不会静默过期）：
#   本门自己的判据脚本 `bench.py` 也 `from gate_par import` —— 但它是**判据**，
#   不是「被并行的门执行器」。豁免集必须**非空**（下 [A2] 断言），
#   否则「派生集为空 ⇒ 双向相等」会在某天静默变绿（M243 的例外表同款纪律）。
EXCLUDE = {'examples/m244_gate_parallel/bench.py'}
DERIVED = set()
for sub in sorted(os.listdir(os.path.join(ROOT, 'examples'))):
    d = os.path.join(ROOT, 'examples', sub)
    if not os.path.isdir(d):
        continue
    for f in sorted(os.listdir(d)):
        if not f.endswith('.py'):
            continue
        p = os.path.join(d, f)
        try:
            txt = open(p, encoding='utf-8').read()
        except OSError:
            continue
        if re.search(r'^\s*from gate_par import\s', txt, re.M):
            DERIVED.add(os.path.relpath(p, ROOT))
DERIVED_RAW = set(DERIVED)
DERIVED -= EXCLUDE

# ── 清单 ────────────────────────────────────────────────────────
LISTED = {}
for ln in open(os.path.join(HERE, 'targets.tsv'), encoding='utf-8'):
    ln = ln.rstrip('\n')
    if not ln.strip() or ln.lstrip().startswith('#'):
        continue
    f = ln.split('\t')
    if f[0] == '门':
        continue
    LISTED[f[1].strip()] = f[0].strip()

derived, listed = set(DERIVED), set(LISTED)

print('  [A] 派生 %d 个（原始 %d，豁免 %d）· 清单 %d 个'
      % (len(DERIVED), len(DERIVED_RAW), len(EXCLUDE), len(listed)))
chk('[A2] 豁免集非空 且 每个豁免项确实在原始派生集里',
    len(EXCLUDE) >= 1 and EXCLUDE <= DERIVED_RAW,
    '豁免 %s 不在原始派生集 %s' % (sorted(EXCLUDE - DERIVED_RAW), sorted(DERIVED_RAW)))
chk('[A] 清单 ⇄ 派生精确相等（漏登记 0 · 悬空 0）', derived == listed,
    '\n       漏登记(派生有、清单无): %s\n       悬空(清单有、派生无): %s'
    % (sorted(derived - listed), sorted(listed - derived)))
chk('[D] 规模锚点：目标 ≥7', len(listed) >= 7, '实测 %d' % len(listed))

for rel in sorted(listed):
    p = os.path.join(ROOT, rel)
    if not os.path.exists(p):
        chk('[D] %s 存在' % rel, False)
        continue
    txt = open(p, encoding='utf-8').read()
    code = '\n'.join(l for l in txt.split('\n') if not l.lstrip().startswith('#'))
    chk('[B] %s：import + 调度调用' % os.path.basename(rel),
        bool(re.search(r'from gate_par import\s', code))
        and bool(re.search(r'\bpmap(_records)?\(', code)))
    # ⚠️ M246（缺陷 421）：判据原先写死 `"'selfhost'"`（**单引号**）⇒ 新执行器用
    #    `"selfhost"`（双引号）时**功能正确却判红** —— 这是**拿引号风格当判据** = 判据**过窄**
    #    （与「判据过宽」一样是错）。放宽为「两种引号都认」，仍要求
    #    `sys.path.insert(0, os.path.join(` 的**字面形态**（那是**形状**，不是风格）。
    chk('[C] %s：sys.path.insert(…selfhost)' % os.path.basename(rel),
        "sys.path.insert(0, os.path.join(" in code
        and ("'selfhost'" in code or '"selfhost"' in code))

# ── [E] 负控锚点自证 ────────────────────────────────────────────
gp = os.path.join(ROOT, 'selfhost/gate_par.py')
if not os.path.exists(gp):
    chk('[E] gate_par.py 存在', False)
else:
    gt = open(gp, encoding='utf-8').read()
    ANCHORS = [
        '        return list(ex.map(fn, items))',        # NC-A 要改成不保序
        "    return os.cpu_count() or 4",                # NC-B 要改成恒 1
    ]
    for an in ANCHORS:
        n = gt.count(an)
        chk('[E] 锚点唯一命中（%r…）' % an[:28], n == 1, '实测 %d 次' % n)

# ── [F] 缺陷 412 静态侧：--list 之后才截断 TSV ──────────────────
rg = os.path.join(ROOT, 'selfhost/run_gates.sh')
rt = open(rg, encoding='utf-8').read()
i_list = rt.find('if [ "$GATE_LIST" = 1 ]; then\n    [ -f "$REG" ]')
i_trunc = rt.find(': > "$GATE_TSV"')
chk('[F] run_gates.sh：TSV 截断在 --list 短路之后（缺陷 412）',
    i_list > 0 and i_trunc > i_list, 'list=%d trunc=%d' % (i_list, i_trunc))

print('M244-STATIC-%s pass=%d fail=%d' % ('OK' if not bad[0] else 'FAIL', ok[0], bad[0]))
sys.exit(1 if bad[0] else 0)
