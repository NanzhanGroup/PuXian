#!/usr/bin/env bash
# ============================================================
# M47 HTTP/3 语义层端到端验证：服务端 + 客户端对拍
#   QPACK 编码 :method/:scheme/:authority/:path + 自定义头
#   → HEADERS/DATA 帧 → QUIC 双向流 → 服务端解码回 200 + body
#   编译模式（pxc build）+ 解释模式（pxi run）双模式一致
# ============================================================
set -u
cd "$(dirname "$0")/.."
B=examples/build

run_server_client() {
    local mode=$1 srv_bin=$2 cli_bin=$3
    echo "--- [$mode] server+client ---"
    bash -c "$srv_bin" >/tmp/m47_h3_srv_$mode.out 2>&1 &
    SRV_PID=$!
    sleep 1
    bash -c "$cli_bin" >/tmp/m47_h3_cli_$mode.out 2>&1
    CLI_EXIT=$?
    wait $SRV_PID
    SRV_EXIT=$?
    if grep -q "server: method=GET path=/hello" /tmp/m47_h3_srv_$mode.out \
       && grep -q "server: x-test=1" /tmp/m47_h3_srv_$mode.out \
       && grep -q "client: status=200" /tmp/m47_h3_cli_$mode.out \
       && grep -q "client: body=\[echo-h3:/hello\]" /tmp/m47_h3_cli_$mode.out \
       && [ "$CLI_EXIT" = "0" ] && [ "$SRV_EXIT" = "0" ]; then
        echo "[$mode] HTTP/3 回环 PASS"
        return 0
    else
        echo "[$mode] HTTP/3 回环 FAIL（exit srv=$SRV_EXIT cli=$CLI_EXIT）"
        echo "--- server out ---"; cat /tmp/m47_h3_srv_$mode.out
        echo "--- client out ---"; cat /tmp/m47_h3_cli_$mode.out
        return 1
    fi
}

echo "=== M47 HTTP/3 语义层回环验证 ==="
ok=1
# 编译模式：先构建
./tools/pxc build examples/m47_h3_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译失败"; ok=0; }
./tools/pxc build examples/m47_h3_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译失败"; ok=0; }
if [ "$ok" = "1" ]; then
    run_server_client comp "$B/m47_h3_server" "$B/m47_h3_client" || ok=0
fi
# 解释模式：pxi run
if [ "$ok" = "1" ]; then
    run_server_client interp "./bootstrap/pxi run examples/m47_h3_server.px" \
        "./bootstrap/pxi run examples/m47_h3_client.px" || ok=0
fi
if [ "$ok" = "1" ]; then
    echo "M47 HTTP/3 语义层回环验证 PASS（编译+解释双模式）"
    exit 0
else
    echo "M47 HTTP/3 语义层回环验证 FAIL"
    exit 1
fi
