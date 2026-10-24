#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────
# packaging/chain/shard_gate_verdict.sh —— **分片档**全量门裁决器
#
# 为什么叫 shard_gate_verdict.sh 而不叫 gate_verdict.sh：
#   仓里 `packaging/chain/gate_verdict.sh` 已是**串行档权威**（三路求交 + 前提自证）。
#   本脚本裁决的是**另一档**：分片档（shard_run.sh 的 N 路 worktree 并行）的日志，
#   取数口径完全不同（合计行 + shardN 行 + 分片日志里的 ❌/⏱）。
#   两者同名会把权威档覆盖掉 ⇒ 独立命名、并列存放，谁也不吞谁。
#
# 为什么必须入仓（缺陷 495 / M283 那类教训的形状）：
#   本脚本此前住在宿主目录（仓库外）⇒ 全仓守卫（check_gate_paths / anchors /
#   registry / check_orchestrator）**全都扫不到它**；它自己写错判据（缺陷 580）
#   没有任何门能抓到，只能靠人肉看日志。分片档裁决长期是「仓库外孤本」。
#
# 沿革：
#   缺陷 578：r15 汇总段用 `grep '^✅ ' $LOG` 统计绿数，而门结果写在 shard*.log
#            ⇒ G=0 R=0 ⇒ 全绿被判 RED。教训：判据找不到先怀疑判据。
#   缺陷 580：判据 `绿数 != 门清单` ⇒ 只要有一个红项就必然成立，
#            于是每轮带红都用错误理由报红（r17b：232 绿 + 1 红 = 233，
#            算术没错，却报「绿数≠门清单(232≠233)」）。
#            红是对的，理由错了；错理由比漏红更容易误导后来者。
#
# 判据（每条都有牙，且各自能单独判红；--self-test 逐条负控自证）：
#   ① 红项：合计红 != 0              ⇒ RED（并列红门名）
#   ② 门数平衡：绿 + 红 == 门清单     ⇒ 不平 = 有门没记账（漏门/空跑）
#      ★ 这条取代旧「绿 == 门清单」：旧式在「有红」时恒真，是缺陷 580 的假警报源。
#   ③ 分片绿之和 == 合计绿            ⇒ 抓「合计与分片两本账不一致」
#   ④ 分片红之和 == 合计红            ⇒ 抓「分片红了但没进合计」
#   ⑤ 分片 rc 非 0 / quiet rc 非 0 / 主 rc 非 0 ⇒ RED
#   ⑥ 红门清单数 == 合计红            ⇒ 判红必须列得出红门（借串行档硬规矩③）
#      （需 --logdir；无 logdir 时该条记为 SKIP 并打印，不当绿也不当红）
#
# 用法：
#   shard_gate_verdict.sh <gate_log> [--logdir <分片日志目录>] [--quiet]
#   shard_gate_verdict.sh --self-test          # 判据自证（合成夹具，不跑任何门）
# 输出：第一行恒为 VERDICT=...，第二行 VERDICT-REASON=...，供调用方 grep。
# 退出码：0=GREEN 1=RED 2=UNKNOWN 3=PREMISE-FAIL（取不到判据所需的数）
# ─────────────────────────────────────────────────────────────────────
set -uo pipefail

QUIET=0
say() { [ "$QUIET" = 1 ] || printf '%s\n' "$*"; }

