#!/usr/bin/env bash
# M131 验证：http_stream 的 POST / manual / chunked / sse_write（qg-issue 87 缺陷 65）
# 依赖：tools/px（编译模式）、python3（裸 socket 抓原始线上字节 —— 必须看得到分块帧）
# 两轨都跑（默认 VM 轨 + C 轨），断言集相同。
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m131_build.log
FAIL=0
PORT="${M131_GATE_PORT:-19871}"
# 端口卫生（M314 全量门实测）：固定 19871 曾被遗留的 mock_mysql 占住，
# VM 轨一路收 MySQL 握手包 ⇒ 门假红。占用时改用空闲端口，绝不量别人的服务。
if python3 - "$PORT" <<'PY' 2>/dev/null
import socket, sys
s = socket.socket(); s.settimeout(0.3)
try:
    s.connect(("127.0.0.1", int(sys.argv[1]))); sys.exit(0)
except Exception:
    sys.exit(1)
finally:
    s.close()
PY
then
    _BUSY="$PORT"
    PORT=$(python3 - <<'PY2'
import socket
s = socket.socket(); s.bind(("127.0.0.1", 0)); print(s.getsockname()[1]); s.close()
PY2
)
    echo "  ⚠ 端口 $_BUSY 已被占用 ⇒ 改用空闲端口 $PORT（脏测量防护）"
fi
# 缺陷 572：选定端口必须**同时**交给服务端（旧版只改探针 ⇒ 服务端仍绑 19871 ⇒ 端口被占必假红）。
export M131_GATE_PORT="$PORT"

cat > /tmp/m131_probe.py <<'PY'
import socket, sys, time
PORT = int(sys.argv[1])
ok = bad = 0
def chk(tag, cond, got=""):
    global ok, bad
    if cond:
        ok += 1; print("PASS %s" % tag)
    else:
        bad += 1; print("FAIL %s  got=%r" % (tag, got))

def raw(req, wait=0.8):
    s = socket.create_connection(("127.0.0.1", PORT), timeout=5)
    s.sendall(req)
    time.sleep(wait); s.settimeout(0.5)
    buf = b""
    while True:
        try: d = s.recv(65536)
        except socket.timeout: break
        if not d: break
        buf += d
    s.close(); return buf

def post(p, body=b""):
    return raw(b"POST %s HTTP/1.1\r\nHost: x\r\nContent-Length: %d\r\n\r\n%s"
               % (p.encode(), len(body), body))

# ① + ③ + ④：POST /s —— 分块帧逐字节
r = post("/s", b'{"messages":[]}')
head, _, body = r.partition(b"\r\n\r\n")
chk("①a 状态行 200", head.startswith(b"HTTP/1.1 200 OK"), head[:40])
chk("③a 声明 chunked", b"Transfer-Encoding: chunked" in head, head)
chk("②b Connection 保留调用方设定", b"Connection: keep-alive" in head, head)
expect = b"".join(b"%x\r\n%s\r\n" % (len(c), c.encode()) for c in
                  ["data: {\"i\":1}\n", "\n", "data: {\"i\":2}\n", "\n", "data: [DONE]\n", "\n"])
chk("③b 分块帧逐字节（含收尾 0\\r\\n\\r\\n）", body == expect + b"0\r\n\r\n", body)
chk("④ 原文透传（未被 SSE 分帧加 data:）", b"data: data:" not in body, body)

# ②：manual + 校验失败 → 普通 400（头未发，非 SSE 流）
r3 = post("/s3", b'{"messages":[]}')
h3, _, b3 = r3.partition(b"\r\n\r\n")
chk("②a 校验失败回普通 400", h3.startswith(b"HTTP/1.1 400 Bad Request"), h3[:40])
chk("②b 400 无 SSE 头", b"text/event-stream" not in h3, h3)
chk("②c 400 体逐字节", b3 == b'{"error":"bad request"}', b3)

# ②：通过 → SSE
r4 = post("/s3", b'{"x":1}')
h4, _, b4 = r4.partition(b"\r\n\r\n")
chk("②d 通过后 200 + SSE", h4.startswith(b"HTTP/1.1 200 OK") and b"text/event-stream" in h4, h4[:60])
chk("②e 未声明 chunked ⇒ 原样体", b4 == b"data: ok\n\n", b4)

# ①：method 掩码 —— POST 打到只注册 GET 的流式路由 ⇒ 不接管，落普通 handler
r5 = post("/s2", b"{}")
chk("①b 方法掩码生效（POST /s2 未被接管）", r5.endswith(b"root") and b"text/event-stream" not in r5, r5)

# 回归：GET 自动 SSE 头不变
r6 = raw(b"GET /s2 HTTP/1.1\r\nHost: x\r\n\r\n")
chk("回归 GET 自动 SSE（charset + Connection: close）",
    b"text/event-stream; charset=utf-8" in r6 and b"Connection: close" in r6 and r6.endswith(b"data: hello-auto\n\n"),
    r6)

print("M131_ASSERT: %dP/%dF" % (ok, bad))
sys.exit(1 if bad else 0)
PY

