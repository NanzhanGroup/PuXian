#!/usr/bin/env bash
# ============================================================
# M53 S4：px_serve opts.http3 —— 三栈合一 WebServer 端到端验证（编译模式）
#   单个 px_serve(18020, docroot, timeout, {http3:{port:18021}}) 同时托管：
#     * HTTP/1.1（TCP :18020）→ curl 对照
#     * HTTP/3  （QUIC/UDP :18021）→ PuXian 自研 client（2 连接并发）
#                                     + aioquic（第三方独立实现）互操作
#   断言：
#     1) HTTP/1.1 与 HTTP/3 命中同一 route/docroot，响应逐项一致
#     2) Alt-Svc: h3=":18021" 自动通告（http3 开启且未显式 alt_svc）
#     3) aioquic ↔ PuXian 跨实现握手/QPACK/请求响应 200（外部互操作零突破）
#     4) SIGTERM → px_serve 优雅关闭（H3 listener 回收），exit 0
#   注：opts.http3 为 runtime.c 新能力，pxi 解释器在 S5 自举重建前不含
#       该支持 —— 本验证跑编译模式（与 m53_s1/s3 同策略）。
# ============================================================
set -u
cd "$(dirname "$0")/.."
B=examples/build
SRV_BIN=$B/m53_s4_pxserve_h3_server
CLI_BIN=$B/m53_s4_pxserve_h3_client
AIOQ=examples/h3_aioquic_client.py

echo "=== M53 S4 px_serve opts.http3 三栈合一 端到端验证（编译模式） ==="
ok=1
if [ "${PXC_SKIP_BUILD:-0}" != "1" ]; then
    ./tools/pxc build examples/m53_s4_pxserve_h3_server.px >/dev/null 2>&1 || { echo "FAIL: server 编译失败"; ok=0; }
    ./tools/pxc build examples/m53_s4_pxserve_h3_client.px >/dev/null 2>&1 || { echo "FAIL: client 编译失败"; ok=0; }
else
    echo "（跳过 pxc build，复用已有产物）"
fi
[ "$ok" = "1" ] || exit 1

http11_body() { curl -s --path-as-is -m 5 "$1" 2>/dev/null; }
http11_code() { curl -s --path-as-is -o /dev/null -w "%{http_code}" -m 5 "$1" 2>/dev/null; }
http11_header() { curl -s --path-as-is -D - -o /dev/null -m 5 "$1" 2>/dev/null; }

rm -f /tmp/m53s4.ready
"$SRV_BIN" >/tmp/m53s4_srv.out 2>&1 &
SRV_PID=$!
waited=0
while [ ! -f /tmp/m53s4.ready ] && [ $waited -lt 20 ]; do sleep 1; waited=$((waited+1)); done
if [ ! -f /tmp/m53s4.ready ]; then
    echo "FAIL: server 未就绪"; cat /tmp/m53s4_srv.out; kill -9 $SRV_PID 2>/dev/null; exit 1
fi
# 轮询到 HTTP/1.1 可服务（px_serve 已进入 accept loop → H3 listener 早已 bind）
waited=0
while [ $waited -lt 15 ]; do
    if [ "$(http11_code http://127.0.0.1:18020/api/hi 2>/dev/null)" = "200" ]; then break; fi
    sleep 1; waited=$((waited+1))
done
echo "server ready (pid=$SRV_PID)"

# ---- 1) HTTP/1.1 对照（curl → px_serve TCP :18020）----
assert_eq() { # desc want got
    if [ "$2" = "$3" ]; then echo "[HTTP/1.1] $1 PASS"; else echo "[HTTP/1.1] $1 FAIL want=[$2] got=[$3]"; ok=0; fi
}
assert_eq "route /api/hi"          "s4-api:hi"              "$(http11_body http://127.0.0.1:18020/api/hi)"
assert_eq "route /api/json/p42+q"  "json-api:p42:q=x=1&y=2" "$(http11_body 'http://127.0.0.1:18020/api/json/p42?x=1&y=2')"
assert_eq "static /hello.txt"      "hello h3 static world"  "$(http11_body http://127.0.0.1:18020/hello.txt | tr -d '\n')"
assert_eq "404 /no-such-file"      "404"                    "$(http11_code http://127.0.0.1:18020/no-such-file)"
assert_eq "403 /../etc/passwd"     "403"                    "$(http11_code 'http://127.0.0.1:18020/../etc/passwd')"
# Alt-Svc 自动通告
if http11_header http://127.0.0.1:18020/api/hi | grep -qi '^Alt-Svc: h3=":18021"'; then
    echo "[HTTP/1.1] Alt-Svc 自动通告 PASS"
