#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M237 门 · 负控补丁器（锚点唯一自证 + 忠实撤回 + 逐字节还原）

用法（verify.sh 调）：
  negctl.py --root R --snap W --selftest        锚点唯一性自证（每处恰 1 次命中）
  negctl.py --root R --snap W --snapshot        备份源码
  negctl.py --root R --snap W --neg-a           撤回 C 轨枚举构造校验（→ 静默假值回来）
  negctl.py --root R --snap W --neg-b           撤回解释轨枚举构造（→ R1004 回来）
  negctl.py --root R --snap W --restore         从快照逐字节还原 + cmp 校验

设计纪律（M235s1）：**撤回必须落在「生效处」**，且撤回后必须能**判红**；
还原必须**逐字节**（cmp），不能只看 git status。
"""
import argparse
import os
import shutil
import sys

# (相对路径, 原文, 新文, 标签) —— 每项**必须恰 1 次命中**（selftest 强制）
PATCHES = {
    "neg-a": [
        # C 轨：把「校验后的构造」退回「不校验的构造」⇒ `Color("Nope")` 静默造出假枚举值
        ("selfhost/cg_expr.px",
         '    var known = 0\n'
         '    if contains(cg_enums[cname], vn):\n'
         '        known = 1\n'
         '    return "px_enum_checked(\\"" + cname + "\\", \\"" + vn + "\\", " + str(known) + ")"',
         '    return "px_enum(\\"" + cname + "\\", \\"" + vn + "\\")"',
         "C 轨枚举构造去校验"),
        ("selfhost/cg_expr.px",
         '                var vk = 0\n'
         '                if contains(cg_enums[oname], fname):\n'
         '                    vk = 1\n'
         '                return "px_enum_checked(\\"" + oname + "\\", \\"" + fname + "\\", " + str(vk) + ")"',
         '                return "px_enum(\\"" + oname + "\\", \\"" + fname + "\\")"',
         "C 轨变体访问去校验"),
    ],
    "neg-b": [
        # 解释轨：撤回枚举构造拦截 ⇒ `Color("Red")` 报 R1004（修前行为）
        ("selfhost/iexpr.px",
         '        if g_enums.has(cname):\n'
         '            return i_enum_ctor(cname, args, pos)\n',
         '        if false:\n'
         '            return i_enum_ctor(cname, args, pos)\n',
         "解释轨枚举构造拦截失效"),
    ],
}


def snap_dir(args):
    return os.path.join(args.snap, "src")


def do_snapshot(args):
    d = snap_dir(args)
    if os.path.isdir(d):
        # M239s1（缺陷 394 同族）：**已有快照就不再重拍** —— 快照语义 = 「进门时的源」。
        #   原实现 `rmtree` + 重拍 ⇒ 若门里出现「先 apply X 再 apply Y」，第二次快照会记录
        #   **已被 X 污染**的源 ⇒ `restore` 回不到进门态（残留）。m239 门已实测踩中。
        print("SNAPSHOT-SKIP 已有进门快照（不可覆盖）")
        return
    files = set()
    for items in PATCHES.values():
        for rel, _o, _n, _t in items:
            files.add(rel)
    for rel in sorted(files):
        src = os.path.join(args.root, rel)
        dst = os.path.join(d, rel)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copy2(src, dst)
    print("SNAPSHOT-OK %d 文件" % len(files))


def do_restore(args):
    d = snap_dir(args)
    n = 0
    bad = 0
    for dirpath, _dn, fns in os.walk(d):
        for fn in fns:
            src = os.path.join(dirpath, fn)
            rel = os.path.relpath(src, d)
            dst = os.path.join(args.root, rel)
            shutil.copy2(src, dst)
            if open(src, "rb").read() != open(dst, "rb").read():
                print("RESTORE-DIFF " + rel)
                bad += 1
            n += 1
    print("RESTORE-OK %d 文件 cmp 逐字节一致（bad=%d）" % (n, bad))
    return 1 if bad else 0


def selftest(args):
    bad = 0
    tot = 0
    for g, items in PATCHES.items():
        for rel, old, _new, tag in items:
            s = open(os.path.join(args.root, rel)).read()
            c = s.count(old)
            tot += 1
            if c != 1:
                print("  ANCHOR-BAD %-14s %-30s 命中 %d 次" % (g, tag, c))
                bad += 1
    if bad:
        print("NEGCTL-SELFTEST-FAIL %d/%d" % (bad, tot))
        return 1
    print("NEGCTL-SELFTEST-OK 锚点 %d 处全部唯一命中" % tot)
    return 0


def apply(args, which):
    for rel, old, new, tag in PATCHES[which]:
        p = os.path.join(args.root, rel)
        s = open(p).read()
        c = s.count(old)
        if c != 1:
            print("APPLY-FAIL %s 命中 %d 次" % (tag, c))
            return 1
        open(p, "w").write(s.replace(old, new))
        print("  已撤回：%s" % tag)
    return 0


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default="/data/code/puxian")
    ap.add_argument("--snap", required=True)
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--snapshot", action="store_true")
    ap.add_argument("--restore", action="store_true")
    ap.add_argument("--neg-a", action="store_true")
    ap.add_argument("--neg-b", action="store_true")
    a = ap.parse_args()
    if a.selftest:
        sys.exit(selftest(a))
    if a.snapshot:
        do_snapshot(a)
    if a.neg_a:
        sys.exit(apply(a, "neg-a"))
    if a.neg_b:
        sys.exit(apply(a, "neg-b"))
    if a.restore:
        sys.exit(do_restore(a))
