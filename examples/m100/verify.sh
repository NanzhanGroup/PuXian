#!/bin/bash
# ============================================================
# M100 verify.sh —— middleware 链协程化验证门（docs/M100_PLAN.md S2）
# ------------------------------------------------------------
#   1) build daemon（px build 自动重建 runtime .o：runtime.c/runtime_route.c 已改，
#      .rtcache key 变 → 编译错误在此暴露）+ Go 客户端（CGO_ENABLED=0 静态）
#   2) daemon max_conn=2（2 个 g_pool worker）+ middleware 3 段链
#   3) Go 客户端 P1 并发 20×/slow-mw（middleware sleep 600ms 让出）wall<4s
#      （同步占 2 worker 串行需 ~6s+）+ P2 30×/fast 不饿死 + P3 短路 401/403
#      + P4 多段链 null 推进 + P5 keep-alive 5 请求续处理
#   4) 服务端日志佐证：短路 (middleware) 文案（≥3：admin×2 + chain2×1）+ 访问日志
#   5) 线程峰值 ≤ 20（链 defer 后 2 pool worker 不随 slow 并发增长）
#   6) 优雅关闭：SIGTERM → 在途 0 干净退出
# 退出码：0=全 PASS；非 0=有失败。依赖：tools/px + Go。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m100"
PORT=18100
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── 准备：build daemon（含 M100 runtime）+ Go 客户端（静态）"
"$PX" build --no-quic "$DIR/m100_daemon.px" >/tmp/m100_build.log 2>&1 || { echo "FAIL build daemon"; tail -8 /tmp/m100_build.log; exit 1; }
CGO_ENABLED=0 go build -o /tmp/m100_go "$DIR/go_mw_client.go" >/tmp/m100_gobuild.log 2>&1 || { echo "FAIL go build"; tail -8 /tmp/m100_gobuild.log; exit 1; }
chk "build(daemon+go 静态客户端)" 0
file /tmp/m100_go | grep -q "statically linked"
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

echo "── 启动 px_serve daemon（max_conn=2）"
rm -f /tmp/px_m100_ready
"$DIR/build/m100_daemon" "$PORT" >/tmp/m100_srv.log 2>&1 </dev/null &
SRV_PID=$!
sleep 0.8
chk "daemon 存活" $(kill -0 $SRV_PID 2>/dev/null && echo 0 || echo 1)
READY=0
for i in $(seq 1 50); do
    R=$(curl -s -m 2 "http://127.0.0.1:$PORT/index.txt" 2>/dev/null)
    if [ "$R" = "M100-OK" ]; then READY=1; break; fi
    sleep 0.2
done
chk "px_serve 就绪（index.txt=M100-OK）" $([ "$READY" = "1" ] && echo 0 || echo 1)

echo "── 全程线程采样（后台；20 慢 middleware 并发不占 worker → 峰值 ≤20）"
rm -f /tmp/m100_threads.txt
(
    while kill -0 $SRV_PID 2>/dev/null; do
        N=$(ps -L -p $SRV_PID -o lwp= 2>/dev/null | wc -l)
        echo "$N" >>/tmp/m100_threads.txt
        sleep 0.1
    done
) &
SAMPLER_PID=$!

echo "── Go 客户端场景执行（P1 slow 并发 / P2 fast 不饿死 / P3-5 短路链 keep-alive）"
T0=$(date +%s.%N)
timeout 90 /tmp/m100_go "127.0.0.1:$PORT" >/tmp/m100_go.out 2>&1
GRC=$?
T1=$(date +%s.%N)
WALL=$(awk -v a="$T0" -v b="$T1" 'BEGIN{printf "%.2f", b-a}')
cat /tmp/m100_go.out
grep -q "M100S2_CLIENT PASS=.* FAIL=0" /tmp/m100_go.out
chk "客户端场景全 PASS（wall=${WALL}s rc=$GRC）" $?

# 线程峰值
PEAK=$(sort -n /tmp/m100_threads.txt 2>/dev/null | tail -1)
PEAK=${PEAK:-0}
chk "线程峰值 $PEAK ≤ 20（链 defer 后 pool2+coro≤8+事件循环+主）" $([ "$PEAK" -le 20 ] && echo 0 || echo 1)

# 服务端日志佐证：middleware 短路 (middleware) 文案 + 访问日志不丢
N_MW=$(grep -c -- "(middleware)" /tmp/m100_srv.log 2>/dev/null || echo 0)
chk "短路日志 (middleware) ≥3（admin×2 + chain2×1）" $([ "$N_MW" -ge 3 ] && echo 0 || echo 1)
N_ACC=$(grep -c -- "\[px-access\]" /tmp/m100_srv.log 2>/dev/null || echo 0)
chk "访问日志条数 ≥ 请求数（50+ 不丢）" $([ "$N_ACC" -ge 50 ] && echo 0 || echo 1)
N_R200=$(grep -c -- "-> 200" /tmp/m100_srv.log 2>/dev/null || echo 0)
chk "route 200 响应日志存在" $([ "$N_R200" -ge 1 ] && echo 0 || echo 1)

echo "── 优雅关闭（SIGTERM；IDLE/在途已清 → 干净退出）"
kill -TERM $SRV_PID 2>/dev/null
WAITED=0
for i in $(seq 1 80); do
    kill -0 $SRV_PID 2>/dev/null || { WAITED=1; break; }
    sleep 0.1
done
if [ "$WAITED" = "0" ]; then
    kill -9 $SRV_PID 2>/dev/null
    wait $SRV_PID 2>/dev/null
    chk "优雅关闭退出（≤8s）" 1
else
    wait $SRV_PID 2>/dev/null
    chk "优雅关闭退出" 0
fi
grep -q "优雅关闭完成（在途 0）" /tmp/m100_srv.log 2>/dev/null
chk "优雅关闭日志：在途 0" $?
kill $SAMPLER_PID 2>/dev/null

cleanup

echo ""
echo "════════ m100_s2: PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ] && exit 0 || exit 1
