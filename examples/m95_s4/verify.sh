#!/bin/bash
# ============================================================
# M95-S4 verify.sh —— sse_serve handler 协程化验证门
# ------------------------------------------------------------
#   1) 功能：VM sse handler 协程化路径 —— /health /sleep(/chan /gc) 全正确
#   2) 并发长业务占协程实证：fserve=2 + 并发 20 × /sleep(0.6s 两段 sleep) handler
#      —— handler 让出 → worker 不被占 → wall 显著小于串行（线程模型 2 worker 需 ≥6s）
#   3) 线程数收敛：并发 /sleep 期间采样 Threads（≈ fserve2+coro+事件+主+accept 少量）
#   4) /chan：sse handler 内 spawn+chan.recv 让出 → 结果正确
#   5) precise 低阈值 GC 压力：并发 /gc + /sleep —— 协程 args/连接注册表根正确
#   6) 客户端断连不崩 + 服务健康
# 退出码：0=全 PASS；非 0=有失败。
# 依赖：tools/px（M91 默认 VM 轨）。端口 18196。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m95_s4"
PORT=18196
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── build daemon（VM 轨；触发 runtime 全量重建若源码变更）"
"$PX" build --no-quic "$DIR/m95_s4_daemon.px" >/tmp/m95s4_build.log 2>&1 || { echo "FAIL build"; tail -20 /tmp/m95s4_build.log; exit 1; }
chk "build" 0

SRV_PID=0
trap 'if [ "$SRV_PID" -gt 0 ] 2>/dev/null; then kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; fi; rm -f /tmp/m95s4_*.out' EXIT
# fserve 仅 2 worker：若 handler 占 worker（旧模型），并发 20 长业务将被串行化 —— 本门核心
PX_SERVE_WORKERS=2 PX_CORO_WORKERS=4 "$DIR/build/m95_s4_daemon" $PORT >/tmp/m95s4_srv.log 2>&1 &
SRV_PID=$!
sleep 1.2

