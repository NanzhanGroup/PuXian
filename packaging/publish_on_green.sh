#!/usr/bin/env bash
# ============================================================================
# publish_on_green.sh —— 发布机械段【唯一实现】（M388 起）
#
# 为什么要单一实现：
#   m380 / m381b / m382 / m383 / m384 / m386 / m387 七份几乎一样的
#   publish_on_green.sh，每轮复制一份再改 ⇒ 缺陷跟着复制：699（超时窗口）
#   / 703（抢锁静默） / 704（令牌 401） 每一轮都在同一个位置重犯一次。
#   本轮把这些踩过的坑一次性写进这一份，里程碑只传参数。
#
# 用法：
#   publish_on_green.sh --ms 388 [--stamp FILE] [--notify FILE] [--repo DIR]
#                       [--mirror-url URL] [--no-push] [--dry-run]
#
#   --ms        里程碑号（决定 tag v0.2.<ms>、默认 STAMP、锁、日志）
#   --stamp     门链 STAMP 文件（默认 /data/autodrive/M<ms>-GATE-DONE）
#   --notify    用户通知正文文件（纯文本；**不含** tag sha，由本脚本尾部追加）
#   --repo      仓库（默认 /data/code/puxian）
#
# 输出：/data/autodrive/m<ms>-publish.log（纯文本时间线）
#
# 自锁五道：不绿不发 · 树脏不发 · head 变了不发 · 落后不发 · 抢不到锁不发
# 退出码：0 已发布 / 1 判红不可发 / 2 前置条件不满足 / 3 孤儿锁（已 p1）
# ============================================================================
set -uo pipefail

MS=""; STAMP=""; NOTIFY=""; REPO=/data/code/puxian
MIRROR_URL=""; DO_PUSH=1; DRY=0

while [ $# -gt 0 ]; do
    case "$1" in
        --ms)         MS="${2:-}"; shift 2 ;;
        --stamp)      STAMP="${2:-}"; shift 2 ;;
        --notify)     NOTIFY="${2:-}"; shift 2 ;;
        --repo)       REPO="${2:-}"; shift 2 ;;
        --mirror-url) MIRROR_URL="${2:-}"; shift 2 ;;
        --no-push)    DO_PUSH=0; shift ;;
        --dry-run)    DRY=1; shift ;;
        -h|--help)    sed -n '2,30p' "$0"; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done
[ -n "$MS" ] || { echo "--ms 必填" >&2; exit 2; }
case "$MS" in ''|*[!0-9]*) echo "--ms 必须是纯数字：$MS" >&2; exit 2 ;; esac

AD="${DY_PUBLISH_AD:-/data/autodrive}"
STAMP="${STAMP:-$AD/M${MS}-GATE-DONE}"
LOG="$AD/m${MS}-publish.log"
# 测试缝：门在 CI/临时目录里跑，不能碰系统 /var/lock
LOCK="${DY_PUBLISH_LOCK_DIR:-/var/lock}/m${MS}-publish.lock"
TAG="v0.2.$MS"
NOTIFY_SH="${DY_NOTIFY_BIN:-/usr/local/sbin/dy-notify.sh}"   # 可覆盖：便于自测注入假件
BODY_SH="$AD/publish_notify_body.sh"
mkdir -p "$AD" 2>/dev/null || true

say() { echo "[$(date +%F' '%T)] $*" >> "$LOG"; }
die() { say "ABORT $*"; exit 2; }

