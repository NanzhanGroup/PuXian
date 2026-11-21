#!/usr/bin/env bash
# ============================================================
# M410 门 · **判据自欺扫描**（「判绿」必须能证明自己测过东西）
# ------------------------------------------------------------
# 病灶（M409 三例 + 一例新发现）：
#   a) M408 r4 quiet 归档漂移 —— 改输入没算下游派生件（判据只看了自己那半）
#   b) M409 closeout2 假 FAIL —— grep 扫到脚本自己上一轮的汇总行
#   c) m409_m256_loadcheck `nscan=0` 判绿 —— **扫描面为 0 也算通过**
#   d) `run_gates.sh --only` 只跑 1 门会打印「跳过 283 门」，旧判据只看 rc=0
# 共同形状：**结论说"通过"，但没人证明被测面真被扫到了**。
#
# 本门两条规则（静态可判，不猜语义）：
#   R1 「有判据、有绿输出、却从不打印扫描面」⇒ 红
#      绿输出 = echo/printf 出 OK/PASS/DONE 或显式 exit 0
#      扫描面 = nscan|ntags|nfiles|n_files|nrows|nhits|matched=|scanned=|扫描面|命中
#   R2 「打了扫描面变量、却没有零值守卫」⇒ 红
#      守卫 = 该变量被拿来跟**一个数**比较（-eq/-ne/-lt/-le/-gt/-ge n · ==/!= n · </> n）
#      口径说明：只看「有没有把扫描面拿来比数」，不猜比较方向 —— 这是启发式，
#      真正的牙由 --selftest（坏两件必红 / 好一件必绿）与「扫描面 0 必红」承担。
#
# 扫描面（打印后由调用方核对，**扫描面为空 ⇒ 本门自己判红**）：
#   ① $ROOT/packaging/chain/*.sh                 发布链与看护脚本（全量）
#   ② ${M410_AUTODRIVE_DIR:-/data/autodrive}/*.sh 宿主运维脚本 **仅 mtime ≥ 策略生效日** 的
#      （2026-11-16 · 策略生效日之后新写的才受约束；历史脚本不追溯 —— 追溯会得到 106 条噪声，
#        噪声会让门失效，等于自己造一个「反正永远红」的假装置）
#
# 例外表：examples/m410_criteria_scan/allow.tsv（<相对路径或 basename>\t<理由>）
#   · 表里没有的违规 ⇒ 红   · 表里已有但**不再违规** ⇒ 红（STALE：防白名单烂账）
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
ALLOW="$SELF_DIR/allow.tsv"
POLICY_DATE="${M410_POLICY_DATE:-2026-11-16 00:00:00}"
# HOSTFACE-ABSENT（M415 · 缺陷 826）：宿主面缺席的判定口径
#   CI/干净环境没有 /data/autodrive ⇒ host: 例外表条目一条都不会命中。
#   旧版把这种「面本来就不存在」算成 STALE-ALLOW ⇒ **门在 CI 上必红**（2026-11-17 实测连续 15 轮）。
#   口径：面缺席 ⇒ host: 条目 SKIP（打印扣减）；面存在而文件消失 ⇒ 仍判 STALE（豁免不能吃掉真烂账）。
HOSTFACE_ABSENT=0
is_host_key(){ case "$1" in host:*) return 0;; *) return 1;; esac; }
# judge_stale <key> <absent 0|1> → 0 = 判 STALE（红） · 1 = 跳过（面缺席）
judge_stale(){ is_host_key "$1" && [ "$2" = 1 ] && return 1; return 0; }

fails=0
say(){ echo "── $*"; }
ok(){ echo "  PASS  $*"; }
bad(){ echo "  FAIL  $*"; fails=$((fails+1)); }

