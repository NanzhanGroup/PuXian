#!/usr/bin/env bash
# ============================================================
# m376 —— SIGTERM 时丢弃「已建连、未 accept」的连接（缺陷 659 · 自 m176 红项定性而来）
#
# 病灶（实测行号，M375 全量门首次暴露）：
#   px_sigstop_handler 里 `shutdown(listen_fd, SHUT_RDWR)` 是为了把**裸 accept** 从阻塞中拉出来
#   （glibc BSD 语义下裸 accept 被 SA_RESTART 自动重启，信号叫不醒它）。
#   代价：shutdown 后 accept 立即失败 → 主循环 break → close(sfd)
#   ⇒ 内核 accept 队列里「已完成三次握手、尚未 accept」的连接被 RST
#   ⇒ 客户端 connect 成功（refused=0）随后读到 EOF（m176 的 bad=1）。
#
# 修法（4 处，见 runtime.c 注释）：
#   ① px_io_accept：裸 accept → poll(POLLIN,100ms) + accept（signal(7)：poll 永不因 SA_RESTART 重启）
#   ② 信号处理器：去掉 shutdown，只置位 g_px_stop
#   ③ px_serve：停止前非阻塞排空 accept 队列 → 入池（顺序在广播 worker 退出之前）
#   ④ fserve HTTP/UNIX/SSE + ws_serve 四个循环补 g_px_stop 检查（否则②之后它们永久阻塞）
#
# 为什么这道门要用「SIGSTOP 构造」而不是「边打边停」：
#   旧夹具（m376_sigterm_drain.sh，边打边 TERM）窗口只有微秒级 —— 修前也只 3/10 轮出现 bad，
#   修后照样出现 ⇒ **判据无牙**（自欺）。本门用 SIGSTOP 把 accept 冻住，让 K 条连接**必然**
#   堆在内核队列里，再投递 SIGTERM ⇒ 停止瞬间队列非空是**构造出来的**，不靠调度与负载。
#
# 判据：
#   [前提] 队列长度 ≥ 8（从 /proc/net/tcp 的 LISTEN rx_queue 读）—— 不成立则判「门坏」rc=2，
#          而不是判绿（CRITERIA §4：负控必须有前提自证）
#   [1] 静态：信号处理器内**没有** shutdown( · px_io_accept 走 poll · 排空段在场
#            · 4 个循环的 g_px_stop 检查在场（计数）
#   [2] 主判据：K=12 条排队连接，SIGTERM 后 **ok=12 / bad=0 / refused=0**，进程 3s 内自行退出
#            （不经 kill -9）—— 3 轮全过
#   [3] 负控：把 shutdown 塞回信号处理器（还原缺陷行为）⇒ 主判据必须红 + sha256 逐字节还原复绿
#   CI 用 --neg-skip（负控要重建 runtime，约 40s/次）。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [m376] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
RT="$ROOT/runtime/runtime.c"
D=examples/m376_sigterm_drain
W=/tmp/m376_gate
BAK=/tmp/m376_runtime.bak
K=12
ROUNDS=3
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
PXI="${PX_PXI_BIN:-$ROOT/bootstrap/pxi}"
pass=0; fail=0; bad_gate=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }
snapshot() { cp -f "$RT" "$BAK"; }
restore_all() { [ -f "$BAK" ] && cp -f "$BAK" "$RT"; }
trap 'restore_all; [ -n "${SPID:-}" ] && kill -9 $SPID 2>/dev/null; rm -f "$BAK"' EXIT

# 监听行在 /proc/net/tcp 里 = 0.0.0.0:<PORT> 状态 0A，第 5 字段 tx:rx，rx = 当前 accept 队列长度。
# 用 /proc 而不是 ss：CI 容器里不一定有 iproute2；且这条读法不产生任何连接（无副作用）。
qlen() {
    local port=$1 hex line qh
    hex=$(printf '%04X' "$port")
    line=$(awk -v a="00000000:$hex" '$2==a && $4=="0A" {print $5; exit}' /proc/net/tcp 2>/dev/null) || return 1
    [ -z "$line" ] && return 1
    qh=${line#*:}
    echo $((16#$qh))
}

build_srv() { (cd "$ROOT" && ./tools/px build "$D/srv.px" > "$W/build.log" 2>&1); [ -x "$D/build/srv" ]; }

run_round() {   # $1=标签；结论写到 $W/sc.<标签>.log
    local tag=$1 port=$((19300 + (RANDOM % 400))) i line
    : > "$W/sc.$tag.log"
    setsid "$D/build/srv" "$port" > "$W/srv.$tag.log" 2>&1 & SPID=$!
    for i in $(seq 1 100); do line=$(qlen "$port" 2>/dev/null); [ -n "$line" ] && break; sleep 0.05; done
    if [ -z "$line" ]; then echo "NO-LISTEN" >> "$W/sc.$tag.log"; kill -9 $SPID 2>/dev/null; return 1; fi
    kill -STOP $SPID 2>/dev/null
    sleep 0.1
    for i in $(seq 1 $K); do "$PXI" "$port" "$D/probe.px" > "$W/p.$tag.$i.out" 2>/dev/null & done
    sleep 0.5
    local q; q=$(qlen "$port")
    echo "QUEUED=${q:-?}" >> "$W/sc.$tag.log"
    kill -TERM $SPID 2>/dev/null
    sleep 0.05
    kill -CONT $SPID 2>/dev/null
    local t0 t1 ms al=0
    t0=$(date +%s%N)
    for i in $(seq 1 60); do kill -0 $SPID 2>/dev/null || break; sleep 0.05; done
    t1=$(date +%s%N); ms=$(( (t1 - t0) / 1000000 ))
    kill -0 $SPID 2>/dev/null && { al=1; kill -9 $SPID 2>/dev/null; }
    wait $SPID 2>/dev/null
    local ok=0 bad=0 refused=0 other=0 v
    for i in $(seq 1 $K); do
        for _ in $(seq 1 40); do [ -s "$W/p.$tag.$i.out" ] && break; sleep 0.05; done
        v=$(head -1 "$W/p.$tag.$i.out" 2>/dev/null)
        case "$v" in
            ok) ok=$((ok+1));;
            refused) refused=$((refused+1));;
            bad*) bad=$((bad+1));;
            *) other=$((other+1));;
        esac
    done
    echo "ok=$ok bad=$bad refused=$refused other=$other exit_ms=$ms killed9=$al" >> "$W/sc.$tag.log"
    grep -o '停止前排空 accept 队列：入池 [0-9]* / 拒绝 [0-9]*' "$W/srv.$tag.log" >> "$W/sc.$tag.log" 2>/dev/null
    cat "$W/sc.$tag.log" | sed 's/^/     /'
}

