#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M398 · 缺陷 761 收口：发射冻结门的**传递依赖指纹**（判据自证门）
# ---------------------------------------------------------------------------
# 缺陷 761 的现场（两天内两次）：
#   旧口径 emitc_freeze.sh 的 src 只对**入口文件**做 sha256。改 selfhost/value.px
#   （被 v01_value.px import）⇒ 入口 src 不变、--emit-c 产物变（PXOP_SRCLINE 平移）
#   ⇒ 门判【类别 B · 发射行为回归 · 必须查】。人工用对照实验才证明是预期变化。
#
# 新口径：dep = 入口 sha + 递归 import 闭包 sha + 未解析 import 字面量（selfhost/freeze_deps.py）
#   类别 A = dep 变 + 产物变（源码闭包已变，确认后 --freeze）
#   类别 B = dep 未变 + 产物变（**真回归嫌疑，门的主职**）
#
# 本门判据（每条都要能红）：
#   T1 幂等        同一入口连续两次 sig 完全一致
#   T2 传递性有牙  注入 selfhost/value.px ⇒ v01_value.px 的 src **不变**、dep **变**
#   T3 无依赖有牙  注入 s01_arith.px（无 import）⇒ 它自己的 dep **变**
#                  （防「无依赖 ⇒ 常量」的假绿：入口必须计入 dep）
#   T4 未解析 import 字面量计入：改一个解析不到的 import 文本 ⇒ dep 变
#   T5 【类别 B 必须红】用 wrapper 篡改指定入口的 --emit-c 输出（**源码一行不动**）
#                  ⇒ --check 必须 rc=1、报【类别 B】、且该文件不得出现在【类别 A】
#   T6 【类别 A 不误判】注入 value.px ⇒ --check 必须 rc=1、报【类别 A】、且【类别 B】为空
#   T7 前提自证    wrapper 篡改确实改变了输出（逐字节比对）；注入确实改变了文件 sha
#   T8 旧格式兼容  基准缺 dep 列时不得崩、不得假红，且必须提示升级
#   约束：全程不替换 bootstrap/pxc_vm 真二进制（用 EMITC_BIN 指向 wrapper）
#        所有注入用 trap 恢复，结束时校验工作树回到原状（本门只碰两个文件，均先备份）
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
SCRIPT="$ROOT/selfhost/emitc_freeze.sh"
DEPS="$ROOT/selfhost/freeze_deps.py"
REAL_BIN="$ROOT/bootstrap/pxc_vm"
TMP="${TMPDIR:-/tmp}/m398_freeze.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
VALUEPX="$ROOT/selfhost/value.px"
S01="$ROOT/selfhost/cases/s01_arith.px"
V01="$ROOT/selfhost/cases/v01_value.px"
BAK_VALUE="$TMP/value.px.bak"
BAK_S01="$TMP/s01.px.bak"
cp "$VALUEPX" "$BAK_VALUE"
cp "$S01" "$BAK_S01"
cleanup() {
    cp "$BAK_VALUE" "$VALUEPX" 2>/dev/null
    cp "$BAK_S01" "$S01" 2>/dev/null
    rm -rf "$TMP"
}
trap cleanup EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

SUB="selfhost/cases/v01_value.px selfhost/cases/s01_arith.px"
GOLD_SUB="$TMP/sub.sha"

sig_of() {  # sig_of <entry> [field]
    python3 "$DEPS" --mode sig "$1" | sed -E "s/.*${2:-dep}=([0-9a-f]+).*/\1/"
}
src_of() { sig_of "$1" src; }

echo "═══ T1 幂等 ═══"
A=$(sig_of selfhost/cases/v01_value.px dep)
B=$(sig_of selfhost/cases/v01_value.px dep)
[ "$A" = "$B" ] && [ -n "$A" ] && ok "两次 sig 一致（dep=$A）" || bad "sig 不可重复：$A vs $B"

echo "═══ T2 传递性有牙（改被 import 的 value.px）═══"
V_SRC0=$(src_of selfhost/cases/v01_value.px); V_DEP0=$(sig_of selfhost/cases/v01_value.px dep)
VPX0=$(sha256sum "$VALUEPX" | cut -c1-16)
python3 - "$VALUEPX" <<'PY'
import sys
p = sys.argv[1]
s = open(p).read()
i = s.index("def v_arith")
open(p, "w").write(s[:i] + "# M398 gate inject\n" + s[i:])
PY
VPX1=$(sha256sum "$VALUEPX" | cut -c1-16)
V_SRC1=$(src_of selfhost/cases/v01_value.px); V_DEP1=$(sig_of selfhost/cases/v01_value.px dep)
if [ "$VPX0" != "$VPX1" ]; then ok "前提自证：value.px 确实被改动（$VPX0 → $VPX1）"; else bad "前提自证失败：注入未生效"; fi
[ "$V_SRC0" = "$V_SRC1" ] && ok "入口 src 不变（$V_SRC0）" || bad "入口 src 变了，本判据测不到该缺陷"
[ "$V_DEP0" != "$V_DEP1" ] && ok "dep 变（$V_DEP0 → $V_DEP1）⇒ 旧口径的误报面已被覆盖" || bad "dep 未变 —— 传递依赖没被算进去"

