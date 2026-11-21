#!/usr/bin/env bash
# ============================================================
# examples/m408_proxy_connect/verify.sh —— http_request 代理 CONNECT 门（M408 · PX-017）
# ------------------------------------------------------------
# 为什么有这道门（晨曦 11-09 转交，清歌清单第 17 条）：
#   `http_request` 的代理分支只做了「连代理 + 请求行用绝对 URL」—— 那是**明文**代理的语义。
#   https 必须先 `CONNECT host:port` 建隧道、再在隧道上做 TLS。修前把 TLS 握手包直接
#   丢给 HTTP 代理 ⇒ 代理回 4xx 或直接断 ⇒ 失败以 Err 值返回，用户拿去索引就得到
#   `R1002: 此类型不支持索引: result`（与真因毫无关系的报文）。
#
# 判据（全部**本机离线**：门内自建代理 + 本机 origin，不碰外网、不借任何节点 —— M405 C6）：
#   X  前提：代理日志里必须出现 `CONNECT <host>:<port>` ⇒ **证明流量真的走了代理**
#      （若实现绕过代理直连，这一行根本不会出现 —— 判据当场抓）
#   A  https 经代理 + 代理回 403 ⇒ 报文必须是「代理 CONNECT 失败 … 403」这一族，
#      且**必须**带代理地址与被拒状态码（修前这里是 TLS 握手失败 / R1002 索引错误）
#   B  https 经代理 + 200 后立刻断 ⇒ CONNECT 目标必须是 `www.example.com:443`
#      （证明 CONNECT 行用的是**目标**而不是代理自己），错误是 TLS 阶段而非索引错误
#   C  明文 http 经代理 → **本机 origin** 200（绝对 URL 路径没被改坏）
#   D  代理不可达（127.0.0.1:1）⇒ 报文带代理地址（可读性）
#   N1 期望扰动：把 A 的期望改成修前的真实形态（TLS 握手失败）⇒ A 必红
#   N2 合法侧：不经代理直连本机 origin → 200（判据不是恒红）
#
# 用法：verify.sh [--neg-skip]
# 退出码：0=全绿 · 1=有红 · 2=门自身前置失败
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
PX="$ROOT/tools/px"
INTERP="${M408_INTERP:-$ROOT/bootstrap/pxi}"
WORK="${TMPDIR:-/tmp}/m408_proxy.$$"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
PROXY_PID=""; ORIGIN_PID=""; PROXY_HOLDER=""
# M430 缺陷 879：收尾必须 wait —— kill 只投递信号，不等它退出就返回时进程可能还在
#   LISTEN（端口扫描宽限 2s 之外仍算泄漏）。stop_proxy 在下面定义，trap 执行时已可用。
cleanup() {
    stop_proxy
    if [ -n "$ORIGIN_PID" ]; then
        kill "$ORIGIN_PID" 2>/dev/null
        wait "$ORIGIN_PID" 2>/dev/null
        ORIGIN_PID=""
    fi
    rm -rf "$WORK"
}
trap cleanup EXIT
PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
mkdir -p "$WORK"
command -v python3 >/dev/null || { echo "❌ 缺 python3"; exit 2; }
[ -x "$INTERP" ] || { echo "❌ 找不到解释器 $INTERP"; exit 2; }

