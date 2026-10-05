#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════
# examples/m271_run_fast/verify.sh —— **run 层三形态对齐门**（M271）
# ------------------------------------------------------------
# 来历：晨曦《回执】M266 已读 + D1 口径的判断》（2026-10-05 12:21）第 4 条：
#   「**D1 必须配一道「run 层三形态对齐门」**（否则 D1 无法验收，我也无法独立复验）」
#     · 同一脚本三形态：`px run`(interp) / `px run --fast`(冷+热) / `px build` 产物
#       ⇒ **stdout 逐字节一致 + 退出码一致**
#     · **错误路径也逐字节**（`spawn` 脚本三形态的消息与退出码一致）
#     · **缓存污染负控**：写坏/替换缓存条目 ⇒ 要么重建、要么响亮失败，不得产出不同结果
#     · **key 敏感性负控**：改一格源码 ⇒ 必须 miss
#     · 三形态里**至少一形态在 CI 常驻**（本门即常驻形态）
#   同信另四条：① key 须含**运行器契约**；② 子集判据须**先于查缓存**；
#   ③ 编译失败**禁止回落解释轨**；⑤ 加一行**发现性提示**。
#
# 判据哲学：「不回落」不能用「跑得快」证明 ⇒ 本门用「四条路径产物/输出/退出码全等」
#   ＋静态形状判据 ＋ 三组负控（各自独立判红）。
#
# 用法：
#   bash examples/m271_run_fast/verify.sh             # 跑门
#   bash examples/m271_run_fast/verify.sh --neg       # 含负控
#   bash examples/m271_run_fast/verify.sh --neg-skip  # CI 用
# 环境：M271_ROOT / M271_W / PX_RUN_HINT_SEC
# ══════════════════════════════════════════════════════════════
set -uo pipefail

ROOT="${M271_ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
DIR="$ROOT/examples/m271_run_fast"
PX="$ROOT/tools/px"
SRC="$DIR/src"
W="$(mktemp -d "${M271_W:-/tmp/m271_run_fast.XXXXXX}")"
NEG=0; NEG_SKIP=0
for a in "$@"; do
    [ "$a" = "--neg" ] && NEG=1
    [ "$a" = "--neg-skip" ] && NEG_SKIP=1
done
SRC_SNAP="$W/px.snapshot"; cp "$PX" "$SRC_SNAP"

