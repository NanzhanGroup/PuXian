#!/usr/bin/env bash
# ============================================================
# packaging/publish_gh_pages.sh —— 把合并好的 rpm 仓库发到 gh-pages
#   （release.yml 的 `rpm-publish` 步：原来是**内联 bash**，M288s1 抽成受版本管理的脚本）
# ------------------------------------------------------------
# 为什么要抽出来（M288s1 · 2026-10-08 实测事故）：
#   本仓 **job 日志对非管理员不可读**（API 403 "Must have admin rights"）⇒ **注解是唯一通道**。
#   而该步内联时注解只有一句 `exit code 1` ⇒ **无法定性**：
#     clone 失败？单调性守卫拒绝？rsync？commit？push 被拒？
#   实测时序证据：该步**失败时只用 3s / 2s**，而成功时（v0.2.286）是 **20s**
#     ⇒ 失败发生在**很前面**（clone 或更早），但读不到是哪一条。
#   ⇒ 抽成脚本 + **逐段计时**（`⏱ +Ns 阶段`）+ 失败时由调用方打一条**富注解**。
#
# 凭证（顺手修掉一处暴露面）：**不把 token 放进 URL** ——
#   `set -x`/`bash -x` 会把整个命令行打进日志与注解 ⇒ 泄漏。
#   改用 `credential.helper store`（`~/.git-credentials`，chmod 600）。
#
# 用法：见 release.yml。环境变量：TOKEN · GITHUB_WORKSPACE · GITHUB_REPOSITORY · GITHUB_REF_NAME
#   可选：PAGES_DIR（默认 /tmp/ghpages）· PXREPO_DIR（默认 /tmp/pxrepo）
#         PX_PUBLISH_REPO_URL（自测/本地复跑用：直连本地 bare 远端，**跳过凭证写入**）
# 退出码：0 = 已发布 / 无变更；非 0 = 失败（调用方负责 annotate_failure.sh）
# ============================================================
set -euo pipefail

PAGES="${PAGES_DIR:-/tmp/ghpages}"
NEW_TREE="${PXREPO_DIR:-/tmp/pxrepo}"
WS="${GITHUB_WORKSPACE:?需要 GITHUB_WORKSPACE}"
REFNAME="${GITHUB_REF_NAME:-unknown}"

t0=$(date +%s)
mark() { echo "⏱ +$(( $(date +%s) - t0 ))s  $*"; }

if [ -n "${PX_PUBLISH_REPO_URL:-}" ]; then
    # 自测 / 本地复跑：直连本地远端（无需凭证）
    REPO_URL="$PX_PUBLISH_REPO_URL"
    mark "（PX_PUBLISH_REPO_URL 覆盖 ⇒ 跳过凭证写入）"
else
    REPO_SLUG="${GITHUB_REPOSITORY:?需要 GITHUB_REPOSITORY}"
    git config --global credential.helper store
    printf 'https://x-access-token:%s@github.com\n' "${TOKEN:?需要 TOKEN}" > "$HOME/.git-credentials"
    chmod 600 "$HOME/.git-credentials"
    REPO_URL="https://github.com/${REPO_SLUG}.git"
fi
export GIT_TERMINAL_PROMPT=0

mark "① 探测 gh-pages 分支"
HAS_PAGES=0
# ⚠️ 不写 `git … | grep -q`：`pipefail` 下 grep 命中即退出 ⇒ 左侧收 SIGPIPE(141)
#   ⇒ 管道非零 ⇒ 判据恒假（本仓 P3 禁形 · selfhost/check_gate_premise.sh 当场判红）。
#   口径：**先取输出、再判内容**，与退出码解耦（M168/M219 记过的同一条）。
LSREM="$(git ls-remote --heads "$REPO_URL" gh-pages 2>/dev/null || true)"
case "$LSREM" in *refs/heads/gh-pages*) HAS_PAGES=1 ;; esac
mark "① 探测完成：HAS_PAGES=$HAS_PAGES"

rm -rf "$PAGES"; mkdir -p "$PAGES"
if [ "$HAS_PAGES" = "1" ]; then
    mark "② clone --depth 1 gh-pages（远端 $REPO_URL）"
    git clone -q --depth 1 --branch gh-pages "$REPO_URL" "$PAGES"
    mark "② clone 完成（$(du -sh "$PAGES" 2>/dev/null | cut -f1)）"
else
    mark "② 无 gh-pages ⇒ orphan 初始化"
    git -C "$PAGES" init -q -b gh-pages
    git -C "$PAGES" remote add origin "$REPO_URL"
fi
git -C "$PAGES" config user.email "actions@github.com"
git -C "$PAGES" config user.name "GitHub Actions"

mkdir -p "$PAGES/rpm"
mark "③ 发布单调性守卫（qg-issue 41）"
bash "$WS/packaging/rpm_monotonic_guard.sh" "$PAGES/rpm" "$NEW_TREE"
mark "③ 守卫通过（单调不减）"

mark "④ rsync 仓库树（--delete）"
rsync -a --delete "$NEW_TREE"/ "$PAGES/rpm/"
mark "④ rsync 完成（$(du -sh "$PAGES/rpm" 2>/dev/null | cut -f1)）"

for f in install-rpm.sh pages/index.html; do
    [ -f "$WS/packaging/$f" ] || { echo "❌ 缺少 packaging/$f"; exit 1; }
done
install -m 0755 "$WS/packaging/install-rpm.sh" "$PAGES/install-rpm.sh"
install -m 0644 "$WS/packaging/pages/index.html" "$PAGES/index.html"
mark "⑤ 站点根文件已同步（install-rpm.sh / index.html）"

cd "$PAGES"
git add -A
if git diff --cached --quiet; then
    mark "ℹ️ rpm 仓库无变更（同版本重复发布）"
else
    git commit -q -m "rpm: 发布 PuXian ${REFNAME}（el7/el9 + openEuler 签名仓库）"
    HAS_TRACK="无"
    git rev-parse --verify -q origin/gh-pages >/dev/null && HAS_TRACK="有"
    mark "⑥ 已 commit（branch=$(git rev-parse --abbrev-ref HEAD) · remote-tracking=$HAS_TRACK）"
    if [ "$HAS_TRACK" = "有" ]; then
        git push origin gh-pages
    else
        # ⚠️ 远端**已有** gh-pages 而本地没有 remote-tracking（例如探测/clone 走了 orphan 分支）
        #   ⇒ 直推会被拒（non-fast-forward）。这里显式**先取回**再推，并在失败时留下可读线索。
        git fetch -q --depth 1 origin gh-pages || true
        git push -u origin gh-pages
    fi
    mark "⑥ push 完成"
fi
mark "✅ 已推送到 gh-pages/rpm/（${REFNAME}）"
if [ -n "${GITHUB_REPOSITORY:-}" ]; then
    echo "🌐 仓库地址: 国内 https://soft.xiusoft.cn/puxian/rpm/  ｜ 上游 https://nanzhanggroup.github.io/${GITHUB_REPOSITORY#*/}/rpm/（含 el7 / el9 / openeuler/<ver>）"
fi
