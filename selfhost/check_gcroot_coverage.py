#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""GC 根登记「覆盖强度」台账 —— native 桥里有多少地方**只有靠过近似判据才看得见**

【来历（M269 · 清歌提案 P0-2）】
  清歌《PuXian 下一步发展方案 v1》§2.2 / §五.3：
    「M240–M259 连出 60+ 同族缺陷 ⇒ 筛网有洞」；
    「东月已有 check_root_contract.py，但那是**契约合规**，不是**覆盖完整**」；
    请求：「建 native 桥 → 是否登记 GC 根的**机械台账**，把『未审计』的桥**显式列出来并计数**」。
  ⚠️ 我方补充的必要条件（本轮写进实现）：**台账必须自带自证**
    （锚点 + 规模下限 + 判据自伤负控）。否则「未审计 = 0」与「扫描器坏了」**不可区分** ——
    M212 实测过这条：判据改成"源码派生触发点"了但名单没改 ⇒ 全仓"候选 0"其实是**漏报**。

【判据】把「桥面」按**判据强度**分两档（用同一份派生器的两个档位对照）：
  · `loose`（默认档）：精确三规则 + 任何 `(*x` 形状 —— **有意的过近似**（M213 缺陷 295）
  · `tight`：仅精确三规则（实测会丢失显式 GC 族覆盖 ⇒ 不作默认，**只用于分诊**）
  ⇒ `STRONG = {producer ∈ TRIGGER_tight}`（**强判据**：它的产出窗口在精确规则下也可见）
  ⇒ `WEAK   = P_loose \\ TRIGGER_tight`（**弱判据**：只有靠过近似才被看见）
  门判据：
    ① 规模锚点（P_loose ≥ 300 · T_loose ≥ 500）—— 防派生静默失效
    ② 划分完备：STRONG ∪ WEAK == P_loose 且不相交
    ③ **WEAK 不许扩大**：实测 WEAK − 基线 ⇒ 非空即判红（逐条指名）
    ④ **基线不许过期**：基线 − 实测 WEAK ⇒ 非空即判红
    ⑤ 每条基线条目必须带**图例里定义过的**分类标签；图例标签不许悬空（双向）
    ⑥ **判据自伤**：把 WEAK 定义改成恒空 ⇒ ③ 必须不再红（证明红来自判据而非别处）

【这不是"有没有缺陷"的判据，而是"还有多少地方看不清"的判据】
  它不保证零缺陷 —— 它让「判据强度不足的面」变成一个**可见、可追踪、只减不增**的数字。

用法:
    check_gcroot_coverage.py [--root .] [--baseline selfhost/gcroot_coverage.txt]
                             [--json OUT] [--update] [--self-test] [--weak-empty]
