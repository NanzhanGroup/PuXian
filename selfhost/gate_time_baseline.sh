#!/usr/bin/env bash
# ============================================================
# selfhost/gate_time_baseline.sh —— 门耗时基线生成器（M311 · A5）
# ------------------------------------------------------------
# 为什么有它：
#   `check_gate_timeouts.sh`（A4）要判「某门的实测耗时是否已贴到上限」，而**耗时是实测数据**，
#   不许手抄（M212 缺陷 288 的纪律：**判据的输入必须可复算**）。本脚本从
#   `run_gates.sh` 的计时 TSV（`GATE_TSV`，列 = 门名 / 状态 / 秒 / rc）**生成**基线文件。
#
# 口径（写进文件头，便于日后复核）：
#   · 只收 `状态 != SKIP` 的行（SKIP 是「本轮没跑」，不是「耗时 0」）
#   · 同一门出现在多份输入里 ⇒ 取 **elapsed 最大** 的那一行
#     （最大 = 最坏情况；**超时行 rc=124 的值是「被砍断时的值」**，因此若同时有单跑实测，用后者更大者）
#   · 来源列写 `<label>`，逐行可追溯到哪一份 TSV
#
# 用法：
#   selfhost/gate_time_baseline.sh --from <tsv> --label <名> [--from <tsv> --label <名> …] \
#                                  [--out selfhost/gate_time_baseline.tsv]
#   selfhost/gate_time_baseline.sh --check [--out <file>]     # 只校验基线文件形态（只读）
#
# 退出码：0 成功 / 2 用法错 / 3 输入缺失或无数据 / 4 --check 发现形态问题
# ============================================================
set -uo pipefail
cd "$(cd "$(dirname "$0")/.." && pwd)"

OUT="selfhost/gate_time_baseline.tsv"
CHECK=0
SOURCES=()   # "文件<TAB>标签"
PENDING=""   # 等 --label 的输入文件

while [ $# -gt 0 ]; do
    case "$1" in
        --from)
            [ -z "$PENDING" ] || { echo "❌ $PENDING 还没给 --label（每个 --from 必须紧跟 --label）" >&2; exit 2; }
            PENDING="${2:-}"; shift 2 ;;
        --label)
            [ -n "$PENDING" ] || { echo "❌ --label 必须紧跟在 --from <tsv> 之后" >&2; exit 2; }
            [ -n "${2:-}" ] || { echo "❌ --label 缺值" >&2; exit 2; }
            SOURCES+=("$PENDING"$'\t'"$2"); PENDING=""; shift 2 ;;
        --out)   OUT="${2:-}"; shift 2 ;;
        --check) CHECK=1; shift ;;
        -h|--help) sed -n '2,30p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数: $1（--help 看用法）" >&2; exit 2 ;;
    esac
done
[ -z "$PENDING" ] || { echo "❌ $PENDING 缺 --label" >&2; exit 2; }

if [ "$CHECK" = 1 ]; then
    [ -f "$OUT" ] || { echo "❌ 基线文件不存在：$OUT" >&2; exit 3; }
    rows=$(grep -vc '^#' "$OUT" || true)
    bad=0
    # 形态：4 列（门名 / 秒 / 状态 / 来源）· 秒是正整数 · 门名非空
    while IFS=$'\t' read -r g s st src; do
        case "$g" in ''|'#'*) continue ;; esac
        # 秒数允许 0（一批门是亚秒级 —— 实测 `check_defect_refs`/`gate_registry` 等就是 0s，
        # 把它们判成「形态错」是判据自己太窄）
        if ! grep -qE '^[0-9]+$' <<< "$s"; then
            echo "❌ 形态错：$g 的秒数不是非负整数（「$s」）" >&2; bad=$((bad+1))
        fi
        [ -n "${st:-}" ] || { echo "❌ 形态错：$g 缺状态列" >&2; bad=$((bad+1)); }
        [ -n "${src:-}" ] || { echo "❌ 形态错：$g 缺来源列（无可追溯性）" >&2; bad=$((bad+1)); }
    done < "$OUT"
    [ "$bad" = 0 ] || exit 4
    echo "✅ 基线形态 OK：$rows 行（$OUT）"
    exit 0
fi

[ "${#SOURCES[@]}" -gt 0 ] || { echo "❌ 缺 --from（至少一份计时 TSV）" >&2; exit 2; }
for s in "${SOURCES[@]}"; do
    f="${s%%$'\t'*}"; l="${s##*$'\t'}"
    [ -f "$f" ] || { echo "❌ 输入不存在：$f" >&2; exit 3; }
    [ -n "$l" ] || { echo "❌ $f 缺 --label（来源列必须可追溯）" >&2; exit 3; }
done

TMP="$(mktemp /tmp/gtb.XXXXXX)" || exit 3
trap 'rm -f "$TMP" "$TMP.raw"' EXIT

: > "$TMP.raw"
for s in "${SOURCES[@]}"; do
    f="${s%%$'\t'*}"; l="${s##*$'\t'}"
    awk -F'\t' -v lab="$l" '
        $1 == "" { next }
        $2 == "SKIP" { next }
        NF < 3 { next }
        { printf "%s\t%s\t%s\t%s\n", $1, $3, $2, lab }
    ' "$f" >> "$TMP.raw"
done

[ -s "$TMP.raw" ] || { echo "❌ 所有输入都没有可用行（全是 SKIP？）" >&2; exit 3; }

{
    echo "# ── 门耗时基线（**生成物，不要手改**）· M311 · selfhost/gate_time_baseline.sh ──"
    echo "#"
    echo "# 口径：只收非 SKIP 行；同一门取 elapsed 最大者；来源列 = 该行的 --label。"
    echo "# 用途：check_gate_timeouts.sh 判「实测耗时是否已贴到生效上限」（≥80% 判红）。"
    echo "# 刷新时机：门的实际工作量变化后（如上游用例 128→238）跑一次全量门，再用本脚本重生成。"
    echo "#"
    echo "# 本次输入："
    for s in "${SOURCES[@]}"; do
        f="${s%%$'\t'*}"; l="${s##*$'\t'}"
        echo "#   - $l = $f（$(grep -vc '^#' "$f" 2>/dev/null || echo 0) 行）"
    done
    echo "# 生成时间：$(date '+%Y-%m-%d %H:%M:%S %z')"
    echo "#"
    echo "# 门名	秒	状态	来源"
} > "$TMP"

# 每门取 max(秒)；同秒时优先非 FAIL（单跑 OK 比被砍断的 FAIL 更可信）
awk -F'\t' '
    {
        g=$1; s=$2+0; st=$3; src=$4
        if (!(g in best) || s > best[g] || (s == best[g] && st == "OK" && bst[g] != "OK")) {
            best[g]=s; bst[g]=st; bsrc[g]=src
        }
    }
    END { for (g in best) printf "%s\t%d\t%s\t%s\n", g, best[g], bst[g], bsrc[g] }
' "$TMP.raw" | sort -t$'\t' -k1,1 >> "$TMP"

mv "$TMP" "$OUT"
n=$(grep -vc '^#' "$OUT" || true)
echo "✅ 已生成 $OUT：$n 门"
echo "   Top 5："
grep -v '^#' "$OUT" | sort -t$'\t' -k2,2 -rn | head -5 | sed 's/^/     /'
