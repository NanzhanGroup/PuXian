#!/bin/bash
# ============================================================
# M225 门：HTTP/3 连接槽位复用「会话状态串味」（缺陷 327）
# ------------------------------------------------------------
# 缺陷：g_h3st[] / g_h3buf[] / g_last_sid[] 按 **conn 号**（= QUIC 槽位号）索引，
#       而槽位**可复用**。连接收尾后槽位释放，下一个连接拿到同一 conn 号时，
#       h3_conn_setup_c 的幂等短路（只看 st->used）直接返回 ⇒ 新连接沿用
#       上一个连接的 QPACK 动态表会话 ⇒ QUIC 层正常（能收能 ACK）、
#       H3/QPACK 层编码上下文与对端不一致 ⇒ **客户端永远读不到响应**。
# 修复：① 连接回收钩子（槽位释放前清 h3 会话）② setup 幂等判据带「连接代次」身份。
#
# 判据设计要点：
#   * H3 连接空闲超时用 PX_H3_IDLE_MS=300（~0.6s 收尾）⇒ 槽位复用在秒级可测；
#     生产默认仍是 2×8000ms（本门不改默认语义）。
#   * 判据只比**契约行**（GRACE closed / ABRUPT exit-without-close），不比整份输出。
#   * 负控 A 撤回修复主体（钩子+身份）⇒ 必须判红；负控 B 只撤钩子 ⇒
#     身份判据兜住 ⇒ 仍应全绿（证明"双保险"真的在互为兜底）。
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
W=$(mktemp -d /tmp/m225gate.XXXXXX)
PORT=19398
PASS=0; FAIL=0
ok()  { PASS=$((PASS+1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1"; else bad "$1（got=[$2] want=[$3]）"; fi; }
SRV_PID=""

cleanup_all() {
  [ -n "$SRV_PID" ] && kill -9 "$SRV_PID" 2>/dev/null
  rm -f "$W/ready.txt" 2>/dev/null
}
trap 'restore_src; cleanup_all; rm -rf "$W"' EXIT

# ---------- 源码补丁（负控用）----------
H3SRC="$ROOT/runtime/runtime_h3.c"
patch_src() {   # $1 = a（撤钩子+身份）| b（只撤钩子）
  python3 - "$H3SRC" "$1" <<'PY'
import io, sys
p, mode = sys.argv[1], sys.argv[2]
s = io.open(p, encoding="utf-8").read()
E1_OLD = '''    // M225（缺陷 327）：把 h3 会话回收挂进 QUIC 连接回收路径（槽位释放 / quic_close 前）
    px_quic_set_conn_recycle_cb(px_h3_recycle_conn);
'''
E1_NEW = '''    // NEGCTL-M225：撤回「连接回收钩子」注册（负控）
'''
E2_OLD = '''    // M225（缺陷 327）：幂等判据必须带「连接身份」—— 只看 conn 号会把**上一个连接的**
    // QPACK 会话误当成本连接的（槽位复用后 conn 号相同）⇒ 响应编码上下文与对端不一致。
    int64_t ep = px_quic_raw_conn_epoch(conn);
    if (st->used) {
        if (ep != 0 && st->owner_epoch == ep) return true;   // 同一连接：幂等返回
        px_h3_recycle_conn(conn);                            // 身份不符：丢弃陈旧会话（防御层）
    }'''
E2_NEW = '''    // NEGCTL-M225：撤回「身份校验」（负控）
    if (st->used) return true;
'''
E3_OLD = '''    st->owner_epoch = ep;                      // M225：记住本会话属于哪个连接代次'''
E3_NEW = '''    // NEGCTL-M225：撤回 owner_epoch 记录
'''
edits = [(E1_OLD, E1_NEW, "hook")]
if mode == "a":
    edits += [(E2_OLD, E2_NEW, "identity"), (E3_OLD, E3_NEW, "epoch")]
for old, new, tag in edits:
    if s.count(old) != 1:
        print("PATCH-ANCHOR-FAIL %s count=%d" % (tag, s.count(old))); sys.exit(1)
    s = s.replace(old, new, 1)
io.open(p, "w", encoding="utf-8").write(s)
print("PATCH-OK mode=%s edits=%d" % (mode, len(edits)))
PY
}
restore_src() {
  # M225s1：正常路径**必须静默** —— CI 里门是以 `> /tmp/v_*.log 2>&1` 跑的，
  #   **任何流**的输出都会成为日志末行；而 packaging/ci_diagnose.py 对
  #   `/tmp/v_*.log` 做**末行裁决** ⇒ 提示语会把 `M225-VERIFY-OK` 顶掉
  #   （本机实测：非 ok 1 份 ['v_m225.log'] ⇒ CI step 13 假红，而 job 日志 403 读不出真因）。
  #   加固层在 ci_diagnose（可跳过尾巴），但**门本身也不该污染这条契约**。
  if [ -f "$W/h3.bak" ]; then
    cp -f "$W/h3.bak" "$H3SRC"
    cmp -s "$W/h3.bak" "$H3SRC" || echo "  ❌ 源码还原失败" >&2
  fi
}

# ---------- 编译 / 运行 ----------
build_all() {
  cp -f "$DIR/srv.px" "$DIR/cli_grace.px" "$DIR/cli_abrupt.px" "$W/" 2>/dev/null
  local f
  for f in srv cli_grace cli_abrupt; do
    ( cd "$W" && "$ROOT/tools/px" build "$W/$f.px" > "$W/b_$f.log" 2>&1 ) || { echo "BUILDFAIL $f"; tail -4 "$W/b_$f.log"; return 1; }
  done
  return 0
}

run_one() {   # $1 = grace|abrupt → 契约行
  local m="$1" out
  if [ "$m" = "grace" ]; then
    out=$(M225_HTTP_PORT=$PORT PX_H3_IDLE_MS=300 timeout -k 5 20 "$W/build/cli_grace" 2>/dev/null | tail -1)
  else
    out=$(M225_HTTP_PORT=$PORT PX_H3_IDLE_MS=300 timeout -k 5 20 "$W/build/cli_abrupt" 2>/dev/null | tail -1)
  fi
  if [ "${M225_GATE_LOOSE:-0}" = "1" ]; then [ -n "$out" ] && echo "OK" || echo "EMPTY"; return; fi
  if [ "$m" = "grace" ]; then echo "$out"; else echo "$out"; fi
}

run_series() {   # stdout 只输出最后一行 OK=<n> ACCESS=<m>；过程信息走 stderr
  rm -f "$W/ready.txt"
  M225_HTTP_PORT=$PORT M225_READY="$W/ready.txt" PX_H3_IDLE_MS=300 \
    "$W/build/srv" > "$W/srv.log" 2>&1 &
  SRV_PID=$!
  local i=0
  while [ "$i" -lt 80 ]; do [ -f "$W/ready.txt" ] && break; sleep 0.2; i=$((i+1)); done
  if [ ! -f "$W/ready.txt" ]; then echo "NO-READY"; kill -9 "$SRV_PID" 2>/dev/null; SRV_PID=""; return; fi
  sleep 0.6
  local n_ok=0 k=0 m out want
  for m in grace grace abrupt grace grace grace; do
    k=$((k+1))
    out=$(run_one "$m")
    if [ "${M225_GATE_LOOSE:-0}" = "1" ]; then
      want="OK"
    else
      if [ "$m" = "grace" ]; then want="GRACE closed"; else want="ABRUPT exit-without-close"; fi
    fi
    if [ "$out" = "$want" ]; then
      n_ok=$((n_ok+1))
    else
      echo "     · 第 $k 次（$m）→ [${out:-<空>}]" >&2
    fi
    sleep 1.0
  done
  local access
  access=$(grep -c 'px-access' "$W/srv.log" 2>/dev/null)
  [ -z "$access" ] && access=0
  kill -9 "$SRV_PID" 2>/dev/null; SRV_PID=""
  echo "OK=$n_ok ACCESS=$access"
}

echo "===== M225 门：H3 连接槽位复用「会话状态串味」（缺陷 327）====="
echo "【0】源码备份"
cp -f "$H3SRC" "$W/h3.bak" && ok "已备份 runtime_h3.c" || bad "备份失败"

echo "【1】静态判据（修复在位 + 规模下限）"
n_recycle=$(grep -c 'void px_h3_recycle_conn(int64_t conn)' "$H3SRC" 2>/dev/null); [ -z "$n_recycle" ] && n_recycle=0
chk "px_h3_recycle_conn 定义在位" "$n_recycle" "1"
n_reg=$(grep -c 'px_quic_set_conn_recycle_cb(px_h3_recycle_conn)' "$H3SRC" 2>/dev/null); [ -z "$n_reg" ] && n_reg=0
chk "回收钩子已注册（h3→quic）" "$n_reg" "1"
n_ep=$(grep -c 'st->owner_epoch == ep' "$H3SRC" 2>/dev/null); [ -z "$n_ep" ] && n_ep=0
chk "setup 幂等判据带连接身份" "$n_ep" "1"
n_own=$(grep -c 'int64_t owner_epoch;' "$H3SRC" 2>/dev/null); [ -z "$n_own" ] && n_own=0
chk "h3conn_state.owner_epoch 字段在位" "$n_own" "1"
QS="$ROOT/runtime/runtime_quic.c"
n_hook=$(grep -c 'g_quic_recycle(cid)' "$QS" 2>/dev/null); [ -z "$n_hook" ] && n_hook=0
n_hook2=$(grep -c 'g_quic_recycle(args\[0\].as.i)' "$QS" 2>/dev/null); [ -z "$n_hook2" ] && n_hook2=0
chk "回收钩子调用点=2（托管连接 + quic_close）" "$((n_hook + n_hook2))" "2"
n_lock=$(grep -c 'pthread_mutex_lock(&g_quic_srv_mu);' "$QS" 2>/dev/null); [ -z "$n_lock" ] && n_lock=0
[ "$n_lock" -ge 4 ] && ok "quic 锁使用点 ≥4（alloc 已纳入互斥）" || bad "quic 锁使用点偏少（got=$n_lock）"
# 规模下限：回收函数体必须真有内容（防"判据静默失效"——函数被掏空仍判绿）
body_lines=$(awk '/^void px_h3_recycle_conn\(int64_t conn\) \{/,/^\}/' "$H3SRC" | wc -l)
[ "$body_lines" -ge 12 ] && ok "px_h3_recycle_conn 函数体 $body_lines 行（≥12）" || bad "回收函数体过短（got=$body_lines）"

echo "【2】编译门语料"
if build_all; then ok "srv + cli_grace + cli_abrupt 编译通过"; else bad "编译失败"; echo "PASS=$PASS FAIL=$FAIL"; exit 1; fi

echo "【3】动态正判据：同槽复用 6 连（grace grace abrupt grace grace grace）"
S=$(run_series)
SOK=$(echo "$S" | sed -n 's/^OK=\([0-9]*\).*/\1/p')
SAC=$(echo "$S" | sed -n 's/.*ACCESS=\([0-9]*\)$/\1/p')
[ -z "$SOK" ] && SOK=0
[ -z "$SAC" ] && SAC=0
chk "6 次全部拿到响应（契约行）" "$SOK" "6"
chk "服务端处理次数 == 6（排除\"根本没处理\"）" "$SAC" "6"

if [ "$NEG_SKIP" = "1" ]; then
  echo "【4】负控：--neg-skip 跳过"
else
  echo "【4】负控 A：撤回修复主体（钩子 + 身份校验）⇒ 必须判红"
  patch_src a
  if build_all; then
    SA=$(run_series)
    AOK=$(echo "$SA" | sed -n 's/^OK=\([0-9]*\).*/\1/p'); [ -z "$AOK" ] && AOK=0
    [ "$AOK" -lt 6 ] && ok "缺陷复现（成功 $AOK/6 < 6）" || bad "撤回修复后仍全绿（负控无牙）"
  else
    bad "负控 A 编译失败"
  fi
  restore_src

  echo "【5】负控 B：只撤钩子（保留身份判据）⇒ 应仍全绿（双保险兜底）"
  patch_src b
  if build_all; then
    SB=$(run_series)
    BOK=$(echo "$SB" | sed -n 's/^OK=\([0-9]*\).*/\1/p'); [ -z "$BOK" ] && BOK=0
    chk "身份判据独立兜住（成功 6/6）" "$BOK" "6"
  else
    bad "负控 B 编译失败"
  fi
  restore_src

  echo "【6】负控 C：判据自伤（成功判据改宽松）⇒ 负控 A 的红必须消失"
  patch_src a
  if build_all; then
    M225_GATE_LOOSE=1 SC=$(run_series)
    COK=$(echo "$SC" | sed -n 's/^OK=\([0-9]*\).*/\1/p'); [ -z "$COK" ] && COK=0
    chk "宽松判据下不再判红（证明红来自契约比对）" "$COK" "6"
  else
    bad "负控 C 编译失败"
  fi
  restore_src
  echo "【7】收尾：重编回修复态"
  build_all >/dev/null 2>&1 && ok "已回修复态产物" || bad "回修复态编译失败"
fi

echo
echo "覆盖边界（如实登记）："
echo "  · 本门只覆盖「托管 listener（px_serve+http3 / h3_server_listen）」路径；"
echo "    裸 quic_listen+quic_accept 的 demo 路径不经连接回收钩子（其 conn 状态随 memset 复位）。"
echo "  · 不断言 QUIC 层内部状态（如 ERR_DRAINING 日志文本）—— 只断言「客户端能否拿到响应」这一契约。"
echo "  · PX_H3_IDLE_MS 仅本门用于加速；生产默认为 2×8000ms 空闲关闭。"
echo "  · 未覆盖：0-RTT / 连接迁移路径下的 h3 会话复用（同族，已登记）。"
echo
echo "===== 失败 $FAIL 项 · 通过 $PASS 项 ====="
[ "$FAIL" -eq 0 ] && echo "M225-VERIFY-OK"
exit 0
