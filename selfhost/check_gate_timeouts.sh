#!/usr/bin/env bash
# ============================================================
# selfhost/check_gate_timeouts.sh —— 门超时口径守卫（M311 · A4 · 缺陷 532）
# ------------------------------------------------------------
# 为什么有它（M310 实测）：
#   `upstream_tests` 的真实耗时 = 963s（空闲）/ 902s（4 路分片），而运行器的**默认上限 900s**
#   ⇒ 串行权威档一跑就 rc=124（D2 段2 唯一红）。而「本门该给 1800s」这份知识此前**只在宿主侧**
#   `/data/gatevm/shard_run.sh` 的两行硬编码里 —— 仓里没有任何记录，也**没有任何判据**看得见
#   「某门已经贴到上限」。⇒ 认知：**门的上限与门的实测耗时必须有判据对拍**，否则下一次顶穿
#   只是时间问题（M274 缺陷 480 的口径：一个会偶发判红的门等于没有门）。
#
# 判据（每条都配自证，见 --self-test）：
#   P1 **声明有效性**：注册源里每条 `gate_timeout <门名> <秒>` 的门必须在门清单里（悬空 ⇒ 红）。
#      为什么：声明写错名字 = 静默无效（这条声明从此不存在，而你以为设了）。
#   P2 **贴上限**：任一门「基线实测耗时 ≥ 80% × 生效上限」⇒ 红。
#      （含未声明、走默认 900s 的门 —— 这才是本轮缺陷的正面判据。）
#   P3 **声明裕度**：声明了超时的门必须给出 ≥1.5× 基线实测的裕度（否则「声明了也不够」）。
#   P4 **声明无基线** ⇒ ℹ️（不装作有数据）；**基线里有、门清单里没有** ⇒ ℹ️（改名/删门的残留）。
#   P5 **规模锚点**：门清单 ≥150 · 基线 ≥150 行 · 声明 ≥1 条（否则判红 —— 提取器坏了不能装绿）。
#
# 输入：
#   --registry <file>   默认 selfhost/gates.registry.sh
#   --baseline <file>   默认 selfhost/gate_time_baseline.tsv（由 gate_time_baseline.sh 生成）
#   --list-file <file>  门清单（自证用；默认调 `run_gates.sh --list`，该入口**零副作用**）
#   --default <秒>      默认上限（默认 900，与 run_gates.sh 的 GATE_TIMEOUT_DEFAULT 同源口径）
#   --no-near / --no-headroom   仅**自证/负控**用（关掉对应判据以证明「红确实来自它」）
#   --self-test         跑内置自证（含**动态**：假注册源声明 2s ⇒ 真被运行器执行）
# 退出码：0 绿 / 1 有违例 / 2 用法错 / 3 前提不成立（输入缺失/解析不出）
# ============================================================
set -uo pipefail
cd "$(cd "$(dirname "$0")/.." && pwd)"

REG="selfhost/gates.registry.sh"
BASE="selfhost/gate_time_baseline.tsv"
LISTF=""
DEFTMO=900
NEAR_ON=1
HEAD_ON=1
SELFTEST=0

while [ $# -gt 0 ]; do
    case "$1" in
        --registry)   REG="${2:-}"; shift 2 ;;
        --baseline)   BASE="${2:-}"; shift 2 ;;
        --list-file)  LISTF="${2:-}"; shift 2 ;;
        --default)    DEFTMO="${2:-}"; shift 2 ;;
        --no-near)    NEAR_ON=0; shift ;;
        --no-headroom) HEAD_ON=0; shift ;;
        --self-test)  SELFTEST=1; shift ;;
        -h|--help)    sed -n '2,36p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数: $1（--help 看用法）" >&2; exit 2 ;;
    esac
done
case "$DEFTMO" in ''|*[!0-9]*) echo "❌ --default 必须是整数（给的是「$DEFTMO」）" >&2; exit 2 ;; esac

PASS=0; FAILN=0
ok()  { PASS=$((PASS+1)); echo "  ✅ $*"; }
bad() { FAILN=$((FAILN+1)); echo "  ❌ $*"; }
info(){ echo "  ℹ️  $*"; }

