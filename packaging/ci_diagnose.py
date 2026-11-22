#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""CI 失败诊断器 v2 —— **编码长度感知** + 「一条注解只放一件事」。

为什么必须重写（M216s2 · **缺陷 299 家族第四次复发**）
------------------------------------------------------------
* `::error::` 的 message 硬上限 = **4096 字符**（GitHub **workflow command** 的限制；
  与 check-run 注解的 64KB 限制**不是一回事**）。
* 本仓日志**大量中文**，而 `urllib.parse.quote()` 会把中文**膨胀 9 倍**
  （`中` → `%E4%B8%AD`）。旧版按「**原始字符数** 3900」裁剪 ⇒ 编码后可达 3.5 万
  ⇒ **必然被 GitHub 从中间截断** ⇒ 排在后面的块**永远看不见**。
  实测（run 36248218474）：注解长度**恰好 4096**，恰好切在「`DBL_MAX` 本」中间，
  而 `M216-VERIFY-OK/FAIL` 这个**最终结论行**正好在被切掉的那几行里 ⇒ 白跑一轮。
* ⇒ 本版两条硬规矩：
    ① 每条注解按「**编码后长度**」二分裁剪（`fit()`，默认预算 3800）；
    ② **一条注解只放一件事**（摘要 / 尾部 / 头部 / 可疑一览）⇒ 互不挤占。
* 判据不变：**最后写入的日志 = 失败门**（其后门根本没跑）。这是**确定性**定位，
  不依赖关键词。

输出：若干行 `::error::<url-encoded>`（GitHub workflow command，每条独立成注解）。

用法：
    ci_diagnose.py                        # 扫 /tmp/v_*.log
    ci_diagnose.py --logs '/tmp/v_*.log' --budget 3800
    ci_diagnose.py --selftest             # 自证（合成 + 真实语料）
