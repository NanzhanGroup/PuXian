#!/usr/bin/env python3
# ============================================================
# M220 三轨对拍：**每例一个独立小程序**（上下文忠实 —— 见 gen_cases.py 头注）
# ------------------------------------------------------------
# 判据：三轨的 (rc, R码, 消息体, stdout) 必须**完全一致**；且与 CASES.tsv 的期望一致。
#   归一化：三轨错误**通道前缀**不同（`运行时错误: 错误 [R####] 行:列:` /
#   `运行时错误 [<top> 行N]: R####:` / `运行时错误 [fn 行N]: R####:`）⇒ 只比
#   「R 码 + 剥掉前缀的消息体」（M196/M199/M218 同口径）。
#   ⚠️ 通道本身（stdout vs stderr）是**已登记的缺陷 186 族**，本门不判它；
#      实测结论：三轨错误**都写 stderr** ⇒ 本门判据覆盖得到。
# 用法：three_tracks.py --root <仓库> --work <目录> [--interp <pxi>] [--cbin <pxc>] [--vmb <pxc_vm>]
# ============================================================
import argparse, hashlib, os, re, shutil, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
ap.add_argument('--work', default='/tmp/m220/tt')
ap.add_argument('--cases', default=None, help='gen_cases.py 的输出目录（默认 --work/gen）')
ap.add_argument('--interp', default=None)
ap.add_argument('--cbin', default=None)
ap.add_argument('--vmb', default=None)
ap.add_argument('--timeout', type=int, default=120)
ap.add_argument('--only', default=None, help='只跑这些 label（逗号分隔；负控用，省时间）')
a = ap.parse_args()
ONLY = set(x for x in (a.only or '').split(',') if x)

ROOT = os.path.abspath(a.root)
HERE = os.path.join(ROOT, 'examples', 'm220_result_ops')
W = a.work
os.makedirs(W, exist_ok=True)
GEN = a.cases or os.path.join(W, 'gen')
INTERP = a.interp or os.path.join(ROOT, 'bootstrap', 'pxi')


def run(cmd, env=None):
    p = subprocess.run(cmd, capture_output=True, text=True, timeout=a.timeout,
                       cwd=W, env=env, errors='replace')
    return p.returncode, p.stdout, p.stderr


def norm(txt):
    """取 (R码, 剥掉通道前缀的消息体)。三轨前缀不同 ⇒ 只比语义部分。"""
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


def _sh(p):
    try:
        with open(p, 'rb') as f:
            return hashlib.sha256(f.read()).hexdigest()[:16]
    except OSError:
        return 'MISSING'


env = dict(os.environ)
if a.cbin:
    env['PX_PXC_BIN'] = a.cbin
if a.vmb:
    env['PXC_VM_BIN'] = a.vmb

rows = []
for i, ln in enumerate(open(os.path.join(GEN, 'CASES.tsv'), encoding='utf-8')):
    ln = ln.rstrip('\n')
    if not ln:
        continue
    parts = ln.split('\t')
    if len(parts) != 3:
        print('❌ CASES.tsv 第 %d 行字段数 = %d（要求 3）：%r' % (i + 1, len(parts), ln))
        sys.exit(3)
    rows.append(parts)

print('溯源 interp=%s(%s) vm=%s(%s) c=%s(%s)'
      % (INTERP, _sh(INTERP), os.path.join(W, 'b.vm'), _sh(os.path.join(W, 'b.vm')),
         os.path.join(W, 'b.c'), _sh(os.path.join(W, 'b.c'))))

bad, nok, nran = [], 0, 0
for fn in sorted(os.listdir(os.path.join(GEN, 'cases'))):
    if not fn.endswith('.px'):
        continue
    label = fn[:-3].split('_', 1)[1]
    if ONLY and label not in ONLY:
        continue
    exp = None
    kind = None
    for r in rows:
        if r[0] == label:
            kind, exp = r[1], r[2]
    if exp is None:
        print('❌ 语料 %s 不在 CASES.tsv 里' % label)
        sys.exit(3)
    nran += 1
    src = os.path.join(GEN, 'cases', fn)
    dst = os.path.join(W, fn)
    shutil.copy(src, dst)
    rec = {'label': label, 'kind': kind, 'exp': exp}
    # --- interp ---
    rc, out, err = run([INTERP, dst], env=env)
    rec['interp'] = dict(rc=rc, out=out.strip(), **dict(zip(('code', 'body'), norm(err + '\n' + out))))
    # --- vm / c（各自 build 一次） ---
    for tag, extra, exe in (('vm', [], 'b.vm'), ('c', ['--c'], 'b.c')):
        shutil.rmtree(os.path.join(W, 'build'), ignore_errors=True)
        p = subprocess.run([os.path.join(ROOT, 'tools', 'px'), 'build'] + extra + [dst],
                           capture_output=True, text=True, cwd=W, env=env, errors='replace')
        built = os.path.join(W, 'build', fn[:-3])
        if p.returncode != 0 or not os.path.exists(built):
            rec[tag] = dict(rc='BUILDFAIL', code='BUILDFAIL', body='BUILDFAIL', out='')
            rec[tag + '_log'] = (p.stdout + p.stderr)[-400:]
            continue
        target = os.path.join(W, exe)
        shutil.copy(built, target)
        os.chmod(target, 0o755)
        rc, out, err = run([target], env=env)
        rec[tag] = dict(rc=rc, out=out.strip(), **dict(zip(('code', 'body'), norm(err + '\n' + out))))
    # --- 判据 ---
    sig = {(rec[t]['rc'], rec[t]['code'], rec[t]['body'], rec[t]['out']) for t in ('interp', 'vm', 'c')}
    if len(sig) > 1:
        bad.append(('DIVERGE', label, rec))
        continue
    if kind == 'ok':
        if rec['interp']['rc'] != 0 or rec['interp']['out'] != exp:
            bad.append(('EXPECT', label, rec))
            continue
    else:
        want_code, want_body = (exp.split('|', 1) + [''])[:2]
        if rec['interp']['rc'] == 0 or rec['interp']['code'] != want_code:
            bad.append(('EXPECT', label, rec))
            continue
        if want_body and rec['interp']['body'] != want_body:
            bad.append(('EXPECT', label, rec))
            continue
    nok += 1

print('对拍 %d 例 · 通过 %d · 问题 %d' % (nran, nok, len(bad)))
for why, label, r in bad:
    print('▲ %s %s' % (why, label))
    for t in ('interp', 'vm', 'c'):
        print('    %-6s rc=%-9s %-8s out=[%s] %s'
              % (t, r[t]['rc'], r[t]['code'], r[t]['out'][:30], r[t]['body'][:70]))
    for t in ('vm', 'c'):
        if t + '_log' in r:
            print('    %s build-log: %s' % (t, r[t + '_log'].replace('\n', ' | ')[-200:]))
    print('    期望: %s' % r['exp'][:90])
sys.exit(1 if bad else 0)
