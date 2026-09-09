#!/usr/bin/env bash
# M93-S2 验证：spawn N 纯计算协程 → 进程线程数 = worker 数(+1 主)，非 spawn 数。
# 期望：默认 PX_CORO_WORKERS=min(CPU,8)；本机 8 核 → Threads ≤ 9，远小于 spawn N。
set -u
BIN="${1:?用法: thr_count.sh <vm二进制> [spawn数] [期望线程上限]}"
N="${2:-128}"
CAP="${3:-9}"
"$BIN" > /tmp/m93_thr.out 2>&1 &
BPID=$!
sleep 1.2
TPID=$(pgrep -x "$(basename "$BIN")" | head -1)
[ -n "$TPID" ] || TPID=$BPID
THREADS=$(grep Threads /proc/$TPID/status 2>/dev/null | awk '{print $2}')
wait $BPID
RC=$?
echo "spawn=$N threads=$THREADS (cap=$CAP) rc=$RC"
echo "--- program out:"
cat /tmp/m93_thr.out
echo "---"
# 断言：线程数 ≤ cap（远小于 spawn 数）且程序正常退出
if [ -z "$THREADS" ] || [ "$THREADS" -gt "$CAP" ]; then
    echo "THR-COUNT-FAIL: threads=$THREADS > cap=$CAP"
    exit 1
fi
[ "$RC" = "0" ] && echo "THR-COUNT-OK" || { echo "RC-FAIL"; exit 1; }
