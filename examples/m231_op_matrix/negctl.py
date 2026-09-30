#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M231 门 · 负控锚点库（打桩 / 还原 / 自证）

三个负控，各自**证明不同的判据层有牙**（这是本轮最重要的一组设计）：
  A  忠实撤回到 M231 修前：`ival.px` 的 `string × int` 重复分支去掉 `op == "Mul"`
     ⇒ 解释轨重新静默 ⇒ **「三轨对拍」层必须判红**。
  B  **同时**再让编译轨也静默（`runtime.c` 的 `px_sub` 加一个 str×int 直返分支）
     ⇒ 三轨响亮性**一致**（都 VAL）⇒ 对拍层**变绿** ⇒
        **「期望值（MODEL）判据」必须独立判红** —— 这正面证明「三轨一致 ≠ 正确」。
  C  判据自伤：在 A+B 在位时把 MODEL 核对也短路 ⇒ 必须**不再红**
     （证明「红」来自比对本身，而不是别的副作用）。

⚠️ 三个负控都要求**源逐字节还原**（快照 + cmp）。
"""
import argparse, os, shutil, subprocess, sys

ROOT = None
SNAP = None
FILES = ["selfhost/ival.px", "runtime/runtime.c"]

# 锚点必须**唯一命中**（不许猜次数 —— M223 教训）
ANCHOR_A = 'if op == "Mul" and tl == "string" and tr == "int":'
ANCHOR_B = '    px_error("R1002: 无法相减: %s - %s", px_type_name(a), px_type_name(b));'
PATCH_B = '''    /* M231-NEGCTL-B：构造「编译轨也静默」—— 期望值判据必须独立判红 */
    if (a.type == PX_STR && b.type == PX_INT) { return a; }
'''


def cnt(path, s):
    return open(os.path.join(ROOT, path), encoding="utf-8").read().count(s)


def sub(path, old, new, expect=1):
    p = os.path.join(ROOT, path)
    s = open(p, encoding="utf-8").read()
    n = s.count(old)
    if n != expect:
        raise SystemExit("锚点命中 %d 次（期望 %d）：%s ... %s" % (n, expect, path, old[:50]))
    open(p, "w", encoding="utf-8").write(s.replace(old, new))
    return n


def snapshot():
    os.makedirs(SNAP, exist_ok=True)
    for f in FILES:
        d = os.path.join(SNAP, f.replace("/", "__"))
        shutil.copyfile(os.path.join(ROOT, f), d)
    print("SNAP-OK %s" % SNAP)


def restore():
    bad = []
    for f in FILES:
        d = os.path.join(SNAP, f.replace("/", "__"))
        if not os.path.exists(d):
            continue
        cur = open(os.path.join(ROOT, f), "rb").read()
        old = open(d, "rb").read()
        if cur != old:
            open(os.path.join(ROOT, f), "wb").write(old)
            bad.append(f)
    print("RESTORE-OK%s" % ("（还原了 %s）" % ",".join(bad) if bad else "（无需还原）"))


def selftest():
    checks = [
        ("A 锚点唯一", "selfhost/ival.px", ANCHOR_A, 1),
        ("B 锚点唯一", "runtime/runtime.c", ANCHOR_B, 1),
        ("B 补丁未残留", "runtime/runtime.c", "M231-NEGCTL-B", 0),
        ("A 补丁未残留", "selfhost/ival.px", 'M231-NEGCTL-A', 0),
    ]
    bad = 0
    for name, path, s, exp in checks:
        n = cnt(path, s)
        ok = (n == exp)
        print("  %s %-16s 命中 %d（期望 %d）" % ("✅" if ok else "❌", name, n, exp))
        bad += 0 if ok else 1
    if bad:
        print("M231-NEGCTL-SELFTEST-FAIL")
        return 1
    print("M231-NEGCTL-SELFTEST-OK")
    return 0


def apply_a():
    sub("selfhost/ival.px", ANCHOR_A, 'if tl == "string" and tr == "int":  # M231-NEGCTL-A')


def apply_b():
    sub("runtime/runtime.c", ANCHOR_B, PATCH_B + ANCHOR_B)


def main():
    global ROOT, SNAP
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", required=True)
    ap.add_argument("--snap", required=True)
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--snapshot", action="store_true")
    ap.add_argument("--restore", action="store_true")
    ap.add_argument("--apply", choices=["A", "B"])
    a = ap.parse_args()
    ROOT, SNAP = os.path.abspath(a.root), a.snap
    if a.selftest:
        return selftest()
    if a.snapshot:
        snapshot(); return 0
    if a.restore:
        restore(); return 0
    if a.apply == "A":
        apply_a(); print("NEGCTL-A-APPLIED"); return 0
    if a.apply == "B":
        apply_b(); print("NEGCTL-B-APPLIED"); return 0
    return 2


sys.exit(main())
