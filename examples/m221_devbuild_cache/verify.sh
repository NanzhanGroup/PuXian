#!/usr/bin/env bash
# ============================================================
# M221 门（第 100 轮）：`selfhost/devbuild.sh` 的**产物指纹短路**
# ------------------------------------------------------------
# 背景（step8 时长问题的真因）：
#   · CI「工具自测」步 = 131 门 / 507 行，上限 **45 分钟**，实测 **23 / 40+ / 37 分钟**；
#   · 其中 **23 门**各自 `devbuild.sh pxc pxi --vm`，实测单次（暖缓存）**≈45s**
#     ⇒ **23×45 ≈ 17 分钟纯重复**（占该步 40–75%）；CI 上 `.rtcache` 冷 ⇒ 单次更贵
#     ⇒ 这就是 23↔40 分钟跨度的来源。
#   · 而 23 门在同一步、同一源码链、同一 `.rtcache` ⇒ 产物**逐字节相同**。
#   ⇒ M221 给 devbuild 加「产物存在 + 指纹 == 当前源码链指纹 ⇒ 复用」（打印 `⏭`）。
#
# 本门守什么（这是**可能隐藏重建需求**的机制，假绿风险高 ⇒ 必须自己看着）：
#   ① 机制在位 + 两条关键不变量：**`runtime/*.c,h` 必须在指纹里**（门内负控会篡改它们；
#      若只按「产物比源码新」判，负控的**重建**会被吃掉 ⇒ 假绿 —— M169 老坑）、
#      **写指纹必须在构建成功之后**（失败也写 ⇒ 下次复用坏件）。
#   ② 同源码链连跑两次 ⇒ 第二次必须**全 ⏭**（且 wall 明显更短）。
#   ③ 敏感性：改 `runtime/*.c` ⇒ 指纹**必须变**且必须重建；还原后再变（证「不会误命中」）。
#   ④ `--rebuild` ⇒ 无条件重建（负控/自证的口子）。
#   ⑤ 生态自查：用 devbuild 的门数 ≥ 20，且 CI 里这些门**全带 `--neg-skip`**
#      （⇒ CI 上源码不被改 ⇒ 缓存稳定，这才是 17 分钟能被省掉的前提）。
#   ⑥ 负控 3 道**各自独立判红**：
#      A 短路判据恒假（退回无条件构建）⇒ ② 判据必红
#      B 让指纹**完全不跟踪 runtime**（去 glob **且**冻结 rtcache 名 —— 只去 glob 没牙，
#        因为 `rtcache` 目录名本身就是 runtime 内容哈希 rt_key）⇒ ③ 判据必红
#      C 判据自伤（② 的判据无视 ⏭ 数）⇒ A 的红必须**消失**（证红来自比对）
#   ⑦ 覆盖边界登记（如实）。
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
cd "$ROOT"
export LC_ALL=C LANG=C
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
W=$(mktemp -d /tmp/m221_gate.XXXXXX)

D="$ROOT/selfhost/devbuild.sh"
RC="$ROOT/runtime/runtime.c"
CI="$ROOT/.github/workflows/ci.yml"
SNAP="$W/snap"; mkdir -p "$SNAP"

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo; echo "── $*"; }

# ⚠️ 纪律（M213/M220 教训）：每道负控**各自**先 restore 再 snap —— 各自干净起点。
restore_all() {
    [ -f "$SNAP/devbuild.sh.snap" ] && cp -a "$SNAP/devbuild.sh.snap" "$D"
    [ -f "$SNAP/runtime.c.snap" ]   && cp -a "$SNAP/runtime.c.snap"    "$RC"
}
snap_all() {
    cp -a "$D"  "$SNAP/devbuild.sh.snap"
    cp -a "$RC" "$SNAP/runtime.c.snap"
}

devkey() { sed -n 's/.*源码链指纹：\([0-9a-f]\{16\}\).*/\1/p' "$1" | head -1; }