"""

import argparse
import glob
import os
import re
import sys
import urllib.parse

# ── 判据 ──────────────────────────────────────────────────────────
# ⚠️ 两条判据**必须并存**：末行同时含「通过」与「失败」时（`══ 通过 165 · 失败 3 ══`）
#   **失败优先** —— 这是 M216s1 记下的真盲区（旧版 OK_LAST 含「通过」⇒ 判成绿）。
OK_LAST = re.compile(
    r'(✅|VERIFY-OK|VERIFY:|ALL-OK|FAIL=0|fail=0|失败\s*0|PASS|通过|一致|EXPECTED'
    r'|SELFTEST-OK|\bdone\b|\d+P/0F/)')
BAD_LAST = re.compile(
    r'(FAIL=[1-9]|FAIL-[A-Z]|[1-9]\s*失败|失败\s*[1-9]|问题\s*[1-9]'
    r'|fail=[1-9]|FAILED|❌|VERIFY-FAIL)')

HARD_CAP = 4096          # GitHub workflow command 的 message 硬上限（实测）
DEFAULT_BUDGET = 3800    # 留 296 字符给 `::error::` 前缀与编码余量

# M432 补2 · 缺陷 895（2026-11-22 CI 实测）：
#   **负控日志不属于「门日志」**。m406/m407 的期望扰动负控把日志写到
#   /tmp/v_m406_nc.log · /tmp/v_m407_nc.log ⇒ 与门日志同命名空间 ⇒
#   ① 诊断器按 mtime 取「最后写入」时会把负控当失败门（真因被顶掉）；
#   ② 本文件自测的「真实语料 ⇒ 0 误报」因负控**按设计必含 ❌** 而假红
#      （CI step 166：`非 ok 2 份 ['v_m406_nc.log','v_m407_nc.log']`）。
#   治本在调用方（负控日志落 /tmp/nc_*.log，已随本补丁改 ci.yml）；
#   这里是**第二道**：即便将来有人又写回 v_*.log，也不会污染语料。
#   ⚠️ 只排除 `_nc.log`（**判决被故意反转**的负控产物：末行就是「失败 N」，没有裁决语义）。
#      不要顺手排除 `_neg.log` —— 那是「开了负控的门日志」，末行是权威裁决行，
#      正是自测 ②d 守的场景（❌ 在前 + `-VERIFY-OK` 在后 ⇒ 必须判 ok）。
SKIP_SUFFIX = ('_nc.log',)

# M432 补2 · 缺陷 895：**受控语料**（判据不许依赖 /tmp 的临时残留）
#   旧的自测拿 /tmp/v_*.log 当「真实语料」要求「0 非 ok」，而 /tmp 里同时躺着**设计上就该红**
#   的日志（负控）与本机残留的失败日志 ⇒ 判据与环境耦合 ⇒ CI 连红两轮。
#   现改为：受控夹具打包进仓库 + 双向覆盖对照（见 check_corpus），/tmp 只报信息不计分。
CORPUS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                          'fixtures', 'ci_diagnose_corpus')


def qlen(s):
    """URL 编码后的长度（这才是 GitHub 看到的大小）。"""
    return len(urllib.parse.quote(s, safe=''))


def fit(s, budget):
    """按**编码后长度**二分裁剪，保证 `qlen(结果) <= budget`。

    ⚠️ 后缀**必须计入预算**（自测第 ⑤ 项当场抓到的 bug：首版先裁主体再贴
    「…（截断）」⇒ 主体刚好用满 3800 时，贴完后缀变 3880 ⇒ 溢出）。
    """
    if qlen(s) <= budget:
        return s
    suffix = '\n…（按编码长度截断）'
    room = budget - qlen(suffix)
    if room <= 0:
        return suffix.lstrip()
    lo, hi = 0, len(s)
    while lo < hi:
        mid = (lo + hi + 1) // 2
        if qlen(s[:mid]) <= room:
            lo = mid
        else:
            hi = mid - 1
    return s[:lo] + suffix


VERDICT_SCAN = 8   # M225s1：裁决窗口（行）


def verdict(x):
    """裁决：fail / ok / unknown / empty。

    M225s1：入参可为 str（单行）或 list（多行）。**只看最后一行是脆的** ——
      门的 `trap ... EXIT` 会在脚本收尾后追加提示（还原/清理），把
      `...-VERIFY-OK` 顶掉 ⇒ 绿门被判 unknown ⇒ 本门「真实语料 0 误报」判红。
      （实测：m225 门末行 `↩︎ 源码已逐字节还原` ⇒ CI step 13 红，
       而 job 日志 403 ⇒ 真因读不出 —— 属「判据自己制造假红」。）
      现改为**向下扫最后 VERDICT_SCAN 行**：fail 优先，其次 ok，都没有才 unknown。
    """
    lines = [x] if isinstance(x, str) else list(x)
    lines = [l for l in lines if l.strip()]
    if not lines:
        return 'empty'
    # 从下往上找**第一个「裁决行」**（自带 ok/fail 判据的行）即返回 ——
    #   不许"扫窗口内任何 ❌"：门日志里 `❌` 常出现在**负控/自证**的预期失败输出里，
    #   放大窗口去扫会把这些**绿门**判成 fail（M225s1 实测：5 份绿门日志被误判）。
    for l in reversed(lines[-VERDICT_SCAN:]):
        bad = BAD_LAST.search(l)
        ok = OK_LAST.search(l)
        if bad and ok:
            return 'fail'        # 同行兼具「通过/✅」与「失败 N」⇒ 失败优先（M216s1 真盲区）
        if bad:
            return 'fail'
        if ok:
            return 'ok'
    return 'unknown'


def collect(pat):
    """采集门日志。

    M225s2：**显式排除本诊断器自己的日志** —— 诊断器的输出不属于「门的日志」，
      而它一旦落进扫描面就会**自我包含**（扫到正在写的自己 ⇒ 判 unknown ⇒ 非 ok
      ⇒ 必然假红；CI 实测 `非 ok 1 份 ['v_ci_diagnose.log']`）。
      治本在调用方（守卫日志落 /tmp/guard_*.log），这里是**第二道**：即使将来
      有人把它的日志写回 v_*.log，也不会把判据带进自指。
    """
    infos = []
    skip_self = ('ci_diagnose',)
    for f in sorted(glob.glob(pat)):
        if any(k in os.path.basename(f) for k in skip_self):
            continue
        if os.path.basename(f).endswith(SKIP_SUFFIX):   # 缺陷 895：负控不是门日志
            continue
        try:
            t = open(f, encoding='utf-8', errors='replace').read()
        except Exception:
            continue
        lines = [l for l in t.splitlines() if l.strip()]
        last = lines[-1] if lines else ''
        infos.append(dict(path=f, name=os.path.basename(f),
                          mtime=os.path.getmtime(f), text=t, lines=lines,
                          last=last, v=verdict(lines)))
    infos.sort(key=lambda x: x['mtime'])
    return infos


def check_corpus(d, tsv=None):
    """受控语料对照 ⇒ (问题列表, 命中份数, 期望行数, 被排除负控数)。

    M432 补2 · 缺陷 895 的「0 误报」**可判定版**（替代「拿 /tmp 当语料」）：
      · 目录里的门日志集合 ⇄ expect.tsv 的键集合**双向相等**
        （期望表登记了但目录里没有 = 覆盖不全；目录里有但没登记 = 语料漂移）
      · 每一份的 `verdict()` 必须等于期望值（ok / fail / unknown）
      · 一份都没有（全被当负控排除）⇒ 问题 —— **空集不许判绿**
      · 受控语料里不许出现负控产物（`*_nc.log` 应改名到 /tmp/nc_*.log）
    """
    tsv = tsv or os.path.join(d, 'expect.tsv')
    expect = {}
    for ln in open(tsv, encoding='utf-8'):
        ln = ln.rstrip('\n')
        if not ln.strip() or ln.lstrip().startswith('#'):
            continue
        parts = ln.split('\t')
        if len(parts) < 2:
            return (['期望表格式错（缺 TAB）：%r' % ln], 0, len(expect), 0)
        expect[parts[0].strip()] = parts[1].strip()
    names = [os.path.basename(f)
             for f in sorted(glob.glob(os.path.join(d, '*.log')))]
    skipped = [n for n in names if n.endswith(SKIP_SUFFIX)]
    infos = collect(os.path.join(d, '*.log'))
    got = {i['name']: i['v'] for i in infos}
    probs = []
    if not infos:
        probs.append('语料为空（全被当负控排除？）—— 空集不许判绿')
    if skipped:
        probs.append('受控语料含负控产物（应改名 /tmp/nc_*.log）：%s' % skipped[:5])
    for n in sorted(set(expect) - set(got)):
        probs.append('期望表登记了但目录里没有：%s' % n)
    for n in sorted(set(got) - set(expect)):
        probs.append('目录里有但期望表没登记（语料漂移）：%s' % n)
    for n in sorted(set(expect) & set(got)):
        if expect[n] != got[n]:
            probs.append('%s 期望 %s 实测 %s' % (n, expect[n], got[n]))
    return (probs, len(infos), len(expect), len(skipped))


def build_annotations(infos, budget):
    """返回 [(text, budget), ...] —— 每条独立成注解。"""
    anns = []
    if not infos:
        anns.append(('★★ 没有 /tmp/v_*.log ⇒ 失败**不在门内**'
                     '（查 step 里非门的命令：python3 -c / grep / echo）', budget))
        return anns

    lastlog = infos[-1]
    susp = [i for i in infos if i['v'] != 'ok']
    hard = [l for l in lastlog['lines']
            if ('❌' in l or 'FAIL' in l or '失败 ' in l or 'Traceback' in l
                or 'MISMATCH' in l)]

    # ── A 摘要（最短最要紧：门名 + 末行 + 判定 + ❌ 行）──
    L = ['★★ 失败门定位 v2（编码长度感知）',
         '最后写入: %s | 判定: %s' % (lastlog['name'], lastlog['v']),
         '末行: %s' % lastlog['last'][:140],
         '该日志命中 ❌/FAIL 的行 %d 条:' % len(hard)]
    for b in hard[:5]:
        L.append('  ' + b.strip()[:170])
    if susp:
        L.append('其它可疑门 %d 个（末行无成功标记）: %s'
                 % (len(susp), ', '.join(i['name'] for i in susp[-6:])))
    anns.append(('\n'.join(L), min(budget, 2400)))

    # ── B 尾部（结论行在这里）──
    anns.append(('★★ %s 末 20 行\n%s'
                 % (lastlog['name'], '\n'.join(lastlog['lines'][-20:])), budget))

    # ── C 头部（失败常在开头；旧版只有尾部 ⇒ 天然后天失明）──
    anns.append(('★★ %s 头 20 行（失败常在开头）\n%s'
                 % (lastlog['name'], '\n'.join(lastlog['lines'][:20])), budget))

    # ── D 可疑门一览（每个只有一行末行 ⇒ 极短，信息密度最高）──
    if susp:
        L = ['★ 可疑门末行一览（%d 个）' % len(susp)]
        for i in susp[-10:]:
            L.append('%s: %s' % (i['name'], i['last'][:120]))
        anns.append(('\n'.join(L), min(budget, 3000)))
    # ⚠️ **裁剪必须在 build 内完成**：emit 只负责 quote。
    #   自测之所以能抓住旧版 bug，正是因为断言直接看这里的返回值 ——
    #   若把 fit 留在 emit，「编码后超限」这条判据就测不到（测的是未裁的原串）。
    # ── E 其它失败门的诊断（M237s3 · 缺陷 377）──
    #   旧版只对「最后写入」的日志给头/尾，其余失败门**只有末行** —— 而末行往往就是
    #   `XXX-VERIFY-FAIL`（**指不到根因**）。CI 实测（M237）：m236 与 m237 同时失败，
    #   而 m237 恰好是「最后写入」且恰好是绿的 ⇒ m236 的根因**完全不可见**。
    others = [i for i in susp if i['name'] != lastlog['name']]
    if others:
        L = ['★★ 其它失败门诊断（%d 个 · 各取 ❌/FAIL 行，无则取末 6 行）' % len(others)]
        for i in others[:4]:
            hs = [l for l in i['lines']
                  if ('❌' in l or 'FAIL' in l or '失败 ' in l or 'Traceback' in l
                      or 'MISMATCH' in l or 'error' in l.lower())]
            L.append('── %s（判定=%s）末行：%s' % (i['name'], i['v'], i['last'][:90]))
            if hs:
                for b in hs[:6]:
                    L.append('   ' + b.strip()[:170])
            else:
                for b in i['lines'][-6:]:
                    L.append('   ' + b.strip()[:170])
        if len(others) > 4:
            L.append('（还有 %d 个未列）' % (len(others) - 4))
        anns.append(('\n'.join(L), budget))
    return [(fit(t, b), b) for t, b in anns]


def m187_repro(ws, budget):
    """m187（registry 引入门）的**现场复现** —— 从 ci.yml 内联搬来（M188/M195 的原逻辑）。

    ⚠️ 搬运时的**唯一改动**：末尾不再 `[:900]` 硬切（那会切在 UTF-8 中间），
    改为统一走 `fit()` 的编码长度预算。其余步骤顺序、命令、cwd、env **逐字保留**。
    """
    import subprocess
    env = dict(os.environ)
    env['PX_REGISTRY'] = os.path.join(ws, 'registry')
    env['LC_ALL'] = 'C'
    rep = '/tmp/pkgrepro'
    subprocess.run(['rm', '-rf', rep])
    os.makedirs(rep, exist_ok=True)
    px = os.path.join(ws, 'tools', 'pxpkg')
    pxpkgpx = os.path.join(ws, 'tools', 'pxpkg.px')
    pxi = os.path.join(ws, 'bootstrap', 'pxi')

    def run(cmd, cwd=rep, e=env, t=300):
        try:
            p = subprocess.run(cmd, cwd=cwd, env=e, capture_output=True,
                               text=True, timeout=t)
            return 'rc=%d\n%s' % (p.returncode, (p.stdout + p.stderr)[-900:])
        except Exception as ex:
            return '异常: %s' % ex

    out = []
    out.append('which pxpkg=%s exists=%s' % (px, os.path.exists(px)))
    out.append('pxpkg.px exists=%s  pxi exists=%s'
               % (os.path.exists(pxpkgpx), os.path.exists(pxi)))
    out.append('PX_REGISTRY=%s' % env['PX_REGISTRY'])
    out.append('ls registry 前 5:\n' + run(['ls', os.path.join(ws, 'registry')], cwd=ws))
    out.append('ls registry/base58/0.1.0:\n'
               + run(['ls', '-la', os.path.join(ws, 'registry', 'base58', '0.1.0')], cwd=ws))
    out.append('init:\n' + run([px, 'init', '--name', 'repro']))
    out.append('px.toml:\n' + run(['cat', 'px.toml']))
    out.append('add base58@0.1.0:\n' + run([px, 'add', 'base58@0.1.0']))
    out.append('px.toml after add:\n' + run(['cat', 'px.toml']))
    out.append('install:\n' + run([px, 'install']))
    out.append('ls -la .px_modules:\n' + run(['ls', '-la', '.px_modules']))
    out.append('直接跑 pxpkg.px（绕开包装脚本）:\n'
               + run([pxi, 'run', pxpkgpx],
                     e=dict(env, PXPKG_DIR='.', PXPKG_CMD='list',
                            PXPKG_ARG1='', PXPKG_ARG2='')))
    return ('=== ① CI 现场复现（m187） ===\n' + '\n'.join(out), budget)


def extra_annotations(pattern, budget, tail=30):
    """额外的门内诊断文件（`/tmp/m187_gate*/diag.txt` / `install.err` / `*.log`）。"""
    anns = []
    files = []
    for d in sorted(glob.glob(pattern)):
        for n in ('diag.txt', 'install.err', 'install.log'):
            p = os.path.join(d, n)
            if os.path.exists(p):
                files.append(p)
        files.extend(sorted(glob.glob(os.path.join(d, '*.log'))))
    if not files:
        return anns
    L = ['=== ② 门内日志（m187 现场） ===']
    for f in files[:6]:
        try:
            txt = open(f, encoding='utf-8', errors='replace').read().strip()
        except Exception:
            continue
        if txt:
            L.append('--- %s（尾 %d 行）---' % (f, tail))
            L.append('\n'.join(txt.splitlines()[-tail:]))
    anns.append((fit('\n'.join(L), budget), budget))
    return anns


def emit(anns):
    for text, _budget in anns:
        print('::error::' + urllib.parse.quote(text, safe=''))


# ── 自证 ──────────────────────────────────────────────────────────
def _mk(tmp, name, body):
    p = os.path.join(tmp, name)
    with open(p, 'w', encoding='utf-8') as f:
        f.write(body)
    return p


def selftest():
    import shutil
    import tempfile
    fails = []

    def chk(cond, what):
        print(('  ✅ ' if cond else '  ❌ ') + what)
        if not cond:
            fails.append(what)

    tmp = tempfile.mkdtemp(prefix='cidg_')
    try:
        # ① 全绿语料 ⇒ 0 误报
        _mk(tmp, 'v_a.log', '── 层1\n   ✅ 全过\nM1-VERIFY-OK\n')
        _mk(tmp, 'v_b.log', '── 层1\n══ 通过 12 · 失败 0 ══\n')
        _mk(tmp, 'v_c.log', '── 层1\nALL-OK\n')
        ins = collect(os.path.join(tmp, 'v_*.log'))
        chk(all(i['v'] == 'ok' for i in ins), '全绿语料 3 份 ⇒ 全部判 ok（0 误报）')

        # ② 真盲区：「通过 N · 失败 3」必须判 fail（旧版含「通过」⇒ 判绿）
        chk(verdict('══ 通过 165 · 失败 3 ══') == 'fail',
            '末行「通过 165 · 失败 3」⇒ 判 fail（M216s1 的真盲区）')

        # ②b M225s1：**末行之后还有 trap 尾巴** ⇒ 仍须判 ok
        #    （旧版只看最后一行 ⇒ unknown ⇒ 非 ok ⇒ 假红；本机实测 m225 门即此形态）
        import time
        time.sleep(0.01)
        _mk(tmp, 'v_tail.log', '── [1/3] 语料\n   ✅ 全过\n'
                               '===== 失败 0 项 · 通过 11 项 =====\nM225-VERIFY-OK\n'
                               '  ↩︎ 源码已逐字节还原\n')
        ins_t = collect(os.path.join(tmp, 'v_tail.log'))
        chk(ins_t and ins_t[0]['v'] == 'ok',
            '末行是 trap 尾巴 ⇒ 仍判 ok（M225s1：旧版判 unknown ⇒ 绿门假红）')

        # ②c M225s1：**真失败 + 尾巴** ⇒ 仍须判 fail（加固不能放水）
        time.sleep(0.01)
        _mk(tmp, 'v_tail2.log', '── [2/3]\n   ❌ 某断言失败\n'
                                '===== 失败 1 项 · 通过 10 项 =====\nM225-VERIFY-FAIL\n'
                                '  ↩︎ 源码已逐字节还原\n')
        ins_t2 = collect(os.path.join(tmp, 'v_tail2.log'))
        chk(ins_t2 and ins_t2[0]['v'] == 'fail',
            '真失败 + 尾巴 ⇒ 仍判 fail（窗口扫描不放水）')

        # ②d M225s1：**❌ 在前（负控的预期输出）但裁决行是 OK** ⇒ 判 ok
        #    放大窗口去"扫任何 ❌"会把绿门误判成 fail
        #    （本机实测：5 份绿门日志被这个坑误判 —— 判据不能只看"窗口里有没有 ❌"）
        time.sleep(0.01)
        _mk(tmp, 'v_neg.log',
            '── [4/4] 负控 A\n     · 第 4 次（grace）→ [GRACE timeout]\n'
            '   ❌ 缺陷复现（成功 3/6 < 6）\n'
            '===== 失败 0 项 · 通过 15 项 =====\nM225-VERIFY-OK\n'
            '  ↩︎ 源码已逐字节还原\n')
        ins_n = collect(os.path.join(tmp, 'v_neg.log'))
        chk(ins_n and ins_n[0]['v'] == 'ok',
            '负控 ❌ 在前 + 裁决行在后 ⇒ 判 ok（窗口内扫 ❌ 会误判绿门）')

        # ②e M432 补2（缺陷 895 · 2026-11-22 CI step 166 实测）：
        #   **判决被故意反转的负控产物**（`*_nc.log`，末行就是「失败 N」）不许进语料 ——
        #   否则 ①它会以「最后写入」冒充失败门（真因被顶掉）②本门「真实语料 0 误报」假红。
        #   （CI 实测 `非 ok 2 份 ['v_m406_nc.log','v_m407_nc.log']`）
        _mk(tmp, 'v_x_nc.log', '  ✅ N1 负控：期望扰动应判红\n'
                               '  ❌ 期望扰动后门仍判绿（判据无鉴别力）\n== FAIL 1 ==\n')
        ins_nc = collect(os.path.join(tmp, 'v_*.log'))
        chk(all(not i['name'].endswith('_nc.log') for i in ins_nc),
            '负控产物 *_nc.log ⇒ 不进语料（缺陷 895）')

        # ③ 末行 VERIFY-FAIL 必须判 fail 且**点名**
        import time
        time.sleep(0.01)
        _mk(tmp, 'v_d.log', '── [1/3] 语料\n   ❌ 语料规模低于下限\n── [3/3]\nM9-VERIFY-FAIL\n')
        ins = collect(os.path.join(tmp, 'v_*.log'))
        last = ins[-1]
        chk(last['name'] == 'v_d.log' and last['v'] == 'fail',
            '失败门被识别为「最后写入」且判 fail')
        anns = build_annotations(ins, DEFAULT_BUDGET)
        head = anns[0][0]
        chk('v_d.log' in head and 'VERIFY-FAIL' in head and '❌' in head,
            '摘要注解点名门名 + 末行 + ❌ 行')

        # ④ **编码长度**硬约束（这是本轮的核心 bug）
        # ④b M237s3（缺陷 377）：**两个失败门** ⇒ 两个都要在注解里可见
        #    （旧版只解释「最后写入」的那个 ⇒ 另一个只有末行 ⇒ 根因不可见）
        time.sleep(0.01)
        _mk(tmp, 'v_f.log', '── [1/3] 甲门\n   ❌ 甲门真因：端口被占\nM6-VERIFY-FAIL\n')
        time.sleep(0.01)
        _mk(tmp, 'v_g.log', '── [1/3] 乙门\n   ✅ 全过\nM7-VERIFY-OK\n')
        ins2 = collect(os.path.join(tmp, 'v_*.log'))
        anns2 = build_annotations(ins2, DEFAULT_BUDGET)
        blob2 = '\n'.join(t for t, _ in anns2)
        chk('甲门真因：端口被占' in blob2,
            '两个失败门 ⇒ 非最后写入的那个也给出 ❌ 行（缺陷 377）')

        chk(all(qlen(t) <= b for t, b in anns),
            '每条注解的**编码后**长度 ≤ 预算（旧版按原始字符裁 ⇒ 溢出 9 倍）')
        _mk(tmp, 'v_e.log', ('中文超长行 ' * 4000) + '\nM1-VERIFY-OK\n')
        ins = collect(os.path.join(tmp, 'v_*.log'))
        anns = build_annotations(ins, DEFAULT_BUDGET)
        chk(all(qlen(t) <= DEFAULT_BUDGET for t, b in anns),
            '超长中文日志 ⇒ 每条仍 ≤ 预算（二分裁剪生效）')
        chk(all(qlen(t) + len('::error::') <= HARD_CAP for t, b in anns),
            '每条 `::error::` + 编码后 ≤ 4096（GitHub 硬上限）')

        # ⑤ 一条注解一件事 ⇒ 注解条数 ≥ 3（摘要 / 尾 / 头）
        chk(len(anns) >= 3, '注解拆成 ≥3 条（互不挤占）⇒ 实测 %d 条' % len(anns))

        # ⑥ 受控语料（M432 补2 · 缺陷 895）：判据**不许依赖 /tmp 的临时残留**。
        #   旧写法拿 /tmp/v_*.log 当「真实语料」要求 0 非 ok —— 而 /tmp 里同时躺着
        #   设计上就该红的负控日志（m406/m407）与本机残留的失败日志 ⇒ 判据自相矛盾
        #   ⇒ CI step「发布侧守卫自测」连红两轮（270 份语料里 2 份非 ok）。
        probs, nhit, nexp, nskip = check_corpus(CORPUS_DIR)
        print('  · 受控语料 %d 份 / 期望表 %d 行 / 负控排除 %d' % (nhit, nexp, nskip))
        for p in probs:
            print('      ✗ ' + p)
        chk(not probs, '受控语料 ⇄ 期望表双向相等且逐份裁决相符（缺陷 895）')

        # ⑥b 反控：判据自己有没有牙（少了这些，「改判据把红改绿」就无从发现）
        src = os.path.join(CORPUS_DIR, 'expect.tsv')
        bd = tempfile.mkdtemp(prefix='cidg_neg_', dir=tmp)
        for f in glob.glob(os.path.join(CORPUS_DIR, '*.log')):
            shutil.copy(f, bd)
        base = open(src, encoding='utf-8').read()
        #   b1 期望被改错（fail 那份写成 ok）⇒ 必报问题
        open(os.path.join(bd, 'expect.tsv'), 'w', encoding='utf-8').write(
            base.replace('fail_mixed_line.log\tfail', 'fail_mixed_line.log\tok'))
        p1 = check_corpus(bd)[0]
        chk(bool(p1), '反控 b1：期望值被改错 ⇒ 必报问题（判据有牙）')
        #   b2 期望表漏登记一份 ⇒ 必报「语料漂移」
        open(os.path.join(bd, 'expect.tsv'), 'w', encoding='utf-8').write(
            '\n'.join([l for l in base.splitlines()
                       if not l.startswith('unknown_no_verdict.log')]) + '\n')
        p2 = check_corpus(bd)[0]
        chk(any('语料漂移' in x for x in p2),
            '反控 b2：期望表漏登记一份 ⇒ 必报语料漂移（覆盖不全不许过）')
        #   b3 只含负控产物 ⇒ 必须「排除 N 份」且判「空集不许判绿」
        bd3 = tempfile.mkdtemp(prefix='cidg_nc_', dir=tmp)
        open(os.path.join(bd3, 'v_x_nc.log'), 'w', encoding='utf-8').write(
            '   ❌ 期望扰动后门仍判绿\n== FAIL 1 ==\n')
        open(os.path.join(bd3, 'expect.tsv'), 'w', encoding='utf-8').write('')
        p3, _, _, s3 = check_corpus(bd3)
        chk(bool(p3) and s3 == 1,
            '反控 b3：只含负控 ⇒ 排除 1 份且判「空集不许判绿」')

        # ⑥c /tmp 真实语料：**只报信息、不计分** —— 环境残留不该决定判据生死。
        real_all = sorted(glob.glob('/tmp/v_*.log'))
        negs = [f for f in real_all if os.path.basename(f).endswith(SKIP_SUFFIX)]
        real = [f for f in real_all if not os.path.basename(f).endswith(SKIP_SUFFIX)]
        if real:
            rins = collect('/tmp/v_*.log')
            nbad = [i['name'] for i in rins if i['v'] != 'ok']
            print('  · 信息性：本机 /tmp 语料 %d 份 ⇒ 非 ok %d 份 %s（另排除负控 %d 份）'
                  % (len(rins), len(nbad), nbad[:5], len(negs)))
        else:
            print('  · 信息性：本机 /tmp 无语料（不影响判据）')
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    print()
    print('CI-DIAGNOSE-SELFTEST-OK' if not fails
          else 'CI-DIAGNOSE-SELFTEST-FAIL（%d 项）' % len(fails))
    return 1 if fails else 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--logs', default='/tmp/v_*.log')
    ap.add_argument('--budget', type=int, default=DEFAULT_BUDGET)
    ap.add_argument('--extra', default='', help='额外门内日志目录 glob（如 /tmp/m187_gate*）')
    ap.add_argument('--m187-repro', action='store_true',
                    help='跑 m187（registry 引入门）的现场复现，作为一条注解')
    ap.add_argument('--workspace', default=os.environ.get('GITHUB_WORKSPACE', os.getcwd()))
    ap.add_argument('--selftest', action='store_true')
    a = ap.parse_args()
    if a.selftest:
        sys.exit(selftest())
    anns = build_annotations(collect(a.logs), a.budget)
    if a.extra:
        anns += extra_annotations(a.extra, a.budget)
    if a.m187_repro:
        anns.append(m187_repro(a.workspace, a.budget))
    emit(anns)
    return 0


if __name__ == '__main__':
    sys.exit(main())
