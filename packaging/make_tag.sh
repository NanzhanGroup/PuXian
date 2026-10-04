#!/usr/bin/env bash
# ============================================================
# packaging/make_tag.sh —— 发布 tag 的**唯一创建入口**（M248 · 用户令 2026-10-03）
# ------------------------------------------------------------
# 为什么需要它（事故 → 本脚本的由来）：
#   规则「v<主版本>-m<里程碑>，**没有补丁后缀**」此前只有**事后检查**
#   （tag_guard.sh ③ · pxrepo_mirror.sh 的 tag-name-guard + xcheck-rpm-tree）。
#   ⇒ 我可以在本地随手 `git tag v0.2.0-m245s1`，直到 CI / 镜像侧才发现。
#   2026-10-03 实测代价（本脚本存在的直接原因）：
#     该 tag ⇒ release.yml 触发 ⇒ build_rpm.sh 的 MILESTONE 取短横线之后**整段**
#     ⇒ gh-pages 出现 `puxian-0.2.0-1.m245s1.el9.x86_64.rpm`
#     ⇒ pxrepo_mirror.sh 的 xcheck-rpm-tree 判「rpm 树里的版本标记不合规（m245s1）」
#     ⇒ **镜像同步停摆**（用户报障「影响镜像同步」）。
#   ⇒ 把规则挪到**创建那一刻**：不合规的名字**根本打不出来**。
#
# 三道防线（本脚本 = 第 ① 道 · 预防）：
#   ① 事前（本脚本）      —— 名字不合规 ⇒ 拒绝创建（exit 3）
#   ② 事中（build_rpm.sh / make_release.sh）—— MILESTONE 不合规 ⇒ 响亮 die（不产出污染包名）
#   ③ 事后（tag_guard.sh ③ · pxrepo_mirror.sh）—— 未登记的违规 tag ⇒ 判红
#
# 用法:
#   packaging/make_tag.sh                          # HEAD + 里程碑自动取 CHANGELOG 最高
#   packaging/make_tag.sh --milestone 248          # 显式指定里程碑号
#   packaging/make_tag.sh --at <commit>            # 指定 tag 指向的提交
#   packaging/make_tag.sh --msg "M248：摘要"       # 追加 tag 消息
#   packaging/make_tag.sh --move                   # tag 已存在且指向别处 ⇒ 重定向（合规做法）
#   packaging/make_tag.sh --push                   # 创建后推 main + tag（**同一次 push**）
#   packaging/make_tag.sh --dry-run                # 只打印将要做什么（**无副作用**）
#
# 退出码: 0 = 成功 · 2 = 参数/环境错误 · 3 = 命名不合规 · 4 = tag 已存在（需 --move）
#         · 5 = 推送失败
# ============================================================
set -uo pipefail

# ------------------------------------------------------------
# 规则（唯一真相 · 与 packaging/tag_guard.sh 的 TAG_NAME_RE 逐字符一致）
#   ⚠ 两处必须一致 —— selftest_make_tag.sh 有一段判据专门断言这一点。
# ------------------------------------------------------------
TAG_NAME_RE='^v[0-9]+\.[0-9]+\.[0-9]+-m[0-9]+$'

# 仓库根解析顺序（自测要在 mktemp 夹具里跑 ⇒ 不能只认脚本所在位置）：
#   ① PX_TAG_REPO（显式指定，自测用） ② 当前目录所在的 git 顶层 ③ 脚本的上一级
REPO="${PX_TAG_REPO:-}"
if [ -z "$REPO" ]; then
    REPO="$(git rev-parse --show-toplevel 2>/dev/null || true)"
fi
[ -n "$REPO" ] || REPO="$(cd "$(dirname "$0")/.." 2>/dev/null && pwd || true)"
[ -n "$REPO" ] || { echo "❌ 定位不到仓库根（用 PX_TAG_REPO=<路径> 显式指定）" >&2; exit 2; }
CHANGELOG_PATH="CHANGELOG.md"
AT=""; MILESTONE=""; MSG=""; NAME=""; NAME_GIVEN=0; DO_MOVE=0; DO_PUSH=0; DRY=0

die() { echo "❌ $*" >&2; exit "${EXIT_CODE:-1}"; }
say() { echo "$*"; }

