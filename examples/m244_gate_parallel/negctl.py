#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M244 负控：把 `selfhost/gate_par.py` 打成**已知会坏**的形态，验证门有牙。

三段（各自证明**不同判据层**有牙）
----------------------------------
A · **破坏保序**   `pmap` 返回值反序      ⇒ bench [4] 的「输出逐字节相同」必须红
      （为什么挑 [4] 不挑 [3]：[3] 的合成负载也依赖保序，但它是自造的；
        用**真实驱动**更能证明「并行没改语义」这条立论）
B · **忽略并发度** `jobs()` 恒返回 1      ⇒ bench [3] 的「加速比 ≥3.0」必须红
      （证明加速比判据不是恒真 —— 若它恒真，"并行到底有没有生效"就无从判断）
C · **判据自伤**   A 的补丁在位 + `bench --harm` ⇒ A 的红**必须消失**
      （证明 A 的红**来自那条比较本身**，而不是来自别的偶然因素）

⚠️ 只改 `selfhost/gate_par.py` **一个文件**；`--restore` 逐字节还原（`cmp` 自证）。
"""
import argparse
import os
import shutil
import sys

ap = argparse.ArgumentParser()
ap.add_argument('--root', required=True)
ap.add_argument('--work', required=True)
ap.add_argument('--apply', choices=['A', 'B'])
ap.add_argument('--restore', action='store_true')
ap.add_argument('--selftest', action='store_true')
a = ap.parse_args()

FILE = os.path.join(a.root, 'selfhost/gate_par.py')
SNAP = os.path.join(a.work, 'gate_par.py.snap')

PATCH = {
    'A': ('        return list(ex.map(fn, items))',
          '        _r = list(ex.map(fn, items))\n'
          '        return list(reversed(_r))          # NEGCTL-M244-A：故意破坏保序'),
    'B': ('    return os.cpu_count() or 4',
          '    return 1                               # NEGCTL-M244-B：忽略并发度，恒串行'),
}


def read(p):
    with open(p, encoding='utf-8') as f:
        return f.read()


def write(p, s):
    with open(p, 'w', encoding='utf-8') as f:
        f.write(s)


def snap():
    """**只做首次快照** —— 重复快照会把已被污染的源存进快照
    （M239s1 缺陷 394 的老病：`--apply A` 后 `--apply D` 的第二次 snap 存的是 A 的补丁）。"""
    if not os.path.exists(SNAP):
        shutil.copy(FILE, SNAP)
        print('snapshot: %s' % SNAP)


def restore():
    if os.path.exists(SNAP):
        shutil.copy(SNAP, FILE)
        os.remove(SNAP)
        print('restore: ok')
    else:
        print('restore: (无快照)')


if a.restore:
    restore()
    sys.exit(0)

if a.selftest:
    txt = read(FILE)
    bad = 0
    for k, (old, _new) in sorted(PATCH.items()):
        n = txt.count(old)
        print('  锚点 %s：命中 %d 次（须 1）%s' % (k, n, '' if n == 1 else '  ❌'))
        bad += (n != 1)
    print('negctl 锚点自证 %s' % ('OK' if not bad else 'FAIL'))
    sys.exit(1 if bad else 0)

if not a.apply:
    print('用法: --apply A|B | --restore | --selftest'); sys.exit(2)

snap()
old, new = PATCH[a.apply]
txt = read(FILE)
if txt.count(old) != 1:
    print('❌ 锚点命中 %d 次（须 1）—— 拒绝改动' % txt.count(old)); sys.exit(3)
write(FILE, txt.replace(old, new))
print('apply %s ok' % a.apply)
