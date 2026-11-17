#!/usr/bin/env bash
# ============================================================
# M413 门 · **已通知 ≠ 已被读到**（缺陷 823）
# ------------------------------------------------------------
# 病史（2026-11-17，真数据）：dy-notify.sh 的 p1/esc 写完台账就调 ws-core -task-park
#   「叫醒东月」，但 buswake 有 30 分钟空闲门禁（最近 30m 无 token 业务请求才唤醒）。
#   用户在场连续对话时门禁永不满足 ⇒ 台账写着 ESCALATED（已叫过），实际没人被叫醒。
#   证据：/var/log/dy-ops/inbox.log 15 天 82 次 ESCALATE，能配对到后续了结的 12 条里
#   11 条是 EXPIRE（12h TTL 到期自清）而非被处理；M408 三次红全是用户问才知道。
#
# 本门守三件：
#   P1 判据有牙：packaging/chain/notify_ack.sh 的沙箱自测（S1~S8），含 ≥5 条负控，
#      且**正控必须红**（已升级 + 超时 + 用户不在场 ⇒ rc=1 blind=1）。永远绿的判据等于没有判据。
#   P2 前提不许假绿：台账目录不存在 ⇒ rc=3（不许当成「没有待办」）；无在场判据 ⇒ 单列 wait-unknown。
#   P3 真接线：宿主看门狗必须调用它（只写脚本不接线 = 行为没变）。
#
# 扫描面显式打印（nscan=0 ⇒ 本门自己判红，M410 R1/R2 口径）
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

# 造一条台账：首行 epoch+title，正文，末行 marker
mk_pend() { # mk_pend <file> <first_epoch> <marker>
    local f="$1" e="$2" m="$3"
    { printf '%s\t%s\n' "$e" "夹具：某发布链判红（自测用）"
      printf 'CHAIN-VERDICT=RED green=0 red=1 head=deadbee\n'
      [ -n "$m" ] && printf '%s\t%s\tdeadbeef\n' "$m" "$((e + 3600))"
    } > "$f"
}

# 造 token_cache db：max(updated_at) = 指定值
mk_db() { # mk_db <path> <updated_at>
    local db="$1" ts="$2"
    rm -f "$db"
    sqlite3 "$db" "create table token_cache(key text primary key, value text, module text, expire_at int, created_at int default 0, updated_at int default 0);" 2>/dev/null || return 1
    sqlite3 "$db" "insert into token_cache(key,value,module,expire_at,created_at,updated_at) values('k','v','token_cache',0,$ts,$ts);" 2>/dev/null || return 1
}

