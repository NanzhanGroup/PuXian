#!/bin/bash
# 单次定点测量：<bin> <secs>，输出 sched_yield 速率 + futex 速率 + 总 CPU
BIN=${1:?bin}; SECS=${2:-40}; N=${3:-8}
"$BIN" "$N" "$SECS" > /tmp/single.out 2>&1 &
PID=$!
sleep 5
echo "PID=$PID exists=$([ -d /proc/$PID ] && echo yes || echo no) threads=$(grep -c . /proc/$PID/task/. 2>/dev/null; ls /proc/$PID/task | wc -l)"
read -r _ _ _ _ _ _ _ _ _ _ _ _ _ U0 S0 _ < /proc/$PID/stat
echo "--- strace: sched_yield (5s) ---"
strace -c -f -e trace=sched_yield -p "$PID" -o /tmp/sy.txt -- sleep 5
echo "exists=$([ -d /proc/$PID ] && echo yes || echo no)"
if [ -s /tmp/sy.txt ]; then cat /tmp/sy.txt; else echo "(0 次 sched_yield —— strace 无该行输出)"; fi
echo "--- strace: futex (5s) ---"
strace -c -f -e trace=futex -p "$PID" -o /tmp/fx.txt -- sleep 5
if [ -s /tmp/fx.txt ]; then cat /tmp/fx.txt; else echo "(0 次 futex)"; fi
read -r _ _ _ _ _ _ _ _ _ _ _ _ _ U1 S1 _ < /proc/$PID/stat
echo "--- 总 CPU（15s 窗口, ticks@100Hz）---"
echo "utime_delta=$((U1-U0)) stime_delta=$((S1-S0)) => CPU=$(( (U1-U0+S1-S0)/10 ))% sys_share=$(( 100*(S1-S0)/(U1-U0+S1-S0+1) ))%"
echo "--- perf (10s) ---"
perf record -F 997 -p "$PID" -o /tmp/pf.data -- sleep 10 >/dev/null 2>&1 && perf report -i /tmp/pf.data --stdio --no-children -q 2>/dev/null | grep -v '^#' | head -10
kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
tail -2 /tmp/single.out
