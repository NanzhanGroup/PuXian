#!/usr/bin/env bash
# ============================================================
# examples/m295_quic_slot/verify.sh
# ------------------------------------------------------------
# M295 门：**QUIC 连接槽位耗尽**（缺陷 508）—— 从「静默拒连」到「有界可观测」
#
# 立论（为什么需要这道门）：
#   M294 只用「探测串行建连到第几次失败」这种**人工**手段发现 508，而那条手段不在任何
#   门里 ⇒ 修好之后**没有东西看着它**。本门把它常设化，并把三件事同时钉住：
#     ① 上限（64 → 256，且是**单一真相**）
#     ② 「满了」必须**可观测**（不再静默）
#     ③ `quic_close` 必须**告知对端**（否则优雅关闭与进程被杀对服务端不可区分）
#
# 判据层次：
#   [1] 静态（源码派生；**只读代码行**，注释不算）—— 8 条，逐条对应一处修复
#   [2] 编译 + 强就绪（M293 F1：不写 ready 文件、只认运行时绑定后的那一行）
#   [3]/[4] 动态契约：`quic_pool_stats()` 键与取值 · 参数校验
#   [5] 动态行为（优雅关闭）：串行 24 ⇒ **全 OK**
#   [6] 动态行为（静默退出）：串行 **264** ⇒ **首次失败 @256**（核心行为判据）
#   [7] 负控（3 道，各自独立判红；`--neg-skip` 跳过）
#   [8] 覆盖边界（如实登记）
#
# 用法：bash verify.sh [--neg-skip] [<仓库根>]
#   ⚠️ 参数解析刻意**不吞掉** `--neg-skip`（M288b 缺陷：`ROOT="${1:-…}"` 会把开关当路径，
#      于是 CI 上 0s 判红且零输出）。
# ============================================================
# M276：门级互斥锁（99 个门会在负控里就地改源码 ⇒ 必须与全量门互斥）。
# ⚠️ 必须在 set -u 之前 source（gate_lock.sh 自己管理严格模式）。
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
NEG=1
ROOT=""
for a in "$@"; do
  case "$a" in
    --neg-skip) NEG=0 ;;
    --*) ;;
    *) [ -z "$ROOT" ] && ROOT="$a" ;;
  esac
done
[ -z "$ROOT" ] && ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT" || { echo "❌ 无法进入仓库根：$ROOT"; exit 1; }
D="examples/m295_quic_slot"
TMPL="/tmp/m295_gate.$$"
rm -rf "$TMPL"; mkdir -p "$TMPL/logs"

PASS=0; FAIL=0
chk() { # chk <名> <实际> <期望>
  if [ "$2" = "$3" ]; then PASS=$((PASS+1)); printf '  ✅ %s\n' "$1"
  else FAIL=$((FAIL+1)); printf '  ❌ %s\n      实际=[%s]\n      期望=[%s]\n' "$1" "$2" "$3"; fi
}
chk_ge() { # chk_ge <名> <实际> <下界>
  case "$2" in ''|*[!0-9]*) FAIL=$((FAIL+1)); printf '  ❌ %s（非数字：[%s]）\n' "$1" "$2"; return ;; esac
  if [ "$2" -ge "$3" ]; then PASS=$((PASS+1)); printf '  ✅ %s（%s ≥ %s）\n' "$1" "$2" "$3"
  else FAIL=$((FAIL+1)); printf '  ❌ %s（%s < %s）\n' "$1" "$2" "$3"; fi
}
num() { case "${1:-}" in ''|*[!0-9]*) echo 0 ;; *) echo "$1" ;; esac; }
chk_range() { # chk_range <名> <实际> <下界> <上界>
  local v; v="$(num "$2")"
  if [ "$v" -ge "$3" ] && [ "$v" -le "$4" ]; then PASS=$((PASS+1)); printf '  ✅ %s（%s ∈ [%s,%s]）\n' "$1" "$v" "$3" "$4"
  else FAIL=$((FAIL+1)); printf '  ❌ %s（%s ∉ [%s,%s]）\n' "$1" "$v" "$3" "$4"; fi
}

echo "===== M295 门 · QUIC 连接槽位耗尽（缺陷 508）====="
echo "    仓库根 = $ROOT · 负控 = $([ "$NEG" = 1 ] && echo 跑 || echo 跳过)"

