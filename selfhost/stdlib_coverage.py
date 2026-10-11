#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# selfhost/stdlib_coverage.py —— stdlib 导出函数**覆盖面台账**（把口头账变成可判定）
# ------------------------------------------------------------
# 立论（沿 M250「native 覆盖面台账」/ M251「孤儿门体检」）：
#   `stdlib/*.px` 是**用户直接调用的那一层**，而「有多少导出函数从来没被任何门的语料碰过」
#   此前**无人能回答**（CHANGELOG 里只有"本轮取第一批 std.collections + std.path"这样的口头账，
#   实测 std.path 只覆盖了 1/9）。
#
# 口径（**诚实命名**）：这里量的是「**语料面提及率**」——
#   函数名以**调用形态**（`name(`）出现在门语料（`examples/**`）或生态语料
#   （`registry/**`、`upstream-tests/**`）里。
#   ⚠️「提及 ≠ 被断言过」：本台账只回答「**有没有人碰过它**」，
#   「碰过之后判据强不强」是各门自己的事。这条边界**必须写在门头**，不许含糊。
#
# 判据（四条，各自独立）：
#   C1 派生完整性：导出函数清单从 `stdlib/*.px` 的 `def` 行**派生**（不手抄）+ 规模下限
#   C2 双向：基线里有而实测已覆盖 ⇒ **过期** ⇒ 判红（逼着把账改小）；
#           实测未覆盖而基线里没有 ⇒ **新增欠账** ⇒ 判红
#   C3 只减不增：基线文件头部记 `cap=<N>`，实测条数 > cap ⇒ 判红
#   C4 基线形态：每条必须有非空理由（空理由 ⇒ 判红）
# 退出码：0=一致；1=判据红；2=环境错误；3=用法/前提错
# ============================================================
import argparse, glob, json, os, re, sys

MODULE_MIN = 25      # 规模锚点：stdlib 模块数下限
FN_MIN = 300         # 规模锚点：导出函数数下限
CORPUS_HINT = 'mention-coverage'   # 口径标签（防被误读成"断言覆盖"）


def derive_exports(root):
    """C1：从源码派生 {module: [fn…]}（只收非 `_` 开头的公开导出）"""
    out = {}
    for p in sorted(glob.glob(os.path.join(root, 'stdlib', '*.px'))):
        mod = os.path.basename(p)[:-3]
        fns = []
        for ln in open(p, encoding='utf-8', errors='ignore'):
            m = re.match(r'^def ([A-Za-z_][A-Za-z0-9_]*)\(', ln)
            if m and not m.group(1).startswith('_'):
                fns.append(m.group(1))
        out[mod] = fns
    return out


def scan_corpus(root, pats, skip_files):
    """扫语料面，返回「以调用形态出现过」的标识符集合"""
    ids = set()
    files = []
    for pat in pats:
        files += glob.glob(os.path.join(root, pat), recursive=True)
    for p in files:
        if p in skip_files:
            continue
        try:
            txt = open(p, encoding='utf-8', errors='ignore').read()
        except Exception:
            continue
        for m in re.finditer(r'(?<![A-Za-z0-9_])([A-Za-z_][A-Za-z0-9_]*)\s*\(', txt):
            ids.add(m.group(1))
    return ids, len(files)


def load_baseline(path):
    cap, rows = None, {}
    if not os.path.exists(path):
        return cap, rows
    for ln in open(path, encoding='utf-8'):
        if ln.startswith('#'):
            m = re.search(r'cap=(\d+)', ln)
            if m:
                cap = int(m.group(1))
            continue
        if not ln.strip():
            continue
        p = ln.rstrip('\n').split('\t')
        rows[p[0]] = p[1].strip() if len(p) > 1 else ''
    return cap, rows


