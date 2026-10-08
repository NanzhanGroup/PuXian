#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M287 门：devbuild **多槽产物缓存** —— 把「同一 key 每轮重编一次」变成跨轮复用
#
# 为什么存在（实测，不是推测）：
#   本机 2026-10-07 全量门 193 门 / 5848s 的分解：
#     · devbuild 重建 **152 次 × 14.7s = 54 分钟 = 全量门的 55%**
#     · 而这 152 次里 **148 次的 key 在之前就已经建过**（只有 3 个 key 是全新的）
#   根因**不是**「短路坏了」—— 短路本身有效（实测 14.7s → 0.18s，82×）：
#     重建 152 次 / 复用 17 次（复用率 10%），而 **76% 的调用处于「打桩态」**
#     （负控改了 `selfhost/*.px` / `runtime/*.c`）⇒ 每次打桩又还原 = **2 次重建**。
#   真根因是「**单槽覆盖写**」：产物路径只有一份 `/tmp/<name>dev` ⇒ 每换一次 key
#   就把上一份覆盖掉 ⇒ 下一轮回到同一个 key 时又要从零重编一遍
#   **哪怕上一轮刚建过一模一样的二进制**。
#
# 修法：`/tmp/devbuild_slots/<件>/<key>/` 按 key 分槽持久保存；命中即复制回契约路径。
#   · 命中判据与既有指纹短路**同一条**（key 逐字节相等）⇒ 正确性保证不变
#   · 命中后必须 `touch` 契约路径（既有门 m210 `neg_dev` 用 `-nt` 判「产物比源码新」）
#   · 命中打印 **`⏭`**（不是新符号）—— 全仓多处 `grep -c '^⏭'` 判复用，
#     换符号会让它们**静默变成 0**（本仓纪律：判据不许静默失效）
#   · 指纹新增 `devbuild.sh` 自身 + gcc 版本（**产出规则**变了就不能命中旧槽）
#
# 层：
#   [1] 静态 8 条（机制在位 · 命中**先于**单槽 · touch · ⏭ 兼容 · 回填在链接后 ·
#       剪枝在建之前 · 台账第 6 列 · 指纹纳入口径）
#   [2] 冷启动（私有槽）：首调必真建 + 槽生成（key / src 齐备）
#   [3] 跨调用命中（核心）：删契约产物 + 删指纹 ⇒ 命中槽 · **0 重建** · 逐字节一致
#   [4] A→B→A 回摆（**收益的精确形状**）：打桩 ⇒ 真建；还原 ⇒ **命中基线槽**（0 重建）
#   [5] 开关两档：`--rebuild` 无视槽 · `DEVB_SLOTS=0` 关闭多槽（剪枝在 [6] 末）
#   [6] 完整性：槽内 key == 目录名 · 命中产物 sha == 真建产物 sha
#   [7] 负控 3 道（各自独立判红 · 源逐字节还原）
#       NC-A 多槽命中分支恒假 ⇒ [3] 必红（退化为真建）
#       NC-B 命中时**不 touch** ⇒ 契约产物比源码旧（**m210 `-nt` 的精确形状**）必红
#       NC-C 判据自伤 ⇒ NC-A 的红**必须消失**
#   [8] 覆盖边界（如实登记）
#
# ⚠️ 纪律：提示文本用**单引号**（反引号在双引号里会触发命令替换）·
#    `local a="$1"` 与 `local b=…` 拆两行（bash5.1+set -u 会取到外层）· 不写死开发机路径
# ⚠️ 本门**所有实验用私有槽目录**（`$W/slots`）⇒ 不干扰共享的 `/tmp/devbuild_slots`
#    （否则本门会把后续门的槽剪掉/清掉，把收益变成成本）。
# ⚠️ `--neg-skip`：CI 用（三道负控各要一次重建，≈50s）
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }

# M288s1（2026-10-08 CI 实测）：**位置参数不能当 ROOT** —— CI 会传 `--neg-skip`，
#   而 `cd "--neg-skip"` 会让 cd 去解析选项 ⇒ `cd: --: invalid option` ⇒ 门在 CI 上
#   **0s 判红、且一个字都没输出**（真因完全读不出来）。
#   ⇒ ROOT 走环境变量（`ROOT=`），argv[1] 仅在**不是开关**时按旧约定接受。
ROOT="${ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
case "${1:-}" in
    ""|--*) ;;
    *) ROOT="$1" ;;