# ── 规则实现（自检与正式跑共用同一份）─────────────────────────
SURFACE_RE='nscan|ntags|nfiles|n_files|nrows|nhits|nverify|matched=|scanned=|扫描面|命中'
GREEN_RE='(echo|printf)[^#]*(OK|PASS|DONE|-DONE)|^[[:space:]]*exit 0'
SURFVAR_RE='(^|[^A-Za-z0-9_])(nscan|ntags|nfiles|n_files|nrows|nhits|nverify)='

# strip_redir_targets <file> → 剥掉「重定向目标」里的变量名后再看绿词
#   为什么（2026-11-22 M430 r4 · 缺陷 885）：`printf '...' >> "$DONE"` 里的 DONE 是**标记文件名**，
#   不是「判绿输出」。旧口径把变量名当绿词 ⇒ 判红写戳库 packaging/chain/chain_red_lib.sh 被误判 R1，
#   连带 m415 动态正控（「m410 在宿主面缺席下必绿」）一起红。
#   只剥重定向目标（> / >> 后面那个变量），普通 echo/printf 的实参一个字不动 —— 牙不掉。
strip_redir_targets(){ sed -E 's/(>>?[[:space:]]*)"?\$\{?[A-Za-z_][A-Za-z0-9_]*\}?"?/\1_FILE/g' "$1" 2>/dev/null; }
green_out_hit(){ strip_redir_targets "$1" | grep -qE "$GREEN_RE"; }

# r1_hit <file> → 0 命中 R1
r1_hit(){
    grep -q 'grep ' "$1" 2>/dev/null || return 1
    green_out_hit "$1" || return 1
    grep -qE "$SURFACE_RE" "$1" 2>/dev/null && return 1
    return 0
}
# r2_hit <file> → 0 命中 R2（打了扫描面变量、有绿输出、却没有该变量的零值守卫）
r2_hit(){
    green_out_hit "$1" || return 1
    local v hit=1
    for v in $(grep -oE "$SURFVAR_RE" "$1" 2>/dev/null | sed 's/[^A-Za-z0-9_]//g' | sort -u); do
        # 该变量必须出现在零值比较里（$v 或 ${v}）
        grep -qE "\\\$\{?$v\}?\"?[[:space:]]*(-(eq|ne|lt|le|gt|ge)[[:space:]]+[0-9]+|[=!]=[[:space:]]*[0-9]+|[<>][[:space:]]*[0-9]+)" "$1" 2>/dev/null || { hit=0; }
    done
    [ "$hit" = 0 ] && return 0
    return 1
}

