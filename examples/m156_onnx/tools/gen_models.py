#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_models.py —— 构造性 ONNX 模型生成器 + **构造性真值**（M156 门）

**手写 protobuf 编码**（零第三方依赖：本机无 pip / 无 onnx 包）。

## 为什么每个构造函数返回 `(bytes, fact)`

门需要**两类**真值，缺一不可：
  ① 构造性真值（fact）：我知道自己写进去了什么 —— 直接由生成参数给出；
  ② 独立实现真值：`tools/onnx_ref.py`（另一份手写 protobuf 解码器）。
只做 ② 会有一个致命盲区：**两个实现可能在同一份错误输入上"一致地错"**
（本轮就踩了：`node()` 忘了把属性包成 field 5，属性被两侧当未知字段一起丢掉，
事实集照样"完全一致"）。① 才能照出这种错。

覆盖面（每条都对应 C 解析器里一段分支）：
  t01 raw_data / dim_value          t02 全属性类型        t03 非 packed repeated
  t04 窄类型（int32_data 承载）      t05 attribute 无 type  t06 未知字段（前向兼容）
  t07 dim_param（符号维）            t08 负整数（补码 varint）  t09 空图
  t13 大 dim（2^40）                 t14 同名 initializer    t15 8 维（边界）
负控（必须被 C 侧判错）：
  t10 无 graph   t11 截断   t12 field 0   t16 9 维（超 ONNX_MAXDIM）
