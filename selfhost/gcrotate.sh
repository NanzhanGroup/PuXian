#!/usr/bin/env bash
# ============================================================
# selfhost/gcrotate.sh —— GC 压力筛**分批轮转**驱动器（M291）
# ------------------------------------------------------------
# 为什么需要它（M207 待办 · M290 结论的下一步）：
#   M207 把压力筛做成「可重复 · 可分批 · 可复现」的工具，但它**仍然要人手动跑**
#   ——「还剩多少没扫」依旧没人回答。M290 复跑 136 轮 0 异常，结论是
#   **瓶颈在「复现」而不是「取证」** ⇒ 下一步该做的是「让它自己冒出来」。
#
# 机制（把「一次性人工筛」变成「每轮全量门自动推进一批」）：
#   · 台账 `$GCROTATE_STATE` 记「下一批」；每次 `--next` 跑一批、批号 +1（模 N）
#   · 覆盖记录 `$GCROTATE_COVERED`：累计「已跑过的候选」；`covered ⊇ 全量` ⇒
#     rounds++ 并清空（进入下一圈）⇒ 「距全量覆盖还差几批」从此是**实测数据**
#   · 单轮成本 = 全量 / N（本机实测 N=20 时 70–220s，取决于该批含多少超时语料）
#
# ⚠️ 判据的语义（别误读）：门跑「本批」⇒ 门绿 = **本批**无 FAIL_*，不是「全量绿」。
#   全量覆盖靠**轮转累积**。所以：本文件**不缓存**「上次 PASS」的候选 ——
#   压力筛的价值恰恰是「同一个候选**再跑一次**看是否还一致」（= 回归检测），
#   缓存会让它退化成「只扫没见过的新语料」，那正是它要防的东西。
#
# 用法：
#   selfhost/gcrotate.sh            # = --next：跑下一批，推进台账
#   selfhost/gcrotate.sh --status   # 只打印进度（**无副作用**）
#   selfhost/gcrotate.sh --reset    # 重置台账与覆盖记录
#   selfhost/gcrotate.sh --batch K/N  # 强制跑指定批（**不推进台账**，复现用）
#   selfhost/gcrotate.sh --only PAT   # 限定候选集（自证用；**全量的定义随之收窄**）
#
# 环境：
#   GCROTATE_N=20                    批数
#   GCROTATE_STATE=/tmp/gcrotate.state
#   GCROTATE_RESULTS=/tmp/gcrotate.results.tsv   （只追加）
#   GCROTATE_COVERED=/tmp/gcrotate.covered.list
#   GCROTATE_KNOWN / GCROTATE_SLOW   透传 gcstress_sweep.sh 的两张表
#   GCROTATE_EXTRA                   额外参数（空格分隔，如 "--t-stress 30"）
#
# 退出码：
#   0  本批无 FAIL_*（STIMEOUT / NONDET 不算红，但记账）
#   1  本批出现 FAIL_*   ← 真信号（真缺陷或假阳，**必须人工定性**）
#   2  参数错        3  台账损坏        4  依赖缺失
# ============================================================
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
cd "$ROOT" || exit 9
# ⚠️ comm 的「有序」判定按**当前 locale** —— 不固定 LC_ALL 会在 C 环境排序的清单上
#    报 "file 2 is not in sorted order"（实测），而结果**仍然是对的** ⇒ 属于**静默噪声**。
export LC_ALL=C LANG=C

SWEEP="$HERE/gcstress_sweep.sh"
N="${GCROTATE_N:-20}"
STATE="${GCROTATE_STATE:-/tmp/gcrotate.state}"
RESULTS="${GCROTATE_RESULTS:-/tmp/gcrotate.results.tsv}"
COVERED="${GCROTATE_COVERED:-/tmp/gcrotate.covered.list}"
KNOWN="${GCROTATE_KNOWN:-examples/m207_gcstress/KNOWN.tsv}"
SLOW="${GCROTATE_SLOW:-examples/m207_gcstress/SLOW.tsv}"
EXTRA="${GCROTATE_EXTRA:-}"

MODE=next
FORCE=""
ONLY=""
while [ $# -gt 0 ]; do
    case "$1" in
        --next)    MODE=next ;;
        --status)  MODE=status ;;
        --reset)   MODE=reset ;;
        --batch)   FORCE="${2:-}"; shift ;;
        --only)    ONLY="${2:-}"; shift ;;
        -h|--help) sed -n '2,44p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "gcrotate: 未知选项 '$1'" >&2; exit 2 ;;
    esac
    shift
done

[ -x "$SWEEP" ] || { echo "gcrotate: 依赖缺失：$SWEEP 不可执行" >&2; exit 4; }

# 公共参数（`--only` 必须经数组**带引号**传：直接拼进字符串会被 shell 的 glob 提前展开）
SW=()
[ -n "$ONLY" ] && SW+=(--only "$ONLY")

