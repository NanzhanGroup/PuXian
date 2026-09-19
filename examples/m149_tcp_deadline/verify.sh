#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M149 验证：TCP「带超时 + 可辨别失败」族（tcp_connect_ex / tcp_opt /
#            tcp_recv_ex / tcp_send_ex）
# ---------------------------------------------------------------
# 门做五件事：
#   ① VM 轨：受控服务端（独立进程）+ 客户端断言集 → M149_ASSERT: nP/0F
#   ② C 轨：同上（服务端也用 C 轨编译）⇒ 与 VM 轨输出**逐字节一致**
#   ③ 解释轨：interp_smoke.px（无服务端通路）→ M149_SMOKE: 9P/0F
#      （解释器无 spawn / 无 tcp_listen 名册项 ⇒ 起不了服务端，见该文件头注释）
#   ④ 负控 A/B/C：把三处关键语义分别退回错误语义 ⇒ 门必须变红
#        A：tcp_connect_ex 忽略 timeout_ms（连接超时面必须被覆盖）
#        B：tcp_connect_ex 默认不设 NODELAY（Go net.Dial 默认开启 ⇒ D4 必红）
#        C：tcp_recv_ex 把 EOF 报成非 EOF（F2 必红）
#      （negative control 是「这条面到底跑没跑」的硬判据；每条都**备份/还原**
#        runtime/runtime.c 本体。）
#   ⑤ 旧接口对照行（INFO K）：旧 tcp_recv 在超时/EOF 都返回 ""，证明新接口
#      补的是**可辨别性**，不是"再包一层"。
#
# 口径：断言里凡涉及时间只取**宽上下界**；服务端与客户端分进程 ⇒ 不受协程
#       调度语义影响；端口从 19741 起找第一个空闲（可用 M149_PORT_OVERRIDE 固定）。
# 用法：bash examples/m149_tcp_deadline/verify.sh
# 退出码：0 = M149-VERIFY-OK；1 = 有失败项
# ═══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")"
PX=../../tools/px
RT=../../runtime/runtime.c
LOG=/tmp/m149_build.log
FAIL=0
mkdir -p build

# ── 端口：找第一个空闲 ──
PORT=""
if [ -n "${M149_PORT_OVERRIDE:-}" ]; then
    PORT="$M149_PORT_OVERRIDE"
else
    for p in $(seq 19741 19800); do
        if python3 -c "
import socket,sys
s=socket.socket(); s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
try:
    s.bind(('', $p))
except OSError:
    sys.exit(1)
s.close()
" 2>/dev/null; then PORT=$p; break; fi
    done
fi
[ -n "$PORT" ] || { echo "FAIL 找不到空闲端口（19741-19800）"; exit 1; }
echo "== [配置] 端口 $PORT =="

SRV_PID=""
start_server() {
    M149_PORT="$PORT" setsid nohup ./build/m149_server > /tmp/m149_srv.out 2>&1 &
    SRV_PID=$!
    local i=0
    while [ $i -lt 50 ]; do
        if grep -q '^M149SRV listening' /tmp/m149_srv.out 2>/dev/null; then return 0; fi
        sleep 0.1
        i=$((i + 1))
    done
    echo "FAIL 服务端未就绪"; cat /tmp/m149_srv.out 2>/dev/null; return 1
}
stop_server() {
    if [ -n "$SRV_PID" ]; then
        kill -TERM "$SRV_PID" 2>/dev/null || true
        sleep 0.2
        kill -KILL "$SRV_PID" 2>/dev/null || true
        wait "$SRV_PID" 2>/dev/null || true
        SRV_PID=""
    fi
}
trap 'stop_server' EXIT

build_one() {
    if [ "$2" = "c" ]; then
        env PX_BUILD_ENGINE=c "$PX" build "$1" > "$LOG" 2>&1
    else
        "$PX" build "$1" > "$LOG" 2>&1
    fi
}

