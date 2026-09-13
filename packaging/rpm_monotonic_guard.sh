#!/usr/bin/env bash
# ============================================================
# packaging/rpm_monotonic_guard.sh —— rpm 发布单调性守卫（qg-issue 41）
# ------------------------------------------------------------
# 背景（为什么需要这道门）：
#   .github/workflows/release.yml 的 rpm-publish 用
#       rsync -a --delete <新仓库> <gh-pages>/rpm/
#   注释写着"只保留最新版" —— 但**从不比较新旧**。2026-09-11 实况：
#       v0.2.0-m111 先发布（18:04Z）→ v0.2.0-m110 后发布（18:54Z）覆盖
#   这一次是"新代码配旧编号"（m111 的提交恰是 m110 的祖先）⇒ 侥幸无害。
#   若两个 tag 的先后关系反过来、或有人重跑旧 tag 的 release，
#   rsync --delete 会把 rpm/ 与 repodata **整体替换成旧版**，且**零告警**
#   ⇒ 对外表现为"dnf 升级后能力回落"。
#
# 本脚本（对应 ISSUE.md §4 方案 A）：
#   比较「已发布仓库树」与「待发布仓库树」里的 puxian rpm 版本；
#   待发布**更旧** ⇒ 失败（rc=1），让流水线红，而不是静默覆盖。
#
# 用法:
#   rpm_monotonic_guard.sh <已发布仓库树> <待发布仓库树>
#     已发布仓库树 = gh-pages 的 rpm/ 目录（形如 <dist>/<arch>/puxian-*.rpm）
#     待发布仓库树 = CI 组装好的新仓库（同布局，形如 /tmp/pxrepo）
# 退出码:
#   0 = 允许发布（待发布 ≥ 已发布，或无基线）
#   1 = 检测到版本回退（拒绝发布）
#   2 = 参数/环境错误（含"待发布树里没有任何 puxian rpm"）
# 覆盖开关（人工显式、留痕；默认禁止静默通过）:
#   RPM_ALLOW_REGRESSION=<非空理由>   允许一次回退发布，理由会打进日志
# 依赖:
#   仅 bash + find/sed/awk/sort —— **不依赖 rpm**（rpm-publish job 未装 rpm）
# ============================================================
set -uo pipefail

OLD_TREE="${1:-}"
NEW_TREE="${2:-}"

usage() {
    echo "用法: $(basename "$0") <已发布仓库树> <待发布仓库树>" >&2
    echo "  退出码: 0=允许发布 · 1=版本回退（拒绝） · 2=参数/环境错误" >&2
}

if [ -z "$OLD_TREE" ] || [ -z "$NEW_TREE" ]; then
    usage
    exit 2
fi
if [ ! -d "$NEW_TREE" ]; then
    echo "❌ 待发布仓库树不存在: $NEW_TREE" >&2
    exit 2
fi

# ------------------------------------------------------------
# rpm 文件名 → "<排序键>\t<人类可读版本-发布>"
#   文件名格式: <name>-<version>-<release>.<arch>.rpm
#     puxian-0.2.0-1.m112.el9.x86_64.rpm
#   rpm 规范**禁止 version 段含 '-'** ⇒ 第一个 '-' 即 version/release 分界，可靠。
#   排序键 = "<version>-<6 位补零里程碑>"，例如 0.2.0-000112
#   为何补零：里程碑是 release 段里的 -m<NNN>；补零后 `sort -V` 对
#     "同主版本不同里程碑" 与 "不同主版本" 两种比较都给出正确顺序。
# ------------------------------------------------------------
_rpm_key_of() {   # $1 = rpm 路径 → printf "<key>\t<ver-rel>"；非 puxian 包则无输出
    local base ver rel rel_lc ms
    base="$(basename "$1")"
    case "$base" in puxian-*.rpm) ;; *) return 0 ;; esac
    base="${base%.rpm}"        # puxian-0.2.0-1.m112.el9.x86_64
    base="${base%.*}"          # 去掉末段 arch → puxian-0.2.0-1.m112.el9
    base="${base#puxian-}"     # → 0.2.0-1.m112.el9
    case "$base" in *-*) ;; *) return 0 ;; esac   # 无 release 段 ⇒ 非法，跳过
    ver="${base%%-*}"          # → 0.2.0
    rel="${base#*-}"           # → 1.m112.el9
    rel_lc="$(printf '%s' "$rel" | tr 'A-Z' 'a-z')"
    ms="$(printf '%s' "$rel_lc" | grep -o 'm[0-9][0-9]*' | head -1 || true)"
    ms="${ms#m}"
    [ -n "$ms" ] || ms=0
    printf '%s-%06d\t%s\n' "$ver" "$ms" "$ver-$rel"
}