# ── 缺陷 703（2026-11-10 19:04 实测）：抢锁失败 exit 0 = 静默失效 ──────────
#   老写法 `flock -n 9 || exit 0` 把两种情况混成一件事：
#     (a) 真有一个 publish 在跑 —— 跳过是对的，exit 0 也对；
#     (b) 锁被孤儿持有（父进程被 kill，sleep 子进程继承了 fd 9）—— 此时
#         新 publish 永远抢不到锁 ⇒ 永远不发布，而日志只有一行 SKIP、rc=0
#         ⇒ 不挂 p1、不算失败、没人知道（19:04 我差点当成"在等戳"放过去）。
#   修法：抢不到锁时查持锁进程，能判定是"非 publish 进程"⇒ 判孤儿锁，
#         出声（p1 叫人）并 exit 3；只有确认是同脚本在跑才安静 exit 0。
exec 9>"$LOCK" 2>/dev/null || die "无法打开锁文件 $LOCK"
if ! flock -n 9; then
    # 注意（M388 自测抓出）：持锁的可能不止一个进程 —— 主脚本 + 它 fork 的
    # 子进程都继承了 fd 9，fuser 会把这些 pid 全列出来。只要**有一个**的
    # cmdline 是本脚本，就说明锁是正经发布进程持有的（不是孤儿）。
    # ★必须排除自己（$$）：本脚本 flock 失败前已经 exec 9> 打开了锁文件，
    #   fuser 会把自己也列进来，cmdline 天然含 publish_on_green ⇒ 不排除就是
    #   「永远判定「有人正常持锁」⇒ 孤儿锁永远发现不了」= 判据自己把自己骗过。
    ORPHAN=1; HOLDER=""; CMD=""
    for _p in $(fuser "$LOCK" 2>/dev/null); do
        [ "$_p" = "$$" ] && continue
        [ -n "$HOLDER" ] || HOLDER="$_p"
        [ -r "/proc/$_p/cmdline" ] || continue
        _c="$(tr '\0' ' ' < "/proc/$_p/cmdline" 2>/dev/null)"
        case "$_c" in *publish_on_green*) ORPHAN=0; CMD="$_c" ;; esac
    done
    if [ "$ORPHAN" = "1" ]; then
        say "WARN-LOCK-ORPHAN holder=${HOLDER:-unknown} cmd=${CMD:-读不到} —— 孤儿锁，新发布永远抢不到"
        if [ "$DRY" = "0" ] && [ -x "$NOTIFY_SH" ]; then
            DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 "m${MS}-publish-orphan-lock" \
                "M${MS} 发布锁被非发布进程持有：pid=${HOLDER:-unknown}" \
                "cmd=${CMD:-读不到} lock=$LOCK" \
                "后果：新发布静默退出、tag 永不落地。处置：kill $HOLDER 后重起" || true
        fi
        exit 3
    fi
    say "SKIP-LOCKED holder=${HOLDER:-?}（同一脚本在跑，正常退出）"
    exit 0
fi

say "=== publish_on_green start ms=$MS pid=$$ repo=$REPO ==="

# ── 1 等 STAMP（240 × 60s = 4h） ───────────────────────────────────────────
SEEN=0
for _i in $(seq 1 240); do
    [ -f "$STAMP" ] && { SEEN=1; break; }
    sleep 60 9>&-          # 9>&- ：子进程不继承锁 fd（缺陷 703 的根因就是这个继承）
done
[ "$SEEN" = "1" ] || {
    say "TIMEOUT 4h 未见 STAMP=$STAMP"
    [ "$DRY" = "0" ] && [ -x "$NOTIFY_SH" ] && DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 \
        "m${MS}-gate-stamp-missing" "M${MS} 全量门 4h 未落 STAMP，门进程可能被杀" \
        "STAMP=$STAMP 不存在" || true
    exit 2
}

RES="$(tr '\n' ' ' < "$STAMP" 2>/dev/null)"
say "STAMP=$RES"

# ── 2 判绿（缺陷 696：判据只认 chain_verdict.sh，不许自己 grep） ────────────
if ! grep -q 'CHAIN-VERDICT=GREEN' "$STAMP"; then
    say "VERDICT-NOT-GREEN —— 不发布（判门段已 p1，这里不重复推用户）"
    exit 1
fi

cd "$REPO" || die "no-repo $REPO"
HEAD="$(git rev-parse --short=7 HEAD)"

# ── 3 head 一致（门跑的是不是现在这棵树） ──────────────────────────────────
STAMP_HEAD="$(grep -oE 'head=[0-9a-f]{7}' "$STAMP" 2>/dev/null | head -1 | cut -d= -f2)"
if [ -n "${STAMP_HEAD:-}" ] && [ "$STAMP_HEAD" != "$HEAD" ]; then
    say "ABORT head-mismatch stamp=$STAMP_HEAD now=$HEAD"
    [ "$DRY" = "0" ] && [ -x "$NOTIFY_SH" ] && DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 \
        "m${MS}-head-mismatch" "M${MS} 门绿但 HEAD 已变：stamp=$STAMP_HEAD 现=$HEAD" "不发布" || true
    exit 2
fi

# ── 4 树必须干净 ───────────────────────────────────────────────────────────
DIRTY="$(git status --porcelain | head -5)"
if [ -n "$DIRTY" ]; then
    say "ABORT dirty-tree"
    [ "$DRY" = "0" ] && [ -x "$NOTIFY_SH" ] && DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 \
        "m${MS}-dirty-tree" "M${MS} 门绿但工作树脏，拒绝发布" "$DIRTY" || true
    exit 2
fi

# ── 5 不落后（缺陷 682/690：用 ls-remote 不 fetch；方向是 remote 为 HEAD 祖先） ─
REMOTE_SHA="$(timeout 60 git ls-remote origin refs/heads/main 2>/dev/null | cut -f1)"
[ -n "$REMOTE_SHA" ] || die "fetch-failed（ls-remote 空）"
if ! git merge-base --is-ancestor "$REMOTE_SHA" HEAD 2>/dev/null; then
    say "ABORT behind（远端 $REMOTE_SHA 不是 HEAD $HEAD 的祖先）"
    [ "$DRY" = "0" ] && [ -x "$NOTIFY_SH" ] && DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 \
        "m${MS}-behind" "M${MS} 落后 origin/main，不发布" "head=$HEAD remote=$REMOTE_SHA" || true
    exit 2
