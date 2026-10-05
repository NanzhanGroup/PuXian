#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════
# examples/m269_gcroot_coverage/verify.sh —— GC 根登记「覆盖强度」台账门（M269）
# ------------------------------------------------------------
# 来历：清歌《PuXian 下一步发展方案 v1》P0-2 ——
#   「M240–M259 连出 60+ 同族缺陷 ⇒ 筛网有洞」；
#   「东月已有 check_root_contract.py，但那是**契约合规**，不是**覆盖完整**」；
#   请求：「建 native 桥 → 是否登记 GC 根的**机械台账**，把『未审计』显式列出并计数」。
#
# ⚠️ 我方补充的必要条件（**写进实现**）：
#   台账必须**自带自证**（锚点 + 规模下限 + 判据自伤负控）。
#   否则「未审计 = 0」与「扫描器坏了」**不可区分** —— 这不是理论：
#     · M212：判据改成"源码派生触发点"了但名单没改 ⇒ 全仓"候选 0"其实是**漏报**（派生后是 3）
#     · M214：首版豁免规则写太宽 ⇒ **一口吃掉 6 条真形状的锚点**（是自证锚点当场抓回的）
#   ⇒ **台账的价值不在数字，在于「数字不可信时能自己判红」。**
#
# 判据（全部在 selfhost/check_gcroot_coverage.py 里）：
#   WEAK = P_loose \ TRIGGER_tight（= **只有靠 loose 过近似才被看见**的桥）
#   ① 规模锚点 ② 划分完备 ③ WEAK 不许扩大 ④ 基线不许过期 ⑤ 图例双向 ⑥ 判据自伤
#
# 用法：bash examples/m269_gcroot_coverage/verify.sh [--neg-skip]
# ══════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TOOL="$ROOT/selfhost/check_gcroot_coverage.py"
BASE="$ROOT/selfhost/gcroot_coverage.txt"
W="${M269_W:-/tmp/m269_gcroot_coverage}"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PASS=0; FAILN=0
ok()   { echo "  ✅ $*"; PASS=$((PASS+1)); }
bad()  { echo "  ❌ $*"; FAILN=$((FAILN+1)); }
info() { echo "  · $*"; }

# ── 基线还原护栏（M213 教训：门被中途杀掉会留下负控残留）──
BASE_ORIG="$W/base.orig"
restore_base() { [ -f "$BASE_ORIG" ] && cp "$BASE_ORIG" "$BASE" 2>/dev/null; return 0; }
trap 'restore_base' EXIT
trap 'restore_base; exit 130' INT TERM HUP

mkdir -p "$W"
echo "══ M269 GC 根登记覆盖强度台账 ══  机=$(uname -n) · $(python3 --version 2>&1)"

echo
echo "[1] 工具自证（锚点 + 判据自身有牙）"
if timeout 900 python3 "$TOOL" --root "$ROOT" --self-test > "$W/selftest.log" 2>&1; then
    ok "check_gcroot_coverage.py --self-test 通过（$(grep -c '^  ✅' "$W/selftest.log") 项）"
else
    bad "工具自证失败"; tail -12 "$W/selftest.log" | sed 's/^/     /'
fi

echo
echo "[2] 主判据（相对基线）"
cp "$BASE" "$BASE_ORIG"
h0="$(sha256sum "$BASE" | cut -c1-16)"
if timeout 900 python3 "$TOOL" --root "$ROOT" > "$W/main.log" 2>&1; then
    ok "M269-VERIFY-OK（0 失败）"
else
    bad "台账判红"; grep -E '^  ❌' "$W/main.log" | head -10 | sed 's/^/     /'
fi
grep -E '^  P_loose|^  ✅' "$W/main.log" | head -6 | sed 's/^/     /'

if [ "$NEG_SKIP" = 1 ]; then
    echo
    echo "[3] 负控 —— **已跳过（--neg-skip；CI 用）**"
    info "负控含 4 道，各自必须独立判红；本地全量门跑全量档"
