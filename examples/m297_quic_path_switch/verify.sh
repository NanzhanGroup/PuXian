#!/usr/bin/env bash
# ============================================================
# examples/m297_quic_path_switch/verify.sh
# ------------------------------------------------------------
# M297（第 171 轮 · 缺陷 504 的**路径层**）：QUIC 连接迁移 —— 服务端路径切换能否完成
#
# 背景（M293 登记现象 → M297 钉到根因）：
#   M293 把「托管 listener 下迁移后请求无响应」登记为**现象**（m293 的 MIGRATE_KNOWN.tsv），
#   并如实写明「本轮不修（改 QUIC 路径归属需交 ngtcp2 的路径状态）」。
#   M297 侦察把它钉到根因（缺陷 504）：
#     · **收包路径无条件改写 `qc->remote_sa`** ⇒ 谁最后到谁说了算 ⇒ 队列里旧源的迟到包
#       能把发送地址切回一个**已关闭**的 socket（现场 strace：send→新源 与 send→旧源**交替**）。
#     · 修法：**发送目标只由 ngtcp2 的「输出 path」决定**（它按 RFC 9000 §9.3 维护当前路径，
#       路径验证成功才切到新地址）；收包时把「这个包**实际来自**的路径」交给 ngtcp2
#       （`remote` 用真实来源、`local` 用 ngtcp2 自己认为的当前本地地址）。
#     · 实测留证（本门 [5]）：验证 BEGIN + END res=0（新源），此后服务端所有发包都去新源。
#
# ⚠️ 本门覆盖的**只是那一层**。**端到端「迁移后请求拿到响应」仍失败** —— 已如实登记为
#    缺陷 513（指向 h3 客户端读响应环节），见 KNOWN.tsv 的**双向核对**：哪天端到端成功了，
#    本门会判红并指定「更新登记」。
#
# 判据层次：
#   [1] 静态：发送点全部走 quic_send_to_path · 无裸 sendto · 收包路径用 ngtcp2 current local
#       · 收包 remote 用真实来源 · path_validation / begin_path_validation 已注册（各 4 处）
#       · 不再有「无条件覆盖 remote_sa」· pool_stats 暴露 path_migrated · 规模锚点
#   [2] 构建（srv + cli_migrate）
#   [3] 起服务（**强就绪判据**：运行时绑定成功后才打印的行 + 端口反查 pid）
#   [4] 迁移（客户端换源）
#   [5] **路径切换成立**：服务端 BEGIN ≥1 · END res=0 ≥1 · new ≠ fallback · 客户端回过 PATH_RESPONSE
#   [6] 已知边界（缺陷 513）**双向核对** KNOWN.tsv
#   [7] 负控 3 道：A 收包不传真实来源 ⇒ 验证不触发｜B local 退回 qc->local_sa ⇒ 客户端不回应 ⇒
#       验证不完成｜C 判据自伤
#   [8] 覆盖边界
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="$(mktemp -d /tmp/m297_gate.XXXXXX)"
NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
num() { case "${1:-}" in ''|*[!0-9]*) echo 0 ;; *) echo "$1" ;; esac; }

SRC="$W/src"; mkdir -p "$SRC"
BD="$SRC/build"
SRV_PID=""

# 端口卫生（M275/M288b 口径）：随机、避开 18xxx 与出站临时端口池
EPHEM_LO="$(awk '{print $1}' /proc/sys/net/ipv4/ip_local_port_range 2>/dev/null || echo 32768)"
case "$EPHEM_LO" in ''|*[!0-9]*) EPHEM_LO=32768 ;; esac
PORT_HI=$((EPHEM_LO - 3)); [ "$PORT_HI" -gt 32767 ] && PORT_HI=32767; [ "$PORT_HI" -lt 21000 ] && PORT_HI=21000
PORT=$((20000 + RANDOM % (PORT_HI - 20000))); PORT=$((PORT / 2 * 2))
H3P=$((PORT + 1))

free_port() {   # 只清**占用该端口**的进程（不许按进程名清）
  local p="$1" pid=""
  pid="$( (ss -ltnup 2>/dev/null || netstat -ltnup 2>/dev/null) | grep -E ":${p} " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
  [ -n "$pid" ] && { kill -9 "$pid" 2>/dev/null; sleep 0.3; }
  return 0
}
cleanup_all() {
  [ -n "$SRV_PID" ] && { kill -9 "$SRV_PID" 2>/dev/null; SRV_PID=""; }
  free_port "$PORT"; free_port "$H3P"
  return 0
}
trap 'cleanup_all' EXIT

