#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M234 门 · 负控打桩库

· **A**（忠实撤回缺陷 352 的修复）
    摘掉 `px_type_name` 里那段**标记字典识别** ⇒ 解释轨的 struct/enum/function 又会被
    报成 `dict` ⇒ [3] 三轨对拍必须红（18 例 + 6 例同族）。
· **B**（**判据自伤**）在 A 在位时把三层判据全关 ⇒ A 的红必须消失。

用法： negctl.py --root R --snap DIR snap | apply A | restore | check | anchors
"""
import argparse, os, shutil, sys

FILES = ["runtime/runtime.c"]
_OLD = '''    if (v.type == PX_DICT) {
        if (px_dict_get(v, "__ufn__").type != PX_NULL) return "function";
        if (px_dict_get(v, "__builtin__").type != PX_NULL) return "native";
        if (px_dict_get(v, "__struct__").type != PX_NULL) return "struct";
        if (px_dict_get(v, "__enum__").type != PX_NULL) return "enum";
        if (px_dict_get(v, "__typeref__").type != PX_NULL) return "type";
        if (px_dict_get(v, "__gen__").type != PX_NULL) return "generator";
    }
'''
_NEW = ""
PATCHES = {"A": [("runtime/runtime.c", _OLD, _NEW, 1)]}
ANCHORS = {"A": [("runtime/runtime.c", _OLD, 1)]}


def path(root, rel):
    return os.path.join(root, rel)


def do_snap(root, snap):
    os.makedirs(snap, exist_ok=True)
    for rel in FILES:
        dst = os.path.join(snap, rel.replace("/", "__"))
        if not os.path.exists(dst):
            shutil.copy2(path(root, rel), dst)
    return 0


def do_restore(root, snap):
    for rel in FILES:
        src = os.path.join(snap, rel.replace("/", "__"))
        if not os.path.exists(src):
            print("❌ 备份缺失: %s" % src); return 1
        shutil.copy2(src, path(root, rel))
    return 0


def do_check(root, snap):
    bad = 0
    for rel in FILES:
        src = os.path.join(snap, rel.replace("/", "__"))
        if open(src, "rb").read() != open(path(root, rel), "rb").read():
            print("❌ 未还原: %s" % rel); bad += 1
    print("M234-NEG-RESTORED-OK" if bad == 0 else "M234-NEG-RESTORED-FAIL")
    return 0 if bad == 0 else 1


def do_apply(root, snap, name):
    do_snap(root, snap)
    if name not in PATCHES:
        print("❌ 未知补丁: %s" % name); return 2
    do_restore(root, snap)
    for rel, old, new, want in PATCHES[name]:
        p = path(root, rel)
        s = open(p, encoding="utf-8").read()
        n = s.count(old)
        if n != want:
            print("❌ 锚点命中数不符: 在 %s 命中 %d 次（期望 %d）" % (rel, n, want))
            do_restore(root, snap); return 3
        open(p, "w", encoding="utf-8").write(s.replace(old, new))
    print("M234-NEG-APPLY-%s-OK" % name)
    return 0


def do_anchors(root, snap):
    bad = 0
    for name, items in sorted(ANCHORS.items()):
        for rel, blob, want in items:
            hits = open(path(root, rel), encoding="utf-8").read().count(blob)
            ok = (hits == want)
            print("  %s ×%d(=期望 %d)  [%s]" % ("ok " if ok else "BAD", hits, want, name))
            if not ok:
                bad += 1
    print("M234-ANCHORS-OK" if bad == 0 else "M234-ANCHORS-FAIL")
    return 0 if bad == 0 else 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--snap", required=True)
    ap.add_argument("cmd", choices=["snap", "apply", "restore", "check", "anchors"])
    ap.add_argument("name", nargs="?", default=None)
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    if a.cmd == "snap":
        return do_snap(root, a.snap)
    if a.cmd == "restore":
        return do_restore(root, a.snap)
    if a.cmd == "check":
        return do_check(root, a.snap)
    if a.cmd == "anchors":
        return do_anchors(root, a.snap)
    return do_apply(root, a.snap, a.name)


sys.exit(main())
