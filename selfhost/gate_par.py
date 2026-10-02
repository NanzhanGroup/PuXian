#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gate_par.py —— 「门内并行」公共执行器（M244）

问题
----
全量门实测 **6997s / 154 门**，其中 12 个「矩阵对拍门」占 **3513s（一半）**，
而它们的耗时主体是**逐例 spawn 一个进程**的三重串行循环：

    for case in cases:              # 197
        for face in ('f', 'm'):     #  ×2
            for track in (...):     #  ×3
                subprocess.run(...) #  ⇒ 1182 次**串行** spawn

每次 spawn 之间**毫无依赖**（同一份驱动器、不同 case 选择；各自独立进程、不写共享文件）
⇒ 天生可并行。而全量门是**一门一门串行**跑的（`run_gates.sh` 一次一门 + PID 锁）
⇒ **门内可以吃满整机核数**，且不影响「门之间禁止并行」这条既有的正确性护栏
（53 个门的负控会改源码 ⇒ 门之间一旦并行必然互相踩）。

口径
----
`GATE_JOBS`（环境变量，默认 = `os.cpu_count()`）：
    GATE_JOBS=1      ⇒ 强制串行（**判据用**：与并行档对拍，证明并行不改语义）
    GATE_JOBS=8      ⇒ 8 路
    非法值（非整数 / <1）⇒ **响亮退出 rc=2**，绝不静默回退
（「失败必须给出可执行的下一步」—— 静默回退会让 GATE_JOBS=1 的对拍假绿。）

用法
----
    import sys, os
    sys.path.insert(0, os.path.join(ROOT, 'selfhost'))
    from gate_par import pmap, jobs      # noqa: E402

    cmd = {(c, t): [...] for c in cases for t in ('interp', 'vm', 'c')}
    got = dict(pmap(lambda k: (k, run(cmd[k])), list(cmd)))   # 保序 · 并发

自证
----
    python3 selfhost/gate_par.py --selftest      # rc=0 才可用
