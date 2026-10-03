#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M231 门 · 三轨对拍器（「运算符 × 逐类型组合」矩阵）

判据
----
① **响亮性**（VAL / ERR）三轨必须一致；
② VAL ⇒ 三轨**值**必须逐字节一致；
③ ERR ⇒ 三轨 **R 码**一致，且 R 码相同时**词条**（归一化后的正文）一致；
   ⇒ 这是「错误必须指到真因」的落地（M185/M187 纪律）—— 缺陷 346 修前
      `"s" - 1` 在解释轨**没有任何报错**（VAL `s`），编译轨报 `无法相减: string - int`。
④ 与 `MODEL.tsv` **双向**核对（漏登记 / 过期都判红）。

诊断通道
--------
解释轨写 `运行时错误: 错误 [R1002] 行:列: <词条>`（含行:列）；编译轨写
`运行时错误 [tN 行M]: R1002: <词条>`。**通道与行:列前缀的差异是已登记缺陷 186**，
不属本轮判据 ⇒ 归一化时抹掉，只留 `R码 + 词条`。**归一化规则写在 norm_err() 里**
（不许"看着不一样就放过"）。
"""
import argparse, json, os, re, subprocess, sys

NORM_TAIL = re.compile(r'^\s*[\]\s]*')


def norm_err(out):
    """→ ('VAL', 值) | ('ERR', R码, 词条) | ('OTHER', 原文)"""
    o = out.strip()
    m = re.search(r'(R\d{4})', o)
    if m:
        rc = m.group(1)
        tail = o[m.start() + len(rc):]
        tail = tail.lstrip(":] \t")
        tail = re.sub(r'^\d+:\d+:\s*', '', tail)      # 解释轨「行:列:」
        tail = tail.lstrip(":] \t").strip()
        return ("ERR", rc, tail)
    if "语法错误" in o or "E20" in o:
        return ("ERR", "SYNTAX", o[-60:])
    if "=" in o:
        return ("VAL", o.split("=", 1)[1])
    return ("OTHER", o)


def run(cmd, env_extra, timeout=30):
    env = dict(os.environ)
    env.update(env_extra)
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout,
                           cwd=env.get("M246_CWD") or None, env=env)
        first = (r.stdout or r.stderr or "").strip().split("\n")[0]
        return first
    except subprocess.TimeoutExpired:
        return "运行时错误 TIMEOUT"


def shape346(root, work, drv, cases):
    """缺陷 346 的**精确形状**独立判据（不依赖「三轨一致」）

    `str <op> int` 中**除 `*`（合法重复）外的全部 12 个**（`+` 走 i_bin_add、
    其余 11 个走 i_bin_numeric —— 346 的直接形状正是后者）必须：
      ① 三轨**均响亮**（修前解释轨静默给 `"s"`）；
      ② R 码均为 R1002；
      ③ **词条三轨逐字相同**（归一化后）；
      ④ 词条**含 `string`**（指到真因：左操作数的实际类型）。

    为什么单独一层：负控 B 会让「响亮性一致」变绿（三轨都静默）⇒ 只有本层能独立判红。
    """
    tracks = {
        "interp": [os.environ.get("M246_PXI", os.path.join(root, "bootstrap", "pxi")), drv],
        "vm":     [os.path.join(work, "build", "drv_vm")],
        "c":      [os.path.join(work, "build", "drv_c")],
    }
    sel = [c for c in cases
           if c[4] in ("arith-undefined-pair", "bit-int-only")
           and c[1].split("_")[-2:] == ["str", "int"]]
    bad = 0
    for idx, tag, expr, kind, basis in sel:
        res = {t: norm_err(run(cmd, {"M246_CASE": str(idx), "M246_CWD": os.path.dirname(drv)}))
               for t, cmd in tracks.items()}
        errs = [res[t][0] == "ERR" for t in tracks]
        if not all(errs):
            print("  ❌ %s %s：不是三轨均响亮 %s" % (tag, expr, res)); bad += 1; continue
        if any(res[t][1] != "R1002" for t in tracks):
            print("  ❌ %s %s：R 码不是 R1002 %s" % (tag, expr, res)); bad += 1; continue
        tails = {res[t][2] for t in tracks}
        if len(tails) != 1:
            print("  ❌ %s %s：词条三轨不同 %s" % (tag, expr, res)); bad += 1; continue
        tail = tails.pop()
        if "string" not in tail:
            print("  ❌ %s %s：词条未指到真因（缺 string）%r" % (tag, expr, tail)); bad += 1
    print("  形状例数 %d · 违例 %d" % (len(sel), bad))
    if len(sel) != 12:
        print("  ❌ 形状例数应为 12（11 个 i_bin_numeric 直接形状 + 1 个 `+` 对照；实测 %d）"
              "—— 覆盖面变了，请复核筛法" % len(sel))
        return 1
    if bad:
        print("M231-SHAPE346-FAIL")
        return 1
    print("M231-SHAPE346-OK")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--work", default="/tmp/m231_gate")
    ap.add_argument("--drv", default=None)
    ap.add_argument("--json", default=None)
    ap.add_argument("--shape346", action="store_true")
    a = ap.parse_args()
    root, work = os.path.abspath(a.root), a.work
    # ── M244（门内并行）：逐例 spawn 彼此**无依赖**（同一驱动器、不同 case 选择，
    #    独立进程、不写共享文件）⇒ 用 gate_par.pmap 吃满整机核数。
    #    ⚠️ 位置纪律：必须在 `main()` 的**最早使用之前**（`det_check` 在更下面会被调用）；
    #       且**同一区域不要再插入别的块**（事故 1 就是插入点与替换区重叠）。
    sys.path.insert(0, os.path.join(root, 'selfhost'))
    from gate_par import pmap, pmap_records   # noqa: E402
    d = os.path.join(root, "examples", "m246_compound_assign")
    drv = a.drv or os.path.join(d, "drv.px")

    cases = []
    for ln in open(os.path.join(d, "cases.tsv"), encoding="utf-8"):
        f = ln.rstrip("\n").split("\t")
        if len(f) >= 5 and f[0] != "idx":
            cases.append((int(f[0]), f[1], f[2], f[3], f[4]))

    if a.shape346:
        return shape346(root, work, drv, cases)

    tracks = {
        "interp": [os.environ.get("M246_PXI", os.path.join(root, "bootstrap", "pxi")), drv],
        "vm":     [os.path.join(work, "build", "drv_vm")],
        "c":      [os.path.join(work, "build", "drv_c")],
    }
    for t, cmd in tracks.items():
        if not os.path.exists(cmd[0]):
            print("❌ 缺少 %s 轨件 %s" % (t, cmd[0]))
            return 2

    _GOT231 = pmap_records(
        lambda k: (k, norm_err(run(tracks[k[1]],
                                   {"M246_CASE": str(k[0]), "M246_CWD": os.path.dirname(drv)}))),
        [(idx, t) for idx, *_r in cases for t in tracks])
    diffs, actual = [], []
    for idx, tag, expr, kind, basis in cases:
        res = {t: _GOT231[(idx, t)] for t in tracks}
        # ① 响亮性
        kinds = {res[t][0] for t in tracks}
        if len(kinds) != 1:
            diffs.append(("响亮性", tag, expr, res))
            continue
        k = kinds.pop()
        # ② VAL ⇒ 值一致
        if k == "VAL":
            vals = {res[t][1] for t in tracks}
            if len(vals) != 1:
                diffs.append(("值不同", tag, expr, res))
            actual.append((tag, "VAL"))
            continue
        # ③ ERR ⇒ R 码 + 词条一致
        rcs = {res[t][1] for t in tracks}
        if len(rcs) != 1:
            diffs.append(("R码不同", tag, expr, res))
            actual.append((tag, "ERR"))
            continue
        tails = {res[t][2] for t in tracks}
        if len(tails) != 1:
            diffs.append(("词条不同", tag, expr, res))
        actual.append((tag, "ERR"))

    # ④ 与 MODEL 双向核对
    model = {}
    for ln in open(os.path.join(d, "MODEL.tsv"), encoding="utf-8"):
        f = ln.rstrip("\n").split("\t")
        if len(f) >= 3 and f[0] != "tag":
            model[f[0]] = f[1]
    got = dict(actual)
    miss = sorted(set(model) - set(got))       # 表里有、实测没有 ⇒ 过期
    extra = sorted(set(got) - set(model))      # 实测有、表里没有 ⇒ 漏登记
    mismatch = sorted(t for t in got if t in model and got[t] != model[t])
    with open(os.path.join(work, "model_actual.tsv"), "w") as f:
        f.write("tag\tkind\n")
        for tag in sorted(got):
            f.write("%s\t%s\n" % (tag, got[tag]))

    print("例数 %d · 三轨差异 %d · MODEL 不一致 %d" % (len(cases), len(diffs), len(mismatch)))
    for kind_, tag, expr, res in diffs[:20]:
        print("  ❌ [%s] %s  %s" % (kind_, tag, expr))
        for t in ("interp", "vm", "c"):
            print("       %-6s %s" % (t, res[t]))
    for t in mismatch[:20]:
        print("  ❌ [MODEL] %s：表 %s ⇄ 实测 %s" % (t, model[t], got[t]))
    for t in extra[:20]:
        print("  ❌ [MODEL] 漏登记 %s（实测 %s 不在表里）" % (t, got[t]))
    for t in miss[:20]:
        print("  ❌ [MODEL] 登记过期 %s（表里有、实测没有）" % t)
    if a.json:
        json.dump({"n": len(cases), "diffs": len(diffs), "model_bad": len(mismatch) + len(miss) + len(extra),
                   "actual": got}, open(a.json, "w"), ensure_ascii=False, indent=1)

    ok = (not diffs) and (not mismatch) and (not miss) and (not extra)
    if ok:
        print("M246-THREE-TRACKS-OK")
        return 0
    return 1


sys.exit(main())
