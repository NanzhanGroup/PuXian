#!/usr/bin/env bash
# ============================================================
# selfhost/gate_time_baseline.sh —— 门耗时基线生成器（M311 · A5 · M400 补回灌）
# ------------------------------------------------------------
# 为什么有它：
#   `check_gate_timeouts.sh`（A4）要判「某门的实测耗时是否已贴到上限」，而**耗时是实测数据**，
#   不许手抄（M212 缺陷 288 的纪律：**判据的输入必须可复算**）。本脚本从
#   `run_gates.sh` 的计时 TSV（`GATE_TSV`，列 = 门名 / 状态 / 秒 / rc）**生成**基线文件。
#
# 口径（写进文件头，便于日后复核）：
#   · 只收 `状态 != SKIP` 的行（SKIP 是「本轮没跑」，不是「耗时 0」）
#   · **丢掉 rc=124 的行**（M400 缺陷 763）：124 = 被上限砍断时的秒数，不是「这门需要多少」。
#     把它当实测喂进来 ⇒ 基线直接等于上限 ⇒ 下一轮 P2 判绿，缺陷被自己的基线掩盖。
#     要旧行为（仅自证用）：`--keep-timeout-rows`
#   · 同一门出现在多份输入里 ⇒ 取 **elapsed 最大** 的那一行（最大 = 最坏情况）
#   · 来源列写 `<label>`，逐行可追溯到哪一份 TSV
#   · `--retain <旧基线>`：**兜底**，只补「这批输入里没有的门」，**不参与取最大**
#     —— 为什么不能把旧基线当普通 `--from` 喂进来：
#       ① 形态不同（旧基线是 名/秒/状态/来源，计时 TSV 是 名/状态/秒/rc）⇒ 读错列，秒数变 0；
#       ② 即使列对齐，取最大会让「已修好的门」永远停在旧的最大值（m187 476s 修完仍判 83%）。
#       这两条都是 M400 实测踩到的，不是推的。
#
# 用法：
#   selfhost/gate_time_baseline.sh --from <tsv> --label <名> [--from … --label …] \
#                                  [--retain <旧基线>] [--out selfhost/gate_time_baseline.tsv]
#   selfhost/gate_time_baseline.sh --check [--out <file>]     # 只校验基线文件形态（只读）
#
# 退出码：0 成功 / 2 用法错 / 3 输入缺失或无数据 / 4 --check 发现形态问题
# ============================================================
set -uo pipefail
cd "$(cd "$(dirname "$0")/.." && pwd)"

OUT="selfhost/gate_time_baseline.tsv"
CHECK=0
SOURCES=()      # "文件<TAB>标签"
PENDING=""      # 等 --label 的输入文件
RETAIN=""       # 兜底旧基线（只补缺，不参与取最大）
KEEP_TO=0       # 1 = 保留 rc=124 行（仅自证/负控用）

while [ $# -gt 0 ]; do
    case "$1" in
        --from)
            [ -z "$PENDING" ] || { echo "❌ $PENDING 还没给 --label（每个 --from 必须紧跟 --label）" >&2; exit 2; }
            PENDING="${2:-}"; shift 2 ;;
        --label)
            [ -n "$PENDING" ] || { echo "❌ --label 必须紧跟在 --from <tsv> 之后" >&2; exit 2; }
            [ -n "${2:-}" ] || { echo "❌ --label 缺值" >&2; exit 2; }
            SOURCES+=("$PENDING"$'\t'"$2"); PENDING=""; shift 2 ;;
        --retain)   RETAIN="${2:-}"; shift 2 ;;
        --keep-timeout-rows) KEEP_TO=1; shift ;;
        --out)   OUT="${2:-}"; shift 2 ;;
        --check) CHECK=1; shift ;;
        -h|--help) sed -n '2,32p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
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
if [ -n "$RETAIN" ] && [ ! -f "$RETAIN" ]; then
    echo "❌ --retain 指向的文件不存在：$RETAIN" >&2; exit 3
fi

TMP="$(mktemp /tmp/gtb.XXXXXX)" || exit 3
trap 'rm -f "$TMP" "$TMP.raw"' EXIT

: > "$TMP.raw"
for s in "${SOURCES[@]}"; do
    f="${s%%$'\t'*}"; l="${s##*$'\t'}"
    awk -F'\t' -v lab="$l" -v keep_to="$KEEP_TO" '
        $1 == "" { next }
        $2 == "SKIP" { next }
        NF < 3 { next }
        # 缺陷 763：rc=124 = 被上限砍断的秒数，不是耗时 ⇒ 默认丢掉
        (keep_to == 0 && $4 == "124") { next }
        { printf "%s\t%s\t%s\t%s\n", $1, $3, $2, lab }
    ' "$f" >> "$TMP.raw"
