#!/usr/bin/env bash
# ============================================================
# check_pairs_run.sh —— 回环配对示例必须**真跑**（缺陷 872 同族 · 顶层示例真跑的配对面）
# ------------------------------------------------------------
# 为什么有它：
#   examples/ 里 12 对 *_client.px / *_server.px（H3/QUIC/QPACK/pipe）此前只能整对
#   豁免（examples_run_exempt.tsv 的 pair 类 18 条）：端口写死 + 无人配对调度
#   ⇒ 顶层示例门「跳过它们」等于不跑，而它们恰是 H3/QUIC 唯一的端到端证据。
#
# 判据：
#   C1 配对枚举：pair_runner --list 的 TOTAL ≥ MIN_PAIRS（写死 12；少了 = 发现器坏）
#   C2 逐对真跑：pair_runner 全对 ok（每对内部 P1 端口替换 / P2 两端 rc=0 / P3 端口已释放）
#   C3 前提自证：日志里必须出现 ≥1 条 P1-LISTEN-OK（证明真的起过服务，不是空转判绿）
#   N1 负控：--pair m47_h3 --break-server ⇒ 该对必须「按预期失败」（N1-AS-EXPECTED-FAIL）
#   N2 负控前提自证：同一次必须出现 P2-RC-FAIL（证明扰动真发生了，而不是被跳过）
#   G1 环境守卫：tools/pxc 或 examples/ 不可用 ⇒ rc=3（不降级成 0）
#
# 用法：check_pairs_run.sh [--repo DIR] [--list]
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

MIN_PAIRS=${MIN_PAIRS:-12}
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE=check
while [ $# -gt 0 ]; do
  case "$1" in
    --repo) ROOT="$2"; shift 2 ;;
    --list) MODE=list; shift ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done

RUNNER="$ROOT/selfhost/pair_runner.py"
ok_n=0; bad_n=0
ok()  { echo "✅ $*"; ok_n=$((ok_n+1)); }
bad() { echo "❌ $*"; bad_n=$((bad_n+1)); }

# G1
[ -f "$RUNNER" ] || { echo "ENV: 缺 $RUNNER"; exit 3; }
[ -x "$ROOT/tools/pxc" ] || { echo "ENV: 缺 tools/pxc（构建前提前提）"; exit 3; }
[ -d "$ROOT/examples" ] || { echo "ENV: 缺 examples/"; exit 3; }

LIST=$(python3 "$RUNNER" --repo "$ROOT" --list 2>&1)
TOTAL=$(printf '%s\n' "$LIST" | sed -n 's/^TOTAL=\([0-9][0-9]*\)$/\1/p')
TOTAL=${TOTAL:-0}
if [ "$MODE" = list ]; then printf '%s\n' "$LIST"; exit 0; fi

# C1
if [ "$TOTAL" -ge "$MIN_PAIRS" ]; then
  ok "C1 配对枚举 TOTAL=$TOTAL（≥$MIN_PAIRS）"
else
  bad "C1 配对枚举 TOTAL=$TOTAL < $MIN_PAIRS（发现器坏了，不是「配对变少」）"
fi

TMP=$(mktemp -d /tmp/pairgate.XXXXXX) || exit 3
trap 'rm -rf "$TMP"' EXIT

# C2 + C3
if python3 "$RUNNER" --repo "$ROOT" --work "$TMP/w" > "$TMP/main.log" 2>&1; then
  if grep -q 'PAIR-SUMMARY' "$TMP/main.log"; then
    SUM=$(grep 'PAIR-SUMMARY' "$TMP/main.log" | tail -1)
    if printf '%s' "$SUM" | grep -q 'fail=0'; then
      ok "C2 全对通过（$SUM）"
    else
      bad "C2 有失败：$SUM"; grep '^❌' "$TMP/main.log" | sed 's/^/     /'
    fi
  else
    bad "C2 运行器未输出 PAIR-SUMMARY（异常退出）"
  fi
else
  bad "C2 运行器 rc≠0"; tail -8 "$TMP/main.log" | sed 's/^/     /'
fi
if grep -q 'P1-LISTEN-OK' "$TMP/main.log"; then
  LIVE=$(grep -c 'P1-LISTEN-OK' "$TMP/main.log")
  ok "C3 前提自证：$LIVE 对真起过服务（P1-LISTEN-OK）"
else
  bad "C3 没有任何 P1-LISTEN-OK ⇒ 判绿可能是空转"
fi

# N1 + N2
if python3 "$RUNNER" --repo "$ROOT" --pair m47_h3 --break-server --work "$TMP/n" > "$TMP/neg.log" 2>&1; then
  if grep -q 'N1-AS-EXPECTED-FAIL' "$TMP/neg.log" && grep -q 'P2-RC-FAIL' "$TMP/neg.log"; then
    ok "N1/N2 负控：server 端口 +1 ⇒ 该对失败且扰动已自证"
  else
    bad "N1/N2 负控判据不全：$(grep '^❌' "$TMP/neg.log" | head -2)"
  fi
else
  bad "N1 负控运行器 rc≠0（负控应「按预期失败」而整体仍 rc=0）"
  tail -5 "$TMP/neg.log" | sed 's/^/     /'
fi

echo "PAIRS-RUN-OK pass=$ok_n fail=$bad_n pairs=$TOTAL"
[ "$bad_n" = 0 ] && exit 0 || exit 1
