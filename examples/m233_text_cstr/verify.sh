#!/bin/bash
# M233 门 · **「文本语义接口 × 逐类型实参」全量对拍**（第 111 轮 · 缺陷 348/349/350/351）
#
# 主题：`docs/ERROR_CODES.md` §6.5 立过一条**语义豁免** ——「参数本身就是一段**文本**」的接口
#   （哈希 / 编码 / 正则 / 字符处理）对任意实参**宽容**，语义是取「其 `str()` 形态」。
#   M194–M199 把「守卫」那一侧写成了判据，**但『其 str() 形态』这一侧从来没有被清单级度量过**
#   —— 这一侧恰恰是 `val_cstr` / `bdata` / `blen` 三处兜底渲染器的职责。
#
# 缺陷 348 / 349 / 350：三处兜底都写 `snprintf("%s", fmt_num(v))`，而 `fmt_num`
#   **只对 int/float 正确**（读 `v.as.i` / `v.as.f`）：
#     · `bool`：位模式重解释（`true` 的 `as.b=1` 读成 double 得 `5e-324`）⇒ `sha256(true)` 静默错值；
#     · 容器 / struct / enum / result / function / native：读的是 **`as.obj` 指针位**
#       ⇒ ① **非确定**（连跑 5 次 5 个结果）② **把堆地址位当文本吐出去**（`base64_encode([1])` 的
#          base64 里就是 ASLR 位）——实测 `sha256([1])` 三次三个值、`md5([1])` 同；
#     · `g_tmp_ring` 每槽 64 字节 ⇒ 长渲染**静默截断**。
#
# 缺陷 351（同批探针照出）：**解释轨的「标记字典」表示泄漏** —— 解释轨把 struct/enum/function/
#   native/type/generator 表示为带标记的 dict，宿主 native 的通用渲染把它们当**普通字典**
#   （`userfn` 会渲出**整个闭包**，含全部内置名）⇄ 编译两轨 `<fn userfn>` ⇒ 三轨分叉。
#
# 判据
#   [1] 静态与源码派生：统一入口在位 · **反向判据**（不许再有 `fmt_num` 兜底）·
#       解释轨规范化在位 · 生成幂等 · 负控锚点自证 · 规模锚点
#   [2] 生成探针（125 例）+ 构建两轨
#   [3] 三轨对拍（响亮性/值/词条）+ **对齐**（`f(x)` ⇄ `f(str(x))`，非 bytes 实参）+
#       `MODEL.tsv` **双向**
#   [4] **确定性**（独立层）：同一二进制整批连跑两遍，输出逐字节一致 —— 读 union 垃圾的
#       缺陷由此**必然**现身，是这类 UB 的**指纹判据**
#   [5] 负控 A（撤回 runtime 兜底）⇒ [3][4] 必须红
#   [6] 负控 B（只撤回解释轨规范化）⇒ [3] 的**对齐**必须红、而 [4] 仍绿（两组判据互相独立）
#   [7] 负控 C（判据自伤）⇒ B 的红必须消失
#   [8] 源逐字节还原 · [9] 覆盖边界
#
# CI 用 `--neg-skip`（负控各要重编解释轨 / 两轨驱动 ≈ 2–4 min）。
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m233_text_cstr
W=/tmp/m233_gate
rm -rf "$W"; mkdir -p "$W/build"
export M233_NEG_W="$W/snap"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
PXI="${M233_PXI:-$ROOT/bootstrap/pxi}"
# ⚠️ `devbuild.sh pxi` 的产物固定叫 `/tmp/pxidev` —— 两个工作树同时跑会互相覆盖
#    （M221/M223 的指纹短路口径同源）。故 dev 件路径**可覆盖**：`M233_DEVPXI`。
DEVPXI="${M233_DEVPXI:-/tmp/pxidev}"
NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" "$@"; }
has() { grep -qF "$2" "$ROOT/$1"; }
TT() { python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --drv "$W/drv.px" "$@"; }

echo "── [1] 静态与源码派生"
AUDIT_OUT=$(python3 "$D/gen_probes.py" --root "$ROOT" --audit 2>&1); AUDIT_RC=$?
echo "$AUDIT_OUT" | sed 's/^/     /'
chk "源码派生：兜底渲染器已统一到 px_cstr_any（含反向判据）" "[ $AUDIT_RC -eq 0 ]"
NCI=$(grep -c 'i_cstr_arg(args\[0\])' "$ROOT/selfhost/ibuiltin.px" || true)
chk "解释轨标记值规范化在位（5 处）" "[ ${NCI:-0} -eq 5 ]"
python3 "$D/gen_probes.py" --root "$ROOT" --gen >/dev/null 2>&1
cp -f "$D/drv.px" "$W/gen_again.px"
python3 "$D/gen_probes.py" --root "$ROOT" --gen >/dev/null 2>&1
cmp -s "$W/gen_again.px" "$D/drv.px" && chk "生成幂等（两次 --gen 逐字节一致）" true \
  || chk "生成幂等（两次 --gen 逐字节一致）" false
