#!/usr/bin/env bash
# ============================================================================
# m394_release_watch —— 发布观察器与 starter 的自测必须全绿（M394 · 缺陷 745–747）
# ----------------------------------------------------------------------------
# 这道门守的不是语言本体，是**发布链的最后一条腿**：
#   2026-11-12 01:31 M392 判绿，「可 dnf upgrade」通报从未生成（publish 只 WARN 就过），
#   而用户那条「已发布」通知恰好 3 次投递失败被放弃 ⇒ 他整晚不知道发布成功。
# 判据：
#   G1  packaging/chain/release_watch.sh --selftest 全绿（判据/归一/通知标题分级）
#   G2  packaging/chain/start_release_watcher.sh --selftest 全绿（起单元 + 自检 + p1）
#   G3  两件必须可执行且 shebang 是 bash（门里用 bash 调，但发布链直接执行）
#   N1  前提自证：把 release_watch 的体积判据改坏后，它的自测**必须变红**
#       —— 否则「自测全绿」只是「自测没有鉴别力」
#   N2  starter 的「秒完成」判据：伪造 finished(ok) 必须 rc=0
#       （2026-11-12 10:51 真踩到的假红：is-active=inactive 被当成没起来）
# 用法：bash examples/m394_release_watch/verify.sh
# 退出码：0 全绿 · 1 有判据红
# ============================================================================
# M276：门级互斥锁（全量门要求每扇门都 source，否则 gate_lock_guard / m276_gate_lock 判红）
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
RW="$ROOT/packaging/chain/release_watch.sh"
ST="$ROOT/packaging/chain/start_release_watcher.sh"
FAIL=0
pass() { printf 'PASS %s\n' "$*"; }
bad()  { printf 'FAIL %s\n' "$*"; FAIL=1; }

# ── G1/G2 自测全绿 ─────────────────────────────────────────────────────
run_selftest() {  # run_selftest <标签> <脚本>
    local tag="$1" script="$2" out rc last
    if [ ! -f "$script" ]; then bad "[$tag] 脚本不存在：$script"; return; fi
    out="$(bash "$script" --selftest 2>&1)"; rc=$?
    last="$(printf '%s\n' "$out" | tail -1)"
    if [ "$rc" = 0 ]; then pass "[$tag] $last"
    else bad "[$tag] rc=$rc"; printf '%s\n' "$out" | tail -12; fi
}
run_selftest release_watch "$RW"
run_selftest start_release_watcher "$ST"

# ── G3 必须可执行 + shebang bash（发布链直接执行，不经 bash 包装时靠它）──
for f in "$RW" "$ST"; do
    b="$(basename "$f")"
    if [ -x "$f" ]; then pass "[G3] $b 可执行"; else bad "[G3] $b 不可执行"; fi
    if grep -qx '#!/usr/bin/env bash' <<< "$(head -1 "$f" 2>/dev/null)"; then
        pass "[G3] $b shebang=bash"
    else
        bad "[G3] $b shebang 不是 bash"
    fi
done

# ── N1 前提自证：破坏体积判据 ⇒ 自测必须变红（证明自测有鉴别力）──────
TMP="$(mktemp -d /tmp/m394rw.XXXXXX)"
trap 'rm -rf "$TMP" 2>/dev/null || true' EXIT
if [ -f "$RW" ]; then
    cp "$RW" "$TMP/rw-broken.sh"
    sed -i 's/^TGZ_MIN=62914560/TGZ_MIN=999999999999/' "$TMP/rw-broken.sh"
    if bash "$TMP/rw-broken.sh" --selftest >"$TMP/n1.log" 2>&1; then
        bad "[N1] 破坏 TGZ_MIN 后自测仍绿 ⇒ 自测没有鉴别力"
    else
        pass "[N1] 破坏 TGZ_MIN 后自测变红（有鉴别力）"
    fi
    # 扰动前提自证：确认 sed 真的改到了（没改到就没有扰动，N1 就是空判）
    if grep -qx 'TGZ_MIN=999999999999' "$TMP/rw-broken.sh"; then
        pass "[N1] 扰动真发生（TGZ_MIN 已被改）"
    else
        bad "[N1] 扰动没发生 ⇒ N1 是空判"
    fi
fi

# ── N2 starter「秒完成」判据必须有牙 ───────────────────────────────────
if [ -f "$ST" ]; then
    N2="$TMP/n2"; mkdir -p "$N2/repo/packaging/chain" "$N2/ad"
    printf '#!/usr/bin/env bash\nexit 0\n' > "$N2/repo/packaging/chain/release_watch.sh"
    chmod +x "$N2/repo/packaging/chain/release_watch.sh"
    printf 'finished(ok)\n' > "$N2/state"
    printf '%-40s\n' "ok" > "$N2/fake-notify"
    : > "$N2/run.log"; : > "$N2/notify.log"
    RUN_HOOK="$N2/run.log" NOTIFY_HOOK="$N2/notify.log" STATE_HOOK="$N2/state" \
        bash "$ST" --ms 394 --repo "$N2/repo" --ad "$N2/ad" --notify-bin "$N2/fake-notify" \
        >/dev/null 2>&1
    if [ "$?" = 0 ]; then pass "[N2] finished(ok) ⇒ rc=0（秒完成不是没起来）"
    else bad "[N2] finished(ok) 被判成失败"; fi
    if [ -s "$N2/notify.log" ]; then bad "[N2] 秒完成却报了 p1"; else pass "[N2] 秒完成未误报 p1"; fi
fi

echo "----"
if [ "$FAIL" = 0 ]; then echo "m394_release_watch: ALL GREEN"; exit 0; fi
echo "m394_release_watch: RED"; exit 1
