#!/usr/bin/env bash
# ============================================================
# M51 QPACK 会话接入线上端到端验证：服务端 + 客户端对拍
#   一条 QUIC 连接上：双端 h3_conn_setup 各开 3 条单向流（控制/编码器/解码器），
#   控制流 SETTINGS 协商（qcap=4096），QPACK 动态表指令经真实编码器单向流传输：
#   请求 1 插入动态表（对端编码器流字节 >0），请求 2 同头复用（零新增），
#   请求/响应字段双端还原正确。编译模式（pxc build）+ 解释模式（pxi run）
#   双模式：client 输出逐字节一致。
# ============================================================
set -u
cd "$(dirname "$0")/.."
B=examples/build

run_server_client() {
    local mode=$1 srv_bin=$2 cli_bin=$3 wait_s=$4
    echo "--- [$mode] server+client ---"
    bash -c "$srv_bin" >/tmp/m51_h3_srv_$mode.out 2>&1 &
    SRV_PID=$!
    # print 输出在重定向下全缓冲，无法实时 grep；固定等待服务端就绪
    sleep $wait_s
    bash -c "$cli_bin" >/tmp/m51_h3_cli_$mode.out 2>&1
    CLI_EXIT=$?
    wait $SRV_PID
    SRV_EXIT=$?
    local ok=1
    # 服务端断言
    grep -q "setup ok" /tmp/m51_h3_srv_$mode.out || { echo "[$mode] FAIL: server setup"; ok=0; }
    grep -q "req path=/wire-a token=tok-abcdef0123456789-http3-qpack-wire" /tmp/m51_h3_srv_$mode.out || { echo "[$mode] FAIL: server 请求头还原"; ok=0; }
    grep -q "qcap=4096" /tmp/m51_h3_srv_$mode.out || { echo "[$mode] FAIL: server SETTINGS 协商"; ok=0; }
    grep -q "server: responded stream" /tmp/m51_h3_srv_$mode.out || { echo "[$mode] FAIL: server 未响应"; ok=0; }
    grep -q "server: done" /tmp/m51_h3_srv_$mode.out || { echo "[$mode] FAIL: server 未正常退出"; ok=0; }
    # 客户端断言
    grep -q "setup ok ctrl=" /tmp/m51_h3_cli_$mode.out || { echo "[$mode] FAIL: client setup"; ok=0; }
    grep -q "qcap=4096" /tmp/m51_h3_cli_$mode.out || { echo "[$mode] FAIL: client SETTINGS 协商"; ok=0; }
    grep -q "qpack dynamic table over uni streams OK" /tmp/m51_h3_cli_$mode.out || { echo "[$mode] FAIL: client 断言未过"; ok=0; }
    grep -q "client: done" /tmp/m51_h3_cli_$mode.out || { echo "[$mode] FAIL: client 未正常退出"; ok=0; }
    if [ "$ok" = "1" ] && [ "$CLI_EXIT" = "0" ] && [ "$SRV_EXIT" = "0" ]; then
        echo "[$mode] QPACK 动态表走真实单向流 PASS"
        return 0
    else
        echo "[$mode] QPACK 线上接线 FAIL（exit srv=$SRV_EXIT cli=$CLI_EXIT）"
        echo "--- server out ---"; cat /tmp/m51_h3_srv_$mode.out
        echo "--- client out ---"; cat /tmp/m51_h3_cli_$mode.out
        return 1
    fi
}

echo "=== M51 QPACK 会话接入线上回环验证 ==="
ok=1
if [ "${PXC_SKIP_BUILD:-0}" != "1" ]; then
    ./tools/pxc build examples/m51_h3_qpack_wire_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译失败"; ok=0; }
    ./tools/pxc build examples/m51_h3_qpack_wire_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译失败"; ok=0; }
else
    echo "（跳过 pxc build，复用已有产物）"
fi
if [ "$ok" = "1" ]; then
    run_server_client "compiled" "$B/m51_h3_qpack_wire_server" "$B/m51_h3_qpack_wire_client" 2 || ok=0
fi
if [ "$ok" = "1" ] && [ "${RUN_INTERP:-1}" = "1" ]; then
    echo ""
    echo "--- 解释模式（bootstrap/pxi run） ---"
    run_server_client "interp" "./tools/pxc run examples/m51_h3_qpack_wire_server.px" "./tools/pxc run examples/m51_h3_qpack_wire_client.px" 3 || ok=0
    # client 输出逐字节一致校验
    if diff -q /tmp/m51_h3_cli_compiled.out /tmp/m51_h3_cli_interp.out >/dev/null 2>&1; then
        echo "client 双模式输出逐字节一致 PASS"
    else
        echo "client 双模式输出不一致 FAIL"
        diff /tmp/m51_h3_cli_compiled.out /tmp/m51_h3_cli_interp.out
        ok=0
    fi
fi
if [ "$ok" = "1" ]; then
    echo "=== M51 PASS ==="
else
    echo "=== M51 FAIL ==="
    exit 1
fi
