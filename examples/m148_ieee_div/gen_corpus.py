#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_corpus.py — m148 语料生成器（**唯一事实源**）

本门三件语料（两侧读**同一份**，PuXian 侧 fidelity.px / Go 侧 truth/main.go）：

  ① corpus_div.txt   `<op>|<ahex16>|<bhex16>`      → 除/取模的结果**位模式**
       op=d → a / b         （浮点除法，IEEE 754）
       op=m → fmod(a, b)    （浮点取模 = Go math.Mod）
     产出行：`<op>|<ahex>|<bhex>|<结果16位hex 或 nan>`

  ② corpus_strnf.txt `<hex16>`                     → **不对拍**：`str()` 的**非有限值**文本是
     **语言约定**（`inf` / `-inf` / `nan`，有 `golden/s09` 钉着）——只做"约定检查"。
     （M148 一度把这一族改成 Go 的 `+Inf`/`-Inf`/`NaN`，被 s09 的 lexer/parser/codegen
      三个 golden 当场拦下；更根本的是**不自洽**：`str()` 在有限值上本来就不向 Go 的
      `%v` 看齐（`250.0` vs `250`）。Go 的口径由 ③ 的 `go_float_text` 提供。）

  ③ corpus_gv.txt    `<hex16>[|32]`                → 新增 native `go_float_text(f[, bits])`
     产出行：`<hex16>|<bits>|<Go fmt.Sprint 文本>`

  ④ corpus_str.txt   `<hex16>`                     → **不对拍**，只做往返性质检查（见 ② ）

为什么这个门必须存在（缺陷 118 / 132）：

  · **缺陷 118**：Go 的浮点换算是良定义的 —— `x/0.0` = ±Inf、`0.0/0.0` = NaN、
    `math.Mod(x, 0)` = NaN；而 PuXian 的 `/` 对零除数**一律 px_error 杀进程**
    ⇒ 移植任何 Go 浮点算式（比值、余弦、成功率、均值）都得先手写零判断，
    **表达不出**「÷0 → Inf / NaN」这条路。

  · **缺陷 132**：`str()` 的**有限**浮点文本是语言自身约定（"250.0"、固定区更宽），
    与 Go 的 `fmt.Sprint`（"250"、十进制指数 ≥6 即科学计数）**不同**。本门用新增
    native `go_float_text(f[, bits])` 把 **Go 的 `%v` 文本**也搬进来（字宽 32/64 两套
    最短往返），并把两侧**逐字节**钉死。

  · `str()` 的**非有限**值文本（NaN/+Inf/-Inf）则**对齐 Go**（libc 给 nan/inf）。

用法：
    python3 gen_corpus.py          # 写三份语料
    python3 gen_corpus.py --check  # 校验语料与生成器一致（verify.sh 用）
