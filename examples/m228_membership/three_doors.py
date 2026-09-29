#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M228 门 · **三个门**全量对拍（运算符 `in`/`not in` ⇄ 函数 `contains` ⇄ 方法 `.contains`）

判据：
  H1 响亮性一致 —— 同一个例在**三门 × 三轨**上「报错 / 正常」必须一致
  H2 值一致     —— 正常例的取值必须等于 **Python 独立真值**，且跨门跨轨逐字节一致
  H3 词条归属   —— 各门只说自己的话（M227 纪律）：运算符门不得说 `contains`／`方法`；
                   函数门不得说 `运算符`；方法门不得说 `运算符`
  跨轨一致     —— 归一化后的 (R 码, 文本) 逐字节一致（通道/行号前缀按 M186 归一）
  RCODE 双向   —— 实测「门 × 错误形状 ⇒ R 码」⇄ RCODE.tsv **精确相等**（漏登记与过期都判红）
"""
import argparse
import os
import re
import subprocess
import sys

ap = argparse.ArgumentParser()
ap.add_argument("--root", required=True)
ap.add_argument("--work", required=True)
ap.add_argument("--drv", required=True)
ap.add_argument("--cases", required=True)
ap.add_argument("--rcode", required=True)
ap.add_argument("--timeout", type=int, default=60)
ap.add_argument("--harm", action="store_true",
                help="判据自伤（负控 C 用）：值判据取实测值为期望 ⇒ 一切恒真")
a = ap.parse_args()

ROOT = a.root
DOORS = ("op", "notop", "func", "method")
TRACKS = ("interp", "vm", "c")

BIN = {
    "interp": [os.environ.get("M228_PXI", os.path.join(ROOT, "bootstrap", "pxi")), a.drv],
    "vm": [os.path.join(a.work, "build", "drv_vm")],
    "c": [os.path.join(a.work, "build", "drv_c")],
}
for t, cmd in BIN.items():
    if not os.path.exists(cmd[0]):
        print(f"❌ 缺少 {t} 轨件 {cmd[0]}")
        sys.exit(2)

# ---- 读 cases.tsv ----
cases = {}
with open(a.cases, encoding="utf-8") as fh:
    hdr = fh.readline().rstrip("\n").split("\t")
    for ln in fh:
        f = ln.rstrip("\n").split("\t")
        if len(f) < 6:
            continue
        cases[f[0]] = dict(ckind=f[1], ekind=f[2], kind=f[3], val=f[4], supported=f[5])
assert len(cases) >= 70, f"例数下限不足: {len(cases)}"


def run(track, case, door):
    env = dict(os.environ, M228_CASE=case, M228_DOOR=door)
    try:
        p = subprocess.run(BIN[track], capture_output=True, text=True, timeout=a.timeout,
                           cwd=a.work, env=env, errors="replace")
    except subprocess.TimeoutExpired:
        return dict(val=None, code="TIMEOUT", text="", raw="TIMEOUT")
    out = (p.stdout or "").strip()
    if p.returncode == 0 and out in ("true", "false"):
        return dict(val=out, code="", text="", raw=out)
    blob = (p.stdout or "") + "\n" + (p.stderr or "")
    for ln in blob.splitlines():
        m = re.search(r"(R\d{4})", ln)
        if m:
            rest = ln[m.end():]
            # 解释轨是 `错误 [R1002] 行:列: 文本`、编译轨是 `R1002: 文本`
            # ⇒ 依次剥 `] ` / `: `、再剥 `行:列: `、最后剥前导标点
            #   （M186：通道与位置前缀不参与对拍）
            rest = re.sub(r"^[\s\]:]*", "", rest)
            rest = re.sub(r"^\d+:\d+:\s*", "", rest)
            rest = re.sub(r"^[^0-9A-Za-z\u4e00-\u9fff]+", "", rest).strip()
            return dict(val=None, code=m.group(1), text=rest, raw=blob.strip())
    return dict(val=None, code="?", text=blob.strip(), raw=blob.strip())


EXP_DOOR_CODE = {
    ("elem", "op"): "R1002", ("elem", "notop"): "R1002",
    ("elem", "func"): "R1002", ("elem", "method"): "R1002",
    ("coll", "op"): "R1002", ("coll", "notop"): "R1002",
    ("coll", "func"): "R1002", ("coll", "method"): "R1007",
}

bad = []
obs_rcode = {}
for cid, c in sorted(cases.items()):
    # 期望形状
    if c["kind"] == "VAL":
        exp_shape = None
        exp_val = c["val"]
    else:
        exp_val = None
        # 形状：str/dict 集合 ⇒ elem；其余 ⇒ coll
        exp_shape = "elem" if c["ckind"] in ("string", "dict") else "coll"

    per_door = {}
    for door in DOORS:
        res = {t: run(t, cid, door) for t in TRACKS}
        # 跨轨一致（归一化后的码 + 文本）
        norm = {(r["code"], r["text"]) for r in res.values()}
        vals = {r["val"] for r in res.values()}
        if len(norm) != 1 or len(vals) != 1:
            bad.append(f"跨轨分叉 {cid}/{door}: " + repr({t: (res[t]['code'], res[t]['text'], res[t]['val']) for t in TRACKS}))
        r = res["c"]
        # H1 响亮性
        got_err = r["val"] is None
        if exp_shape is None:
            if got_err:
                bad.append(f"H1 {cid}/{door}: 期望正常，实测 {r['code']} {r['text']}")
            else:
                want = exp_val if door != "notop" else ("false" if exp_val == "true" else "true")
                if r["val"] != want:
                    bad.append(f"H2 {cid}/{door}: 期望 {want}，实测 {r['val']}")
        else:
            if not got_err:
                bad.append(f"H1 {cid}/{door}: 期望响亮（{exp_shape}），实测正常 {r['val']}")
            else:
                exp_code = EXP_DOOR_CODE[(exp_shape, door)]
                if r["code"] != exp_code:
                    bad.append(f"RCODE {cid}/{door}: 期望 {exp_code}，实测 {r['code']}")
                # H3 词条归属
                if door in ("op", "notop"):
                    if "contains" in r["text"] or "方法" in r["text"]:
                        bad.append(f"H3 {cid}/{door}: 运算符门借用了别的门的词条: {r['text']}")
                elif door == "func":
                    if "运算符" in r["text"] or "方法" in r["text"]:
                        bad.append(f"H3 {cid}/{door}: 函数门借用了别的门的词条: {r['text']}")
                else:
                    if "运算符" in r["text"]:
                        bad.append(f"H3 {cid}/{door}: 方法门借用了别的门的词条: {r['text']}")
                obs_rcode.setdefault((door, exp_shape), set()).add(r["code"])
        per_door[door] = r

    # 跨门值/响亮性一致（H1/H2 的门间面）
    kinds = {d: (per_door[d]["val"] is None) for d in DOORS}
    if len(set(kinds.values())) != 1:
        bad.append(f"跨门响亮性不一致 {cid}: {kinds}")

# ---- RCODE 双向 ----
exp_rcode = set()
with open(a.rcode, encoding="utf-8") as fh:
    for ln in fh:
        if not ln.strip() or ln.lstrip().startswith("#") or ln.startswith("门\t"):
            continue
        f = ln.rstrip("\n").split("\t")
        if len(f) >= 3:
            exp_rcode.add((f[0], f[1], f[2]))
got_rcode = set()
for (door, shape), codes in obs_rcode.items():
    for code in codes:
        got_rcode.add((door, shape, code))
missing = sorted(exp_rcode - got_rcode)
stale = sorted(got_rcode - exp_rcode)
if missing:
    bad.append("RCODE 漏登记: " + repr(missing))
if stale:
    bad.append("RCODE 登记过期: " + repr(stale))

print(f"── 例 {len(cases)} × 门 {len(DOORS)} × 轨 {len(TRACKS)} = {len(cases)*len(DOORS)*len(TRACKS)} 次执行")
if a.harm:
    # 判据自伤（负控 C）：**只停裁决**，数据仍全部采集（崩/超时依旧会暴露）
    print(f"M228-THREE-DOORS-OK [--harm 判据自伤 · 已采集的差异 {len(bad)} 项不予裁决]")
    sys.exit(0)
if bad:
    for b in bad[:25]:
        print("  ❌ " + b)
    print(f"M228-THREE-DOORS-FAIL 差异 {len(bad)} 项")
    sys.exit(1)
print("M228-THREE-DOORS-OK 跨轨 0 · 跨门 0 · 与独立真值一致 · 词条归属 0 违规")