# 台账（简单 KV，缺省即初始化 —— 「不存在」不是错误，「损坏」才是）
K=1; ROUNDS=0; RUNS=0
read_state() {
    [ -f "$STATE" ] || return 0
    local v
    v="$(awk -F= '$1=="batch"{print $2; exit}'  "$STATE" 2>/dev/null)"
    [ -n "$v" ] && K="$v"
    v="$(awk -F= '$1=="rounds"{print $2; exit}' "$STATE" 2>/dev/null)"
    [ -n "$v" ] && ROUNDS="$v"
    v="$(awk -F= '$1=="runs"{print $2; exit}'   "$STATE" 2>/dev/null)"
    [ -n "$v" ] && RUNS="$v"
    v="$(awk -F= '$1=="n"{print $2; exit}'      "$STATE" 2>/dev/null)"
    [ -n "$v" ] && N="$v"
}

# 合法性：**损坏必须响亮**（绝不静默回退到 1 —— 那会把「台账坏了」伪装成「第一圈」）
check_state() {
    case "$N" in ''|*[!0-9]*|0) echo "gcrotate: 台账损坏：n='$N' 不是正整数（$STATE）" >&2; exit 3 ;; esac
    [ "$N" -ge 1 ] || { echo "gcrotate: 台账损坏：n=$N < 1" >&2; exit 3; }
    case "$K" in ''|*[!0-9]*) echo "gcrotate: 台账损坏：batch='$K' 不是整数（$STATE）" >&2; exit 3 ;; esac
    if [ "$K" -lt 1 ] || [ "$K" -gt "$N" ]; then
        echo "gcrotate: 台账损坏：batch=$K 越界（应在 1..$N）（$STATE）" >&2; exit 3
    fi
    case "$ROUNDS" in ''|*[!0-9]*) echo "gcrotate: 台账损坏：rounds='$ROUNDS'" >&2; exit 3 ;; esac
    case "$RUNS"   in ''|*[!0-9]*) echo "gcrotate: 台账损坏：runs='$RUNS'"     >&2; exit 3 ;; esac
}

write_state() {   # 原子写（先临时文件再 mv）
    local t="$STATE.tmp.$$"
    { printf 'batch=%s\n' "$1"; printf 'n=%s\n' "$N"
      printf 'rounds=%s\n' "$2"; printf 'runs=%s\n' "$3"
      printf 'last_batch=%s\n' "$4"
      printf 'last_run=%s\n' "$(date -Is)"
    } > "$t" && mv -f "$t" "$STATE"
}

# ⚠️ 带 --only：全量的定义必须与「跑的那一集合」一致
#   （否则 --only 档下 covered ⊂ 全量 恒成立 ⇒「一圈完成」永远不触发）
full_list() { "$SWEEP" --list "${SW[@]+"${SW[@]}"}" 2>/dev/null; }

# 覆盖率：|covered ∩ 全量| / |全量|
progress_line() {
    local full="$1" nfull nint
    nfull="$(wc -l < "$full")"
    if [ -f "$COVERED" ]; then
        nint="$(comm -12 <(LC_ALL=C sort -u "$COVERED") <(LC_ALL=C sort -u "$full") | wc -l)"
    else
        nint=0
    fi
    printf '%s %s\n' "$nint" "$nfull"
}

read_state
check_state

# ---- --status：**无副作用**（不建台账、不建覆盖记录）----
if [ "$MODE" = status ]; then
    _W="$(mktemp -d "${TMPDIR:-/tmp}/gcrot.st.XXXXXX")"
    trap 'rm -rf "$_W"' EXIT
    full_list > "$_W/full"
    nint=0; nfull=0
    read -r nint nfull <<EOF
$(progress_line "$_W/full")
EOF
    left=$(( nfull - nint ))
    per=$(( nfull / N )); [ "$per" -ge 1 ] || per=1
    batches=$(( (left + per - 1) / per ))
    [ "$left" -le 0 ] && batches=0
    printf 'GCROTATE-STATUS next=%s/%s rounds=%s runs=%s covered=%s/%s left=%s 约需 %s 批\n' \
        "$K" "$N" "$ROUNDS" "$RUNS" "$nint" "$nfull" "$left" "$batches"
    exit 0
fi

if [ "$MODE" = reset ]; then
    rm -f "$STATE" "$COVERED"
    : > "$RESULTS"
    echo "GCROTATE-RESET 台账已重置（state=$STATE covered=$COVERED results=$RESULTS）"
    exit 0
fi

# ---- --next（默认）：跑一批 ----
W="$(mktemp -d "${TMPDIR:-/tmp}/gcrot.XXXXXX")"
trap 'rm -rf "$W"' EXIT

FORCED=0
if [ -n "$FORCE" ]; then
    FK="${FORCE%%/*}"; FN="${FORCE##*/}"
    case "$FK$FN" in *[!0-9]*) echo "gcrotate: --batch 需要 K/N 形式（收到 '$FORCE'）" >&2; exit 2 ;; esac
    [ "$FN" -ge 1 ] || { echo "gcrotate: --batch N 必须 ≥ 1" >&2; exit 2; }
    [ "$FK" -ge 1 ] && [ "$FK" -le "$FN" ] || { echo "gcrotate: --batch K 越界（1..$FN）" >&2; exit 2; }
    K="$FK"; N="$FN"; FORCED=1
