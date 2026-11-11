#!/usr/bin/env bash
# ============================================================================
# release_red_guard.sh —— 全量门「判红必有人叫」伴生单元（M393）
#
# 为什么要有它（两条实测病史）：
#   ① 2026-11-11 13:49 起 M390 r4 时只起了主链这一个 systemd 单元，
#      red-guard / postupgrade 两个伴生单元没起 ⇒ 判红的时候没人叫我，
#      我只会等用户下次来问（那次空转 4h55m 的一部分）。
#   ② 2026-11-12 01:31 M392 判绿但用户通知投递失败（设备离线）⇒ 用户不知道。
#      这条归 dy-notify 的离线持仓（同轮修），本脚本只管「红」这一侧。
#
# 判据（三条，任一成立即 p1 叫醒，**只报一次**）：
#   A  STAMP 出现且不含 CHAIN-VERDICT=GREEN            ⇒ 判红/中止
#   B  STAMP 不存在，且主链单元已不是 active 超过 2 轮  ⇒ 链消失无裁决
#   C  超过 --max-hours                                  ⇒ 超时
#   GREEN ⇒ 静默退出 0（判绿的通知由 publish 负责，本脚本不重复推用户）
#
# 用法：release_red_guard.sh --ms 393 [--stamp FILE] [--unit m393-chain]
#                            [--interval 60] [--max-hours 6] [--self-test]
# 退出码：0 判绿 / 1 已报警（红·链消失·超时）/ 2 参数错
# ============================================================================
set -uo pipefail

MS=""; STAMP=""; UNIT=""; INTERVAL="${RED_GUARD_INTERVAL:-60}"; MAXH=6; SELF=0
NOTIFY_SH="${DY_NOTIFY_BIN:-/usr/local/sbin/dy-notify.sh}"
LOG="${RED_GUARD_LOG:-/data/autodrive/red-guard-$(date +%Y%m%d).log}"

while [ $# -gt 0 ]; do
    case "$1" in
        --ms)        MS="${2:-}"; shift 2 ;;
        --stamp)     STAMP="${2:-}"; shift 2 ;;
        --unit)      UNIT="${2:-}"; shift 2 ;;
        --interval)  INTERVAL="${2:-60}"; shift 2 ;;
        --max-hours) MAXH="${2:-6}"; shift 2 ;;
        --self-test) SELF=1; shift ;;
        -h|--help)   sed -n '2,26p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done

say() { printf '[%s] %s\n' "$(date +%F' '%T)" "$*" >> "$LOG" 2>/dev/null || true; }

alert() {  # alert <台账键> <标题> <正文>
    say "ALERT $1 $2"
    if [ -x "$NOTIFY_SH" ] && [ "${DY_NOTIFY_MUTATE:-local}" != "off" ]; then
        DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 "$1" "$2" "$3" >/dev/null 2>&1 \
            || say "ALERT-FAIL notify rc=$?"
    fi
}

unit_active() {  # 主链单元是否 active（可注入 RED_GUARD_UNIT_PROBE 便于自测）
    if [ -n "${RED_GUARD_UNIT_PROBE:-}" ]; then
        "$RED_GUARD_UNIT_PROBE" "$1"; return $?
    fi
    [ "$(systemctl is-active "$1" 2>/dev/null)" = "active" ]
}

red_lines() {  # 从门日志里捞红门明细（最多 10 行）；日志路径由 STAMP 同轮 chain.log 提供
    local logf="${RED_GUARD_GATELOG:-}"
    [ -n "$logf" ] && [ -r "$logf" ] || return 0
    grep -E '^FAIL|❌|判红|NEGCTL' "$logf" 2>/dev/null | head -10
}

run_guard() {
    [ -n "$MS" ] || { echo "--ms 必填" >&2; return 2; }
    STAMP="${STAMP:-/data/autodrive/M${MS}-GATE-DONE}"
    UNIT="${UNIT:-m${MS}-chain}"
    local deadline=$(( $(date +%s) + MAXH * 3600 )) idle=0
    say "=== red-guard start ms=$MS stamp=$STAMP unit=$UNIT max=${MAXH}h pid=$$ ==="
    while :; do
        if [ -f "$STAMP" ]; then
            if grep -q 'CHAIN-VERDICT=GREEN' "$STAMP" 2>/dev/null; then
                say "GREEN —— 静默退出（用户通知由 publish 负责，这里不重复推）"
                return 0
            fi
            local v; v="$(tr '\n' ' ' < "$STAMP" 2>/dev/null | cut -c1-400)"
            alert "m${MS}-chain-red" "M${MS} 全量门判红/中止" \
                "STAMP=$STAMP
$v
$(red_lines)
处置：看门日志，修完重跑；本条只叫醒东月，不推用户。"
            return 1
        fi
        if ! unit_active "$UNIT"; then
            idle=$((idle + 1))
            say "unit=$UNIT 非 active（第 $idle 轮）"
            if [ "$idle" -ge 2 ]; then
                alert "m${MS}-chain-vanished" "M${MS} 主链单元消失且无裁决" \
                    "unit=$UNIT 不是 active，且 $STAMP 不存在
后果：既没绿也没红，没人会再动手。处置：查 journalctl -u $UNIT，重起链。"
                return 1
            fi
        else
            idle=0
        fi
        [ "$(date +%s)" -ge "$deadline" ] && {   # -ge：--max-hours 0 必须在首轮就到点（自测 N3 钉这条；用 -gt 会死循环）
            alert "m${MS}-chain-timeout" "M${MS} 全量门超时（${MAXH}h）未裁决" \
                "STAMP=$STAMP 仍不存在，unit 状态=$(systemctl is-active "$UNIT" 2>/dev/null || echo unknown)"
            return 1
        }
        sleep "$INTERVAL"
    done
}

