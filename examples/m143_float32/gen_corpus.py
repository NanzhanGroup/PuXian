#!/usr/bin/env python3
# M143 语料生成器（确定性）：float32 值族 / 位模式 / Go JSON 文本 / float32 运算的对照输入。
#
# 语料行格式（两侧**同一份**文件；真值由 Go 本尊产出，见 truth/main.go）：
#   v <十进制字面量>         —— 一个 float64 值（两侧各自 strtod/atof 解析，逐位一致）
#   b <8 位十六进制>         —— 一个 float32 **位模式**（走 bits_to_float32 / Float32frombits）
#   m <十进制> <十进制>      —— 二元运算对（float32 的 * + / 三条）
#   d <十进制> <十进制> <n>  —— n 次累加（float32 += 逐步收口；验证「每步 f32 收口」纪律）
#
# 为什么要随机位模式：float32 的**最短往返**（32 位 Ryu）在边界上极少数取值会出现
#   「多个最短候选」，这正是移植最容易分叉的地方 —— 用固定种子的伪随机位模式扫一遍
#   （含 NaN / Inf / 次正规区间）比"手挑几个漂亮值"可信得多（m142 quoteChar 全 256 取值的同款思路）。
import random
import sys

OUT = "corpus.txt"

TARGETED_V = [
    "0.0", "-0.0", "1.0", "-1.0", "0.5", "-0.5", "0.1", "-0.1", "0.2", "0.3",
    "0.3333333333333333", "0.33333334", "2.5", "3.25", "-7.75",
    "1e-45", "1.4e-45", "1.1754943508222875e-38", "-1.1754943508222875e-38",
    "3.4028234663852886e+38", "-3.4028234663852886e+38", "1e-6", "9.999999974752427e-7",
    "1e-7", "1e20", "1e21", "9.999999e20", "1e22", "-1e22", "1e-4", "1e-5",
    "123456.0", "1234567.0", "123456789.123", "16777216.0", "16777217.0",
    "0.000001", "0.0000001", "1e15", "1e16", "2.0e-8", "-2.5e-8",
]

# float32 位模式：定点覆盖（零 / 次正规 / 正规边界 / 大值 / Inf / NaN 各种载荷）
TARGETED_B = [
    "00000000", "80000000", "00000001", "80000001", "007fffff", "00800000",
    "3f800000", "bf800000", "3dcccccd", "3eaaaaab", "40000000", "40400000",
    "7f7fffff", "ff7fffff", "7f800000", "ff800000", "7fc00000", "ffc00000",
    "7fc00001", "7f800001", "7f9e3779", "ffffffff", "5d7fffff", "1e3ce508",
    "5f000000", "1e000001", "4b000000", "4b7fffff", "00000002", "00ffffff",
]

TARGETED_M = [
    ("0.1", "0.3"), ("1.0", "3.0"), ("3.0", "7.0"), ("1e20", "1e-20"),
    ("1.1754943508222875e-38", "2.0"), ("3.4028234663852886e+38", "0.5"),
    ("0.1", "1e20"), ("1e-45", "1e-45"), ("16777217.0", "1.0000001"),
    ("-0.0", "5.0"), ("0.1", "-5.0"), ("2.5", "4.0"),
    # ⚠️ 语料里**不出现** 0 除数：Go 的浮点除零是良定义（±Inf / NaN），而 PuXian 的
    #    `/` 对**浮点**除零也 `px_error` 杀进程（缺陷 118 已登记，见 README/CHEATSHEET）。
    #    该语义分歧单独跟踪，不混进本门（本门只对"同口径可达面"逐字节）。
]

TARGETED_D = [
    ("0.1", "0.2", "10"), ("1.0", "3.0", "7"), ("1e20", "-1e20", "3"),
    ("1.1754943508222875e-38", "1.1754943508222875e-38", "4"),
    ("-2.5e-8", "0.1", "100"),
]


def build_text():
    rnd = random.Random(20260918)   # 固定种子：语料可复现
    lines = []
    for v in TARGETED_V:
        lines.append("v " + v)
    for b in TARGETED_B:
        lines.append("b " + b)
    # 伪随机位模式：全 32 位空间均匀（含 NaN / Inf / 次正规）
    for _ in range(1024):
        lines.append("b %08x" % rnd.getrandbits(32))
    # 伪随机「正规区间」位模式（指数 1..254，避开 NaN/Inf/次正规）—— 最短往返的主力面
    for _ in range(512):
        bits = (rnd.randint(1, 254) << 23) | rnd.getrandbits(23)
        lines.append("b %08x" % bits)
    for a, b in TARGETED_M:
        lines.append("m " + a + " " + b)
    for a, b, n in TARGETED_D:
        lines.append("d " + a + " " + b + " " + n)
    return "\n".join(lines) + "\n"


def main():
    text = build_text()
    n = len(text.splitlines())
    if "--check" in sys.argv:
        # 门内用：**不写盘**，只校验已入库的 corpus.txt 与生成器一致（防止"改了生成器忘了重建"）
        try:
            cur = open(OUT, encoding="utf-8").read()
        except OSError as e:
            print("FAIL: 读不到 %s（%s）" % (OUT, e))
            return 1
        if cur != text:
            print("FAIL: %s 与 gen_corpus.py 的当前输出不一致（请重建并提交语料）" % OUT)
            return 1
        print("OK: %s 与生成器一致（%d 行）" % (OUT, n))
        return 0
    open(OUT, "w", encoding="utf-8").write(text)
    print("written %s (%d 行)" % (OUT, n))
    return 0


if __name__ == "__main__":
    sys.exit(main())