# ══════════════════════════════════════════════════════════════════════
# 裁决主体（CLI 与 --self-test 共用同一份代码 —— 自证的不是副本）
#   $1 = 门日志；$2 = 分片日志目录（可空）
# ══════════════════════════════════════════════════════════════════════
run_verdict() {
  local LOG="${1:-}" LOGDIR="${2:-}"
  local TOTAL_LINE TG TR TK SG SR BADRC NSH QRC MRC REDGATES REDSRC NRED FAILS g r rc

  [ -n "$LOG" ] || { echo "VERDICT=PREMISE-FAIL"; echo "VERDICT-REASON=未给门日志路径"; return 3; }
  if [ ! -r "$LOG" ]; then
    echo "VERDICT=UNKNOWN"
    echo "VERDICT-REASON=日志不可读: $LOG"
    return 2
  fi

  # ── 1) 合计行 ───────────────────────────────────────────────────────
  TOTAL_LINE=$(grep -aE '合计：绿 [0-9]+ · 红 [0-9]+' "$LOG" | tail -1)
  if [ -z "$TOTAL_LINE" ]; then
    echo "VERDICT=UNKNOWN"
    echo "VERDICT-REASON=无合计行（分片未跑完/日志被截断）"
    return 2
  fi
  TG=$(printf '%s\n' "$TOTAL_LINE" | sed -nE 's/.*合计：绿 ([0-9]+).*/\1/p')
  TR=$(printf '%s\n' "$TOTAL_LINE" | sed -nE 's/.*合计：绿 [0-9]+ · 红 ([0-9]+).*/\1/p')
  TK=$(printf '%s\n' "$TOTAL_LINE" | sed -nE 's/.*门清单 ([0-9]+).*/\1/p')
  TG=${TG:-0}; TR=${TR:-0}; TK=${TK:-0}

  # ── 2) 分片/quiet 行：绿和、红和、rc ───────────────────────────────
  SG=0; SR=0; BADRC=""; NSH=0
  while IFS= read -r line; do
    g=$(printf '%s\n' "$line" | sed -nE 's/^[[:space:]]+[a-z0-9]+[[:space:]]*:[[:space:]]*绿 ([0-9]+).*/\1/p')
    r=$(printf '%s\n' "$line" | sed -nE 's/^[[:space:]]+[a-z0-9]+[[:space:]]*:[[:space:]]*绿 [0-9]+ · 红 ([0-9]+).*/\1/p')
    rc=$(printf '%s\n' "$line" | sed -nE 's/.*rc=([0-9]+).*/\1/p')
    [ -n "$g" ] && { SG=$((SG + g)); NSH=$((NSH + 1)); }
    [ -n "$r" ] && SR=$((SR + r))
    if [ -n "$rc" ] && [ "$rc" != "0" ]; then
      BADRC="$BADRC $(printf '%s' "$line" | sed -nE 's/^[[:space:]]+([a-z0-9]+).*/\1/p')(rc=$rc)"
    fi
  done < <(grep -aE '^[[:space:]]+(shard[0-9]+|quiet)[[:space:]]*:' "$LOG")

  # quiet 段的 rc（形状：`quiet rc=N`，单独一行）
  QRC=$(grep -aE '^[[:space:]]+quiet rc=[0-9]+' "$LOG" | tail -1 | sed -nE 's/.*rc=([0-9]+).*/\1/p')
  if [ -n "${QRC:-}" ] && [ "$QRC" != "0" ]; then BADRC="$BADRC quiet(rc=$QRC)"; fi
  # 主 rc（shard_run.sh 退出码）
  MRC=$(grep -aE '^rc=[0-9]+$' "$LOG" | tail -1 | sed -nE 's/rc=([0-9]+)/\1/p')
  if [ -n "${MRC:-}" ] && [ "$MRC" != "0" ]; then BADRC="$BADRC 主(rc=$MRC)"; fi

  # ── 3) 红门清单（只认门实现自己打的 ❌ / ⏱ 行）────────────────────
  REDGATES=""; REDSRC="SKIP"
  if [ -n "$LOGDIR" ] && [ -d "$LOGDIR" ]; then
    REDGATES=$(grep -ahE '^(❌|⏱)[[:space:]]+[a-zA-Z0-9_]+（' "$LOGDIR"/shard*.log "$LOGDIR"/quiet.log 2>/dev/null \
        | sed -nE 's/^(❌|⏱)[[:space:]]+([a-zA-Z0-9_]+)（.*/\2/p' | sort -u | tr '\n' ' ')
    REDGATES="${REDGATES% }"
    REDSRC="OK"
  elif [ -n "$LOGDIR" ]; then
    REDSRC="LOGDIR-MISSING"
  fi
  NRED=0; [ -n "$REDGATES" ] && NRED=$(printf '%s\n' "$REDGATES" | wc -w)

  say "取数：合计绿=$TG 红=$TR 门清单=$TK · 分片绿之和=$SG 红之和=$SR（$NSH 段）· 红门清单[$REDSRC]=${REDGATES:-空}"

  # ── 4) 判据 ────────────────────────────────────────────────────────
  FAILS=""
  ADD() { if [ -z "$FAILS" ]; then FAILS="$1"; else FAILS="$FAILS · $1"; fi; }

  [ "$TR" -ne 0 ] && ADD "有红项 $TR（${REDGATES:-未列出}）"
  [ "$TK" -eq 0 ] && ADD "门清单数缺失"
  # ② 门数平衡：绿 + 红 == 门清单（旧「绿 == 门清单」在有红时恒假，是缺陷 580）
  if [ "$TK" -ne 0 ] && [ $((TG + TR)) -ne "$TK" ]; then
    ADD "门数不平(绿$TG+红$TR=$((TG + TR))≠清单$TK，差$((TK - TG - TR)))"
  fi
  # ③④ 两本账
  [ "$SG" -ne "$TG" ] && ADD "分片绿之和≠合计绿($SG≠$TG)"
  [ "$SR" -ne "$TR" ] && ADD "分片红之和≠合计红($SR≠$TR)"
  # ⑤ rc
  [ -n "$BADRC" ] && ADD "rc非0[$BADRC ]"
  # ⑥ 判红必须列得出红门
  if [ "$REDSRC" = "OK" ] && [ "$NRED" -ne "$TR" ]; then
    ADD "红门清单数≠合计红($NRED≠$TR)"
  elif [ "$REDSRC" = "SKIP" ]; then
    say "  ⚠️ 未给 --logdir ⇒ 判据⑥（红门清单数==合计红）本轮 SKIP，不参与裁决"
  elif [ "$REDSRC" = "LOGDIR-MISSING" ]; then
    ADD "logdir 不存在($LOGDIR) ⇒ 无法列红门"
  fi

  if [ -n "$FAILS" ]; then
    echo "VERDICT=RED"
    echo "VERDICT-REASON=$FAILS"
    return 1
  fi
  echo "VERDICT=GREEN"
  echo "VERDICT-REASON=红0·绿$TG+红$TR=门清单$TK·分片两本账一致·各段rc=0·红门清单一致"
  return 0
}

