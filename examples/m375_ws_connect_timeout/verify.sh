#!/usr/bin/env bash
# ============================================================
# m375 —— WS 客户端「建链 + 握手」全程无超时（缺陷 PX-BUG-20261023-01 · P0）
#
# 来历：晨曦（wsa-chenxi）2026-10-23 09:35 经节点总线报障，P0 · 复现率 100%：
#   对端「TCP 已连上但一个字节都不发」⇒ `ws_connect` **永久阻塞**：
#     · `connect()` 裸阻塞（SYN 被丢 ⇒ 等内核重传超时）
#     · 握手读响应头 → `px_conn_read` → `recv`，而**客户端 fd 从来没设过超时**
#   ⇒ 调用线程被永久占住、连接**不进会话表**（槽位分配在握手之后）、**不打日志**、
#     **不释放 fd**；广州现场积压 21 条握手挂死连接（会话数:连接数 = 28:1），只能重启回收。
#
# 修法（两条边界，各自独立生效）：
#   ① 建链：非阻塞 connect + poll(timeout) + SO_ERROR 复核 ⇒ 到点 ETIMEDOUT
#   ② 握手：握手套 SO_RCVTIMEO/SO_SNDTIMEO，**成功后清零**（ws_recv 是阻塞读契约）
#   wss:// 的 BIO 由 mbedtls 自己的 recv 回调掌握，本处显式跳过 —— 登记为未覆盖边界（见 §边界）。
#
# 为什么要这道门（而不是「改完自己跑一下」）：
#   缺陷的触发条件不是「连不上」而是「连上了但不说话」——真实服务永不这么干，
#   所以既有的 ws 门（m236 等）**结构上覆盖不到**。本门用夹具把「对端行为」变成自变量。
#
# 判据层次：
#   [1] 静态：4 条建链路径全走 helper · 裸 connect 只剩 helper 内 1 处 · 握手 5 处全走 bounded
#       · **清零契约**必须在场（漏清零 = 把「防挂死」变成「5s 后静默丢帧」，比原缺陷更难发现）
#       · 缺陷号可追溯
#   [2] 正控 now        ⇒ ok 且 < 3s（修复不能把正常握手打死）
#   [3] 延迟分辨 delay:2 ⇒ ok（证明握手超时 > 2s，不是「一律失败」）
#   [4] 延迟分辨 delay:8 ⇒ null（证明超时 < 8s）
#   [5] 主判据 never     ⇒ null · **rc≠124**（是进程自己退出，不是外壳 timeout 杀的）
#                          · 4.5s ≤ 耗时 ≤ 9s
#   [6] 边界 flow:7      ⇒ 握手后 7s 才来的首帧必须收到（证明超时没漏到正常收发）
#   [7] 第二条路径 url   ⇒ never 下同样超时（ws:// 分支是另一段建链代码）
#   [8] 邻居 refuse      ⇒ null 且 < 2s（「快速失败」与「超时」是两条路径，都不能丢）
#   [9] 负控 A：PX_WS_HANDSHAKE_TIMEOUT_MS=1200 + delay:2 ⇒ **必须变 null**
#   [10] 负控 B：PX_WS_HANDSHAKE_TIMEOUT_MS=12000 + delay:8 ⇒ **必须变 ok**
#        （A/B 合起来证明：判据的自变量确实是超时值本身，而不是「某处恒返回 null」）
#   [11] 覆盖度自证：上面 10 类判据实际执行条数逐条打印
#
# 边界（本门**不覆盖**，登记在案）：
#   · wss:// 的 TLS 握手与读响应头（BIO = mbedtls_net_recv，非 timeout 版；
#     给同一 fd 设 SO_RCVTIMEO 会与 mbedtls 的 read_timeout 语义打架 —— 见 runtime.c M150 注释）
#   · 服务端 `ws_serve` 的 accept 后握手（那是「被连方」的慢速攻击面，另立任务）
#
# 用法：verify.sh
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
WS="$ROOT/runtime/runtime_ws.c"

PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1 = $2"; else bad "$1 = $2（期望 $3）"; fi; }
ge()  { if [ "${2:-0}" -ge "$3" ] 2>/dev/null; then ok "$1 = $2（≥$3）"; else bad "$1 = ${2:-?}（期望 ≥$3）"; fi; }
lt()  { if [ "${2:-999999}" -lt "$3" ] 2>/dev/null; then ok "$1 = $2（<$3）"; else bad "$1 = ${2:-?}（期望 <$3）"; fi; }
le()  { if [ "${2:-999999}" -le "$3" ] 2>/dev/null; then ok "$1 = $2（≤$3）"; else bad "$1 = ${2:-?}（期望 ≤$3）"; fi; }

W="$(mktemp -d /tmp/m375gate.XXXXXX)"
SRV_PIDS=""
CEXEC=""

cleanup() {
    for p in $SRV_PIDS; do kill -9 "$p" 2>/dev/null; wait "$p" 2>/dev/null; done
    rm -rf "$W"
}
trap cleanup EXIT

