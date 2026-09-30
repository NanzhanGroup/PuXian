_B_DATA = 'static const char* bdata(LXValue v) {\n    // M233（缺陷 349）：兜底改走与 `val_cstr` **同一个**「任意值 → 文本」入口\n    //   （修前是 `snprintf("%s", fmt_num(v))` ⇒ 非数值实参读 union 垃圾）。\n    return px_cstr_any(v, NULL);\n}\n'
_BLEN = 'static int blen(LXValue v) {\n    // M233（缺陷 349）：修前这里**二次渲染**（`strlen(bdata(v))` ⇒ 又占一个 tmp 槽，\n    //   且两次渲染取的是不同槽）—— 现在按**显式长度**返回，与 `bdata` 同源同值。\n    //   ⚠️ 刻意**不占** `g_cstr_ring` 槽（渲染完立即释放）：`bdata(x); blen(x);` 成对出现，\n    //      若 `blen` 也占槽会白白把环的预算减半（8 槽预算要留给「一次调用里多个实参」）。\n    if (v.type == PX_STR || v.type == PX_BYTES) return v.as.obj->as.str.len;\n    if (v.type == PX_NULL) return 4;\n    if (v.type == PX_BOOL) return v.as.b ? 4 : 5;\n    if (v.type == PX_INT || v.type == PX_FLOAT) {\n        int n = 0;\n        (void)px_cstr_any(v, &n);\n        return n;\n    }\n    int n = 0;\n    char* s = px_fmt_value_n(v, &n);\n    xfree(s);\n    return n;\n}\n'
_LEG_DATA = 'static const char* bdata(LXValue v) {\n    if (v.type == PX_STR || v.type == PX_BYTES) return v.as.obj->as.str.data;\n    char* tmp = px_tmp_slot();\n    snprintf(tmp, PX_TMPSZ, "%s", fmt_num(v));\n    return tmp;\n}\n'
_LEG_BLEN = 'static int blen(LXValue v) {\n    if (v.type == PX_STR || v.type == PX_BYTES) return v.as.obj->as.str.len;\n    return (int)strlen(bdata(v));\n}\n'

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M233 门 · 负控打桩库（锚点从补丁定义**机械导出**）

三组补丁（各自证明**不同判据层**有牙）
------------------------------------
· **A**（忠实撤回 `runtime.c` 的兜底修复）
    `val_cstr` / `bdata` 退回 `snprintf(tmp, PX_TMPSZ, "%s", fmt_num(v))`
    ⇒ ① **确定性**判据必须红（读 union 垃圾 ⇒ 同一二进制连跑两次结果不同）
      ② **对齐**判据（`f(x)` ⇄ `f(str(x))`）与 ③ 三轨对拍也必须红。
    为控制成本，本组只改 `val_cstr`（`bdata` 同族、判据同层，避免重复重建）。
· **B**（忠实撤回解释轨的标记值规范化）
    只把 `i_cstr_arg(args[0])` 退回 `args[0]`（runtime 侧的修复**保持**）
    ⇒ **对齐**判据在 `enum` / `struct` / `function` 三类型上必须红（15 例），
      而**确定性**判据仍绿 ⇒ 证明两组判据**各自独立**、抓的不是同一件事。
· **C**（**判据自伤**）
    在 B 的补丁在位时关掉对齐与 MODEL 层（`three_tracks.py --harm`）⇒ B 的红**必须消失**。

用法
----
  negctl.py --root R --snap DIR snap | apply A|B | restore | check | anchors