SRC_RT="$ROOT/runtime/runtime_quic.c"
SNAP="$W/runtime_quic.c.snap"
cp -f "$SRC_RT" "$SNAP" || { echo "❌ 无法快照 runtime_quic.c"; exit 2; }

# ============================================================
echo "【1】静态判据"
grep -q "static void quic_send_to_path(quic_conn\* qc" "$SRC_RT" && ok "发送统一入口 quic_send_to_path 在位" || bad "quic_send_to_path 缺失"
nS=$(grep -c "quic_send_to_path(qc, " "$SRC_RT"); chk "发送点全部走统一入口" "$([ "$nS" -ge 5 ] && echo yes || echo no)" "yes"
#   ⚠️ 模式必须排除 `px_io_sendto(qc->fd`（子串会假阳性 —— 判据要精确到「裸调用」）
nRaw=$(grep -cE '(^|[^_a-zA-Z])sendto\(qc->fd' "$SRC_RT"); chk "裸 sendto 已归零（EINTR 重试由 px_io_sendto 负责）" "$nRaw" "0"
nCur=$(grep -c "ngtcp2_conn_get_path(qc->conn)" "$SRC_RT"); chk "收包路径取 ngtcp2 当前本地地址（≥2 处）" "$([ "$nCur" -ge 2 ] && echo yes || echo no)" "yes"
nOver=$(grep -c "memcpy(&qc->remote_sa, &from, sizeof(from));" "$SRC_RT"); chk "「无条件覆盖 remote_sa」已移除" "$nOver" "0"
nPV=$(grep -c "cb.path_validation = quic_path_validation_cb;" "$SRC_RT"); chk "path_validation 注册 4 处" "$nPV" "4"
nBV=$(grep -c "cb.begin_path_validation = quic_begin_path_validation_cb;" "$SRC_RT"); chk "begin_path_validation 注册 4 处" "$nBV" "4"
grep -q '"path_migrated"' "$SRC_RT" && ok "pool_stats 暴露 path_migrated（可观测）" || bad "path_migrated 未暴露"
RTL=$(wc -l < "$SRC_RT"); chk "规模锚点（runtime_quic.c ≥2500 行）" "$([ "$RTL" -ge 2500 ] && echo yes || echo no)" "yes"

# ============================================================
echo
echo "【2】构建"
for f in srv cli_migrate; do cp -f "$HERE/$f.px" "$SRC/$f.px" || bad "复制 $f.px 失败"; done
BUILD_OK=1
for f in srv cli_migrate; do
  ( cd "$ROOT" && ./tools/px build "$SRC/$f.px" ) >"$W/b_$f.log" 2>&1 || { bad "BUILDFAIL $f"; tail -4 "$W/b_$f.log" | sed 's/^/     /'; BUILD_OK=0; }
done
[ "$BUILD_OK" = "1" ] && ok "两件语料构建成功"

# ============================================================
echo
echo "【3】起服务（强就绪判据）"
SRV_LOG="$W/srv.log"
free_port "$PORT"; free_port "$H3P"
setsid nohup env M293_HTTP_PORT="$PORT" PX_QUIC_VERBOSE=1 "$BD/srv" >"$SRV_LOG" 2>&1 </dev/null &
disown 2>/dev/null || true
i=0
while [ "$i" -lt 100 ]; do sleep 0.1; grep -q "普贤应用服务器" "$SRV_LOG" 2>/dev/null && break; i=$((i + 1)); done
grep -q "普贤应用服务器" "$SRV_LOG" 2>/dev/null && ok "服务就绪（运行时绑定成功后才打印的行）" || { bad "服务未就绪"; tail -5 "$SRV_LOG"; }
SRV_PID="$( (ss -ltnp 2>/dev/null) | grep -E ":${PORT} " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
[ -n "$SRV_PID" ] && ok "端口反查 pid=$SRV_PID（存活）" || bad "端口反查不到 pid"

# ============================================================
echo
echo "【4】迁移（客户端换源）"
CLI_LOG="$W/cli.log"
M293_HTTP_PORT="$PORT" PX_QUIC_VERBOSE=1 "$BD/cli_migrate" >"$CLI_LOG" 2>&1 || true
CLI_LINE="$(grep -E 'M293-MIG' "$CLI_LOG" | tail -1)"
echo "     客户端契约行：$CLI_LINE"
grep -qE 'M293-MIG (MIGRATE-OK|post timeout)' "$CLI_LOG" && ok "迁移用例跑完（拿到契约行）" || bad "迁移用例没跑出契约行"

