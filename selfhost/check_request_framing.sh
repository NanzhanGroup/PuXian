#!/usr/bin/env bash
# ============================================================
# selfhost/check_request_framing.sh —— HTTP/1.1 请求体「长度声明」契约（M432 · 缺陷 891）
# ------------------------------------------------------------
# 病史（2026-11-22 12:4x 实测 · /tmp/m432probe，px_serve，修前）：
#   P3  Content-Length: 5 + Transfer-Encoding: chunked  ⇒ **200**（静默按 chunked 处理，CL 丢弃）
#   P4  双 Content-Length: 5 / 10                      ⇒ **200 len=10**（最后一个赢）
#   P5  Content-Length: abc                            ⇒ **200 len=0**（atoi=0，后续字节当下一请求）
#   P6  Content-Length: -1                             ⇒ **200**（负值当「无体」）
#   全部是同一句 `content_length = atoi(v)` 的四个副本（http_serve worker / px_serve worker /
#   两处客户端读路径）。前三条让「本服务端认定的请求边界」与反向代理（多数取第一个 CL）
#   产生分歧 = 经典请求走私（RFC 9112 §6.2/§6.3）。
#   ⚠ 还有一个**判据自身的坑**：拒绝后若直接 close，内核在接收队列仍有未读数据时发 RST，
#     客户端可能**收不到**这条 400（实测：同一请求，客户端看到空响应）⇒ 必须先吸干再关。
#
# 判据：
#   P1  CL=3000 > max_body_size 1024        ⇒ 413（回归：修前就有的行为不许退化）
#   P2  chunked 3000 > max                   ⇒ 413
#   P3  CL=5 + TE: chunked                   ⇒ 400（歧义 ⇒ 拒绝，不猜）
#   P4  双 CL 5 / 10（冲突）                 ⇒ 400
#   P4b 双 CL 5 / 5（相同）                  ⇒ 200（**防过度修复**：RFC 允许同值重复）
#   P5  CL=abc                               ⇒ 400
#   P6  CL=-1                                ⇒ 400
#   P6b CL=+5                                ⇒ 400
#   P6c CL=' 5 '（首尾 OWS）                 ⇒ 200（合法容忍）
#   P6d CL='1 0'（内嵌空格）                 ⇒ 400
#   P7  GET                                  ⇒ 200（正常路径不许被挡）
#   P8  TE: gzip                             ⇒ 501（不支持的传输编码必须响亮）
#   P9  TE: chunked, gzip                    ⇒ 501
#   P10 chunked 正常                         ⇒ 200 body 长度正确
#   P11 CL 20 位（溢出）                     ⇒ 400
#   P12 CL=3000000000（超 int）              ⇒ 413
#   P14 拒绝响应**真的能被客户端读到**        ⇒ 400 响应体非空且含「Content-Length」
#        （这一条守的是上面那个 RST 坑：只看状态码不看「读得到」会假绿）
#   N1 负控自证：把 px_framing_resolve 打成 `return 0;` 重建 ⇒ P3/P4/P5/P6/P8/P9/P11 必须**不再**被拒
#        （且补丁树必须真重建 —— rt_key 含 runtime 源码，NOCACHE 由 key 变化保证）
#   G1 环境守卫：tools/px / python3 / cc 不可用或探针构建失败 ⇒ rc=3（绝不降级成绿）
#   G2 零污染：全程在 $TMP 构建；仓库 git status 行数前后一致（补丁在 $TMP 的备份上回滚）
#   G3 收尾：本门自己不留监听
#
# 用法：check_request_framing.sh              # 全跑（约 60~160s，含一次负控重建）
#       check_request_framing.sh --self-test  # 只跑判据夹具（离线 <1s）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime/runtime.c"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m432_framing.XXXXXX")"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }

RT_BAK="$TMP/runtime.c.bak"
RT_PATCHED=0
SRV_PID=""
cleanup() {
    [ -n "${SRV_PID}" ] && kill -KILL "$SRV_PID" 2>/dev/null
    if [ "$RT_PATCHED" = "1" ] && [ -f "$RT_BAK" ]; then cp -p "$RT_BAK" "$RT" 2>/dev/null; fi
    chmod -R u+w "$TMP" 2>/dev/null
    rm -rf "$TMP"
}
trap cleanup EXIT

