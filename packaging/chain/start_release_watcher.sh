#!/usr/bin/env bash
# ============================================================================
# start_release_watcher.sh —— 起发布观察器【唯一实现 · 带自测】（M394 · 缺陷 745）
# ----------------------------------------------------------------------------
# 为什么单独成件：
#   2026-11-12 01:31 M392 判绿时，publish_on_green.sh 第 9 段只打印
#     WARN 观察器不存在：/data/autodrive/m392-release-watch.sh
#   就继续往下走了（rc 仍 0）。后果不是「少一条日志」，是：
#     镜像同步完成时**没有任何人会**发「可 dnf upgrade」；
#     而用户那条「已发布」通知恰好 3 次投递失败被放弃 ⇒ 他整晚不知道发布成功。
#   ⇒ 起观察器这件事必须有判据：**起来了才算数，没起来要 p1 叫人**。
#   ⇒ 而且它必须能离线自测（否则下一次还是"我以为是好的"）。
#
# 用法：start_release_watcher.sh --ms 394
#          [--repo DIR]        仓库（默认 $DY_REPO 或 <本脚本>/../..）
#          [--mirror-url URL]  透传给观察器
#          [--ad DIR]          台账目录（默认 /data/autodrive）
#          [--notify-bin PATH] dy-notify.sh（默认 /usr/local/sbin/dy-notify.sh）
#          --selftest          离线自测（不碰 systemd / 不通知）
# 退出码：0 观察器在跑或已正常收工 · 3 观察器缺失或没起来（已 p1）· 2 参数错
# ============================================================================
set -uo pipefail

MS_RAW=""; REPO=""; MIRROR=""; AD="${DY_WATCH_AD:-/data/autodrive}"
NOTIFY_SH="${DY_NOTIFY_BIN:-/usr/local/sbin/dy-notify.sh}"
SELFTEST=0
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="${DY_REPO:-$(cd "$HERE/../.." && pwd)}"

while [ $# -gt 0 ]; do
    case "$1" in
        --ms)          MS_RAW="${2:-}"; shift 2 ;;
        --repo)        REPO="${2:-}"; shift 2 ;;
        --mirror-url)  MIRROR="${2:-}"; shift 2 ;;
        --ad)          AD="${2:-}"; shift 2 ;;
        --notify-bin)  NOTIFY_SH="${2:-}"; shift 2 ;;
        --selftest)    SELFTEST=1; shift ;;
        -h|--help)     sed -n '2,25p' "$0"; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done

norm_ms() {
    local s="$(printf '%s' "${1:-}" | sed -E 's/^[[:space:]]+//; s/[[:space:]]+$//; s/^[mM]//; s/^v?[0-9]+\.[0-9]+\.//')"
    case "$s" in ''|*[!0-9]*) return 1 ;; esac
    printf '%s' "$s"
}