# 主判据：ok==K · bad==0 · refused==0 · other==0 · killed9=0
main_ok() { grep -q "ok=$K bad=0 refused=0 other=0 exit_ms=[0-9]* killed9=0" "$W/sc.$1.log"; }

echo "=== [0] 静态：修复的四个支点都在场"
# ⚠️ 先剥注释再判：处理器里**注释**说明了「不再 shutdown」，注释里的 shutdown( 不是病灶
#   （CRITERIA §6 的反面用法：注释里的代码算代码，但注释里的**散文**不能当病灶）
HANDLER=$(awk '/static void px_sigstop_handler\(int sig\)/{f=1} f{print} f&&/^}/{exit}' "$RT" \
          | sed 's://.*$::' | awk '{ line=$0; sub(/^[ \t]+/, "", line); if (line !~ /^\*/ && line !~ /^\/\*/) print }')
chk "信号处理器代码内**没有** shutdown(（缺陷 659 的直接病灶）" "! printf '%s' \"\$HANDLER\" | grep -q 'shutdown('"
chk "信号处理器只置位 g_px_stop" "printf '%s' \"\$HANDLER\" | grep -q 'g_px_stop = 1;'"
chk "px_io_accept 用 poll 做等待原语（裸 accept 会被 SA_RESTART 重启）" "grep -q 'poll(&p, 1, PX_ACCEPT_POLL_MS)' \"$RT\""
chk "停止前排空 accept 队列的代码段在场" "grep -q '停止前排空 accept 队列' \"$RT\""
chk "四个服务循环的 g_px_stop 检查在场（≥5 处）" "[ \$(grep -c 'if (g_px_stop) break;' \"$RT\") -ge 5 ]"

echo "=== [1] 主场景：K=$K 条排队连接 · SIGTERM 后必须全部拿到 200 · $ROUNDS 轮"
if ! build_srv; then
    chk "构建 srv" "false"
    tail -20 "$W/build.log" | sed 's/^/     /'
else
    for r in $(seq 1 $ROUNDS); do
        echo "  --- round $r ---"
        run_round "pos$r"
        if ! grep -q 'QUEUED=' "$W/sc.pos$r.log"; then
            echo "  ❌ 前提不自证（无监听行）⇒ 判「门坏」，不判绿"; bad_gate=1; continue
        fi
        q=$(sed -n 's/^QUEUED=//p' "$W/sc.pos$r.log" | head -1)
        if [ "${q:-0}" -lt 8 ] 2>/dev/null; then
            echo "  ❌ 前提不自证：队列长度 $q < 8（SIGSTOP 构造失败）⇒ 判「门坏」，不判绿"; bad_gate=1; continue
        fi
        chk "round $r：ok=$K 全部 200 · bad=0 · refused=0 · 未被 kill -9（队列 $q）" "main_ok pos$r"
    done
fi

if [ $NEG -eq 1 ]; then
echo "=== [2] 负控：把 shutdown 塞回信号处理器（还原缺陷 659）⇒ 主判据必须红"
snapshot
SHA0=$(sha256sum "$RT" | cut -c1-16)
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
i = s.index('static void px_sigstop_handler(int sig) {')
j = s.index('\n}', i)
body = s[i:j]
assert 'shutdown(fd, SHUT_RDWR)' not in body, 'NC 前置不成立：处理器里已有 shutdown'
new = body.replace('    g_px_stop = 1;',
                   '    g_px_stop = 1;\n    { int fd = (int)g_px_listen_fd; if (fd >= 0) shutdown(fd, SHUT_RDWR); }   // NC: 缺陷 659')
assert new != body, 'NC 打点未命中'
open(p, 'w', encoding='utf-8').write(s[:i] + new + s[j:])
PYEOF
if build_srv; then
    run_round nc
    chk "NC 判红（排队连接被 RST：bad>0）" "grep -qE 'bad=[1-9]' \"$W/sc.nc.log\""
else
    chk "NC 构建" "false"; tail -10 "$W/build.log" | sed 's/^/     /'
fi
restore_all
chk "NC 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
if build_srv; then
    run_round posr
    chk "NC 还原后复绿" "main_ok posr"
fi
fi

echo
echo "结果: $pass 通过 / $fail 失败 / 门坏=$bad_gate"
if [ $bad_gate -ne 0 ]; then echo "m376-GATE-BROKEN"; exit 2; fi
if [ $fail -eq 0 ]; then echo "m376-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