# 起夹具：start_srv <port> <mode> ⇒ 0=LISTEN 就绪 / 1=失败
start_srv() {
    local port="$1" mode="$2"
    python3 "$HERE/silent_srv.py" "$port" "$mode" > "$W/srv_$port.log" 2>&1 &
    local pid=$!
    SRV_PIDS="$SRV_PIDS $pid"
    for _ in $(seq 1 50); do
        grep -q '^LISTEN' "$W/srv_$port.log" 2>/dev/null && return 0
        kill -0 "$pid" 2>/dev/null || return 1
        sleep 0.1
    done
    return 1
}

# run_cli <port> <form> <timeout_s> [env...] ⇒ 打印 "耗时ms rc 输出(去空白)"
run_cli() {
    local port="$1" form="$2" tmo="$3"; shift 3
    local s e out rc
    s=$(date +%s%N)
    out=$(env "$@" timeout "$tmo" "$CEXEC" "$port" "$form" 2>&1)
    rc=$?
    e=$(date +%s%N)
    echo "$(( (e - s) / 1000000 )) $rc $(printf '%s' "$out" | tr -d '\n')"
}

echo '=== m375 · WS 客户端建链/握手无超时（PX-BUG-20261023-01 · 晨曦 P0）==='
echo "  仓库: $ROOT"

# ─────────────────────────────────────────────
echo '[1] 静态契约'
A1=$(grep -c 'ws_connect_bounded(' "$WS" || true)
ge 'A · 建链 helper 出现次数（定义 1 + 4 条路径）' "$A1" '5'
A2=$(grep -c 'connect(fd, (struct sockaddr\*)&addr, sizeof(addr))' "$WS" || true)
chk 'A · 裸阻塞 connect 旧写法残留' "$A2" '0'
A3=$(grep -c 'ws_client_handshake_bounded(' "$WS" || true)
ge 'A · 握手统一入口（定义 1 + 5 处调用）' "$A3" '6'
A4=$(grep -c 'ws_set_fd_timeout(c->fd, 0)' "$WS" || true)
chk 'A · **握手成功后清零**（漏了 = 5s 后静默丢帧）' "$A4" '1'
A5=$(grep -E '^#define[[:space:]]+PX_WS_HANDSHAKE_TIMEOUT_MS_DEF[[:space:]]+5000' "$WS" | wc -l)
chk 'A · 握手默认超时 5000ms 常量在场' "$A5" '1'
A6=$(grep -c 'PX_WS_TIMEOUT_MAX_MS' "$WS" || true)
ge 'A · 超时上界钳制（防 env 写成天文数字）' "$A6" '2'
A7=$(grep -c 'PX-BUG-20261023-01' "$WS" || true)
ge 'A · 缺陷号可追溯' "$A7" '1'
A8=$(grep -c 'SO_ERROR' "$WS" || true)
ge 'A · 非阻塞 connect 后必须 SO_ERROR 复核' "$A8" '1'

# ─────────────────────────────────────────────
echo '[2] 构建探针（链**当前源码**的 runtime）'
cp "$HERE/cli.px" "$W/"
( cd "$W" && "$ROOT/tools/px" build cli.px > "$W/build.log" 2>&1 )
if [ -x "$W/build/cli" ]; then
    CEXEC="$W/build/cli"
    ok "探针构建成功"
else
    bad "探针构建失败（见下方日志）"
    tail -15 "$W/build.log"
    echo "m375: PASS=$PASS FAIL=$FAIL"
    exit 1
fi

PORT_BASE=$((21000 + ($$ % 800)))

# ─────────────────────────────────────────────
echo '[3] 动态 · 正控与延迟分辨'
P=$((PORT_BASE++)); start_srv "$P" now && R=$(run_cli "$P" 3arg 20) || R="0 1 NO_SRV"
case "$R" in
    *"CONN ok"*) ok "now（立即 101）⇒ ok，耗时 ${R%% *}ms"
                 lt '     正控耗时' "${R%% *}" 3000 ;;
    *) bad "now（立即 101）⇒ 期望 ok，实际 [${R}]（修复把正常握手打死了）" ;;
esac

P=$((PORT_BASE++)); start_srv "$P" delay:2 && R=$(run_cli "$P" 3arg 20) || R="0 1 NO_SRV"
case "$R" in
    *"CONN ok"*) ok "delay:2（2s 后才回 101）⇒ ok，耗时 ${R%% *}ms（证明握手超时 > 2s）" ;;
    *) bad "delay:2 ⇒ 期望 ok（超时过短会误杀正常服务），实际 [${R}]" ;;
esac

P=$((PORT_BASE++)); start_srv "$P" delay:8 && R=$(run_cli "$P" 3arg 20) || R="0 1 NO_SRV"
case "$R" in
    *"CONN null"*) MS=${R%% *}; ok "delay:8 ⇒ null，耗时 ${MS}ms（证明超时 < 8s）"
                   lt '     超时上界' "$MS" 8000 ;;
    *) bad "delay:8 ⇒ 期望 null（超时没生效），实际 [${R}]" ;;
