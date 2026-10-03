#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M246 门 · 负控锚点库（打桩 / 还原 / 自证）

负控 A（**人为注入**，与 M231 的「忠实撤回修复」语义不同 —— 本轮**没有**待撤回的修复）：
  把 `cg_stmt.px` 的 `cg_assign_op_local` 里 `+=` 那一支**退化成直接赋值**
  （`return "px_add(v, rhs)"` → `return rhs`），模拟缺陷 245 在 **Var 目标**上重演。
  ⇒ C 轨 `x += 1` 给出 `1`，而解释/VM 轨给出 `2` ⇒ **「三轨对拍」层必须判红**。

负控 B（判据自伤）：把三轨比对短路 ⇒ A 的红**必须消失**（证红来自比对本身）。

⚠️ 打桩后必须**源逐字节还原**（快照 + cmp）—— 门里守。
"""
import argparse, os, shutil, sys

ROOT = None
SNAP = None
FILES = ["selfhost/cg_stmt.px"]

# 锚点必须**唯一命中**（不许猜次数 —— M223 教训）
ANCHOR_A = '        return "px_add(" + v + ", " + rhs + ")"'
PATCH_A = '        return rhs   # M246-NEGCTL-A：丢弃运算符（模拟缺陷 245 在 Var 目标重演）'


def cnt(path, s):
    return open(os.path.join(ROOT, path), encoding="utf-8").read().count(s)


def sub(path, old, new, expect=1):
    p = os.path.join(ROOT, path)
    s = open(p, encoding="utf-8").read()
    n = s.count(old)
    if n != expect:
        raise SystemExit("锚点命中 %d 次（期望 %d）：%s ... %s" % (n, expect, path, old[:60]))
    open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
    return n


def snapshot():
    os.makedirs(SNAP, exist_ok=True)
    for f in FILES:
        d = os.path.join(SNAP, f.replace("/", "__"))
        shutil.copyfile(os.path.join(ROOT, f), d)


def restore():
    for f in FILES:
        d = os.path.join(SNAP, f.replace("/", "__"))
        if os.path.exists(d):
            shutil.copyfile(d, os.path.join(ROOT, f))


def main():
    global ROOT, SNAP
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--snap", default="/tmp/m246_snap")
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--restore", action="store_true")
    a = ap.parse_args()
    ROOT, SNAP = os.path.abspath(a.root), a.snap

    if a.selftest:
        ok = True
        n = cnt(FILES[0], ANCHOR_A)
        if n != 1:
            print("❌ 锚点 A 命中 %d 次（期望 1）" % n)
            ok = False
        if cnt(FILES[0], "M246-NEGCTL-A"):
            print("❌ 源里已存在打桩痕迹（说明上次没还原）")
            ok = False
        print("NEGCTL-SELFTEST-%s（锚点 A 唯一命中）" % ("OK" if ok else "FAIL"))
        return 0 if ok else 1

    if a.apply:
        snapshot()
        sub(FILES[0], ANCHOR_A, PATCH_A, 1)
        print("NEGCTL-APPLIED")
        return 0

    if a.restore:
        restore()
        print("NEGCTL-RESTORED")
        return 0

    print("usage: negctl.py --root R --snap S [--selftest|--apply|--restore]")
    return 2


if __name__ == "__main__":
    sys.exit(main())
