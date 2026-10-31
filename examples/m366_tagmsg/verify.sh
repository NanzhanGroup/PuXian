#!/usr/bin/env bash
# ============================================================
# examples/m366_tagmsg/verify.sh —— make_tag.sh 消息与里程碑推断门（M366）
# ------------------------------------------------------------
# 为什么有这道门（缺陷 636 / 637，M366 实测）：
#   636  `_SUBJ="$(git log -1 --format=%s | head -c 80)"` —— `head -c` 是**字节**截断，
#        截在多字节字符中间 ⇒ **非法 UTF-8**。实证：v0.2.364 / v0.2.365 的 tag 消息
#        尾部都是 "�"；修前 dry-run 同样打出 "…书写��"。
#   637  里程碑只从 CHANGELOG **标题行**推断，而本仓流程是「先发布、后补 CHANGELOG」
#        ⇒ 发布那一刻 CHANGELOG 顶部还是**上一轮**的条目，消息前缀张冠李戴：
#        v0.2.365 的消息 = 「M364：M365 第 2 轮…」，v0.2.364 = 「M363：M364 第 3 轮…」。
#
# 判据（在**临时仓库**里跑真脚本，不碰本仓、不创建任何 tag）：
#   J1 里程碑取 max(CHANGELOG 顶部, 提交主题) —— 主题 M101 / CHANGELOG 顶部 M100 ⇒ M101
#   J2 tag 消息**严格 UTF-8 合法**（iconv 严解码，零容错）
#   J3 反向：主题号更小（M099）时取 CHANGELOG 的 M100，并打出「两源不一致」提示
#   J4 tag 名合规（v0.2.101），且 == v0.2.<里程碑>
#
# 负控（证明判据有鉴别力）：
#   NC-old   用旧法 `head -c 80` 复算消息 ⇒ 必须**解不出合法 UTF-8**（否则 J2 是空判据）
#   NC-nosub 去掉提交主题的 `M<NNN>：` 前缀 ⇒ 必须回落到 CHANGELOG 的 M100（否则 J1 没读主题）
#
# 用法：verify.sh [--neg-skip]
# 退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
MT="$ROOT/packaging/make_tag.sh"
WORK="${TMPDIR:-/tmp}/m366_tagmsg.$$"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
PASS=0; FAIL=0
ok()  { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad() { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK"
[ -f "$MT" ] || { echo "❌ 找不到 $MT"; exit 1; }

LONG='这是一个超过八十字节的中文提交主题用来验证截断不会把多字节字符切一半从而导致非法UTF-8的情况真的会发生'

mk_repo() {  # $1=目录 $2=提交主题
    # ⚠️ 夹具的关键：CHANGELOG **只写到 M100**（模拟「先发布、后补 CHANGELOG」的真实现场），
    #    本轮号只出现在提交主题里。首版夹具多写了一行 `## M101` ⇒ CHANGELOG 顶部也是 101，
    #    J3 / NC-nosub 全假红（判据没错，是我把「滞后」这个前提给抹掉了）。
    local d="$1"
    rm -rf "$d"; mkdir -p "$d"; cd "$d" || return 1
    git init -q . && git config user.email t@example.invalid && git config user.name t
    printf '# 变更日志\n\n## M100：上一轮（旧）\n\n内容\n' > CHANGELOG.md
    git add -A && git commit -qm 'M100：旧'
    git tag v0.2.100
    printf '## M100 补充一行\n' >> CHANGELOG.md
    git add -A && git commit -qm "$2"
    cp "$MT" ./mt.sh && chmod +x ./mt.sh
}

field() {  # $1=输出 $2=字段名 → 值
    printf '%s\n' "$1" | sed -n "s/^  $2 *: //p" | head -1
}

echo "═══ M366 · make_tag 消息与里程碑推断门 ═══"

# ── J1/J2/J4：正向（主题 M101 > CHANGELOG M100，且主题远超 80 字节）──
mk_repo "$WORK/r1" "M101：$LONG"
OUT1="$(bash ./mt.sh --dry-run 2>&1)"
MSGV="$(field "$OUT1" '消息')"
TAGV="$(field "$OUT1" 'tag 名')"

if printf '%s' "$OUT1" | grep -q '里程碑推断: M101'; then ok "J1 里程碑取较大者 = M101（CHANGELOG 顶部仍是 M100）"
else bad "J1 里程碑未取到 M101：$(printf '%s' "$OUT1" | grep '里程碑' | head -2)"; fi

if [ -z "$MSGV" ]; then
    bad "J2 取不到 tag 消息（dry-run 输出变了？）"
elif printf '%s' "$MSGV" | iconv -f UTF-8 -t UTF-8 >/dev/null 2>&1; then
    ok "J2 tag 消息严格 UTF-8 合法（$(printf '%s' "$MSGV" | wc -c) 字节）"
else
    bad "J2 tag 消息含非法 UTF-8（截断切在多字节字符中间）"
fi

case "$TAGV" in
    v0.2.101) ok "J4 tag 名合规且与里程碑一致（$TAGV）" ;;
    *)        bad "J4 tag 名异常：$TAGV（期望 v0.2.101）" ;;
esac

# ── J3：反向（主题号更小 ⇒ 取 CHANGELOG，并提示不一致）──
mk_repo "$WORK/r2" "M099：小号主题"
OUT2="$(bash ./mt.sh --dry-run 2>&1)"
if printf '%s' "$OUT2" | grep -q '里程碑推断: M100'; then ok "J3 主题号更小时取 CHANGELOG 的 M100"
else bad "J3 反向失败：$(printf '%s' "$OUT2" | grep '里程碑' | head -2)"; fi
if printf '%s' "$OUT2" | grep -q '两源不一致'; then ok "J3 两源不一致有明示（不静默择一）"
else bad "J3 两源不一致时没有提示"; fi

# ── 负控 ────────────────────────────────────────────────────
if [ "$NEG_SKIP" -eq 0 ]; then
    cd "$WORK/r1" || exit 1
    # NC-old：旧法（head -c 80）必须解不出合法 UTF-8
    OLDMSG="M101：$(git log -1 --format=%s | head -c 80)"
    if printf '%s' "$OLDMSG" | iconv -f UTF-8 -t UTF-8 >/dev/null 2>&1; then
        bad "NC-old 旧法 head -c 80 竟然合法 ⇒ J2 无鉴别力（用例主题不够长？）"
    else
        ok "NC-old 旧法 head -c 80 复现非法 UTF-8 ⇒ J2 有鉴别力"
    fi
    # NC-nosub：去掉主题前缀 ⇒ 必须回落 CHANGELOG 的 M100
    cd "$WORK" || exit 1
    mk_repo "$WORK/r3" "$LONG"
    OUT3="$(bash ./mt.sh --dry-run 2>&1)"
    if printf '%s' "$OUT3" | grep -q '里程碑推断: M100'; then
        ok "NC-nosub 主题无 M<NNN> 前缀 ⇒ 回落 CHANGELOG M100（J1 确实读了主题）"
    else
        bad "NC-nosub 未回落：$(printf '%s' "$OUT3" | grep '里程碑' | head -2)"
    fi
else
    echo "  ·  负控跳过（--neg-skip）"
fi

echo "───────────────────────────────────────────────"
echo "M366-tagmsg 门结果：PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
