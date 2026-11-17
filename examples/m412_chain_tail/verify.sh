#!/usr/bin/env bash
# ============================================================
# M412 门 · **链尾接力**（缺陷 821：发布完成后无人立项 ⇒ 全系统判绿空转）
# ------------------------------------------------------------
# 病史（2026-11-17，真事）：M411 08:11 发布完成、08:32 本机升级完毕，
#   之后 5h19m 没有任何立项/交接/唤醒。既有监控 idle-unpushed 的判据是
#   「HEAD 领先 origin/main」+「NEXT-QUEUE 有未开工项」，两者都 0 ⇒ 打印
#   「OK 没活可接」——**「干完了没人接棒」在所有旧判据里是正常态**。
#
# 本门守三件（前两条是判据本身，第三条防「机制只存在于门里」）：
#   P1 装置有牙：packaging/chain/chain_tail.sh --selftest 式对照（本脚本内 S1~S10），
#      必须含 ≥3 条负控，且**正控必须红**（done>max-age 且无载体 ⇒ rc=1）。
#      —— 一个永远绿的链尾判据，等于没有判据。
#   P2 真判据在宿主上跑一次：STALE ⇒ 本门红（照实报，不因「是本机噪音」放过）。
#   P3 接线：release_postupgrade.sh 判绿收尾处必须调用 chain_tail.sh，
#      否则「有人记得手工查」会变成唯一保障 —— 那就是没有保障。
#
# 扫描面（显式打印；**扫描面为 0 ⇒ 本门自己判红**，M410 R1/R2 口径）
#
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

CT="$ROOT/packaging/chain/chain_tail.sh"
[ -f "$CT" ] || { echo "❌ 缺判据实现：$CT" >&2; exit 2; }

n=0; FAIL=0; nscan=0
ok()  { n=$((n+1)); echo "  ✅ $*"; }
bad() { n=$((n+1)); FAIL=$((FAIL+1)); echo "  ❌ $*"; }