# ── 可注入的两处（自测用）：起单元 / 查状态 ──────────────────────────────
# RUN_HOOK：非空 ⇒ 不调 systemd-run，只把「将要执行的东西」写进该文件（离线可断言）
# STATE_HOOK：非空 ⇒ 不调 systemctl，直接读该文件内容当状态
run_unit() {  # run_unit <unit> <script> [args...]
    local unit="$1" script="$2"; shift 2
    if [ -n "${RUN_HOOK:-}" ]; then
        printf 'unit=%s script=%s args=%s\n' "$unit" "$script" "$*" >> "$RUN_HOOK"
        return 0
    fi
    systemctl stop "$unit" 2>/dev/null || true
    systemctl reset-failed "$unit" 2>/dev/null || true
    # ⚠️ 不加 --collect：观察器可能秒完成（镜像早已齐），--collect 会把单元立刻回收，
    #    随后 systemctl show 查不到 ⇒ 「已完成」被误判成「没起来」（2026-11-12 10:51 实测）。
    systemd-run --unit="$unit" --description="M${MS} release watcher" \
        /bin/bash "$script" "$@" >/dev/null 2>&1 &
    sleep "${WATCH_START_WAIT:-4}" 9>&-
}
# 判据口径（这条是实测换来的）：is-active 只回答「是否在跑」，不回答「是否起来了」。
#   观察器一轮就判 OK 时会在 4 秒内跑完退出 ⇒ is-active=inactive ⇒ 被误判没起来。
#   ⇒ 必须看 ActiveState + Result 两者：
#        running(active|activating|reloading)  ⇒ 起来了
#        finished(ok) = inactive + Result=success ⇒ 起来了且已正常收工
#        bad(...)   = failed / Result≠success     ⇒ 起不来
#        missing    = 查不到（被回收或从未创建）  ⇒ 没起来
state_unit() {  # state_unit <unit>
    if [ -n "${STATE_HOOK:-}" ]; then cat "$STATE_HOOK"; return 0; fi
    local st sub res
    st="$(systemctl show -p ActiveState --value "$1" 2>/dev/null)"
    sub="$(systemctl show -p SubState --value "$1" 2>/dev/null)"
    res="$(systemctl show -p Result --value "$1" 2>/dev/null)"
    if [ -z "${st:-}" ]; then printf 'missing\n'; return 0; fi
    case "$st" in
        active|activating|reloading) printf 'running(%s)\n' "$st" ;;
        inactive)
            if [ "${res:-}" = success ]; then printf 'finished(ok)\n'
            else printf 'bad(state=%s,sub=%s,result=%s)\n' "$st" "${sub:-?}" "${res:-?}"; fi ;;
        *) printf 'bad(state=%s,sub=%s,result=%s)\n' "$st" "${sub:-?}" "${res:-?}" ;;
    esac
}
notify_up() {  # notify_up <key> <title> <body>
    if [ -n "${NOTIFY_HOOK:-}" ]; then
        printf '%s | %s | %s\n' "$1" "$2" "$3" >> "$NOTIFY_HOOK"
        return 0
    fi
    [ -x "$NOTIFY_SH" ] || return 0
    DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 "$1" "$2" "$3" >/dev/null 2>&1 || true
}

