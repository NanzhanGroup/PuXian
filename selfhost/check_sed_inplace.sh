#!/usr/bin/env bash
# ============================================================================
# selfhost/check_sed_inplace.sh —— 「就地改写仓库跟踪文件」必须留交代（M325 · 缺陷 575）
# ----------------------------------------------------------------------------
# 为什么要有（我自己犯的，574）：
#   我在 /tmp 的临时补丁脚本里用 sed -i 改 examples/m324_tls_failclosed/verify.sh 的两行 trap。
#   替换串里有 ; ' 和信号名，sed 静默吃掉一个字符 ⇒ 两行并成一条、trap 收到非法信号 HU
#   ⇒ **一个 trap 都没挂上**，而 bash -n 判绿、门「绿着」留下 6 个孤儿进程 + 不还原源码。
#   sed 的错法是**静默**的（没命中 rc 也是 0，改坏了也不报）。本仓有一类「无命中即报错、
#   批量先整体校验、原子写保留权限」的编辑原语，改正式文件时应当优先用它。
#
# 为什么**不是**「一见到 sed -i 就判红」（第一版就是这么写的，真仓实测推翻了它）：
#   首版在真仓判出 8 处命中，**全部是合法用法** —— 门在负控里就地改正式文件、再靠
#   restore_all/snapshot **逐字节还原并自证**（m193/m195/m196/m200/m263），
#   以及把它们当 **fixture 输入**喂给静态分析器（m285）。
#   ⇒ 一刀切是「贴表象不贴契约」（CRITERIA ②）：文本里有 sed，不等于它会留下破坏。
#   本门的职责因此收窄成两件**可机器核对**的事：
#     ① 现役命中集合 ⇄ selfhost/sed_inplace_allow.txt **双向相等**（新增 / 过期都判红）
#     ② 每条声明 class=restore 的，**该脚本必须真含还原机制**（理由是可核对的事实，不是文字）
#   诚实边界：这是**台账门**（查「有没有交代」），不是「安全证明」；变量间接引用是逃逸面。
#   真正的兜底是 574 那种**运行时自证**（trap -p 四项俱全）+ check_neg_residue 的工作树卫生。
#
# 用法：bash selfhost/check_sed_inplace.sh [--self-test]
# ============================================================================
set -uo pipefail
cd "$(cd "$(dirname "$0")/.." && pwd)" || exit 1
ALLOW=${SED_GUARD_ALLOW:-selfhost/sed_inplace_allow.txt}
RESTORE_RE='restore_all|restore_rt|restore_|snapshot|git checkout --|git -C [^ ]+ checkout'

