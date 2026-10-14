#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gelu_tiers.py —— M308 门的「Gelu 两档可判异」判据（**独立于三方对拍**）。

为什么这一条要单独存在（M308-PLAN §三.S1 的硬判据）：
    「`approximate` 两档必须都在场**且可判异**」。把两档写成同一式之后，模型照样
    "能跑"、形状照样对、三方对拍照样有一份手算真值 —— **只要那份真值也是按同一式
    算的，三方一致地错**（M156 §38.4 的原话）。⇒ 只有「两档互异」这一条能戳穿它。

因此本脚本刻意**不复用** `exec_ref.py` 的比较逻辑，也**不读** `expected.json`：
直接对 C 侧 dump 做两件事 —— ① 闭式复算（另一份纯 Python 的 Gelu，用 `math.erf`
/`math.tanh`）· ② 逐元素判异。它和 exec_ref 是**两条独立的腿**：负控 NC-D 会把
exec_ref 的比较打成失明，此时本判据必须**仍然红**（跨判据冗余，不是单点）。

判据（每条都能单独变红）：
  ① G1 == 0.5·x·(1+erf(x/√2))            （erf 精确档，atol 2e-6）
  ② G2 == 0.5·x·(1+tanh(√(2/π)·(x+0.044715x³)))（tanh 近似档，atol 2e-6）
  ③ 逐元素 |G1-G2| > 1e-5（**两档不许等价** —— 本脚本存在的唯一理由）

用法：python3 tools/gelu_tiers.py <C 侧 dump.json> <feeds.json>
退出码：0 全绿 / 1 判红 / 2 用法或输入缺失（**取不到判据 ≠ 判据满足**，一律非 0）
"""
import json
import math
import struct
import sys

ATOL = 2e-6            # fp32 dump 的舍入在 ~3e-7 量级，2e-6 留 6× 余量
MIN_GAP = 1e-5         # 两档在本门语料上的最小可判异间距（实测 ~9.8e-5，留 10× 余量）


def f32_list(hexs):
    """C 侧 dump 是 fp32 的十六进制串 ⇒ 解回 float（**按 dump 的实际位型**解，不猜）。"""
    return list(struct.unpack("<%df" % (len(hexs) // 8), bytes.fromhex(hexs)))


def gelu_erf(x):
    return 0.5 * x * (1.0 + math.erf(x / math.sqrt(2.0)))


def gelu_tanh(x):
    c = math.sqrt(2.0 / math.pi)
    return 0.5 * x * (1.0 + math.tanh(c * (x + 0.044715 * x * x * x)))


def main():
    if len(sys.argv) < 3:
        print("用法：gelu_tiers.py <C 侧 dump.json> <feeds.json>")
        return 2
    dump, feeds = sys.argv[1], sys.argv[2]
    try:
        d = json.load(open(dump))
        x = [float(v) for v in json.load(open(feeds))["X"]["f32"]]
    except Exception as e:                                    # 判据缺失 ⇒ 非 0
        print("  ❌ 取不到判据（dump 或 feeds 读不到 X）：%r" % (e,))
        return 2
    for nm in ("G1", "G2"):
        if nm not in d.get("outputs", {}):
            print("  ❌ 取不到判据：dump 里没有输出 %s" % nm)
            return 2
    g1 = f32_list(d["outputs"]["G1"]["hex"])
    g2 = f32_list(d["outputs"]["G2"]["hex"])
    if len(g1) != len(x) or len(g2) != len(x) or not x:
        print("  ❌ 判据形状不符：|X|=%d |G1|=%d |G2|=%d" % (len(x), len(g1), len(g2)))
        return 2

    bad = 0

    def ck(cond, label):
        nonlocal bad
        print("  %s %s" % ("✅" if cond else "❌", label))
        if not cond:
            bad += 1

    w1 = max(abs(a - gelu_erf(v)) for a, v in zip(g1, x))
    w2 = max(abs(a - gelu_tanh(v)) for a, v in zip(g2, x))
    ck(w1 <= ATOL, "① G1（无属性）== erf 闭式：%d 点最大绝对误差 %.3g（阈值 %.1g）"
       % (len(x), w1, ATOL))
    ck(w2 <= ATOL, "② G2（approximate=\"tanh\"）== tanh 闭式：%d 点最大绝对误差 %.3g（阈值 %.1g）"
       % (len(x), w2, ATOL))
    gaps = [abs(a - b) for a, b in zip(g1, g2)]
    small = [i for i, g in enumerate(gaps) if g <= MIN_GAP]
    ck(not small,
       "③ **两档逐元素可判异**：最小间距 %.3g（阈值 %.1g）%s"
       % (min(gaps), MIN_GAP,
          "" if not small else " ← 下标 %s 上两档等价（写成同一式？）" % small))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
