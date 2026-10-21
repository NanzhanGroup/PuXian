#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M317 门 · 负控打桩器（正向 ⇄ 撤回，逐字节可逆）

纪律（M285 A4）：每次替换断言**恰 1 处**命中；命中数不符即拒绝写入。
纪律（M294）：负控必须**忠实** —— 撤回后的源码要与 M317 之前**逐字节**一致（--restore 用备份还原）。
纪律（M289）：负控生效前必须**自证不是空操作** —— 撤了什么必须能在行为上看见（见 verify.sh [7.A1]/[7.B1]）。

本轮实证（首版教训）：本缺陷的修复有**两条触发路径** ——
  ① `gc_register` 的 `need`（有分配时）② `px_gc_poll` 的空闲推进（无分配时）。
首版负控 A 只撤 ①，结果「开启档」照旧被 ② 驱动（实测 time_runs=9）⇒ **负控没有复现缺陷**。
忠实撤回 = 回到「只有对象数 / 字节阈值」的旧行为 ⇒ 两条都必须撤。
"""
import sys
import shutil
import os

P = "/data/code/puxian/runtime/runtime.c"
BAK = "/tmp/m317_negctl_runtime.c.bak"

# ── 负控 A-1：gc_register 里的「时间 + 增量」判据 ────────────────────────────
NEED_FIX = """    // M317（缺陷 551）：时间 + 增量判据（与上面两条并列，只决定「何时」，不改回收语义）。
    //   持 g_gc_mu 内 ⇒ g_gc_time_check_at 无需原子；时钟只在每分配 256KiB 后取一次。
    if (!need && g_gc_max_interval_ms > 0 && g_alloc_bytes >= g_gc_time_floor &&
        g_alloc_bytes >= g_gc_time_check_at) {"""

NEED_OLD = """    // NEGCTL-317A：撤掉时间 + 增量判据（回到「只有对象数 / 字节阈值」）
    if (0) {"""

# ── 负控 A-2：px_gc_poll 里的时间 / 空闲触发 ────────────────────────────────
POLL_FIX = """        if (g_gc_max_interval_ms <= 0) return;
        int idle = __atomic_load_n(&g_gc_idle_reclaim, __ATOMIC_RELAXED);"""

POLL_OLD = """        if (1) return;   /* NEGCTL-317A：撤时间/空闲触发（回到只靠阈值的旧行为） */
        int idle = __atomic_load_n(&g_gc_idle_reclaim, __ATOMIC_RELAXED);"""

# ── 负控 B：撤掉空闲 GC 心跳线程（px_gc_tick_start 直接返回 ⇒ 不建线程）──────
BEAT_FIX = """    if (__atomic_exchange_n(&g_gc_tick_started, 1, __ATOMIC_SEQ_CST)) return;
    pthread_t t;"""

BEAT_OLD = """    if (__atomic_exchange_n(&g_gc_tick_started, 1, __ATOMIC_SEQ_CST)) return;
    if (1) return;   /* NEGCTL-317B：撤心跳（不建线程） */
    pthread_t t;"""

RULES = {
    "A": [(NEED_FIX, NEED_OLD), (POLL_FIX, POLL_OLD)],
    "B": [(BEAT_FIX, BEAT_OLD)],
}


def die(msg):
    sys.stderr.write("NEGCTL-ERROR: " + msg + "\n")
    sys.exit(1)


def do_apply(which):
    if which not in RULES:
        die("未知负控 %s（A=时间/空闲判据 · B=空闲心跳线程）" % which)
    parts = RULES[which]
    s = open(P, encoding="utf-8").read()
    for _new, old in parts:
        if old in s:
            die("负控 %s 已在位（拒绝重复打桩）" % which)
    if not os.path.exists(BAK):
        shutil.copy2(P, BAK)
    total = 0
    for new, old in parts:
        n = s.count(new)
        if n != 1:
            die("锚点命中数 = %d（要求恰 1）：%s" % (n, new.splitlines()[0]))
        s = s.replace(new, old)
        total += n
    open(P, "w", encoding="utf-8").write(s)
    print("NEGCTL-APPLIED %s（%d 处）" % (which, total))


def do_restore():
    if not os.path.exists(BAK):
        print("NEGCTL-RESTORE none（无备份，等价于从未打桩）")
        return
    shutil.copy2(BAK, P)
    os.unlink(BAK)
    print("NEGCTL-RESTORED")


def do_selftest():
    """自证：锚点在场且唯一（不打桩）· 拒绝重复打桩 · 还原后逐字节等于原文。"""
    s = open(P, encoding="utf-8").read()
    ok = 0
    fail = 0
    for k, parts in RULES.items():
        good = True
        for new, old in parts:
            c = s.count(new)
            if c != 1 or old in s:
                print("  ❌ 锚点 %s 异常：new=%d old_present=%s" % (k, c, old in s))
                good = False
        if good:
            print("  ✅ 锚点 %s 在场且唯一（%d 段）" % (k, len(parts)))
            ok += 1
        else:
            fail += 1
    # A 打桩 → 拒绝第二次 → 还原
    do_apply("A")
    try:
        do_apply("A")
        print("  ❌ A 可重复打桩（应拒绝）")
        fail += 1
    except SystemExit:
        print("  ✅ A 拒绝重复打桩")
        ok += 1
    do_restore()
    again = open(P, encoding="utf-8").read()
    if again == s:
        print("  ✅ 还原逐字节一致")
        ok += 1
    else:
        print("  ❌ 还原后与原文不一致")
        fail += 1
    # B 单独自证（不同锚点）
    do_apply("B")
    mid = open(P, encoding="utf-8").read()
    if "NEGCTL-317B" in mid:
        print("  ✅ B 打桩命中")
        ok += 1
    else:
        print("  ❌ B 打桩未命中")
        fail += 1
    do_restore()
    print("NEGCTL-SELFTEST ok=%d fail=%d" % (ok, fail))
    sys.exit(0 if fail == 0 else 1)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        die("用法：negctl.py --apply A|B | --restore | --selftest")
    a = sys.argv[1]
    if a == "--selftest":
        do_selftest()
    elif a == "--restore":
        do_restore()
    elif a == "--apply":
        do_apply(sys.argv[2])
    else:
        die("未知参数 " + a)
