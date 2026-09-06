#!/bin/bash
# M83-S1 (Issue 16) 专项验证：http_serve/http_serve_unix 服务端 body 动态缓冲（64KB → 可配上限+413）
#   + 语言层 len()/contains() 对含 NUL str 一致化（尊重 str.len，字节 memmem）
# 覆盖：① 1MB 大 body 动态接收（旧实现截 65535）② 含 NUL body 的 len()=4 / contains 跨 NUL 命中
#       ③ NUL body 原样回显（req.body 与响应双向 str.len 保真）④ unix 入口同测（共享 worker 一处改双入口）
#       ⑤ 小 body 无回归 ⑥ PX_HTTP_BODY_MAX 超限 → 413
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
PORT=18085
PORT2=18086
SOCK=/tmp/m83_s1.sock
SOCK2=/tmp/m83_s1_413.sock
BIN=./build/serve_daemon

echo "== [1/2] 编译（--no-quic；runtime.c 改动 → pxc 按源 hash 自动重建 .rtcache runtime.o）=="
$PX build --no-quic serve_daemon.px >/tmp/m83s1_build.log 2>&1 || { echo "FAIL build"; tail -25 /tmp/m83s1_build.log; exit 1; }

echo "== [2/2] 运行验证 =="
rm -f "$SOCK" "$SOCK2" /tmp/m83s1_big.bin
"$BIN" "$PORT" "$SOCK" >/tmp/m83s1_daemon.log 2>&1 &
DAEMON=$!
ok=0
for i in $(seq 1 60); do
    H=$(curl -s --max-time 2 "http://127.0.0.1:$PORT/health" 2>/dev/null)
    [ "$H" = "ok" ] && { ok=1; break; }
    sleep 0.1
done
if [ "$ok" != "1" ]; then echo "FAIL daemon 未就绪"; cat /tmp/m83s1_daemon.log; kill $DAEMON 2>/dev/null; exit 1; fi
echo "PASS daemon 就绪（TCP $PORT + unix $SOCK）"

# ① 1MB 大 body（全 ASCII 可打印 → UTF-8 字符数==字节数==1048576，可精确断言；旧固定 64KB → 截后 65535）
python3 -c "import sys; sys.stdout.buffer.write(bytes([0x21 + (i*7) % 94 for i in range(1024*1024)]))" > /tmp/m83s1_big.bin
BIGSZ=$(stat -c %s /tmp/m83s1_big.bin)
python3 verify_cli.py len-tcp "$PORT" /tmp/m83s1_big.bin "$BIGSZ" || { echo "FAIL ①"; kill $DAEMON 2>/dev/null; exit 1; }
echo "   → ① 1MB body 动态接收通过（旧 64KB 截断已修）"

# ② 含 NUL body len()/contains() 语言层语义（TCP）
python3 verify_cli.py probe-tcp "$PORT" || { echo "FAIL ②"; kill $DAEMON 2>/dev/null; exit 1; }
echo "   → ② len()/contains() NUL 一致化通过"
# ③ NUL body 原样回显（TCP）
python3 verify_cli.py echo-tcp "$PORT" || { echo "FAIL ③"; kill $DAEMON 2>/dev/null; exit 1; }
# ④ unix 入口同测（1MB + NUL 语义 + 回显）
python3 verify_cli.py len-unix "$SOCK" /tmp/m83s1_big.bin "$BIGSZ" || { echo "FAIL ④a"; kill $DAEMON 2>/dev/null; exit 1; }
python3 verify_cli.py probe-unix "$SOCK" || { echo "FAIL ④b"; kill $DAEMON 2>/dev/null; exit 1; }
python3 verify_cli.py echo-unix "$SOCK" || { echo "FAIL ④c"; kill $DAEMON 2>/dev/null; exit 1; }
# ⑤ 小 body 无回归
E=$(curl -s --max-time 5 -X POST -H 'Content-Type: application/octet-stream' --data-binary 'hello-123' "http://127.0.0.1:$PORT/echo")
[ "$E" = "hello-123" ] || { echo "FAIL ⑤ 小 body 回归: $E"; kill $DAEMON 2>/dev/null; exit 1; }
kill $DAEMON 2>/dev/null
echo "PASS ②③④⑤ NUL 语义 + unix 双入口 + 小 body 无回归"

# ⑥ 413（PX_HTTP_BODY_MAX=102400 起第二实例 → POST 200KB → 413）
PX_HTTP_BODY_MAX=102400 "$BIN" "$PORT2" "$SOCK2" >/tmp/m83s1_daemon2.log 2>&1 &
DAEMON2=$!
ok=0
for i in $(seq 1 60); do
    H=$(curl -s --max-time 2 "http://127.0.0.1:$PORT2/health" 2>/dev/null)
    [ "$H" = "ok" ] && { ok=1; break; }
    sleep 0.1
done
if [ "$ok" != "1" ]; then echo "FAIL 413 daemon 未就绪"; cat /tmp/m83s1_daemon2.log; kill $DAEMON2 2>/dev/null; exit 1; fi
dd if=/dev/urandom of=/tmp/m83s1_200k.bin bs=1024 count=200 2>/dev/null
CODE=$(curl -s -o /dev/null -w '%{http_code}' --max-time 10 -X POST -H 'Content-Type: application/octet-stream' --data-binary @/tmp/m83s1_200k.bin "http://127.0.0.1:$PORT2/len")
kill $DAEMON2 2>/dev/null
[ "$CODE" = "413" ] || { echo "FAIL ⑥ 超限应 413 实得 $CODE"; exit 1; }
echo "PASS ⑥ PX_HTTP_BODY_MAX=102400 + 200KB POST → 413"

rm -f "$SOCK" "$SOCK2" /tmp/m83s1_big.bin /tmp/m83s1_200k.bin
echo "m83_s1_http_body_nul verify done"
exit 0