# ② 的核心判据（抽成函数 —— 负控 C 要**直接调用**它才算证明，M211 教训）
#   $1=日志 $2=wall秒 $3=期望复用数（⏭）
chk_reuse() {
    local log="$1" el="$2" want="$3" nb nr
    if [ "${M221_IGNORE_REUSE:-0}" = "1" ]; then
        return 0    # ⚠️ 判据自伤档（仅供负控 C；默认关闭）
    fi
    # ⚠️ 不能写 `|| echo 0`：`grep -c` 无命中时**已经**打印 "0"（rc=1）⇒ 再 echo 一次
    #    得 "0\n0" ⇒ `[ "$nb" = "0" ]` 恒假 ⇒ 判据**恒红**（本轮实测踩中）。
    nb=$(grep -c '^✅' "$log" 2>/dev/null)
    nr=$(grep -c '^⏭' "$log" 2>/dev/null)
    [ "$nb" = "0" ] && [ "$nr" = "$want" ] && [ "$el" -le 10 ]
}

snap_all
trap 'restore_all; rm -rf "$W"' EXIT

echo "══ M221 门：devbuild 产物指纹短路 ══"

# ───────────────────────── ① 静态 ─────────────────────────
hdr "[1/7] 静态：短路机制在位 + 两条关键不变量"
src=$(cat "$D")
check_grep() {  # $1=说明 $2=模式
    # ⚠️ `-e` 必须有：模式可能以 `-` 开头（如 --rebuild 那条），否则 grep 把它当
    #    **选项** ⇒ `unrecognized option` ⇒ 判据假红（本轮实测踩中）。
    if grep -qF -e "$2" "$D"; then note "✅ $1"; else bad "$1（未命中：$2）"; fi
}
check_grep "「--rebuild」 选项在位"            '--rebuild) FORCE_REBUILD=1 ;;'
check_grep "指纹变量 DEVB_KEY 在位"          'DEVB_KEY="$(src_line | sha256sum'
check_grep "指纹含 selfhost/*.px"            'sha256sum $SRC_PATTERNS'
check_grep "产物指纹文件 fp 在位"            'fp="/tmp/devbuild_${name}.fp"'
check_grep "复用标记 ⏭ 在位"                 '⏭  $name：源码链未变'
check_grep "VM 轨指纹含 pxcdev 的 sha"       'VM_KEY="$DEVB_KEY/$(sha256sum /tmp/pxcdev'

# 不变式 A：runtime/*.c 与 runtime/*.h 必须在**源码链**里
#   ⚠️ M223：判据载体从「src_line 里的 for-glob」改为 **SRC_PATTERNS 变量**
#   —— 指纹由 mtime 三元组改成内容哈希，glob 从函数体提到了模块级；
#   语义不变（负控篡改 runtime/*.c,h 时缓存必须失效），但锚点必须跟着改。
if printf '%s' "$src" | grep -q 'SRC_PATTERNS="selfhost/\*\.px runtime/\*\.c runtime/\*\.h tools/\*\.px stdlib/\*\.px"'; then
    note "✅ 源码链含 runtime/*.c 与 runtime/*.h（负控篡改它们时缓存必然失效）"
else
    bad "源码链**未**包含 runtime/*.c,h ⇒ 门内负控的重建会被缓存吃掉（假绿）"
fi

# 不变式 B：写指纹必须在**构建成功之后**（失败也写 ⇒ 下次复用坏件）
ln_link=$(grep -n 'gcc -static -O2 -pthread -o "$out"' "$D" | head -1 | cut -d: -f1)
ln_fp=$(grep -n 'echo "$DEVB_KEY" > "$fp"' "$D" | head -1 | cut -d: -f1)
if [ -n "$ln_link" ] && [ -n "$ln_fp" ] && [ "$ln_fp" -gt "$ln_link" ]; then
    note "✅ 写指纹（行 $ln_fp）在链接成功（行 $ln_link）**之后** ⇒ 失败不会污染缓存"
else
    bad "写指纹不在构建成功之后（link=$ln_link fp=$ln_fp）⇒ 坏件可能被缓存"
fi

