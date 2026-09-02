#!/usr/bin/env bash
# ============================================================
# M50 HTTP/3 多路复用端到端验证：服务端 + 客户端对拍
#   一条 QUIC 连接上 3 条双向流并发 3 个请求（/alpha /beta /gamma），
#   服务端按流分拣、交错响应，客户端逐流收响应并断言一一对应（无串扰）。
#   编译模式（pxc build）+ 解释模式（pxi run）双模式：client 输出逐字节一致。
# ============================================================
set -u
cd "$(dirname "$0")/.."
B=examples/build

run_server_client() {
    local mode=$1 srv_bin=$2 cli_bin=$3
    echo "--- [$mode] server+client ---"
    bash -c "$srv_bin" >/tmp/m50_h3_srv_$mode.out 2>&1 &
    SRV_PID=$!
    # 等服务端完成 listen（轮询输出标志，最多 ~10s）——比固定 sleep 抗启动时序
    local w=0
    while [ $w -lt 100 ]; do
        grep -q "listening 17998" /tmp/m50_h3_srv_$mode.out 2>/dev/null && break
        sleep 0.1; w=$((w+1))
    done
    sleep 0.5
    bash -c "$cli_bin" >/tmp/m50_h3_cli_$mode.out 2>&1
    CLI_EXIT=$?
    wait $SRV_PID
    SRV_EXIT=$?
    # 服务端：3 个请求都收到并按流处理（顺序因到达时序可能不同 → grep 断言）
    local srv_ok=1
    grep -q "path=/alpha" /tmp/m50_h3_srv_$mode.out || { echo "[$mode] FAIL: server 未处理 /alpha"; srv_ok=0; }
    grep -q "path=/beta" /tmp/m50_h3_srv_$mode.out || { echo "[$mode] FAIL: server 未处理 /beta"; srv_ok=0; }
    grep -q "path=/gamma" /tmp/m50_h3_srv_$mode.out || { echo "[$mode] FAIL: server 未处理 /gamma"; srv_ok=0; }
    grep -q "server: responded stream" /tmp/m50_h3_srv_$mode.out || { echo "[$mode] FAIL: server 未全部响应"; srv_ok=0; }
    grep -q "server: done" /tmp/m50_h3_srv_$mode.out || { echo "[$mode] FAIL: server 未正常退出"; srv_ok=0; }
    # 客户端：固定顺序读取 → 3 条响应断言
    grep -q "body=\[echo-mux:/alpha:stream-" /tmp/m50_h3_cli_$mode.out || { echo "[$mode] FAIL: client /alpha 响应缺失"; srv_ok=0; }
    grep -q "body=\[echo-mux:/beta:stream-" /tmp/m50_h3_cli_$mode.out || { echo "[$mode] FAIL: client /beta 响应缺失"; srv_ok=0; }
    grep -q "body=\[echo-mux:/gamma:stream-" /tmp/m50_h3_cli_$mode.out || { echo "[$mode] FAIL: client /gamma 响应缺失"; srv_ok=0; }
    grep -q "3 responses multiplexed OK" /tmp/m50_h3_cli_$mode.out || { echo "[$mode] FAIL: client 断言未过"; srv_ok=0; }
    if [ "$srv_ok" = "1" ] && [ "$CLI_EXIT" = "0" ] && [ "$SRV_EXIT" = "0" ]; then
        echo "[$mode] HTTP/3 多路复用 PASS"
        return 0
    else
        echo "[$mode] HTTP/3 多路复用 FAIL（exit srv=$SRV_EXIT cli=$CLI_EXIT）"
        echo "--- server out ---"; cat /tmp/m50_h3_srv_$mode.out
        echo "--- client out ---"; cat /tmp/m50_h3_cli_$mode.out
        return 1
    fi
}

echo "=== M50 HTTP/3 多路复用回环验证 ==="
ok=1
# 编译模式：先构建（PXC_SKIP_BUILD=1 跳过，产物已存在时快速复跑）
if [ "${PXC_SKIP_BUILD:-0}" != "1" ]; then
    ./tools/pxc build examples/m50_h3_mux_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译失败"; ok=0; }
    ./tools/pxc build examples/m50_h3_mux_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译失败"; ok=0; }
else
    echo "（跳过 pxc build，复用已有产物）"
fi
if [ "$ok" = "1" ]; then
    run_server_client comp "$B/m50_h3_mux_server" "$B/m50_h3_mux_client" || ok=0
fi
# 解释模式：pxi run
if [ "$ok" = "1" ]; then
    run_server_client interp "./bootstrap/pxi run examples/m50_h3_mux_server.px" \
        "./bootstrap/pxi run examples/m50_h3_mux_client.px" || ok=0
fi
# 双模式 client 输出逐字节一致
if [ "$ok" = "1" ]; then
    if diff -q /tmp/m50_h3_cli_comp.out /tmp/m50_h3_cli_interp.out >/dev/null 2>&1; then
        echo "双模式 client 输出逐字节一致 ✅"
    else
        echo "FAIL: 双模式 client 输出不一致"
        diff /tmp/m50_h3_cli_comp.out /tmp/m50_h3_cli_interp.out
        ok=0
    fi
fi
if [ "$ok" = "1" ]; then
    echo "M50 HTTP/3 多路复用回环验证 PASS（编译+解释双模式）"
    exit 0
else
    echo "M50 HTTP/3 多路复用回环验证 FAIL"
    exit 1
fi
