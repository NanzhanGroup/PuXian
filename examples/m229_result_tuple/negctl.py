#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M229 负控补丁器（忠实撤回 · 各自独立）

A · runtime.c：撤回 `ok`/`err` 的精确 arity（回到「完全不查」= 静默）⇒ H1 必红
B · runtime.c：撤回措辞统一（`方法 is_ok 不接受参数` → `is_ok 不接受参数`）⇒ H2 必红
C · 判据自伤（gen_probes.py 的 `--harm`，本器不管）

纪律（M223/M226/M228）：锚点**唯一性**断言 · 施加后必须真的变了 · 还原用 cmp 逐字节核验 ·
每道负控先 restore 再 apply（避免改源残留）。
"""
import argparse
import os
import shutil
import sys

PATCHES = {
    "A": ("runtime/runtime.c",
          '            if (nargs != 0) px_error("R1005: 方法 ok 不接受参数");\n',
          "            // NEGCTL-229A：撤回 ok 的 arity 检查（回到静默）\n"),
    "B": ("runtime/runtime.c",
          '            if (nargs != 0) px_error("R1005: 方法 is_ok 不接受参数");\n',
          '            if (nargs != 0) px_error("R1005: is_ok 不接受参数");  // NEGCTL-229B\n'),
}
# A 还要撤掉 err 的（两处一起，才是「同族放行」的完整形状）
EXTRA_A = ("runtime/runtime.c",
           '            if (nargs != 0) px_error("R1005: 方法 err 不接受参数");\n',
           "            // NEGCTL-229A：撤回 err 的 arity 检查\n")

ap = argparse.ArgumentParser()
ap.add_argument("--root", required=True)
ap.add_argument("--snap", required=True)
ap.add_argument("--apply", choices=sorted(PATCHES))
ap.add_argument("--restore", action="store_true")
ap.add_argument("--selftest", action="store_true")
a = ap.parse_args()


def load(rel):
    return open(os.path.join(a.root, rel), encoding="utf-8").read()


def store(rel):
    dst = os.path.join(a.snap, rel)
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    if not os.path.exists(dst):
        shutil.copy2(os.path.join(a.root, rel), dst)


def edits_for(k):
    rel, old, new = PATCHES[k]
    out = [(rel, old, new)]
    if k == "A":
        out.append(EXTRA_A)
    return out


def do_selftest():
    ok = True
    for k in sorted(PATCHES):
        for rel, old, new in edits_for(k):
            n = load(rel).count(old)
            print(f"  锚点 {k} ({rel}): 命中 {n} 次")
            if n != 1:
                print(f"  ❌ 锚点 {k} 不唯一（期望恰 1）")
                ok = False
    print("M229-NEG-SELFTEST-OK" if ok else "M229-NEG-SELFTEST-FAIL")
    return 0 if ok else 1


def do_apply(k):
    for rel, old, new in edits_for(k):
        store(rel)
        s = load(rel)
        if s.count(old) != 1:
            print(f"❌ 锚点不唯一 {k} ({rel}): {s.count(old)}")
            return 2
        open(os.path.join(a.root, rel), "w", encoding="utf-8").write(s.replace(old, new, 1))
        assert load(rel) != s, "补丁后未变"
    print(f"✅ 已施加负控 {k}")
    return 0


def do_restore():
    rc = 0
    for k in sorted(PATCHES):
        for rel, _o, _n in edits_for(k):
            src = os.path.join(a.snap, rel)
            if not os.path.exists(src):
                continue
            dst = os.path.join(a.root, rel)
            ref = open(src, "rb").read()
            if open(dst, "rb").read() != ref:
                shutil.copy2(src, dst)
                print(("✔  已还原 " if open(dst, "rb").read() == ref else "❌ 还原失败 ") + rel)
                rc |= (0 if open(dst, "rb").read() == ref else 1)
    return rc


if a.selftest:
    sys.exit(do_selftest())
if a.restore:
    sys.exit(do_restore())
if a.apply:
    sys.exit(do_apply(a.apply))
print("用法：--selftest | --apply A|B | --restore")
sys.exit(2)