run_track() {
    local t="$1" out="/tmp/m149_$1.out" rc=0
    echo "== [$t 轨] 编译服务端与客户端 =="
    build_one m149_server.px "$t" || { echo "FAIL [$t] 服务端编译失败"; tail -15 "$LOG"; FAIL=$((FAIL+1)); return; }
    build_one tcp_deadline.px "$t" || { echo "FAIL [$t] 客户端编译失败"; tail -15 "$LOG"; FAIL=$((FAIL+1)); return; }
    stop_server
    rm -f /tmp/m149_srv.out
    start_server || { FAIL=$((FAIL+1)); return; }
    timeout 90 env M149_PORT="$PORT" ./build/tcp_deadline > "$out" 2>&1; rc=$?
    stop_server
    if [ "$rc" != "0" ]; then
        echo "FAIL [$t] 客户端运行 rc=$rc（124=挂起 ⇒ 超时面没生效）"
        tail -20 "$out" | sed 's/^/   /'
        FAIL=$((FAIL+1))
        return
    fi
    local n
    n=$(grep -c '^PASS ' "$out" || true)
    echo "   通过 $n / 失败 $(grep -c '^FAIL ' "$out" || true) / 跳过 $(grep -c '^SKIP ' "$out" || true)"
    grep -E '^(SKIP|INFO) ' "$out" | sed 's/^/   /'
    if ! grep -qE '^M149_ASSERT: [0-9]+P/0F(/[0-9]+S)?$' "$out"; then
        echo "FAIL [$t] 断言未全绿："
        grep -E '^(FAIL |M149_ASSERT)' "$out" | sed 's/^/   /'
        FAIL=$((FAIL+1))
        return
    fi
    if [ "$n" -lt 40 ]; then
        echo "FAIL [$t] 通过项数 $n < 40（防门空转）"
        FAIL=$((FAIL+1))
        return
    fi
    if ! grep -q 'i/o timeout' "$out"; then
        echo "FAIL [$t] 未见读超时语义（E4 面未跑到）"; FAIL=$((FAIL+1)); return
    fi
    if ! grep -q '^INFO K.旧接口对照' "$out"; then
        echo "FAIL [$t] 缺旧接口对照行"; FAIL=$((FAIL+1)); return
    fi
    echo "   PASS [$t] M149_ASSERT $(grep -E '^M149_ASSERT' "$out")"
}

run_track vm
run_track c

echo "--- VM / C 两轨输出逐字节一致（**走钟字段归一**：INFO *.dt 的毫秒数）---"
norm() { sed -E 's/^(INFO [A-Z]\.)dt dt=[0-9]+ms/\1dt dt=<ms>/' "$1"; }
if [ -f /tmp/m149_vm.out ] && [ -f /tmp/m149_c.out ] \
   && diff <(norm /tmp/m149_vm.out) <(norm /tmp/m149_c.out) > /dev/null 2>&1; then
    echo "PASS 两轨一致（归一后；原始差异仅 INFO *.dt 的实测毫秒）"
else
    echo "FAIL 两轨不一致："
    diff <(norm /tmp/m149_vm.out) <(norm /tmp/m149_c.out) 2>/dev/null | head -10 | sed 's/^/   /'
    FAIL=$((FAIL+1))
fi

echo "== [解释轨] interp_smoke.px（无服务端通路；三轨都要过）=="
for mode in run vm c; do
    out="/tmp/m149_smoke_$mode.out"
    rc=1
    case "$mode" in
        run) "$PX" run interp_smoke.px > "$out" 2>&1; rc=$? ;;
        vm)  build_one interp_smoke.px "" >/dev/null 2>&1 && { ./build/interp_smoke > "$out" 2>&1; rc=$?; } ;;
        c)   build_one interp_smoke.px c >/dev/null 2>&1 && { ./build/interp_smoke > "$out" 2>&1; rc=$?; } ;;
    esac
    if [ "$rc" != "0" ] || ! grep -q '^M149_SMOKE: 9P/0F$' "$out"; then
        echo "FAIL 解释轨冒烟（$mode）：rc=$rc"
        tail -15 "$out" | sed 's/^/   /'
        FAIL=$((FAIL+1))
    else
        echo "   PASS 冒烟 $mode：$(grep -E '^M149_SMOKE' "$out")"
    fi
done

