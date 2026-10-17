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

M310（丙2 W2）追加 5 个（小规模 + 内层 repeat，单次测量 0.1–0.5 s 档）：
  <outdir>/onnx_quant.onnx    量化族：QuantizeLinear→DequantizeLinear→MatMulInteger→DynamicQuantizeLinear
  <outdir>/onnx_mlp.onnx      端到端一步：MatMul→RMSNormalization→Gelu→MatMul→Silu
  <outdir>/onnx_kv.onnx       KV cache 写入：ScatterND
  <outdir>/onnx_rope.onnx     RoPE 前置：Cos + Sin
  <outdir>/onnx_trilu.onnx    Trilu（规范名）
  ⚠️ 负载清单与模型文件的**单一事实源** = `examples/m267_perf/onnx_loads.tsv`；
     本文件只负责**产出**，清单覆盖核对见 `check_load_coverage.sh`（缺一个即判红）。

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


def _tensor(name, dims, raw, dtype):
    """通用 TensorProto：raw_data 承载（小端），dtype 见 ONNX_DT_*"""
    return packed_i64(1, dims) + i_field(2, dtype) + s_field(8, name) + ld(9, raw)


def tensor_i8(name, dims, vals):
    return _tensor(name, dims, bytes((int(v) & 0xFF) for v in vals), 3)


def tensor_u8(name, dims, vals):
    return _tensor(name, dims, bytes((int(v) & 0xFF) for v in vals), 2)


def tensor_i32(name, dims, vals):
    return _tensor(name, dims, b"".join(struct.pack("<i", int(v)) for v in vals), 6)


def tensor_i64(name, dims, vals):
    return _tensor(name, dims, b"".join(struct.pack("<q", int(v)) for v in vals), 7)


def attr_i(name, v):
    return s_field(1, name) + i_field(3, int(v)) + i_field(20, 2)


def attr_f(name, v):
    return s_field(1, name) + tag(2, 5) + struct.pack("<f", float(v)) + i_field(20, 1)


def attr_s(name, v):
    return s_field(1, name) + s_field(4, v) + i_field(20, 3)


def node_a(op, ins, outs, name, attrs=()):
    """带属性的节点（attrs = [bytes]）"""
    b = b""
    for x in ins:
        b += s_field(1, x)
    for x in outs:
        b += s_field(2, x)
    b += s_field(3, name)
    b += s_field(4, op)
    for a in attrs:
        b += ld(5, a)
    return b


# ═══════════════════ M310_W2_MARK：丙2 · 5 个新负载模型 ═══════════════════
# 设计口径（W2）：
#   · **小规模**：单次成本落在 0.1–0.5 s（门槛 <0.5s 档），别让账本吃掉门时长预算；
#   · **确定性**：固定种子 ⇒ 同种子同字节（幂等）；
#   · **端到端一步**：onnx_mlp 把 MatMul + RMSNorm + Gelu + MatMul + Silu 串起来
#     （= 一个 decoder FFN 步），M306/M308 的新算子因此在「真实形状的组合」里也被计时；
#   · 每个模型只做**它那族算子**的事，避免「一个负载覆盖 11 个算子」那种一损俱损。

def gen_quant(_=None):
    """量化族：QuantizeLinear → DequantizeLinear → MatMulInteger → DynamicQuantizeLinear"""
    rng = random.Random(SEED + 41)
    M, K, N = 128, 512, 256
    x = [round(rng.uniform(-3.0, 3.0), 6) for _ in range(M * K)]
    a = [rng.randint(-128, 127) for _ in range(M * K)]
    w = [rng.randint(-128, 127) for _ in range(K * N)]
    g = graph(
        nodes=[
            node("QuantizeLinear", ["X", "sc", "zp"], ["q"], "quant"),
            node("DequantizeLinear", ["q", "sc", "zp"], ["dq"], "dequant"),
            node("MatMulInteger", ["A", "W", "azp", "bzp"], ["mi"], "matmuli"),
            node("DynamicQuantizeLinear", ["dq"], ["dy", "dsc", "dzp"], "dynq"),
        ],
        inits=[
            tensor_f32("X", [M, K], x),
            tensor_f32("sc", [1], [0.0234375]),
            tensor_u8("zp", [1], [128]),
            tensor_i8("A", [M, K], a),
            tensor_i8("W", [K, N], w),
            tensor_i8("azp", [1], [0]),
            tensor_i8("bzp", [1], [0]),
        ],
        inputs=[],
        outputs=[value_info("dq", [M, K]), value_info("mi", [M, N], 6),
                 value_info("dy", [M, K], 2)],
        name="quant_%d_%d_%d" % (M, K, N),
    )
    return model(g)


