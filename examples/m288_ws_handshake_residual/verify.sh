#!/usr/bin/env bash
# ============================================================
# examples/m288_ws_handshake_residual/verify.sh
# ------------------------------------------------------------
# M288（缺陷 501）：WS 握手「残余字节」被丢弃 ⇒ 首帧静默丢失
#
# 形状（一行）：握手读头的函数在找到 `\r\n\r\n` 之后，把**同一次读入**的其余字节整块丢掉
#   （`buf[header_end]=0; *out_len=header_end`）。而 RFC 6455 允许两种「同段到达」：
#     ① 服务端把 101 与首帧合并发出；② 客户端在升级请求之后**不等 101** 就发首帧。
#   两种情形都会让**首帧**与头部落在同一次读里 ⇒ 静默丢弃 ⇒ 帧流从第一个字节就错位。
#
# 三个丢弃点（本轮一并收口）：
#   A. `ws_read_http_header(PxConn*)`    —— 客户端（明文/TLS）+ `ws_serve` 服务端
#   B. `ws_read_http_header_fd(int fd)`  —— 客户端明文（**重复实现**，本轮删除）
#   C. `px_pxpend` worker 的读头循环      —— `px_serve` 的 WS 接管点（残余落在 `buf` 里）
#
# 为什么本门用 python 裸 socket 对端（而不用 .px 对端）：
#   「两次 send 会不会被内核合成一段」不由应用层决定（Nagle/发送队列/接收方调度）——
#   .px 的 `ws_send` 无法保证「同段」。裸 socket 的 `sendall(101 + frame)` 才固定了这个
#   变量 ⇒ 本门**与负载无关**（负载档实测 7.5% 复现；本夹具 100%）。
#
# 判据层次：
#   [1] 静态：三个收口点在位 + 重复实现已删 + 残余在 free_res 释放
#   [2] 客户端面（三模式对照 × 三轨）：split / combined / partial（+ big 大帧）
#   [3] 服务端面（两模式对照）：together（同段）/ after（分开）
#   [4] 负载档：服务端「紧接 101 就 ws_send」× N 次，首帧丢失必须为 0
#   [5] 负控 3 道（A 撤头部交还 / B 撤接管点交还 / C 判据自伤）；源逐字节还原
#   [6] 覆盖边界登记
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail

ROOT="${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
HERE="$ROOT/examples/m288_ws_handshake_residual"
W="${M288_GATE_W:-/tmp/m288_gate}"
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
chk()  { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }

LEFT="$W/pids"
cleanup_all() {
    [ -f "$LEFT" ] || return 0
    local p
    while read -r p; do
        [ -n "$p" ] || continue
        kill "$p" 2>/dev/null; kill -9 "$p" 2>/dev/null
    done < "$LEFT"
}
# ⚠ 绝不用 pkill -f（命令串含模式时会把工具调用自己打掉 —— 本仓记过多次）
trap 'cleanup_all' EXIT

echo "=== M288 门 · WS 握手残余字节（首帧静默丢失）==="
echo "ROOT=$ROOT  W=$W"
rm -rf "$W"; mkdir -p "$W"; : > "$LEFT"

RS="$ROOT/runtime/runtime_ws.c"
RC="$ROOT/runtime/runtime.c"
RHDR="$ROOT/runtime/runtime.h"

# ------------------------------------------------------------
echo "[1] 静态：三个收口点在位 + 重复实现已删"
# ------------------------------------------------------------
S=0
grep -q 'void px_conn_pushback(PxConn\* c, const void\* data, size_t n)' "$RC" && S=$((S+1))
grep -q 'void px_conn_pushback(PxConn\* c, const void\* data, size_t n);' "$RHDR" && S=$((S+1))
grep -q 'char\* pb;                // 残余字节' "$RHDR" && S=$((S+1))
grep -q 'if (c->pb && c->pb_off < c->pb_len) {' "$RC" && S=$((S+1))
grep -q 'if (c->pb) { xfree(c->pb); c->pb = NULL;' "$RC" && S=$((S+1))
chk '静态 · px_conn_pushback 定义/声明/字段/消费/释放（5 处）' "$S" "5"