# ══════════════════════════════════════════════════════════════════════
# --self-test：13 条断言（10 条行为 + 3 条静态防改窄）
#   每条负控只扰动一处，且**期望串**指名到判据，避免「红了但不知道红在哪」。
# ══════════════════════════════════════════════════════════════════════
self_test() {
  local T; T=$(mktemp -d /tmp/shard_gate_verdict_ST.XXXXXX) || return 2
  trap 'rm -rf "$T"' RETURN
  local OK=0 BAD=0

  pass() { OK=$((OK + 1)); printf '  ✅ %s\n' "$*"; }
  fail() { BAD=$((BAD + 1)); printf '  ❌ %s\n' "$*"; }

  # expect <名称> <期望rc> <期望串> <日志> [logdir]
  expect() {
    local name="$1" wantrc="$2" wantre="$3" log="$4" dir="${5:-}" out rc
    out=$( (QUIET=1; run_verdict "$log" "$dir") 2>&1 ); rc=$?
    if [ "$rc" -ne "$wantrc" ]; then
      fail "$name —— rc=$rc（期望 $wantrc）；输出：$(printf '%s' "$out" | tr '\n' '|')"
      return
    fi
    if [ -n "$wantre" ] && ! printf '%s\n' "$out" | grep -qF -- "$wantre"; then
      fail "$name —— rc=$rc 对，但理由里没有「$wantre」；输出：$(printf '%s' "$out" | tr '\n' '|')"
      return
    fi
    pass "$name（rc=$rc · 理由含「${wantre:-裁决}」）"
  }

  mkdir -p "$T/d1" "$T/d_empty" "$T/d_two"
  printf '❌ m_demo_red（rc=1, 3s）\n'            > "$T/d1/shard1.log"
  : > "$T/d_empty/shard1.log"
  printf '❌ m_demo_red（rc=1, 3s）\n❌ m_demo_extra（rc=1, 2s）\n' > "$T/d_two/shard1.log"

  # 夹具：形状逐字取自真实分片日志（shard_run.sh 输出）
  cat > "$T/green.log" <<'EOF'
  shard1: 绿 5 · 红 0 · rc=0
  shard2: 绿 5 · 红 0 · rc=0
  合计：绿 10 · 红 0（门清单 10）
EOF
  cat > "$T/nc1_red.log" <<'EOF'
  shard1: 绿 9 · 红 1 · rc=1
  合计：绿 9 · 红 1（门清单 10）
EOF
  cat > "$T/nc2_imbalance.log" <<'EOF'
  shard1: 绿 9 · 红 0 · rc=0
  合计：绿 9 · 红 0（门清单 10）
EOF
  cat > "$T/nc3_sg.log" <<'EOF'
  shard1: 绿 8 · 红 0 · rc=0
  合计：绿 9 · 红 0（门清单 9）
EOF
  cat > "$T/nc4_sr.log" <<'EOF'
  shard1: 绿 9 · 红 0 · rc=0
  合计：绿 9 · 红 1（门清单 10）
EOF
  cat > "$T/nc5_trunc.log" <<'EOF'
  shard1: 绿 3 · 红 0 · rc=0
EOF
  cat > "$T/nc6_rc.log" <<'EOF'
  shard1: 绿 10 · 红 0 · rc=3
  合计：绿 10 · 红 0（门清单 10）
EOF
  cat > "$T/nc7_nolist.log" <<'EOF'
  shard1: 绿 9 · 红 1 · rc=1
  合计：绿 9 · 红 1（门清单 10）
EOF
  cat > "$T/nc9_twolist.log" <<'EOF'
  shard1: 绿 9 · 红 1 · rc=1
  合计：绿 9 · 红 1（门清单 10）
EOF

  printf '== 正控 ==\n'
  expect "F1 全绿无红（无 logdir）"                0 "VERDICT=GREEN" "$T/green.log"
  expect "NC8 全绿 + 空 logdir（不误报）"           0 "VERDICT=GREEN" "$T/green.log" "$T/d_empty"

  printf '== 负控（每条单独判红）==\n'
  expect "NC1 有红项必须判红并指名"                1 "有红项 1（m_demo_red）"   "$T/nc1_red.log" "$T/d1"
  expect "NC2 门数不平(绿+红≠清单)"                1 "门数不平"                  "$T/nc2_imbalance.log"
  expect "NC3 分片绿之和≠合计绿"                   1 "分片绿之和≠合计绿"        "$T/nc3_sg.log"
  expect "NC4 分片红之和≠合计红"                   1 "分片红之和≠合计红"        "$T/nc4_sr.log"
  expect "NC5 无合计行⇒UNKNOWN(rc=2)"              2 "无合计行"                  "$T/nc5_trunc.log"
  expect "NC6 分片 rc 非0 而无红项"                1 "rc非0"                     "$T/nc6_rc.log"
  expect "NC7 logdir 给但列不出红门"               1 "红门清单数≠合计红(0≠1)"   "$T/nc7_nolist.log" "$T/d_empty"
  expect "NC9 红门清单虚高(2≠1)"                   1 "红门清单数≠合计红(2≠1)"   "$T/nc9_twolist.log" "$T/d_two"

  # ── 静态防改窄：判据不许被退回旧式，也不许被删 ──────────────────────
  # ⚠️ 扫描面**只取 run_verdict 函数体**，不许扫整文件：
  #   首版扫整文件时 S3 立刻假红 —— 因为 S3 的**判据串本身就写在本函数里**，
  #   判据把自己判红了（与 check_gate_paths.sh 必须跳过自身是同一个坑）。
  local code
  code=$(awk '/^run_verdict\(\) \{/{f=1} f&&/^\}$/{print;exit} f' "$0" | grep -vE '^[[:space:]]*#')
  printf '== 静态防改窄 ==\n'
  if printf '%s\n' "$code" | grep -qF 'TG + TR'; then
    pass "S1 门数平衡判据（绿+红==清单）在"
  else
    fail "S1 门数平衡判据被删/改窄"
  fi
  if printf '%s\n' "$code" | grep -qF '"$SR" -ne "$TR"'; then
    pass "S2 分片红两本账判据在"
  else
    fail "S2 分片红两本账判据被删"
  fi
  if printf '%s\n' "$code" | grep -qF '"$TG" -ne "$TK"'; then
    fail "S3 旧式「绿==门清单」回来了（缺陷 580 复发）"
  else
    pass "S3 旧式「绿==门清单」不在（缺陷 580 未复发）"
  fi

  printf '\n通过 %d · 失败 %d\n' "$OK" "$BAD"
  [ "$BAD" -eq 0 ] && { printf 'SHARD-GATE-VERDICT-SELFTEST-OK\n'; return 0; }
  printf 'SHARD-GATE-VERDICT-SELFTEST-FAIL\n'
  return 2
}

# ══════════════════════════════════════════════════════════════════════
# CLI
# ══════════════════════════════════════════════════════════════════════
LOG=""; LOGDIR=""
while [ $# -gt 0 ]; do
  case "$1" in
    --self-test) self_test; exit $? ;;
    --logdir) LOGDIR="${2:-}"; shift 2 ;;
    --quiet)  QUIET=1; shift ;;
    -h|--help) sed -n '2,45p' "$0"; exit 0 ;;
    -*) echo "未知参数: $1" >&2; exit 3 ;;
    *)  LOG="$1"; shift ;;
  esac
done

run_verdict "$LOG" "$LOGDIR"
exit $?
