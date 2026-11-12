#!/usr/bin/env bash
# ============================================================================
# launch_release_chain.sh —— 全量门链「唯一入口」：起主链 + 全部伴生单元 + 起完自检
#
# 为什么要有它（两次实测事故，同一形状）：
#   ① 2026-11-11 13:49 起 M390 r4：只复制了主链那一条 systemd-run，
#      red-guard / postupgrade 两个伴生单元没起 ⇒ 判红没人叫我、判绿不会自动升级。
#   ② 2026-11-12 01:31 M392 r2 判绿，postupgrade 单元在 r1 判红时已 SKIP 退出、
#      **没有跟着重启** ⇒ 自动 dnf 升级从未触发，06:27 才人工补。
#   根因不在某个脚本写错，而在「起链」这件事每次靠手敲，敲漏一条没人知道。
#   ⇒ 把「起链」收成一个入口：起完立刻 list-units 自检，缺一条就 p1 叫醒。
#
# 用法：launch_release_chain.sh --ms 393 [--repo DIR] [--head SHA] [--dry-run] [--self-test]
# 退出码：0 三单元都起来了 / 2 前置条件不满足 / 3 有单元没起来（已 p1）
#
# 自测缝（全部环境变量，生产不设）：
#   SR_SYSTEMD_RUN  替代 systemd-run   SR_IS_ACTIVE  替代 systemctl is-active
#   SR_SLEEP        起完后的等待秒数（默认 5）      SR_DIRTY=1  假装树脏
# ============================================================================
set -uo pipefail

MS=""; REPO=""; HEAD_SHA=""; DRY=0; SELF=0
SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
NOTIFY_SH="${DY_NOTIFY_BIN:-/usr/local/sbin/dy-notify.sh}"
SLICE="${SR_SLICE:-ws-debug.slice}"
SLEEP="${SR_SLEEP:-5}"
LOG="${SR_LOG:-/data/autodrive/launch-chain-$(date +%Y%m%d).log}"

while [ $# -gt 0 ]; do
    case "$1" in
        --ms)        MS="${2:-}"; shift 2 ;;
        --repo)      REPO="${2:-}"; shift 2 ;;
        --head)      HEAD_SHA="${2:-}"; shift 2 ;;
        --dry-run)   DRY=1; shift ;;
        --self-test) SELF=1; shift ;;
        -h|--help)   sed -n '2,24p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done

say() { printf '[%s] %s\n' "$(date +%F' '%T)" "$*" >> "$LOG" 2>/dev/null || true; }
alert() { say "ALERT $1 $2"; [ -x "$NOTIFY_SH" ] && DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 "$1" "$2" "$3" >/dev/null 2>&1 || true; }

SYSTEMD_RUN() {  # SYSTEMD_RUN <unit> <desc> <cmd...>
    local unit="$1" desc="$2"; shift 2
    # ★同名单元可能还活着（上一轮 red-guard 报警后退出、postupgrade 还在等镜像）：
    #   不先停/重置 ⇒ systemd-run 报 "unit already exists" 静默失败 ⇒ 新链没有伴生单元。
    #   这就是 M392 的漏（「链重启但伴生单元没跟着重启」）的机制版成因，M393 重跑时实测踩到。
    if [ -z "${SR_SYSTEMD_RUN:-}" ]; then
        systemctl stop "$unit" >/dev/null 2>&1 || true
        systemctl reset-failed "$unit" >/dev/null 2>&1 || true
    else
        "$SR_SYSTEMD_RUN" --pre-stop "$unit" >/dev/null 2>&1 || true
    fi
    if [ -n "${SR_SYSTEMD_RUN:-}" ]; then "$SR_SYSTEMD_RUN" "$unit" "$@"; return $?; fi
    systemd-run --unit="$unit" --collect --slice="$SLICE" --description="$desc" "$@"
}
IS_ACTIVE() {
    if [ -n "${SR_IS_ACTIVE:-}" ]; then "$SR_IS_ACTIVE" "$1"; return $?; fi
    [ "$(systemctl is-active "$1" 2>/dev/null)" = "active" ]
}