# 三个收口点：按**形状**计数（不绑变量名 —— M242 缺陷 407 的教训）
N1=$(grep -c 'px_conn_pushback(c, buf + rest_off, (size_t)rest_len)' "$RS")
chk "静态 · 收口 A：ws_read_http_header 交还残余" "$N1" "1"
N2=$(grep -c 'ws_client_handshake_px' "$RS")
chk "静态 · 客户端握手点全部走 PxConn 版（1 处定义 + 5 处调用）" "$N2" "6"
grep -q 'ws_client_handshake(' "$RS" && N3=1 || N3=0
chk "静态 · 重复实现 ws_client_handshake（fd 版）已删除" "$N3" "0"
grep -q 'ws_read_http_header_fd' "$RS" && N4=1 || N4=0
chk "静态 · 重复实现 ws_read_http_header_fd 已删除" "$N4" "0"
N5=$(grep -c 'px_conn_pushback(ws_c, buf + (header_end + 4),' "$RC")
chk "静态 · 收口 C：px_serve WS 接管点交还残余" "$N5" "1"
grep -q 'if (c->pb && c->pb_off < c->pb_len) return 1;' "$RS"
chk "静态 · has_buffered 把 pb 也算「已就绪」（否则被 poll 挡住）" "$?" "0"

# ------------------------------------------------------------
echo "[2] 客户端面（三模式对照 × 三轨）—— 与负载无关的确定性判据"
# ------------------------------------------------------------
build_probe() {   # $1=engine  $2=dir  $3=file
    local eng="$1" d="$2" f="$3"
    if [ "$eng" = "c" ]; then
        PX_PXC_BIN="$ROOT/bootstrap/pxc" PX_BUILD_ENGINE=c "$ROOT/tools/px" build "$d/$f" >"$d/build_$f.log" 2>&1
    else
        PXC_VM_BIN="$ROOT/bootstrap/pxc_vm" PX_BUILD_ENGINE=vm "$ROOT/tools/px" build "$d/$f" >"$d/build_$f.log" 2>&1
    fi
}

cli_run() {   # $1=engine  $2=mockmode  $3=port  $4=dirtag   → $d/cli.out
    local eng="$1" mm="$2" port="$3" tag="$4"
    local d="$W/$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$HERE/cli.px" "$d/"
    build_probe "$eng" "$d" cli.px || { echo "BUILDFAIL cli($eng)" > "$d/cli.out"; tail -3 "$d/build_cli.px.log" >> "$d/cli.out"; return 9; }
    python3 "$HERE/mock_ws.py" "$port" "$mm" > "$d/mock.log" 2>&1 &
    local mpid=$!
    echo "$mpid" >> "$LEFT"
    local k
    for k in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30; do
        grep -q 'listening' "$d/mock.log" 2>/dev/null && break
        sleep 0.1
    done
    if [ "$eng" = "i" ]; then
        ( cd "$d" && timeout 25 "$ROOT/bootstrap/pxi" "$port" "$mm" "$d/cli.px" > cli.out 2>&1 )
    else
        ( cd "$d" && timeout 25 "./build/cli" "$port" "$mm" > cli.out 2>&1 )
    fi
    sleep 0.3
    kill "$mpid" 2>/dev/null; kill -9 "$mpid" 2>/dev/null
    wait "$mpid" 2>/dev/null   # 消费作业状态 ⇒ 不再打印 "Killed"
    return 0
}

# WEAK=1 时判决恒真（仅负控 C 用：证明「红」确实来自该判决）
WEAK=0
R1_CHK() {   # $1=标签  $2=cli.out路径  $3=期望(键=值 串)
    local tag="$1" f="$2" want="$3" got
    got=$(grep -E '^R1NULL =|^R1LEN =|^R1EQ =' "$f" 2>/dev/null | tr '\n' ' ')
    if [ "$WEAK" = "1" ]; then ok "$tag（弱判据·恒真）"; return 0; fi
    case "$got" in
        *"$want"*) ok "$tag（$got）" ;;
        *) bad "$tag：期望含 [$want]，实际 [$got]" ;;
    esac
}

