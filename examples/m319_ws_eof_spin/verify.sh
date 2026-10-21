#!/bin/bash
# ============================================================
# m319 —— WS 接管读循环在 EOF 上自旋（缺陷 552）的常设门
#
# 来历：晨曦（wsa-chenxi）2026-10-22 报障 —— 广州 Ma 进程 12 个线程烧满 2 核 5h38m
#   （累计 CPU 9h01m / 墙钟 5h38m），8 条 CLOSE-WAIT，热栈 4/4 同型：
#     __pthread_mutex_unlock_usercnt / __pthread_mutex_lock
#       ← px_conn_read ← ws_read_frame ← px_ws_takeover_http_conn ← px_conn_worker
#   /proc/<tid>/syscall = running，5s 内 voluntary_ctxt_switches +0 ⇒ 纯用户态自旋。
#
# 根因（缺陷 552，两处叠加）：
#   ① px_conn_read 的失败出口（acquire 失败 / 已关闭）return -1 **不设 errno**
#      ⇒ 调用方读到的是上一次调用**残留**的 errno；
#   ② ws_read_exact 用 `errno == EINTR` 判「信号打断、应重试」并 `continue`
#      ⇒ 该线程被 GC 的 SIG_GC_STOP 打断过一次 recv 后，errno 长期是 EINTR
#      ⇒ 已关闭连接上的每一次读都 continue，且**没有任何轮次上限** ⇒ 死循环。
#   实测（本门负控）：单线程 **2.13 亿次**重试 / 5s **297% CPU**。
#
# 判据层次：
#   [1] 静态：四处失败出口显式设 errno（读/写 × 入口/body）· ws_read_exact 无 errno 判据、
#       无 continue 重试 · 注释点名缺陷 552
#   [2] 动态（正判据）：8 并发「中继协程 + ws_close + C 泵循环」× 3 轮，
#       压力档 PX_GC_STRESS=1 PX_GC_INLINE=1 ⇒ 每轮 utime 增量必须 < 阈值
#   [3] 负控 A（确定性灵敏度）：把失败分支改成无条件 continue（等价「errno 判据恒真」）
#       ⇒ 必须判红 —— 证明判据不是恒绿
#   [4] 负控 B（忠实撤回）：**同时**恢复层 1（四处 errno 设置）与层 2（errno == EINTR ⇒ continue）
#       ⇒ N 轮里**至少 1 轮**判红。⚠️ 忠实态的自旋依赖 errno 残留，是概率性的
#       （实测 3/4）；所以判据写成「至少 1 次」，**不会**因为「这次没抽中」把门判红
#       —— 这与 M274「会偶发判红的门等于没有门」同口径。
#       ⚠️ **必须撤两层**：只撤层 2 时自旋不复现（层 1 已把 errno 设成 ECONNRESET）。
#       这一点本身是收获 —— 两层是纵深防御、各自独立生效；但负控要测的是**缺陷本身**，
#       所以得回到修前的组合（M209「判据回放」同款纪律）。
#   [5] 负控 C（判据自伤）：阈值放大 1e6 倍 ⇒ 负控 A 的红必须消失
#   [6] 还原保真：三层跑完，runtime_ws.c 必须与门开头的快照**逐字节相同**
#   [7] 覆盖边界登记
#
# 用法：verify.sh [--neg-skip]     （CI 用 --neg-skip：负控要重建三次 runtime，约 60s）
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"

NEG_SKIP=0
while [ $# -gt 0 ]; do
    case "$1" in
        --neg-skip) NEG_SKIP=1; shift ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done

PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1 = $2"; else bad "$1 = $2（期望 $3）"; fi; }
ge()  { if [ "$2" -ge "$3" ]; then ok "$1 = $2（≥$3）"; else bad "$1 = $2（期望 ≥$3）"; fi; }

# 判据阈值（ticks / 4s，100 ticks = 1 秒 CPU）
#   实测：修复版 24~38（压力档自身的 GC 开销）· 缺陷态 249 / 792 / 397 · 确定性放大 1300+
#   取 120 ⇒ 修复侧有 3 倍余量、缺陷侧有 2 倍余量
TH="${M319_TH:-120}"