# ---------- 去注释器（判据只读代码行；M251 的教训：注释里的旧文本会伪造命中）----------
strip_c() {
  python3 - "$1" <<'PY'
import re, sys
src = open(sys.argv[1], encoding='utf-8').read()
src = re.sub(r'/\*.*?\*/', '', src, flags=re.S)
out = []
for ln in src.split('\n'):
    i = ln.find('//')
    if i >= 0: ln = ln[:i]
    out.append(ln)
sys.stdout.write('\n'.join(out))
PY
}
RH="$TMPL/runtime.h.code"; RQ="$TMPL/runtime_quic.c.code"
H3="$TMPL/runtime_h3.c.code"; QD="$TMPL/runtime_h3_qpack_dyn.c.code"
strip_c runtime/runtime.h > "$RH"
strip_c runtime/runtime_quic.c > "$RQ"
strip_c runtime/runtime_h3.c > "$H3"
strip_c runtime/runtime_h3_qpack_dyn.c > "$QD"

# ============================================================
echo
echo "[1] 静态判据（源码派生 · 只读代码行）"
# --- 1.1 单一真相：上限只定义一次 ---
chk "runtime.h 定义 PX_QUIC_CONN_MAX" \
    "$(grep -cE '^#define[[:space:]]+PX_QUIC_CONN_MAX[[:space:]]+256' "$RH")" "1"
chk "runtime_quic.c 的 QUIC_CONN_MAX 引用它" \
    "$(grep -cE '^#define[[:space:]]+QUIC_CONN_MAX[[:space:]]+PX_QUIC_CONN_MAX' "$RQ")" "1"
chk "runtime_h3.c 的 H3_MAX_CONN 引用它" \
    "$(grep -cE '^#define[[:space:]]+H3_MAX_CONN[[:space:]]+PX_QUIC_CONN_MAX' "$H3")" "1"
chk "runtime_h3_qpack_dyn.c 的 QD_MAX_SESS 引用它" \
    "$(grep -cE '^#define[[:space:]]+QD_MAX_SESS[[:space:]]+PX_QUIC_CONN_MAX' "$QD")" "1"
# --- 1.2 不得再有硬编码 64（三处旧病灶）---
chk "H3/QD 无残留硬编码 64" \
    "$(grep -cE '^#define[[:space:]]+(H3_MAX_CONN|QD_MAX_SESS)[[:space:]]+64[[:space:]]*$' "$H3" "$QD" | awk -F: '{s+=$2} END{print s+0}')" "0"
# --- 1.3 满了必须响亮 ---
chk_ge "quic_alloc_fail_note 定义且被调用" "$(grep -c 'quic_alloc_fail_note' "$RQ")" 2
chk "cidtab 容量按连接上限成比例（非硬编码 256）" \
    "$(grep -cE '^#define[[:space:]]+QUIC_CIDTAB_MAX' "$RQ")" "1"
# --- 1.4 quic_close 必须发 CONNECTION_CLOSE，且 CCERR 用结构体（防 SEGV 回退）---
chk "bi_quic_close 发 CONNECTION_CLOSE" "$(grep -c 'ngtcp2_conn_write_connection_close' "$RQ")" "1"
chk "CCERR 走 ngtcp2_ccerr 结构体（不得强行转整数）" \
    "$(grep -c 'ngtcp2_ccerr_set_transport_error' "$RQ")" "1"
# ⚠️ 判据必须**贴着调用点**：`(uint64_t)NGTCP2_NO_ERROR` 在 `ngtcp2_ccerr_set_transport_error`
#    的实参位是**正确写法**（该形参就是 uint64_t）；危险的是把它放到 write_connection_close 的 CCERR 位。
chk "CCERR 位传的是 &ccerr（结构体指针），不是整数" \
    "$(awk '/ngtcp2_conn_write_connection_close\(/{f=1} f&&/&ccerr/{c++} f&&/\);/{f=0} END{print c+0}' "$RQ")" "1"
# --- 1.5 线程创建失败不得泄漏槽位 ---
chk_ge "pthread_create 失败路径释放槽位（置位/判据/释放三处）" "$(grep -c 'thr_create_failed' "$RQ")" 3
# --- 1.6 draining 不再刷屏（改计数）---
chk "draining 分支只计数、不打印" \
    "$(grep -A 6 'NGTCP2_ERR_DRAINING || rv == NGTCP2_ERR_DROP_CONN' "$RQ" | grep -c 'fprintf')" "0"
