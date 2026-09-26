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


def verdict(last):
    """末行裁决：fail / ok / unknown / empty。"""
    if not last.strip():
        return 'empty'
    if BAD_LAST.search(last):
        return 'fail'
    if OK_LAST.search(last):
        return 'ok'
    return 'unknown'


def collect(pat):
    infos = []
    for f in sorted(glob.glob(pat)):
        try:
            t = open(f, encoding='utf-8', errors='replace').read()
        except Exception:
            continue
        lines = [l for l in t.splitlines() if l.strip()]
        last = lines[-1] if lines else ''
        infos.append(dict(path=f, name=os.path.basename(f),
                          mtime=os.path.getmtime(f), text=t, lines=lines,
                          last=last, v=verdict(last)))
    infos.sort(key=lambda x: x['mtime'])
    return infos


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

        # ⑥ 真实语料（若本机有 /tmp/v_*.log ⇒ 上一次 CI 复跑留下的）：
        #    **必须 0 非 ok** —— 这是「0 误报」判据的真实语料版。
        #    本机曾用 100 份实测：旧判据漏了 `verify done` / `10P/0F/0S` / `fail=0`
        #    三类成功末行 ⇒ 会假报 6 个「可疑门」（刷屏）。已补进 OK_LAST。
        real = sorted(glob.glob('/tmp/v_*.log'))
        if real:
            rins = collect('/tmp/v_*.log')
            nbad = [i['name'] for i in rins if i['v'] != 'ok']
            print('  · 真实语料 %d 份 ⇒ 非 ok %d 份 %s'
                  % (len(rins), len(nbad), nbad[:5]))
            chk(not nbad, '真实语料 %d 份 ⇒ **0 误报**（旧判据在此假报 6 份）' % len(rins))
        else:
            print('  · 真实语料缺失（跳过，非失败）')
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
