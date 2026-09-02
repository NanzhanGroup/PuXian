#!/usr/bin/env bash
# ============================================================
# M54 S2：0-RTT early data 端到端验证
#   server: quic_h3_listen(echo, :17998) + h3_server_listen_stateless(:17999)
#   client: ① 传输层 0-RTT：connect_0rtt 返回时握手未完成 + 立即 send 成功 + echo 正确
#           ② H3 0-RTT 子集：0-RTT 连接上静态表 GET → 200 "0rtt-get:200"
# ============================================================
set -u
cd "$(dirname "$0")/.."
PXC=./tools/pxc
B=examples/build
rm -f /tmp/m54s2.ready

echo "== build =="
$PXC build examples/m54_s2_0rtt_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译"; exit 1; }
$PXC build examples/m54_s2_0rtt_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译"; exit 1; }

echo "== run server =="
$B/m54_s2_0rtt_server >/tmp/m54s2_server.log 2>&1 &
SRV=$!
for i in $(seq 1 40); do
    [ -f /tmp/m54s2.ready ] && break
    sleep 0.3
done
if [ ! -f /tmp/m54s2.ready ]; then
    echo "❌ server 未就绪"; cat /tmp/m54s2_server.log
    kill $SRV 2>/dev/null; exit 1
fi
echo "server ready"

echo "== run client =="
$B/m54_s2_0rtt_client >/tmp/m54s2_client.log 2>&1
CR=$?
echo "== client log =="
cat /tmp/m54s2_client.log
kill $SRV 2>/dev/null
wait $SRV 2>/dev/null

if [ $CR -ne 0 ]; then
    echo "❌ M54-S2: client 退出码 $CR"; exit 1
fi
if ! grep -q "client: ok" /tmp/m54s2_client.log; then
    echo "❌ M54-S2: client 未完成"; exit 1
fi
grep -q "handshake_done=false" /tmp/m54s2_client.log \
    && grep -q "A: transport 0-RTT PASS" /tmp/m54s2_client.log \
    && grep -q "B: h3 0-RTT PASS" /tmp/m54s2_client.log \
    || { echo "❌ M54-S2: 断言缺失"; exit 1; }
echo "== server log =="
cat /tmp/m54s2_server.log
echo "✅ M54-S2: 0-RTT early data（传输层 + H3 静态表子集）PASS"
