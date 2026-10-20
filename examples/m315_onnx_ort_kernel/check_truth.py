#!/usr/bin/env python3
"""M315 · 判据层：PuXian ⇄ 规范真值 / onnxruntime ⇄ 规范真值

两个子命令（判据分开，因为它们的**可用性不同**：规范真值随时可算，onnxruntime 只在本地有）：

  px  <devices_dir> <pxout_dir>
      逐装置比 PuXian 输出 ⇄ 规范真值（`.ref.txt`）。
      **CI 上也能跑**（不需要 onnxruntime）⇒ 这是判据强度的实质提升。
      rc=0 全部逐位一致 / 1 有不一致 / 3 缺装置或缺输出

  ort <devices_dir>
      逐装置比 onnxruntime ⇄ 规范真值，按 index.json 的 `expect_ort_diverge` 做**双向登记核对**：
        · true  ⇒ **必须偏离**（实测确认稳定触发 ORT kernel 偏差的装置）
        · false ⇒ **必须一致**（对照）
        · null  ⇒ 只报告，不断言（ORT 的偏差是**数据依赖**的：A 含 >127 是必要条件、
                   不是充分条件 ⇒ 对这些装置硬断言会偶发判红，M274 的教训）
      任何一条不符 ⇒ 判红。
      ⚠️ 这条不只是"记录已知问题" —— 它同时是**登记过期判据**：
         哪天 ORT 修好了，true 的装置不再偏离 ⇒ 判红，逼我们更新文档与期望，
         而不是让"已知偏离"永远挂着（M293 的 MIGRATE_KNOWN 口径）。
      rc=0 符合登记 / 1 不符 / 3 缺 onnxruntime

判据为什么不能只看「PuXian ⇄ ORT」：M311 就是这么做的，于是把一个会出错的实现当真值，
把 PuXian 正确的 0.477 记成了「我们的数值缺陷」（缺陷 533 的全部由来）。
"""
import glob
import json
import os
import sys

import numpy as np


def load_ref(path):
    for line in open(path, encoding="utf-8"):
        if line.startswith("HEX "):
            return np.frombuffer(bytes.fromhex(line[4:].strip()), dtype="<i4").astype(np.int64)
    raise SystemExit("缺 HEX 行：%s" % path)


def load_index(d):
    return json.load(open(os.path.join(d, "index.json"), encoding="utf-8"))


def cmd_px(d, outd):
    idx = load_index(d)
    pass_n = fail_n = 0
    for dev in idx["devices"]:
        name = dev["name"]
        ref_p = os.path.join(d, name + ".ref.txt")
        px_p = os.path.join(outd, name + ".pxout")
        if not os.path.exists(ref_p):
            print("❌ %-12s 缺规范真值 %s" % (name, ref_p))
            fail_n += 1
            continue
        if not os.path.exists(px_p):
            print("❌ %-12s PuXian 没有输出（%s）" % (name, px_p))
            fail_n += 1
            continue
        ref = load_ref(ref_p)
        got = load_ref(px_p)
        if got.size != ref.size:
            print("❌ %-12s 长度不同：PuXian %d ⇄ 规范 %d" % (name, got.size, ref.size))
            fail_n += 1
            continue
        if np.array_equal(got, ref):
            print("✅ %-12s PuXian ⇄ 规范**逐位一致**（%d 元素）" % (name, ref.size))
            pass_n += 1
        else:
            diff = int((got != ref).sum())
            print("❌ %-12s PuXian 与规范有 %d/%d 个元素不同" % (name, diff, ref.size))
            bad = np.nonzero(got != ref)[0][:4]
            for i in bad:
                print("      [%d] PuXian %d ⇄ 规范 %d" % (i, got[i], ref[i]))
            fail_n += 1
    print("\nPuXian ⇄ 规范：%d 通过 / %d 失败" % (pass_n, fail_n))
    return 0 if fail_n == 0 else 1


def cmd_ort(d):
    try:
        import onnxruntime as ort
    except ImportError as e:                                  # noqa: BLE001
        print("onnxruntime 不可用（%s）⇒ 本层 SKIP（CI 常态；权威覆盖在本地全量门）" % e)
        return 3
    idx = load_index(d)
    so = ort.SessionOptions()
    so.graph_optimization_level = ort.GraphOptimizationLevel.ORT_DISABLE_ALL
    pass_n = fail_n = 0
    print("onnxruntime %s" % ort.__version__)
    for dev in idx["devices"]:
        name = dev["name"]
        ref = load_ref(os.path.join(d, name + ".ref.txt"))
        A = np.frombuffer(bytes.fromhex(dev["A_hex"]), dtype=np.uint8).reshape(dev["M"], dev["K"])
        B = np.frombuffer(bytes.fromhex(dev["B_hex"]), dtype=np.int8).reshape(dev["K"], dev["N"])
        s = ort.InferenceSession(os.path.join(d, name + ".onnx"), so, providers=["CPUExecutionProvider"])
        got = s.run(None, {"A": A, "B": B})[0].astype(np.int64).ravel()
        same = np.array_equal(got, ref)

        expect = dev.get("expect_ort_diverge")
        if expect is True:
            ok = not same
            verdict = ("偏离规范（登记：已知 ORT kernel 偏差，此处为稳定触发装置）" if ok
                       else "**与规范一致** ⇒ 登记可能已过期（ORT 或许已修好）")
        elif expect is False:
            ok = same
            verdict = ("与规范一致（对照）" if ok
                       else "**偏离规范** ⇒ 对照失守：触发条件判据需重查")
        else:
            ok = True
            verdict = ("偏离规范（不断言 · 数据依赖的已知现象）" if not same
                       else "与规范一致（不断言）")
        if ok:
            print("✅ %-14s %s" % (name, verdict))
            pass_n += 1
        else:
            print("❌ %-14s %s" % (name, verdict))
            if not same:
                print("      ORT %s ⇄ 规范 %s" % (got[:4].tolist(), ref[:4].tolist()))
            fail_n += 1
    print("\nORT 登记核对：%d 符合 / %d 不符" % (pass_n, fail_n))
    if fail_n:
        print("ℹ️ 若 ORT 全绿而本层判红：很可能是 ORT 修好了 ⇒ 更新 docs/ONNX_ORT_KERNEL.md 与本门期望")
    return 0 if fail_n == 0 else 1


if __name__ == "__main__":
    if len(sys.argv) < 3:
        raise SystemExit("用法：check_truth.py px <devices_dir> <pxout_dir> | ort <devices_dir>")
    if sys.argv[1] == "px":
        sys.exit(cmd_px(sys.argv[2], sys.argv[3]))
    if sys.argv[1] == "ort":
        sys.exit(cmd_ort(sys.argv[2]))
    raise SystemExit("未知子命令：%s" % sys.argv[1])
