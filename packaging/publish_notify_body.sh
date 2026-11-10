#!/usr/bin/env bash
# ============================================================================
# publish_notify_body.sh —— 发布通知正文拼装（唯一实现）
#
# 为什么单独一个文件（缺陷 698-②）：
#   老写法把正文写在 heredoc 里，尾部一行写成  tag v0.2.$MS → \$TAGSHA，
#   反斜杠让变量不展开 ⇒ 用户手机上看到的就是字面量「$TAGSHA」。
#   这里改成 printf 参数化，sha 只能从 $2 来，不可能再写成字面量。
#
# 用法：publish_notify_body.sh <TAG> <SHA7> <正文文件或字符串> [镜像URL]
# 输出：stdout（纯文本），失败 rc=2
# ============================================================================
set -uo pipefail
TAG="${1:-}"; SHA="${2:-}"; BODY="${3:-}"; MIRROR="${4:-}"
[ -n "$TAG" ] || { echo "用法: $0 <TAG> <SHA7> <正文文件|正文> [镜像URL]" >&2; exit 2; }
[ -n "$SHA" ] || { echo "缺少 tag 指向的 sha（不许留空：空 sha 会被读成「没落地」）" >&2; exit 2; }

if [ -f "$BODY" ] && [ -r "$BODY" ]; then
    BODY="$(cat "$BODY")"
fi

printf '%s\n' "$BODY"
printf '\n%s\n' "tag $TAG → $SHA"
printf '%s\n' "时间 $(date '+%m-%d %H:%M')"
if [ -n "$MIRROR" ]; then
    printf '%s\n' "镜像 $MIRROR 通常 25 分钟内同步，齐了我再报「可 dnf upgrade」"
fi