# ───────────────────────── ② 动态：复用 ─────────────────────────
hdr "[2/7] 同源码链连跑两次 ⇒ 第二次必须全 ⏭"
# ⚠️ 先清指纹（冷启动）：开发机上 /tmp/devbuild_*.fp 往往已在（上一次 run 留下）
#    ⇒ 第一次就全 ⏭、第二次也全 ⏭ ⇒ 「第二次必须全复用」这条判据什么都没证明。
rm -f /tmp/devbuild_pxc.fp /tmp/devbuild_vm.fp
timeout 900 bash "$D" pxc --vm > "$W/r1.log" 2>&1; rc1=$?
t0=$SECONDS; timeout 900 bash "$D" pxc --vm > "$W/r2.log" 2>&1; rc2=$?; e2=$((SECONDS-t0))
note "第一次 rc=$rc1（建 $(grep -c '^✅' "$W/r1.log") 件 / 复用 $(grep -c '^⏭' "$W/r1.log") 件）"
note "第二次 rc=$rc2 wall=${e2}s（建 $(grep -c '^✅' "$W/r2.log") 件 / 复用 $(grep -c '^⏭' "$W/r2.log") 件）"
if [ "$rc2" = "0" ] && chk_reuse "$W/r2.log" "$e2" 2; then
    note "✅ 第二次**全复用**（pxc + VM 轨），wall=${e2}s —— 这就是 23 门 × 45s 里边省下来的部分"
else
    bad "第二次未全复用（wall=${e2}s）"; sed 's/^/     | /' "$W/r2.log" | head -8
fi

# ───────────────────────── ③ 敏感性 ─────────────────────────
hdr "[3/7] 敏感性：改 runtime/*.c ⇒ 指纹必变 +（单槽层）必重建；还原后回到**干净基线产物**（多槽层：0 重建）"
K0=$(devkey "$W/r2.log")
# ── M287：留一份**探针之前**的干净产物 —— 还原后的判据要比「是否重建」更本质：
#   真正要保证的是「产物**没有被探针污染**」，而不是「必须重建一次」。
cp -a /tmp/pxcdev "$W/pxcdev.clean" 2>/dev/null || true
printf '\n/* M221-GATE-PROBE（本门自证用，随即还原）*/\n' >> "$RC"
# ⚠️ M287：这一段测的是**单槽指纹短路**层 ⇒ 显式关掉外层（多槽）。否则：
#   探针的 patched key 在**上一轮**已经建过 ⇒ 多槽命中（建 0）⇒ 把「短路对 runtime 敏感」
#   误判成缺陷（实测：未关时 建0/复用1 ⇒ 本判据红，而产物其实**正确地**对应 patched 源码）。
DEVB_SLOTS=0 timeout 900 bash "$D" pxc > "$W/s1.log" 2>&1
K1=$(devkey "$W/s1.log"); nb1=$(grep -c '^✅' "$W/s1.log"); nr1=$(grep -c '^⏭' "$W/s1.log")
note "改 runtime.c（关多槽）：key $K0 → $K1（建 $nb1 / 复用 $nr1）"
if [ "$K1" != "$K0" ] && [ "$nb1" = "1" ] && [ "$nr1" = "0" ]; then
    note "✅ 指纹随 runtime/*.c 变化且强制重建（**单槽层**不会把负控吃掉）—— 多槽层由 m287 门覆盖"
else
    bad "改 runtime.c 后（关多槽）未重建（key $K0→$K1, 建$nb1/复用$nr1）⇒ 指纹不跟踪 runtime"
fi
cp -a "$SNAP/runtime.c.snap" "$RC"          # 还原
cmp -s "$SNAP/runtime.c.snap" "$RC" || bad "runtime.c 还原后与原文件不一致"
timeout 900 bash "$D" pxc > "$W/s2.log" 2>&1
K2=$(devkey "$W/s2.log"); nb2=$(grep -c '^✅' "$W/s2.log")
note "还原 runtime.c：key $K1 → $K2（建 $nb2 / 复用 $(grep -c '^⏭' "$W/s2.log")）"
# ── M287 语义更新（**期望值移位**，不是删掉判据）────────────────────────────
#   单槽时代：还原后基线的产物已被探针那次覆盖 ⇒ **只剩「重建」一条路**。
#   多槽时代：基线的干净产物在**独立槽**里 ⇒ 命中即得，**0 重建**。
#   ⇒ 旧判据（「必须重建」）在新语义下**恰好把收益判成缺陷**。
#   新判据抓的是**更本质的性质**：还原后拿到的产物必须**逐字节等于探针前的干净产物**，
#   且**不为此付一次重建**（两条都要 —— 只留 cmp 则「重建出的干净产物」也能过，
#   判据就没牙了；实测 `建$nb2` + `cmp` 双条件在撤掉多槽层时必红）。
if [ "$K2" = "$K0" ] && [ "$nb2" = "0" ] && cmp -s /tmp/pxcdev "$W/pxcdev.clean"; then
    note "✅ 还原后：key 回基线 · **0 重建** · 产物与探针前**逐字节一致** ⇒ 既没污染也没白付重建"
