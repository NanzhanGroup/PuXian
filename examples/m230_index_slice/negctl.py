#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M230 门 · 负控驱动器（锚点库由本文件机械导出，不许手抄 —— M161/M164/M226 的纪律）

缺陷 345 的修复面 = **三个文件**（这正是本轮最值钱的一条）：
  runtime/runtime.c  6 处（px_index list/tuple/str/bytes · px_index_set · px_iter_at · bytes_get/set）
  runtime/vm.c       2 处（PXOP_INDEX / PXOP_ITERAT 的 LIST[INT] **快路径**）
  selfhost/ival.px   1 处（解释轨 i_as_index）
⇒ 只修 runtime.c 会让 **VM 轨单独报错值**（实测：三轨 12 项差异 → 修 runtime.c 后收敛到 2 项
  且只剩 VM 轨）—— 与 M215 缺陷 305「VM 轨有自己的一份快路径实现」同族。

用法
  negctl.py --snap S --snapshot / --restore / --selftest
  negctl.py --root R --snap S --apply A|B
"""
import argparse, os, shutil, sys

RT, VM, IV = "runtime/runtime.c", "runtime/vm.c", "selfhost/ival.px"
FILES = (RT, VM, IV)

# A · runtime.c（6 条，共 8 处）
A_RT = [
    ('''        int m230_raw = i;   // M230（缺陷 345）：消息一律报**用户输入的原值**
        int len = obj.as.obj->as.list.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) px_error("R1003: 索引越界: %d (len=%d)", m230_raw, len);''',
     '''        int len = obj.as.obj->as.list.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) px_error("R1003: 索引越界: %d (len=%d)", i, len);''', 2),
    ('''        int m230_raw = i;   // M230（缺陷 345）：消息一律报**用户输入的原值**
        int len = obj.as.obj->as.tuple.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) px_error("R1003: 索引越界: %d (len=%d)", m230_raw, len);''',
     '''        int len = obj.as.obj->as.tuple.len;
        if (i < 0) i += len;
        if (i < 0 || i >= len) px_error("R1003: 索引越界: %d (len=%d)", i, len);''', 1),
    ('''        int m230_raw = i;   // M230（缺陷 345）：消息一律报**用户输入的原值**
        if (i < 0) i += ulen;
        if (i < 0 || i >= ulen) px_error("R1003: 索引越界: %d (len=%d)", m230_raw, ulen);''',
     '''        if (i < 0) i += ulen;
        if (i < 0 || i >= ulen) px_error("R1003: 索引越界: %d (len=%d)", i, ulen);''', 1),
    ('''        int m230_raw = i;   // M230（缺陷 345）：消息一律报**用户输入的原值**
        if (i < 0) i += blen;
        if (i < 0 || i >= blen) px_error("R1003: 索引越界: %d (len=%d)", m230_raw, blen);''',
     '''        if (i < 0) i += blen;
        if (i < 0 || i >= blen) px_error("R1003: 索引越界: %d (len=%d)", i, blen);''', 1),
    ('''        int m230_raw = i;   // M230（缺陷 345）：同口径（迭代位置语义亦然）
        if (i < 0) i += o->as.dict.len;
        if (i >= 0 && i < o->as.dict.len)
            return px_str(o->as.dict.keys[i]);
        px_error("R1003: 索引越界: %d (len=%d)", m230_raw, o->as.dict.len);''',
     '''        if (i < 0) i += o->as.dict.len;
        if (i >= 0 && i < o->as.dict.len)
            return px_str(o->as.dict.keys[i]);
        px_error("R1003: 索引越界: %d (len=%d)", i, o->as.dict.len);''', 1),
    ('''    int m230_raw = (int)i;   // M230（缺陷 345）：消息一律报**用户输入的原值**
    if (idx < 0) idx += len;
    if (idx < 0 || idx >= len) px_error("R1003: 索引越界: %d (len=%d)", m230_raw, len);''',
     '''    if (idx < 0) idx += len;
    if (idx < 0 || idx >= len) px_error("R1003: 索引越界: %d (len=%d)", (int)idx, len);''', 2),
]

# A · vm.c（1 条，2 处：PXOP_INDEX + PXOP_ITERAT）
A_VM = [
    ('''                int m230_raw = i;   // M230（缺陷 345）：消息一律报**用户输入的原值**
                int len = obj.as.obj->as.list.len;
                if (i < 0) i += len;
                if (i < 0 || i >= len) px_error("R1003: 索引越界: %d (len=%d)", m230_raw, len);''',
     '''                int len = obj.as.obj->as.list.len;
                if (i < 0) i += len;
                if (i < 0 || i >= len) px_error("R1003: 索引越界: %d (len=%d)", i, len);''', 2),
]

# B · ival.px（解释轨）
B_IV = [
    ('''        var m230_raw = idx
        var i = idx
        if i < 0:
            i = i + n
        if i < 0 or i >= n:
            return Err(i_r1003("索引越界: " + i_to_str(m230_raw) + " (len=" + i_to_str(n) + ")", pos))''',
     '''        var i = idx
        if i < 0:
            i = i + n
            if i < 0:
                return Err(i_r1003("索引越界: " + i_to_str(i) + " (len=" + i_to_str(n) + ")", pos))
        if i >= n:
            return Err(i_r1003("索引越界: " + i_to_str(i) + " (len=" + i_to_str(n) + ")", pos))''', 1),
]

SPECS = {"A": [(RT, A_RT), (VM, A_VM)], "B": [(IV, B_IV)]}
N_ANCHORS = sum(len(p) for v in SPECS.values() for _, p in v)


def snapshot(root, snap):
    os.makedirs(snap, exist_ok=True)
    for rel in FILES:
        shutil.copy2(os.path.join(root, rel), os.path.join(snap, rel.replace("/", "__")))
    print("快照 %d 件 → %s" % (len(FILES), snap))


def restore(root, snap):
    for rel in FILES:
        src = os.path.join(snap, rel.replace("/", "__"))
        if os.path.exists(src):
            shutil.copy2(src, os.path.join(root, rel))
    print("已从快照还原 %d 件" % len(FILES))


def apply(root, which):
    for rel, pairs in SPECS[which]:
        path = os.path.join(root, rel)
        s = open(path, encoding="utf-8").read()
        for old, new, want in pairs:
            n = s.count(old)
            if n != want:
                print("❌ 负控 %s [%s] 锚点命中 %d 处，期望 %d 处" % (which, rel, n, want))
                return 1
            s = s.replace(old, new)
        open(path, "w", encoding="utf-8").write(s)
        print("  撤回 %s" % rel)
    print("负控 %s 已施加" % which)
    return 0


def selftest(root, snap):
    bad = 0
    for which, items in SPECS.items():
        for rel, pairs in items:
            s = open(os.path.join(root, rel), encoding="utf-8").read()
            for i, (old, new, want) in enumerate(pairs):
                n = s.count(old)
                if n != want:
                    print("❌ 锚点 %s[%s][%d] 命中 %d 处（期望 %d）" % (which, rel, i, n, want))
                    bad += 1
    if bad == 0:
        print("NEGCTL-SELFTEST-OK（%d 条锚点全部唯一命中）" % N_ANCHORS)
        return 0
    print("NEGCTL-SELFTEST-FAIL（%d 处异常）" % bad)
    return 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--snap", required=True)
    ap.add_argument("--snapshot", action="store_true")
    ap.add_argument("--restore", action="store_true")
    ap.add_argument("--apply", choices=["A", "B"])
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    if a.snapshot:
        snapshot(a.root, a.snap); return 0
    if a.restore:
        restore(a.root, a.snap); return 0
    if a.apply:
        return apply(a.root, a.apply)
    if a.selftest:
        return selftest(a.root, a.snap)
    ap.print_help()
    return 2


if __name__ == "__main__":
    sys.exit(main())
