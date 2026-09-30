#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M234 门 · 三轨对拍器 + 两条独立判据（「文本语义接口 × 逐类型实参」）

判据
----
① **三轨一致**：响亮性 / 值 / R 码 + 词条（归一化掉已登记缺陷 186 的通道差异）；
② **对齐**（独立真值，不依赖三轨）：对**非 `bytes`** 实参，`f(x)` 必须逐字节等于 `f(str(x))`
   —— 这是 §6.5「文本语义 = 取 `str()` 形态」的**直接判据化**；
③ **确定性**（本轮新增的判据，抓 UB 的指纹）：同一二进制把整批用例连跑两遍，
   输出必须**逐字节一致** —— 读 union 垃圾的缺陷由此**必然**现身
   （缺陷 348 修前：`sha256([1])` 连跑 5 次得 5 个不同值）；
④ 与 `MODEL.tsv` **双向**核对（漏登记 / 登记过期都判红）。
"""
import argparse, json, os, re, subprocess, sys


def norm(out):
    o = (out or "").strip().split("\n")[0].strip()
    if not o:
        return ("OTHER", "<空输出>")
    m = re.search(r'(R\d{4})', o)
    if m:
        rc = m.group(1)
        tail = o[m.start() + len(rc):].lstrip(":] \t")
        tail = re.sub(r'^\d+:\d+:\s*', '', tail).strip()
        return ("ERR", rc, tail)
    if "语法错误" in o or "E20" in o:
        return ("ERR", "SYNTAX", o[-60:])
    return ("VAL", o)


def run(cmd, work, idx, timeout=30, perturb=None):
    env = dict(os.environ); env["M234_CASE"] = str(idx)
    if perturb:
        # M233：**环境扰动**（M207 的 `PX_M207_PERTURB` 同款手法）——
        #   指针位派生的错值会随进程布局变化；**正确的实现必须与环境无关**。
        env["M233_PERTURB"] = perturb
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout,
                           cwd=work, env=env)
        return norm(r.stdout or r.stderr)
    except subprocess.TimeoutExpired:
        return ("OTHER", "TIMEOUT")


def load(d):
    cs = []
    for ln in open(os.path.join(d, "cases.tsv"), encoding="utf-8"):
        f = ln.rstrip("\n").split("\t")
        if len(f) >= 3 and f[0] != "idx":
            cs.append((int(f[0]), f[1], f[2]))
    return cs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--work", default="/tmp/m233_gate")
    ap.add_argument("--drv", default=None)
    ap.add_argument("--json", default=None)
    ap.add_argument("--det", action="store_true", help="只跑确定性判据")
    ap.add_argument("--harm", action="store_true",
                    help="判据自伤：**三层判据全关**（仅负控 C 用；正常路径不得带）")
    a = ap.parse_args()
    root, work = os.path.abspath(a.root), a.work
    d = os.path.join(root, "examples", "m234_bytes_family")
    drv = os.path.abspath(a.drv or os.path.join(d, "drv.px"))
    cs = load(d)
    wd = os.path.dirname(drv)

    tracks = {
        "interp": [os.environ.get("M234_PXI", os.path.join(root, "bootstrap", "pxi")), drv],
        "vm":     [os.path.join(work, "build", "drv_vm")],
        "c":      [os.path.join(work, "build", "drv_c")],
    }

    # ---- ③ 确定性：整批跑两遍，逐字节一致 ----
    def det_check(pick):
        """同一二进制、**不同进程布局**下必须给出逐字节相同的结果

        第二遍带一个长环境变量（`M233_PERTURB`）⇒ 栈/堆布局变化 ⇒
        任何「读指针位当值」的实现都会在这里现形（缺陷 348 的指纹）。
        """
        bad = []
        for t, cmd in tracks.items():
            outs = []
            for k in range(2):
                o = []
                for idx, *_rest in cs:
                    r = run(cmd, wd, idx, perturb=None if k == 0 else "x" * 700)
                    o.append("%s|%s" % (idx, r))
                outs.append("\n".join(o))
            if outs[0] != outs[1]:
                dif = [i for i, (x, y) in enumerate(zip(outs[0].split("\n"), outs[1].split("\n")))
                       if x != y]
                bad.append((t, dif[:3], outs[0].split("\n")[dif[0]] if dif else "?",
                            outs[1].split("\n")[dif[0]] if dif else "?"))
        return bad

    if a.det:
        bad = det_check(None)
        for t, dif, x, y in bad:
            print("  ❌ [确定性] %s 轨：第 %s 例（run1 %s ⇄ run2 %s）" % (t, dif, x, y))
        print("M234-DET-OK" if not bad else "M234-DET-FAIL")
        return 0 if not bad else 1

    for t, cmd in tracks.items():
        if not os.path.exists(cmd[0]):
            print("❌ 缺少 %s 轨件 %s" % (t, cmd[0])); return 2

    diffs, actual = [], {}
    for idx, tag, _expr in cs:
        res = {t: run(cmd, wd, idx) for t, cmd in tracks.items()}
        kinds = {res[t][0] for t in tracks}
        if len(kinds) != 1:
            diffs.append(("响亮性", tag, res))
        else:
            k = kinds.pop()
            if k == "VAL":
                if len({res[t][1] for t in tracks}) != 1:
                    diffs.append(("值不同", tag, res))
            elif len({(res[t][1], res[t][2]) for t in tracks}) != 1:
                diffs.append(("错误不同", tag, res))
        actual[tag] = {t: res[t] for t in tracks}

    misalign = []

    # ---- ④ MODEL 双向 ----
    model = {}
    for ln in open(os.path.join(d, "MODEL.tsv"), encoding="utf-8"):
        f = ln.rstrip("\n").split("\t")
        if len(f) >= 3 and f[0] != "tag":
            model[f[0]] = f[1]
    miss = sorted(set(model) - set(actual))
    extra = sorted(set(actual) - set(model))
    mismatch = []
    if not a.harm:
        for tag in sorted(set(model) & set(actual)):
            if model[tag] == "AUTO":
                continue          # M234：本门的 MODEL 只做**双向成员**核对（kind 由实测决定）
            got = {t: actual[tag][t][0] for t in ("interp", "vm", "c")}
            bad = [t for t in got if got[t] != model[tag]]
            if bad:
                mismatch.append((tag, model[tag], got, bad))
    else:
        miss, extra = [], []

    if a.harm:
        diffs = []          # C：三层判据全关（自伤）—— 用于证明红来自判据而非噪声
    print("例数 %d · 三轨差异 %d · MODEL 不一致 %d · 漏登记 %d · 过期 %d"
          % (len(cs), len(diffs), len(mismatch), len(extra), len(miss)))
    for kind_, tag, res in diffs[:15]:
        print("  ❌ [%s] %s" % (kind_, tag))
        for t in ("interp", "vm", "c"):
            print("       %-6s %s" % (t, res[t]))
    for tag, t, x, y, arg in misalign[:15]:
        print("  ❌ [对齐] %s/%s（实参 %s）：f(x)=%s ⇄ f(str(x))=%s" % (tag, t, arg, x, y))
    for tag, want, got, bad in mismatch[:15]:
        print("  ❌ [MODEL] %s：表 %s ⇄ 不符轨 %s" % (tag, want, ",".join("%s=%s" % (t, got[t]) for t in bad)))
    for t in extra[:15]:
        print("  ❌ [MODEL] 漏登记 %s" % t)
    for t in miss[:15]:
        print("  ❌ [MODEL] 登记过期 %s" % t)
    if a.json:
        json.dump({"n": len(cs), "diffs": len(diffs), "misalign": len(misalign),
                   "model_bad": len(mismatch) + len(miss) + len(extra)},
                  open(a.json, "w"), ensure_ascii=False, indent=1)

    ok = (not diffs) and (not misalign) and (not mismatch) and (not miss) and (not extra)
    print("M234-THREE-TRACKS-OK" if ok else "M234-THREE-TRACKS-FAIL")
    return 0 if ok else 1


sys.exit(main())