# ── 负控：篡改 runtime.c → 重建客户端 → 门必须变红 ──
negctl_explicit() {
    local tag="$1" old="$2" new="$3" why="$4"
    echo "== [负控 $tag] 篡改 runtime.c ⇒ 门必须变红（$why）=="
    cp "$RT" /tmp/m149_runtime_keep.c
    local pok=1
    OLD="$old" NEW="$new" python3 - <<'PY' || pok=0
import os
p = "../../runtime/runtime.c"
s = open(p, encoding="utf-8").read()
old, new = os.environ["OLD"], os.environ["NEW"]
if s.count(old) != 1:
    raise SystemExit(2)
open(p, "w", encoding="utf-8").write(s.replace(old, new, 1))
PY
    if [ "$pok" != "1" ]; then
        echo "FAIL 负控 $tag：篡改未生效（锚点与源码不同步）"
        FAIL=$((FAIL+1)); cp /tmp/m149_runtime_keep.c "$RT"; return
    fi
    local verdict="green"
    stop_server; rm -f /tmp/m149_srv.out
    build_one m149_server.px "" > /dev/null 2>&1 || true
    if build_one tcp_deadline.px "" > "$LOG" 2>&1; then
        start_server || true
        timeout 90 env M149_PORT="$PORT" ./build/tcp_deadline > "/tmp/m149_neg_$tag.out" 2>&1; local rc=$?
        stop_server
        if [ "$rc" = "0" ] && grep -qE '^M149_ASSERT: [0-9]+P/0F(/[0-9]+S)?$' "/tmp/m149_neg_$tag.out"; then
            verdict="green"
        else
            verdict="red"
        fi
    else
        verdict="red（编译失败）"
    fi
    cp /tmp/m149_runtime_keep.c "$RT"
    if [ "$verdict" = "green" ]; then
        echo "FAIL 负控 $tag：篡改后门仍绿（该面没被覆盖）"
        grep -E '^(FAIL |M149_ASSERT)' "/tmp/m149_neg_$tag.out" 2>/dev/null | head -5 | sed 's/^/   /'
        FAIL=$((FAIL+1))
    else
        echo "PASS 负控 $tag：篡改后门变红（$verdict）"
    fi
}

# ⚠️ 锚点必须**唯一定位**：M150（第 32 轮）把同一对钳位语句也写进了 `bi_tls_connect`
#   ⇒ 原先两行的锚点变成"出现 2 次"，本门报 **"篡改未生效（锚点与源码不同步）"**。
#   这不只是改错——**它证明了门在自查"锚点是否还唯一"**。此处把锚点加长到含 `int fd = ...`
#   那一行（`px_tcp_connect_timeout` 的调用形态只在 `bi_tcp_connect_ex` 里出现一次）。
negctl_explicit A '    if (timeout_ms < 0) timeout_ms = 0;
    if (timeout_ms > 2147483647LL) timeout_ms = 2147483647LL;
    int stage = 0, er = 0;
    char addr[128];
    addr[0] = 0;
    int fd = px_tcp_connect_timeout(host, port, (int)timeout_ms, &stage, &er, addr, (int)sizeof(addr));' '    timeout_ms = 0;   /* NEGCTL-149A */
    int stage = 0, er = 0;
    char addr[128];
    addr[0] = 0;
    int fd = px_tcp_connect_timeout(host, port, (int)timeout_ms, &stage, &er, addr, (int)sizeof(addr));' '连接超时面（黑洞连接必须按 timeout_ms 返回）'
negctl_explicit B '    int64_t timeout_ms = 0;
    int nodelay = 1;' '    int64_t timeout_ms = 0;
    int nodelay = 0;   /* NEGCTL-149B */' 'NODELAY 默认（Go net.Dial 默认开启）'
negctl_explicit C '    int ok = (n >= 0);
    int eof = (n == 0);' '    int ok = (n >= 0);
    int eof = 0;   /* NEGCTL-149C */' 'EOF 可辨别（对端关闭）'

echo "== [还原] 确认 runtime.c 无篡改残留 =="
if grep -q 'NEGCTL-149' "$RT"; then
    echo "FAIL 负控残留：runtime.c 仍含 NEGCTL 标记"; FAIL=$((FAIL+1))
else
    echo "   PASS 无篡改残留"
fi
build_one tcp_deadline.px "" > /dev/null 2>&1 || true

if [ "$FAIL" = "0" ]; then
    echo "M149-VERIFY-OK"
    exit 0
fi
echo "M149-VERIFY-FAIL（$FAIL 项）"
exit 1
