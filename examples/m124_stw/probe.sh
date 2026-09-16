#!/bin/bash
# qg-issue 80 · STW 屏障 A/B 测量器（一次跑完，结果落 $OUT）
# 用法：bash probe.sh <label> <二进制> <worker数> <跑秒数>
LABEL=${1:?label}; BIN=${2:?bin}; N=${3:-8}; SECS=${4:-120}
OUT=/tmp/m124_${LABEL}.txt
: > "$OUT"

"$BIN" "$N" "$SECS" > /tmp/m124_${LABEL}.out 2>&1 &
PID=$!
sleep 5     # 预热：8 个 coro worker 起齐 + GC 阈值爬升
echo "PID=$PID" >> "$OUT"

snap() { for t in /proc/$PID/task/*/stat; do tid=${t%/stat}; tid=${tid##*/}; awk -v t="$tid" '{print t, $14, $15}' "$t"; done 2>/dev/null; }
snap > /tmp/m124_t0.txt
sleep 10
snap > /tmp/m124_t1.txt

{
  echo "--- 每线程 CPU（10s 窗口，ticks@100Hz）---"
  join /tmp/m124_t0.txt /tmp/m124_t1.txt | awk '{d=($4+$5)-($2+$3); printf "%s %d\n", $1, d; tot+=d} END{printf "TOTAL ticks=%d => %.1f%% CPU (%.2f cores per 10s)\n", tot, tot/10.0, tot/1000.0}'
} >> "$OUT"

# strace：sched_yield 速率（5s）
strace -c -f -e trace=sched_yield -p "$PID" -o /tmp/m124_strace_${LABEL}.txt -- sleep 5 >> "$OUT" 2>&1
{
  echo "--- strace -c -f -e trace=sched_yield（5s）---"
  grep -i sched_yield /tmp/m124_strace_${LABEL}.txt
  awk '/sched_yield/ {for(i=NF-1;i>=1;i--) if ($i ~ /^[0-9]+$/) {printf "  => sched_yield calls=%s  rate=%.0f/s\n", $i, $i/5; exit}}' /tmp/m124_strace_${LABEL}.txt
} >> "$OUT"

# perf：采样归属（12s）
perf record -F 997 -p "$PID" -o /tmp/m124_perf_${LABEL}.data -- sleep 12 > /dev/null 2>&1
{
  echo "--- perf 采样归属（-F 997, 12s）---"
  perf report -i /tmp/m124_perf_${LABEL}.data --stdio --no-children 2>/dev/null | head -25
} >> "$OUT"

wait "$PID" 2>/dev/null
{
  echo "--- 吞吐（.px 自报）---"
  cat /tmp/m124_${LABEL}.out
} >> "$OUT"
echo "=== $OUT ==="
cat "$OUT"