for eng in c vm i; do
    cli_run "$eng" split    "$((18950 + RANDOM % 40))" "$eng-split"
    cli_run "$eng" combined "$((18950 + RANDOM % 40))" "$eng-combined"
    cli_run "$eng" partial  "$((18950 + RANDOM % 40))" "$eng-partial"
    R1_CHK "$eng · split（101 与首帧分开）"    "$W/$eng-split/cli.out"    "R1EQ = true"
    R1_CHK "$eng · combined（101+首帧同段）"   "$W/$eng-combined/cli.out" "R1EQ = true"
    R1_CHK "$eng · partial（101+半帧同段）"    "$W/$eng-partial/cli.out"  "R1EQ = true"
done
cli_run c big "$((18950 + RANDOM % 40))" "c-big"
R1_CHK "c · big（3000 字节首帧同段）" "$W/c-big/cli.out" "R1LEN = 3000"

# ------------------------------------------------------------
echo "[3] 服务端面（together=同段 / after=分开）"
# ------------------------------------------------------------
pair_raw() {   # $1=engine  $2=mode  $3=port  $4=dirtag   → $d/${mode}.srv.out
    local eng="$1" mode="$2" port="$3" tag="$4"
    local d="$W/$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$HERE/srv.px" "$d/"
    build_probe "$eng" "$d" srv.px || { echo "BUILDFAIL srv($eng)"; tail -3 "$d/build_srv.px.log"; return 9; }
    if [ "$eng" = "i" ]; then
        ( cd "$d" && exec "$ROOT/bootstrap/pxi" "$port" "$d/srv.px" ) > "$d/$mode.srv.out" 2>&1 &
    else
        ( cd "$d" && exec ./build/srv "$port" ) > "$d/$mode.srv.out" 2>&1 &
    fi
    local spid=$!
    echo "$spid" >> "$LEFT"
    sleep 1.5
    timeout 20 python3 "$HERE/client_raw.py" "$port" "$mode" > "$d/$mode.cli.out" 2>&1
    sleep 0.4
    kill "$spid" 2>/dev/null; kill -9 "$spid" 2>/dev/null
    wait "$spid" 2>/dev/null   # 消费作业状态 ⇒ 不再打印 "Killed"
    return 0
}
SRV_CHK() {   # $1=标签  $2=srv.out路径  $3=期望
    local tag="$1" f="$2" want="$3" got
    got=$(grep -E '^SRVNULL =|^SRVLEN =|^SRVEQ =' "$f" 2>/dev/null | tr '\n' ' ')
    if [ "$WEAK" = "1" ]; then ok "$tag（弱判据·恒真）"; return 0; fi
    case "$got" in
        *"$want"*) ok "$tag（$got）" ;;
        *) bad "$tag：期望含 [$want]，实际 [$got]" ;;
    esac
}
for eng in c vm; do
    pair_raw "$eng" together "$((19050 + RANDOM % 40))" "$eng-srv"
    pair_raw "$eng" after    "$((19050 + RANDOM % 40))" "$eng-srv2"
    SRV_CHK "$eng · together（请求+首帧同段 ⇒ 首帧必须收到）" "$W/$eng-srv/together.srv.out"  "SRVEQ = true"
    SRV_CHK "$eng · after（分开发 ⇒ 对照，修前也必须正常）"   "$W/$eng-srv2/after.srv.out"   "SRVEQ = true"
done

