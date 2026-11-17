#!/usr/bin/env bash
# ============================================================
# M414 门 · WAITING 口径（缺陷 824：BLIND 判据把「等工序」当成「叫不醒」）
# ------------------------------------------------------------
# 现场（2026-11-17 21:37:15，用户被假警报叫醒那条 p0）：
#   台账 M413-release 末行 ESCALATED、age=110min、用户不在场 ⇒ M413 判据判 BLIND 并推 p0。
#   同一秒另一条检查项写着「m413-chain 已运行 1h27m（正常推进）」—— 两条互不看对方。
#   事实：该任务的 park 探测条件是 test -f M413-RELEASE.done，当时为假 ⇒ **根本不到该醒的时候**。
#   结构性：发布链典型墙钟 ≈110min > 90min 阈值 ⇒ 每次发版必然误报一次 p0。
#
# 本门守四件（改了哪条调用点，门就覆盖哪条）：
#   P1 判据有牙：notify_ack.sh 沙箱自测，含事故复现正控 + ≥5 条负控 + 前提自证
#   P2 豁免不吃真盲区：probe=true 且无在途单元 ⇒ 必须仍然 BLIND（负控 B1）
#   P3 动态下界不是无限豁免：单元 100min ⇒ 阈值 130min（age=110 等、age=140 醒）
#   P4 p0 防抖：连续 1/CONFIRM 次命中时**不许**推 p0（宿主库功能测试）
#
# 扫描面显式打印 nscan（M410 R1/R2：判绿必须打扫描面，nscan=0 ⇒ 本门自己判红）
# 用法：verify.sh [--selftest] [--root <仓库根>]
# ============================================================
set -uo pipefail
. "$(dirname "$0")/../../selfhost/gate_lock.sh" 2>/dev/null || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }
SELF_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$SELF_DIR/../.." && pwd)
SELFTEST=0
while [ $# -gt 0 ]; do
    case "$1" in
        --selftest) SELFTEST=1 ;;
        --root) shift; ROOT="${1:-$ROOT}" ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
    shift
done

NA="$ROOT/packaging/chain/notify_ack.sh"
[ -f "$NA" ] || { echo "❌ 缺判据实现：$NA" >&2; exit 2; }

n=0; FAIL=0; nscan=0
ok()  { n=$((n+1)); echo "  ✅ $*"; }
bad() { n=$((n+1)); FAIL=$((FAIL+1)); echo "  ❌ $*"; }

mk_pend() { # mk_pend <file> <first_epoch> <marker>
    local f="$1" e="$2" m="$3"
    { printf '%s\t%s\n' "$e" "夹具：某发布链判红（自测用）"
      printf 'CHAIN-VERDICT=RED green=0 red=1 head=deadbee\n'
      [ -n "$m" ] && printf '%s\t%s\tdeadbeef\n' "$m" "$((e + 3600))"
    } > "$f"
}

mk_db() { # mk_db <path> <updated_at>
    local db="$1" ts="$2"
    rm -f "$db"
    sqlite3 "$db" "create table token_cache(key text primary key, value text, module text, expire_at int, created_at int default 0, updated_at int default 0);" 2>/dev/null || return 1
    sqlite3 "$db" "insert into token_cache(key,value,module,expire_at,created_at,updated_at) values('k','v','token_cache',0,$ts,$ts);" 2>/dev/null || return 1
}

# 取汇总行里某个字段的值（不带正则地切，避免 csv 串位）
field() { printf '%s\n' "$1" | tr ' ' '\n' | grep -m1 "^$2=" | cut -d= -f2-; }

