#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M246 门 · 形态等价性对拍：`x op= y` ⇄ `x = x op y`（逐对 · 三轨）

为什么单列一层
--------------
「复合赋值是二元运算的语法糖」这条**语义承诺**在 M204 的门头只写成了一句**口头断言**
（「`Var` 目标那一支走 `cg_assign_op_local`，是对的」）。本层把它做成**可判定的**：

  对**每一个** (运算符, 左类型, 右类型) 组合，两个形态在三轨上必须给出
  **同一个归一化结果**（同 VAL 同值 / 同 ERR 同 R 码同词条）。

⚠️ 归一化用 `three_tracks.norm_err`（**同一份实现**，不抄第二份）——
   通道与行:列 前缀的差异是已登记缺陷 186，不属本层判据。

⚠️ 两个本层专属的坑（都已修）
  ① `three_tracks.py` 末尾是模块级 `sys.exit(main())`（无 `__main__` 守卫）
     ⇒ 首版 `from three_tracks import …` **直接执行了整个对拍并 sys.exit**，
     本脚本的后续代码根本没跑（症状：pairs.log 里出现的是 three_tracks 的输出）。
     ⇒ 已给 `three_tracks.py` 加 `if __name__ == "__main__":` 守卫。
  ② 182 对 × 2 形态 × 3 轨 = **1092 次执行** —— 串行会超时
     ⇒ 用 M244 的 `gate_par.pmap_records`（逐例 spawn 彼此无依赖）。

用法：M246_PXI=<pxi> python3 pair_check.py --root R --work W
退出码：0 = 全等价；1 = 有不等价；2 = 前置（缺轨件/缺对照例）
"""
import argparse, os, sys


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--work", default="/tmp/m246_gate")
    a = ap.parse_args()
    root, work = os.path.abspath(a.root), a.work
    d = os.path.join(root, "examples", "m246_compound_assign")

    # ⚠️ import 必须在**最早使用之前**（M244 事故 1 的教训：插入点与替换区重叠
    #    会让 import 掉到两段中间 ⇒ UnboundLocalError）。
    sys.path.insert(0, d)
    from three_tracks import norm_err, run          # noqa: E402（复用同一份归一化/执行）
    sys.path.insert(0, os.path.join(root, "selfhost"))
    from gate_par import pmap_records               # noqa: E402（M244 的门内并行）

    drv = os.path.join(d, "drv.px")
    cases = []
    for ln in open(os.path.join(d, "cases.tsv"), encoding="utf-8"):
        f = ln.rstrip("\n").split("\t")
        if len(f) >= 5 and f[0] != "idx":
            cases.append((int(f[0]), f[1]))

    tracks = {
        "interp": [os.environ.get("M246_PXI", os.path.join(root, "bootstrap", "pxi")), drv],
        "vm":     [os.path.join(work, "build", "drv_vm")],
        "c":      [os.path.join(work, "build", "drv_c")],
    }
    for t, cmd in tracks.items():
        if not os.path.exists(cmd[0]):
            print("❌ 缺少 %s 轨件 %s" % (t, cmd[0]))
            return 2

    bytag = {}
    for idx, tag in cases:
        bytag.setdefault(tag, idx)
    pairs = []
    for idx, tag in cases:
        if tag.endswith("_ref"):
            continue
        ridx = bytag.get(tag + "_ref")
        if ridx is None:
            print("❌ 缺少对照例：%s" % tag)
            return 2
        pairs.append((tag, idx, ridx))

    base_env = {"M246_CWD": os.path.dirname(drv)}
    keys = [(idx, t) for _, ia, ib in pairs for idx in (ia, ib) for t in tracks]
    got = pmap_records(
        lambda k: (k, norm_err(run(tracks[k[1]], dict(base_env, M246_CASE=str(k[0]))))),
        keys)

    bad = 0
    for tag, ia, ib in pairs:
        for t in tracks:
            ra, rb = got[(ia, t)], got[(ib, t)]
            if ra != rb:
                print("  ❌ [%s] %s：复合 %s ⇄ 展开 %s" % (t, tag, ra, rb))
                bad += 1

    print("  配对数 %d（每对 2 形态 × 3 轨 = %d 次执行）" % (len(pairs), len(pairs) * 6))
    if bad:
        print("M246-PAIRS-FAIL（%d 处不等价）" % bad)
        return 1
    print("M246-PAIRS-OK（%d 对 × 3 轨全等价）" % len(pairs))
    return 0


if __name__ == "__main__":
    sys.exit(main())
