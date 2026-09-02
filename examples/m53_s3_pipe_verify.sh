#!/usr/bin/env bash
# ============================================================
# M53 S3：HTTP/3 接入公共 HTTP 管道 —— 端到端验证（编译模式）
#   服务端同进程双栈：px_serve（HTTP/1.1 TCP :18010）+ h3_server_listen
#   （HTTP/3 UDP :18011，管道托管）。客户端开 4 条 QUIC 连接并发连 H3，
#   断言：路由 handler / :id+query / 静态 / 404 / 403 全部走公共管道且
#   响应与 HTTP/1.1 一致（curl 对照同一 route 表 / docroot 的输出）。
#   注：h3_server_listen 为 M53-S3 新 API，pxi 解释器在 S5 自举重建前
#   不含该内置 —— 本验证跑编译模式（与 m53_s1 同策略）。
# ============================================================
set -u
cd "$(dirname "$0")/.."
B=examples/build
SRV_BIN=$B/m53_s3_pipe_server
CLI_BIN=$B/m53_s3_pipe_client

echo "=== M53 S3 HTTP/3 公共管道 端到端验证（编译模式） ==="
ok=1
if [ "${PXC_SKIP_BUILD:-0}" != "1" ]; then
    ./tools/pxc build examples/m53_s3_pipe_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译失败"; ok=0; }
    ./tools/pxc build examples/m53_s3_pipe_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译失败"; ok=0; }
else
    echo "（跳过 pxc build，复用已有产物）"
fi
[ "$ok" = "1" ] || exit 1

http11_assert() {
    local desc=$1 want=$2 url=$3
    local got
    got=$(curl -s --path-as-is -m 5 "$url" 2>/dev/null)
    if [ "$got" = "$want" ]; then
        echo "[HTTP/1.1] $desc PASS"
    else
        echo "[HTTP/1.1] $desc FAIL want=[$want] got=[$got]"
        ok=0
    fi
}

rm -f /tmp/m53s3.ready
"$SRV_BIN" >/tmp/m53s3_srv.out 2>&1 &
SRV_PID=$!
waited=0
while [ ! -f /tmp/m53s3.ready ] && [ $waited -lt 20 ]; do sleep 1; waited=$((waited+1)); done
if [ ! -f /tmp/m53s3.ready ]; then
    echo "FAIL: server 未就绪"; cat /tmp/m53s3_srv.out; kill $SRV_PID 2>/dev/null; exit 1
fi
rm -f /tmp/m53s3.ready
echo "server ready"

# ---- HTTP/1.1 对照（curl → px_serve 18010，同一 route 表 / docroot）----
http11_assert "route /api/hi"          "h3-api:hi"              "http://127.0.0.1:18010/api/hi"
http11_assert "route /api/json/:id+q"  "json-api:p42:q=x=1&y=2" "http://127.0.0.1:18010/api/json/p42?x=1&y=2"
http11_assert "static /hello.txt"      "hello h3 static world"  "http://127.0.0.1:18010/hello.txt"
code=$(curl -s --path-as-is -o /dev/null -w "%{http_code}" -m 5 "http://127.0.0.1:18010/no-such-file" 2>/dev/null)
[ "$code" = "404" ] && echo "[HTTP/1.1] 404 /no-such-file PASS" || { echo "[HTTP/1.1] 404 FAIL got=$code"; ok=0; }
code=$(curl -s --path-as-is -o /dev/null -w "%{http_code}" -m 5 "http://127.0.0.1:18010/../etc/passwd" 2>/dev/null)
[ "$code" = "403" ] && echo "[HTTP/1.1] 403 /../etc/passwd PASS" || { echo "[HTTP/1.1] 403 FAIL got=$code"; ok=0; }

# ---- H3 客户端（4 连接并发）----
"$CLI_BIN" >/tmp/m53s3_cli.out 2>&1
CLI_EXIT=$?
wait $SRV_PID 2>/dev/null
SRV_EXIT=$?

# 服务端断言
grep -q "server: http/1.1 listening 18010" /tmp/m53s3_srv.out || { echo "FAIL: server 未监听"; ok=0; }
grep -q "server: done" /tmp/m53s3_srv.out || { echo "FAIL: server 未正常退出"; ok=0; }
# 客户端断言
for pat in "4 conns established" \
           "route /api/hi -> 200 body=\[h3-api:hi\]" \
           "route /api/json/p42?x=1&y=2 -> 200 body=\[json-api:p42:q=x=1&y=2\]" \
           "static /hello.txt -> 200 body=\[hello h3 static world\]" \
           "404 /no-such-file -> 404" \
           "403 /../etc/passwd -> 403" \
           "M53-S3 HTTP/3 pipe 多连接一致性 ALL PASS" \
           "client: done"; do
    grep -q "$pat" /tmp/m53s3_cli.out || { echo "FAIL: 缺输出行 [$pat]"; ok=0; }
done

if [ "$ok" = "1" ] && [ "$CLI_EXIT" = "0" ] && [ "$SRV_EXIT" = "0" ]; then
    echo ""
    echo "══════════ M53 S3 HTTP/3 公共管道 全部 PASS ══════════"
    echo "（4 连接 × 5 请求一致性输出：）"
    grep -h "H3 \[" /tmp/m53s3_cli.out
    exit 0
else
    echo "M53 S3 验证失败（exit srv=$SRV_EXIT cli=$CLI_EXIT）"
    echo "--- server out ---"; cat /tmp/m53s3_srv.out
    echo "--- client out ---"; cat /tmp/m53s3_cli.out
    exit 1
fi
