#!/usr/bin/env bash
# ============================================================
# M54 S1：TLS1.3 会话恢复（1-RTT resumption）端到端验证
#   server: quic_listen（stateless session ticket 已开启）
#   client: ① 全新握手 + quic_session_save 导出 session
#           ② quic_connect_resume 恢复 → quic_conn_resumed==true（核心）
# 验证点：同一 server 进程内，第二次连接确为 1-RTT 会话恢复而非全新握手，
#         且恢复连接上应用数据（echo）仍正常。
# ============================================================
set -u
cd "$(dirname "$0")/.."
PXC=./tools/pxc
B=examples/build
rm -f /tmp/m54s1.ready

echo "== build =="
$PXC build examples/m54_s1_resume_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译"; exit 1; }
$PXC build examples/m54_s1_resume_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译"; exit 1; }

echo "== run server =="
$B/m54_s1_resume_server >/tmp/m54s1_server.log 2>&1 &
SRV=$!
for i in $(seq 1 30); do
    [ -f /tmp/m54s1.ready ] && break
    sleep 0.3
done
if [ ! -f /tmp/m54s1.ready ]; then
    echo "❌ server 未就绪"; cat /tmp/m54s1_server.log
    kill $SRV 2>/dev/null; exit 1
fi
echo "server ready"

echo "== run client =="
$B/m54_s1_resume_client >/tmp/m54s1_client.log 2>&1
CR=$?
echo "== client log =="
cat /tmp/m54s1_client.log
kill $SRV 2>/dev/null
wait $SRV 2>/dev/null

if [ $CR -ne 0 ]; then
    echo "❌ M54-S1: client 退出码 $CR"; exit 1
fi
if ! grep -q "client: ok" /tmp/m54s1_client.log; then
    echo "❌ M54-S1: client 未完成"; exit 1
fi
if ! grep -q "resumed=true" /tmp/m54s1_client.log; then
    echo "❌ M54-S1: 第二次连接非会话恢复（resumed=false）"; exit 1
fi
echo "== server log =="
cat /tmp/m54s1_server.log
echo "✅ M54-S1: TLS1.3 会话恢复（1-RTT resumption）PASS"