# ── 判据纯函数（可自测）────────────────────────────────────
# 输入：探针输出的一行 `NAME=STATUS`；$2=名字 $3=期望状态
status_of() {  # $1=整段输出 $2=名字 ⇒ 打印状态（无则打印 NA）
    printf '%s\n' "$1" | awk -F= -v n="$2" '$1==n {print $2}' | head -1
}
status_ok() {  # $1=输出 $2=名字 $3=期望
    local got; got="$(status_of "$1" "$2")"
    [ -n "$got" ] && [ "$got" = "$3" ]
}
resp_readable() {  # $1=输出 $2=名字 ⇒ 该条必须同时有 400 且 body 非 NA/空
    local st bd
    st="$(status_of "$1" "$2")"
    bd="$(printf '%s\n' "$1" | awk -F= -v n="$2" '$1==n"BODY" {print $2}' | head -1)"
    [ "$st" = "400" ] && [ -n "$bd" ] && [ "$bd" != "NA" ] && [ "$bd" -gt 20 ]
}

selftest() {
    local st=0
    echo "── 自测（判据夹具，全离线）"
    local out=$'P3=400\nP4=200\nP5=400\nP5BODY=91'
    if status_ok "$out" P3 400; then echo "  PASS S1 正控：400 被认出"; else echo "  FAIL S1"; st=1; fi
    if ! status_ok "$out" P4 400; then echo "  PASS S2 负控：200 不被当成 400"; else echo "  FAIL S2"; st=1; fi
    if ! status_ok "$out" P9 400; then echo "  PASS S3 缺失条目不算通过（不许静默当绿）"; else echo "  FAIL S3"; st=1; fi
    if resp_readable "$out" P5; then echo "  PASS S4 拒绝响应可读（body 91 字节）"; else echo "  FAIL S4"; st=1; fi
    if resp_readable "$out" P4; then echo "  FAIL S5 只看状态码不看 body ⇒ 假绿"; st=1; else echo "  PASS S5 非 400 不许算可读"; fi
    if resp_readable "P7=400
P7BODY=" P7; then echo "  FAIL S6 空 body 被当成可读"; st=1; else echo "  PASS S6 空 body 被拒"; fi
    return $st
}

if [ "${1:-}" = "--self-test" ]; then
    selftest && { echo "自测：6/6 通过"; exit 0; } || { echo "自测：有失败"; exit 2; }
fi

# ── G1 环境守卫 ──────────────────────────────────────────
command -v python3 >/dev/null 2>&1 || { echo "环境不成立：没有 python3" >&2; exit 3; }
command -v cc >/dev/null 2>&1       || { echo "环境不成立：没有 cc" >&2; exit 3; }
[ -x "$PX" ]                        || { echo "环境不成立：$PX 不可执行" >&2; exit 3; }
[ -f "$RT" ]                        || { echo "环境不成立：$RT 不存在" >&2; exit 3; }

GIT0="$(cd "$ROOT" && git status --porcelain 2>/dev/null | wc -l)"

pick_port() {
    python3 - <<'EOF'
import socket
s=socket.socket(); s.bind(("127.0.0.1",0)); print(s.getsockname()[1]); s.close()
EOF
}

write_case() {  # $1=端口
    cat > "$TMP/srv.px" <<EOF
PORT = $1
def handler(req):
    var n = 0
    let b = req["body"]
    if b != null:
        n = len(b)
    return {"status": 200, "body": "ok len=" + str(n), "headers": {"Content-Type": "text/plain"}}
vhost("*", handler)
def serve():
    px_serve(PORT, "$TMP/doc", 20000, {"max_body_size": 1024})
spawn serve()
sleep(120000)
EOF
    mkdir -p "$TMP/doc"
}