# 收集一棵树里全部 puxian rpm 的 key 行（可能为空）
_collect_keys() {   # $1 = 树根
    local tree="$1" f
    [ -d "$tree" ] || return 0
    while IFS= read -r f; do
        [ -n "$f" ] || continue
        _rpm_key_of "$f"
    done < <(find "$tree" -type f -name 'puxian-*.rpm' 2>/dev/null | sort)
}

# 取 key 最大者（整行返回）
_max_line() { sort -V -k1,1 | tail -1; }

# 取 key 最小者的 key 字段
_min_key() { awk -F'\t' '{print $1}' | sort -V | head -1; }

NEW_KEYS="$(_collect_keys "$NEW_TREE")"
OLD_KEYS="$(_collect_keys "$OLD_TREE")"

if [ -z "$(printf '%s' "$NEW_KEYS" | tr -d '[:space:]')" ]; then
    echo "❌ 待发布仓库树里没有任何 puxian-*.rpm: $NEW_TREE" >&2
    echo "   （发布产物不完整 ⇒ 拒绝，避免把空/残仓库 rsync --delete 推上去）" >&2
    exit 2
fi

OLD_MAX="$(printf '%s\n' "$OLD_KEYS" | grep -v '^$' | _max_line)"
NEW_MAX="$(printf '%s\n' "$NEW_KEYS" | grep -v '^$' | _max_line)"
OLD_KEY="${OLD_MAX%%$'\t'*}"
OLD_FULL="${OLD_MAX#*$'\t'}"
NEW_KEY="${NEW_MAX%%$'\t'*}"
NEW_FULL="${NEW_MAX#*$'\t'}"

echo "== 发布单调性守卫（qg-issue 41）=="

if [ -z "$OLD_KEY" ]; then
    echo "   已发布基线: （无 —— gh-pages 尚无 rpm/ 或首次发布）"
    echo "   待发布:     ${NEW_FULL}  [key=${NEW_KEY}]"
    echo "✅ 无基线可比 ⇒ 允许发布（首次/冷启动）"
    exit 0
fi

echo "   已发布基线: ${OLD_FULL}  [key=${OLD_KEY}]"
echo "   待发布:     ${NEW_FULL}  [key=${NEW_KEY}]"

if [ "$OLD_KEY" = "$NEW_KEY" ]; then
    echo "✅ 同版本重发 ⇒ 允许（幂等；内容差异由 gh-pages 的 git diff 体现）"
    exit 0
fi

if [ "$(printf '%s\n%s\n' "$OLD_KEY" "$NEW_KEY" | sort -V | tail -1)" = "$OLD_KEY" ]; then
    # ---- 版本回退：默认拒绝 ----
    echo ""
    echo "❌ 拒绝发布：待发布版本 **低于** 已发布版本（旧覆盖新）"
    echo "   比较: ${NEW_FULL}  <  ${OLD_FULL}"
    echo "   后果: 紧随其后的 rsync -a --delete 会把 rpm/ 与 repodata **整体替换为旧版**，"
    echo "         已升级的用户在 dnf 侧表现为能力回退，且**没有任何告警**。"
    if [ -n "${RPM_ALLOW_REGRESSION:-}" ]; then
        echo ""
        echo "⚠️  RPM_ALLOW_REGRESSION 已设置 ⇒ **人工显式覆盖**"
        echo "     理由: ${RPM_ALLOW_REGRESSION}"
        echo "     审计: 本次将把 ${OLD_FULL} 替换为更旧的 ${NEW_FULL}（此行为已记入流水线日志）"
        exit 0
    fi
    echo ""
    echo "   处置: ① 若这是「重跑旧 tag 的 release」⇒ 不要发布；"
    echo "         ② 若确需回退（如紧急撤回坏版本）⇒ 在 workflow 里显式设"
    echo "            RPM_ALLOW_REGRESSION=<理由> 覆盖，禁止静默通过。"
    exit 1
fi

echo "✅ 允许发布（单调不减）"
NEW_MIN_KEY="$(printf '%s\n' "$NEW_KEYS" | grep -v '^$' | _min_key)"
if [ -n "$NEW_MIN_KEY" ] && [ "$NEW_MIN_KEY" != "$OLD_KEY" ] \
   && [ "$(printf '%s\n%s\n' "$OLD_KEY" "$NEW_MIN_KEY" | sort -V | tail -1)" = "$OLD_KEY" ]; then
    echo "   ⚠ 待发布树内含低于基线的条目（min=${NEW_MIN_KEY}）—— 检查各 dist 是否同批构建"
fi
exit 0
