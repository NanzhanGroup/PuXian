#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""derive_onnx_baseline.py —— 把新 ONNX 负载的**档行**派生进 5 个参考档（M310 · 丙2 W3）

## 为什么需要它（W0-③ 实测的直接后果）

`verify.sh` 的测量面 = **选定档的行**（`read_baseline()`）⇒ 新负载若只写生成器/清单、
不写档，门**永远不会测它**，且没有任何门会提醒（W0-①）。

而 5 个参考档对应的租机**已全部销毁** ⇒ 新负载**无法**在那 5 台机器上补测真实值。
W0-③ 实测跨机极差 3.9–5.0× ⇒ 拿本机值直接冒充实测会**假红/假绿**。

## 策略（W3 决策 ②：显式登记保守上界）

对每个档：

  基准值 = 本机实测值 × max(1, 本档 onnx_mm512 ÷ 本机 onnx_mm512) × SAFETY
           └─ 机型间的**实测**可比量（同一负载、同一口径）          └─ 安全系数 1.5

  为什么取 max(1, ratio)：合成档（CI 走的那档）= **逐负载取 max**；若快机档按比例
  写小值，合成 max 就会退到「快机值」⇒ 陌生 CI runner 稍慢即假红。取 ≥1 保证每个
  派生行都 ≥ 本机实测 ⇒ 合成上界恒有 SAFETY 倍余量（CI 实测不假红是 W3 唯一判据）。

  · 本机档（`--local-sig`）：**直接写实测值**（严格判据，不乘系数）；
  · 其它档：派生上界，行尾显式标注「派生上界」，并注明待该档机器复采后替换；
  · `SAFETY=1.5` 的理由：新负载的算子组合与 MatMul 的跨机缩放不完全同构；
    1.5 让「陌生机慢 1.5 倍」仍绿，同时仍能判「比参考机慢 3 倍以上」的失控回归；
    下限判据（快 10 倍 ⇒ 疑似未执行）不受影响（派生的上限越高，下限越不易误伤快机）。

⚠️ 本工具**只追加/替换带 `M310 新负载` 标记的行**，既有 12 行**逐字节不动**；重复运行幂等。

用法：
  python3 tools/derive_onnx_baseline.py --measured measured.tsv \
      --baselines baselines/ --local-sig intel-xeon-e5-2682-v4 [--safety 2.0] [--apply]

  measured.tsv 格式：<负载名>\t<秒>   （本机实测：taskset -c3 · 3 组 × 5 次取 min）
  不带 --apply 只打印将写入的行（dry-run）。
"""
import argparse
import os
import sys

TAG = "M310 新负载"
MM512 = "onnx_mm512"


def read_tier(path):
    """返回 (原始行列表)。注释/空行原样保留。"""
    with open(path, encoding="utf-8") as f:
        return f.readlines()


def data_rows(lines):
    return [x.rstrip("\n").split() for x in lines if x.strip() and not x.startswith("#")]


def mm512_of(lines):
    for r in data_rows(lines):
        if r[0] == MM512:
            return float(r[3])
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--measured", required=True)
    ap.add_argument("--baselines", required=True)
    ap.add_argument("--local-sig", required=True)
    ap.add_argument("--safety", type=float, default=1.5)
    ap.add_argument("--apply", action="store_true")
    a = ap.parse_args()

    measured = []
    with open(a.measured, encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            n, s = line.split("\t")[:2]
            measured.append((n, float(s)))
    if not measured:
        print("❌ measured 为空", file=sys.stderr)
        return 2

    local_path = os.path.join(a.baselines, a.local_sig + ".tsv")
    if not os.path.isfile(local_path):
        print("❌ 本机档不存在：%s" % local_path, file=sys.stderr)
        return 2
    local_mm = mm512_of(read_tier(local_path))
    if not local_mm:
        print("❌ 本机档缺 %s 行 ⇒ 无法做跨机比例换算" % MM512, file=sys.stderr)
        return 2

    tiers = sorted(x for x in os.listdir(a.baselines) if x.endswith(".tsv"))
    print("══ W3 派生（本机 %s · %s=%.4f s · SAFETY=%.2f）══" % (a.local_sig, MM512, local_mm, a.safety))
    for t in tiers:
        path = os.path.join(a.baselines, t)
        lines = read_tier(path)
        # 幂等：先摘掉上一轮带标记的行（只动带 TAG 的行）
        kept = [x for x in lines if not (x.strip() and not x.startswith("#") and TAG in x)]
        tier_mm = mm512_of(kept)
        if tier_mm is None:
            print("  ⚠️ %s 缺 %s 行 ⇒ 跳过" % (t, MM512))
            continue
        ratio = tier_mm / local_mm
        factor = 1.0 if t == a.local_sig + ".tsv" else max(1.0, ratio) * a.safety
        new = []
        for n, s in measured:
            v = s * factor
            if factor == 1.0:
                note = "M310 新负载 · 本机实测（3 组 × 5 次 min · taskset -c3）"
            else:
                scale = max(1.0, ratio)
                note = ("M310 新负载 · **派生上界**（本机实测 %.4fs × 安全系数 %.1f；"
                        "档位比例 %.3f ⇒ 取 %.3f；该档机器已销毁，待复采替换）"
                        % (s, a.safety, ratio, scale))
            new.append("%-14s %-8s %-6s %-10.4f %s\n" % (n, "onnx", "s", v, note))
        print("  %-28s ratio=%.3f factor=%.3f" % (t, ratio, factor))
        for line in new:
            print("    " + line.rstrip())
        if a.apply:
            if kept and not kept[-1].endswith("\n"):
                kept[-1] += "\n"
            with open(path, "w", encoding="utf-8") as f:
                f.writelines(kept)
                f.writelines(new)
    return 0


if __name__ == "__main__":
    sys.exit(main())