# ---------- 提取器（唯一实现：自证也复用它）----------
# 注册源声明 → "门名<TAB>秒"（只看行首 gate_timeout，注释里的说明不算）
decl_of() {
    [ -f "$1" ] || return 3
    awk '/^[[:space:]]*gate_timeout[[:space:]]+/ { print $2 "\t" $3 }' "$1"
}
# 基线 → "门名<TAB>秒<TAB>状态<TAB>来源"
base_of() {
    [ -f "$1" ] || return 3
    awk -F'\t' '/^#/ { next } NF >= 2 { print $1 "\t" $2 "\t" ($3==""?"-":$3) "\t" ($4==""?"-":$4) }' "$1"
}
# 门清单（全仓口径：一个都不许少）
list_of() {
    if [ -n "$1" ]; then cat "$1"
    else ./selfhost/run_gates.sh --list; fi
}

# ---------- 主判据 ----------
check_all() {   # $1=registry $2=baseline $3=listfile(可空)
    local reg="$1" base="$2" lf="$3"
    local decl base_rows glist
    local fail0=$FAILN     # 进入时的失败计数（判返回码用）

    decl="$(decl_of "$reg")"   || { echo "❌ 前提不成立：注册源不可读/不存在（$reg）" >&2; return 3; }
    base_rows="$(base_of "$base")" || { echo "❌ 前提不成立：基线不可读/不存在（$base）" >&2; return 3; }
    glist="$(list_of "$lf")"   || { echo "❌ 前提不成立：门清单取不到" >&2; return 3; }
    glist="$(printf '%s\n' "$glist" | grep -v '^[[:space:]]*$' | sort -u)"

    local ng nb nd
    ng=$(printf '%s\n' "$glist" | grep -c . || true)
    nb=$(printf '%s\n' "$base_rows" | grep -c . || true)
    nd=$(printf '%s\n' "$decl" | grep -c . || true)

    # ── P5 规模锚点（先判：提取器坏了，后面所有结论都不算数）──
    echo "── P5 规模锚点 ──"
    local anchor_bad=0
    [ "$ng" -ge 150 ] || { bad "门清单只有 $ng 门（下限 150）—— 提取器坏了？"; anchor_bad=1; }
    [ "$nb" -ge 150 ] || { bad "基线只有 $nb 行（下限 150）—— 基线陈旧或生成器坏了"; anchor_bad=1; }
    #   ⚠️ 「声明数为 0」**不判红**（否则 P2/P3 永远跑不到，而 P2 才是本轮缺陷的正面判据）。
    #      提取器是否失效改用**双计数器对拍**（下面 P5b）—— 那才是精确判据：
    #      「源码里有几条 gate_timeout 行」与「解析出几条」不相等 ⇒ 提取器漂了。
    local nd_raw
    nd_raw=$(awk '/^[[:space:]]*gate_timeout([[:space:]]|$)/ { n++ } END { print n+0 }' "$reg")
    if [ "$nd_raw" != "$nd" ]; then
        bad "声明提取器漂移：源码 $nd_raw 行 gate_timeout ⇄ 解析出 $nd 条"
        anchor_bad=1
    fi
    [ "$anchor_bad" = 0 ] && ok "门清单 $ng 门 · 基线 $nb 行 · 声明 $nd 条（提取 1:1 对拍）"
    [ "$anchor_bad" = 0 ] || return 1

    # ── P1 声明有效性 ──
    echo "── P1 声明有效性（声明的门必须存在）──"
    local dname dsec hit=0
    while IFS=$'\t' read -r dname dsec; do
        [ -n "$dname" ] || continue
        case "$dsec" in ''|*[!0-9]*) bad "声明 $dname 的秒数非法：「$dsec」"; hit=$((hit+1)); continue ;; esac
        if ! printf '%s\n' "$glist" | grep -qxF "$dname"; then
            bad "声明 $dname $dsec 秒 —— **门清单里没有这扇门**（悬空声明 = 静默无效）"
            hit=$((hit+1))
        fi
    done <<< "$decl"
    [ "$hit" = 0 ] && ok "$nd 条声明全部指向存在的门"

    # ── P2 贴上限（含走默认档的门）──
    echo "── P2 裕度（实测 ≥80% 生效上限 ⇒ 红）──"
    local g s st src cap pct maxg="" maxp=0 near=0
    while IFS=$'\t' read -r g s st src; do
        [ -n "$g" ] || continue
        cap="$DEFTMO"
        local dec
        dec="$(printf '%s\n' "$decl" | awk -F'\t' -v n="$g" '$1==n{print $2; exit}')"
        [ -n "$dec" ] && cap="$dec"
        [ "$cap" -gt 0 ] || continue
        pct=$(( s * 100 / cap ))
        [ "$pct" -gt "$maxp" ] && { maxp=$pct; maxg="$g ${s}s/${cap}s"; }
        if [ "$pct" -ge 80 ]; then
            if [ "$NEAR_ON" = 1 ]; then
                bad "**$g** 实测 ${s}s / 上限 ${cap}s = ${pct}%（≥80%）· 来源 $src"
                near=$((near+1))
            fi
        fi
    done <<< "$base_rows"
    if [ "$near" = 0 ]; then
        if [ "$NEAR_ON" = 1 ]; then
            ok "无门贴上限（最高：${maxg:-无数据} = ${maxp}%，阈值 80%）"
        else
            info "P2 已被 --no-near 关闭（仅自证用）"
        fi
    fi

    # ── P3 声明档裕度 ≥1.5× ──
    echo "── P3 声明裕度（≥1.5× 基线实测）──"
    local hbad=0
    while IFS=$'\t' read -r dname dsec; do
        [ -n "$dname" ] || continue
        local bsec bden ratio100
        bsec="$(printf '%s\n' "$base_rows" | awk -F'\t' -v n="$dname" '$1==n{print $2; exit}')"
        if [ -z "$bsec" ]; then
            info "$dname 声明 $dsec 秒，但**基线里没有它的实测值** ⇒ 裕度不可判（刷新基线后再看）"
            continue
        fi
        bden="$bsec"; [ "$bden" -gt 0 ] || bden=1     # 0 秒门：用 1 防除零，裕度必然 ≥1.5×
        ratio100=$(( dsec * 100 / bden ))
        if [ "$ratio100" -lt 150 ]; then
            if [ "$HEAD_ON" = 1 ]; then
                bad "**$dname** 声明 ${dsec}s / 实测 ${bsec}s = $(( ratio100 / 100 )).$(( (ratio100 % 100) / 10 ))×（< 1.5×）—— 声明了也不够"
                hbad=$((hbad+1))
            fi
        else
            ok "$dname：${bsec}s / ${dsec}s = $(( ratio100 / 100 )).$(( (ratio100 % 100) / 10 ))×（≥1.5×）"
        fi
    done <<< "$decl"
    if [ "$HEAD_ON" = 0 ]; then info "P3 已被 --no-headroom 关闭（仅自证用）"; fi

    # ── P4 残留（只 ℹ️，不判红）──
    echo "── P4 残留登记（只提示）──"
    local stale=0
    while IFS=$'\t' read -r g s st src; do
        [ -n "$g" ] || continue
        printf '%s\n' "$glist" | grep -qxF "$g" || { info "基线里的 $g 不在门清单中（改名/删门的残留？）"; stale=$((stale+1)); }
    done <<< "$base_rows"
    [ "$stale" = 0 ] && ok "基线 ⇄ 门清单：无残留"

    # ⚠️ 判据函数的返回码必须反映「本轮有没有新增失败」——
    #   否则自证会全绿（每个 fixture 都"通过"），而那正是 M289/M290 记过的「判据恒真」形态。
    [ "$FAILN" -gt "$fail0" ] && return 1
    return 0
}

