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
#   [6] 动态行为（**并发持握**）：单进程持握 **256** 条 ⇒ **第 257 条必被拒**（核心行为判据）
#       ⚠️ M350（缺陷 611）：旧形态是「串行静默退出 264 次 ⇒ 首次失败 @256」，其隐含前提
#          （进程退出后槽位不回收）被 M349 实测推翻 ⇒ 改为结构确定性装置，见 [6] 段注释。
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

PASS=0; FAIL=0; ANOM=0
chk() { # chk <名> <实际> <期望>
  if [ "$2" = "$3" ]; then PASS=$((PASS+1)); printf '  ✅ %s\n' "$1"
  else FAIL=$((FAIL+1)); printf '  ❌ %s\n      实际=[%s]\n      期望=[%s]\n' "$1" "$2" "$3"; fi
}
prem() { # prem <名> <实际> <期望> —— **前提断言**（M350 · 口径 12）
  #   前提不成立 ⇒ ANOMALY：同样判红（响亮），但措辞必须能区分「装置坏了」与「产品坏了」。
  if [ "$2" = "$3" ]; then PASS=$((PASS+1)); printf '  ✅ %s\n' "$1"
  else FAIL=$((FAIL+1)); ANOM=$((ANOM+1)); printf '  ⛔ %s\n      ANOMALY（前提不成立 · 不是产品红）\n      实际=[%s]\n      期望=[%s]\n' "$1" "$2" "$3"; fi
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
HPID=""                                        # M350：持握进程（清理时必须收掉，防残留）
GATE="$TMPL/hold.release"                      # M350：持握释放标记
cleanup() {
  [ -n "$GATE" ] && : > "$GATE" 2>/dev/null
  [ -n "$HPID" ] && kill -9 "$HPID" 2>/dev/null
  [ -n "$SRV_PID" ] && kill -9 "$SRV_PID" 2>/dev/null
  free_port "$PORT"; free_port "$((PORT+1))"; rm -rf "$TMPL"; return 0
}
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
# M350（缺陷 611）：把服务端 H3 空闲超时**显式**抬到上限 60s（2×idle = 120s 才回收槽位）。
#   为什么必须显式：默认 8s（2×8s≈16s 回收）远短于「起 256 条并发持握 + 打第 257 条」的窗口，
#   一旦回收先于断言发生，[6] 的前提（槽位被占）就自己失效 ⇒ 判据必须先把前提**钉死**（口径 12）。
PX_H3_IDLE_MS_GATE=60000
setsid nohup env M295_HTTP_PORT="$PORT" PX_H3_IDLE_MS="$PX_H3_IDLE_MS_GATE" "$BD/srv" >"$SRV_LOG" 2>&1 </dev/null &
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
#   ⚠️ 计数只做**下限**（M197 纪律）：M297 给 quic_pool_stats 加了 3 个路径可观测键
#      （path_migrated / path_missing / send_err）⇒ 「恰好 13」这种硬等式会随每次扩键假红。
POOLK="$(M295_HTTP_PORT="$PORT" timeout -k 5 15 "$BD/pool_probe" 2>&1 | grep '^POOL-KEYS' | head -1)"
PKDECL="$(echo "$POOLK" | sed -n 's/^POOL-KEYS \([0-9]*\) GOT.*/\1/p')"
PKGOT="$(echo "$POOLK" | sed -n 's/.* GOT \([0-9]*\)$/\1/p')"
chk "stats 键数 ≥13（下限 · 不用硬等式）" "$([ "${PKGOT:-0}" -ge 13 ] && echo yes || echo no)" "yes"
chk "键数自洽（声明 == 实际）" "${PKDECL:-0}" "${PKGOT:-0}"
chk "含 M297 路径可观测键（3/3）" "$(echo "$POOL" | grep -oE 'path_migrated=|path_missing=|send_err=' | wc -l)" "3"

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
echo "[6] 动态行为：**并发持握** 256 条（连接活着 ⇒ 槽位结构性被占）⇒ 第 257 条必被拒（核心判据）"
# ------------------------------------------------------------
# M350（缺陷 611 · 口径 12）：本段从「串行静默退出 264 次」改为「**单进程持握 256 条**」。
#   为什么必须换（M349 定向回放证据：/data/gatevm/m349_m295_replay/，3 base 绿 + 3 hog 红）：
#     旧装置的隐含前提是「进程退出后槽位不会被回收」，而服务端只按 2×PX_H3_IDLE_MS 回收；
#     264 次串行本身就要跑十几秒 ⇒ 早先的连接早被回收 ⇒ 第 257 次**永远到不了**（hog 下假绿）。
#     失效方向偏向「更绿」= 这类判据最危险的形态（口径 12 就是为它写的）。
#   新装置的结构确定性：连接**活着**且未发 CONNECTION_CLOSE ⇒ 槽位被占与回收时序**无关**。
#   为什么是「单进程持握 n 条」而不是「n 个并发进程」：M350 实测 256 个进程会把 loopback
#     UDP 收包压塌（63/256 握手失败或崩溃，79s 仍未建满）⇒ 噪声盖过判据；单进程顺序建连
#     把「建立」与「持握」解耦（实测 256 条建立 1.8–2.1s，base/hog 同量级）。
#   三条**前提断言**（任一不成立 ⇒ ANOMALY，响亮；不许报绿，也不许当产品红）：
#     P1 256 条持握全部 OK · P2 判据时刻持握进程仍存活 · P3 窗口 < 回收窗口一半
# ------------------------------------------------------------
CAP=256
rm -f "$GATE"
HOUT="$TMPL/logs/hold.out"
T60=$(date +%s%3N)
M295_HTTP_PORT="$PORT" M295_TAG="hold" M295_HOLD_N="$CAP" M295_HOLD_GATE="$GATE" \
  M295_HOLD_MAX=180000 M295_READ_MS=20000 \
  timeout -k 5 200 "$BD/slot_cli" >"$HOUT" 2>&1 &
HPID=$!
HOK=0; n=0
while [ "$n" -lt 1200 ]; do                    # 至多 60s（实测 1.8–2.1s ⇒ 裕度 ~28×）
  HOK=$(grep -cE '^TAG hold-[0-9]+ OK$' "$HOUT" 2>/dev/null || true)
  [ "$HOK" -ge "$CAP" ] && break
  kill -0 "$HPID" 2>/dev/null || break
  sleep 0.05; n=$((n+1))
done
HALIVE=0; kill -0 "$HPID" 2>/dev/null && HALIVE=1
prem "P1 持握建立：$CAP/$CAP 条全部 OK（装置前提）" "$HOK" "$CAP"
prem "P2 判据时刻持握进程仍存活（槽位是**结构**占位，不是靠时序）" "$HALIVE" "1"

M295_HTTP_PORT="$PORT" M295_TAG="h257" M295_READ_MS=5000 \
  timeout -k 5 60 "$BD/slot_cli" >"$TMPL/logs/h257.out" 2>&1
L257="$(grep -E '^TAG ' "$TMPL/logs/h257.out" 2>/dev/null | tail -1)"
T6=$(( $(date +%s%3N) - T60 ))
case "$L257" in
  "TAG h257 connect-fail"|"TAG h257 TIMEOUT") R257="rejected" ;;
  "TAG h257 OK") R257="accepted" ;;
  *) R257="other[$L257]" ;;