# ------------------------------------------------------------
echo "[4] 负载档：服务端「紧接 101 就 ws_send」× 20 次（原症状 7.5%）"
# ------------------------------------------------------------
LOAD_N="${M288_LOAD_N:-20}"
d="$W/load"; rm -rf "$d"; mkdir -p "$d"
cp "$HERE/cli.px" "$d/"; cp "$HERE/srv.px" "$d/"
if build_probe c "$d" srv.px && build_probe c "$d" cli.px; then
    for k in 1 2 3 4; do ( while :; do :; done ) & echo $! >> "$LEFT"; done
    sleep 1
    connn=0; eqn=0; nuln=0
    for i in $(seq 1 "$LOAD_N"); do
        port=$((19100 + i))
        ( cd "$d" && exec ./build/srv "$port" ) > "$d/srv_$i.out" 2>&1 &
        spid=$!; echo "$spid" >> "$LEFT"
        sleep 1.2
        ( cd "$d" && timeout 25 ./build/cli "$port" load > "cli_$i.out" 2>&1 )
        sleep 0.2
        kill "$spid" 2>/dev/null; sleep 0.1; kill -9 "$spid" 2>/dev/null; wait "$spid" 2>/dev/null
        grep -q 'CONN = true' "$d/cli_$i.out" && connn=$((connn+1))
        grep -q 'R1EQ = false' "$d/cli_$i.out" && nuln=$((nuln+1))
        grep -q 'R1EQ = true' "$d/cli_$i.out" && eqn=$((eqn+1))
    done
    chk "负载档 · 建链成功 $LOAD_N/$LOAD_N（否则计数无意义）" "$connn" "$LOAD_N"
    chk "负载档 · 首帧丢失次数" "$nuln" "0"
    chk "负载档 · 首帧正确次数" "$eqn" "$LOAD_N"
else
    bad "负载档 · 构建失败"
fi

# ------------------------------------------------------------
echo "[5] 负控（各自独立判红；源逐字节还原）"
# ------------------------------------------------------------
if [ "$NEG_SKIP" = "1" ]; then
    echo "  ⏭ 负控跳过（--neg-skip，CI 用）"
else
    SNAP="$W/neg_snap"; mkdir -p "$SNAP"
    cp "$RS" "$SNAP/runtime_ws.c"; cp "$RC" "$SNAP/runtime.c"
    restore_neg() { cp "$SNAP/runtime_ws.c" "$RS"; cp "$SNAP/runtime.c" "$RC"; }
    trap 'restore_neg 2>/dev/null; cleanup_all' EXIT

    # ── 负控 A：撤「收口 A」（ws_read_http_header 不交还残余）
    #   期望：combined / partial 必红（首帧丢），split **仍绿** ⇒ 红色的来源精确
    python3 - "$RS" <<'PYEOF'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = '        if (rest_len > 0) px_conn_pushback(c, buf + rest_off, (size_t)rest_len);'
if s.count(old) != 1:
    print('NEG-A anchor miss'); sys.exit(1)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, '        (void)rest_len;   /* NEG-A：不交还残余 */', 1))
print('NEG-A patch ok')
PYEOF
    cli_run c combined "$((19250 + RANDOM % 20))" "negA-combined"
    cli_run c partial  "$((19250 + RANDOM % 20))" "negA-partial"
    cli_run c split    "$((19250 + RANDOM % 20))" "negA-split"
    na_c=$(grep -c 'R1EQ = true' "$W/negA-combined/cli.out" 2>/dev/null); na_c=${na_c:-0}
    na_p=$(grep -c 'R1EQ = true' "$W/negA-partial/cli.out" 2>/dev/null); na_p=${na_p:-0}
    na_s=$(grep -c 'R1EQ = true' "$W/negA-split/cli.out" 2>/dev/null); na_s=${na_s:-0}
    if [ "$na_c" = "0" ] && [ "$na_p" = "0" ] && [ "$na_s" = "1" ]; then
        ok "负控 A · 撤头部交还 ⇒ 同段两模式必红、分开发仍绿（来源精确）"
    else
        bad "负控 A · 期望 combined=0 partial=0 split=1，实际 $na_c/$na_p/$na_s"
    fi
    # ── 负控 C：判据自伤 —— 同一份「已打桩」的产物，判决一放宽 ⇒ A 的红必须消失
    WEAK=1
    cli_run c combined "$((19250 + RANDOM % 20))" "negC-combined"
    R1_CHK "负控 C · 弱判据下同一产物" "$W/negC-combined/cli.out" "R1EQ = true"   # 恒真 ⇒ 必 ok
    WEAK=0
    restore_neg
    # ⚠ 还原后必须复跑：证明「弱判据下绿」确由判据造成（否则红可能来自别的变化）
    cli_run c combined "$((19250 + RANDOM % 20))" "negC2-combined"
    R1_CHK "负控 C · 还原并复跑（真判据下必须回到绿）" "$W/negC2-combined/cli.out" "R1EQ = true"

    # ── 负控 B：撤「收口 C」（px_serve 接管点不交还残余）
    #   期望：together 必红（首帧丢），after **仍绿** ⇒ 与 A 相互独立
    python3 - "$RC" <<'PYEOF'
