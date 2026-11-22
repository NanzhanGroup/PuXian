#!/usr/bin/env bash
# ============================================================
# selfhost/check_resp_header_guard.sh —— HTTP **响应**头方向的行级防护（M435 · 缺陷 902/903）
# ------------------------------------------------------------
# 病史（2026-11-22 22:0x 实测 · /tmp/m435probe · http_serve 路径，修前 20 条判据 12 条不符）：
#   R1  headers {"X-A": "a\r\nInjected: yes"} ⇒ 响应里**真的多出两行**（响应头注入 / 响应拆分）
#   R6  名字方向同样成立：{"X-A\r\nInjected4": "1"} ⇒ `X-A` + `Injected4: 1`
#   R8  handler 自设 Content-Length: 0 ⇒ 响应里 **两条** CL（2 与 0）⇒ 中间件/缓存取不同值
#   R9  handler 自设 Transfer-Encoding: chunked ⇒ TE + CL **并存**（RFC 9112 §6.1 明禁；体未分块）
#   R10 自设 Connection ⇒ 两条；R11 自设 Date: x ⇒ 照写（覆盖服务器该自己生成的头）
#   R17 {"status": -5} ⇒ `HTTP/1.1 -5 status code -5`；R18 999 原样；R19 100 当最终响应（体还带 CL）
#   ⇒ 共同病灶：px_serve 主路早已有 px_hdr_append（拒绝名单 + CRLF 防护 + 预算，M109-S1/S4），
#     但 px_http_build_response（http_serve / http_stream manual 错误分支）**自己手搓 "K: V\r\n"**
#     ⇒ 同一件事两份实现，防护只在其中一份上。修法 = 让它走同一份实现（缺陷 902），并补状态码域（903）。
#
# 判据（http_serve 路径；R21 用 px_serve 路径对拍「同口径」）：
#   R0  基线 X-A: 1                              ⇒ 200 且头在（正控）
#   R1/R2/R3  值含 CRLF / 裸 CR / 裸 LF           ⇒ 注入标记**不得**出现在响应
#   R5/R6/R7  名含空格 / 名含 CRLF / 空名          ⇒ 不得写出（含行首 ": "）
#   R8  handler 自设 CL ⇒ 响应只许**一条** CL
#   R9  自设 TE: chunked ⇒ 既不得有 TE，且 CL 仍只有一条
#   R10 自设 Connection ⇒ 只许一条
#   R11 自设 Date: x ⇒ 不得出现
#   R12/R13/R14  值含 HTAB / obs-text(0xFF) / 空值 ⇒ **必须保留**（防过度修复）
#   R15/R16  多值头 list（含一项带 CRLF）          ⇒ 合法项保留 2 行/1 行，注入标记不出现
#   R17/R18/R19  status -5 / 999 / 100            ⇒ 500（1xx 不是最终响应）
#   R20  status 204                              ⇒ 204（合法域不许被挡）
#   R21  px_serve 路径跑同一组非法头               ⇒ 注入同样不出现（两路同口径）
#   N1  负控自证：把手搓拼装注回 + 摘掉状态码域    ⇒ R1 注入复现 · R8 CL=2 · R17 不再是 500
#        （前提自证：负控服务必须活着且 R0 仍 200 —— 否则「全 NA」会被误当放行）
#   G1  环境守卫（tools/px / python3 / cc 缺失或构建失败）⇒ rc=3
#   G2  零污染（只对 runtime/runtime.c：sha + git status 前后一致）
#   G3  收尾：本门不留监听
#
# 用法：check_resp_header_guard.sh              # 全跑（约 120~300s，含一次负控重建）
#       check_resp_header_guard.sh --self-test  # 只跑判据夹具（离线 <1s）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime/runtime.c"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/m435_rh.XXXXXX")"
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
# 缺陷 899 的教训：负控会把 runtime.c 打成「判据摘除」态，信号不进清理就把树留脏。
trap 'cleanup; exit 130' INT TERM HUP