chk_ge "draining 分支已计数（g_quic_peer_closed）" "$(grep -c 'g_quic_peer_closed++' "$RQ")" 1
chk_ge "stats 暴露 peer_closed_cnt" "$(grep -c 'peer_closed_cnt' "$RQ")" 1

# ============================================================
echo
echo "[2] 编译 + 强就绪"
EPHEM_LO="$(awk '{print $1}' /proc/sys/net/ipv4/ip_local_port_range 2>/dev/null || echo 32768)"
case "$EPHEM_LO" in ''|*[!0-9]*) EPHEM_LO=32768 ;; esac
PHI=$((EPHEM_LO - 3)); [ "$PHI" -gt 32767 ] && PHI=32767; [ "$PHI" -lt 26000 ] && PHI=26000
PORT=$((24000 + RANDOM % (PHI - 24000))); PORT=$((PORT / 2 * 2))
free_port() {
  local p="$1" pid=""
  pid="$( (ss -ltnup 2>/dev/null || netstat -ltnup 2>/dev/null) | grep -E ":${p} " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
  [ -n "$pid" ] && { kill -9 "$pid" 2>/dev/null; sleep 0.3; }
  return 0
}
SRV_PID=""
cleanup() { [ -n "$SRV_PID" ] && kill -9 "$SRV_PID" 2>/dev/null; free_port "$PORT"; free_port "$((PORT+1))"; rm -rf "$TMPL"; return 0; }
trap 'cleanup' EXIT

mkdir -p "$TMPL/src"
cp -f "$D/srv.px" "$D/slot_cli.px" "$D/pool_probe.px" "$D/pool_bad.px" "$TMPL/src/"
BD="$TMPL/src/build"
BUILD_OK=1
for f in srv slot_cli pool_probe; do
  if ! ( cd "$ROOT" && ./tools/px build "$TMPL/src/$f.px" ) >"$TMPL/logs/build_$f.log" 2>&1; then
    BUILD_OK=0
    echo "  ⚠️ 编译 $f 失败（尾 5 行）："; tail -5 "$TMPL/logs/build_$f.log" | sed 's/^/      /'
  fi
done
# pool_bad **期望失败**（参数错）⇒ 不参与 BUILD_OK，但必须单独编译以取得日志
( cd "$ROOT" && ./tools/px build "$TMPL/src/pool_bad.px" ) >"$TMPL/logs/build_pool_bad.log" 2>&1 || true
chk "srv/slot_cli/pool_probe 编译成功" "$BUILD_OK" "1"
chk "pool_bad 编译被拦（E3004，精确 arity）" \
    "$(grep -cE 'E3004|R1002' "$TMPL/logs/build_pool_bad.log")" "1"
if [ "$BUILD_OK" != "1" ]; then echo "  ⛔ 编译失败 ⇒ 退出"; printf '\n===== 失败 %s 项 · 通过 %s 项 =====\n' "$FAIL" "$PASS"; exit 1; fi

# 起服务
free_port "$PORT"; free_port "$((PORT+1))"
SRV_LOG="$TMPL/srv.log"
setsid nohup env M295_HTTP_PORT="$PORT" "$BD/srv" >"$SRV_LOG" 2>&1 </dev/null &
disown 2>/dev/null || true
i=0; while [ "$i" -lt 100 ]; do sleep 0.1; grep -q "普贤应用服务器" "$SRV_LOG" 2>/dev/null && break; i=$((i+1)); done
SRV_PID="$( (ss -ltnp 2>/dev/null) | grep -E ":${PORT} " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
READY=0; [ -n "$SRV_PID" ] && kill -0 "$SRV_PID" 2>/dev/null && READY=1
chk "服务已就绪（强判据：绑定后的运行时行 + 进程存活）" "$READY" "1"
if [ "$READY" != "1" ]; then tail -12 "$SRV_LOG" | sed 's/^/      /'; printf '\n===== 失败 %s 项 · 通过 %s 项 =====\n' "$FAIL" "$PASS"; exit 1; fi
# 功能就绪探针（M283 P2：真发一次请求，不只看日志）
M295_HTTP_PORT="$PORT" M295_TAG=r0 timeout -k 5 20 "$BD/slot_cli" >"$TMPL/logs/ready.out" 2>&1
chk "功能就绪探针（真发一次 H3 请求）" "$(grep -E '^TAG ' "$TMPL/logs/ready.out" 2>/dev/null | tail -1)" "TAG r0 OK"

