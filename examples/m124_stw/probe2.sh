#!/bin/bash
# qg-issue 80 · 深入 A/B：sched_yield 速率 + 全 syscall 归属 + perf 符号归属 + 吞吐
# 用法：bash probe2.sh <label> <二进制> <worker数> <跑秒数>
LABEL=${1:?label}; BIN=${2:?bin}; N=${3:-8}; SECS=${4:-90}
OUT=/tmp/m124b_${LABEL}.txt
: > "$OUT"

"$BIN" "$N" "$SECS" > /tmp/m124b_${LABEL}.out 2>&1 &
PID=$!
sleep 5
echo "PID=$PID  load=$(cut -d' ' -f1-3 /proc/loadavg)" >> "$OUT"

snap() { for t in /proc/$PID/task/*/stat; do tid=${t%/stat}; tid=${tid##*/}; awk -v t="$tid" '{print t, $14, $15, $3}' "$t"; done 2>/dev/null; }
snap > /tmp/m124b_t0.txt
sleep 10
snap > /tmp/m124b_t1.txt
{
  echo "--- 每线程 CPU（10s 窗口；ticks@100Hz；utime+stime）---"
  join /tmp/m124b_t0.txt /tmp/m124b_t1.txt | awk '{du=$4-$2; ds=$5-$3; printf "  %s utime_delta=%d stime_delta=%d\n", $1, du, ds; u+=du; s+=ds} END{printf "  TOTAL utime=%d stime=%d => CPU=%.1f%% (sys 占比 %.1f%%)\n", u, s, (u+s)/10.0, 100.0*s/(u+s)}'
} >> "$OUT"

# ① 定向：sched_yield 速率（清歌给的完成判据）
rm -f /tmp/m124b_sy_${LABEL}.txt
strace -c -f -e trace=sched_yield -p "$PID" -o /tmp/m124b_sy_${LABEL}.txt -- sleep 5 >> "$OUT" 2>&1
{
  echo "--- ① strace -c -f -e trace=sched_yield -p $PID -- sleep 5 ---"
  if [ -s /tmp/m124b_sy_${LABEL}.txt ]; then cat /tmp/m124b_sy_${LABEL}.txt; else echo "  (空 —— 该窗口内 0 次 sched_yield)"; fi
} >> "$OUT"

# ② 全 syscall 归属（每次 5s）
rm -f /tmp/m124b_all_${LABEL}.txt
strace -c -f -p "$PID" -o /tmp/m124b_all_${LABEL}.txt -- sleep 5 >> "$OUT" 2>&1
echo "--- ② strace -c -f（全 syscall, 5s）top10 ---" >> "$OUT"
sort -k4 -nr /tmp/m124b_all_${LABEL}.txt 2>/dev/null | awk 'NR<=12' >> "$OUT"

# ③ perf 符号归属（12s）
perf record -F 997 -p "$PID" -o /tmp/m124b_perf_${LABEL}.data -- sleep 12 >> "$OUT" 2>&1
{
  echo "--- ③ perf（-F 997, 12s）top12 符号 ---"
  perf report -i /tmp/m124b_perf_${LABEL}.data --stdio --no-children -q 2>/dev/null | grep -v "^#" | head -12
  perf report -i /tmp/m124b_perf_${LABEL}.data --stdio --no-children -q 2>/dev/null | grep -v "^#" | head -40 | grep -c . >/dev/null
} >> "$OUT"

wait "$PID" 2>/dev/null
{ echo "--- ④ 吞吐（.px 自报）---"; cat /tmp/m124b_${LABEL}.out; } >> "$OUT"
echo "=== $OUT ==="
cat "$OUT"