selftest() {
    local T; T="$(mktemp -d)"
    trap 'rm -rf "$T"' RETURN
    local pen="$T/pending" db="$T/tc.db" out rc
    local NOW=1800000000
    mkdir -p "$pen" || { echo "❌ fixture 建不出来" >&2; return 2; }
    local park="$T/park.json" units0="$T/units.empty" units100="$T/units100.txt"
    : > "$units0"; printf 'm999-chain\t100\n' > "$units100"

    run() { # run <park> [units]
        NA_PEND_DIR="$pen" NA_DB="$db" NA_NOW="$NOW" NA_PARKWHEN="$1" \
        NA_UNITS_FILE="${2:-$units0}" NA_BLIND_MIN=90 NA_WAIT_FLOOR=30 NA_IDLE_MIN=30 \
        bash "$NA" 2>&1
    }

    # ── A1 事故复现正控：probe=false ⇒ 必须 WAITING，不许 BLIND ──
    #    夹具就是现场形状：park 条件是「等 RELEASE.done」，而它还没出现。
    printf '{"dy-ops-M999-release":{"cmds":["test -f %s/NEVER-DONE"]}}\n' "$T" > "$park"
    mk_pend "$pen/M999-release" "$((NOW-6600))" ESCALATED   # age=110min
    mk_db "$db" "$((NOW-10800))" || bad "A1 夹具 db 建失败（后续结论不可信）"
    out="$(run "$park")"; rc=$?
    nscan=$((nscan+1))
    if [ "$rc" = 0 ] && [ "$(field "$out" blind)" = 0 ] && [ "$(field "$out" wait_probe)" = 1 ]; then
        ok "A1 事故复现：probe=false + ESCALATED + age=110min ⇒ wait_probe=1 blind=0 rc=0（不推 p0）"
    else
        bad "A1 rc=$rc blind=$(field "$out" blind) wait_probe=$(field "$out" wait_probe) out=$out"
    fi

    # ── A2 前提自证：那个 probe 命令真的返回非 0（不是脚本自说自话） ──
    nscan=$((nscan+1))
    if bash -c "test -f $T/NEVER-DONE"; then
        bad "A2 前提不成立：probe 命令竟然为真 ⇒ A1 测的不是「等工序」"
    else
        ok "A2 前提自证：probe 命令 test -f $T/NEVER-DONE 实测返回 $?（非 0 = 该醒前提不成立）"
    fi

    # ── B1 负控（豁免不许吃掉真盲区）：probe=true + 无在途单元 + age=110 ⇒ 必须 BLIND ──
    printf '{"dy-ops-M999-release":{"cmds":["test -d %s"]}}\n' "$pen" > "$park"
    nscan=$((nscan+1))
    if bash -c "test -d $pen"; then
        ok "B1a 前提自证：probe 命令 test -d $pen 实测为真（该醒的前提成立）"
    else
        bad "B1a 前提不成立：probe 为假 ⇒ B1 结论不可信"
    fi
    out="$(run "$park")"; rc=$?
    nscan=$((nscan+1))
    if [ "$rc" = 1 ] && [ "$(field "$out" blind)" = 1 ] && grep -q 'BLIND-KEYS' <<<"$out"; then
        ok "B1 负控：probe=true + 无在途单元 ⇒ 仍判 BLIND（waiting 口径不吃真盲区）"
    else
        bad "B1 rc=$rc blind=$(field "$out" blind) out=$out"
    fi

    # ── C 无 park 记录 + 无在途单元 ⇒ wait-unknown（不猜，不报 p0） ──
    out="$(run "$T/no-such-park.json")"; rc=$?
    nscan=$((nscan+1))
    if [ "$rc" = 0 ] && [ "$(field "$out" wait_unknown)" = 1 ] && [ "$(field "$out" blind)" = 0 ]; then
        ok "C 负控：无 park 记录 ⇒ wait_unknown=1 rc=0（宁可不判，不猜）"
    else
        bad "C rc=$rc wait_unknown=$(field "$out" wait_unknown) out=$out"
    fi

    # ── D 无 park 记录 + 在途发布单元 ⇒ WAITING-UNIT，并打印单元名/时长（前提自证） ──
    out="$(run "$T/no-such-park.json" "$units100")"; rc=$?
    nscan=$((nscan+1))
    if [ "$rc" = 0 ] && [ "$(field "$out" wait_unit)" = 1 ] && grep -q 'unit=m999-chain run=100min' <<<"$out"; then
        ok "D 在途单元兜底：wait_unit=1（单元 m999-chain 已跑 100min，等工序 ≠ 叫不醒）"
    else
        bad "D rc=$rc wait_unit=$(field "$out" wait_unit) out=$out"
    fi

    # ── E1 动态下界生效：单元 100min ⇒ 阈值 90→130，age=110 落在等窗口里 ──
    out="$(run "$park" "$units100")"; rc=$?
    nscan=$((nscan+1))
    if [ "$rc" = 0 ] && [ "$(field "$out" blind_min)" = 130 ] && [ "$(field "$out" wait_unit)" = 1 ]; then
        ok "E1 动态下界：blind_min=130（base 90 + unit 100 ⇒ 30+100）⇒ age=110 判 WAITING"
    else
        bad "E1 rc=$rc blind_min=$(field "$out" blind_min) wait_unit=$(field "$out" wait_unit) out=$out"
    fi

    # ── E2 下界不是无限豁免：同一单元下 age=140 > 130 ⇒ 必须 BLIND ──
    mk_pend "$pen/M999-release" "$((NOW-8400))" ESCALATED   # age=140min
    out="$(run "$park" "$units100")"; rc=$?
    nscan=$((nscan+1))
    if [ "$rc" = 1 ] && [ "$(field "$out" blind)" = 1 ]; then
        ok "E2 负控：age=140min > 动态阈值 130min ⇒ 仍判 BLIND（下界有界）"
    else
        bad "E2 rc=$rc blind=$(field "$out" blind) out=$out"
    fi
    mk_pend "$pen/M999-release" "$((NOW-6600))" ESCALATED   # 复原 age=110

    # ── F 前提失守：台账目录不存在 ⇒ rc=3（不许假绿） ──
    out="$(NA_PEND_DIR="$T/nodir" NA_DB="$db" NA_NOW="$NOW" bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 3 ] && grep -q 'PREMISE-FAIL' <<<"$out" && ok "F 前提失守 ⇒ PREMISE-FAIL rc=3" || bad "F rc=$rc（应 3）out=$out"

    # ── G 扫描面零值守卫：空台账 ⇒ SCAN-GUARD + rc=0（真空 ≠ 扫描盲区） ──
    local pen2="$T/pending2"; mkdir -p "$pen2"
    out="$(NA_PEND_DIR="$pen2" NA_DB="$db" NA_NOW="$NOW" NA_PARKWHEN="$park" NA_UNITS_FILE="$units0" bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 0 ] && grep -q 'SCAN-GUARD nscan=0' <<<"$out" && ok "G 空台账 ⇒ SCAN-GUARD + nscan=0 rc=0" || bad "G rc=$rc out=$out"

    # ── H 计数不串：三条混合（等工序 / 真盲 / 未升级）⇒ nscan=3 blind=1 wait_probe=1 ──
    mk_pend "$pen/M999-wait" "$((NOW-6600))" ESCALATED        # probe 无记录 ⇒ wait-unknown
    mk_pend "$pen/M999-done" "$((NOW-6600))" PENDING          # 未升级 ⇒ ok
    printf '{"dy-ops-M999-release":{"cmds":["test -d %s"]}}\n' "$pen" > "$park"
    out="$(run "$park")"; rc=$?
    nscan=$((nscan+1))
    if [ "$rc" = 1 ] && [ "$(field "$out" nscan)" = 3 ] && [ "$(field "$out" blind)" = 1 ] \
       && [ "$(field "$out" wait_unknown)" = 1 ] && [ "$(field "$out" ok)" = 1 ]; then
        ok "H 计数不串：nscan=3 · blind=1 · wait_unknown=1 · ok=1（四类单列）"
    else
        bad "H rc=$rc nscan=$(field "$out" nscan) blind=$(field "$out" blind) wait_unknown=$(field "$out" wait_unknown) ok=$(field "$out" ok) out=$out"
    fi
    rm -f "$pen/M999-wait" "$pen/M999-done"

    # ── I 用户在场 + probe=true ⇒ wait-present（门禁设计如此，不打扰） ──
    mk_db "$db" "$((NOW-300))"
    out="$(run "$park")"; rc=$?
    nscan=$((nscan+1))
    if [ "$rc" = 0 ] && [ "$(field "$out" wait_present)" = 1 ] && [ "$(field "$out" blind)" = 0 ]; then
        ok "I 负控：用户在场 ⇒ wait_present=1 blind=0"
    else
        bad "I rc=$rc wait_present=$(field "$out" wait_present) out=$out"
    fi
    mk_db "$db" "$((NOW-10800))"

    echo "── 自测：$((n-FAIL))/$n 通过 · 断言 $n（要求 ≥12）· 负控 5（B1/C/E2/I + A1 反向）──"
    [ "$n" -ge 12 ] || { echo "❌ 断言数 $n < 12（自测被削弱）" >&2; return 2; }
    [ "$FAIL" = 0 ] || return 1
    return 0
}

