#!/usr/bin/env bash
# ============================================================
# packaging/latest_verify.sh ——「发布到底发出去没有」的自证装置（M354 · 缺陷 620 第二半）
#
# 为什么有它（2026-10-29 实测事故，M352）：
#   tag 与 main 都推了，但 GitHub 的 Release job 因 `actions/checkout`（fetch-depth: 0）
#   在 runner 上跑满 30min 被 cancelled ⇒ 后续 steps 全 skipped ⇒ `releases/latest` 一直
#   停在**上一个版本**。而当时的自证装置把 `PENDING` 当**正常终态**写盘，还照写 `DONE`
#   ⇒ 下游只看见 DONE ⇒ 「发布没发出去」静默 3 小时（park 探针照样满足）。
#
# 三态（口径 11/14：取不到 ≠ 没有，未证 ≠ 通过）：
#   OK      releases/latest == 期望 tag ⇒ 发布自证通过
#   STALE   期望版本**已有 Release**，latest 却指向别处 ⇒ 真缺陷（install.sh 会装旧树）
#   BLOCKED 有时间上限：轮询到 deadline 仍是 PENDING（= 有 tag 没有 Release）/ 读不到 API
#           ⇒ **不是 DONE、不是 GREEN**，必须响亮并写清下一步
#
# 用法：
#   latest_verify.sh <期望tag> [输出md]
# 环境：
#   LV_DEADLINE_SEC  总时长上限秒（默认 1800 = 30min）
#   LV_POLL_SEC      轮询间隔秒（默认 60；负控用 0）
#   LV_FIXTURE       透传给 latest_guard.sh --api-fixture（离线判据，负控用）
#   GH_TOKEN         走环境变量；缺省从 /data/pat.md 读（权限 600，不打印、不落日志）
# 退出码：0=OK · 1=STALE · 2=用法错 · 3=BLOCKED · 4=ENV-FAIL（取不到数/令牌失效）
#
# ★缺陷 709（M389）：guard 现在把「取不到数」报成 exit 2（而不是静默 PENDING）。
#   本装置必须把它区分出来：连续 LV_ENV_FAIL_MAX（默认 3）次 rc=2 ⇒ ENV-FAIL。
#   为什么不用「一次就失败」：瞬时网络毛刺不该误判；又为什么不能等到 deadline：
#   认证错重试 30min 毫无意义，且最终会伪装成 BLOCKED（指向 Release job，指错方向）。
# ============================================================
set -uo pipefail

TAG="${1:?用法: $0 <期望tag> [输出md]}"
OUT="${2:-}"
DEADLINE="${LV_DEADLINE_SEC:-1800}"
POLL="${LV_POLL_SEC:-60}"
FIX="${LV_FIXTURE:-}"

if [ -z "${GH_TOKEN:-}" ] && [ -z "$FIX" ] && [ -r /data/pat.md ]; then
    GH_TOKEN="$(tr -d '\n\r' < /data/pat.md)"; export GH_TOKEN
fi

LOG=""
if [ -n "$OUT" ]; then mkdir -p "$(dirname "$OUT")"; : > "$OUT"; LOG="$OUT"; fi
say(){ echo "$*"; if [ -n "$LOG" ]; then echo "$*" >> "$LOG"; fi; return 0; }

say "# 发布自证（releases/latest）· start $(date '+%F %T') · 期望 $TAG · deadline=${DEADLINE}s · 间隔=${POLL}s"

GUARD="$(dirname "$0")/latest_guard.sh"
t_end=$(( $(date +%s) + DEADLINE ))
verdict=""; reason=""; last=""; i=0; envfail=0; ENV_MAX="${LV_ENV_FAIL_MAX:-3}"
while :; do
    i=$((i + 1))
    if [ -n "$FIX" ]; then
        res="$(bash "$GUARD" --api-fixture "$FIX" 2>&1)"; grc=$?
    else
        res="$(bash "$GUARD" 2>&1)"; grc=$?
    fi
    last="$res"
    say "poll $i $(date '+%T') :: $(printf '%s' "$res" | head -2 | tr '\n' ' ')"
    if [ "$grc" = 2 ]; then
        envfail=$((envfail + 1))
        if [ "$envfail" -ge "$ENV_MAX" ]; then
            verdict=ENV-FAIL
            reason="guard-rc2×${envfail}(取不到数: $(printf '%s' "$res" | grep -m1 'auth=' || printf '%s' "$res" | head -1))"
            break
        fi
    else
        envfail=0
    fi
    case "$res" in
        *LATEST-STALE*) verdict=STALE; reason="latest-not-expected($TAG)"; break;;
        *PENDING*)      : ;;                 # 中间态 ⇒ 继续等（**不是**结论）
        *) if grep -qF "$TAG" <<< "$res"; then verdict=OK; break; fi ;;   # 读不出 ⇒ 当 api-error 继续等
    esac
    if [ "$(date +%s)" -ge "$t_end" ]; then break; fi
    if [ "$POLL" -gt 0 ]; then sleep "$POLL"; fi
done

if [ -z "$verdict" ]; then
    if grep -q 'PENDING' <<< "$last"; then
        verdict=BLOCKED; reason="tag-exists-but-no-release(轮询 ${i} 次仍 PENDING)"
    else
        verdict=BLOCKED; reason="api-unreadable(最后一次：$(printf '%s' "$last" | head -1))"
    fi
fi

say ""
say "LATEST-VERDICT=$verdict expected=$TAG reason=${reason:-none}"
if [ "$verdict" = "BLOCKED" ]; then
    say "⇒ **这不是「发布成功」**：BLOCKED ≠ DONE ≠ GREEN。"
    say "⇒ 下一步（按序）：① 查 Release job 是否 cancelled/timeout（已知形态：actions/checkout"
    say "   fetch-depth:0 在 runner 上跑满 30min 被 cancelled ⇒ 后续 steps 全 skipped）"
    say "   ② rerun-failed-jobs 重跑同一 commit 的 Release job ③ 仍不更新 ⇒ release_reconcile.sh 对账/补发。"
fi
say "DONE $(date '+%F %T')"

case "$verdict" in
    OK)      exit 0;;
    STALE)   exit 1;;
    ENV-FAIL)
        say "⇒ **这不是「发布没发出去」，是「读不到数」**：先查令牌/网络，别去查 Release job。"
        say "   自查：curl -sS -o /dev/null -w '%{http_code}' -H \"Authorization: Bearer \$(cat /data/pat.md)\" https://api.github.com/user  # 401=令牌失效"
        exit 4;;
    *)       exit 3;;
esac
