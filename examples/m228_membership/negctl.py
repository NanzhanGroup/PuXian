#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M228 门 · 负控补丁器（忠实撤回 —— 不是"另造一个错"，M220 教训）

A · runtime/runtime.c 的 `px_membership_op`：**去掉取反** ⇒ `not in` 与 `in` 同值
B · selfhost/ival.px 的 `i_membership`：**去掉 `not in` 取反** ⇒ 解释轨与编译轨分叉
C · 判据自伤（在 three_doors.py 的 `--harm`，本器不管）

纪律（M223/M226）：
  · 锚点**唯一性断言**（不猜次数）
  · 施加后必须**真的变了**（cmp 不同）
  · `--restore` 用 `cmp` 逐字节核验还原
  · 每道负控**各自独立**：先 restore 再 apply（M165 教训：备份表被清空 ⇒ 改源残留）
"""
import argparse
import os
import shutil
import sys

PATCHES = {
    "A": ("runtime/runtime.c",
          "    return px_bool(negate ? (r != PX_MEM_FOUND) : (r == PX_MEM_FOUND));\n",
          "    return px_bool(r == PX_MEM_FOUND);  // NEGCTL-228A\n"),
    "B": ("selfhost/ival.px",
          '    if opname == "not in":\n        return Ok(not hit)\n    return Ok(hit)\n',
          "    return Ok(hit)  # NEGCTL-228B\n"),
}

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


def do_selftest():
    ok = True
    for k, (rel, old, _new) in sorted(PATCHES.items()):
        s = load(rel)
        n = s.count(old)
        print(f"  锚点 {k} ({rel}): 命中 {n} 次")
        if n != 1:
            print(f"  ❌ 锚点 {k} 不唯一（期望恰 1）")
            ok = False
        if _new.strip() in s:
            print(f"  ❌ 补丁 {k} 已在位（源不干净）")
            ok = False
    print("M228-NEG-SELFTEST-OK" if ok else "M228-NEG-SELFTEST-FAIL")
    return 0 if ok else 1


def do_apply(k):
    rel, old, new = PATCHES[k]
    store(rel)
    s = load(rel)
    if s.count(old) != 1:
        print(f"❌ 锚点 {k} 不唯一（{s.count(old)}）")
        return 2
    if new.strip() in s:
        print(f"❌ 补丁 {k} 已在位")
        return 2
    open(os.path.join(a.root, rel), "w", encoding="utf-8").write(s.replace(old, new))
    assert load(rel) != s, "补丁后内容未变"
    print(f"✅ 已施加负控 {k}（{rel}）")
    return 0


def do_restore():
    rc = 0
    for k, (rel, _o, _n) in sorted(PATCHES.items()):
        src = os.path.join(a.snap, rel)
        if not os.path.exists(src):
            continue
        dst = os.path.join(a.root, rel)
        cur = open(dst, "rb").read()
        ref = open(src, "rb").read()
        if cur != ref:
            shutil.copy2(src, dst)
            if open(dst, "rb").read() != ref:
                print(f"❌ 还原失败 {rel}")
                rc = 1
            else:
                print(f"✔  已还原 {rel}（逐字节一致）")
        else:
            print(f"✔  {rel} 本就一致")
    return rc


if a.selftest:
    sys.exit(do_selftest())
if a.restore:
    sys.exit(do_restore())
if a.apply:
    sys.exit(do_apply(a.apply))
print("用法：--selftest | --apply A|B | --restore")
sys.exit(2)
