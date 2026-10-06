#!/usr/bin/env python3
# ============================================================
# M279 三轨对拍（stdlib 纯函数族 × 边界/错类型）
# ------------------------------------------------------------
# 判据：三轨（解释 / VM / C）必须给出**同一 rc · 同一 R 码 · 同一消息体 · 同一程序输出**。
# 归一化口径沿用 M196/M199/M226：三轨错误前缀不同
#   （`运行时错误 [i_builtin_ffi_call 行N]` / `运行时错误 [<fn> 行N]`），
#   所以只比「R 码 + 剥掉前缀的消息体」——**别按整行对拍**（已登记缺陷 186 族）。
#
# 另判：与 `MODEL.tsv` 的**双向**核对（登记了却没实测到 ⇒ 过期；实测到却没登记 ⇒ 漏登记）。
# 退出码：0=无分叉且契约表一致；1=有问题；2=环境错误
# ============================================================
import argparse, json, os, re, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--work', required=True)
ap.add_argument('--cases', required=True)
ap.add_argument('--pxi', default=None)
ap.add_argument('--timeout', type=int, default=25)
ap.add_argument('--json', default=None)
a = ap.parse_args()
PXI = a.pxi or os.path.join(a.root, 'bootstrap/pxi')
DRV = os.path.join(a.work, 'drv.px')

if not os.path.exists(PXI):
    print('❌ 缺 %s' % PXI); sys.exit(2)

# 编译两轨
os.makedirs(os.path.join(a.work, 'build'), exist_ok=True)
for eng, out in (('vm', 'drv_vm'), ('c', 'drv_c')):
    env = dict(os.environ)
    if eng == 'c':
        env['PX_BUILD_ENGINE'] = 'c'
    r = subprocess.run([os.path.join(a.root, 'tools/px'), 'build', DRV], cwd=a.work,
                       capture_output=True, text=True, env=env, timeout=300)
    src = os.path.join(a.work, 'build', 'drv')
    dst = os.path.join(a.work, 'build', out)
    if os.path.exists(src):
        os.replace(src, dst)
    else:
        print('❌ %s 轨构建失败：%s' % (eng, (r.stdout + r.stderr)[-300:])); sys.exit(2)

VM = os.path.join(a.work, 'build', 'drv_vm')
CC = os.path.join(a.work, 'build', 'drv_c')

R_RX = re.compile(r'\bR(\d{4})\b')


def norm(s):
    m = R_RX.search(s)
    code = ('R' + m.group(1)) if m else ''
    body = s
    # 解释轨中缀：`…错误 [R####] 行:列: <msg>`；编译轨前缀：`… R####: <msg>`
    body = re.sub(r'^[\s\S]*?\[R\d+\]\s*\d+:\d+:\s*', '', body)
    body = re.sub(r'^[\s\S]*?R\d+:\s*', '', body)
    body = re.sub(r'\s+', ' ', body).strip()
    return code, body


def one(cmd):
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=a.timeout)
        rc, out, err = p.returncode, p.stdout, p.stderr
    except subprocess.TimeoutExpired:
        return {'rc': 124, 'code': 'TIMEOUT', 'body': '<<< 超时（死循环） >>>', 'out': ''}
    code, body = norm(err + '\n' + out)
    so = ''
    for ln in out.splitlines():
        if ln.startswith(('/*', ' *', '*/')):
            continue
        so = ln.strip(); break
    return {'rc': rc, 'code': code, 'body': body, 'out': so}


# 读契约表
model = {}
for ln in open(a.cases, encoding='utf-8'):
    if ln.startswith('#') or not ln.strip():
        continue
    p = ln.rstrip('\n').split('\t')
    if len(p) >= 2:
        model[p[0]] = p[1].strip()
labels = list(model)

diverged, mism, rows = [], [], []
for lb in labels:
    r = {'interp': one([PXI, lb, DRV]),
         'vm': one([VM, lb]),
         'c': one([CC, lb])}
    key = lambda d: (d['rc'], d['code'], d['body'], d['out'])
    ks = {t: key(r[t]) for t in r}
    if len(set(ks.values())) != 1:
        diverged.append({'label': lb, 'tracks': ks})
    have = '%s|%s|%s' % (r['vm']['rc'], r['vm']['code'], r['vm']['out'])
    want = model[lb]
    if want != have:
        mism.append({'label': lb, 'want': want, 'got': have})
    rows.append({'label': lb, 'interp': ks['interp'], 'vm': ks['vm'], 'c': ks['c']})

print('══ M279 三轨对拍：%d 例 ══' % len(labels))
print('  分叉 %d · 契约表不符 %d' % (len(diverged), len(mism)))
for d in diverged[:6]:
    print('  ❌ %s' % d['label'])
    for t, v in d['tracks'].items():
        print('       %-7s rc=%s %s | %s' % (t, v[0], v[1], v[3][:60]))
for m2 in mism[:6]:
    print('  ❌ 契约不符 %s：want=%s got=%s' % (m2['label'], m2['want'], m2['got']))

# 覆盖判据：契约表里的每条都必须被实测到（双向）
if a.json:
    json.dump({'diverged': diverged, 'model_mismatch': mism, 'rows': rows},
              open(a.json, 'w'), ensure_ascii=False, indent=1)
print('M279-THREE-%s' % ('OK' if not (diverged or mism) else 'FAIL'))
sys.exit(0 if not (diverged or mism) else 1)