fi

OUTD="${DY_PUBLISH_OUT:-/data/gatevm}/m${MS}-publish-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$OUTD" 2>/dev/null || true
say "head=$HEAD 起 make_tag（main+tag 同一次 push）"

# ── 6 打 tag（通知由本脚本统一发，禁用 make_tag 自己的 hook） ───────────────
#   缺陷 698-③（2026-11-10 20:37 实测）：make_tag 的 notify_release.sh 发一条
#   （20:37:07「已发布」），本脚本又发一条（20:37:10「已打 tag」）⇒ 用户手机
#   一次发布收到 2 条。两者 title 不同，hook 内的「同 tag 同状态幂等」挡不住。
#   修法：本条链只留一个出口 —— 把 make_tag 的 hook 指向 /bin/true（可执行、
#   零副作用），由本脚本在 tag 核实之后发唯一一条（信息更全）。
export PX_RELEASE_NOTIFY_HOOK=/bin/true

if [ "$DRY" = "1" ]; then
    say "DRY-RUN 到此为止（不打 tag、不推送）"
    exit 0
fi
if [ "$DO_PUSH" = "0" ]; then
    say "NO-PUSH 模式：只打 tag 不推"
    bash packaging/make_tag.sh --milestone "$MS" >> "$OUTD/tag.log" 2>&1
else
    bash packaging/make_tag.sh --milestone "$MS" --push >> "$OUTD/tag.log" 2>&1
fi
TRC=$?

# ── 7 验远端 tag 指向（缺陷 697：单次 ls-remote 有传播延迟，重试 12×5s） ───
TAGSHA=""
if [ "$DO_PUSH" = "1" ]; then
    for _k in $(seq 1 12); do
        TAGSHA="$(git ls-remote --tags origin "refs/tags/$TAG^{}" 2>/dev/null | awk '{print substr($1,1,7)}')"
        [ "${TAGSHA:-}" = "$HEAD" ] && break
        say "tag 未见（try=$_k got=${TAGSHA:-无} want=$HEAD）5s 后重试"
        sleep 5 9>&-
    done
else
    TAGSHA="$(git rev-parse --short=7 "refs/tags/$TAG^{}" 2>/dev/null || echo 无)"
fi
say "tag-rc=$TRC $TAG=${TAGSHA:-无} want=$HEAD"

if [ "${TAGSHA:-}" != "$HEAD" ]; then
    say "NO-GO tag-mismatch"
    [ -x "$NOTIFY_SH" ] && DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p0 \
        "M${MS} 门绿但 tag 未落到位" "tag=${TAGSHA:-无} want=$HEAD rc=$TRC" \
        "证据 $OUTD/tag.log" || true
    exit 1
fi

# ── 8 用户通知（唯一出口；缺陷 698-②：sha 必须展开，不能写成字面 $TAGSHA） ──
BODY=""
if [ -n "$NOTIFY" ] && [ -r "$NOTIFY" ]; then
    BODY="$(cat "$NOTIFY")"
else
    BODY="$(git log -1 --pretty=%s)"
fi
NOTIFY_OK=0
if [ -x "$NOTIFY_SH" ]; then
    # 正文拼装交给唯一实现（缺陷 698-②：sha 只能从参数来，不可能写成字面量）
    if [ -x "$BODY_SH" ]; then
        bash "$BODY_SH" "$TAG" "$TAGSHA" "$BODY" "$MIRROR_URL" > "$OUTD/notify.txt" 2>/dev/null
    else
        printf '%s\n\ntag %s → %s\n' "$BODY" "$TAG" "$TAGSHA" > "$OUTD/notify.txt"
    fi
    DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" user "PuXian $TAG 已发布（镜像同步中）" \
        "$(cat "$OUTD/notify.txt")" && NOTIFY_OK=1
    say "USER-NOTIFY-$([ "$NOTIFY_OK" = "1" ] && echo OK || echo FAIL)"
else
    say "WARN dy-notify 不存在，跳过用户通知"
fi

# ── 9 起观察器（选配：给了 --mirror-url 才起） ─────────────────────────────
if [ -n "$MIRROR_URL" ]; then
    WATCHER="$AD/m${MS}-release-watch.sh"
    if [ -x "$WATCHER" ]; then
        systemctl reset-failed "m${MS}-release-watch" 2>/dev/null || true
        systemd-run --unit="m${MS}-release-watch" --collect \
            --description="M${MS} release watcher" "$WATCHER" >/dev/null 2>&1 &
        sleep 4 9>&-
        say "watcher=$(systemctl is-active "m${MS}-release-watch" 2>/dev/null || echo unknown)"
    else
        say "WARN 观察器不存在：$WATCHER"
    fi
fi

say "END PUBLISHED tag=$TAG@$TAGSHA evidence=$OUTD notify=$NOTIFY_OK"
exit 0
