#!/usr/bin/env bash
# ============================================================================
# check_chain_red_keeps_stamp.sh —— 负控：链判红不许覆盖 chain_stamp.sh 的详细戳（缺陷 884）
#
# 直接 source packaging/chain/chain_red_lib.sh（生产实现），造戳、断言内容。
# 判据：T1 详细戳保留 + T2 老写法必须失败（证明夹具能抓病） + T3 前置阶段写短戳
#       + T4 下游 green= 解析 + T5 无 green= 的戳可覆盖。
# 用法：./selfhost/check_chain_red_keeps_stamp.sh [-v]
# 退出码：0 全部符合预期 / 1 有断言失败
# ============================================================================
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
LIB="$HERE/../packaging/chain/chain_red_lib.sh"
[ -f "$LIB" ] || { echo "❌ 找不到 $LIB"; exit 2; }
. "$LIB"

TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
DETAIL='CHAIN-VERDICT=RED reason=红 2 门 green=303 red=2 listed=305 expect=305 timeout=0 head=e086279'
PASS=0; FAIL=0
say(){ :; }                       # 生产里落链日志，这里静音
MS=430; DRY=1; LOG="$TMP/chain.log"; NOTIFY_SH=/bin/true
ok(){ PASS=$((PASS+1)); [ "${1:-}" = "-v" ] && echo "  ✅ $2"; return 0; }
no(){ FAIL=$((FAIL+1)); echo "  ❌ $2"; }

mk(){ printf '%s\n' "$DETAIL" > "$DONE"; }

echo "── T1 详细戳 + STAMP_FROM_THIS_RUN=1 ⇒ 保留 green= 只追加原因"
DONE="$TMP/T1"; mk; STAMP_FROM_THIS_RUN=1; chain_red no-go k t
grep -q 'green=303' "$DONE" && grep -q 'CHAIN-VERDICT=RED reason=红 2 门' "$DONE" \
    && grep -q '^CHAIN-RED-REASON=no-go$' "$DONE" && ok "" "T1 详细戳完整 + 追加原因" || no "" "T1 失败：$(cat "$DONE")"

echo "── T4 下游解析：m430_pipeline.sh 口径必须取到 green=303"
G="$(grep -oE 'green=[0-9]+' "$DONE" 2>/dev/null | head -1 | cut -d= -f2)"
[ "$G" = "303" ] && ok "" "T4 green=$G" || no "" "T4 解析不到 green（得 '$G'）"

echo "── T2 负控：老写法必须把 green= 冲掉（否则夹具无效）"
OLD="$TMP/T2"; printf '%s\n' "$DETAIL" > "$OLD"
echo "CHAIN-VERDICT=RED reason=no-go" > "$OLD"
if grep -q 'green=' "$OLD"; then no "" "T2 老写法居然保留了 green= ⇒ 夹具抓不到病"; else ok "" "T2 老写法丢 green=（夹具有效）"; fi

echo "── T3 前置自证阶段（STAMP_FROM_THIS_RUN=0，无戳）⇒ 短戳"
DONE="$TMP/T3"; rm -f "$DONE"; STAMP_FROM_THIS_RUN=0; chain_red dirty-tree k t
[ "$(cat "$DONE")" = "CHAIN-VERDICT=RED reason=dirty-tree" ] && ok "" "T3 短戳正确" || no "" "T3 得 $(cat "$DONE")"

echo "── T3b 前置自证阶段遇上轮残留的详细戳 ⇒ 仍写短戳（不许把上轮的绿数当本轮）"
DONE="$TMP/T3b"; mk; STAMP_FROM_THIS_RUN=0; chain_red rebake-stale k t
[ "$(cat "$DONE")" = "CHAIN-VERDICT=RED reason=rebake-stale" ] && ok "" "T3b 残留戳被本轮短戳替换" || no "" "T3b 得 $(cat "$DONE")"

echo "── T5 本轮戳但无 green=（no-chain-stamp-sh）⇒ 覆盖成短戳"
DONE="$TMP/T5"; echo "CHAIN-VERDICT=RED reason=no-chain-stamp-sh" > "$DONE"
STAMP_FROM_THIS_RUN=1; chain_red no-go k t
[ "$(cat "$DONE")" = "CHAIN-VERDICT=RED reason=no-go" ] && ok "" "T5 可覆盖" || no "" "T5 得 $(cat "$DONE")"

echo "── 小计：PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = 0 ] || exit 1
echo "✅ check_chain_red_keeps_stamp 全部通过"
