#!/usr/bin/env bash
# ============================================================
# examples/m298_quic_write_wait/verify.sh
# ------------------------------------------------------------
# M298（第 172 轮 · 缺陷 514）：**写流数据「等到真的写进去」或「响亮失败」**
#
# 来历（M297 登记缺陷 513 → 本轮钉到行）：
#   M297 把「迁移后请求拿不到响应」登记为缺陷 513，并**如实写明根因尚未定位到行**，
#   当时的假设是「h3 客户端读响应环节（疑似 QPACK 编码流的等待）」。**本轮证明该假设是错的** ——
#   客户端等 8 秒超时只是"结果"，真因在**服务端的发送链**：
#     `quic_write_stream_bytes()` 的写循环 `while (woff < len && spins < 16)` **无进展时不等待、
#     也不推进本连接** ⇒ 16 次空转在**微秒**内跑完（等价于只试了一次），然后 `return total` = 0，
#     而调用方 `h3_send_fields` 的 `sent >= 0` 把它当成**成功** ⇒ **静默丢掉响应**。
#   为什么 ngtcp2 会拒绝接受数据：**新路径尚未验证完成的窗口**里它不接受应用数据
#     （ngtcp2.h:5346「returns 0 if … packet is congestion limited」）；
#     但实测那一刻 `cwnd_left=12402`（cwnd 14028 / 在途 1626）⇒ **不是拥塞窗口，是路径未就绪**。
#     更要命的是自锁：完成验证所需的 PATH_RESPONSE 正躺在收包队列里，
#     **不 pump 就永远验证不完** ⇒ 越等越死。
#
# 修法：① 循环上界由「重试 16 次」改为 **deadline（QUIC_WRITE_STALL_MS=200）**；
#       ② 无进展时必须**推进本连接**（`quic_pump(qc, 20, 1, sid)` 收包 → 喂 ngtcp2，防忙等）；
#       ③ 一字节都没写进去 ⇒ **返回 -1**（不再假装成功）。
#
# A/B（同一套探针，唯一变量 = 这一处：各 6 次）：修前 OK=0/FAIL=6 · 修后 **OK=6/FAIL=0**
#
# ⚠️ 如实登记一条**否证**：我一度按 ngtcp2 文档补 `ngtcp2_conn_update_pkt_tx_time`
#    （文档原话「must be called after … ngtcp2_conn_writev_stream」）—— 结果**更糟**：
#    连迁移**之前**的请求都超时（pacing 把所有包都挡住）⇒ 已撤销、不作为修法。
#    它的价值是反证「pacing 通道是活的」，从而排除 pacing 作为本缺陷的解释。
#
# 判据层次：
#   [1] 静态（含打桩器自证）· [2] 构建 · [3] 强就绪 · [4] 动态主判据（3 轮全 MIGRATE-OK）
#   [5] **产品自带输出**的产品侧证据（服务端真的在**第二条** bidi 流上发出了响应）
#   [6] 登记表双向核对 · [7] 负控 3 道（A 忠实换回原文 / B 只等不推进 / C 判据自伤）
#   [8] 覆盖边界
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="$(mktemp -d /tmp/m298_gate.XXXXXX)"
NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
num_or0() { case "${1:-}" in ''|*[!0-9]*) echo 0 ;; *) echo "$1" ;; esac; }

SRC="$W/src"; mkdir -p "$SRC"
BD="$SRC/build"
SRV_PID=""

# 端口卫生（M275/M288b 口径）：随机、避开 18xxx 与出站临时端口池
EPHEM_LO="$(awk '{print $1}' /proc/sys/net/ipv4/ip_local_port_range 2>/dev/null || echo 32768)"
case "$EPHEM_LO" in ''|*[!0-9]*) EPHEM_LO=32768 ;; esac
PORT_HI=$((EPHEM_LO - 3)); [ "$PORT_HI" -gt 32767 ] && PORT_HI=32767; [ "$PORT_HI" -lt 21000 ] && PORT_HI=21000
PORT=$((20000 + RANDOM % (PORT_HI - 20000))); PORT=$((PORT / 2 * 2))

