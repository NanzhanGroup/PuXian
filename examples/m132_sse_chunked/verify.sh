#!/usr/bin/env bash
# M132 验证：SSE 客户端 chunked 解码 + sse_read_line（qg-issue 87 缺陷 69）
# 手段：python 桩按 seg=1/3/0 三种切法承载**同一串字节**；断言三种上游下行/事件序列全同。
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m132_build.log
SOCK=/tmp/m132_sse.sock
FAIL=0

cat > /tmp/m132_stub.py <<'PY'
import os, socket, sys, time
SOCK, SEG, N = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
BODY = 'data: {"a":1}\n\ndata: [DONE]\n\n'.encode()
try: os.unlink(SOCK)
except OSError: pass
srv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
srv.bind(SOCK); srv.listen(8); srv.settimeout(8)
for _ in range(N):
    try: c, _ = srv.accept()
    except socket.timeout: break
    try:
        c.recv(65536)
        if SEG > 0:
            c.sendall(b"HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\n"
                      b"Cache-Control: no-cache\r\nTransfer-Encoding: chunked\r\n\r\n")
            for i in range(0, len(BODY), SEG):
                blk = BODY[i:i+SEG]
                c.sendall(("%x\r\n" % len(blk)).encode() + blk + b"\r\n")
                time.sleep(0.01)
            c.sendall(b"0\r\n\r\n")
        else:
            c.sendall(b"HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\n"
                      b"Cache-Control: no-cache\r\n\r\n")
            c.sendall(BODY)
        time.sleep(0.15)
    finally:
        try: c.close()
        except Exception: pass
srv.close()
try: os.unlink(SOCK)
except OSError: pass
PY

run_case() {   # $1=seg
    local seg=$1
    # ⚠️ 必须先删残留 socket 文件：桩自己在 bind 前才 unlink，若沿用上一轮的残留文件，
    #    等待循环会**立即通过**，px 客户端可能抢在桩 bind 之前连接 ⇒ ECONNREFUSED
    #    （实测踩到：seg=1 阶段随机 CONNECT_FAIL）。删掉后再等它出现，才是真的"已就绪"。
    rm -f "$SOCK"
    python3 /tmp/m132_stub.py "$SOCK" "$seg" 2 > /tmp/m132_stub.log 2>&1 &
    local sp=$!
    local i=0
    while [ ! -S "$SOCK" ] && [ $i -lt 40 ]; do sleep 0.05; i=$((i+1)); done
    ./build/sse_chunked "$SOCK" "seg$seg" > "/tmp/m132_out_$seg.txt" 2>&1
    wait "$sp" 2>/dev/null
}

mkdir -p build
echo "== 编译 =="
if ! "$PX" build sse_chunked.px > "$LOG" 2>&1; then
    echo "FAIL 编译失败"; tail -20 "$LOG"; exit 1
fi

for seg in 1 3 0; do
    echo "== 上游切法 seg=$seg =="
    run_case "$seg"
    sed 's/^/     /' "/tmp/m132_out_$seg.txt"
done

# ① 三种上游（含 1 字节/块的最恶劣切法）输出必须完全一致
#    （输出里带 seg 标签，比对前先归一化标签，只比内容）
for seg in 1 3 0; do
    sed 's/^\(LINES\|EVENTS\) seg[0-9]* /\1 segT /' "/tmp/m132_out_$seg.txt" > "/tmp/m132_norm_$seg.txt"
done
if diff -q /tmp/m132_norm_1.txt /tmp/m132_norm_0.txt > /dev/null \
   && diff -q /tmp/m132_norm_3.txt /tmp/m132_norm_0.txt > /dev/null; then
    echo "PASS ① chunked(seg=1/3) 与不分块 输出逐字节一致（解码正确）"
else
    echo "FAIL ① 分块与不分块输出不一致："; diff /tmp/m132_norm_1.txt /tmp/m132_norm_0.txt | head -20; FAIL=$((FAIL+1))
fi