W="$(mktemp -d /tmp/m319gate.XXXXXX)"
SNAP="$W/runtime_ws.c.snap"
SRV_PID=""

cleanup() {
    [ -n "${SRV_PID:-}" ] && kill -9 "$SRV_PID" 2>/dev/null
    # 无论怎么退出，源码必须还原（M209/M285 纪律）
    if [ -f "$SNAP" ]; then
        python3 "$HERE/negctl.py" --restore "$SNAP" "$ROOT" >/dev/null 2>&1
    fi
    rm -rf "$W"
}
trap cleanup EXIT

echo '=== m319 · WS 接管读循环在 EOF 上自旋（缺陷 552）· 晨曦广州现场回归门 ==='
echo "  仓库: $ROOT   阈值: $TH ticks/4s"
python3 "$HERE/negctl.py" --snapshot "$SNAP" "$ROOT" >/dev/null

# ─────────────────────────────────────────────
echo '[1] 静态：errno 契约 + 无 errno 重试'
R="$ROOT/runtime/runtime.c"
WS="$ROOT/runtime/runtime_ws.c"

N1=$(awk '/^ssize_t px_conn_read\(PxConn\* c/,/^}/' "$R" | grep -c 'errno = ECONNRESET' || true)
chk 'A · px_conn_read 失败出口设 errno' "$N1" '1'
N2=$(awk '/^ssize_t px_conn_write\(PxConn\* c/,/^}/' "$R" | grep -c 'errno = EPIPE' || true)
chk 'A · px_conn_write 失败出口设 errno' "$N2" '1'
N3=$(awk '/^static ssize_t px_conn_read_body/,/^}/' "$R" | grep -c 'errno = ECONNRESET' || true)
ge 'A · px_conn_read_body 已关闭分支设 errno' "$N3" '1'
N4=$(awk '/^static ssize_t px_conn_write_body/,/^}/' "$R" | grep -c 'errno = EPIPE' || true)
ge 'A · px_conn_write_body 已关闭分支设 errno' "$N4" '1'

# B：ws_read_exact 里不得再有 errno 判据 / 重试（只看代码行，注释允许留历史）
CODE=$(awk '/^static int ws_read_exact/,/^}/' "$WS" | grep -vE '^[[:space:]]*//' || true)
B1=$(printf '%s\n' "$CODE" | grep -c 'errno' || true)
chk 'B · ws_read_exact 代码行里 errno 出现次数' "$B1" '0'
B2=$(printf '%s\n' "$CODE" | grep -c 'continue' || true)
chk 'B · ws_read_exact 代码行里 continue（重试）次数' "$B2" '0'
B3=$(printf '%s\n' "$CODE" | grep -c 'if (r < 0) return -1;' || true)
chk 'B · 失败分支直接返回' "$B3" '1'

# C：根因必须留痕（后人从门就能查到 552）
C1=$(grep -c 'M319（缺陷 552' "$WS" || true)
ge 'C · runtime_ws.c 里缺陷 552 的根因注释' "$C1" '1'
C2=$(grep -c '缺陷 552' "$R" || true)
ge 'C · runtime.c 里缺陷 552 的说明' "$C2" '1'