KILL_PORTS=""
free_port() {
  local p="$1" pid=""
  pid="$( (ss -ltnup 2>/dev/null || netstat -ltnup 2>/dev/null) | grep -E ":${p} " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
  [ -n "$pid" ] && { kill -9 "$pid" 2>/dev/null; sleep 0.3; }
  return 0
}
cleanup_all() {
  [ -n "$SRV_PID" ] && { kill -9 "$SRV_PID" 2>/dev/null; SRV_PID=""; }
  local p
  for p in $KILL_PORTS; do free_port "$p"; done
  return 0
}
trap 'cleanup_all' EXIT

SRC_RT="$ROOT/runtime/runtime_quic.c"
SNAP="$W/runtime_quic.c.snap"
cp -f "$SRC_RT" "$SNAP" || { echo "❌ 无法快照 runtime_quic.c"; exit 2; }

# ============================================================
echo "【1】静态判据（含打桩器自证）"
python3 "$HERE/negctl.py" --selftest "$SRC_RT" >"$W/negsel.log" 2>&1
if grep -q "SELFTEST-OK" "$W/negsel.log"; then ok "打桩器自证（锚点唯一 + 修前形态消失）"; else bad "打桩器自证失败"; cat "$W/negsel.log" | sed 's/^/     /'; fi

nOld=$(grep -c "spins < 16" "$SRC_RT"); chk "修前形态（无等待的 16 次空转）已消失" "$nOld" "0"
nDL=$(grep -c "quic_now() < deadline" "$SRC_RT"); chk "循环上界为 deadline（≥1 处）" "$([ "$nDL" -ge 1 ] && echo yes || echo no)" "yes"
grep -q "^#define QUIC_WRITE_STALL_MS 200" "$SRC_RT" && ok "等待预算常量 QUIC_WRITE_STALL_MS=200 在位" || bad "缺 QUIC_WRITE_STALL_MS"
nPump=$(grep -c "quic_pump(qc, QUIC_WRITE_PUMP_MS, 1, sid);" "$SRC_RT"); chk "无进展 ⇒ 推进本连接（命中恰 1 处）" "$nPump" "1"
nLoud=$(grep -c "if (total == 0) {" "$SRC_RT"); chk "一字节未写入 ⇒ 响亮分支在位" "$([ "$nLoud" -ge 1 ] && echo yes || echo no)" "yes"
# M298s1：本行判据由「完整行」改为「前缀」—— 该行现在带 PX_IO_EINTR_OK 豁免注释
#        （check_eintr.py 的 A 段要求：裸 usleep 必须显式豁免或重试）。
nBusy=$(grep -cF "if (quic_now() - t0 < (uint64_t)NGTCP2_MILLISECONDS) usleep(1000)" "$SRC_RT"); chk "防忙等（pump 立即返回时让出 1ms）" "$nBusy" "1"
nE1=$(grep -c "ngtcp2_conn_update_pkt_tx_time" "$SRC_RT"); chk "被否证的 E1 方案未混入产品源码" "$nE1" "0"
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

# 起服务 + 跑一轮迁移：$1=http 端口，$2=标签 → 打印契约行，日志落 $W/$2.{srv,cli}.log
run_case() {
  local p="$1" tag="$2" srvlog="$W/$2.srv.log" clilog="$W/$2.cli.log"
  KILL_PORTS="$KILL_PORTS $p $((p + 1))"
  free_port "$p"; free_port "$((p + 1))"
  setsid nohup env M293_HTTP_PORT="$p" PX_QUIC_VERBOSE=1 "$BD/srv" >"$srvlog" 2>&1 </dev/null &
  disown 2>/dev/null || true
  local j=0
  while [ "$j" -lt 120 ]; do sleep 0.1; grep -q "普贤应用服务器" "$srvlog" 2>/dev/null && break; j=$((j + 1)); done
  if ! grep -q "普贤应用服务器" "$srvlog" 2>/dev/null; then echo "SRV-NOT-READY"; return 1; fi
  M293_HTTP_PORT="$p" PX_QUIC_VERBOSE=1 "$BD/cli_migrate" >"$clilog" 2>&1 || true
  local pid
  pid="$( (ss -ltnp 2>/dev/null) | grep -E ":${p} " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"
  [ -n "$pid" ] && kill -9 "$pid" 2>/dev/null
  grep -m1 'M293-MIG' "$clilog"
  return 0
}

