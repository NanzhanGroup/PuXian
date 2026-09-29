#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""门锚点哨兵 —— 「改了源码 ⇒ 旧门的**补丁锚点**还在不在」

为什么有这道守卫（M229 建立）：
    「改我动过的那几行之前，先 grep 旧门负控的锚点文本」这条纪律**已在 6 个里程碑里复发**
    （M161 · M164 · M178/M227 · M228 连带 M227 · M229 连带 M190）。症状每次都一样：
    我改了 runtime/selfhost 的某段源码，某个旧门里嵌着那段源码的**逐字副本**当补丁锚点
    ⇒ `assert s.count(anchor)==1` 失败 ⇒ 该门判红，而红的信息**指不到真因**。

判据（**精确** · 只查「断言必须存在」的那一类锚点，不查补丁的 new 侧/sed 模式）：
    对每个门文件，找出 `assert s.count(<变量>)==1` / `assert s.count('<字面量>')==1`，
    再回溯该变量的赋值（`x = '''…'''` 或 `x = "…"`），把 blob 归一化空白后
    在 `runtime/*.{c,h}` / `selfhost/*.px` / `tools/*.px` / `stdlib/*.px` 里查存在性。
    找不到 ⇒ **锚点已过期**（源码往前走了，门还在指旧文本）⇒ 报红。

豁免：`examples/m229_result_tuple/ANCHORS.tsv`（`文件<TAB>片段前缀<TAB>理由`）——
    只用于「有意保留的历史文本」（如断言**旧行为**应当报错的迁移动词）。
"""
import os
import re
import sys

ROOT = os.environ.get("GATE_ANCHOR_ROOT", os.getcwd())
HERE = os.path.dirname(os.path.abspath(__file__))
GATE_ROOTS = [("examples", r"\.(sh|py)$"), ("selfhost", r"\.sh$")]
SRC_ROOTS = [("runtime", r"\.(c|h)$"), ("selfhost", r"\.px$"), ("tools", r"\.px$"), ("stdlib", r"\.px$")]
SKIP_DIRS = ("build", "golden", ".rtcache", "node_modules")
SIG = re.compile(r"(px_error\(|i_r100[0-9]\(|strcmp\(name,|return Err\()")


def norm(s):
    s = s.replace("\\n", "\n").replace('\\"', '"').replace("\\'", "'")
    return re.sub(r"\s+", " ", s).strip()


def files(base, pat):
    out = []
    for dp, dns, fns in os.walk(os.path.join(ROOT, base)):
        dns[:] = [d for d in dns if d not in SKIP_DIRS]
        for fn in fns:
            if re.search(pat, fn):
                out.append(os.path.join(dp, fn))
    return out


def main():
    blob = []
    for base, pat in SRC_ROOTS:
        for p in files(base, pat):
            try:
                blob.append(norm(open(p, encoding="utf-8", errors="replace").read()))
            except OSError:
                pass
    blob = "\n".join(blob)

    exempt = set()
    ex = os.path.join(ROOT, "examples", "m229_result_tuple", "ANCHORS.tsv")
    if os.path.exists(ex):
        for ln in open(ex, encoding="utf-8"):
            f = ln.rstrip("\n").split("\t")
            if len(f) >= 2 and not ln.startswith("#") and not ln.startswith("文件"):
                exempt.add(norm(f[1]))

    stale, checked = [], 0
    for base, pat in GATE_ROOTS:
        for p in files(base, pat):
            rel = os.path.relpath(p, ROOT)
            try:
                txt = open(p, encoding="utf-8", errors="replace").read()
            except OSError:
                continue
            if "s.count(" not in txt:
                continue
            # ⚠️ 同一个文件里 patch_A/B/C 各自都写 `a='''…'''` ⇒ **不能**用 dict 存
            #   （后一个会覆盖前一个，实测导致本工具对 m190 的三条锚点只看到最后一条 = 没牙）。
            #   正确做法：按**位置**配对 —— 每个 assert 取「它之前最近的同名赋值」。
            asigns = []
            for m in re.finditer(r"^[ \t]*(\w+)[ \t]*=[ \t]*'''(.*?)'''", txt, re.S | re.M):
                asigns.append((m.start(), m.group(1), m.group(2)))
            for m in re.finditer(r'^[ \t]*(\w+)[ \t]*=[ \t]*"([^"\n]{12,})"', txt, re.M):
                asigns.append((m.start(), m.group(1), m.group(2)))
            asigns.sort()
            for m in re.finditer(r"assert\s+\w+\.count\(\s*(\w+)\s*\)\s*==\s*1", txt):
                v = m.group(1)
                prev = [t for t in asigns if t[1] == v and t[0] < m.start()]
                if not prev:
                    continue
                a = norm(prev[-1][2])
                if len(a) < 12 or not SIG.search(a):
                    continue
                checked += 1
                if any(e and (a.startswith(e) or e.startswith(a)) for e in exempt):
                    continue
                if a not in blob:
                    ln = txt[: m.start()].count("\n") + 1
                    stale.append((rel, ln, a))

    print(f"── 扫描：`assert s.count(x)==1` 型补丁锚点 {checked} 条")
    if stale:
        print(f"❌ 过期锚点 {len(stale)} 条（源码里已不存在 ⇒ 对应门的负控会**报错而非判红**）：")
        for rel, ln, a in stale:
            print(f"   {rel}:{ln}  «{a[:110]}»")
        return 1
    print("✅ 无过期锚点")
    return 0


if __name__ == "__main__":
    sys.exit(main())
