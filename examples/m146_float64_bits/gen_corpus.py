#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_corpus.py — m146 语料生成器（**唯一事实源**）

语料格式（每行一条，`|` 分隔，两侧（Go 真值 / PuXian 移植）读**同一份** corpus.txt）：

    v|<十进制文本>      按 float64 解析（strconv.ParseFloat / strtod）
    i|<整数文本>        int64 → float64（Go `float64(i)` / PuXian `float(i)`）
    b|<16位十六进制>    **位模式**直接构造（math.Float64frombits / bits_to_float64）

`v`/`b` 两类是**互补**的：`v` 只能表达"能写成十进制文本的数"，`b` 才能表达
NaN / ±Inf / 非规格化数 / -0.0 —— 覆盖浮点**全域**必须两者都有。

用法：
    python3 gen_corpus.py          # 写 corpus.txt
    python3 gen_corpus.py --check  # 只校验 corpus.txt 与生成器一致（verify.sh 用）
"""
import os
import random
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "corpus.txt")

# ── 口岸十进制值 ──
VALS = [
    "0", "-0.0", "1", "-1", "0.5", "0.1", "-0.1", "1.5", "-1.5",
    "3.14159265358979", "0.3333333333333333", "123456789.123456789",
    "1e-300", "1e-310", "4.9e-324", "5e-324", "1e300", "-1e300",
    "1.7976931348623157e308", "-1.7976931348623157e308",
    "2.2250738585072014e-308", "2.225073858507201e-308",
    "1e21", "1e-6", "1e-7", "1e20", "9.999999999999999e20",
    "1.0000000000000002", "0.9999999999999999",
    "9007199254740992", "9007199254740993",
]

# ── 整数 ──
INTS = [
    "0", "1", "-1", "2", "-2", "255", "9007199254740992", "9007199254740993",
    "-9007199254740993", "9223372036854775807", "-9223372036854775807",
    "1000000007", "-123456789012345",
]

# ── 位模式口岸（NaN / Inf / 非规格化 / ±0 / 边界）──
PORTS = [
    0x0000000000000000,  # +0.0
    0x8000000000000000,  # -0.0
    0x0000000000000001,  # 最小非规格化数（4.9e-324）
    0x000FFFFFFFFFFFFF,  # 最大非规格化数
    0x0010000000000000,  # 最小规格化数（2.2250738585072014e-308）
    0x3FF0000000000000,  # 1.0
    0xBFF0000000000000,  # -1.0
    0x3FF0000000000001,  # 1.0 的下一个（1+2^-52）
    0x3FB999999999999A,  # 0.1
    0x3FD5555555555555,  # 1/3 的最近可表示值
    0x7FEFFFFFFFFFFFFF,  # 最大有限值
    0xFFEFFFFFFFFFFFFF,  # 最小有限值（负）
    0x7FF0000000000000,  # +Inf
    0xFFF0000000000000,  # -Inf
    0x7FF8000000000000,  # 静默 NaN
    0x7FF0000000000001,  # 信号 NaN（Go 读为 NaN）
    0xFFF8000000000001,  # 负 NaN（符号位无意义）
    0x43F0000000000000,  # 2^64
    0xC3F0000000000000,  # -2^64
    0x4330000000000000,  # 2^52
    0x3FE0000000000000,  # 0.5
]

N_RAND = 512


def gen():
    lines = []
    for s in VALS:
        lines.append("v|" + s)
    for s in INTS:
        lines.append("i|" + s)
    for u in PORTS:
        lines.append("b|%016x" % u)
    rnd = random.Random(20260920)
    for _ in range(N_RAND):
        # 一半：完全随机的 64 位（大量 NaN/Inf/巨值 —— 覆盖全域）；
        # 一半：由"人写得出"的十进制数换算（规格化值的常见形态）。
        if rnd.random() < 0.5:
            u = rnd.getrandbits(64)
        else:
            f = rnd.uniform(-1.0e6, 1.0e6) * (10.0 ** rnd.randint(-30, 30))
            u = struct.unpack("<Q", struct.pack("<d", f))[0]
        lines.append("b|%016x" % u)
    return "\n".join(lines) + "\n"


def main():
    want = gen()
    if "--check" in sys.argv:
        if not os.path.exists(OUT):
            print("corpus.txt 缺失", file=sys.stderr)
            return 1
        have = open(OUT, encoding="utf-8").read()
        if have != want:
            print("corpus.txt 与生成器不一致（重新运行 gen_corpus.py）", file=sys.stderr)
            return 1
        print("corpus.txt 与生成器一致（%d 行）" % want.count("\n"))
        return 0
    open(OUT, "w", encoding="utf-8").write(want)
    print("wrote corpus.txt (%d 行)" % want.count("\n"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
