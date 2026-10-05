#!/usr/bin/env bash
# ============================================================
# selfhost/gate_lock.sh —— **门级互斥锁**（M276 · 晨曦 2026-10-05 回馈）
# ------------------------------------------------------------
# ⚠️ 本文件是**被 source** 的，不单独执行：
#     . "$(dirname "$0")/../../selfhost/gate_lock.sh"    # 放在 verify.sh 开头
#
# 为什么需要（晨曦原话：「门有并发不安全性（**就地改仓库**，建议加锁或 mktemp 副本）」）：
#   本仓 **93 个门**会在负控里现场改源码（`restore_all` / 负控打桩 / `run_neg` …）。
#   已有的护栏只盖住一半 ——
#
#     | 场景                              | M276 之前 |
#     |-----------------------------------|-----------|
#     | 两个**全量门**并发                 | ✅ `run_gates.sh` 的 PID 锁 |
#     | **人工单跑**一扇门 + 全量门在跑    | ❌ **无覆盖** |
#
#   实测代价（M191 / M213 / M221 / M223 各撞过一次）：
#     · 负控的 snapshot/restore **盖掉**未提交改动
#     · 门读到**别的门打的负控补丁**（中间态）⇒ 假红 / 假绿
#     · abort 时残留**负控打桩标记** ⇒ 被烘进入库件
#
# 语义：
#   · 上层已持锁（`run_gates.sh` 导出了 `PX_GATE_LOCK_HELD=1`）⇒ **直接返回**
#   · 拿到锁 ⇒ **导出 `PX_GATE_LOCK_HELD=1`** ⇒ 本门派生的子进程不再抢锁
#   · 拿不到锁 ⇒ **rc=2 响亮退出**（默认**不等待** —— 全量门要跑 ~2h，静默阻塞看起来就像挂死）
#     `PX_GATE_LOCK_WAIT=<秒>` 可改成轮询等待（opt-in）
#
# ── 为什么用「PID 文件」而不是 `flock`（本轮实测后改的设计）──────────
#   先写的版本是 `exec 201>lock; flock -n 201`。实测**两个致命问题**：
#     ① **锁会随 fd 泄漏给后代**：门里 `setsid nohup px_serve &` 那类**后台进程**继承 fd 201，
#        门退出后它仍持锁 ⇒ **下一扇门无端失败**（实测：父进程退出、`setsid sleep 20` 存活 ⇒ 锁仍被占）。
#        `flock` 是 open-file-description 级的，我们无法给 bash 的 fd 设 CLOEXEC。
#     ② `flock` 的释放只能靠**进程退出**或 `trap EXIT`，而 **`trap EXIT` 会盖掉门自己已有的
#        EXIT 陷阱**（大量门用它做 cleanup）⇒ 要么破坏门，要么泄漏。
#   ⇒ 改用**只认「持有者 PID 还活着吗」**的文件锁：后台子进程**不影响**它，
#     也不需要 `trap`（陈旧文件由**存活判据**自愈）。
#
# 环境变量：
#   PX_GATE_LOCK        锁文件（默认 /tmp/.px_gate_lock）
#   PX_GATE_LOCK_WAIT   轮询等待秒数（默认 0 = 立即失败）
#   PX_GATE_LOCK_HELD   由加锁方导出（值 1 ⇒ 跳过）
#
# 失败方向：**一律 fail-safe** —— 判定不了（抢不到 / 被覆盖 / /tmp 不可写）就 rc=2，
#   绝不「静默放行」。静默放行 = 没有保护却看起来有（本仓反复登记的「把红当绿」）。
#
# ⚠️ 已知取舍（如实登记）：锁是**全局单把**（不按仓库根分键）⇒ 两个 git worktree 会互相串行。
#   选它的理由：分键要 runner / gate 两处各算一遍，**算不一致就是静默失效**（没有保护还看不出来）。
#   宁可过度串行，不可静默失效。
# ============================================================

