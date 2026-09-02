#!/usr/bin/env bash
# ============================================================
# M52 QPACK 解码器流 ack 线上化端到端验证：服务端 + 客户端对拍
#   一条 QUIC 连接上双端 h3_conn_setup：请求/响应字段段走 QPACK 动态表；
#   M52 新增——解码器流真实承载 ack 指令：
#     * 接收方解码含动态引用的字段段（RIC>0）后，立即在解码器流发 Section Ack
#     * 发送方消费对端解码器流（Section Ack → KRC 推进，条目可安全驱逐）
#   断言：双端 dec_sects>=1（都发过 ack）、krc>=1（都收到过 ack）、字段还原正确。
#   编译模式（pxc build）+ 解释模式（pxi run）双模式：client 输出逐字节一致。
# ============================================================
set -u
cd "$(dirname "$0")/.."
B=examples/build

run_server_client() {
    local mode=$1 srv_bin=$2 cli_bin=$3 wait_s=$4
    echo "--- [$mode] server+client ---"
    bash -c "$srv_bin" >/tmp/m52_h3_srv_$mode.out 2>&1 &
    SRV_PID=$!
    sleep $wait_s
    bash -c "$cli_bin" >/tmp/m52_h3_cli_$mode.out 2>&1
    CLI_EXIT=$?
    wait $SRV_PID
    SRV_EXIT=$?
    local ok=1
    # 服务端断言
    grep -q "server: setup ok" /tmp/m52_h3_srv_$mode.out || { echo "[$mode] FAIL: server setup"; ok=0; }
    grep -q "server: req path=/ack-3 token=tok-m52-3" /tmp/m52_h3_srv_$mode.out || { echo "[$mode] FAIL: server 请求头还原"; ok=0; }
    grep -q "server: ack dec_sects=" /tmp/m52_h3_srv_$mode.out || { echo "[$mode] FAIL: server 无 ack 统计"; ok=0; }
    grep -q "server: done" /tmp/m52_h3_srv_$mode.out || { echo "[$mode] FAIL: server 未正常退出"; ok=0; }
    # 客户端断言（关键 ack 闭环行由 .px 内 assert 保证，此处抽查输出）
    grep -q "client: setup ok" /tmp/m52_h3_cli_$mode.out || { echo "[$mode] FAIL: client setup"; ok=0; }
    grep -q "client: resp n=3" /tmp/m52_h3_cli_$mode.out || { echo "[$mode] FAIL: client 第 3 轮响应缺失"; ok=0; }
    grep -q "client: ack dec_sects=" /tmp/m52_h3_cli_$mode.out || { echo "[$mode] FAIL: client 无 ack 统计"; ok=0; }
    grep -q "qpack decoder-stream ack roundtrip OK" /tmp/m52_h3_cli_$mode.out || { echo "[$mode] FAIL: client 断言未过"; ok=0; }
    grep -q "client: done" /tmp/m52_h3_cli_$mode.out || { echo "[$mode] FAIL: client 未正常退出"; ok=0; }
    if [ "$ok" = "1" ] && [ "$CLI_EXIT" = "0" ] && [ "$SRV_EXIT" = "0" ]; then
        echo "[$mode] QPACK 解码器流 ack 闭环 PASS"
        return 0
    else
        echo "[$mode] QPACK ack 闭环 FAIL（exit srv=$SRV_EXIT cli=$CLI_EXIT）"
        echo "--- server out ---"; cat /tmp/m52_h3_srv_$mode.out
        echo "--- client out ---"; cat /tmp/m52_h3_cli_$mode.out
        return 1
    fi
}

echo "=== M52 QPACK 解码器流 ack 线上化回环验证 ==="
ok=1
if [ "${PXC_SKIP_BUILD:-0}" != "1" ]; then
    ./tools/pxc build examples/m52_qpack_decack_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译失败"; ok=0; }
    ./tools/pxc build examples/m52_qpack_decack_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译失败"; ok=0; }
else
    echo "（跳过 pxc build，复用已有产物）"
fi
[ "$ok" = "1" ] || exit 1

# 编译模式
run_server_client "compiled" "$B/m52_qpack_decack_server" "$B/m52_qpack_decack_client" 2 || ok=0
# 解释模式
run_server_client "interp" "./tools/pxc run examples/m52_qpack_decack_server.px" "./tools/pxc run examples/m52_qpack_decack_client.px" 3 || ok=0

# 双模式 client 输出逐字节一致（只比 ack 统计以后部分，启动/时序行可能不同）
if [ "$ok" = "1" ]; then
    if diff -q /tmp/m52_h3_cli_compiled.out /tmp/m52_h3_cli_interp.out >/dev/null 2>&1; then
        echo "双模式 client 输出逐字节一致 PASS"
    else
        echo "双模式 client 输出不一致："
        diff /tmp/m52_h3_cli_compiled.out /tmp/m52_h3_cli_interp.out | head -20
        ok=0
    fi
fi

if [ "$ok" = "1" ]; then
    echo ""
    echo "══════════ M52 QPACK 解码器流 ack 闭环 全部 PASS ══════════"
    echo "（ack 统计：）"
    grep -h "ack dec_sects=" /tmp/m52_h3_srv_compiled.out /tmp/m52_h3_cli_compiled.out
else
    echo ""
    echo "M52 验证失败"
    exit 1
fi
