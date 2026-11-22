#!/usr/bin/env bash
# ============================================================
# lib_pipeline_fail.sh —— 机械段判红必须**叫醒**，不许只写标记就静默退出（M424 · 缺陷 863）
# ------------------------------------------------------------
# 病史（2026-11-20 · M423 实测）：
#   02:16 起 preflight → 02:47 判红，pipeline 写了 M423-PIPELINE-EXIT 后 exit 3，
#   然后**九小时**（02:47 → 11:52 用户来问）没有任何人/机制被叫醒：
#     · park 等的是 M423-CHAIN-EXIT（链根本没起 ⇒ 永不满足）
#     · dy-ops-watch 的五条判据都不覆盖「机械段自己判红并退出」
#     · idle-unpushed 又因「有 park 载体」被豁免 ⇒ 全静默
#   本文件把「判红」接到**原生通道**（dy-notify.sh p1 → ws-core 任务自驱），
#   不再依赖「有人恰好来看一眼」。
#
# 用法（在 pipeline 脚本里 source）：
#     . /data/autodrive/lib_pipeline_fail.sh
#     pipeline_fail 424 "preflight" 3 "$LOG"
#   ⇒ 写 M424-PIPELINE-EXIT + dy-notify p1（含阶段/rc/红项/日志路径）+ exit <rc>
#
# 环境：
#   PIPELINE_NOTIFY_DRYRUN=1    只打印不投递（自测用）
#   PIPELINE_NOTIFY_MUTATE      档位，默认 publish（修缺陷要能 push/tag）
#   AD                          台账目录，默认 /data/autodrive
#
# 退出码：pipeline_fail 永不返回（exit <rc>）；--self-test 0 通过 / 1 失败 / 2 用法错
# ============================================================
set -uo pipefail

pipeline_fail() {
    local ms="$1" stage="$2" rc="$3" log="${4:-}"
    local ad="${AD:-/data/autodrive}"
    local key="m${ms}-pipeline-${stage}"
    local mark="$ad/M${ms}-PIPELINE-EXIT"
    printf 'M%s-PIPELINE-EXIT=%s-FAIL rc=%s at=%s\n' "$ms" "$stage" "$rc" "$(date -Is)" > "$mark"

    local red="" nlines=0 nred=0
    if [ -n "$log" ] && [ -f "$log" ]; then
        # 只取「❌/FAIL」开头的行里的**门名**（判据：红项必须点名，不能只说「失败了」）
        red=$(grep -aoE '^(❌|FAIL)[[:space:]]*[A-Za-z0-9_.-]+' "$log" 2>/dev/null | head -5 | tr '\n' ' ')
        nlines=$(wc -l < "$log" 2>/dev/null || echo 0)
        nred=$(grep -acE '^(❌|FAIL)[[:space:]]*[A-Za-z0-9_.-]+' "$log" 2>/dev/null | head -1)
    fi
    nlines="${nlines//[!0-9]/}"; nred="${nred//[!0-9]/}"
    nlines="${nlines:-0}"; nred="${nred:-0}"
    # ── M410 R1：判红也必须打印**扫描面**（扫了多少行 / 命中几项）──
    #   病史：2026-11-20 M424 preflight 判红，红项之一就是本文件 ——
    #   「有 grep 判据 + 有 PASS 绿输出（自测里）+ 从不打印扫描面」⇒ 判据自欺。
    #   扫描面为 0 时如实说 0（空日志的红项不可信），绝不用「看起来没问题」代替测量。
    local surf="扫描面：日志${nlines}行 · 红项${nred}条"
    [ "$nlines" -eq 0 ] && surf="$surf（日志为空/不存在 ⇒ 红项不可信，须人工看原文）"
    # ── M433（缺陷 896 的顺手项）：红项 0 条时**必须说出真因候选** ──────────
    #   病史 2026-11-22 16:40：M432 链 306/306 全绿，publish 因树脏 ABORT，
    #   而 p1 正文写的是「扫描面 日志47行 · 红项 0 条」—— 真因（ABORT dirty-tree）
    #   就在日志里，判据却只认 ❌/FAIL 行 ⇒ 收到 p1 的人先花时间证伪。
    local why=""
    if [ -n "$log" ] && [ -f "$log" ]; then
        why=$(grep -aoE '(ABORT [^[:cntrl:]]{0,90}|SCOPE-REJECT reason=[A-Za-z-]+|VERDICT-NOT-GREEN|STOP：[^[:cntrl:]]{0,70}|TIMEOUT [^[:cntrl:]]{0,60})' "$log" 2>/dev/null | tail -3 | tr '\n' ' ')
    fi
    local body
    body="阶段=${stage} · rc=${rc}
日志：${log:-（未给）}
${surf}
红项：${red:-（日志里没留下 ❌/FAIL 行 —— 这本身要查：判据可能没说清为什么红）}
真因候选：${why:-（日志里没有 ABORT / SCOPE-REJECT / VERDICT-NOT-GREEN / STOP 行）}
机械段已停：链没有起，本轮的绿戳/tag/镜像都不会有。
复跑：bash $ad/m${ms}_pipeline.sh（幂等）
来历：缺陷 863 —— M423 02:47 判红后静默 9 小时，因为 park 等的是链退出（链没起）。"

    if [ "${PIPELINE_NOTIFY_DRYRUN:-0}" = "1" ]; then
        printf 'DRYRUN key=%s\n标题=%s\n标记=%s\n正文=%s\n' \
            "$key" "M${ms} 机械段判红：${stage}" "$(cat "$mark")" "$body"
        return 0
    fi
    DY_NOTIFY_MUTATE="${PIPELINE_NOTIFY_MUTATE:-publish}" \
        /usr/local/sbin/dy-notify.sh p1 "$key" "M${ms} 机械段判红：${stage}" "$body"
    exit "$rc"
}