launch() {
    [ -n "$MS" ] || { echo "--ms 必填" >&2; return 2; }
    REPO="${REPO:-$(cd "$SELF_DIR/../.." && pwd)}"
    [ -f "$REPO/selfhost/gates.registry.sh" ] || { echo "❌ 不是 PuXian 仓库：$REPO" >&2; return 2; }

    # ① 前置自证：树必须干净（门会跑 2h，跑的是脏树的话结果无意义）
    if [ "${SR_DIRTY:-0}" = "1" ] || [ -n "$(git -C "$REPO" status --porcelain 2>/dev/null)" ]; then
        echo "❌ 工作树不干净 ⇒ 拒绝起链（先 commit；门跑的是提交后的那棵树）" >&2
        return 2
    fi
    HEAD_SHA="${HEAD_SHA:-$(git -C "$REPO" rev-parse --short=7 HEAD)}"

    local U_MAIN="m${MS}-chain" U_RED="m${MS}-red-guard" U_PU="m${MS}-postupgrade"
    say "=== launch start ms=$MS head=$HEAD_SHA repo=$REPO dry=$DRY ==="

    if [ "$DRY" = "1" ]; then
        echo "DRY-RUN 将起三单元：$U_MAIN $U_RED $U_PU（不执行）"
        return 0
    fi

    # ② 主链（前置自证 → 全量分片门 → STAMP → publish_on_green）
    SYSTEMD_RUN "$U_MAIN" "M${MS} full gate chain" \
        /bin/bash "$REPO/packaging/chain/run_release_chain.sh" --ms "$MS" --repo "$REPO" >/dev/null 2>&1
    say "start $U_MAIN rc=$?"

    # ③ 判红守门（红/链消失/超时 ⇒ p1）
    SYSTEMD_RUN "$U_RED" "M${MS} red guard" \
        /bin/bash "$REPO/packaging/chain/release_red_guard.sh" --ms "$MS" --unit "$U_MAIN" >/dev/null 2>&1
    say "start $U_RED rc=$?"

    # ④ 判绿后自动升级（等 GREEN + 镜像齐 ⇒ dnf upgrade + 自验）
    SYSTEMD_RUN "$U_PU" "M${MS} post upgrade" \
        /bin/bash "$REPO/packaging/chain/release_postupgrade.sh" --ms "$MS" >/dev/null 2>&1
    say "start $U_PU rc=$?"

    # ⑤ 起完自检（**这是本脚本存在的理由**：三条都 active 才算起成）
    sleep "$SLEEP" 2>/dev/null || true
    local miss="" n=0 u
    for u in "$U_MAIN" "$U_RED" "$U_PU"; do
        if IS_ACTIVE "$u"; then say "check $u ACTIVE"; n=$((n+1))
        else say "check $u NOT-ACTIVE"; miss="$miss $u"; fi
    done
    if [ -n "$miss" ]; then
        echo "LAUNCH-VERDICT=FAIL missing:$miss（已 p1）"
        alert "m${MS}-launch-incomplete" "M${MS} 起链缺伴生单元：$miss" \
            "只有 $n/3 起来。后果：判红没人叫 / 判绿不会自动升级（M390·M392 两次实测的同一根因）。
处置：journalctl -u <单元> 看原因，手工补起后用 systemctl is-active 复核三条。"
        return 3
    fi
    echo "LAUNCH-VERDICT=OK units=3 main=$U_MAIN red=$U_RED pu=$U_PU head=$HEAD_SHA"
    say "=== launch OK ms=$MS ==="
    return 0
}

# ── 自测：1 正控 + 3 负控 + 2 前提自证，全离线 ──────────────────────────────
if [ "$SELF" = 1 ]; then
    T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
    # 假 systemd-run：记录调用，并按 FAKE_ACTIVE 名单决定「单元是否起来」
    cat > "$T/sdrun" <<'STUB'
#!/usr/bin/env bash
if [ "${1:-}" = "--pre-stop" ]; then echo "prestop $2" >> "$STUB_CALLS"; exit 0; fi
unit="$1"; shift
echo "start $unit" >> "$STUB_CALLS"
case " $FAKE_ACTIVE " in *" $unit "*) echo "$unit" >> "$STUB_UP" ;; esac
exit 0
STUB
    cat > "$T/isactive" <<'STUB'
#!/usr/bin/env bash
case " $FAKE_ACTIVE " in *" $1 "*) exit 0 ;; *) exit 1 ;; esac
STUB
    cat > "$T/notify" <<'STUB'
