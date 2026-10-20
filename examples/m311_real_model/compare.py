#!/usr/bin/env python3
# M311/M315 · C3 数值对拍 + 漂移判据（**口径已按 M314 的侦察结论修正**）
# 用法：compare.py <px.out> <ref_raw.out> <truth_fixed.npy> <baseline.tsv>
# 退出码：0 全部判据通过 / 1 有判据不通过（并逐条打印为什么） / 3 前提缺失
#
# ⚠️ 口径变更的原因（缺陷 533 的收尾 · M315）
# ------------------------------------------------------------------
# 修前的主判据是「PuXian ⇄ onnxruntime 的 max/mean 误差不许比基线恶化 25%」。
# M314 侦察证明：那 0.477 的偏差**全部来自 onnxruntime 的 MatMulInteger(u8s8) kernel**，
# 不是我们的。判别实验（把真模型 72 个 MatMulInteger 换成等价子图 Cast→Sub→MatMul→Cast，
# 绕开有 bug 的 kernel）：
#     ORT 原始 ⇄ 改写后 ORT ：max 0.477362 · mean 0.066037
#     PuXian   ⇄ ORT 原始   ：max 0.477362 · mean 0.066037   ← **逐位相同**
#     PuXian   ⇄ 改写后 ORT ：max 0.000005 · mean 0.000000   ← float32 舍入级
# ⇒ 拿一个会出错的实现当真值，会把正确的实现记成"有缺陷"。
#
# 现在的判据分两类，**别混为一谈**：
#   [A] **真值判据（主）**：PuXian ⇄ `truth_bge_fixed.npy`（绕开 ORT bug kernel 的等价子图输出）
#       —— max_abs_err ≤ 1e-4（实测 5e-6，两个数量级余量）。阈值**不依赖历史基线**：
#         真值就是真值，它不会因为某次改动而"变松"。
#   [B] **漂移判据（辅）**：PuXian ⇄ onnxruntime 原始 —— 不恶化登记基线 25%。
#       保留它的理由：万一将来 ORT 修好了而我们反而偏了，这条也会红；
#       同时它守住「相对第三方实现的整体一致性」这个便宜的性质。
#
# 真值文件是**入仓生成物**（12416 B），由 M314 侦察的改写子图产出；
# 生成方法见 docs/ONNX_ORT_KERNEL.md（可复算，但需要 102MB 生产模型 ⇒ 不进门）。
import sys

import numpy as np

px_path, ref_path, truth_path, base_path = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]

TRUTH_MAX = 1e-4          # 真值判据的绝对阈值（float32 在 12 层量化网络上的舍入级）


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

t = np.load(truth_path).astype(np.float64).ravel()
if t.size != a.size:
    print("❌ 真值文件长度不符：%d ⇄ PuXian %d（%s）" % (t.size, a.size, truth_path))
    raise SystemExit(1)

fails = 0


def stat(x, y, label):
    d = np.abs(x - y)
    mx, mean = float(d.max()), float(d.mean())
    corr = float(np.corrcoef(x, y)[0, 1]) if x.size > 1 else 1.0
    print("%-34s max=%.8f  mean=%.8f  corr=%.6f" % (label, mx, mean, corr))
    return mx, mean, corr


print("样本 %d 个 float32（seq=4 × 768）" % a.size)
print("")
print("── 三方关系（先说清谁是真值）──")
mx_ff, mean_ff, _ = stat(b, t, "onnxruntime 原始 ⇄ 可信真值")
mx_pt, mean_pt, corr_pt = stat(a, t, "**PuXian ⇄ 可信真值**")
mx_pr, mean_pr, corr_pr = stat(a, b, "PuXian ⇄ onnxruntime 原始")
print("")
print("ℹ️ 上面前两行的差（%.6f）就是 ORT kernel 的误差；第三行与它相等 ⇒ 偏差不在我们这边"
      % mx_ff)

print("")
print("── [A] 真值判据（主 · 阈值 %.0e，不依赖历史基线）──" % TRUTH_MAX)
if mx_pt <= TRUTH_MAX:
    print("✅ max_abs_err %.8f ≤ %.0e —— 与规范一致（float32 舍入级）" % (mx_pt, TRUTH_MAX))
else:
    print("❌ max_abs_err %.8f > %.0e —— 与可信真值偏离，**这是真回归**" % (mx_pt, TRUTH_MAX))
    fails += 1
if corr_pt >= 0.99999:
    print("✅ 相关系数 %.8f ≥ 0.99999" % corr_pt)
else:
    print("❌ 相关系数 %.8f < 0.99999" % corr_pt)
    fails += 1

print("")
print("── [B] 漂移判据（辅 · 相对 onnxruntime 原始，基线 +25%）──")
base = {}
for line in open(base_path, encoding="utf-8"):
    if line.startswith("#") or not line.strip():
        continue
    k, v = line.split("\t")[:2]
    base[k.strip()] = float(v)
bmax, bmean = base.get("max_abs_err"), base.get("mean_abs_err")
if bmax is None or bmean is None:
    print("❌ 基线缺键（需要 max_abs_err / mean_abs_err）：%s" % base_path)
    raise SystemExit(3)
tmax, tmean = bmax * 1.25, bmean * 1.25
if mx_pr <= tmax:
    print("✅ max_abs_err %.6f ≤ 基线 %.6f × 1.25 = %.6f" % (mx_pr, bmax, tmax))
else:
    print("❌ max_abs_err %.6f > %.6f —— 与 onnxruntime 的偏差变大了" % (mx_pr, tmax))
    fails += 1
if mean_pr <= tmean:
    print("✅ mean_abs_err %.6f ≤ 基线 %.6f × 1.25 = %.6f" % (mean_pr, bmean, tmean))
else:
    print("❌ mean_abs_err %.6f > %.6f" % (mean_pr, tmean))
    fails += 1

print("")
print("── 自洽：ORT 原始 ⇄ 可信真值的偏差必须与 [B] 的基线同量级 ──")
print("（否则说明真值文件与生产模型/输入对不上，[A] 的绿没有意义）")
if 0.25 <= mx_ff <= 1.0:
    print("✅ ORT 自身误差 max %.6f ∈ [0.25, 1.0] —— 真值文件与生产模型匹配" % mx_ff)
else:
    print("❌ ORT 自身误差 max %.6f ∉ [0.25, 1.0] —— 真值文件可能已过期或输入不符" % mx_ff)
    fails += 1

raise SystemExit(1 if fails else 0)
