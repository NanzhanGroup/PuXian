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
#   RED          ⇒ p1 m<N>-red + **dy-wake 叫醒东月**（红门清单）
#   INCONSISTENT ⇒ p1 m<N>-verdict + **dy-wake**（**措辞刻意不同**：这不是「门红」，是判据问题）
#   PREMISE-FAIL ⇒ p1 m<N>-premise + **dy-wake**（裁决器与门实现脱节）
#
# ⚠️ v2（2026-10-11，用户令）：「无人值守失败」**不该推给用户** ——
#   用户原话：「推给我，难道我能起来干活吗？那就不应该推给我，应该是推给你，
#   你干完活给我报告，我不需要这种无人值守的脚本推送消息。」
#   ⇒ 红/不一致/前提不成立时，除挂 p1（供 esc 重试）外**立即 dy-wake 注入一条
#     owner 原生消息**把东月叫醒（走完整会话链路 ⇒ 东月自己修 + 汇报）。
#   p1 仍保留：唤醒失败时由 `dy-notify.sh esc` 在 45 分钟后**再次唤醒**（现在也是唤醒，不是推用户）。
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

# v2：自唤醒（叫醒东月，不是推给用户）—— 见文件头 v2 说明
W_BIN="$(command -v dy-wake.sh 2>/dev/null || true)"
[ -n "$W_BIN" ] || { [ -x /usr/local/sbin/dy-wake.sh ] && W_BIN=/usr/local/sbin/dy-wake.sh; }
[ -n "$W_BIN" ] || W_BIN=""
wake() {
    [ -n "$W_BIN" ] || return 0
    if [ "$DRY" = 1 ]; then printf '   [dry] dy-wake %s\n' "$1"; return 0; fi
    "$W_BIN" "$@" >>"${LOG}.wake" 2>&1 || true
}

# ── ① 等门（按 PID —— 不用日志内容当状态）──────────────────────────
if [ -n "$PID" ]; then
    printf '等门 PID=%s …\n' "$PID"
    while kill -0 "$PID" 2>/dev/null; do sleep 60; done
fi
printf '门已结束：%s\n' "$(date '+%F %T')"

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
                wake "m$N-tag" "M$N 门已绿但 make_tag 推送失败" "见 $LOG / stdout"
            }
        fi
    fi
    notify p0 "M$N 全量门绿 · 已推 main + tag v0.2.$N" "裁决：$VERD（多路一致）"
    ;;
RED)
    RL="$(printf '%s\n' "$OUT" | awk '/^  红门：/{sub(/^  红门：/,""); print}')"
    [ -n "$RL" ] || RL="（清单为空 —— 这不是门红，请看 INCONSISTENT 分支）"
    notify p1 "m$N-red" "M$N 全量门红（未推送）" "红门：$RL"
    wake "m$N-red" "M$N 全量门红（未推送）" "红门：$RL｜门日志：$LOG"
    ;;
INCONSISTENT)
    notify p1 "m$N-verdict" "M$N 门裁决不一致（未推送）" \
"判据/日志互相矛盾，**不是门红** —— 按缺陷 495 的处置：先查判据串，别查门。
详见 stdout / $LOG。"
    wake "m$N-verdict" "M$N 门裁决不一致（未推送）" \
"判据/日志互相矛盾，**不是门红** —— 按缺陷 495：先查判据串，别查门。门日志：$LOG"
    ;;
*)
    notify p1 "m$N-premise" "M$N 裁决前提不成立" "裁决器与门实现脱节（rc=$rc）⇒ 人工介入。"
    wake "m$N-premise" "M$N 裁决前提不成立（门未推送）" "裁决器与门实现脱节 rc=$rc｜门日志：$LOG"
    ;;
esac
exit "$rc"
