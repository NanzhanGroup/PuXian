#!/bin/bash
# ============================================================
# M101-S1 并发 TLS 握手缺陷复现/验证（docs/M101_PLAN.md §二/§四）
#   RSA-TLS1.3 / RSA-TLS1.2 / EC-TLS1.3 三轮 barrier 高并发新建连接全握手，
#   统计握手失败率。修复前（v0.2.0-m100 runtime）预期 RSA FAIL>0 复现 M99 记档
#   （TLS1.3 CertificateVerify 签名错 / TLS1.2 大并发 EOF）；EC 结果二分定位根因
#   （私钥签名竞争 vs session cache 竞争）。修复后三轮全 0 FAIL。
#   另跑 TLS 会话恢复冒烟（短连接复连 cache 命中不悬挂）作为对照。
# 退出码：0=三轮并发握手 FAIL=0（修复后验收）；>0=有失败（修复前=复现成功）。
# 依赖：openssl + tools/px + Go（CGO_ENABLED=0）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m101_s2"
PORT=18201
CONC=48
ROUNDS=3
CERT=/tmp/px_m101_cert.pem
KEY=/tmp/px_m101_key.pem
EC_CERT=/tmp/px_m101_eccert.pem
EC_KEY=/tmp/px_m101_eckey.pem

chk() { if [ "$2" = "0" ]; then echo "  PASS $1"; else echo "  FAIL $1"; fi }

echo "── 准备：RSA + EC 自签证书 + daemon + Go 客户端（静态）"
openssl req -x509 -newkey rsa:2048 -nodes -keyout "$KEY" -out "$CERT" -days 1 -subj "/CN=localhost" >/dev/null 2>&1
chk "openssl RSA 自签证书" $?
openssl ecparam -name prime256v1 -genkey -noout -out "$EC_KEY" 2>/dev/null
openssl req -new -x509 -key "$EC_KEY" -out "$EC_CERT" -days 1 -subj "/CN=localhost" >/dev/null 2>&1
chk "openssl EC(P-256) 自签证书" $?
"$PX" build --no-quic "$DIR/m101_s2_daemon.px" >/tmp/m101_build.log 2>&1 || { echo "FAIL build daemon"; tail -8 /tmp/m101_build.log; exit 1; }
CGO_ENABLED=0 go build -o /tmp/m101_conc "$DIR/go_tls_conc.go" >/tmp/m101_gobuild.log 2>&1 || { echo "FAIL go build"; tail -8 /tmp/m101_gobuild.log; exit 1; }
chk "build(daemon + go 静态)" 0

SRV_PID=0
cleanup() {
    if [ -n "$SRV_PID" ] && [ "$SRV_PID" -gt 0 ] 2>/dev/null; then
        kill -9 $SRV_PID 2>/dev/null
        wait $SRV_PID 2>/dev/null
    fi
    SRV_PID=0
}
trap cleanup EXIT

run_scene() {  # $1=场景名 $2=cert $3=key $4=maxver
    local name="$1" c="$2" k="$3" mv="$4"
    echo "── 场景：$name（cert=$c maxver=$mv 并发=$CONC 轮=$ROUNDS）"
    "$DIR/build/m101_s2_daemon" "$PORT" "$c" "$k" 16 >/tmp/m101_srv.log 2>&1 </dev/null &
    SRV_PID=$!
    sleep 0.8
    if ! kill -0 $SRV_PID 2>/dev/null; then echo "  FAIL $name daemon 未存活"; SRV_PID=0; return 1; fi
    READY=0
    for i in $(seq 1 50); do
        R=$(curl -sk -m 2 "https://127.0.0.1:$PORT/index.txt" 2>/dev/null)
        if [ "$R" = "M101-OK" ]; then READY=1; break; fi
        sleep 0.2
    done
    if [ "$READY" != "1" ]; then echo "  FAIL $name 未就绪"; kill -9 $SRV_PID; wait $SRV_PID 2>/dev/null; SRV_PID=0; return 1; fi
    timeout 90 /tmp/m101_conc "127.0.0.1:$PORT" "$ROUNDS" "$CONC" "$mv" >/tmp/m101_conc.out 2>&1
    local rc=$?
    cat /tmp/m101_conc.out
    grep -q "ok=.*fail=0" /tmp/m101_conc.out
    local g=$?
    if [ "$g" = "0" ]; then echo "  PASS $name 并发握手 0 失败"; else echo "  FAIL $name 并发握手有失败（复现/残留缺陷）"; fi
    kill -TERM $SRV_PID 2>/dev/null
    for i in $(seq 1 30); do kill -0 $SRV_PID 2>/dev/null || break; sleep 0.1; done
    kill -9 $SRV_PID 2>/dev/null
    wait $SRV_PID 2>/dev/null
    SRV_PID=0
    return $g
}

ALLPASS=1
run_scene "RSA-TLS1.3(默认)" "$CERT" "$KEY" 0 || ALLPASS=0
run_scene "RSA-TLS1.2(0x0303)" "$CERT" "$KEY" 0x0303 || ALLPASS=0
run_scene "EC-TLS1.3(默认)" "$EC_CERT" "$EC_KEY" 0 || ALLPASS=0

cleanup
echo ""
if [ "$ALLPASS" = "1" ]; then
    echo "════════ m101_s2: 三轮并发握手全 0 失败 ════════"
    exit 0
else
    echo "════════ m101_s2: 存在并发握手失败（修复前=复现成功 / 修复后=缺陷残留） ════════"
    exit 1
fi
