#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M250 门 · 探针生成器（ONNX 张量助手族「逐位置 × 错类型 ∪ 错 arity ∪ 边界下标」）

【为什么是这一族】
  M199 量过 native 函数面的「逐位置 × 错类型」，M226/M227 量过方法面与同名两门，
  M230 量过索引/切片，M231 量过运算符矩阵，M246 量过复合赋值 ——
  而 **M248 的覆盖面台账**（`selfhost/check_native_coverage.py`）算出：
  全仓 **21 个 native 从未被任何门语料触碰**，其中一组就是 `f32_*` / `i64_*` / `h3_qs_*`。
  顺着这条账当场证出**缺陷 433**（`f32_at` / `i64_at` 的界判据在 int32 里溢出 ⇒ UB ⇒
  越界读 / 静默错值）。本门把这一族**全面积**收进来。

【面】= 13 个 native × (逐位置 × 6 毒值) ∪ (错 arity) ∪ (合法侧) ∪ (边界下标 17×2)
   毒值：`{}` `7` `"s"` `[1]` `null` `true`

【分派】聚合驱动器 `drv.px`（按 `args()[1]` 选例）⇒ **两次编译**覆盖全量（M199 范式）
"""
import argparse, json, os, re, sys

ap = argparse.ArgumentParser()
ap.add_argument('--out', required=True)
ap.add_argument('--root', required=True)
a = ap.parse_args()
OUT = a.out
os.makedirs(OUT, exist_ok=True)

# ── 面定义：名字 → (逐位置的合法实参, 形参个数) ──
B8 = 'f32_bytes([1.5, 2.5])'      # 8 字节 · count=2
B32 = 'i64_bytes([1, 2, 3, 4])'   # 32 字节 · count=4
GEN = '"/nonexistent-m250.onnx"'  # 路径口：给合法**类型**，值是 Err（不属本门判据）

SPEC = [
    ('f32_bytes',   ['[1.5, 2.5]'],              1),
    ('f32_at',      [B8, '0'],                   2),
    ('f32_count',   [B8],                        1),
    ('i64_bytes',   ['[1, 2, 3, 4]'],            1),
    ('i64_at',      [B32, '0'],                  2),
    ('i64_count',   [B32],                       1),
    ('onnx_model_open',       [GEN],             1),
    ('onnx_info',             ['0'],             1),
    ('onnx_initializer',      ['0', '"w"'],      2),
    ('onnx_initializer_names', ['0'],            1),
    ('onnx_model_close',      ['0'],             1),
    ('onnx_run',              ['0', '{}'],       2),
    ('onnx_op_names',         [],                0),
]

POISONS = ['{}', '7', '"s"', '[1]', 'null', 'true']

# ── 边界下标（缺陷 433 的回归位）：2 的幂 ±1 是 int32 溢出与截断的临界 —— 全部**必须响亮** ──
# ⚠️ 必须**按函数分开**：`f32_at` 的合法域是 [0,2)、`i64_at` 是 [0,4) ⇒
#    共用一张表会把 `i64_at(2)` / `(3)`（**合法**！）误判成「未响亮」。
#    M250 首跑实测就是这个形状（门自己红了两条），遂改为「公共大值 + 各函数首个越界邻域」。
BOUND_COMMON = [-1, -2, -1000000,
                100,
                536870911, 536870912,            # 2^29-1 / 2^29  （i*8 溢出临界）
                1073741823, 1073741824,          # 2^30-1 / 2^30  （i*4 溢出临界）
                2147483647, 2147483648,          # 2^31-1 / 2^31
                4294967295, 4294967296, 4294967297,   # 2^32±1（(int) 截断临界）
                4611686018427387904,             # 2^62
                9223372036854775807]             # INT64_MAX
# 各函数「首个越界」邻域（count + count+1），由面的合法域算出 —— 不写死
BOUND_BY = {'f32_at': [2, 3], 'i64_at': [4, 5]}
# 上下界共用同一组；合法侧下标另给（值必须与独立解码一致）
LEGAL_I64 = [0, 1, 2, 3]
LEGAL_F32 = [0, 1]


def lbl(*ps):
    """标签只允许 [A-Za-z0-9_] —— 毒值里的 `"` / `{}` 直接进标签会截断字符串字面量（M226 教训）"""
    return '_'.join(re.sub(r'[^0-9A-Za-z_]+', '', str(x)) or 'z' for x in ps)


cases = []   # (label, 源码表达式, 类别)
for name, valid, n in SPEC:
    for p in range(n):
        for pz in POISONS:
            args = list(valid)
            args[p] = pz
            cases.append((lbl(name, 'p%d' % p, pz), f'print(str({name}({", ".join(args)})))', 'poison'))
    cases.append((lbl(name, 'ok'), f'print(str({name}({", ".join(valid)})))', 'legal'))
    # 错 arity：0..n+2（跳过正确的 n）
    for k in range(0, n + 3):
        if k == n:
            continue
        args = (list(valid) + ['1', '2', '3'])[:k]
        cases.append((lbl(name, 'n%d' % k), f'print(str({name}({", ".join(args)})))', 'arity'))

# ── 边界与合法下标（只对 f32_at / i64_at）──
#   ⚠️ 每个函数**只用自己的越界集**（`BOUND_COMMON + BOUND_BY[name]`）：`i64_at(2)` 是**合法**的，
#      共表会把它误标成 bounds（M250 首跑实测门自己红了两条 —— 判据逼出了正确的面定义）。
for name, buf, legal in (('f32_at', B8, LEGAL_F32), ('i64_at', B32, LEGAL_I64)):
    for idx in BOUND_COMMON + BOUND_BY[name]:
        cases.append((lbl(name, 'bnd', str(idx).replace('-', 'neg')),
                      f'print(str({name}({buf}, {idx})))', 'bounds'))
    for idx in legal:
        cases.append((lbl(name, 'lcnt', str(idx)),
                      f'print(str({name}({buf}, {idx})))', 'legalidx'))
# count 与 at 的上界必须**同一把尺**（缺陷 433 的定稿口径）—— 独立判据在 verify.sh
# ⚠️ 标签不能叫 `f32_count_ok` —— 那与上面 `lbl('f32_count','ok')` **撞名**（自证当场判红）
cases.append((lbl('cntonly', 'f32'), f'print(str(f32_count({B8})))', 'legal'))
cases.append((lbl('cntonly', 'i64'), f'print(str(i64_count({B32})))', 'legal'))

# ── 聚合驱动器 ──
L = ['let a = args()', 'if len(a) < 2:', '    print("NOCASE")', 'else:',
     '    let c = a[1]', '    var hit = false']
for lb, src, _ in cases:
    L += ['    if c == "%s":' % lb, '        hit = true', '        ' + src]
L += ['    if hit == false:', '        print("NOCASE")']
open(os.path.join(OUT, 'drv.px'), 'w').write('\n'.join(L) + '\n')
with open(os.path.join(OUT, 'cases.tsv'), 'w') as f:
    for lb, src, kind in cases:
        f.write(f'{lb}@@{src}@@{kind}\n')

kinds = {}
for _, _, k in cases:
    kinds[k] = kinds.get(k, 0) + 1
json.dump({'total': len(cases), 'kinds': kinds, 'natives': [s[0] for s in SPEC],
           'poisons': POISONS, 'bound_common': BOUND_COMMON, 'bound_by': BOUND_BY},
          open(os.path.join(OUT, 'surface.json'), 'w'), ensure_ascii=False, indent=1)
print(f'[探针] 生成 {len(cases)} 例 · 类别 {kinds} · native {len(SPEC)} 个 → {OUT}/drv.px（{len(L)} 行）')

# ── 自证：规模下限（M201「锚点自证」纪律 —— 判据不许静默变窄）──
# 判据从**面定义派生**，不写魔法数：改面必须先改上面，这里会自动跟着变。
_n_pos = sum(s[2] for s in SPEC)
_n_bnd = sum(len(BOUND_COMMON) + len(BOUND_BY[n]) for n in BOUND_BY)
assert len(SPEC) == 13, 'native 面被静默收窄'
assert len({c[0] for c in cases}) == len(cases), \
    '标签重复 ⇒ 并行调度会撞键（M250 首跑实测过）'
# BOUND_BY 必须落在各自函数的**合法域之外**（否则门自己会红 —— 首跑实测就是这个形状）
assert min(BOUND_BY['f32_at']) > max(LEGAL_F32), 'f32 的越界集侵入了合法域'
assert min(BOUND_BY['i64_at']) > max(LEGAL_I64), 'i64 的越界集侵入了合法域'
assert kinds.get('poison', 0) == _n_pos * len(POISONS), \
    f'毒值面 {kinds.get("poison")} != {_n_pos * len(POISONS)}'
assert kinds.get('bounds', 0) == _n_bnd, f'边界面 {kinds.get("bounds")} != {_n_bnd}'
assert len(cases) >= 190, f'用例数 {len(cases)} < 190 ⇒ 探针面被静默收窄'
print(f'[探针] 自证 OK：{len(cases)} 例 · 13 native · 位置 {_n_pos} · 边界 {kinds.get("bounds", 0)} 例')
