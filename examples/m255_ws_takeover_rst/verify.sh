#!/bin/bash
# ============================================================
# m255 —— 晨曦报障「WS 统一入口 P0 崩溃」三缺陷的**修复有效性**常设门
#
# 背景：晨曦（wsa-chenxi）2026-10-02 报障两处（缺陷 A/B），同轮又递来一份证书
#   热加载 UAF 的补丁（缺陷 C）。三处**都已在 M240 / M245 修复**；但**此前没有任何门**
#   覆盖「WS 接管 + 客户端连上即撤（RST）」这个**晨曦的真实复现形态** ——
#   M235 门的形态二用的是温和的 ws_client（3 轮），不是高并发 RST。
#   ⇒ 本门把晨曦那份压测器（SO_LINGER=0 + 16 并发）固化下来。
#
# 判据层次：
#   [1] 静态：三处修复在位（A 线程安全解析 · B 释放前失效 WS 注册表 · C 证书注册与握手互斥）
#   [2] 动态（晨曦形态）：ws_stream 接管 + heartbeat + handler spawn 后返回
#       ⇒ 客户端「连上即撤（RST）」压测 · 会话数 ≥ 规模下限 ⇒ **崩溃 0 次**
#   [3] 会话形态自证 ⇒ 证明确实走的是 takeover 路径（101），不是 404/连接失败
#   [4] 负控 A（灵敏度·必崩侧）：必崩替身 ⇒ 检测**必须**报「已崩」
#   [5] 负控 B（灵敏度·必活侧）：必活替身 ⇒ 检测**必须**报「存活」
#       [4]+[5] 合起来 = 双向 ⇒ 同时排除「恒绿」「恒红」
#   [6] 覆盖边界登记
#
# 用法：verify.sh [--neg-skip] [--sessions N]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"

NEG_SKIP=0
SESS_TARGET="${M255_SESSIONS:-6000}"
while [ $# -gt 0 ]; do
    case "$1" in
        --neg-skip) NEG_SKIP=1; shift ;;
        --sessions) SESS_TARGET="$2"; shift 2 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done

PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk() { # chk <描述> <实测> <期望>
    if [ "$2" = "$3" ]; then ok "$1 = $2"; else bad "$1 = $2（期望 $3）"; fi
}

W="$(mktemp -d /tmp/m255gate.XXXXXX)"
cleanup() {
    [ -n "${SRV_PID:-}" ] && kill -9 "$SRV_PID" 2>/dev/null
    [ -n "${FAKE_PID:-}" ] && kill -9 "$FAKE_PID" 2>/dev/null
    rm -rf "$W"
}
trap cleanup EXIT

PORT=$((21000 + ($$ % 3000)))

echo "=== m255 · WS 接管 + 连上即撤（RST）· 晨曦形态回归门 ==="
echo "  仓库: $ROOT"

# ─────────────────────────────────────────────
echo "[1] 静态：三处修复在位"
# A：gethostbyname 不得出现在**代码行**（注释里留作历史说明是允许的）
GA=$(grep -vE '^[[:space:]]*//' "$ROOT/runtime/runtime_ws.c" | grep -c 'gethostbyname' || true)
chk "A · runtime_ws.c 代码行里的 gethostbyname 数" "$GA" "0"
# A 的正向面：应改用 getaddrinfo（或自有解析器）
GA2=$(grep -c 'px_ws_resolve_v4' "$ROOT/runtime/runtime_ws.c" || true)
if [ "$GA2" -ge 3 ]; then ok "A · px_ws_resolve_v4 调用点 = $GA2（≥3）"; else bad "A · px_ws_resolve_v4 调用点 = $GA2（期望 ≥3）"; fi

# B：释放前失效 WS 注册表
GB1=$(grep -c '^void px_ws_detach_conn' "$ROOT/runtime/runtime_ws.c" || true)
chk "B · px_ws_detach_conn 定义" "$GB1" "1"
GB2=$(grep -c '^void px_ws_detach_conn(PxConn\* c);' "$ROOT/runtime/runtime.c" || true)
chk "B · 声明可见" "$GB2" "1"
GB3=$(grep -c 'px_ws_detach_conn(c);' "$ROOT/runtime/runtime.c" || true)
if [ "$GB3" -ge 2 ]; then ok "B · 释放点调用 = $GB3（≥2，幂等多处）"; else bad "B · 释放点调用 = $GB3（期望 ≥2）"; fi