# ---------- 自证 ----------
selftest() {
    local W; W="$(mktemp -d /tmp/m311tmo.XXXXXX)" || exit 3
    #   ⚠️ 自证会**复用** check_all ⇒ 它会改全局 PASS/FAILN。必须存/还原，
    #      否则自证的"失败 fixture"会被算进真判的汇总（首版实测：汇总 22/6 而真判其实全绿）。
    local _p0=$PASS _f0=$FAILN
    local st_pass=0 st_fail=0
    st_ok(){ st_pass=$((st_pass+1)); echo "  ✅ $*"; }
    st_no(){ st_fail=$((st_fail+1)); echo "  ❌ $*"; }

    # 造一份 160 门的清单（过 P5 下限）
    seq 1 160 | sed 's/^/g/' > "$W/list.txt"

    # S1 正常：一条声明 1800，基线里该门 963 ⇒ 1.87×，其余门远低于 80%
    {
        echo "gate_timeout g1 1800"
        for i in $(seq 1 160); do echo -e "g$i\t10\tOK\ts"; done | sed 's/^g1\t10/g1\t963/'
    } > "$W/base_ok.tsv"
    printf 'gate_timeout g1 1800\n' > "$W/reg_ok.sh"
    if out="$(check_all "$W/reg_ok.sh" "$W/base_ok.tsv" "$W/list.txt" 2>&1)"; then st_ok "S1 正常档 rc=0"; else st_no "S1 正常档应 rc=0，实际 rc=$?：$(echo "$out" | grep '❌' | head -2)"; fi

    # S2 悬空声明 ⇒ 红
    printf 'gate_timeout ghost 1800\n' > "$W/reg_ghost.sh"
    if check_all "$W/reg_ghost.sh" "$W/base_ok.tsv" "$W/list.txt" > "$W/s2.log" 2>&1; then
        st_no "S2 悬空声明应判红"
    else
        grep -q '没有这扇门' "$W/s2.log" && st_ok "S2 悬空声明判红且指名" || st_no "S2 红但没指名"
    fi

    # S3 贴上限（864/900 = 96%）⇒ 红；且 --no-near 时不再红（证明红来自 P2）
    { for i in $(seq 1 160); do if [ "$i" = 2 ]; then echo -e "g$i\t864\tOK\ts"; else echo -e "g$i\t10\tOK\ts"; fi; done; } > "$W/base_near.tsv"
    if check_all "$W/reg_ok.sh" "$W/base_near.tsv" "$W/list.txt" > "$W/s3.log" 2>&1; then
        st_no "S3 贴上限应判红"
    else
        grep -q 'g2.*864s / 上限 900s = 96%' "$W/s3.log" && st_ok "S3 贴上限判红且指名" || st_no "S3 红但没指名"
    fi
    printf 'gate_timeout g1 1800\n' > "$W/reg_ok.sh"
    # 判据自伤：**关掉 P2**（NEAR_ON=0）后同一 fixture 必须不再红 ⇒ 证明 S3 的红来自 P2 本身
    NEAR_ON=0
    if check_all "$W/reg_ok.sh" "$W/base_near.tsv" "$W/list.txt" > "$W/s3b.log" 2>&1; then
        st_ok "S3b 判据自伤：关掉 P2 ⇒ 同 fixture 不再红"
    else
        st_no "S3b 关掉 P2 仍红（说明红不只来自 P2）"
    fi
    NEAR_ON=1

    # S4 声明裕度不足（1000 / 963 = 1.03×）
    printf 'gate_timeout g1 1000\n' > "$W/reg_tight.sh"
    if check_all "$W/reg_tight.sh" "$W/base_ok.tsv" "$W/list.txt" > "$W/s4.log" 2>&1; then
        st_no "S4 裕度不足应判红"
    else
        grep -q '声明了也不够' "$W/s4.log" && st_ok "S4 裕度不足判红" || st_no "S4 红但没说明裕度"
    fi

    # S5 声明无基线 ⇒ 只 ℹ️（不红）
    #   ⚠️ fixture 必须自洽：若这里留着 base_ok 的 g1=963 而本 fixture 不给 g1 声明，
    #      P2 会（正确地）判 g1 贴默认上限 900 —— 那就变成在测 P2 而不是测 S5。
    printf 'gate_timeout g160 1800\n' > "$W/reg_nobase.sh"
    { for i in $(seq 1 160); do printf 'g%s\t10\tOK\ts\n' "$i"; done; } > "$W/base_all10.tsv"
    awk -F'\t' '$1 != "g160"' "$W/base_all10.tsv" > "$W/base_nog160.tsv"
    if check_all "$W/reg_nobase.sh" "$W/base_nog160.tsv" "$W/list.txt" > "$W/s5.log" 2>&1; then
        grep -q '没有它的实测值' "$W/s5.log" && st_ok "S5 声明无基线 ⇒ ℹ️ 且不红" || st_no "S5 绿但没提示"
    else
        st_no "S5 声明无基线不该判红"
    fi

    # S6 规模锚点（清单只有 3 门 ⇒ 提取器坏）
    head -3 "$W/list.txt" > "$W/list_small.txt"
    if check_all "$W/reg_ok.sh" "$W/base_ok.tsv" "$W/list_small.txt" > "$W/s6.log" 2>&1; then
        st_no "S6 规模锚点应判红"
    else
        grep -q '门清单只有 3 门' "$W/s6.log" && st_ok "S6 规模锚点判红" || st_no "S6 红但没指名锚点"
    fi

    # S7 前提缺失 ⇒ rc=3（不是「绿」）
    check_all "$W/nonexistent.sh" "$W/base_ok.tsv" "$W/list.txt" > "$W/s7.log" 2>&1
    [ "$?" = 3 ] && st_ok "S7 缺注册源 ⇒ rc=3（不许当绿）" || st_no "S7 缺输入时的退出码不是 3"

    # S8 ⭐ **动态**：注册源的声明真的被执行器采用（假注册源声明 2s + sleep 30）
    cat > "$W/reg_dyn.sh" <<'EOS'
step "dyn"
gate_timeout dyn_probe 2
run dyn_probe bash -c 'sleep 30'
EOS
    GATE_REGISTRY="$W/reg_dyn.sh" GATE_TSV="$W/dyn.tsv" \
    PX_GATE_PIDLOCK="$W/dyn.lock" PX_GATE_LOCK="$W/dyn.glock" \
        ./selfhost/run_gates.sh --allow-dirty > "$W/s8.log" 2>&1
    if grep -q '上限 2s · 来源 注册源' "$W/s8.log" && grep -q 'dyn_probe	FAIL	2	124' "$W/dyn.tsv"; then
        st_ok "S8 动态：注册源声明 2s 真被运行器执行（rc=124）"
    else
        st_no "S8 动态：声明没被执行器采用（见 $W/s8.log）"
    fi

    echo "══ 自证：$st_pass 通过 / $st_fail 失败 ══"
    PASS=$_p0; FAILN=$_f0      # 还原：自证不参与真判的计数
    rm -rf "$W"
    trap - EXIT
    [ "$st_fail" = 0 ] || return 1
    return 0
}

