#!/usr/bin/env python3
# M311 · C3 数值对拍 + 漂移判据
# 用法：compare.py <px.out> <ref.out> <baseline.tsv>
# 退出码：0 全部判据通过 / 1 有判据不通过（并逐条打印为什么）
#
# 判据分两类，**别混为一谈**（这是本轮最重要的口径）：
#   ① **硬判据**（等价性的下限）：非退化 —— 相关系数 ≥ 0.95 且尺度比 ∈ [0.9, 1.1]。
#      理由：量化模型跑错一个算子维度/轴，输出要么错位、要么整体缩放失真 ⇒ 这两条立刻炸。
#   ② **漂移判据**（回归守护）：max/mean 绝对误差不得比登记基线恶化 25%。
#      理由：当前与 onnxruntime **并非逐位一致**（缺陷 533，根因待定）——
#      在根因收口之前，把它当成常数基线守住「不许更差」，比假装等价更诚实。
import sys

import numpy as np

px_path, ref_path, base_path = sys.argv[1], sys.argv[2], sys.argv[3]


def hex_of(path):
    for line in open(path, encoding="utf-8", errors="replace"):
        if line.startswith("HEX "):
            return bytes.fromhex(line[4:].strip())
    raise SystemExit("缺少 HEX 行：%s" % path)


a = np.frombuffer(hex_of(px_path), dtype=np.float32).astype(np.float64)
b = np.frombuffer(hex_of(ref_path), dtype=np.float32).astype(np.float64)
if a.size != b.size:
    print("❌ 长度不同：PuXian %d ⇄ onnxruntime %d" % (a.size, b.size))
    raise SystemExit(1)

d = np.abs(a - b)
max_err = float(d.max())
mean_err = float(d.mean())
corr = float(np.corrcoef(a, b)[0, 1]) if a.size > 1 else 1.0
scale_ratio = float(np.abs(a).max() / max(1e-12, np.abs(b).max()))
rel = max_err / max(1e-12, float(np.abs(b).max()))

print("样本 %d 个 float32（seq=4 × 768）" % a.size)
print("max_abs_err = %.6f（相对参考峰值 %.4f%%）" % (max_err, rel * 100))
print("mean_abs_err = %.6f" % mean_err)
print("corr = %.6f · 尺度比 = %.6f" % (corr, scale_ratio))

base = {}
for line in open(base_path, encoding="utf-8"):
    if line.startswith("#") or not line.strip():
        continue
    k, v = line.split("\t")[:2]
    base[k.strip()] = float(v)

bmax, bmean = base.get("max_abs_err"), base.get("mean_abs_err")
if bmax is None or bmean is None:
    print("❌ 基线缺键（需要 max_abs_err / mean_abs_err）：%s" % base_path)
    raise SystemExit(1)

fails = 0
# ① 硬判据
if corr >= 0.95:
    print("✅ [硬] 相关系数 %.6f ≥ 0.95（输出不是乱码/错位）" % corr)
else:
    print("❌ [硬] 相关系数 %.6f < 0.95 —— 输出已经退化了" % corr)
    fails += 1
if 0.9 <= scale_ratio <= 1.1:
    print("✅ [硬] 尺度比 %.4f ∈ [0.9,1.1]（没有整体缩放失真）" % scale_ratio)
else:
    print("❌ [硬] 尺度比 %.4f 越界 [0.9,1.1]" % scale_ratio)
    fails += 1

# ② 漂移判据（+25%）
tmax, tmean = bmax * 1.25, bmean * 1.25
if max_err <= tmax:
    print("✅ [漂移] max_abs_err %.6f ≤ 基线 %.6f × 1.25 = %.6f" % (max_err, bmax, tmax))
else:
    print("❌ [漂移] max_abs_err %.6f > %.6f —— 与 onnxruntime 的偏差**变大了**" % (max_err, tmax))
    fails += 1
if mean_err <= tmean:
    print("✅ [漂移] mean_abs_err %.6f ≤ 基线 %.6f × 1.25 = %.6f" % (mean_err, bmean, tmean))
else:
    print("❌ [漂移] mean_abs_err %.6f > %.6f" % (mean_err, tmean))
    fails += 1

raise SystemExit(1 if fails else 0)