# ── 判据纯函数（可自测）────────────────────────────────────
val_of() {  # $1=输出 $2=键
    printf '%s\n' "$1" | awk -F= -v n="$2" '$1==n {print $2}' | head -1
}
is_eq() {   # $1=输出 $2=键 $3=期望
    local g; g="$(val_of "$1" "$2")"
    [ -n "$g" ] && [ "$g" = "$3" ]
}

selftest() {
    local st=0
    local out=$'R0_ST=200\nR0_KEEP=1\nR1_INJ=0\nR8_CL=1\nR17_ST=500'
    is_eq "$out" R1_INJ 0   && echo "  PASS S1 注入标记=0 被认出" || { echo "  FAIL S1"; st=1; }
    is_eq "$out" R1_INJ 1   && { echo "  FAIL S2 0 被当成 1"; st=1; } || echo "  PASS S2 0≠1"
    is_eq "$out" R8_CL 1    && echo "  PASS S3 CL 计数=1" || { echo "  FAIL S3"; st=1; }
    is_eq "$out" R8_CL 2    && { echo "  FAIL S4 CL=1 被当成 2"; st=1; } || echo "  PASS S4 1≠2"
    is_eq "$out" R17_ST 500 && echo "  PASS S5 状态码 500" || { echo "  FAIL S5"; st=1; }
    is_eq "$out" R99_ST 500 && { echo "  FAIL S6 缺失键不许当通过"; st=1; } || echo "  PASS S6 缺失键不算通过"
    is_eq "$out" R0_KEEP 1  && echo "  PASS S7 合法头保留" || { echo "  FAIL S7"; st=1; }
    return $st
}

if [ "${1:-}" = "--self-test" ]; then
    selftest && { echo "自测：7/7 通过"; exit 0; } || { echo "自测：有失败"; exit 2; }
fi

# ── G1 环境守卫 ───────────────────────────────────────────
command -v python3 >/dev/null 2>&1 || { echo "环境不成立：没有 python3" >&2; exit 3; }
command -v cc >/dev/null 2>&1      || { echo "环境不成立：没有 cc" >&2; exit 3; }
[ -x "$PX" ]                       || { echo "环境不成立：$PX 不可执行" >&2; exit 3; }
[ -f "$RT" ]                       || { echo "环境不成立：$RT 不存在" >&2; exit 3; }

GIT0="$(cd "$ROOT" && git status --porcelain -- runtime/runtime.c 2>/dev/null | wc -l)"
RT_SHA0="$(sha256sum "$RT" 2>/dev/null | cut -d' ' -f1)"

pick_port() {
    python3 - <<'EOF'
import socket
s=socket.socket(); s.bind(("127.0.0.1",0)); print(s.getsockname()[1]); s.close()
EOF
}