import io, sys
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = ('                    if (len > header_end + 4)\n'
       '                        px_conn_pushback(ws_c, buf + (header_end + 4),\n'
       '                                         (size_t)(len - (header_end + 4)));')
if s.count(old) != 1:
    print('NEG-B anchor miss'); sys.exit(1)
io.open(p, 'w', encoding='utf-8').write(s.replace(old, '                    /* NEG-B：不交还残余 */', 1))
print('NEG-B patch ok')
PYEOF
    pair_raw c together "$((19350 + RANDOM % 20))" "negB-srv"
    pair_raw c after    "$((19350 + RANDOM % 20))" "negB-srv2"
    nb_t=$(grep -c 'SRVEQ = true' "$W/negB-srv/together.srv.out" 2>/dev/null); nb_t=${nb_t:-0}
    nb_a=$(grep -c 'SRVEQ = true' "$W/negB-srv2/after.srv.out" 2>/dev/null); nb_a=${nb_a:-0}
    if [ "$nb_t" = "0" ] && [ "$nb_a" = "1" ]; then
        ok "负控 B · 撤接管点交还 ⇒ together 必红、after 仍绿（与 A 独立）"
    else
        bad "负控 B · 期望 together=0 after=1，实际 $nb_t/$nb_a"
    fi
    restore_neg
    cmp -s "$SNAP/runtime_ws.c" "$RS" && cmp -s "$SNAP/runtime.c" "$RC"
    chk "负控 · 源已逐字节还原" "$?" "0"
fi

# ------------------------------------------------------------
echo "[6] 覆盖边界（如实登记）"
# ------------------------------------------------------------
cat <<'EOF'
  · TLS 面的「握手残余」走同一条 `px_conn_read`（本门用明文覆盖；TLS 未单独压）
  · `ws_serve`（独立 bind 的服务端）走 `ws_server_handshake` ⇒ 与收口 A 同一条路径，
    本门未单独起 `ws_serve` 进程验证（同一函数的同一处修改）
  · `http_serve` 轨的 WS 接管复用 http_conn_worker 的读头循环 —— M144（缺陷 124）
    已为该轨引入 fd 级 `px_conn_pend_*` 余留机制；本轮未改动该轨
  · `px_serve` 的 **HTTP 管道化**（同一段里的下一个请求）走的是另一条余留路径，
    不在本门面内（缺陷 124 修的是 http_serve 轨）
  · 残余缓冲的容量上界 = 一次读入量（读头循环单次 ≤ 64KB）；超大首帧会被拆成多次读，
    由 `ws_read_exact` 的循环天然覆盖（本门 big 用例 3000 字节为抽样）
EOF

echo "══ 汇总：通过 $PASS / 失败 $FAIL ══"
[ "$FAIL" = "0" ] && echo "M288-VERIFY-OK" || echo "M288-VERIFY-FAIL"
exit $([ "$FAIL" = "0" ] && echo 0 || echo 1)
