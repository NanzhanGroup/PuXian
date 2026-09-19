#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""onnx2graphjson.py —— 把任意 .onnx 转成 M157 参考执行器的图级 JSON（真实模型对拍用）

为什么要它：M157 的语料都是手造小模型（op 级对拍用）；**真实 MiniLM** 不可能手造，
   而"只跑通"不等于"算得对"。做法：把真模型解码成图级 JSON，交给 exec_ref.py 的
   独立执行器算一遍，与 C 侧逐值比 ⇒ 真模型也有"独立实现真值"。

为什么不复用 M156 的 onnx_ref.py：那个解码器只保留**事实集**（结构摘要 + raw_data
   切片），不带 int64_data/float_data 通道的数值，做执行器输入不够用。本文件因此自带
   一个小型 protobuf 读者（只覆盖本用途需要的字段），**与 C 侧解析器无关**。

用法： onnx2graphjson.py <model.onnx> <out_graphs.json> [模型名]
"""
import json
import os
import struct
import sys

FLOAT, INT32, INT64, BOOL, UINT8, UINT16, INT16, INT8, DOUBLE = 1, 6, 7, 9, 2, 4, 5, 3, 11


# ══════════════════ 极简 protobuf 读者 ══════════════════
class P:
    def __init__(self, b, p=0, e=None):
        self.b = b
        self.p = p
        self.e = len(b) if e is None else e

    def varint(self):
        s, r = 0, 0
        while True:
            x = self.b[self.p]
            self.p += 1
            r |= (x & 0x7F) << s
            if not (x & 0x80):
                break
            s += 7
        return r

    def tag(self):
        t = self.varint()
        return t >> 3, t & 7

    def blob(self):
        n = self.varint()
        v = self.b[self.p:self.p + n]
        self.p += n
        return v

    def skip(self, w):
        if w == 0:
            self.varint()
        elif w == 1:
            self.p += 8
        elif w == 2:
            self.blob()
        elif w == 5:
            self.p += 4
        else:
            raise ValueError("wire %d" % w)


def s64(v):
    return v - (1 << 64) if v >= (1 << 63) else v


def s32(v):
    return v - (1 << 32) if v >= (1 << 31) else v


def rep_varint(p, w):
    if w == 0:
        return [p.varint()]
    if w == 2:
        q = P(p.b, p.p, p.p + (0 if False else 0))
        n = p.varint()
        end = p.p + n
        out = []
        q = P(p.b, p.p, end)
        while q.p < q.e:
            out.append(q.varint())
        p.p = end
        return out
    p.skip(w)
    return []


def rep_fixed(p, w, width):
    fmt = {4: "<f", 8: "<d"}[width]
    if w == 5:
        v = struct.unpack(fmt, p.b[p.p:p.p + width])[0]
        p.p += width
        return [v]
    if w == 2:
        n = p.varint()
        end = p.p + n
        out = []
        while p.p < end:
            out.append(struct.unpack(fmt, p.b[p.p:p.p + width])[0])
            p.p += width
        return out
    p.skip(w)
    return []


def read_tensor(buf):
    """TensorProto → {name,dtype,dims,vals}"""
    p = P(buf)
    dims, fd, i32, i64, f64, u64, name, raw, dtype = [], [], [], [], [], [], "", None, 0
    while p.p < p.e:
        f, w = p.tag()
        if f == 1:
            dims += [s64(x) for x in rep_varint(p, w)]
        elif f == 2 and w == 0:
            dtype = p.varint()
        elif f == 4:
            fd += rep_fixed(p, w, 4)
        elif f == 5:
            i32 += [s32(x) for x in rep_varint(p, w)]
        elif f == 7:
            i64 += [s64(x) for x in rep_varint(p, w)]
        elif f == 8 and w == 2:
            name = p.blob().decode("utf-8", "replace")
        elif f == 9 and w == 2:
            raw = p.blob()
        elif f == 10:
            f64 += rep_fixed(p, w, 8)
        elif f == 11:
            u64 += rep_varint(p, w)
        else:
            p.skip(w)
    n = 1
    for d in dims:
        n *= d
    if raw is not None and len(raw) >= n * {FLOAT: 4, INT64: 8}.get(dtype, 1):
        if dtype == FLOAT:
            vals = list(struct.unpack("<%df" % n, raw[:n * 4]))
        elif dtype == INT64:
            vals = list(struct.unpack("<%dq" % n, raw[:n * 8]))
        elif dtype == BOOL:
            vals = [1 if x else 0 for x in raw[:n]]
        else:
            vals = [0] * n
    elif fd:
        vals = [float(x) for x in fd]
    elif i64:
        vals = [int(x) for x in i64]
    elif i32:
        vals = [int(x) for x in i32]
    elif f64:
        vals = [float(x) for x in f64]
    elif u64:
        vals = [int(x) for x in u64]
    else:
        vals = [0] * n
    return {"name": name, "dtype": dtype, "dims": dims, "vals": vals}


def read_attr(buf):
    """AttributeProto → {name,type,i,f,ints,floats,t}"""
    p = P(buf)
    a = {"name": "", "type": 0, "i": 0, "f": 0.0, "ints": [], "floats": [], "t": None, "s": None}
    while p.p < p.e:
        f, w = p.tag()
        if f == 1 and w == 2:
            a["name"] = p.blob().decode("utf-8", "replace")
        elif f == 2 and w == 5:
            a["f"] = struct.unpack("<f", p.b[p.p:p.p + 4])[0]
            p.p += 4
        elif f == 3 and w == 0:
            a["i"] = s64(p.varint())
        elif f == 4 and w == 2:
            a["s"] = p.blob()
        elif f == 5 and w == 2:
            a["t"] = read_tensor(p.blob())
        elif f == 7:
            a["floats"] += rep_fixed(p, w, 4)
        elif f == 8:
            a["ints"] += [s64(x) for x in rep_varint(p, w)]
        elif f == 20 and w == 0:
            a["type"] = p.varint()
        else:
            p.skip(w)
    if not a["type"]:
        if a["t"] is not None:
            a["type"] = 4
        elif a["ints"]:
            a["type"] = 7
        elif a["floats"]:
            a["type"] = 6
        elif a["s"] is not None:
            a["type"] = 3
    return a


def read_node(buf):
    p = P(buf)
    nd = {"in": [], "out": [], "op": "", "name": "", "attrs": [], "domain": ""}
    while p.p < p.e:
        f, w = p.tag()
        if f == 1 and w == 2:
            nd["in"].append(p.blob().decode("utf-8", "replace"))
        elif f == 2 and w == 2:
            nd["out"].append(p.blob().decode("utf-8", "replace"))
        elif f == 3 and w == 2:
            nd["name"] = p.blob().decode("utf-8", "replace")
        elif f == 4 and w == 2:
            nd["op"] = p.blob().decode("utf-8", "replace")
        elif f == 5 and w == 2:
            nd["attrs"].append(read_attr(p.blob()))
        elif f == 7 and w == 2:
            nd["domain"] = p.blob().decode("utf-8", "replace")
        else:
            p.skip(w)
    return nd


def read_type(buf):
    p = P(buf)
    out = {"dtype": 0, "dims": [], "params": []}
    while p.p < p.e:
        f, w = p.tag()
        if f == 1 and w == 2:                      # tensor_type
            q = P(p.blob())
            while q.p < q.e:
                g, w2 = q.tag()
                if g == 1 and w2 == 0:
                    out["dtype"] = q.varint()
                elif g == 2 and w2 == 2:           # shape
                    r = P(q.blob())
                    while r.p < r.e:
                        h, w3 = r.tag()
                        if h == 1 and w3 == 2:     # dim
                            z = P(r.blob())
                            dv, dp = None, None
                            while z.p < z.e:
                                k, w4 = z.tag()
                                if k == 1 and w4 == 0:
                                    dv = s64(z.varint())
                                elif k == 2 and w4 == 2:
                                    dp = z.blob().decode("utf-8", "replace")
                                else:
                                    z.skip(w4)
                            out["dims"].append(dv if dv is not None else -1)
                            out["params"].append(dp)
                        else:
                            r.skip(w3)
                else:
                    q.skip(w2)
        else:
            p.skip(w)
    return out


def read_value_info(buf):
    p = P(buf)
    vi = {"name": "", "dtype": 0, "dims": []}
    while p.p < p.e:
        f, w = p.tag()
        if f == 1 and w == 2:
            vi["name"] = p.blob().decode("utf-8", "replace")
        elif f == 2 and w == 2:
            t = read_type(p.blob())
            vi["dtype"] = t["dtype"]
            vi["dims"] = t["dims"]
        else:
            p.skip(w)
    return vi


def read_model(buf):
    p = P(buf)
    m = {"nodes": [], "initializers": [], "inputs": [], "outputs": [], "opset": 14}
    while p.p < p.e:
        f, w = p.tag()
        if f == 7 and w == 2:                      # graph
            g = P(p.blob())
            while g.p < g.e:
                gf, gw = g.tag()
                if gf == 1 and gw == 2:
                    m["nodes"].append(read_node(g.blob()))
                elif gf == 5 and gw == 2:
                    m["initializers"].append(read_tensor(g.blob()))
                elif gf == 11 and gw == 2:
                    m["inputs"].append(read_value_info(g.blob()))
                elif gf == 12 and gw == 2:
                    m["outputs"].append(read_value_info(g.blob()))
                else:
                    g.skip(gw)
        elif f == 8 and w == 2:                    # opset_import
            q = P(p.blob())
            while q.p < q.e:
                of, ow = q.tag()
                if of == 2 and ow == 0:
                    m["opset"] = q.varint()
                else:
                    q.skip(ow)
        else:
            p.skip(w)
    return m


def main():
    path, out = sys.argv[1], sys.argv[2]
    name = sys.argv[3] if len(sys.argv) > 3 else os.path.basename(path).replace(".onnx", "")
    m = read_model(open(path, "rb").read())
    g = {"name": name, "opset": m["opset"],
         "inputs": [{"name": i["name"], "dims": i["dims"], "dtype": i["dtype"]} for i in m["inputs"]],
         "outputs": [{"name": o["name"], "dims": o["dims"], "dtype": o["dtype"]} for o in m["outputs"]],
         "initializers": [{"name": t["name"], "dims": t["dims"], "vals": t["vals"],
                           "dtype": t["dtype"]} for t in m["initializers"]],
         "nodes": []}
    for nd in m["nodes"]:
        attrs = {}
        for a in nd["attrs"]:
            if a["type"] == 4 and a["t"] is not None:
                attrs[a["name"]] = {"kind": "tensor", "name": a["t"]["name"],
                                    "dims": a["t"]["dims"], "vals": a["t"]["vals"],
                                    "dtype": a["t"]["dtype"]}
            elif a["type"] == 7:
                attrs[a["name"]] = [int(x) for x in a["ints"]]
            elif a["type"] == 6:
                attrs[a["name"]] = [float(x) for x in a["floats"]]
            elif a["type"] == 2:
                attrs[a["name"]] = int(a["i"])
            elif a["type"] == 1:
                attrs[a["name"]] = float(a["f"])
            elif a["type"] == 3:
                attrs[a["name"]] = (a["s"] or b"").decode("utf-8", "replace")
        g["nodes"].append({"op": nd["op"], "in": nd["in"], "out": nd["out"],
                           "attrs": attrs, "name": nd["name"]})
    json.dump({name: g}, open(out, "w"), ensure_ascii=False)
    print("已写出 %s：节点 %d · initializer %d · 输入 %s · 输出 %s · opset %d"
          % (out, len(g["nodes"]), len(g["initializers"]),
             [i["name"] for i in g["inputs"]], [o["name"] for o in g["outputs"]], m["opset"]))


if __name__ == "__main__":
    main()
