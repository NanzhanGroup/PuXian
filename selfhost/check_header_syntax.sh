#!/usr/bin/env bash
# ============================================================
# selfhost/check_header_syntax.sh —— HTTP/1.1 头部**字段语法**契约（M434 · 缺陷 897）
# ------------------------------------------------------------
# 病史（2026-11-22 19:5x 实测 · /tmp/m434probe，px_serve，修前 13 条判据 10 条不符）：
#   H1  `Content-Length : 5`（冒号前空格）⇒ 字段名成为 "Content-Length " ⇒ 认不出 ⇒ **200，体被静默丢弃**
#       更强的一条（A'）：体分片到达时，那 43 字节「代理认为是体」被本服务端**当成下一条请求执行**
#       （同一条连接上出现两个 200）⇒ 与前置代理的消息边界分歧 = 请求走私入口（RFC 9112 §6.4）
#   H2  `Content-Length\t: 5`（制表）· H3 `Content Length: 5`（名内空格）· H10 `X@Y: 1`（非 token）⇒ 同上，静默
#   H4  `GARBAGE`（无冒号）· H5 `: value`（空名）· H6 obs-fold 续行 ⇒ **静默丢弃该行**
#   H8  `Host: a` + `Host: b` ⇒ last-win 静默覆盖（Host 注入面）· H9 `Host: a b` ⇒ 照收
#   ⇒ 共同病灶：字段行**只找第一个冒号**，不校验名字/值/重复 —— 失败方式是「静默」而非「拒绝」。
#
# 判据：
#   H0  基线 CL=5                                  ⇒ 200 len=5（正控：正常路径不许被挡）
#   H1  CL + 空格冒号 / H2 制表 / H3 名内空格        ⇒ 400
#   H4  无冒号字段行 / H5 空字段名                   ⇒ 400
#   H6  obs-fold 续行                              ⇒ 400
#   H7  TE 折行（TE 空值先触 501）                  ⇒ **拒绝**（400 或 501，绝不许 200）
#   H8  重复 Host 冲突 / H9 Host 含空格              ⇒ 400
#   H10 非 token 字段名（X@Y）                      ⇒ 400
#   H11 正常多字段请求                              ⇒ 200 len=5
#   H12 chunked 正常 / H13 GET 无体                 ⇒ 200（回归）
#   H14 值含 HTAB / H15 值含 obs-text(>0x7f)        ⇒ 200（**防过度修复**：合法值不许误判）
#   H16 Host 带端口 + IPv6 字面量 `[::1]:8080`      ⇒ 200（防过度修复）
#   H17 端到端 desync：CL 空格冒号 + 体分片到达      ⇒ 只有一个 400（走私入口被封）
#   H18 拒绝响应**真的读得到**（body > 20 字节）      ⇒ 守缺陷 892 的 RST 坑
#   N1  负控自证：摘掉三个字段判据 + 两条行级判据 ⇒ H1/H2/H3/H4/H5/H6/H9/H10/H17 必须**不再**被拒
#        （补丁在 $TMP 的备份上回滚；rt_key 含 runtime 源码 ⇒ 真重建）
#   G1  环境守卫（tools/px / python3 / cc 缺失或探针构建失败）⇒ rc=3
#   G2  零污染：git status 行数前后一致
#   G3  收尾：本门自己不留监听
#
# 用法：check_header_syntax.sh              # 全跑（约 60~180s，含一次负控重建）
#       check_header_syntax.sh --self-test  # 只跑判据夹具（离线 <1s）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime/runtime.c"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m434_hdr.XXXXXX")"
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
# M434（缺陷 899）：负控会把 runtime.c 打成「判据摘除」态 —— 若门被 SIGTERM/INT/HUP 打断，
#   只挂 EXIT 不会执行清理，源码就留在被打伤的状态里（实测：一次工具超时把树留成 gutted）。
#   ⇒ 信号也要进清理，宁可重复清理一次。
trap 'cleanup; exit 130' INT TERM HUP