# C：证书热加载与并发握手互斥（注册段必须先取 hs_mu）
GC1=$(awk '/^static LXValue bi_tls_server/,/^}/' "$ROOT/runtime/runtime.c" | grep -c 'pthread_mutex_lock(&g_srv_hs_mu)' || true)
if [ "$GC1" -ge 1 ]; then ok "C · bi_tls_server 取 g_srv_hs_mu"; else bad "C · bi_tls_server 未取 g_srv_hs_mu"; fi
GC2=$(awk '/^static LXValue bi_tls_server/,/^}/' "$ROOT/runtime/runtime.c" | grep -c 'pthread_mutex_unlock(&g_srv_hs_mu)' || true)
if [ "$GC2" -ge "$GC1" ]; then ok "C · 解锁配对数 = $GC2（≥ 加锁 $GC1）"; else bad "C · 解锁 $GC2 < 加锁 $GC1（有出口漏解锁）"; fi

# ─────────────────────────────────────────────
echo "[2] 动态（晨曦形态 16 并发 · 连上即撤 RST）"
cp "$HERE/srv.px" "$HERE/rst_client.py" "$W/"
BUILD_OK=1
( cd "$W" && "$ROOT/tools/px" build srv.px > "$W/build.log" 2>&1 ) || BUILD_OK=0
if [ "$BUILD_OK" = "1" ] && [ -x "$W/build/srv" ]; then
    ok "服务端编译（ws_stream 接管 + heartbeat + handler spawn 后返回）"
else
    bad "服务端编译失败"; tail -5 "$W/build.log" 2>/dev/null | sed 's/^/      /'
fi

CONC=$((SESS_TARGET / 400))
[ "$CONC" -lt 4 ] && CONC=4
[ "$CONC" -gt 16 ] && CONC=16
ROUNDS=$((SESS_TARGET / CONC))

if [ "$BUILD_OK" = "1" ]; then
    ( exec "$W/build/srv" "$PORT" 5 > "$W/srv.log" 2>&1 ) &
    SRV_PID=$!
    # 就绪等待（最多 5s）：直接探测端口（px_serve 的启动日志字样不保证稳定）
    READY=0
    for _ in $(seq 1 50); do
        kill -0 "$SRV_PID" 2>/dev/null || break
        if python3 -c "import socket,sys; s=socket.socket(); s.settimeout(0.2); sys.exit(0 if s.connect_ex(('127.0.0.1',$PORT))==0 else 1)" 2>/dev/null; then
            READY=1; break
        fi
        sleep 0.1
    done
    [ "$READY" = "1" ] || sleep 1.0
    if kill -0 "$SRV_PID" 2>/dev/null; then
        ok "服务端起监听（pid=$SRV_PID port=$PORT）"
    else
        bad "服务端未能起监听"; tail -5 "$W/srv.log" 2>/dev/null | sed 's/^/      /'
    fi

    OUT="$(python3 "$W/rst_client.py" "127.0.0.1:$PORT" "$CONC" "$ROUNDS" 2>&1 | tail -1)"
    echo "      $OUT"
    CL_OK=$(echo "$OUT" | sed -n 's/.*ok=\([0-9]*\).*/\1/p')
    CL_UP=$(echo "$OUT" | sed -n 's/.*upgraded=\([0-9]*\).*/\1/p')
    CL_OK=${CL_OK:-0}; CL_UP=${CL_UP:-0}

    sleep 0.8
    if kill -0 "$SRV_PID" 2>/dev/null; then
        chk "2 · 压测后服务端崩溃次数" "0" "0"
        ACC=$(grep -c 'SRV accept' "$W/srv.log" || true)
        HR=$(grep -c 'SRV handler-return' "$W/srv.log" || true)
        echo "      会话=$CL_OK 升级101=$CL_UP 服务端accept=$ACC handler-return=$HR"
    else
        wait "$SRV_PID" 2>/dev/null
        bad "2 · 压测后服务端崩溃次数 = 1（期望 0）"
        tail -8 "$W/srv.log" 2>/dev/null | sed 's/^/      /'
    fi
    kill -9 "$SRV_PID" 2>/dev/null; wait "$SRV_PID" 2>/dev/null; SRV_PID=""

    # ─────────────────────────────────────────
    echo "[3] 会话形态自证（必须真走 takeover，不能是 404/连接失败）"
    if [ "$CL_OK" -ge "$SESS_TARGET" ]; then
        ok "会话数 $CL_OK ≥ 规模下限 $SESS_TARGET"
    else
        bad "会话数 $CL_OK < 规模下限 $SESS_TARGET（压测没跑满 ⇒ 判据无意义）"
    fi
    if [ "$CL_UP" -ge "$CL_OK" ] && [ "$CL_OK" -gt 0 ]; then
        ok "全部会话均返回 HTTP 101（upgraded=$CL_UP / ok=$CL_OK）"
    else
        bad "upgraded=$CL_UP 少于 ok=$CL_OK ⇒ 未走 WS 接管路径"
    fi