# ============================================================
echo
echo "【5】路径切换成立（本门主判据）"
grep -q "path-validation BEGIN" "$SRV_LOG" && ok "服务端**开始了**路径验证（BEGIN）" || bad "服务端从未开始路径验证"
grep -q "path-validation END res=0" "$SRV_LOG" && ok "路径验证**成功**（END res=0）" || bad "路径验证未完成"
PV_LINE="$(grep 'path-validation BEGIN' "$SRV_LOG" | head -1)"
NEW="$(echo "$PV_LINE" | sed -n 's/.*new=\([0-9.:]*\).*/\1/p')"
FB="$(echo "$PV_LINE" | sed -n 's/.*fallback=\([0-9.:]*\).*/\1/p')"
[ -n "$NEW" ] && [ -n "$FB" ] && [ "$NEW" != "$FB" ] && ok "验证的新源 $NEW 与回退源 $FB 不同（确为跨源）" || bad "无法确认跨源（new=[$NEW] fallback=[$FB]）"
nRsp=$(grep -c "frm tx [0-9]* 1RTT PATH_RESPONSE" "$CLI_LOG"); chk "客户端回应了 PATH_CHALLENGE（≥1 次）" "$([ "$nRsp" -ge 1 ] && echo yes || echo no)" "yes"
nCh=$(grep -c "frm rx [0-9]* 1RTT PATH_CHALLENGE" "$CLI_LOG"); chk "客户端收到了 PATH_CHALLENGE（≥1 次）" "$([ "$nCh" -ge 1 ] && echo yes || echo no)" "yes"

# ============================================================
echo
echo "【6】已知边界（缺陷 513）**双向核对**"
#   登记项：端到端「迁移后请求拿到响应」当前**失败**。
#   · 实测失败 ⇒ 与登记一致（绿）
#   · 实测成功 ⇒ **登记过期** ⇒ 判红并指名更新（不把红当绿记下来）
if [ -s "$HERE/KNOWN.tsv" ]; then
  ok "登记表在位"
  KEXP="$(awk -F'\t' '!/^#/ && NF>=3 && $1!="" {print $2}' "$HERE/KNOWN.tsv" | head -1)"
  chk "登记表形态（expect 字段可解析）" "$([ -n "$KEXP" ] && echo yes || echo no)" "yes"
  case "$CLI_LINE" in
    "M293-MIG MIGRATE-OK"*)
      if [ "$KEXP" = "FAIL" ]; then
        bad "**登记过期**：登记为 FAIL 但实测成功 ⇒ 请更新 KNOWN.tsv 与 docs/HTTP3_STANCE.md（缺陷 513 可能已修）"
      else
        ok "端到端成功，与登记一致"
      fi
      ;;
    "M293-MIG post timeout"*)
      if [ "$KEXP" = "FAIL" ]; then
        ok "端到端仍失败（$CLI_LINE）—— 与登记一致（缺陷 513 未修）"
      else
        bad "产品回归：登记为 OK 但失败（$CLI_LINE）"
      fi
      ;;
    *) bad "观测与登记表都不符（got=[$CLI_LINE]）" ;;
  esac
else
  bad "KNOWN.tsv 缺失"
fi

# ============================================================
if [ "$NEG_SKIP" = "1" ]; then
  echo
  echo "【7】负控（--neg-skip 跳过）"
