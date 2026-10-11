#!/usr/bin/env python3
# ============================================================
# M301 · 探针生成器：函数调用「实参个数」面（缺陷 518）
# ------------------------------------------------------------
# 为什么用**聚合驱动器**（M199/M226 范式）：一例一次 build 需 7.5 小时（不可行）；
#   合成一个驱动器 + args()[1] 分派 ⇒ **两次编译**覆盖全量，每例执行 ~10ms。
# 用法：gen_probes.py --out <work>
# 输出：<work>/drv.px · <work>/cases.json
# ============================================================
import argparse, json, os

ap = argparse.ArgumentParser()
ap.add_argument('--out', required=True)
a = ap.parse_args()
W = a.out
os.makedirs(W, exist_ok=True)

PRELUDE = '''def f2(a, b):
    return a + b

def g2(a, b = 10):
    return a + b

def z0():
    return 7

def apply1(f):
    return f(1)

struct Pt:
    x: int
    y: int

impl Pt:
    def scaled(self, k, base = 0):
        return self.x * k + base
'''

# label, body lines, 说明, 类别(legal / strict)
CASES = [
 ("a1", ["return f2(1)"],                    "少给·无默认值",            "strict"),
 ("a2", ["return g2(1)"],                    "少给·有默认值⇒补齐",        "legal"),
 ("a3", ["return g2()"],                     "少给全部",                 "strict"),
 ("b1", ["return f2(1, 2, 3)"],              "多给·无默认值",            "strict"),
 ("b2", ["return g2(1, 2, 3)"],              "多给·有默认值",            "strict"),
 ("b3", ["return z0(1)"],                    "多给·零参",                "strict"),
 ("c1", ["let cl1 = fn (x, y): x + y", "return cl1(1)"],         "闭包少给·无默认值",   "strict"),
 ("c2", ["let cl2 = fn (x, y = 5): x + y", "return cl2(1)"],     "闭包少给·有默认值",   "legal"),
 ("c3", ["let cl3 = fn (x, y): x + y", "return cl3(1, 2, 3)"],   "闭包多给",            "strict"),
 ("d1", ["let p1 = Pt(1, 2)", "return p1.scaled(3, 1, 9)"],      "方法多给",            "strict"),
 ("d2", ["let p2 = Pt(1, 2)", "return p2.scaled()"],             "方法少给",            "strict"),
 ("d3", ["let p3 = Pt(1, 2)", "return p3.scaled(3)"],            "方法少给·有默认值",   "legal"),
 ("e1", ["let fa = f2", "return fa(1)"],                         "间接(值调用)少给",    "strict"),
 ("e2", ["let fb = f2", "return fb(1, 2, 3)"],                   "间接(值调用)多给",    "strict"),
 ("e3", ["return apply1(z0)"],               "回调·零参函数被 1 参调用",  "strict"),
 ("e4", ["return apply1(fn (v): v * 3)"],    "回调·正确形态",            "legal"),
 ("f1", ["return sorted([3, 1, 2])"],        "内置 sorted",              "legal"),
 ("g1", ["return g2(1, 2)"],                 "恰好给齐",                 "legal"),
 ("g2", ["return f2(1, 2)"],                 "恰好给齐2",                "legal"),
 ("h1", ["let gg = [x * 2 for x in [1, 2, 3]]", "return gg"],    "生成器",                   "legal"),
 ("h2", ["let mm = map([1, 2, 3], fn (v): v + 1)", "return mm"], "map 回调",                 "legal"),
]

drv = [PRELUDE, '']
for lb, body, _d, _k in CASES:
    drv.append('def c_%s():' % lb)
    for bl in body:
        drv.append('    ' + bl)
    drv.append('')

drv.append('def main():')
drv.append('    let av = args()')
drv.append('    if len(av) < 2:')
drv.append('        print("NEEDCASE")')
drv.append('        return 1')
drv.append('    let n = av[1]')
for lb, _b, _d, _k in CASES:
    drv.append('    if n == "%s":' % lb)
    drv.append('        print(c_%s())' % lb)
    drv.append('        return 0')
drv.append('    print("NOCASE")')
drv.append('    return 2')

open(os.path.join(W, 'drv.px'), 'w').write('\n'.join(drv) + '\n')
with open(os.path.join(W, 'cases.json'), 'w') as f:
    json.dump([(lb, d, k) for lb, _b, d, k in CASES], f, ensure_ascii=False, indent=1)
print('生成 drv.px：%d 例（strict %d · legal %d）'
      % (len(CASES), sum(1 for c in CASES if c[3] == 'strict'), sum(1 for c in CASES if c[3] == 'legal')))