esac
cd "$ROOT" || exit 1
W="${M287_W:-/tmp/m287_gate}"
rm -rf "$W"; mkdir -p "$W"
D="selfhost/devbuild.sh"
S="$W/slots"                    # 私有槽目录
NEG_SKIP=0
for a in "$@"; do case "$a" in --neg-skip) NEG_SKIP=1 ;; esac; done

PASS=0; FAIL=0
ok()  { PASS=$((PASS+1)); printf '  ✅ %s\n' "$1"; }
bad() { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
hdr() { printf '\n══ %s ══\n' "$1"; }
note(){ printf '     %s\n' "$1"; }

# 每次调用都在**干净起点**（避免 m191 老病：备份表被前一轮清空）
snap_all() { rm -rf "$W/snap"; mkdir -p "$W/snap"; cp -a "$D" "$W/snap/devbuild.sh.snap"; }
restore_all() { [ -f "$W/snap/devbuild.sh.snap" ] && cp -a "$W/snap/devbuild.sh.snap" "$D"; }
trap 'restore_all; rm -rf "$S"' EXIT

# ── 判据：多槽层成立 = 「命中槽」且「0 重建」 ──
#   抽成函数的原因：NC-C（判据自伤）必须伤**同一条**判据，否则自伤证明不了什么。
hit_ok() {   # $1=命中槽数 $2=真建数
    [ "${M287_IGNORE_HIT:-0}" = '1' ] && return 0     # ← 自伤档：恒真
    [ "$1" = '1' ] && [ "$2" = '0' ]
}

# 私有槽 + 私有台账（台账不进共享文件，避免污染别人的统计）
run_dev() {   # $1=日志 $2...=参数
    local lg="$1"; shift
    DEVB_SLOTS="$S" DEVB_STATS="$W/stats.tsv" timeout 900 bash "$D" "$@" > "$lg" 2>&1
}
nb() { grep -c '^✅' "$1" 2>/dev/null || true; }     # 真建件数
nr() { grep -c '^⏭' "$1" 2>/dev/null || true; }     # 复用件数
hit() { grep -c '多槽命中' "$1" 2>/dev/null || true; }

printf '\n══ M287 门：devbuild 多槽产物缓存（跨轮复用）══\n'
echo "ROOT=$ROOT  W=$W  私有槽=$S"
snap_all

# ───────────────────────── [1] 静态 ─────────────────────────
hdr '[1/8] 静态：多槽机制在位（8 条）'
g() { grep -qE "$1" "$D"; }
g 'DEVB_SLOTS="\$\{DEVB_SLOTS:-/tmp/devbuild_slots\}"' && ok '多槽根目录变量在位（可覆盖）' || bad '缺 DEVB_SLOTS 变量'
g 'DEVB_SLOTS_MAX="\$\{DEVB_SLOTS_MAX:-[0-9]+\}"'      && ok '槽数上限变量在位（LRU 上界）' || bad '缺 DEVB_SLOTS_MAX'
g 'slot="\$DEVB_SLOTS/\$name/\$DEVB_KEY"'              && ok '槽路径 = 件名/key（按 key 分槽）' || bad '槽路径不是 件名/key'
# 命中分支必须**先于**单槽短路（否则恒被单槽吃掉，多槽永不生效）
n_slot=$(grep -n '多槽命中（key=' "$D" | head -1 | cut -d: -f1)
n_fp=$(grep -n '源码链未变（key=' "$D" | head -1 | cut -d: -f1)
if [ -n "$n_slot" ] && [ -n "$n_fp" ] && [ "$n_slot" -lt "$n_fp" ]; then
    ok "命中分支在单槽短路**之前**（行 $n_slot < $n_fp）—— 否则多槽永不生效"
else bad "命中分支未先于单槽短路（slot 行=$n_slot · fp 行=$n_fp）"; fi
g 'touch "\$out"; touch "\$slot"' && ok '命中后 touch 契约路径（m210 `-nt` 兼容）' || bad '命中后未 touch 契约路径'
g '多槽命中（key=\$DEVB_KEY）⇒ 复用' && ok '命中打印 `⏭`（既有门的 grep 不会静默变 0）' || bad '命中未打印 ⏭'
# 回填必须在链接成功之后（失败不许污染槽）
n_link=$(grep -n 'gcc -static -O2 -pthread -o "\$out"' "$D" | tail -1 | cut -d: -f1)
n_fill=$(grep -n '回填多槽' "$D" | head -1 | cut -d: -f1)
if [ -n "$n_link" ] && [ -n "$n_fill" ] && [ "$n_fill" -gt "$n_link" ]; then
    ok "回填在链接成功**之后**（$n_fill > $n_link）—— 失败不污染槽"
else bad "回填不在链接之后（link=$n_link · fill=$n_fill）"; fi
n_prune=$(grep -n '^slot_prune$' "$D" | head -1 | cut -d: -f1)
n_loop=$(grep -n 'for n in \$NAMES; do build_one' "$D" | head -1 | cut -d: -f1)
if [ -n "$n_prune" ] && [ -n "$n_loop" ] && [ "$n_prune" -lt "$n_loop" ]; then
    ok "剪枝在建之前调用（$n_prune < $n_loop）"
else bad "剪枝未在建之前（prune=$n_prune · loop=$n_loop）"; fi
g '命中层（M287）' && ok '台账/汇总含**命中层**（slot / fp / -，可归因）' || bad '缺命中层记录'
g 'devbuild=\$\(sha256sum "\$ROOT/selfhost/devbuild.sh"' && ok '指纹纳入 `devbuild.sh` 自身（产出规则变了不命中旧槽）' || bad '指纹未纳入 devbuild.sh'
g 'echo "gcc=\$\(gcc -dumpversion' && ok '指纹纳入 gcc 版本（换工具链不命中旧槽）' || bad '指纹未纳入 gcc 版本'

# ───────────────────────── [2] 冷启动 ─────────────────────────
hdr '[2/8] 冷启动（私有槽为空）⇒ 首调必真建 + 槽生成'
rm -f /tmp/pxcdev /tmp/devbuild_pxc.fp
t0=$SECONDS; run_dev "$W/c1.log" pxc; e1=$((SECONDS-t0))
chk '首调真建 1 件' "$(nb "$W/c1.log")" '1'
chk '首调无复用'   "$(nr "$W/c1.log")" '0'
K=$(sed -n 's/^── 源码链指纹：\([0-9a-f]*\).*/\1/p' "$W/c1.log" | head -1)
[ -n "$K" ] && ok "拿到基线 key（$K）" || bad '取不到 key'
if [ -d "$S/pxc/$K" ] && [ -f "$S/pxc/$K/key" ] && [ -f "$S/pxc/$K/pxc" ]; then
    ok '槽已生成（key 文件 + 产物齐备）'
    chk '槽内 key == 目录名' "$(cat "$S/pxc/$K/key")" "$K"
else bad "槽未生成或缺文件（$S/pxc/$K/{key,pxc}）"; fi
note "冷启动耗时 ${e1}s（真建基准）"
SHA_BUILD="$(sha256sum /tmp/pxcdev | cut -c1-64)"

# ───────────────────────── [3] 跨调用命中（核心）─────────────────────────
hdr '[3/8] 跨调用命中：删契约产物 + 删指纹 ⇒ 必须命中槽 · 0 重建 · 逐字节一致'
rm -f /tmp/pxcdev /tmp/devbuild_pxc.fp
t0=$SECONDS; run_dev "$W/h1.log" pxc; e2=$((SECONDS-t0))
chk '命中槽 1 件' "$(hit "$W/h1.log")" '1'
chk '0 重建'      "$(nb "$W/h1.log")"  '0'
chk '复用 1 件'   "$(nr "$W/h1.log")"  '1'
if hit_ok "$(hit "$W/h1.log")" "$(nb "$W/h1.log")"; then ok '★ 判据 hit_ok（命中槽 + 0 重建）成立'; else bad '判据 hit_ok 不成立'; fi
if [ "$e2" -lt 5 ]; then ok "命中耗时 ${e2}s < 5s（真建 ${e1}s ⇒ 加速 ≥$(( e1 / (e2+1) ))×）"; else bad "命中耗时 ${e2}s ≥ 5s（未走槽？）"; fi
chk '命中产物 sha == 真建产物 sha' "$(sha256sum /tmp/pxcdev | cut -c1-64)" "$SHA_BUILD"
[ -f /tmp/devbuild_pxc.fp ] && chk '命中后补写指纹' "$(cat /tmp/devbuild_pxc.fp)" "$K" || bad '命中后未补写指纹'

# ───────────────────────── [4] A→B→A 回摆 ─────────────────────────
hdr '[4/8] A→B→A 回摆（收益的精确形状）：打桩 ⇒ 真建；还原 ⇒ 命中**基线槽**'
cp -a selfhost/cg_stmt.px "$W/cg_stmt.ref"
printf '\n# M287-GATE-PROBE\n' >> selfhost/cg_stmt.px
t0=$SECONDS; run_dev "$W/p1.log" pxc; e3=$((SECONDS-t0))
KB=$(sed -n 's/^── 源码链指纹：\([0-9a-f]*\).*/\1/p' "$W/p1.log" | head -1)
chk '打桩 ⇒ 真建 1 件（key 首次出现，必建）' "$(nb "$W/p1.log")" '1'
[ "$KB" != "$K" ] && ok "打桩换 key（$K → $KB）" || bad '打桩未换 key'
cp -a "$W/cg_stmt.ref" selfhost/cg_stmt.px          # 还原
cmp -s "$W/cg_stmt.ref" selfhost/cg_stmt.px || bad 'cg_stmt.px 还原后不一致'
t0=$SECONDS; run_dev "$W/p2.log" pxc; e4=$((SECONDS-t0))
chk '还原 ⇒ **命中基线槽**' "$(hit "$W/p2.log")" '1'
chk '还原 ⇒ 0 重建（M287 收益）' "$(nb "$W/p2.log")" '0'
[ "$e4" -lt 5 ] && ok "回摆命中耗时 ${e4}s < 5s（单槽时代这里必付 ${e3}s）" || bad "回摆耗时 ${e4}s ≥ 5s"
chk '回摆后产物 == 基线产物' "$(sha256sum /tmp/pxcdev | cut -c1-64)" "$SHA_BUILD"
nslot=$(find "$S" -mindepth 2 -maxdepth 2 -type d 2>/dev/null | wc -l)
[ "$nslot" -ge 2 ] && ok "两个 key 各占一槽（现 $nslot 个）—— 互不覆盖" || bad "槽数 $nslot < 2（未分槽）"

# ───────────────────────── [5] 开关三档 ─────────────────────────
hdr '[5/8] 开关：--rebuild 无视槽 · DEVB_SLOTS=0 关闭多槽'
run_dev "$W/rb.log" pxc --rebuild
chk '--rebuild 真建 1 件' "$(nb "$W/rb.log")" '1'
chk '--rebuild 无命中'    "$(hit "$W/rb.log")" '0'
grep -q '（--rebuild：忽略缓存）' "$W/rb.log" && ok '--rebuild 打印忽略缓存标记' || bad '--rebuild 缺标记'
rm -f /tmp/pxcdev /tmp/devbuild_pxc.fp
DEVB_SLOTS=0 DEVB_STATS="$W/stats.tsv" timeout 900 bash "$D" pxc > "$W/off.log" 2>&1
chk 'DEVB_SLOTS=0 ⇒ 关多槽（0 命中）' "$(hit "$W/off.log")" '0'
chk 'DEVB_SLOTS=0 ⇒ 真建 1 件'        "$(nb "$W/off.log")"  '1'

# ───────────────────────── [6] 完整性 ─────────────────────────
hdr '[6/8] 完整性：槽内 key == 目录名 · 命中产物与真建产物逐字节一致'
badslot=0
while IFS= read -r d; do
    [ -n "$d" ] || continue
    b=$(basename "$d")
    kf="$d/key"
    if [ ! -f "$kf" ] || [ "$(cat "$kf")" != "$b" ]; then badslot=$((badslot+1)); fi
    # 槽名（件/key）里的 key 片段必须与 key 文件一致（vm 槽的 key 含 /，用 basename 到件名层）
done < <(find "$S/pxc" -mindepth 1 -maxdepth 1 -type d 2>/dev/null)
[ "$badslot" = '0' ] && ok 'pxc 槽：key 文件与目录名逐个一致' || bad "$badslot 个槽的 key 与目录名不一致"
if [ -d "$S/pxc/$K" ]; then
    chk '基线槽产物 sha == 真建产物 sha（槽没被写坏）' "$(sha256sum "$S/pxc/$K/pxc" | cut -c1-64)" "$SHA_BUILD"
    [ -f "$S/pxc/$K/src" ] && ok '槽内存有源码清单 src（供归因 diff 用）' || bad '槽内缺 src 清单'
fi
# 剪枝（**放最后** —— 它会把槽清掉，前面的完整性检查需要槽还在）
run_dev "$W/pr.log" pxc
DEVB_SLOTS_MAX=1 DEVB_SLOTS="$S" DEVB_STATS="$W/stats.tsv" timeout 900 bash "$D" pxc > "$W/pz.log" 2>&1
grep -q '多槽：.*> 上限' "$W/pz.log" && ok '剪枝触发（打印了 > 上限 与清理数）' || bad '剪枝未触发'
nslot2=$(find "$S" -mindepth 2 -maxdepth 2 -type d 2>/dev/null | wc -l)
[ "$nslot2" -lt "$nslot" ] && ok "剪枝后槽数下降（$nslot → $nslot2）" || bad "剪枝后槽数未降（$nslot2）" 

# ───────────────────────── [7] 负控 ─────────────────────────
if [ "$NEG_SKIP" = '1' ]; then
    hdr '[7/8] 负控：--neg-skip（跳过）'
else
hdr '[7/8] 负控 3 道（各自独立判红 · 源逐字节还原）'

# ── NC-A：多槽命中分支恒假 ⇒ [3] 必红（退化为真建）──
restore_all; snap_all
python3 - "$D" <<'PY' || bad 'NC-A 打桩失败'
import sys
p=sys.argv[1]; s=open(p,encoding="utf-8").read()
old='       && [ -f "$slot/$name" ] && [ "$(cat "$slot/key" 2>/dev/null)" = "$DEVB_KEY" ]; then'
assert s.count(old)==1, "锚点不唯一（%d）"%s.count(old)
open(p,"w",encoding="utf-8").write(s.replace(old,'       && [ -f "/nonexistent_m287" ] && [ "$(cat "$slot/key" 2>/dev/null)" = "$DEVB_KEY" ]; then',1))
PY
rm -rf "$S"; rm -f /tmp/pxcdev /tmp/devbuild_pxc.fp
run_dev "$W/na1.log" pxc >/dev/null; rm -f /tmp/pxcdev /tmp/devbuild_pxc.fp
run_dev "$W/na2.log" pxc
na_hit=$(hit "$W/na2.log"); na_b=$(nb "$W/na2.log")
note "NC-A 下「删产物+删指纹」再调：命中 $na_hit / 真建 $na_b"
if hit_ok "$na_hit" "$na_b"; then
    bad "NC-A 未判红（命中 $na_hit / 建 $na_b —— 多槽层被撤后判据仍成立？）"
else
    ok '★ NC-A 判红：撤掉多槽层后 **hit_ok（命中槽 + 0 重建）不再成立** ⇒ 退化为真建'
fi
restore_all
cmp -s "$W/snap/devbuild.sh.snap" "$D" || bad 'NC-A 还原后 devbuild.sh 不一致'

# ── NC-B：命中时**不 touch** ⇒ 契约产物比源码旧（m210 `-nt` 的精确形状）──
restore_all; snap_all
python3 - "$D" <<'PY' || bad 'NC-B 打桩失败'
import sys
p=sys.argv[1]; s=open(p,encoding="utf-8").read()
old='            touch "$out"; touch "$slot" 2>/dev/null || true'
assert s.count(old)==1, "锚点不唯一（%d）"%s.count(old)
open(p,"w",encoding="utf-8").write(s.replace(old,'            touch "$slot" 2>/dev/null || true',1))
PY
rm -rf "$S"; cp -a selfhost/parser.px "$W/parser.ref"
run_dev "$W/nb0.log" pxc >/dev/null          # 冷建（槽为**旧** mtime）
sleep 1; touch selfhost/parser.px            # 源码 mtime 变新（**内容不变** ⇒ key 不变，M223 口径）
rm -f /tmp/pxcdev /tmp/devbuild_pxc.fp       # 逼它走槽
run_dev "$W/nb1.log" pxc
nb_hit=$(hit "$W/nb1.log")
if [ "$nb_hit" = '1' ]; then
    if [ /tmp/pxcdev -nt selfhost/parser.px ]; then
        bad 'NC-B 未判红（不 touch 也仍比源码新 —— 该判据没牙）'
    else
        ok '★ NC-B 判红：命中后**不 touch** ⇒ 契约产物比源码旧 ⇒ 触发 m210 `-nt` 的失败形状'
    fi
else bad "NC-B 底座异常（未命中槽：命中 $nb_hit）"; fi
restore_all; cp -a "$W/parser.ref" selfhost/parser.px
cmp -s "$W/parser.ref" selfhost/parser.px || bad 'NC-B 还原后 parser.px 不一致'
cmp -s "$W/snap/devbuild.sh.snap" "$D" || bad 'NC-B 还原后 devbuild.sh 不一致'

# ── NC-C：判据自伤 ⇒ NC-A 的红**必须消失**（伤的必须是**同一条**判据）──
#   做法：同一故障（多槽层被撤）下，把 `hit_ok` 放宽为**恒真** ⇒ 断言「不再判红」。
#   若自伤后**仍**判红 ⇒ 说明 NC-A 的红另有来源，NC-A 的结论不成立。
restore_all; snap_all
python3 - "$D" <<'PY' || bad 'NC-C 打桩失败'
import sys
p=sys.argv[1]; s=open(p,encoding="utf-8").read()
old='       && [ -f "$slot/$name" ] && [ "$(cat "$slot/key" 2>/dev/null)" = "$DEVB_KEY" ]; then'
assert s.count(old)==1
open(p,"w",encoding="utf-8").write(s.replace(old,'       && [ -f "/nonexistent_m287" ] && [ "$(cat "$slot/key" 2>/dev/null)" = "$DEVB_KEY" ]; then',1))
PY
rm -rf "$S"; rm -f /tmp/pxcdev /tmp/devbuild_pxc.fp
run_dev "$W/nc1.log" pxc >/dev/null; rm -f /tmp/pxcdev /tmp/devbuild_pxc.fp
run_dev "$W/nc2.log" pxc
nc_hit=$(hit "$W/nc2.log"); nc_b=$(nb "$W/nc2.log")
note "NC-C 下：命中 $nc_hit / 真建 $nc_b"
if hit_ok "$nc_hit" "$nc_b"; then bad 'NC-C 底座异常（未自伤就判过）'; else ok '★ NC-C 底座：未自伤 ⇒ 同一故障判红'; fi
if M287_IGNORE_HIT=1 hit_ok "$nc_hit" "$nc_b"; then
    ok '★ NC-C 判红：自伤档下**同一故障不再判红** ⇒ NC-A 的红确来自 hit_ok 这条判据'
else bad 'NC-C 未判红（自伤后仍判红 ⇒ 红另有来源）'; fi
restore_all
cmp -s "$W/snap/devbuild.sh.snap" "$D" || bad 'NC-C 还原后 devbuild.sh 不一致'
fi

# ───────────────────────── [8] 覆盖边界 ─────────────────────────
hdr '[8/8] 覆盖边界（如实登记）'
cat <<'EOF'
      · 本门只判**产物级**多槽：命中判据 = key 逐字节相等（与既有指纹短路同一条）。
        槽内容**不做校验和自检**（信任「key = 全部输入的内容哈希」这条不变量）。
      · 槽是**本机加速**，不是缓存协议：跨机不共享、`/tmp` 被清即失效（可接受）。
      · 本门实验全部用**私有槽**（$W/slots），不触碰共享的 /tmp/devbuild_slots。
        共享槽的**默认路径**只由 [1] 静态判据覆盖。
      · `vm` 槽的 key 形如 `<pxc_key>/<pxcdev_sha>`（含 `/`）⇒ 目录层级为 `vm/<a>/<b>/`，
        本门的槽名一致性判据只覆盖 `pxc` 一族。
      · 收益是**稳态**量：冷启动（首个 key）仍付全额；实测稳态可省 ≈36 分钟/轮
        （见 CHANGELOG M287 的分解），本门不测**时长**收益，只测**机制**是否成立。
EOF

printf '\n══ 汇总：%d 通过 / %d 失败 ══\n' "$PASS" "$FAIL"
[ "$FAIL" = '0' ] && echo 'M287-VERIFY-OK（全部门通过）' || echo 'M287-VERIFY-FAIL'
exit $(( FAIL > 0 ? 1 : 0 ))
