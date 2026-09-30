#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M232 门 · 负控打桩库（锚点**从补丁定义机械导出**，不许手抄）

三组补丁（各自证明**不同判据层**有牙）
------------------------------------
· **A**（真值性表 · `tuple` 方向）
    忠实撤回 `runtime.c` 的 `PX_TUPLE` 分支 ⇒ 编译两轨退回 `default: return true`
    ⇒ **① 三轨对拍**层必须红，**② 逐轨 MODEL** 层也必须红。
· **B**（**构造出「三轨一致地错」** —— 本仓立论「三轨一致 ≠ 正确」的判据化）
    `runtime.c`：`PX_BYTES` 与 `PX_TUPLE` 两个 case 都撤回；
    `ival.px`：`bytes` 分支撤回（退回兜底 `true`）、`tuple` 分支改写为 `return true`。
    ⇒ 两个值在**三个实现里都恒为真**（一致地错）⇒ **① 变绿**，
      只有 **② 逐轨 MODEL** 与 **③ 缺陷 347 精确形状**两层能独立判红。
    ⚠️ 若判据只做「三轨对拍」，这一版补丁**永远看不出来**。
· **C**（**判据自伤**）
    在 B 的补丁在位时，把 ② 关掉（`three_tracks.py --harm`）
    ⇒ B 的红**必须消失** ⇒ 证明那红确实来自判据，不是噪声。

用法
----
  negctl.py --root R --snap DIR snap       # 打补丁前备份（幂等）
  negctl.py --root R --snap DIR apply A|B  # 应用补丁（锚点必须唯一）
  negctl.py --root R --snap DIR restore    # 从备份还原
  negctl.py --root R --snap DIR check      # 还原后逐字节复核
  negctl.py --root R --snap DIR anchors    # 导出补丁锚点清单（供静态判据自证）
"""
import argparse, os, shutil, sys

FILES = ["runtime/runtime.c", "selfhost/ival.px"]

_TP = "        case PX_TUPLE: return v.as.obj->as.tuple.len > 0;\n"
_BP = "        case PX_BYTES: return v.as.obj->as.str.len > 0;\n"
_IB = '    if t == "bytes":\n        return len(v) > 0\n'
_IT_OLD = '    if t == "tuple":\n        return len(v) > 0\n'
_IT_NEW = '    if t == "tuple":\n        return true\n'

PATCHES = {
    "A": [("runtime/runtime.c", _TP, "")],
    "B": [("runtime/runtime.c", _BP, ""),
          ("runtime/runtime.c", _TP, ""),
          ("selfhost/ival.px", _IB, ""),
          ("selfhost/ival.px", _IT_OLD, _IT_NEW)],
}

# 补丁用到的**前**文本（静态判据据此「自证锚点仍在源码里且唯一」）
ANCHORS = [_TP, _BP, _IB, _IT_OLD]
# **替换后**文本：在**干净源码**里必须**不出现**（反向判据）——
#   它若出现，说明「已经处在被打桩的状态」⇒ 后面的负控会「打不上桩」而假绿。
NEG_ANCHORS = [_IT_NEW]


def path(root, rel):
    return os.path.join(root, rel)


def do_snap(root, snap):
    os.makedirs(snap, exist_ok=True)
    for rel in FILES:
        dst = os.path.join(snap, rel.replace("/", "__"))
        if not os.path.exists(dst):          # 幂等：不覆盖既有备份
            shutil.copy2(path(root, rel), dst)
    return 0


def do_restore(root, snap):
    for rel in FILES:
        src = os.path.join(snap, rel.replace("/", "__"))
        if not os.path.exists(src):
            print("❌ 备份缺失: %s" % src)
            return 1
        shutil.copy2(src, path(root, rel))
    return 0


def do_check(root, snap):
    bad = 0
    for rel in FILES:
        src = os.path.join(snap, rel.replace("/", "__"))
        a = open(src, "rb").read()
        b = open(path(root, rel), "rb").read()
        if a != b:
            print("❌ 未还原: %s" % rel)
            bad += 1
    print("M232-NEG-RESTORED-OK" if bad == 0 else "M232-NEG-RESTORED-FAIL")
    return 0 if bad == 0 else 1


def do_apply(root, snap, name):
    do_snap(root, snap)
    if name not in PATCHES:
        print("❌ 未知补丁: %s" % name)
        return 2
    # 先把涉及的文件还原成「打补丁前」的干净态，再逐条替换（多次 apply 幂等）
    do_restore(root, snap)
    for rel, old, new in PATCHES[name]:
        p = path(root, rel)
        s = open(p, encoding="utf-8").read()
        n = s.count(old)
        if n != 1:
            print("❌ 锚点不唯一: %s 在 %s 命中 %d 次" % (old[:40].strip(), rel, n))
            do_restore(root, snap)
            return 3
        open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
    print("M232-NEG-APPLY-%s-OK" % name)
    return 0


def do_anchors(root, snap):
    """导出锚点清单：每条必须**恰好命中 1 次**（判据：改源码后旧锚点会失效）

    ⚠️ 加上**反向**判据：替换后文本在干净源码里必须**不出现**
    （出现了 = 已处于打桩态 ⇒ 负控「打不上桩」而**假绿**）。
    """
    bad = 0
    for a in ANCHORS:
        hits = 0
        for rel in FILES:
            s = open(path(root, rel), encoding="utf-8").read()
            hits += s.count(a)
        tag = a.strip().split("\n")[0][:52]
        if hits == 1:
            print("  ok   ×%d  %s" % (hits, tag))
        else:
            print("  BAD  ×%d  %s" % (hits, tag))
            bad += 1
    for a in NEG_ANCHORS:
        hits = 0
        for rel in FILES:
            s = open(path(root, rel), encoding="utf-8").read()
            hits += s.count(a)
        tag = a.strip().split("\n")[0][:52]
        if hits == 0:
            print("  ok   ×0(反向)  %s => %s" % (tag, a.strip().split("\n")[-1][:24]))
        else:
            print("  BAD  ×%d(反向, 应为 0)  %s" % (hits, tag))
            bad += 1
    print("M232-ANCHORS-OK" if bad == 0 else "M232-ANCHORS-FAIL")
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
