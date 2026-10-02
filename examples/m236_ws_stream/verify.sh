#!/usr/bin/env bash
# ============================================================
# examples/m236_ws_stream/verify.sh
# ------------------------------------------------------------
# M236（晨曦 ws-edge 特性请求）：px_serve / http_serve 的 **WebSocket 升级接管**
#
# 需求（晨曦 · 2026-09-30）：Ma（.px 服务）独占 443 且在同进程按 SNI/Host 分发多域名；
#   要做 `wss://<节点>/agent/<节点名>` 的多节点路由，就必须能在**同一监听、同一进程**
#   上完成 WS 升级 —— 另起进程占 443 或让 Ma 让出 443 都不理想。
#   现状缺口：px_serve 的请求路径对 `Upgrade` 只处理 h2c（忽略），`Upgrade: websocket`
#   落普通 dispatch ⇒ handler 只能回普通响应；而 ws_serve 是独立 bind，无法共存。
#
# 交付：`ws_stream(path, fn[, opts])` —— 与 `http_stream`（SSE 同端口接管）**同构**；
#   外加 `ws_reply_101(conn)`（`opts.manual: true` 时由语言层在准入决策后完成握手）。
#   连接元信息复用 M235 的 ws_conn_path / ws_conn_header / ws_conn_peer（零改动）。
#
# 判据层次：
#   [1] 静态：实现点在位 + 派生索引同步
#   [2] C 轨端到端（服务端与客户端**两个进程**：顶层无 spawn ⇒ 解释轨亦可跑）
#   [3] 三轨一致（C / VM / 解释，输出逐字节相同）
#   [4] 连接元信息三 API（ws_conn_path / ws_conn_header / ws_conn_peer）
#   [5] 未注册路径（即使带 Upgrade）⇒ **落回普通 HTTP**（不升级、不被吞）
#   [6] 普通 HTTP 请求（无 Upgrade）⇒ 行为不变（404）
#   [7] manual 模式：接受（ws_reply_101 → 101）与拒绝（不写 101）两条路
#   [8] 拒绝侧 4 例：path 非法 / opts.headers / 非 GET methods / 同路径 SSE⇄WS 冲突
#   [9] 负控 3 道（各自的锚点 ⇒ 各自独立判红；源逐字节还原）
#   [10] 覆盖边界登记
#
# 用法：verify.sh [--neg-skip]
# ============================================================
set -uo pipefail

ROOT="${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
HERE="$ROOT/examples/m236_ws_stream"
W="${M236_GATE_W:-/tmp/m236_gate}"
NEG_SKIP=0
while [ $# -gt 0 ]; do
    case "$1" in
        --neg-skip) NEG_SKIP=1 ;;
        *) echo "未知参数: $1"; exit 2 ;;
    esac
    shift
done