#!/usr/bin/env bash
printf '%s\n' "$*" >> "$STUB_ALERTS"
exit 0
STUB
    chmod +x "$T/sdrun" "$T/isactive" "$T/notify"
    export DY_NOTIFY_BIN="$T/notify" SR_LOG="$T/launch.log" SR_SLEEP=0
    export SR_SYSTEMD_RUN="$T/sdrun" SR_IS_ACTIVE="$T/isactive" STUB_CALLS="$T/calls" STUB_UP="$T/up"
    : > "$STUB_CALLS"; : > "$T/alerts.txt"; export STUB_ALERTS="$T/alerts.txt"

    # 夹具仓库（自测只判「树净 + 有 gates.registry.sh」，不真跑门）
    mkdir -p "$T/repo/selfhost" "$T/repo/packaging/chain"
    : > "$T/repo/selfhost/gates.registry.sh"
    ( cd "$T/repo" && git init -q && git add -A 2>/dev/null \
          && git -c user.email=a@b -c user.name=t commit -qm init 2>/dev/null )

    n=0; FAIL=0
    ok()  { n=$((n+1)); echo "  ✅ $*"; }
    bad() { n=$((n+1)); FAIL=$((FAIL+1)); echo "  ❌ $*"; }

    # P1 正控：三单元全 active ⇒ rc=0
    FAKE_ACTIVE="m991-chain m991-red-guard m991-postupgrade"
    out="$(FAKE_ACTIVE="$FAKE_ACTIVE" bash "$0" --ms 991 --repo "$T/repo" 2>&1)"; rc=$?
    [ "$rc" = 0 ] && ok "P1 正控：三单元都在 ⇒ rc=0" || bad "P1 rc=$rc（应 0）"
    grep -q 'LAUNCH-VERDICT=OK units=3' <<<"$out" && ok "P1 输出 LAUNCH-VERDICT=OK units=3" || bad "P1 汇总行不对"
    [ "$(grep -c '^start ' "$STUB_CALLS")" = 3 ] && ok "前提自证：systemd-run 真起了 3 个单元" \
                                       || bad "前提缺失：起单元调用数=$(grep -c '^start ' "$STUB_CALLS")（应 3）"
    # N5 负控：每个单元起前必须先 stop + reset-failed（否则同名单元存在 ⇒ systemd-run 静默失败）；
    #   M392 的漏（链重启但伴生单元没跟着重启）就是这条不在时发生的。
    [ "$(grep -c '^prestop ' "$STUB_CALLS")" = 3 ] && ok "N5 每个单元起前都做了 pre-stop（3 次）" \
                                              || bad "N5 pre-stop 次数=$(grep -c '^prestop ' "$STUB_CALLS")（应 3）"

    # N1 负控：postupgrade 没起来 ⇒ rc=3 + 点名 + p1（这正是 M390/M392 的事故）
    : > "$T/alerts.txt"
    out="$(FAKE_ACTIVE="m992-chain m992-red-guard" bash "$0" --ms 992 --repo "$T/repo" 2>&1)"; rc=$?
    [ "$rc" = 3 ] && ok "N1 负控：缺 postupgrade ⇒ rc=3" || bad "N1 rc=$rc（应 3）"
    grep -q 'm992-postupgrade' <<<"$out" && ok "N1 点名了缺的那个单元" || bad "N1 没点名"
    grep -q 'launch-incomplete' "$T/alerts.txt" 2>/dev/null && ok "N1 已 p1 叫醒" || bad "N1 没报警"
    [ ! -s "$STUB_UP" ] || true

    # N2 负控：主链没起来 ⇒ rc=3
    : > "$T/alerts.txt"
    FAKE_ACTIVE="m993-red-guard m993-postupgrade" bash "$0" --ms 993 --repo "$T/repo" >/dev/null 2>&1
    [ "$?" = 3 ] && ok "N2 负控：缺主链 ⇒ rc=3" || bad "N2 没判缺主链"

    # N3 负控：树脏 ⇒ rc=2（且不许起任何单元）
    : > "$STUB_CALLS"
    SR_DIRTY=1 bash "$0" --ms 994 --repo "$T/repo" >/dev/null 2>&1
    rc=$?
    [ "$rc" = 2 ] && ok "N3 负控：树脏 ⇒ rc=2" || bad "N3 rc=$rc（应 2）"
    [ ! -s "$STUB_CALLS" ] && ok "N3 树脏时一个单元都没起（真拒绝，不是先起再报）" || bad "N3 脏树仍起了单元"

    # N4 负控：缺 --ms ⇒ rc=2
    bash "$0" --repo "$T/repo" >/dev/null 2>&1; [ "$?" = 2 ] && ok "N4 负控：缺 --ms ⇒ rc=2" || bad "N4 没报错"

    echo "── launch 自测：$((n-FAIL))/$n 通过 ──"
    [ "$FAIL" = 0 ] || exit 1
    exit 0
fi

launch
