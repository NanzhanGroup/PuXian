#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────
# packaging/chain/watch_chain.sh —— 通用「等门 ⇒ 裁决 ⇒ 推送」观察器
#
# 每一轮手工写的 watcher 都重复了三件事：等门结束、判绿红、推 tag。
# 而「判绿红」正是出事的地方（缺陷 495：判据串凭空捏造）。
# ⇒ 本脚本把那一步**收敛到唯一权威** `gate_verdict.sh`，各轮不再自己写判据串。
#
# 用法：
#   watch_chain.sh <里程碑号> <门日志> --pid <门PID> [--times t.tsv]
#                  [--runner run_gates.sh] [--tag-push] [--dry]
# 行为：
#   GREEN        ⇒ resolve-topic m<N> · make_tag --milestone N --push（默认）· p0
#   RED          ⇒ p1 m<N>-red（挂 p1 即**唤醒**东月 —— 见下方 v2/v3 说明）
#   INCONSISTENT ⇒ p1 m<N>-verdict（**措辞刻意不同**：这不是「门红」，是判据问题）
#   PREMISE-FAIL ⇒ p1 m<N>-premise（裁决器与门实现脱节）
#
# ⚠️ v2（2026-10-11，用户令）：「无人值守失败」**不该推给用户** ——
#   用户原话：「推给我，难道我能起来干活吗？那就不应该推给我，应该是推给你，
#   你干完活给我报告，我不需要这种无人值守的脚本推送消息。」
#   ⇒ 红/不一致/前提不成立时**只挂 p1** —— **叫醒那一步由 `dy-notify.sh p1` 自己完成**：
#     v2.5 起它写**交接单** + `ws-core -task-park`，走文殊系统原生任务自驱（唤醒时把交接单
#     正文喂进提示词 ⇒ 有上下文；自驱任务不受黑名单限制），不再伪造 QQ 消息注入。
#   `dy-notify.sh esc` 仍保留，作为 45 分钟仍未了结的兜底（同样是自驱，不是推用户）。
#   ⚠️ v3：本脚本**已删掉自己那次 wake**（两条并行通道会一次事故叫醒两个会话 —— 见下方 v3 注释）。
#
# ⚠️ 三条纪律（都是实测换来的）：
#   ① **等待用 PID，不用日志内容** —— 追加式日志里的旧 FAIL 行会让「曾经失败」
#      冒充「现在失败」（M280：watcher 因此自杀、M280 永不启动）。
#   ② **state 判据只认 origin 上的 tag**（幂等）—— 本地文件、日志行都可能陈旧。
#   ③ 裁决为 INCONSISTENT 时**绝不**挂 m<N>-red —— 措辞不同是有意的：
#      「门红」要去看门，「判据不一致」要去看判据，混在一起会把人带错方向。
# ─────────────────────────────────────────────────────────────────────
set -uo pipefail
cd "$(cd "$(dirname "$0")/../.." && pwd)"

N="${1:-}"; LOG="${2:-}"
[ -n "$N" ] && [ -n "$LOG" ] || {
    echo "用法：watch_chain.sh <里程碑号> <门日志> --pid <门PID> [--times t.tsv]" >&2
    echo "      [--runner run_gates.sh] [--tag-push] [--dry]" >&2
    exit 4
}
shift 2
PID=""; TIMES=""; RUNNER="selfhost/run_gates.sh"; TAGPUSH=1; DRY=0
while [ $# -gt 0 ]; do
    case "$1" in
        --pid)     PID="${2:-}"; shift 2 ;;
        --times)   TIMES="${2:-}"; shift 2 ;;
        --runner)  RUNNER="${2:-}"; shift 2 ;;
        --no-push) TAGPUSH=0; shift ;;
        --dry)     DRY=1; shift ;;
        *) echo "未知参数: $1" >&2; exit 4 ;;
    esac
done

