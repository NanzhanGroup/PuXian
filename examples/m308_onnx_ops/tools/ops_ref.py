#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""ops_ref.py —— M308 门的**第三条腿**：对 C 侧 dump 做独立复算（纯 Python，无第三方库）。

为什么要有它（M308-PLAN §三.S3「独立第三份实现」）：
  门里已经有两条腿 ——
    ① 手算真值（`models/expected.json`，M157 的 `m13_s4_ops()` 里逐条写死）；
    ② 独立参考执行器（`../m157_onnx_exec/tools/exec_ref.py`，从 ONNX 定义从零实现）；
    ③ 被测实现（`runtime/onnx_ops.c`，语言侧走 `onnx_run`）。
  ② 是**跨门共用**的：m157 与 m308 都在用它。共用一件工具本身不是坏事（同一份定义、
  同一份语义 ⇒ 不会两眼各看一份），但它有一个隐患：**② 与 ③ 若同时错向同一边，
  门照样全绿**（M156 §38.4 的原话）。所以本文件刻意**不复用 ②**、也**不读 ①**：
  只拿「图描述（graphs.json）+ feed」，从头把 6 个新算子里**除 Gelu 外**的 5 条
  （Silu / Cos / Sin / RMSNormalization / ScatterND）重算一遍，再与 C 侧 dump 逐元素对。

  为什么把 Gelu 排除在外：Gelu 有一条**比数值对拍更硬**的判据 —— 「两档必须可判异」，
  它由 `tools/gelu_tiers.py` 单独守（两档写成同一式时，三方会**一致地错**，
  只有「互异」这条能戳穿）。本文件不重复那条腿。

判据（每条都能单独变红）：
  ① 五个算子**逐个**都有 dump 输出可比（取不到判据 ≠ 判据满足 ⇒ 判红，不判绿）
  ② 每个输出逐元素 |C - 复算| ≤ 1e-6（fp32 dump 的舍入在 ~3e-7 量级）
  ③ **属性真的生效**：`e14_rms_axis_eps` 专测 —— epsilon=0.5（而非默认 1e-5）与
     axis=1（而非默认最后一维）。若 C 侧把属性读错/忽略，这里的偏差是 **O(1)** 级，
     不是浮点噪声 —— 这是「epsilon/axis 语义写死进 spec」那句的可执行版本。