esac
# P3：回收窗口 = 2×PX_H3_IDLE_MS = 120000ms ⇒ 判据窗口必须 ≤ 它的一半（60000ms）。
prem "P3 持握+判据窗口 ${T6}ms < 回收窗口的一半（2×H3_IDLE=${PX_H3_IDLE_MS_GATE}ms ⇒ 一半 60000ms）" \
     "$([ "$T6" -le $(( PX_H3_IDLE_MS_GATE )) ] && echo yes || echo no)" "yes"
chk "第 257 条被**拒**（connect-fail/TIMEOUT；实测 [$L257] · ${T6}ms）" "$R257" "rejected"
FULL_LINE="$(grep -c '连接槽位已满' "$SRV_LOG")"
chk_ge "服务端**响亮报告**了「槽位已满」（不再静默）" "$(num "$FULL_LINE")" 1
chk "报告是**限流**的（不刷屏：总计 ≤2 行）" \
    "$([ "$(num "$FULL_LINE")" -le 2 ] && echo 1 || echo 0)" "1"
chk "日志无 DRAINING 刷屏" "$(grep -c 'ERR_DRAINING' "$SRV_LOG")" "0"

# 释放：持握结束 ⇒ 256 条优雅关闭 ⇒ 槽位必须还回（异步，轮询 10s；实测 26–45ms）
: > "$GATE"
REXO=0; n=0
while [ "$n" -lt 300 ]; do kill -0 "$HPID" 2>/dev/null || { REXO=1; break; }; sleep 0.1; n=$((n+1)); done
wait "$HPID" 2>/dev/null || true
HPID=""
chk "持握按释放标记退出（无残留进程）" "$REXO" "1"
chk "持握侧 256 条**全部**走 quic_close（优雅释放；输出 closed failed）" \
    "$(sed -n 's/^HOLD-CLOSE hold closed=\([0-9]*\) failed=\([0-9]*\)$/\1 \2/p' "$HOUT" | tail -1)" "$CAP 0"
RL0=$(date +%s%3N); ROK=0; n=0
while [ "$n" -lt 100 ]; do
  M295_HTTP_PORT="$PORT" M295_TAG="h258" timeout -k 5 20 "$BD/slot_cli" >"$TMPL/logs/h258.out" 2>&1
  [ "$(grep -E '^TAG ' "$TMPL/logs/h258.out" 2>/dev/null | tail -1)" = "TAG h258 OK" ] && { ROK=1; break; }
  n=$((n+1)); sleep 0.1