else
    if cmp -s /tmp/pxcdev "$W/pxcdev.clean"; then _cmp=同; else _cmp=异; fi
    bad "还原后不是「0 重建 + 干净产物」（key $K1→$K2 · 建$nb2 · cmp $_cmp）⇒ 可能复用了被探针污染的产物 / 或多了不必要的重建"
fi
# ── M287 反向判据：显式关掉多槽 ⇒ 必须回到「真建」（旧口径**保留**，只是不再是唯一路径）──
rm -f /tmp/devbuild_pxc.fp
DEVB_SLOTS=0 timeout 900 bash "$D" pxc > "$W/s2b.log" 2>&1
nb2b=$(grep -c '^✅' "$W/s2b.log")
if [ "$nb2b" = "1" ]; then
    note "✅ 反向判据：DEVB_SLOTS=0（关多槽）+ 清指纹 ⇒ 还原后**必须真建**（M221 原口径保留）"
else
    bad "DEVB_SLOTS=0 时还原后未真建（建$nb2b）⇒ 旧口径被破坏"
fi

# ───────────────────────── ④ --rebuild ─────────────────────────
hdr "[4/7] 「--rebuild」 ⇒ 无条件重建（负控/自证的口子）"
timeout 900 bash "$D" pxc --rebuild > "$W/rb.log" 2>&1; rcrb=$?
nbrb=$(grep -c '^✅' "$W/rb.log"); nrrb=$(grep -c '^⏭' "$W/rb.log")
if [ "$rcrb" = "0" ] && [ "$nbrb" = "1" ] && [ "$nrrb" = "0" ] && grep -q '（--rebuild：忽略缓存）' "$W/rb.log"; then
    note "✅ --rebuild 强制重建（建 $nbrb / 复用 $nrrb，且打印了忽略缓存标记）"
else
    bad "--rebuild 未强制重建（rc=$rcrb, 建$nbrb/复用$nrrb）"
fi

# ───────────────────────── ⑤ 生态自查 ─────────────────────────
hdr "[5/7] 生态自查：用 devbuild 的门数下限 + CI 全带 --neg-skip"
nd=$(grep -l 'devbuild' "$ROOT"/examples/*/verify.sh 2>/dev/null | wc -l)
if [ "$nd" -ge 20 ]; then note "✅ 用 devbuild 的门 = $nd（≥20，与本轮实测 23 一致）"
else bad "用 devbuild 的门只有 $nd（<20）⇒ 与「23 门 × 45s」的前提不符，请复核"; fi

# CI：这些门的调用必须带 --neg-skip（否则 CI 上会跑负控⇒改源码⇒缓存不稳定）
bad_ci=""; nskip=0
for f in $(grep -l 'devbuild' "$ROOT"/examples/*/verify.sh 2>/dev/null); do
    m=$(basename "$(dirname "$f")")
    if grep -q "$m/verify.sh" "$CI" 2>/dev/null; then
        if grep -q "$m/verify.sh --neg-skip" "$CI"; then nskip=$((nskip+1)); else bad_ci="$bad_ci $m"; fi
    fi
done
if [ -z "$bad_ci" ] && [ "$nskip" -ge 20 ]; then
    note "✅ CI 里 $nskip 个 devbuild 门的调用**全部**带 --neg-skip（缓存稳定的前提成立）"