NANCH=$(python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" anchors 2>&1)
echo "$NANCH" | sed 's/^/     /'
echo "$NANCH" | grep -q 'M233-ANCHORS-OK' && chk "负控锚点自证（A 三处 ×1 · B ×5）" true \
  || chk "负控锚点自证（A 三处 ×1 · B ×5）" false
NC=$(awk -F'\t' 'NR>1' "$D/cases.tsv" | wc -l)
NA=$(awk -F'\t' '$6=="YES"' "$D/cases.tsv" | wc -l)
chk "规模锚点（用例 ≥120）" "[ ${NC:-0} -ge 120 ]"
chk "规模锚点（对齐判据 ≥110）" "[ ${NA:-0} -ge 110 ]"

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

echo "── [3] 三轨对拍 + 对齐（独立真值）+ MODEL 双向"
M233_PXI="$PXI" TT > "$W/tt.log" 2>&1; TT_RC=$?
grep -E '^例数' "$W/tt.log" | sed 's/^/     /'
grep -E '❌' "$W/tt.log" | head -6 | sed 's/^/     /'
chk "三轨一致 + 对齐（f(x) ⇄ f(str(x))）+ MODEL 双向（125 例 · 0 差异）" "[ $TT_RC -eq 0 ]"

echo "── [4] 确定性（同一二进制跑两遍，逐字节一致）"
M233_PXI="$PXI" TT --det > "$W/det.log" 2>&1; DET_RC=$?
grep -E '❌|M233-DET' "$W/det.log" | head -6 | sed 's/^/     /'
chk "确定性：整批连跑两遍逐字节一致（UB 的指纹判据）" "[ $DET_RC -eq 0 ]"

if [ "$NEG" = "1" ]; then
echo "── [5] 负控 A：忠实撤回 runtime 兜底（val_cstr 退回 fmt_num）"
NEGCTL apply A > "$W/negA_apply.log" 2>&1
build_drivers >/dev/null 2>&1
M233_PXI="$PXI" TT --det > "$W/negA_det.log" 2>&1; NEG_A_DET=$?
M233_PXI="$PXI" TT > "$W/negA_tt.log" 2>&1; NEG_A_TT=$?
grep -E '❌|^例数' "$W/negA_tt.log" | head -4 | sed 's/^/     /'
chk "负控 A：**确定性**层必须红" "[ $NEG_A_DET -ne 0 ]"
chk "负控 A：三轨/对齐层必须红" "[ $NEG_A_TT -ne 0 ]"
NEGCTL restore >/dev/null 2>&1

echo "── [6] 负控 B：只撤回解释轨的标记值规范化"
NEGCTL apply B > "$W/negB_apply.log" 2>&1
bash selfhost/devbuild.sh pxi > "$W/negB_dev.log" 2>&1
[ "$DEVPXI" = "/tmp/pxidev" ] || cp -f /tmp/pxidev "$DEVPXI"
build_drivers >/dev/null 2>&1
M233_PXI="$DEVPXI" TT > "$W/negB_tt.log" 2>&1; NEG_B_TT=$?
M233_PXI="$DEVPXI" TT --det > "$W/negB_det.log" 2>&1; NEG_B_DET=$?
grep -E '^例数' "$W/negB_tt.log" | sed 's/^/     /'
chk "负控 B：**对齐**层必须红（enum/struct/function 三类型）" \
    "grep -qE '对齐不符 [1-9]' '$W/negB_tt.log'"
chk "负控 B：**确定性**层仍绿（两组判据互相独立）" "[ $NEG_B_DET -eq 0 ]"

echo "── [7] 负控 C：判据自伤（关掉对齐与 MODEL）⇒ B 的红必须消失"
M233_PXI="$DEVPXI" TT --harm > "$W/negC_tt.log" 2>&1; NEG_C_RC=$?
grep -E '^例数' "$W/negC_tt.log" | sed 's/^/     /'
chk "负控 C：三层判据全关后（B 的补丁仍在位）必须不再红" "[ $NEG_C_RC -eq 0 ]"

echo "── [8] 源逐字节还原"
NEGCTL restore >/dev/null 2>&1
NEGCTL check > "$W/restore.log" 2>&1
grep -E 'M233-NEG-RESTORED' "$W/restore.log" | sed 's/^/     /'
chk "负控后源码逐字节还原" "grep -q 'M233-NEG-RESTORED-OK' '$W/restore.log'"
python3 "$D/gen_probes.py" --root "$ROOT" --audit > "$W/final_audit.log" 2>&1 \
  && chk "还原后源码派生判据复绿" true || chk "还原后源码派生判据复绿" false
fi

echo "── [9] 覆盖边界（如实登记）"
cat <<'EOF'
     · 面只取「一元文本语义接口」5 个（sha256 / md5 / base64_encode / ord / bytes_to_hex）：
       `xxhash` 不在 `ibuiltin.px` 的显式分派表里（走另一条解析路径），本轮**未纳面**；
       二元接口（`hmac_sha256` / `regex_match` / `regex_replace` / `pbkdf2_sha256`）同理待扩。
     · `bytes` 实参**不参与对齐判据**：它走「对象自身缓冲」（语义是**原始字节**，
       不是文本形态）⇒ `f(bytes)` ≠ `f(str(bytes))` 是**设计**，已由 cases.tsv 的 align 列登记。
     · 数据语义接口（`json_stringify` / `len` / `s3_get` …）**必须**拿到原值，
       故解释轨的 `i_cstr_arg` 规范化**只对 §6.5 文本语义接口**做（否则会改语义）。
     · `chan` / `mutex` / `rwlock` 在解释轨不可构造（Mini 子集排除）⇒ 不在面内。
     · 缺陷 351 的**根治**（让 C 侧渲染器认识解释轨的标记字典）未做：
       本轮采用的是「在解释轨侧**先把标记值规范化**」—— 与「渲染器必须与表示无关」的理想
       尚有差距，登记为后续候选。
EOF

echo
echo "══ 汇总：通过 $pass · 失败 $fail ══"
[ "$fail" -eq 0 ] && { echo "M233-VERIFY-OK"; exit 0; }
echo "M233-VERIFY-FAIL"; exit 1