# ============================================================
echo
echo "[3] 动态契约：quic_pool_stats()"
POOL="$(M295_HTTP_PORT="$PORT" timeout -k 5 15 "$BD/pool_probe" 2>&1 | grep '^POOL ' | head -1)"
pl() { echo "$POOL" | tr ' ' '\n' | grep "^$1=" | cut -d= -f2; }
chk "POOL 行存在" "$([ -n "$POOL" ] && echo 1 || echo 0)" "1"
chk "conns_max == 256" "$(pl conns_max)" "256"
chk "conns_used == 0（池空）" "$(pl conns_used)" "0"
chk "cidtab_max == 2048" "$(pl cidtab_max)" "2048"
chk "alloc_fail == 0（尚未满过）" "$(pl alloc_fail)" "0"
chk "13 键齐全" "$(M295_HTTP_PORT="$PORT" timeout -k 5 15 "$BD/pool_probe" 2>&1 | grep '^POOL-KEYS' | head -1)" "POOL-KEYS 13 GOT 13"

echo
echo "[4] 动态契约：参数校验（精确 arity）"
if [ -x "$BD/pool_bad" ]; then
  MB="$(M295_HTTP_PORT="$PORT" timeout -k 5 15 "$BD/pool_bad" 2>&1 | tail -1)"
  chk "给参数 ⇒ 响亮失败（R1002）" "$(echo "$MB" | grep -c 'R1002')" "1"
else
  chk "给参数 ⇒ 编译期已拦（E3004）" \
      "$(grep -cE 'E3004' "$TMPL/logs/build_pool_bad.log")" "1"
fi

# ============================================================
echo
# ⚠️ 判据设计的教训（本门首跑踩到）：`pool_probe` 是**独立进程**，它读的是**自己的**空池
#    ⇒ 「对端关闭计数」在服务端进程里，从外面读不到（那条判据恒 0 = 假红）。
#    改用量级**远超上限**的串行优雅关闭 —— 若 `quic_close` 不告知对端（修前），
#    槽位不回收 ⇒ 第 257 次起必然失败；现在**全 OK** 就是「及时回收」的行为级证明。
echo "[5] 动态行为：**优雅关闭** 串行 264 次（> 上限）⇒ 全 OK（及时回收的行为级证明）"
GOK=0
for k in $(seq 264); do
  M295_HTTP_PORT="$PORT" M295_TAG="g${k}" timeout -k 5 20 "$BD/slot_cli" >"$TMPL/logs/g$k.out" 2>&1
  [ "$(grep -E '^TAG ' "$TMPL/logs/g$k.out" 2>/dev/null | tail -1)" = "TAG g${k} OK" ] && GOK=$((GOK+1))
done
chk "优雅关闭 264/264（> 256 上限 ⇒ 槽位必须被及时回收）" "$GOK" "264"
chk "此段服务端**从未**报「槽位已满」" "$(grep -c '连接槽位已满' "$SRV_LOG")" "0"

# ============================================================
echo
echo "[6] 动态行为：**静默退出** 串行 264 次 ⇒ 首次失败 @256（核心判据）"
EXFUT=0
FF=""
for k in $(seq 264); do
  M295_HTTP_PORT="$PORT" M295_TAG="a${k}" M295_CLOSE=0 timeout -k 5 20 "$BD/slot_cli" >"$TMPL/logs/a$k.out" 2>&1
  L="$(grep -E '^TAG ' "$TMPL/logs/a$k.out" 2>/dev/null | tail -1)"
  case "$L" in "TAG a${k} OK") ;; *) [ -z "$FF" ] && FF="$k" ;; esac
done
# 上限 N 个槽 ⇒ **第 N+1 次**才失败；留 3 的容差吸收「上一条连接尚未回收完」的时序差。
chk_range "首次失败 ∈ [256,259]（上限已从 64 抬到 256）" "$FF" 256 259
FULL_LINE="$(grep -c '连接槽位已满' "$SRV_LOG")"
chk_ge "服务端**响亮报告**了「槽位已满」（不再静默）" "$(num "$FULL_LINE")" 1
chk "报告是**限流**的（不刷屏：总计 ≤2 行）" \
    "$([ "$(num "$FULL_LINE")" -le 2 ] && echo 1 || echo 0)" "1"
chk "日志无 DRAINING 刷屏" "$(grep -c 'ERR_DRAINING' "$SRV_LOG")" "0"

