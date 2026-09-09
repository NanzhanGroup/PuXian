#!/bin/bash
# ============================================================
# M97-S3 verify.sh —— qg-issue 32（服务端 http_serve_unix keep-alive 长连生命周期）验证门
# ------------------------------------------------------------
#   1) Go http.Transport keep-alive 客户端复用同一 unix 连接，100 请求（/big 2MB 与
#      /ok 交替）全部成功、响应完整 → 修复后零悬挂零丢零截断
#      （/big > unix socket 发送缓冲 → 必触发非阻塞 EAGAIN/部分写 → px_send_all
#       全量写路径；修复前单次裸 send 截断 + IDLE 只等可读 → 剩余永不写出）
#   2) 连接数 == 1：http.Transport keep-alive 真实复用同一连接（专测长连生命周期）
#   3) 服务端 audit 行数 == 客户端成功数（100）→ 无丢无悬挂对拍闭合
# 退出码：0=全 PASS；非 0=有失败。
# 依赖：tools/px + Go（CGO_ENABLED=0 静态编译 keep-alive 客户端）。sock /tmp/m97s3.sock。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m97_s3"
SOCK=/tmp/m97s3.sock
REQ=100
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── 准备：build daemon（VM 轨含新 runtime）+ Go 客户端（静态）"
"$PX" build --no-quic "$DIR/m97_s3_daemon.px" >/tmp/m97s3_build.log 2>&1 || { echo "FAIL build daemon"; tail -5 /tmp/m97s3_build.log; exit 1; }
CGO_ENABLED=0 go build -o /tmp/m97s3_go_ka "$DIR/go_ka_client.go" >/tmp/m97s3_gobuild.log 2>&1 || { echo "FAIL go build"; tail -5 /tmp/m97s3_gobuild.log; exit 1; }
chk "build(daemon+go 静态客户端)" 0
file /tmp/m97s3_go_ka | grep -q "statically linked"
chk "Go 客户端静态链接" $?

SRV_PID=0
cleanup() { if [ -n "$SRV_PID" ] && [ "$SRV_PID" -gt 0 ] 2>/dev/null; then kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; fi; SRV_PID=0; }
trap cleanup EXIT

echo "── 启动 http_serve_unix daemon"
rm -f "$SOCK"
"$DIR/build/m97_s3_daemon" >/tmp/m97s3_srv.log 2>&1 </dev/null &
SRV_PID=$!
for i in $(seq 1 50); do
    [ -S "$SOCK" ] && break
    sleep 0.1
done
chk "unix sock 就绪" $([ -S "$SOCK" ] && echo 0 || echo 1)
# 就绪探测（服务端 worker 已 accept 循环）
sleep 0.3

echo "── Go keep-alive 客户端：$REQ 请求（/big 2MB 与 /ok 交替，单连接复用）"
T0=$(date +%s.%N)
timeout 90 /tmp/m97s3_go_ka "$SOCK" "$REQ" >/tmp/m97s3_go.out 2>&1
GRC=$?
T1=$(date +%s.%N)
WALL=$(awk -v a="$T0" -v b="$T1" 'BEGIN{printf "%.2f", b-a}')
grep -q "M97S3_CLIENT_OK_$REQ" /tmp/m97s3_go.out
chk "客户端 100/100 成功零悬挂零丢（wall=${WALL}s rc=$GRC）" $?
CONNS=$(grep -o 'conns=[0-9]*' /tmp/m97s3_go.out | head -1 | cut -d= -f2)
chk "keep-alive 单连接复用（conns=$CONNS == 1）" $([ "$CONNS" = "1" ] && echo 0 || echo 1)
cat /tmp/m97s3_go.out | head -8

AUDIT=$(grep -c '^AUDIT ' /tmp/m97s3_srv.log 2>/dev/null || echo 0)
chk "服务端 audit==100（$AUDIT 次请求全服务，无丢）" $([ "$AUDIT" = "$REQ" ] && echo 0 || echo 1)

cleanup

echo ""
echo "════════ m97_s3: PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ] && exit 0 || exit 1
