#!/usr/bin/env bash
# ============================================================
# M94-S3 verify.sh —— 定时器并入调度循环验证门
# ------------------------------------------------------------
#   1) coro_timer_thr：20 协程各 sleep 2s，0.8s 窗口抓线程数 —— WORKERS=4
#      预期 5（4 worker + main），无独立 timer 线程（旧版 6）
#   2) coro_sleep_par 复跑：40 并发 sleep 总墙钟 ≈ max(120ms)（并入后精度保持）
#   3) m94_s2 抢占套件复绿（定时合并不破坏抢占轮转）
# 退出码：0=全 PASS；非 0=有失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m94_s3"
PASS=0 FAIL=0

chk() { # $1=名 $2=条件
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── 1) 定时并入调度循环：无独立 timer 线程（WORKERS=4 → 线程=5，旧版=6）"
"$PX" build "$DIR/coro_timer_thr.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_WORKERS=4 "$DIR/build/coro_timer_thr" >/tmp/m94s3_1.out 2>&1 &
PID=$!
sleep 0.8
THR=$(awk '/^Threads/{print $2}' /proc/$PID/status 2>/dev/null)
wait $PID
RC=$?
grep -q "TIMER-THR DONE" /tmp/m94s3_1.out
chk "coro_timer_thr 完成（rc=$RC）" $?
echo "    （抓取线程数=$THR，预期 ≤5=4worker+main，无 timer 线程）"
[ -n "$THR" ] && [ "$THR" -le 5 ]
chk "线程数收敛 THR=$THR ≤5（无 timer 线程）" $?

echo "── 2) 并发 sleep 40 协程复跑（并入后精度 ≈ max 120ms）"
"$PX" build "$ROOT/examples/m93_s3/coro_sleep_par.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_WORKERS=8 timeout 40 "$ROOT/examples/m93_s3/build/coro_sleep_par" >/tmp/m94s3_2.out 2>&1
grep -q "CORO-SLEEP-PAR OK" /tmp/m94s3_2.out
chk "coro_sleep_par 并发 sleep（wall≈max）" $?

echo "── 3) 抢占套件复绿（m94_s2）"
timeout 200 "$ROOT/examples/m94_s2/verify.sh" >/tmp/m94s3_3.out 2>&1
grep -q "PASS=4 FAIL=0" /tmp/m94s3_3.out
chk "m94_s2 抢占套件复绿（rc=$?）" $?

echo "════════ M94-S3 verify：PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ]