# ② 行序列逐行核对（含空行；末行为空行后的 "" 不应多出）
exp=$'LINES segT 0:<data: {"a":1}>\nLINES segT 1:<>\nLINES segT 2:<data: [DONE]>\nLINES segT 3:<>'
got=$(grep '^LINES' /tmp/m132_norm_1.txt)
if [ "$got" = "$exp" ]; then
    echo "PASS ② sse_read_line 行序列正确（4 行，含 2 空行，无多余尾部）"
else
    echo "FAIL ② 行序列不符"; echo "期望："; printf '%s\n' "$exp" | sed 's/^/     /'; echo "实得："; printf '%s\n' "$got" | sed 's/^/     /'; FAIL=$((FAIL+1))
fi

# ③ sse_read 事件无空事件（修前 chunked 会多出终结块产生的空事件）
ne=$(grep -cF 'data":""}' /tmp/m132_norm_1.txt || true)
nt=$(grep -c '^EVENTS' /tmp/m132_norm_1.txt || true)
if [ "$ne" = "0" ] && [ "$nt" = "2" ]; then
    echo "PASS ③ sse_read 事件数=2 且无空事件（修前为 4 个、含 2 个空事件）"
else
    echo "FAIL ③ 事件数=$nt 空事件=$ne（期望 2 / 0）"; FAIL=$((FAIL+1))
fi

# ④ 内容未被块长行污染
if grep -q '\\r\\n' /tmp/m132_norm_1.txt || grep -qE '<[0-9a-f]+$' /tmp/m132_norm_1.txt; then
    echo "FAIL ④ 输出里出现分块帧残留"; FAIL=$((FAIL+1))
else
    echo "PASS ④ 输出无分块帧残留（无 \\r\\n、无孤立十六进制行）"
fi

# ⑤ 同进程「下游服务端连接 + 上游客户端连接」并存（缺陷 70：id 命名空间重合）
echo "== 代理自检（缺陷 70）=="
rm -f "$SOCK"
python3 /tmp/m132_stub.py "$SOCK" 1 4 > /tmp/m132_stub2.log 2>&1 &
sp=$!
i=0; while [ ! -S "$SOCK" ] && [ $i -lt 40 ]; do sleep 0.05; i=$((i+1)); done
"$PX" build proxy.px > "$LOG" 2>&1 || { echo "FAIL 代理编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); }
./build/proxy "$SOCK" 19861 > /tmp/m132_proxy.log 2>&1 &
pp=$!
i=0; while ! grep -q M132_PROXY_READY /tmp/m132_proxy.log 2>/dev/null && [ $i -lt 60 ]; do sleep 0.1; i=$((i+1)); done
python3 - <<'PY' > /tmp/m132_proxy_out.txt 2>&1
import socket, time
s = socket.create_connection(("127.0.0.1", 19861), timeout=10)
b = b'{"x":1}'
s.sendall(b'POST /proxy HTTP/1.1\r\nHost: x\r\nContent-Type: application/json\r\n'
          b'Content-Length: ' + str(len(b)).encode() + b'\r\n\r\n' + b)
time.sleep(1.0); s.settimeout(0.6); buf = b""
while True:
    try: d = s.recv(65536)
    except socket.timeout: break
    if not d: break
    buf += d
s.close()
print(repr(buf))
PY
kill "$pp" 2>/dev/null; wait "$pp" 2>/dev/null
kill "$sp" 2>/dev/null; wait "$sp" 2>/dev/null
got=$(cat /tmp/m132_proxy_out.txt)
if echo "$got" | grep -qF 'data: [DONE]'; then
    if echo "$got" | grep -qF "0\r\n\r\n'"; then
        echo "PASS ⑤ 代理收尾完整（含 chunked 终块；缺陷 70 未复现）"
    else
        echo "FAIL ⑤ 缺 chunked 终块（疑似 sse_close 关错连接，缺陷 70）"; echo "     $got"; FAIL=$((FAIL+1))
    fi
else
    echo "FAIL ⑤ 透传内容不全：$got"; FAIL=$((FAIL+1))
fi

echo "───"
if [ "$FAIL" = "0" ]; then echo "✅ M132 门：全部通过"; else echo "❌ M132 门：$FAIL 项失败"; fi
exit $FAIL
