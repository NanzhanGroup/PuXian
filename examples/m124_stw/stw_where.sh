#!/bin/bash
# M124 验收②：GC 暂停期间「被暂停线程落在哪里」——__sched_yield 自旋 vs __futex 阻塞等待
# 用法：sh stw_where.sh <label> <二进制> [worker数] [跑秒数]
LABEL=${1:?label}; BIN=${2:?bin}; N=${3:-8}; SECS=${4:-30}
OUT=/tmp/m124_where_${LABEL}.txt
"$BIN" "$N" "$SECS" > /tmp/m124_where_${LABEL}.out 2>&1 &
PID=$!
sleep 5
: > "$OUT"
for i in 1 2 3 4 5 6; do
  eu-stack -p "$PID" >> "$OUT" 2>/dev/null
  sleep 0.4
done
kill "$PID" 2>/dev/null; wait "$PID" 2>/dev/null
echo "label=$LABEL pid=$PID 快照=6"
echo "  在 gc_stop_handler 内的线程栈顶统计："
echo "    __sched_yield : $(grep -c '__sched_yield' "$OUT")"
echo "    __futex*      : $(grep -c '__futex' "$OUT")"
echo "  GC 暂停帧（gc_stop_handler）出现次数: $(grep -c 'gc_stop_handler' "$OUT")"
echo "  样例（含 gc_stop_handler 的栈顶3行）:"
grep -A3 'gc_stop_handler' "$OUT" | head -12