write_case() {  # $1=端口 $2=http|serve
    mkdir -p "$TMP/doc"
    cat > "$TMP/srv.px" <<EOF
PORT = $1
def handler(req):
    p = req["path"]
    if p == "/base":
        return {"status": 200, "body": "ok", "headers": {"X-A": "1"}}
    if p == "/val-crlf":
        return {"status": 200, "body": "ok", "headers": {"X-A": "a\r\nInjected: yes"}}
    if p == "/val-cr":
        return {"status": 200, "body": "ok", "headers": {"X-A": "a\rInjected2: yes"}}
    if p == "/val-lf":
        return {"status": 200, "body": "ok", "headers": {"X-A": "a\nInjected3: yes"}}
    if p == "/name-space":
        return {"status": 200, "body": "ok", "headers": {"X Bad": "1"}}
    if p == "/name-crlf":
        return {"status": 200, "body": "ok", "headers": {"X-A\r\nInjected4": "1"}}
    if p == "/name-empty":
        return {"status": 200, "body": "ok", "headers": {"": "1"}}
    if p == "/hdr-cl":
        return {"status": 200, "body": "ok", "headers": {"Content-Length": "0"}}
    if p == "/hdr-te":
        return {"status": 200, "body": "ok", "headers": {"Transfer-Encoding": "chunked"}}
    if p == "/hdr-conn":
        return {"status": 200, "body": "ok", "headers": {"Connection": "keep-alive"}}
    if p == "/hdr-date":
        return {"status": 200, "body": "ok", "headers": {"Date": "x"}}
    if p == "/val-tab":
        return {"status": 200, "body": "ok", "headers": {"X-Tab": "a\tb"}}
    if p == "/val-empty":
        return {"status": 200, "body": "ok", "headers": {"X-Empty": ""}}
    if p == "/dup":
        return {"status": 200, "body": "ok", "headers": {"Set-Cookie": ["a=1", "b=2"]}}
    if p == "/dup-crlf":
        return {"status": 200, "body": "ok", "headers": {"Set-Cookie": ["a=1", "b=2\r\nInjected5: yes"]}}
    if p == "/st-neg":
        return {"status": -5, "body": "ok"}
    if p == "/st-999":
        return {"status": 999, "body": "ok"}
    if p == "/st-100":
        return {"status": 100, "body": "ok"}
    if p == "/st-204":
        return {"status": 204, "body": ""}
    return {"status": 404, "body": "nf"}
EOF
    if [ "$2" = "serve" ]; then
        cat >> "$TMP/srv.px" <<EOF
vhost("*", handler)
def go():
    px_serve(PORT, "$TMP/doc", 20000)
spawn go()
sleep(300000)
EOF
    else
        cat >> "$TMP/srv.px" <<EOF
http_serve(PORT, handler)
EOF
    fi
}

