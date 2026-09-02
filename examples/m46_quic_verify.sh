#!/usr/bin/env bash
# M46 QUIC 回环端到端验证：服务端+客户端，QUIC 握手 + stream 0 收发 + echo
set -u
cd "$(dirname "$0")/.."
B=examples/build
echo "=== M46 QUIC 回环验证 ==="
# 启动服务端（后台）
"$B/m46_quic_server" >/tmp/m46_srv.out 2>&1 &
SRV_PID=$!
sleep 1
# 启动客户端
"$B/m46_quic_client" >/tmp/m46_cli.out 2>&1
CLI_EXIT=$?
wait $SRV_PID
SRV_EXIT=$?
echo "--- server out ---"
cat /tmp/m46_srv.out
echo "--- client out ---"
cat /tmp/m46_cli.out
echo "--- exit: server=$SRV_EXIT client=$CLI_EXIT ---"
# 断言
ok=1
grep -q "server: recv \[hello-quic-42\]" /tmp/m46_srv.out || { echo "FAIL: server 未收到 hello-quic-42"; ok=0; }
grep -q "client: recv \[echo:hello-quic-42\]" /tmp/m46_cli.out || { echo "FAIL: client 未收到 echo:hello-quic-42"; ok=0; }
[ "$CLI_EXIT" = "0" ] || { echo "FAIL: client 退出码非 0"; ok=0; }
[ "$SRV_EXIT" = "0" ] || { echo "FAIL: server 退出码非 0"; ok=0; }
if [ "$ok" = "1" ]; then
    echo "M46 QUIC 回环验证 PASS（QUIC 握手 + 双向数据收发 + echo 对拍）"
    exit 0
else
    echo "M46 QUIC 回环验证 FAIL"
    exit 1
fi
