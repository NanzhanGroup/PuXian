#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M149 验证：TCP「带超时 + 可辨别失败」族（tcp_connect_ex / tcp_opt /
#            tcp_recv_ex / tcp_send_ex）
# ---------------------------------------------------------------
# 门做五件事：
#   ① VM 轨：受控服务端（独立进程）+ 客户端断言集 → M149_ASSERT: nP/0F
#   ② C 轨：同上（服务端也用 C 轨编译）⇒ 与 VM 轨输出**逐字节一致**
#      ⚠️ M237s1（缺陷 372）：比对前先归一**已登记**的环境/走钟字段（走钟 dt、INFO F/G/H
#         轮数、A 段黑洞探测分支及其派生的 SKIP 行与 M149_ASSERT 的 S 计数）——
#         本机到 TEST-NET-1 的路由状态在**两次运行之间**就可能变（110 超时 ⇄ 101/113
#         不可达），修前会把「两轨各自 53P/0F」误判成不一致（2026-10-01 全量门假红）。
#         归一清单之外**任何**差异仍逐字节判红；另有硬判据守「分支必须是登记的那两种」
#         与三条判据自证（见 §判据自证）。
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
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")"
PX=../../tools/px
RT=../../runtime/runtime.c

# ── 缺陷 139（第 33 轮）：负控会改写 runtime/*.c；门**被打断**时篡改态会静默留在工作区 ──
#   （语法合法、语义反向 ⇒ 编译器不报错；而本轮 runtime.c 本来就带未提交改动 ⇒ `git diff` 判不出来。）
#   两道防线：① 开门先查「负控残留标记 NEGCTL」；② 信号兜底还原快照。
for _f in ../../runtime/runtime.c ../../runtime/vm.c; do
    if [ -f "$_f" ] && grep -q 'NEGCTL' "$_f" 2>/dev/null; then
        echo "FAIL 负控残留：$_f 仍含 NEGCTL 标记（上一轮门被中断？先还原再跑）"
        exit 1
    fi
done

cp "$RT" /tmp/m149_rt_snapshot.c
restore_rt() { [ -f /tmp/m149_rt_snapshot.c ] && cp /tmp/m149_rt_snapshot.c "$RT" 2>/dev/null; }
trap restore_rt INT TERM HUP
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

echo "--- VM / C 两轨输出逐字节一致（归一字段：走钟 dt · INFO F/G/H 轮数 · A 段黑洞分支）---"
# ── 归一化：只折叠**已登记**的环境/走钟字段；其余必须逐字节一致 ──
#   M237s1（缺陷 372）登记表（新增字段必须同时加进这里与 abranch 的硬判据，并补自证）：
#     · INFO *.dt 的毫秒          —— 走钟（既有）
#     · INFO F/G/H.rounds 的轮数  —— socket 缓冲/调度决定；本门对它们只断言**下限**（H5: ≥6 等）
#     · A 段黑洞探测的**分支**     —— 110 超时 ⇄ 101/113 不可达，取决于本机到 TEST-NET-1 的路由
#       派生物：SKIP 行（整行删除，它是分支的产物，分支合法性由 abranch 单独守）
#               M149_ASSERT 的 S 计数（随分支走）
NORM_EXPR=(
  -e 's/^(INFO [A-Z]\.)dt dt=[0-9]+ms/\1dt dt=<ms>/'
  -e 's/^(INFO [FGH]\.rounds recv 轮数=)[0-9]+/\1<n>/'
  -e '/^SKIP A4-A6 超时语义 /d'
  -e 's/^(PASS A4 )(超时被遵守|不可达即失败).*/\1<A4>/'
  -e 's/^(PASS A5 )(未远超超时|不是 refused).*/\1<A5>/'
  -e 's/^(PASS A6 )(errno=ETIMEDOUT\(110\)|err 文案非空).*/\1<A6>/'
  -e 's/^(INFO A\.dt ).*/\1<A 分支详情>/'
  -e 's/^(M149_ASSERT: [0-9]+P\/0F)\/[0-9]+S$/\1\/<S>/'
)
norm() { sed -E "${NORM_EXPR[@]}" "$1"; }