else
    bad "CI 里有 devbuild 门未带 --neg-skip：$bad_ci（会现场改源码 ⇒ 缓存反复失效）"
fi

# ───────────────────────── ⑥ 负控 ─────────────────────────
if [ "$NEG_SKIP" = "0" ]; then
hdr "[6/7] 负控 A：单槽短路判据恒假（退回无条件构建）⇒ ② 的判据必须红"
restore_all; snap_all
python3 - "$D" <<'PY' || bad "负控 A 打桩失败"
import sys
p = sys.argv[1]; s = open(p, encoding="utf-8").read()
old = '    if [ "$FORCE_REBUILD" = 0 ] && [ -f "$out" ] && [ -f "$fp" ]'
assert s.count(old) == 1, "锚点不唯一"
open(p, "w", encoding="utf-8").write(s.replace(old, '    if false && [ -f "$out" ] && [ -f "$fp" ]', 1))
PY
# ⚠️ M287：负控**被测层 = 单槽指纹短路**；外层（多槽缓存）必须显式关闭，否则
#   「短路被禁」也仍会被多槽接住（实测：首版未关 ⇒ 负控 A 假红、判据无牙）。
export DEVB_SLOTS=0
timeout 900 bash "$D" pxc > "$W/na.log" 2>&1; rcna=$?
t0=$SECONDS; timeout 900 bash "$D" pxc > "$W/na2.log" 2>&1; ena=$((SECONDS-t0))
unset DEVB_SLOTS
note "负控 A 下第二次：rc=$rcna wall=${ena}s（建 $(grep -c '^✅' "$W/na2.log") / 复用 $(grep -c '^⏭' "$W/na2.log")）"
if chk_reuse "$W/na2.log" "$ena" 1; then
    bad "负控 A 未判红（短路被禁后仍全复用 ⇒ ② 的判据无牙）"
else
    note "✅ 负控 A 判红（短路被禁 ⇒ 每次都重建）"
fi
restore_all; cmp -s "$SNAP/devbuild.sh.snap" "$D" || bad "负控 A 还原后 devbuild.sh 不一致"

hdr "[6/7] 负控 B：让指纹**完全不跟踪 runtime** ⇒ ③ 的判据必须红"
# ⚠️ 负控 B 的设计要点（首版**没牙**，本轮实测抓出）：
#   ⚠️ M223：载体由 for-glob 改为 SRC_PATTERNS 变量 —— 锚点更稳（不会再因重写
#   函数体而失效），语义一字未变。
#   首版只把 `runtime/*.c` 从 glob 删掉 ⇒ 判据**不红**。因为指纹还有另一路
#   `rtcache=$(basename "$CACHE")`，而该目录名 = `tools/px` 的 `rt_key`
#   = **runtime 源内容哈希**（rt_src_files：.c 与 .h 都在内）⇒ runtime.c 一变，
#   目录名就变、指纹照样变。⇒ 要复现「指纹不跟踪 runtime」这个形状，**两路必须一起中和**。
#   ⇒ 顺带说明：显式 glob 是**双保险**（rtcache 名本就覆盖 runtime），两路都在 = 安全。
restore_all; snap_all
python3 - "$D" <<'PY' || bad "负控 B 打桩失败"
import sys
p = sys.argv[1]; s = open(p, encoding="utf-8").read()
old = 'SRC_PATTERNS="selfhost/*.px runtime/*.c runtime/*.h tools/*.px stdlib/*.px"'
assert s.count(old) == 1, "锚点不唯一（SRC_PATTERNS）"
s = s.replace(old, 'SRC_PATTERNS="selfhost/*.px tools/*.px stdlib/*.px"', 1)
old2 = '    echo "rtcache=$(basename \"$CACHE\")"'
assert s.count(old2) == 1, "锚点不唯一（rtcache 名）"
s = s.replace(old2, '    echo "rtcache=<frozen-by-NEGCTL-B>"', 1)
open(p, "w", encoding="utf-8").write(s)
PY
timeout 900 bash "$D" pxc > "$W/nb0.log" 2>&1           # 建立基准（新指纹）
KB0=$(devkey "$W/nb0.log")
printf '\n/* M221-GATE-PROBE-B（自证用，随即还原）*/\n' >> "$RC"
timeout 900 bash "$D" pxc > "$W/nb1.log" 2>&1
KB1=$(devkey "$W/nb1.log"); nbb=$(grep -c '^✅' "$W/nb1.log")
note "负控 B 下改 runtime.c：key $KB0 → $KB1（建 $nbb）"
if [ "$KB1" = "$KB0" ] && [ "$nbb" = "0" ]; then
    note "✅ 负控 B 判红（指纹不含 runtime.c ⇒ 改了也命中缓存 = 假绿形状复现）"
