#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M260 · GC 压力筛「分批常态化」—— 批次结果合并进台账

为什么需要（M207/M212 的欠账）：
  M207 建了 `gcstress_sweep.sh`（判据扎实：2+2 跑差分 · KNOWN/SLOW 两张表），
  但**全量 425 个候选在压力档下是 O(n²)** ⇒ 单轮跑完要数小时 ⇒ **从来没被完整跑过第二轮**。
  M212 又加了 `--slow` 表，工具越来越完整，**而「覆盖率」这件事始终没有账**：
  没人知道哪些语料跑过压力档、结论是什么、什么时候跑的。
  ⇒ 本脚本 + `gcstress_ledger.tsv` 把「跑了多少」变成**可查、只增不减**的账。

台账格式（`selfhost/gcstress_ledger.tsv`）：
    路径 <TAB> 结论 <TAB> 日期 <TAB> 详情
  · 结论 ∈ PASS / FAIL_OUT / FAIL_RC / FAIL_SIG / FAIL_DIAG / STIMEOUT / NONDET /
            SKIP_NORM / SKIP_KNOWN / BUILDFAIL
  · **FAIL_\* 条目的详情必须带定性**（`缺陷NNN` 或 `假阳：<理由>`）—— 由 check 脚本判红。

用法：
    merge_ledger.py --ledger selfhost/gcstress_ledger.tsv --in /tmp/m260/batch_1.tsv [--date YYYY-MM-DD]
    merge_ledger.py --self-test

合并语义：**同路径以新结果覆盖**（`--in` 里的批次是较新的实跑）；其余条目原样保留。
⚠️ 保留「已定性」信息：若旧条目是 FAIL_* 且带缺陷编号，而新结果是 PASS，
   则写 `PASS(修复缺陷NNN后)` —— **修复的历史不丢**。
"""
import argparse, io, os, re, sys

TAGS = ('PASS', 'FAIL_OUT', 'FAIL_RC', 'FAIL_SIG', 'FAIL_DIAG', 'STIMEOUT',
        'NONDET', 'SKIP_NORM', 'SKIP_KNOWN', 'BUILDFAIL')
DEFECT = re.compile(r'缺陷\s*(\d+)|(假阳)')


def read_tsv(path):
    """读两类输入：① 批次结果（`TAG<TAB>路径<TAB>详情`）② 台账本身（`路径<TAB>结论<TAB>日期<TAB>详情`）。

    返回 [(路径, 结论, 详情, 日期或 None)] —— 日期 None 表示「本次跑出来的」（用 --date 填）。
    ⚠️ M260 首版 bug：台账分支把 `日期<TAB>详情` 一起塞进详情 ⇒ 写回变成 **5 列**。
       分列必须显式取 p[3] 作为详情、p[2] 作为旧日期。
    """
    rows = []
    if not os.path.exists(path):
        return rows
    for ln in io.open(path, encoding='utf-8'):
        if ln.startswith('#') or not ln.strip():
            continue
        p = ln.rstrip('\n').split('\t')
        if len(p) >= 2 and p[0] in TAGS:
            rows.append((p[1], p[0], p[2] if len(p) > 2 else '', None))
        elif len(p) >= 4:
            rows.append((p[0], p[1], p[3], p[2]))
        elif len(p) >= 2:
            rows.append((p[0], p[1], '', None))
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--ledger', default='selfhost/gcstress_ledger.tsv')
    ap.add_argument('--in', dest='inp', default=None)
    ap.add_argument('--date', default=None)
    ap.add_argument('--self-test', action='store_true')
    a = ap.parse_args()

    if a.self_test:
        import tempfile
        d = tempfile.mkdtemp()
        led = os.path.join(d, 'l.tsv')
        io.open(led, 'w', encoding='utf-8').write(
            "# 台账\nexamples/a.px\tPASS\t2026-10-01\tok\n"
            "examples/b.px\tFAIL_SIG\t2026-10-01\t缺陷 267\n")
        bt = os.path.join(d, 'b.tsv')
        io.open(bt, 'w', encoding='utf-8').write(
            "PASS\texamples/a.px\t\nPASS\texamples/b.px\t\n"
            "PASS\texamples/c.px\t\n")
        os.system('python3 %s --ledger %s --in %s --date 2026-10-05' % (sys.argv[0], led, bt))
        out = io.open(led, encoding='utf-8').read()
        ok = 0
        if 'examples/a.px\tPASS\t2026-10-05' in out:
            ok += 1
        if '缺陷 267' in out and 'PASS(修复缺陷 267 后)' in out:
            ok += 1
        if 'examples/c.px\tPASS\t2026-10-05' in out:
            ok += 1
        print("self-test: %d/3" % ok)
        sys.exit(0 if ok == 3 else 1)

    if not a.inp:
        sys.stderr.write("需要 --in\n")
        sys.exit(2)
    date = a.date or 'unknown'
    old = {}
    for path, tag, det, d0 in read_tsv(a.ledger):
        old[path] = (tag, det, d0 or date)
    new = read_tsv(a.inp)
    if not new:
        sys.stderr.write("输入为空：%s\n" % a.inp)
        sys.exit(2)
    for path, tag, det, _d in new:
        prev = old.get(path)
        if prev and tag == 'PASS' and prev[0].startswith('FAIL'):
            m = DEFECT.search(prev[1])
            if m:
                det = ('PASS(修复缺陷 %s 后)' % m.group(1)) if m.group(1) else 'PASS(原为假阳)'
        old[path] = (tag, det, date)
    with io.open(a.ledger, 'w', encoding='utf-8') as f:
        f.write("# M260 · GC 压力档台账 —— 「哪些语料跑过压力档 · 结论 · 何时跑的」\n")
        f.write("# 由 selfhost/merge_ledger.py 维护（本表**只增不减**：条目数只能增，路径不删）。\n")
        f.write("# 结论 ∈ %s\n" % ' / '.join(TAGS))
        f.write("# FAIL_* 的详情**必须带定性**（缺陷NNN 或 假阳：<理由>）—— 由 check 脚本判红。\n")
        for path in sorted(old):
            tag, det, d0 = old[path]
            f.write("%s\t%s\t%s\t%s\n" % (path, tag, d0, det))
    print("台账已更新：%s · 本次 %d 条 · 合计 %d 条" % (a.ledger, len(new), len(old)))


main()