run_track() {   # $1=标签；其余为编译前置（env 形式）
    local label="$1"; shift
    local SRVLOG="/tmp/m131_srv_$$.log"
    local attempt=0 srv=0
    echo "== [$label] 编译 =="
    if ! env "$@" "$PX" build stream_post.px > "$LOG" 2>&1; then
        echo "FAIL [$label] 编译失败"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    echo "== [$label] 启动 + 探针 =="
    while :; do
        attempt=$((attempt+1))
        : > "$SRVLOG"
        ./build/stream_post > "$SRVLOG" 2>&1 &
        srv=$!
        local i=0
        while [ $i -lt 40 ]; do
            grep -q M131_READY "$SRVLOG" 2>/dev/null && break
            sleep 0.1; i=$((i+1))
        done
        kill -0 "$srv" 2>/dev/null && grep -q M131_READY "$SRVLOG" 2>/dev/null && break
        # 缺陷 572：READY 是 http_serve 之前打的，绑失败也照样出现 ⇒ 单纯等 READY 无牙。
        # M364（缺陷 632）留现场：旧版只 tail -5，且日志在 /tmp 会被负控覆盖 ⇒ 红过之后无证可查。
        # 失败即把「服务端全量输出 + 退出码 + 端口归属」落盘 /data/gatevm/，并原样打印。
        local srv_rc=0
        kill -0 "$srv" 2>/dev/null && { echo "  （进程存活但未就绪 ⇒ 收掉）" >&2; kill "$srv" 2>/dev/null; }
        wait "$srv" 2>/dev/null; srv_rc=$?
        local evid="/data/gatevm/m131_fail_$(date +%Y%m%d-%H%M%S)_${attempt}.log"
        {
            echo "label=$label attempt=$attempt port=$PORT srv_rc=$srv_rc ready=$(grep -c M131_READY "$SRVLOG" 2>/dev/null || true)"
            echo "--- 服务端全量输出 ---"; cat "$SRVLOG" 2>/dev/null
            echo "--- 端口归属 ---"; ss -lntp 2>/dev/null | grep ":$PORT " || echo "（$PORT 无监听）"
        } > "$evid" 2>&1
        echo "FAIL [$label] 服务端未就绪（端口 $PORT · srv_rc=$srv_rc）"
        sed 's/^/     | /' "$evid" | head -24
        echo "     证据：$evid"
        # 端口争用是**可证的环境问题**（日志明写「绑定端口 N 失败」）⇒ 换空闲端口重试一次，判据一字不改。
        # 其它原因（信号崩/未打 READY/超时）一律判红，不重试。
        if [ "$srv_rc" = "1" ] && grep -q "绑定端口.*失败" "$SRVLOG" 2>/dev/null && [ "$attempt" -lt 2 ]; then
            PORT=$(python3 -c 'import socket
s = socket.socket(); s.bind(("127.0.0.1", 0)); print(s.getsockname()[1]); s.close()')
            export M131_GATE_PORT="$PORT"
            echo "  ⚠ 端口争用（绑定失败）⇒ 换空闲端口 $PORT 重试（判据不变）"
            continue
        fi
        FAIL=$((FAIL+1)); return
    done
    local out rc
    out=$(python3 /tmp/m131_probe.py "$PORT" 2>&1); rc=$?
    echo "$out"
    kill "$srv" 2>/dev/null; wait "$srv" 2>/dev/null
    echo "$out" | grep -qE "M131_ASSERT: [0-9]+P/0F" || { echo "FAIL [$label] 有断言失败"; FAIL=$((FAIL+1)); return; }
    [ "$rc" = "0" ] || { echo "FAIL [$label] 退出码 $rc"; FAIL=$((FAIL+1)); return; }
    echo "PASS [$label]"
}

mkdir -p build
run_track "VM轨"
run_track "C轨" PX_BUILD_ENGINE=c

# ── 负控：断言"分块终结块必须存在"。把源码里的终结块期望删掉（由运行时正常产出），
#    改为探针期望"无终结块" ⇒ 必须变红，证明门不是自欺。
echo "== 负控（门自欺检测）=="
sed 's|body == expect + b"0\\r\\n\\r\\n"|body == expect|' /tmp/m131_probe.py > /tmp/m131_probe_neg.py
grep -q 'body == expect,' /tmp/m131_probe_neg.py || { echo "FAIL 负控改写失败"; FAIL=$((FAIL+1)); }
./build/stream_post > /tmp/m131_srv_neg.log 2>&1 &
srv=$!
i=0; while [ $i -lt 40 ]; do grep -q M131_READY /tmp/m131_srv.log 2>/dev/null && break; sleep 0.1; i=$((i+1)); done
if python3 /tmp/m131_probe_neg.py "$PORT" > /tmp/m131_neg.out 2>&1; then
    echo "FAIL 负控未生效（去掉终结块期望后仍全绿 = 门在自欺）"; FAIL=$((FAIL+1))
else
    echo "PASS 负控生效（终结块断言确有效力）"
fi
kill "$srv" 2>/dev/null; wait "$srv" 2>/dev/null

echo "───"
if [ "$FAIL" = "0" ]; then echo "✅ M131 门：全部通过"; else echo "❌ M131 门：$FAIL 项失败"; fi
exit $FAIL