# ── 1) 上层已持锁 ⇒ 短路 ────────────────────────────────────
if [ "${PX_GATE_LOCK_HELD:-0}" = "1" ]; then
    return 0 2>/dev/null || exit 0
fi

_PX_GLF="${PX_GATE_LOCK:-/tmp/.px_gate_lock}"
_PX_GLW="${PX_GATE_LOCK_WAIT:-0}"

_px_gl_bail() {   # $1=原因行（多行用 \n）
    printf '❌ [gate_lock] %s\n' "$1" >&2
    printf '   ⇒ 本门在负控里会改源码（snapshot/restore），并发运行会让两边的还原互相盖掉\n' >&2
    printf '      （M191/M213/M221/M223 各撞过一次：假红、假绿、残留被烘进产物）。\n' >&2
    printf '   ⇒ 等它结束后重跑；或 `PX_GATE_LOCK_WAIT=7200` 轮询等待。\n' >&2
    printf '   ⇒ 锁文件：%s（全量门的 PID 另见 /tmp/.m116_gates.lock）\n' "$_PX_GLF" >&2
    exit 2
}

# 持有者「真的在跑门吗」——
#   只 `kill -0` 不够：PID 会被复用（陈旧锁 + 复用 ⇒ **所有门永久被挡**）。
#   故再核 cmdline 是否像门/全量门；读不到 /proc（非 Linux）⇒ 退回只看存活。
_px_gl_alive() {   # $1=pid  ⇒ rc 0=在跑门 1=不是/已死
    [ -n "$1" ] || return 1
    kill -0 "$1" 2>/dev/null || return 1
    if [ -r "/proc/$1/cmdline" ]; then
        tr '\0' ' ' < "/proc/$1/cmdline" 2>/dev/null \
            | grep -qE 'verify\.sh|run_gates\.sh|m116_gates\.sh' && return 0
        return 1
    fi
    return 0
}

# ── 2) 抢占（`set -C` = O_EXCL ⇒ 原子）──────────────────────
_px_gl_claim() {
    ( set -o noclobber; printf '%s\n' "$$" > "$_PX_GLF" ) 2>/dev/null
}

_px_gl_ok=0
_px_gl_deadline=$(( $(date +%s) + _PX_GLW ))
_px_gl_tries=0
while :; do
    _px_gl_tries=$((_px_gl_tries+1))
    if _px_gl_claim; then _px_gl_ok=1; break; fi
    _px_gl_own="$(cat "$_PX_GLF" 2>/dev/null || true)"
    if _px_gl_alive "$_px_gl_own" && [ "$_px_gl_own" != "$$" ]; then
        # 真的有人在跑门 ⇒ 等或失败
        if [ "$_PX_GLW" -gt 0 ] 2>/dev/null && [ "$(date +%s)" -lt "$_px_gl_deadline" ]; then
            sleep 1; continue
        fi
        _px_gl_bail "另一扇门 / 全量门正在跑（PID $_px_gl_own）—— **拒绝并发**。"
    fi
    # 陈旧（持有者已死 或 压根不是门进程）⇒ 清掉重试
    rm -f "$_PX_GLF" 2>/dev/null || true
    [ "$_px_gl_tries" -ge 8 ] && break
    sleep 0.02
done
[ "$_px_gl_ok" = "1" ] || _px_gl_bail "抢占锁失败（8 次重试仍被占）—— 疑似并发启动或 /tmp 异常。"

# ── 3) 抢占后复核（防「两扇门同时启动」的微竞态：A 建 → B 删 → B 建）──
sleep 0.03
_px_gl_now="$(cat "$_PX_GLF" 2>/dev/null || true)"
if [ "$_px_gl_now" != "$$" ]; then
    _px_gl_bail "抢占后被另一进程覆盖（检测到**并发启动**）—— 请只跑一扇门。"
fi

# ── 4) 交棒：让派生进程跳过 ─────────────────────────────────
export PX_GATE_LOCK_HELD=1
