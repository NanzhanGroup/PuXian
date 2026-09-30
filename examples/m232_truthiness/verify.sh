#!/bin/bash
# M232 门 · **「真值性表 + 短路族」全量对拍**（第 110 轮 · 缺陷 347）
#
# 主题：M231 把 `(运算符 × 左类型 × 右类型)` 矩阵做完时，**显式排除**了短路族
#   （`and` / `or` / `??` / `|>`）—— 理由写在 m231 的 gen_probes.py 头注：
#   「and/or 对任意类型做真值性转换，矩阵判据对这些运算符退化为恒 VAL ⇒ 无判别力」。
#   本轮换一个判据来量同一个面：**真值性本身**。
#   `if` / `while` / `not` / `and` / `or` / `bool()` / `??` 全都建在同一个原语上
#   ——「这个值算真还是算假」。它是**整个语言里最公共的判据**（一处错，处处错），
#   而它从来没有被清单级度量过。
#
# 缺陷 347：真值性表在**三个实现之间有两处分叉**：
#   · `bytes("")`  解释轨 **TRUTHY** ⇄ 编译两轨 **FALSY**
#   · `()`         解释轨 **FALSY**  ⇄ 编译两轨 **TRUTHY**
#   根因 = 两份真值性实现**各漏一个分支**（「一条公共路径漏一个分支」的第二次，
#   M231 缺陷 346 是同族第一次）：
#   · `selfhost/ival.px` 的 `i_truthy` 缺 `bytes` ⇒ 落到兜底 `return true`
#   · `runtime/runtime.c` 的 `px_is_truthy` 缺 `PX_TUPLE` ⇒ 落到 `default: return true`
#
# 判据
#   [1] 静态：两处修复在位 · **源码派生**（两份实现的「专门分支集」必须逐个相等 + 探针全覆盖）
#             · 生成幂等 · 负控锚点自证（含反向）· 规模锚点
#   [2] 生成探针（46 例）+ 构建两轨
#   [3] **三轨对拍**（响亮性 / 值 / 错误）+ **逐轨**与 MODEL.tsv **双向**核对
#   [4] **缺陷 347 精确形状**独立判据（不依赖「三轨一致」）：
#       `bytes("")` / `()` 的真值性必须为 FALSY，且 4 条派生的短路后果必须一致
#   [5] 负控 A（忠实撤回 `runtime.c` 的 `PX_TUPLE`）⇒ [1]/[3] 必须红
#   [6] 负控 B（**构造「三轨一致地错」**：两处实现都让 bytes/tuple 恒真）
#       ⇒ [3] 的「三轨对拍」**变绿**、只有「逐轨 MODEL」与 [4] 能独立判红
#   [7] 负控 C（**判据自伤**：关掉 MODEL 核对）⇒ [6] 的红**必须消失**
#   [8] 源逐字节还原 · [9] 覆盖边界
#
# CI 用 `--neg-skip`（负控各要重编解释轨 / 两轨驱动 ≈ 2–3 min）。
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m232_truthiness
W=/tmp/m232_gate
rm -rf "$W"; mkdir -p "$W" "$W/build"
export M232_NEG_W="$W/snap"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
PXI="${M232_PXI:-$ROOT/bootstrap/pxi}"
NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" "$@"; }
has() { grep -qF "$2" "$ROOT/$1"; }
cnt() { grep -cF "$2" "$ROOT/$1" || true; }
TT() { python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --drv "$W/drv.px" "$@"; }

echo "── [1] 静态判据与源码派生"
has runtime/runtime.c 'case PX_TUPLE: return v.as.obj->as.tuple.len > 0;' \
  && chk "runtime.c 的 PX_TUPLE 分支在位" true || chk "runtime.c 的 PX_TUPLE 分支在位" false
has selfhost/ival.px 'if t == "bytes":' \
  && chk "ival.px 的 bytes 分支在位" true || chk "ival.px 的 bytes 分支在位" false
AUDIT_OUT=$(python3 "$D/gen_probes.py" --root "$ROOT" --audit 2>&1); AUDIT_RC=$?
echo "$AUDIT_OUT" | sed 's/^/     /'
chk "源码派生：两份实现「专门分支集」逐个相等 + 探针全覆盖" "[ $AUDIT_RC -eq 0 ]"