u() {  # u <说明>
    cat >&2 <<EOF
用法: packaging/make_tag.sh [选项]
  --name <tag>        直接给出完整 tag 名（**会按规则强校验**；给错名 ⇒ exit 3）
  --milestone <N>     里程碑号（默认取 CHANGELOG 标题行里最高的 M<NNN>）
  --at <commit>       tag 指向的提交（默认 HEAD）
  --msg <文本>        tag 消息（默认自动生成）
  --move              tag 已存在且指向别处 ⇒ 重定向到新提交（合规的补丁轮做法）
  --push              创建后推 main + tag（同一次 push，消 M245 补实测的 push 竞态）
  --dry-run           只打印计划，不创建、不推送
  --help              本帮助
规则: tag 名必须是 $TAG_NAME_RE（例 v0.2.0-m248）——**没有补丁后缀**。
      $*
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --name)      [ $# -ge 2 ] || u "--name 缺参数";      NAME="$2"; NAME_GIVEN=1; shift 2 ;;
        --milestone) [ $# -ge 2 ] || u "--milestone 缺参数"; MILESTONE="$2"; shift 2 ;;
        --at)        [ $# -ge 2 ] || u "--at 缺参数";        AT="$2";        shift 2 ;;
        --msg)       [ $# -ge 2 ] || u "--msg 缺参数";       MSG="$2";       shift 2 ;;
        --move)      DO_MOVE=1; shift ;;
        --push)      DO_PUSH=1; shift ;;
        --dry-run)   DRY=1; shift ;;
        --help|-h)   u "（--help）"; exit 0 ;;
        *)           u "未知选项: $1"; exit 2 ;;
    esac
done

cd "$REPO" || die "进不去仓库 $REPO"
git rev-parse --git-dir >/dev/null 2>&1 || die "$REPO 不是 git 仓库"

# ---------- 1. 解析 tag 指向的提交 ----------
AT="${AT:-HEAD}"
git rev-parse -q --verify "${AT}^{commit}" >/dev/null 2>&1 \
    || { EXIT_CODE=2 die "--at '$AT' 不是有效提交"; }
SHA_FULL="$(git rev-parse "${AT}^{commit}")"
SHA="$(git rev-parse --short "$SHA_FULL")"

# ---------- 2. 主版本段：自最高 tag **递增 patch**（M249 · 用户令 2026-10-03）----------
# 规则：每个新 tag 把 patch 段 +1；patch 累计到 100 ⇒ 进位 minor（patch 归 0）。
#   例：v0.2.0-m246 → v0.2.1-m247 → … → v0.2.99-m345 → v0.3.0-m346
# 为什么（用户原话）：「以后每次 tag 之前都改变一下 0.2.*，直到 * 变为 100 就升为 0.3.0」
#   —— `0.2.0` 自 M73 起挂了 175 个里程碑，用户在版本号上**看不出任何进展**。
# ⚠ `--move`（把已有 tag 重定向到最终提交）**不递增**：那不是新版本，只是同一版换个提交。
#   若递增，自测 E 段（--move v0.2.0-m248）会算出 v0.2.1-m248 ⇒ 名字对不上。
# ⚠ `--name` 路径不参与本段（手输完整名，直接过命名校验）。
VER="0.2.0"
LAST_TAG="$(git tag -l --sort=-v:refname 2>/dev/null | grep -E '^v[0-9]+\.[0-9]+\.[0-9]+-m[0-9]+$' | head -1 || true)"
if [ -n "$LAST_TAG" ]; then
    _tv="${LAST_TAG#v}"; _tv="${_tv%%-*}"
    case "$_tv" in
        [0-9]*.[0-9]*.[0-9]*)
            VER="$_tv"
            if [ "$DO_MOVE" != 1 ]; then
                _maj="${_tv%%.*}"; _rest="${_tv#*.}"
                _min="${_rest%%.*}"; _pat="${_rest#*.}"
                _pat=$((_pat + 1))
                if [ "$_pat" -ge 100 ]; then
                    _pat=0; _min=$((_min + 1))
                fi
                VER="${_maj}.${_min}.${_pat}"
                say "ℹ️ 版本段自 $LAST_TAG **递增** patch: $_tv → $VER（patch 到 100 ⇒ 进位 minor）"
            else
                say "ℹ️ 版本段沿用 $LAST_TAG: $VER（--move 不递增）"
            fi
            ;;
    esac
fi

