#!/usr/bin/env bash
# ============================================================
# M53 S1：h3 server 多连接托管冒烟验证
#   server: quic_h3_listen（收包路由 + 自动 accept + 默认 echo）
#   client: 8 进程并发独立 QUIC 连接 → echo 校验
# 验证点：多连接收包路由正确（无抢包/串扰）、握手+收发全通。
# ============================================================
set -u
cd "$(dirname "$0")/.."
PXC=./tools/pxc
rm -f /tmp/m53s1.ready

echo "== build =="
$PXC build examples/m53_s1_h3echo_server.px || exit 1
$PXC build examples/m53_s1_h3echo_client.px || exit 1

echo "== run server =="
./examples/build/m53_s1_h3echo_server >/tmp/m53s1_server.log 2>&1 &
SRV=$!
for i in $(seq 1 30); do
    [ -f /tmp/m53s1.ready ] && break
    sleep 0.3
done
if [ ! -f /tmp/m53s1.ready ]; then
    echo "❌ server 未就绪"; cat /tmp/m53s1_server.log
    kill $SRV 2>/dev/null; exit 1
fi
echo "server ready"

echo "== 8 并发 client =="
for i in $(seq 1 8); do
    ./examples/build/m53_s1_h3echo_client >/tmp/m53s1_client_$i.log 2>&1 &
done
wait

fail=0
for i in $(seq 1 8); do
    if grep -q "client: ok" /tmp/m53s1_client_$i.log; then
        echo "client$i PASS"
    else
        echo "client$i FAIL"; cat /tmp/m53s1_client_$i.log; fail=1
    fi
done

kill $SRV 2>/dev/null
wait $SRV 2>/dev/null
echo "== server log =="
cat /tmp/m53s1_server.log

if [ $fail -eq 0 ]; then
    echo "✅ M53-S1: 多连接托管全部 PASS"
else
    echo "❌ M53-S1: 存在 FAIL"
    exit 1
fi