write_probe() {  # $1=端口
    cat > "$TMP/probe.py" <<'EOF'
import socket, sys
P=int(sys.argv[1])
def send(raw, wait=2.5):
    s=socket.create_connection(("127.0.0.1",P),3); s.settimeout(wait)
    s.sendall(raw); d=b""
    try:
        while True:
            b=s.recv(65536)
            if not b: break
            d+=b
    except socket.timeout: pass
    s.close(); return d
def make(hdrs=b"", body=b"", method=b"POST"):
    return method+b" /x HTTP/1.1\r\nHost: t\r\n"+hdrs+b"\r\n"+body
def st(d):
    if not d.startswith(b"HTTP/1.1 "): return "NA"
    return d.split(b" ")[1].decode()
def blen(d):
    p=d.find(b"\r\n\r\n")
    return "NA" if p<0 else str(len(d)-p-4)
cases=[
 ("P1",  make(b"Content-Length: 3000\r\n", b"a"*3000)),
 ("P2",  make(b"Transfer-Encoding: chunked\r\n", b"800\r\n"+b"a"*2048+b"\r\n0\r\n\r\n")),
 ("P3",  make(b"Content-Length: 5\r\nTransfer-Encoding: chunked\r\n", b"5\r\nhello\r\n0\r\n\r\n")),
 ("P4",  make(b"Content-Length: 5\r\nContent-Length: 10\r\n", b"hello01234")),
 ("P4b", make(b"Content-Length: 5\r\nContent-Length: 5\r\n", b"hello")),
 ("P5",  make(b"Content-Length: abc\r\n", b"hello")),
 ("P6",  make(b"Content-Length: -1\r\n", b"hello")),
 ("P6b", make(b"Content-Length: +5\r\n", b"hello")),
 ("P6c", make(b"Content-Length:  5 \r\n", b"hello")),
 ("P6d", make(b"Content-Length: 1 0\r\n", b"hello01234")),
 ("P7",  make(method=b"GET")),
 ("P8",  make(b"Transfer-Encoding: gzip\r\n", b"x")),
 ("P9",  make(b"Transfer-Encoding: chunked, gzip\r\n", b"x")),
 ("P10", make(b"Transfer-Encoding: chunked\r\n", b"5\r\nhello\r\n0\r\n\r\n")),
 ("P11", make(b"Content-Length: 99999999999999999999\r\n", b"x")),
 ("P12", make(b"Content-Length: 3000000000\r\n", b"x")),
]
for n,raw in cases:
    d=send(raw)
    print(f"{n}={st(d)}")
    print(f"{n}BODY={blen(d)}")
EOF
}

build_srv() {  # $1=输出名
    ( cd "$TMP" && "$PX" build srv.px >"$TMP/build.log" 2>&1 ) || return 1
    [ -x "$TMP/build/srv" ] || return 1
    cp -f "$TMP/build/srv" "$TMP/$1"
    return 0
}

start_srv() {  # $1=端口 $2=二进制
    ( cd "$TMP" && setsid "$2" >"$TMP/srv.log" 2>&1 & echo $! >"$TMP/pid" )
    SRV_PID="$(cat "$TMP/pid" 2>/dev/null || echo "")"
    local i=0
    while [ "$i" -lt 60 ]; do
        ss -ltn 2>/dev/null | grep -q ":$1 " && return 0
        i=$((i+1)); sleep 0.25
    done
    return 1
}
stop_srv() {  # $1=端口（可选）—— setsid 的子进程不一定是 $!，所以按端口兜底收回
    [ -n "${SRV_PID}" ] && kill -TERM "$SRV_PID" 2>/dev/null
    if [ -n "${1:-}" ]; then
        local p
        for p in $(ss -ltnp 2>/dev/null | grep ":$1 " | grep -o 'pid=[0-9]*' | cut -d= -f2 | sort -u); do
            kill -TERM "$p" 2>/dev/null
        done
        sleep 0.4
        for p in $(ss -ltnp 2>/dev/null | grep ":$1 " | grep -o 'pid=[0-9]*' | cut -d= -f2 | sort -u); do
            kill -KILL "$p" 2>/dev/null
        done
    fi
    sleep 0.3
    SRV_PID=""
}

# ── 主测 ────────────────────────────────────────────────
PORT1="$(pick_port)"
write_case "$PORT1"; write_probe "$PORT1"

echo "── 构建探针服务（$PORT1）"
if ! build_srv srv_new; then
    echo "构建失败（日志 $TMP/build.log）：" >&2
    tail -15 "$TMP/build.log" >&2
    echo "环境不成立：探针构建失败" >&2
    exit 3
fi
if ! start_srv "$PORT1" "$TMP/srv_new"; then
    echo "环境不成立：探针服务未起来" >&2; cat "$TMP/srv.log" >&2; exit 3
fi
OUT="$(python3 "$TMP/probe.py" "$PORT1" 2>&1)"
stop_srv "$PORT1"