# ---------- 3. 里程碑号：显式 > CHANGELOG 标题行最高 ----------
if [ -z "$MILESTONE" ]; then
    if git cat-file -e "${SHA_FULL}:${CHANGELOG_PATH}" 2>/dev/null; then
        MILESTONE="$(git show "${SHA_FULL}:${CHANGELOG_PATH}" \
                     | grep -E '^#{2,4}[[:space:]]' \
                     | grep -oE '\bM[0-9]{1,4}\b' | sed 's/^M//' | sort -nu | tail -1 || true)"
    fi
    [ -n "$MILESTONE" ] \
        || { EXIT_CODE=2 die "推断不出里程碑：$CHANGELOG_PATH 标题行里没有 M<NNN>；用 --milestone 显式指定"; }
    say "ℹ️ 里程碑自 $CHANGELOG_PATH 标题行推断: M$MILESTONE"
fi

# 里程碑必须是纯数字（容忍 --milestone m248 这种写法）
MILESTONE="${MILESTONE#m}"; MILESTONE="${MILESTONE#M}"
# 「给了 --name 但值是空」= 用错了（忘了填）⇒ 响亮，不静默退回构造名（否则会悄悄建出别的 tag）
if [ "$NAME_GIVEN" = 1 ] && [ -z "$NAME" ]; then
    EXIT_CODE=2 die "--name 收到空字符串（要么不给这个选项，要么给完整 tag 名，例 v0.2.0-m248）"
fi
if [ "$NAME_GIVEN" = 1 ]; then
    # --name 是「手输完整名」的入口 —— 也**正是事故的入口形态**（手打 v0.2.0-m245s1）。
    #   ⇒ 此处不做构造，直接用给定的名字过第 ① 道防线；里程碑号仅供消息/重定向提示用。
    TAG="$NAME"
    [ -n "$MILESTONE" ] || MILESTONE="$(printf '%s' "$TAG" | grep -oE 'm[0-9]+$' | sed 's/^m//' || true)"
else
    case "$MILESTONE" in
        ''|*[!0-9]*) EXIT_CODE=2 die "--milestone '$MILESTONE' 不是纯数字（规则: v<主版本>-m<里程碑>，无补丁后缀）";;
    esac
    TAG="v${VER}-m${MILESTONE}"
fi

# ---------- 4. 命名合规（第 ① 道防线的核心）----------
# 先给**已知的错误形态**一句人话，再给通用兜底 —— 用户/后人不该靠猜。
if ! printf '%s' "$TAG" | grep -qE "$TAG_NAME_RE"; then
    echo "❌ 拒绝创建不合规 tag: $TAG" >&2
    echo "   规则只有一条: $TAG_NAME_RE（例 v0.2.0-m248）——**没有补丁后缀**。" >&2
    # 「补丁后缀」是最常见的错法（我 2026-10-03 就是这么错的）：直接给出合规名。
    if printf '%s' "$TAG" | grep -qE -- '-m[0-9]+s[0-9]+$'; then
        SUGGEST="$(printf '%s' "$TAG" | sed 's/\(-m[0-9][0-9]*\)s[0-9][0-9]*$/\1/')"
        echo "   你写的是「补丁后缀」形态（-mNsN）。合规做法有两条:" >&2
        echo "     ① 该里程碑**已有** tag ⇒ 把同一个 tag 重定向到最终提交（**不要造新名字**）:" >&2
        echo "          packaging/make_tag.sh --move --name $SUGGEST" >&2
        echo "     ② 该里程碑**还没有** tag ⇒ 用合规名:" >&2
        echo "          packaging/make_tag.sh --name $SUGGEST" >&2
    fi
    echo "   为什么这不是风格问题（2026-10-03 实测，用户报障「影响镜像同步」）:" >&2
    echo "     · pxrepo_mirror.sh 的版本序会**静默过滤**违规 tag ⇒ 镜像永远停在旧版；" >&2
    echo "     · build_rpm.sh 把短横线之后整段当 MILESTONE ⇒ rpm 包名被污染" >&2
    echo "       (puxian-0.2.0-1.m248s1.el9.x86_64.rpm) ⇒ xcheck-rpm-tree 判红 ⇒ **镜像停摆**。" >&2
    exit 3
fi

# 默认消息：提交主题若已带 `M<N>` 前缀就不再叠一层（否则会出现「M248：M248：xxx」）。
_SUBJ="$(git log -1 --format=%s "$SHA_FULL" | head -c 80)"
if [ -n "$MSG" ]; then
    TARGET_TAG_MSG="$MSG"
elif printf '%s' "$_SUBJ" | grep -qE "^M${MILESTONE}([:：]| |\$)"; then
    TARGET_TAG_MSG="$_SUBJ"