if [ "$SELFTEST" = 1 ]; then
    selftest; rc=$?
    [ "$rc" = 0 ] && echo "M414-NOTIFY-WAITING-SELFTEST=OK" || echo "M414-NOTIFY-WAITING-SELFTEST=FAIL rc=$rc"
    [ "$rc" = 0 ] && exit 0 || exit 1
fi

# ── 正式判定 ──────────────────────────────────────────────────
scan=0
for f in "$NA" "$ROOT/selfhost/gates.registry.sh" "$ROOT/examples/m413_notify_ack/verify.sh" /usr/local/sbin/dy-ops-watch-lib-ack.sh; do
    [ -f "$f" ] && scan=$((scan+1))
done
nscan=$((nscan + scan))
echo "扫描面 nscan=$nscan（判据实现 · 注册源 · M413 前置门 · 宿主 p0 分支）"
[ "$nscan" -gt 0 ] || { echo "❌ 扫描面为 0 ⇒ 本门自己判红（M410 R1/R2）" >&2; exit 1; }

# P1 装置自测
if selftest; then ok "P1 装置自测全绿（事故复现正控 + 5 条负控 + 前提自证）"; else bad "P1 装置自测失败（判据不可信）"; fi

# P4 p0 防抖（宿主库功能测试：连续 1 次不许推 p0，达 CONFIRM 才许）
LIB=/usr/local/sbin/dy-ops-watch-lib-ack.sh
if [ -f "$LIB" ] && [ -f /usr/local/sbin/dy-ops-watch.sh ]; then
    T3="$(mktemp -d)"
    cat > "$T3/stub-blind.sh" <<'STUB'