else
    echo
    echo "[3] 负控（每道必须独立判红）"

    # NC-A：基线**多一条** ⇒ 「过期」必须判红
    { cat "$BASE"; echo "zzz_fake_bridge_never_exists	misc"; } > "$BASE"
    outA="$(timeout 900 python3 "$TOOL" --root "$ROOT" 2>&1)"; rcA=$?
    cp "$BASE_ORIG" "$BASE"
    if [ "$rcA" != 0 ] && printf '%s' "$outA" | grep -q '过期'; then
        ok "NC-A 生效：基线多一条 ⇒ 判红且文案含「过期」"
    else
        bad "NC-A 未生效（rc=$rcA）"
    fi

    # NC-B：基线**删一条** ⇒ 「WEAK 扩大」必须判红并指名
    grep -v '^bi_ws_stream' "$BASE_ORIG" > "$BASE"
    outB="$(timeout 900 python3 "$TOOL" --root "$ROOT" 2>&1)"; rcB=$?
    cp "$BASE_ORIG" "$BASE"
    if [ "$rcB" != 0 ] && printf '%s' "$outB" | grep -q 'bi_ws_stream'; then
        ok "NC-B 生效：基线删一条 ⇒ 判红且**指名**（bi_ws_stream）"
    else
        bad "NC-B 未生效（rc=$rcB）"
    fi

    # NC-C：图例失效 ⇒ 「未定义标签」必须判红
    sed 's/^bi_ws_stream\tws$/bi_ws_stream\tno_such_tag/' "$BASE_ORIG" > "$BASE"
    outC="$(timeout 900 python3 "$TOOL" --root "$ROOT" 2>&1)"; rcC=$?
    cp "$BASE_ORIG" "$BASE"
    if [ "$rcC" != 0 ] && printf '%s' "$outC" | grep -q '未定义的标签'; then
        ok "NC-C 生效：标签未定义 ⇒ 判红"
    else
        bad "NC-C 未生效（rc=$rcC）"
    fi

    # NC-D：**判据自伤** —— 与 NC-B 同一故障，但关掉 [3] 段 ⇒ 必须不再红
    grep -v '^bi_ws_stream' "$BASE_ORIG" > "$BASE"
    outD="$(timeout 900 python3 "$TOOL" --root "$ROOT" --no-weak-check 2>&1)"; rcD=$?
    cp "$BASE_ORIG" "$BASE"
    if [ "$rcD" = 0 ]; then
        ok "NC-D 生效：同一故障 + --no-weak-check ⇒ 不再红（证明 NC-B 的红来自台账判据本身）"
    else
        bad "NC-D 未生效（rc=$rcD，仍有判红）"
    fi
fi

echo
echo "[4] 源还原核对（负控跑完 baseline 必须逐字节回原样）"
h1="$(sha256sum "$BASE" | cut -c1-16)"
if [ "$h0" = "$h1" ]; then ok "baseline 逐字节还原（sha $h1）"; else bad "还原失败：$h0 → $h1"; fi

echo
echo "[5] 覆盖边界（如实登记）"
info "WEAK 的定义依赖 **tight 档**（= 只用于分诊的档位）⇒ 台账本身不是缺陷判据，是**判据强度**的账"
info "它**不保证零缺陷** —— 它让「还有多少地方看不清」变成可见、可追踪、**只减不增**的数字"
info "本门**不覆盖**：跨函数 F1 的间接调用链（M213 只覆盖直接调用）·「后置存活」未判据化"
info "同类工具分工：check_root_contract.py = **契约合规**（J1–J6）· gcroot_audit.py = **候选判定**"
info "  · 本门 = **覆盖强度**（三者正交，缺一不可）"

echo
echo "══ M269 结果：$PASS 通过 / $FAILN 失败 ══"
[ "$FAILN" = 0 ] && echo "M269-VERIFY-OK-INNER"
exit $([ "$FAILN" = 0 ] && echo 0 || echo 1)