# hits <跟踪表> <脚本清单> → "脚本<TAB>目标<TAB>行号"（注释行不算：不执行的东西不会改文件）
hits() {
    awk '
    FNR==1 && NR!=FNR { file=FILENAME }
    FNR==NR { trk[$0]=1; next }
    {
        l=$0; sub(/^[ \t]+/, "", l)
        if (substr(l,1,1)=="#") next
        if ($0 !~ /(sed[ \t]+-[A-Za-z]*i[A-Za-z.]*)|(sed[ \t]+--in-place)|(perl[ \t]+-[A-Za-z]*i)/) next
        n = split($0, tok, /[ \t"\047`;|&()<>]+/)
        found=""
        for (i = 1; i <= n; i++) {
            t = tok[i]; sub(/^\.\//, "", t)
            if (t == "" || t ~ /^build\//) continue
            if (t in trk) found = t
        }
        if (found != "") printf "%s\t%s\t%d\n", file, found, FNR
    }
    ' "$1" $(cat "$2") 2>/dev/null | sort -u
}

# 命中里「未登记」的条数（用 FILENAME 判首文件：首文件为空时 NR==FNR 会误判，这就是自证抓到的那一版）
cnt_new() { awk -F'\t' -v af="$2" 'FILENAME==af{seen[$1"\t"$2]=1; next} !(($1"\t"$2) in seen){c++} END{print c+0}' \
    "$2" <(cut -f1,2 "$1" | sort -u); }

# 白名单里「已过期（不再命中）」的条数：先装命中表，再逐条核台账
cnt_stale() { awk -F'\t' -v hf="$1" 'FILENAME==hf{h[$1"\t"$2]=1; next} /^#/ || NF<2 {next} (($1"\t"$2) in h){next} {c++} END{print c+0}' \
    "$1" "$2"; }

if [ "${1:-}" = "--self-test" ]; then
    echo "=== 自证（反例必须红）==="
    T=$(mktemp -d "${TMPDIR:-/tmp}/px-sed.XXXXXX") || exit 1
    trap 'rm -rf "$T"' EXIT
    printf '%s\n' 'examples/m1/verify.sh' 'tools/px' 'build/tamper.px' > "$T/trk"
    printf 'sed -i %s build/tamper.px\n' "'s/O/N/'"              > "$T/good1.sh"
    printf 'sed -i %s "$TARGET"\n' "'s/a/b/'"                     > "$T/good2.sh"
    printf '# 这里讨论 sed -i 改 tools/px 的风险（注释不执行）\n'    > "$T/good3.sh"
    printf 'sed -i %s examples/m1/verify.sh\n' "'s/t/x/'"         > "$T/bad1.sh"
    printf 'perl -pi -e %s tools/px\n' "'s/a/b/'"                 > "$T/bad2.sh"
    P=0; F=0
    chk() { if [ "$2" = "$3" ]; then echo "  ✅ $1（$2）"; P=$((P+1)); else echo "  ❌ $1（实测「$2」· 期望「$3」）"; F=$((F+1)); fi; }
    one() { printf '%s\n' "$1" > "$T/l"; hits "$T/trk" "$T/l" | wc -l; }
    for g in good1 good2 good3; do chk "正控 $g 不误判" "$(one "$T/$g.sh")" 0; done
    chk "负控 bad1 必判红" "$(one "$T/bad1.sh")" 1
    chk "负控 bad2 必判红（无扩展名的 tools/px 也要认）" "$(one "$T/bad2.sh")" 1
    printf '%s\n' "$T/nonexist.sh" > "$T/l"
    chk "前提自证：不存在的脚本扫到 0（空转 ≠ 没命中）" "$(hits "$T/trk" "$T/l" | wc -l)" 0
    printf '%s\n' "$T/bad1.sh" > "$T/l"; hits "$T/trk" "$T/l" > "$T/h.txt"
    printf '%s\tself\trestore\t自证用\n' "$(cut -f1,2 "$T/h.txt")" > "$T/allow_ok.txt"
    printf 'examples/zzz/verify.sh\tgone.sh\trestore\t过期条目\n'   > "$T/allow_stale.txt"
    chk "白名单已登记 ⇒ 未登记 0 条" "$(cnt_new "$T/h.txt" "$T/allow_ok.txt")" 0
    chk "白名单未登记 ⇒ 未登记 1 条" "$(cnt_new "$T/h.txt" /dev/null)" 1
    chk "白名单过期 ⇒ 过期 1 条" "$(cnt_stale "$T/h.txt" "$T/allow_stale.txt")" 1
    echo "── 自证：通过 $P · 失败 $F"
    [ "$F" -eq 0 ] && { echo "SED-GUARD-SELFTEST-OK"; exit 0; }
    echo "SED-GUARD-SELFTEST-FAIL"; exit 1
fi

echo "=== 就地改写跟踪文件 · 台账门（缺陷 575）==="
T=$(mktemp -d "${TMPDIR:-/tmp}/px-sed.XXXXXX") || exit 1
trap 'rm -rf "$T"' EXIT
git ls-files > "$T/trk"
{ ls selfhost/*.sh tools/*.sh packaging/*.sh examples/*/verify.sh 2>/dev/null || true; } \
    | grep -vxF 'selfhost/check_sed_inplace.sh' > "$T/list"
# ↑ 排除本脚本自身：它的 --self-test 必须内嵌 sed -i /perl -pi 字串当反例（否则自证无从谈起）。
#   代价说清楚：本脚本自身的改动不在本门视野内 —— 这是自觉接受的边界，不是遗漏。
S=$(grep -c . "$T/list" || true)
echo "  扫描面：$S 个脚本（selfhost/*.sh · tools/*.sh · packaging/*.sh · examples/*/verify.sh）"
[ "$S" -ge 100 ] || { echo "❌ 扫描面只 $S 个脚本 —— 提取器坏了？"; echo "SED-GUARD-UNKNOWN"; exit 2; }

hits "$T/trk" "$T/list" > "$T/h.txt"
H=$(wc -l < "$T/h.txt" | tr -d ' ')
echo "  现役命中：$H 处"
echo "  台账文件：$ALLOW（$( [ -f "$ALLOW" ] && grep -vc '^#\|^$' "$ALLOW" || echo 0 ) 条）"

# 台账文件缺失 ⇒ 全部命中都算未登记（**不许**绿：没有台账不等于「没命中」）
if [ ! -f "$ALLOW" ]; then
    OK=0; NEW=$H; STALE=0
    echo "  ❌ 缺台账文件 $ALLOW —— $H 处命中无处登记"
else
    NEW=$(cnt_new "$T/h.txt" "$ALLOW"); NEW=${NEW:-0}
    STALE=$(cnt_stale "$T/h.txt" "$ALLOW"); STALE=${STALE:-0}
fi
OK=1

if [ "$NEW" -gt 0 ]; then
    OK=0
    if [ -f "$ALLOW" ]; then
        awk -F'\t' -v af="$ALLOW" 'FILENAME==af{seen[$1"\t"$2]=1; next}
            !(($1"\t"$2) in seen){printf "     %s → %s（行 %s）\n",$1,$2,$3}' \
            "$ALLOW" <(cut -f1,2,3 "$T/h.txt") | sort -u > "$T/newlist.txt"
    else
        awk -F'\t' '{printf "     %s → %s（行 %s）\n",$1,$2,$3}' "$T/h.txt" | sort -u > "$T/newlist.txt"
    fi
    echo "  ❌ 有 $NEW 处命中**未登记**："
    sed 's/^/  /' "$T/newlist.txt"
    echo "     ⇒ 就地改仓库文件必须交代：在 $ALLOW 加一行「脚本<TAB>目标<TAB>类别<TAB>理由」"
fi
if [ "$STALE" -gt 0 ]; then
    OK=0
    echo "  ❌ 台账有 $STALE 条**已过期**（不再命中 ⇒ 说明改动没走这里，或文件被改名/删）:"
    cnt_stale_list() { awk -F'\t' 'NR==FNR{h[$1"\t"$2]=1; next} /^#/ || NF<2 {next} (($1"\t"$2) in h){next} {printf "     %s → %s（%s）\n",$1,$2,$4}' \
        <(cut -f1,2 "$T/h.txt" | sort -u) "$ALLOW"; }
    cnt_stale_list
fi
# ② 声明 restore 的必须真含还原机制
if [ -f "$ALLOW" ]; then
    while IFS=$'\t' read -r scr tgt cls why; do
        [ -z "${scr:-}" ] && continue
        case "$scr" in \#*) continue;; esac
        [ "${cls:-}" = "restore" ] || continue
        [ -f "$scr" ] || continue
        if ! grep -Eq "$RESTORE_RE" "$scr"; then
            OK=0
            echo "  ❌ $scr 声明 restore，但脚本里查不到还原机制（restore_*/snapshot/git checkout --）"
        fi
    done < "$ALLOW"
fi

if [ "$OK" -eq 1 ]; then
    echo "✅ 台账一致：$H 处命中全部已交代，且 restore 声明可核对"
    echo "SED-GUARD-OK"; exit 0
fi
echo "SED-GUARD-FAIL"; exit 1
