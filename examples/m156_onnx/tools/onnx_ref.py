#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""onnx_ref.py —— 独立的 ONNX/protobuf 解码器（M156 对拍真值）

与 `runtime/onnx_proto.c` **分别独立实现**（不共享代码、不共享中间产物）：
两侧各自把同一份 .onnx 解析成「事实集」，再逐条比对。
用途：证明 C 侧解析不是自说自话（对拍精神：两个独立实现读同一份字节）。

用法：
  onnx_ref.py <model.onnx>            打印事实集摘要
  onnx_ref.py <model.onnx> <info.json> 与 C 侧 onnx_info 的 JSON 比对（事实集口径）
"""
import json
import struct
import sys

DT = {0: "undefined", 1: "float", 2: "uint8", 3: "int8", 4: "uint16", 5: "int16",
      6: "int32", 7: "int64", 8: "string", 9: "bool", 10: "float16", 11: "double",
      12: "uint32", 13: "uint64", 16: "bfloat16"}
DTW = {1: 4, 2: 1, 3: 1, 4: 2, 5: 2, 6: 4, 7: 8, 8: 8, 9: 1, 10: 2, 11: 8, 12: 4, 13: 8, 16: 2}

ATTR_NAME = {1: "FLOAT", 2: "INT", 3: "STRING", 4: "TENSOR", 5: "GRAPH", 6: "FLOATS",
             7: "INTS", 8: "STRINGS", 9: "TENSORS", 10: "GRAPHS", 11: "SPARSE_TENSOR",
             12: "SPARSE_TENSORS", 13: "TYPE_PROTO", 14: "TYPE_PROTOS", 0: "UNDEFINED"}
# 窄类型（1/2 字节）在 int32_data 里承载 → 取低 elem_w 字节
NARROW = (2, 3, 4, 5, 9, 10, 16)

# 权重数据的**独立**真值通道：名字 → raw_data（切片引用，不额外占内存）
#   用途：`--init <model> <name>` 打印 head16 / sha256，与 C 侧 onnx_initializer 对拍。
RAW_STORE = {}


class Pb(object):
    __slots__ = ("b", "p", "e")

    def __init__(self, buf, pos=0, end=None):
        self.b = buf
        self.p = pos
        self.e = len(buf) if end is None else end

    def varint(self):
        v = 0
        s = 0
        while self.p < self.e:
            c = self.b[self.p]
            self.p += 1
            v |= (c & 0x7F) << s
            if not (c & 0x80):
                return v
            s += 7
            if s >= 64:
                raise ValueError("varint too long")
        raise ValueError("truncated varint")

    def tag(self):
        k = self.varint()
        f = k >> 3
        w = k & 7
        if f == 0:
            raise ValueError("field 0")
        return f, w

    def skip(self, w):
        if w == 0:
            self.varint()
        elif w == 1:
            self.p += 8
        elif w == 2:
            # 注意：**必须先读长度再推进**。写成 `self.p += self.varint()` 是错的 ——
            # Python 对属性做 `+=` 时左操作数 `self.p` 在 varint() 调用**之前**求值，
            # 于是少推进「长度域自身占的字节数」（本行曾是本文件的真 bug，被 t06 用例照出）。
            n = self.varint()
            self.p += n
        elif w == 5:
            self.p += 4
        else:
            raise ValueError("wire %d" % w)
        if self.p > self.e:
            raise ValueError("overrun")

    def bytes_(self):
        n = self.varint()
        d = self.b[self.p:self.p + n]
        if len(d) != n:
            raise ValueError("truncated bytes")
        self.p += n
        return d

    def f32(self):
        v = int.from_bytes(self.b[self.p:self.p + 4], "little")
        self.p += 4
        return v

    def f64(self):
        v = int.from_bytes(self.b[self.p:self.p + 8], "little")
        self.p += 8
        return v


def s64(v):
    """protobuf int64 用 varint 存**补码**：无符号读出的 2^64-1 就是 -1。"""
    return v - (1 << 64) if v >= (1 << 63) else v


def s32(v):
    """int32 同理（32 位补码）。"""
    v &= 0xFFFFFFFF
    return v - (1 << 32) if v >= (1 << 31) else v


def _rep_int(pb, w):
    """repeated int（varint 或 packed）"""
    if w == 0:
        return [pb.varint()]
    q = Pb(pb.bytes_())
    out = []
    while q.p < q.e:
        out.append(q.varint())
    return out


def _rep_fixed(pb, w, width):
    if w == (5 if width == 4 else 1):
        return [pb.f32() if width == 4 else pb.f64()]
    q = Pb(pb.bytes_())
    if (q.e - q.p) % width:
        raise ValueError("packed fixed len not multiple of %d" % width)
    out = []
    while q.p < q.e:
        out.append(q.f32() if width == 4 else q.f64())
    return out


def p_tensor(buf):
    """TensorProto → {name,dtype,dims,nbytes}（nbytes 口径与 C 侧一致）"""
    pb = Pb(buf)
    dims, i32, i64, fd, f64, u64 = [], [], [], [], [], []
    dtype = 0
    name = None
    raw = None
    while pb.p < pb.e:
        f, w = pb.tag()
        if f == 1:
            if w == 0:
                dims.append(s64(pb.varint()))
            elif w == 2:
                q = Pb(pb.bytes_())
                while q.p < q.e:
                    dims.append(s64(q.varint()))
            else:
                pb.skip(w)
        elif f == 2 and w == 0:
            dtype = pb.varint()
        elif f == 4:
            fd += _rep_fixed(pb, w, 4)
        elif f == 5:
            i32 += [s32(x) for x in _rep_int(pb, w)]
        elif f == 7:
            i64 += [s64(x) for x in _rep_int(pb, w)]
        elif f == 8 and w == 2:
            name = pb.bytes_().decode("utf-8", "surrogateescape")
        elif f == 9 and w == 2:
            raw = pb.bytes_()
        elif f == 10:
            f64 += _rep_fixed(pb, w, 8)
        elif f == 11:
            u64 += _rep_int(pb, w)
        else:
            pb.skip(w)
    if raw is not None:
        RAW_STORE[name or ""] = raw
        nbytes = len(raw)
    else:
        ew = DTW.get(dtype, 0)
        arr, iw = None, 0
        if dtype == 1:
            arr, iw = fd, 4
        elif dtype == 7:
            arr, iw = i64, 8
        elif dtype == 6:
            arr, iw = i32, 4
        elif dtype == 11:
            arr, iw = f64, 8
        elif dtype == 13:
            arr, iw = u64, 8
        elif dtype in NARROW:
            arr, iw = i32, 4
        if arr:
            nbytes = len(arr) * (ew if (iw == 4 and 0 < ew < 4) else iw)
        else:
            nbytes = 0
    return {"name": name or "", "dtype": dtype, "dims": list(dims), "nbytes": nbytes}


def p_attr(buf):
    pb = Pb(buf)
    a = {"name": "", "type": 0, "i": 0, "f": 0.0, "s": None, "ints": [], "floats": [],
         "t": None}
    seen = set()
    while pb.p < pb.e:
        f, w = pb.tag()
        if f == 1 and w == 2:
            a["name"] = pb.bytes_().decode("utf-8", "surrogateescape")
        elif f == 2 and w == 5:
            # 注意：`import struct` 必须在**模块顶部**（放在函数里会让 struct 变成局部名，
            # 于同一函数更早用到的列表推导里引用它是 "free variable ... before assignment"）
            a["f"] = struct.unpack("<f", pb.f32().to_bytes(4, "little"))[0]
            seen.add("f")
        elif f == 3 and w == 0:
            a["i"] = s64(pb.varint())
            seen.add("i")
        elif f == 4 and w == 2:
            a["s"] = pb.bytes_()
            seen.add("s")
        elif f == 5 and w == 2:
            a["t"] = p_tensor(pb.bytes_())
            seen.add("t")
        elif f == 7:
            a["floats"] += [struct.unpack("<f", x.to_bytes(4, "little"))[0]
                            for x in _rep_fixed(pb, w, 4)]
            seen.add("floats")
        elif f == 8:
            a["ints"] += [s64(x) for x in _rep_int(pb, w)]
            seen.add("ints")
        elif f == 20 and w == 0:
            a["type"] = pb.varint()
        else:
            pb.skip(w)
    if a["type"] == 0:   # type 缺失 → 与 C 侧同一推断序（单值优先，其次列表）
        for key, tp in (("t", 4), ("floats", 6), ("ints", 7), ("s", 3), ("f", 1), ("i", 2)):
            if key in seen:
                a["type"] = tp
                break
    return a


def p_node(buf):
    pb = Pb(buf)
    nd = {"op": "", "name": "", "domain": "", "in": [], "out": [], "attrs": []}
    while pb.p < pb.e:
        f, w = pb.tag()
        if f == 1 and w == 2:
            nd["in"].append(pb.bytes_().decode("utf-8", "surrogateescape"))
        elif f == 2 and w == 2:
            nd["out"].append(pb.bytes_().decode("utf-8", "surrogateescape"))
        elif f == 3 and w == 2:
            nd["name"] = pb.bytes_().decode("utf-8", "surrogateescape")
        elif f == 4 and w == 2:
            nd["op"] = pb.bytes_().decode("utf-8", "surrogateescape")
        elif f == 5 and w == 2:
            nd["attrs"].append(p_attr(pb.bytes_()))
        elif f == 7 and w == 2:
            nd["domain"] = pb.bytes_().decode("utf-8", "surrogateescape")
        else:
            pb.skip(w)
    return nd


def p_type(buf, vi):
    pb = Pb(buf)
    while pb.p < pb.e:
        f, w = pb.tag()
        if f == 1 and w == 2:
            tt = Pb(pb.bytes_())
            while tt.p < tt.e:
                f2, w2 = tt.tag()
                if f2 == 1 and w2 == 0:
                    vi["dtype"] = tt.varint()
                elif f2 == 2 and w2 == 2:
                    sh = Pb(tt.bytes_())
                    if vi["ndim"] < 0:
                        vi["ndim"] = 0
                    while sh.p < sh.e:
                        f3, w3 = sh.tag()
                        if f3 == 1 and w3 == 2:
                            dm = Pb(sh.bytes_())
                            dv, dp = -1, None
                            while dm.p < dm.e:
                                f4, w4 = dm.tag()
                                if f4 == 1 and w4 == 0:
                                    dv = s64(dm.varint())
                                elif f4 == 2 and w4 == 2:
                                    dp = dm.bytes_().decode("utf-8", "surrogateescape")
                                else:
                                    dm.skip(w4)
                            vi["dims"].append(dv)
                            vi["dim_param"].append(dp)
                            vi["ndim"] += 1
                        else:
                            sh.skip(w3)
                else:
                    tt.skip(w2)
        else:
            pb.skip(w)


def p_value_info(buf):
    pb = Pb(buf)
    vi = {"name": "", "dtype": 0, "ndim": -1, "dims": [], "dim_param": []}
    while pb.p < pb.e:
        f, w = pb.tag()
        if f == 1 and w == 2:
            vi["name"] = pb.bytes_().decode("utf-8", "surrogateescape")
        elif f == 2 and w == 2:
            p_type(pb.bytes_(), vi)
        else:
            pb.skip(w)
    return vi


def p_opset(buf):
    pb = Pb(buf)
    dom, ver = "", 0
    while pb.p < pb.e:
        f, w = pb.tag()
        if f == 1 and w == 2:
            dom = pb.bytes_().decode("utf-8", "surrogateescape")
        elif f == 2 and w == 0:
            ver = pb.varint()
        else:
            pb.skip(w)
    return dom, ver


def decode(buf):
    """ModelProto → 事实集（与 C 侧 onnx_model_info_json 同口径）"""
    m = {"ir_version": 0, "producer": "", "producer_version": "", "graph": "",
         "opset": [], "inputs": [], "outputs": [], "initializers": [], "nodes": [],
         "value_infos": []}
    pb = Pb(buf)
    while pb.p < pb.e:
        f, w = pb.tag()
        if f == 1 and w == 0:
            m["ir_version"] = pb.varint()
        elif f == 2 and w == 2:
            m["producer"] = pb.bytes_().decode("utf-8", "surrogateescape")
        elif f == 3 and w == 2:
            m["producer_version"] = pb.bytes_().decode("utf-8", "surrogateescape")
        elif f == 7 and w == 2:
            g = Pb(pb.bytes_())
            while g.p < g.e:
                gf, gw = g.tag()
                if gf == 1 and gw == 2:
                    m["nodes"].append(p_node(g.bytes_()))
                elif gf == 2 and gw == 2:
                    m["graph"] = g.bytes_().decode("utf-8", "surrogateescape")
                elif gf == 5 and gw == 2:
                    m["initializers"].append(p_tensor(g.bytes_()))
                elif gf in (11, 12, 13) and gw == 2:
                    vi = p_value_info(g.bytes_())
                    if gf == 11:
                        m["inputs"].append(vi)
                    elif gf == 12:
                        m["outputs"].append(vi)
                    else:
                        m["value_infos"].append(vi)
                else:
                    g.skip(gw)
        elif f == 8 and w == 2:
            m["opset"].append(p_opset(pb.bytes_()))
        else:
            pb.skip(w)
    return m


# ─────────────── 事实集规范化（C JSON ↔ 参考解码器） ───────────────
def _dims_of_c(v):
    return list(v["dims"])


def facts_from_c(j):
    f = {"ir_version": j["ir_version"], "producer": j["producer"],
         "producer_version": j["producer_version"], "graph": j["graph"],
         "opset": [(o["domain"], o["version"]) for o in j["opset"]],
         "inputs": [(v["name"], v["dtype"], tuple(_dims_of_c(v))) for v in j["inputs"]],
         "outputs": [(v["name"], v["dtype"], tuple(_dims_of_c(v))) for v in j["outputs"]],
         "value_infos": [(v["name"], v["dtype"], tuple(_dims_of_c(v))) for v in j["value_infos"]],
         "initializers": [(t["name"], t["dtype"], tuple(t["dims"]), t["nbytes"])
                          for t in j["initializers"]],
         "nodes": []}
    for nd in j["nodes"]:
        attrs = []
        for a in nd["attrs"]:
            tp = a["type"]
            if tp == "INT":
                attrs.append((a["name"], tp, a.get("i")))
            elif tp == "FLOAT":
                attrs.append((a["name"], tp, a.get("f")))
            elif tp == "STRING":
                attrs.append((a["name"], tp, a.get("s")))
            elif tp == "INTS":
                attrs.append((a["name"], tp, tuple(a.get("ints", []))))
            elif tp == "FLOATS":
                attrs.append((a["name"], tp, tuple(a.get("floats", []))))
            elif tp == "TENSOR":
                t = a.get("t")
                attrs.append((a["name"], tp, (t["dtype"], tuple(t["dims"]), t["nbytes"])
                              if t else None))
            else:
                attrs.append((a["name"], tp, None))
        f["nodes"].append((nd["op"], nd["name"], nd["domain"], tuple(nd["in"]),
                           tuple(nd["out"]), tuple(attrs)))
    return f


def _vi_facts(vi):
    dims = []
    for i in range(vi["ndim"]):
        dims.append(vi["dim_param"][i] if vi["dim_param"][i] is not None else vi["dims"][i])
    return (vi["name"], DT.get(vi["dtype"], "undefined"), tuple(dims))


def facts_from_ref(m):
    f = {"ir_version": m["ir_version"], "producer": m["producer"],
         "producer_version": m["producer_version"], "graph": m["graph"],
         "opset": [tuple(x) for x in m["opset"]],
         "inputs": [_vi_facts(v) for v in m["inputs"]],
         "outputs": [_vi_facts(v) for v in m["outputs"]],
         "value_infos": [_vi_facts(v) for v in m["value_infos"]],
         "initializers": [(t["name"], DT.get(t["dtype"], "undefined"), tuple(t["dims"]),
                           t["nbytes"]) for t in m["initializers"]],
         "nodes": []}
    for nd in m["nodes"]:
        attrs = []
        for a in nd["attrs"]:
            tp = ATTR_NAME.get(a["type"], "UNDEFINED")
            if tp == "INT":
                attrs.append((a["name"], tp, a["i"]))
            elif tp == "FLOAT":
                attrs.append((a["name"], tp, a["f"]))
            elif tp == "STRING":
                attrs.append((a["name"], tp,
                              a["s"].decode("utf-8", "surrogateescape") if a["s"] is not None else None))
            elif tp == "INTS":
                attrs.append((a["name"], tp, tuple(a["ints"])))
            elif tp == "FLOATS":
                attrs.append((a["name"], tp, tuple(a["floats"])))
            elif tp == "TENSOR":
                t = a["t"]
                attrs.append((a["name"], tp,
                              (DT.get(t["dtype"], "undefined"), tuple(t["dims"]),
                               t["nbytes"]) if t else None))
            else:
                attrs.append((a["name"], tp, None))
        f["nodes"].append((nd["op"], nd["name"], nd["domain"], tuple(nd["in"]),
                           tuple(nd["out"]), tuple(attrs)))
    return f


def compare(fc, fr, label="", verbose=True):
    bad = 0
    keys = ["ir_version", "producer", "producer_version", "graph", "opset",
            "inputs", "outputs", "value_infos", "initializers", "nodes"]
    for k in keys:
        a, b = fc[k], fr[k]
        if a == b:
            if verbose:
                print("  ✅ %-16s 一致（%s）" % (k, ("%d 项" % len(a)) if isinstance(a, list) else a))
            continue
        bad += 1
        print("  ❌ %-16s 不一致" % k)
        if isinstance(a, list) and isinstance(b, list):
            if len(a) != len(b):
                print("     长度 C=%d 参考=%d" % (len(a), len(b)))
            for i in range(min(len(a), len(b))):
                if a[i] != b[i]:
                    print("     [%d] C      = %r" % (i, a[i]))
                    print("     [%d] 参考   = %r" % (i, b[i]))
                    if bad > 4:
                        break
        else:
            print("     C    = %r" % (a,))
            print("     参考 = %r" % (b,))
    return bad


def norm(x):
    """递归把 list → tuple（dict 值一并处理）——
    比较三方（构造真值 / C 侧 JSON / 参考解码器）时必须先统一形状，
    否则 tuple 与 list 会**内容相同而 `==` 为假**（本轮踩过：12/12 全判不一致）。"""
    if isinstance(x, list):
        return tuple(norm(i) for i in x)
    if isinstance(x, dict):
        return {k: norm(v) for k, v in x.items()}
    return x


def facts_from_expected(j):
    return norm(j)


def triple(model_path, info_path, expected_path):
    """三重对拍：构造性真值（生成参数）· C 侧输出 · 独立参考解码器。
    只做「C ↔ 参考」会有一个盲区：两份实现可能在**同一份错误输入**上一致地错
    （本轮踩过：生成器漏包 NodeProto.attribute 的 field 5，两侧一起当未知字段丢掉）。"""
    import os
    with open(model_path, "rb") as f:
        buf = f.read()
    with open(info_path, "r", encoding="utf-8") as f:
        cj = json.load(f)
    with open(expected_path, "r", encoding="utf-8") as f:
        ej = json.load(f)
    key = os.path.basename(model_path)
    if key not in ej:
        print("  ⚠️  %s 无构造性真值（负控？）" % key)
        return 0
    fc = norm(facts_from_c(cj))
    fr = norm(facts_from_ref(decode(buf)))
    fe = facts_from_expected(ej[key])
    bad = 0
    bad += compare(fe, fr, "构造真值 ↔ 参考", verbose=False)
    bad += compare(fc, fe, "C 侧 ↔ 构造真值", verbose=False)
    if bad == 0:
        n = len(fe.get("nodes", []))
        print("  ✅ %-20s 三重一致（节点 %d · 初始化 %d · 输入 %d · 输出 %d）"
              % (key, n, len(fe.get("initializers", [])), len(fe.get("inputs", [])),
                 len(fe.get("outputs", []))))
    else:
        print("  ❌ %-20s 三重对拍有 %d 组不一致" % (key, bad))
    return 1 if bad else 0


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    if sys.argv[1] == "--triple":
        return triple(sys.argv[2], sys.argv[3], sys.argv[4])
    if sys.argv[1] == "--init":
        import hashlib
        with open(sys.argv[2], "rb") as f:
            buf = f.read()
        decode(buf)                     # 顺带填 RAW_STORE
        want = sys.argv[3]
        if want not in RAW_STORE:
            print("NOT_FOUND %s" % want)
            return 1
        raw = RAW_STORE[want]
        print("name=%s" % want)
        print("nbytes=%d" % len(raw))
        print("head16=%s" % raw[:16].hex())
        print("sha256=%s" % hashlib.sha256(raw).hexdigest())
        return 0
    path = sys.argv[1]
    with open(path, "rb") as f:
        buf = f.read()
    m = decode(buf)
    print("[参考解码器] %s" % path)
    print("  ir=%d producer=%s/%s graph=%s nodes=%d inits=%d in=%d out=%d vi=%d opset=%d"
          % (m["ir_version"], m["producer"], m["producer_version"], m["graph"],
             len(m["nodes"]), len(m["initializers"]), len(m["inputs"]), len(m["outputs"]),
             len(m["value_infos"]), len(m["opset"])))
    # 一致性自检：initializer 的 nbytes 与 prod(dims)×elem_w 是否吻合（raw 之外的口径）
    mism = 0
    for t in m["initializers"]:
        ew = DTW.get(t["dtype"], 0)
        n = 1
        for d in t["dims"]:
            n *= d
        if ew and n * ew != t["nbytes"]:
            mism += 1
            if mism <= 3:
                print("  ⚠️  nbytes 与形状不符: %s dims=%s dtype=%s nbytes=%d 期望=%d"
                      % (t["name"], t["dims"], DT.get(t["dtype"]), t["nbytes"], n * ew))
    if not mism:
        print("  ✅ initializer nbytes 与 prod(dims)×elem_w 全部吻合（%d 项）"
              % len(m["initializers"]))
    if len(sys.argv) > 2:
        with open(sys.argv[2], "r", encoding="utf-8") as f:
            cj = json.load(f)
        print("[对拍] C 侧 JSON(%s) ↔ 参考解码器" % sys.argv[2])
        bad = compare(facts_from_c(cj), facts_from_ref(m), sys.argv[2])
        print("  → %s" % ("✅ 事实集完全一致" if bad == 0 else "❌ 有 %d 组不一致" % bad))
        return 1 if bad else 0
    return 0


if __name__ == "__main__":
    sys.exit(main())