# A 段分支分类 —— 硬判据：归一化只认登记的那两种分支，第三种行为必须判红
abranch() {
    if grep -q '^PASS A6 errno=ETIMEDOUT(110)$' "$1"; then echo timeout
    elif grep -q '^PASS A5 不是 refused（101/113，非 111）$' "$1"; then echo unreachable
    else echo BAD; fi
}

# 返回 0=一致；1=不一致（并打印原因）
cmp_tracks() {
    local v="$1" c="$2" bv bc ok=0
    bv=$(abranch "$v"); bc=$(abranch "$c")
    if [ "$bv" = "BAD" ] || [ "$bc" = "BAD" ]; then
        echo "FAIL A 段分支不在登记集（vm=$bv · c=$bc）—— 既非 110 超时、亦非 101/113 不可达"
        ok=1
    fi
    if ! diff <(norm "$v") <(norm "$c") > /tmp/m149_twdiff.txt 2>&1; then
        echo "FAIL 两轨不一致（归一后仍不同）："
        head -10 /tmp/m149_twdiff.txt | sed 's/^/   /'
        ok=1
    fi
    if [ "$ok" = "0" ]; then
        echo "PASS 两轨一致（归一后；A 段分支 vm=$bv · c=$bc）"
        if [ "$bv" != "$bc" ]; then
            echo "   ℹ️ A 段分支两轨不同 —— 环境相关（本机到 TEST-NET-1 的路由状态），已归一；两轨各自断言均 0F"
        fi
    fi
    return $ok
}

if [ -f /tmp/m149_vm.out ] && [ -f /tmp/m149_c.out ]; then
    cmp_tracks /tmp/m149_vm.out /tmp/m149_c.out || FAIL=$((FAIL+1))
else
    echo "FAIL 缺轨产物（/tmp/m149_vm.out 或 /tmp/m149_c.out 未生成）"; FAIL=$((FAIL+1))
fi

echo "== [判据自证] 两轨对拍 —— 归一化不得掩盖真差异，也不得吞掉第三种行为 =="
ST=/tmp/m149_selftest; rm -rf "$ST"; mkdir -p "$ST"
cat > "$ST/v1" <<'SEOFT1'
PASS A4 超时被遵守（≥250ms）
PASS A5 未远超超时（≤2500ms）
PASS A6 errno=ETIMEDOUT(110)
INFO A.dt dt=300ms errno=110 err=x
INFO F.rounds recv 轮数=1
INFO I.reply reply=got 100000
M149_ASSERT: 53P/0F/0S
SEOFT1
cat > "$ST/c1" <<'SEOFT2'
SKIP A4-A6 超时语义 —— 本机对该网段直达不可达（errno=101，非丢包）⇒ 试不到 ETIMEDOUT
PASS A4 不可达即失败（≤2500ms）
PASS A5 不是 refused（101/113，非 111）
PASS A6 err 文案非空
INFO A.dt dt=161ms errno=101 err=y
INFO F.rounds recv 轮数=2
INFO I.reply reply=got 100000
M149_ASSERT: 53P/0F/1S
SEOFT2
st() {
    local nm="$1" want="$2" v="$3" c="$4" got=0
    cmp_tracks "$v" "$c" >/dev/null 2>&1 || got=1
    if [ "$got" = "$want" ]; then echo "   PASS 自证 $nm（rc=$got）"
    else echo "FAIL 自证 $nm（rc=$got，期望 $want）"; FAIL=$((FAIL+1)); fi
}
# T1：A 段分支不同 + 轮数不同 + S 计数不同（全属登记字段）⇒ 必须判「一致」
st "T1 仅登记字段差异（A 分支/轮数/S 计数）⇒ 判一致" 0 "$ST/v1" "$ST/c1"
# T2：真实差异（归一清单之外的一行变了）⇒ 必须判「不一致」
sed 's/^INFO I.reply reply=got 100000$/INFO I.reply reply=got 99999/' "$ST/c1" > "$ST/c2"
st "T2 真实差异（I.reply 值变）⇒ 判不一致" 1 "$ST/v1" "$ST/c2"
# T3：A 段出现「第三种行为」（归一化会折叠成同形）⇒ 硬判据必须判「不一致」
sed -e 's/^PASS A4 .*/PASS A4 第三种行为/' \
    -e 's/^PASS A5 .*/PASS A5 第三种行为/' \
    -e 's/^PASS A6 .*/PASS A6 第三种行为/' "$ST/c1" > "$ST/c3"
