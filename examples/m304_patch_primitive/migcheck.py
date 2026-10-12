#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""migcheck.py —— 「sed 打桩 → patch_file/gp_patch」迁移台账核对（M304）

为什么：M304 起，门的负控打桩从 `sed -i`（两类**静默失效**：元字符陷阱 / 找不到也 rc=0）
迁移到 `selfhost/patch_file.py`（字面量 · 零转义 · 失败响亮 · --count 强校验）。
迁移是**逐门推进**的 ⇒ 需要一份**双向**台账：
  · 台账里有的      ⇒ 必须真迁移了（否则「登记过期」⇒ 判红）
  · 真迁移了的      ⇒ 必须在台账里（否则「漏登记」  ⇒ 判红）
不做成双向的话，台账会退化成「写了就算」的装饰
（M226 的 UNPAIRED.tsv / M227 的两张表都是同款纪律：**双向**才有牙）。

判据（**命令位置**，不是「字符串出现即算」）：
  A 台账内的门：verify.sh 必须 source gate_patch.sh 且调用 gp_patch/gp_patch_spec
  B 台账内的门：`sed -i` 打桩数必须 == 台账登记数（登记保留的豁免，**且必须写理由**）
  C 台账外    ：不得出现 gp_patch 调用
  D 规模锚点  ：扫描到的门数 ≥ --min-gates（默认 150）· 台账条数 ≥ 1
      —— 防「扫描器坏了 ⇒ 空集 ⊇ 空集 ⇒ 全绿」（M212 缺陷 288 的纪律）

⚠️ 为什么判据必须是**命令位置**（行首可缩进）而不是「body 里出现 gp_patch」：
   本门（m304）自己的负控会 `printf '…\\ngp_patch f a b\\n' > 夹具/verify.sh` ——
   那行**含 gp_patch 字面量但不是在调用它**。首版用「出现即算」⇒ 把本门自己判成「漏登记」（假阳）。
   ⇒ 与 M265 的那条同族：**判据要贴着契约写（「调用了 gp_patch」），不是贴着表象（「出现过这个串」）**。
   覆盖边界：只认**行首**调用；`cmd && gp_patch …` 这类内联形态不在面内（本仓无此写法）。

用法：migcheck.py [--root <repo>] [--ledger <tsv>] [--min-gates N] [--list]
退出码：0 = 一致；1 = 有违例；3 = 用法 / IO
输出：  MIG-OK migrated=N ledger=N scanned=N remaining_sed=N
        MIG-FAIL <逐条违例>
"""
import argparse
import glob
import os
import re
import sys

# 「命令位置」= 行首（允许缩进）。`gp_patch` / `gp_patch_spec` 都算。
GP_CALL = re.compile(r"(?m)^[ \t]*gp_patch(_spec)?\b")
# source 形态：行首 `.` + 空白 + … + gate_patch.sh
GP_SRC = re.compile(r"(?m)^[ \t]*\.[ \t].*gate_patch\.sh")


def code_lines(path):
    """返回**非注释**行（行首 # 跳过）—— 注释里提到 sed / gp_patch 不算数。"""
    out = []
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            if line.lstrip().startswith("#"):
                continue
            out.append(line)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--ledger", default=None)
    ap.add_argument("--min-gates", type=int, default=150)
    ap.add_argument("--list", action="store_true")
    a = ap.parse_args()

    root = os.path.abspath(a.root)
    here = os.path.dirname(os.path.abspath(__file__))
    ledger_path = a.ledger or os.path.join(here, "MIGRATED.tsv")

    files = sorted(glob.glob(os.path.join(root, "examples", "*", "verify.sh")))
    if not files:
        print("MIG-FAIL 扫不到任何门（%s/examples/*/verify.sh）" % root)
        return 3

    ledger = {}
    reasons = {}
    if not os.path.exists(ledger_path):
        print("MIG-FAIL 台账不存在：%s" % ledger_path)
        return 3
    with open(ledger_path, encoding="utf-8") as fh:
        for ln, line in enumerate(fh, 1):
            s = line.rstrip("\n")
            if not s.strip() or s.lstrip().startswith("#"):
                continue
            parts = s.split("\t")
            if len(parts) < 2:
                print("MIG-FAIL 台账第 %d 行缺列（需「门名<TAB>保留sed数<TAB>理由」）：%s" % (ln, s))
                return 3
            try:
                cnt = int(parts[1])
            except ValueError:
                print("MIG-FAIL 台账第 %d 行的保留数不是整数：%s" % (ln, parts[1]))
                return 3
            ledger[parts[0]] = cnt
            reasons[parts[0]] = parts[2] if len(parts) > 2 else ""

    viol = []
    migrated = []
    remaining_sed = 0
    for f in files:
        name = os.path.basename(os.path.dirname(f))
        body = "".join(code_lines(f))
        has_gp = bool(GP_CALL.search(body))
        has_src = bool(GP_SRC.search(body))
        n_sed = body.count("sed -i")
        remaining_sed += n_sed
        if has_gp:
            migrated.append(name)
        if name in ledger:
            if not has_src:
                viol.append("A %s：台账已登记，但 verify.sh 未 source gate_patch.sh" % name)
            if not has_gp:
                viol.append("A %s：台账已登记，但 verify.sh 里没有 gp_patch 调用" % name)
            if n_sed != ledger[name]:
                viol.append("B %s：sed -i 打桩 %d 处，台账登记 %d 处" % (name, n_sed, ledger[name]))
        elif has_gp:
            viol.append("C %s：已用 gp_patch，但不在台账里（漏登记）" % name)

    for name in sorted(ledger):
        if not os.path.exists(os.path.join(root, "examples", name, "verify.sh")):
            viol.append("A %s：台账条目指向的门不存在" % name)
        if ledger[name] > 0 and not reasons.get(name, "").strip():
            viol.append("B %s：登记保留 %d 处 sed，但没写理由（无理由豁免 = 把红当绿）"
                         % (name, ledger[name]))

    if a.list:
        for n in sorted(ledger):
            print("  %-22s keep_sed=%d  %s" % (n, ledger[n], reasons.get(n, "")))

    if len(files) < a.min_gates:
        viol.append("D 扫描到的门数 %d < 规模锚点 %d（扫描器可能坏了）" % (len(files), a.min_gates))
    if len(ledger) < 1:
        viol.append("D 台账条数 0（空台账 ⊇ 任何集合 ⇒ 判据恒真）")

    if viol:
        print("MIG-FAIL scanned=%d ledger=%d migrated=%d" % (len(files), len(ledger), len(migrated)))
        for v in viol[:20]:
            print("  " + v)
        return 1
    print("MIG-OK migrated=%d ledger=%d scanned=%d remaining_sed=%d"
          % (len(migrated), len(ledger), len(files), remaining_sed))
    return 0


if __name__ == "__main__":
    sys.exit(main())
