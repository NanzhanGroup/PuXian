#!/bin/bash
# M82 http_serve_unix 专项验证（Issue 15 GAP-SRV-1）：unix socket HTTP 服务端 native
# 覆盖：① 进程内自检（http_unix 客户端 → spawn http_serve_unix）
#       ② curl --unix-socket 冒烟（/health GET、/echo POST、/remote）
#       ③ sock 文件权限 0600
#       ④ 残留 sock 文件自动清理（异常退出遗留 → unlink 后 bind 成功）
#       ⑤ 客户端断连容忍（每次请求独立连接，服务不崩，续连仍通）
#       ⑥ 同 handler TCP http_serve 双跑对拍（/tcp 路由）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
SOCK=/tmp/m82_puxian.sock
PORT=18083

echo "== [1/2] 编译（--no-quic 编译模式）=="
$PX build --no-quic serve_daemon.px >/dev/null 2>&1 || { echo "FAIL build"; exit 1; }

echo "== [2/2] 运行冒烟 =="
# ② 前置：制造"残留 sock 文件"（普通文件占位 = 上次异常退出遗留形态）→ serve 应 unlink 后 bind 成功
rm -f "$SOCK"
echo "residue" > "$SOCK"   # 残留占位（非 socket 文件，bind 前必须清理）
./build/serve_daemon > /tmp/m82_daemon.log 2>&1 &
DAEMON=$!
# 等 sock 出现（自检也在此窗口完成）
for i in $(seq 1 50); do [ -S "$SOCK" ] && break; sleep 0.1; done
[ -S "$SOCK" ] || { echo "FAIL sock 未出现（残留清理/bind 失败）"; kill $DAEMON 2>/dev/null; exit 1; }
echo "PASS 残留 sock 清理 + bind 成功（$SOCK 为 socket）"

# ① 进程内自检输出（daemon 需 sleep(600)+连接处理，轮询等待 ≤5s）
SC=0
for i in $(seq 1 50); do
    if grep -q "SELFCHECK-PASS" /tmp/m82_daemon.log 2>/dev/null; then SC=1; break; fi
    sleep 0.1
done
if [ "$SC" != "1" ]; then
    echo "FAIL 进程内自检未通过"; cat /tmp/m82_daemon.log; kill $DAEMON 2>/dev/null; exit 1
fi
echo "PASS 进程内自检（http_unix → http_serve_unix）"

# ② curl --unix-socket 冒烟
H=$(curl -s --max-time 3 --unix-socket "$SOCK" http://localhost/health)
echo "$H" | grep -q "http_serve_unix" || { echo "FAIL curl /health: $H"; kill $DAEMON 2>/dev/null; exit 1; }
echo "PASS curl --unix-socket GET /health"

E=$(curl -s --max-time 3 --unix-socket "$SOCK" -X POST --data 'hello-curl' http://localhost/echo)
[ "$E" = "echo:hello-curl" ] || { echo "FAIL curl POST /echo: $E"; kill $DAEMON 2>/dev/null; exit 1; }
echo "PASS curl --unix-socket POST /echo"

R=$(curl -s --max-time 3 --unix-socket "$SOCK" http://localhost/remote)
[ "$R" = "remote=unix" ] || { echo "FAIL curl /remote（期望 remote=unix）: $R"; kill $DAEMON 2>/dev/null; exit 1; }
echo "PASS curl /remote = unix（AF_UNIX remote 兼容）"

# ③ 权限 0600
M=$(stat -c '%a' "$SOCK" 2>/dev/null)
[ "$M" = "600" ] || { echo "FAIL sock 权限（期望 600 实得 $M）"; kill $DAEMON 2>/dev/null; exit 1; }
echo "PASS sock 权限 0600"

# ⑤ 客户端断连容忍（curl 每次即断；等 0.5s 后服务应仍在，续连仍通）
sleep 0.5
kill -0 $DAEMON 2>/dev/null || { echo "FAIL daemon 因断连崩溃"; exit 1; }
H2=$(curl -s --max-time 3 --unix-socket "$SOCK" http://localhost/health)
echo "$H2" | grep -q "http_serve_unix" || { echo "FAIL 断连后续连 /health"; kill $DAEMON 2>/dev/null; exit 1; }
echo "PASS 客户端断连容忍（续连仍通）"

# ⑥ 同 handler TCP http_serve 双跑对拍
T=$(curl -s --max-time 3 http://127.0.0.1:$PORT/tcp)
[ "$T" = "tcp-ok" ] || { echo "FAIL TCP 双跑 /tcp: $T"; kill $DAEMON 2>/dev/null; exit 1; }
echo "PASS TCP http_serve 同 handler 双跑"

kill $DAEMON 2>/dev/null
rm -f "$SOCK"
echo "m82_http_serve_unix verify done"
exit 0