def gen_mlp(_=None):
    """端到端一步：MatMul → RMSNormalization → Gelu(tanh) → MatMul → Silu"""
    rng = random.Random(SEED + 42)
    T, D, H = 16, 512, 1024
    x = [round(rng.uniform(-1.0, 1.0), 6) for _ in range(T * D)]
    w1 = [round(rng.uniform(-0.5, 0.5), 6) for _ in range(D * H)]
    w2 = [round(rng.uniform(-0.5, 0.5), 6) for _ in range(H * D)]
    s = [round(rng.uniform(0.5, 1.5), 6) for _ in range(H)]
    g = graph(
        nodes=[
            node("MatMul", ["X", "W1"], ["h1"], "mm1"),
            node_a("RMSNormalization", ["h1", "S"], ["n1"], "rms",
                   [attr_i("axis", -1), attr_f("epsilon", 1e-5)]),
            node_a("Gelu", ["n1"], ["g1"], "gelu", [attr_s("approximate", "tanh")]),
            node("MatMul", ["g1", "W2"], ["h2"], "mm2"),
            node("Silu", ["h2"], ["out"], "silu"),
        ],
        inits=[
            tensor_f32("X", [T, D], x),
            tensor_f32("W1", [D, H], w1),
            tensor_f32("W2", [H, D], w2),
            tensor_f32("S", [H], s),
        ],
        inputs=[],
        outputs=[value_info("out", [T, D])],
        name="mlp_%d_%d_%d" % (T, D, H),
    )
    return model(g)


def gen_kv(_=None):
    """KV cache 写入：ScatterND(data[layer,pos,dim], indices[upd,2], updates[upd,dim])"""
    rng = random.Random(SEED + 43)
    L, P, D, U = 2, 128, 64, 64
    data = [round(rng.uniform(-1.0, 1.0), 6) for _ in range(L * P * D)]
    upd = [round(rng.uniform(-1.0, 1.0), 6) for _ in range(U * D)]
    idx = []
    for u in range(U):
        idx.append(u % L)
        idx.append(u % P)
    g = graph(
        nodes=[node("ScatterND", ["data", "idx", "upd"], ["out"], "kv_write")],
        inits=[
            tensor_f32("data", [L, P, D], data),
            tensor_i64("idx", [U, 2], idx),
            tensor_f32("upd", [U, D], upd),
        ],
        inputs=[],
        outputs=[value_info("out", [L, P, D])],
        name="kv_%d_%d_%d_%d" % (L, P, D, U),
    )
    return model(g)


def gen_rope(_=None):
    """RoPE 前置：Cos(X) + Sin(X)"""
    rng = random.Random(SEED + 44)
    T, D = 256, 128
    x = [round(rng.uniform(-8.0, 8.0), 6) for _ in range(T * D)]
    g = graph(
        nodes=[node("Cos", ["X"], ["c"], "cos"), node("Sin", ["X"], ["s"], "sin")],
        inits=[tensor_f32("X", [T, D], x)],
        inputs=[],
        outputs=[value_info("c", [T, D]), value_info("s", [T, D])],
        name="rope_%d_%d" % (T, D),
    )
    return model(g)


def gen_trilu(_=None):
    """Trilu（规范名）· 上三角保留 —— 512×512"""
    rng = random.Random(SEED + 45)
    N = 512
    x = [round(rng.uniform(-1.0, 1.0), 6) for _ in range(N * N)]
    g = graph(
        nodes=[node_a("Trilu", ["X"], ["out"], "trilu", [attr_i("k", 0)])],
        inits=[tensor_f32("X", [N, N], x)],
        inputs=[],
        outputs=[value_info("out", [N, N])],
        name="trilu_%d" % N,
    )
    return model(g)


# ⭐ M310_W2：新增 5 个（name → builder）；**既有两行生成路径一字未动**
EXTRA_SPECS = [
    ("onnx_quant.onnx", gen_quant),
    ("onnx_mlp.onnx", gen_mlp),
    ("onnx_kv.onnx", gen_kv),
    ("onnx_rope.onnx", gen_rope),
    ("onnx_trilu.onnx", gen_trilu),
]


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
    for fn, builder in EXTRA_SPECS:
        p = os.path.join(outdir, fn)
        with open(p, "wb") as f:
            f.write(builder())
        print("  %-20s %-22s %.2f MiB" % (fn, builder.__name__, os.path.getsize(p) / 1048576.0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