fi

# ─────────────────────────────────────────────
if [ "$NEG_SKIP" = "0" ]; then
    echo "[4][5] 负控（检测器灵敏度 · 双向）"
    # [4] 必崩侧：替身进程自杀（SIGSEGV）⇒ 检测必须报「已崩」
    ( exec sh -c 'kill -SEGV $$' > "$W/crash.log" 2>&1 ) &
    FAKE_PID=$!
    sleep 0.5
    if kill -0 "$FAKE_PID" 2>/dev/null; then
        bad "4 · 必崩替身仍存活 ⇒ 检测器或替身有问题"
        kill -9 "$FAKE_PID" 2>/dev/null
    else
        wait "$FAKE_PID" 2>/dev/null; rc=$?
        if [ "$rc" -ge 128 ]; then
            ok "4 · 必崩替身被检出（rc=$rc ≥128 ⇒ 信号致死）"
        else
            bad "4 · 必崩替身 rc=$rc（期望 ≥128）"
        fi
    fi
    FAKE_PID=""
    # [5] 必活侧：替身存活 ⇒ 检测必须报「存活」
    ( exec sleep 30 ) &
    FAKE_PID=$!
    sleep 0.5
    if kill -0 "$FAKE_PID" 2>/dev/null; then
        ok "5 · 必活替身被检出为存活"
        kill -9 "$FAKE_PID" 2>/dev/null; wait "$FAKE_PID" 2>/dev/null
    else
        bad "5 · 必活替身被判为已死 ⇒ 检测器恒红"
    fi
    FAKE_PID=""
else
    echo "[4][5] 负控已跳过（--neg-skip）"
fi

# ─────────────────────────────────────────────
echo "[6] 覆盖边界（如实登记）"
cat <<'EOF'
      · 本门跑 **明文 TCP**；晨曦生产是 **TLS(8443)**。缺陷 B（takeover 所有权）与传输层
        无关，故明文可覆盖；但缺陷 399（px_pxpend 跨 xrealloc 野指针）的窗口由**慢握手**放大
        ⇒ **明文档对该条覆盖较弱**，需 TLS 档另测（见 CHANGELOG「未做」）。
      · 压测器**不带 CPU 竞争**（本机/CI 负载低）⇒ M240 记录的「带 CPU 竞争」档更易复现，
        本门是**回归**门（证不退化），不是**复现**门（证必崩）。
      · 撤回单层修复**不能**让本门变红（M255 实测：撤 detach / 撤 399 各 32000 会话均存活）
        ⇒ 本门的「修复必要性」由 [1] 静态层 + M240 的修前基线（58 会话即崩 + 在册 core）承担。
      · 会话数上限受 fd/backlog 限制（实测 64000 会话时 fail=32，属连接被拒，非崩溃）。
EOF

echo "────────────────────────────────"
echo "m255: 通过 $PASS · 失败 $FAIL"
if [ "$FAIL" -eq 0 ]; then
    echo "M255-VERIFY-OK"
    exit 0
else
    echo "M255-VERIFY-FAIL"
    exit 1
fi