N_BIN="$(command -v dy-notify.sh 2>/dev/null || true)"
[ -n "$N_BIN" ] || N_BIN="$(command -v /usr/local/sbin/dy-notify.sh 2>/dev/null || true)"
[ -n "$N_BIN" ] || N_BIN="$(command -v /usr/local/sbin/dy-notify 2>/dev/null || true)"
[ -n "$N_BIN" ] || { N_BIN="$(ls /usr/local/sbin/dy-notify* 2>/dev/null | head -1)"; }
notify() {
    [ -n "$N_BIN" ] || return 0
    if [ "$DRY" = 1 ]; then printf '   [dry] %s\n' "$*"; return 0; fi
    "$N_BIN" "$@" >/dev/null 2>&1 || true
}

# ⚠️ v3（2026-10-11）：**本脚本不再自己调 dy-wake.sh**。
#   到此为止 `wake()` 与 `notify p1` 是**两条并行的唤醒通道**（文殊系统原生自驱 +
#   伪造 QQ 消息注入）—— 一次事故会被叫醒**两个会话**、同抢一个仓库：M299 实测
#   11:58 `dy-wake` 注入 + 12:00 `buswake` 总线唤醒，两个会话互踩（缺陷 517）。
#   既然 `dy-notify.sh p1` 自 v2.5 起已改走**文殊系统原生任务自驱**（写交接单 +
#   `ws-core -task-park`，唤醒时把交接单正文喂进提示词 ⇒ 有上下文；自驱任务不受
#   黑名单限制），这里再 wake 一次纯属重复 ⇒ **删掉 `wake()` 与 W_BIN 探测**。
#   （`dy-wake.sh` 文件保留在 /usr/local/sbin/ 仅作回滚，任何脚本都不再调用它。）

# ── ①.5 等待前提守卫（缺陷 495 同族 · 2026-10-11）────────────────────
#   症状（M301 实测）：观察器等的是 PID-A（已结束），却拿一份属于**仍在跑的
#   PID-B** 的日志去裁决 ⇒ 日志里没有汇总行 ⇒ 立刻 INCONSISTENT 挂 p1（误报）。
#   错不在「等待用 PID」（纪律①没错），而在**这个 PID 指错了对象**：同一份日志
#   被先后两场门写。补救：PID 结束后若目标日志**没有汇总行**，先别裁决 ——
#     ① 宽限等待 N 秒（写者可能尚在刷新）；② 校验「该日志是否正被另一活门写」
#        （锁文件里的 PID 仍存活 / 日志仍在增长 / /proc 里有人开着这个文件）。
#   命中 ⇒ 报「未跑完·继续等」并接着等，而不是立刻 INCONSISTENT 挂 p1。
WAIT_GUARD_GRACE="${WAIT_GUARD_GRACE:-20}"                     # 每轮宽限秒数
WAIT_GUARD_MAX="${WAIT_GUARD_MAX:-3}"                          # 最多宽限轮数
WAIT_GUARD_LOCKS="${WAIT_GUARD_LOCKS-/tmp/.m116_gates.lock /tmp/.px_gate_lock}"

