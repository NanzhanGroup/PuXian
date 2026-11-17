#!/usr/bin/env bash
# ============================================================
# M415 门 · **干净环境自证**（「本地绿」不许靠这台机器恰好有什么）
# ------------------------------------------------------------
# 病灶（2026-11-18 04:2x 实测，缺陷 826）：
#   GitHub 上 PuXian CI **连续 15 轮红**（最早可查到 11-14），而本地全量门 289/289 绿。
#   逐门定性后四条同形：**判据在「东月这台机器」上成立，在干净环境里不成立**。
#     · m410_criteria_scan  无 /data/autodrive ⇒ 4 条 host 例外被判 STALE ⇒ 必红
#     · m389_gh_auth        无 /data/pat.md ⇒ 负控只打了「带令牌」那条分支 ⇒ 负控假红
#     · m256_eintr          [5] 两档不一致，但 diff 行被 CI 注解的 grep 筛掉 ⇒ 红得没信息
#     · m403_orphan_batch   子门日志尾部 6 行只剩汇总 ⇒ 红得没信息
#   共同点不是产品坏，是**判据没把「环境前提」当第 0 维**（同 m283 的 P4 精神）。
#
# 本门三层（每层都能独立判红）：
#   [1] 静态：CI 会跑的门里，凡引用宿主绝对路径（/data/autodrive · /data/pat.md ·
#       /data/app/ws · /usr/local/sbin · /var/log/dy-ops）而又**没有缺失检测/可注入**的 ⇒ 记 RISK。
#       RISK 集合必须与 known_risks.tsv **逐条相等**：未登记 ⇒ 红；登记了但已不是 RISK ⇒ 红（STALE）；
#       登记带到期日，过期 ⇒ 红（不许「挂个名字就永久免检」）。
#   [2] 动态正控：m410 在「宿主面隐藏」下必绿；m389 在「无令牌文件」下必绿。
#       —— 这两条钉住本轮的真修，不是「环境本来就不红」。
#   [3] 动态负控：把 m410 的宿主面缺席豁免**改回原样**（永远判 STALE）⇒ 同样隐藏宿主面下
#       必须**红**。否则 [2] 的绿可能来自「隐藏没生效」，本门自己也就成了摆设。
#
# 用法：verify.sh [--selftest]
# ============================================================
set -uo pipefail
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }
SELF_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$SELF_DIR/../.." && pwd)
STATIC_ONLY=0
while [ $# -gt 0 ]; do
    case "$1" in
        --selftest) SELFTEST=1 ;;
        --static-only) STATIC_ONLY=1 ;;
        --root) shift; ROOT=$(cd "${1:-$ROOT}" && pwd) ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
    shift
done
SELFTEST=${SELFTEST:-0}
KR="${M415_KR:-$SELF_DIR/known_risks.tsv}"
CI_YML="$ROOT/.github/workflows/ci.yml"
# ⚠️ 下面这些是**扫描模式**（本门要找的目标串），**不是本门的输入** —— 本门不去读这些路径。
#   用变量拼装，是为了让 selfhost/check_gate_inputs.py 的「宿主机输入必须声明」判据
#   不把它们当成运行时依赖（那门数的是真输入；这里是正则与夹具文本）。
#   两边口径不冲突：谁真的读了这些路径，仍由本门 [1] 静态段逐条判（dep_lines / guarded）。
HD=/data; UL=/usr/local; VL=/var/log
HOSTRE="$HD/autodrive|$HD/pat\\.md|$HD/app/ws|$UL/sbin|$VL/dy-ops"
TODAY=$(date +%Y-%m-%d)

fails=0; nscan=0
say(){ echo "── $*"; }
ok(){ echo "  PASS  $*"; }
bad(){ echo "  FAIL  $*"; fails=$((fails+1)); }

