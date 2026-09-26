#!/usr/bin/env python3
# ============================================================
# M216 三轨对拍：驱动器**一次编译**，逐 label 只跑不编（M199 教训）
# ------------------------------------------------------------
# 判据：三轨的 (rc, R码, 消息体, stdout) 必须**完全一致**；且与 CASES.tsv 的期望值一致。
#   归一化：三轨错误前缀不同（`运行时错误: 错误 [R####] 行:列:` / `运行时错误 [<top> 行N]: R####:`
#   / `运行时错误 [行N]: R####:`）⇒ 只比「R 码 + 剥掉前缀的消息体」（M196/M199 同口径）。
# 用法：three_tracks.py --root <仓库> --work <目录> [--interp <pxi>] [--cbin <pxc>] [--vmb <pxc_vm>]
# ============================================================
import argparse, json, os, re, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
ap.add_argument('--work', default='/tmp/m216/tt')
ap.add_argument('--interp', default=None)
ap.add_argument('--cbin', default=None)
ap.add_argument('--vmb', default=None)
ap.add_argument('--skip-build', action='store_true')
ap.add_argument('--timeout', type=int, default=60)
a = ap.parse_args()

ROOT = os.path.abspath(a.root)
HERE = os.path.join(ROOT, 'examples', 'm217_shift_count')
W = a.work
os.makedirs(W, exist_ok=True)

INTERP = a.interp or os.path.join(ROOT, 'bootstrap', 'pxi')
CSRC = os.path.join(HERE, 'cases.px')
CAND = os.path.join(W, 'cases.px')
shutil_ok = True


def run(cmd, cwd=None):
    p = subprocess.run(cmd, capture_output=True, text=True, timeout=a.timeout,
                       cwd=cwd or W, errors='replace')
    return p.returncode, p.stdout, p.stderr


def norm(txt):
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


# ---- 布场：只拷源码，产物落 W ----
import shutil
shutil.copy(CSRC, CAND)

env = dict(os.environ)
if a.cbin:
    env['PX_PXC_BIN'] = a.cbin
if a.vmb:
    env['PXC_VM_BIN'] = a.vmb

if not a.skip_build:
    for tag, extra in (('vm', []), ('c', ['--c'])):
        out_exe = os.path.join(W, 'cases.' + tag)
        if os.path.exists(out_exe):
            os.remove(out_exe)
        p = subprocess.run([os.path.join(ROOT, 'tools', 'px'), 'build'] + extra + [CAND],
                           capture_output=True, text=True, cwd=W, env=env, errors='replace')
        built = os.path.join(W, 'build', 'cases')
        if p.returncode != 0 or not os.path.exists(built):
            print('❌ %s 轨构建失败 rc=%d' % (tag, p.returncode))
            print((p.stdout + p.stderr)[-800:])
            sys.exit(2)
        shutil.move(built, out_exe)

cases = []
for ln in open(os.path.join(HERE, 'CASES.tsv')):
    ln = ln.strip()
    if not ln:
        continue
    parts = ln.split('\t')
    if len(parts) == 3:
        # M217：期望里的 `\n` 是**转义后的换行**（TSV 必须单行）
        parts[2] = parts[2].replace('\\n', '\n')
    cases.append(parts)

# 溯源行（M216 教训）：门红了要能**当场读出**用的是哪几个件 ——
#   「devbuild 给了名字就只编那些」曾让 /tmp/pxcdev 保持上一版而门看不出原因。
import hashlib


def _sh(p):
    try:
        with open(p, 'rb') as f:
            return hashlib.sha256(f.read()).hexdigest()[:16]
    except OSError:
        return 'MISSING'


print('溯源 interp=%s(%s) vm=%s(%s) c=%s(%s)'
      % (INTERP, _sh(INTERP), os.path.join(W, 'cases.vm'), _sh(os.path.join(W, 'cases.vm')),
         os.path.join(W, 'cases.c'), _sh(os.path.join(W, 'cases.c'))))

bad, nok = [], 0
for label, kind, exp in cases:
    rec = {'label': label, 'kind': kind, 'exp': exp}
    cmds = {
        'interp': [INTERP, label, CAND],
        'vm': [os.path.join(W, 'cases.vm'), label],
        'c': [os.path.join(W, 'cases.c'), label],
    }
    for name, cmd in cmds.items():
        if not os.path.exists(cmd[0]):
            print('❌ 缺少 %s 件：%s' % (name, cmd[0]))
            sys.exit(2)
        rc, out, err = run(cmd)
        code, body = norm(err + '\n' + out)
        # M217：期望可能是**多行**（扫描类语料 64 行输出）⇒ 判据比**完整 stdout**，
        #   同时留一份首行（out1）供失败时紧凑显示。
        rec[name] = {'rc': rc, 'code': code, 'body': body,
                     'out': out.strip(),
                     'out1': (out.strip().splitlines() or [''])[0].strip()}
    sig = {(rec[t]['rc'], rec[t]['code'], rec[t]['body'], rec[t]['out'])
           for t in ('interp', 'vm', 'c')}
    if len(sig) > 1:
        bad.append(('DIVERGE', label, rec))
        continue
    if kind == 'ok':
        got = rec['interp']['out']
        if rec['interp']['rc'] != 0 or got != exp:
            bad.append(('EXPECT', label, rec))
            continue
    else:
        want_code, want_body = (exp.split('|', 1) + [''])[:2]
        if rec['interp']['rc'] == 0:
            bad.append(('EXPECT', label, rec))
            continue
        if rec['interp']['code'] != want_code:
            bad.append(('EXPECT', label, rec))
            continue
        if want_body and rec['interp']['body'] != want_body:
            bad.append(('EXPECT', label, rec))
            continue
    nok += 1

json.dump(cases, open(os.path.join(W, 'cases.json'), 'w'), ensure_ascii=False)
print('对拍 %d 例 · 通过 %d · 问题 %d' % (len(cases), nok, len(bad)))
for why, label, r in bad:
    print('▲ %s %s' % (why, label))
    for t in ('interp', 'vm', 'c'):
        print('    %-6s rc=%-3s %-6s out=[%s] %s'
              % (t, r[t]['rc'], r[t]['code'], r[t]['out1'][:40], r[t]['body'][:70]))
    print('    期望: %s' % r['exp'][:90])
sys.exit(1 if bad else 0)