"""
import argparse, os, shutil, sys

FILES = ["runtime/runtime.c", "selfhost/ibuiltin.px"]
UNDO_FILES = {                       # 每个补丁要还原的文件（A 只涉及 runtime.c）
    "A": ["runtime/runtime.c"],
    "B": ["selfhost/ibuiltin.px"],
}

_BODY_A = ('static const char* val_cstr(LXValue v) {\n'
           '    // M129（Issue 87 缺陷 35）：null 必须字符串化为 "null"（`str(null)` 一致）——\n'
           '    //   现由 `px_cstr_any` 统一保证；M233 起该保证覆盖**全部**类型，不再只覆盖 null。\n'
           '    return px_cstr_any(v, NULL);\n'
           '}\n')
_LEG_A = ('static const char* val_cstr(LXValue v) {\n'
          '    if (v.type == PX_STR) return v.as.obj->as.str.data;\n'
          '    if (v.type == PX_NULL) return "null";\n'
          '    char* tmp = px_tmp_slot();\n'
          '    snprintf(tmp, PX_TMPSZ, "%s", fmt_num(v));\n'
          '    return tmp;\n'
          '}\n')
# ⚠️ **M234 连带更新（第 112 轮）**：M234 又给 `bytes_base64` / `bytes_concat` 加了
#   `i_cstr_arg` 调用 ⇒ 原来「一把替换全部 i_cstr_arg(args[0])」的锚点命中数从 5 变 7
#   （本门报「锚点不唯一」）。按纪律「**判据跟着结构走**」：改成**逐个点名** M233 引入的
#   五个站点（精确到函数名）⇒ 与 M234 的新增互不干扰。
_B_SITES = ["sha256", "md5", "base64_encode", "ord", "bytes_to_hex"]

# 每条 = (文件, 旧文本, 新文本, 期望命中次数)
PATCHES = {
    "A": [("runtime/runtime.c", _BODY_A, _LEG_A, 1),
          ("runtime/runtime.c", _B_DATA, _LEG_DATA, 1),
          ("runtime/runtime.c", _BLEN, _LEG_BLEN, 1)],
    "B": [("selfhost/ibuiltin.px", "%s(i_cstr_arg(args[0]))" % f,
           "%s(args[0])" % f, 1) for f in _B_SITES],
}
ANCHORS = {                          # 必须命中断言次数的锚点
    "A": [("runtime/runtime.c", _BODY_A, 1),
          ("runtime/runtime.c", _B_DATA, 1),
          ("runtime/runtime.c", _BLEN, 1)],
    "B": [("selfhost/ibuiltin.px", "%s(i_cstr_arg(args[0]))" % f, 1) for f in _B_SITES],
}


def path(root, rel):
    return os.path.join(root, rel)


def do_snap(root, snap):
    os.makedirs(snap, exist_ok=True)
    for rel in FILES:
        dst = os.path.join(snap, rel.replace("/", "__"))
        if not os.path.exists(dst):
            shutil.copy2(path(root, rel), dst)
    return 0


def do_restore(root, snap, only=None):
    for rel in (only or FILES):
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
        if open(src, "rb").read() != open(path(root, rel), "rb").read():
            print("❌ 未还原: %s" % rel)
            bad += 1
    print("M233-NEG-RESTORED-OK" if bad == 0 else "M233-NEG-RESTORED-FAIL")
    return 0 if bad == 0 else 1


def do_apply(root, snap, name):
    do_snap(root, snap)
    if name not in PATCHES:
        print("❌ 未知补丁: %s" % name)
        return 2
    do_restore(root, snap, UNDO_FILES[name])       # 幂等起点
    for rel, old, new, want in PATCHES[name]:
        p = path(root, rel)
        s = open(p, encoding="utf-8").read()
        n = s.count(old)
        if n != want:
            print("❌ 锚点命中数不符: %r 在 %s 命中 %d 次（期望 %d）" % (old[:40], rel, n, want))
            do_restore(root, snap, UNDO_FILES[name])
            return 3
        open(p, "w", encoding="utf-8").write(s.replace(old, new))
    print("M233-NEG-APPLY-%s-OK" % name)
    return 0


def do_anchors(root, snap):
    """导出锚点清单：每条必须命中**声明次数**（改源码后旧锚点会整体失效）"""
    bad = 0
    for name, items in sorted(ANCHORS.items()):
        for rel, blob, want in items:
            hits = open(path(root, rel), encoding="utf-8").read().count(blob)
            tag = blob.strip().split("\n")[0][:48]
            ok = (hits == want)
            print("  %s ×%d(=期望 %d)  [%s] %s" % ("ok " if ok else "BAD", hits, want, name, tag))
            if not ok:
                bad += 1
    print("M233-ANCHORS-OK" if bad == 0 else "M233-ANCHORS-FAIL")
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
