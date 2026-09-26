#!/usr/bin/env bash
# ============================================================
# packaging/tag_guard.sh —— 发布守卫：最高里程碑必须有对应发布 tag
# ------------------------------------------------------------
# 背景（为什么需要这道门）:
#   本仓发布**完全由 tag 驱动** —— .github/workflows/release.yml 的触发条件只有
#       on: push: tags: ['v*']
#   ⇒ 「把 main 推上去」不会发布任何东西；推 main 与发一版是两件事。
#   2026-09-17 实况（qg-issue 86）：M126 / M127 / M128 三个里程碑**连续只推 main、
#   一个 tag 都没打** ⇒ GitHub Releases 页最新仍是 v0.2.0-m125（落后 3 个里程碑、
#   5 条提交），而 main 与全部门**全绿** —— 没有任何信号，直到本源自己去看
#   Releases 页才发现。本脚本把「漏打 tag」变成**可检测**（rc=1）。
#
# 判据（唯一）:
#   1) 取被检查提交里 CHANGELOG.md 的**标题行**（^## / ^### / ^####）中出现过的
#      最大里程碑号 M<NNN> —— 标题行 = 「已入版里程碑名册」；
#      **不看正文**：正文常出现「⇒ 建议 M129」这类**未来**里程碑，会把判据带偏。
#   2) 要求存在 tag（默认 glob `v*-m*`）匹配 `-m<NNN>`（容忍前导零），
#      且该 tag 指向的提交**可被被检查提交到达**（祖先或自身）。
#
# 用法:
#   packaging/tag_guard.sh [--ref <提交>] [--repo <仓库目录>]
#                          [--grace-min N] [--tag-glob 'v*-m*'] [--quiet]
# 退出码:
#   0 = 通过（含 grace 窗口内跳过）
#   1 = 缺 tag（最高里程碑没有对应的、可达的发布 tag）
#   2 = 参数/环境错误（非 git 仓库、ref 不存在、CHANGELOG 缺失、标题行无里程碑）
# 覆盖开关（人工显式、留痕）:
#   TAG_GUARD_ALLOW_MISSING=<非空理由>   允许一次「缺 tag」通过（理由打进日志）
# 环境变量:
#   TAG_GUARD_REPO=<仓库目录>            等价于 --repo
# 边界（如实）:
#   · 只要求**最高**里程碑有 tag；中间里程碑缺 tag 只提示（发布包含全部提交，
#     最新 tag 即最新版），不判红 —— 见「名册」提示行。
#   · 不校验版本段是否该升主版本（v0.2.0 → v0.3.0 属人工判断）。
#   · 只认 `-m<NNN>` 结尾的 tag（本仓约定，见 docs/RELEASE_PROCESS.md）。
# ============================================================
set -uo pipefail

REF="HEAD"
GRACE_MIN=0
TAG_GLOB='v*-m*'
QUIET=0
REPO_DIR="${TAG_GUARD_REPO:-$(cd "$(dirname "$0")/.." && pwd)}"

usage() {
    cat >&2 <<'USAGE'
用法: tag_guard.sh [--ref <提交>] [--repo <仓库目录>] [--grace-min N]
                    [--tag-glob 'v*-m*'] [--quiet]
退出码: 0=通过（或 grace 跳过） · 1=缺 tag · 2=参数/环境错误
USAGE
}

while [ $# -gt 0 ]; do
    case "$1" in
        --ref)       [ $# -ge 2 ] || { echo "❌ --ref 缺参数" >&2; exit 2; };       REF="$2"; shift 2;;
        --repo)      [ $# -ge 2 ] || { echo "❌ --repo 缺参数" >&2; exit 2; };      REPO_DIR="$2"; shift 2;;
        --grace-min) [ $# -ge 2 ] || { echo "❌ --grace-min 缺参数" >&2; exit 2; }; GRACE_MIN="$2"; shift 2;;
        --tag-glob)  [ $# -ge 2 ] || { echo "❌ --tag-glob 缺参数" >&2; exit 2; };  TAG_GLOB="$2"; shift 2;;
        --quiet)     QUIET=1; shift;;
        -h|--help)   usage; exit 0;;
        *)           echo "❌ 未知参数: $1" >&2; usage; exit 2;;
    esac
done

say() { [ "$QUIET" = 1 ] || echo "$@"; }

