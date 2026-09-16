#!/bin/bash
# 暂停线程落在哪：eu-stack 采样（正确的 PID 捕获）
BIN=${1:?}; N=${2:-8}; SECS=${3:-20}
"$BIN" "$N" "$SECS" > /dev/null 2>&1 &
PID=$!
sleep 5
echo "PID=$PID alive=$([ -d /proc/$PID ] && echo yes || echo no)"
for i in $(seq 1 "${4:-12}"); do
    eu-stack -p "$PID" 2>&1 | awk -v i="$i" '/^TID /{tid=$2} /gc_stop_handler|__sched_yield|px_futex_wait|futex/ {print "snap"i" "tid" "$0}' | head -8
    sleep 0.3
done
kill "$PID" 2>/dev/null
wait "$PID" 2>/dev/null