# ── 自测（离线，不投递）──────────────────────────────────────
if [ "${1:-}" = "--self-test" ]; then
    T="$(mktemp -d)"
    P=0; F=0
    ok() { echo "  PASS $1"; P=$((P+1)); }
    bad() { echo "  FAIL $1"; F=$((F+1)); }

    # 夹具日志：一条 preflight 红（点名门）+ 一条普通行
    printf '✅ ok_gate（1s）\n❌ gate_lock_guard 缺互斥锁\nFAIL m400_baseline_feedback 前提不成立\n' > "$T/red.log"
    out=$(AD="$T" PIPELINE_NOTIFY_DRYRUN=1 pipeline_fail 424 preflight 3 "$T/red.log")
    grep -q 'key=m424-pipeline-preflight' <<< "$out" && ok "F1 台账 key 由里程碑+阶段构成" || bad "F1 key 不对"
    grep -q 'M424-PIPELINE-EXIT=preflight-FAIL rc=3' <<< "$out" && ok "F2 退出标记写明阶段与 rc" || bad "F2 标记内容不对"
    grep -q 'gate_lock_guard' <<< "$out" && ok "F3 红项点名（gate_lock_guard）" || bad "F3 没解析出红项"
    grep -q 'm400_baseline_feedback' <<< "$out" && ok "F4 第二条 FAIL 行也点名" || bad "F4 漏了 FAIL 行"
    grep -q '缺陷 863' <<< "$out" && ok "F5 正文自带来历（可追溯）" || bad "F5 正文没写来历"
    [ -f "$T/M424-PIPELINE-EXIT" ] && ok "F6 标记文件真落盘" || bad "F6 标记文件没写"

    # 负控：日志里没有 ❌/FAIL ⇒ 正文必须**如实说没留下**（不许假装有红项）
    printf '一切都好\n' > "$T/clean.log"
    out2=$(AD="$T" PIPELINE_NOTIFY_DRYRUN=1 pipeline_fail 424 launch 4 "$T/clean.log")
    grep -q '没留下 ❌/FAIL 行' <<< "$out2" && ok "F7 负控：无红项时如实说明（不编造）" || bad "F7 无红项时编造了红项"
    grep -q 'launch' <<< "$out2" && ok "F8 阶段名进标题" || bad "F8 标题没带阶段"
    # 负控：标记文件内容随阶段变化（不是写死）
    grep -q 'launch-FAIL rc=4' "$T/M424-PIPELINE-EXIT" && ok "F9 负控：标记随阶段/rc 变化" || bad "F9 标记写死"

    # M410 R1：判红自身必须打印扫描面（日志行数 / 红项条数），且 0 时如实报 0
    grep -qE '扫描面：日志[0-9]+行 · 红项[0-9]+条' <<< "$out" && ok "F10 判红也打印扫描面（行数/红项数）" || bad "F10 没打印扫描面"
    grep -qE '扫描面：日志[0-9]+行 · 红项0条' <<< "$out2" && ok "F11 负控：红项 0 时如实报 0（不编造）" || bad "F11 红项数没如实报"
    : > "$T/empty.log"
    out3=$(AD="$T" PIPELINE_NOTIFY_DRYRUN=1 pipeline_fail 424 preflight 5 "$T/empty.log")
    grep -q '日志为空/不存在' <<< "$out3" && ok "F12 负控：空日志 ⇒ 扫描面 0 且标注不可信" || bad "F12 空日志没标注不可信"

    # M433（缺陷 896 顺手项）：红项 0 条时必须说出**真因候选**
    printf '== 交 publish_on_green\nABORT dirty-tree\n' > "$T/abort.log"
    out4=$(AD="$T" PIPELINE_NOTIFY_DRYRUN=1 pipeline_fail 432 publish 2 "$T/abort.log")
    grep -q '真因候选：.*ABORT dirty-tree' <<< "$out4" && ok "F13 红项 0 但日志有 ABORT ⇒ 真因进正文" || bad "F13 真因没进正文：$out4"
    printf 'SCOPE-REJECT reason=wip-version-source wip=1\n' > "$T/scope.log"
    out6=$(AD="$T" PIPELINE_NOTIFY_DRYRUN=1 pipeline_fail 433 publish 2 "$T/scope.log")
    grep -qE 'SCOPE-REJECT reason=wip-version-source' <<< "$out6" \
        && ok "F13b SCOPE-REJECT reason 被当真因候选" || bad "F13b SCOPE-REJECT 没被识别"
    out5=$(AD="$T" PIPELINE_NOTIFY_DRYRUN=1 pipeline_fail 434 launch 4 "$T/clean.log")
    grep -q '真因候选：（日志里没有' <<< "$out5" && ok "F14 负控：无真因线索时如实说没有（不编造）" || bad "F14 无线索时编造了真因"

    rm -rf "$T"
    echo "──────────────────────────────"
    echo "LIB-PIPELINE-FAIL-SELFTEST pass=$P fail=$F"
    [ "$F" -eq 0 ] && { echo "SELFTEST=OK"; exit 0; }
    echo "SELFTEST=FAIL"; exit 1
fi

if [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
    sed -n '2,30p' "$0"; exit 0
fi