else
    bad "负控 B 未判红（key $KB0→$KB1, 建$nbb ⇒ 指纹仍在跟踪 runtime.c？）"
fi
cp -a "$SNAP/runtime.c.snap" "$RC"
restore_all; cmp -s "$SNAP/devbuild.sh.snap" "$D" || bad "负控 B 还原后 devbuild.sh 不一致"

hdr "[6/7] 负控 C：判据自伤（② 的判据无视 ⏭ 数）⇒ A 的红必须**消失**"
restore_all; snap_all
python3 - "$D" <<'PY' || bad "负控 C 打桩失败"
import sys
p = sys.argv[1]; s = open(p, encoding="utf-8").read()
old = '    if [ "$FORCE_REBUILD" = 0 ] && [ -f "$out" ] && [ -f "$fp" ]'
assert s.count(old) == 1, "锚点不唯一"
open(p, "w", encoding="utf-8").write(s.replace(old, '    if false && [ -f "$out" ] && [ -f "$fp" ]', 1))
PY
timeout 900 bash "$D" pxc > "$W/nc1.log" 2>&1
t0=$SECONDS; timeout 900 bash "$D" pxc > "$W/nc2.log" 2>&1; enc=$((SECONDS-t0))
if M221_IGNORE_REUSE=1 chk_reuse "$W/nc2.log" "$enc" 1; then
    note "✅ 负控 C（自伤档）下同一条故障**不再判红** ⇒ 负控 A 的红确来自比对"
else
    bad "负控 C 未生效（自伤档仍判红 ⇒ A 的红另有来源，判据与被测对象没对齐）"
fi
restore_all; cmp -s "$SNAP/devbuild.sh.snap" "$D" || bad "负控 C 还原后 devbuild.sh 不一致"
else
    note "（--neg-skip：跳过负控 A/B/C）"
fi

# ───────────────────────── ⑦ 覆盖边界 ─────────────────────────
hdr "[7/7] 覆盖边界登记（如实）"
note "① 短路只覆盖 「devbuild.sh」 的**产物级**复用；门内**非 devbuild** 的重复成本（如各自"
note "   重编 runtime 对象、各自跑 .rtcache 生成）不在本轮范围内。"
note "② 【M223 已收口】修前指纹用 「stat -c '%n %s %Y'」（**含 mtime**）⇒ 内容不变、只是 mtime 变"
note "   （「touch」/「cp」/解包/编辑器重写）就换 key ⇒ 白付一轮冷重建。M223 改为**源码内容哈希**"
note "   （三方资产用 名字+大小，去 mtime），并给「key 为什么变」加了**归因**（diff 逐文件清单）。"
note "   ⇒ 本条的正面判据见 examples/m223_devbuild_fingerprint/。"
note "③ CI 上 「.rtcache」 冷 ⇒ **第一次** devbuild 仍要付全额（这是不可省的），"
note "   本轮省的是**第 2..23 次**；CI 实测收益待发布后从 step8 时长对比。"
note "④ 本门自带 1 次 「pxc --vm」 全量构建（冷缓存时约 100s）＋若干次 「pxc」。"
note "   ⚠️ M223 前：门内改 runtime.c 再还原后 **mtime 已变** ⇒ 后续门还得**再重建一次**；"
note "   M223 后：指纹看**内容** ⇒ 还原即回基线 key ⇒ 后续门直接复用（这条副作用已收口）。"

echo
if [ "$fail" = "0" ]; then
    echo "M221-VERIFY-OK（全部门通过）"
else
    echo "M221-VERIFY-FAIL"
fi
exit "$fail"