退出码: 0=通过 · 1=判红 · 2=用法/输入错 · 3=判据自身失效
"""
import argparse, json, os, re, subprocess, sys, tempfile, shutil

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
ap.add_argument('--baseline', default=None)
ap.add_argument('--json', default=None)
ap.add_argument('--update', action='store_true', help='用当前实测重写基线（保留已有标签）')
ap.add_argument('--self-test', action='store_true')
ap.add_argument('--weak-empty', action='store_true',
                help='A/B 用：把 WEAK 强制为空（观察划分/台账判据的反应）')
ap.add_argument('--no-weak-check', action='store_true',
                help='**判据自伤**用：跳过 [3] 段（证明红来自台账判据本身，而不是别处）')
ap.add_argument('--min-producers', type=int, default=300)
ap.add_argument('--min-triggers', type=int, default=500)
a = ap.parse_args()
ROOT = os.path.abspath(a.root)
BASE = a.baseline or os.path.join(ROOT, 'selfhost/gcroot_coverage.txt')
DERIVE = os.path.join(ROOT, 'selfhost/gcroot_derive.py')

PASS = 0; FAILN = 0
def ok(m):  global PASS; PASS += 1; print('  ✅ ' + m)
def bad(m): global FAILN; FAILN += 1; print('  ❌ ' + m)
def info(m): print('  · ' + m)

# ── 调派生器（同一份实现，只换档位）────────────────────────────
def derive(indirect):
    r = subprocess.run([sys.executable, DERIVE, '--list', 'both',
                        '--indirect', indirect, '--json'],
                       cwd=ROOT, capture_output=True, text=True, timeout=600)
    if r.returncode != 0:
        print('❌ 派生器失败（--indirect %s）：%s' % (indirect, (r.stderr or '')[:300]), file=sys.stderr)
        sys.exit(3)
    d = json.loads(r.stdout)
    return set(d['triggers']), set(d['producers'])

T_loose, P_loose = derive('loose')
T_tight, P_tight = derive('tight')

WEAK   = set() if a.weak_empty else (P_loose - T_tight)
STRONG = P_loose - WEAK

# ── 基线读写 ──────────────────────────────────────────────────
TAG_RE = re.compile(r'^#\s*@([A-Za-z0-9_\-]+)\s*:\s*(.+)$')

def read_baseline():
    tags, entries = {}, {}
    if not os.path.exists(BASE):
        return tags, entries
    for ln in open(BASE, encoding='utf-8'):
        ln = ln.rstrip('\n')
        m = TAG_RE.match(ln)
        if m:
            tags[m.group(1)] = m.group(2).strip(); continue
        if ln.startswith('#') or not ln.strip():
            continue
        parts = ln.split('\t')
        if len(parts) >= 2:
            entries[parts[0].strip()] = parts[1].strip()
    return tags, entries

def write_baseline(tags, entries):
    names = sorted(entries)
    with open(BASE, 'w', encoding='utf-8') as f:
        f.write('# M269 GC 根登记「覆盖强度」台账 —— 由 check_gcroot_coverage.py --update 生成\n')
        f.write('# ⚠️ 本表列的是「**判据强度不足**的桥」（WEAK = 只有靠 loose 过近似才进 TRIGGER）。\n')
        f.write('#   它**不是**缺陷清单，是「还有多少地方看不清」的账 —— **只许缩小**。\n')
        f.write('#   每行：<函数名>\\t<分类标签>；标签必须在下面的图例里定义过。\n')
        f.write('#\n')
        for t in sorted(tags):
            f.write('# @%s: %s\n' % (t, tags[t]))
        f.write('#\n')
        for n in names:
            f.write('%s\t%s\n' % (n, entries[n]))

def guess_tag(name):
    if name.startswith('bi_quic') or name.startswith('bi_h3') or 'quic' in name: return 'quic_h3'
    if name.startswith('bi_ws'):    return 'ws'
    if name.startswith('bi_sse'):   return 'sse'
    if name.startswith('bi_qs'):    return 'qpack'
    if name.startswith('bi_http') or name.startswith('bi_route') or name.startswith('bi_vhost'): return 'http_serve'
    return 'misc'

# ── 自证 ──────────────────────────────────────────────────────
if a.self_test:
    print('══ check_gcroot_coverage --self-test ══')
    # S1 派生器可跑且规模过锚点
    if len(P_loose) >= a.min_producers and len(T_loose) >= a.min_triggers:
        ok('S1 派生器规模过锚点（P=%d ≥ %d · T=%d ≥ %d）' % (len(P_loose), a.min_producers, len(T_loose), a.min_triggers))
    else:
        bad('S1 规模不足（P=%d · T=%d）' % (len(P_loose), len(T_loose)))
    # S2 tight ⊆ loose（档位关系：tight 只会更少）
    if T_tight <= T_loose:
        ok('S2 T_tight ⊆ T_loose（档位单调：%d ≤ %d）' % (len(T_tight), len(T_loose)))
    else:
        bad('S2 T_tight 不是 T_loose 的子集（档位关系被破坏）')
    # S3 WEAK 非空（否则判据退化 ⇒ 全部"强"是假象）
    w = P_loose - T_tight
    if len(w) >= 1:
        ok('S3 WEAK 非空（%d 条）—— 判据不是恒空' % len(w))
    else:
        bad('S3 WEAK 恒空 ⇒ 判据退化（"全强"是假象）')
    # S4 划分完备 + 不相交
    if STRONG | WEAK == P_loose and not (STRONG & WEAK):
        ok('S4 划分完备：STRONG ∪ WEAK == P_loose（%d）且不相交' % len(P_loose))
    else:
        bad('S4 划分不完备')
    # S5 图例机制可用（造一份临时基线验证双向判据有牙）
    tmpd = tempfile.mkdtemp()
    try:
        tbase = os.path.join(tmpd, 'cov.txt')
        open(tbase, 'w', encoding='utf-8').write('# @foo: 说明\nzzz_not_exist\tfoo\n')
        sys.argv = [sys.argv[0], '--root', ROOT, '--baseline', tbase]
        r = subprocess.run([sys.executable, os.path.abspath(__file__)] + sys.argv[1:],
                           capture_output=True, text=True, timeout=900)
        if r.returncode == 1 and '过期' in r.stdout:
            ok('S5 基线过期判据有牙（造一条不存在条目 ⇒ 判红并说"过期"）')
        else:
            bad('S5 过期判据没牙（rc=%d）' % r.returncode)
    finally:
        shutil.rmtree(tmpd, ignore_errors=True)
    print()
    print('══ 自证：%d 通过 / %d 失败 ══' % (PASS, FAILN))
    print('M269-SELFTEST-OK' if FAILN == 0 else 'M269-SELFTEST-FAIL')
    sys.exit(0 if FAILN == 0 else 1)

# ── 主判据 ────────────────────────────────────────────────────
print('══ M269 GC 根登记覆盖强度台账 ══')
print('  P_loose=%d  T_loose=%d  T_tight=%d  ⇒ STRONG=%d  WEAK=%d'
      % (len(P_loose), len(T_loose), len(T_tight), len(STRONG), len(WEAK)))

print()
print('[1] 规模锚点（防派生静默失效 ⇒ 空集 ⊇ 任意集 ⇒ 假绿）')
ok('P_loose = %d（下限 %d）' % (len(P_loose), a.min_producers)) if len(P_loose) >= a.min_producers \
    else bad('P_loose = %d < 下限 %d' % (len(P_loose), a.min_producers))
ok('T_loose = %d（下限 %d）' % (len(T_loose), a.min_triggers)) if len(T_loose) >= a.min_triggers \
    else bad('T_loose = %d < 下限 %d' % (len(T_loose), a.min_triggers))
ok('T_tight ⊆ T_loose（档位单调）') if T_tight <= T_loose else bad('T_tight ⊄ T_loose')

print()
print('[2] 划分完备（STRONG ∪ WEAK == P_loose，且不相交）')
if STRONG | WEAK == P_loose and not (STRONG & WEAK):
    ok('STRONG(%d) ∪ WEAK(%d) == P_loose(%d)' % (len(STRONG), len(WEAK), len(P_loose)))
else:
    bad('划分不完备：差 %d 条 / 交 %d 条' % (len(P_loose - (STRONG | WEAK)), len(STRONG & WEAK)))

print()
tags, entries = read_baseline()
if a.no_weak_check:
    print('[3] WEAK 台账 —— **已跳过（--no-weak-check：判据自伤档）**')
    skipped = True
else:
    skipped = False
    print('[3] WEAK 台账（判据强度不足的面 —— **只许缩小**）')
base_names = set(entries)
if not skipped:
    new = WEAK - base_names
    gone = base_names - WEAK
    if new:
        bad('WEAK 扩大了 %d 条（**必须逐条定性后 --update**）：' % len(new))
        for n in sorted(new)[:20]:
            print('       %s' % n)
    else:
        ok('WEAK 未扩大（实测 %d 条 ⊆ 基线 %d 条）' % (len(WEAK), len(base_names)))
    if gone:
        bad('基线有 %d 条已过期（实测不再是 WEAK ⇒ 删掉登记）：%s' % (len(gone), ' '.join(sorted(gone)[:10])))
    else:
        ok('基线无过期条目')
if skipped:
    new, gone = set(), set()

print()
print('[4] 图例双向（每条登记必须带定义过的标签；每个标签必须被用到）')
used = set(entries.values())
undef = used - set(tags)
unused = set(tags) - used
if undef:
    bad('有登记用了未定义的标签：%s' % ' '.join(sorted(undef)))
else:
    ok('全部登记条目的标签都在图例里定义过（%d 条 / %d 标签）' % (len(entries), len(used)))
if unused:
    bad('图例里有悬空标签（定义了但没被用）：%s' % ' '.join(sorted(unused)))
else:
    ok('无悬空标签')

if a.json:
    json.dump({'producers_loose': sorted(P_loose), 'triggers_tight': sorted(T_tight),
               'strong': sorted(STRONG), 'weak': sorted(WEAK),
               'weak_new': sorted(new), 'weak_stale': sorted(gone)},
              open(a.json, 'w'), ensure_ascii=False, indent=1)

if a.update:
    nt = dict(tags)
    for n in sorted(WEAK):
        if n not in entries:
            nt.setdefault(guess_tag(n), '**待人工定性**：由 --update 自动登记，请补理由')
    ne = {n: entries.get(n, guess_tag(n)) for n in WEAK}
    write_baseline(nt, ne)
    print()
    print('  ✅ 已重写基线：%s（%d 条）' % (BASE, len(ne)))

print()
print('══ M269 结果：%d 通过 / %d 失败 ══' % (PASS, FAILN))
print('M269-VERIFY-OK' if FAILN == 0 else 'M269-VERIFY-FAIL')
sys.exit(0 if FAILN == 0 else 1)