"""
import os
import sys


def jobs(default=None):
    """→ 并发度。GATE_JOBS 非法 ⇒ rc=2（**不静默回退**）。"""
    v = (os.environ.get('GATE_JOBS') or '').strip()
    if v:
        try:
            n = int(v)
        except ValueError:
            print('❌ GATE_JOBS 不是整数: %r（要正整数，或删掉它用默认）' % v, file=sys.stderr)
            raise SystemExit(2)
        if n < 1:
            print('❌ GATE_JOBS 必须 ≥1: %d（1 = 串行，用于与并行档对拍）' % n, file=sys.stderr)
            raise SystemExit(2)
        return n
    if default is not None:
        return default
    return os.cpu_count() or 4


def pmap(fn, items, n=None):
    """保序并行 map。

    **保序是判据不是巧合**：多个门的产物里带「按输入顺序排列」的行
    （如 `cases.tsv` 逐例结果、`res.txt`、MODEL 对拍表）—— 顺序一漂，
    `cmp` 类的还原判据就会假红（而那是**判据**，不是噪声）。

    threads 而非 processes：`subprocess.run` 会释放 GIL ⇒ 真并行；
    且结果对象直接传回，无需序列化。
    """
    items = list(items)
    if n is None:
        n = jobs()
    if n <= 1 or len(items) <= 1:
        return [fn(x) for x in items]
    from concurrent.futures import ThreadPoolExecutor
    with ThreadPoolExecutor(max_workers=min(n, len(items))) as ex:
        return list(ex.map(fn, items))


def pmap_records(fn, items, n=None):
    """`pmap` 的便利包装：fn 返回 `(key, value)` ⇒ 回传 dict（键唯一由调用方保证）。"""
    out = {}
    for k, v in pmap(fn, items, n):
        if k in out:
            raise SystemExit('❌ pmap_records 键重复: %r（并行任务必须一一对应）' % (k,))
        out[k] = v
    return out


# ══════════════════════════════════════════════════════════════
# 自证（--selftest）：判据本身必须能被证明
# ══════════════════════════════════════════════════════════════
def _selftest():
    import time
    ok = [0]
    bad = [0]

    def chk(name, cond):
        if cond:
            print('  PASS %s' % name)
            ok[0] += 1
        else:
            print('  FAIL %s' % name)
            bad[0] += 1

    # ① 保序：100 个元素，fn 故意让**后面的先完成**（逆序 sleep）
    N = 100
    rev = lambda i: (N - i) * 0.0005  # noqa: E731
    def f_order(i):
        time.sleep(rev(i))
        return i * 3
    r = pmap(f_order, range(N), n=8)
    chk('① 保序（乱序完成 ⇒ 结果仍按输入序）', r == [i * 3 for i in range(N)])

    # ② 等价：串行档 ⇄ 并行档 **逐字节相同**
    def f_rand(i):
        return 'row%04d|%s' % (i, 'x' * (i % 7))
    ser = pmap(f_rand, range(60), n=1)
    par = pmap(f_rand, range(60), n=8)
    chk('② 等价（n=1 ⇄ n=8 逐字节相同）', ser == par)

    # ③ 确定性：同一输入连跑两次相同
    chk('③ 确定性（重复跑相同）', pmap(f_rand, range(60), n=8) == par)

    # ④ 规模下限：真的用了 >1 个 worker（否则"并行"没发生 = 判据假绿）
    import threading
    live = [0]
    peak = [0]
    lk = threading.Lock()
    def f_conc(i):
        with lk:
            live[0] += 1
            peak[0] = max(peak[0], live[0])
        time.sleep(0.05)
        with lk:
            live[0] -= 1
        return i
    pmap(f_conc, range(32), n=8)
    chk('④ 真并发（峰值在飞 ≥4，实测 %d）' % peak[0], peak[0] >= 4)

    # ⑤ jobs() 解析：合法值生效 / 非法值**响亮**
    os.environ['GATE_JOBS'] = '3'
    chk('⑤ GATE_JOBS=3 ⇒ jobs()==3', jobs() == 3)
    os.environ['GATE_JOBS'] = '1'
    chk('⑤ GATE_JOBS=1 ⇒ jobs()==1', jobs() == 1)
    os.environ.pop('GATE_JOBS', None)
    chk('⑤ 无 GATE_JOBS ⇒ 默认 = cpu_count（>0）', jobs() >= 1)
    for badv in ('abc', '0', '-2', '1.5'):
        os.environ['GATE_JOBS'] = badv
        try:
            sys.stderr = open(os.devnull, 'w')
            jobs()
            got = 'no-exit'
        except SystemExit as e:
            got = e.code
        finally:
            sys.stderr = sys.__stderr__
        chk('⑤ GATE_JOBS=%r ⇒ 响亮 rc=2（实测 %r）' % (badv, got), got == 2)
    os.environ.pop('GATE_JOBS', None)

    # ⑥ 异常必须**传播**（不许被吞掉 ⇒ 否则门会假绿）
    def f_boom(i):
        if i == 17:
            raise RuntimeError('boom')
        return i
    try:
        pmap(f_boom, range(40), n=8)
        chk('⑥ 任务异常 ⇒ 传播（不许吞）', False)
    except RuntimeError:
        chk('⑥ 任务异常 ⇒ 传播（不许吞）', True)

    # ⑦ pmap_records：键重复必须响亮
    try:
        pmap_records(lambda x: ('k', x), [1, 2], n=4)
        chk('⑦ 键重复 ⇒ 响亮', False)
    except SystemExit:
        chk('⑦ 键重复 ⇒ 响亮', True)
    chk('⑦ 键唯一 ⇒ 正确汇总', pmap_records(lambda x: (x, x * 2), [1, 2, 3], n=4) == {1: 2, 2: 4, 3: 6})

    print('GATE-PAR-SELFTEST-%s pass=%d fail=%d' % ('OK' if not bad[0] else 'FAIL', ok[0], bad[0]))
    return 1 if bad[0] else 0


if __name__ == '__main__':
    if '--selftest' in sys.argv:
        raise SystemExit(_selftest())
    print(__doc__)
    print('cpu_count=%s · jobs()=%s' % (os.cpu_count(), jobs()))