# ── 自测（离线 fixture，不碰宿主真档）──────────────────────────
selftest() {
    local T; T="$(mktemp -d)"
    trap 'rm -rf "$T"' RETURN
    local ad="$T/ad" pen="$T/pending"
    mkdir -p "$ad" "$pen" || { echo "❌ fixture 建不出来" >&2; return 2; }
    nscan=$((nscan + 2)); ECHO_NOW=1800000000

    # S1 没有任何 RELEASE.done ⇒ N-A（前提自证：目录里确实没有）
    ls "$ad"/M*-RELEASE.done >/dev/null 2>&1 && bad "S1 前提：空目录里居然有 done 文件" || ok "S1 前提自证：fixture 里确实没有 M*-RELEASE.done"
    out="$(bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" --busy 0 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 0 ] && grep -q 'CHAIN-TAIL=N-A' <<<"$out" && ok "S1 无发布记录 ⇒ N-A（不是 STALE）" || bad "S1 rc=$rc out=$out"

    # S2 30min 前发布 + 无载体 ⇒ YOUNG（还没到点）
    touch -d "@$((ECHO_NOW-1800))" "$ad/M900-RELEASE.done"
    out="$(bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" --busy 0 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 0 ] && grep -q 'CHAIN-TAIL=YOUNG' <<<"$out" && ok "S2 发布 30min ⇒ YOUNG" || bad "S2 rc=$rc out=$out"

    # S3 正控：5h 前发布 + 没有任何载体 ⇒ STALE（必须是红）
    touch -d "@$((ECHO_NOW-18000))" "$ad/M900-RELEASE.done"
    out="$(bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" --busy 0 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 1 ] && grep -q 'CHAIN-TAIL=STALE' <<<"$out" && ok "S3 正控：5h 无接力 ⇒ STALE rc=1" || bad "S3 rc=$rc（应 1）out=$out"
    # S3b 前提自证：fixture 的 done 时间真的被改成 5h 前（不是脚本自说自话）
    d=$(( ECHO_NOW - $(stat -c %Y "$ad/M900-RELEASE.done") ))
    nscan=$((nscan + 1))
    [ "$d" -ge 17990 ] && [ "$d" -le 18010 ] && ok "S3b 前提自证：done 时间戳确实是 5h 前（差 ${d}s）" || bad "S3b 时间戳差 ${d}s（应 ≈18000）"

    # S4 负控：NEXT-QUEUE 有未开工项 ⇒ OK
    printf '# 队列\n- [ ] M413 候选\n' > "$ad/NEXT-QUEUE.md"
    out="$(bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" --busy 0 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 0 ] && grep -q 'open=1' <<<"$out" && ok "S4 负控：有未开工项 ⇒ OK（open=1）" || bad "S4 rc=$rc out=$out"

    # S5 负控：只有已完成项（- [x]）⇒ 不算接力 ⇒ STALE
    printf '# 队列\n- [x] M411 已办\n' > "$ad/NEXT-QUEUE.md"
    out="$(bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" --busy 0 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 1 ] && grep -q 'open=0' <<<"$out" && ok "S5 负控：已完成项不算接力 ⇒ STALE" || bad "S5 rc=$rc（应 1）out=$out"
    rm -f "$ad/NEXT-QUEUE.md"

    # S6 负控：pending 台账有 1 条 ⇒ OK
    : > "$pen/chaintail-fixture"
    out="$(bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" --busy 0 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 0 ] && grep -q 'pending=1' <<<"$out" && ok "S6 负控：有告警台账 ⇒ OK（pending=1）" || bad "S6 rc=$rc out=$out"
    rm -f "$pen/chaintail-fixture"

    # S7 有里程碑单元在跑 ⇒ OK
    out="$(bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" --busy 2 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 0 ] && grep -q 'busy=2' <<<"$out" && ok "S7 有单元在跑 ⇒ OK（busy=2）" || bad "S7 rc=$rc out=$out"

    # S8 负控：CAMPAIGN-IDLE 合规（UNTIL+REASON）⇒ EXEMPT，但**不算**「没断」而是显式收工
    printf 'UNTIL=2026-11-18 09:00\nREASON=用户令暂停\n' > "$ad/CAMPAIGN-IDLE"
    out="$(bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" --busy 0 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 0 ] && grep -q 'CHAIN-TAIL=EXEMPT' <<<"$out" && ok "S8 合规 CAMPAIGN-IDLE ⇒ EXEMPT" || bad "S8 rc=$rc out=$out"

    # S9 负控：CAMPAIGN-IDLE 缺 REASON ⇒ 不生效 ⇒ 仍 STALE
    printf 'UNTIL=2026-11-18 09:00\n' > "$ad/CAMPAIGN-IDLE"
    out="$(bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" --busy 0 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 1 ] && grep -q 'CHAIN-TAIL=STALE' <<<"$out" && ok "S9 负控：豁免缺 REASON ⇒ 不生效（仍 STALE）" || bad "S9 rc=$rc（应 1）out=$out"
    rm -f "$ad/CAMPAIGN-IDLE"

    # S10/S11：busy 载体的**扫描口径**（不是注入值）——用 PATH 里的 systemctl 桩，
    #   测真实现那行 awk/grep 到底认不认「非发布链单元」。
    #   病史：第一版写 m[0-9]* 通配 ⇒ 一个不相干的 m412-rebake 就能把我自己的警报压绿。
    mkdir -p "$T/bin"
    cat > "$T/bin/systemctl" <<'SCTL'