write_probe() {  # $1=端口
    cat > "$TMP/probe.py" <<'EOF'
import socket, sys
P = int(sys.argv[1])

def send(path, wait=2.0):
    s = socket.create_connection(("127.0.0.1", P), 3); s.settimeout(wait)
    s.sendall(b"GET " + path.encode() + b" HTTP/1.1\r\nHost: t\r\nConnection: close\r\n\r\n")
    d = b""
    try:
        while True:
            b = s.recv(65536)
            if not b: break
            d += b
    except socket.timeout: pass
    s.close(); return d

def st(d):
    if not d.startswith(b"HTTP/1.1 "): return "NA"
    return d.split(b" ")[1].decode("latin-1")

def head(d):
    p = d.find(b"\r\n\r\n")
    return d[:p] if p >= 0 else d

def cnt(h, name):
    n = 0
    for line in h.split(b"\r\n"):
        if line.lower().startswith(name.lower() + b":"): n += 1
    return n

# 名字: (路径, 探针键)
cases = [
    ("R0",  "/base"), ("R1", "/val-crlf"), ("R2", "/val-cr"), ("R3", "/val-lf"),
    ("R5",  "/name-space"), ("R6", "/name-crlf"), ("R7", "/name-empty"),
    ("R8",  "/hdr-cl"), ("R9", "/hdr-te"), ("R10", "/hdr-conn"), ("R11", "/hdr-date"),
    ("R12", "/val-tab"), ("R13", "/val-empty"), ("R14", "/dup"), ("R16", "/dup-crlf"),
    ("R17", "/st-neg"), ("R18", "/st-999"), ("R19", "/st-100"), ("R20", "/st-204"),
]
for n, p in cases:
    d = send(p); h = head(d); hl = h.lower()
    print(f"{n}_ST={st(d)}")
    print(f"{n}_CL={cnt(h, b'Content-Length')}")
    print(f"{n}_CONN={cnt(h, b'Connection')}")
    print(f"{n}_TE={cnt(h, b'Transfer-Encoding')}")
    print(f"{n}_COOKIE={cnt(h, b'Set-Cookie')}")
    # 注入/非法标记是否出现在响应头域
    inj = b"Injected" in h
    print(f"{n}_INJ={1 if inj else 0}")
    # 合法头是否保留
    keep = 0
    if n == "R0"  and b"X-A: 1" in h: keep = 1
    if n == "R12" and b"X-Tab: a\tb" in h: keep = 1
    if n == "R13" and b"X-Empty:" in h: keep = 1
    if n == "R14" and b"Set-Cookie: a=1" in h and b"Set-Cookie: b=2" in h: keep = 1
    if n == "R16" and b"Set-Cookie: a=1" in h: keep = 1
    print(f"{n}_KEEP={keep}")
    if n == "R5":
        print(f"R5_BADNAME={1 if b'X Bad' in h else 0}")
    if n == "R11":
        print(f"R11_DATE={1 if b'ate: x' in h else 0}")
    # 空名写出检测：头域里是否有以 ": " 开头的行（Python 3.9：f-string 里不许出现反斜杠）
    if n == "R7":
        segs = h.split(b'\r\n')
        print("R7_EMPTYNAME=" + ("1" if any(l.startswith(b':') for l in segs) else "0"))
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

# ── 主测（http_serve 路径）─────────────────────────────────
PORT1="$(pick_port)"
write_case "$PORT1" http; write_probe "$PORT1"

echo "── 构建探针服务（http_serve · $PORT1）"
if ! build_srv srv_http; then
    echo "构建失败（日志 $TMP/build.log）：" >&2; tail -15 "$TMP/build.log" >&2
    echo "环境不成立：探针构建失败" >&2; exit 3
fi
if ! start_srv "$PORT1" "$TMP/srv_http"; then
    echo "环境不成立：探针服务未起来" >&2; cat "$TMP/srv.log" >&2; exit 3
fi
OUT="$(python3 "$TMP/probe.py" "$PORT1" 2>&1)"
stop_srv "$PORT1"
# 前提自证：探针必须真的吐出可解析的结果（否则「实得为空」会把一堆 FAIL 堆成噪声，难定位）
if [ -z "$(val_of "$OUT" R0_ST)" ]; then
    echo "  FAIL G0 前提自证：探针输出无可解析键（probe.py 出错？净输出前 12 行）"
    printf '%s\n' "$OUT" | head -12 | sed 's/^/    probe| /' >&2
    echo; echo "合计：PASS $PASS · FAIL $((FAIL+1))"; exit 1
fi

ck() {  # $1=键 $2=期望 $3=说明
    if is_eq "$OUT" "$1" "$2"; then ok "$1 $3（$2）"; else bad "$1 $3（期望 $2，实得 $(val_of "$OUT" "$1")）"; fi
}

ck R0_ST 200   "基线 ⇒ 200（正控）"
ck R0_KEEP 1   "基线头被写出"
ck R1_INJ 0    "值含 CRLF ⇒ 注入标记不得出现（响应拆分）"
ck R2_INJ 0    "值含裸 CR ⇒ 不得注入"
ck R3_INJ 0    "值含裸 LF ⇒ 不得注入"
ck R5_BADNAME 0 "名含空格 ⇒ 不得写出"
ck R6_INJ 0    "名含 CRLF ⇒ 不得注入"
ck R7_EMPTYNAME 0 "空名 ⇒ 不得写出「: v」行"
ck R8_CL 1     "handler 自设 CL ⇒ 响应该只有一条 CL"
ck R9_TE 0     "handler 自设 TE ⇒ 不得出现（且 CL 仍单条）"
ck R9_CL 1     "TE 被拒后 CL 仍只有一条"
ck R10_CONN 1  "handler 自设 Connection ⇒ 只许一条"
ck R11_DATE 0  "handler 自设 Date: x ⇒ 不得写出（拒绝名单）"
ck R12_KEEP 1  "值含 HTAB ⇒ 保留（防过度修复）"
ck R13_KEEP 1  "空值头 ⇒ 保留（防过度修复）"
ck R14_KEEP 1  "多值头 list ⇒ 两行都在（M136 回归）"
ck R14_COOKIE 2 "多值头计数=2"
ck R16_KEEP 1  "多值其中一项带 CRLF ⇒ 合法项仍保留"
ck R16_INJ 0   "多值其中一项带 CRLF ⇒ 注入不出现"
ck R17_ST 500  "status -5 ⇒ 500（状态码域）"
ck R18_ST 500  "status 999 ⇒ 500"
ck R19_ST 500  "status 100 ⇒ 500（1xx 不是最终响应）"
ck R20_ST 204  "status 204 ⇒ 204（合法域不许被挡）"

# ── R21 px_serve 路径同口径（既有防护面参照）──────────────
echo "── R21 px_serve 路径对拍（同一 handler，另一条实现）"
PORT3="$(pick_port)"
write_case "$PORT3" serve
R21_OK=1
if build_srv srv_serve; then
    if start_srv "$PORT3" "$TMP/srv_serve"; then
        OUT2="$(python3 "$TMP/probe.py" "$PORT3" 2>&1)"
        for k in R1_INJ R2_INJ R3_INJ R6_INJ R16_INJ R5_BADNAME; do
            [ "$(val_of "$OUT2" "$k")" = "0" ] || { bad "R21 px_serve 路径 $k 仍注入（两路口径不一致）"; R21_OK=0; }
        done
        [ "$(val_of "$OUT2" R0_ST)" = "200" ] || { bad "R21 前提自证失败：px_serve 基线不是 200（$(val_of "$OUT2" R0_ST)）"; R21_OK=0; }
        stop_srv "$PORT3"
        [ "$R21_OK" = "1" ] && ok "R21 px_serve 路径同一组非法头同样不注入（两路同口径 · 基线 200）"
    else
        bad "R21 px_serve 探针未起来"; R21_OK=0
    fi
else
    bad "R21 px_serve 探针构建失败"; R21_OK=0
fi

# ── N1 负控自证：手搓拼装注回 + 摘状态码域 ──────────────────
echo "── N1 负控自证（手搓拼装注回 build_response + 摘状态码域）"
cp -p "$RT" "$RT_BAK"
python3 - "$RT" <<'PYEOF' || { echo "补丁失败" >&2; exit 1; }
import re, sys
p = sys.argv[1]; s = open(p).read()

# ① 把 build_response 里的 px_hdr_append 调用换回「手搓拼装」（不校验）—— 只动这一处调用。
old = "            (void)px_hdr_append(h, extra_headers, 0, (int)sizeof(extra_headers), 0, &dropped_hdr);"
new = ("            (void)dropped_hdr;\n"
       "            for (int i2 = 0; i2 < h.as.obj->as.dict.len; i2++) {\n"
       "                LXValue hv2 = h.as.obj->as.dict.vals[i2];\n"
       "                const char* k2 = h.as.obj->as.dict.keys[i2];\n"
       "                if (hv2.type != PX_STR) continue;\n"
       "                int l2 = (int)strlen(extra_headers);\n"
       "                snprintf(extra_headers + l2, sizeof(extra_headers) - (size_t)l2, \"%s: %s\\r\\n\", k2, hv2.as.obj->as.str.data);\n"
       "            }")
if s.count(old) != 1:
    raise SystemExit("px_hdr_append 调用处数不符：%d（应为 1）" % s.count(old))
s = s.replace(old, new)

# ② 摘掉状态码域（只摘这一处 build_response 的判定）
old2 = "    if (status < 200 || status > 599) {"
if s.count(old2) != 1:
    raise SystemExit("状态码判定处数不符：%d（应为 1）" % s.count(old2))
s = s.replace(old2, "    if (0) {   /* M435 N1：状态码域摘除（负控自证） */")
open(p, "w").write(s)
print("N1 补丁：build_response 手搓拼装注回 + 状态码域摘除")
PYEOF
RT_PATCHED=1
PORT4="$(pick_port)"
write_case "$PORT4" http
NEG_OK=1
if build_srv srv_neg; then
    if start_srv "$PORT4" "$TMP/srv_neg"; then
        NEG="$(python3 "$TMP/probe.py" "$PORT4" 2>&1)"
        # 前提自证：负控服务必须活着且基线仍 200（否则「NA 不是 0」类判定会假绿）
        if ! kill -0 "${SRV_PID:-0}" 2>/dev/null; then
            bad "N1 前提自证失败：负控服务已退出（结论不能算数）"; NEG_OK=0
            tail -8 "$TMP/srv.log" 2>/dev/null | sed 's/^/    srv.log| /' >&2
        fi
        if [ "$(val_of "$NEG" R0_ST)" != "200" ]; then
            bad "N1 前提自证失败：负控基线不是 200（$(val_of "$NEG" R0_ST)）"; NEG_OK=0
        fi
        if [ "$NEG_OK" = "1" ]; then
            # ① 注入必须复现
            [ "$(val_of "$NEG" R1_INJ)" = "1" ] || { bad "N1 注回手搓后 R1 仍无注入 ⇒ 判据没长在那条路径上"; NEG_OK=0; }
            [ "$(val_of "$NEG" R6_INJ)" = "1" ] || { bad "N1 注回手搓后 R6 仍无注入"; NEG_OK=0; }
            # ② 重复 CL 必须复现（2 条）
            [ "$(val_of "$NEG" R8_CL)" = "2" ] || { bad "N1 注回手搓后 R8 CL 计数=$(val_of "$NEG" R8_CL)（期望 2）"; NEG_OK=0; }
            # ③ 状态码域摘除后 -5 不再是 500
            [ "$(val_of "$NEG" R17_ST)" != "500" ] || { bad "N1 摘状态码域后 R17 仍是 500 ⇒ 状态码判据没被测到"; NEG_OK=0; }
            [ "$NEG_OK" = "1" ] && ok "N1 负控成立：手搓注回 ⇒ 注入复现 + CL=2 + status -5 不再 500（基线 200 全程存活）"
        fi
        stop_srv "$PORT4"
    else
        bad "N1 负控服务未起来"; NEG_OK=0
    fi
else
    bad "N1 负控构建失败"; NEG_OK=0
fi
cp -p "$RT_BAK" "$RT"; RT_PATCHED=0

# ── G2 零污染（只对被测文件负责）───────────────────────────
GIT1="$(cd "$ROOT" && git status --porcelain -- runtime/runtime.c 2>/dev/null | wc -l)"
RT_SHA1="$(sha256sum "$RT" 2>/dev/null | cut -d' ' -f1)"
if [ "$GIT0" = "$GIT1" ] && [ "$RT_SHA0" = "$RT_SHA1" ]; then
    ok "G2 runtime.c 原样回滚（git status $GIT0 → $GIT1 · sha ${RT_SHA0:0:12}）"
else
    bad "G2 runtime.c 未回滚（git status $GIT0 → $GIT1 · sha ${RT_SHA0:0:12} → ${RT_SHA1:0:12}）"
fi

# ── G3 本门不留监听 ───────────────────────────────────────
LEFT=0
for p in "$PORT1" "$PORT3" "$PORT4"; do ss -ltn 2>/dev/null | grep -q ":$p " && LEFT=$((LEFT+1)); done
if [ "$LEFT" = "0" ]; then ok "G3 本门不留监听"; else bad "G3 残留监听 $LEFT 个端口"; fi

echo
echo "合计：PASS $PASS · FAIL $FAIL"
[ "$FAIL" = "0" ] && exit 0 || exit 1