# ─────────────────────────────────────────────
# dyn_run <标签> ⇒ 打印 utime 增量（ticks/4s）；BUILD_FAIL / NO_SRV 时打印对应字串
CONC=8
PORT_BASE=$((22000 + ($$ % 600)))
dyn_run() {
    local tag="$1"
    local port=$((PORT_BASE++))
    ( cd "$W" && "$ROOT/tools/px" build srv.px > "$W/build_$tag.log" 2>&1 )
    if [ ! -x "$W/build/srv" ]; then echo "BUILD_FAIL"; return; fi
    env PX_GC_STRESS=1 PX_GC_INLINE=1 "$W/build/srv" "$port" > "$W/srv_$tag.log" 2>&1 &
    SRV_PID=$!
    local ready=0
    for _ in $(seq 1 60); do
        kill -0 "$SRV_PID" 2>/dev/null || break
        if python3 -c "import socket,sys; s=socket.socket(); s.settimeout(0.2); sys.exit(0 if s.connect_ex(('127.0.0.1',$port))==0 else 1)" 2>/dev/null; then
            ready=1; break
        fi
        sleep 0.1
    done
    if [ "$ready" != "1" ]; then kill -9 "$SRV_PID" 2>/dev/null; echo "NO_SRV"; return; fi
    local i
    for i in $(seq 1 $CONC); do
        python3 "$W/cli.py" "$port" 1.0 fin >/dev/null 2>&1 &
    done
    sleep 3.2
    local a b k
    for k in $(seq 1 20); do
        a=$(awk '{print $14+$15}' "/proc/$SRV_PID/stat" 2>/dev/null)
        sleep 1
        b=$(awk '{print $14+$15}' "/proc/$SRV_PID/stat" 2>/dev/null)
        [ -z "$a" ] && break
        [ $((b - a)) -lt 8 ] && break
    done
    a=$(awk '{print $14+$15}' "/proc/$SRV_PID/stat" 2>/dev/null)
    sleep 4
    b=$(awk '{print $14+$15}' "/proc/$SRV_PID/stat" 2>/dev/null)
    kill -9 "$SRV_PID" 2>/dev/null
    wait "$SRV_PID" 2>/dev/null
    SRV_PID=""
    if [ -z "$a" ] || [ -z "$b" ]; then echo "NO_STAT"; return; fi
    local dg
    dg=$(grep -o 'acquire-fail #[0-9]*' "$W/srv_$tag.log" 2>/dev/null | sed 's/.*#//' | sort -n | tail -1)
    echo "$((b - a)) ${dg:-0}"
}

# spin_of <"utime diag"> ⇒ 0=未自旋 · 1=自旋 · 2=基础设施异常
#   双判据：utime（进程级 CPU 增量）**或** 诊断重试计数（acquire-fail 的次数）。
#   为什么两个都要：errno 残留 EINTR 时，acquire 失败会被 ws_read_exact 无条件重试
#   ⇒ 计数直接反映「死循环转了多少圈」，比 utime 灵敏（实测 1100 万次重试只体现 85 ticks）；
#   而 utime 覆盖「没有诊断打印但真的在烧 CPU」的形态。任一命中即判自旋。
spin_of() {
    [ "${M319_SELFHARM:-0}" = "1" ] && { echo 0; return; }   # 负控 C：判据自伤
    local u="${1%% *}" d="${1##* }"
    case "$u" in ''|*[!0-9]*) echo 2; return ;; esac
    # 重试门槛取 1000：正常路径每条连接只 acquire-fail 一次（实测 1~8），
    # 上千次即说明进了重试循环（插桩实测最高 1100 万次）
    if [ "$u" -ge "$TH" ] || [ "${d:-0}" -ge 1000 ]; then echo 1; else echo 0; fi
}

cp "$HERE/srv.px" "$HERE/cli.py" "$W/"

echo '[2] 动态（正判据）：修复版 · 8 并发 · 压力档 × 3 轮'
SPIN_FIXED=0
for r in 1 2 3; do
    U=$(dyn_run "fixed$r")
    S=$(spin_of "$U")
    case "$S" in
        0) ok "第 $r 轮「$U」（utime ticks/4s + 重试次数）⇒ 无自旋" ;;
        1) bad "第 $r 轮「$U」⇒ 自旋"; SPIN_FIXED=$((SPIN_FIXED+1)) ;;
        *) bad "第 $r 轮基础设施异常（$U）" ;;
    esac
done

# ─────────────────────────────────────────────
echo '[3] 负控 A（确定性灵敏度）：失败分支改成无条件 continue ⇒ 必须判红'
if [ "$NEG_SKIP" = "1" ]; then
    echo '  ⏭ 已跳过（--neg-skip）'
