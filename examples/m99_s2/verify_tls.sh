#!/bin/bash
# ============================================================
# M99-S2 TLS 验证 —— px_serve TLS 连接事件化 IDLE（docs/M99_PLAN.md §S3）
#   1) openssl 生成自签证书 → m99_s2_tls_daemon（tls_server + px_serve max_conn=2）
#   2) 顺序建 20 TLS keep-alive conns × /fast 全成功（事件化前 max_conn=2 池被 TLS 空闲占死
#      → 第 3+ 悬挂；M99 空闲交 IDLE → 全即时建连保持）【注：并发 TLS 握手既有缺陷先于 M99，
#      顺序建连规避——聚焦 M99 的已建 TLS 连接空闲事件化，qg 二期候选另行排查】
#   3) 线程峰值 ≤ 18（TLS 空闲连接 0 占 worker：池2+coro+事件循环+主）
#   4) TLS 空闲后 20 conns 并发续请求全成功（IDLE 唤醒 + mbedtls 缓冲探测，不悬挂不丢）
#   5) 优雅关闭干净退出
# 退出码：0=全 PASS。依赖：openssl + tools/px + Go（CGO_ENABLED=0）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m99_s2"
PORT=18199
PASS=0 FAIL=0
CERT=/tmp/px_m99_cert.pem
KEY=/tmp/px_m99_key.pem

chk() { if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi }

echo "── 准备：自签证书 + TLS daemon + Go TLS 客户端（静态）"
openssl req -x509 -newkey rsa:2048 -nodes -keyout "$KEY" -out "$CERT" -days 1 -subj "/CN=localhost" >/dev/null 2>&1
chk "openssl 自签证书" $?
"$PX" build --no-quic "$DIR/m99_s2_tls_daemon.px" >/tmp/m99tls_build.log 2>&1 || { echo "FAIL build tls daemon"; tail -8 /tmp/m99tls_build.log; exit 1; }
CGO_ENABLED=0 go build -o /tmp/m99tls_go "$DIR/go_tls_client.go" >/tmp/m99tls_gobuild.log 2>&1 || { echo "FAIL go build"; tail -8 /tmp/m99tls_gobuild.log; exit 1; }
chk "build(tls daemon + go 静态)" 0

SRV_PID=0
cleanup() {
    if [ -n "$SRV_PID" ] && [ "$SRV_PID" -gt 0 ] 2>/dev/null; then
        kill -9 $SRV_PID 2>/dev/null
        wait $SRV_PID 2>/dev/null
    fi
    SRV_PID=0
}
trap cleanup EXIT

echo "── 启动 TLS px_serve daemon（max_conn=2）"
"$DIR/build/m99_s2_tls_daemon" "$PORT" "$CERT" "$KEY" >/tmp/m99tls_srv.log 2>&1 </dev/null &
SRV_PID=$!
sleep 0.8
chk "TLS daemon 存活" $(kill -0 $SRV_PID 2>/dev/null && echo 0 || echo 1)
READY=0
for i in $(seq 1 50); do
    R=$(curl -sk -m 2 "https://127.0.0.1:$PORT/index.txt" 2>/dev/null)
    if [ "$R" = "TLS-OK" ]; then READY=1; break; fi
    sleep 0.2
done
chk "TLS px_serve 就绪（https index.txt=TLS-OK）" $([ "$READY" = "1" ] && echo 0 || echo 1)

echo "── Go TLS 客户端场景（P1 并发20 / P2 空闲 / P3 续请求）"
rm -f /tmp/m99tls_threads.txt
(
    while kill -0 $SRV_PID 2>/dev/null; do
        N=$(ps -L -p $SRV_PID -o lwp= 2>/dev/null | wc -l)
        echo "$N" >>/tmp/m99tls_threads.txt
        sleep 0.1
    done
) &
timeout 60 /tmp/m99tls_go "127.0.0.1:$PORT" >/tmp/m99tls_go.out 2>&1
GRC=$?
cat /tmp/m99tls_go.out
grep -q "M99S2_TLS PASS=.* FAIL=0" /tmp/m99tls_go.out
chk "TLS 客户端场景全 PASS（rc=$GRC）" $?
PEAK=$(sort -n /tmp/m99tls_threads.txt 2>/dev/null | tail -1)
PEAK=${PEAK:-0}
chk "线程峰值 $PEAK ≤ 18（20 TLS 空闲连接不占 worker）" $([ "$PEAK" -le 18 ] && echo 0 || echo 1)

echo "── 优雅关闭"
kill -TERM $SRV_PID 2>/dev/null
WAITED=0
for i in $(seq 1 50); do
    kill -0 $SRV_PID 2>/dev/null || { WAITED=1; break; }
    sleep 0.1
done
if [ "$WAITED" = "0" ]; then
    kill -9 $SRV_PID 2>/dev/null
    wait $SRV_PID 2>/dev/null
    chk "TLS daemon 优雅关闭退出（≤5s）" 1
else
    wait $SRV_PID 2>/dev/null
    chk "TLS daemon 优雅关闭退出" 0
fi
grep -q "优雅关闭完成（在途 0）" /tmp/m99tls_srv.log 2>/dev/null
chk "TLS 优雅关闭日志：在途 0" $?

cleanup
echo ""
echo "════════ m99_s2(TLS): PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ] && exit 0 || exit 1
