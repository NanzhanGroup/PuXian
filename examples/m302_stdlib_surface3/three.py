#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M302 三轨对拍器：stdlib 第二批（逐位置 × 错类型毒值 ∪ 错 arity ∪ 合法侧）
# ------------------------------------------------------------
# 判据四层（每层独立有牙）：
#   [A] 三轨一致：解释 / VM / C 必须给出**同一 rc · 同一 R 码 · 同一消息体 · 同一程序输出**
#       （归一化口径沿用 M196/M199/M226/M279 —— 三轨错误前缀不同，只比 R 码 + 消息体）
#   [B] 合法侧**独立期望值**：`legal` 用例的程序输出必须逐字节等于期望（不由三轨自证）
#   [C] **确定性**：同一二进制跑两遍，原始 rc+stdout+stderr 必须逐字节相同
#       （M233 建立的「读指针位当值」这类 UB 的指纹判据）
#   [D] **契约表双向**：与 MODEL.tsv 比 —— 表里有实测没有 ⇒ 过期；实测有表里没有 ⇒ 漏登记
# 超时 = 死循环 ⇒ 判红（M279）
# 退出码：0=全绿；1=有问题；2=环境错误
# ============================================================
import argparse, json, os, re, subprocess, sys

# ── 门内并行（M244）：2250 次 spawn **彼此无依赖**（同一驱动器 · 不同选择 · 不写共享文件）
#    ⇒ 用 selfhost/gate_par.py 的 pmap 吃满核数（threads + subprocess ⇒ 真并行）。
#    ⚠️ 与「门**之间**禁止并行」正交：那条护栏是为「负控改源码」立的。
_GP = None

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--work', required=True)
ap.add_argument('--cases', required=True)
ap.add_argument('--pxi', default=None)
ap.add_argument('--model', default=None)      # 既有契约表（比对）
ap.add_argument('--model-out', default=None)  # 实测生成的契约表
ap.add_argument('--json', default=None)
ap.add_argument('--timeout', type=int, default=20)
ap.add_argument('--runs', type=int, default=2)   # 确定性重复次数
ap.add_argument('--only', default=None)          # 只跑 label 匹配该正则的用例（负控用：把运行压到秒级）
ap.add_argument('--need', type=int, default=0)   # 至少跑多少条（防 --only 写错 ⇒ 静默跑 0 条）
ap.add_argument('--fork-allow', default=None)    # **已登记分叉**表（双向）：真缺陷不许记成 OK，
                                                 #   本表记的是「分叉**存在**」= 缺陷；修好后本表过期 ⇒ 判红
a = ap.parse_args()
sys.path.insert(0, os.path.join(a.root, 'selfhost'))
try:
    from gate_par import pmap, jobs      # noqa: E402
    _GP = 'gate_par'
except Exception as _e:                  # 拿不到就**响亮记一行**并串行（不静默假绿）
    pmap = None
    print('ℹ️ 未能加载 gate_par（%s）⇒ 串行执行' % _e, file=sys.stderr)
PXI = a.pxi or os.path.join(a.root, 'bootstrap/pxi')
DRV = os.path.join(a.work, 'drv.px')
R_RX = re.compile(r'\bR(\d{4})\b')


def norm(s):
    m = R_RX.search(s)
    code = ('R' + m.group(1)) if m else ''
    body = re.sub(r'^[\s\S]*?\[R\d+\]\s*\d+:\d+:\s*', '', s)
    body = re.sub(r'^[\s\S]*?R\d+:\s*', '', body)
    body = re.sub(r'\s+', ' ', body).strip()
    return code, body


def one(cmd):
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=a.timeout)
        rc, out, err = p.returncode, p.stdout, p.stderr
    except subprocess.TimeoutExpired:
        return {'rc': 124, 'code': 'TIMEOUT', 'body': '<<< 超时（死循环） >>>', 'out': '',
                'raw': 'TIMEOUT'}
    code, body = norm(err + '\n' + out)
    so = ''
    for ln in out.splitlines():
        if ln.startswith(('/*', ' *', '*/')):
            continue
        # ⚠️ 不能 .strip()：pad_left/pad_right/trim 族的结果**首尾空白**是契约的一部分
        #   （实测：`pad_right("7",3,"")` 期望 `"7  "`，strip 后成 `"7"` ⇒ 判据假红）
        so = ln.rstrip('\r'); break
    return {'rc': rc, 'code': code, 'body': body, 'out': so, 'raw': '%s\x00%s\x00%s' % (rc, out, err)}