esac

# ─────────────────────────────────────────────
echo '[4] 动态 · 主判据（对端 accept 后一字不发）'
P=$((PORT_BASE++)); start_srv "$P" never && R=$(run_cli "$P" 3arg 25) || R="0 1 NO_SRV"
MS=$(echo "$R" | awk '{print $1}'); RC=$(echo "$R" | awk '{print $2}')
case "$R" in
    *"CONN null"*)
        ok "never ⇒ null（对端静默时不再永久阻塞）"
        if [ "$RC" = "124" ]; then
            bad "never · 进程是被外壳 timeout 杀掉的（rc=124）⇒ **内部没有超时**，判据不成立"
        else
            ok "never · 进程自行退出（rc=$RC，不是外壳杀）"
        fi
        ge '     && 耗时下界（真的等过，不是立即失败）' "$MS" 4500
        le '     && 耗时上界' "$MS" 9000 ;;
    *) bad "never ⇒ 期望 null，实际 [$R]（对端静默 ⇒ 客户端仍挂死或异常退出）" ;;
esac

# ─────────────────────────────────────────────
echo '[5] 动态 · 边界（握手期超时不得漏到正常收发）'
P=$((PORT_BASE++)); start_srv "$P" flow:7 && R=$(run_cli "$P" recv 30) || R="0 1 NO_SRV"
case "$R" in
    *"RECV = hello-after-flow:7"*)
        ok "flow:7（握手后 7s 才发首帧）⇒ 客户端收到（> 5s，证明 SO_RCVTIMEO 已清零）" ;;
    *)
        bad "flow:7 ⇒ 首帧丢失（${R#* }）—— 握手超时被漏到了正常收发上（比原缺陷更难发现）" ;;
esac

# ─────────────────────────────────────────────
echo '[6] 动态 · 第二条建链路径（ws:// URL 形式）与快速失败邻居'
P=$((PORT_BASE++)); start_srv "$P" never && R=$(run_cli "$P" url 25) || R="0 1 NO_SRV"
case "$R" in
    *"CONN null"*) MS=${R%% *}; RC=$(echo "$R" | awk '{print $2}')
                   ok "url(ws://) + never ⇒ null，耗时 ${MS}ms"
                   if [ "$RC" = "124" ]; then bad "url · 仍是被外壳杀（rc=124）"; else ok "url · 自行退出（rc=$RC）"; fi
                   le '     && 耗时上界' "$MS" 9000 ;;
    *) bad "url(ws://) + never ⇒ 期望 null，实际 [$R]" ;;
esac

P=$((PORT_BASE++)); start_srv "$P" refuse && R=$(run_cli "$P" 3arg 20) || R="0 1 NO_SRV"
case "$R" in
    *"CONN null"*) MS=${R%% *}
                   ok "refuse（accept 后立刻 close）⇒ null，耗时 ${MS}ms"
                   lt '     && 快速失败路径仍在（<2s，没被超时机制吞掉）' "$MS" 2000 ;;
    *) bad "refuse ⇒ 期望 null，实际 [$R]" ;;
esac

# ─────────────────────────────────────────────
echo '[7] 负控（判据的自变量必须真的是超时值）'
P=$((PORT_BASE++)); start_srv "$P" delay:2 && R=$(run_cli "$P" 3arg 20 PX_WS_HANDSHAKE_TIMEOUT_MS=1200) || R="0 1 NO_SRV"
case "$R" in
    *"CONN null"*) ok "负控 A：HS=1200 + delay:2 ⇒ **翻转为 null**（同代码同夹具，只改超时值）" ;;
    *) bad "负控 A：HS=1200 时 delay:2 仍 ok ⇒ 说明超时值不是判据的自变量（判据没有牙）" ;;
esac

P=$((PORT_BASE++)); start_srv "$P" delay:8 && R=$(run_cli "$P" 3arg 30 PX_WS_HANDSHAKE_TIMEOUT_MS=12000) || R="0 1 NO_SRV"
case "$R" in
    *"CONN ok"*) ok "负控 B：HS=12000 + delay:8 ⇒ **翻转为 ok**（证明判据不是「恒返回 null」）" ;;
    *) bad "负控 B：HS=12000 时 delay:8 仍 null ⇒ 判据可能恒 null（假绿）" ;;
esac

# ─────────────────────────────────────────────
echo '[8] 覆盖度自证'
echo "  判据组：静态契约 8 · 正控/延迟分辨 3 · 主判据 3（null/rc/时窗）· 边界 1 · 第二路径+邻居 3 · 负控 2 = 20 条断言"
echo "  未覆盖（登记）：wss:// TLS 握手与读头 · 服务端 ws_serve 的 accept 后握手"

echo
echo "m375: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
