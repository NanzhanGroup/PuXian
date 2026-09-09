#!/bin/bash
# ============================================================
# M98-S2a verify.sh —— px_serve route VM handler 协程化验证门
# ------------------------------------------------------------
#   1) 并发 20×/slow（handler sleep 1200ms）+ max_conn=2 池：
#      协程化后 worker 释放 → 全完成 < 6s（同步占线程模型 2 worker ≈ 12s）
#   2) 慢 handler 在途时单连接 30×/fast 不被饿死（wall<800ms，dials=1）
#   3) keep-alive 单连接顺序 2×/slow（续处理成立，2×1.2s 窗）
#   4) /p/:id 路径参数 + /big 1.5MB body（落盘 tmp）defer 期可读 + HEAD + 静态
#   5) /alloc ×10 分配风暴（precise 低阈值 GC 下挂起表 req/resp GC 根）
#   6) 服务端访问日志条数与客户端请求数对拍（无丢无悬挂）
# 退出码：0=全 PASS；非 0=有失败。依赖：tools/px + Go（CGO_ENABLED=0 静态客户端）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m98_s2"
PORT=18198
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── 准备：build daemon（含 M98 runtime）+ Go 客户端（静态）"
"$PX" build --no-quic "$DIR/m98_s2_daemon.px" >/tmp/m98s2_build.log 2>&1 || { echo "FAIL build daemon"; tail -8 /tmp/m98s2_build.log; exit 1; }
CGO_ENABLED=0 go build -o /tmp/m98s2_go "$DIR/go_ka_client.go" >/tmp/m98s2_gobuild.log 2>&1 || { echo "FAIL go build"; tail -8 /tmp/m98s2_gobuild.log; exit 1; }
chk "build(daemon+go 静态客户端)" 0
file /tmp/m98s2_go | grep -q "statically linked"
chk "Go 客户端静态链接" $?

SRV_PID=0
cleanup() {
    if [ -n "$SRV_PID" ] && [ "$SRV_PID" -gt 0 ] 2>/dev/null; then
        kill $SRV_PID 2>/dev/null
        for i in $(seq 1 30); do kill -0 $SRV_PID 2>/dev/null || break; sleep 0.1; done
        kill -9 $SRV_PID 2>/dev/null   # daemon 优雅关闭等待 keep-alive 空闲连接超时 → 兜底强杀
        wait $SRV_PID 2>/dev/null
    fi
    SRV_PID=0
}
trap cleanup EXIT

echo "── 启动 px_serve daemon（max_conn=2；PX_GC_THRESHOLD=1000 precise 低阈值）"
rm -f /tmp/px_m98_s2_ready
PX_GC_THRESHOLD=1000 "$DIR/build/m98_s2_daemon" "$PORT" >/tmp/m98s2_srv.log 2>&1 </dev/null &
SRV_PID=$!
sleep 0.8
chk "daemon 存活" $(kill -0 $SRV_PID 2>/dev/null && echo 0 || echo 1)
# 就绪探测
READY=0
for i in $(seq 1 50); do
    R=$(curl -s -m 2 "http://127.0.0.1:$PORT/index.txt" 2>/dev/null)
    if [ "$R" = "OK" ]; then READY=1; break; fi
    sleep 0.2
done
chk "px_serve 就绪（index.txt=OK）" $([ "$READY" = "1" ] && echo 0 || echo 1)

echo "── Go 客户端场景执行（并发 slow / fast 饿死检测 / keep-alive 续处理 / big / alloc）"
T0=$(date +%s.%N)
timeout 120 /tmp/m98s2_go "127.0.0.1:$PORT" >/tmp/m98s2_go.out 2>&1
GRC=$?
T1=$(date +%s.%N)
WALL=$(awk -v a="$T0" -v b="$T1" 'BEGIN{printf "%.2f", b-a}')
cat /tmp/m98s2_go.out
grep -q "M98S2_CLIENT PASS=.* FAIL=0" /tmp/m98s2_go.out
chk "客户端场景全 PASS（wall=${WALL}s rc=$GRC）" $?

# 服务端访问日志对拍：请求条数 = 客户端 PASS 名次计数 >= 服务端应服务次数
N_REQ=$(grep -c '\[px-access\]' /tmp/m98s2_srv.log 2>/dev/null || echo 0)
chk "服务端访问日志 $N_REQ 条（20 slow+30 fast+2 ka slow+1 big+1 p+1 head+1 static+10 alloc+就绪 ≥ 66）" $([ "$N_REQ" -ge 66 ] && echo 0 || echo 1)

# 线程峰值采样（daemon 进程；协程模型：池2 + coro≤8 + 主/accept ≈ ≤14）
PEAK=0
for i in $(seq 1 30); do
    N=$(ps -L -p $SRV_PID -o lwp= 2>/dev/null | wc -l)
    [ "$N" -gt "$PEAK" ] && PEAK=$N
    sleep 0.1
done
chk "线程峰值 $PEAK ≤ 18（池 2 + coro ≤8 + 主线程/accept）" $([ "$PEAK" -le 18 ] && echo 0 || echo 1)

cleanup

echo ""
echo "════════ m98_s2(S2a): PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ] && exit 0 || exit 1
