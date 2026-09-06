#!/bin/bash
# M83-S6（Issue 19 GAP-SRV-SSE）http_stream 同端口流式验证
#   同端口 /json（普通 JSON handler）与 /stream（http_stream 流式 SSE）共存；
#   http_serve(TCP) 与 http_serve_unix 同享流式路由；sse_serve 独立端口旧行为回归；
#   客户端中途断开服务端不崩。
# 依赖：tools/pxc（编译模式；http_serve/sse_serve/spawn 非 pxi Mini 子集）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
PORT=18097
SOCK=/tmp/m83_s6.sock
LEGACY=18098
mkdir -p build
SRV_PID=0
trap 'kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; rm -f $SOCK /tmp/m83s6_*.log' EXIT

echo "== [1/5] 编译并启动 daemon（http_serve + http_serve_unix + sse_serve）=="
$PX build --no-quic serve_sse_daemon.px > /tmp/m83s6_build.log 2>&1 || { echo "FAIL build"; tail -15 /tmp/m83s6_build.log; exit 1; }
./build/serve_sse_daemon $PORT $SOCK $LEGACY > /tmp/m83s6_srv.log 2>&1 &
SRV_PID=$!
sleep 1.0
curl -s --max-time 2 http://127.0.0.1:$PORT/health | grep -q 'ok' || { echo "FAIL daemon 未就绪"; tail -5 /tmp/m83s6_srv.log; exit 1; }

echo "== [2/5] 同端口普通 JSON 路由与流式路由共存 =="
J=$(curl -s --max-time 2 http://127.0.0.1:$PORT/json)
echo "json=$J"
[ "$J" = "json-ok" ] || { echo "FAIL 普通路由 /json"; exit 1; }

echo "== [3/5] /stream TCP 流式（text/event-stream + 逐块 chunk + [DONE] + EOF）=="
HDR=$(curl -sN -D - --max-time 5 http://127.0.0.1:$PORT/stream -o /tmp/m83s6_stream.txt)
echo "$HDR" | grep -qi 'Content-Type: text/event-stream' || { echo "FAIL SSE Content-Type"; echo "$HDR"; exit 1; }
cat /tmp/m83s6_stream.txt
grep -q '^data: chunk1$' /tmp/m83s6_stream.txt || { echo "FAIL chunk1"; exit 1; }
grep -q '^data: chunk2$' /tmp/m83s6_stream.txt || { echo "FAIL chunk2"; exit 1; }
grep -q '^data: chunk3$' /tmp/m83s6_stream.txt || { echo "FAIL chunk3"; exit 1; }
grep -q '^event: done$' /tmp/m83s6_stream.txt || { echo "FAIL event done"; exit 1; }
grep -q '^data: \[DONE\]$' /tmp/m83s6_stream.txt || { echo "FAIL [DONE]"; exit 1; }
# on_connect 返回后连接关闭（curl 自然 EOF，--max-time 未触发）
echo "stream-tcp PASS（3 chunk + [DONE] + EOF 正常关闭）"

echo "== [4/5] /stream Unix socket 同享（http_serve_unix 同 worker）=="
curl -sN --max-time 5 --unix-socket $SOCK http://localhost/stream > /tmp/m83s6_stream_unix.txt || { echo "FAIL unix stream curl"; exit 1; }
grep -q '^data: chunk1$' /tmp/m83s6_stream_unix.txt || { echo "FAIL unix chunk1"; exit 1; }
grep -q '^data: chunk3$' /tmp/m83s6_stream_unix.txt || { echo "FAIL unix chunk3"; exit 1; }
grep -q '\[DONE\]' /tmp/m83s6_stream_unix.txt || { echo "FAIL unix [DONE]"; exit 1; }
echo "stream-unix PASS"

echo "== [5/5] sse_serve 独立端口旧行为回归 + 断连不崩 =="
L=$(curl -sN --max-time 2 http://127.0.0.1:$LEGACY/x)
echo "legacy=$L"
echo "$L" | grep -q 'legacy-1' || { echo "FAIL sse_serve legacy 回归"; exit 1; }
echo "$L" | grep -q 'legacy-2' || { echo "FAIL sse_serve legacy 回归 2"; exit 1; }
# 客户端中途断开（--max-time 0.3 切断 /stream）→ 服务端仍健康（不崩）
curl -sN --max-time 0.3 http://127.0.0.1:$PORT/stream > /dev/null 2>&1
sleep 0.4
curl -s --max-time 2 http://127.0.0.1:$PORT/health | grep -q 'ok' || { echo "FAIL 断连后服务端崩"; tail -8 /tmp/m83s6_srv.log; exit 1; }
echo "断连不崩 PASS"
kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null
SRV_PID=0
rm -f $SOCK
echo "m83_s6 verify done"
exit 0
