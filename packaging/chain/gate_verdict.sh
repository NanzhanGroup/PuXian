#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────
# packaging/chain/gate_verdict.sh —— 全量门裁决器（唯一权威）
#
# 为什么存在（缺陷 495 / 490）：
#   M283 的 watcher 用 `grep -qE '双路判据一致：0 红'` 判绿 —— 而这个串
#   **在仓库里从来不存在**（全仓 0 命中；`git grep -F` 实测为空）。
#   门其实是绿的（汇总：失败 0 项 · 190 ✅ · 0 ❌），却被判「红」+ 挂 p1
#   ⇒ 45 分钟后升级成一条假警报推到用户 QQ。
#   ⇒ 编排脚本各写各的判据串 = 每次都可能凭空捏造；且它们住在 /tmp
#     ⇒ 仓库那套守卫（check_gate_paths / anchors / registry）全都扫不到。
#
# 三条硬规矩：
#   ① **前提自证**：判据依赖的串必须在**门实现**（run_gates.sh）里找得到；
#      找不到 ⇒ rc=3 拒绝裁决 —— 不许「先判了再说」。
#   ② **多路一致才裁决**：汇总行 / 失败行（❌ 与 ⏱）/ 时序 TSV 三路求交；
#      不一致 ⇒ rc=2 报 INCONSISTENT，**不许挑一路当结论**。
#      （本轮形状：判红而红门清单为空 = 自相矛盾，正好被这条抓住。）
#   ③ **判红必须列得出红门**：列不出 ⇒ 是判据的问题，不是门的问题。
#
# ⚠️ 为什么失败行要数两种（❌ 与 ⏱）：
#   run_gates.sh 里**超时**走的是 `⏱`（rc=124/137），**不打印 ❌**，但同样 FAIL++。
#   只数 ❌ ⇒ 超时门会被漏掉 ⇒ 「看似 0 红」—— 这正是 M207/M221 那类静默失败。
#
# 用法：
#   gate_verdict.sh <gates.log> [--times <times.tsv>] [--runner <run_gates.sh>]
#                   [--quiet]
# 输出（第一行恒为裁决，便于调用方 grep）：
#   VERDICT=GREEN|RED|INCONSISTENT|PREMISE-FAIL
# 退出码：0 绿 / 1 红 / 2 不一致（含未跑完） / 3 前提不成立
# ─────────────────────────────────────────────────────────────────────
set -uo pipefail

LOG=""; TIMES=""; RUNNER=""; QUIET=0
while [ $# -gt 0 ]; do
    case "$1" in
        --times)  TIMES="${2:-}";  shift 2 ;;
        --runner) RUNNER="${2:-}"; shift 2 ;;
        --quiet)  QUIET=1; shift ;;
        -h|--help) sed -n '2,40p' "$0"; exit 0 ;;
        -*) echo "未知参数: $1" >&2; exit 3 ;;
        *)  LOG="$1"; shift ;;
    esac
done

die_premise() {   # rc=3：拒绝裁决（前提不成立）
    echo "VERDICT=PREMISE-FAIL"
    printf '  ⛔ %s\n' "$*"
    exit 3
}
say() { [ "$QUIET" = 1 ] || printf '%s\n' "$*"; }

[ -n "$LOG" ] || die_premise "用法：gate_verdict.sh <gates.log> [--times t.tsv] [--runner run_gates.sh]"
[ -f "$LOG" ] || die_premise "门日志不存在：$LOG"

# ── ① 前提自证：裁决器依赖的「门输出词汇」必须真在门实现里 ────────────
#    判据串的骨架（不是具体值）—— 值随运行而变，骨架是裁决器与实现之间的契约。
if [ -n "$RUNNER" ]; then
    [ -f "$RUNNER" ] || die_premise "门实现不存在：$RUNNER"
    _missing=""
    for _tok in '汇总：失败' '✅' '❌' '⏱'; do
        grep -qF -e "$_tok" "$RUNNER" || _missing="$_missing $_tok"
    done
    [ -z "$_missing" ] || die_premise "判据词汇不在门实现里：$_missing —— 裁决器与 $RUNNER 已脱节"
    say "  前提自证：门实现词汇 ✅（$RUNNER）"
fi

# ── 取数（一律 awk，避开 `grep -c` + pipefail + SIGPIPE 的坑，缺陷 487）──
# A 路：汇总行的失败数
A=$(awk '
    /汇总：失败/ { if (match($0, /失败[ ]+[0-9]+[ ]+项/)) {
            s = substr($0, RSTART, RLENGTH); gsub(/[^0-9]/, "", s); n = s + 0 } }
    END { print (n == "") ? "NA" : n }
' "$LOG")

# B 路：失败行（❌ 与 ⏱ 都算）+ 红门名（同一趟 awk 取完，不另起管道）
B=$(awk '/^❌ |^⏱ / { c++ } END { print c + 0 }' "$LOG")
REDLIST=$(awk '/^❌ |^⏱ / { c++; if (c <= 12) {
        n = $2; sub(/（.*$/, "", n); sub(/^[[:space:]]+/, "", n)
        printf "%s ", n } }
    END { }' "$LOG")
REDLIST="${REDLIST% }"

# C 路：时序 TSV（可选第三路，判据最强）
C="NA"
if [ -n "$TIMES" ] && [ -f "$TIMES" ]; then
    C=$(awk -F'\t' '$2 != "OK" && $2 != "SKIP" { c++ } END { print c + 0 }' "$TIMES")
fi

say "  取数：汇总(A)=$A · 失败行(B)=$B · 时序非OK(C)=$C"

# ── ② 多路一致才裁决 ────────────────────────────────────────────────
if [ "$A" = "NA" ]; then
    echo "VERDICT=INCONSISTENT"
    say "  ⚠️ 门日志里没有汇总行（未跑完 / 被中止 / 日志被截断）"
    say "  ⇒ 不裁决。中止与判红是两件事：前者要查为什么停，后者要查哪扇门。"
    exit 2
fi

if [ "$A" = "0" ] && [ "$B" = "0" ] && { [ "$C" = "NA" ] || [ "$C" = "0" ]; }; then
    echo "VERDICT=GREEN"
    say "  ✅ 多路一致：汇总 0 失败 · 无失败行 · 时序无非OK"
    exit 0
fi

if [ "$A" != "0" ] && [ "$B" != "0" ]; then
    echo "VERDICT=RED"
    say "  ❌ $A 项失败（失败行 $B · 时序非OK $C）"
    say "  红门：$REDLIST"
    exit 1
fi

# 到这里都是「自相矛盾」—— 正是缺陷 495 的形状
echo "VERDICT=INCONSISTENT"
if [ "$A" != "0" ] && [ "$B" = "0" ]; then
    say "  ⚠️ 汇总说失败 $A 项，但日志里**一条失败行都没有**（红门清单为空）"
    say "  ⇒ 这不是「门红」，是**判据/日志的问题**。按缺陷 495 的处置：查判据串，别查门。"
elif [ "$A" = "0" ] && [ "$B" != "0" ]; then
    say "  ⚠️ 汇总说 0 失败，但日志里有 $B 条失败行：$REDLIST"
elif [ "$A" = "0" ] && [ "$C" != "NA" ] && [ "$C" != "0" ]; then
    say "  ⚠️ 汇总说 0 失败，但时序 TSV 里有 $C 条非 OK"
else
    say "  ⚠️ 多路取数互相矛盾（A=$A B=$B C=$C）"
fi
exit 2