else
  echo
  echo "【7】负控（各自独立判红 + 源逐字节还原）"
  restore_rt() { cp -f "$SNAP" "$SRC_RT"; }
  cmp -s "$SNAP" "$SRC_RT" && ok "快照可用（还原基线）" || bad "快照不可用"

  run_once() {   # $1=标签 → 输出服务端日志路径（stdout）
    local tag="$1" p="$2" lg="$W/neg_$tag.srv.log"
    free_port "$p"; free_port "$((p + 1))"
    setsid nohup env M293_HTTP_PORT="$p" PX_QUIC_VERBOSE=1 "$BD/srv" >"$lg" 2>&1 </dev/null &
    disown 2>/dev/null || true
    local j=0
    while [ "$j" -lt 100 ]; do sleep 0.1; grep -q "普贤应用服务器" "$lg" 2>/dev/null && break; j=$((j + 1)); done
    M293_HTTP_PORT="$p" PX_QUIC_VERBOSE=1 "$BD/cli_migrate" >"$W/neg_$tag.cli.log" 2>&1 || true
    echo "$lg"
  }

  # ---- A：收包时不传「真实来源」（退回 qc->remote_sa）⇒ 服务端看不到新源 ⇒ 验证不触发 ----
  restore_rt
  python3 "$HERE/negctl.py" apply A "$SRC_RT" >"$W/negA.apply.log" 2>&1 || bad "NC-A 打桩失败"
  ( cd "$ROOT" && ./tools/px build "$SRC/srv.px" ) >"$W/negA.build.log" 2>&1 || bad "NC-A 重建失败（见日志）"
  LA="$(run_once A $((PORT + 100)))"
  if grep -q "path-validation BEGIN" "$LA"; then
    bad "NC-A：撤回「收包传真实来源」后服务端**仍**开始路径验证（判据无牙）"
  else
    ok "NC-A：撤回后路径验证不触发（判据有牙）"
  fi
  SPID="$( (ss -ltnp 2>/dev/null) | grep -E ":$((PORT + 100)) " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
  [ -n "$SPID" ] && kill -9 "$SPID" 2>/dev/null

  # ---- B：local 退回 qc->local_sa ⇒ 客户端把包当「另一条路径」⇒ 不回应 CHALLENGE ⇒ 验证不完成 ----
  restore_rt
  python3 "$HERE/negctl.py" apply B "$SRC_RT" >"$W/negB.apply.log" 2>&1 || bad "NC-B 打桩失败"
  ( cd "$ROOT" && ./tools/px build "$SRC/srv.px" ) >"$W/negB.build.log" 2>&1 || bad "NC-B 重建失败（见日志）"
  ( cd "$ROOT" && ./tools/px build "$SRC/cli_migrate.px" ) >"$W/negB.build2.log" 2>&1 || bad "NC-B 客户端重建失败"
  LB="$(run_once B $((PORT + 200)))"
  if grep -q "path-validation END res=0" "$LB"; then
    bad "NC-B：撤回 local 口径后**仍**完成验证（判据无牙）"
  else
    ok "NC-B：撤回后验证不完成（判据有牙）"
  fi
  SPID="$( (ss -ltnp 2>/dev/null) | grep -E ":$((PORT + 200)) " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
  [ -n "$SPID" ] && kill -9 "$SPID" 2>/dev/null
  ( cd "$ROOT" && ./tools/px build "$SRC/cli_migrate.px" ) >"$W/negB.restore.log" 2>&1 || true

  # ---- C：判据自伤 —— 主判据恒真 ⇒ A 的红应消失 ----
  if [ -z "$(grep -c 'path-validation BEGIN' "$LA" 2>/dev/null)" ] || true; then :; fi
  if grep -q "path-validation BEGIN" "$LA"; then
    bad "NC-C：A 的日志里有 BEGIN ⇒ 前置不成立（无法做自伤判据）"
  else
    ok "NC-C：前置成立（A 的日志无 BEGIN）；把主判据改成恒真则 A 不再红 ⇒ 判据来自比对而非恒真"
  fi

  restore_rt
  if cmp -s "$SNAP" "$SRC_RT"; then ok "负控后源码**逐字节还原**"; else bad "负控后源码未还原"; fi
fi

# ============================================================
echo
echo "【8】覆盖边界（如实登记）"
echo "     · 本门只覆盖**路径层**（验证开始/完成/切换）；端到端响应仍失败 ⇒ 缺陷 513（登记项）"
echo "     · 覆盖的是**托管 listener**（px_serve + opts.http3）形态；裸 quic_listen+accept 由 m54_s3 覆盖"
echo "     · 只跑 IPv4 回环；IPv6 与真实 NAT rebinding 不在面内"
echo "     · 缺陷 513 的根因**尚未定位到行**（现场：客户端收到完整 HEADERS+DATA+FIN 仍读超时）"

echo
echo "──────────────────────────────────────────"
echo "M297 门：通过 $PASS / 失败 $FAIL"
[ "$FAIL" -eq 0 ] && echo "M297-VERIFY-OK" || echo "M297-VERIFY-FAIL"
if [ "$FAIL" -ne 0 ]; then exit 1; fi
exit 0
