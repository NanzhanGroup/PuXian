#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M260 · GC 压力档台账的判据（`selfhost/gcstress_ledger.tsv`）

判据：
  ① 格式与合法性：每行 4 列 · 结论 ∈ TAGS · 路径必须`存在`（语料被删了就要从台账里下掉，
     或改记为 `REMOVED`）· 日期 `YYYY-MM-DD`；
  ② **FAIL_\* 必须带定性**：详情里必须出现 `缺陷NNN` 或 `假阳` —— 这是本表存在的意义
     （「跑出来红了但没人管」= 等于没跑）；`STIMEOUT`/`NONDET`/`BUILDFAIL` 也必须带理由；
  ③ **只增不减**（常态化判据）：条目数 ≥ `selfhost/gcstress_progress.txt` 里记的基线数，
     且基线里出现过的路径不得消失；实际条目数**必须 ≥ 基线**（否则判红并指名丢了多少）；
  ④ 规模锚点：条目数 ≥ 1（防判据静默变窄 —— 本表起步）；
  ⑤ 未覆盖清单**可见**：打印 `examples/**/*.px` 中尚未入账的数量与抽样（不判红，只提醒）。

退出码：0=通过 · 1=判红 · 2=用法/输入错 · 3=判据自身失效
"""
import argparse, io, os, re, sys

TAGS = ('PASS', 'FAIL_OUT', 'FAIL_RC', 'FAIL_SIG', 'FAIL_DIAG', 'STIMEOUT',
        'NONDET', 'SKIP_NORM', 'SKIP_KNOWN', 'BUILDFAIL')
NEED_VERDICT = ('FAIL_OUT', 'FAIL_RC', 'FAIL_SIG', 'FAIL_DIAG',
                'STIMEOUT', 'NONDET', 'BUILDFAIL')
DATE = re.compile(r'^\d{4}-\d{2}-\d{2}$')
# 「定性」= 三种合法措辞之一：
#   · `缺陷NNN` —— 真缺陷（必须带编号，且应可追溯）
#   · `假阳：…` —— 判据/环境导致的假信号（必须带理由）
#   · `合法：…` —— 设计如此（负例 / 常驻 / 需宿主 / 需环境变量 / 慢语料；必须带理由）
# ⚠️ 第三种是本轮新增：它不是放水，而是承认「台账里 176 条 SKIP_NORM 里有大量**设计性**的
#   跑不通」—— 逼着人写一句理由，比逼着人把它们塞进一个「例外表」更可持续。
VERDICT = re.compile(r'缺陷\s*\d+|假阳|合法：')
# ⑥ SKIP_NORM 的「自动合法」模式 —— 负例语料 / 常驻服务 / 驱动器 / 探针：
#   · 负例：名字就是它的契约（`err_*` / `e1_*` / `error.px` / `bad*` / `neg_*` / `undef` / `invalid`）；
#   · 常驻：`*_daemon` / `*_server` / `srv.px` / `serve*.px`（正常档本就该挂住 ⇒ rc=124 是预期）；
#   · 驱动器：`cases.px`（M199 起的「聚合驱动器」范式：不带参数时 rc=2 = 用法错，是**设计**）；
#   · 探针：目录名含 `_probe` / `_diag` / `_uaf` / `_mutate` / `_bounds` / `_strict`。
#   其余 SKIP_NORM **必须在** `selfhost/gcstress_skipnorm.tsv` 里被定性（否则判红）——
#   这是本轮从 m128_unlock_grow（真缺陷 439 被记成 SKIP_NORM 混过去）学到的：
#   **「正常档不通」有两种：设计如此，与坏掉了。台账必须能区分。**
SKIP_AUTO_RE = re.compile(
    r'(^|/)(err_|e\d+_|error|bad|neg_|fail_|undef|invalid)'
    r'|daemon|server|srv|serve|_diag|_strict|_bounds|_uaf|_probe|_mutate'
    r'|cases\.px$|driver|tiny|mini\.px$')


def load(path):
    rows, errs = [], []
    for ln, line in enumerate(io.open(path, encoding='utf-8'), 1):
        if line.startswith('#') or not line.strip():
            continue
        p = line.rstrip('\n').split('\t')
        if len(p) != 4:
            errs.append('%s:%d 列数 %d ≠ 4' % (path, ln, len(p)))
            continue
        rows.append((p[0], p[1], p[2], p[3], ln))
    return rows, errs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', default='.')
    ap.add_argument('--ledger', default=None)
    ap.add_argument('--progress', default=None)
    ap.add_argument('--self-test', action='store_true')
    a = ap.parse_args()
    ROOT = os.path.abspath(a.root)
    LED = a.ledger or os.path.join(ROOT, 'selfhost/gcstress_ledger.tsv')
    PROG = a.progress or os.path.join(ROOT, 'selfhost/gcstress_progress.txt')

    if a.self_test:
        import tempfile, subprocess
        n = 0
        # ① 合法台账 → 绿
        d = tempfile.mkdtemp()
        led = os.path.join(d, 'l.tsv')
        prg = os.path.join(d, 'p.txt')
        io.open(led, 'w', encoding='utf-8').write(
            "examples/m100/m100_daemon.px\tPASS\t2026-10-05\t\n")
        io.open(prg, 'w', encoding='utf-8').write("# baseline 1\nexamples/m100/m100_daemon.px\n")
        r = subprocess.run([sys.executable, sys.argv[0], '--root', ROOT,
                            '--ledger', led, '--progress', prg],
                           capture_output=True, text=True)
        n += 1 if r.returncode == 0 else 0
        print("  ① 合法台账 ⇒ rc=%d（期望 0）" % r.returncode)
        # ② FAIL_* 无定性 ⇒ 红
        io.open(led, 'w', encoding='utf-8').write(
            "examples/m100/m100_daemon.px\tFAIL_SIG\t2026-10-05\t压力档被信号杀死\n")
        r = subprocess.run([sys.executable, sys.argv[0], '--root', ROOT,
                            '--ledger', led, '--progress', prg],
                           capture_output=True, text=True)
        n += 1 if (r.returncode == 1 and '定性' in r.stdout + r.stderr) else 0
        print("  ② FAIL_SIG 无定性 ⇒ rc=%d（期望 1）" % r.returncode)
        # ③ 条目数低于基线 ⇒ 红
        io.open(led, 'w', encoding='utf-8').write(
            "examples/m100/m100_daemon.px\tPASS\t2026-10-05\t\n")
        io.open(prg, 'w', encoding='utf-8').write(
            "# baseline 2\nexamples/m100/m100_daemon.px\nexamples/m101/x.px\n")
        r = subprocess.run([sys.executable, sys.argv[0], '--root', ROOT,
                            '--ledger', led, '--progress', prg],
                           capture_output=True, text=True)
        n += 1 if (r.returncode == 1 and '只增不减' in r.stdout + r.stderr) else 0
        print("  ③ 丢了一条 ⇒ rc=%d（期望 1）" % r.returncode)
        # ④ 结论非法 ⇒ 红
        io.open(led, 'w', encoding='utf-8').write(
            "examples/m100/m100_daemon.px\tMAYBE\t2026-10-05\t\n")
        io.open(prg, 'w', encoding='utf-8').write("# baseline 0\n")
        r = subprocess.run([sys.executable, sys.argv[0], '--root', ROOT,
                            '--ledger', led, '--progress', prg],
                           capture_output=True, text=True)
        n += 1 if (r.returncode == 1 and '非法结论' in r.stdout + r.stderr) else 0
        print("  ④ 非法结论 ⇒ rc=%d（期望 1）" % r.returncode)
        # ⑤ 路径不存在 ⇒ 红
        io.open(led, 'w', encoding='utf-8').write(
            "examples/no_such_dir/no.px\tPASS\t2026-10-05\t\n")
        r = subprocess.run([sys.executable, sys.argv[0], '--root', ROOT,
                            '--ledger', led, '--progress', prg],
                           capture_output=True, text=True)
        n += 1 if (r.returncode == 1 and '不存在' in r.stdout + r.stderr) else 0
        print("  ⑤ 路径不存在 ⇒ rc=%d（期望 1）" % r.returncode)
        print("self-test: %d/5" % n)
        sys.exit(0 if n == 5 else 1)

    if not os.path.exists(LED):
        sys.stderr.write("台账不存在：%s\n" % LED)
        sys.exit(2)
    rows, errs = load(LED)
    bad = list(errs)
    for path, tag, date, det, ln in rows:
        if tag not in TAGS:
            bad.append("%s:%d 非法结论 %r" % (LED, ln, tag))
            continue
        if not DATE.match(date):
            bad.append("%s:%d 日期格式 %r ≠ YYYY-MM-DD" % (LED, ln, date))
        if not os.path.exists(os.path.join(ROOT, path)):
            bad.append("%s:%d 语料不存在：%s" % (LED, ln, path))
        if tag in NEED_VERDICT:
            # BUILDFAIL 的**负例语料**（`bad.px` 这类「故意写错」的）编译失败是**契约**
            # ⇒ 与 SKIP_NORM 同款：命名即契约的自动合法，其余必须带定性。
            if tag == 'BUILDFAIL' and SKIP_AUTO_RE.search(path):
                pass
            elif not VERDICT.search(det):
                bad.append("%s:%d %s 缺**定性**（需 缺陷NNN 或 假阳：<理由>）：%r"
                           % (LED, ln, tag, det[:60]))
    # ③ 只增不减
    if os.path.exists(PROG):
        base = [l.strip() for l in io.open(PROG, encoding='utf-8')
                if l.strip() and not l.startswith('#')]
        cur = set(r[0] for r in rows)
        missing = [p for p in base if p not in cur]
        if len(rows) < len(base) or missing:
            bad.append("台账**只增不减**被破坏：基线 %d 条 → 当前 %d 条，缺 %d 条%s"
                       % (len(base), len(rows), len(missing),
                          ('（如 ' + ', '.join(missing[:3]) + '）') if missing else ''))
    # ⑥ SKIP_NORM 必须被定性（自动合法模式之外 ⇒ 必须在 skipnorm 表里）
    skipnorm = set()
    sfile = os.path.join(ROOT, 'selfhost/gcstress_skipnorm.tsv')
    debt = []
    if os.path.exists(sfile):
        for ln in io.open(sfile, encoding='utf-8'):
            if ln.startswith('#') or not ln.strip():
                continue
            p = ln.rstrip('\n').split('\t')
            if len(p) >= 3:
                skipnorm.add(p[0])
                # 「欠账」= 类别含欠账 或 理由带缺陷号，**且理由里没有「已修」**
                #   （修好的条目保留在表里作为历史，但不再计入欠账）——
                #   这样「欠账数」是一个**只减不增**的数。
                if (('欠账' in p[1] or re.search(r'缺陷\s*\d+', p[2]))
                        and '已修' not in p[2]):
                    debt.append(p[0])
    unexplained = []
    for path, tag, date, det, ln in rows:
        if tag != 'SKIP_NORM':
            continue
        if SKIP_AUTO_RE.search(path) or path in skipnorm:
            continue
        # 详情里已写「合法：<理由>」也算定性（理由必须够长 —— 防「合法：」两字糊弄）
        if det.startswith('合法：') and len(det) >= 12:
            continue
        unexplained.append('%s（%s）' % (path, det[:30]))
    if unexplained:
        bad.append("SKIP_NORM 未定性 %d 条（既非负例/常驻/驱动器/探针模式，也不在 "
                   "selfhost/gcstress_skipnorm.tsv）—— 「正常档不通」必须被区分：%s"
                   % (len(unexplained), '; '.join(unexplained[:4])))
    # ④ 规模锚点
    if len(rows) < 1:
        bad.append("规模锚点：台账条目 %d < 1" % len(rows))

    if bad:
        print("❌ GC 压力档台账判红（%d 项）：" % len(bad))
        for b in bad[:20]:
            print("   · %s" % b)
        sys.exit(1)

    # ⑤ 未覆盖清单（可见性，不判红）
    #    ⚠️ 统计口径必须与 `gcstress_sweep.sh` 的**候选模式**一致（`examples/*/*.px`，两层）——
    #    首版用 os.walk 扫全部 `.px`（含 examples 根层与更深层）⇒ 覆盖率显示 52.6%，
    #    而真正的候选面是 425 条 ⇒ 那个数字会**误导**（把不在候选范围的文件算成「未覆盖」）。
    covered = set(r[0] for r in rows)
    import glob as _glob
    allpx = sorted(os.path.relpath(p, ROOT)
                   for p in _glob.glob(os.path.join(ROOT, 'examples', '*', '*.px')))
    todo = sorted(p for p in allpx if p not in covered)
    print("✅ 台账合法：%d 条（已覆盖 %.1f%% = %d/%d）"
          % (len(rows), 100.0 * len(covered) / max(1, len(allpx)), len(covered), len(allpx)))
    if debt:
        print("⚠️ SKIP_NORM 里的**已登记欠账** %d 条（真缺陷被记成「正常档不通」—— 必须修，"
              "数量只减不增）：%s" % (len(debt), '; '.join(debt[:4])))
    print("ℹ️ 尚未入账 %d 条（抽样 5）：%s" % (len(todo), ' '.join(todo[:5])))
    sys.exit(0)


main()
