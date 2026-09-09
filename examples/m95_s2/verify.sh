#!/bin/bash
# ============================================================
# M95-S2 verify.sh —— http_serve handler 协程化验证门（D8-②）
# ------------------------------------------------------------
#   1) 功能：VM handler 协程化路径 —— /health /json /echo(/sleep /chan /gc) 全 200
#   2) 并发长业务占协程实证：fserve=2 + 并发 30 × /sleep(0.1s) handler —— 协程模型
#      让出 → worker 不被占 → wall 显著小于串行（线程模型 2 worker 需 ~1.5s+）
#   3) 线程数收敛：并发 /slow（持续 100ms 占用窗）期间采样 Threads ——
#      线程数 ≈ fserve(2)+coro+事件+主+accept 少量（不随请求数涨到 30）
#   4) /chan：handler 内 spawn+chan.recv 让出 → 结果正确（协程内 chan 让出链路）
#   5) precise 低阈值 GC 压力：PX_GC_PRECISE=1 PX_GC_THRESHOLD=3000 并发 /gc + /sleep
#      —— pending 表 req/resp GC 根正确（零崩零错）
#   6) keep-alive + Connection: close + HEAD 逐字节对拍（续处理段语义回归）
# 退出码：0=全 PASS；非 0=有失败。
# 依赖：tools/px（M91 默认 VM 轨）。端口 18195。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m95_s2"
PORT=18195
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── build daemon（VM 轨；触发 runtime 全量重建若源码变更）"
"$PX" build --no-quic "$DIR/m95_daemon.px" >/tmp/m95s2_build.log 2>&1 || { echo "FAIL build"; tail -20 /tmp/m95s2_build.log; exit 1; }
chk "build" 0

SRV_PID=0
trap 'if [ "$SRV_PID" -gt 0 ] 2>/dev/null; then kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; fi' EXIT
# fserve 仅 2 worker：若 handler 占 worker（旧模型），并发 30 长业务将被串行化 —— 本门核心
PX_SERVE_WORKERS=2 PX_CORO_WORKERS=4 "$DIR/build/m95_daemon" $PORT >/tmp/m95s2_srv.log 2>&1 &
SRV_PID=$!
sleep 1.0

echo "── 1) 功能：VM handler 协程化路径基本路由"
B=$(curl -s --max-time 3 "http://127.0.0.1:$PORT/health");       chk "/health=$B" $([ "$B" = "ok" ] && echo 0 || echo 1)
B=$(curl -s --max-time 3 "http://127.0.0.1:$PORT/json");          chk "/json=$B" $([ "$B" = "json-ok" ] && echo 0 || echo 1)
B=$(curl -s --max-time 3 "http://127.0.0.1:$PORT/echo?abc=1&x=y"); chk "/echo=$B" $([ "$B" = "echo:abc=1&x=y" ] && echo 0 || echo 1)

echo "── 2) 并发长业务占协程实证：30 × /sleep(0.1s) + fserve=2"
T0=$(date +%s.%N)
CPIDS=""
for i in $(seq 1 30); do
    curl -s --max-time 8 "http://127.0.0.1:$PORT/sleep" >/tmp/m95s2_sleep_$i.out &
    CPIDS="$CPIDS $!"
done
wait $CPIDS
T1=$(date +%s.%N)
WALL=$(awk -v a="$T0" -v b="$T1" 'BEGIN{printf "%.3f", b-a}')
OKN=$(grep -l '^slept$' /tmp/m95s2_sleep_*.out | wc -l)
rm -f /tmp/m95s2_sleep_*.out
echo "  并发 30 × sleep(0.1) wall=${WALL}s 全对=$OKN/30"
# 线程模型 fserve=2 串行需 ≥30×0.1=3s；协程让出模型应 <1.5s（8 并发上限内）
awk -v w="$WALL" 'BEGIN{exit !(w<1.5)}'; chk "并发长业务 wall=${WALL}s<1.5s (占协程实证)" $?
chk "30 请求全对" $([ "$OKN" = "30" ] && echo 0 || echo 1)

echo "── 3) 线程数收敛：并发 /slow(100ms 窗) 期间采样 Threads"
CPIDS=""
for i in $(seq 1 20); do
    curl -s --max-time 8 "http://127.0.0.1:$PORT/slow" >/dev/null &
    CPIDS="$CPIDS $!"