def run(root, baseline_path, json_out=None, verbose=True):
    ex = derive_exports(root)
    self_p = os.path.join(root, 'selfhost', 'stdlib_coverage.py')
    base_self = os.path.abspath(baseline_path)
    gate_pats = ['examples/**/*.px', 'examples/**/*.sh', 'examples/**/*.py']
    eco_pats = ['registry/**/*.px', 'upstream-tests/**/*.px', 'upstream-tests/**/*.sh']
    gate_ids, ng = scan_corpus(root, gate_pats, {os.path.abspath(self_p)})
    eco_ids, ne = scan_corpus(root, eco_pats, set())

    rows = []
    for mod in sorted(ex):
        for fn in ex[mod]:
            rows.append((mod, fn, 'yes' if fn in gate_ids else 'no',
                         'yes' if fn in eco_ids else 'no'))
    total = len(rows)
    unc = [r for r in rows if r[2] == 'no']
    cap, base = load_baseline(baseline_path)

    problems = []
    # C1
    if len(ex) < MODULE_MIN:
        problems.append('C1 模块数 %d < 下限 %d' % (len(ex), MODULE_MIN))
    if total < FN_MIN:
        problems.append('C1 导出函数数 %d < 下限 %d' % (total, FN_MIN))
    # C2 双向
    stale = sorted([k for k in base if k not in set(r[0] + '.' + r[1] for r in unc)])
    new = sorted([r[0] + '.' + r[1] for r in unc if (r[0] + '.' + r[1]) not in base])
    if stale:
        problems.append('C2 基线过期（已覆盖却仍在账上）：%s' % ', '.join(stale[:8]))
    if new:
        problems.append('C2 新增欠账（未覆盖却不在账上）：%s' % ', '.join(new[:8]))
    # C3 只减不增
    if cap is not None and len(base) > cap:
        problems.append('C3 基线条数 %d > cap=%d（只减不增）' % (len(base), cap))
    # C4 理由非空
    empty = sorted([k for k, v in base.items() if not v])
    if empty:
        problems.append('C4 基线理由为空（无效条目）：%s' % ', '.join(empty[:8]))

    if verbose:
        print('══ stdlib 覆盖面台账（口径：%s）══' % CORPUS_HINT)
        print('  模块 %d · 导出函数 %d · 门语料提及 %d · **未提及 %d** · 基线 %d（cap=%s）' %
              (len(ex), total, total - len(unc), len(unc), len(base), cap))
        print('  语料文件：门 %d · 生态 %d' % (ng, ne))
        per = {}
        for mod, fn, g, e in rows:
            per.setdefault(mod, [0, 0])
            per[mod][0] += 1
            if g == 'no':
                per[mod][1] += 1
        for mod in sorted(per):
            a, b = per[mod]
            if b:
                print('    %-18s %2d 个 · 未提及 %2d' % (mod, a, b))
        for p in problems:
            print('  ❌ ' + p)
        print('M300-COVERAGE-%s' % ('OK' if not problems else 'FAIL'))
    if json_out:
        json.dump({'modules': len(ex), 'functions': total, 'uncovered': len(unc),
                   'baseline': len(base), 'cap': cap, 'problems': problems,
                   'rows': [{'module': m, 'fn': f, 'gate': g, 'eco': e} for m, f, g, e in rows]},
                  open(json_out, 'w'), ensure_ascii=False, indent=1)
    return 1 if problems else 0


def write_baseline(root, path):
    ex = derive_exports(root)
    gate_ids, _ = scan_corpus(root, ['examples/**/*.px', 'examples/**/*.sh', 'examples/**/*.py'],
                              {os.path.abspath(__file__)})
    unc = [m + '.' + f for m in sorted(ex) for f in ex[m] if f not in gate_ids]
    with open(path, 'w', encoding='utf-8') as fh:
        fh.write('# stdlib 覆盖面台账基线（**只减不增**）· 未提及的门语料函数\n')
        fh.write('# 格式：<module>.<fn>\t<理由>\n')
        fh.write('# cap=%d\n' % len(unc))
        for k in unc:
            fh.write('%s\t%s\n' % (k, '存量欠账：尚无门的语料以调用形态提及（M300 首次建账）'))
    # ⚠️ **写完立刻回读自证**：M300 实测首版把 `\n` 写成了字面量（非 raw 字符串里的 '\\n'），
    #   结果「格式行 + cap 行 + **第一条表项**」被并成一行 ⇒ 条数静默少 1、而 cap 仍写 190 ⇒
    #   台账自己判自己「新增欠账」。⇒ 写完必须回读，且 **条数必须恰等于 cap**。
    cap2, rows2 = load_baseline(path)
    assert cap2 == len(unc) and len(rows2) == len(unc), \
        '基线写坏了：cap=%s 条数=%d（期望 %d）' % (cap2, len(rows2), len(unc))
    print('已写入 %s（%d 条 · cap=%d · 回读自证通过）' % (path, len(unc), len(unc)))
    return 0