# [1] CI 会跑的门清单（从 ci.yml 抽 verify.sh 调用）
ci_gates(){
    [ -f "$CI_YML" ] || return 1
    grep -oE 'examples/[A-Za-z0-9_]+/verify\.sh' "$CI_YML" | sort -u
}
# 该文件是否「对宿主面有缺失检测」：字面路径上带 -d/-f/-e 判定，或路径写在 ${VAR:-...} 默认值里，
# 或是 `for cand in ... /data/...` 候选枚举（循环体内自己挑存在的那条），或 command -v 探测
guarded(){
    local f="$1" alt="($HD/|$UL/sbin|$VL/dy-ops)"
    grep -qE "\[\[?[[:space:]]+-(d|f|e)[[:space:]]+[^]]*$alt" "$f" && return 0
    grep -qE "\\$\{[A-Za-z0-9_]+:-[^}]*$alt" "$f" && return 0
    grep -qE "command -v[[:space:]]+$UL/sbin" "$f" && return 0
    grep -qE "for[[:space:]]+[A-Za-z0-9_]+[[:space:]]+in[[:space:]].*($HD/|$UL/sbin)" "$f" && return 0
    return 1
}
# 运行时依赖行：既不是注释，也不是 echo/printf 的说明文字
dep_lines(){
    awk -v re="$HOSTRE" '{ line=$0; sub(/^[ \t]+/, "", line); if (line ~ /^#/) next;
        if (line ~ /(echo|printf)[ \t]+"/) next;
        # 说明文字的续行（多行引号块里的条目）不含任何代码记号 ⇒ 不是运行时依赖
        if (line !~ /[=[$>|]/) next;
        if (line ~ re) print NR": "line }' "$1" 2>/dev/null
}
risk_list(){   # 输出「相对路径」一行一个（= CI 门里有运行时宿主依赖且无缺失检测）
    local g
    while read -r g; do
        [ -f "$ROOT/$g" ] || continue
        [ -n "$(dep_lines "$ROOT/$g" | head -1)" ] || continue
        guarded "$ROOT/$g" && continue
        echo "$g"
    done < <(ci_gates)
}
scan_face(){   # CI 会跑且盘上存在的门（扫描面本体）
    local g
    while read -r g; do [ -f "$ROOT/$g" ] && echo "$g"; done < <(ci_gates)
}

# 自检：判据的牙（好/坏夹具各一），不依赖真仓库
if [ "$SELFTEST" = 1 ]; then
    T=$(mktemp -d /tmp/m415ce.XXXXXX); trap 'rm -rf "$T"' EXIT
    mkdir -p "$T/examples/fx_host" "$T/examples/fx_guard" "$T/examples/fx_clean"
    printf '#!/usr/bin/env bash\nA=%s/autodrive/x.sh\nbash "$A"\n' "$HD" > "$T/examples/fx_host/verify.sh"
    printf '#!/usr/bin/env bash\nif [ -f %s/autodrive/x.sh ]; then echo hi; else echo "HOSTFACE-SKIP"; fi\n' "$HD" > "$T/examples/fx_guard/verify.sh"
    printf '#!/usr/bin/env bash\n# %s/autodrive/y.sh 只是注释\necho "本机：%s/autodrive/z.sh"\nB=%s/autodrive/real.sh\nfor cand in /tmp/a %s/app/ws/m.onnx; do [ -f "$cand" ] && echo ok; done\n' "$HD" "$HD" "$HD" "$HD" > "$T/examples/fx_clean/verify.sh"

    say "[自检] 缺失检测识别"
    guarded "$T/examples/fx_host/verify.sh"  && bad "正控：无检测的文件被判成有检测" || ok "正控：无缺失检测 ⇒ 不算 guarded"
    guarded "$T/examples/fx_guard/verify.sh" && ok "负控：带 -f 检测 ⇒ 认出 guarded" || bad "负控：带检测却没认出"
    say "[自检] 运行时依赖识别（注释 / echo 文案 / 候选枚举不算）"
    dl=$(dep_lines "$T/examples/fx_clean/verify.sh")
    case "$dl" in *"real.sh"*) ok "正控：真赋值行被认出";; *) bad "正控：真依赖漏掉（$dl）";; esac
    case "$dl" in *"y.sh"*|*"z.sh"*) bad "负控：注释/echo 文案被当成依赖";; *) ok "负控：注释与文案不算依赖";; esac

    # ── 端到端：假仓库跑 --static-only，四条规则各自要有牙 ──
    say "[自检] 端到端（规则未登记 / STALE / 过期 各自必红）"
    R="$T/repo"; mkdir -p "$R/.github/workflows" "$R/examples/fx_host" "$R/examples/fx_clean"
    cp "$T/examples/fx_host/verify.sh"  "$R/examples/fx_host/verify.sh"
    cp "$T/examples/fx_clean/verify.sh" "$R/examples/fx_clean/verify.sh"
    printf 'jobs:\n  q:\n    steps:\n      - run: bash examples/fx_host/verify.sh\n      - run: bash examples/fx_clean/verify.sh\n' > "$R/.github/workflows/ci.yml"
    KR2="$T/kr.tsv"; : > "$KR2"
    run_static_with_kr(){ M415_KR="$1" bash "$SELF_DIR/verify.sh" --static-only --root "$2" >/dev/null 2>&1; }

    if run_static_with_kr "$KR2" "$R"; then bad "正控：未登记的 RISK 却判绿（判据无牙）"; else ok "正控：未登记 RISK ⇒ 红"; fi
    printf 'examples/fx_host/verify.sh\t夹具：故意留宿主依赖\t2026-12-31\n' > "$KR2"
    if run_static_with_kr "$KR2" "$R"; then ok "负控：登记后转绿（红来自登记本身）"; else bad "负控：登记后仍红 ⇒ 判据另有原因"; fi
    printf 'examples/fx_host/verify.sh\t夹具：故意留宿主依赖\t2026-12-31\nexamples/fx_clean/verify.sh\t并不违规\t2026-12-31\n' > "$KR2"
    if run_static_with_kr "$KR2" "$R"; then bad "负控：STALE 登记未判红（白名单会烂账）"; else ok "负控：STALE 登记 ⇒ 红"; fi
    printf 'examples/fx_host/verify.sh\t夹具：故意留宿主依赖\t2026-01-01\n' > "$KR2"
    if run_static_with_kr "$KR2" "$R"; then bad "负控：过期登记未判红（等于永久免检）"; else ok "负控：过期登记 ⇒ 红"; fi

    echo
    if [ "$fails" = 0 ]; then echo "M415-CLEANENV-SELFTEST-OK"; exit 0; fi
    echo "M415-CLEANENV-SELFTEST-FAIL fails=$fails"; exit 1