# ============================================================
echo
echo "[7] 负控（各自独立判红）"
if [ "$NEG" != "1" ]; then
  echo "  ⏭ --neg-skip：跳过（CI 用）"
else
  RQ_SRC="runtime/runtime_quic.c"
  H3_SRC="runtime/runtime_h3.c"
  QD_SRC="runtime/runtime_h3_qpack_dyn.c"
  RH_SRC="runtime/runtime.h"
  snap() { cp -f "$1" "$TMPL/$(basename "$1").snap"; }
  restore_all() { for f in "$RQ_SRC" "$H3_SRC" "$QD_SRC" "$RH_SRC"; do
      [ -f "$TMPL/$(basename "$f").snap" ] && cp -f "$TMPL/$(basename "$f").snap" "$f"; done; }
  snapshot_all() { for f in "$RQ_SRC" "$H3_SRC" "$QD_SRC" "$RH_SRC"; do snap "$f"; done; }

  # ---------- NC-A：把单一真相改回 64 ⇒ 静态判据必红 ----------
  snapshot_all
  sed -i 's|^#define PX_QUIC_CONN_MAX 256|#define PX_QUIC_CONN_MAX 64|' "$RH_SRC"
  strip_c runtime/runtime.h > "$TMPL/nc_a.h"
  A_RED=0
  grep -qE '^#define[[:space:]]+PX_QUIC_CONN_MAX[[:space:]]+256' "$TMPL/nc_a.h" || A_RED=1
  chk "NC-A：上限改回 64 ⇒ 静态判据判红" "$A_RED" "1"
  restore_all

  # ---------- NC-B：CCERR 退回整数形态 ⇒ 静态判据必红（防 SEGV 回退）----------
  sed -i 's|&ccerr, quic_now());|(uint64_t)NGTCP2_NO_ERROR, quic_now());|' "$RQ_SRC"
  strip_c runtime/runtime_quic.c > "$TMPL/nc_b.c"
  B_RED=0
  grep -q '(uint64_t)NGTCP2_NO_ERROR' "$TMPL/nc_b.c" && B_RED=1
  chk "NC-B：CCERR 退回整数 ⇒ 静态判据判红（防 SIGSEGV 回退）" "$B_RED" "1"
  restore_all

  # ---------- NC-C：判据自伤 —— 源码逐字节还原后，NC-A 的红必须消失 ----------
  C_CLEAN=0
  for f in "$RQ_SRC" "$H3_SRC" "$QD_SRC" "$RH_SRC"; do
    cmp -s "$f" "$TMPL/$(basename "$f").snap" && C_CLEAN=$((C_CLEAN+1))
  done
  chk "NC-C：还原保真（4 文件逐字节相同）" "$C_CLEAN" "4"
  strip_c runtime/runtime.h > "$TMPL/nc_c.h"
  chk "NC-C：还原后静态判据不再红（判据自伤）" \
      "$(grep -cE '^#define[[:space:]]+PX_QUIC_CONN_MAX[[:space:]]+256' "$TMPL/nc_c.h")" "1"
fi

# ============================================================
echo
echo "[8] 覆盖边界（如实登记）"
cat <<'EOF'
  · 本门只覆盖 **H3 托管 listener**（px_serve + opts.http3）这条形态；裸 `quic_listen`
    + `quic_accept` 的 demo 路径共用同一套槽位表（`g_qconns`），但独立路径未单独覆门。
  · 「首次失败 @256」依赖**连接槽**才是瓶颈：本门用 `M295_CLOSE=0`（静默退出）制造
    「槽位不释放」；优雅关闭场景由 [5] 覆盖（全 OK，且不再依赖上限）。
  · 上限值 256 的**内存代价**已量化（`nm -S --size-sort`：g_qds 由静态 64MB 改为按需分配，
    进程 BSS 76.5MB → 9.4MB）—— 该量测在门里**不复现**（需要 nm 与固定构建），只在文档登记。
  · `QUIC_CIDTAB_MAX`（2048）**不是**本轮的瓶颈（实测把 conn 上限抬高后仍是 H3/QD 的 64 先卡住），
    它是**同族**的潜在瓶颈，一并按比例扩容 + 满时响亮，但**没有**单独的行为判据。
  · 服务端的 fail 计数（`h3_setup_fail` / `cid_drop`）在正常档恒为 0 ⇒ 无正向判据，只作诊断。
EOF

echo
printf '===== M295-VERIFY-OK · 通过 %s 项 · 失败 %s 项 =====\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
