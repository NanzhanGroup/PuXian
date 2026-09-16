#!/bin/sh
# qg-issue 80 验收测量器：同一压力下量
#   ① sched_yield/s（strace -c -f -e trace=sched_yield）
#   ② 每线程 CPU（/proc/<tid>/stat utime+stime 差分）→ 总 CPU%（核）
# 用法：sh measure.sh <二进制> <worker数> [跑秒数]
set -e
BIN=${1:-./build/stw_stress}
N=${2:-8}
SECS=${3:-60}
HZ=100

"$BIN" "$N" "$SECS" &
PID=$!
sleep 4    # 预热：等 8 个 coro worker 起齐 + 首次 GC 阈值爬升

snap() { # $1=pid → "tid utime stime" 列表
    for t in /proc/$1/task/*; do
        [ -r "$t/stat" ] || continue
        tid=$(basename "$t")
        awk -v tid="$tid" '{print tid, $14, $15}' "$t/stat" 2>/dev/null || true
    done
}
snap "$PID" > /tmp/m124_t0.txt || true
sleep 10
snap "$PID" > /tmp/m124_t1.txt || true

echo "--- 每线程 CPU（10s 窗口，单位 ticks@100Hz）---"
join /tmp/m124_t0.txt /tmp/m124_t1.txt 2>/dev/null | \
    awk -v hz=$HZ '{d=($4+$5)-($2+$3); if (d>0) print $1, d; tot+=d} END {printf "TOTAL ticks=%d  => %.1f%% CPU (%.2f 核) / 10s\n", tot, tot*100.0/hz/10, tot/1.0/hz/10}' | sort -k2 -nr

echo "--- strace -c -f -e trace=sched_yield（5s）---"
strace -c -f -e trace=sched_yield -p "$PID" -o /tmp/m124_strace.txt -- sleep 5 2>/dev/null || true
grep -i "sched_yield" /tmp/m124_strace.txt || echo "(no sched_yield line)"
awk '/sched_yield/ {printf "  => sched_yield/s = %.0f\n", $4/5}' /tmp/m124_strace.txt || true

kill "$PID" 2>/dev/null || true
wait "$PID" 2>/dev/null || true