done

[ -s "$TMP.raw" ] || {
    # 全 SKIP / 全 rc=124（后者=这一轮所有门都被砍断，理论上不该发生）。
    # 有 --retain 就**整份沿用旧基线**并在 stderr 出声：宁可沿用旧值 + 警告，
    # 不要 rc=3 让回灌断在半路（回灌断掉 = 下一次 P7 判红 = 又有实测没进基线）。
    if [ -n "$RETAIN" ] && [ -s "$RETAIN" ]; then
        echo "⚠️ 输入里没有可用行（全 SKIP / 全 rc=124）⇒ 整份沿用 --retain 的旧值：$RETAIN" >&2
    else
        echo "❌ 所有输入都没有可用行（全是 SKIP / 全是 rc=124？）且没有 --retain 可兜底" >&2
        exit 3
    fi
}

{
    echo "# ── 门耗时基线（**生成物，不要手改**）· M311 · selfhost/gate_time_baseline.sh ──"
    echo "#"
    echo "# 口径：只收非 SKIP 行；**丢 rc=124**（砍断值不是耗时）；同一门取 elapsed 最大者；来源列 = 该行的 --label。"
    echo "# 兜底：--retain 的旧基线只补「本批输入里没有的门」，不参与取最大（否则修好的门永远停在旧最大值）。"
    echo "# 用途：check_gate_timeouts.sh 判「实测耗时是否已贴到生效上限」（≥80% 判红）· P6 判覆盖 · P7 判回灌新鲜度。"
    echo "# 刷新：不用手跑 —— packaging/chain/release_postupgrade.sh 判绿后自动调 selfhost/refresh_gate_baseline.sh。"
    echo "#"
    echo "# 本次输入："
    for s in "${SOURCES[@]}"; do
        f="${s%%$'\t'*}"; l="${s##*$'\t'}"
        echo "#   - $l = $f（$(grep -vc '^#' "$f" 2>/dev/null || echo 0) 行）"
    done
    [ -n "$RETAIN" ] && echo "#   - retain（只补缺）= $RETAIN"
    echo "# 生成时间：$(date '+%Y-%m-%d %H:%M:%S %z')"
    echo "#"
    echo "# 门名	秒	状态	来源"
} > "$TMP"

# 每门取 max(秒)；同秒时优先非 FAIL（单跑 OK 比被砍断的 FAIL 更可信）。
# 兜底行（--retain）只在「本批输入里没有这扇门」时补进来。
awk -F'\t' -v rf="$RETAIN" '
    BEGIN {
        if (rf != "") {
            while ((getline line < rf) > 0) {
                if (line ~ /^#/ || line == "") continue
                n = split(line, a, "\t")
                if (n >= 3 && a[1] != "") {
                    rg[a[1]]  = a[2]
                    rst[a[1]] = a[3]
                    rsrc[a[1]] = (n >= 4 && a[4] != "") ? a[4] : "retained"
                }
            }
            close(rf)
        }
    }
    {
        g=$1; s=$2+0; st=$3; src=$4
        if (!(g in best) || s > best[g] || (s == best[g] && st == "OK" && bst[g] != "OK")) {
            best[g]=s; bst[g]=st; bsrc[g]=src
        }
    }
    END {
        for (g in best) printf "%s\t%d\t%s\t%s\n", g, best[g], bst[g], bsrc[g]
        for (g in rg) if (!(g in best)) printf "%s\t%d\t%s\t%s\n", g, rg[g]+0, rst[g], rsrc[g]
    }
' "$TMP.raw" | sort -t$'\t' -k1,1 >> "$TMP"

mv "$TMP" "$OUT"
n=$(grep -vc '^#' "$OUT" || true)
echo "✅ 已生成 $OUT：$n 门"
echo "   Top 5："
# ⚠️ 不用 `| head -5`：head 提前关管道 ⇒ sort 收 SIGPIPE ⇒ pipefail 下整脚本 rc=141
#    （M400 实测：生成其实成功了，却被调用方判成失败）。用 awk 读到底再截前 5 行。
grep -v '^#' "$OUT" | sort -t$'\t' -k2,2 -rn | awk 'NR<=5' | sed 's/^/     /'