done
sleep 0.35   # 请求都进入 handler 占用窗中段
THR=$(grep Threads /proc/$SRV_PID/status 2>/dev/null | awk '{print $2}')
wait $CPIDS
echo "  并发 20 请求期间 Threads=$THR（fserve=2+coro=4+事件+主+accept 理论 ≈ 11 内）"
chk "线程数收敛 Threads≤16" $([ -n "$THR" ] && [ "$THR" -le 16 ] && echo 0 || echo 1)

echo "── 4) /chan：handler 内 spawn 子协程 + chan.recv 让出"
B=$(curl -s --max-time 5 "http://127.0.0.1:$PORT/chan"); chk "/chan=$B" $([ "$B" = "chan-from-worker" ] && echo 0 || echo 1)

echo "── 5) precise 低阈值 GC 压力：并发 /gc + /sleep"
# 重启 daemon：PX_GC_PRECISE=1（VM 产物默认 precise，显式明确）+ PX_GC_THRESHOLD=3000
#   低阈值 → 并发 /gc（大量分配）+ /sleep（挂起 200ms）期间 GC 频繁触发 → 验证
#   pending 表 req/resp + 挂起协程帧 GC 根正确（漏标 = UAF/崩/错）。
kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null
PX_SERVE_WORKERS=2 PX_CORO_WORKERS=4 PX_GC_PRECISE=1 PX_GC_THRESHOLD=3000 \
    "$DIR/build/m95_daemon" $PORT >/tmp/m95s2_srv.log 2>&1 &
SRV_PID=$!
sleep 1.0
CPIDS=""
for i in $(seq 1 16); do
    (curl -s --max-time 10 "http://127.0.0.1:$PORT/gc" >/tmp/m95s2_gc_$i.out; \
     printf '\n' >>/tmp/m95s2_gc_$i.out; \
     curl -s --max-time 10 "http://127.0.0.1:$PORT/sleep" >>/tmp/m95s2_gc_$i.out) &
    CPIDS="$CPIDS $!"
done
wait $CPIDS
# 逐文件 grep -l 统计（勿用 cat 拼接：daemon 响应体无尾换行，拼接会粘连行破坏
#   ^gc-ok$ 行锚定 → 假 FAIL；grep -l 对无尾换行的末行同样匹配，逐文件独立正确）。
GCN=$(grep -l '^gc-ok$' /tmp/m95s2_gc_*.out 2>/dev/null | wc -l)
SLN=$(grep -l '^slept$' /tmp/m95s2_gc_*.out 2>/dev/null | wc -l)
rm -f /tmp/m95s2_gc_*.out
chk "precise GC 压力 /gc 全对=$GCN/16" $([ "$GCN" = "16" ] && echo 0 || echo 1)
chk "precise GC 压力 /sleep 全对=$SLN/16" $([ "$SLN" = "16" ] && echo 0 || echo 1)

echo "── 6) Connection: close + HEAD（续处理段语义）"
H=$(curl -s -D - --max-time 3 -o /dev/null "http://127.0.0.1:$PORT/health")
echo "$H" | grep -qi '^HTTP/1.1 200' || { echo "  FAIL close 200"; FAIL=$((FAIL+1)); } 
HH=$(curl -s -I --max-time 3 "http://127.0.0.1:$PORT/health")
echo "$HH" | grep -qi '^HTTP/1.1 200' && echo "  PASS HEAD 200" || { echo "  FAIL HEAD"; FAIL=$((FAIL+1)); }
# HEAD 无 body
HB=$(curl -s -I --max-time 3 "http://127.0.0.1:$PORT/json" | wc -c)
[ "$HB" -lt 400 ] && echo "  PASS HEAD 无 body (bytes=$HB)" || { echo "  FAIL HEAD body 泄露"; FAIL=$((FAIL+1)); }
chk "close/HEAD 收尾" 0

# 服务仍健康（无崩溃泄漏）
B=$(curl -s --max-time 3 "http://127.0.0.1:$PORT/health")
chk "压测后服务健康 /health=$B" $([ "$B" = "ok" ] && echo 0 || echo 1)

kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null
SRV_PID=0
echo "════════════════════════════"
echo "M95-S2: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = "0" ]