check() {  # $1=名字 $2=期望 $3=说明
    if status_ok "$OUT" "$1" "$2"; then ok "$1 $3（$2）"; else bad "$1 $3（期望 $2，实得 $(status_of "$OUT" "$1")）"; fi
}
check P1  413 "CL > max_body_size ⇒ 413（回归）"
check P2  413 "chunked > max ⇒ 413（回归）"
check P3  400 "CL + TE 并存 ⇒ 400"
check P4  400 "双 CL 冲突 ⇒ 400"
check P4b 200 "双 CL 相同 ⇒ 200（防过度修复）"
check P5  400 "CL 非数字 ⇒ 400"
check P6  400 "CL 负数 ⇒ 400"
check P6b 400 "CL 带 + 号 ⇒ 400"
check P6c 200 "CL 首尾 OWS ⇒ 200（合法容忍）"
check P6d 400 "CL 内嵌空格 ⇒ 400"
check P7  200 "GET 正常 ⇒ 200"
check P8  501 "TE: gzip ⇒ 501"
check P9  501 "TE: chunked, gzip ⇒ 501"
check P10 200 "chunked 正常 ⇒ 200"
check P11 400 "CL 20 位溢出 ⇒ 400"
check P12 413 "CL 超 int ⇒ 413"

# P14：拒绝响应必须**真的读得到**（守 RST 坑）
if resp_readable "$OUT" P5; then ok "P14 400 响应可读（body $(printf '%s\n' "$OUT" | awk -F= '$1=="P5BODY"{print $2}' | head -1) 字节）"
else bad "P14 400 响应读不到（RST 把响应吞了 ⇒ 客户端只看到空）"; fi

# ── N1 负控自证：摘掉终判 ⇒ 判据必须失效 ────────────────────
echo "── N1 负控自证（摘除 px_framing_resolve ⇒ 同一批探针必须不再被拒）"
cp -p "$RT" "$RT_BAK"
python3 - "$RT" <<'EOF' || { echo "补丁失败" >&2; exit 1; }
import re,sys
p=sys.argv[1]; s=open(p).read()
def gut(name, repl):
    global s
    m=re.search(r"(static int "+name+r"\([^)]*\) \{)(.*?)(\n\})", s, re.S)
    if not m: raise SystemExit(1)
    s = s[:m.start(2)] + repl + s[m.end(2):]
gut("px_framing_feed", "\n    (void)f; (void)k; (void)v; return 0;   // M432 N1：故意摘除（负控自证）")
gut("px_framing_resolve", "\n    (void)f; return 0;   // M432 N1：故意摘除（负控自证）")
open(p,"w").write(s)
EOF
RT_PATCHED=1
PORT2="$(pick_port)"
write_case "$PORT2"; write_probe "$PORT2"
NEG_OK=1
if build_srv srv_neg; then
    if start_srv "$PORT2" "$TMP/srv_neg"; then
        NEG="$(python3 "$TMP/probe.py" "$PORT2" 2>&1)"
        stop_srv "$PORT2"
        for pair in "P3 400" "P4 400" "P5 400" "P6 400" "P8 501" "P9 501" "P11 400"; do
            set -- $pair
            if status_ok "$NEG" "$1" "$2"; then
                bad "N1 摘除判据后 $1 仍被拒（$2）⇒ 负控不成立，判据可能没被测到"
                NEG_OK=0
            fi
        done
        [ "$NEG_OK" = "1" ] && ok "N1 摘除判据后 7 条非法请求全部放行（判据确实长在那条路径上）"
    else
        bad "N1 负控服务未起来"; NEG_OK=0
    fi
else
    bad "N1 负控构建失败"; NEG_OK=0
fi
cp -p "$RT_BAK" "$RT"; RT_PATCHED=0

# ── G2 零污染 ───────────────────────────────────────────
GIT1="$(cd "$ROOT" && git status --porcelain 2>/dev/null | wc -l)"
if [ "$GIT0" = "$GIT1" ]; then ok "G2 仓库零污染（git status $GIT0 → $GIT1）"
else bad "G2 仓库被改动（git status $GIT0 → $GIT1）"; fi

# ── G3 本门不留监听 ────────────────────────────────────
LEFT=0
for p in "$PORT1" "$PORT2"; do ss -ltn 2>/dev/null | grep -q ":$p " && LEFT=$((LEFT+1)); done
if [ "$LEFT" = "0" ]; then ok "G3 本门不留监听"; else bad "G3 残留监听 $LEFT 个端口"; fi

echo
echo "合计：PASS $PASS · FAIL $FAIL"
[ "$FAIL" = "0" ] && exit 0 || exit 1
