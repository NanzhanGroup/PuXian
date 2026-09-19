#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_corpus.py — m147 语料生成器（**唯一事实源**）

语料格式（每行一条，`|` 分隔；两侧读**同一份** corpus.txt）：

    <dec>|v|<十进制文本>      f = float64(文本)（Go strconv.ParseFloat / PuXian float()）
    <dec>|b|<16位十六进制>    f = 位模式直接构造（math.Float64frombits / bits_to_float64）

产出行格式一律 `<dec>|<kind>|<txt>|<GO %.dec f 文本>`。

为什么这个门必须存在（缺陷 127）：
    语言里此前**没有** `%.Nf`。token-cache 侧三处需要 Go `fmt.Sprintf("%.2f")`
    文案（规则摘要的置信度 / 聚类阈值 / 探索日志），都是手搓定点
    （`floor(x*100+0.5)` ⇒ 四舍五入-**半向上**）。而 Go（与 glibc）对**精确并列**
    取 `round-half-even`：

        0.125 → Go "0.12"   手搓 "0.13"
        2.625 → Go "2.62"   手搓 "2.63"
        4.5 @0 → Go "4"     手搓 "5"

    ⇒ 只要语料里出现**可精确表示的 .x5**，手搓实现必然分叉。本门把「定点文本」
    与 Go 本尊逐字节钉死，并**刻意把并列值灌满**（它们才是判据）。

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

# ── 口岸值：**并列（half-way）值优先** —— 它们是本门的主判据 ──
TIE_VALS = [
    "0.125", "0.375", "0.625", "0.875", "0.0625", "0.03125", "0.0078125",
    "2.625", "4.125", "8.5", "0.5", "1.5", "2.5", "3.5", "4.5", "10.5",
    "-0.125", "-2.625", "-3.5", "-4.5", "1.005", "2.675", "0.145", "0.35", "0.45",
]
EDGE_VALS = [
    "0", "-0.0", "1", "-1", "0.1", "-0.1", "0.7", "0.56", "0.3333333333333333",
    "1e-7", "1e-6", "0.9999999", "123456789.123", "1e20", "12345678901234567168",
    "1.2345678901234567e19", "1e300", "-1e300", "1.7976931348623157e308",
    "2.2250738585072014e-308", "4.9e-324", "9007199254740992", "9007199254740993",
    "0.049999999999999996", "1.0000000000000002", "-0.0000001",
]

# ── 位模式口岸值（十进制文本写不出来的那些）──
BIT_PATS = [
    "7ff8000000000000",   # quiet NaN
    "fff8000000000000",   # -NaN
    "7ff0000000000001",   # signaling NaN
    "7ff0000000000000",   # +Inf
    "fff0000000000000",   # -Inf
    "8000000000000000",   # -0.0
    "0000000000000000",   # +0.0
    "0000000000000001",   # 最小非规格化数
    "000fffffffffffff",   # 最大非规格化数
    "0010000000000000",   # 最小规格化数
    "7fefffffffffffff",   # 最大有限值
    "3ff0000000000001",   # 1+eps
    "bff0000000000000",   # -1.0
    "3fd5555555555555",   # 1/3 的最近双精度
    "c0c3880000000000",   # -10000.0
    "4340000000000001",   # 9007199254740994.0
]

DECS = [0, 1, 2, 3, 4, 5, 6, 8, 17]
DECS_SPECIAL = [0, 1, 2, 3]


def go_fmt(v: float, dec: int) -> str:
    """Go `strconv.FormatFloat(v,'f',dec,64)` 的等价物（Python 的格式化即正确舍入、
    并列取偶 —— 与 glibc/Go 同规则；非有限值 Go 给 NaN/+Inf/-Inf）。"""
    if v != v:
        return "NaN"
    if v == float("inf"):
        return "+Inf"
    if v == float("-inf"):
        return "-Inf"
    if dec == 0:
        # Go 的 %.0f 不带小数点（Python 的 f"{v:.0f}" 同）
        return f"{v:.0f}"
    return f"{v:.{dec}f}"


def lines():
    out = []
    # ① 并列值 × 全部精度
    for t in TIE_VALS:
        v = float(t)
        for d in DECS:
            out.append(f"{d}|v|{t}|{go_fmt(v, d)}")
    # ② 口岸值 × 全部精度
    for t in EDGE_VALS:
        v = float(t)
        for d in DECS:
            out.append(f"{d}|v|{t}|{go_fmt(v, d)}")
    # ③ 位模式口岸值 × 小精度
    for h in BIT_PATS:
        v = struct.unpack("<d", bytes.fromhex(h)[::-1])[0]
        for d in DECS_SPECIAL:
            out.append(f"{d}|b|{h}|{go_fmt(v, d)}")
    # ④ 伪随机值（固定种子 ⇒ 语料可复现）：十进制文本用 repr（最短往返）
    rnd = random.Random(20260919)
    vals = []
    for _ in range(40):
        vals.append(rnd.uniform(0.0, 1.0))
    for _ in range(40):
        vals.append(rnd.uniform(-1e6, 1e6))
    for _ in range(20):
        vals.append(rnd.uniform(1e15, 1e21))
    for _ in range(20):
        vals.append(rnd.uniform(-1e-6, 1e-6))
    for i, v in enumerate(vals):
        d = DECS[i % len(DECS)]
        out.append(f"{d}|v|{repr(v)}|{go_fmt(v, d)}")
    # ⑤ 伪随机**位模式**（有限值）：覆盖任意指数/尾数组合
    rnd2 = random.Random(777)
    n = 0
    while n < 40:
        h = "%016x" % rnd2.getrandbits(64)
        v = struct.unpack("<d", bytes.fromhex(h)[::-1])[0]
        if v != v or v in (float("inf"), float("-inf")):
            continue
        d = DECS_SPECIAL[n % len(DECS_SPECIAL)]
        out.append(f"{d}|b|{h}|{go_fmt(v, d)}")
        n += 1
    return out


def main():
    body = lines()
    text = "\n".join(body) + "\n"
    if "--check" in sys.argv:
        if not os.path.exists(OUT):
            print("corpus.txt 缺失", file=sys.stderr)
            return 1
        cur = open(OUT, encoding="utf-8").read()
        if cur != text:
            print("corpus.txt 与生成器不一致（%d vs %d 字节）" % (len(cur), len(text)), file=sys.stderr)
            return 1
        print("corpus.txt 与生成器一致（%d 行）" % len(body))
        return 0
    open(OUT, "w", encoding="utf-8").write(text)
    print("written corpus.txt (%d lines)" % len(body))
    return 0


if __name__ == "__main__":
    sys.exit(main())