cd "$REPO_DIR" 2>/dev/null || { echo "❌ 仓库目录不存在: $REPO_DIR" >&2; exit 2; }
git rev-parse --git-dir >/dev/null 2>&1 || { echo "❌ 不是 git 仓库: $REPO_DIR" >&2; exit 2; }
COMMIT=$(git rev-parse --verify --quiet "${REF}^{commit}") || { echo "❌ 无法解析到提交: $REF" >&2; exit 2; }
SHORT=$(git rev-parse --short "$COMMIT")

# ------------------------------------------------------------
# grace：刚推上来的提交允许「tag 还没打」的窗口期，避免误报
#   （workflow 的 push 触发天然落在这个窗口里；定时任务会在窗口过后复查）
# ------------------------------------------------------------
if [ "$GRACE_MIN" -gt 0 ] 2>/dev/null; then
    TS=$(git show -s --format=%ct "$COMMIT")
    NOW=$(date -u +%s)
    AGE_MIN=$(( (NOW - TS) / 60 )); [ "$AGE_MIN" -lt 0 ] && AGE_MIN=0
    # M215 收尾（2026-09-26 实测事故）：**年龄行无条件打印**。
    #   事故形状：commit 16:45 CST → 跑完 `m116_gates.sh` 全量门（~70 分钟）→ push 18:08 CST
    #   ⇒ 年龄 83 分钟 > grace 45 ⇒ push 触发的 run **误红**，而红条只说「缺发布 tag」，
    #   **读不出真因是「窗口太小」**。⇒ ① grace 提到 180（见 workflow / RELEASE_PROCESS）；
    #   ② 把年龄打成**信息行**，这样判红时 run 摘要里能直接读到「年龄 vs grace」。
    say "ℹ️ $SHORT 距今年龄 ${AGE_MIN} 分钟（grace ${GRACE_MIN} 分钟）"
    if [ "$AGE_MIN" -lt "$GRACE_MIN" ]; then
        say "⏳ 落在 grace 窗口内 ⇒ 跳过检查"
        say "   刚推上来、tag 尚未打属正常窗口；定时任务会在窗口过后复查。"
        exit 0
    fi
fi

CHANGELOG_PATH="CHANGELOG.md"
if ! git cat-file -e "${COMMIT}:${CHANGELOG_PATH}" 2>/dev/null; then
    echo "❌ $SHORT 里没有 $CHANGELOG_PATH（判据的事实源缺失）" >&2
    exit 2
fi

HEADINGS=$(git show "${COMMIT}:${CHANGELOG_PATH}" | grep -E '^#{2,4}[[:space:]]')
MILESTONES=$(printf '%s\n' "$HEADINGS" | grep -oE '\bM[0-9]{1,4}\b' | sed 's/^M//' | sort -nu)
TOP=$(printf '%s\n' "$MILESTONES" | grep -E '^[0-9]+$' | sort -n | tail -1)

if [ -z "${TOP:-}" ]; then
    echo "❌ $CHANGELOG_PATH 标题行里没有任何 M<NNN> 里程碑（无事实源，无法判定）" >&2
    exit 2
fi

ALL_TAGS=$(git tag -l "$TAG_GLOB")
# 里程碑号 → tag 名（容忍前导零：-m128 / -m0128 都算）
tag_for() { printf '%s\n' "$ALL_TAGS" | grep -E -e "-m0*$1$" | sort -V | tail -1; }

