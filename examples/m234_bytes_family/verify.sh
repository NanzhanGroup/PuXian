#!/bin/bash
# M234 门 · **「`bytes` 族 × 逐类型实参」全量对拍**（第 112 轮）
#
# 主题：M230 的**覆盖边界**里如实登记过一条缺口 ——「**`bytes` 的 `b[i]` 未覆盖**」；
#   M233 把「任意值 ⇒ 其 `str()` 形态」这一侧的兜底统一了，但 **`bytes` 走的是另一条档**
#   （对象自身缓冲 —— 语义是「原始字节」而不是文本形态）。
#   ⇒ `bytes` **族**是唯一既「有专门的数据语义」又「曾明确登记为未覆盖」的面。
#
# 判据
#   [1] 静态与源码派生：`bdata` 的 `bytes` 数据档在位 · M233 统一入口在位 · 生成幂等 ·
#       负控锚点自证 · 规模锚点
#   [2] 生成探针 + 构建两轨
#   [3] 三轨对拍（响亮性 / 值 / R 码 + 词条）+ `MODEL.tsv` **双向**
#   [4] **确定性**（独立层）：同二进制 · **不同进程布局**两遍必须逐字节一致
#       （M233 建立的 UB 指纹判据）
#   [5] 负控 A（撤回 `px_type_name` 的**标记字典识别**）⇒ [3] 必须红
#       ⚠️ 必须**重编 dev 解释器**：解释轨用的是入库件 `bootstrap/pxi`，
#          只改 runtime.c 不会让它变 ⇒ 不重编则三轨仍一致（**假绿**）—— 实测踩过。
#   [6] 负控 B（判据自伤）⇒ A 的红必须消失
#   [7] 源逐字节还原 · [8] 覆盖边界
#
# CI 用 `--neg-skip`。
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m234_bytes_family
W=/tmp/m234_gate
rm -rf "$W"; mkdir -p "$W/build"
export M234_NEG_W="$W/snap"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
PXI="${M234_PXI:-$ROOT/bootstrap/pxi}"
DEVPXI="${M234_DEVPXI:-/tmp/pxidev}"
NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" "$@"; }
TT() { python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --drv "$W/drv.px" "$@"; }

echo "── [1] 静态与源码派生"
AUDIT_OUT=$(python3 "$D/gen_probes.py" --root "$ROOT" --audit 2>&1); AUDIT_RC=$?
echo "$AUDIT_OUT" | sed 's/^/     /'
chk "源码派生：bytes 数据档 + M233 统一入口在位（含反向判据）" "[ $AUDIT_RC -eq 0 ]"
python3 "$D/gen_probes.py" --root "$ROOT" --gen >/dev/null 2>&1
cp -f "$D/drv.px" "$W/gen_again.px"
python3 "$D/gen_probes.py" --root "$ROOT" --gen >/dev/null 2>&1
cmp -s "$W/gen_again.px" "$D/drv.px" && chk "生成幂等（两次 --gen 逐字节一致）" true \
  || chk "生成幂等（两次 --gen 逐字节一致）" false
NANCH=$(python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" anchors 2>&1)
echo "$NANCH" | sed 's/^/     /'
echo "$NANCH" | grep -q 'M234-ANCHORS-OK' && chk "负控锚点自证" true || chk "负控锚点自证" false
NC=$(awk -F'\t' 'NR>1' "$D/cases.tsv" | wc -l)
chk "规模锚点（用例 ≥200）" "[ ${NC:-0} -ge 200 ]"

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

echo "── [3] 三轨对拍 + MODEL 双向"
M234_PXI="$PXI" TT > "$W/tt.log" 2>&1; TT_RC=$?
grep -E '^例数' "$W/tt.log" | sed 's/^/     /'
grep -E '❌' "$W/tt.log" | head -8 | sed 's/^/     /'
chk "三轨一致 + MODEL 双向" "[ $TT_RC -eq 0 ]"

echo "── [4] 确定性（同二进制 · 不同进程布局两遍逐字节一致）"
M234_PXI="$PXI" TT --det > "$W/det.log" 2>&1; DET_RC=$?
grep -E '❌|M234-DET' "$W/det.log" | head -6 | sed 's/^/     /'
chk "确定性（UB 的指纹判据）" "[ $DET_RC -eq 0 ]"

if [ "$NEG" = "1" ]; then
echo "── [5] 负控 A：撤回 bdata 的 bytes 数据档"
NEGCTL apply A > "$W/negA_apply.log" 2>&1
bash selfhost/devbuild.sh pxi > "$W/negA_dev.log" 2>&1
[ "$DEVPXI" = "/tmp/pxidev" ] || cp -f /tmp/pxidev "$DEVPXI"
build_drivers >/dev/null 2>&1
M234_PXI="$DEVPXI" TT > "$W/negA_tt.log" 2>&1; NEG_A_TT=$?
grep -E '^例数' "$W/negA_tt.log" | sed 's/^/     /'
chk "负控 A：三轨对拍层必须红" "[ $NEG_A_TT -ne 0 ]"
NEGCTL restore >/dev/null 2>&1

echo "── [6] 负控 B：判据自伤（三层全关）"
NEGCTL apply A > "$W/negB_apply.log" 2>&1
build_drivers >/dev/null 2>&1
M234_PXI="$DEVPXI" TT --harm > "$W/negB_tt.log" 2>&1; NEG_B_RC=$?
grep -E '^例数' "$W/negB_tt.log" | sed 's/^/     /'
chk "负控 B：判据全关后（A 的补丁仍在位）必须不再红" "[ $NEG_B_RC -eq 0 ]"

echo "── [7] 源逐字节还原"
NEGCTL restore >/dev/null 2>&1
NEGCTL check > "$W/restore.log" 2>&1
grep -E 'M234-NEG-RESTORED' "$W/restore.log" | sed 's/^/     /'
chk "负控后源码逐字节还原" "grep -q 'M234-NEG-RESTORED-OK' '$W/restore.log'"
python3 "$D/gen_probes.py" --root "$ROOT" --audit > "$W/final_audit.log" 2>&1 \
  && chk "还原后源码派生判据复绿" true || chk "还原后源码派生判据复绿" false
fi

echo "── [8] 覆盖边界（如实登记）"
cat <<'EOF'
     · 二元接口取的是**逐位置**形态（另一位置放合法值），不是全叉积 —— 全叉积（13×13）
       在解释轨每例都要重解析驱动器（实测 3s/例）⇒ 1352 例需 68 分钟/轨，不可行。
     · `bytes_get` / `bytes_set` 的**下标**面（越界 / 负下标 / 非 int）已在 M189 覆盖，
       本门不重复；本门关心的是「**类型**」面。
     · `mmap` / `read` / `write` 等 fd 语义接口不在面内（需真实设备/文件）。
EOF

echo
echo "══ 汇总：通过 $pass · 失败 $fail ══"
[ "$fail" -eq 0 ] && { echo "M234-VERIFY-OK"; exit 0; }
echo "M234-VERIFY-FAIL"; exit 1
