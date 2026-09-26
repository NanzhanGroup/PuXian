#!/usr/bin/env python3
# ============================================================
# M217 语料生成器（**单一事实源**）—— 移位计数的越界语义（缺陷 309）
# ------------------------------------------------------------
# 产出：
#   cases.px     —— 驱动器（按 args()[1] 分派；**一次编译**覆盖全量用例，见 M199 教训）
#   CASES.tsv    —— label / kind / 期望（ok ⇒ 期望 stdout；rej ⇒ `R####|消息体`）
# 真值来源（**独立第三份**）：本文件的 `px_shl/px_shr/px_shru` —— 按**定义式**用 Python
#   bignum 算（`(a & MASK64) << (n & 63)` 再回绕成有符号），与被测 C 实现**不同构**。
# 语义（三轨同一真相，见 runtime.h 的 M217 段）：
#   · 计数 ∈ [0,63]  ⇒ 正常移位
#   · 计数 >= 64     ⇒ **掩码 `n & 63`**（= Rust wrapping_*；与本仓既有 `>>>` 口径一致）
#   · 计数 < 0       ⇒ **R1003 响亮**
# 漂移检测：verify.sh 第 1 层重跑本脚本并与入库的 cases.px / CASES.tsv 逐字节比。
# ============================================================
import argparse
import os
import sys

MASK64 = (1 << 64) - 1


def s64(u):
    """把 64 位无符号位型解释为有符号（二补数）。"""
    u &= MASK64
    return u - (1 << 64) if (u >> 63) else u


def px_shl(a, n):
    """定义式：计数掩码后左移（经无符号，回绕良定义）。"""
    return s64((a & MASK64) << (n & 63))


def px_shr(a, n):
    """算术右移（Python 的 >> 对负数即 floor ⇒ 与算术右移逐值相同）。"""
    return a >> (n & 63)


def px_shru(a, n):
    """逻辑右移：按无符号解释后右移。"""
    return s64((a & MASK64) >> (n & 63))


def lit(v):
    """把整数写成 .px 字面量。

    ⚠️ 陷阱（M215 事实 240）：`INT64_MIN` **不能**写成 `0 - 9223372036854775808` ——
    词法阶段会按正整数解析 `9223372036854775808` 并报 `E1004 无效整数`（与 Go 同款限制）
    ⇒ 必须写成 `0 - 9223372036854775807 - 1`（先减到 MAX 再减 1，全程在范围内）。
    """
    if v == -(1 << 63):
        return '0 - 9223372036854775807 - 1'
    return '0 - %d' % -v if v < 0 else '%d' % v


def triple(a, n):
    return '%d %d %d' % (px_shl(a, n), px_shr(a, n), px_shru(a, n))


# ---- 合法侧：(label, .px 源码, 期望 stdout) ----
LEGAL = []

# ① 计数全量扫描（0..63）—— **运行期形态**（变量计数）× 五种操作数
for oname, oval in (('pos', 1), ('neg', -1), ('neg8', -8),
                    ('min', -(1 << 63)), ('max', (1 << 63) - 1)):
    body = ['var a = %s' % lit(oval), 'var n = 0', 'while n < 64:',
            '    print(a << n, a >> n, a >>> n)', '    n = n + 1']
    exp = '\n'.join(triple(oval, n) for n in range(64))
    LEGAL.append(('ok_scan_%s' % oname, '\n'.join(body), exp))

# ② 越界计数（掩码语义）—— 含 64/65/66/127/128/129/255/256/1000/4095/INT64_MAX
OVER = [64, 65, 66, 127, 128, 129, 255, 256, 1000, 4095, (1 << 63) - 1]
body = ['var a = 3', 'var n = 0']
for n in OVER:
    body.append('n = %d' % n)
    body.append('print(a << n, a >> n, a >>> n)')
LEGAL.append(('ok_over_counts', '\n'.join(body),
              '\n'.join(triple(3, n) for n in OVER)))

# ③ 负操作数 × 越界计数（掩码 + 算术/逻辑右移的区别）
NEG_CASES = [(-1, 63), (-1, 64), (-1, 65), (-8, 63), (-8, 64), (-8, 127),
             (-(1 << 63), 1), (-(1 << 63), 63), (-(1 << 63), 64),
             ((1 << 63) - 1, 1), ((1 << 63) - 1, 64)]
body, exp = [], []
for i, (a, n) in enumerate(NEG_CASES):
    body.append('var a%d = %s' % (i, lit(a)))
    body.append('print(a%d << %d, a%d >> %d, a%d >>> %d)' % (i, n, i, n, i, n))
    exp.append(triple(a, n))
LEGAL.append(('ok_neg_operand', '\n'.join(body), '\n'.join(exp)))

# ④ 常量形态 vs 运行期形态 —— **必须同答案**
#   （修前由「gcc 是否折叠」决定：`-O0` 给 1、`-O2` 给 0 ⇒ 见 ub_proof.sh 的举证）
CONST = [(1, 64), (1, 63), (1, 65), (1, 0), (1, 1), (-1, 63), (-1, 64),
         (255, 8), (7, 65), (-(1 << 63), 1)]