echo "═══ T3 无依赖入口也要有牙（入口自身必须计入 dep）═══"
cp "$BAK_VALUE" "$VALUEPX"   # 先恢复，避免干扰
S_DEP0=$(sig_of selfhost/cases/s01_arith.px dep)
S_SRC0=$(src_of selfhost/cases/s01_arith.px)
printf '\n# M398 gate inject (entry only)\n' >> "$S01"
S_DEP1=$(sig_of selfhost/cases/s01_arith.px dep)
S_SRC1=$(src_of selfhost/cases/s01_arith.px)
[ "$S_SRC0" != "$S_SRC1" ] && ok "前提自证：无 import 入口确实被改动" || bad "前提自证失败"
[ "$S_DEP0" != "$S_DEP1" ] && ok "dep 变（$S_DEP0 → $S_DEP1）⇒ 无 import 入口不会退化成常量" || bad "无 import 入口的 dep 未变 ⇒ 会假绿"
cp "$BAK_S01" "$S01"

echo "═══ T4 未解析 import 的字面量计入指纹 ═══"
PROBE="$TMP/probe.px"
printf 'import "nope_a.px"\nfn main() { }\n' > "$PROBE"
D0=$(python3 "$DEPS" --mode sig "$PROBE" | sed -E 's/.*dep=([0-9a-f]+).*/\1/')
printf 'import "nope_b.px"\nfn main() { }\n' > "$PROBE"
D1=$(python3 "$DEPS" --mode sig "$PROBE" | sed -E 's/.*dep=([0-9a-f]+).*/\1/')
[ "$D0" != "$D1" ] && ok "改未解析 import 的字面量 ⇒ dep 变（$D0 → $D1）" || bad "未解析 import 未计入指纹"

echo "═══ T5 类别 B 必须红（源码一行不动、产物被篡改）═══"
# 5.1 先用未篡改的编译器给子集建临时基准
EMITC_FREEZE_FILES="$SUB" EMITC_FREEZE_GOLDEN="$GOLD_SUB" bash "$SCRIPT" --freeze >"$TMP/freeze.log" 2>&1 \
    && ok "子集基准已建（$(wc -l < "$GOLD_SUB") 行）" || { bad "子集 --freeze 失败：$(tail -3 "$TMP/freeze.log")"; }
# 5.2 造 wrapper（**不碰真二进制**）
WRAP="$TMP/pxc_wrap"
cat > "$WRAP" <<WRAPEOF
#!/usr/bin/env bash
# M398 门负控：只篡改 v01_value.px 的 --emit-c 输出，其余原样转发
if [ "\${1:-}" = "--emit-c" ] && [ "\${2:-}" = "selfhost/cases/v01_value.px" ]; then
    out=\$("$REAL_BIN" "\$@"); rc=\$?
    printf '%s\n/* m398 tamper */' "\$out"
    exit \$rc
fi
exec "$REAL_BIN" "\$@"
WRAPEOF
chmod +x "$WRAP"
# 5.3 前提自证：wrapper 的输出与真二进制不同
O1=$(printf '%s' "$("$REAL_BIN" --emit-c selfhost/cases/v01_value.px 2>/dev/null)" | sha256sum | cut -c1-16)
O2=$(printf '%s' "$("$WRAP" --emit-c selfhost/cases/v01_value.px 2>/dev/null)" | sha256sum | cut -c1-16)
O3=$(printf '%s' "$("$WRAP" --emit-c selfhost/cases/s01_arith.px 2>/dev/null)" | sha256sum | cut -c1-16)
O4=$(printf '%s' "$("$REAL_BIN" --emit-c selfhost/cases/s01_arith.px 2>/dev/null)" | sha256sum | cut -c1-16)
[ "$O1" != "$O2" ] && ok "前提自证：wrapper 确实篡改了 v01_value 的产物（$O1 → $O2）" || bad "前提自证失败：wrapper 未篡改"
[ "$O3" = "$O4" ] && ok "前提自证：wrapper 对其余入口透明（s01 产物不变）" || bad "wrapper 污染了非目标入口"
# 5.4 判据
EMITC_FREEZE_FILES="$SUB" EMITC_FREEZE_GOLDEN="$GOLD_SUB" EMITC_BIN="$WRAP" \
    bash "$SCRIPT" --check >"$TMP/checkB.log" 2>&1