selftest() {
    local T; T="$(mktemp -d)"
    trap 'rm -rf "$T"' RETURN
    local pen="$T/pending" db="$T/tc.db" out rc
    local NOW=1800000000
    mkdir -p "$pen" || { echo "❌ fixture 建不出来" >&2; return 2; }

    # M414 口径更正后，「BLIND」必须先满足「该醒」= park probe 为真。
    #   ⇒ 正控夹具必须显式提供 probe=true 的 park 记录 + 空在途单元清单，否则测的是
    #     「无 park 记录」那类 wait-unknown，正控不再是正控（M414 门 N2 守这条）。
    local park="$T/park.json" units="$T/units.empty"
    printf '{"dy-ops-fixture-red":{"cmds":["test -d %s"]}}\n' "$pen" > "$park"
    : > "$units"
    echo "  · 夹具 park=$park（probe=true 指向 fixture 目录）· units=$units（空）"
    # S1 前提自证：空台账目录 ⇒ nscan=0 且 rc=0（不是「假绿」，是真空）
    out="$(NA_PEND_DIR="$pen" NA_DB="$db" NA_NOW="$NOW" NA_PARKWHEN="$park" NA_UNITS_FILE="$units" bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 0 ] && grep -q 'nscan=0' <<<"$out" && ok "S1 空台账 ⇒ nscan=0 rc=0（扫描面自证）" || bad "S1 rc=$rc out=$out"

    # S2 前提：台账目录不存在 ⇒ rc=3（不许当成「没有待办」）
    out="$(NA_PEND_DIR="$T/nope" NA_DB="$db" NA_NOW="$NOW" bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 3 ] && grep -q 'PREMISE-FAIL' <<<"$out" && ok "S2 前提失守 ⇒ PREMISE-FAIL rc=3（不假绿）" || bad "S2 rc=$rc（应 3）out=$out"

    # S3 正控：已升级 + 120min + 用户 180min 未活动 ⇒ BLIND=1 rc=1
    mk_pend "$pen/dy-ops-fixture-red" "$((NOW-7200))" ESCALATED
    mk_db "$db" "$((NOW-10800))" || bad "S3 夹具 db 建失败（后续结论不可信）"
    out="$(NA_PEND_DIR="$pen" NA_DB="$db" NA_NOW="$NOW" NA_PARKWHEN="$park" NA_UNITS_FILE="$units" NA_BLIND_MIN=90 NA_IDLE_MIN=30 bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 1 ] && grep -q 'blind=1' <<<"$out" && grep -q 'BLIND-KEYS' <<<"$out" \
        && ok "S3 正控：已叫未读（120min/用户离线180min）⇒ BLIND rc=1" || bad "S3 rc=$rc out=$out"

    # S3b 前提自证：夹具真的满足三条（末行 ESCALATED · age=120min · 用户 180min 未动）
    local lm age_s dbts
    lm="$(tail -1 "$pen/dy-ops-fixture-red" | cut -f1)"
    age_s=$(( NOW - $(head -1 "$pen/dy-ops-fixture-red" | cut -f1) ))
    dbts="$(sqlite3 "$db" 'select max(updated_at) from token_cache;' 2>/dev/null)"
    nscan=$((nscan+1))
    [ "$lm" = ESCALATED ] && [ "$age_s" = 7200 ] && [ "$(( NOW - dbts ))" = 10800 ] \
        && ok "S3b 前提自证：末行=ESCALATED · age=$((age_s/60))min · 用户静默$(( (NOW-dbts)/60 ))min（不是脚本自说自话）" \
        || bad "S3b 夹具前提不成立 lm=$lm age=${age_s}s db_gap=$(( NOW - dbts ))s"

    # S3c 前提自证（M414 新增）：probe 必须真为 true —— 否则「该醒」不成立，正控退化
    nscan=$((nscan+1))
    if grep -q 'dy-ops-fixture-red' "$park" && bash -c "test -d $pen"; then
        ok "S3c 前提自证：park 记录存在且 probe=true（该醒的前提成立）"
    else
        bad "S3c park 夹具不成立 ⇒ S3 结论不可信"
    fi

    # S4 负控：用户在场（5min 前有 token 写入）⇒ 不许判 BLIND（这是设计如此，不打扰）
    mk_db "$db" "$((NOW-300))" || bad "S4 db 建失败"
    out="$(NA_PEND_DIR="$pen" NA_DB="$db" NA_NOW="$NOW" NA_PARKWHEN="$park" NA_UNITS_FILE="$units" bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 0 ] && grep -q 'blind=0' <<<"$out" && grep -q 'wait_present=1' <<<"$out" \
        && ok "S4 负控：用户在场 ⇒ blind=0 / wait_present=1 rc=0" || bad "S4 rc=$rc out=$out"

    # S5 负控：末行是 PENDING（没升级过）⇒ 不算「已通知」⇒ blind=0
    mk_db "$db" "$((NOW-10800))"
    mk_pend "$pen/dy-ops-fixture-red" "$((NOW-7200))" PENDING
    out="$(NA_PEND_DIR="$pen" NA_DB="$db" NA_NOW="$NOW" NA_PARKWHEN="$park" NA_UNITS_FILE="$units" bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 0 ] && grep -q 'blind=0' <<<"$out" \
        && ok "S5 负控：未升级（PENDING）⇒ 不算 BLIND（判据不越界）" || bad "S5 rc=$rc out=$out"

    # S6 负控：已升级但只挂了 30min（< BLIND_MIN 90）⇒ 不许提前报
    mk_pend "$pen/dy-ops-fixture-red" "$((NOW-1800))" ESCALATED
    out="$(NA_PEND_DIR="$pen" NA_DB="$db" NA_NOW="$NOW" NA_PARKWHEN="$park" NA_UNITS_FILE="$units" bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 0 ] && grep -q 'blind=0' <<<"$out" \
        && ok "S6 负控：30min < 90min 阈值 ⇒ 不提前报" || bad "S6 rc=$rc out=$out"

    # S7 负控：拿不到在场判据（无 db）⇒ 单列 wait-unknown，既不假红也不假绿
    mk_pend "$pen/dy-ops-fixture-red" "$((NOW-7200))" ESCALATED
    out="$(NA_PEND_DIR="$pen" NA_DB="$T/no-such.db" NA_NOW="$NOW" NA_PARKWHEN="$park" NA_UNITS_FILE="$units" bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 0 ] && grep -q 'user_present=unknown' <<<"$out" && grep -q 'wait_unknown=1' <<<"$out" \
        && ok "S7 负控：无在场判据 ⇒ wait_unknown=1 rc=0（不猜）" || bad "S7 rc=$rc out=$out"

    # S8 负控：多条目混合 —— 只有真 BLIND 那条计数（其余三类不许污染）
    mk_db "$db" "$((NOW-10800))"
    mk_pend "$pen/dy-ops-fixture-ok1" "$((NOW-1800))" ESCALATED       # 未到点
    mk_pend "$pen/dy-ops-fixture-ok2" "$((NOW-7200))" PENDING         # 未升级
    out="$(NA_PEND_DIR="$pen" NA_DB="$db" NA_NOW="$NOW" NA_PARKWHEN="$park" NA_UNITS_FILE="$units" bash "$NA" 2>&1)"; rc=$?
    nscan=$((nscan+1))
    [ "$rc" = 1 ] && grep -q 'nscan=3' <<<"$out" && grep -q 'blind=1' <<<"$out" \
        && ok "S8 负控：3 条里只有 1 条成立（nscan=3 blind=1，计数不串）" || bad "S8 rc=$rc out=$out"

    echo "── 自测：$((n-FAIL))/$n 通过 · 断言 $n（要求 ≥10）· 负控 5（S4/S5/S6/S7/S8）+ 前提 3（S1/S2/S3c）──"
    [ "$n" -ge 10 ] || { echo "❌ 断言数 $n < 10（自测被削弱）" >&2; return 2; }
    [ "$FAIL" = 0 ] || return 1
    return 0
}