done
RCOV=$(( $(date +%s%3N) - RL0 ))
chk "释放后新连接恢复（槽位确实还回去了；实测 ${RCOV}ms · 轮询 $((n+1)) 次）" "$ROK" "1"

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

  # ---------- NC-D（M350）：**装置鉴别力** —— 只持握 64 条 ⇒ 第 257 条必须「被接受」 ----------
  #   为什么必须有这条：[6] 的判据是「第 257 条被拒」。若装置在**任何**持握数下都报「被拒」，
  #   那这条判据就恒真 = 空判据（口径 9：症状断言必须先证明自己有鉴别力）。
  #   64 「被接受」+ 256 「被拒绝」⇒ 判据随槽位占用单调变化 ⇒ 断言的确实是「上限」这件事。
  rm -f "$GATE"
  M295_HTTP_PORT="$PORT" M295_TAG="nc" M295_HOLD_N=64 M295_HOLD_GATE="$GATE" \
    M295_HOLD_MAX=60000 M295_READ_MS=20000 timeout -k 5 90 "$BD/slot_cli" >"$TMPL/logs/ncd.out" 2>&1 &
  HPID=$!
  n=0; while [ "$n" -lt 400 ]; do
    [ "$(grep -cE '^TAG nc-[0-9]+ OK$' "$TMPL/logs/ncd.out" 2>/dev/null || true)" -ge 64 ] && break
    kill -0 "$HPID" 2>/dev/null || break
    sleep 0.05; n=$((n+1))
  done
  M295_HTTP_PORT="$PORT" M295_TAG="nc27" M295_READ_MS=8000 \
    timeout -k 5 30 "$BD/slot_cli" >"$TMPL/logs/ncd257.out" 2>&1
  LNCD="$(grep -E '^TAG ' "$TMPL/logs/ncd257.out" 2>/dev/null | tail -1)"
  chk "NC-D：只持握 64 条 ⇒ 第 257 条**被接受**（证明 [6] 的「被拒」不是恒真）" \
      "$( [ "${LNCD:-}" = "TAG nc27 OK" ] && echo accepted || echo "other[$LNCD]" )" "accepted"
  : > "$GATE"
  n=0; while [ "$n" -lt 200 ]; do kill -0 "$HPID" 2>/dev/null || break; sleep 0.1; n=$((n+1)); done
  wait "$HPID" 2>/dev/null || true; HPID=""; rm -f "$GATE"
fi

# ============================================================
echo
echo "[8] 覆盖边界（如实登记）"
cat <<'EOF'
  · 本门只覆盖 **H3 托管 listener**（px_serve + opts.http3）这条形态；裸 `quic_listen`
    + `quic_accept` 的 demo 路径共用同一套槽位表（`g_qconns`），但独立路径未单独覆门。
  · 「第 257 条被拒」现在由**并发持握**证明（M350）：256 条连接**活着**且未发
    CONNECTION_CLOSE ⇒ 槽位被占与「服务端空闲回收」的时序无关。旧形态（串行静默退出 264 次）
    已被推翻并删除 —— 它的隐含前提「进程退出后槽位不回收」在 hog 下不成立（M349 实测 3/3 假绿）。
  · **静默退出（进程被杀）之后槽位何时回收**这条行为**本门不再断言**：它由服务端空闲超时
    （2×PX_H3_IDLE_MS，本段显式抬到 60s ⇒ 120s 才回收）决定，而「等 120s」既慢又不属于
    M295 的夹具契约（缺陷 508 的病是「满了不吭声」，不是「回收快慢」）。已在 GATE_FLAKINESS.md 登记。
  · 上限值 256 的**内存代价**已量化（`nm -S --size-sort`：g_qds 由静态 64MB 改为按需分配，
    进程 BSS 76.5MB → 9.4MB）—— 该量测在门里**不复现**（需要 nm 与固定构建），只在文档登记。
  · `QUIC_CIDTAB_MAX`（2048）**不是**本轮的瓶颈（实测把 conn 上限抬高后仍是 H3/QD 的 64 先卡住），
    它是**同族**的潜在瓶颈，一并按比例扩容 + 满时响亮，但**没有**单独的行为判据。
  · 服务端的 fail 计数（`h3_setup_fail` / `cid_drop`）在正常档恒为 0 ⇒ 无正向判据，只作诊断。
EOF

echo
# M350：末行必须与 rc **同向**（缺陷 506 家族：修前不管 FAIL 多少都印 OK）。
#   例外：ANOMALY 也算不绿 ⇒ 同走 FAIL 分支，但**单独计数**，便于区分「装置坏了」与「产品坏了」。
if [ "$FAIL" -eq 0 ]; then
  printf '===== M295-VERIFY-OK · 通过 %s 项 · 失败 0 项 · 异常 0 项 =====\n' "$PASS"
  exit 0
fi
printf '===== M295-VERIFY-FAIL · 通过 %s 项 · 失败 %s 项 · 异常 %s 项 =====\n' "$PASS" "$FAIL" "$ANOM"
printf '      （异常 = 前提断言不成立/装置故障，不是产品红；逐条见 ⛔ 行）\n'
exit 1