body, exp = [], []
for i, (a, n) in enumerate(CONST):
    # 常量形态（字面量 + 字面量计数）
    body.append('print(%s << %d, %s >> %d, %s >>> %d)'
                % (lit(a), n, lit(a), n, lit(a), n))
    exp.append(triple(a, n))
    # 运行期形态（变量）
    body.append('var c%d = %s' % (i, lit(a)))
    body.append('var d%d = %d' % (i, n))
    body.append('print(c%d << d%d, c%d >> d%d, c%d >>> d%d)' % (i, i, i, i, i, i))
    exp.append(triple(a, n))
LEGAL.append(('ok_const_vs_runtime', '\n'.join(body), '\n'.join(exp)))

# ⑤ 位运算族回归（`&`/`|`/`^`/`~` 不受本次改动影响）
body = ['print(7 & 3, 7 | 8, 7 ^ 5, ~0, ~(0 - 1))',
        'var x = 0 - 1',
        'print(x & 255, x | 0, x ^ x, ~x)']
exp = '%d %d %d %d %d\n%d %d %d %d' % (7 & 3, 7 | 8, 7 ^ 5, ~0, ~(-1),
                                       (-1) & 255, (-1) | 0, (-1) ^ (-1), ~(-1))
LEGAL.append(('ok_bitops_regress', '\n'.join(body), exp))

# ⑥ 链式/组合（确保收敛没有破坏正常路径）
body = ['var a = 1', 'var n = 40',
        'print((a << n) >> 20, (a << 40) >>> 39)',
        'var m = 63', 'print(a << m, (a << m) >> m)']
exp = '%d %d\n%d %d' % (px_shr(px_shl(1, 40), 20), px_shru(px_shl(1, 40), 39),
                        px_shl(1, 63), px_shr(px_shl(1, 63), 63))
LEGAL.append(('ok_chain', '\n'.join(body), exp))

# ---- 拒绝侧：(label, .px 源码, 期望 `R1003|消息体`) ----
REJECT = []
OPNAME = {'<<': '左移', '>>': '右移', '>>>': '无符号右移'}
NEGS = [('rej_shl_neg1', '<<', -1), ('rej_shr_neg1', '>>', -1), ('rej_shru_neg1', '>>>', -1),
        ('rej_shl_neg64', '<<', -64), ('rej_shr_neg64', '>>', -64),
        ('rej_shru_neg64', '>>>', -64),
        ('rej_shl_negmin', '<<', -(1 << 63)), ('rej_shru_negmin', '>>>', -(1 << 63))]
for label, op, n in NEGS:
    body = 'var a = 8\nvar n = %s\nprint(a %s n)' % (lit(n), op)
    REJECT.append((label, body, 'R1003|%s 计数不能为负数，实际是 %d' % (OPNAME[op], n)))
# 常量形态（字面量负计数 ⇒ 同码同文）
REJECT.append(('rej_shl_const_neg', 'print(8 << (0 - 1))',
               'R1003|左移 计数不能为负数，实际是 -1'))


def emit(outdir):
    HEAD = [
        '# ⚠️ 本文件由 gen_cases.py 生成（单一事实源）—— 手改会被第 1 层的漂移检测判红',
        '# M217 驱动器：按 args()[1] 分派（一次编译覆盖全量用例）',
        'a = args()',
        'sel = ""',
        'if len(a) >= 2:',
        '    sel = a[1]',
        '',
        'def run_case(sel):',
    ]
    lines = list(HEAD)
    rows = []
    for label, body, exp in LEGAL:
        lines.append('    if sel == "%s":' % label)
        lines += ['        ' + ln for ln in body.strip().splitlines()]
        lines.append('        return 0')
        rows.append((label, 'ok', exp))
    for label, body, exp in REJECT:
        lines.append('    if sel == "%s":' % label)
        lines += ['        ' + ln for ln in body.strip().splitlines()]
        lines.append('        return 0')
        rows.append((label, 'rej', exp))
    lines += ['    print("NO-SUCH-CASE: " + sel)', '    return 2', '',
              'exit(run_case(sel))']
    with open(os.path.join(outdir, 'cases.px'), 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines) + '\n')
    with open(os.path.join(outdir, 'CASES.tsv'), 'w', encoding='utf-8') as f:
        for label, kind, exp in rows:
            # ⚠️ 期望可能是**多行**（扫描类语料 64 行输出）⇒ TSV 必须单行：
            #   把换行转义成字面 `\n`，由 three_tracks.py 反转义。
            f.write('%s\t%s\t%s\n' % (label, kind, exp.replace('\\', '\\\\').replace('\n', '\\n')))
    return len(LEGAL), len(REJECT)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--out', default=os.path.dirname(os.path.abspath(__file__)))
    a = ap.parse_args()
    nok, nrej = emit(a.out)
    print('生成：合法侧 %d · 拒绝侧 %d ⇒ %s' % (nok, nrej, a.out))
    return 0


if __name__ == '__main__':
    sys.exit(main())