else
    TARGET_TAG_MSG="M${MILESTONE}：${_SUBJ}"
fi

# ---------- 5. 已存在？ ----------
EXISTS=0; OLD_SHA=""
if git rev-parse -q --verify "refs/tags/$TAG" >/dev/null 2>&1; then
    EXISTS=1
    OLD_SHA="$(git rev-parse "refs/tags/$TAG^{commit}" 2>/dev/null || git rev-parse "refs/tags/$TAG")"
fi

if [ "$EXISTS" = 1 ] && [ "$OLD_SHA" = "$SHA_FULL" ]; then
    say "✅ $TAG 已存在且已指向 $SHA —— 无需动作"
    say "   推送（若尚未推）: git push origin main $TAG"
    exit 0
fi

if [ "$EXISTS" = 1 ] && [ "$DO_MOVE" != 1 ]; then
    echo "❌ $TAG 已存在，指向 ${OLD_SHA:0:7}（≠ $SHA）" >&2
    echo "   若这是**该里程碑的补丁轮**（把同一 tag 移到最终提交），加 --move:" >&2
    echo "     packaging/make_tag.sh --move --milestone $MILESTONE${AT:+ --at $AT}" >&2
    echo "   若这是**新的里程碑**，说明 CHANGELOG 还没写新条目（或该用 --milestone 指定）。" >&2
    exit 4
fi

# ---------- 6. 干跑（只读入口：**不得有副作用** · M244 缺陷 412 的纪律）----------
if [ "$DRY" = 1 ]; then
    say "── dry-run（不创建、不推送）──"
    say "  tag 名   : $TAG"
    say "  指向提交 : $SHA ($(git log -1 --format=%s "$SHA_FULL" | head -c 60))"
    say "  消息     : $TARGET_TAG_MSG"
    say "  动作     : $([ "$EXISTS" = 1 ] && echo '重定向（原指向 '"${OLD_SHA:0:7}"'）' || echo '新建')"
    say "  推送命令 : git push${DO_PUSH:+} origin main $TAG"
    exit 0
fi

# ---------- 7. 创建（重定向时先删后建 —— 保留 annotated 形态）----------
if [ "$EXISTS" = 1 ]; then
    git tag -d "$TAG" >/dev/null || die "删除旧 tag 失败（$TAG）"
    say "♻️ 已删除旧 $TAG（原 ${OLD_SHA:0:7}）⇒ 将重定向到 $SHA"
fi
git tag -a "$TAG" "$SHA_FULL" -m "$TARGET_TAG_MSG" || { EXIT_CODE=3 die "创建 tag 失败"; }
say "✅ 已创建 annotated tag $TAG → $SHA"

# ---------- 8. 自证：名字必须能被「事后守卫」的正则接受 ----------
if ! printf '%s' "$TAG" | grep -qE "$TAG_NAME_RE"; then
    EXIT_CODE=3 die "自证失败：刚创建的 $TAG 不匹配规则（不该发生）"
fi
say "   ✔ 自证：匹配 $TAG_NAME_RE"

# ---------- 9. 推送（main + tag **同一次** push）----------
if [ "$DO_PUSH" = 1 ]; then
    REMOTE="${PX_TAG_REMOTE:-origin}"
    CUR_BRANCH="$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo '')"
    REFS=("$TAG")
    # M245 补 实测事故：先推 main、后推 tag ⇒ tag-guard 的 checkout 快照可能早于 tag 到达。
    #   本仓正常节奏是「commit → 跑全量门 ~90min → push」⇒ 提交年龄 > grace ⇒ 触发。
    #   ⇒ 只要本地领先，就把 main 与 tag **一次推**。
    if [ -n "$CUR_BRANCH" ] && git rev-parse -q --verify "refs/remotes/$REMOTE/$CUR_BRANCH" >/dev/null 2>&1; then
        AHEAD="$(git rev-list --count "refs/remotes/$REMOTE/$CUR_BRANCH..$CUR_BRANCH" 2>/dev/null || echo 0)"
        [ "${AHEAD:-0}" -gt 0 ] && REFS=("$CUR_BRANCH" "$TAG")
    fi
    say "📤 git push $REMOTE ${REFS[*]}"
    git push "$REMOTE" "${REFS[@]}" || { EXIT_CODE=5 die "推送失败"; }
    say "✅ 已推送: ${REFS[*]}"
else
    say "   下一步（**main 与 tag 同一次 push** · 消 push 竞态）:"
    say "     git push origin main $TAG"
fi