用法：python3 tools/ops_ref.py <C 侧 dump 目录>
退出码：0 全绿 / 1 判红 / 2 用法或输入缺失
"""
import json
import math
import os
import struct
import sys

ATOL = 1e-6
HERE = os.path.dirname(os.path.abspath(__file__))
BASE = os.path.abspath(os.path.join(HERE, ".."))
MODELS = os.path.join(BASE, "models")
FEEDS = os.path.join(MODELS, "feeds")
GRAPHS = os.path.join(MODELS, "graphs.json")

# 本文件**负责**的算子（Gelu 由 gelu_tiers.py 守，见文件头）
COVERED = ("Silu", "Cos", "Sin", "RMSNormalization", "ScatterND")


def f32_of(hexs):
    """C 侧 dump 是 fp32 的十六进制串 —— 按**实际位型**解回 float（不猜 dtype）。"""
    return list(struct.unpack("<%df" % (len(hexs) // 8), bytes.fromhex(hexs)))


def prod(dims):
    n = 1
    for d in dims:
        n *= d
    return n


def strides_of(dims):
    st = [1] * len(dims)
    for i in range(len(dims) - 2, -1, -1):
        st[i] = st[i + 1] * dims[i + 1]
    return st


# ══════════════════ 独立复算（每条按 ONNX 定义从零写，与 C 侧无共享代码） ══════════════════

def r_silu(x):
    # x·σ(x)。用 |x| 写，避免大负 x 上 exp(-x) 溢出（与 C 侧同样的数值考量，独立写）
    return x / (1.0 + math.exp(-x)) if x >= 0 else x * math.exp(x) / (1.0 + math.exp(x))


def r_rmsnorm(x, dims, axis, eps, scale):
    """y = x / sqrt(mean_tail(x²)+eps) · scale（axis 起的**尾部维**归一化，opset23 口径）"""
    nd = len(dims)
    ax = axis + nd if axis < 0 else axis
    outer, inner = prod(dims[:ax]), prod(dims[ax:])
    out = [0.0] * len(x)
    for o in range(outer):
        seg = x[o * inner:(o + 1) * inner]
        inv = 1.0 / math.sqrt(sum(v * v for v in seg) / inner + eps)
        for i, v in enumerate(seg):
            w = v * inv
            if scale:
                w *= scale[0] if len(scale) == 1 else scale[i]
            out[o * inner + i] = w
    return out


def r_scatternd(data, ddims, idx, idims, upd):
    """out = data；out[indices[u]] = updates[u]（indices 末维 k 个坐标解释前 k 维）"""
    k = idims[-1]
    nupd, sl = prod(idims[:-1]), prod(ddims[k:])
    st = strides_of(ddims)
    out = list(data)
    for u in range(nupd):
        base = 0
        for j in range(k):
            v = int(idx[u * k + j])
            if not (0 <= v < ddims[j]):
                raise ValueError("复算遇到越界下标（%d 维取 %d）—— 该负控本该在 C 侧报错" % (j, v))
            base += v * st[j]
        for s in range(sl):
            out[base + s] = upd[u * sl + s]
    return out


def evaluate(node, env):
    """返回 (输出名, 复算值列表) —— 只认 COVERED 里的算子"""
    op = node["op"]
    dims = lambda t: t["dims"]
    vals = lambda t: t["vals"]
    if op == "Silu":
        return node["out"][0], [r_silu(x) for x in vals(env[node["in"][0]])]
    if op == "Cos":
        return node["out"][0], [math.cos(x) for x in vals(env[node["in"][0]])]
    if op == "Sin":
        return node["out"][0], [math.sin(x) for x in vals(env[node["in"][0]])]
    if op == "RMSNormalization":
        X = env[node["in"][0]]
        sc = env[node["in"][1]]["vals"] if len(node["in"]) > 1 and node["in"][1] else None
        a = node["attrs"]
        return node["out"][0], r_rmsnorm(vals(X), dims(X), int(a.get("axis", -1)),
                                         float(a.get("epsilon", 1e-5)), sc)
    if op == "ScatterND":
        d, i, u = (env[nm] for nm in node["in"])
        return node["out"][0], r_scatternd(vals(d), dims(d), vals(i), dims(i), vals(u))
    raise AssertionError(op)


def main():
    if len(sys.argv) < 2:
        print("用法: python3 tools/ops_ref.py <C 侧 dump 目录>")
        return 2
    dumps = sys.argv[1]
    if not os.path.isdir(dumps):
        print("❌ dump 目录不存在：%s" % dumps)
        return 2
    graphs = json.load(open(GRAPHS))
    fail, judged, worst = 0, {}, 0.0
    for name in sorted(graphs):
        g = graphs[name]
        dp = os.path.join(dumps, name + ".json")
        if not os.path.exists(dp):
            print("  ❌ %s：缺 C 侧 dump（**取不到判据 ≠ 判据满足**）" % name)
            fail += 1
            continue
        dump = json.load(open(dp))
        env = {i["name"]: {"dims": i["dims"], "vals": i["vals"]} for i in g["initializers"]}
        fp = os.path.join(FEEDS, name + ".json")
        for nm, f in json.load(open(fp)).items():
            key = "f32" if "f32" in f else "i64"
            env[nm] = {"dims": f["dims"], "vals": f[key]}
        for node in g["nodes"]:
            if node["op"] not in COVERED:
                continue
            oname, want = evaluate(node, env)
            got = dump["outputs"].get(oname)
            if got is None:
                print("  ❌ %s.%s（%s）：dump 里没有这个输出" % (name, oname, node["op"]))
                fail += 1
                continue
            have = f32_of(got["hex"])
            if len(have) != len(want):
                print("  ❌ %s.%s：元素数 %d ≠ 复算 %d" % (name, oname, len(have), len(want)))
                fail += 1
                continue
            e = max(abs(a - b) for a, b in zip(have, want)) if want else 0.0
            worst = max(worst, e)
            judged[node["op"]] = judged.get(node["op"], 0) + 1
            tag = "✅" if e <= ATOL else "❌"
            print("  %s 复算 %-18s %-8s %d 点最大绝对误差 %.3g" % (tag, name, node["op"], len(want), e))
            if e > ATOL:
                fail += 1
    # 覆盖率：五个算子**逐个**都得真的复算过（否则这条腿在某个算子上是空转）
    miss = [op for op in COVERED if op not in judged]
    if miss:
        print("  ❌ 复算覆盖不全：%s 一次都没复算过（判据缺失 ⇒ 判红）" % ",".join(miss))
        fail += 1
    else:
        print("  ✅ 复算覆盖：%s 逐个都在语料里复算过（共 %d 项）"
              % ("/".join(COVERED), sum(judged.values())))
    print("M308-OPSREF: 复算最大绝对误差 %.3g（阈值 %.1g）" % (worst, ATOL))
    print("M308-OPSREF-%s" % ("OK" if fail == 0 else "FAIL(%d)" % fail))
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
