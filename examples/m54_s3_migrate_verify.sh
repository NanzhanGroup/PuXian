#!/usr/bin/env bash
# ============================================================
# M54 S3：连接迁移端到端验证（client 换源 + server path 跟随）
#   server: quic_h3_listen echo（托管 :18000）
#   client: M1 基准 → quic_migrate 换源端口 → M2 echo 从新源收到
#           （服务器回包已切到新地址 = 同连接续传、无重新握手）
# ============================================================
set -u
cd "$(dirname "$0")/.."
PXC=./tools/pxc
B=examples/build
rm -f /tmp/m54s3.ready

echo "== build =="
$PXC build examples/m54_s3_migrate_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译"; exit 1; }
$PXC build examples/m54_s3_migrate_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译"; exit 1; }

echo "== run server =="
$B/m54_s3_migrate_server >/tmp/m54s3_server.log 2>&1 &
SRV=$!
for i in $(seq 1 40); do
    [ -f /tmp/m54s3.ready ] && break
    sleep 0.3
done
if [ ! -f /tmp/m54s3.ready ]; then
    echo "❌ server 未就绪"; cat /tmp/m54s3_server.log
    kill $SRV 2>/dev/null; exit 1
fi
echo "server ready"

echo "== run client =="
$B/m54_s3_migrate_client >/tmp/m54s3_client.log 2>&1
CR=$?
echo "== client log =="
cat /tmp/m54s3_client.log
kill $SRV 2>/dev/null
wait $SRV 2>/dev/null

if [ $CR -ne 0 ]; then
    echo "❌ M54-S3: client 退出码 $CR"; exit 1
fi
if ! grep -q "client: ok" /tmp/m54s3_client.log; then
    echo "❌ M54-S3: client 未完成"; exit 1
fi
grep -q "after migrate  local=" /tmp/m54s3_client.log \
    && grep -q "post-migration echo ok" /tmp/m54s3_client.log \
    || { echo "❌ M54-S3: 断言缺失"; exit 1; }
echo "== server log =="
cat /tmp/m54s3_server.log
echo "✅ M54-S3: 连接迁移（client 换源 + server path 跟随）PASS"