else
    echo "[HTTP/1.1] Alt-Svc 自动通告 FAIL"; ok=0
fi

# ---- 2) PuXian 自研 H3 client（2 连接并发）----
"$CLI_BIN" >/tmp/m53s4_cli.out 2>&1
CLI_EXIT=$?
# ---- 3) aioquic 第三方互操作（连 H3 UDP :18021）----
python3 "$AIOQ" 127.0.0.1 18021 /api/hi /api/json/p42 /hello.txt /no-such-file >/tmp/m53s4_aioq.out 2>&1
AIOQ_EXIT=$?

# ---- 4) 优雅关闭：SIGTERM → px_serve 回收 H3 listener → exit 0 ----
kill -TERM $SRV_PID 2>/dev/null
wait $SRV_PID 2>/dev/null
SRV_EXIT=$?

# ---- 服务端断言 ----
grep -q "HTTP/3 listening udp/18021" /tmp/m53s4_srv.out || { echo "FAIL: server 未起 H3 listener"; ok=0; }
grep -q "HTTP/3 listener 已关闭" /tmp/m53s4_srv.out || { echo "FAIL: server 未回收 H3 listener"; ok=0; }
# 自研 client 断言
for pat in "2 conns established" \
           "H3 \[c0\] route /api/hi -> 200 body=\[s4-api:hi\]" \
           "H3 \[c0\] route /api/json/p42?x=1&y=2 -> 200 body=\[json-api:p42:q=x=1&y=2\]" \
           "H3 \[c0\] static /hello.txt -> 200 body=\[hello h3 static world\]" \
           "H3 \[c0\] 404 /no-such-file -> 404" \
           "H3 \[c0\] 403 /../etc/passwd -> 403" \
           "M53-S4 px_serve http3 自研客户端 ALL PASS" \
           "client: done"; do
    grep -q "$pat" /tmp/m53s4_cli.out || { echo "FAIL: client 缺输出行 [$pat]"; ok=0; }
done
# aioquic 断言（body 与 HTTP/1.1 / 自研 client 一致）
for line in "AIOQUIC /api/hi 200 s4-api:hi" \
            "AIOQUIC /api/json/p42 200 json-api:p42:q=" \
            "AIOQUIC /hello.txt 200 hello h3 static world" \
            "AIOQUIC /no-such-file 404"; do
    grep -qF "$line" /tmp/m53s4_aioq.out || { echo "FAIL: aioquic 缺输出 [$line]"; ok=0; }
done

echo ""
echo "--- aioquic 输出（第三方 HTTP/3 互操作）---"
cat /tmp/m53s4_aioq.out
echo "-------------------------------------------"

if [ "$ok" = "1" ] && [ "$CLI_EXIT" = "0" ] && [ "$AIOQ_EXIT" = "0" ] && [ "$SRV_EXIT" = "0" ]; then
    echo ""
    echo "══════════ M53 S4 px_serve opts.http3 全部 PASS ══════════"
    echo "HTTP/1.1(TCP:18020) + HTTP/3(UDP:18021) 三栈合一；aioquic 互操作 OK；优雅关闭 exit=$SRV_EXIT"
    exit 0
else
    echo "M53 S4 验证失败（exit srv=$SRV_EXIT cli=$CLI_EXIT aioq=$AIOQ_EXIT）"
    echo "--- server out ---"; cat /tmp/m53s4_srv.out
    echo "--- client out ---"; cat /tmp/m53s4_cli.out
    echo "--- aioquic out ---"; cat /tmp/m53s4_aioq.out
    exit 1
fi