if [ "$SELFTEST" = 1 ]; then
    selftest; rc=$?
    [ "$rc" = 0 ] && echo "M413-NOTIFY-ACK-SELFTEST=OK" || echo "M413-NOTIFY-ACK-SELFTEST=FAIL rc=$rc"
    [ "$rc" = 0 ] && exit 0 || exit 1
fi

# ── 正式判定 ──────────────────────────────────────────────────
scan=0
for f in "$NA" "$ROOT/selfhost/gates.registry.sh" /usr/local/sbin/dy-ops-watch.sh; do
    [ -f "$f" ] && scan=$((scan+1))
done
nscan=$((nscan + scan))
echo "扫描面 nscan=$nscan（判据实现 · 注册源 · 宿主看门狗）"
[ "$nscan" -gt 0 ] || { echo "❌ 扫描面为 0 ⇒ 本门自己判红（M410 R1/R2）" >&2; exit 1; }

# P1 装置自测
if selftest; then
    ok "P1 装置自测全绿（正控会红 · 5 条负控 · 3 条前提）"
else
    bad "P1 装置自测失败（判据不可信）"
fi

# P3 接线：宿主看门狗必须调它
if [ -f /usr/local/sbin/dy-ops-watch.sh ]; then
    if grep -q 'notify_ack' /usr/local/sbin/dy-ops-watch.sh 2>/dev/null || \
       grep -rq 'notify_ack' /usr/local/sbin/dy-ops-watch-lib-*.sh 2>/dev/null; then
        ok "P3 宿主看门狗已接线 notify_ack"
    else
        bad "P3 宿主看门狗未接线 notify_ack（只写判据不接线 = 没人执行）"
    fi

    # P4 接线有牙：桩判据 rc=1，连跑两次巡检必须升级成 p1（dry-run，不落真状态）
    T2="$(mktemp -d)"
    cat > "$T2/stub-blind.sh" <<'STUB'
#!/usr/bin/env bash
echo "BLIND         dy-ops-fixture-x(120min) title=夹具"
echo "NOTIFY-ACK nscan=1 blind=1 wait_present=0 wait_unknown=0 ok=0 blind_min=90 idle_min=30 user_present=no tok_age_min=180"
exit 1
STUB
    cat > "$T2/stub-ok.sh" <<'STUB'
#!/usr/bin/env bash
echo "NOTIFY-ACK nscan=0 blind=0 wait_present=0 wait_unknown=0 ok=0 blind_min=90 idle_min=30 user_present=yes tok_age_min=5"
exit 0
STUB
    chmod +x "$T2"/stub-*.sh
    cat > "$T2/stub-notify.sh" <<'STUB'
