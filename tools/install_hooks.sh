#!/usr/bin/env bash
# ============================================================================
# tools/install_hooks.sh —— 装本仓的 git 钩子（core.hooksPath=.githooks）
# ----------------------------------------------------------------------------
# 背景（缺陷 566）：此前 .git/hooks 里只有 git 自带的 *.sample，core.hooksPath 未设
#   ⇒ 这台机器上**根本没有任何钩子在跑**。「提交前预警」从来没有载体，
#   所以「忘了 --freeze」只能等到全量门 2 小时后才知道。
#
# 用法：bash tools/install_hooks.sh [--check|--uninstall]
#   无参数      安装（幂等）
#   --check     只查现状（已装 ⇒ rc=0；未装 ⇒ rc=1），不写任何东西
#   --uninstall 撤掉 core.hooksPath
# ============================================================================
set -uo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT" || exit 1
WANT=".githooks"

mode="${1:-install}"

cur=$(git config --get core.hooksPath || true)

case "$mode" in
--check)
    if [ "$cur" = "$WANT" ] && [ -x "$WANT/pre-commit" ]; then
        echo "✅ 钩子已装：core.hooksPath=$cur · $WANT/pre-commit 可执行"
        exit 0
    fi
    echo "❌ 钩子未装（core.hooksPath='${cur}' · pre-commit 可执行=$( [ -x "$WANT/pre-commit" ] && echo 是 || echo 否 ))"
    echo "   ⇒ bash tools/install_hooks.sh"
    exit 1
    ;;
--uninstall)
    git config --unset core.hooksPath 2>/dev/null || true
    echo "已撤掉 core.hooksPath（git 回到默认 .git/hooks）"
    exit 0
    ;;
install)
    if [ ! -f "$WANT/pre-commit" ]; then
        echo "❌ 缺 $WANT/pre-commit —— 仓库不完整？" >&2
        exit 1
    fi
    chmod +x "$WANT/pre-commit" 2>/dev/null || true
    git config core.hooksPath "$WANT" || exit 1
    # 自证：装完立刻回读（不靠「我说装好了」）
    got=$(git config --get core.hooksPath)
    if [ "$got" != "$WANT" ] || [ ! -x "$WANT/pre-commit" ]; then
        echo "❌ 装完回读不一致：core.hooksPath='$got' · 可执行=$( [ -x "$WANT/pre-commit" ] && echo 是 || echo 否 )" >&2
        exit 1
    fi
    echo "✅ 已装：core.hooksPath=$got · $WANT/pre-commit 可执行"
    echo "   功能自证（不靠信任）：bash selfhost/check_emitc_hook.sh"
    exit 0
    ;;
*)
    echo "用法：$0 [--check|--uninstall]" >&2
    exit 2
    ;;
esac