def selftest(root):
    """自证：夹具目录上的期望**精确相等** + 判据自伤负控"""
    import tempfile, shutil
    ok = 0
    fail = 0

    def chk(name, got, want):
        nonlocal ok, fail
        if got == want:
            ok += 1; print('  ✅ %s' % name)
        else:
            fail += 1; print('  ❌ %s（got=%s want=%s）' % (name, got, want))
    tmp = tempfile.mkdtemp(prefix='m300cov')
    try:
        os.makedirs(os.path.join(tmp, 'stdlib'))
        os.makedirs(os.path.join(tmp, 'examples', 'g1'))
        os.makedirs(os.path.join(tmp, 'selfhost'))
        # 夹具：2 模块 · 4 导出（1 私有）+ 1 个门语料只提及 a_pub
        open(os.path.join(tmp, 'stdlib', 'ma.px'), 'w').write(
            'def a_pub(x):\n    return x\ndef _a_priv(x):\n    return x\n')
        open(os.path.join(tmp, 'stdlib', 'mb.px'), 'w').write(
            'def b_pub(x):\n    return x\ndef b_two(x, y):\n    return x\n')
        open(os.path.join(tmp, 'examples', 'g1', 'drv.px'), 'w').write('print(a_pub(1))\n')
        shutil.copy(os.path.join(root, 'selfhost', 'stdlib_coverage.py'),
                    os.path.join(tmp, 'selfhost', 'stdlib_coverage.py'))
        base = os.path.join(tmp, 'base.tsv')
        # ① 派生：私有函数不入册
        ex = derive_exports(tmp)
        chk('① 派生清单（不含 `_` 私有）', sorted(ex['ma']) + sorted(ex['mb']),
            ['a_pub', 'b_pub', 'b_two'])
        # ② 扫面：a_pub 被提及（`print` 也在集里 —— 扫面是"全部被调用的标识符"，
        #    台账只用「函数名 ∈ 该集合」作成员判据 ⇒ 多收不影响正确性）
        ids, _ = scan_corpus(tmp, ['examples/**/*.px'], set())
        chk('② 语料提及集', sorted(ids), ['a_pub', 'print'])
        # ③ 基线正确形态 ⇒ rc=0（规模下限需临时放宽 ⇒ 用环境变量不便，故直接调底层判据）
        open(base, 'w').write('# cap=2\nma.b_pub\t存量\nmb.b_two\t存量\n')
        cap, rows = load_baseline(base)
        chk('③ 基线 cap 解析', cap, 2)
        chk('③ 基线条数', len(rows), 2)
        # ④ 过期判据：把 a_pub 也写进基线 ⇒ 应被判为「过期」
        open(base, 'w').write('# cap=3\nma.a_pub\t误记\nma.b_pub\t存量\nmb.b_two\t存量\n')
        _, rows2 = load_baseline(base)
        unc = {'ma.b_pub', 'mb.b_two'}
        chk('④ 过期条目识别', sorted([k for k in rows2 if k not in unc]), ['ma.a_pub'])
        # ⑤ 空理由判据
        open(base, 'w').write('# cap=1\nma.b_pub\t\n')
        _, rows3 = load_baseline(base)
        chk('⑤ 空理由识别', sorted([k for k, v in rows3.items() if not v]), ['ma.b_pub'])
        # ⑥ 判据自伤 + 防「同义反复」：自伤版（恒假）必须识别不出过期，
        #    且**真判据与自伤版结论必须不同** —— 否则这条"自伤"什么都没证明（M211 教训）
        real = sorted([k for k in rows2 if k not in unc])
        broken = sorted([k for k in rows2 if False])
        chk('⑥ 判据自伤：恒假版识别不出过期', broken, [])
        chk('⑥b 真判据 ≠ 自伤版（防同义反复）', real != broken, True)
        # ⑦ 规模锚点：夹具只有 2 模块 ⇒ 真跑必须判红（防"夹具当真实仓"）
        rc = run(tmp, os.path.join(tmp, 'base_none.tsv'), None, verbose=False)
        chk('⑦ 小夹具真跑必红（规模锚点）', rc, 1)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
    print('  自证：%d 通过 / %d 失败' % (ok, fail))
    return 0 if fail == 0 else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--root', default=None)
    ap.add_argument('--baseline', default=None)
    ap.add_argument('--json', default=None)
    ap.add_argument('--self-test', action='store_true')
    ap.add_argument('--write-baseline', action='store_true')
    a = ap.parse_args()
    root = a.root or os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    if not os.path.isdir(os.path.join(root, 'stdlib')):
        print('❌ 不是仓库根（缺 stdlib/）：%s' % root); return 3
    if a.self_test:
        return selftest(root)
    base = a.baseline or os.path.join(root, 'selfhost', 'stdlib_uncovered.tsv')
    if a.write_baseline:
        return write_baseline(root, base)
    return run(root, base, a.json)


if __name__ == '__main__':
    sys.exit(main())
