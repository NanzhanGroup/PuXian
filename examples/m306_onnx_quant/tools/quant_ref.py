#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""quant_ref.py —— M306 门的**独立参考实现** + 三方对拍

三方关系（沿用 M156 §38.4 / M157 的方法论）：
   手算真值（models/expected.json，构造性）
     ├── 对拍 A：**独立参考**（本文件，纯 Python 从零写：自己的半精度位运算、
     │           自己的仿射/舍入/饱和）—— 验证"我真的理解这份语义"
     └── 对拍 B：**C 侧**（tools/px 编译出的 quant_exec 产物，读同一批 .onnx）

⚠️ 只做 B（C ⇄ 手算）不够：两侧可能**一致地错**（例如我对 ONNX 语义的理解整体偏了）。
   只做 A 也不够：它不证明 C 侧对。故两个都做，并各自独立报数。

用法： python3 tools/quant_ref.py <C输出目录> <spec.json> <expected.json> <feeds目录>
"""
import json
import math
import os
import struct
import sys
from fractions import Fraction

SDT = {"float": 1, "uint8": 2, "int8": 3, "uint16": 4, "int16": 5, "int32": 6,
       "int64": 7, "bool": 9, "float16": 10, "double": 11, "bfloat16": 16}
NAME = {v: k for k, v in SDT.items()}
_UFMT = {1: "<f", 11: "<d", 6: "<i", 7: "<q", 3: "<b", 2: "<B", 9: "<B",
         4: "<H", 5: "<h", 10: "<H", 16: "<H"}
_SZ = {1: 4, 11: 8, 6: 4, 7: 8, 3: 1, 2: 1, 9: 1, 4: 2, 5: 2, 10: 2, 16: 2}
ISFLOAT = {1, 11, 10, 16}


# ── 半精度（**独立实现**：不依赖 struct 的 f/e 格式支持，纯整数/有理数运算）──
def f16_to_f(h):
    s, e, m = (h >> 15) & 1, (h >> 10) & 0x1F, h & 0x3FF
    if e == 0:
        v = m * Fraction(1, 2 ** 24)
    elif e == 31:
        return float("-inf") if s else (float("nan") if m else float("inf"))
    else:
        v = Fraction(1024 + m, 1024) * Fraction(2) ** (e - 15)
    f = float(v)
    return -f if s else f


def _floor_log2(fr):
    e = 0
    while fr >= 2:
        fr /= 2
        e += 1
    while fr < 1:
        fr *= 2
        e -= 1
    return e


def _rne(fr):
    """Fraction ⇒ 就近偶数取整（非负）"""
    n, d = fr.numerator, fr.denominator
    q, r = divmod(n, d)
    if 2 * r > d or (2 * r == d and q % 2 == 1):
        q += 1
    return q


def f_to_f16(x):
    if math.isnan(x):
        return 0x7E00
    sign = 0x8000 if math.copysign(1.0, x) < 0 else 0
    fr = Fraction(abs(x))
    if fr == 0:
        return sign
    if fr >= 65520:                       # 就近偶数下 ≥65520 ⇒ 上溢到 Inf
        return sign | 0x7C00
    if fr < Fraction(1, 2 ** 25):         # 与最小次正规的中点之下 ⇒ 0
        return sign
    if fr < Fraction(1, 2 ** 14):         # 次正规区（步长 2^-24）
        q = _rne(fr * 2 ** 24)
        return sign | q                   # q 最大 1023，不会越界
    e = _floor_log2(fr)
    mi = _rne(fr * Fraction(2) ** (10 - e))
    if mi >= 2048:
        e, mi = e + 1, 1024
    if e + 15 >= 31:
        return sign | 0x7C00
    return sign | ((e + 15) << 10) | (mi - 1024)


def bf16_to_f(h):
    return struct.unpack("<f", struct.pack("<I", (h & 0xFFFF) << 16))[0]


def f_to_bf16(x):
    u = struct.unpack("<I", struct.pack("<f", x))[0]
    top, rem = u >> 16, u & 0xFFFF
    if (u & 0x7F800000) == 0x7F800000:              # Inf / NaN
        if (u & 0x7FFFFF) and (top & 0x7F) == 0:
            top |= 1                                  # NaN 不许退化成 Inf
        return top & 0xFFFF
    if rem > 0x8000 or (rem == 0x8000 and (top & 1)):
        top += 1
    return top & 0xFFFF


# ── 字节 ⇄ 值 ──
def decode(dt, raw, n):
    v = list(struct.unpack("<%d%s" % (n, _UFMT[dt][1]), raw))
    if dt == 10:
        return [f16_to_f(h) for h in v]
    if dt == 16:
        return [bf16_to_f(h) for h in v]
    return v


def as_f32(x):
    return struct.unpack("<f", struct.pack("<f", x))[0]


def r_half_even(x):
    """就近偶数（不依赖 Python 的 round，因为它对 .5 的行为与 C 侧要逐步对齐）"""
    if math.isnan(x) or math.isinf(x):
        return x
    f = math.floor(x)
    d = x - f
    if d > 0.5:
        return f + 1
    if d < 0.5:
        return f
    return f if math.fmod(f, 2.0) == 0.0 else f + 1


QRANGE = {3: (-128, 127), 2: (0, 255), 5: (-32768, 32767), 4: (0, 65535),
          6: (-2147483648, 2147483647)}


def _val(c, feeds):
    dt = SDT[c["dtype"]]
    if c["name"] in feeds:
        f = feeds[c["name"]]
        return decode(SDT[f["dtype"]], struct.pack("<%d%s" % (len(f["f32"]), "f"),
                                                   *f["f32"]), len(f["f32"]))
    if dt == 10:                       # 半精度常量：vals 里**已经是位型**
        return [f16_to_f(h) for h in c["vals"]]
    if dt == 16:
        return [bf16_to_f(h) for h in c["vals"]]
    return list(c["vals"])


def run_model(m, feeds):
    env, byop = {}, {}
    for c in m.get("consts", []):
        env[c["name"]] = {"dt": SDT[c["dtype"]], "vals": _val(c, feeds),
                          "dims": list(c["dims"]), "name": c["name"]}
    for g in m.get("inputs", []):                 # 图输入：值来自 feeds
        fd = feeds[g["name"]]
        env[g["name"]] = {"dt": SDT[fd["dtype"]], "dims": list(fd["dims"]),
                          "vals": [as_f32(v) for v in fd["f32"]], "name": g["name"]}
    for nd in m["nodes"]:
        op = nd["op"]
        byop[op] = byop.get(op, 0) + 1
        a = {k: v for k, _t, v in nd.get("attrs", ())}
        ins = [env[n] if n else None for n in nd["in"]]   # 空名 =「该位置未提供」⇒ 保留占位
        if op == "DequantizeLinear":
            x, sc = ins[0], ins[1]
            zp = ins[2] if len(ins) > 2 else None
            dims = list(ins[0].get("dims", []))
            nsc = len(sc["vals"])
            per = nsc != 1
            axis = a.get("axis", 1)
            if per:
                axis = axis + len(dims) if axis < 0 else axis
                inner = 1
                for d in dims[axis + 1:]:
                    inner *= d
            else:
                inner = 1
            out = []
            for i, xv in enumerate(x["vals"]):
                si = (i // inner) % nsc if per else 0
                zv = zp["vals"][si] if zp else 0
                out.append(as_f32((float(xv) - float(zv)) * float(sc["vals"][si])))
            env[nd["out"][0]] = {"dt": 1, "vals": out, "dims": dims, "name": nd["out"][0]}
        elif op == "QuantizeLinear":
            x, sc = ins[0], ins[1]
            zp = ins[2] if len(ins) > 2 else None
            odt = zp["dt"] if zp else 2
            lo, hi = QRANGE[odt]
            dims = list(x.get("dims", []))
            nsc = len(sc["vals"])
            per = nsc != 1
            axis = a.get("axis", 1)
            inner = 1
            if per:
                axis = axis + len(dims) if axis < 0 else axis
                for d in dims[axis + 1:]:
                    inner *= d
            out = []
            for i, xv in enumerate(x["vals"]):
                si = (i // inner) % nsc if per else 0
                r = r_half_even(float(xv) / float(sc["vals"][si])) + (zp["vals"][si] if zp else 0)
                r = r if r >= lo else lo
                r = hi if r > hi else r
                out.append(int(r))
            env[nd["out"][0]] = {"dt": odt, "vals": out, "dims": dims, "name": nd["out"][0]}
        elif op == "DynamicQuantizeLinear":
            x = ins[0]
            xs = [float(v) for v in x["vals"]]
            mn = min([0.0] + xs)                 # 规范：min/max **含 0**
            mx = max([0.0] + xs)
            sc = (mx - mn) / 255.0
            if sc == 0.0 or sc != sc or math.isinf(sc):
                sc = 1.0
            sc = as_f32(sc)
            zp = r_half_even(0.0 - mn / sc)
            zp = zp if zp >= 0.0 else 0.0
            zp = 255.0 if zp > 255.0 else zp
            yv = []
            for v in xs:
                r = r_half_even(v / sc) + zp
                r = r if r >= 0.0 else 0.0
                r = 255.0 if r > 255.0 else r
                yv.append(int(r))
            for idx, (dt, vals, dims) in enumerate(((2, yv, list(x["dims"])),
                                                    (1, [sc], []),
                                                    (2, [int(zp)], []))):
                env[nd["out"][idx]] = {"dt": dt, "vals": vals, "dims": dims,
                                       "name": nd["out"][idx]}
        elif op == "MatMulInteger":
            A, B = ins[0], ins[1]
            AZ = ins[2] if len(ins) > 2 else None
            BZ = ins[3] if len(ins) > 3 else None
            M, K = A["dims"][-2], A["dims"][-1]
            K2, N = B["dims"][-2], B["dims"][-1]
            assert K == K2, "MatMulInteger 内维不匹配"
            av = [int(v) for v in A["vals"]]
            bv = [int(v) for v in B["vals"]]
            azv = [int(v) for v in AZ["vals"]] if AZ else [0]
            bzv = [int(v) for v in BZ["vals"]] if BZ else [0]
            out = []
            for i in range(M):
                az = azv[i] if len(azv) > 1 else azv[0]
                for j in range(N):
                    bz = bzv[j] if len(bzv) > 1 else bzv[0]
                    out.append(sum((av[i * K + k] - az) * (bv[k * N + j] - bz)
                                   for k in range(K)))
            env[nd["out"][0]] = {"dt": 6, "vals": out, "dims": [M, N], "name": nd["out"][0]}
        elif op == "Cast":
            to = int(a["to"])
            src = ins[0]
            out = []
            for v in src["vals"]:
                if to in ISFLOAT:
                    out.append(as_f32(f16_to_f(f_to_f16(v)) if to == 10 else
                                      (bf16_to_f(f_to_bf16(struct.unpack('<f', struct.pack('<f', v))[0]))
                                       if to == 16 else float(v))))
                elif to == 9:
                    out.append(1 if float(v) != 0.0 else 0)
                else:
                    out.append(int(v))
            env[nd["out"][0]] = {"dt": to, "vals": out, "dims": list(src["dims"]),
                                 "name": nd["out"][0]}
        elif op == "MatMul":
            # rank-2 × rank-2（本门语料只用这一形态）；中间量按 float32 逐步落库
            A, B = ins[0], ins[1]
            M, K = A["dims"][-2], A["dims"][-1]
            K2, N = B["dims"][-2], B["dims"][-1]
            assert K == K2, "MatMul 内维不匹配"
            out = []
            for r in range(M):
                for c in range(N):
                    s = 0.0
                    for k in range(K):
                        s += as_f32(float(A["vals"][r * K + k]) * float(B["vals"][k * N + c]))
                    out.append(as_f32(s))
            env[nd["out"][0]] = {"dt": 1, "vals": out, "dims": [M, N], "name": nd["out"][0]}
        else:
            raise SystemExit("参考实现不认识算子 %s" % op)
    return {o["name"]: env[o["name"]] for o in m["outputs"] if o["name"] in env}, byop


def eq(a, b):
    if isinstance(a, float) or isinstance(b, float):
        if isinstance(a, float) and math.isnan(a):
            return isinstance(b, float) and math.isnan(b)
        return float(a) == float(b)
    return int(a) == int(b)


def main():
    cdir, specf, expf, feeddir = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
    spec = json.load(open(specf))
    exp = json.load(open(expf))
    ok = bad = 0
    for m in spec["models"]:
        name = m["name"]
        feeds = json.load(open(os.path.join(feeddir, name + ".json")))
        ref, _byop = run_model(m, feeds)
        cpath = os.path.join(cdir, name + ".json")
        if not os.path.exists(cpath):
            print("  ❌ %s：C 侧没有产物" % name)
            bad += 1
            continue
        cd = json.load(open(cpath))
        truth = exp[name]
        msgs = []
        npt = 0
        for oname, t in truth.items():
            dt = SDT[t["dtype"]]
            cvals = decode(dt, bytes.fromhex(cd["outputs"][oname]["hex"]), len(t["values"]))
            rvals = ref[oname]["vals"]
            npt += len(t["values"])
            if len(cvals) != len(t["values"]) or len(rvals) != len(t["values"]):
                msgs.append("%s：长度 C=%d 参考=%d 真值=%d" % (oname, len(cvals), len(rvals),
                                                              len(t["values"])))
                continue
            for i, (tv, cv, rv) in enumerate(zip(t["values"], cvals, rvals)):
                if not eq(cv, tv):
                    msgs.append("%s[%d]：C=%r ≠ 真值 %r" % (oname, i, cv, tv))
                if not eq(rv, tv):
                    msgs.append("%s[%d]：参考=%r ≠ 真值 %r" % (oname, i, rv, tv))
        if msgs:
            print("  ❌ %s（%s）" % (name, "；".join(msgs[:4])))
            bad += 1
        else:
            print("  ✅ %s（参考==手算 · C==手算 · %d 点）" % (name, npt))
            ok += 1
    print("M306-REF: %d OK / %d BAD" % (ok, bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
