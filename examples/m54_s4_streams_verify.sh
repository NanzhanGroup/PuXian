#!/usr/bin/env bash
# ============================================================
# M54 S4：BLOCKED_STREAMS 流控协商端到端验证
#   server: demo accept，上限 2 → accept 单连接 echo + EXTEND 放行
#   client: 开流 0/4 成功 → 开流 8 得 -206（阻塞）→ server extend +4
#           → 重试开流 8 成功 → 新流数据往返
# ============================================================
set -u
cd "$(dirname "$0")/.."
PXC=./tools/pxc
B=examples/build
rm -f /tmp/m54s4.ready

echo "== build =="
$PXC build examples/m54_s4_streams_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译"; exit 1; }
$PXC build examples/m54_s4_streams_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译"; exit 1; }

echo "== run server =="
$B/m54_s4_streams_server >/tmp/m54s4_server.log 2>&1 &
SRV=$!
for i in $(seq 1 40); do
    [ -f /tmp/m54s4.ready ] && break
    sleep 0.3
done
if [ ! -f /tmp/m54s4.ready ]; then
    echo "❌ server 未就绪"; cat /tmp/m54s4_server.log
    kill $SRV 2>/dev/null; exit 1
fi
echo "server ready"

echo "== run client =="
$B/m54_s4_streams_client >/tmp/m54s4_client.log 2>&1
CR=$?
echo "== client log =="
cat /tmp/m54s4_client.log
kill $SRV 2>/dev/null
wait $SRV 2>/dev/null

if [ $CR -ne 0 ]; then
    echo "❌ M54-S4: client 退出码 $CR"; exit 1
fi
if ! grep -q "client: ok" /tmp/m54s4_client.log; then
    echo "❌ M54-S4: client 未完成"; exit 1
fi
grep -q -- "open 3rd stream -> rv=-206" /tmp/m54s4_client.log \
    && grep -q "retry open 3rd stream -> sid=" /tmp/m54s4_client.log \
    || { echo "❌ M54-S4: 断言缺失"; exit 1; }
echo "== server log =="
cat /tmp/m54s4_server.log
echo "✅ M54-S4: BLOCKED_STREAMS 流控协商 PASS"
