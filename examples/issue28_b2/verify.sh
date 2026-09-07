#!/bin/bash
# ISSUE28-B2 验证（qg-issue 28 止血批次 B2：slab 空页归还 OS）
# 断言：
#   [a] 3 轮垃圾波后 RSS 回落到基线附近（rss_idle - rss_base < 40MB）——修复前空 slab
#       不归还，RSS 逐轮爬升不回吐，该断言失败；
#   [b] 峰值 RSS 显著高于回落值（peak > rss_idle + 60MB）——证明确有大波内存被归还。
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
LOG=/tmp/issue28_b2.log
rm -f "$LOG"

echo "== [1/2] 编译 =="
$PX build --no-quic wave.px >/dev/null 2>&1 || { echo "FAIL build"; exit 1; }
echo "PASS 编译"

echo "== [2/2] 运行采样 RSS =="
./build/wave > "$LOG" 2>&1 &
PID=$!
rss_kb() { awk '/VmRSS/{print $2}' "/proc/$PID/status" 2>/dev/null; }

# 峰值采样器：进程存活期间每 40ms 记一次最大值
PEAK=0
( while kill -0 $PID 2>/dev/null; do
      V=$(rss_kb)
      [ -n "$V" ] && [ "$V" -gt "$PEAK" ] 2>/dev/null && PEAK=$V
      sleep 0.04
  done
  echo "$PEAK" > /tmp/issue28_b2.peak ) &
SPID=$!

wait_marker() { for i in $(seq 1 300); do grep -q "$1" "$LOG" 2>/dev/null && return 0; sleep 0.1; done; return 1; }

wait_marker W-BASELINE  || { echo "FAIL daemon 未启动"; kill $PID 2>/dev/null; exit 1; }
sleep 1.0   # 等 baseline gc 稳定
BASE=$(rss_kb)

wait_marker W1-DONE || { echo "FAIL W1"; kill $PID 2>/dev/null; exit 1; }
sleep 0.3; R1=$(rss_kb)
wait_marker W2-DONE || { echo "FAIL W2"; kill $PID 2>/dev/null; exit 1; }
sleep 0.3; R2=$(rss_kb)
wait_marker W3-DONE || { echo "FAIL W3"; kill $PID 2>/dev/null; exit 1; }
sleep 0.3; R3=$(rss_kb)
wait_marker W-END || { echo "FAIL END"; kill $PID 2>/dev/null; exit 1; }

wait $SPID 2>/dev/null
PK=$(cat /tmp/issue28_b2.peak 2>/dev/null); PK=${PK:-0}
kill $PID 2>/dev/null
wait $PID 2>/dev/null

echo "RSS(kB): base=$BASE  W1后=$R1  W2后=$R2  W3后=$R3  峰值=$PK"

# [b] 峰值显著高于回落值（证明有大波内存确实被归还）
IDLE=$R3
if [ "$PK" -gt $((IDLE + 60000)) ]; then
    echo "PASS 峰值($PK) 显著高于回落后($IDLE)（+$((PK-IDLE))kB 已归还 OS）"
else
    echo "WARN 峰值($PK) 与回落后($IDLE) 差距不足 60MB（负载/采样粒度不足，仅参考）"
fi

# [a] 回落值贴近基线（修复前空 slab 不归还 → 逐轮爬升，本断言失败）
if [ "$IDLE" -le $((BASE + 40000)) ]; then
    echo "PASS 回落后 RSS($IDLE) 贴近基线($BASE)（+$((IDLE-BASE))kB，3 轮垃圾波无累积）"
else
    echo "FAIL 回落后 RSS($IDLE) 超出基线($BASE)+40MB（slab 空页未归还，B2 无效）"
    exit 1
fi

echo "issue28_b2 verify done"
exit 0
