#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""native 覆盖面台账 —— 「哪些 native 从未被任何门语料触碰过」

【来历（M248 → M250）】
  M199/M226/M227/M228/M229/M230/M231/M246 一路把「逐位置 × 错类型」「同名两门」「运算符矩阵」
  「复合赋值」逐个做成了清单级判据 —— 但**没有人量过这件事本身**：
  **注册表里有 392 个 native，其中有多少个从来没被任何门语料碰到过？**
  M248 第一次算出来是 **21 个**；顺着这份账当场证出**缺陷 433**
  （`f32_at` / `i64_at` 的界判据在 int32 里溢出 ⇒ UB ⇒ 越界读 / 静默错值）。
  ⇒ 本脚本把那次一次性分析变成**常设判据**。

【判据】
  ① 期望集与规模锚点：`docs/native_index.json`（**源码派生**：`px_set_global(px_native)`）
     自身的 `count` 必须等于 `names` 去重后长度，且 ≥ 350（防判据静默变窄）。
  ② 划分完备：`已覆盖 ∪ 未覆盖 == 全部`，且两边不相交。
  ③ **未覆盖不许扩大**：实测未覆盖集 − 基线 ⇒ 非空即判红（并逐条指名）。
  ④ **基线不许过期**：基线 − 实测未覆盖集 ⇒ 非空即判红（已覆盖了就把登记删掉）。
  ⑤ 基线条目必须带理由。

【为什么「消费面」必须与「期望集」**不同源**（M248 的教训）】
  `tools/gen_native_table.sh` 是**生成**那份 native 名单的生成器 ⇒ 它必然提到所有名字。
  首版把它算进消费面，得到「覆盖 392/392」的假象。本脚本**只扫消费方**
  （`examples/` 门语料与门脚本 · `registry/` 生态库），不扫 `tools/` 与 `selfhost/`。

用法:
    check_native_coverage.py [--root .] [--baseline selfhost/native_coverage.txt]
                             [--json OUT] [--update] [--self-test]
