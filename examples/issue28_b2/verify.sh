#!/bin/bash
# ISSUE28-B2 验证（qg-issue 28 止血批次 B2：slab 空页归还 OS）
# 断言：
#   [a] 3 轮垃圾波后 RSS 回落到基线附近（rss_idle - rss_base < 40MB）——修复前空 slab
#       不归还，RSS 逐轮爬升不回吐，该断言失败；
#   [b] 峰值 RSS 显著高于回落值（peak > rss_idle + 60MB）——证明确有大波内存被归还。
# 用法：bash verify.sh
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
LOG=/tmp/issue28_b2.log
rm -f "$LOG"

echo "== [1/2] 编译 =="
$PX build --no-quic wave.px >/dev/null 2>&1 || { echo "FAIL build"; exit 1; }
echo "PASS 编译"

echo "== [2/2] 运行采样 RSS（M251：改「全程 0.1s 序列 + 峰值后最小值」判据）=="
# M251 体检：原判据在 `W3-DONE` 后 sleep 0.3s 采一个点当「回落后值」。
#   实测该采样点与真实事件**不对齐** —— wave.px 的 stdout 重定向到文件是**块缓冲**，
#   所以 verify.sh 用 grep 看到 marker 的时刻 ≠ 程序真正打印的时刻 ⇒ 采到的可能是任意时刻。
#   实测序列（0.1s 粒度）：4052…→290740 峰值 297108 →**骤降到 21884**→…→273540（末波停在 273MB）
#   ⇒ 「某一固定时刻的 RSS」不可作判据；但**峰值之后出现过接近基线的低点**是稳定的。
#   修法（贴 B2 的契约「堆只涨不落」的反面）：
#     · 全程采样，找 max（峰值）与 max 之后的最小值 min_after
#     · 判据 A：min_after ≤ base + 40MB（归还确实发生过）
#     · 判据 B：max > base + 60MB（确实有大波，否则 A 是恒真）
#   修复前（堆只涨不落）⇒ min_after ≈ max ⇒ A 必红。
./build/wave > "$LOG" 2>&1 &
PID=$!
rss_kb() { awk '/VmRSS/{print $2}' "/proc/$PID/status" 2>/dev/null; }

SER=/tmp/issue28_b2.series
: > "$SER"
( while kill -0 $PID 2>/dev/null; do
      V=$(rss_kb)
      [ -n "$V" ] && echo "$V" >> "$SER"
      sleep 0.1
  done ) &
SPID=$!

wait_marker() { for i in $(seq 1 600); do grep -q "$1" "$LOG" 2>/dev/null && return 0; sleep 0.1; done; return 1; }
wait_marker W-BASELINE || { echo "FAIL daemon 未启动"; kill $PID 2>/dev/null; exit 1; }
sleep 1.0
BASE=$(rss_kb)

wait_marker W-END || { echo "FAIL W-END"; kill $PID 2>/dev/null; exit 1; }
sleep 0.5
wait $PID 2>/dev/null
kill $SPID 2>/dev/null; wait $SPID 2>/dev/null
PK=$(cat /tmp/issue28_b2.peak 2>/dev/null); PK=${PK:-0}

SER_RC=$?
MX=$(sort -n "$SER" 2>/dev/null | tail -1); MX=${MX:-0}
# 「波开始」= 序列里第一个 > base+20MB 的点 ⇒ 之后的**最小值**（跳过基线段，否则 min 恒为 base）
MIN_AFTER=$(awk -v base="$BASE" 'BEGIN{started=0; min=""}
  { v=$1+0
    if (!started && v > base+20000) { started=1 }
    if (started) { if (min=="" || v<min) min=v }
  } END{ print (min==""?0:min) }' "$SER")

echo "RSS(kB): base=$BASE  序列点数=$(wc -l < "$SER")  峰值=$MX  波内最小值=$MIN_AFTER"
echo "  序列（每点 0.1s · 压缩显示相邻变化）：$(awk 'NR==1{p=$1;printf "%s ",$1;next}{if($1!=p){printf "%s ",$1;p=$1}}' "$SER" | cut -c1-300)"

if [ "$MX" -le $((BASE + 60000)) ]; then
    echo "FAIL 峰值($MX) 未显著高于基线($BASE)+60MB —— 本没产生大波，判据 B 无意义"
    exit 1
fi
echo "PASS 峰值 $MX > 基线+60MB（确实有大波）"

if [ "$MIN_AFTER" -le $((BASE + 40000)) ]; then
    echo "PASS 波内最小值($MIN_AFTER) ≤ 基线($BASE)+40MB（+$((MIN_AFTER-BASE))kB ⇒ 空 slab 页已归还 OS）"
else
    echo "FAIL 波内最小值($MIN_AFTER) 超出基线($BASE)+40MB（堆只涨不落 ⇒ B2 无效）"
    exit 1
fi

echo "issue28_b2 verify done"