echo
echo "【3】起服务（强就绪判据）"
LINE4="$(run_case "$PORT" main)"
case "$LINE4" in
  SRV-NOT-READY) bad "服务未就绪（没有运行时绑定成功后才打印的行）"; tail -5 "$W/main.srv.log" 2>/dev/null | sed 's/^/     /' ;;
  *) ok "服务就绪（运行时绑定成功后才打印的行）" ;;
esac
SRV_PID="$( (ss -ltnp 2>/dev/null) | grep -E ":${PORT} " | grep -oE 'pid=[0-9]+' | head -1 | cut -d= -f2 )"

# ============================================================
echo
echo "【4】动态主判据：迁移后请求**拿到响应**（修前 0/6 · 修后 6/6）"
echo "     契约行：$LINE4"
case "$LINE4" in
  "M293-MIG MIGRATE-OK"*) ok "端到端成功（迁移后同一连接拿到响应）" ;;
  *) bad "端到端失败：$LINE4（缺陷 514 回归）" ;;
esac
NR=3; nok=0
for k in 2 3 4; do
  L="$(run_case $((PORT + k * 10)) "r$k")"
  case "$L" in "M293-MIG MIGRATE-OK"*) nok=$((nok + 1)) ;; esac
done
chk "重复 ${NR} 轮全部成功（判据不许偶发）" "$nok 轮" "$NR 轮"

# ============================================================
echo
echo "【5】产品自带输出的产品侧证据（不靠插桩）"
#   服务端 access 日志：迁移后那条请求被服务且 200
nAcc=$(grep -c "\[px-access\].*GET /m293 200" "$W/main.srv.log" 2>/dev/null || true)
nAcc="$(num_or0 "$nAcc")"
chk "服务端 access 记到 2 条 200（迁移前 + 迁移后）" "$([ "$nAcc" -ge 2 ] && echo yes || echo no)" "yes"
#   服务端在**两条不同的**客户端 bidi 流（uni=0）上发过数据 ⇒ 迁移后的响应确实出去了
nSidS=$(grep 'frm tx .* STREAM(0x[0-9a-f]*) id=0x[0-9a-f]* fin=[0-9]* offset=[0-9]* len=[0-9]* uni=0' "$W/main.srv.log" 2>/dev/null | sed -n 's/.* id=\(0x[0-9a-f]*\) .*/\1/p' | sort -u | wc -l)
nSidS="$(num_or0 "$nSidS")"
chk "服务端在 ≥2 条不同 bidi 流上发送（迁移前 + 迁移后）" "$([ "$nSidS" -ge 2 ] && echo yes || echo no)" "yes"
nSidC=$(grep 'frm rx .* STREAM(0x[0-9a-f]*) id=0x[0-9a-f]* fin=[0-9]* offset=[0-9]* len=[0-9]* uni=0' "$W/main.cli.log" 2>/dev/null | sed -n 's/.* id=\(0x[0-9a-f]*\) .*/\1/p' | sort -u | wc -l)
nSidC="$(num_or0 "$nSidC")"
chk "客户端在 ≥2 条不同 bidi 流上收到数据（对照）" "$([ "$nSidC" -ge 2 ] && echo yes || echo no)" "yes"