PASS=0; FAIL=0
ok()  { PASS=$((PASS+1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1"; else bad "$1（期望 [$3] 实得 [$2]）"; fi; }

cleanup() { cp "$SRC_SNAP" "$PX"; chmod +x "$PX"; rm -rf "$SRC/build" "$SRC/.m271_tmp.px" "$W"; }
trap cleanup EXIT

echo "═══ M271 · run 层三形态对齐（晨曦 D1 五点）═══"

# ─────────────────────────────────────────────────────────────
# [1] 静态形状（四点实现；**只看代码行** —— 注释里的片段不算，M270b 教训）
# ─────────────────────────────────────────────────────────────
echo "── [1] 静态形状"
CODE="$W/px.code"; grep -vE '^[[:space:]]*#' "$PX" > "$CODE" || true

grep -q 'bootstrap/pxi' "$CODE" && ok "[1.1] key 含运行器契约（bootstrap/pxi）" || bad "[1.1] key 未含 bootstrap/pxi"

s=$(grep -n 'run_subset_ok "\$abs"' "$PX" | head -1 | cut -d: -f1)
p=$(grep -n 'if \[ -x "\$prog" \]' "$PX" | head -1 | cut -d: -f1)
if [ -n "$s" ] && [ -n "$p" ] && [ "$s" -lt "$p" ]; then
    ok "[1.2] 子集判据先于查缓存（行 $s < $p）"
else
    bad "[1.2] 顺序不符（subset=$s 查缓存=$p）"
fi

if grep -A12 '编译失败（脚本在 Mini 子集内' "$PX" | grep -q 'PXI_BIN'; then
    bad "[1.3] 编译失败分支调用解释器 ⇒ 回落未禁"
else
    ok "[1.3] 编译失败分支不回落（窗口内无 PXI_BIN）"
fi

grep -q 'PX_RUN_HINT_SEC' "$CODE" && ok "[1.4] 发现性提示在位" || bad "[1.4] 缺发现性提示"
grep -q 'prog\.sha256' "$CODE" && ok "[1.5] 缓存完整性校验在位" || bad "[1.5] 缺完整性校验"
grep -qF 'PX_RUN_NOCACHE:-}" ]; then "$PXI_BIN" "$@" "$file"' "$CODE" \
    && ok "[1.6] NOCACHE 分支参数序正确（脚本在末尾）" || bad "[1.6] NOCACHE 分支缺脚本参数"

# ─────────────────────────────────────────────────────────────
# [1b] 发现性提示**不得污染单流捕获**（M271 补 · 缺陷 470）
#   修前无条件写 stderr ⇒ `px run x.px > out 2>&1` 会多一行，实测 examples/m148_ieee_div
#   的「三轨输出逐字节一致」当场判红（提示行混进被 diff 的解释轨输出）。
#   判据两层（静态 + 动态），动态那一层是关键 —— 「有守卫」不等于「守卫生效」：
#     静态：源码必须有 `[ -t 2 ]`
#     动态：非 TTY 捕获**不得**出现提示；`PX_RUN_HINT=1` 强制**必须**出现（证明功能没被掐死）
# ─────────────────────────────────────────────────────────────
echo "── [1b] 提示不污染单流捕获（缺陷 470）"
grep -qF '[ -t 2 ]' "$CODE" \
    && ok "[1.7] 提示带 TTY 守卫" || bad "[1.7] 提示缺 TTY 守卫（会污染 > out 2>&1）"
_h1="$W/hint_plain.txt"; _h2="$W/hint_forced.txt"
PX_RUN_HINT_SEC=0 "$ROOT/tools/px" run "$SRC/ok.px" > "$_h1" 2>&1 || true
if grep -q '提示：本脚本' "$_h1"; then
    bad "[1.8] 非 TTY 捕获里出现提示 ⇒ 污染脚本输出"
else
    ok "[1.8] 非 TTY 捕获无提示（2>&1 干净）"
fi
PX_RUN_HINT_SEC=0 PX_RUN_HINT=1 "$ROOT/tools/px" run "$SRC/ok.px" > "$_h2" 2>&1 || true
grep -q '提示：本脚本' "$_h2" \
    && ok "[1.9] PX_RUN_HINT=1 可强制（功能未被掐死）" || bad "[1.9] 强制开关无效"

# ─────────────────────────────────────────────────────────────
# [2] 三形态对齐
# ─────────────────────────────────────────────────────────────
echo "── [2] 三形态对齐（interp / --fast 冷 / --fast 热 / build 产物）"
CACHE="$W/cache"; mkdir -p "$CACHE"
okpx="$SRC/ok.px"

"$PX" run "$okpx" > "$W/o.interp" 2>"$W/e.interp"; r_interp=$?
PX_RUNCACHE_DIR="$CACHE" "$PX" run --fast "$okpx" > "$W/o.fast1" 2>"$W/e.fast1"; r_fast1=$?
PX_RUNCACHE_DIR="$CACHE" "$PX" run --fast "$okpx" > "$W/o.fast2" 2>"$W/e.fast2"; r_fast2=$?
( cd "$SRC" && "$PX" build ok.px ) > "$W/o.buildlog" 2>&1
bo="$SRC/build/ok"
if [ -x "$bo" ]; then "$bo" > "$W/o.prog" 2>"$W/e.prog"; r_prog=$?; else r_prog=99; fi

chk "[2.1] 四形态退出码一致" "$r_interp,$r_fast1,$r_fast2,$r_prog" "0,0,0,0"
cmp -s "$W/o.interp" "$W/o.fast1" && ok "[2.2] interp == fast(冷)" || bad "[2.2] interp ≠ fast(冷)"
cmp -s "$W/o.interp" "$W/o.fast2" && ok "[2.3] interp == fast(热)" || bad "[2.3] interp ≠ fast(热)"
cmp -s "$W/o.interp" "$W/o.prog"  && ok "[2.4] interp == build产物" || bad "[2.4] interp ≠ build产物"
if grep -q '首次运行' "$W/e.fast2"; then
    bad "[2.5] 热态未命中（stderr 仍含「首次运行」）"
else
    ok "[2.5] 热态命中缓存（未重编）"
fi

# ─────────────────────────────────────────────────────────────
# [3] 错误路径逐字节（子集外脚本）
# ─────────────────────────────────────────────────────────────
echo "── [3] 错误路径（子集外 spawn）"
conc="$SRC/conc.px"
"$PX" run "$conc" > "$W/co.interp" 2>&1; rc_interp=$?
PX_RUNCACHE_DIR="$CACHE" "$PX" run --fast "$conc" > "$W/co.fast1" 2>&1; rc_fast1=$?
PX_RUNCACHE_DIR="$CACHE" "$PX" run --fast "$conc" > "$W/co.fast2" 2>&1; rc_fast2=$?

[ "$rc_interp" != "0" ] && ok "[3.1] interp 响亮拒绝（rc=$rc_interp）" || bad "[3.1] interp 未拒绝"
chk "[3.2] --fast(冷) rc == interp" "$rc_fast1" "$rc_interp"
chk "[3.3] --fast(热) rc == interp" "$rc_fast2" "$rc_interp"
grep -q '回落解释轨' "$W/co.fast1" && ok "[3.4] --fast 响亮提示回落" || bad "[3.4] 未提示回落"
cmp -s "$W/co.fast1" "$W/co.fast2" && ok "[3.5] 冷/热逐字节一致（判据未被缓存跳过）" \
    || bad "[3.5] 冷/热不一致 ⇒ 缓存改变了语义"
n_cache=$(find "$CACHE" -name prog 2>/dev/null | wc -l)
chk "[3.6] 子集外脚本未入缓存" "$n_cache" "1"

# ─────────────────────────────────────────────────────────────
# [4] 缓存污染（写坏 prog）
# ─────────────────────────────────────────────────────────────
echo "── [4] 缓存污染"
CDIR="$(find "$CACHE" -name prog -printf '%h\n' 2>/dev/null | head -1)"
if [ -n "$CDIR" ] && [ -f "$CDIR/prog" ]; then
    printf '#!/bin/sh\necho POLLUTED\n' > "$CDIR/prog"; chmod +x "$CDIR/prog"
    PX_RUNCACHE_DIR="$CACHE" "$PX" run --fast "$okpx" > "$W/o.pol" 2>"$W/e.pol"
    grep -q 'POLLUTED' "$W/o.pol" 2>/dev/null && bad "[4.1] 坏产物被执行（缓存改变了结果）" \
        || ok "[4.1] 污染被检出"
    cmp -s "$W/o.interp" "$W/o.pol" && ok "[4.2] 污染后输出与 interp 一致（已重建）" \
        || bad "[4.2] 污染后输出不一致"
else
    bad "[4.1] 未找到缓存条目（前置 [2] 未建缓存）"
fi

# ─────────────────────────────────────────────────────────────
# [5] key 敏感性（改一格源码）
# ─────────────────────────────────────────────────────────────
echo "── [5] key 敏感性"
sed 's/\[0, 1, 2, 3, 4\]/[0, 1, 2, 3, 4, 5]/' "$okpx" > "$SRC/.m271_tmp.px"
PX_RUNCACHE_DIR="$CACHE" "$PX" run --fast "$SRC/.m271_tmp.px" > "$W/o.mod" 2>"$W/e.mod"; r_mod=$?
rm -f "$SRC/.m271_tmp.px"
chk "[5.1] 改一格源码 rc=0" "$r_mod" "0"
grep -q '首次运行' "$W/e.mod" && ok "[5.2] 改一格 ⇒ 缓存 miss" || bad "[5.2] 改一格仍命中旧缓存"
cmp -s "$W/o.mod" "$W/o.interp" && bad "[5.2b] 输出与改前相同 ⇒ 未真正重编" \
    || ok "[5.2b] 输出已变（确证走新产物）"

# ─────────────────────────────────────────────────────────────
# [6] 发现性提示
# ─────────────────────────────────────────────────────────────
echo "── [6] 发现性提示"
# ⚠️ M273（CI 红修复）：**必须显式压阈值**。slow.px 是 30000 次循环 —— 本机 ~3s，
#   而 CI runner 上 **< 2s**（实测：CI 上 [6.1c] 判红「强制开关无效」）。
#   不压阈值时：[6.1b] 会**空过**（提示压根没触发，不是因为 TTY 守卫生效）、
#   [6.1c] 会**假红**（强制开关本来是生效的，只是没到阈值）。
#   ⇒ 纪律：**判据不得依赖墙上时钟**（同族：M168 ldd 退出码 / M169 按 mtime 选料 /
#     M213 写死路径 / M215 tag 守卫窗口 / M219 PIE 垫片）。压到 0 后判据只测「开关」本身。
PX_RUN_HINT_SEC=0 "$PX" run "$SRC/slow.px" > "$W/o.slow" 2>"$W/e.slow"; rc_slow=$?
chk "[6.1] 慢脚本 rc=0" "$rc_slow" "0"
grep -q 'px run --fast' "$W/e.slow" \
    && bad "[6.1b] 非 TTY 捕获出现提示（缺陷 470 回归：污染 > out 2>&1）" \
    || ok "[6.1b] 非 TTY 捕获无提示（缺陷 470 守卫生效）"
PX_RUN_HINT_SEC=0 PX_RUN_HINT=1 "$PX" run "$SRC/slow.px" > /dev/null 2>"$W/e.slow2"
grep -q 'px run --fast' "$W/e.slow2" \
    && ok "[6.1c] PX_RUN_HINT=1 强制下出现提示（功能未掐死）" \
    || bad "[6.1c] 强制开关无效"
"$PX" run "$SRC/tiny.px" > /dev/null 2>"$W/e.tiny"
grep -q 'px run --fast' "$W/e.tiny" && bad "[6.2] 秒起脚本也打提示（噪声）" || ok "[6.2] 秒起脚本不打提示"

# ─────────────────────────────────────────────────────────────
# [7] 负控（3 道，各自独立判红）
# ─────────────────────────────────────────────────────────────
if [ "$NEG" = 1 ] && [ "$NEG_SKIP" = 0 ]; then
    echo "── [7] 负控"
    neg() { # $1=名 $2=sed 表达式
        cp "$SRC_SNAP" "$PX"
        sed -i "$2" "$PX"
        if ! bash -n "$PX" 2>/dev/null; then echo "  ⚠️ [7-$1] 语法错（负控无效）"; FAIL=$((FAIL+1)); cp "$SRC_SNAP" "$PX"; chmod +x "$PX"; return; fi
        out="$(bash "$0" --neg-skip 2>&1)"
        cp "$SRC_SNAP" "$PX"; chmod +x "$PX"
        printf '%s' "$out" | grep -q '❌' && echo "  ✅ [7-$1] 判红（有牙）" || { echo "  ❌ [7-$1] 未判红（无牙）"; FAIL=$((FAIL+1)); }
    }
    # A 撤回「先判源码」⇒ 子集外脚本被编译 ⇒ [3.2]/[3.6] 必红
    neg "A-先判源码" 's#if ! viol="\$(run_subset_ok "\$abs")"; then#if false; then#'
    # B 撤回完整性校验的**写入** ⇒ 老缓存无 sha256 ⇒ 永远重建 ⇒ [2.5] 必红
    neg "B-完整性" 's@^    sha256sum "\$prog".*@    : M271-NEG@'
    # C 撤回 key 里的**源码哈希** ⇒ 改一格仍命中 ⇒ [5.2] 必红
    neg "C-key源码" 's#sha256sum "\$1" | cut -c1-16#sha256sum /dev/null | cut -c1-16#'
fi

echo "── 结果：$PASS 通过 / $FAIL 失败"
[ "$FAIL" = 0 ] && echo "M271-VERIFY-OK" || echo "M271-VERIFY-FAIL"
exit $(( FAIL > 0 ))