fi

full_list > "$W/full" || { echo "gcrotate: 取候选清单失败" >&2; exit 4; }
nfull="$(wc -l < "$W/full")"
[ "$nfull" -ge 1 ] || { echo "gcrotate: 候选清单为空（examples 目录异常？）" >&2; exit 4; }

t0=$SECONDS
# shellcheck disable=SC2086
# shellcheck disable=SC2086
"$SWEEP" --batch "$K/$N" --out "$W/out.tsv" --quiet \
    --known "$KNOWN" --slow "$SLOW" "${SW[@]+"${SW[@]}"}" $EXTRA 2>"$W/sweep.err"
rc=$?
el=$((SECONDS - t0))
# sweep 的 rc 只有 0/1 是有意义的；其它值 ⇒ **响亮**（别把工具崩了当「本批绿」）
if [ "$rc" -gt 1 ]; then
    echo "gcrotate: gcstress_sweep.sh 异常退出 rc=$rc（不是 0/1）" >&2
    tail -20 "$W/sweep.err" >&2
    exit 4
fi

ncand="$(wc -l < "$W/out.tsv" 2>/dev/null || echo 0)"
cnt() { awk -F'\t' -v t="$1" '$1==t' "$W/out.tsv" 2>/dev/null | wc -l; }
npass=$(cnt PASS); nskipn=$(cnt SKIP_NORM); nskipk=$(cnt SKIP_KNOWN)
nstim=$(cnt STIMEOUT); nnond=$(cnt NONDET); nbuild=$(cnt BUILDFAIL)
nfail=$(awk -F'\t' '$1 ~ /^FAIL_/' "$W/out.tsv" 2>/dev/null | wc -l)

# 累积结果（只追加；不覆写 ⇒ 趋势可回看）
{
    printf '# batch %s/%s %s cand=%s fail=%s elapsed=%ss\n' "$K" "$N" "$(date -Is)" "$ncand" "$nfail" "$el"
    cat "$W/out.tsv"
} >> "$RESULTS"

# ---- 覆盖记账（仅真跑推进；--batch 强制档不推进，避免污染轮转进度）----
closed=0
if [ "$FORCED" != 1 ]; then
    "$SWEEP" --list --batch "$K/$N" "${SW[@]+"${SW[@]}"}" 2>/dev/null > "$W/batch.list"
    if [ -s "$W/batch.list" ]; then
        { [ -f "$COVERED" ] && cat "$COVERED"; cat "$W/batch.list"; } | LC_ALL=C sort -u > "$W/cov.new"
        mv -f "$W/cov.new" "$COVERED"
    fi
    nint=0; nfull2=0
    read -r nint nfull2 <<EOF
$(progress_line "$W/full")
EOF
    if [ "$nint" -ge "$nfull2" ]; then
        closed=1
        ROUNDS=$((ROUNDS + 1))
        : > "$COVERED"
        nint=0
    fi
    RUNS=$((RUNS + 1))
    next=$(( K % N + 1 ))
    write_state "$next" "$ROUNDS" "$RUNS" "$K"
else
    nint=0; nfull2=0
    read -r nint nfull2 <<EOF
$(progress_line "$W/full")
EOF
    next="$K"
fi

printf 'GCROTATE-RESULT batch=%s/%s cand=%s pass=%s skip_norm=%s skip_known=%s stim=%s nondet=%s build=%s fail=%s elapsed=%ss\n' \
    "$K" "$N" "$ncand" "$npass" "$nskipn" "$nskipk" "$nstim" "$nnond" "$nbuild" "$nfail" "$el"
printf 'GCROTATE-PROGRESS covered=%s/%s round=%s runs=%s next=%s/%s%s\n' \
    "$nint" "$nfull2" "$ROUNDS" "$RUNS" "$next" "$N" \
    "$( [ "$closed" = 1 ] && echo ' 一圈完成!' )"

if [ "$nfail" -gt 0 ]; then
    echo
    echo "❌ 本批出现 FAIL_*（$nfail 条）—— 需人工定性：真缺陷 ⇒ 立编号；假阳 ⇒ 进 KNOWN/SLOW 表"
    awk -F'\t' '$1 ~ /^FAIL_/ { printf "   %s  %s  %s\n", $1, $2, $3 }' "$W/out.tsv"
    echo "   复现：selfhost/gcrotate.sh --batch $K/$N     （同批再跑一次）"
    echo "   单点：selfhost/gcstress_sweep.sh --only '<路径>' --t-norm 10 --t-stress 60"
    exit 1
fi
[ "$nstim" -gt 0 ] && {
    echo "ℹ️  本批有 $nstim 条 STIMEOUT（O(n²) 慢语料，**不是缺陷**）—— 实测定性后可进 SLOW.tsv 给足时间："
    awk -F'\t' '$1=="STIMEOUT" { printf "   %s\n", $2 }' "$W/out.tsv"
}
exit "$rc"
