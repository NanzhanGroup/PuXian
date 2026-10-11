#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_perf_models.py —— m267_perf（性能回归门）的 **ONNX 负载模型**生成器（M303 / S1）

## 为什么要有它

M267 建立时（M266/M267）的负载全是**纯语言负载**（fib28 / while_sum / mixed / jsonwb），
**一个 ONNX 项都没有** ⇒ 谁把 MatMul 改快改慢，**没有任何判据看得见**。
M303（路线 S1）把 ONNX 内核接进性能账本，本生成器负责产出那两张"账"的**输入语料**。

## 为什么模型不直接入库

1024³ 的 float32 矩阵 = 4 MiB，两个就是 8 MiB —— 入库会让仓库无谓膨胀，且二进制
不可 diff。改为**生成器入库 + 产物运行时生成到 work 目录**（与 m156/m157 的小语料
不同：那些是几 KB，值得入库）。

## 为什么用形状确定的随机值

  · **确定性**：固定种子 ⇒ 同一份代码永远产出同一份模型 ⇒ 基线可比、可复现；
  · **数值无害**：MatMul 的**耗时与数值无关**（无分支、无短路），取值只影响输出的
    字节内容（本门只用输出**字节数**做锚点，不比数值 —— 数值正确性由 m157 门守）；
  · **值域有界**（[-1,1]）：避免运算中途出 inf/nan 影响某些实现的快路径判定。

## 产物

  <outdir>/onnx_mm512.onnx    A[512,512]  @ B[512,512]   = 2 MiB（无页对齐崩塌区）
  <outdir>/onnx_mm1024.onnx   A[1024,1024] @ B[1024,1024] = 8 MiB（**崩塌探针**）

⚠️ 两个都是**全 initializer**（无 graph input）⇒ `onnx_run(id, {})` 即可跑，
   不需要 feeds 文件 ⇒ 门里没有多余 IO 干扰计时。

用法： gen_perf_models.py <outdir>
"""
import os
import random
import struct
import sys

SEED = 20261012

# ─────────────────────── protobuf 编码原语（零第三方依赖）───────────────────────
# ⚠️ 本文件**刻意自包含**：不复用 m157 门里的同名原语。
#    理由：跨门 import 会让 m157 的重构意外打断性能门的语料生成 —— 两个门应当独立可跑。
#    （代价是约 60 行重复；这是**刻意的**取舍，不是疏忽。）


def varint(n):
    if n < 0:
        n += 1 << 64
    out = bytearray()
    while True:
        b = n & 0x7F
        n >>= 7
        if n:
            out.append(b | 0x80)
        else:
            out.append(b)
            return bytes(out)


def tag(f, wt):
    return varint((f << 3) | wt)


def ld(f, payload):
    return tag(f, 2) + varint(len(payload)) + payload


def i_field(f, v):
    return tag(f, 0) + varint(v)


def s_field(f, s):
    return ld(f, s.encode("utf-8") if isinstance(s, str) else s)


def packed_i64(f, vals):
    return ld(f, b"".join(varint(v) for v in vals))


def tensor_f32(name, dims, vals):
    """TensorProto（raw_data 承载，小端 float32 —— 与 C 侧解析口径一致）"""
    raw = b"".join(struct.pack("<f", v) for v in vals)
    return packed_i64(1, dims) + i_field(2, 1) + s_field(8, name) + ld(9, raw)


def node(op, ins, outs, name):
    b = b""
    for x in ins:
        b += s_field(1, x)
    for x in outs:
        b += s_field(2, x)
    b += s_field(3, name)
    b += s_field(4, op)
    return b


def value_info(name, dims, dtype=1):
    shape = b"".join(ld(1, i_field(1, d)) for d in dims)
    typ = ld(1, i_field(1, dtype) + ld(2, shape))
    return s_field(1, name) + ld(2, typ)


def graph(nodes, inits, inputs, outputs, name="g"):
    b = b""
    for n in nodes:
        b += ld(1, n)
    b += s_field(2, name)
    for t in inits:
        b += ld(5, t)
    for v in inputs:
        b += ld(11, v)
    for v in outputs:
        b += ld(12, v)
    return b


def model(g, opset=14):
    b = i_field(1, 7) + s_field(2, "puxian-m267-perf") + ld(7, g)
    b += ld(8, s_field(1, "") + i_field(2, opset))
    return b


# ─────────────────────────────── 语料描述 ───────────────────────────────
def gen_matmul(m_, k_, n_):
    """A[m,k] @ B[k,n] = C[m,n]，两边都是 initializer（自包含模型）"""
    rng = random.Random(SEED + m_ * 31 + k_ * 17 + n_)
    a = [round(rng.uniform(-1.0, 1.0), 6) for _ in range(m_ * k_)]
    b = [round(rng.uniform(-1.0, 1.0), 6) for _ in range(k_ * n_)]
    g = graph(
        nodes=[node("MatMul", ["A", "B"], ["C"], "mm")],
        inits=[tensor_f32("A", [m_, k_], a), tensor_f32("B", [k_, n_], b)],
        inputs=[],
        outputs=[value_info("C", [m_, n_])],
        name="mm_%d_%d_%d" % (m_, k_, n_),
    )
    return model(g)


SPECS = [
    ("onnx_mm512.onnx", 512, 512, 512),
    ("onnx_mm1024.onnx", 1024, 1024, 1024),
]


def main():
    if len(sys.argv) < 2:
        print("用法: gen_perf_models.py <outdir>", file=sys.stderr)
        return 2
    outdir = sys.argv[1]
    os.makedirs(outdir, exist_ok=True)
    for fn, m_, k_, n_ in SPECS:
        p = os.path.join(outdir, fn)
        with open(p, "wb") as f:
            f.write(gen_matmul(m_, k_, n_))
        print("  %-20s %d x %d x %d  %.2f MiB" % (fn, m_, k_, n_, os.path.getsize(p) / 1048576.0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