"""
import os
import random
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
F_DIV = os.path.join(HERE, "corpus_div.txt")
F_STR = os.path.join(HERE, "corpus_str.txt")
F_STRNF = os.path.join(HERE, "corpus_strnf.txt")
F_GV = os.path.join(HERE, "corpus_gv.txt")

# ── 口岸位模式：零 / 非有限 / 极值 / 常见值（判据集中在这批）──
SPECIAL = [
    "0000000000000000",   # +0.0
    "8000000000000000",   # -0.0
    "3ff0000000000000",   # 1.0
    "bff0000000000000",   # -1.0
    "4000000000000000",   # 2.0
    "7ff0000000000000",   # +Inf
    "fff0000000000000",   # -Inf
    "7ff8000000000000",   # NaN
    "0010000000000000",   # 最小规格化数
    "0000000000000001",   # 最小非规格化数
    "000fffffffffffff",   # 最大非规格化数
    "7fefffffffffffff",   # 最大有限值
    "3fb999999999999a",   # 0.1
    "bff999999999999a",   # -0.1
    "3fd5555555555555",   # 1/3 最近双精度
    "400921fb54442d18",   # π
]


def f64(h: str) -> float:
    return struct.unpack("<d", bytes.fromhex(h)[::-1])[0]


def h64(v: float) -> str:
    return bytes.hex(struct.pack("<d", v)[::-1])


def finite_pattern(rnd: random.Random) -> str:
    """伪随机**有限**位模式（指数取 [1,0x7FE]，避开 0/非规格化/Inf/NaN 的边界噪声）。"""
    mant = rnd.getrandbits(52)
    exp = rnd.randint(1, 0x7FE)
    u = (exp << 52) | mant
    return "%016x" % u


def any_pattern(rnd: random.Random) -> str:
    return "%016x" % rnd.getrandbits(64)


def lines():
    rnd = random.Random(20260920)
    pool = list(SPECIAL)
    for _ in range(24):
        pool.append(finite_pattern(rnd))
    for _ in range(12):
        pool.append(any_pattern(rnd))

    # ① 除 / 取模：口岸集**全对**（含 ÷0、0/0、Inf/Inf、NaN 全组合）
    div = []
    for a in SPECIAL:
        for b in SPECIAL:
            div.append("d|%s|%s" % (a, b))
            div.append("m|%s|%s" % (a, b))
    # ② 伪随机对（有限值优先，保证既有正常面也覆盖）
    for _ in range(120):
        a = finite_pattern(rnd)
        b = finite_pattern(rnd)
        div.append("d|%s|%s" % (a, b))
        div.append("m|%s|%s" % (a, b))
    for _ in range(40):
        div.append("d|%s|%s" % (any_pattern(rnd), any_pattern(rnd)))

    # ③ str() 文本：口岸 + 随机有限 + 随机任意（含 NaN/Inf/非规格化）
    st = list(pool)
    for _ in range(80):
        st.append(finite_pattern(rnd))
    for _ in range(60):
        st.append(any_pattern(rnd))

    # ② 非有限值（str() 与 Go 同文本的那一族）：各类 NaN 载荷 + ±Inf
    strnf = [
        "7ff0000000000000", "fff0000000000000",          # ±Inf
        "7ff8000000000000", "fff8000000000000",          # quiet NaN 两种符号
        "7ff0000000000001", "fff0000000000001",          # signaling NaN 两种符号
        "7fffffffffffffff", "ffffffffffffffff",          # 载荷全 1（符号不同）
        "7ff4000000000000", "fffa000000000000",          # 任意载荷
        "7ff0000000000002", "7ffdeadbeef00000",
    ]

    # ④ go_float_text：同池，另加 32 位字宽面（Go `fmt.Sprint(float32(v))`）
    gv = []
    for h in st:
        gv.append("%s|64" % h)
        gv.append("%s|32" % h)

    return div, st, strnf, gv


def main():
    div, st, strnf, gv = lines()
    want = {
        F_DIV: "\n".join(div) + "\n",
        F_STR: "\n".join(st) + "\n",
        F_STRNF: "\n".join(strnf) + "\n",
        F_GV: "\n".join(gv) + "\n",
    }
    if "--check" in sys.argv:
        bad = 0
        for p, text in want.items():
            if not os.path.exists(p):
                print("缺失 %s" % os.path.basename(p), file=sys.stderr)
                bad += 1
                continue
            cur = open(p, encoding="utf-8").read()
            if cur != text:
                print("%s 与生成器不一致（%d vs %d 字节）"
                      % (os.path.basename(p), len(cur), len(text)), file=sys.stderr)
                bad += 1
        if bad:
            return 1
        print("语料与生成器一致（div %d / str %d / strnf %d / gv %d 行）"
              % (len(div), len(st), len(strnf), len(gv)))
        return 0
    for p, text in want.items():
        open(p, "w", encoding="utf-8").write(text)
        print("written %s (%d lines)" % (os.path.basename(p), len(text.splitlines())))
    return 0


if __name__ == "__main__":
    sys.exit(main())
