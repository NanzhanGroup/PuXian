#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_exec_models.py —— M157 门（执行面）的语料生成器：**一份描述，三个产物**

产物：
  1. models/*.onnx      —— 手写 protobuf 编码器产出的模型字节（C 侧被测输入）
  2. models/graphs.json —— **同一份描述**的图级 JSON（独立参考执行器的输入）
  3. models/expected.json —— **手算真值**（能精确算的就精确给，例如整数运算、
                             单位矩阵乘、常数行 LayerNorm=0、Softmax(axis=4) 均匀=0.25）
  另产出 models/feeds/<m>.json（图输入的数值，双方共用）

为什么要"一份描述、三个产物"（沿用 M156 §38.4 的方法论）：
  对拍只能证明"两个实现一致"，不能证明"输入是我以为的那样"。
  ⇒ 必须有**构造性真值**：真值直接来自我要写进去的东西（手算），
    并且参考实现（exec_ref.py）与 C 侧各自独立读同一份语料。
  三方关系：手算 ↔ 参考（验证参考没写错）· 参考 ↔ C（验证 C 没写错）。
"""
import json
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..")
MODELS = os.path.join(OUT, "models")
FEEDS = os.path.join(MODELS, "feeds")

FLOAT, INT64, BOOL, INT32 = 1, 7, 9, 6


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


def tensor_bytes(name, dims, vals, dtype=FLOAT):
    """TensorProto：用 raw_data（小端）承载 ⇒ 与 C 侧解析口径一致"""
    raw = b"".join(struct.pack("<f", v) for v in vals) if dtype == FLOAT else \
          b"".join(struct.pack("<q", v) for v in vals)
    body = packed_i64(1, dims) + i_field(2, dtype) + ld(9, raw) + s_field(8, name)
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
        b += ld(5, value if isinstance(value, bytes) else tensor_bytes(
            value[0], value[1], value[2], value[3] if len(value) > 3 else FLOAT))
    elif kind == "floats":
        b += packed_f32(7, value)
    else:
        raise ValueError(kind)
    return b


def attr(name, kind, value):
    """描述层：attr 只记 (名字, 种类, 值)；字节在 build() 时才编码
    ⇒ graphs.json 与 .onnx 出自**同一份描述**（M156 §38.4 的方法论）"""
    return (name, kind, value)


def node(op, ins, outs, attrs=(), name=None):
    b = b""
    for x in ins:
        b += s_field(1, x)
    for x in outs:
        b += s_field(2, x)
    b += s_field(3, name or (outs[0] if outs else op))
    b += s_field(4, op)
    for a in attrs:
        b += ld(5, a)
    return b


def shape_proto(dims):
    b = b""
    for d in dims:
        b += ld(1, i_field(1, d))
    return b


def value_info(name, dims, dtype=FLOAT):
    typ = ld(1, i_field(1, dtype) + ld(2, shape_proto(dims)))
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
    b = i_field(1, 7) + s_field(2, "puxian-m157-gen") + ld(7, g)
    b += ld(8, s_field(1, "") + i_field(2, opset))
    return b


# ══════════════════ 语料的"描述"层（同一份描述产出字节 + JSON） ══════════════════
def flat(v):
    """把嵌套列表摊平成一维（语料里写 [[1,2],[3,4]] 更好读）"""
    if isinstance(v, (list, tuple)):
        out = []
        for x in v:
            out.extend(flat(x))
        return out
    return [v]


class M:
    def __init__(self, name, opset=14):
        self.name = name
        self.opset = opset
        self.nodes = []        # (op, ins, outs, attrs, name)
        self.inits = []        # (name, dims, vals, dtype)
        self.inputs = []       # (name, dims, dtype)
        self.outputs = []      # (name, dims, dtype)
        self.feeds = {}        # 输入数值
        self.hand = {}         # 手算真值（可精确的才给）
        self.hand_note = {}

    def init(self, name, dims, vals, dtype=FLOAT):
        self.inits.append((name, list(dims), flat(vals), dtype))

    def inp(self, name, dims, vals, dtype=FLOAT):
        vals = flat(vals)
        n = 1
        for d in dims:
            n *= d
        assert len(vals) == n, "%s: 给 %d 个值，形状 %s 需要 %d 个" % (name, len(vals), dims, n)
        self.inputs.append((name, list(dims), dtype))
        self.feeds[name] = {"dims": list(dims),
                            "dtype": {FLOAT: "float", INT64: "int64", BOOL: "bool",
                                      INT32: "int32"}[dtype],
                            ("f32" if dtype == FLOAT else "i64"):
                                [(float(v) if dtype == FLOAT else int(v)) for v in vals]}

    def op(self, op, ins, outs, attrs=(), name=None):
        self.nodes.append((op, list(ins), list(outs), list(attrs), name))
        return outs[0] if len(outs) == 1 else outs

    def out(self, name, dims, dtype=FLOAT):
        self.outputs.append((name, list(dims), dtype))

    def exact(self, name, vals, note=""):
        # 稀疏断言：{"at":[下标…],"vals":[…]} 原样保留（只断言指定下标）
        self.hand[name] = vals if isinstance(vals, dict) else flat(vals)
        self.hand_note[name] = note

    # ── 序列化 ──
    def build(self):
        nodes = [node(o, ins, outs, [enc_attr(*a) for a in attrs], nm)
                 for (o, ins, outs, attrs, nm) in self.nodes]
        inits = [tensor_bytes(n, d, v, dt) for (n, d, v, dt) in self.inits]
        ins = [value_info(n, d, dt) for (n, d, dt) in self.inputs]
        outs = [value_info(n, d, dt) for (n, d, dt) in self.outputs]
        return model(graph(nodes, inits, ins, outs, self.name), self.opset)

    def graph_json(self):
        return {"name": self.name, "opset": self.opset,
                "inputs": [{"name": n, "dims": d, "dtype": dt} for (n, d, dt) in self.inputs],
                "outputs": [{"name": n, "dims": d, "dtype": dt} for (n, d, dt) in self.outputs],
                "initializers": [{"name": n, "dims": d, "vals": v, "dtype": dt}
                                 for (n, d, v, dt) in self.inits],
                "nodes": [{"op": o, "in": ins, "out": outs,
                           "attrs": {a[0]: ({"kind": "tensor", "name": a[2][0],
                                             "dims": a[2][1], "vals": a[2][2],
                                             "dtype": a[2][3] if len(a[2]) > 3 else FLOAT}
                                            if a[1] == "tensor" else a[2])
                                     for a in attrs}, "name": nm}
                          for (o, ins, outs, attrs, nm) in self.nodes]}


# ══════════════════ 各模型 ══════════════════
def m01_eltwise():
    m = M("e01_eltwise_exact")
    A = [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
    B = [10.0, 20.0, 30.0]
    m.inp("A", [2, 3], A)
    m.inp("B", [3], B)
    m.init("two", [], [2.0])
    m.init("seven", [], [7.0])
    m.init("four", [], [4.0])
    m.op("Add", ["A", "B"], ["C"])
    m.op("Sub", ["C", "A"], ["D"])
    m.op("Mul", ["C", "A"], ["E"])
    m.op("Div", ["C", "B"], ["F"])
    m.op("Pow", ["A", "two"], ["G"])
    m.init("MI", [6], [11, 22, 33, 14, 25, 36], INT64)
    m.init("MI7", [], [7], INT64)
    m.op("Mod", ["MI", "MI7"], ["H"])
    m.op("Max", ["A", "four"], ["I"])
    m.op("Min", ["A", "four"], ["J"])
    m.op("Sum", ["A", "B"], ["K"])
    m.op("Mean", ["A", "B"], ["L"])
    m.op("Neg", ["A"], ["NA"])
    m.op("Abs", ["NA"], ["M"])
    m.op("Ceil", ["A"], ["CE"])
    m.op("Floor", ["A"], ["FL"])
    m.op("Round", ["A"], ["RO"])
    m.op("Sign", ["NA"], ["SG"])
    m.op("Greater", ["A", "four"], ["GT"])
    m.op("Less", ["A", "four"], ["LT"])
    m.op("Equal", ["A", "four"], ["EQ"])
    m.op("LessOrEqual", ["A", "four"], ["LE"])
    m.op("GreaterOrEqual", ["A", "four"], ["GE"])
    m.op("Not", ["GT"], ["NT"])
    m.op("And", ["GT", "LT"], ["AN"])
    m.op("Or", ["GT", "LT"], ["OR"])
    m.op("Xor", ["GT", "LT"], ["XO"])
    m.op("Reciprocal", ["B"], ["RC"])
    for nm, dt in [("C", FLOAT), ("D", FLOAT), ("E", FLOAT), ("F", FLOAT), ("G", FLOAT),
                   ("I", FLOAT), ("J", FLOAT), ("K", FLOAT), ("L", FLOAT),
                   ("M", FLOAT), ("CE", FLOAT), ("FL", FLOAT), ("RO", FLOAT), ("SG", FLOAT),
                   ("H", INT64),
                   ("RC", FLOAT), ("GT", BOOL), ("LT", BOOL), ("EQ", BOOL), ("LE", BOOL),
                   ("GE", BOOL), ("NT", BOOL), ("AN", BOOL), ("OR", BOOL), ("XO", BOOL)]:
        m.out(nm, [2, 3] if nm not in ("RC",) else [3], dt)
    m.exact("C", [11.0, 22.0, 33.0, 14.0, 25.0, 36.0], "A+B 手算")
    m.exact("D", [10.0, 20.0, 30.0, 10.0, 20.0, 30.0], "C-A = B（手算）")
    m.exact("E", [11.0, 44.0, 99.0, 56.0, 125.0, 216.0], "C*A 手算")
    m.exact("G", [1.0, 4.0, 9.0, 16.0, 25.0, 36.0], "A^2 手算")
    m.exact("I", [4.0, 4.0, 4.0, 4.0, 5.0, 6.0], "max(A,4) 手算")
    m.exact("J", [1.0, 2.0, 3.0, 4.0, 4.0, 4.0], "min(A,4) 手算")
    m.exact("K", [11.0, 22.0, 33.0, 14.0, 25.0, 36.0], "A+B 手算")
    m.exact("L", [5.5, 11.0, 16.5, 7.0, 12.5, 18.0], "mean = (A+B)/2 手算")
    m.exact("M", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "abs(neg(A)) = A")
    m.exact("CE", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "整数值 ceil 不变")
    m.exact("FL", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "整数值 floor 不变")
    m.exact("RO", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "整数值 round 不变")
    m.exact("SG", [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0], "sign(-x) = -1")
    m.exact("GT", [0.0, 0.0, 0.0, 0.0, 1.0, 1.0], "A>4")
    m.exact("LT", [1.0, 1.0, 1.0, 0.0, 0.0, 0.0], "A<4")
    m.exact("EQ", [0.0, 0.0, 0.0, 1.0, 0.0, 0.0], "A==4")
    m.exact("LE", [1.0, 1.0, 1.0, 1.0, 0.0, 0.0], "A<=4")
    m.exact("GE", [0.0, 0.0, 0.0, 1.0, 1.0, 1.0], "A>=4")
    m.exact("NT", [1.0, 1.0, 1.0, 1.0, 0.0, 0.0], "not(A>4)")
    m.exact("AN", [0.0] * 6, "(A>4) and (A<4)")
    m.exact("OR", [1.0, 1.0, 1.0, 0.0, 1.0, 1.0], "(A>4) or (A<4)：A=4 处两者皆假")
    m.exact("XO", [1.0, 1.0, 1.0, 0.0, 1.0, 1.0], "(A>4) xor (A<4)：A=4 处两者皆假")
    m.exact("H", [11 % 7, 22 % 7, 33 % 7, 14 % 7, 25 % 7, 36 % 7], "整型 Mod 手算（int64）")
    m.exact("F", [11.0 / 10, 22.0 / 20, 33.0 / 30, 14.0 / 10, 25.0 / 20, 36.0 / 30],
            "C/B（float32 舍入后比对，用容差）")
    m.exact("RC", [0.1, 0.05, 1.0 / 30], "1/B")
    return m


def m02_matmul():
    m = M("e02_matmul_identity")
    A = [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
    W = [[1.0, 0.0, 0.0, 0.0], [0.0, 1.0, 0.0, 0.0], [0.0, 0.0, 1.0, 0.0]]
    I3 = [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]
    bias = [0.5, -0.5, 1.5, 2.5]
    m.inp("A", [2, 3], A)
    m.init("I3", [3, 3], I3)
    m.init("W", [3, 4], W)
    m.init("bias", [4], bias)
    m.op("MatMul", ["A", "I3"], ["AI"])
    m.op("MatMul", ["A", "W"], ["AW"])
    m.op("Add", ["AW", "bias"], ["AWb"])
    m.init("I4", [4, 4], [1.0 if i % 5 == 0 else 0.0 for i in range(16)])
    m.op("MatMul", ["AWb", "I4"], ["AWbI"])
    m.out("AI", [2, 3])
    m.out("AWb", [2, 4])
    m.out("AWbI", [2, 4])
    m.exact("AI", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "A@I3 = A（逐位精确）")
    m.exact("AWb", [1.5, 1.5, 4.5, 2.5, 4.5, 4.5, 7.5, 2.5],
            "A@W + bias 手算（W 取 3 列截断的偏移矩阵）")
    m.exact("AWbI", [1.5, 1.5, 4.5, 2.5, 4.5, 4.5, 7.5, 2.5], "(A@W+b)@I3 = A@W+b")
    return m


def m03_softmax_reduce():
    m = M("e03_softmax_uniform")
    X = [0.0] * 24
    m.inp("X", [2, 3, 4], X)
    m.op("Softmax", ["X"], ["S"], [attr("axis", "i", 2)])
    m.op("ReduceMean", ["S"], ["RM"], [attr("axes", "ints", [2])])
    m.op("ReduceSum", ["S"], ["RS"], [attr("axes", "ints", [2])])
    m.op("ReduceMax", ["S"], ["RX"], [attr("axes", "ints", [2])])
    m.op("ReduceMin", ["S"], ["RN"], [attr("axes", "ints", [2])])
    m.op("ReduceProd", ["S"], ["RP"], [attr("axes", "ints", [2])])
    m.op("ArgMax", ["X"], ["AM"], [attr("axis", "i", 2), attr("keepdims", "i", 0)])
    m.out("S", [2, 3, 4])
    for nm in ("RM", "RS", "RX", "RN", "RP"):
        m.out(nm, [2, 3, 1])
    m.out("AM", [2, 3], INT64)
    m.exact("S", [0.25] * 24, "全 0 输入 ⇒ softmax 均匀 1/4（0.25 二进制精确）")
    m.exact("RM", [0.25] * 6, "均值 = 1/4")
    m.exact("RS", [1.0] * 6, "和 = 1")
    m.exact("RX", [0.25] * 6, "max = 1/4")
    m.exact("RN", [0.25] * 6, "min = 1/4")
    m.exact("RP", [0.25 ** 4] * 6, "prod = (1/4)^4 = 2^-8 精确")
    m.exact("AM", [0] * 6, "全相等 ⇒ 首个最大（下标 0）")
    return m


def m04_movement():
    m = M("e04_movement")
    X = [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
    m.inp("X", [2, 3], X)
    m.init("idx1", [1], [1], INT64)
    m.init("sh6", [1], [6], INT64)
    m.init("sh26", [2], [2, 6], INT64)
    m.init("rep2", [1], [2], INT64)
    m.init("ax0", [1], [0], INT64)
    m.init("sp33", [2], [3, 3], INT64)
    m.init("sl_s", [1], [1], INT64)
    m.init("sl_e", [1], [4], INT64)
    m.op("Transpose", ["X"], ["T"], [attr("perm", "ints", [1, 0])])
    m.op("Reshape", ["X", "sh6"], ["R"])
    m.op("Unsqueeze", ["R", "ax0"], ["U"])
    m.op("Squeeze", ["U", "ax0"], ["U2"])
    m.op("Flatten", ["X"], ["FL"], [attr("axis", "i", 1)])
    m.op("Concat", ["T", "T"], ["CT"], [attr("axis", "i", 0)])
    m.op("Shape", ["T"], ["ST"])
    m.op("Gather", ["ST", "idx1"], ["GT1"], [attr("axis", "i", 0)])
    m.op("Slice", ["R", "sl_s", "sl_e"], ["SL"])
    m.op("Tile", ["R", "rep2"], ["TL"])
    m.op("Split", ["R", "sp33"], ["SP0", "SP1"], [attr("axis", "i", 0)])
    m.op("Cast", ["U"], ["CI"], [attr("to", "i", INT64)])
    m.op("Cast", ["CI"], ["CB"], [attr("to", "i", BOOL)])
    m.op("Expand", ["R", "sh26"], ["EX"])
    for nm, dims, dt in (("T", [2, 3], FLOAT), ("R", [6], FLOAT), ("U", [1, 6], FLOAT),
                         ("U2", [6], FLOAT), ("FL", [1, 6], FLOAT), ("CT", [4, 3], FLOAT),
                         ("ST", [2], FLOAT), ("GT1", [1], FLOAT), ("SL", [3], FLOAT),
                         ("TL", [12], FLOAT), ("SP0", [3], FLOAT), ("SP1", [3], FLOAT),
                         ("EX", [2, 6], FLOAT), ("CI", [1, 6], INT64), ("CB", [1, 6], BOOL)):
        m.out(nm, dims, dt)
    m.exact("T", [1.0, 4.0, 2.0, 5.0, 3.0, 6.0], "转置手算")
    m.exact("R", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "reshape 行主序")
    m.exact("U", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "unsqueeze 不改数据")
    m.exact("U2", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "squeeze 回来")
    m.exact("FL", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "flatten axis=1 ⇒ [1,6]")
    m.exact("CT", [1.0, 4.0, 2.0, 5.0, 3.0, 6.0, 1.0, 4.0, 2.0, 5.0, 3.0, 6.0], "concat 两遍")
    m.exact("ST", [3.0, 2.0], "shape(T) = [3,2]")
    m.exact("GT1", [2.0], "shape(T)[1] = 2")
    m.exact("SL", [2.0, 3.0, 4.0], "slice R[1:4] 手算")
    m.exact("TL", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0] * 2, "tile ×2")
    m.exact("SP0", [1.0, 2.0, 3.0], "split 前半")
    m.exact("SP1", [4.0, 5.0, 6.0], "split 后半")
    m.exact("EX", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0] * 2, "expand [6]→[2,6] = 复制两行")
    m.exact("CI", [1, 2, 3, 4, 5, 6], "cast float→int64 手算")
    m.exact("CB", [1, 1, 1, 1, 1, 1], "cast 非零→true")
    return m


def m05_gelu_funcs():
    m = M("e05_funcs")
    X = [0.0, 1.0, -1.0, 2.0]
    m.init("sq2", [], [1.4142135623730951])
    m.init("half", [], [0.5])
    m.init("one", [], [1.0])
    m.init("lo", [], [-0.5])
    m.init("hi", [], [0.5])
    m.inp("X", [1, 4], X)
    m.op("Div", ["X", "sq2"], ["D"])
    m.op("Erf", ["D"], ["E"])
    m.op("Add", ["E", "one"], ["E1"])
    m.op("Mul", ["E1", "half"], ["H"])
    m.op("Mul", ["H", "X"], ["GELU"])
    m.op("Abs", ["X"], ["AB"])
    m.op("Sqrt", ["AB"], ["SQ"])
    m.op("Exp", ["X"], ["EX"])
    m.op("Log", ["EX"], ["LG"])
    m.op("Tanh", ["X"], ["TH"])
    m.op("Sigmoid", ["X"], ["SI"])
    m.op("Relu", ["X"], ["RE"])
    m.op("LeakyRelu", ["X"], ["LR"], [attr("alpha", "f", 0.1)])
    m.op("Elu", ["X"], ["EL"], [attr("alpha", "f", 1.0)])
    m.op("Softplus", ["X"], ["SP"])
    m.op("Clip", ["X", "lo", "hi"], ["CL"])
    for nm in ("D", "E", "E1", "H", "GELU", "AB", "SQ", "EX", "LG", "TH", "SI", "RE",
               "LR", "EL", "SP", "CL"):
        m.out(nm, [1, 4])
    m.exact("GELU", [0.0, 0.8413447460685429, -0.15865525393145707, 1.9544997361036416],
            "GELU 手算 0.5x(1+erf(x/√2))（x=0 ⇒ 精确 0）")
    m.exact("AB", [0.0, 1.0, 1.0, 2.0], "abs 手算")
    m.exact("RE", [0.0, 1.0, 0.0, 2.0], "relu 手算")
    m.exact("CL", [0.0, 0.5, -0.5, 0.5], "clip(x,-0.5,0.5) 手算")
    m.exact("LG", [0.0, 1.0, -1.0, 2.0], "log(exp(x)) = x（容差）")
    return m


def m06_layernorm():
    m = M("e06_layernorm", opset=17)
    X = [1.0, 1.0, 1.0, 1.0, 2.0, 3.0, 4.0, 5.0]
    scale = [1.0, 1.0, 1.0, 1.0]
    bias = [0.0, 0.0, 0.0, 0.0]
    m.inp("X", [2, 4], X)
    m.init("scale", [4], scale)
    m.init("bias", [4], bias)
    m.init("zero", [], [0.0])
    m.op("LayerNormalization", ["X", "scale", "bias"], ["LN"], [attr("axis", "i", 1)])
    m.op("Less", ["LN", "zero"], ["NEG"])
    m.op("Where", ["NEG", "zero", "LN"], ["W"])
    m.out("LN", [2, 4])
    m.out("W", [2, 4])
    m.exact("LN", {"at": [0, 1, 2, 3], "vals": [0.0, 0.0, 0.0, 0.0]},
            "常数行 ⇒ 归一化后恒 0（与 eps 无关，精确；只断言第 0 行）")
    m.exact("W", {"at": [0], "vals": [0.0]}, "行 0 非负 ⇒ where 取 LN，仍为 0")
    return m


def m07_const_range():
    m = M("e07_const_range_triu")
    sh = [2, 3]
    m.init("sh", [2], sh, INT64)
    m.init("seven", [], [7.0])
    m.init("r0", [], [0.0])
    m.init("r3", [], [3.0])
    m.init("r1", [], [1.0])
    tri = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0]
    m.init("M3", [3, 3], tri)
    m.inp("X", [1, 3], [0.0, 0.0, 0.0])
    m.op("ConstantOfShape", ["sh"], ["CZ"], [attr("value", "tensor", ("v", [1], [7.0], FLOAT))])
    m.op("Range", ["r0", "r3", "r1"], ["RG"])
    m.op("Add", ["CZ", "RG"], ["SUM"])
    m.op("Triu", ["M3"], ["TU"]) 
    m.op("Triu", ["M3"], ["TU1"], [attr("k", "i", -1)])
    m.op("Identity", ["X"], ["ID"])
    m.init("sh23", [2], [2, 3], INT64)
    m.op("Expand", ["RG", "sh23"], ["EXP"])
    m.out("CZ", [2, 3])
    m.out("RG", [3])
    m.out("SUM", [2, 3])
    m.out("TU", [3, 3])
    m.out("TU1", [3, 3])
    m.out("ID", [1, 3])
    m.out("EXP", [2, 3])
    m.exact("CZ", [7.0] * 6, "ConstantOfShape(value=7) 手算")
    m.exact("RG", [0.0, 1.0, 2.0], "Range(0,3,1)")
    m.exact("SUM", [7.0, 8.0, 9.0, 7.0, 8.0, 9.0], "CZ + RG 广播手算")
    m.exact("TU", [1.0, 2.0, 3.0, 0.0, 5.0, 6.0, 0.0, 0.0, 9.0], "上三角 k=0")
    m.exact("TU1", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 0.0, 8.0, 9.0], "上三角 k=-1")
    m.exact("EXP", [0.0, 1.0, 2.0, 0.0, 1.0, 2.0], "expand [3]→[2,3] = 复制两行")
    return m


def m08_attn():
    """小块注意力掩码流：MatMul/Transpose/Div/Greater/Unsqueeze/Where/Softmax/Shape/Gather/Add/Mul"""
    m = M("e08_attn_mask")
    n, t, d = 1, 3, 4
    X = [0.1, 0.2, 0.3, 0.4,
         0.5, 0.6, 0.7, 0.8,
         0.9, 1.0, 1.1, 1.2]
    mask = [1.0, 1.0, 0.0]
    Wq = [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0]
    Wk = [0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5]
    m.inp("X", [n, t, d], X)
    m.inp("mask_in", [t], mask)
    m.init("Wq", [d, d], Wq)
    m.init("Wk", [d, d], Wk)
    m.init("two", [], [2.0])
    m.init("neg", [], [-1.0e9])
    m.init("zero", [], [0.0])
    m.init("shidx", [1], [1], INT64)
    m.init("ax1", [1], [1], INT64)
    m.op("MatMul", ["X", "Wq"], ["Q"])
    m.op("MatMul", ["X", "Wk"], ["K"])
    m.op("Transpose", ["K"], ["KT"], [attr("perm", "ints", [0, 2, 1])])
    m.op("MatMul", ["Q", "KT"], ["S"])
    m.op("Div", ["S", "two"], ["SS"])
    m.op("Greater", ["mask_in", "zero"], ["MB"])
    m.op("Unsqueeze", ["MB", "ax1"], ["MB2"])
    m.op("Where", ["MB2", "SS", "neg"], ["MS"])
    m.op("Softmax", ["MS"], ["AT"], [attr("axis", "i", -1)])
    m.op("Shape", ["AT"], ["SH"])
    m.op("Gather", ["SH", "shidx"], ["GT"], [attr("axis", "i", 0)])
    m.op("MatMul", ["AT", "X"], ["CTX"])
    m.op("Add", ["CTX", "X"], ["OUT"])
    m.op("Mul", ["AT", "zero"], ["Z"])
    for nm, dims in (("Q", [n, t, d]), ("AT", [n, t, t]), ("CTX", [n, t, d]),
                     ("OUT", [n, t, d]), ("SH", [3]), ("GT", [1]), ("MS", [n, t, t]),
                     ("Z", [n, t, t]), ("MB2", [1, t])):
        m.out(nm, dims)
    m.exact("Z", [0.0] * 9, "AT * 0 = 0（手算）")
    return m


def m09_erf_grid():
    """erf 精度网格：1001 点在 [-6,6] 上均匀取样 ⇒ 门里报"与 math.erf 的最大绝对误差"。
    为什么单独一个模型：erf 是**双路径实现**（|x|<2.5 级数 / ≥2.5 连分式），
    误差必须跨切换点连续观察（x=±2.5 附近最容易露问题）。"""
    m = M("e09_erf_grid")
    xs = [-6.0 + 12.0 * i / 1000.0 for i in range(1001)]
    m.inp("X", [1001], xs)
    m.op("Erf", ["X"], ["E"])
    m.out("E", [1001])
    return m


def neg_models():
    """行为负控（必须**报错**）：乱序图要能跑（不是负控）、缺算子/环/广播不兼容要报错"""
    out = []
    # n01：节点顺序**整体颠倒**（ONNX 要求拓扑序，但执行器不该依赖它）
    m = M("n01_reversed")
    m.inp("A", [2, 3], [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])
    m.init("B", [3], [10.0, 20.0, 30.0])
    m.op("Add", ["A", "B"], ["C"])
    m.op("Mul", ["C", "A"], ["D"])
    m.nodes.reverse()
    m.out("D", [2, 3])
    out.append(("n01_reversed", m, False))          # 期望成功
    # n02：未实现的算子
    m2 = M("n02_unknown_op")
    m2.inp("A", [2], [1.0, 2.0])
    m2.op("MysteryOp", ["A"], ["B"])
    m2.out("B", [2])
    out.append(("n02_unknown_op", m2, True))        # 期望失败
    # n03：环（X 依赖 Y，Y 依赖 X）
    m3 = M("n03_cycle")
    m3.inp("A", [2], [1.0, 2.0])
    m3.op("Add", ["A", "Y"], ["X"])
    m3.op("Add", ["X", "A"], ["Y"])
    m3.out("X", [2])
    out.append(("n03_cycle", m3, True))
    # n04：广播不兼容（[2,3] + [4]）
    m4 = M("n04_bad_broadcast")
    m4.inp("A", [2, 3], [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])
    m4.init("B", [4], [1.0, 2.0, 3.0, 4.0])
    m4.op("Add", ["A", "B"], ["C"])
    m4.out("C", [2, 3])
    out.append(("n04_bad_broadcast", m4, True))
    # n05：feed 少给一个输入（A 给了、B 没给且不是 initializer）
    m5 = M("n05_missing_feed")
    m5.inputs.append(("A", [2], FLOAT))
    m5.inputs.append(("B", [2], FLOAT))
    m5.feeds["A"] = {"dims": [2], "dtype": "float", "f32": [1.0, 2.0]}
    m5.op("Add", ["A", "B"], ["C"])
    m5.out("C", [2])
    out.append(("n05_missing_feed", m5, True))
    return out


def m10_gather_rank2():
    """Gather 的**秩变化**用例（axis=0，data 秩 2，indices 秩 1）：
    正确输出 = data.dims[:0] + indices.dims + data.dims[1:] = [2, 3]；
    若把 indices 的形状追加到末尾（常见实现错法）⇒ [3, 2] ⇒ 形状立刻判红。
    为什么单独加：M157 的负控 NC-A 就是这条 —— 用秩 1 的 data（Shape 的输出）时
    前缀/后缀都为空，两种写法**形状相同**，篡改观察不到（门要能抓住才叫门）。"""
    m = M("e10_gather_rank2")
    m.inp("D", [2, 3], [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])
    m.init("idx2", [2], [1, 0], INT64)
    m.op("Gather", ["D", "idx2"], ["G"], [attr("axis", "i", 0)])
    m.out("G", [2, 3])
    m.exact("G", [4.0, 5.0, 6.0, 1.0, 2.0, 3.0], "按 axis=0 取 [1,0] ⇒ 两行互换")
    return m


def main():
    os.makedirs(MODELS, exist_ok=True)
    os.makedirs(FEEDS, exist_ok=True)
    ms = [m01_eltwise(), m02_matmul(), m03_softmax_reduce(), m04_movement(),
          m05_gelu_funcs(), m06_layernorm(), m07_const_range(), m08_attn(), m09_erf_grid(), m10_gather_rank2()]
    nego = os.path.join(MODELS, "neg")
    os.makedirs(nego, exist_ok=True)
    negs = []
    for nm, mm, expect_fail in neg_models():
        open(os.path.join(nego, nm + ".onnx"), "wb").write(mm.build())
        json.dump(mm.feeds, open(os.path.join(nego, nm + ".json"), "w"), ensure_ascii=False)
        negs.append({"name": nm, "expect_fail": expect_fail})
    json.dump(negs, open(os.path.join(nego, "cases.json"), "w"), ensure_ascii=False)
    graphs, hands, notes = {}, {}, {}
    for m in ms:
        open(os.path.join(MODELS, m.name + ".onnx"), "wb").write(m.build())
        graphs[m.name] = m.graph_json()
        json.dump(m.feeds, open(os.path.join(FEEDS, m.name + ".json"), "w"),
                  ensure_ascii=False, indent=1)
        if m.hand:
            hands[m.name] = m.hand
            notes[m.name] = m.hand_note
    json.dump(graphs, open(os.path.join(MODELS, "graphs.json"), "w"), ensure_ascii=False, indent=1)
    json.dump(hands, open(os.path.join(MODELS, "expected.json"), "w"), ensure_ascii=False, indent=1)
    json.dump(notes, open(os.path.join(MODELS, "expected_notes.json"), "w"), ensure_ascii=False, indent=1)
    ops = sorted({n["op"] for g in graphs.values() for n in g["nodes"]})
    print("模型 %d 个" % len(ms))
    print("语料覆盖算子 %d 个：%s" % (len(ops), ",".join(ops)))


if __name__ == "__main__":
    main()