# ── 自证：**默认档也跑**（与 check_gate_paths / check_shell_contract 同口径：门自带自证）──
#   为什么：本判据的输入全部来自「提取器」（注册源 / 基线 / 门清单）。提取器一旦坏了，
#   判据会**恒绿**（空集 ⊇ 空集）—— 那比判红危险得多（M212 缺陷 288 / M213 的教训）。
#   `--self-test` 只跑自证；默认档 = 自证 + 真判。
selftest; selftest_rc=$?
if [ "$SELFTEST" = 1 ]; then exit "$selftest_rc"; fi
if [ "$selftest_rc" != 0 ]; then
    echo "❌ 自证未全通过 ⇒ 本门的判据可信度不成立（下面照跑真判，但结论一律按红处理）"
    FAILN=$((FAILN+1))
fi

echo ""
echo "══ M311 门超时守卫（注册源声明 ⇄ 实测基线）══"
check_all "$REG" "$BASE" "$LISTF"; rc=$?
echo ""
if [ "$rc" = 0 ] && [ "$FAILN" = 0 ]; then
    echo "══ 汇总：$PASS 通过 / 0 失败 ══"
    echo "M311-TIMEOUTS-OK"
    exit 0
fi
echo "══ 汇总：$PASS 通过 / $FAILN 失败 ══"
echo "   口径：docs/GATE_TIMEOUTS.md · 基线刷新：selfhost/gate_time_baseline.sh"
exit 1