log_has_summary() {   # 0=有汇总行（门跑完）
    awk '/汇总：失败/ { f = 1 } END { exit(f ? 0 : 1) }' "$LOG" 2>/dev/null
}
live_gate_writer() {  # 打印正在写 $LOG 的活进程 PID（无则 rc=1）
    local lk p fd tgt
    for lk in $WAIT_GUARD_LOCKS; do
        [ -f "$lk" ] || continue
        p="$(tr -dc '0-9' <"$lk" 2>/dev/null || true)"; [ -n "$p" ] || continue
        kill -0 "$p" 2>/dev/null && { printf '%s\n' "$p"; return 0; }
    done
    for fd in /proc/[0-9]*/fd/*; do
        tgt="$(readlink "$fd" 2>/dev/null || true)"; [ "$tgt" = "$LOG" ] || continue
        p="${fd#/proc/}"; p="${p%%/*}"
        [ "$p" = "$$" ] && continue
        kill -0 "$p" 2>/dev/null && { printf '%s\n' "$p"; return 0; }
    done
    return 1
}
wait_gate_finished() {
    local round=0 s1 s2 writer
    while :; do
        log_has_summary && return 0
        round=$((round + 1))
        [ "$round" -le "$WAIT_GUARD_MAX" ] || return 0   # 宽限用尽 ⇒ 交裁决器
        printf '⏳ 无汇总行：宽限 %ss 观察写者（第 %s/%s 轮）…\n' "$WAIT_GUARD_GRACE" "$round" "$WAIT_GUARD_MAX"
        s1="$(wc -c <"$LOG" 2>/dev/null || echo 0)"
        sleep "$WAIT_GUARD_GRACE"
        s2="$(wc -c <"$LOG" 2>/dev/null || echo 0)"
        writer="$(live_gate_writer || true)"
        if [ "$s2" -gt "$s1" ] || [ -n "$writer" ]; then
            printf '   ↳ 未跑完：日志 %s→%s 字节%s ⇒ **继续等**（不裁决、不挂 p1）\n' \
                "$s1" "$s2" "${writer:+ · 活门 PID=$writer}"
            if [ -n "$writer" ]; then
                while kill -0 "$writer" 2>/dev/null; do sleep 10; done
                printf '   ↳ 活门 PID=%s 已结束\n' "$writer"
            fi
            continue
        fi
        printf '   ↳ 日志不再增长且无活门写者 ⇒ 视作真·未跑完/被中止，交裁决器\n'
        return 0
    done
}

# ── ① 等门（按 PID —— 不用日志内容当状态）──────────────────────────
if [ -n "$PID" ]; then
    printf '等门 PID=%s …\n' "$PID"
    while kill -0 "$PID" 2>/dev/null; do sleep 60; done
fi
printf '门已结束：%s\n' "$(date '+%F %T')"

# ── ①.5 等待前提守卫：日志没跑完就别急着裁决 ───────────────────────
wait_gate_finished || true

# ── ② 裁决（唯一权威）──────────────────────────────────────────────
VB="packaging/chain/gate_verdict.sh"
ARGS=("$LOG" --runner "$RUNNER")
[ -n "$TIMES" ] && ARGS+=(--times "$TIMES")
OUT="$(bash "$VB" "${ARGS[@]}" 2>&1)"; rc=$?
printf '%s\n' "$OUT"
VERD="$(printf '%s\n' "$OUT" | awk '/^VERDICT=/{sub(/^VERDICT=/,""); print; exit}')"

case "$VERD" in
GREEN)
    notify resolve-topic "m$N"
    if [ "$TAGPUSH" = 1 ]; then
        if [ "$DRY" = 1 ]; then
            printf '   [dry] make_tag --milestone %s --push\n' "$N"
        else
            bash packaging/make_tag.sh --milestone "$N" --push || {
                notify p1 "m$N-tag" "make_tag rc≠0（门已绿但推送失败）" "见 stdout"
            }
        fi
    fi
    notify p0 "M$N 全量门绿 · 已推 main + tag v0.2.$N" "裁决：$VERD（多路一致）"
    ;;
RED)
    RL="$(printf '%s\n' "$OUT" | awk '/^  红门：/{sub(/^  红门：/,""); print}')"
    [ -n "$RL" ] || RL="（清单为空 —— 这不是门红，请看 INCONSISTENT 分支）"
    notify p1 "m$N-red" "M$N 全量门红（未推送）" "红门：$RL"
    ;;
INCONSISTENT)
    notify p1 "m$N-verdict" "M$N 门裁决不一致（未推送）" \
"判据/日志互相矛盾，**不是门红** —— 按缺陷 495 的处置：先查判据串，别查门。
详见 stdout / $LOG。"
    ;;
*)
    notify p1 "m$N-premise" "M$N 裁决前提不成立" "裁决器与门实现脱节（rc=$rc）⇒ 人工介入。"
    ;;
esac
exit "$rc"