RC=$?
[ "$RC" -eq 1 ] && ok "--check rc=1（按设计判红）" || bad "--check rc=$RC（应为 1）"
grep -q '类别 B' "$TMP/checkB.log" && ok "报出【类别 B · 发射行为回归】" || bad "未报类别 B：$(head -3 "$TMP/checkB.log")"
# ⚠️ 判据只认**完整标题**：脚本的提示文案里也有「类别 A/B 为空」这类字样，
#   用宽 grep 会被自己的提示骗成假红（CRITERIA ⑥：注释/提示里的字样也算代码）。
HDR_B='【类别 B · 发射行为回归】'
HDR_A='【类别 A · 源码闭包已变】'
awk -v hb="$HDR_B" -v ha="$HDR_A" 'index($0,hb){b=1;a=0} index($0,ha){a=1;b=0} b && /v01_value\.px/{found=1} END{exit !found}' "$TMP/checkB.log" \
    && ok "被篡改的文件落在类别 B（源码闭包未变）" || bad "被篡改的文件没有落在类别 B"
grep -qF "$HDR_A" "$TMP/checkB.log" && bad "类别 A 非空（源码没动，不该出现）" || ok "类别 A 为空（不误判）"
grep -q '归因' "$TMP/checkB.log" && ok "判红时输出了归因段" || bad "判红时没有归因段"

echo "═══ T6 类别 A 不误判（改被 import 的模块）═══"
python3 - "$VALUEPX" <<'PY'
import sys
p = sys.argv[1]
s = open(p).read()
i = s.index("def v_arith")
open(p, "w").write(s[:i] + "# M398 gate inject A\n" + s[i:])
PY
EMITC_FREEZE_FILES="$SUB" EMITC_FREEZE_GOLDEN="$GOLD_SUB" \
    bash "$SCRIPT" --check >"$TMP/checkA.log" 2>&1
RC=$?
cp "$BAK_VALUE" "$VALUEPX"
[ "$RC" -eq 1 ] && ok "--check rc=1" || bad "--check rc=$RC（应为 1）"
awk -v hb="$HDR_B" -v ha="$HDR_A" 'index($0,ha){a=1;b=0} index($0,hb){b=1;a=0} a && /v01_value\.px/{found=1} END{exit !found}' "$TMP/checkA.log" \
    && ok "改 value.px ⇒ v01_value 落在类别 A（这是缺陷 761 的收口点）" || bad "未落在类别 A：$(head -8 "$TMP/checkA.log")"
grep -qF "$HDR_B" "$TMP/checkA.log" && bad "类别 B 非空（依赖变了却被判回归 ⇒ 缺陷 761 未收口）" || ok "类别 B 为空（不再误报回归）"

echo "═══ T8 旧格式基准兼容（缺 dep 列不得崩、不得假红）═══"
OLD="$TMP/old.sha"
sed -E 's/ dep=[0-9a-f]+//' "$GOLD_SUB" > "$OLD"
EMITC_FREEZE_FILES="$SUB" EMITC_FREEZE_GOLDEN="$OLD" bash "$SCRIPT" --check >"$TMP/checkOLD.log" 2>&1
RC=$?
[ "$RC" -eq 0 ] && ok "旧格式基准：产物未变 ⇒ rc=0（不假红）" || bad "旧格式基准 rc=$RC（应为 0）：$(head -5 "$TMP/checkOLD.log")"
grep -q '缺 dep 列' "$TMP/checkOLD.log" && ok "提示升级（不静默降级）" || bad "未提示格式升级"

echo "═══ T9 收尾自证（工作树已恢复）═══"
[ "$(sha256sum "$VALUEPX" | cut -c1-16)" = "$VPX0" ] && ok "value.px 已还原" || bad "value.px 未还原"
[ "$(sha256sum "$S01" | cut -c1-16)" = "$(sha256sum "$BAK_S01" | cut -c1-16)" ] && ok "s01_arith.px 已还原" || bad "s01_arith.px 未还原"
[ -x "$REAL_BIN" ] && ok "bootstrap/pxc_vm 未被替换（本门只用 EMITC_BIN 覆盖）" || bad "真二进制丢失"

echo
echo "════════════════════════════════════"
echo "m398_freeze_transitive: pass=$pass fail=$fail"
[ "$fail" -eq 0 ] || exit 1
exit 0