else
    python3 "$HERE/negctl.py" --inject "$SNAP" "$ROOT" >/dev/null
    U=$(dyn_run "inject")
    case "$(spin_of "$U")" in
        1) ok "负控 A 复现自旋：「$U」" ;;
        0) bad "负控 A 未复现：「$U」（期望判自旋）⇒ 判据没有牙" ;;
        *) bad "负控 A 基础设施异常（$U）" ;;
    esac
    U_INJECT="$U"
    python3 "$HERE/negctl.py" --restore "$SNAP" "$ROOT" >/dev/null

    echo '[4] 负控 B（忠实撤回 + 还原现场 I/O）：4 轮至少 1 轮红'
    NEG_OUT=$(python3 "$HERE/negctl.py" --orig-instr "$SNAP" "$ROOT")
    echo "  · 打桩：$NEG_OUT"
    case "$NEG_OUT" in
        ORIG-INSTR*) : ;;
        *) bad "负控 B 打桩失败（$NEG_OUT）" ;;
    esac
    HITS=0; ERRINFRA=0
    for r in 1 2 3 4; do
        U=$(dyn_run "orig$r")
        S=$(spin_of "$U")
        case "$S" in
            1) HITS=$((HITS+1)); echo "  · 第 $r 轮「$U」⇒ 自旋" ;;
            0) echo "  · 第 $r 轮「$U」（本次未抽中 errno 残留 —— 记为观察，不判红）" ;;
            *) ERRINFRA=$((ERRINFRA+1)); echo "  · 第 $r 轮基础设施异常（$U）" ;;
        esac
    done
    if [ "$HITS" -ge 1 ]; then ok "负控 B 复现：4 轮里 $HITS 轮自旋"; else bad "负控 B 未复现：4 轮全绿（基础设施异常 $ERRINFRA 轮）⇒ 忠实撤回后缺陷不可见，判据失去意义"; fi
    python3 "$HERE/negctl.py" --restore "$SNAP" "$ROOT" >/dev/null

    echo '[5] 负控 C（判据自伤）：把 spin_of 改成恒绿 ⇒ 负控 A 的红必须消失'
    M319_SELFHARM=1
    python3 "$HERE/negctl.py" --inject "$SNAP" "$ROOT" >/dev/null
    U=$(dyn_run "selfharm")
    M319_SELFHARM=0
    case "$(spin_of "$U")" in
        0) ok "判据自伤后不再判红（实测「$U」）⇒ 负控 A 的红确实来自判据" ;;
        *) bad "负控 C：判据自伤后仍判红（$U）" ;;
    esac
    python3 "$HERE/negctl.py" --restore "$SNAP" "$ROOT" >/dev/null
fi

# ─────────────────────────────────────────────
echo '[6] 还原保真'
if cmp -s "$SNAP" "$WS"; then ok 'runtime_ws.c 与快照逐字节一致'; else bad 'runtime_ws.c 未还原'; fi
RC="$ROOT/runtime/runtime.c"
if cmp -s "$SNAP.c" "$RC"; then ok 'runtime.c 与快照逐字节一致'; else bad 'runtime.c 未还原（层 1 打桩残留）'; fi

echo '[7] 覆盖边界'
echo '  · 判据量的是「服务端进程 utime 增量」，不区分自旋发生在读还是写路径（写路径同族已静态覆盖）'
echo '  · 负控 B 是概率性复现（依赖该线程 errno 残留 EINTR）；确定性由负控 A 提供，两者各司其职'
echo '  · 只覆盖 ws_stream 接管的 px_serve 轨；ws_serve 轨（bi_ws_recv）走同一条 px_conn_read / ws_read_frame，'
echo '    修复同样生效，但本门未单独构造该轨的动态形态'
echo '  · TLS 连接的 mbedtls_ssl_read 路径未单独放大（WANT_READ 自旋是另一条同族路径，已登记为后续候选）'

echo
echo "汇总：$PASS 通过 / $FAIL 失败"
if [ "$FAIL" -eq 0 ]; then echo 'M319-VERIFY-OK'; exit 0; else echo 'M319-VERIFY-FAIL'; exit 1; fi