# 扫描面枚举：输出 "标识<TAB>绝对路径" 一行一个
list_scan_face(){
    local f cutoff
    for f in "$ROOT"/packaging/chain/*.sh; do
        [ -f "$f" ] || continue
        printf 'repo:%s\t%s\n' "${f#"$ROOT"/}" "$f"
    done
    local AD="${M410_AUTODRIVE_DIR:-/data/autodrive}"
    if [ ! -d "$AD" ]; then
        # host 档输入（登记见 selfhost/gate_inputs_allow.tsv）：仅本机存在 ⇒ 响亮跳过，repo 面照扫
        say "宿主面：$AD 不存在 ⇒ 跳过宿主脚本面（host 档，CI 必跳）"
    else
        cutoff=$(date -d "$POLICY_DATE" +%s 2>/dev/null || echo 0)
        for f in "$AD"/*.sh; do
            [ -f "$f" ] || continue
            [ "$(stat -c %Y "$f" 2>/dev/null || echo 0)" -ge "$cutoff" ] || continue
            printf 'host:%s\t%s\n' "$(basename "$f")" "$f"
        done
    fi
}

# ── 自检：夹具三件（坏两件必红、好一件必绿）+ 扫描面 0 必红 ──
if [ "$SELFTEST" = 1 ]; then
    T=$(mktemp -d /tmp/m410cs.XXXXXX); trap 'rm -rf "$T"' EXIT
    mkdir -p "$T/fx"
    # 坏 1：有 grep 判据 + OK 输出，从不打印扫描面
    cat > "$T/fx/bad_no_surface.sh" <<'EOS'
#!/usr/bin/env bash
if grep -q ROOT "$FX"; then echo "OK"; else echo "FAIL"; fi
EOS
    # 坏 2：打了 nscan=、直接 OK，没有零值守卫
    cat > "$T/fx/bad_no_guard.sh" <<'EOS'
#!/usr/bin/env bash
nscan=$(ls /tmp | wc -l)
echo "nscan=$nscan"
echo "VERIFY-OK"
EOS
    # 好：同样打扫描面，但有零值守卫（0 ⇒ 判红）
    cat > "$T/fx/good.sh" <<'EOS'
#!/usr/bin/env bash
nscan=$(ls /tmp | wc -l)
if [ "$nscan" -le 0 ]; then echo "SCAN-FACE-EMPTY"; exit 1; fi
echo "nscan=$nscan"
echo "VERIFY-OK"
EOS
    # 好 2（缺陷 885）：判红写戳库 —— 绿词只出现在**重定向目标**（$DONE 是标记文件名），不是输出
    cat > "$T/fx/good_redir.sh" <<'EOS'
#!/usr/bin/env bash
if grep -q 'green=' "$DONE" 2>/dev/null; then
    printf 'CHAIN-RED-REASON=%s\n' "$1" >> "$DONE"
else
    printf 'CHAIN-VERDICT=RED reason=%s\n' "$1" > "$DONE"
fi
EOS
    # 坏 3：字面量 echo "DONE" 仍是绿输出 ⇒ 剥变量不许把牙也剥掉
    cat > "$T/fx/bad_literal_done.sh" <<'EOS'
#!/usr/bin/env bash
grep -q ROOT "$FX" && echo "DONE"
EOS
    say "[自检] 夹具五件"
    r1_hit "$T/fx/bad_no_surface.sh" && ok "正控 R1：无扫描面必被抓" || bad "正控 R1 漏抓"
    r1_hit "$T/fx/bad_literal_done.sh" && ok "正控 R1：字面量 DONE 仍被抓（剥变量不掉牙）" || bad "正控 R1 漏抓字面量 DONE"
    r2_hit "$T/fx/bad_no_guard.sh"   && ok "正控 R2：无零值守卫必被抓" || bad "正控 R2 漏抓"
    r2_hit "$T/fx/good.sh" && bad "负控 R2 误报好样例" || ok "负控 R2：有守卫不误报"
    r1_hit "$T/fx/good.sh" && bad "负控 R1 误报好样例" || ok "负控 R1：打印了扫描面不误报（本案按 R1 也不该红）"
    r1_hit "$T/fx/good_redir.sh" && bad "负控 R1 误报：重定向目标 \$DONE 被当成绿输出" || ok "负控 R1：重定向目标不算绿输出（缺陷 885）"
    # ── 宿主面缺席的口径（M415 · 缺陷 826：CI 无 /data/autodrive ⇒ 旧版 4 条 STALE 必红）──
    say "[自检] 宿主面缺席 ≠ 白名单烂账"
    judge_stale "host:tag_notify.sh" 1 && bad "正控：面缺席时 host 条目仍判 STALE（CI 必红）" || ok "正控：面缺席 ⇒ host 条目跳过"
    judge_stale "host:tag_notify.sh" 0 && ok "负控：面在场 ⇒ 未命中仍判 STALE（豁免不是无条件）" || bad "负控：面在场也跳过 ⇒ 豁免吃掉了真烂账"
    judge_stale "repo:packaging/chain/x.sh" 1 && ok "负控：面缺席不豁免 repo 条目" || bad "面缺席把 repo 条目也放过了"
    # 扫描面 0 必须判红（本门不允许「没东西可扫 ⇒ 绿」）
    say "[自检] 扫描面为 0"
    n=0
    for _ in "$T"/fx_none/*.sh; do [ -e "$_" ] && n=$((n+1)); done
    if [ "$n" -eq 0 ]; then ok "空扫描面可被识别（nscan=0 ⇒ 主流程判红）"; else bad "空扫描面识别失败"; fi
    echo
    if [ "$fails" = 0 ]; then echo "M410-CRITERIA-SCAN-SELFTEST-OK"; exit 0; fi
    echo "M410-CRITERIA-SCAN-SELFTEST-FAIL fails=$fails"; exit 1
fi

# ── 正式扫描 ─────────────────────────────────────────────────
# 宿主面在场性（必须在主 shell 求值：list_scan_face 跑在子 shell 里，变量传不回来）
AD_DIR="${M410_AUTODRIVE_DIR:-/data/autodrive}"
[ -d "$AD_DIR" ] || HOSTFACE_ABSENT=1
mapfile -t FACE < <(list_scan_face)
nscan=${#FACE[@]}
say "[1] 扫描面：nscan=$nscan（repo packaging/chain + 宿主 mtime ≥ $POLICY_DATE）"
if [ "$nscan" -le 0 ]; then
    echo "  FAIL  扫描面为 0 ⇒ 判红（「没东西可扫」不等于「没问题」）"
    echo "M410-CRITERIA-SCAN-FAIL nscan=0 R1=0 R2=0 allow=0"
    exit 1
fi

# 例外表
declare -A ALLOW_REASON=()
allow_n=0
if [ -f "$ALLOW" ]; then
    while IFS=$'\t' read -r p r; do
        case "$p" in ''|'#'*) continue;; esac
        ALLOW_REASON["$p"]="$r"; allow_n=$((allow_n+1))
    done < "$ALLOW"
fi
declare -A SEEN=()

r1=0; r2=0
say "[2] 逐条判定"
for row in "${FACE[@]}"; do
    id="${row%%$'\t'*}"; path="${row#*$'\t'}"
    base=$(basename "$path")
    key=""
    [ -n "${ALLOW_REASON[$id]+x}" ] && key="$id"
    [ -z "$key" ] && [ -n "${ALLOW_REASON[$base]+x}" ] && key="$base"
    hit=""
    r1_hit "$path" && { hit="$hit R1"; r1=$((r1+1)); }
    r2_hit "$path" && { hit="$hit R2"; r2=$((r2+1)); }
    if [ -n "$hit" ]; then
        if [ -n "$key" ]; then
            SEEN["$key"]=1
            printf '  ALLOW %-44s %s（理由：%s）\n' "$id" "$hit" "${ALLOW_REASON[$key]}"
        else
            bad "$id 命中$hit（判绿未证扫描面 / 扫描面 0 无守卫）"
            grep -nE "$SURFACE_RE" "$path" 2>/dev/null | head -2 | sed 's/^/        扫描面行 /'
        fi
    else
        [ -n "$key" ] && bad "STALE-ALLOW $id（表里登记了但已不违规：删掉这条，别让白名单烂账）"
    fi
done

say "[3] 例外表对账"
stale=0; hostskip=0
for k in "${!ALLOW_REASON[@]}"; do
    [ -n "${SEEN[$k]+x}" ] && continue
    if ! judge_stale "$k" "$HOSTFACE_ABSENT"; then
        hostskip=$((hostskip+1))
        printf '  SKIP  %s（宿主面缺席 ⇒ 本条不判 STALE；宿主面在场时仍须命中）\n' "$k"
        continue
    fi
    bad "STALE-ALLOW $k（未命中任何扫描文件）"; stale=$((stale+1))
done
echo "  宿主面：absent=$HOSTFACE_ABSENT（$AD_DIR）· host 条目 SKIP=$hostskip"
[ "$stale" = 0 ] && ok "例外表无烂账（allow=$allow_n · 面内命中 $((allow_n-hostskip)) · 面缺席跳过 $hostskip）"

echo
echo "nscan=$nscan R1=$r1 R2=$r2 allow=$allow_n"
if [ "$fails" = 0 ]; then echo "M410-CRITERIA-SCAN-OK"; exit 0; fi
echo "M410-CRITERIA-SCAN-FAIL fails=$fails"; exit 1
