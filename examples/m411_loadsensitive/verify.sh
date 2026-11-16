#!/usr/bin/env bash
# ============================================================
# M411 门 · **LOAD-SENSITIVE 豁免机制自身的守卫**
# ------------------------------------------------------------
# 背景：M411 S2 给 shard_run 加了「分片失败门 → 同轮低负载复跑」分类：
#   复跑绿 ⇒ LOAD-SENSITIVE 豁免（不计产品红）；复跑仍红 ⇒ 产品红。
#   ⚠️ 这个机制天生是一个「可以把红洗成绿」的口子 —— 所以必须有判据守着，
#      否则它会变成 M409/M410 已经被抓过三次的「判据自欺」新入口。
#
# 本门四条（静态可判 + 一份自测）：
#   P1 复跑分类器自测：recheck_loadsensitive.sh --selftest 必须全绿，
#      且断言数 ≥ 11、**负控行 ≥ 3**（S2 仍红不豁免 / S3 换代码不豁免 / S4 连续两轮升级）。
#      负控被删是最危险的退化：删掉之后「豁免」恒真，门却还是绿的。
#   P2 接线存在：shard_run.sh 必须真的调用分类器，且必须打印豁免门名单。
#      （只写脚本不接线 = 行为没变，却让人以为开了闸。）
#   P3 计数守恒：shard_run.sh 里豁免后必须同时 TOT_FAIL 扣减与 TOT_OK 增加；
#      只扣不加 ⇒ 判据会报「绿 != 门清单（有门被跳过）」，方向上是红但归因错。
#   P4 唯一判据没被放宽：chain_verdict.sh 必须仍以「红 != 0 ⇒ RED」判红
#      （豁免只作用于 shard 汇总，不许有人顺手把这条判据改成「红可有可无」）。
#
# 扫描面（打印后由调用方核对，**扫描面为 0 ⇒ 本门自己判红**）：
#   ① packaging/chain/recheck_loadsensitive.sh
#   ② packaging/chain/shard_run.sh
#   ③ /data/autodrive/chain_verdict.sh（宿主唯一判据实现；不存在则 SKIP 该项并打印原因）
#
# 用法：verify.sh [--selftest] [--root <仓库根>]
# ============================================================
set -uo pipefail
# M276 门级互斥锁（gate_lock_guard J1：每个 examples/*/verify.sh 必须 source）
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }
SELF_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$SELF_DIR/../.." && pwd)
SELFTEST=0
while [ $# -gt 0 ]; do
    case "$1" in
        --selftest) SELFTEST=1 ;;
        --root) shift; ROOT=$1 ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
    shift
done

RECHECK="$ROOT/packaging/chain/recheck_loadsensitive.sh"
SHARD="$ROOT/packaging/chain/shard_run.sh"
VERDICT="${M411_VERDICT_SH:-/data/autodrive/chain_verdict.sh}"

fails=0
say() { echo "── $*"; }
ok()  { echo "  PASS  $*"; }
bad() { echo "  FAIL  $*"; fails=$((fails+1)); }

# 扫描面（显式计数 + 零值守卫；M410 R1/R2 口径）
nscan=0
for f in "$RECHECK" "$SHARD" "$VERDICT"; do
    [ -s "$f" ] && nscan=$((nscan+1))
done
say "[1] 扫描面：nscan=$nscan（recheck · shard_run · chain_verdict）"
if [ "$nscan" -le 0 ]; then
    echo "M411-LOADSENSITIVE-FAIL nscan=0（被测面为空 ⇒ 判据没有对象，按红）"
    exit 1
fi
[ -s "$RECHECK" ] || bad "缺分类器 $RECHECK"
[ -s "$SHARD" ]   || bad "缺调度器 $SHARD"
if [ ! -s "$VERDICT" ]; then
    say "[1b] chain_verdict.sh 不存在（$VERDICT）⇒ P4 SKIP（宿主件不在时的正常情形）"
fi

# ── 自检夹具（--selftest：证明四条判据都能红）──────────────────
if [ "$SELFTEST" = 1 ]; then
    T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
    n=0; f=0
    sok() { n=$((n+1)); echo "  ✅ $*"; }
    sbad() { n=$((n+1)); f=$((f+1)); echo "  ❌ $*"; }
    # 造一份「被放宽」的 verdict 与「只扣不加」的 shard，验证 P3/P4 真能抓
    printf 'VERDICT=GREEN\n' > "$T/verdict_lax.sh"
    printf 'TOT_FAIL=$((TOT_FAIL-NEX))\n' > "$T/shard_halved.sh"
    printf 'TOT_FAIL=$((TOT_FAIL-NEX)); TOT_OK=$((TOT_OK+NEX))\n' > "$T/shard_ok.sh"
    grep -q 'RED:-1}" != "0"' "$T/verdict_lax.sh" && sbad "P4 自检失效：放宽的 verdict 没被抓" \
        || sok "P4 正控：放宽后的 verdict 必被判红"
    grep -q 'TOT_OK=$((TOT_OK+NEX))' "$T/shard_halved.sh" && sbad "P3 自检失效：只扣不加没被抓" \
        || sok "P3 正控：只扣不加必被判红"
    grep -q 'TOT_OK=$((TOT_OK+NEX))' "$T/shard_ok.sh" && sok "P3 负控：正确写法不误报" \
        || sbad "P3 负控误报"
    echo "── 自测：$((n-f))/$n 通过 ──"
    [ "$f" = 0 ] && { echo "M411-LOADSENSITIVE-SELFTEST-OK nscan=$nscan"; exit 0; }
    echo "M411-LOADSENSITIVE-SELFTEST-FAIL fails=$f nscan=$nscan"; exit 1