# ── 起门内代理（随机端口）──────────────────────────────────
# ⚠️ M430 缺陷 879：**不能**用 `$(start_proxy ...)` —— 命令替换在子 shell 里跑，
#   后台 python 的 pid 落在子 shell 的 PROXY_PID 里，父 shell 永远看不到 ⇒ stop_proxy/
#   cleanup 一个都杀不到，三个 mini_proxy 全成 PPID=1 孤儿（2026-11-21 23:2x 实测 3 端口残留）。
#   ⇒ 端口经全局 PROXY_HOLDER 回传，函数本身不写 stdout。
start_proxy() {   # $1=mode  $2=logpath  → 端口写入 PROXY_HOLDER
    local mode="$1" log="$2"
    python3 "$HERE/mini_proxy.py" --mode "$mode" --log "$log" > "$WORK/proxy.$$.$mode.out" 2>&1 &
    PROXY_PID=$!
    local i=0
    while [ $i -lt 100 ]; do
        if grep -q '^PROXY-READY ' "$WORK/proxy.$$.$mode.out" 2>/dev/null; then
            local p; p=$(awk '/^PROXY-READY /{print $2; exit}' "$WORK/proxy.$$.$mode.out")
            PROXY_HOLDER="127.0.0.1:$p"; return 0
        fi
        sleep 0.1; i=$((i+1))
    done
    stop_proxy
    PROXY_HOLDER=""; return 1
}
stop_proxy() {
    if [ -n "$PROXY_PID" ]; then
        kill "$PROXY_PID" 2>/dev/null
        wait "$PROXY_PID" 2>/dev/null      # M430：必须等它真退出，否则端口还挂着
        PROXY_PID=""
    fi
}

# ── 本机 origin（明文 HTTP，供 C/N2 用）────────────────────
ORIGIN_PORT=$(( (RANDOM % 20000) + 20000 ))
mkdir -p "$WORK/origin"
printf 'ok\n' > "$WORK/origin/index.html"
( cd "$WORK/origin" && exec python3 -m http.server "$ORIGIN_PORT" --bind 127.0.0.1 > "$WORK/origin.log" 2>&1 ) &
ORIGIN_PID=$!
sleep 1

echo "═══ M408 · http_request 代理 CONNECT（PX-017）门 ═══"

# 前提：源码在位（不许"门绿但功能不在"）
if grep -q 'px_proxy_connect_tunnel(pfd, proxy, host, port' runtime/runtime.c; then
    ok "[0] 源码在 CONNECT 位上（https+代理分支走 px_proxy_connect_tunnel）"
else
    bad "[0] 源码**没有** CONNECT 调用 ⇒ 本门失去前提（拒绝判绿）"
fi

run_px() {   # $1=program  → stdout+stderr 合并
    timeout 60 "$INTERP" "$1" 2>&1
}

# ── [X]+[A] deny 代理：必须报「代理 CONNECT 失败 … 403」────────────────
LOG_DENY="$WORK/proxy_deny.log"
if start_proxy deny "$LOG_DENY"; then PD="$PROXY_HOLDER"; else bad "A 门内代理起不来"; PD=""; fi
if [ -n "$PD" ]; then
    cat > "$WORK/a.px" <<PXEOF
var r = http_request("https://www.example.com", "GET", null, null, {"proxy": "$PD", "timeout_ms": 6000})
if r.is_err():
    print("ERR " + str(r.err()))
else:
    print("OK " + str(r["status"]))
PXEOF
    OUT_A="$(run_px "$WORK/a.px")"
    echo "     实测：$OUT_A"
    case "$OUT_A" in
        *"ERR net: 代理 CONNECT 失败 "*"403"*) ok "A https 经代理被拒 ⇒ 可读报文（含代理地址与 403）" ;;
        *) bad "A 报文不符（期望「net: 代理 CONNECT 失败 … 403」）：$OUT_A" ;;
    esac
    case "$OUT_A" in
        *"此类型不支持索引"*) bad "A 仍出现 R1002 索引错误（修前形态）" ;;
        *) ok "A' 不再退化成「此类型不支持索引」" ;;
    esac
    if grep -q "^CONNECT www.example.com:443$" "$LOG_DENY" 2>/dev/null; then
        ok "X 代理日志有 CONNECT www.example.com:443 ⇒ 流量真走了代理（不是直连）"
    else
        bad "X 代理日志里没有 CONNECT 行 ⇒ 实现可能绕过了代理（日志：$(tr '\n' '|' < "$LOG_DENY" 2>/dev/null | cut -c1-80)）"
    fi
    # N1：把期望改成修前形态 ⇒ A 必红
    if [ "$NEG" -eq 1 ]; then
        case "$OUT_A" in
            *"net: TLS 握手失败"*) bad "N1 扰动未生效（输出本就是修前形态）" ;;
            *) ok "N1 期望扰动（修前形态 = TLS 握手失败）⇒ A 必红（判据有牙）" ;;
        esac
    fi
    stop_proxy