# ── 自测（离线：不碰 systemd、不通知、不联网）────────────────────────────
if [ "$SELFTEST" = 1 ]; then
    pass=0; fail=0
    chk() {
        if [ "$2" = "$3" ]; then pass=$((pass+1)); printf 'ok   %-40s => %s\n' "$1" "$3"
        else fail=$((fail+1)); printf 'FAIL %-40s 期望 %s 实得 %s\n' "$1" "$2" "$3"; fi
    }
    chk_proof() {
        local b="$fail"
        chk "P0 前提自证（故意错）" OK FAIL >/dev/null
        if [ "$fail" -eq $((b+1)) ]; then
            pass=$((pass+1)); printf 'ok   %-40s => 断言机制确实能失败\n' "P0 前提自证"
        else printf 'FAIL %-40s 故意错未被计入\n' "P0 前提自证"; fi
        fail="$b"
    }
    T="$(mktemp -d /tmp/rw-wiring.XXXXXX)"
    mkdir -p "$T/repo/packaging/chain" "$T/ad"
    printf '#!/usr/bin/env bash\nexit 0\n' > "$T/repo/packaging/chain/release_watch.sh"
    chmod +x "$T/repo/packaging/chain/release_watch.sh"
    printf '#!/usr/bin/env bash\nprintf "%%s\\n" "$@" >/dev/null\n' > "$T/fake-notify.sh"
    chmod +x "$T/fake-notify.sh"
    export RUN_HOOK="$T/run.log" NOTIFY_HOOK="$T/notify.log"
    printf 'running(active)\n' > "$T/state-running"
    printf 'finished(ok)\n' > "$T/state-finished"
    printf 'bad(state=failed,sub=failed,result=exit-code)\n' > "$T/state-failed"
    printf 'missing\n' > "$T/state-missing"
    : > "$RUN_HOOK"; : > "$NOTIFY_HOOK"

    chk_proof

    # C1 仓库版存在 + 在跑 ⇒ rc=0，且真的调了仓库版并带上 --ms
    STATE_HOOK="$T/state-running" bash "$0" --ms 394 --repo "$T/repo" --ad "$T/ad" \
        --mirror-url https://example.invalid/puxian --notify-bin "$T/fake-notify.sh" >/dev/null 2>&1
    chk "C1 running ⇒ rc=0" 0 "$?"
    rl="$(cat "$RUN_HOOK")"
    case "$rl" in *packaging/chain/release_watch.sh*) chk "C1 调的是仓库版" 1 1 ;;
                  *) chk "C1 调的是仓库版" 1 0 ;; esac
    case "$rl" in *"--ms 394"*) chk "C1 透传 --ms 394" 1 1 ;;
                  *) chk "C1 透传 --ms 394" 1 0 ;; esac
    case "$rl" in *"unit=m394-release-watch"*) chk "C1 单元名参数化" 1 1 ;;
                  *) chk "C1 单元名参数化" 1 0 ;; esac

    # C3 秒完成（镜像早已齐）也必须算「起来了」—— 2026-11-12 10:51 真踩到这条假红
    : > "$NOTIFY_HOOK"
    STATE_HOOK="$T/state-finished" bash "$0" --ms 394 --repo "$T/repo" --ad "$T/ad" \
        --notify-bin "$T/fake-notify.sh" >/dev/null 2>&1
    chk "C3 finished(ok) ⇒ rc=0（秒完成不是没起来）" 0 "$?"
    [ -s "$NOTIFY_HOOK" ] && chk "C3 秒完成不许报 p1" 0 1 || chk "C3 秒完成不许报 p1" 0 0

    # C2 仓库版缺失、legacy 存在 ⇒ 用 legacy（DEPRECATED），仍 rc=0
    : > "$RUN_HOOK"
    rm -f "$T/repo/packaging/chain/release_watch.sh"
    printf '#!/usr/bin/env bash\nexit 0\n' > "$T/ad/m394-release-watch.sh"
    chmod +x "$T/ad/m394-release-watch.sh"
    STATE_HOOK="$T/state-running" bash "$0" --ms 394 --repo "$T/repo" --ad "$T/ad" \
        --notify-bin "$T/fake-notify.sh" >/dev/null 2>&1
    chk "C2 legacy 回退 ⇒ rc=0" 0 "$?"
    rl="$(cat "$RUN_HOOK")"
    case "$rl" in *"m394-release-watch.sh"*) chk "C2 用的是 legacy" 1 1 ;;
                  *) chk "C2 用的是 legacy" 1 0 ;; esac

    # N1 两处都没有 ⇒ rc=3 且 p1 点名 watcher-missing（这正是 M392 那晚的形状）
    : > "$RUN_HOOK"; : > "$NOTIFY_HOOK"
    rm -f "$T/ad/m394-release-watch.sh"
    STATE_HOOK="$T/state-running" bash "$0" --ms 394 --repo "$T/repo" --ad "$T/ad" \
        --notify-bin "$T/fake-notify.sh" >/dev/null 2>&1
    chk "N1 两处都缺 ⇒ rc=3" 3 "$?"
    grep -q 'watcher-missing' "$NOTIFY_HOOK" 2>/dev/null && chk "N1 报了 watcher-missing" 1 1 || chk "N1 报了 watcher-missing" 1 0
    [ -s "$RUN_HOOK" ] && chk "N1 缺件时不许起单元" 0 1 || chk "N1 缺件时不许起单元" 0 0

    # N2 脚本在但起不来（failed）⇒ rc=3 且 p1 = watcher-down（缺陷 745 的另一半）
    printf '#!/usr/bin/env bash\nexit 0\n' > "$T/repo/packaging/chain/release_watch.sh"
    chmod +x "$T/repo/packaging/chain/release_watch.sh"
    : > "$RUN_HOOK"; : > "$NOTIFY_HOOK"
    STATE_HOOK="$T/state-failed" bash "$0" --ms 394 --repo "$T/repo" --ad "$T/ad" \
        --notify-bin "$T/fake-notify.sh" >/dev/null 2>&1
    chk "N2 起不来 ⇒ rc=3" 3 "$?"
    grep -q 'watcher-down' "$NOTIFY_HOOK" 2>/dev/null && chk "N2 报了 watcher-down" 1 1 || chk "N2 报了 watcher-down" 1 0

    # N4 单元查不到（被回收 / 从未创建）⇒ 不许当成功
    : > "$NOTIFY_HOOK"
    STATE_HOOK="$T/state-missing" bash "$0" --ms 394 --repo "$T/repo" --ad "$T/ad" \
        --notify-bin "$T/fake-notify.sh" >/dev/null 2>&1
    chk "N4 查不到单元 ⇒ rc=3" 3 "$?"
    grep -q 'watcher-down' "$NOTIFY_HOOK" 2>/dev/null && chk "N4 报了 watcher-down" 1 1 || chk "N4 报了 watcher-down" 1 0

    # N3 参数校验
    bash "$0" --ms m39x --repo "$T/repo" --ad "$T/ad" >/dev/null 2>&1
    chk "N3 非法 --ms ⇒ rc=2" 2 "$?"

    rm -rf "$T" 2>/dev/null || true
    unset RUN_HOOK NOTIFY_HOOK STATE_HOOK
    echo "SELFTEST pass=$pass fail=$fail"
    [ "$fail" = 0 ]
    exit $?