PASS=0; FAIL=0
ok()   { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad()  { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

# 收尾清理：本门会跑很多「常驻服务端」，任何提前退出都必须按 PID 收干净
#   （⚠ 绝不用 pkill -f —— 命令串里含模式时会把工具调用自己打掉，本仓记过多次）
cleanup_all() {
    local f p
    for f in "$W"/*/srv.pid; do
        [ -f "$f" ] || continue
        p="$(cat "$f" 2>/dev/null)"
        if [ -n "$p" ]; then kill "$p" 2>/dev/null; kill -9 "$p" 2>/dev/null; fi
    done
}
trap 'cleanup_all' EXIT

# 端口是否空闲（bash /dev/tcp；连得上 = 被占）
port_free() { ! (exec 3<>/dev/tcp/127.0.0.1/"$1") 2>/dev/null; }
chk()  { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }

echo "=== M236 门 · ws_stream（HTTP → WebSocket 升级接管）==="
echo "ROOT=$ROOT  W=$W"
rm -rf "$W"; mkdir -p "$W"

RH="$ROOT/runtime/runtime_ws.c"
RC="$ROOT/runtime/runtime.c"
RM="$ROOT/runtime/native_mod_map.txt"

# ------------------------------------------------------------
echo "[1] 静态：实现点在位 + 派生索引同步"
# ------------------------------------------------------------
S=0
grep -q 'int px_ws_takeover_http_conn(PxConn\* c, LXValue req, const char\* path, int route_idx, int manual)' "$RH" && S=$((S+1))
grep -q 'int px_ws_can_takeover(LXValue req)' "$RH" && S=$((S+1))
grep -q 'LXValue bi_ws_reply_101(LXValue\* args, int nargs, void\* ctx)' "$RH" && S=$((S+1))
grep -q 'static int ws_write_101(PxConn\* c, const char\* hdrs)' "$RH" && S=$((S+1))
grep -q 'int is_ws;' "$RC" && S=$((S+1))
grep -q 'static int stream_match_ws(const char\* path)' "$RC" && S=$((S+1))
grep -q 'static LXValue bi_ws_stream(LXValue\* args, int nargs, void\* ctx)' "$RC" && S=$((S+1))
grep -q 'static char\* ws_rebuild_head_from_req(LXValue req)' "$RH" && S=$((S+1))
chk "静态 · runtime_ws.c/runtime.c 实现点（8 处）" "$S" "8"

# stream_match 必须排除 WS 路由（否则 SSE 接管会串到 WS 路由上）
grep -q 'if (g_stream_routes\[i\].active && !g_stream_routes\[i\].is_ws' "$RC"
chk "静态 · stream_match 排除 is_ws（两类接管不串扰）" "$?" "0"
# bi_http_stream 复用槽位时须复位 is_ws（否则 WS 标记残留）
grep -q 'g_stream_routes\[idx\].is_ws = 0;' "$RC"
chk "静态 · bi_http_stream 复用槽位复位 is_ws" "$?" "0"
# 两处接管点
# M242（缺陷 407）：判据**不得绑定变量名**（那是实现细节）。
#   原锚点写死 `conn` / `wc` 两个变量名；M240 把 px_serve 轨的接管点改成
#   `PxConn* ws_c = px_pxpend_detach_conn(fd); … (void)px_ws_takeover_http_conn(ws_c, …)`
#   ⇒ 写死 `conn` 的那半从此匹配不到 ⇒ 报「实际 1」= **假红**。
#   改为按**调用点形状**（`(void)` 前缀）计数 —— 与变量名解耦。
N_HOOK=$(grep -c '(void)px_ws_takeover_http_conn(' "$RC")
chk "静态 · 两处接管点（px_serve 轨 + http_serve 轨）" "$N_HOOK" "2"
# 派生索引三处
grep -q '^ws_stream=ws$' "$RM" && grep -q '^ws_reply_101=ws$' "$RM"
chk "静态 · native_mod_map.txt 已同步（自动裁剪依赖）" "$?" "0"
grep -q 'ws_stream' "$ROOT/tools/lint_core.px" && grep -q 'ws_reply_101' "$ROOT/tools/lint_core.px"
chk "静态 · lint_core.px 内置名册已同步" "$?" "0"
grep -q '"ws_stream"' "$ROOT/docs/native_index.json" && grep -q '"ws_reply_101"' "$ROOT/docs/native_index.json"
chk "静态 · docs/native_index.json 已同步" "$?" "0"

# ------------------------------------------------------------
# 工具函数
# ------------------------------------------------------------
# 负控必须用「打过补丁后重建」的编译器（PXC_BIN_OVERRIDE），否则测的还是入库件 = 假绿
PXC_BIN_OVERRIDE=""
build_one() {   # $1=engine(c|vm)  $2=dir  $3=file
    local eng="$1" d="$2" f="$3"
    if [ "$eng" = "c" ]; then
        PX_PXC_BIN="${PXC_BIN_OVERRIDE:-$ROOT/bootstrap/pxc}" PX_BUILD_ENGINE=c \
            "$ROOT/tools/px" build "$d/$f" >"$d/build_$f.log" 2>&1
    else
        PXC_VM_BIN="$ROOT/bootstrap/pxc_vm" PX_BUILD_ENGINE=vm \
            "$ROOT/tools/px" build "$d/$f" >"$d/build_$f.log" 2>&1
    fi
    return $?
}

# 目录名与「传给 cli 的 mode」分开：负控必须用独立目录，否则 rm -rf 会吃掉前面层的产物
pair() {   # $1=engine  $2=srvfile  $3=port  $4=mode  [$5=dirtag]
    local eng="$1" sfile="$2" port="$3" mode="$4" dirtag="${5:-$4}"
    local d="$W/$eng-$dirtag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$HERE/$sfile" "$d/srv.px"; cp "$HERE/cli.px" "$d/cli.px"
    local SB CB
    case "$eng" in
        c|vm)
            build_one "$eng" "$d" srv.px || { echo "BUILDFAIL srv($eng) ->"; tail -5 "$d/build_srv.px.log"; return 9; }
            build_one "$eng" "$d" cli.px || { echo "BUILDFAIL cli($eng) ->"; tail -5 "$d/build_cli.px.log"; return 9; }
            SB="$d/build/srv $port"; CB="$d/build/cli $port $mode" ;;
        i)
            # ⚠ 解释器 CLI 约定：**脚本路径必须在参数末尾**（`px run s.px xx` 驱动的形状）
            #   ⇒ 其余参数一律排在脚本之前
            SB="$ROOT/bootstrap/pxi $port $d/srv.px"
            CB="$ROOT/bootstrap/pxi $port $mode $d/cli.px" ;;
        *) echo "unknown engine $eng"; return 2 ;;
    esac
    if ! port_free "$port"; then
        echo "PORT-BUSY $port（上一次跑的常驻服务端未收干净）" > "$d/cli.out"
        echo "9" > "$d/cli.rc"
        return 9
    fi
    # ⚠ 必须 `exec`：否则 `( … ) &` 的 $! 是**子 shell** 的 pid，kill 打不到真正的服务端
    #   ⇒ 服务端成为孤儿并一直占着端口（M201/M235 记过的坑；本门首版实测又踩）
    ( cd "$d" && exec $SB ) > "$d/srv.out" 2>&1 &
    local spid=$!
    echo "$spid" > "$d/srv.pid"
    sleep 2
    ( cd "$d" && timeout 25 $CB > cli.out 2>&1 ); local rc=$?
    sleep 0.5
    kill "$spid" 2>/dev/null
    local i
    for i in 1 2 3 4 5 6 7 8 9 10; do
        kill -0 "$spid" 2>/dev/null || break
        sleep 0.2
    done
    kill -9 "$spid" 2>/dev/null
    echo "$rc" > "$d/cli.rc"
    return 0
}

# 业务行归一（去掉 [px-serve] 启动/诊断行 + 行尾空白）
norm() { grep -v '^\[px-serve\]' "$1" | sed 's/[[:space:]]*$//'; }

# ------------------------------------------------------------
echo "[2] 动态 · C 轨端到端（两进程）"
# ------------------------------------------------------------
pair c srv.px 18821 basic
chk "C 轨 · 服务端未报绑定/启动错" "$(grep -c 'Address already in use\|绑定端口' "$W/c-basic/srv.out")" "0"
chk "C 轨 · cli rc" "$(cat "$W/c-basic/cli.rc")" "0"
chk "C 轨 · 握手成功" "$(grep -c 'CLI connected = true' "$W/c-basic/cli.out")" "1"
chk "C 轨 · 首帧" "$(grep -c 'CLI r1 = hello:/agent/node1' "$W/c-basic/cli.out")" "1"
chk "C 轨 · 回显 1" "$(grep -c 'CLI r2 = echo:m1' "$W/c-basic/cli.out")" "1"
chk "C 轨 · 收尾" "$(grep -c 'CLI done' "$W/c-basic/cli.out")" "1"

echo "[3] 动态 · 三轨一致（C / VM / 解释）"
pair vm srv.px 18822 basic
pair i  srv.px 18823 basic
chk "VM 轨 · 握手成功" "$(grep -c 'CLI connected = true' "$W/vm-basic/cli.out")" "1"
chk "解释轨 · 握手成功" "$(grep -c 'CLI connected = true' "$W/i-basic/cli.out")" "1"
tri_ok() { diff <(norm "$1") <(norm "$2") >/dev/null 2>&1; }
for pair_name in "c vm" "c i"; do
    engA="${pair_name% *}"; engB="${pair_name#* }"
    A="$W/$engA-basic"; B="$W/$engB-basic"
    if tri_ok "$A/cli.out" "$B/cli.out"; then
        ok "三轨一致 · cli（$engA ⇄ $engB 逐字节相同）"
    else
        bad "三轨一致 · cli（$engA ⇄ $engB 有差异）"; diff <(norm "$A/cli.out") <(norm "$B/cli.out") | head -6
    fi
    if tri_ok "$A/srv.out" "$B/srv.out"; then
        ok "三轨一致 · srv（$engA ⇄ $engB 逐字节相同）"
    else
        bad "三轨一致 · srv（$engA ⇄ $engB 有差异）"; diff <(norm "$A/srv.out") <(norm "$B/srv.out") | head -6
    fi
done

echo "[4] 连接元信息三 API"
chk "元信息 · path" "$(grep -c 'SRV path=/agent/node1' "$W/c-basic/srv.out")" "1"
chk "元信息 · Upgrade 头可见" "$(grep -c 'upg=websocket' "$W/c-basic/srv.out")" "1"
chk "元信息 · 对端非空（ws_conn_peer）" "$(grep -c 'peer_ok=true' "$W/c-basic/srv.out")" "1"
chk "元信息 · query 可见（多节点路由/鉴权的依据）" "$(grep -c 'query=token=abc' "$W/c-basic/srv.out")" "1"

echo "[5] 未注册路径（带 Upgrade）⇒ 落回普通 HTTP"
pair c srv.px 18824 miss
chk "miss · 未升级（ws_connect 失败）" "$(grep -c 'MISS upgraded = false' "$W/c-miss/cli.out")" "1"
chk "miss · 未产生 WS 连接（srv 无业务行）" "$(grep -c '^SRV' "$W/c-miss/srv.out")" "0"

echo "[6] 普通 HTTP 请求（无 Upgrade）⇒ 行为不变"
pair c srv.px 18825 plain
chk "plain · 404（docroot 无该文件）" "$(grep -c 'PLAIN status = 404' "$W/c-plain/cli.out")" "1"

echo "[7] manual 模式（101 由语言层调 ws_reply_101）"
pair c srv_manual.px 18826 manual
chk "manual · 接受路径握手成功" "$(grep -c 'MANUAL connected = true' "$W/c-manual/cli.out")" "1"
chk "manual · ws_reply_101 返回 true" "$(grep -c 'SEC 101=true' "$W/c-manual/srv.out")" "1"
chk "manual · 升级后双向通" "$(grep -c 'MANUAL r1 = auth-ok' "$W/c-manual/cli.out")" "1"
pair c srv_manual.px 18827 manual_reject
chk "manual · 拒绝路径不升级" "$(grep -c 'REJECT upgraded = false' "$W/c-manual_reject/cli.out")" "1"
chk "manual · 拒绝路径语言层可达" "$(grep -c 'SEC reject q=' "$W/c-manual_reject/srv.out")" "1"

echo "[8] 拒绝侧（必须响亮，不静默忽略）"
rej() {   # $1=文件  $2=关键字
    local d="$W/rej-$1"
    mkdir -p "$d"; cp "$HERE/$1.px" "$d/"
    ( cd "$d" && PX_PXC_BIN="$ROOT/bootstrap/pxc" PX_BUILD_ENGINE=c \
        "$ROOT/tools/px" build "$d/$1.px" >build.log 2>&1 )
    ( cd "$d" && timeout 15 ./build/"$1" >out.txt 2>&1 ); local rc=$?
    local hit; hit=$(grep -c "$2" "$d/out.txt")
    local reach; reach=$(grep -c 'SHOULD-NOT-REACH' "$d/out.txt")
    if [ "$rc" -ne 0 ] && [ "$hit" -ge 1 ] && [ "$reach" = "0" ]; then
        ok "拒绝 · $1（rc=$rc 且报出「$2」）"
    else
        bad "拒绝 · $1（rc=$rc hit=$hit reach=$reach）"
    fi
}
rej rej_path 'path 必须以 / 开头'
rej rej_headers '暂不支持 opts.headers'
rej rej_method '只支持 GET'
rej rej_conflict '已被 http_stream（SSE）注册'

echo "[9] 负控（各自独立判红；源逐字节还原）"
if [ "$NEG_SKIP" = "1" ]; then
    echo "  ⏭ 负控跳过（--neg-skip，CI 用）"
else
    SNAP="$W/neg_snap"; mkdir -p "$SNAP"
    cp "$RH" "$SNAP/runtime_ws.c"; cp "$RC" "$SNAP/runtime.c"
    restore_neg() { cp "$SNAP/runtime_ws.c" "$RH"; cp "$SNAP/runtime.c" "$RC"; }
    # 保留 $W 供诊断（本仓纪律：门红了要能读到原因）；收尾必须同时还原源码 + 收干净服务端
    trap 'restore_neg 2>/dev/null; cleanup_all' EXIT

    # ── 负控 A：拿走「升级接管」整段（px_conn_worker 的 WS 块删掉）⇒ [2] 必须红
    python3 - "$RC" <<'PYEOF'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
anchor = '        // ═══ M236：WebSocket 升级接管（晨曦特性请求 · 见 docs/WS_UPGRADE.md）═══'
i = s.find(anchor)
if i < 0:
    print('NEG-A anchor miss'); sys.exit(1)
# ⚠ 必须从配对的 `#ifndef PX_NO_WS` 开始删：只删块体而留下 `#ifndef` 会让源码
#   `unterminated #ifndef` ⇒ 不可编译 ⇒ 负控变成"构建失败"而非"行为差异"（假红）
k = s.rfind('#ifndef PX_NO_WS', 0, i)
if k < 0:
    print('NEG-A ifndef miss'); sys.exit(1)
j = s.find('#endif', i)
if j < 0:
    print('NEG-A endif miss'); sys.exit(1)
j = s.index('\n', j) + 1
io.open(p, 'w', encoding='utf-8').write(s[:k] + s[j:])
print('NEG-A patch ok（删 %d 字节，含 #ifndef/#endif 两侧）' % (j - k))
PYEOF
    ( cd "$ROOT" && ./selfhost/devbuild.sh pxc >"$W/negA_build.log" 2>&1 )
    if grep -q '^✅ pxc' "$W/negA_build.log"; then
        cp /tmp/pxcdev "$W/negA_pxc"
        PXC_BIN_OVERRIDE="$W/negA_pxc" pair c srv.px 18831 basic negA
        negA_rc=$(cat "$W/c-negA/cli.rc" 2>/dev/null); negA_rc=${negA_rc:-9}
        # ⚠ 不要写 `grep -c … || echo 0`：grep -c 无命中时**已经**打印 0（rc=1）⇒ 会得到 "0\n0"
        #   ⇒ 判据恒红（本仓 R74 记过的坑）。这里先取再补默认值。
        negA_hit=$(grep -c 'CLI connected = true' "$W/c-negA/cli.out" 2>/dev/null)
        negA_hit=${negA_hit:-0}
        [ "$negA_rc" = "9" ] && echo "    （负控 A 的 pair 未正常完成：见 $W/c-negA/）"
        if [ "$negA_hit" = "0" ]; then ok "负控 A · 撤接管块 ⇒ 握手必失败（判据有牙）"
        else bad "负控 A · 撤接管块后仍握手成功（判据无牙）"; fi
    else
        bad "负控 A · 重建失败"; tail -5 "$W/negA_build.log"
    fi
    restore_neg

    # ── 负控 B：只拿走 `Sec-WebSocket-Key` 预判 ⇒ 接管面变宽/行为改变，必须能观测到差异
    python3 - "$RH" <<'PYEOF'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = "    g_ws_conns[slot].hdrs = hdr_raw;   // 所有权移交（槽复用时由 ws_free_hs_locked 归还）"
if s.count(old) != 1:
    print('NEG-B anchor miss'); sys.exit(1)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, "    /* NEG-B: 不登记握手头原文 */", 1))
print('NEG-B patch ok')
PYEOF
    ( cd "$ROOT" && ./selfhost/devbuild.sh pxc >"$W/negB_build.log" 2>&1 )
    if grep -q '^✅ pxc' "$W/negB_build.log"; then
        cp /tmp/pxcdev "$W/negB_pxc"
        PXC_BIN_OVERRIDE="$W/negB_pxc" pair c srv.px 18832 basic negB
        negB_hs=$(grep -c 'CLI connected = true' "$W/c-negB/cli.out" 2>/dev/null); negB_hs=${negB_hs:-0}
        negB_hdr=$(grep -c 'upg=websocket' "$W/c-negB/srv.out" 2>/dev/null); negB_hdr=${negB_hdr:-0}
        if [ "$negB_hs" = "1" ] && [ "$negB_hdr" = "0" ]; then
            ok "负控 B · 不登记握手头 ⇒ 握手仍成功但**元信息层**独立判红（与 A 相互独立）"
        else
            bad "负控 B · 期望「握手成功 + 头缺失」，实际 hs=$negB_hs hdr=$negB_hdr"
        fi
    else
        bad "负控 B · 重建失败"; tail -5 "$W/negB_build.log"
    fi
    restore_neg

    # ── 负控 C：判据自伤 —— 把「三轨一致」比对改成恒真 ⇒ 上面若真有差异也不会红
    # 负控 C（正判据）：人为给一侧加一行 ⇒ 三轨比对判据**必须**发现
    A="$W/c-basic"; B="$W/vm-basic"
    cp "$A/cli.out" "$W/negC_orig.out"
    echo "NEG-C-EXTRA" >> "$A/cli.out"
    if tri_ok "$A/cli.out" "$B/cli.out"; then
        bad "负控 C · 人为差异未被三轨比对发现（比对无牙）"
    else
        ok "负控 C · 人为差异被三轨比对发现（比对有区分力）"
    fi
    cp "$W/negC_orig.out" "$A/cli.out"
    if tri_ok "$A/cli.out" "$B/cli.out"; then
        ok "负控 C · 还原后重新一致（红确由人为差异引起）"
    else
        bad "负控 C · 还原后仍不一致 ⇒ 前面的红来源不明"
    fi
    cmp -s "$SNAP/runtime_ws.c" "$RH" && cmp -s "$SNAP/runtime.c" "$RC"
    chk "负控 · 源已逐字节还原" "$?" "0"
fi

# ------------------------------------------------------------
echo "[10] 覆盖边界（如实登记）"
# ------------------------------------------------------------
cat <<'EOF'
  · httq_serve_unix（Unix socket）轨的 WS 接管未单独覆盖（走同一 http_conn_worker 分支）
  · TLS + WS：本门用明文；px_serve 轨的 TLS 复用 M235 的 PxConn（握手已完成），未单独覆盖
  · `opts.headers` 未实现（101 的 Upgrade/Connection/Sec-WebSocket-Accept 由 RFC 6455 规定）
  · manual 模式的**拒绝**路径只提供「不写 101 + ws_close」；专用「回普通 HTTP 响应」API 待后续
  · 语言层在回调里自行 spawn 读循环的用法未覆盖（本门为同步回调 + runtime 泵循环）
EOF

echo "══ 汇总：通过 $PASS / 失败 $FAIL ══"
[ "$FAIL" = "0" ] && echo "M236-VERIFY-OK" || echo "M236-VERIFY-FAIL"
exit $([ "$FAIL" = "0" ] && echo 0 || echo 1)