cp -f "$D/drv.px" "$W/gen1.px" 2>/dev/null || true
python3 "$D/gen_probes.py" --root "$ROOT" --gen >/dev/null 2>&1
cp -f "$D/drv.px" "$W/gen_again.px"
python3 "$D/gen_probes.py" --root "$ROOT" --gen >/dev/null 2>&1
cmp -s "$W/gen_again.px" "$D/drv.px" && chk "生成幂等（两次 --gen 逐字节一致）" true \
  || chk "生成幂等（两次 --gen 逐字节一致）" false
NANCH=$(python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" anchors 2>&1)
echo "$NANCH" | sed 's/^/     /'
echo "$NANCH" | grep -q 'M232-ANCHORS-OK' && chk "负控锚点自证（4 前向 ×1 + 1 反向 ×0）" true \
  || chk "负控锚点自证（4 前向 ×1 + 1 反向 ×0）" false
NC=$(awk -F'\t' 'NR>1' "$D/cases.tsv" | wc -l)
NT=$(awk -F'\t' '$2=="T"' "$D/MODEL.tsv" | wc -l)
NS=$(awk -F'\t' '$2=="S"' "$D/MODEL.tsv" | wc -l)
NP=$(awk -F'\t' '$2=="P"' "$D/MODEL.tsv" | wc -l)
chk "规模锚点（用例 ≥44）" "[ ${NC:-0} -ge 44 ]"
chk "规模锚点（真值性面 ≥20）" "[ ${NT:-0} -ge 20 ]"
chk "规模锚点（短路族面 ≥18）" "[ ${NS:-0} -ge 18 ]"
chk "规模锚点（管道面 ≥3）" "[ ${NP:-0} -ge 3 ]"

echo "── [2] 生成探针 + 构建两轨"
build_drivers() {
    rm -rf "$W/a_c" "$W/a_vm" "$W/build"; mkdir -p "$W/a_c" "$W/a_vm" "$W/build"
    cp -f "$D/drv.px" "$W/drv.px"
    cp -f "$W/drv.px" "$W/a_c/drv.px"; cp -f "$W/drv.px" "$W/a_vm/drv.px"
    ( cd "$W/a_c" && PX_BUILD_ENGINE=c timeout 2400 "$ROOT/tools/px" build drv.px ) >"$W/b_c.log" 2>&1 \
      || { tail -6 "$W/b_c.log"; return 1; }
    ( cd "$W/a_vm" && timeout 2400 "$ROOT/tools/px" build drv.px ) >"$W/b_vm.log" 2>&1 \
      || { tail -6 "$W/b_vm.log"; return 1; }
    cp -f "$W/a_c/build/drv" "$W/build/drv_c"
    cp -f "$W/a_vm/build/drv" "$W/build/drv_vm"
    [ -x "$W/build/drv_c" ] && [ -x "$W/build/drv_vm" ]
}
if build_drivers; then chk "两轨驱动器构建" true; else chk "两轨驱动器构建" false; fi

echo "── [3] 三轨对拍 + 逐轨 MODEL 双向"
M232_PXI="$PXI" TT > "$W/tt.log" 2>&1; TT_RC=$?
cat "$W/tt.log" | sed 's/^/     /'
chk "三轨对拍 + MODEL 双向（46 例 · 差异 0）" "[ $TT_RC -eq 0 ]"

echo "── [4] 缺陷 347 精确形状（独立层）"
M232_PXI="$PXI" TT --shape347 > "$W/shape.log" 2>&1; SH_RC=$?
grep -E '\[347\]' "$W/shape.log" | sed 's/^/     /'
chk "形状层：bytes(\"\") / () 必须 FALSY + 4 条短路后果一致" "[ $SH_RC -eq 0 ]"

if [ "$NEG" = "1" ]; then
echo "── [5] 负控 A：忠实撤回 runtime.c 的 PX_TUPLE"
NEGCTL apply A > "$W/negA_apply.log" 2>&1
python3 "$D/gen_probes.py" --root "$ROOT" --audit > "$W/negA_audit.log" 2>&1; NEG_AUDIT_RC=$?
build_drivers >/dev/null 2>&1
M232_PXI="$PXI" TT > "$W/negA_tt.log" 2>&1; NEG_A_TT=$?
grep -E 'tuple_empty|and_etuple|or_etuple' "$W/negA_tt.log" | head -6 | sed 's/^/     /'
chk "负控 A：源码派生判据必须红（专门分支集不相等）" "[ $NEG_AUDIT_RC -ne 0 ]"
chk "负控 A：三轨对拍层必须红" "[ $NEG_A_TT -ne 0 ]"
NEGCTL restore >/dev/null 2>&1

echo "── [6] 负控 B：构造「三轨一致地错」（两处实现都恒真）"
NEGCTL apply B > "$W/negB_apply.log" 2>&1
bash selfhost/devbuild.sh pxi > "$W/negB_dev.log" 2>&1
build_drivers >/dev/null 2>&1
M232_PXI=/tmp/pxidev TT > "$W/negB_tt.log" 2>&1; NEG_B_TT=$?
M232_PXI=/tmp/pxidev TT --shape347 > "$W/negB_shape.log" 2>&1; NEG_B_SH=$?
grep -E '^例数|MODEL' "$W/negB_tt.log" | head -8 | sed 's/^/     /'
chk "负控 B：① 三轨对拍**变绿**（三轨一致地错 ⇒ 该层看不见）" \
    "grep -qE '三轨差异 0' '$W/negB_tt.log'"
chk "负控 B：② 逐轨 MODEL 层必须独立判红" \
    "grep -qE 'MODEL 不一致 [1-9]' '$W/negB_tt.log'"
chk "负控 B：④ 形状层必须独立判红" "[ $NEG_B_SH -ne 0 ]"

echo "── [7] 负控 C：判据自伤（关掉 MODEL 核对）⇒ B 的红必须消失"
M232_PXI=/tmp/pxidev TT --harm > "$W/negC_tt.log" 2>&1; NEG_C_RC=$?
grep -E '^例数' "$W/negC_tt.log" | sed 's/^/     /'
chk "负控 C：判据关掉后（B 的补丁仍在位）必须不再红" "[ $NEG_C_RC -eq 0 ]"

echo "── [8] 源逐字节还原"
NEGCTL restore >/dev/null 2>&1
NEGCTL check > "$W/restore.log" 2>&1
grep -E 'M232-NEG-RESTORED' "$W/restore.log" | sed 's/^/     /'
chk "负控后源码逐字节还原" "grep -q 'M232-NEG-RESTORED-OK' '$W/restore.log'"
python3 "$D/gen_probes.py" --root "$ROOT" --audit > "$W/final_audit.log" 2>&1 \
  && chk "还原后源码派生判据复绿" true || chk "还原后源码派生判据复绿" false
fi

echo "── [9] 覆盖边界（如实登记）"
cat <<'EOF'
     · `and` / `or` / `??` 的**优先级**（M220 已登记：`??` 全表最低）不在本门面内；
     · `chan` / `mutex` / `rwlock` / `generator` 的真值性**解释轨不可达**
       （`interp 不支持通道 / mutex（Mini 子集排除）`）⇒ 本门不覆盖；
       编译两轨对它们走 `default: return true`，与解释轨「未支持」不构成分叉，
       但**日后若解释轨补上支持，需同步回来扩面**（登记为覆盖缺口）。
     · 真值性在 **`while`** 门上的行为与 `if` 同源（同为 `px_is_truthy`），
       本门只在 `if` 上观察（避免重复）；`assert` / `?:` 不存在（M220 已清除幽灵运算符）。
     · `and` / `or` 的**返回值**在 M219 前的解释轨曾是布尔（已登记的历史缺陷 245 家族），
       本门锁定现口径（返回**操作数**），但**不回溯**更早版本。
EOF

echo
echo "══ 汇总：通过 $pass · 失败 $fail ══"
[ "$fail" -eq 0 ] && { echo "M232-VERIFY-OK"; exit 0; }
echo "M232-VERIFY-FAIL"; exit 1