# ── 自测：4 例（1 正控 + 3 负控）+ 1 前提自证，全离线，不真 sleep ──────────
if [ "$SELF" = 1 ]; then
    T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
    # 假 dy-notify：把调用参数落盘，供断言「真的叫了人」
    cat > "$T/notify" <<'STUB'
#!/usr/bin/env bash
printf '%s\n' "$*" >> "$STUB_ALERTS"
exit 0
STUB
    chmod +x "$T/notify"
    export STUB_ALERTS="$T/alerts.txt"; : > "$STUB_ALERTS"
    export DY_NOTIFY_BIN="$T/notify" RED_GUARD_LOG="$T/guard.log" RED_GUARD_INTERVAL=0
    n=0; FAIL=0
    ok()  { n=$((n+1)); echo "  ✅ $*"; }
    bad() { n=$((n+1)); FAIL=$((FAIL+1)); echo "  ❌ $*"; }
    # 假 unit 探针：由 RED_GUARD_UNIT_STATE 决定返回
    printf '#!/usr/bin/env bash\nexit %s\n' "${P:-1}" > "$T/probe-down"
    printf '#!/usr/bin/env bash\nexit 0\n' > "$T/probe-up"; chmod +x "$T/probe-down" "$T/probe-up"
    : "${P:=1}"

    # P1 正控：STAMP 含 GREEN ⇒ rc=0 且**不许**报警
    printf 'CHAIN-VERDICT=GREEN rc=0 green=269\n' > "$T/stamp-green"
    : > "$STUB_ALERTS"
    RED_GUARD_UNIT_PROBE="$T/probe-up" MS=999 STAMP="$T/stamp-green" UNIT=u \
        bash "$0" --ms 999 --stamp "$T/stamp-green" --unit u >/dev/null 2>&1
    rc=$?
    [ "$rc" = 0 ] && ok "P1 正控：GREEN ⇒ rc=0" || bad "P1 rc=$rc（应 0）"
    [ ! -s "$STUB_ALERTS" ] && ok "P1 GREEN 不许叫人（不重复推）" || bad "P1 GREEN 却叫了人"

    # N1 负控：STAMP 含 NO-GO ⇒ rc=1 且叫了人
    printf 'CHAIN-VERDICT=NO-GO reason=shard-rc-1\n' > "$T/stamp-red"
    : > "$STUB_ALERTS"
    MS=999 STAMP="$T/stamp-red" UNIT=u RED_GUARD_UNIT_PROBE="$T/probe-up" \
        bash "$0" --ms 999 --stamp "$T/stamp-red" --unit u >/dev/null 2>&1
    rc=$?
    [ "$rc" = 1 ] && ok "N1 负控：NO-GO ⇒ rc=1" || bad "N1 rc=$rc（应 1）"
    grep -q 'chain-red' "$STUB_ALERTS" 2>/dev/null && ok "N1 报出台账键 m999-chain-red" || bad "N1 没报警"

    # N2 负控：单元消失 + 无 STAMP ⇒ 2 轮后 rc=1，键 chain-vanished
    : > "$STUB_ALERTS"
    MS=998 STAMP="$T/none-$$" UNIT=u RED_GUARD_UNIT_PROBE="$T/probe-down" \
        bash "$0" --ms 998 --stamp "$T/none-$$" --unit u --max-hours 1 >/dev/null 2>&1
    rc=$?
    [ "$rc" = 1 ] && ok "N2 负控：链消失且无 STAMP ⇒ rc=1" || bad "N2 rc=$rc（应 1）"
    grep -q 'chain-vanished' "$STUB_ALERTS" 2>/dev/null && ok "N2 报出 chain-vanished" || bad "N2 键不对"
    [ ! -f "$T/none-$$" ] && ok "N2 前提自证：STAMP 确实不存在（不是判据编的）" || bad "N2 前提缺失"

    # N3 负控：超时上限 ⇒ rc=1，键 chain-timeout（interval=0、max-hours 设 0 即立刻到点）
    : > "$STUB_ALERTS"
    MS=997 STAMP="$T/none2-$$" UNIT=u RED_GUARD_UNIT_PROBE="$T/probe-up" \
        bash "$0" --ms 997 --stamp "$T/none2-$$" --unit u --max-hours 0 >/dev/null 2>&1
    rc=$?
    [ "$rc" = 1 ] && ok "N3 负控：超时 ⇒ rc=1" || bad "N3 rc=$rc（应 1）"
    grep -q 'chain-timeout' "$STUB_ALERTS" 2>/dev/null && ok "N3 报出 chain-timeout" || bad "N3 键不对"

    # N4 负控：参数错 ⇒ rc=2（不许静默当绿）
    bash "$0" --stamp x --unit u >/dev/null 2>&1
    [ "$?" = 2 ] && ok "N4 负控：缺 --ms ⇒ rc=2" || bad "N4 缺参数没报错"

    # 前提自证：假 notify 真的被执行过（否则「叫了人」是空判据）
    [ -s "$STUB_ALERTS" ] && ok "前提自证：假 dy-notify 真被调用（$(( $(wc -l < "$STUB_ALERTS") )) 次）" \
                         || bad "前提缺失：notify 从未被调用"

    echo "── red-guard 自测：$((n-FAIL))/$n 通过 ──"
    [ "$FAIL" = 0 ] || exit 1
    exit 0
fi

run_guard