fi

# ── [B] 200 后立刻断：CONNECT 目标必须是 www.example.com:443 ──────────
LOG_OK="$WORK/proxy_close.log"
if start_proxy close "$LOG_OK"; then PC="$PROXY_HOLDER"; else bad "B 门内代理起不来"; PC=""; fi
if [ -n "$PC" ]; then
    cat > "$WORK/b.px" <<PXEOF
var r = http_request("https://www.example.com", "GET", null, null, {"proxy": "$PC", "timeout_ms": 6000})
if r.is_err():
    print("ERR " + str(r.err()))
else:
    print("OK " + str(r["status"]))
PXEOF
    OUT_B="$(run_px "$WORK/b.px")"
    echo "     实测：$OUT_B"
    if grep -q "^CONNECT www.example.com:443$" "$LOG_OK" 2>/dev/null; then
        ok "B CONNECT 目标 = www.example.com:443（用目标而非代理自己）"
    else
        bad "B CONNECT 目标不对（日志：$(tr '\n' '|' < "$LOG_OK" 2>/dev/null | cut -c1-80)）"
    fi
    case "$OUT_B" in
        *"此类型不支持索引"*) bad "B 退化成 R1002 索引错误" ;;
        *) ok "B' 失败形态不再是索引错误（隧道已建，TLS 阶段失败属预期）" ;;
    esac
    stop_proxy
fi

# ── [C] 明文 http 经代理 → 本机 origin 200 ───────────────────
LOG_PLAIN="$WORK/proxy_plain.log"
if start_proxy ok "$LOG_PLAIN"; then PP="$PROXY_HOLDER"; else bad "C 门内代理起不来"; PP=""; fi
if [ -n "$PP" ]; then
    cat > "$WORK/c.px" <<PXEOF
var r = http_request("http://127.0.0.1:$ORIGIN_PORT/index.html", "GET", null, null, {"proxy": "$PP", "timeout_ms": 6000})
if type(r) == "result" and r.is_err():
    print("ERR " + str(r.err()))
else:
    print("STATUS " + str(r["status"]))
PXEOF
    OUT_C="$(run_px "$WORK/c.px")"
    echo "     实测：$OUT_C"
    case "$OUT_C" in
        *"STATUS 200"*) ok "C 明文经代理 = 200（绝对 URL 路径未改坏）" ;;
        *) bad "C 明文经代理失败：$OUT_C" ;;
    esac
    stop_proxy
fi

# ── [D] 代理不可达 ⇒ 报文带代理地址 ─────────────────────────
cat > "$WORK/d.px" <<'PXEOF'
var r = http_request("https://www.example.com", "GET", null, null, {"proxy": "127.0.0.1:1", "timeout_ms": 2000})
if r.is_err():
    print("ERR " + str(r.err()))
else:
    print("OK " + str(r["status"]))
PXEOF
OUT_D="$(run_px "$WORK/d.px")"
echo "     实测：$OUT_D"
case "$OUT_D" in
    *"ERR "*"127.0.0.1:1"*) ok "D 代理不可达 ⇒ 报文含代理地址（可读）" ;;
    *) bad "D 报文没指明代理地址：$OUT_D" ;;
esac

# ── [N2] 不经代理直连本机 origin ⇒ 200（判据不是恒红）────────
cat > "$WORK/n2.px" <<PXEOF
var r = http_request("http://127.0.0.1:$ORIGIN_PORT/index.html", "GET", null, null, {"timeout_ms": 6000})
if type(r) == "result" and r.is_err():
    print("ERR " + str(r.err()))
else:
    print("STATUS " + str(r["status"]))
PXEOF
OUT_N2="$(run_px "$WORK/n2.px")"
echo "     实测：$OUT_N2"
case "$OUT_N2" in
    *"STATUS 200"*) ok "N2 直连（无代理）= 200 ⇒ 判据不是恒红" ;;
    *) bad "N2 直连失败（被误判）：$OUT_N2" ;;
esac

echo "── 结果：PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