def main():
    if not os.path.exists(PXI):
        print('❌ 缺 %s' % PXI); return 2
    os.makedirs(os.path.join(a.work, 'build'), exist_ok=True)
    for eng, out in (('vm', 'drv_vm'), ('c', 'drv_c')):
        env = dict(os.environ)
        if eng == 'c':
            env['PX_BUILD_ENGINE'] = 'c'
        r = subprocess.run([os.path.join(a.root, 'tools/px'), 'build', DRV], cwd=a.work,
                           capture_output=True, text=True, env=env, timeout=600)
        src = os.path.join(a.work, 'build', 'drv')
        dst = os.path.join(a.work, 'build', out)
        if os.path.exists(src):
            os.replace(src, dst)
        else:
            print('❌ %s 轨构建失败：%s' % (eng, (r.stdout + r.stderr)[-400:])); return 2
    VM = os.path.join(a.work, 'build', 'drv_vm')
    CC = os.path.join(a.work, 'build', 'drv_c')

    cases = []
    for ln in open(a.cases, encoding='utf-8'):
        if ln.startswith('#') or not ln.strip():
            continue
        p = ln.rstrip('\n').split('\t')
        if a.only and not re.search(a.only, p[0]):
            continue
        cases.append((p[0], p[1], p[3] if len(p) > 3 else 'LOCK'))
    if a.need and len(cases) < a.need:
        print('❌ --only 过滤后只剩 %d 条（< need %d）—— 用例选择器写错了' % (len(cases), a.need)); return 2

    sel = set(c[0] for c in cases)          # --only 时：契约表比对**也**收窄到同一子集
    model_in = {}
    if a.model and os.path.exists(a.model):
        for ln in open(a.model, encoding='utf-8'):
            if ln.startswith('#') or not ln.strip():
                continue
            p = ln.rstrip('\n').split('\t')
            if len(p) >= 2 and p[0] in sel:
                # ⚠️ **不许 .strip()**：契约值里 `pad_right`/`pad_left` 族的**尾随空白是契约的一部分**
                #   （M302 实测：`Ls_pad_right_3` 的真值就是 `0||7␠␠`）⇒ strip 会把 3 条判成「值变化」假红
                model_in[p[0]] = p[1].rstrip('\r')

    tracks = {'interp': lambda lb: [PXI, lb, DRV], 'vm': lambda lb: [VM, lb], 'c': lambda lb: [CC, lb]}
    diverged, nondet, mism, timeout_c, rows, nocase = [], [], [], [], {}, []
    RUNS = max(2, a.runs)

    def run_one(key):
        lb, t, rep = key
        return key, one(tracks[t](lb))

    keys = [(lb, t, rep) for lb, _k, _w in cases for t in tracks for rep in range(RUNS)]
    if pmap is not None:
        got = dict(pmap(run_one, keys))
    else:
        got = dict(run_one(k) for k in keys)
    print('  执行：%d 次 %s' % (len(keys), ('（门内并行 · GATE_JOBS=%s）' % os.environ.get('GATE_JOBS', 'auto'))
                                if pmap is not None else '（串行）'))

    n = 0
    for lb, kind, want in cases:
        n += 1
        r = {}
        for t in tracks:
            reps = [got[(lb, t, rep)] for rep in range(RUNS)]
            if kind == 'arity':
                # M302：**只比 (rc, R码)**，不比文案 —— M301（缺陷 518）已定稿
                #   「三轨的 arity 文案**有意**不同」（解释轨「缺少第N个参数 'v'」/
                #   VM「VM main:LINE CALL f 参数不足: 需 N 给 M」/ C「调用 f 参数不足: 需 N 给 M」）。
                #   ⚠️ 覆盖边界：本层**不覆盖 arity 文案的三轨一致性**（那属 M301 的定稿面）。
                key0 = (reps[0]['rc'], reps[0]['code'], '', reps[0]['out'])
            else:
                key0 = (reps[0]['rc'], reps[0]['code'], reps[0]['body'], reps[0]['out'])
            if len(set(x['raw'] for x in reps)) != 1:
                nondet.append({'label': lb, 'track': t,
                               'v': [x['rc'] for x in reps]})
            r[t] = key0
        if len(set(r.values())) != 1:
            diverged.append({'label': lb, 'tracks': {k: list(v) for k, v in r.items()}})
        for t, v in r.items():
            if v[1] == 'TIMEOUT':
                timeout_c.append({'label': lb, 'track': t})
        if any(v[3] == 'NOCASE' for v in r.values()):
            # 驱动器没有命中任何分支 ⇒ 用例表与驱动器**脱节**（静默不执行，M299s1 实测过同族）
            nocase.append(lb)
        have = '%s|%s|%s' % (r['vm'][0], r['vm'][1], r['vm'][3])
        rows[lb] = have
        if kind == 'legal' and want != 'LOCK' and r['vm'][3] != want:
            mism.append({'label': lb, 'want': want, 'got': r['vm'][3], 'all': {k: list(v) for k, v in r.items()}})
        if kind == 'neg' and want != 'LOCK' and r['vm'][1] != want:
            # M302：非法输入必须**响亮**且 R 码符合契约（只靠 [A] 层「三轨一致」是不够的 ——
            #   三轨**一致地静默**同样会让 [A] 变绿；这条正是 M215/M230/M232 那条立论的门化）
            mism.append({'label': lb, 'want': want, 'got': r['vm'][1],
                         'all': {k: list(v) for k, v in r.items()}})

    # [F] 已登记分叉（双向）
    fork_allow = {}
    if a.fork_allow and os.path.exists(a.fork_allow):
        for ln in open(a.fork_allow, encoding='utf-8'):
            if ln.startswith('#') or not ln.strip():
                continue
            q = ln.rstrip('\n').split('\t')
            fork_allow[q[0]] = (q[1].strip() if len(q) > 1 else '')
    fl = set(x['label'] for x in diverged)
    fork_new = sorted(fl - set(fork_allow))            # 新分叉（未登记）⇒ 判红
    fork_stale = sorted(set(fork_allow) - fl)          # 登记过期（已不分叉）⇒ 判红

    # [D] 契约表双向
    stale = sorted([k for k in model_in if k not in rows])
    unreg = sorted([k for k in rows if k not in model_in]) if model_in else []
    changed = sorted([k for k in model_in if k in rows and model_in[k] != rows[k]])

    print('══ M302 三轨对拍：%d 例 · %d 次执行 ══' % (n, n * 3 * max(2, a.runs)))
    print('  [A] 三轨分叉 %d（未登记 %d · 登记过期 %d）· [B] 期望不符 %d · [C] 非确定 %d · 超时 %d' %
          (len(diverged), len(fork_new), len(fork_stale), len(mism), len(nondet), len(timeout_c)))
    print('  [D] 契约表：过期 %d · 漏登记 %d · 值变化 %d' % (len(stale), len(unreg), len(changed)))
    print('  [E] 驱动器未命中的用例（NOCASE） %d' % len(nocase))
    for d in diverged[:8]:
        print('  ❌ 分叉 %s' % d['label'])
        for t, v in d['tracks'].items():
            print('       %-7s rc=%s %s | %s' % (t, v[0], v[1], v[3][:70]))
    for m in mism[:8]:
        print('  ❌ 期望不符 %s：want=%s got=%s' % (m['label'], m['want'], m['got']))
    for m in nondet[:5]:
        print('  ❌ 非确定 %s/%s：%s' % (m['label'], m['track'], m['v']))
    for m in timeout_c[:5]:
        print('  ❌ 超时 %s/%s' % (m['label'], m['track']))
    for m in stale[:8]:
        print('  ❌ 契约表过期 %s（表=%s）' % (m, model_in[m]))
    for m in unreg[:8]:
        print('  ❌ 契约表漏登记 %s（实测=%s）' % (m, rows[m]))
    for m in changed[:8]:
        print('  ❌ 契约值变化 %s：表=%s 实测=%s' % (m, model_in[m], rows[m]))

    if a.model_out:
        with open(a.model_out, 'w', encoding='utf-8') as f:
            f.write('# M302 契约表（实测行为 = 规格）· label\\t<rc>|<R码>|<程序输出首行>\n')
            for lb, kind, want in cases:
                f.write('%s\t%s\n' % (lb, rows[lb]))
    if a.json:
        json.dump({'diverged': diverged, 'mismatch': mism, 'nondet': nondet,
                   'timeout': timeout_c, 'stale': stale, 'unreg': unreg, 'changed': changed,
                   'nocase': nocase, 'fork_new': fork_new, 'fork_stale': fork_stale,
                   'fork_allowed': sorted(fl & set(fork_allow)),
                   'rows': rows, 'n': n},
                  open(a.json, 'w'), ensure_ascii=False, indent=1)
    bad = (len(mism) + len(nondet) + len(timeout_c)
           + len(stale) + len(unreg) + len(changed) + len(nocase)
           + len(fork_new) + len(fork_stale))
    print('M302-THREE-%s' % ('OK' if bad == 0 else 'FAIL'))
    return 0 if bad == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