st "T3 A 段第三种行为（归一化会被折叠）⇒ 判不一致" 1 "$ST/v1" "$ST/c3"
rm -rf "$ST"

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
    # ── M245 补（缺陷 415）：A 段有**两条已登记分支**（110 超时 ⇄ 101/113 不可达，
    #   取决于本机到 TEST-NET-1 的**瞬时**路由；M237s1 已为跨轨对拍登记过，但负控没跟上）。
    #   实测（2026-10-03 全量门）：主判据两次都是 errno=110/0S，而**同一轮**负控复跑就是
    #   101/1S —— 中间只隔几分钟。而「忽略 timeout_ms」这条篡改**只在 110 分支可观测**
    #   ⇒ 拿 101 分支判「仍绿」= **假红**（判据比被测面更依赖环境）。
    #   ⇒ 判据改为**分支感知**：篡改后没走到 110 ⇒ 复跑一次；仍不是 ⇒ **SKIP（带原因）**，
    #     与主判据自身的 `SKIP A4-A6 …` **同口径**（如实跳过，不假装红、也不假装绿）。
    #   ⚠️ 只对 tag=A 加此守卫：B（NODELAY）/C（EOF）与网络分支无关，必须照旧硬判红。
    local verdict="green" aerr=""
    neg_run_once() {
        verdict="green"; aerr=""
        stop_server; rm -f /tmp/m149_srv.out
        build_one m149_server.px "" > /dev/null 2>&1 || true
        if build_one tcp_deadline.px "" > "$LOG" 2>&1; then
            start_server || true
            timeout 90 env M149_PORT="$PORT" ./build/tcp_deadline > "/tmp/m149_neg_$tag.out" 2>&1; local rc=$?
            stop_server
            aerr=$(grep -m1 'INFO A.dt' "/tmp/m149_neg_$tag.out" 2>/dev/null | grep -oE 'errno=[0-9]+' | head -1 | cut -d= -f2)
            if [ "$rc" = "0" ] && grep -qE '^M149_ASSERT: [0-9]+P/0F(/[0-9]+S)?$' "/tmp/m149_neg_$tag.out"; then
                verdict="green"
            else
                verdict="red"
            fi
        else
            verdict="red（编译失败）"
        fi
    }
    neg_run_once
    if [ "$tag" = "A" ] && [ "$verdict" = "green" ] && [ "$aerr" != "110" ]; then
        echo "   ℹ️ 复跑：本次 A 段 errno=${aerr:-?}（非 110 ⇒ 时间面试不到）⇒ 重试一次"
        neg_run_once
    fi
    cp /tmp/m149_runtime_keep.c "$RT"
    if [ "$verdict" = "green" ]; then
        if [ "$tag" = "A" ] && [ "$aerr" != "110" ]; then
            echo "   SKIP 负控 A：本环境 A 段走 errno=${aerr:-?}（不可达分支）⇒ 篡改「忽略 timeout_ms」不可观测"
            echo "        （与主判据的 SKIP A4-A6 同口径：**如实跳过，不算通过**；该面由 110 分支覆盖）"
        else
            echo "FAIL 负控 $tag：篡改后门仍绿（该面没被覆盖）"
            grep -E '^(FAIL |M149_ASSERT)' "/tmp/m149_neg_$tag.out" 2>/dev/null | head -5 | sed 's/^/   /'
            FAIL=$((FAIL+1))
        fi
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