#!/usr/bin/env bash
printf 'judged\n' >> "$STUB_JUDGE_REC"
echo "BLIND         dy-ops-fixture-x(110min) probe=true title=夹具"
echo "NOTIFY-ACK nscan=1 blind=1 wait_present=0 wait_unknown=0 wait_probe=0 wait_unit=0 ok=0 blind_min=90 base_min=90 unit_n=0 unit_max=0 unit=none idle_min=30 user_present=no tok_age_min=180"
exit 1
STUB
    cat > "$T3/stub-ok.sh" <<'STUB'
#!/usr/bin/env bash
printf 'judged\n' >> "$STUB_JUDGE_REC"
echo "NOTIFY-ACK nscan=0 blind=0 wait_present=0 wait_unknown=0 wait_probe=0 wait_unit=0 ok=0 blind_min=90 base_min=90 unit_n=0 unit_max=0 unit=none idle_min=30 user_present=yes tok_age_min=5"
exit 0
STUB
    cat > "$T3/stub-notify.sh" <<'STUB'
#!/usr/bin/env bash
printf '%s\n' "$*" >> "$STUB_REC"
[ "$1" = p1 ] && : > "$STUB_PEND/$2"
exit 0
STUB
    chmod +x "$T3"/stub-*.sh
    runonce() { # runonce <rec> <statdir> <judge>
        local d; d="$(dirname "$1")"
        mkdir -p "$d/dyops/pending"; : >> "$1"
        STUB_REC="$1" STUB_PEND="$d/dyops/pending" STUB_JUDGE_REC="$d/judge.calls" \
            DYOPS_NOTIFY="$T3/stub-notify.sh" DYOPS_NOTIFY_ACK="$3" \
            DYOPS_STATE_DIR="$2" DYOPS_CONFIRM=2 DY_OPS_DIR="$d/dyops" \
            bash /usr/local/sbin/dy-ops-watch.sh --only notify-blind >/dev/null 2>&1
    }
    # N3 负控：第 1 次命中（n=1 < CONFIRM=2）⇒ 记录里不许出现 p0
    runonce "$T3/rec1" "$T3/sta1" "$T3/stub-blind.sh"
    nscan=$((nscan+1))
    if [ "$(wc -l < "$T3/judge.calls" 2>/dev/null || echo 0)" -lt 1 ]; then
        bad "P4 前提不成立：判据桩一次都没被调 ⇒ 下面的「没推 p0」不算证据"
    elif grep -q '^p0 ' "$T3/rec1" 2>/dev/null; then
        bad "P4 负控失败：连续 1/2 次就推了 p0（M414 修的正是这条）"
    else
        ok "P4 负控：判据跑了 1 次、连续 1/2 命中 ⇒ 不推 p0（防抖对 p0 生效）"
    fi
    # P5 正控：第 2 次达阈值 ⇒ p1 必须出现，p0 也应当出现（证明没把 p0 永久闩死）
    runonce "$T3/rec1" "$T3/sta1" "$T3/stub-blind.sh"
    if grep -q '^p1 ' "$T3/rec1" 2>/dev/null && grep -q '^p0 ' "$T3/rec1" 2>/dev/null; then
        ok "P5 正控：连续 2 次 ⇒ p1 与 p0 都到位（防抖没有将升级路 / p0 通道堵死）"
    else
        bad "P5 正控失败：达阈值后 p1=$(grep -c '^p1 ' "$T3/rec1" 2>/dev/null) p0=$(grep -c '^p0 ' "$T3/rec1" 2>/dev/null)（预期各至少 1）"
    fi
    # P6 负控：判据绿 ⇒ 两次都不许 p1/p0
    jb="$(wc -l < "$T3/judge.calls" 2>/dev/null || echo 0)"
    runonce "$T3/rec2" "$T3/sta2" "$T3/stub-ok.sh"
    runonce "$T3/rec2" "$T3/sta2" "$T3/stub-ok.sh"
    nscan=$((nscan+1))
    if [ "$(( $(wc -l < "$T3/judge.calls" 2>/dev/null || echo 0) - jb ))" -lt 2 ]; then
        bad "P6 前提不成立：绿判据桩没有跑到 2 次"
    elif grep -qE '^(p0|p1) ' "$T3/rec2" 2>/dev/null; then
        bad "P6 负控失败：判据绿却升级 ⇒ 假红"
    else
        ok "P6 负控：绿判据跑 2 次 ⇒ 不升级（不假红）"
    fi
    rm -rf "$T3"