fi

# ── P1 分类器自测（含负控计数）────────────────────────────────
say "[P1] 分类器自测（要求：全绿 · 断言 ≥11 · 负控 ≥3）"
if ! s_out="$(bash "$RECHECK" --selftest 2>&1)"; then
    bad "P1 分类器 --selftest 判红"
    printf '%s\n' "$s_out" | grep -E '❌|自测：' | sed 's/^/      /' | head -10
else
    nall=$(printf '%s\n' "$s_out" | sed -n 's/.*自测：\([0-9]*\)\/\([0-9]*\).*/\2/p' | tail -1)
    nok=$(printf '%s\n' "$s_out" | sed -n 's/.*自测：\([0-9]*\)\/\([0-9]*\).*/\1/p' | tail -1)
    nneg=$(printf '%s\n' "$s_out" | grep -cE '✅ S[234][a-z]? ' || true)
    case "$nall" in ''|*[!0-9]*) nall=0 ;; esac
    case "$nok" in ''|*[!0-9]*) nok=0 ;; esac
    case "$nneg" in ''|*[!0-9]*) nneg=0 ;; esac
    echo "      断言：${nok}/${nall} · 负控行 nneg=${nneg}（扫描面 nscan=${nscan}）"
    [ "$nall" -ge 11 ] && ok "P1 断言数 ${nall} ≥ 11" || bad "P1 断言数 ${nall} < 11（自测被削）"
    [ "$nok" = "$nall" ] && ok "P1 全部通过" || bad "P1 通过 ${nok}/${nall}"
    [ "$nneg" -ge 3 ] && ok "P1 负控 ${nneg} ≥ 3（仍红/换代码/连续两轮）" \
                      || bad "P1 负控 ${nneg} < 3（豁免可能退化成恒真）"
fi

# ── P2 接线 ──────────────────────────────────────────────────
say "[P2] shard_run 接线"
if grep -q 'recheck_loadsensitive\.sh' "$SHARD" 2>/dev/null; then
    ok "P2 shard_run 调用了分类器"
else
    bad "P2 shard_run 没有调用 recheck_loadsensitive.sh（机制没接上）"
fi
if grep -q 'LOAD-SENSITIVE 豁免' "$SHARD" 2>/dev/null; then
    ok "P2 豁免时打印门名单（不静默）"
else
    bad "P2 豁免没有打印门名单（静默豁免 = 自欺）"
fi
if grep -q '不豁免' "$SHARD" 2>/dev/null; then
    ok "P2 故障路径写明「不豁免」（宁红不假绿）"
else
    bad "P2 缺故障路径的「不豁免」声明"
fi

# ── P3 计数守恒 ──────────────────────────────────────────────
say "[P3] 豁免后计数守恒（TOT_FAIL 扣减 ∧ TOT_OK 增加）"
if grep -q 'TOT_OK=\$((TOT_OK+NEX))' "$SHARD" 2>/dev/null \
   && grep -q 'TOT_FAIL=\$((TOT_FAIL-NEX))' "$SHARD" 2>/dev/null; then
    ok "P3 扣减与增加同时存在（green+red 守恒，不会被判「有门被跳过」）"
else
    bad "P3 缺 TOT_FAIL 扣减或 TOT_OK 增加（豁免后计数不守恒）"
fi

# ── P4 唯一判据未被放宽 ──────────────────────────────────────
say "[P4] chain_verdict 仍以「红 != 0 ⇒ RED」判红"
if [ -s "$VERDICT" ]; then
    if grep -q 'RED:-1}" != "0"' "$VERDICT" 2>/dev/null; then
        ok "P4 判红条件在位"
    else
        bad "P4 chain_verdict 的「红 != 0 ⇒ RED」不见了（豁免可能被用来改判据）"
    fi
fi

say "[5] 扫描面核对：nscan=${nscan}"
if [ "$fails" = 0 ]; then
    echo "M411-LOADSENSITIVE-OK nscan=$nscan"
    exit 0
fi
echo "M411-LOADSENSITIVE-FAIL fails=$fails nscan=$nscan"
exit 1