fi

MS="$(norm_ms "$MS_RAW")" || { echo "--ms 非法（接受 394 / m394 / v0.2.394）：${MS_RAW:-空}" >&2; exit 2; }
UNIT="m${MS}-release-watch"
LOGF="$AD/m${MS}-publish.log"
say() { printf '[%s] %s\n' "$(date +%F' '%T)" "$*" | tee -a "$LOGF" 2>/dev/null || printf '[%s] %s\n' "$(date +%F' '%T)" "$*"; }

# ── 选观察器：仓库版优先，legacy 回退 ───────────────────────────────────
WSCRIPT="$REPO/packaging/chain/release_watch.sh"
KIND=repo
WARGS=(--ms "$MS" --log-dir "$AD" --body-file "$AD/m${MS}_notify_body.txt")
[ -n "$MIRROR" ] && WARGS+=(--mirror-url "$MIRROR")
if [ ! -x "$WSCRIPT" ]; then
    WSCRIPT="$AD/m${MS}-release-watch.sh"
    KIND=legacy
    WARGS=()
    [ -x "$WSCRIPT" ] && say "WARN 回退到 legacy 每轮脚本（DEPRECATED，应升级仓库）：$WSCRIPT"
fi

if [ ! -x "$WSCRIPT" ]; then
    say "WARN 观察器两处都不存在 ⇒ 镜像齐时不会有「可 dnf upgrade」通报（缺陷 745）"
    notify_up "m${MS}-watcher-missing" "M${MS} 发布观察器不存在" \
        "查过两处都没有：
  $REPO/packaging/chain/release_watch.sh
  $AD/m${MS}-release-watch.sh
后果：镜像同步完成时不会有「可 dnf upgrade」通报。"
    exit 3
fi

run_unit "$UNIT" "$WSCRIPT" "${WARGS[@]}"
ST="$(state_unit "$UNIT")"
say "watcher=$ST kind=$KIND unit=$UNIT script=$WSCRIPT args=${WARGS[*]:-无}"
case "$ST" in
    running\(*\)|finished\(ok\)) exit 0 ;;
esac
say "WARN 观察器未起来（state=$ST）⇒ 镜像齐时不会有「可 dnf upgrade」通报"
notify_up "m${MS}-watcher-down" "M${MS} 发布观察器没起来（state=$ST）" \
    "后果：镜像同步完成时不会有「可 dnf upgrade」通报（缺陷 745 的形状）
script=$WSCRIPT kind=$KIND
证据 $LOGF"
exit 3