# ── 判据纯函数（可自测）────────────────────────────────────
status_of() {  # $1=整段输出 $2=名字 ⇒ 打印状态（无则 NA）
    printf '%s\n' "$1" | awk -F= -v n="$2" '$1==n {print $2}' | head -1
}
status_ok() {  # $1=输出 $2=名字 $3=期望
    local got; got="$(status_of "$1" "$2")"
    [ -n "$got" ] && [ "$got" = "$3" ]
}
status_any() {  # $1=输出 $2=名字 ⇒ 状态属于给定的多个「拒绝」之一（参数 3..）
    local got; got="$(status_of "$1" "$2")"; shift 2
    [ -n "$got" ] || return 1
    local want
    for want in "$@"; do [ "$got" = "$want" ] && return 0; done
    return 1
}
resp_readable() {  # $1=输出 $2=名字 ⇒ 该条必须 400 且 body 非空且 > 20
    local st bd
    st="$(status_of "$1" "$2")"
    bd="$(printf '%s\n' "$1" | awk -F= -v n="$2" '$1==n"BODY" {print $2}' | head -1)"
    [ "$st" = "400" ] && [ -n "$bd" ] && [ "$bd" != "NA" ] && [ "$bd" -gt 20 ]
}
count_of() {  # $1=输出 $2=名字 ⇒ COUNT 值
    printf '%s\n' "$1" | awk -F= -v n="$2" '$1==n"COUNT" {print $2}' | head -1
}

selftest() {
    local st=0
    echo "── 自测（判据夹具，全离线）"
    local out=$'H1=400\nH1BODY=128\nH0=200\nH0BODY=15\nH7=501'
    if status_ok "$out" H1 400; then echo "  PASS S1 正控：400 被认出"; else echo "  FAIL S1"; st=1; fi
    if ! status_ok "$out" H0 400; then echo "  PASS S2 负控：200 不被当成 400"; else echo "  FAIL S2"; st=1; fi
    if ! status_ok "$out" H9 400; then echo "  PASS S3 缺失条目不算通过（不许静默当绿）"; else echo "  FAIL S3"; st=1; fi
    if resp_readable "$out" H1; then echo "  PASS S4 拒绝响应可读（body 128）"; else echo "  FAIL S4"; st=1; fi
    if resp_readable "$out" H0; then echo "  FAIL S5 非 400 不许算可读"; st=1; else echo "  PASS S5 非 400 拒绝"; fi
    if resp_readable "H1=400
H1BODY=" H1; then echo "  FAIL S6 空 body 被当成可读"; st=1; else echo "  PASS S6 空 body 被拒"; fi
    if status_any "$out" H7 400 501; then echo "  PASS S7 H7 接受「拒绝」集合（400/501）"; else echo "  FAIL S7"; st=1; fi
    if ! status_any "$out" H0 400 501; then echo "  PASS S8 H0 不在拒绝集合内"; else echo "  FAIL S8"; st=1; fi
    if [ "$(count_of $'H17COUNT=1' H17)" = "1" ]; then echo "  PASS S9 COUNT 解析"; else echo "  FAIL S9"; st=1; fi
    if [ "$(count_of $'H17COUNT=2' H17)" != "1" ]; then echo "  PASS S10 COUNT=2 判不通过"; else echo "  FAIL S10"; st=1; fi
    return $st
}

if [ "${1:-}" = "--self-test" ]; then
    selftest && { echo "自测：10/10 通过"; exit 0; } || { echo "自测：有失败"; exit 2; }
fi

# ── G1 环境守卫 ──────────────────────────────────────────
command -v python3 >/dev/null 2>&1 || { echo "环境不成立：没有 python3" >&2; exit 3; }
command -v cc >/dev/null 2>&1       || { echo "环境不成立：没有 cc" >&2; exit 3; }
[ -x "$PX" ]                        || { echo "环境不成立：$PX 不可执行" >&2; exit 3; }
[ -f "$RT" ]                        || { echo "环境不成立：$RT 不存在" >&2; exit 3; }

GIT0="$(cd "$ROOT" && git status --porcelain -- runtime/runtime.c 2>/dev/null | wc -l)"
RT_SHA0="$(sha256sum "$RT" 2>/dev/null | cut -d' ' -f1)"

pick_port() {
    python3 - <<'EOF'
import socket
s=socket.socket(); s.bind(("127.0.0.1",0)); print(s.getsockname()[1]); s.close()
EOF
}

