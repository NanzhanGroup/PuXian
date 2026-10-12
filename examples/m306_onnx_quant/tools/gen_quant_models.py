#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_quant_models.py —— M306 门（ONNX 量化算子面 / S2）的语料生成器：**一份描述，四个产物**

产物：
  1. models/*.onnx          —— 手写 protobuf 编码器产出的模型字节（C 侧被测输入）
  2. models/spec.json       —— **同一份描述**（节点/常量/属性），独立参考执行的唯一输入
  3. models/expected.json   —— **手算真值**（能精确算的就精确给：整型仿射、二进可表示浮点、
                               就近偶数舍入、饱和边界，全部是构造性的）
  4. models/neg/*.onnx      —— 行为负控（每个都必须**响亮报错**，不允许静默成功）

为什么"一份描述、多个产物"（沿用 M156 §38.4 / M157 的方法论）：
  对拍只能证明"两个实现一致"，不能证明"输入是我以为的那样"。
  ⇒ 必须有**构造性真值**：真值直接来自我要写进去的东西（手算），
    参考实现（quant_ref.py，另一套纯 Python、含自己写的半精度位运算）与 C 侧各自独立读同一份语料。
  三方关系：手算 ↔ 参考（验证参考没写错）· 手算 ↔ C（验证 C 没写错）。

⚠️ 与 m157 的 tools/gen_exec_models.py **刻意各自持有**编码器（约 90 行纯工具）：
  gate 的语料生成器互相 import 会让"一个门的目录被移动"打断另一个门，
  而这段编码器不含任何语义（只是 protobuf wire 的写出）⇒ 各自持有更稳。

半精度约定：dtype 为 float16 / bfloat16 的常量，`vals` 里的数**已经是 16 位位型**（由调用方写好），
  这样"我写进去的位 = 参考解出来的值"是构造性的，不依赖 Python 侧是否有 numpy/半精度支持。
"""
import json
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(os.path.join(HERE, ".."))
MODELS = os.path.join(OUT, "models")
NEG = os.path.join(MODELS, "neg")
FEEDS = os.path.join(MODELS, "feeds")

FLOAT, UINT8, INT8, UINT16, INT16, INT32, INT64, BOOL, FLOAT16, DOUBLE, BFLOAT16 = \
    1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 16

DT = {"float": FLOAT, "uint8": UINT8, "int8": INT8, "uint16": UINT16, "int16": INT16,
      "int32": INT32, "int64": INT64, "bool": BOOL, "float16": FLOAT16,
      "double": DOUBLE, "bfloat16": BFLOAT16}

_PACK = {FLOAT: "<f", DOUBLE: "<d", INT8: "<b", UINT8: "<B", INT16: "<h", UINT16: "<H",
         INT32: "<i", INT64: "<q", FLOAT16: "<H", BFLOAT16: "<H"}


# ══════════════════ protobuf 编码（只编码本门用到的字段） ══════════════════
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


def s_field(f, text):
    return ld(f, text.encode("utf-8"))


def i_field(f, v):
    return tag(f, 0) + varint(v)


def packed_i64(f, vals):
    return ld(f, b"".join(varint(v) for v in vals))


def packed_f32(f, vals):
    return ld(f, b"".join(struct.pack("<f", v) for v in vals))


def raw_bytes(dt, vals):
    if dt == BOOL:
        return b"".join(struct.pack("<B", 1 if v else 0) for v in vals)
    return b"".join(struct.pack(_PACK[dt], v) for v in vals)


def tensor_bytes(name, dims, vals, dt=FLOAT):
    """TensorProto：一律用 raw_data（小端）承载 ⇒ 与 C 侧解析口径一致"""
    body = packed_i64(1, dims) + i_field(2, dt) + ld(9, raw_bytes(dt, vals)) + s_field(8, name)
    return body


def enc_attr(name, kind, value):
    b = s_field(1, name)
    if kind == "i":
        b += i_field(3, value)
    elif kind == "f":
        b += tag(2, 5) + struct.pack("<f", value)
    elif kind == "ints":
        b += packed_i64(8, value)
    elif kind == "tensor":
        b += ld(5, value if isinstance(value, bytes) else tensor_bytes(*value))
    elif kind == "floats":
        b += packed_f32(7, value)
    else:
        raise ValueError(kind)
    return b


def node(op, ins, outs, attrs=(), name=None):
    b = b""
    for x in ins:
        b += s_field(1, x)
    for x in outs:
        b += s_field(2, x)
    b += s_field(3, name or (outs[0] if outs else op))
    b += s_field(4, op)
    for a in attrs:
        b += ld(5, enc_attr(a[0], a[1], a[2]))
    return b


def shape_proto(dims):
    return b"".join(ld(1, i_field(1, d)) for d in dims)


def value_info(name, dims, dt=FLOAT):
    typ = ld(1, i_field(1, dt) + ld(2, shape_proto(dims)))
    return s_field(1, name) + ld(2, typ)


def build_model(m, opset=19):
    """m 是描述字典 ⇒ 模型字节"""
    nb = b""
    for nd in m["nodes"]:
        nb += ld(1, node(nd["op"], nd["in"], nd["out"], nd.get("attrs", ())))
    ib = b""
    for c in m.get("consts", []):
        ib += ld(5, tensor_bytes(c["name"], c["dims"], c["vals"], DT[c["dtype"]]))
    xb = b""
    for g in m.get("inputs", []):
        xb += ld(11, value_info(g["name"], g["dims"], DT[g["dtype"]]))
    ob = b""
    for o in m["outputs"]:
        ob += ld(12, value_info(o["name"], o.get("dims", []), DT.get(o.get("dtype", "float"))))
    g = nb + s_field(2, m["name"]) + ib + xb + ob
    b = i_field(1, 7) + s_field(2, "puxian-m306-gen") + ld(7, g)
    b += ld(8, s_field(1, "") + i_field(2, opset))
    return b


# ══════════════════ 语料（描述层） ══════════════════
def C(name, dtype, dims, vals):
    return {"name": name, "dtype": dtype, "dims": dims, "vals": vals}


def N(op, ins, outs, **attrs):
    a = tuple((k, "i" if isinstance(v, int) else "f", v) for k, v in attrs.items())
    return {"op": op, "in": ins, "out": outs, "attrs": a}


def positive_models():
    """返回 (模型描述, 手算真值)。
    真值形状：{输出名: {"dtype": <名>, "values": [...]}}；浮点用 double 精确写出。"""
    out = []

    # ① 逐张量 DequantizeLinear（uint8 + 零点 128）
    m = {"name": "q01_dq_u8_pertensor",
         "consts": [C("X", "uint8", [3], [0, 128, 255]),
                    C("S", "float", [1], [0.5]), C("Z", "uint8", [1], [128])],
         "nodes": [N("DequantizeLinear", ["X", "S", "Z"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [3], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float", "values": [-64.0, 0.0, 63.5]}}))

    # ② 逐张量 QuantizeLinear（回量化；与 ① 互为逆）
    m = {"name": "q02_q_u8_pertensor",
         "consts": [C("X", "float", [3], [-64.0, 0.0, 63.5]),
                    C("S", "float", [1], [0.5]), C("Z", "uint8", [1], [128])],
         "nodes": [N("QuantizeLinear", ["X", "S", "Z"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [3], "dtype": "uint8"}]}
    out.append((m, {"Y": {"dtype": "uint8", "values": [0, 128, 255]}}))

    # ③ 往返恒等：Quantize(Dequantize(x)) == x（可表示范围内必须逐元素相等）
    m = {"name": "q03_roundtrip",
         "consts": [C("X", "uint8", [5], [0, 1, 64, 128, 255]),
                    C("S", "float", [1], [0.25]), C("Z", "uint8", [1], [128])],
         "nodes": [N("DequantizeLinear", ["X", "S", "Z"], ["D"]),
                   N("QuantizeLinear", ["D", "S", "Z"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [5], "dtype": "uint8"}]}
    out.append((m, {"Y": {"dtype": "uint8", "values": [0, 1, 64, 128, 255]}}))

    # ④ 逐轴 DequantizeLinear（axis=0 · int8）
    m = {"name": "q04_dq_i8_peraxis",
         "consts": [C("X", "int8", [2, 3], [1, 2, 3, 4, 5, 6]),
                    C("S", "float", [2], [1.0, 2.0]), C("Z", "int8", [2], [0, 0])],
         "nodes": [N("DequantizeLinear", ["X", "S", "Z"], ["Y"], axis=0)],
         "outputs": [{"name": "Y", "dims": [2, 3], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float", "values": [1.0, 2.0, 3.0, 8.0, 10.0, 12.0]}}))

    # ⑤ 逐轴 QuantizeLinear（axis=0 · 输出 int8）—— 与 ④ 互为逆
    m = {"name": "q05_q_i8_peraxis",
         "consts": [C("X", "float", [2, 3], [1.0, 2.0, 3.0, 8.0, 10.0, 12.0]),
                    C("S", "float", [2], [1.0, 2.0]), C("Z", "int8", [2], [0, 0])],
         "nodes": [N("QuantizeLinear", ["X", "S", "Z"], ["Y"], axis=0)],
         "outputs": [{"name": "Y", "dims": [2, 3], "dtype": "int8"}]}
    out.append((m, {"Y": {"dtype": "int8", "values": [1, 2, 3, 4, 5, 6]}}))

    # ⑥ ⭐ 就近偶数 + 双端饱和（uint8 输出，零点 0）
    #     x   = [-1000, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 1000]
    #     ÷1  ⇒ [-1000, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 1000]
    #     round_half_even ⇒ [-1000, -2, -2, -0, 0, 2, 2, 1000]
    #     饱和 [0,255] ⇒ [0, 0, 0, 0, 0, 2, 2, 255]
    m = {"name": "q06_half_even_saturate",
         "consts": [C("X", "float", [8], [-1000.0, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 1000.0]),
                    C("S", "float", [1], [1.0]), C("Z", "uint8", [1], [0])],
         "nodes": [N("QuantizeLinear", ["X", "S", "Z"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [8], "dtype": "uint8"}]}
    out.append((m, {"Y": {"dtype": "uint8", "values": [0, 0, 0, 0, 0, 2, 2, 255]}}))

    # ⑦ 就近偶数（int8 输出 · 不触发饱和，单看舍入）
    m = {"name": "q07_half_even_i8",
         "consts": [C("X", "float", [8], [-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5]),
                    C("S", "float", [1], [1.0]), C("Z", "int8", [1], [0])],
         "nodes": [N("QuantizeLinear", ["X", "S", "Z"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [8], "dtype": "int8"}]}
    out.append((m, {"Y": {"dtype": "int8", "values": [-4, -2, -2, 0, 0, 2, 2, 4]}}))

    # ⑧ 无零点（缺省 0）的 DequantizeLinear
    m = {"name": "q08_dq_no_zp",
         "consts": [C("X", "int8", [5], [-128, -1, 0, 1, 127]), C("S", "float", [1], [2.0])],
         "nodes": [N("DequantizeLinear", ["X", "S"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [5], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float", "values": [-256.0, -2.0, 0.0, 2.0, 254.0]}}))

    # ⑨ int32 量化（scale=1 ⇒ 值原样浮点化；含 int32 零点）
    m = {"name": "q09_dq_i32",
         "consts": [C("X", "int32", [4], [0, 1, -5, 1000000]),
                    C("S", "float", [1], [1.0]), C("Z", "int32", [1], [0])],
         "nodes": [N("DequantizeLinear", ["X", "S", "Z"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [4], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float", "values": [0.0, 1.0, -5.0, 1000000.0]}}))

    # ⑩ int8 输出 + 负零点 ⇒ 负端饱和
    #    x=[-1,0,1] ÷1 = [-1,0,1]；+(-128) = [-129,-128,-127]；饱和 [-128,127] ⇒ [-128,-128,-127]
    m = {"name": "q10_q_i8_negzp",
         "consts": [C("X", "float", [3], [-1.0, 0.0, 1.0]),
                    C("S", "float", [1], [1.0]), C("Z", "int8", [1], [-128])],
         "nodes": [N("QuantizeLinear", ["X", "S", "Z"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [3], "dtype": "int8"}]}
    out.append((m, {"Y": {"dtype": "int8", "values": [-128, -128, -127]}}))

    # ⑪ 刻度是 float16（位型 0x3800 = 0.5）⇒ 覆盖 fp16 读路径
    m = {"name": "q11_dq_fp16_scale",
         "consts": [C("X", "int8", [3], [1, 2, 3]),
                    C("S", "float16", [1], [0x3800]), C("Z", "int8", [1], [0])],
         "nodes": [N("DequantizeLinear", ["X", "S", "Z"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [3], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float", "values": [0.5, 1.0, 1.5]}}))

    # ⑫ 刻度是 bfloat16（位型 0x3F00 = 0.5）⇒ 覆盖 bf16 读路径
    m = {"name": "q12_dq_bf16_scale",
         "consts": [C("X", "int8", [3], [1, 2, 3]),
                    C("S", "bfloat16", [1], [0x3F00]), C("Z", "int8", [1], [0])],
         "nodes": [N("DequantizeLinear", ["X", "S", "Z"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [3], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float", "values": [0.5, 1.0, 1.5]}}))

    # ⑬ 负 axis 归一（axis=-1 · 3 维）⇒ 逐轴刻度 [10,20,30]
    m = {"name": "q13_dq_neg_axis",
         "consts": [C("X", "int8", [1, 2, 3], [1, 2, 3, 4, 5, 6]),
                    C("S", "float", [3], [10.0, 20.0, 30.0]), C("Z", "int8", [3], [0, 0, 0])],
         "nodes": [N("DequantizeLinear", ["X", "S", "Z"], ["Y"], axis=-1)],
         "outputs": [{"name": "Y", "dims": [1, 2, 3], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float", "values": [10.0, 40.0, 90.0, 40.0, 100.0, 180.0]}}))

    # ⑭ ⭐ 真实形态：int8 权重 Dequantize → 与 float 激活 MatMul → 回量化
    #    Wf = [[0.5,1],[1.5,2]] · X = [[1,2]] ⇒ X·Wf = [[3.5, 5.0]]
    #    再量化（scale 0.5 · zp 128）：round([7,10])+128 = [135, 138]
    m = {"name": "q14_int8_w_matmul_q",
         "consts": [C("W", "int8", [2, 2], [1, 2, 3, 4]),
                    C("S", "float", [1], [0.5]), C("Z", "int8", [1], [0]),
                    C("QS", "float", [1], [0.5]), C("QZ", "uint8", [1], [128])],
         "inputs": [{"name": "X", "dims": [1, 2], "dtype": "float", "_feed": [1.0, 2.0]}],
         "nodes": [N("DequantizeLinear", ["W", "S", "Z"], ["WF"]),
                   N("MatMul", ["X", "WF"], ["M"]),
                   N("QuantizeLinear", ["M", "QS", "QZ"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [1, 2], "dtype": "uint8"}]}
    out.append((m, {"Y": {"dtype": "uint8", "values": [135, 138]}}))

    # ⑮ DynamicQuantizeLinear（运行时求刻度）：**刻意取刻度为二进制精确值**，
    #    这样真值可以手算而不用依赖任何浮点库
    #    x=[-127.5,-1.5,0,1.5,127.5] ⇒ min=min(0,-127.5)=-127.5 · max=max(0,127.5)=127.5
    #    scale=(127.5+127.5)/255=1.0（精确）· zp=round(0-(-127.5)/1)=round(127.5)=128（就近偶数）
    #    y=round(x)+128 = [-128,-2,0,2,128]+128 = [0,126,128,130,255]
    m = {"name": "q15_dynquant_range",
         "consts": [C("X", "float", [5], [-127.5, -1.5, 0.0, 1.5, 127.5])],
         "nodes": [N("DynamicQuantizeLinear", ["X"], ["Y", "YS", "YZ"])],
         "outputs": [{"name": "Y", "dims": [5], "dtype": "uint8"},
                     {"name": "YS", "dims": [], "dtype": "float"},
                     {"name": "YZ", "dims": [], "dtype": "uint8"}]}
    out.append((m, {"Y": {"dtype": "uint8", "values": [0, 126, 128, 130, 255]},
                    "YS": {"dtype": "float", "values": [1.0]},
                    "YZ": {"dtype": "uint8", "values": [128]}}))

    # ⑯ 全零输入 ⇒ scale 退化 ⇒ **取 1.0**（否则除零）；zp=0 · y 全 0
    m = {"name": "q16_dynquant_allzero",
         "consts": [C("X", "float", [3], [0.0, 0.0, 0.0])],
         "nodes": [N("DynamicQuantizeLinear", ["X"], ["Y", "YS", "YZ"])],
         "outputs": [{"name": "Y", "dims": [3], "dtype": "uint8"},
                     {"name": "YS", "dims": [], "dtype": "float"},
                     {"name": "YZ", "dims": [], "dtype": "uint8"}]}
    out.append((m, {"Y": {"dtype": "uint8", "values": [0, 0, 0]},
                    "YS": {"dtype": "float", "values": [1.0]},
                    "YZ": {"dtype": "uint8", "values": [0]}}))

    # ⑰ 全非负输入 ⇒ 检验「min/max **含 0**」这一条
    #    x=[0,127.5] ⇒ min=min(0,0)=0 · max=127.5 ⇒ scale=127.5/255=0.5（精确）· zp=0
    #    y=round(x/0.5)=[0,255]
    m = {"name": "q17_dynquant_nonneg",
         "consts": [C("X", "float", [2], [0.0, 127.5])],
         "nodes": [N("DynamicQuantizeLinear", ["X"], ["Y", "YS", "YZ"])],
         "outputs": [{"name": "Y", "dims": [2], "dtype": "uint8"},
                     {"name": "YS", "dims": [], "dtype": "float"},
                     {"name": "YZ", "dims": [], "dtype": "uint8"}]}
    out.append((m, {"Y": {"dtype": "uint8", "values": [0, 255]},
                    "YS": {"dtype": "float", "values": [0.5]},
                    "YZ": {"dtype": "uint8", "values": [0]}}))

    # ⑱ 全负输入（零点顶到上界 255）
    #    x=[-255,-127.5] ⇒ min=-255 · max=max(0,-127.5)=0 ⇒ scale=1.0 · zp=round(255)=255
    #    y=round(x)+255=[-255,-128]+255=[0,127]
    m = {"name": "q18_dynquant_neg",
         "consts": [C("X", "float", [2], [-255.0, -127.5])],
         "nodes": [N("DynamicQuantizeLinear", ["X"], ["Y", "YS", "YZ"])],
         "outputs": [{"name": "Y", "dims": [2], "dtype": "uint8"},
                     {"name": "YS", "dims": [], "dtype": "float"},
                     {"name": "YZ", "dims": [], "dtype": "uint8"}]}
    out.append((m, {"Y": {"dtype": "uint8", "values": [0, 127]},
                    "YS": {"dtype": "float", "values": [1.0]},
                    "YZ": {"dtype": "uint8", "values": [255]}}))

    # ⑲ 整数矩阵乘（无零点）：A 2×3 · B 3×2
    m = {"name": "q19_matmulinteger",
         "consts": [C("A", "int8", [2, 3], [1, 2, 3, 4, 5, 6]),
                    C("B", "int8", [3, 2], [1, 0, 0, 1, 1, 1])],
         "nodes": [N("MatMulInteger", ["A", "B"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [2, 2], "dtype": "int32"}]}
    out.append((m, {"Y": {"dtype": "int32", "values": [4, 5, 10, 11]}}))

    # ⑳ 逐张量零点（A-1 · B-2）
    m = {"name": "q20_matmulinteger_zp",
         "consts": [C("A", "int8", [2, 2], [2, 4, 6, 8]), C("AZ", "int8", [1], [1]),
                    C("B", "int8", [2, 2], [3, 5, 7, 9]), C("BZ", "int8", [1], [2])],
         "nodes": [N("MatMulInteger", ["A", "B", "AZ", "BZ"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [2, 2], "dtype": "int32"}]}
    out.append((m, {"Y": {"dtype": "int32", "values": [16, 24, 40, 64]}}))

    # ㉑ 逐列零点（B 侧长度 = N）
    #    B=[[1,2,3],[4,5,6]] · BZ=[1,2,3] ⇒ B-BZ = [[0,0,0],[3,3,3]] · A=[[1,2]]
    #    Y[0,j] = 1*0 + 2*3 = 6
    m = {"name": "q21_matmulinteger_percol",
         "consts": [C("A", "int8", [1, 2], [1, 2]),
                    C("B", "int8", [2, 3], [1, 2, 3, 4, 5, 6]),
                    C("BZ", "int8", [3], [1, 2, 3])],
         "nodes": [N("MatMulInteger", ["A", "B", "", "BZ"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [1, 3], "dtype": "int32"}]}
    out.append((m, {"Y": {"dtype": "int32", "values": [6, 6, 6]}}))

    # ㉒ uint8 变体
    m = {"name": "q22_matmulinteger_u8",
         "consts": [C("A", "uint8", [2, 2], [10, 20, 30, 40]), C("AZ", "uint8", [1], [10]),
                    C("B", "uint8", [2, 2], [5, 6, 7, 8]), C("BZ", "uint8", [1], [5])],
         "nodes": [N("MatMulInteger", ["A", "B", "AZ", "BZ"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [2, 2], "dtype": "int32"}]}
    out.append((m, {"Y": {"dtype": "int32", "values": [20, 30, 60, 110]}}))

    # ㉓ ⭐ 真实形态：**动态量化激活 → 整数矩阵乘**（同一张图里两个新算子串联）
    #    X=[[0,127.5]] ⇒ scale=0.5 · zp=0 · YQ=[[0,255]]
    #    W=[[1,2],[3,4]]（int8 · w_zp=0）⇒ Y = [0*1+255*3, 0*2+255*4] = [765, 1020]
    m = {"name": "q23_dynquant_matmulinteger",
         "consts": [C("W", "int8", [2, 2], [1, 2, 3, 4]), C("WZ", "int8", [1], [0])],
         "inputs": [{"name": "X", "dims": [1, 2], "dtype": "float", "_feed": [0.0, 127.5]}],
         "nodes": [N("DynamicQuantizeLinear", ["X"], ["YQ", "XS", "XZ"]),
                   N("MatMulInteger", ["YQ", "W", "XZ", "WZ"], ["Y"])],
         "outputs": [{"name": "Y", "dims": [1, 2], "dtype": "int32"}]}
    out.append((m, {"Y": {"dtype": "int32", "values": [765, 1020]}}))

    # ㉔ fp16 读：位型 → float（含次正规、最大值）
    #    0x0000=0 · 0x3C00=1 · 0xC000=-2 · 0x3800=0.5 · 0x7BFF=65504 · 0x0001=2^-24
    m = {"name": "c01_fp16_read",
         "consts": [C("F", "float16", [6], [0x0000, 0x3C00, 0xC000, 0x3800, 0x7BFF, 0x0001])],
         "nodes": [N("Cast", ["F"], ["Y"], to=FLOAT)],
         "outputs": [{"name": "Y", "dims": [6], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float",
                          "values": [0.0, 1.0, -2.0, 0.5, 65504.0, 2.0 ** -24]}}))

    # ⑯ fp16 写（float → float16 → float）：就近偶数 + 次正规
    #    0.1 ⇒ 0.0999755859375（0x2E66）· 3.14159 ⇒ 3.140625 · 1.0000001 ⇒ 1.0
    m = {"name": "c02_fp16_write",
         "consts": [C("F", "float", [6], [0.0, 1.0, -2.0, 0.1, 1.0000001, 3.14159])],
         "nodes": [N("Cast", ["F"], ["H"], to=FLOAT16),
                   N("Cast", ["H"], ["Y"], to=FLOAT)],
         "outputs": [{"name": "Y", "dims": [6], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float",
                          "values": [0.0, 1.0, -2.0, 0.0999755859375, 1.0, 3.140625]}}))

    # ⑰ bf16 读：位型 << 16 精确重建（含 bf16 次正规 0x0001 ⇒ 2^-133）
    m = {"name": "c03_bf16_read",
         "consts": [C("F", "bfloat16", [6], [0x0000, 0x3F80, 0xC000, 0x3F00, 0x7F7F, 0x0001])],
         "nodes": [N("Cast", ["F"], ["Y"], to=FLOAT)],
         "outputs": [{"name": "Y", "dims": [6], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float",
                          "values": [0.0, 1.0, -2.0, 0.5, 3.3895313892515355e+38,
                                     2.0 ** -133]}}))

    # ⑱ bf16 写（float → bfloat16 → float）：就近偶数
    #    0x3F808000 是 1.0 与 1.0078125 的**严格中点** ⇒ 取偶 ⇒ 1.0
    #    0x3F818000 是 1.0078125 与 1.015625 的中点（高位尾段为奇）⇒ 取偶 ⇒ 1.015625
    m = {"name": "c04_bf16_write",
         "consts": [C("F", "float", [6],
                      [struct.unpack("<f", struct.pack("<I", b))[0] for b in
                       (0x3F800000, 0x3F808000, 0x3F818000, 0x40000000, 0x3E800000, 0x00000001)])],
         "nodes": [N("Cast", ["F"], ["H"], to=BFLOAT16),
                   N("Cast", ["H"], ["Y"], to=FLOAT)],
         "outputs": [{"name": "Y", "dims": [6], "dtype": "float"}]}
    out.append((m, {"Y": {"dtype": "float",
                          "values": [1.0, 1.0, 1.015625, 2.0, 0.25, 0.0]}}))

    return out


def negative_models():
    """行为负控：每个都必须**响亮报错**（rc≠0 且给出指名消息）。
    `why` 写清"这条负控在守什么"——不许有"反正跑不起来"的条目。"""
    base = dict(C("X", "int8", [3], [1, 2, 3]), S=C("S", "float", [1], [0.5]),
                Z=C("Z", "int8", [1], [0]))
    qbase = dict(C("X", "float", [3], [1.0, 2.0, 3.0]), S=C("S", "float", [1], [0.5]),
                 Z=C("Z", "uint8", [1], [128]))
    return [
        ({"name": "n01_block_size",
          "consts": [C("X", "int8", [3], [1, 2, 3]), C("S", "float", [1], [0.5])],
          "nodes": [N("DequantizeLinear", ["X", "S"], ["Y"], block_size=1)],
          "outputs": [{"name": "Y", "dims": [3], "dtype": "float"}]},
         "块量化（block_size≠0）必须报错，不许静默当逐张量"),
        ({"name": "n02_saturate0",
          "consts": [C("X", "float", [3], [1.0, 2.0, 3.0]), C("S", "float", [1], [1.0])],
          "nodes": [N("QuantizeLinear", ["X", "S"], ["Y"], saturate=0)],
          "outputs": [{"name": "Y", "dims": [3], "dtype": "uint8"}]},
         "非饱和（saturate=0）必须报错，不许静默回绕"),
        ({"name": "n03_scale_len",
          "consts": [C("X", "int8", [2, 3], [1, 2, 3, 4, 5, 6]), C("S", "float", [2], [1.0, 2.0])],
          "nodes": [N("DequantizeLinear", ["X", "S"], ["Y"], axis=1)],
          "outputs": [{"name": "Y", "dims": [2, 3], "dtype": "float"}]},
         "逐轴刻度长度与 dims[axis] 不符必须报错（axis=1 时轴上是 3 而不是 2，这里只给 2"),
        ({"name": "n04_zp_dtype",
          "consts": [C("X", "int8", [3], [1, 2, 3]), C("S", "float", [1], [0.5]),
                     C("Z", "float", [1], [0.0])],
          "nodes": [N("DequantizeLinear", ["X", "S", "Z"], ["Y"])],
          "outputs": [{"name": "Y", "dims": [3], "dtype": "float"}]},
         "零点类型与 x 不一致必须报错（float 零点当作 0 用是最典型的静默错值）"),
        ({"name": "n05_x_dtype_dq",
          "consts": [C("X", "float", [3], [1.0, 2.0, 3.0]), C("S", "float", [1], [0.5])],
          "nodes": [N("DequantizeLinear", ["X", "S"], ["Y"])],
          "outputs": [{"name": "Y", "dims": [3], "dtype": "float"}]},
         "DequantizeLinear 的 x 必须是整型（给浮点应报错，不许当恒等）"),
        ({"name": "n06_q_x_dtype",
          "consts": [C("X", "int8", [3], [1, 2, 3]), C("S", "float", [1], [0.5])],
          "nodes": [N("QuantizeLinear", ["X", "S"], ["Y"])],
          "outputs": [{"name": "Y", "dims": [3], "dtype": "uint8"}]},
         "QuantizeLinear 的 x 必须是浮点"),
        ({"name": "n07_axis_oob",
          "consts": [C("X", "int8", [2, 3], [1, 2, 3, 4, 5, 6]), C("S", "float", [2], [1.0, 2.0])],
          "nodes": [N("DequantizeLinear", ["X", "S"], ["Y"], axis=5)],
          "outputs": [{"name": "Y", "dims": [2, 3], "dtype": "float"}]},
         "axis 越界必须报错（不许夹到合法区间）"),
        ({"name": "n08_scale_zero",
          "consts": [C("X", "float", [3], [1.0, 2.0, 3.0]), C("S", "float", [1], [0.0])],
          "nodes": [N("QuantizeLinear", ["X", "S"], ["Y"])],
          "outputs": [{"name": "Y", "dims": [3], "dtype": "uint8"}]},
         "刻度为 0 必须报错（不许静默饱和到端点）"),
        ({"name": "n09_scale_rank3",
          "consts": [C("X", "int8", [2, 3], [1, 2, 3, 4, 5, 6]),
                     C("S", "float", [2, 3], [1.0, 1.0, 1.0, 1.0, 1.0, 1.0])],
          "nodes": [N("DequantizeLinear", ["X", "S"], ["Y"])],
          "outputs": [{"name": "Y", "dims": [2, 3], "dtype": "float"}]},
         "刻度不是标量也不是 1 维必须报错"),
        ({"name": "n10_mi_zp_len",
          "consts": [C("A", "int8", [2, 3], [1, 2, 3, 4, 5, 6]),
                     C("B", "int8", [3, 2], [1, 0, 0, 1, 1, 1]),
                     C("AZ", "int8", [3], [0, 0, 0])],
          "nodes": [N("MatMulInteger", ["A", "B", "AZ"], ["Y"])],
          "outputs": [{"name": "Y", "dims": [2, 2], "dtype": "int32"}]},
         "a_zero_point 长度既不是 1 也不是「逐行」（这里给了 3，而行数是 2）必须报错"),
        ({"name": "n11_mi_inner",
          "consts": [C("A", "int8", [2, 3], [1, 2, 3, 4, 5, 6]),
                     C("B", "int8", [2, 2], [1, 0, 0, 1])],
          "nodes": [N("MatMulInteger", ["A", "B"], ["Y"])],
          "outputs": [{"name": "Y", "dims": [2, 2], "dtype": "int32"}]},
         "MatMulInteger 内维不一致必须报错"),
        ({"name": "n12_dynquant_arity",
          "consts": [C("X", "float", [2], [0.0, 1.0]), C("Y2", "float", [2], [0.0, 1.0])],
          "nodes": [N("DynamicQuantizeLinear", ["X", "Y2"], ["Y", "YS", "YZ"])],
          "outputs": [{"name": "Y", "dims": [2], "dtype": "uint8"},
                      {"name": "YS", "dims": [], "dtype": "float"},
                      {"name": "YZ", "dims": [], "dtype": "uint8"}]},
         "DynamicQuantizeLinear 只收 1 个输入（给 2 个必须报错，不许忽略）"),
        ({"name": "n13_mi_zp_dtype",
          "consts": [C("A", "int8", [2, 2], [1, 2, 3, 4]),
                     C("B", "int8", [2, 2], [1, 2, 3, 4]),
                     C("AZ", "float", [1], [0.0])],
          "nodes": [N("MatMulInteger", ["A", "B", "AZ"], ["Y"])],
          "outputs": [{"name": "Y", "dims": [2, 2], "dtype": "int32"}]},
         "a_zero_point 类型与 A 不一致必须报错（浮点零点当 0 用是最典型的静默错值）"),
        ({"name": "n14_dynquant_int_in",
          "consts": [C("X", "int8", [2], [1, 2])],
          "nodes": [N("DynamicQuantizeLinear", ["X"], ["Y", "YS", "YZ"])],
          "outputs": [{"name": "Y", "dims": [2], "dtype": "uint8"},
                      {"name": "YS", "dims": [], "dtype": "float"},
                      {"name": "YZ", "dims": [], "dtype": "uint8"}]},
         "DynamicQuantizeLinear 的 x 必须是浮点"),
    ]


def main():
    os.makedirs(MODELS, exist_ok=True)
    os.makedirs(NEG, exist_ok=True)
    os.makedirs(FEEDS, exist_ok=True)

    pos = positive_models()
    spec = {"models": []}
    expected = {}
    feeds = {}
    for m, truth in pos:
        with open(os.path.join(MODELS, m["name"] + ".onnx"), "wb") as f:
            f.write(build_model(m))
        spec["models"].append(m)
        expected[m["name"]] = truth
        fd = {}
        for g in m.get("inputs", []):
            fd[g["name"]] = {"dims": g["dims"], "dtype": g["dtype"],
                             "f32": g.pop("_feed", [1.0, 2.0])}
        with open(os.path.join(FEEDS, m["name"] + ".json"), "w") as f:
            json.dump(fd, f, indent=1, ensure_ascii=False)

    negs = negative_models()
    negcases = []
    for m, why in negs:
        with open(os.path.join(NEG, m["name"] + ".onnx"), "wb") as f:
            f.write(build_model(m))
        negcases.append({"name": m["name"], "expect_fail": True, "why": why})

    with open(os.path.join(MODELS, "spec.json"), "w") as f:
        json.dump(spec, f, indent=1, ensure_ascii=False)
    with open(os.path.join(MODELS, "expected.json"), "w") as f:
        json.dump(expected, f, indent=1, ensure_ascii=False)
    with open(os.path.join(NEG, "cases.json"), "w") as f:
        json.dump(negcases, f, indent=1, ensure_ascii=False)

    print("模型 %d 个（正例）· 负控 %d 个 · 输出点 %d"
          % (len(pos), len(negs), sum(len(t) for _, t in pos)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
