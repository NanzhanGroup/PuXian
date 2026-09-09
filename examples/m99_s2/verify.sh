#!/bin/bash
# ============================================================
# M99-S2 verify.sh —— px_serve 连接级事件化 IDLE 验证门
# ------------------------------------------------------------
#   1) max_conn=4 下并发 40 条 keep-alive 连接 × /fast 全成功：
#      事件化前 max_conn 预派生池被 keep-alive 空闲占死（处理完请求阻塞 recv 等下一请求）
#      → conn5..40 悬挂超时；M99 空闲交 IDLE → 40/40 即时完成（核心铁证）
#   2) 全程线程峰值 ≤ 20（40 空闲连接 0 占用 worker：池4+coro≤8+事件循环+主，不随连接增长；
#      事件化前每空闲连接占线程 → 峰值 ≥40+）
#   3) 40 conns 并发 × /slow（sleep 300ms 协程让出）：M98 defer + M99 IDLE 组合
#      并发全成功 wall<3s（顺序 40×300ms 才需 12s）
#   4) 空闲 16.5s 后连接被服务端 15s tick 超时关闭（8/8 探测失败 = fd 回收）
#      （HTTP/1.1 pipelining 同缓冲残余续接不在 M99 范围：px_conn_worker 无 pbuf 续接 =
#      px_serve 既有限制，http_serve 有 pbuf，二期候选）
#   6) 优雅关闭：SIGTERM → join + 清 IDLE → 「在途 0」干净退出（≤8s）
# 退出码：0=全 PASS；非 0=有失败。依赖：tools/px + Go（CGO_ENABLED=0 静态客户端）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m99_s2"
PORT=18199
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── 准备：build daemon（含 M99 runtime）+ Go 客户端（静态）"
"$PX" build --no-quic "$DIR/m99_s2_daemon.px" >/tmp/m99s2_build.log 2>&1 || { echo "FAIL build daemon"; tail -8 /tmp/m99s2_build.log; exit 1; }
CGO_ENABLED=0 go build -o /tmp/m99s2_go "$DIR/go_ka_client.go" >/tmp/m99s2_gobuild.log 2>&1 || { echo "FAIL go build"; tail -8 /tmp/m99s2_gobuild.log; exit 1; }
chk "build(daemon+go 静态客户端)" 0
file /tmp/m99s2_go | grep -q "statically linked"
chk "Go 客户端静态链接" $?

SRV_PID=0
cleanup() {
    if [ -n "$SRV_PID" ] && [ "$SRV_PID" -gt 0 ] 2>/dev/null; then
        kill $SRV_PID 2>/dev/null
        for i in $(seq 1 30); do kill -0 $SRV_PID 2>/dev/null || break; sleep 0.1; done
        kill -9 $SRV_PID 2>/dev/null
        wait $SRV_PID 2>/dev/null
    fi
    SRV_PID=0
}
trap cleanup EXIT

echo "── 启动 px_serve daemon（max_conn=4）"
rm -f /tmp/px_m99_s2_ready
"$DIR/build/m99_s2_daemon" "$PORT" >/tmp/m99s2_srv.log 2>&1 </dev/null &
SRV_PID=$!
sleep 0.8
chk "daemon 存活" $(kill -0 $SRV_PID 2>/dev/null && echo 0 || echo 1)
READY=0
for i in $(seq 1 50); do
    R=$(curl -s -m 2 "http://127.0.0.1:$PORT/index.txt" 2>/dev/null)
    if [ "$R" = "M99-OK" ]; then READY=1; break; fi
    sleep 0.2
done
chk "px_serve 就绪（index.txt=M99-OK）" $([ "$READY" = "1" ] && echo 0 || echo 1)

echo "── 全程线程采样（后台；40 空闲连接不占 worker → 峰值 ≤20）"
PEAK=0
SAMPLER_PID=0
(
    while kill -0 $SRV_PID 2>/dev/null; do
        N=$(ps -L -p $SRV_PID -o lwp= 2>/dev/null | wc -l)
        [ "$N" -gt "${PEAK_FILE_VAL:-0}" ] 2>/dev/null || true
        echo "$N" >>/tmp/m99s2_threads.txt
        sleep 0.1
    done
) &
SAMPLER_PID=$!
rm -f /tmp/m99s2_threads.txt

echo "── Go 客户端场景执行（P1 并发40 / P3 slow 并发 / P5-6 空闲超时回收）"
T0=$(date +%s.%N)
timeout 90 /tmp/m99s2_go "127.0.0.1:$PORT" >/tmp/m99s2_go.out 2>&1
GRC=$?
T1=$(date +%s.%N)
WALL=$(awk -v a="$T0" -v b="$T1" 'BEGIN{printf "%.2f", b-a}')
cat /tmp/m99s2_go.out
grep -q "M99S2_CLIENT PASS=.* FAIL=0" /tmp/m99s2_go.out
chk "客户端场景全 PASS（wall=${WALL}s rc=$GRC）" $?

# 线程峰值（含 40 空闲连接全程保持）
PEAK=$(sort -n /tmp/m99s2_threads.txt 2>/dev/null | tail -1)
PEAK=${PEAK:-0}
chk "线程峰值 $PEAK ≤ 20（40 空闲连接不占 worker：池4+coro≤8+事件循环+主）" $([ "$PEAK" -le 20 ] && echo 0 || echo 1)

# 服务端日志佐证：空闲连接由事件循环 tick 关闭（P6 后 8 连接回收）
N_TMO=$(grep -c "优雅关闭完成" /tmp/m99s2_srv.log 2>/dev/null || echo 0)

echo "── 优雅关闭（SIGTERM；IDLE 已清 → 干净退出）"
T0=$(date +%s.%N)
kill -TERM $SRV_PID 2>/dev/null
WAITED=0
for i in $(seq 1 80); do
    kill -0 $SRV_PID 2>/dev/null || { WAITED=1; break; }
    sleep 0.1
done
T1=$(date +%s.%N)
GCT=$(awk -v a="$T0" -v b="$T1" 'BEGIN{printf "%.1f", b-a}')
if [ "$WAITED" = "0" ]; then
    kill -9 $SRV_PID 2>/dev/null
    wait $SRV_PID 2>/dev/null
    chk "优雅关闭退出（≤8s）" 1
else
    wait $SRV_PID 2>/dev/null; SRC=$?
    chk "优雅关闭退出（${GCT}s）" 0
fi
grep -q "优雅关闭完成（在途 0）" /tmp/m99s2_srv.log 2>/dev/null
chk "优雅关闭日志：在途 0（IDLE 连接已清）" $?

cleanup

echo ""
echo "════════ m99_s2(S2): PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ] && exit 0 || exit 1
