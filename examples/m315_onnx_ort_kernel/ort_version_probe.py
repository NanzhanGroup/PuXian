#!/usr/bin/env python3
# ══════════════════════════════════════════════════════════════════════════
# ort_version_probe.py —— 「这个 onnxruntime 版本还有那个 MatMulInteger 偏差吗？」
#
# 用途（M315 补 · 2026-10-20）：
#   `check_truth.py ort` 只回答「本机这一版 ORT 的表现如何」。
#   而报告上游之前必须先回答**另一个问题**：**是不是新版本已经修好了？**
#   —— 否则对方第一句就是「请升级到最新版」，白跑一轮。
#   本探针就是为这一问准备的：**同一份 devices/ 装置，换一个 ORT 版本重跑**。
#
# 与门里那两个脚本的分工（三者不重复）：
#   gen_devices.py   生成装置 + 规范真值（真值 = 规范定义，可手算）
#   check_truth.py   门内判据载体（ORT 侧 / PuXian 侧，双向登记核对）
#   本文件           跨版本复核（**不进任何门**：CI 只有一个 ORT 版本，装第二个没有意义）
#
# 依赖：**只要 numpy + onnxruntime**（刻意不依赖 onnx —— 目标是「换个 ORT 版本」而不是重建装置）
#
# 用法：
#   python3 ort_version_probe.py [devices_dir]
#   # 换版本：用独立解释器/venv，例如
#   #   python3.12 -m pip install onnxruntime numpy
#   #   python3.12 ort_version_probe.py
#   # 对照本机默认：PYTHONPATH=/opt/fw python3 ort_version_probe.py
#
# 判据（脚本自己不做"通过/失败"判定，只逐装置打印差异）：
#   == 规范            ⇒ 本版本在该装置上正确
#   ✗ n/m 元素错 ...   ⇒ 本版本偏离规范（首处 + 差值）
#   两个优化级都跑一遍（ORT_DISABLE_ALL / default）：
#   若两档结论一致 ⇒ 病灶在 kernel；若不一致 ⇒ 在图优化器。这条区分历史上救过一次误判。
# ══════════════════════════════════════════════════════════════════════════
import json
import os
import sys

import numpy as np
import onnxruntime as ort

D = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "devices")


def load_ref(path):
    """装置真值：`.ref.txt` 里的 HEX 行 = int32 小端序列（由 gen_devices.py::ref_manual 独立算得）。"""
    for line in open(path, encoding="utf-8"):
        if line.startswith("HEX "):
            return np.frombuffer(bytes.fromhex(line[4:].strip()), dtype="<i4").astype(np.int64)
    raise SystemExit("no HEX line in " + path)


def main():
    idx = json.load(open(os.path.join(D, "index.json"), encoding="utf-8"))
    print("onnxruntime %s   (devices: %s)" % (ort.__version__, D))
    for opt in ("ORT_DISABLE_ALL", "default"):
        print("--- graph_optimization_level = %s" % opt)
        so = ort.SessionOptions()
        so.log_severity_level = 3
        if opt == "ORT_DISABLE_ALL":
            so.graph_optimization_level = ort.GraphOptimizationLevel.ORT_DISABLE_ALL
        for dev in idx["devices"]:
            n = dev["name"]
            ref = load_ref(os.path.join(D, n + ".ref.txt"))
            A = np.frombuffer(bytes.fromhex(dev["A_hex"]), np.uint8).reshape(dev["M"], dev["K"])
            B = np.frombuffer(bytes.fromhex(dev["B_hex"]), np.int8).reshape(dev["K"], dev["N"])
            sess = ort.InferenceSession(os.path.join(D, n + ".onnx"), so, providers=["CPUExecutionProvider"])
            got = sess.run(None, {"A": A, "B": B})[0].astype(np.int64).ravel()
            diff = got - ref
            bad = np.nonzero(diff)[0]
            if bad.size == 0:
                print("   %-16s == 规范 (%d 元素)" % (n, ref.size))
            else:
                i = int(bad[0])
                print("   %-16s ✗ %d/%d 元素错  首处 [%d]: ORT=%d 规范=%d 差=%d"
                      % (n, bad.size, ref.size, i, got[i], ref[i], diff[i]))


if __name__ == "__main__":
    main()
