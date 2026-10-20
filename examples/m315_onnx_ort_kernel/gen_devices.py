#!/usr/bin/env python3
"""M315 · MatMulInteger 装置生成器（**确定性**：不含随机，换机器/换时间结果相同）

为什么需要这个门（缺陷 533 的收尾）
------------------------------------
M311 立的判据是「PuXian ⇄ onnxruntime，误差不许比基线恶化 25%」。
M314 侦察发现：那 0.477 的误差**全部来自 onnxruntime 的 MatMulInteger(u8s8) kernel**，
PuXian 与规范一致（闭环实测 5e-6，float32 舍入级）。
⇒ 「拿一个会出错的实现当真值」是**判据本身的方向错误**。

本门把方向掉过来：
  · **真值 = 规范定义**（ONNX 规范：Σ(A[i,k]-a_zp)·(B[k,j]-b_zp)，整数域无饱和），
    由本文件的 `ref_manual()` **逐元素整数累加**算出，不经 numpy 矩阵乘、不经任何执行器；
  · **PuXian 必须与规范逐位一致**（硬判据，CI 上就能跑，不需要 onnxruntime）；
  · **onnxruntime 必须偏离规范**（在 A 含 >127 的装置上）—— 这条是**双向登记**：
    哪天 ORT 修好了，这一层会判红并逼我们更新文档，而不是让"已知偏离"永远挂着。

触发条件（M315 实测 1536 个组合）
----------------------------------
  · A(uint8) 全部 ≤ 127 ⇒ ORT **0 / 768 错**  ⇒ 「含 > 127」是**必要条件**
  · A(uint8) 含 > 127   ⇒ ORT **568 / 768 错**（K=1 除外：单元素没有向量化路径）
  ⚠️ 但**不是充分条件** —— 即 74% 触发，剩下的具体数据不触发：同一形状（K=3 N=4）
    枚举 **4860 组**候选才找到本门那个稳定触发的 `mmi_k3_ortbad`。
    所以门里只对**实测确认稳定偏离**的装置断言（`expect_ort_diverge`），
    其余标 null「只报告不断言」—— 硬断言会偶发判红（M274：会偶发判红的门等于没有门）。
  ⇒ 生产 int8 bge 有 72 个 MatMulInteger、`a_zero_point` 全部来自 DynamicQuantizeLinear
    （输出域 0..255，必然同时含 ≤127 与 >127 的值）⇒ 端到端**必然**受影响（已实测：
    把 72 个节点换成等价子图后，ORT 与自己相差 max 0.477362 = M311 记录的那个数）。
"""
import json
import os
import sys

import numpy as np
import onnx
from onnx import helper, TensorProto, numpy_helper

OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "devices")


def ref_manual(A, B, a_zp, b_zp):
    """ONNX MatMulInteger 的规范定义 —— 逐元素整数累加（不经矩阵乘、不经执行器）。"""
    M, K = A.shape
    N = B.shape[1]
    out = np.zeros((M, N), dtype=np.int64)
    for i in range(M):
        for j in range(N):
            acc = 0
            for k in range(K):
                acc += (int(A[i, k]) - int(a_zp)) * (int(B[k, j]) - int(b_zp))
            out[i, j] = acc
    return out


def ref_alt(A, B, a_zp, b_zp):
    """第二份规范实现（不同构：先各自减去零点再点积）—— 与 ref_manual 互校。"""
    a = (A.astype(np.int64) - int(a_zp))
    b = (B.astype(np.int64) - int(b_zp))
    return a @ b


# ── 装置表：固定数据（**手写，不用随机**）────────────────────────────
# B 的每一列都刻意混入正负，避免"全正"掩盖符号问题。
B8 = np.array([[1, -2, 3, -4],
               [-5, 6, -7, 8],
               [9, -10, 11, -12],
               [13, -14, 15, -16],
               [-17, 18, -19, 20],
               [21, -22, 23, -24],
               [-25, 26, -27, 28],
               [29, -30, 31, -32]], dtype=np.int8)