#!/usr/bin/env bash
printf '%s\n' "$*" >> "$STUB_REC"
exit 0
STUB
    chmod +x "$T2/stub-notify.sh"
    if [ -f /usr/local/sbin/dy-ops-watch-lib-ack.sh ]; then
        # 注意：不能用 --dry-run 测防抖 —— dry-run 不写计数文件，永远到不了 CONFIRM=2。
        #   这里改注入 DYOPS_NOTIFY 桩：真写沙箱计数、真走升级路径，只是不落真台账。
        : > "$T2/rec1"
        STUB_REC="$T2/rec1" DYOPS_NOTIFY="$T2/stub-notify.sh" DYOPS_NOTIFY_ACK="$T2/stub-blind.sh" \
            DYOPS_STATE_DIR="$T2/sta1" DYOPS_CONFIRM=2 \
            bash /usr/local/sbin/dy-ops-watch.sh --only notify-blind >/dev/null 2>&1
        STUB_REC="$T2/rec1" DYOPS_NOTIFY="$T2/stub-notify.sh" DYOPS_NOTIFY_ACK="$T2/stub-blind.sh" \
            DYOPS_STATE_DIR="$T2/sta1" DYOPS_CONFIRM=2 \
            bash /usr/local/sbin/dy-ops-watch.sh --only notify-blind >/dev/null 2>&1
        nscan=$((nscan+1))
        if grep -q 'p1 notify-blind' "$T2/rec1" 2>/dev/null; then
            ok "P4 正控：接线有牙（BLIND 连续 2 次巡检 ⇒ 真发 p1 notify-blind）"
        else
            bad "P4 正控失败：接线不升级 ⇒ 判据再好也不响（rec=$(tr '\n' ' ' < "$T2/rec1" | cut -c1-120)）"
        fi

        # P5 负控：判据绿 ⇒ 两次巡检都不许升级
        : > "$T2/rec2"
        STUB_REC="$T2/rec2" DYOPS_NOTIFY="$T2/stub-notify.sh" DYOPS_NOTIFY_ACK="$T2/stub-ok.sh" \
            DYOPS_STATE_DIR="$T2/sta2" DYOPS_CONFIRM=2 \
            bash /usr/local/sbin/dy-ops-watch.sh --only notify-blind >/dev/null 2>&1
        STUB_REC="$T2/rec2" DYOPS_NOTIFY="$T2/stub-notify.sh" DYOPS_NOTIFY_ACK="$T2/stub-ok.sh" \
            DYOPS_STATE_DIR="$T2/sta2" DYOPS_CONFIRM=2 \
            bash /usr/local/sbin/dy-ops-watch.sh --only notify-blind >/dev/null 2>&1
        nscan=$((nscan+1))
        if grep -q 'p1 notify-blind' "$T2/rec2" 2>/dev/null; then
            bad "P5 负控失败：判据绿却升级 ⇒ 假红"
        else
            ok "P5 负控：判据绿 ⇒ 不升级（不假红）"
        fi
    else
        bad "P4/P5 缺宿主库 dy-ops-watch-lib-ack.sh ⇒ 接线无从验证"
    fi
    rm -rf "$T2"
else
    nscan=$((nscan-1)); echo "  · 宿主面无 dy-ops-watch.sh ⇒ P3/P4/P5 跳过（nscan=$nscan）"
    [ "$nscan" -gt 0 ] || { echo "❌ 扣完扫描面为 0 ⇒ 本门自己判红" >&2; exit 1; }
fi

# P2 真判据（宿主）：有 BLIND 就红，照实报
if [ -d /var/log/dy-ops/pending ]; then
    out="$(bash "$NA" 2>&1)"; rc=$?
    echo "  · $(grep -E 'NOTIFY-ACK' <<<"$out" | tail -1)"
    if [ "$rc" = 0 ]; then
        ok "P2 宿主台账：无 BLIND（rc=0）"
    elif [ "$rc" = 1 ]; then
        bad "P2 宿主存在 BLIND（已通知但没人读到）—— 自驱唤醒在失效窗口内"
        grep 'BLIND ' <<<"$out" | sed 's/^/     /' | head -5
    else
        bad "P2 判据自身前提失守（rc=$rc）"
    fi
fi

echo "── 断言：$((n-FAIL))/$n 通过 ──"
[ "$FAIL" = 0 ] && { echo "M413-NOTIFY-ACK-OK"; exit 0; } || { echo "M413-NOTIFY-ACK-FAIL"; exit 1; }