退出码: 0=通过 · 1=判红 · 2=用法/输入错 · 3=判据自身失效
"""
import argparse, json, os, re, shutil, sys, tempfile

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
ap.add_argument('--baseline', default=None)
ap.add_argument('--json', default=None)
ap.add_argument('--update', action='store_true', help='用当前实测重写基线（保留已有理由）')
ap.add_argument('--self-test', action='store_true')
a = ap.parse_args()
ROOT = os.path.abspath(a.root)
BASE = a.baseline or os.path.join(ROOT, 'selfhost/native_coverage.txt')

# 消费面（**与期望集不同源**）：门语料/门脚本 + 生态库
SURFACES = ['examples', 'registry']
# ⚠️ **只扫 `.px`**（M250 实测定的口径）：
#   「被门语料触碰」= 该名字出现在**一份普贤源程序**里（那才会真的执行到它）。
#   门脚本（`.sh`/`.py`/`.md`）是**装置**不是语料 —— 把它们计进来会**自我污染**：
#   M250 首跑实测，本门的「覆盖边界」段**用文字列了** 16 个未覆盖名字
#   ⇒ 立刻被判成「已覆盖」⇒ 基线全变「过期」⇒ 判据③④同时红。
#   ⇒ 推论（写进约定）：**语料若是运行期生成的，必须另外提交一份 `.px` 冒烟**
#      （本门就是 `examples/m250_onnx_tensor/surface.px`），否则台账看不见它。
EXTS = ('.px',)
SKIP_DIRS = {'.git', 'build', 'node_modules', '.px_modules', '__pycache__'}

MIN_TOTAL = 350     # 规模锚点：native 总数下限（判据①）


def die(msg, rc=2):
    print(f'❌ {msg}')
    sys.exit(rc)


def load_expected(root):
    p = os.path.join(root, 'docs/native_index.json')
    if not os.path.exists(p):
        die(f'缺 {p}（先跑 bash tools/gen_native_table.sh）', 3)
    d = json.load(open(p, encoding='utf-8'))
    names = sorted(set(d.get('names') or []))
    if not names:
        die('native_index.json 里没有 names ⇒ 判据自身失效', 3)
    return names, int(d.get('count') or 0)


def scan(root, names):
    """→ {native: 命中文件数}。只扫 SURFACES，**不扫 tools/ 与 selfhost/**（同源伪影）。"""
    pats = {n: re.compile(r'(?<![0-9A-Za-z_])' + re.escape(n) + r'(?![0-9A-Za-z_])') for n in names}
    hits = {}
    for s in SURFACES:
        d = os.path.join(root, s)
        if not os.path.isdir(d):
            continue
        for dp, dns, fns in os.walk(d):
            dns[:] = [x for x in dns if x not in SKIP_DIRS]
            for fn in fns:
                if not fn.endswith(EXTS):
                    continue
                try:
                    txt = open(os.path.join(dp, fn), encoding='utf-8', errors='ignore').read()
                except OSError:
                    continue
                for n in names:
                    if n in txt and pats[n].search(txt):
                        hits[n] = hits.get(n, 0) + 1
    return hits


def read_base(path):
    base = {}
    if os.path.exists(path):
        for ln in open(path, encoding='utf-8'):
            ln = ln.rstrip('\n')
            if not ln.strip() or ln.lstrip().startswith('#'):
                continue
            p = ln.split('\t')
            base[p[0].strip()] = (p[1].strip() if len(p) > 1 else '')
    return base


def write_base(path, mapping):
    with open(path, 'w', encoding='utf-8') as f:
        f.write('# native 覆盖面基线 —— **「从未被任何门语料触碰过」的 native 清单**\n')
        f.write('# 由 selfhost/check_native_coverage.py 维护；纪律：\n')
        f.write('#   ① 本表**只许缩小** —— 新出现的未覆盖条目 ⇒ 判红；\n')
        f.write('#   ② 已覆盖的条目必须删掉（否则判「过期」）；\n')
        f.write('#   ③ 每条必须带理由（新增条目默认「（待定性）」⇒ 不满足③ ⇒ 会判红）。\n')
        f.write('#   ④ 「已覆盖」= 名字以**独立词**（两侧非 [0-9A-Za-z_]）出现在\n')
        f.write('#      examples/ 或 registry/ 的文件里；消费面**不含** tools/ 与 selfhost/。\n')
        for k in sorted(mapping):
            f.write(f'{k}\t{mapping[k]}\n')


# ── 自证（10 锚点）：判据自身的坑要做成永久夹具（M207/M209 纪律）──
if a.self_test:
    ME = os.path.abspath(__file__)
    T = tempfile.mkdtemp(prefix='m250cov')
    ok = fail = 0

    def chk(name, cond):
        global ok, fail
        if cond:
            ok += 1; print(f'  ✅ {name}')
        else:
            fail += 1; print(f'  ❌ {name}')

    def mk(names_json, corpus, baseline):
        shutil.rmtree(T, ignore_errors=True)
        os.makedirs(f'{T}/docs'); os.makedirs(f'{T}/examples/g1'); os.makedirs(f'{T}/registry/r1')
        os.makedirs(f'{T}/selfhost')
        json.dump(names_json, open(f'{T}/docs/native_index.json', 'w'))
        for rel, txt in corpus.items():
            os.makedirs(os.path.dirname(f'{T}/{rel}'), exist_ok=True)
            open(f'{T}/{rel}', 'w').write(txt)
        open(f'{T}/selfhost/native_coverage.txt', 'w').write(baseline)

    def run(extra=''):
        return os.system(f'{sys.executable} {ME} --root {T} --json {T}/j.json '
                         f'--baseline {T}/selfhost/native_coverage.txt {extra} >{T}/o 2>&1')

    BIG = {'count': 400, 'names': ['alpha', 'beta_1', 'gamma'] + ['pad%d' % i for i in range(397)]}
    with open(f'{T}/o', 'w') as _:
        pass
    # 夹具：alpha 独立词命中；beta_1 只出现在词内（zz_alpha_beta_1）⇒ 不算；gamma 同理
    mk(BIG, {'examples/g1/x.px': 'alpha\nzz_alpha_beta_1\n', 'registry/r1/y.px': 'gamma_and_more\n'},
       '# a\n# b\n# c\n# d\n# e\n# f\n')
    r = run()
    got = json.load(open(f'{T}/j.json'))
    chk('S1 独立词命中：alpha 在已覆盖集里', 'alpha' in got['hits'])
    chk('S2 词边界：`zz_alpha_beta_1` 不误命中 beta_1', 'beta_1' not in got['hits'])
    chk('S3 词边界：`gamma_and_more` 不误命中 gamma', 'gamma' not in got['hits'])
    chk('S4 划分完备：covered + uncovered == 全部', len(got['hits']) + len(got['uncovered']) == got['total'])
    # 负控①：漏登记 gamma ⇒ 「新增未覆盖」判红 + 点名
    mk(BIG, {'examples/g1/x.px': 'alpha\nzz_alpha_beta_1\n', 'registry/r1/y.px': 'gamma_and_more\n'},
       '# a\n# b\n# c\n# d\n# e\n# f\npad0\t理由\n')
    r1 = run()
    o1 = open(f'{T}/o').read()
    chk('S5 负控①：漏登记 ⇒ 判红且点名到具体 native', r1 != 0 and 'gamma' in o1 and '新增未覆盖' in o1)
    # 负控②：登记一个**已覆盖**的 ⇒ 「过期」判红
    full = ''.join(f'pad{i}\t理由\n' for i in range(397))
    mk(BIG, {'examples/g1/x.px': 'alpha\n', 'registry/r1/y.px': 'gamma_and_more\n'},
       '# a\n# b\n# c\n# d\n# e\n# f\n' + full + 'alpha\t理由A\n')
    r2 = run()
    o2 = open(f'{T}/o').read()
    chk('S6 负控②：登记已覆盖的 alpha ⇒ 判「过期」', r2 != 0 and '过期' in o2)
    # 负控③：基线条目**无理由** ⇒ 判红
    mk(BIG, {'examples/g1/x.px': 'alpha\n', 'registry/r1/y.px': 'gamma_and_more\n'},
       '# a\n# b\n# c\n# d\n# e\n# f\n' + full + 'alpha\n')
    r3 = run()
    o3 = open(f'{T}/o').read()
    chk('S7 负控③：基线条目无理由 ⇒ 判红', r3 != 0 and '没有理由' in o3)
    # 期望集为空 ⇒ rc=3（**拒绝静默回退成「全绿」**）
    mk({'count': 0, 'names': []}, {}, '# a\n')
    r4 = run()
    chk('S8 期望集为空 ⇒ rc=3（拒绝静默回退）', (r4 >> 8) == 3)
    # 规模锚点：只有 3 个 native（< 350）⇒ 判红
    mk({'count': 3, 'names': ['alpha', 'beta', 'gamma']}, {'examples/g1/x.px': 'alpha\n'}, '# a\n')
    r5 = run()
    o5 = open(f'{T}/o').read()
    chk('S9 规模锚点：总数 < 350 ⇒ 判红', r5 != 0 and '锚点' in o5)
    # 同源伪影：tools/ 下的文件**不得**算进消费面
    shutil.rmtree(T, ignore_errors=True)
    chk('S10 消费面不含 tools/ 且**只扫 .px**（装置文件排除 ⇒ 防自我污染）',
        'tools' not in SURFACES and EXTS == ('.px',))
    shutil.rmtree(T, ignore_errors=True)
    print(f'\n自证：{ok} 通过 / {fail} 失败')
    sys.exit(0 if fail == 0 else 1)

NAMES, CNT = load_expected(ROOT)
if len(NAMES) != CNT:
    print(f'⚠️ native_index.json 的 count={CNT} 与 names 去重后 {len(NAMES)} 不一致（判据①）')
if len(NAMES) < MIN_TOTAL:
    print(f'❌ 判据① 规模锚点：native 总数 {len(NAMES)} < {MIN_TOTAL} ⇒ 期望集被静默收窄')
    sys.exit(1)

HITS = scan(ROOT, NAMES)
covered = sorted(n for n in NAMES if n in HITS)
uncovered = sorted(n for n in NAMES if n not in HITS)
base = read_base(BASE)

if a.update:
    keep = {k: base.get(k, '（待定性）') for k in uncovered}
    write_base(BASE, keep)
    print(f'✅ 已重写基线 {BASE}（{len(keep)} 条）')
    sys.exit(0)

new_unc = sorted(set(uncovered) - set(base))
stale = sorted(set(base) - set(uncovered))
no_reason = sorted(k for k, v in base.items() if not v)

ok = True
print('── native 覆盖面台账 ──')
print(f'   期望集：docs/native_index.json（源码派生 px_set_global(px_native)）· {len(NAMES)} 个')
print('   消费面：examples/ + registry/（**不含 tools/ 与 selfhost/** —— 与期望集不同源）')
print(f'   已覆盖：{len(covered)}（{100.0 * len(covered) / len(NAMES):.1f}%）· 未覆盖：{len(uncovered)}')
if len(covered) + len(uncovered) != len(NAMES):
    print('❌ 判据② 划分不完备')
    ok = False
if new_unc:
    print(f'❌ 判据③ **新增未覆盖** {len(new_unc)} 个（基线只许缩小）——')
    for n in new_unc[:20]:
        print(f'      {n}')
    ok = False
if stale:
    print(f'❌ 判据④ 基线**过期** {len(stale)} 条（这些已经覆盖了，请从基线删掉）——')
    for n in stale[:20]:
        print(f'      {n}')
    ok = False
if no_reason:
    print(f'❌ 判据⑤ {len(no_reason)} 条基线没有理由 —— {no_reason[:10]}')
    ok = False
if ok:
    print(f'✅ 判据 ①②③④⑤ 全通过（基线 {len(base)} 条 · 未覆盖 {len(uncovered)} 条与基线一致）')
if a.json:
    # ⚠️ 两个键必须**同型**（都是列表）—— M250 首跑时 `covered` 是 int、`uncovered` 是 list，
#    消费方（门）`len(d['covered'])` 当场 `TypeError: object of type 'int' has no len()`。
    json.dump({'total': len(NAMES), 'covered': covered, 'uncovered': uncovered,
               'n_covered': len(covered), 'n_uncovered': len(uncovered),
               'hits': HITS}, open(a.json, 'w'), ensure_ascii=False, indent=1)
sys.exit(0 if ok else 1)