write_case() {  # $1=端口
    mkdir -p "$TMP/doc"
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
}

write_probe() {  # $1=端口
    cat > "$TMP/probe.py" <<'EOF'
import socket, sys
P = int(sys.argv[1])

def send(raw, wait=2.5):
    s = socket.create_connection(("127.0.0.1", P), 3); s.settimeout(wait)
    s.sendall(raw); d = b""
    try:
        while True:
            b = s.recv(65536)
            if not b: break
            d += b
    except socket.timeout: pass
    s.close(); return d

def split2(raw, wait=2.0):   # 分两段写：模拟「头与体在不同 TCP 段到达」
    s = socket.create_connection(("127.0.0.1", P), 3); s.settimeout(wait)
    s.sendall(raw[0]); import time; time.sleep(0.25); s.sendall(raw[1])
    d = b""
    try:
        while True:
            b = s.recv(65536)
            if not b: break
            d += b
    except socket.timeout: pass
    s.close(); return d

def make(hdrs=b"", body=b"", method=b"POST", path=b"/x", host=b"Host: t\r\n"):
    return method + b" " + path + b" HTTP/1.1\r\n" + host + hdrs + b"\r\n" + body

def st(d):
    if not d.startswith(b"HTTP/1.1 "): return "NA"
    return d.split(b" ")[1].decode()

def blen(d):
    p = d.find(b"\r\n\r\n")
    return "NA" if p < 0 else str(len(d) - p - 4)

cases = [
    ("H0",  make(b"Content-Length: 5\r\n", b"hello")),
    ("H1",  make(b"Content-Length : 5\r\n", b"hello")),
    ("H2",  make(b"Content-Length\t: 5\r\n", b"hello")),
    ("H3",  make(b"Content Length: 5\r\n", b"hello")),
    ("H4",  make(b"GARBAGE\r\n")),
    ("H5",  make(b": value\r\n")),
    ("H6",  make(b"X-Test: a\r\n X-B: 1\r\n")),
    ("H6b", make(b"X-Test: a\r\n b\r\n")),
    ("H7",  make(b"Transfer-Encoding:\r\n chunked\r\n", b"5\r\nhello\r\n0\r\n\r\n")),
    ("H9",  make(b"Host: a b\r\n")),
    ("H10", make(b"X@Y: 1\r\n")),
    ("H11", make(b"Content-Length: 5\r\nX-A: 1\r\nX-B: 2\r\n", b"hello")),
    ("H12", make(b"Transfer-Encoding: chunked\r\n", b"5\r\nhello\r\n0\r\n\r\n")),
    ("H13", make(method=b"GET")),
    ("H14", make(b"Content-Length: 5\r\nX-Tab: a\tb\r\n", b"hello")),
    ("H15", make(b"Content-Length: 5\r\nX-Obs: caf\xe9\r\n", b"hello")),
    ("H16", make(b"X-A: 1\r\n", host=b"Host: [::1]:8080\r\n")),
]
for n, raw in cases:
    d = send(raw)
    print(f"{n}={st(d)}")
    print(f"{n}BODY={blen(d)}")

# H8 重复 Host（make() 自带一个 Host，再加一个）
d = send(make(b"Host: b\r\n")); print(f"H8={st(d)}"); print(f"H8BODY={blen(d)}")

# H17 端到端 desync：体分片到达 —— 修前「体」被当成下一条请求执行（两个 200）
inner = b"GET /smuggled HTTP/1.1\r\nHost: t\r\n\r\n"
d = split2((b"POST /a HTTP/1.1\r\nHost: t\r\nContent-Length : %d\r\n\r\n" % len(inner), inner))
print(f"H17={st(d)}"); print(f"H17BODY={blen(d)}")
print(f"H17COUNT={d.count(b'HTTP/1.1 ')}")
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

stop_srv() {  # $1=端口
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

check H0  200 "基线 CL=5 ⇒ 200（正控）"
check H1  400 "冒号前空格 ⇒ 400"
check H2  400 "冒号前制表 ⇒ 400"
check H3  400 "字段名内空格 ⇒ 400"
check H4  400 "无冒号字段行 ⇒ 400"
check H5  400 "空字段名 ⇒ 400"
check H6  400 "obs-fold 续行（续行里带冒号）⇒ 400"
check H6b 400 "obs-fold 续行（续行里无冒号）⇒ 400（由无冒号判据兜住）"
check H9  400 "Host 含空格 ⇒ 400"
check H10 400 "字段名非 token（X@Y）⇒ 400"
check H11 200 "正常多字段 ⇒ 200"
check H12 200 "chunked 正常 ⇒ 200（回归）"
check H13 200 "GET 无体 ⇒ 200（回归）"
check H14 200 "值含 HTAB ⇒ 200（防过度修复）"
check H15 200 "值含 obs-text ⇒ 200（防过度修复）"
check H16 200 "Host 带端口/IPv6 字面量 ⇒ 200（防过度修复）"
check H8  400 "重复 Host ⇒ 400"

if status_any "$OUT" H7 400 501; then ok "H7 TE 折行 ⇒ 拒绝（$(status_of "$OUT" H7)，首个致命项优先）"
else bad "H7 TE 折行 ⇒ 期望拒绝(400/501)，实得 $(status_of "$OUT" H7)"; fi

# H17 端到端：只许一个响应，且必须是 400
H17C="$(count_of "$OUT" H17)"
if status_ok "$OUT" H17 400 && [ "$H17C" = "1" ]; then
    ok "H17 体分片到达 ⇒ 单条 400（走私入口被封：体不再被当成下一条请求）"
else
    bad "H17 体分片到达 ⇒ 期望 400 且 COUNT=1，实得 $(status_of "$OUT" H17) COUNT=${H17C:-NA}"
fi

if resp_readable "$OUT" H1; then ok "H18 400 响应可读（body $(printf '%s\n' "$OUT" | awk -F= '$1=="H1BODY"{print $2}' | head -1) 字节）"
else bad "H18 400 响应读不到（RST 把响应吞了 ⇒ 客户端只看到空）"; fi

# ── N1 负控自证：摘掉字段判据 + 行级判据 ⇒ 判据必须失效 ──────
echo "── N1 负控自证（摘除 px_hdr_name_ok / px_hdr_value_ok / px_host_value_ok + 两条行级判据）"
cp -p "$RT" "$RT_BAK"
python3 - "$RT" <<'PYEOF' || { echo "补丁失败" >&2; exit 1; }
import re, sys
p = sys.argv[1]; s = open(p).read()

def gut(name, repl):
    global s
    m = re.search(r"(static int " + name + r"\([^)]*\) \{)(.*?)(\n\})", s, re.S)
    if not m: raise SystemExit("not found: " + name)
    s = s[:m.start(2)] + repl + s[m.end(2):]

gut("px_hdr_name_ok",  "\n    (void)k; return 1;   // M434 N1：故意摘除（负控自证）")
gut("px_hdr_value_ok", "\n    (void)v; return 1;   // M434 N1：故意摘除（负控自证）")
gut("px_host_value_ok","\n    (void)v; return 1;   // M434 N1：故意摘除（负控自证）")

# 行级：obs-fold 判据（两个 worker 各一处，共 2 处）—— **不摘 `if (!colon)`**：
#   摘了以后 `*colon = 0` 会对 NULL 解引用直接崩（实测），负控自身死于崩溃 ⇒ 结论全 NA ⇒ 假绿。
#   这条也是 M434 顺手抓到的「判据自身的坑」（缺陷 898）：负控必须能整场活着跑完。
n1 = s.count("            if (line[0] == ' ' || line[0] == '\\t') {")
s = s.replace("            if (line[0] == ' ' || line[0] == '\\t') {",
              "            if (0) {   // M434 N1：obs-fold 判据摘除")
if n1 != 2:
    raise SystemExit("行级判据处数不符：obs-fold=%d（应为 2）" % n1)
# 行级：Host 计数/值判据（两个 worker 各一处，共 2 处）
n2 = s.count("if (host_n > 1 || !px_host_value_ok(v))")
s = s.replace("if (host_n > 1 || !px_host_value_ok(v))",
              "if (0)   /* M434 N1：Host 判据摘除 */")
if n2 != 2:
    raise SystemExit("Host 判据处数不符：%d（应为 2）" % n2)
open(p, "w").write(s)
print("N1 补丁：字段判据 3 处 + obs-fold 判据 %d 处 + Host 判据 %d 处" % (n1, n2))
PYEOF
RT_PATCHED=1
PORT2="$(pick_port)"
write_case "$PORT2"; write_probe "$PORT2"
NEG_OK=1
if build_srv srv_neg; then
    if start_srv "$PORT2" "$TMP/srv_neg"; then
        NEG="$(python3 "$TMP/probe.py" "$PORT2" 2>&1)"
        # N1 前提自证：负控服务必须**活着**且正常请求仍然 200。
        #   否则「9 条全放行」可能只是服务已崩（状态全 NA）⇒ 负控假绿。
        NEG_ALIVE=1
        if ! kill -0 "${SRV_PID:-0}" 2>/dev/null; then
            bad "N1 前提自证失败：负控服务已退出（探针结论不能算数，日志见下）"; NEG_ALIVE=0; NEG_OK=0
            tail -8 "$TMP/srv.log" 2>/dev/null | sed 's/^/    srv.log| /' >&2
        fi
        if ! status_ok "$NEG" H0 200; then
            bad "N1 前提自证失败：负控服务连正常请求都不 200（实得 $(status_of "$NEG" H0)）⇒ 负控结论无效"
            NEG_ALIVE=0; NEG_OK=0
        fi
        if [ "$NEG_ALIVE" = "1" ]; then
            for pair in "H1 400" "H2 400" "H3 400" "H5 400" "H6 400" "H8 400" "H9 400" "H10 400"; do
                set -- $pair
                if status_ok "$NEG" "$1" "$2"; then
                    bad "N1 摘除判据后 $1 仍被拒（$2）⇒ 负控不成立，判据可能没被测到"
                    NEG_OK=0
                fi
            done
            # H17 端到端也要失效：摘除后「体」应再次被当成下一条请求（COUNT 回到 2）
            if [ "$(count_of "$NEG" H17)" = "1" ]; then
                bad "N1 摘除判据后 H17 仍只有一条响应 ⇒ desync 判据没长在那条路径上"
                NEG_OK=0
            fi
        fi
        stop_srv "$PORT2"
        [ "$NEG_OK" = "1" ] && ok "N1 摘除判据后 9 条非法头全部放行 + H17 恢复到 COUNT=2（判据确实长在那条路径上 · 服务全程存活）"
    else
        bad "N1 负控服务未起来"; NEG_OK=0
    fi
else
    bad "N1 负控构建失败"; NEG_OK=0
fi
cp -p "$RT_BAK" "$RT"; RT_PATCHED=0

# ── G2 零污染（**只对被测文件负责**）──────────────────────
#   口径：本门会就地改 runtime.c 再回滚，所以要证明的是「runtime.c 原样回来了」，
#   不是「整棵树没动」—— 别人（包括并行的我）改其它文件不该把本门判红（M434 实测过这个假红）。
GIT1="$(cd "$ROOT" && git status --porcelain -- runtime/runtime.c 2>/dev/null | wc -l)"
RT_SHA1="$(sha256sum "$RT" 2>/dev/null | cut -d' ' -f1)"
if [ "$GIT0" = "$GIT1" ] && [ "$RT_SHA0" = "$RT_SHA1" ]; then
    ok "G2 runtime.c 原样回滚（git status $GIT0 → $GIT1 · sha ${RT_SHA0:0:12}）"
else
    bad "G2 runtime.c 未回滚（git status $GIT0 → $GIT1 · sha ${RT_SHA0:0:12} → ${RT_SHA1:0:12}）"
fi

# ── G3 本门不留监听 ────────────────────────────────────
LEFT=0
for p in "$PORT1" "$PORT2"; do ss -ltn 2>/dev/null | grep -q ":$p " && LEFT=$((LEFT+1)); done
if [ "$LEFT" = "0" ]; then ok "G3 本门不留监听"; else bad "G3 残留监听 $LEFT 个端口"; fi

echo
echo "合计：PASS $PASS · FAIL $FAIL"
[ "$FAIL" = "0" ] && exit 0 || exit 1
