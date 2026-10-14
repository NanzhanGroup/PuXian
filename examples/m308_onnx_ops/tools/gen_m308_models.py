#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""gen_m308_models.py —— M308 门（ONNX 推理算子补齐 · S4）的语料生成器。

产物（都在本门 models/ 下，幂等可重跑）：
  1. models/e13_s4_ops.onnx   —— **正例**：Gelu（erf/tanh 两档）· Silu · Cos / Sin ·
     RMSNormalization（带 scale / 不带 scale）· ScatterND（k=2 坐标）。
     定义**不在此处重写** —— 直接 `import` M157 的 `m13_s4_ops()`：
     同一份「描述」同时喂 m157 门与本门 ⇒ 两个门看到的是**同一个模型**，
     不会出现「两份语料各自漂移」（M306 的「舍入不许有两份」同一条纪律）。
     m157 侧落库该模型是 §二.0-A 硬约束（m157 门断言「注册表 == 语料执行集合」）。
  2. models/graphs.json / models/feeds/ / models/expected.json / expected_notes.json
     —— 供独立参考 `examples/m157_onnx_exec/tools/exec_ref.py` 三方对拍（手算真值随模型定义走）。
  3. models/neg/*.onnx + models/neg/cases.json —— **行为负控** 7 个：
       ScatterND：下标越界 · updates 元素数不符 · indices 末维 > data 秩 · data/updates 类型不符
       Gelu：approximate 非法值
       RMSNormalization：axis 越界 · scale 元素数不符
     判据：**每一个都必须响亮报错**，且报错必须来自**它自己那个算子**
     （cases.json 里逐条登记 `op`，门第 ⑤ 层据此断言 —— 防「报错是别的节点崩的」。

为什么要「正例 import、负控自建」：
  · 正例若复制一份 ⇒ 改了算子语义要改两处，早晚漂移（这正是 §二.0-A 想避免的形状）。
  · 负控是**本门专有**的判据（m157 的负控清单一律 5 个、只测执行器行为，不宜混入），
    所以这里单独建，落在本门自己的 models/ 下，互不干扰。
"""
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.abspath(os.path.join(HERE, ".."))
MODELS = os.path.join(OUT, "models")
FEEDS = os.path.join(MODELS, "feeds")
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "examples", "m157_onnx_exec", "tools"))
import gen_exec_models as G  # noqa: E402  （只借用 protobuf 编码器与 M 描述层）

FLOAT, INT64 = G.FLOAT, G.INT64


def negs():
    """行为负控：每个都必须**在运行时响亮报错**（门第 ⑤ 层逐条断言）。

    返回 `(模型, 人读理由, 期望报错的算子名)` —— 算子名进 cases.json：
    门据此断言「这条负控的报错来自**它自己那个算子**」，而不是图里别的节点崩了。
    """
    out = []

    # n01 ScatterND：下标越界（data 第一维长 3，取 5）
    m = G.M("n01_scatternd_oob", opset=20)
    m.init("DATA", [3, 4], [float(i) for i in range(12)])
    m.init("IDX", [1, 1], [5], INT64)
    m.init("UPD", [4], [1.0, 1.0, 1.0, 1.0])
    m.op("ScatterND", ["DATA", "IDX", "UPD"], ["Y"])
    m.out("Y", [3, 4])
    out.append((m, "下标越界", "ScatterND"))

    # n02 ScatterND：updates 元素数不符（indices.shape[:-1]=2 × data.shape[1:]=4 ⇒ 要 8，给 3）
    m = G.M("n02_scatternd_updates_len", opset=20)
    m.init("DATA", [3, 4], [float(i) for i in range(12)])
    m.init("IDX", [2, 1], [0, 2], INT64)
    m.init("UPD", [3], [1.0, 2.0, 3.0])
    m.op("ScatterND", ["DATA", "IDX", "UPD"], ["Y"])
    m.out("Y", [3, 4])
    out.append((m, "updates 元素数不符", "ScatterND"))

    # n03 ScatterND：indices 末维 k=3 > data 秩 2
    m = G.M("n03_scatternd_k_gt_rank", opset=20)
    m.init("DATA", [3, 4], [float(i) for i in range(12)])
    m.init("IDX", [1, 3], [0, 1, 2], INT64)
    m.init("UPD", [1], [7.0])
    m.op("ScatterND", ["DATA", "IDX", "UPD"], ["Y"])
    m.out("Y", [3, 4])
    out.append((m, "indices 末维 > data 秩", "ScatterND"))

    # n04 ScatterND：data(f32) 与 updates(i64) 类型不符
    m = G.M("n04_scatternd_dtype", opset=20)
    m.init("DATA", [3, 4], [float(i) for i in range(12)])
    m.init("IDX", [1, 1], [0], INT64)
    m.init("UPD", [4], [7, 7, 7, 7], INT64)
    m.op("ScatterND", ["DATA", "IDX", "UPD"], ["Y"])
    m.out("Y", [3, 4])
    out.append((m, "data/updates 类型不符", "ScatterND"))

    # n05 Gelu：approximate 非法值（只认 none/tanh）
    m = G.M("n05_gelu_bad_approx", opset=20)
    m.inp("X", [1, 3], [0.5, 1.5, -2.5])
    m.op("Gelu", ["X"], ["Y"], [G.attr("approximate", "s", "relu")])
    m.out("Y", [1, 3])
    out.append((m, "Gelu approximate 非法", "Gelu"))

    # n06 RMSNormalization：axis 越界（秩 2，axis=5）
    m = G.M("n06_rms_axis_oob", opset=23)
    m.inp("X", [1, 4], [1.0, 2.0, 3.0, 4.0])
    m.op("RMSNormalization", ["X"], ["Y"], [G.attr("axis", "i", 5)])
    m.out("Y", [1, 4])
    out.append((m, "RMSNormalization axis 越界", "RMSNormalization"))

    # n07 RMSNormalization：scale 元素数既不是 1 也不是归一化长度（4）
    m = G.M("n07_rms_scale_len", opset=23)
    m.inp("X", [1, 4], [1.0, 2.0, 3.0, 4.0])
    m.init("SC", [3], [1.0, 1.0, 1.0])
    m.op("RMSNormalization", ["X", "SC"], ["Y"])
    m.out("Y", [1, 4])
    out.append((m, "scale 元素数不符", "RMSNormalization"))

    return out


def judgment():
    """M308 门**专有**的判据模型（不进 m157 语料 —— 它测的是「属性真的生效」，
    不是「算子能跑」；m157 门只需覆盖新算子即可，本门的正判据要更细一层）。

    `e14_rms_axis_eps`：RMSNormalization 的 **epsilon / axis / scale 语义**专测 ——
      · R_EPS：`epsilon=0.5`（属性显式给值，**不是**默认 1e-5）
      · R_AX1：`axis=1`（秩 3 ⇒ 归一化 dims[1:] 共 12 个元素，**不是**最后一维）
      · R_SC ：`scale=[1,2,3,4]`（默认 axis=-1 ⇒ 尾部 4 个）
    **为什么非它不可**：`tools/ops_ref.py` 按**属性值**复算 —— 属性被忽略/读错时偏差是
    O(1e-3~1e-1) 级（eps: 1e-5 vs 0.5；axis: 尾部 4 个 vs 12 个），**远大于 fp32 噪声**
    ⇒ 这条判据能戳穿「属性不生效」这种最常见的历史缺陷（M306 同族纪律）。
    """
    X = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
         -1.0, -2.0, 3.0, 0.5, 0.0, 0.25, -0.5, 0.75,
         2.0, -3.0, 4.0, -5.0, 0.1, 0.2, 0.3, 0.4]
    m = G.M("e14_rms_axis_eps", opset=23)
    m.inp("X", [2, 3, 4], X)
    m.init("SC", [4], [1.0, 2.0, 3.0, 4.0])
    m.op("RMSNormalization", ["X"], ["R_EPS"], [G.attr("epsilon", "f", 0.5)])
    m.op("RMSNormalization", ["X"], ["R_AX1"], [G.attr("axis", "i", 1)])
    m.op("RMSNormalization", ["X", "SC"], ["R_SC"])
    for nm in ("R_EPS", "R_AX1", "R_SC"):
        m.out(nm, [2, 3, 4])

    # e15_trilu_alias：**规范名 Trilu** 必须真的能跑（与历史名 Triu 同一个实现）。
    #   为什么非它不可（M308-PLAN §二.4 命名缺口）：ONNX 规范名是 Trilu，而本仓历史注册名
    #   是 Triu，而算子查找走**精确 strcmp** ⇒ 只注册 Triu 时，**真实模型**里的 Trilu
    #   节点会「未实现的算子」。本模型**只用规范名**，把「别名被摘掉」变成一条**运行时**
    #   判据（源码负控 NC-C：删掉注册行 ⇒ 本模型必须跑不起来 ⇒ 本门判红）。
    alias = G.M("e15_trilu_alias", opset=20)
    alias.inp("X", [3, 3], [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0])
    alias.op("Trilu", ["X"], ["T"], [G.attr("k", "i", -1)])
    alias.out("T", [3, 3])
    alias.exact("T", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 0.0, 8.0, 9.0],
                "上三角 k=-1（**规范名 Trilu**，与 m157 语料 TU1 逐值一致）")

    return [m, alias]


def rope_kv():
    """attention 辅助的两条判据模型（M308-PLAN §二.S2：KV cache 布局 / RoPE）。

    `e16_kv_decode` —— **KV cache 增量解码等价性**：预分配 cache 上的「逐步 ScatterND 写入」
      必须与「一次性 Concat 全长」**逐值等价**（§二.S2 的硬判据）。
      ⚠️ 这条关系 **exec_ref 表达不了**（它比对的是「图 vs 手算真值」，不是「两个输出互等」）
      ⇒ 由 `tools/kv_equiv.py` 单独守（门的 ③d 层）；两条腿都在，不互相代偿。

    `e17_rope` —— **RoPE 复合式**：Cos/Sin（本里程碑 S1 新增）+ Gather（rotate_half 置换）
      + 广播乘加，串成一张真实的旋转位置编码图 —— 证明「补的算子真能组起来跑一个解码步」，
      而不只是各自孤零零地过一遍。真值手算（10 位），并交由 exec_ref 独立复算双保险。
    """
    # ── e16：KV cache 增量解码（O_STEP ⇄ O_ONCE 必须逐值等价）──
    kv = G.M("e16_kv_decode", opset=20)
    kv.init("CACHE", [3, 2], [1.0, 2.0, 3.0, 4.0, 0.0, 0.0])   # 预分配 T_max=3，前 2 槽有效
    kv.init("SLOT", [1, 1], [2], INT64)                        # 本次写入第 2 槽（0 基）
    kv.init("K_NEW", [1, 2], [5.0, 6.0])                       # 新 token 的 K
    kv.init("K_PAST", [2, 2], [1.0, 2.0, 3.0, 4.0])            # 已解码部分（全长路径用）
    kv.op("ScatterND", ["CACHE", "SLOT", "K_NEW"], ["O_STEP"])
    kv.op("Concat", ["K_PAST", "K_NEW"], ["O_ONCE"], [G.attr("axis", "i", 0)])
    kv.out("O_STEP", [3, 2])
    kv.out("O_ONCE", [3, 2])
    kv.exact("O_STEP", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0], "ScatterND 写第 2 槽（增量路径）")
    kv.exact("O_ONCE", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0],
             "Concat 全长（**必须与 O_STEP 逐值等价** —— ③d 层直接判这条）")

    # ── e17：RoPE 复合式（rotate_half：Gather 置换 + 前半取负）──
    rp = G.M("e17_rope", opset=20)
    rp.inp("X", [2, 4], [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0])
    rp.init("POS", [2, 1], [0.0, 1.0])                         # 位置（T=2）
    rp.init("ROT_IDX", [4], [2, 3, 0, 1], INT64)               # rotate_half 的置换
    rp.init("SIGN", [4], [-1.0, -1.0, 1.0, 1.0])               # 置换后前半取负
    rp.op("Cos", ["POS"], ["COS"])
    rp.op("Sin", ["POS"], ["SIN"])
    rp.op("Gather", ["X", "ROT_IDX"], ["GX"], [G.attr("axis", "i", 1)])
    rp.op("Mul", ["GX", "SIGN"], ["ROT"])
    rp.op("Mul", ["X", "COS"], ["XC"])
    rp.op("Mul", ["ROT", "SIN"], ["RS"])
    rp.op("Add", ["XC", "RS"], ["OUT"])
    for nm in ("COS", "SIN"):
        rp.out(nm, [2, 1])
    for nm in ("GX", "ROT", "XC", "RS"):
        rp.out(nm, [2, 4])
    rp.out("OUT", [2, 4])
    rp.exact("COS", [1.0, 0.5403023059], "cos(0), cos(1)（10 位）")
    rp.exact("SIN", [0.0, 0.8414709848], "sin(0), sin(1)（10 位）")
    rp.exact("GX", [3.0, 4.0, 1.0, 2.0, 7.0, 8.0, 5.0, 6.0], "rotate_half 置换 [2,3,0,1]")
    rp.exact("ROT", [-3.0, -4.0, 1.0, 2.0, -7.0, -8.0, 5.0, 6.0], "置换后前半取负")
    rp.exact("XC", [1.0, 2.0, 3.0, 4.0,
                    2.7015115293, 3.2418138352, 3.7821161411, 4.3224184469],
             "x·cos（位置 0/1 广播 · 10 位）")
    rp.exact("RS", [0.0, 0.0, 0.0, 0.0,
                    -5.8902968937, -6.7317678785, 4.2073549240, 5.0488259088],
             "rotate_half(x)·sin（位置 0/1 广播 · 10 位）")
    rp.exact("OUT", [1.0, 2.0, 3.0, 4.0,
                     -3.1887853643, -3.4899540433, 7.9894710651, 9.3712443558],
             "RoPE：x·cos + rotate_half(x)·sin（位置 0/1，10 位）")
    return [kv, rp]


def main():
    os.makedirs(MODELS, exist_ok=True)
    os.makedirs(FEEDS, exist_ok=True)
    nego = os.path.join(MODELS, "neg")
    os.makedirs(nego, exist_ok=True)

    graphs, hands, notes, names = {}, {}, {}, []

    # ── 正例（import M157 的同一份定义 ⇒ 两个门看到的是**同一个模型**）──
    pos = G.m13_s4_ops()
    open(os.path.join(MODELS, pos.name + ".onnx"), "wb").write(pos.build())
    graphs[pos.name] = pos.graph_json()
    json.dump(pos.feeds, open(os.path.join(FEEDS, pos.name + ".json"), "w"),
              ensure_ascii=False, indent=1)
    hands[pos.name] = pos.hand
    notes[pos.name] = pos.hand_note
    names.append(pos.name)

    # ── 本门专有判据模型（属性语义：epsilon / axis / scale）──
    # ── + attention 辅助（§二.S2：KV cache 增量解码等价性 / RoPE 复合式）──
    for m in judgment() + rope_kv():
        open(os.path.join(MODELS, m.name + ".onnx"), "wb").write(m.build())
        graphs[m.name] = m.graph_json()
        json.dump(m.feeds, open(os.path.join(FEEDS, m.name + ".json"), "w"),
                  ensure_ascii=False, indent=1)
        if m.hand:
            hands[m.name] = m.hand
            notes[m.name] = m.hand_note
        names.append(m.name)

    json.dump(graphs, open(os.path.join(MODELS, "graphs.json"), "w"), ensure_ascii=False, indent=1)
    json.dump(hands, open(os.path.join(MODELS, "expected.json"), "w"), ensure_ascii=False, indent=1)
    json.dump(notes, open(os.path.join(MODELS, "expected_notes.json"), "w"),
              ensure_ascii=False, indent=1)

    # ── 行为负控 ──
    cases = []
    for mm, why, op in negs():
        open(os.path.join(nego, mm.name + ".onnx"), "wb").write(mm.build())
        json.dump(mm.feeds, open(os.path.join(nego, mm.name + ".json"), "w"), ensure_ascii=False)
        cases.append({"name": mm.name, "why": why, "op": op})
    json.dump(cases, open(os.path.join(nego, "cases.json"), "w"), ensure_ascii=False, indent=1)

    ops = sorted({n["op"] for g in graphs.values() for n in g["nodes"]})
    print("正例模型 %d 个：%s" % (len(names), ",".join(names)))
    print("正例覆盖算子 %d 个：%s" % (len(ops), ",".join(ops)))
    print("负控 %d 个：%s" % (len(cases), ",".join(c["name"] for c in cases)))


if __name__ == "__main__":
    main()
