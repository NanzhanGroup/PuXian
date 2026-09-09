#!/usr/bin/env bash
# ============================================================
# M93-S2 verify.sh —— 帧协程内核（M:N）验证门
# ------------------------------------------------------------
# 内容：
#   1) coro_print.px    64 纯计算协程全部执行（PX_CORO_DIAG done 计数 = 64）
#   2) coro_many.px     1000 纯计算协程全部完成（done = 1000）
#   3) thr_count.px     spawn 128 → 进程线程数 ≤ worker+1（远小于 spawn 数）
#   4) coro_gc.px       spawn 500 × 高频对象分配 + PX_GC_THRESHOLD=4000
#                       （多轮并发 GC STW × 协程帧槽精确根）→ 500/500 无崩溃
#   5) 逃生舱：--c build thr_count → 线程数 = spawn+1（pthread 语义零变化）
# 退出码：0=全 PASS；非 0=有失败。
# 前置：仓库 tools/px + 最新 rtcache（含 coro.o，跑过任一 px build）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m93_s2"
PASS=0 FAIL=0

chk() { # $1=名 $2=条件
    if [ "$2" = "0" ] || [ "$2" = "1" ]; then
        [ "$2" = "0" ] && { echo "  PASS $1"; PASS=$((PASS+1)); } || { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
    else
        echo "  FAIL $1 (条件异常: $2)"; FAIL=$((FAIL+1))
    fi
}

echo "── 1) 纯计算 spawn 64（全部执行 + 回收）"
"$PX" build "$DIR/coro_print.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_DIAG=1 timeout 20 "$DIR/build/coro_print" >/tmp/m93s2_1.out 2>/tmp/m93s2_1.diag
D=$(grep -c "done (f=" /tmp/m93s2_1.diag 2>/dev/null); S=$(grep -cE "^R[0-9]+=" /tmp/m93s2_1.out)
chk "coro_print 64 全部完成 (done=$D)" $([ "$D" = "64" ] && echo 0 || echo 1)
chk "coro_print 64 结果行 (R=$S)" $([ "$S" = "64" ] && echo 0 || echo 1)

echo "── 2) 纯计算 spawn 1000（全部完成）"
"$PX" build "$DIR/coro_many.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_DIAG=1 timeout 25 "$DIR/build/coro_many" >/tmp/m93s2_2.out 2>/tmp/m93s2_2.diag
D=$(grep -c "done (f=" /tmp/m93s2_2.diag 2>/dev/null)
chk "coro_many 1000 全部完成 (done=$D)" $([ "$D" = "1000" ] && echo 0 || echo 1)

echo "── 3) 线程数收敛（spawn 128 → 线程 ≤ 9 = worker+1）"
"$PX" build "$DIR/thr_count.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
"$DIR/build/thr_count" >/tmp/m93s2_3.out 2>&1 &
BP=$!
sleep 1.2
TP=$(pgrep -x thr_count | head -1)
[ -n "$TP" ] || TP=$BP
TH=$(grep Threads /proc/$TP/status 2>/dev/null | awk '{print $2}')
wait $BP
chk "thr_count 线程数=$TH (≤9)" $([ -n "$TH" ] && [ "$TH" -le 9 ] && echo 0 || echo 1)

echo "── 4) spawn 500 × 高频对象分配 × 并发 GC（PX_GC_THRESHOLD=4000）"
"$PX" build "$DIR/coro_gc.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_GC_THRESHOLD=4000 PX_CORO_DIAG=1 timeout 50 "$DIR/build/coro_gc" >/tmp/m93s2_4.out 2>/tmp/m93s2_4.diag
RC=$?
D=$(grep -c "done (f=" /tmp/m93s2_4.diag 2>/dev/null)
chk "coro_gc 500 完成 (done=$D rc=$RC)" $([ "$D" = "500" ] && [ "$RC" = "0" ] && echo 0 || echo 1)

echo "── 5) C 轨逃生舱（--c fn_* spawn → pthread：线程 = spawn+1）"
"$PX" build --c "$DIR/thr_count.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
"$DIR/build/thr_count" >/tmp/m93s2_5.out 2>&1 &
BP=$!
sleep 1.2
TP=$(pgrep -x thr_count | head -1)
[ -n "$TP" ] || TP=$BP
TH=$(grep Threads /proc/$TP/status 2>/dev/null | awk '{print $2}')
wait $BP
chk "C轨逃生舱 线程数=$TH (=129, pthread 语义保留)" $([ -n "$TH" ] && [ "$TH" = "129" ] && echo 0 || echo 1)

echo "════════ M93-S2 verify：PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ]