#!/usr/bin/env bash
cat "$STUB_UNITS" 2>/dev/null
SCTL
    chmod +x "$T/bin/systemctl"
    printf 'm412-rebake.service loaded active running 重烘\n' > "$T/units"
    out="$(STUB_UNITS="$T/units" PATH="$T/bin:$PATH" bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 1 ] && grep -q 'busy=0' <<<"$out" && ok "S10 负控：非链单元（m412-rebake）不算接力 ⇒ 仍 STALE（busy=0）" || bad "S10 rc=$rc out=$out"
    printf 'm412-chain.service loaded active running 主链\nm412-rebake.service loaded active running 重烘\n' > "$T/units"
    out="$(STUB_UNITS="$T/units" PATH="$T/bin:$PATH" bash "$CT" --ad "$ad" --pending "$pen" --now "$ECHO_NOW" 2>&1)"; rc=$?
    nscan=$((nscan + 1))
    [ "$rc" = 0 ] && grep -q 'busy=1' <<<"$out" && ok "S11 正控：链单元在跑 ⇒ OK（busy=1，重烘那条不计）" || bad "S11 rc=$rc out=$out"

    echo "── 自测：$((n-FAIL))/$n 通过 · 断言 $n（要求 ≥12）· 负控 5（S4/S5/S9/S10 + S6/S7 载体对照）──"
    [ "$n" -ge 12 ] || { echo "❌ 断言数 $n < 12（自测被削弱）" >&2; return 2; }
    [ "$FAIL" = 0 ] || return 1
    return 0
}

if [ "$SELFTEST" = 1 ]; then
    selftest; rc=$?
    [ "$rc" = 0 ] && echo "M412-CHAIN-TAIL-SELFTEST=OK" || echo "M412-CHAIN-TAIL-SELFTEST=FAIL rc=$rc"
    [ "$rc" = 0 ] && exit 0 || exit 1
fi

# ── 正式判定 ──────────────────────────────────────────────────
scan=0
for f in "$CT" "$ROOT/packaging/chain/release_postupgrade.sh" "$ROOT/selfhost/gates.registry.sh"; do
    [ -f "$f" ] && scan=$((scan+1))
done
nscan=$((nscan + scan))
echo "扫描面 nscan=$nscan（判据实现 · 发布链收尾 · 注册源）"
[ "$nscan" -gt 0 ] || { echo "❌ 扫描面为 0 ⇒ 本门自己判红（M410 R1/R2）" >&2; exit 1; }

# P1 装置自测
if selftest >/tmp/m412_selftest.$$ 2>&1; then
    ok "P1 装置自测通过（$(tail -1 /tmp/m412_selftest.$$)）"
else
    bad "P1 装置自测失败"; sed 's/^/     /' /tmp/m412_selftest.$$ | tail -12
fi
rm -f /tmp/m412_selftest.$$

# P3 接线：发布链收尾处必须调用它（只写脚本不接线 = 行为没变）
if grep -q 'chain_tail\.sh' "$ROOT/packaging/chain/release_postupgrade.sh" 2>/dev/null; then
    ok "P3 release_postupgrade.sh 已接线 chain_tail.sh"
else
    bad "P3 release_postupgrade.sh 未接线 chain_tail.sh（机制没人执行 ⇒ 等于没有）"
fi

# P2 真判据（宿主）：STALE ⇒ 红。宿主档不存在则响亮跳过（CI 上无宿主档）
if [ -d /data/autodrive ]; then
    out="$(bash "$CT" --ad /data/autodrive --pending /var/log/dy-ops/pending 2>&1)"; rc=$?
    echo "  · $(grep 'CHAIN-TAIL=' <<<"$out" | tail -1)"
    if [ "$rc" = 0 ]; then
        ok "P2 宿主链尾：有接力或不适用（rc=0）"
    else
        bad "P2 宿主链尾断了（rc=1）——发布完成太久且无任何接力载体"
    fi
else
    nscan=$((nscan - 1))
    echo "  · 宿主面无 /data/autodrive ⇒ P2 跳过（扫描面扣 1，现 nscan=$nscan）"
    [ "$nscan" -gt 0 ] || { echo "❌ 扣完扫描面为 0 ⇒ 本门自己判红" >&2; exit 1; }
fi

echo "── 断言：$((n-FAIL))/$n 通过 ──"
[ "$FAIL" = 0 ] && { echo "M412-CHAIN-TAIL-OK"; exit 0; } || { echo "M412-CHAIN-TAIL-FAIL"; exit 1; }