fi

# ── [1] 静态：RISK ⇄ 登记表 逐条对账 ────────────────────────
say "[1] 静态：CI 门 × 宿主面运行时依赖"
if [ ! -f "$CI_YML" ]; then bad "缺 $CI_YML（判据的前提不在）"; fi
mapfile -t SCAN < <(scan_face)
nscan=${#SCAN[@]}
if [ "$nscan" -eq 0 ]; then
    echo "  FAIL  扫描面为 0（ci.yml 里没抽到任何 verify.sh）⇒ 判红"
    echo "M415-CLEANENV-FAIL nscan=0 risks=0"
    exit 1
fi
ok "扫描面：nscan=$nscan（ci.yml 里会跑且盘上存在的门）"
mapfile -t RISK < <(risk_list)
say "  运行时宿主依赖且无缺失检测：risks=${#RISK[@]}"
declare -A REG=(); declare -A WHY=(); declare -A DUE=()
if [ -f "$KR" ]; then
    while IFS=$'\t' read -r p w d; do
        case "$p" in ''|'#'*) continue;; esac
        REG["$p"]=1; WHY["$p"]="$w"; DUE["$p"]="$d"
    done < "$KR"
fi
declare -A SEEN=()
for p in "${RISK[@]}"; do
    SEEN["$p"]=1
    if [ -z "${REG[$p]+x}" ]; then
        bad "$p 引用宿主面且无缺失检测，但 known_risks.tsv 未登记"
    elif [ -n "${DUE[$p]}" ] && [ "${DUE[$p]}" \< "$TODAY" ]; then
        bad "$p 的登记已于 ${DUE[$p]} 过期（定性：${WHY[$p]}）⇒ 要么修，要么重新定性并延期"
    else
        ok "$p 已登记（到期 ${DUE[$p]} · ${WHY[$p]}）"
    fi