输出：
  models/*.onnx                模型字节
  models/expected.json         构造性真值（事实集；与 onnx_ref.facts 同格式）
"""
import json
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "models")

DT_NAME = {1: "float", 2: "uint8", 3: "int8", 4: "uint16", 5: "int16", 6: "int32",
           7: "int64", 8: "string", 9: "bool", 10: "float16", 11: "double",
           12: "uint32", 13: "uint64", 16: "bfloat16"}
DT_W = {1: 4, 2: 1, 3: 1, 4: 2, 5: 2, 6: 4, 7: 8, 8: 8, 9: 1, 10: 2, 11: 8, 12: 4, 13: 8, 16: 2}


# ─────────── protobuf 编码原语 ───────────
def vi(v):
    if v < 0:
        v += 1 << 64
    out = bytearray()
    while True:
        b = v & 0x7F
        v >>= 7
        if v:
            out.append(b | 0x80)
        else:
            out.append(b)
            break
    return bytes(out)


def T(f, w):
    return vi((f << 3) | w)


def f_vi(f, v):
    return T(f, 0) + vi(v)


def f_ld(f, b):
    b = b if isinstance(b, bytes) else b.encode()
    return T(f, 2) + vi(len(b)) + b


def f_f32(f, x):
    return T(f, 5) + struct.pack("<f", x)


def f_f64(f, x):
    return T(f, 1) + struct.pack("<d", x)


# ─────────── ONNX 消息（每个返回 (bytes, fact)） ───────────
def tensor(name, dtype, dims, raw=None, float_data=None, int32=None, int64=None,
           double=None, uint64=None, packed=True):
    b = b""
    for d in dims:
        b += f_vi(1, d)
    b += f_vi(2, dtype)
    if raw is not None:
        b += f_ld(9, raw)
        nbytes = len(raw)
    elif float_data is not None:
        b += (f_ld(4, b"".join(struct.pack("<f", x) for x in float_data)) if packed
              else b"".join(f_f32(4, x) for x in float_data))
        nbytes = 4 * len(float_data)
    elif int32 is not None:
        b += (f_ld(5, b"".join(vi(x) for x in int32)) if packed
              else b"".join(f_vi(5, x) for x in int32))
        nbytes = DT_W[dtype] * len(int32)         # 窄类型：每元素 per dtype 宽度
    elif int64 is not None:
        b += (f_ld(7, b"".join(vi(x) for x in int64)) if packed
              else b"".join(f_vi(7, x) for x in int64))
        nbytes = 8 * len(int64)
    elif double is not None:
        b += (f_ld(10, b"".join(struct.pack("<d", x) for x in double)) if packed
              else b"".join(f_f64(10, x) for x in double))
        nbytes = 8 * len(double)
    elif uint64 is not None:
        b += f_ld(11, b"".join(vi(x) for x in uint64))
        nbytes = 8 * len(uint64)
    else:
        nbytes = 0
    b += f_ld(8, name)
    return b, (name, DT_NAME[dtype], tuple(dims), nbytes)


def value_info(name, dtype, dims):
    shape = b""
    for d in dims:
        shape += f_ld(1, (f_ld(2, d) if isinstance(d, str) else f_vi(1, d)))
    tt = f_vi(1, dtype) + f_ld(2, shape)
    return f_ld(1, name) + f_ld(2, f_ld(1, tt)), (name, DT_NAME[dtype], tuple(dims))


def node(op, ins, outs, name="", attrs=None, domain="", raw_extra=b""):
    attrs = attrs or []
    b = b""
    for i in ins:
        b += f_ld(1, i)
    for o in outs:
        b += f_ld(2, o)
    if name:
        b += f_ld(3, name)
    b += f_ld(4, op)
    for ab, _af in attrs:
        b += f_ld(5, ab)          # ★ 属性必须包成 NodeProto.attribute(field 5)
    b += raw_extra                # 未定义字段（前向兼容路径；不产生 fact）
    if domain:
        b += f_ld(7, domain)
    return b, (op, name, domain, tuple(ins), tuple(outs), tuple(af for _ab, af in attrs))


def A_INT(name, v, with_type=True):
    b = f_ld(1, name) + f_vi(3, v)
    return b + (f_vi(20, 2) if with_type else b""), (name, "INT", v)


def A_FLOAT(name, v, with_type=True):
    b = f_ld(1, name) + f_f32(2, v)
    return b + (f_vi(20, 1) if with_type else b""), (name, "FLOAT", v)


def A_STRING(name, s, with_type=True):
    b = f_ld(1, name) + f_ld(4, s)
    return b + (f_vi(20, 3) if with_type else b""), (name, "STRING", s.encode())


def A_INTS(name, vs, with_type=True, packed=True):
    body = (f_ld(8, b"".join(vi(x) for x in vs)) if packed
            else b"".join(f_vi(8, x) for x in vs))
    b = f_ld(1, name) + body
    return b + (f_vi(20, 7) if with_type else b""), (name, "INTS", tuple(vs))


def A_FLOATS(name, vs, with_type=True, packed=True):
    body = (f_ld(7, b"".join(struct.pack("<f", x) for x in vs)) if packed
            else b"".join(f_f32(7, x) for x in vs))
    b = f_ld(1, name) + body
    return b + (f_vi(20, 6) if with_type else b""), (name, "FLOATS", tuple(vs))


def A_TENSOR(name, t, with_type=True):
    tb, tfact = t
    b = f_ld(1, name) + f_ld(5, tb)
    # TENSOR 属性的 fact 口径与 onnx_ref 一致：(dtype_name, dims, nbytes)
    return b + (f_vi(20, 4) if with_type else b""), (name, "TENSOR",
                                                      (tfact[1], tfact[2], tfact[3]))


UNKNOWN = f_vi(999, 12345) + f_ld(1000, b"\x01\x02\x03")   # 未知字段（前向兼容）


def model(nodes, inits, inputs, outputs, ir=7, producer="puxian-m156", graph="g",
          opset=14, extra=b"", graph_extra=b"", nograph=False):
    # ★ 每个子消息都必须包上 GraphProto 的字段号 —— 漏一层包装会让内容"泄漏"到
    #   上一层被当成别的字段（本行曾漏过 `f_ld(1, ...)`，结果是**两侧实现一致地失败**，
    #   正是「只有对拍没有构造性真值」照不出来的那类错）。
    g = b"".join(f_ld(1, nb) for nb, _nf in nodes)
    g += f_ld(2, graph)
    g += b"".join(f_ld(5, ib) for ib, _if in inits)
    g += b"".join(f_ld(11, ib) for ib, _if in inputs)
    g += b"".join(f_ld(12, ob) for ob, _of in outputs)
    g += graph_extra
    m = f_vi(1, ir) + f_ld(2, producer) + f_ld(7, g) + f_ld(8, f_ld(1, "") + f_vi(2, opset)) + extra
    facts = {
        "ir_version": ir, "producer": producer, "producer_version": "",
        "graph": "" if nograph else graph, "opset": [("", opset)],
        "inputs": [f for _b, f in ([] if nograph else inputs)],
        "outputs": [f for _b, f in ([] if nograph else outputs)],
        "value_infos": [],
        "initializers": [f for _b, f in ([] if nograph else inits)],
        "nodes": [f for _b, f in ([] if nograph else nodes)],
    }
    return m, facts


# ─────────── 各用例 ───────────
def build():
    out = {}
    cst = tensor("", 1, [2], raw=struct.pack("<2f", 0.5, -1.5))

    # t01：raw_data（float32）+ dim_value
    out["t01_basic.onnx"] = model(
        [node("MatMul", ["X", "W"], ["Y"], "mm")],
        [tensor("W", 1, [2, 3], raw=struct.pack("<6f", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0))],
        [value_info("X", 1, [1, 2])], [value_info("Y", 1, [1, 3])])

    # t02：全属性类型（含负 INT）
    out["t02_attrs.onnx"] = model(
        [node("Foo", ["A"], ["B"], "n_attr",
              [A_FLOAT("alpha", 0.5), A_INT("axis", -1), A_STRING("mode", "test"),
               A_INTS("axes", [0, 2]), A_FLOATS("scales", [1.0, 2.5]), A_TENSOR("value", cst)])],
        [], [value_info("A", 1, [1, 3])], [value_info("B", 1, [1, 3])])

    # t03：非 packed repeated
    out["t03_unpacked.onnx"] = model(
        [node("Add", ["A", "B"], ["C"], "add", [A_INTS("axes", [-1, 2], packed=False)])],
        [tensor("B", 1, [3], float_data=[1.5, 2.5, 3.5], packed=False),
         tensor("I", 7, [2], int64=[7, -9], packed=False),
         tensor("D", 11, [2], double=[0.25, -0.125], packed=False)],
        [value_info("A", 1, [1, 3])], [value_info("C", 1, [1, 3])])

    # t04：窄类型（int32_data 承载 → 每元素取低 elem_w 字节）
    out["t04_narrow.onnx"] = model(
        [node("Cast", ["A"], ["C"], "cast")],
        [tensor("i8", 3, [3], int32=[-1, 2, -128]),
         tensor("u8", 2, [3], int32=[0, 255, 128]),
         tensor("bo", 9, [2], int32=[1, 0]),
         tensor("f16", 10, [2], int32=[0x3C00, 0xC000])],   # fp16 1.0 / -2.0 位模式
        [value_info("A", 2, [1, 3])], [value_info("C", 2, [1, 3])])

    # t05：attribute 无 type 字段（由字段推断）
    out["t05_notype.onnx"] = model(
        [node("Bar", ["A"], ["B"], "n5",
              [A_INT("i_axis", 3, with_type=False),
               A_FLOATS("fl", [0.5, 1.0], with_type=False),
               A_TENSOR("t", cst, with_type=False)])],
        [], [value_info("A", 1, [1])], [value_info("B", 1, [1])])

    # t06：未知字段（每一层都塞）
    out["t06_unknown.onnx"] = model(
        [node("Baz", ["A"], ["B"], "n6", raw_extra=UNKNOWN)],
        [tensor("U", 1, [1], raw=struct.pack("<f", 9.0))],
        [value_info("A", 1, [1])], [value_info("B", 1, [1])],
        extra=UNKNOWN, graph_extra=UNKNOWN)

    # t07：dim_param（符号维）
    out["t07_symbolic.onnx"] = model(
        [node("Relu", ["X"], ["Y"], "relu")],
        [], [value_info("X", 7, ["batch", "seq"])],
        [value_info("Y", 7, ["batch", "seq"])])

    # t08：负整数（补码 varint）·两个节点各一属性
    out["t08_negative.onnx"] = model(
        [node("Softmax", ["X"], ["Y"], "sm", [A_INT("axis", -1)]),
         node("ReduceMean", ["Y"], ["Z"], "rm", [A_INTS("axes", [-1, -2])])],
        [], [value_info("X", 1, [2, 3])], [value_info("Z", 1, [2, 1])])

    # t09：空图
    out["t09_emptygraph.onnx"] = model([], [], [], [])

    # t13：大 dim（2^40，int64 边界）
    out["t13_bigdim.onnx"] = model(
        [node("Identity", ["A"], ["B"], "id")],
        [], [value_info("A", 1, [1 << 40])], [value_info("B", 1, [1 << 40])])

    # t14：同名 initializer（查找取第一个）
    out["t14_dupname.onnx"] = model(
        [node("Identity", ["D"], ["E"], "id2")],
        [tensor("D", 1, [1], raw=struct.pack("<f", 1.0)),
         tensor("D", 1, [1], raw=struct.pack("<f", 2.0))],
        [], [value_info("E", 1, [1])])

    # t15：8 维（ONNX_MAXDIM 边界）
    out["t15_rank8.onnx"] = model(
        [node("Identity", ["A"], ["B"], "id8")],
        [], [value_info("A", 1, [1, 2, 3, 4, 5, 6, 7, 8])],
        [value_info("B", 1, [1, 2, 3, 4, 5, 6, 7, 8])])

    # ── 负控（必须被 C 侧判错；不给 facts） ──
    good = out["t01_basic.onnx"][0]
    out["t11_truncated.onnx"] = (good[:len(good) - 7], None)
    out["t12_field0.onnx"] = (f_vi(1, 7) + b"\x00" + f_ld(2, "x"), None)
    out["t16_rank9.onnx"] = model(
        [node("Identity", ["A"], ["B"], "id9")],
        [], [value_info("A", 1, [1, 2, 3, 4, 5, 6, 7, 8, 9])],
        [value_info("B", 1, [1, 2, 3, 4, 5, 6, 7, 8, 9])])
    out["t10_nograph.onnx"] = (f_vi(1, 7) + f_ld(2, "puxian-m156"), None)
    return out


def main():
    os.makedirs(OUT, exist_ok=True)
    models = build()
    expected = {}
    for n in sorted(models):
        b, facts = models[n]
        with open(os.path.join(OUT, n), "wb") as f:
            f.write(b)
        if facts is not None:
            expected[n] = facts
        print("%-24s %6d 字节%s" % (n, len(b), "" if facts is not None else "  [负控]"))
    # 构造性真值落盘（列表 → JSON 需要转成 list；tuple 自动变 list，比较侧统一处理）
    def conv(o):
        if isinstance(o, dict):
            return {k: conv(v) for k, v in o.items()}
        if isinstance(o, (list, tuple)):
            return [conv(x) for x in o]
        if isinstance(o, bytes):
            return o.decode("utf-8", "surrogateescape")
        return o
    with open(os.path.join(OUT, "expected.json"), "w", encoding="utf-8") as f:
        json.dump(conv(expected), f, ensure_ascii=False, indent=1, sort_keys=True)
    print("共 %d 个模型（%d 个正例带构造性真值）→ 见 models/expected.json"
          % (len(models), len(expected)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