echo "── 1) 功能：VM sse handler 协程化路径基本路由（curl -N 读流）"
H=$(curl -sN --max-time 4 http://127.0.0.1:$PORT/health)
echo "  health=$H"
echo "$H" | grep -q '^data: ok$' || { echo "  FAIL /health（srv log:）"; tail -15 /tmp/m95s4_srv.log; exit 1; }
B=$(curl -sN --max-time 5 http://127.0.0.1:$PORT/sleep)
echo "  /sleep=$B"
echo "$B" | grep -q '^data: hello-sse$' || { echo "  FAIL /sleep hello-sse"; FAIL=$((FAIL+1)); }
echo "$B" | grep -q '^event: done$' || { echo "  FAIL /sleep event done"; FAIL=$((FAIL+1)); }
echo "$B" | grep -q '^data: \[DONE\]$' || { echo "  FAIL /sleep [DONE]"; FAIL=$((FAIL+1)); }
C=$(curl -sN --max-time 5 http://127.0.0.1:$PORT/chan)
echo "  /chan=$C"
echo "$C" | grep -q '^data: via-chan:from-sub$' || { echo "  FAIL /chan"; FAIL=$((FAIL+1)); }
chk "功能路由（health/sleep/chan）" $([ "$FAIL" = "0" ] && echo 0 || echo 1)

echo "── 2) 并发长业务占协程实证：20 × /sleep(0.6s) + fserve=2"
T0=$(date +%s.%N)
CPIDS=""
for i in $(seq 1 20); do
    curl -sN --max-time 10 "http://127.0.0.1:$PORT/sleep" >/tmp/m95s4_sleep_$i.out &
    CPIDS="$CPIDS $!"
done
wait $CPIDS
T1=$(date +%s.%N)
WALL=$(awk -v a="$T0" -v b="$T1" 'BEGIN{printf "%.3f", b-a}')
OKN=$(grep -l '^data: hello-sse$' /tmp/m95s4_sleep_*.out 2>/dev/null | wc -l)
DN=$(grep -l '^data: \[DONE\]$' /tmp/m95s4_sleep_*.out 2>/dev/null | wc -l)
echo "  并发 20 × /sleep(0.6s) wall=${WALL}s 全对=$OKN/20 done=$DN/20"
# 线程模型 fserve=2 串行需 ≥20×0.6/1≈6s+；协程让出模型应 <2s（8 并发内并行）
awk -v w="$WALL" 'BEGIN{exit !(w<2.0)}'; chk "并发长业务 wall=${WALL}s<2s (占协程实证)" $?
chk "20 请求全对" $([ "$OKN" = "20" ] && echo 0 || echo 1)
chk "[DONE] 全到" $([ "$DN" = "20" ] && echo 0 || echo 1)

echo "── 3) 线程数收敛：并发 /sleep 期间采样 Threads"
CPIDS=""
for i in $(seq 1 12); do
    curl -sN --max-time 10 "http://127.0.0.1:$PORT/sleep" >/dev/null 2>&1 &
    CPIDS="$CPIDS $!"
done
sleep 0.5
THR=$(grep Threads /proc/$SRV_PID/status 2>/dev/null | awk '{print $2}')
wait $CPIDS 2>/dev/null
echo "  并发期间 Threads=$THR（fserve=2+coro=4+事件+主+accept 理论 ≈ 11 内）"
chk "线程数收敛 Threads≤16" $([ -n "$THR" ] && [ "$THR" -le 16 ] && echo 0 || echo 1)

echo "── 4) /chan handler 内 spawn 子协程 + chan.recv 让出"
CB=$(curl -sN --max-time 5 "http://127.0.0.1:$PORT/chan")
chk "/chan=$CB" $([ "$CB" = "data: via-chan:from-sub" ] && echo 0 || echo 1)

echo "── 5) precise 低阈值 GC 压力：并发 /gc + /sleep"
kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null
PX_SERVE_WORKERS=2 PX_CORO_WORKERS=4 PX_GC_PRECISE=1 PX_GC_THRESHOLD=3000 \
    "$DIR/build/m95_s4_daemon" $PORT >/tmp/m95s4_srv.log 2>&1 &
SRV_PID=$!
sleep 1.2
CPIDS=""
for i in $(seq 1 12); do
    (curl -sN --max-time 10 "http://127.0.0.1:$PORT/gc" >/tmp/m95s4_gc_$i.out; \
     curl -sN --max-time 10 "http://127.0.0.1:$PORT/sleep" >/tmp/m95s4_gcsleep_$i.out) &
    CPIDS="$CPIDS $!"
done
wait $CPIDS
GCN=$(grep -l '^data: gc-ok$' /tmp/m95s4_gc_*.out 2>/dev/null | wc -l)
SLN=$(grep -l '^data: hello-sse$' /tmp/m95s4_gcsleep_*.out 2>/dev/null | wc -l)
rm -f /tmp/m95s4_gc_*.out /tmp/m95s4_gcsleep_*.out
chk "precise GC 压力 /gc 全对=$GCN/12" $([ "$GCN" = "12" ] && echo 0 || echo 1)
chk "precise GC 压力 /sleep 全对=$SLN/12" $([ "$SLN" = "12" ] && echo 0 || echo 1)

echo "── 6) 客户端断连不崩 + 服务健康"
curl -sN --max-time 1 "http://127.0.0.1:$PORT/sleep" >/dev/null 2>&1 &  # 中途断连（1s 后 curl 退出）
sleep 2.5
HH=$(curl -sN --max-time 4 "http://127.0.0.1:$PORT/health")
echo "  /health=$HH"
chk "断连后服务健康" $([ "$HH" = "data: ok" ] && echo 0 || echo 1)

kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null
SRV_PID=0
rm -f /tmp/m95s4_*.out
echo "════════════════════════════"
echo "M95-S4: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = "0" ]
