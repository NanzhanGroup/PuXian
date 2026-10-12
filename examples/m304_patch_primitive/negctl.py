#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M304 负控注入器 —— 每道负控**独立**注入（先还原、再打桩），逐字节可还原。

三个动作分别对应「打桩原语」存在的三条理由，各自必须能**独立判红**：
  A  not-found 静默成功   ⇒ 治「sed 找不到也 rc=0」这条
  B  命中数不校验         ⇒ 治「打桩范围超出声明意图」（M286 的 A4）
  C  判据自伤（比较恒真） ⇒ 证明 A 的红**来自判据本身**，不是别处的噪声

⚠️ 纪律（本仓反复踩过）：
  · 锚点必须**恰好命中 1 处**（不猜次数；命中 0 或 ≥2 ⇒ 立刻 rc=3 拒绝注入 —— M285 的 A1 语义）
  · 注入前先 `restore`（各负控**干净起点** —— M165 那次的教训：负控级联污染）
  · 还原后**逐字节核对** sha256（M214 的教训：比 HEAD 而不是比快照，会恒判「未还原」）
"""
import argparse
import hashlib
import os
import shutil
import sys

ACTIONS = {
    # (目标, 旧文本, 新文本, 说明)
    "A": ("selfhost/patch_file.py",
          '''        actual = text.count(old)
        if actual == 0:
            fail_line(path, idx, "not-found", want, actual)
            die("第 %d 处：文件中未找到 OLD（文件一字未改）" % idx, RC_NOTFOUND)''',
          '''        actual = text.count(old)
        if actual == 0:
            # NEGCTL-A：故意退回 "找不到也静默成功"（= sed 的失效形态）
            sys.stdout.write(
                "PATCH-OK file=%s patches=0 bytes_delta=+0 dry_run=0 changed=0\\n" % path
            )
            return RC_OK''',
          "not-found 静默成功（sed 失效形态 ②）"),
    "B": ("selfhost/patch_file.py",
          '''        if want >= 0 and actual != want:
            fail_line(path, idx, "count-mismatch", want, actual)''',
          '''        if want >= 0 and actual != want and False:
            # NEGCTL-B：故意关掉命中数强校验
            fail_line(path, idx, "count-mismatch", want, actual)''',
          "命中数不再强校验（打桩范围失控）"),
    "C": ("examples/m304_patch_primitive/diffcheck.py",
          "        same = (ca == cb) and (sa == sb) and (ma == mb)",
          "        same = True  # NEGCTL-C：判据自伤（比较恒真）",
          "判据自伤（差分比较恒真）"),
}


def sha(p):
    with open(p, "rb") as fh:
        return hashlib.sha256(fh.read()).hexdigest()


class Ctl(object):
    def __init__(self, root, bak):
        self.root = root
        self.bak = bak
        self.targets = sorted({v[0] for v in ACTIONS.values()})

    def path(self, rel):
        return os.path.join(self.root, rel)

    def bakpath(self, rel):
        return os.path.join(self.bak, rel.replace("/", "__"))

    def snapshot(self):
        os.makedirs(self.bak, exist_ok=True)
        for rel in self.targets:
            shutil.copy2(self.path(rel), self.bakpath(rel))
        self.write_shas()

    def write_shas(self):
        with open(os.path.join(self.bak, "SHA256"), "w", encoding="utf-8") as fh:
            for rel in self.targets:
                fh.write("%s  %s\n" % (sha(self.bakpath(rel)), rel))

    def shas(self):
        out = {}
        with open(os.path.join(self.bak, "SHA256"), encoding="utf-8") as fh:
            for line in fh:
                h, rel = line.split(None, 1)
                out[rel.strip()] = h
        return out

    def restore(self, verify=True):
        want = self.shas()
        for rel in self.targets:
            if not os.path.exists(self.bakpath(rel)):
                continue
            shutil.copy2(self.bakpath(rel), self.path(rel))
        if verify:
            bad = [rel for rel in self.targets
                   if os.path.exists(self.bakpath(rel)) and sha(self.path(rel)) != want[rel]]
            if bad:
                print("NEGCTL-RESTORE-FAIL 还原后 sha256 不符：%s" % ",".join(bad))
                return 3
            print("NEGCTL-RESTORE-OK %d 件逐字节还原" % len(want))
        return 0

    def apply(self, action):
        rel, old, new, desc = ACTIONS[action]
        p = self.path(rel)
        with open(p, "r", encoding="utf-8") as fh:
            text = fh.read()
        n = text.count(old)
        if n != 1:
            print("NEGCTL-ANCHOR-FAIL action=%s 锚点命中 %d 处（要求恰好 1）" % (action, n))
            return 3
        with open(p, "w", encoding="utf-8") as fh:
            fh.write(text.replace(old, new))
        print("NEGCTL-APPLIED action=%s target=%s desc=%s" % (action, rel, desc))
        return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("action", choices=["snapshot", "restore", "A", "B", "C"])
    ap.add_argument("--root", required=True)
    ap.add_argument("--bak", default="/tmp/m304_gate/negbak")
    a = ap.parse_args()
    c = Ctl(a.root, a.bak)
    if a.action == "snapshot":
        c.snapshot()
        print("NEGCTL-SNAPSHOT-OK %d 件" % len(c.targets))
        return 0
    if a.action == "restore":
        return c.restore()
    rc = c.restore(verify=False)  # 各负控从**干净起点**注入
    if rc:
        return rc
    return c.apply(a.action)


if __name__ == "__main__":
    sys.exit(main())
