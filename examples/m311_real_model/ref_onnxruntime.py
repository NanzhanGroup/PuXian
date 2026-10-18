#!/usr/bin/env python3
# M311 · C3 独立参考 —— onnxruntime（业界实现）跑同一个真模型、同一份输入
# 用法：PYTHONPATH=/opt/fw python3 ref_onnxruntime.py <model.onnx>
# 输出：DIMS ... / HEX <float32 小端字节的 hex>（与 PuXian 探针同格式，便于逐字节对拍）
#
# 为什么是独立参考：本仓自己的 op 级参考（m306 的 quant_ref.py）与 C 侧共享同一份 spec 理解；
#   onnxruntime 是完全独立的第三方实现 ⇒ 它能发现「我们一起理解错了」这种系统性错误。
import sys

import numpy as np

sys.path.insert(0, "/opt/fw")
import onnxruntime as ort  # noqa: E402

model = sys.argv[1]
so = ort.SessionOptions()
so.intra_op_num_threads = 1          # 与 PuXian 单线程执行对齐（避免归约顺序差异）
so.inter_op_num_threads = 1
so.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL
sess = ort.InferenceSession(model, so, providers=["CPUExecutionProvider"])

ids = np.array([[101, 2023, 1029, 102]], dtype=np.int64)
mask = np.ones((1, 4), dtype=np.int64)
tt = np.zeros((1, 4), dtype=np.int64)
out = sess.run(None, {"input_ids": ids, "attention_mask": mask, "token_type_ids": tt})[0]

print("DIMS", list(out.shape))
print("NBYTES", out.nbytes)
print("HEX", out.astype("<f4").tobytes().hex())