# ⚠️ **关于 `expect_ort_diverge`（这是本门最容易写错的一处判据）**
#   ORT 的偏差是**数据依赖**的：A 含 >127 是**必要条件**（A 全 ≤127 时实测 0/768 错），
#   但不是充分条件（实测 568/768 = 74% 触发；K=3 N=4 形状下枚举 4860 组候选，
#   只有特定数据组合会错）。⇒ 「A 含 >127 就必须偏离」作为判据会**偶发判红**。
#   所以：只有**实测确认稳定偏离**的装置才标 true（跨 session 各 10 次、结果唯一）；
#   其余标 None = **只报告不断言**。双向核对仍成立：true 的哪天不偏离了 ⇒ 判红（登记过期）。
DEVICES = [
    # name             A(row-major)                                         B         a_zp b_zp  说明                                          expect_ort_diverge
    ("mmi_k3_ortbad", [[188, 152, 1], [217, 74, 65]],
     [[36, -30, 1, -101], [-21, 57, -53, -95], [94, 82, -121, -12]],       114, 0,
     "⭐ 实测确定触发 ORT 偏差（错 [0,3] 一个元素 · 差 660 · 跨 session 10/10 稳定）", True),
    ("mmi_k3_hi",  [[188, 152, 1], [217, 74, 65]],                          B8[:3], 114, 0,
     "生产形态 · A 含 >127 · K=3（本数据**不触发** ORT 偏差 —— 证明它是数据依赖的）", None),
    ("mmi_k3_lo",  [[40, 52, 1], [17, 74, 65]],                             B8[:3], 114, 0,
     "对照：A 全 ≤127 · K=3", False),
    ("mmi_k8_hi",  [[200, 201, 202, 203, 204, 205, 206, 207],
                    [255, 128, 129, 130, 131, 132, 133, 134]],              B8, 114, 0,
     "A 含 >127 · K=8（本数据不触发）", None),
    ("mmi_k8_lo",  [[100, 101, 102, 103, 104, 105, 106, 107],
                    [127, 0, 1, 2, 3, 4, 5, 6]],                            B8, 114, 0,
     "对照：A 全 ≤127 · K=8", False),
    ("mmi_k16_hi", [[130 + i for i in range(16)],
                    [255 - i for i in range(16)]],                          None, 128, -7,
     "A 含 >127 · K=16 · a_zp=128 b_zp=-7（本数据**触发** ORT 偏差）", True),
    ("mmi_k1_hi",  [[200], [255]],                                          None, 100, 0,
     "边界：K=1（ORT 正确 —— 单元素无向量化路径）", False),
]


def main():
    os.makedirs(OUT, exist_ok=True)
    index = []
    for name, arows, B, a_zp, b_zp, note, expect_diverge in DEVICES:
        A = np.array(arows, dtype=np.uint8)
        K = A.shape[1]
        BB = np.array(B, dtype=np.int8) if B is not None else np.array(
            [[(k * 7 + j * 3) % 251 - 125 for j in range(4)] for k in range(K)], dtype=np.int8)
        t1 = ref_manual(A, BB, a_zp, b_zp)
        t2 = ref_alt(A, BB, a_zp, b_zp)
        assert np.array_equal(t1, t2), "%s：两份规范真值不一致" % name

        g = helper.make_graph(
            [helper.make_node("MatMulInteger", ["A", "B", "a_zp", "b_zp"], ["Y"])], name,
            [helper.make_tensor_value_info("A", TensorProto.UINT8, list(A.shape)),
             helper.make_tensor_value_info("B", TensorProto.INT8, list(BB.shape))],
            [helper.make_tensor_value_info("Y", TensorProto.INT32, list(t1.shape))],
            [numpy_helper.from_array(np.array(a_zp, np.uint8), "a_zp"),
             numpy_helper.from_array(np.array(b_zp, np.int8), "b_zp")])
        m = helper.make_model(g, opset_imports=[helper.make_opsetid("", 13)])
        # onnx 新版默认 IR version 12，onnxruntime 1.19.2 最高支持 10 ⇒ 不降就加载不了
        m.ir_version = 10
        onnx.save(m, os.path.join(OUT, name + ".onnx"))

        open(os.path.join(OUT, name + ".in.txt"), "w").write(
            "IN A dtype=uint8 shape=[%d, %d] hex=%s\nIN B dtype=int8 shape=[%d, %d] hex=%s\n"
            % (A.shape[0], K, A.tobytes().hex(), K, 4, BB.tobytes().hex()))
        open(os.path.join(OUT, name + ".ref.txt"), "w").write(
            "OUT Y dtype=int32 shape=[%d, 4]\nHEX %s\n"
            % (A.shape[0], t1.astype("<i4").tobytes().hex()))

        index.append({
            "name": name, "K": int(K), "M": int(A.shape[0]), "N": 4,
            "a_zero_point": int(a_zp), "b_zero_point": int(b_zp),
            "A_has_gt127": bool(int(A.max()) > 127), "note": note,
            "expect_ort_diverge": expect_diverge,
            "A_hex": A.tobytes().hex(), "B_hex": BB.tobytes().hex(),
            "truth": t1.tolist(),
        })
        print("%-14s K=%-2d A>127=%-5s a_zp=%-3d b_zp=%-3d ORT期望=%-5s 真值 %s" % (
            name, K, str(int(A.max()) > 127), a_zp, b_zp,
            {True: "偏离", False: "一致", None: "不断言"}[expect_diverge], t1.ravel().tolist()))

    # ⚠️ ensure_ascii=False 是**必须**的：默认 True 会把中文转成 \uXXXX，
    #    而门里用 grep 核对「真值来源声明」⇒ 转义后判据永远找不到（假红）。
    json.dump({"op": "MatMulInteger", "dtype_pair": "u8s8",
               "truth_source": "ONNX 规范定义，逐元素整数累加（gen_devices.py::ref_manual）",
               "devices": index},
              open(os.path.join(OUT, "index.json"), "w", encoding="utf-8"),
              indent=1, ensure_ascii=False)
    print("\n已写出 %d 个装置到 %s" % (len(index), OUT))


if __name__ == "__main__":
    main()
