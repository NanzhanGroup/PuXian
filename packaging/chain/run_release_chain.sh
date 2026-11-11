#!/usr/bin/env bash
# ============================================================================
# run_release_chain.sh —— 主链本体（参数化）：前置检查 → 全量分片门 → 判据 → 发布
#
# 为什么要有它：M379–M392 每轮都**手写一个 m<N>_chain.sh**（m390_chain.sh / m391_chain.sh /
# m392_chain.sh…），93 行里 80 行是一模一样的骨架，只有里程碑号/锁名/STAMP 名不同。
# 代价不是抄写，是**每轮都可能漏一步**：M390 起链时忘了伴生单元、M392 忘了 --mirror-url。
# ⇒ 把骨架参数化进仓库，里程碑只传 --ms。
#
# 分工：
#   launch_release_chain.sh  起 3 个 systemd 单元（主链 + red-guard + postupgrade）并自检
#   本脚本                    主链单元做的事：前置自证 → 门 → STAMP → publish_on_green
#
# 用法：run_release_chain.sh --ms 393 [--repo DIR] [--mirror-url URL] [--no-publish] [--dry-run]
# 退出码：0 绿并已交发布 / 1 判红（已 p1）/ 2 前置不满足 / 3 已在跑（锁）
# ============================================================================
set -uo pipefail

MS=""; REPO=""; MIRROR_URL="${PX_MIRROR_URL:-https://soft.xiusoft.cn/puxian}"; PUB=1; DRY=0
SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
AD="${PXC_AD:-/data/autodrive}"
STAMP_SH="${PXC_STAMP_SH:-$AD/chain_stamp.sh}"
PUB_SH="${PXC_PUB_SH:-$AD/publish_on_green.sh}"
NOTIFY_SH="${DY_NOTIFY_BIN:-/usr/local/sbin/dy-notify.sh}"

while [ $# -gt 0 ]; do
    case "$1" in
        --ms)         MS="${2:-}"; shift 2 ;;
        --repo)       REPO="${2:-}"; shift 2 ;;
        --mirror-url) MIRROR_URL="${2:-}"; shift 2 ;;
        --no-publish) PUB=0; shift ;;
        --dry-run)    DRY=1; shift ;;
        -h|--help)    sed -n '2,22p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done
[ -n "$MS" ] || { echo "--ms 必填" >&2; exit 2; }
REPO="${REPO:-$(cd "$SELF_DIR/../.." && pwd)}"

# 缺陷 730：两个链实例并发 ⇒ 共用 /tmp 夹具 ⇒ 假红。锁按里程碑分（同里程碑独占）。
LOCK="${PXC_LOCK_DIR:-/var/lock}/m${MS}-chain.lock"
DONE="${PXC_STAMP:-$AD}/M${MS}-GATE-DONE"
TS="$(date +%Y%m%d-%H%M%S)"
LOG="${PXC_LOG_DIR:-/data/gatevm}/m${MS}-chain-$TS.log"
mkdir -p "$(dirname "$LOG")" 2>/dev/null || true

# 自驱会话的 mutate-shim 会让门里的 git 被写闸拦成假红（m392_chain.sh 已踩）
case "$PATH" in *mutate-shim*) echo "❌ PATH 含 mutate-shim ⇒ 拒绝起链" >&2; exit 9 ;; esac
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin LC_ALL=C
export QUIET_LANES="${QUIET_LANES:-3}"

exec 8>"$LOCK" || { echo "无法打开锁 $LOCK" >&2; exit 2; }
flock -n 8 || { echo "ABORT 已有 m${MS}-chain 在跑（$LOCK）" >> "$AD/m${MS}-chain-skip.log"; exit 3; }

cd "$REPO" || exit 2
EXPECT="$(grep -cE '^run ' selfhost/gates.registry.sh)"
say() { echo "== $*" >> "$LOG"; }
{
    echo "== M${MS} chain start $TS head=$(git rev-parse --short HEAD) ver=$(./tools/px --version 2>&1 | head -1)"
    echo "== quiet_lanes=$QUIET_LANES expect_gates=$EXPECT dirty=$(git status --porcelain | wc -l)"
} > "$LOG"

red() {  # red <reason> <键> <标题>
    echo "CHAIN-VERDICT=RED reason=$1" > "$DONE"
    say "RED $1"
    [ "$DRY" = "1" ] || DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 "$2" "$3" \
        "M${MS} head=$(git rev-parse --short HEAD) · 证据 $LOG · 修后重起 launch_release_chain.sh" || true
}

# ① 前置自证（都是 O(秒)：树净 / 版本位 / 14 件重烘一致）
[ -z "$(git status --porcelain)" ] || { red dirty-tree "m${MS}-dirty" "M${MS} 树脏 ⇒ 不起门"; exit 2; }
rm -f "$DONE"
if [ "$DRY" = "1" ]; then
    say "DRY-RUN 前置检查后停止（不起门、不发布）"
    [ -f "$AD/release_preflight.sh" ] && bash "$AD/release_preflight.sh" --root "$REPO" >> "$LOG" 2>&1
    exit 0
fi
if [ -f "$AD/release_preflight.sh" ]; then
    bash "$AD/release_preflight.sh" --root "$REPO" >> "$LOG" 2>&1; vp=$?
    say "version-position rc=$vp"
    [ "$vp" = 0 ] || { red "version-position-$vp" "m${MS}-verpos" "M${MS} 版本位不对（rc=$vp）"; exit 1; }
fi
if ! bash selfhost/rebake_bin.sh --check-all 2>&1 | grep -q '14/14 件与当前源码一致'; then
    red rebake-stale "m${MS}-rebake" "M${MS} 内嵌件未全部重烘（先 --rebake-all）"
    exit 3
fi

# ② 全量分片门
say "起 shard_run.sh $(date +%H:%M:%S)"
bash packaging/chain/shard_run.sh >> "$LOG" 2>&1
rc=$?
say "shard_run rc=$rc $(date +%H:%M:%S)"

# ③ 判据唯一实现（缺陷 696）：本脚本不自己算绿
if [ -f "$STAMP_SH" ]; then
    bash "$STAMP_SH" "$LOG" "$EXPECT" "$DONE" >> "$LOG" 2>&1
else
    echo "CHAIN-VERDICT=RED reason=no-chain-stamp-sh" > "$DONE"
fi
say "stamp: $(cat "$DONE" 2>/dev/null)"

if ! grep -q 'CHAIN-VERDICT=GREEN' "$DONE" 2>/dev/null; then
    red no-go "m${MS}-gate-no-go" "M${MS} 全量门 NO-GO：$(cat "$DONE" 2>/dev/null)"
    exit 1
fi

# ④ 交给唯一的发布实现
[ "$PUB" = "1" ] || { say "NO-PUBLISH：门绿，未发布"; exit 0; }
say "交 publish_on_green $(date +%H:%M:%S)"
bash "$PUB_SH" --ms "$MS" --stamp "$DONE" --repo "$REPO" --mirror-url "$MIRROR_URL" >> "$LOG" 2>&1
prc=$?
say "publish_on_green rc=$prc"
[ "$prc" = 0 ] || exit 1
exit 0