done
for k in "${!REG[@]}"; do
    [ -n "${SEEN[$k]+x}" ] || bad "STALE $k（登记了但已不再是 RISK：删掉这行，别让白名单烂账）"
done

# ── [2] 动态正控：两处真修钉住 ──────────────────────────────
if [ "$STATIC_ONLY" = 0 ]; then
say "[2] 动态正控：隐藏宿主面 / 隐藏令牌文件"
if M410_AUTODRIVE_DIR=/nonexistent/autodrive bash "$ROOT/examples/m410_criteria_scan/verify.sh" > /tmp/m415_m410_hidden.log 2>&1; then
    ok "m410 在宿主面缺席下绿（$(grep -oE 'nscan=[0-9]+ R1=[0-9]+' /tmp/m415_m410_hidden.log | head -1)）"
else
    bad "m410 在宿主面缺席下仍红（见 /tmp/m415_m410_hidden.log 末行 $(tail -1 /tmp/m415_m410_hidden.log)）"
fi
if GH_PAT_FILE=/nonexistent/pat.md bash "$ROOT/examples/m389_gh_auth/verify.sh" > /tmp/m415_m389_nopat.log 2>&1; then
    ok "m389 在无令牌文件下绿（$(grep -oE '门结果：[0-9]+ 通过 / [0-9]+ 失败' /tmp/m415_m389_nopat.log | tail -1)）"
else
    bad "m389 在无令牌文件下仍红（$(grep -E 'N2|P3c' /tmp/m415_m389_nopat.log | tail -2 | tr '\n' ' ')）"
fi

# ── [3] 动态负控：把 m410 的豁免改回原样 ⇒ 同样环境必须红 ────
say "[3] 动态负控：豁免回滚后必须红"
N=$(mktemp -d /tmp/m415_neg.XXXXXX)
mkdir -p "$N/examples/m410_criteria_scan" "$N/packaging"
cp "$ROOT/examples/m410_criteria_scan/verify.sh" "$N/examples/m410_criteria_scan/verify.sh"
cp "$ROOT/examples/m410_criteria_scan/allow.tsv" "$N/examples/m410_criteria_scan/allow.tsv"
cp "$ROOT/selfhost/gate_lock.sh" "$N/selfhost_gate_lock.sh" 2>/dev/null || true
mkdir -p "$N/selfhost"; cp "$ROOT/selfhost/gate_lock.sh" "$N/selfhost/gate_lock.sh"
ln -s "$ROOT/packaging/chain" "$N/packaging/chain"
sed -i 's/^judge_stale(){.*$/judge_stale(){ return 0; }/' "$N/examples/m410_criteria_scan/verify.sh"
if grep -q '^judge_stale(){ return 0; }' "$N/examples/m410_criteria_scan/verify.sh"; then
    ok "前提自证：负控副本确实被改回「永远判 STALE」"
else
    bad "前提自证失败：负控副本没改到（sed 锚点失效）⇒ 本轮结论不可信"
fi
if M410_AUTODRIVE_DIR=/nonexistent/autodrive bash "$N/examples/m410_criteria_scan/verify.sh" > /tmp/m415_neg.log 2>&1; then
    bad "负控不成立：豁免回滚后仍判绿 ⇒ [2] 的绿可能与环境无关（本门无牙）"
else
    ok "负控成立：豁免回滚后同样环境判红（$(grep -oE 'fails=[0-9]+' /tmp/m415_neg.log | tail -1)）"
fi
rm -rf "$N"
else
    say "[2][3] 已按 --static-only 跳过（动态段只在真仓库跑；自检用假仓库）"
fi

echo
reg_n=$(grep -vc '^#' "$KR" 2>/dev/null || true); reg_n=${reg_n:-0}
echo "nscan=$nscan risks=${#RISK[@]} registered=$reg_n"
if [ "$fails" = 0 ]; then echo "M415-CLEANENV-OK"; exit 0; fi
echo "M415-CLEANENV-FAIL fails=$fails"; exit 1