else
    nscan=$((nscan-1))
    echo "  · 宿主面无 dy-ops-watch(-lib-ack).sh ⇒ P4/P5/P6 跳过（nscan=$nscan）"
    [ "$nscan" -gt 0 ] || { echo "❌ 扣完扫描面为 0 ⇒ 本门自己判红" >&2; exit 1; }
fi

# P7 真判据（宿主）：现在的真实台账按新口径判一遍，照实报
if [ -d /var/log/dy-ops/pending ]; then
    out="$(bash "$NA" 2>&1)"; rc=$?
    echo "  · $(grep -E '^NOTIFY-ACK' <<<"$out" | tail -1)"
    if [ "$rc" = 0 ]; then
        ok "P7 宿主台账：无 BLIND（rc=0，等工序不再算盲区）"
    elif [ "$rc" = 1 ]; then
        bad "P7 宿主存在真 BLIND（probe 已满足却没人处理）"
        grep 'BLIND ' <<<"$out" | sed 's/^/     /' | head -5
    else
        bad "P7 判据自身前提失守（rc=$rc）：$(printf '%s' "$out" | head -1)"
    fi
fi

# P8 · 在途单元查询抖动（缺陷 833 · M415 轮次连红两轮的真因）：不许打成 PREMISE-FAIL rc=3
T4="$(mktemp -d)"
mkdir -p "$T4/bin" "$T4/pend"
printf '#!/bin/sh\nexit 1\n' > "$T4/bin/systemctl"; chmod +x "$T4/bin/systemctl"
printf '%s\t夹具\nESCALATED\t%s\n' "$(( $(date +%s) - 7200 ))" "$(( $(date +%s) - 7200 ))" > "$T4/pend/jitter-case"
printf '{"jitter-case":{"cmds":["true"]}}' > "$T4/park.json"
sqlite3 "$T4/tok.db" "create table token_cache(updated_at integer); insert into token_cache values ($(( $(date +%s) - 7200 )));" 2>/dev/null
out="$(PATH="$T4/bin:$PATH" NA_PEND_DIR="$T4/pend" NA_DB="$T4/tok.db" NA_PARKWHEN="$T4/park.json" bash "$NA" 2>&1)"; rc=$?
if [ "$rc" = 3 ]; then
    bad "P8 负控失败：查询抖动仍被判前提失守（rc=3）out=$(printf '%s' "$out" | head -1)"
elif [ "$rc" = 1 ] || grep -qE '^BLIND ' <<<"$out"; then
    bad "P8 负控失败：查询抖动下假报 BLIND（rc=$rc）"
elif grep -q 'units_state=unavailable' <<<"$out" && grep -q 'wait-unknown' <<<"$out"; then
    ok "P8 负控：systemctl 抖动 ⇒ units_state=unavailable · 归 wait-unknown（不 exit 3、不假报 BLIND）"
else
    bad "P8 查询抖动退档不自证：rc=$rc out=$(printf '%s' "$out" | head -1)"
fi
rm -rf "$T4"

echo "── 断言：$((n-FAIL))/$n 通过 ──"
[ "$FAIL" = 0 ] && { echo "M414-NOTIFY-WAITING-OK"; exit 0; } || { echo "M414-NOTIFY-WAITING-FAIL"; exit 1; }