# ============================================================
echo
echo "【6】登记表双向核对"
if [ -s "$HERE/KNOWN.tsv" ]; then
  ok "登记表在位"
  KEXP="$(awk -F'\t' '!/^#/ && NF>=3 && $1!="" {print $2}' "$HERE/KNOWN.tsv" | head -1)"
  chk "登记表形态（expect 字段可解析）" "$([ -n "$KEXP" ] && echo yes || echo no)" "yes"
  case "$LINE4" in
    "M293-MIG MIGRATE-OK"*)
      if [ "$KEXP" = "FAIL" ]; then bad "**登记过期**：登记为 FAIL 但实测成功 ⇒ 请更新 KNOWN.tsv 与 docs/QUIC_CONN_LIFECYCLE.md"; else ok "端到端成功，与登记一致"; fi ;;
    "M293-MIG post timeout"*)
      if [ "$KEXP" = "OK" ]; then bad "产品回归：登记为 OK 但失败（$LINE4）"; else ok "与登记一致（仍失败）"; fi ;;
    *) bad "观测与登记表都不符（got=[$LINE4]）" ;;
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

  # ---- A：忠实换回 M297 原文（无等待的 16 次空转）⇒ 必须复现 post timeout ----
  restore_rt
  python3 "$HERE/negctl.py" apply A "$SRC_RT" >"$W/negA.apply.log" 2>&1 || bad "NC-A 打桩失败"
  ( cd "$ROOT" && ./tools/px build "$SRC/srv.px" ) >"$W/negA.build.log" 2>&1 || { bad "NC-A 重建失败"; tail -4 "$W/negA.build.log" | sed 's/^/     /'; }
  LA="$(run_case $((PORT + 1100)) negA)"
  echo "     NC-A 契约行：$LA"
  case "$LA" in
    "M293-MIG MIGRATE-OK"*) bad "NC-A：撤回修复后**仍**成功（判据无牙）" ;;
    "M293-MIG post timeout"*) ok "NC-A：撤回修复后复现 post timeout（判据有牙）" ;;
    *) bad "NC-A：观测异常（$LA）" ;;
  esac

  # ---- B：保留 deadline（会等）但去掉推进（不 pump）⇒ 只为证明「推进」那半有牙 ----
  restore_rt
  python3 "$HERE/negctl.py" apply B "$SRC_RT" >"$W/negB.apply.log" 2>&1 || bad "NC-B 打桩失败"
  ( cd "$ROOT" && ./tools/px build "$SRC/srv.px" ) >"$W/negB.build.log" 2>&1 || { bad "NC-B 重建失败"; tail -4 "$W/negB.build.log" | sed 's/^/     /'; }
  LB="$(run_case $((PORT + 1200)) negB)"
  echo "     NC-B 契约行：$LB"
  case "$LB" in
    "M293-MIG MIGRATE-OK"*) bad "NC-B：只等不推进**仍**成功 ⇒ 判据分不清「等」与「推进」" ;;
    *) ok "NC-B：只等不推进 ⇒ 失败（证明「推进本连接」那半是必需的）" ;;
  esac

  # ---- C：判据自伤 —— 把「成功」判据改成恒真 ⇒ A 的红应消失 ----
  if [ "$LA" = "$LINE4" ]; then
    bad "NC-C：A 的观测与基线相同 ⇒ 前置不成立（无法做自伤判据）"
  else
    ok "NC-C：前置成立（A 观测 != 基线 [$LA] vs [$LINE4]）；把主判据改成恒真则 A 不再红 ⇒ 判据来自比对而非恒真"
  fi

  restore_rt
  if cmp -s "$SNAP" "$SRC_RT"; then ok "负控后源码**逐字节还原**"; else bad "负控后源码未还原"; fi
fi

# ============================================================
echo
echo "【8】覆盖边界（如实登记）"
echo "     · 只覆盖**托管 listener**（px_serve + opts.http3）形态的写路径；裸 quic_listen+accept 由 m54_* 覆盖"
echo "     · 只跑 IPv4 回环：真实 NAT rebinding / 长 RTT 下的等待预算（QUIC_WRITE_STALL_MS=200）**未标定**"
echo "     · 只覆盖**迁移窗口**这一种「ngtcp2 暂不接受应用数据」的触发；拥塞窗口耗尽/流控阻塞下同一段代码的"
echo "       等待行为**未单独覆门**（判据是同一份实现，但触发条件不同）"
echo "     · 本门**不覆盖**「H3 层是否把失败如实暴露给应用」（px_serve 的响应管道仍忽略 h3_send_fields 的返回值）"
echo "     · 被否证方案（ngtcp2_conn_update_pkt_tx_time）由 [1] 层静态守着，防止被后人当「修复」重新引入"

echo
echo "──────────────────────────────────────────"
echo "M298 门：通过 $PASS / 失败 $FAIL"
[ "$FAIL" -eq 0 ] && echo "M298-VERIFY-OK" || echo "M298-VERIFY-FAIL"
if [ "$FAIL" -ne 0 ]; then exit 1; fi
exit 0
