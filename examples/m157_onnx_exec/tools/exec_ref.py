#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""exec_ref.py —— M157 的**独立参考执行器** + 三方对拍器（纯 Python，无第三方库）

它是 M157 门里"三条腿"中的一条：
  ① **手算真值**（expected.json，gen_exec_models.py 里逐条写死）
  ② **独立参考**（本文件：只按 ONNX 算子定义从零实现，绝不参考 C 代码）
  ③ **被测实现**（runtime/onnx_*.c，语言侧走 onnx_run）

本文件做的比对：
  · ② vs ① —— 先证明"参考实现没写错"（否则 ②↔③ 一致也没意义）
  · ③ vs ② —— 证明 C 侧没写错（浮点用容差，int64/bool 逐位）
  · ③ vs ① —— 手算那部分直接对（对 C 侧更强的判据）

为什么不用 numpy：本机无 pip / 无第三方包（与 M156 同环境约束）。
张量规模都是小语料，纯 Python 足够快。
"""
import json
import math
import os
import struct
import sys
import itertools

FLOAT, INT64, BOOL, INT32 = 1, 7, 9, 6
DTNAME = {FLOAT: "float", INT64: "int64", BOOL: "bool", INT32: "int32"}


# ══════════════════ 张量工具（行主序） ══════════════════
def T(dims, vals, dtype=FLOAT):
    return {"dims": list(dims), "vals": list(vals), "dtype": dtype}


def prod(dims):
    n = 1
    for d in dims:
        n *= d
    return n


def st_of(dims):
    st = [1] * len(dims)
    for i in range(len(dims) - 2, -1, -1):
        st[i] = st[i + 1] * dims[i + 1]
    return st


def coords(dims):
    return [list(c) for c in itertools.product(*[range(d) for d in dims])]


def get_b(t, idx, oshape):
    """按 NumPy 广播语义取 t 在输出坐标 idx（在 oshape 下）处的元素"""
    shift = len(oshape) - len(t["dims"])
    st = st_of(t["dims"])
    off = 0
    for i, d in enumerate(t["dims"]):
        j = i + shift
        c = idx[j] if j >= 0 else 0
        if d == 1:
            c = 0
        off += c * st[i]
    return t["vals"][off]


def bcast2(a, b):
    n = max(len(a), len(b))
    out = []
    for i in range(n):
        x = a[len(a) - n + i] if len(a) - n + i >= 0 else 1
        y = b[len(b) - n + i] if len(b) - n + i >= 0 else 1
        if x == y:
            out.append(x)
        elif x == 1:
            out.append(y)
        elif y == 1:
            out.append(x)
        else:
            raise ValueError("广播不兼容: %s vs %s" % (a, b))
    return out


def bcast_all(shapes):
    out = []
    for s in shapes:
        out = bcast2(out, s)
    return out


def ew(ts, fn, dtype=FLOAT, odims=None):
    dims = odims or bcast_all([t["dims"] for t in ts])
    vals = []
    for idx in coords(dims):
        vals.append(fn(*[get_b(t, idx, dims) for t in ts]))
    return T(dims, vals, dtype)


def clone(t):
    return T(t["dims"], t["vals"], t["dtype"])


# ══════════════════ 算子实现（按 ONNX 定义从零写） ══════════════════
def op_matmul(env, n):
    A, B = env[n["in"][0]], env[n["in"][1]]
    ad, bd = A["dims"], B["dims"]
    va, vb = len(ad) == 1, len(bd) == 1
    a = ([1] + list(ad)) if va else list(ad)
    b = (list(bd) + [1]) if vb else list(bd)
    M, K, K2, N = a[-2], a[-1], b[-2], b[-1]
    assert K == K2, "MatMul 内维不一致"
    batch = bcast_all([a[:-2], b[:-2]])
    vals = []
    for bi in coords(batch):
        for i in range(M):
            for j in range(N):
                acc = 0.0
                for k in range(K):
                    acc += get_b(A, bi + [i, k], batch + [M, K]) * \
                           get_b(B, bi + [k, j], batch + [K, N])
                vals.append(acc)
    od = list(batch)
    if not va:
        od.append(M)
    if not vb:
        od.append(N)
    return T(od, vals, A["dtype"])


def axes_of(env, n, default_all=True):
    if "axes" in n["attrs"]:
        return list(n["attrs"]["axes"])
    if len(n["in"]) >= 2 and n["in"][1]:
        return [int(v) for v in env[n["in"][1]]["vals"]]
    return None if not default_all else None


def op_reduce(env, n, kind):
    X = env[n["in"][0]]
    nd = len(X["dims"])
    ax = axes_of(env, n)
    if ax is None:
        ax = list(range(nd))
    ax = sorted({(a + nd) if a < 0 else a for a in ax})
    keep = int(n["attrs"].get("keepdims", 1))
    oshape = [1 if i in ax else d for i, d in enumerate(X["dims"])]
    n_out = prod(oshape)
    if kind == "prod":
        acc = [1.0] * n_out
    elif kind == "max":
        acc = [-math.inf] * n_out
    elif kind == "min":
        acc = [math.inf] * n_out
    else:
        acc = [0.0] * n_out
    ost = st_of(oshape)
    for idx in coords(X["dims"]):
        o = 0
        for i, c in enumerate(idx):
            if i not in ax:
                o += c * ost[i]
        v = X["vals"][sum(idx[i] * st_of(X["dims"])[i] for i in range(nd))]
        if kind in ("sum", "mean"):
            acc[o] += v
        elif kind == "prod":
            acc[o] *= v
        elif kind == "max":
            acc[o] = max(acc[o], v)
        elif kind == "min":
            acc[o] = min(acc[o], v)
    if kind == "mean":
        cnt = prod([X["dims"][i] for i in ax])
        acc = [x / cnt for x in acc]
    odims = oshape if keep else [d for i, d in enumerate(X["dims"]) if i not in ax]
    return T(odims, acc, FLOAT if kind == "mean" else X["dtype"])


def op_softmax(env, n):
    X = env[n["in"][0]]
    nd = len(X["dims"])
    ax = int(n["attrs"].get("axis", -1))
    if ax < 0:
        ax += nd
    outer = prod(X["dims"][:ax])
    inner = prod(X["dims"][ax + 1:])
    m = X["dims"][ax]
    out = [0.0] * prod(X["dims"])
    for o in range(outer):
        for k in range(inner):
            base = o * m * inner + k
            vals = [X["vals"][base + i * inner] for i in range(m)]
            mx = max(vals)
            es = [math.exp(v - mx) for v in vals]
            s = sum(es)
            for i in range(m):
                out[base + i * inner] = es[i] / s
    return T(X["dims"], out, FLOAT)


def op_argmax(env, n):
    X = env[n["in"][0]]
    nd = len(X["dims"])
    ax = int(n["attrs"].get("axis", 0))
    if ax < 0:
        ax += nd
    keep = int(n["attrs"].get("keepdims", 1))
    outer = prod(X["dims"][:ax])
    inner = prod(X["dims"][ax + 1:])
    m = X["dims"][ax]
    out = []
    for o in range(outer):
        for k in range(inner):
            base = o * m * inner + k
            best, bv = 0, -math.inf
            for i in range(m):
                v = X["vals"][base + i * inner]
                if v > bv:
                    bv, best = v, i
            out.append(best)
    od = [1 if i == ax else d for i, d in enumerate(X["dims"])] if keep else \
         [d for i, d in enumerate(X["dims"]) if i != ax]
    return T(od, out, INT64)


def op_layernorm(env, n):
    X = env[n["in"][0]]
    nd = len(X["dims"])
    ax = int(n["attrs"].get("axis", -1))
    if ax < 0:
        ax += nd
    eps = float(n["attrs"].get("epsilon", 1e-5))
    outer = prod(X["dims"][:ax])
    inner = prod(X["dims"][ax:])
    scale = env[n["in"][1]]["vals"] if len(n["in"]) > 1 and n["in"][1] else None
    bias = env[n["in"][2]]["vals"] if len(n["in"]) > 2 and n["in"][2] else None
    out = [0.0] * prod(X["dims"])
    for o in range(outer):
        base = o * inner
        seg = X["vals"][base:base + inner]
        mean = sum(seg) / inner
        var = sum((v - mean) ** 2 for v in seg) / inner
        inv = 1.0 / math.sqrt(var + eps)
        for i in range(inner):
            v = (seg[i] - mean) * inv
            if scale:
                v *= scale[i % len(scale)]
            if bias:
                v += bias[i % len(bias)]
            out[base + i] = v
    return T(X["dims"], out, FLOAT)


def op_transpose(env, n):
    X = env[n["in"][0]]
    nd = len(X["dims"])
    perm = list(n["attrs"].get("perm", list(range(nd - 1, -1, -1))))
    od = [X["dims"][p] for p in perm]
    ost = st_of(od)
    out = [0.0] * prod(od)
    for idx in coords(X["dims"]):
        # 源坐标 idx → 输出坐标：输出轴 i 对应输入轴 perm[i]（od[i] = X.dims[perm[i]]）
        oidx = [idx[perm[i]] for i in range(nd)]
        o = sum(oidx[i] * ost[i] for i in range(nd))
        s = sum(idx[i] * st_of(X["dims"])[i] for i in range(nd))
        out[o] = X["vals"][s]
    return T(od, out, X["dtype"])


def op_reshape(env, n):
    X = env[n["in"][0]]
    want = [int(v) for v in env[n["in"][1]]["vals"]]
    allowzero = int(n["attrs"].get("allowzero", 0))
    dims, infer, known = [], -1, 1
    for i, d in enumerate(want):
        if d == -1:
            infer = i
            dims.append(1)
        elif d == 0 and not allowzero:
            dims.append(X["dims"][i])
            known *= dims[-1]
        else:
            dims.append(d)
            known *= d
    if infer >= 0:
        dims[infer] = X["vals"].__len__() // known
    return T(dims, X["vals"], X["dtype"])


def op_unsqueeze(env, n):
    X = env[n["in"][0]]
    nd = len(X["dims"])
    if "axes" in n["attrs"]:
        ax = list(n["attrs"]["axes"])
    else:
        ax = [int(v) for v in env[n["in"][1]]["vals"]]
    ax = sorted([a + nd + len(ax) if a < 0 else a for a in ax])
    od, si = [], 0
    for i in range(nd + len(ax)):
        if i in ax:
            od.append(1)
        else:
            od.append(X["dims"][si])
            si += 1
    return T(od, X["vals"], X["dtype"])


def op_squeeze(env, n):
    X = env[n["in"][0]]
    nd = len(X["dims"])
    if "axes" in n["attrs"]:
        ax = list(n["attrs"]["axes"])
    elif len(n["in"]) > 1 and n["in"][1]:
        ax = [int(v) for v in env[n["in"][1]]["vals"]]
    else:
        ax = [i for i, d in enumerate(X["dims"]) if d == 1]
    ax = [(a + nd) if a < 0 else a for a in ax]
    od = [d for i, d in enumerate(X["dims"]) if i not in ax]
    return T(od, X["vals"], X["dtype"])


def op_concat(env, n):
    ts = [env[i] for i in n["in"]]
    nd = len(ts[0]["dims"])
    ax = int(n["attrs"]["axis"])
    if ax < 0:
        ax += nd
    od = list(ts[0]["dims"])
    od[ax] = sum(t["dims"][ax] for t in ts)
    out = [0.0] * prod(od)
    ost, shift = st_of(od), 0
    for t in ts:
        st = st_of(t["dims"])
        for idx in coords(t["dims"]):
            oidx = list(idx)
            oidx[ax] += shift
            out[sum(oidx[i] * ost[i] for i in range(nd))] = \
                t["vals"][sum(idx[i] * st[i] for i in range(nd))]
        shift += t["dims"][ax]
    return T(od, out, ts[0]["dtype"])


def op_gather(env, n):
    X = env[n["in"][0]]
    I = env[n["in"][1]]
    nd, ir = len(X["dims"]), len(I["dims"])
    ax = int(n["attrs"].get("axis", 0))
    if ax < 0:
        ax += nd
    od = X["dims"][:ax] + I["dims"] + X["dims"][ax + 1:]
    outer = prod(X["dims"][:ax])
    m = X["dims"][ax]
    inner = prod(X["dims"][ax + 1:])
    out = [0.0] * prod(od)
    st = st_of(od)
    for k, sel in enumerate(I["vals"]):
        sel = int(sel)
        if sel < 0:
            sel += m
        for o in range(outer):
            for i in range(inner):
                dst_idx = []
                # 输出坐标 = 外层 + 索引形状 + 内层
                rem = o * prod(I["dims"]) * inner + k * inner + i
                out[rem] = X["vals"][(o * m + sel) * inner + i]
    return T(od, out, X["dtype"])


def op_slice(env, n):
    X = env[n["in"][0]]
    nd = len(X["dims"])
    starts = [int(v) for v in env[n["in"][1]]["vals"]]
    ends = [int(v) for v in env[n["in"][2]]["vals"]]
    axes = [int(v) for v in env[n["in"][3]]["vals"]] if len(n["in"]) > 3 and n["in"][3] else list(range(len(starts)))
    steps = [int(v) for v in env[n["in"][4]]["vals"]] if len(n["in"]) > 4 and n["in"][4] else [1] * len(starts)
    rng = [[0, X["dims"][i], 1] for i in range(nd)]
    for k, a in enumerate(axes):
        a = a + nd if a < 0 else a
        dim = X["dims"][a]
        s, e, p = starts[k], ends[k], steps[k]
        if p > 0:
            if s < 0:
                s += dim
            if e < 0:
                e += dim
            s = min(max(s, 0), dim)
            e = min(max(e, 0), dim)
            rng[a] = [s, e, p]
        else:
            if s < 0:
                s += dim
            if e != -(2 ** 63):
                if e < 0:
                    e += dim
            s = min(s, dim - 1)
            e = max(e, -1)
            rng[a] = [s, e, p]
    real = []
    for a in range(nd):
        s, e, p = rng[a]
        real.append(list(range(s, e, p)) if p > 0 else list(range(s, e, p)))
    od = [len(r) for r in real]
    out = []
    for idx in coords(od):
        src = [real[i][idx[i]] for i in range(nd)]
        out.append(X["vals"][sum(src[i] * st_of(X["dims"])[i] for i in range(nd))])
    return T(od, out, X["dtype"])


def op_expand(env, n):
    X = env[n["in"][0]]
    want = [int(v) for v in env[n["in"][1]]["vals"]]
    nd = len(want)
    od = []
    for i in range(nd):
        ti = len(X["dims"]) - nd + i
        d = X["dims"][ti] if ti >= 0 else 1
        od.append(d if want[i] == 1 else want[i])
    vals = [get_b(X, idx, od) for idx in coords(od)]
    return T(od, vals, X["dtype"])


def op_tile(env, n):
    X = env[n["in"][0]]
    rep = [int(v) for v in env[n["in"][1]]["vals"]]
    od = [X["dims"][i] * rep[i] for i in range(len(X["dims"]))]
    out = []
    for idx in coords(od):
        src = [idx[i] % X["dims"][i] for i in range(len(od))]
        out.append(X["vals"][sum(src[i] * st_of(X["dims"])[i] for i in range(len(od)))])
    return T(od, out, X["dtype"])


def op_split(env, n, nout):
    X = env[n["in"][0]]
    ax = int(n["attrs"].get("axis", 0))
    if ax < 0:
        ax += len(X["dims"])
    if len(n["in"]) > 1 and n["in"][1]:
        sizes = [int(v) for v in env[n["in"][1]]["vals"]]
    elif "split" in n["attrs"]:
        sizes = list(n["attrs"]["split"])
    else:
        sizes = [X["dims"][ax] // nout] * nout
    outer = prod(X["dims"][:ax])
    inner = prod(X["dims"][ax + 1:])
    outs, off = [], 0
    for s in sizes:
        od = list(X["dims"])
        od[ax] = s
        vals = []
        for o in range(outer):
            for i in range(s):
                for j in range(inner):
                    vals.append(X["vals"][(o * X["dims"][ax] + off + i) * inner + j])
        outs.append(T(od, vals, X["dtype"]))
        off += s
    return outs


def op_cast(env, n):
    X = env[n["in"][0]]
    to = int(n["attrs"]["to"])
    if to in (FLOAT,):
        vals = [float(v) for v in X["vals"]]
    elif to == BOOL:
        vals = [1 if v != 0 else 0 for v in X["vals"]]
    else:
        vals = [int(v) for v in X["vals"]]
    return T(X["dims"], vals, to)


def op_where(env, n):
    C, X, Y = env[n["in"][0]], env[n["in"][1]], env[n["in"][2]]
    dims = bcast_all([C["dims"], X["dims"], Y["dims"]])
    vals = []
    for idx in coords(dims):
        c = get_b(C, idx, dims)
        vals.append(get_b(X, idx, dims) if c != 0 else get_b(Y, idx, dims))
    return T(dims, vals, X["dtype"])


def op_constofshape(env, n):
    sh = [int(v) for v in env[n["in"][0]]["vals"]]
    spec = n["attrs"]["value"]
    v = spec["vals"][0]
    dt = spec.get("dtype", FLOAT)
    return T(sh, [v] * prod(sh), dt)


def op_range(env, n):
    start = env[n["in"][0]]["vals"][0]
    limit = env[n["in"][1]]["vals"][0]
    delta = env[n["in"][2]]["vals"][0]
    cnt = int(math.ceil((limit - start) / delta))
    vals = [start + i * delta for i in range(max(cnt, 0))]
    return T([len(vals)], vals, env[n["in"][0]]["dtype"])


def op_triu(env, n):
    X = env[n["in"][0]]
    k = int(n["attrs"].get("k", 0))
    rows, cols = X["dims"][-2], X["dims"][-1]
    batch = prod(X["dims"][:-2])
    out = list(X["vals"])
    for b in range(batch):
        for i in range(rows):
            for j in range(cols):
                if j < i + k:
                    out[b * rows * cols + i * cols + j] = 0.0
    return T(X["dims"], out, X["dtype"])


def unary(fn):
    return lambda env, n: ew([env[n["in"][0]]], lambda x: fn(x), env[n["in"][0]]["dtype"])


def binop(fn, dtype=None):
    def f(env, n):
        ts = [env[i] for i in n["in"]]
        return ew(ts, fn, dtype or ts[0]["dtype"])
    return f


def cmp_op(fn):
    def f(env, n):
        ts = [env[i] for i in n["in"]]
        return ew(ts, lambda a, b: 1.0 if fn(a, b) else 0.0, BOOL)
    return f


def rounded(x):
    f = math.floor(x)
    d = x - f
    if d > 0.5:
        return f + 1
    if d < 0.5:
        return f
    return f if f % 2 == 0 else f + 1


def leaky(env, n):
    al = float(n["attrs"].get("alpha", 0.01))
    return ew([env[n["in"][0]]], lambda x: x if x >= 0 else al * x, FLOAT)


def elu(env, n):
    al = float(n["attrs"].get("alpha", 1.0))
    return ew([env[n["in"][0]]], lambda x: x if x >= 0 else al * (math.exp(x) - 1), FLOAT)


def clip(env, n):
    X = env[n["in"][0]]
    lo = env[n["in"][1]]["vals"][0] if len(n["in"]) > 1 and n["in"][1] else \
        n["attrs"].get("min", -math.inf)
    hi = env[n["in"][2]]["vals"][0] if len(n["in"]) > 2 and n["in"][2] else \
        n["attrs"].get("max", math.inf)
    if "min" in n["attrs"] and (len(n["in"]) < 2 or not n["in"][1]):
        lo = n["attrs"]["min"]
    if "max" in n["attrs"] and (len(n["in"]) < 3 or not n["in"][2]):
        hi = n["attrs"]["max"]
    return ew([X], lambda x: min(max(x, lo), hi), X["dtype"])


def div_op(env, n):
    ts = [env[i] for i in n["in"]]
    if ts[0]["dtype"] == FLOAT:
        return ew(ts, lambda a, b: a / b, FLOAT)
    return ew(ts, lambda a, b: int(a / b), ts[0]["dtype"])


def mod_op(env, n):
    ts = [env[i] for i in n["in"]]
    if ts[0]["dtype"] == FLOAT:
        return ew(ts, lambda a, b: math.fmod(a, b), FLOAT)
    return ew(ts, lambda a, b: a - b * int(a / b), ts[0]["dtype"])


OPS = {
    "Identity": lambda env, n: [clone(env[n["in"][0]])],
    "Reshape": lambda env, n: [op_reshape(env, n)],
    "Transpose": lambda env, n: [op_transpose(env, n)],
    "Concat": lambda env, n: [op_concat(env, n)],
    "Shape": lambda env, n: [T([len(env[n["in"][0]]["dims"])],
                               [float(d) for d in env[n["in"][0]]["dims"]], INT64)],
    "Cast": lambda env, n: [op_cast(env, n)],
    "Unsqueeze": lambda env, n: [op_unsqueeze(env, n)],
    "Squeeze": lambda env, n: [op_squeeze(env, n)],
    "Flatten": lambda env, n: [T([prod(env[n["in"][0]]["dims"][:int(n["attrs"].get("axis", 1))]),
                                  prod(env[n["in"][0]]["dims"][int(n["attrs"].get("axis", 1)):])],
                                 env[n["in"][0]]["vals"], env[n["in"][0]]["dtype"])],
    "Gather": lambda env, n: [op_gather(env, n)],
    "Slice": lambda env, n: [op_slice(env, n)],
    "Expand": lambda env, n: [op_expand(env, n)],
    "Tile": lambda env, n: [op_tile(env, n)],
    "Split": lambda env, n: op_split(env, n, len(n["out"])),
    "Add": binop(lambda a, b: a + b),
    "Sub": binop(lambda a, b: a - b),
    "Mul": binop(lambda a, b: a * b),
    "Div": div_op,
    "Pow": binop(lambda a, b: a ** b),
    "Mod": mod_op,
    "Max": binop(lambda a, b: max(a, b)),
    "Min": binop(lambda a, b: min(a, b)),
    "Sum": binop(lambda a, b: a + b),
    "Mean": lambda env, n: ew([env[i] for i in n["in"]],
                              lambda *vs: sum(vs) / len(vs), FLOAT),
    "Equal": cmp_op(lambda a, b: a == b),
    "Less": cmp_op(lambda a, b: a < b),
    "Greater": cmp_op(lambda a, b: a > b),
    "LessOrEqual": cmp_op(lambda a, b: a <= b),
    "GreaterOrEqual": cmp_op(lambda a, b: a >= b),
    "And": cmp_op(lambda a, b: (a != 0) and (b != 0)),
    "Or": cmp_op(lambda a, b: (a != 0) or (b != 0)),
    "Xor": cmp_op(lambda a, b: (a != 0) != (b != 0)),
    "Not": lambda env, n: T(env[n["in"][0]]["dims"],
                            [0.0 if v != 0 else 1.0 for v in env[n["in"][0]]["vals"]], BOOL),
    "Sqrt": unary(math.sqrt),
    "Erf": unary(math.erf),
    "Exp": unary(math.exp),
    "Log": unary(math.log),
    "Abs": unary(abs),
    "Neg": unary(lambda x: -x),
    "Ceil": unary(lambda x: float(math.ceil(x))),
    "Floor": unary(lambda x: float(math.floor(x))),
    "Round": unary(rounded),
    "Reciprocal": unary(lambda x: 1.0 / x),
    "Sign": unary(lambda x: (x > 0) - (x < 0)),
    "Tanh": unary(math.tanh),
    "Sigmoid": unary(lambda x: 1.0 / (1.0 + math.exp(-x))),
    "Relu": unary(lambda x: max(x, 0.0)),
    "LeakyRelu": leaky,
    "Elu": elu,
    "Softplus": unary(lambda x: math.log1p(math.exp(-abs(x))) + (x if x > 0 else 0.0)),
    "Clip": clip,
    "MatMul": lambda env, n: [op_matmul(env, n)],
    "Softmax": lambda env, n: [op_softmax(env, n)],
    "ReduceMean": lambda env, n: [op_reduce(env, n, "mean")],
    "ReduceSum": lambda env, n: [op_reduce(env, n, "sum")],
    "ReduceMax": lambda env, n: [op_reduce(env, n, "max")],
    "ReduceMin": lambda env, n: [op_reduce(env, n, "min")],
    "ReduceProd": lambda env, n: [op_reduce(env, n, "prod")],
    "ArgMax": lambda env, n: [op_argmax(env, n)],
    "LayerNormalization": lambda env, n: [op_layernorm(env, n)],
    "Where": lambda env, n: [op_where(env, n)],
    "ConstantOfShape": lambda env, n: [op_constofshape(env, n)],
    "Constant": lambda env, n: [T(n["attrs"]["value"]["dims"], n["attrs"]["value"]["vals"],
                                  n["attrs"]["value"].get("dtype", FLOAT))],
    "Range": lambda env, n: [op_range(env, n)],
    "Triu": lambda env, n: [op_triu(env, n)],
}


# ══════════════════ 执行（拓扑序：输入齐了就执行） ══════════════════
def run_graph(g, feeds):
    env = {}
    for it in g["initializers"]:
        env[it["name"]] = T(it["dims"], [float(v) if it["dtype"] == FLOAT else int(v)
                                         for v in it["vals"]], it["dtype"])
    for k, spec in feeds.items():
        fi = [x for x in g["inputs"] if x["name"] == k][0]
        env[k] = T(spec["dims"], spec["f32"] if "f32" in spec else spec["i64"],
                   fi["dtype"]) if False else T(spec["dims"],
                                                spec.get("f32", spec.get("i64")), fi["dtype"])
    todo = list(g["nodes"])
    ops_used = {}
    while todo:
        again = []
        progress = False
        for nd in todo:
            if all((not i) or (i in env) for i in nd["in"]):
                fn = OPS[nd["op"]]
                try:
                    res = fn(env, nd)
                except Exception as ex:
                    raise RuntimeError("%s（节点 %s · 输入形状 %s）: %s" % (
                        nd["op"], nd.get("name"),
                        [env[i]["dims"] if i in env else None for i in nd["in"]], ex))
                if not isinstance(res, list):
                    res = [res]
                for nm, t in zip(nd["out"], res):
                    env[nm] = t
                ops_used[nd["op"]] = ops_used.get(nd["op"], 0) + 1
                progress = True
            else:
                again.append(nd)
        todo = again
        if todo and not progress:
            raise RuntimeError("图无法继续：%s" % [n["op"] for n in todo])
    return env, ops_used


# ══════════════════ 比对 ══════════════════
def decode_hex(hexs, dtype):
    b = bytes.fromhex(hexs)
    n = len(b) // (4 if dtype in (FLOAT,) else 8 if dtype in (INT64,) else 1)
    if dtype == FLOAT:
        return list(struct.unpack("<%df" % n, b)), n
    if dtype == INT64:
        return list(struct.unpack("<%dq" % n, b)), n
    return [1 if x else 0 for x in b], n


def cmp_vals(got, want, dtype, rtol=1e-5, atol=1e-6):
    if len(got) != len(want):
        return "长度 %d ≠ %d" % (len(got), len(want))
    worst, wi = 0.0, -1
    for i, (a, b) in enumerate(zip(got, want)):
        if dtype in (INT64, BOOL, INT32):
            if int(a) != int(b):
                return "下标 %d：%r ≠ %r（整型必须逐位相等）" % (i, a, b)
            continue
        d = abs(a - b)
        tol = atol + rtol * abs(b)
        if d > tol:
            return "下标 %d：%r vs %r（差 %.3g > 容差 %.3g）" % (i, a, b, d, tol)
        if abs(b) > 0:
            worst = max(worst, d / abs(b))
            if d / abs(b) == worst:
                wi = i
    return None


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.join(here, "..")
    # 环境变量逃生舱：真实模型对拍时换用别的图 JSON / feeds 目录（默认走手造语料）
    gpath = os.environ.get("M157_GRAPHS", os.path.join(root, "models", "graphs.json"))
    fdir = os.environ.get("M157_FEEDS_DIR", os.path.join(root, "models", "feeds"))
    epath = os.environ.get("M157_EXPECTED", os.path.join(root, "models", "expected.json"))
    graphs = json.load(open(gpath))
    hand = json.load(open(epath)) if os.path.exists(epath) else {}
    cdir = sys.argv[1] if len(sys.argv) > 1 else None
    only = sys.argv[2] if len(sys.argv) > 2 else None
    # M157_ONLY=a,b：只跑指定模型（门做"单点负控"时用 —— 否则"缺 dump"本身就会让它失败，
    #   那样证明的是"缺文件会红"，而不是"这条判据能抓住这个篡改"）
    if os.environ.get("M157_ONLY"):
        only = None
        import re as _re
        allowed = {x for x in _re.split(r"[,\s]+", os.environ["M157_ONLY"]) if x}
    else:
        allowed = None
    fail = 0
    all_ops_used = {}
    ran = 0
    for name in sorted(graphs):
        if only and only != name:
            continue
        if allowed is not None and name not in allowed:
            continue
        g = graphs[name]
        feeds = json.load(open(os.path.join(fdir, name + ".json")))
        try:
            env, used = run_graph(g, feeds)
        except Exception as ex:
            print("  ❌ %s：参考执行失败：%s" % (name, ex))
            fail += 1
            continue
        for k, v in used.items():
            all_ops_used[k] = all_ops_used.get(k, 0) + v
        nref = len(used)
        fail0 = fail
        # ② vs ①
        ok_hand = 0
        if name in hand:
            for out, want in hand[name].items():
                t = env[out]
                dt = t["dtype"]
                # 稀疏手算：{"at":[下标…], "vals":[…]} ⇒ 只断言指定下标（其余交给参考）
                if isinstance(want, dict):
                    idxs, wv = want["at"], want["vals"]
                    gv = [t["vals"][i] for i in idxs]
                else:
                    idxs, wv, gv = None, want, t["vals"]
                msg = cmp_vals([float(x) for x in gv] if dt == FLOAT else gv,
                               [float(x) for x in wv] if dt == FLOAT else wv, dt)
                if msg and dt == FLOAT and out in ("F", "RC", "LG"):
                    msg = cmp_vals(gv, wv, dt, rtol=1e-6, atol=1e-7)
                if msg:
                    print("  ❌ %s.%s：参考 vs 手算不一致：%s" % (name, out, msg))
                    fail += 1
                else:
                    ok_hand += 1
        tag = "（手算 %d 项）" % ok_hand if ok_hand else ""
        # ③ vs ②
        nout = 0
        if cdir:
            p = os.path.join(cdir, name + ".json")
            if not os.path.exists(p):
                print("  ❌ %s：缺 C 侧 dump %s" % (name, p))
                fail += 1
                continue
            d = json.load(open(p))
            for out, t in env.items():
                if out not in d["outputs"]:
                    continue
                c = d["outputs"][out]
                nout += 1
                got, cnt = decode_hex(c["hex"], c["dtype"] if isinstance(c["dtype"], int)
                                      else {"float": FLOAT, "int64": INT64, "bool": BOOL,
                                            "int32": INT32}[c["dtype"]])
                dt = t["dtype"] if isinstance(t["dtype"], int) else t["dtype"]
                if list(c["dims"]) != list(t["dims"]):
                    print("  ❌ %s.%s：形状 %s ≠ 参考 %s" % (name, out, c["dims"], t["dims"]))
                    fail += 1
                    continue
                msg = cmp_vals(got, t["vals"], dt)
                if msg:
                    print("  ❌ %s.%s：C vs 参考不一致：%s" % (name, out, msg))
                    fail += 1
        ran += 1
        mfail = fail - fail0
        print("  %s %-24s 参考节点 %d · 输出 %d · C↔参考 %d 项 %s"
              % ("✅" if mfail == 0 else "❌", name, nref, len(env), nout, tag))
    if allowed is not None and ran == 0:
        # 缺陷 147 同族：**取不到判据** ≠ 判据满足。M157_ONLY 过滤后一个模型都没跑，
        #   绝不能返回 0（否则门的负控会"仍然全绿"——本轮实测踩过：名字用空格分隔，
        #   而这里按逗号切 ⇒ allowed 成了 {"a b"} ⇒ 一个都不匹配 ⇒ 假绿）。
        print("❌ M157_ONLY=%r 没有匹配到任何模型（判据缺失 ⇒ 判红，不判绿）"
              % os.environ.get("M157_ONLY"))
        print("M157-REF-FAIL(1)")
        return 1
    print("M157-REF: 独立参考覆盖算子 %d 个" % len(all_ops_used))
    print("M157-REF-%s" % ("OK" if fail == 0 else "FAIL(%d)" % fail))
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