TAG=$(tag_for "$TOP")
if [ -n "$TAG" ]; then
    TAG_COMMIT=$(git rev-list -n1 "$TAG" 2>/dev/null)
    if [ -n "$TAG_COMMIT" ] && git merge-base --is-ancestor "$TAG_COMMIT" "$COMMIT" 2>/dev/null; then
        say "✅ 发布 tag 守卫通过：最高里程碑 M$TOP ← $TAG（$(git rev-parse --short "$TAG_COMMIT")）"
        # 旁证：提交主题里出现过更高里程碑（只提示，不判红）
        SUBJ_TOP=$(git log --format=%s "$COMMIT" 2>/dev/null | grep -oE '\bM[0-9]{1,4}\b' | sed 's/^M//' | sort -n | tail -1)
        if [ -n "${SUBJ_TOP:-}" ] && [ "$SUBJ_TOP" -gt "$TOP" ] 2>/dev/null; then
            say "⚠️ 提交主题里出现过更高的里程碑 M$SUBJ_TOP，而 CHANGELOG 标题行名册最高 M$TOP"
            say "   若 M$SUBJ_TOP 已入版，请补 CHANGELOG 标题行（本守卫以标题行为事实源）。"
        fi
        # 中间里程碑缺 tag：只提示（最多列举 6 个）
        GAPS=$(printf '%s\n' "$MILESTONES" | sort -rn | while read -r _g; do
                   [ "$_g" = "$TOP" ] && continue
                   [ -z "$(tag_for "$_g")" ] && printf 'M%s ' "$_g"
               done)
        if [ -n "${GAPS:-}" ]; then
            N=$(printf '%s\n' "$GAPS" | wc -w)
            if [ "$N" -gt 6 ]; then
                GAPS="$(printf '%s\n' "$GAPS" | cut -d' ' -f1-6)…（共 $N 个）"
            fi
            say "ℹ️ 名册里以下里程碑无 tag（不判红：发布包含全部提交，最新 tag 即最新版）：$GAPS"
        fi
        exit 0
    fi
    REASON="tag $TAG 指向 $(git rev-parse --short "$TAG_COMMIT" 2>/dev/null)，不在 $SHORT 的可达链上"
else
    REASON="找不到匹配 -m$TOP 的 tag（glob: $TAG_GLOB）"
fi

# ------------------------------------------------------------
# 判红
# ------------------------------------------------------------
echo "❌ 发布 tag 守卫未通过：最高里程碑 M$TOP 缺发布 tag"
echo "   被检查提交: $SHORT  $(git show -s --format=%s "$COMMIT" | cut -c1-72)"
echo "   原因: $REASON"

if [ -n "${TAG_GUARD_ALLOW_MISSING:-}" ]; then
    echo "⚠️ TAG_GUARD_ALLOW_MISSING 已设 ⇒ 本次放行（理由: $TAG_GUARD_ALLOW_MISSING）"
    exit 0
fi

echo
echo "── 里程碑名册（CHANGELOG 标题行 → 发布 tag）──"
printf '%s\n' "$MILESTONES" | sort -rn | head -12 | while read -r _m; do
    _t=$(tag_for "$_m")
    if [ -n "$_t" ]; then printf '   M%-4s ✅ %s\n' "$_m" "$_t"
    else                  printf '   M%-4s ❌ 无 tag\n' "$_m"; fi
done

# 版本段沿用「以 -m<数字> 结尾的 tag」里版本序最大的那个。
#   ⚠️ 必须先按 `-m<数字>$` **过滤**再取最大值：本仓存在 `v0.2.0-m114s2`
#      这类**非里程碑** tag；若先 sed 剥离再 sort -V，剥离出的裸 `v0.2.0`
#      在版本序里**排在 `v0.2.0-m114s2` 之前** ⇒ 会建议出
#      `v0.2.0-m114s2-m127` 这种畸形 tag 名（2026-09-17 首跑实测踩到，
#      已在 selftest 的 A2/A3 钉死）。
NEWEST_M_TAG=$(printf '%s\n' "$ALL_TAGS" | grep -E -e '-m0*[0-9]+$' | sort -V | tail -1)
SUGGEST_VER=$(printf '%s\n' "${NEWEST_M_TAG:-}" | sed -E 's/-m0*[0-9]+$//')
[ -n "$SUGGEST_VER" ] || SUGGEST_VER="v0.0.0"

echo
echo "⇒ 处置（二选一）:"
echo "   a) 补打 tag（推荐）—— 发布由 tag 驱动，推 main 不触发任何发布:"
echo "        git tag -a ${SUGGEST_VER}-m${TOP} ${SHORT}    # 版本段沿用最新里程碑 tag（${NEWEST_M_TAG:-无}）；要升主版本请自行判断"
echo "        git push origin ${SUGGEST_VER}-m${TOP}"
echo "      → .github/workflows/release.yml 全自动出包 + 建 Release"
echo "   b) 本次确实无需发布（纯文档/CI 等）:"
echo "        TAG_GUARD_ALLOW_MISSING='<理由>' bash packaging/tag_guard.sh --ref $SHORT"
echo "      （理由会打进日志；勿常态化使用）"
echo
echo "   SOP: docs/RELEASE_PROCESS.md"
exit 1
