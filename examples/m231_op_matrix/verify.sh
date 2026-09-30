#!/bin/bash
# M231 门 · **「运算符 × 逐类型组合」全量矩阵**（第 109 轮 · 缺陷 346）
#
# 主题：M199 量过 native **函数面**的「逐位置 × 错类型」、M226 量过**方法面**的同款、
#   M227 量过「同名两门」（函数 ⇄ 方法）、M228 量过「三个门」（运算符 ⇄ 函数 ⇄ 方法，
#   只针对 `in`）、M229 量过 tuple/result 接收者、M230 量过索引/切片族 ——
#   而 **`(运算符 × 左类型 × 右类型)` 这一整个矩阵从来没有被清单级度量过**。
#
# 缺陷 346：`str` 作左操作数、`int` 作右操作数时，**除 `+`（走 i_bin_add）与 `*`（合法重复）
#   之外的 11 个二元运算符**（`- / // % ** & | ^ << >> >>>`）在**解释轨**被静默路由到
#   「字符串重复」：
#       `"s" - 1` ⇒ `s` · `"abc" - 3` ⇒ `ababab` · `"s" & 1` ⇒ `s` · `"s" << 1` ⇒ `s`
#   而编译轨各自分函数实现（`px_sub`/`px_bitand`/…）⇒ 一律响亮 `R1002` ⇒ **三轨分叉**。
#   根因：`selfhost/ival.px` 的 `i_bin_numeric` 里，M179 加的 `string × int` → 重复分支
#   **没有限定 `op == "Mul"`**，且它位于**所有二元算术/位运算符的公共路径**、
#   并在**类型守卫之前**。
#   ⚠️ 资源面：`"a" - 1000000` 修前静默分配 **1 MB** 重复串（可放大）。
#
# 判据
#   [1] 静态：运算符清单**源码派生** ⇄ 门内清单双向一致 · 守卫在位 · 无残留无守卫分支 ·
#             规模锚点 · 负控锚点自证
#   [2] 生成探针（315 例 = 21 运算符 × 14 类型对 + 3 一元 × 7 类型）+ 构建两轨
#   [3][4] 三轨对拍：响亮性 / 值 / R 码 / 词条 四层 + MODEL.tsv **双向**核对
#   [5] **缺陷 346 精确形状**（12 例：11 个 346 直接形状 + `+` 对照）**独立**判据
#   [6] 资源面（放大路径必须关闭 + 合法重复仍在）
#   [7] 负控 A / B / C —— **各自证明不同判据层有牙**：
#        A 撤回守卫 ⇒ 「三轨对拍」层必须红
#        B **同时**让编译轨也静默 ⇒ 对拍层**变绿**、只有 [5] 期望值层能红
#        C 判据自伤 ⇒ 必须不再红
#   [8] 覆盖边界（如实登记）
#
# CI 用 `--neg-skip`（负控各要重编解释轨 / 两轨驱动 ≈ 3–8 min）。
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m231_op_matrix
W=/tmp/m231_gate
rm -rf "$W"; mkdir -p "$W" "$W/build"
export M231_NEG_W="$W/snap"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
PXI="${M231_PXI:-$ROOT/bootstrap/pxi}"
NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" "$@"; }
has() { grep -qF "$2" "$ROOT/$1"; }
cnt() { grep -cF "$2" "$ROOT/$1" || true; }

echo "── [1] 静态判据与清单派生"
if python3 "$D/gen_probes.py" --root "$ROOT" >"$W/derive.log" 2>&1; then
    chk "运算符清单：parser 源码派生 ⇄ 门内清单 双向一致" "grep -q '双向一致' '$W/derive.log'"
else chk "运算符清单双向一致" false; fi
grep -E '派生二元运算符|双向' "$W/derive.log" | sed 's/^/     /' | head -3
chk "ival.px：string×int 重复分支带 op == \"Mul\" 守卫" \
    "has selfhost/ival.px 'if op == \"Mul\" and tl == \"string\" and tr == \"int\":'"
chk "ival.px：无残留的「无守卫」形态（行首即 if tl == ...）" \
    "[ \"\$(grep -c '^    if tl == \"string\" and tr == \"int\":' \"$ROOT/selfhost/ival.px\")\" = 0 ]"
N_CASES=$(( $(wc -l < "$D/cases.tsv") - 1 ))
chk "规模锚点：例数 ≥ 300（实测 $N_CASES）" "[ '$N_CASES' -ge 300 ]"
# ⚠️ 复杂表达式一律**抽成变量**在脚本主体算好再进 chk（M213 教训：chk 的 eval 串里
#    裸 $N 会被 set -u 打成 unbound；本次第 [1] 层就栽在这上面）
N_BIN=$(awk -F'\t' 'NR>1 && $2 ~ /^b_/' "$D/cases.tsv" | wc -l)
N_UN=$(awk -F'\t' 'NR>1 && $2 ~ /^u_/' "$D/cases.tsv" | wc -l)
chk "规模锚点：二元例数 = 294（21 运算符 × 14 类型对，实测 $N_BIN）" "[ '$N_BIN' = 294 ]"
chk "规模锚点：一元例数 = 21（3 × 7，实测 $N_UN）" "[ '$N_UN' = 21 ]"
chk "MODEL.tsv 行数 == 例数 + 1" "[ \"\$(wc -l < \"$D/MODEL.tsv\")\" = \"$((N_CASES+1))\" ]"
chk "负控锚点自证（4 项，锚点唯一命中）" "NEGCTL --selftest >/dev/null"

echo "── [2] 生成探针 + 构建两轨驱动器"
python3 "$D/gen_probes.py" --root "$ROOT" --gen >"$W/gen.log" 2>&1 \
    && chk "探针 + MODEL 生成" true || { chk "探针生成" false; tail -5 "$W/gen.log"; }
[ -f "$PXI" ] && chk "解释轨件存在（$PXI）" true || chk "解释轨件存在" false

build_drivers() {
    rm -rf "$W/a_c" "$W/a_vm" "$W/build"; mkdir -p "$W/a_c" "$W/a_vm" "$W/build"
    cp -f "$D/drv.px" "$W/a_c/drv.px"; cp -f "$D/drv.px" "$W/a_vm/drv.px"
    ( cd "$W/a_c" && PX_BUILD_ENGINE=c timeout 2400 "$ROOT/tools/px" build drv.px ) >"$W/b_c.log" 2>&1 \
        || { echo "  C 轨构建日志尾："; tail -8 "$W/b_c.log"; return 1; }
    ( cd "$W/a_vm" && timeout 2400 "$ROOT/tools/px" build drv.px ) >"$W/b_vm.log" 2>&1 \
        || { echo "  VM 轨构建日志尾："; tail -8 "$W/b_vm.log"; return 1; }
    cp -f "$W/a_c/build/drv" "$W/build/drv_c"
    cp -f "$W/a_vm/build/drv" "$W/build/drv_vm"
    [ -x "$W/build/drv_c" ] && [ -x "$W/build/drv_vm" ]
}
if build_drivers; then chk "两轨驱动器构建" true; else chk "两轨驱动器构建" false; fi

echo "── [3][4] 三轨对拍（$N_CASES 例）+ MODEL 双向核对"
if M231_PXI="$PXI" timeout 1800 python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" >"$W/three.log" 2>&1; then
    chk "三轨对拍 + MODEL 双向（$N_CASES 例）" "grep -q M231-THREE-TRACKS-OK '$W/three.log'"
else
    chk "三轨对拍 + MODEL 双向（$N_CASES 例）" false; head -25 "$W/three.log" | sed 's/^/     /'
fi
grep -E '^例数 ' "$W/three.log" | sed 's/^/     /'

echo "── [5] 缺陷 346 精确形状（独立判据 · 12 例）"
if M231_PXI="$PXI" timeout 900 python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --shape346 >"$W/shape.log" 2>&1; then
    chk "形状层：str×int（非 *）三轨均响亮 · R1002 · 词条逐字同 · 含 string" \
        "grep -q M231-SHAPE346-OK '$W/shape.log'"
else
    chk "形状层（12 例）" false; tail -12 "$W/shape.log" | sed 's/^/     /'
fi
grep -E '形状例数' "$W/shape.log" | sed 's/^/     /'

echo "── [6] 资源面（放大路径关闭 + 合法重复仍在）"
cat > "$W/res_neg.px" <<'PX'
def main():
    let s = "a" - 1000000
    print("LEN=" + str(len(s)))
PX
RN=$(timeout 120 "$PXI" "$W/res_neg.px" 2>&1 | head -2)
chk "str-int（非 *）：必须响亮（修前静默产出 10^6 字符）" \
    "echo \"\$RN\" | grep -q R1002 && ! echo \"\$RN\" | grep -q 'LEN='"
cat > "$W/res_pos.px" <<'PX'
def main():
    print("LEN=" + str(len("a" * 1000000)))
PX
RP=$(timeout 120 "$PXI" "$W/res_pos.px" 2>&1 | head -1)
chk "str*int 重复仍可用（合法侧对照 LEN=1000000）" "[ \"\$RP\" = 'LEN=1000000' ]"

echo "── [7] 负控（$([ "$NEG" = 1 ] && echo '全量' || echo '--neg-skip 跳过')）"
if [ "$NEG" = 1 ]; then
    NEGCTL --snapshot >/dev/null
    # --- 负控 A：撤回守卫 ⇒ 三轨对拍层必须红 ---
    NEGCTL --apply A >/dev/null
    bash selfhost/devbuild.sh pxi >"$W/dev_A.log" 2>&1
    if M231_PXI=/tmp/pxidev timeout 1800 python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" >"$W/negA.log" 2>&1; then
        chk "负控 A（撤回 op==Mul 守卫）：对拍层必须判红" false
    else
        chk "负控 A（撤回 op==Mul 守卫）：对拍层必须判红" "grep -q '响亮性' '$W/negA.log'"
        grep -E '^例数 ' "$W/negA.log" | sed 's/^/     /'
    fi
    NEGCTL --restore >/dev/null
    # --- 负控 B：同时让编译轨也静默 ⇒ 对拍层变绿、只有形状层能红 ---
    NEGCTL --apply A >/dev/null; NEGCTL --apply B >/dev/null
    bash selfhost/devbuild.sh pxi >"$W/dev_B.log" 2>&1
    build_drivers >/dev/null 2>&1
    # ⚠️ B 只把 **`px_sub` 一个函数**改成静默（其余 10 个运算符的编译轨仍响亮）
    #    ⇒ 对拍层的分叉数应当 **11 → 10**（`-` 那一例对它失效），而 MODEL 层
    #    独立报出那 1 例不一致 —— 这就是「两轨都错时对拍看不见、期望值判据看得见」的实证。
    M231_PXI=/tmp/pxidev timeout 1800 python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" >"$W/negB_t.log" 2>&1
    chk "负控 B：分叉数 11 → 10（`-` 被 B 掩盖 ⇒ 对拍对它失效）" \
        "grep -q '三轨差异 10' '$W/negB_t.log'"
    # ⚠️ 判据必须**只看响亮性分叉段**：`b_M_str_int` 仍然会出现在日志里 —— 但那是
    #    **MODEL 段**（`表 ERR ⇄ 实测 VAL`），正是「期望值判据抓到了对拍抓不到的那一例」。
    NB_FMT=$(grep -c '响亮性. b_M_str_int' "$W/negB_t.log" || true)
    NB_ALL=$(grep -c 'b_M_str_int' "$W/negB_t.log" || true)
    chk "负控 B：`-` 不在响亮性分叉段（实测 $NB_FMT 次；全文 $NB_ALL 次 = MODEL 段抓到它）" \
        "[ '$NB_FMT' = 0 ] && [ '$NB_ALL' -ge 1 ]"
    chk "负控 B：MODEL 层独立报出 1 例不一致（期望值判据的另一种牙）" \
        "grep -q 'MODEL 不一致 1' '$W/negB_t.log'"
    if M231_PXI=/tmp/pxidev timeout 900 python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --shape346 >"$W/negB_s.log" 2>&1; then
        chk "负控 B：形状层必须**独立**判红（期望值判据有牙）" false
    else
        chk "负控 B：形状层必须**独立**判红（期望值判据有牙）" "grep -q M231-SHAPE346-FAIL '$W/negB_s.log'"
        grep -E '❌' "$W/negB_s.log" | head -3 | sed 's/^/     /'
    fi
    NEGCTL --restore >/dev/null
    bash selfhost/devbuild.sh pxi >"$W/dev_R.log" 2>&1; build_drivers >/dev/null 2>&1
    # 负控后必须回到全绿（还原证明）
    if M231_PXI=/tmp/pxidev timeout 1800 python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" >"$W/negR.log" 2>&1; then
        chk "负控后源逐字节还原（对拍回到全绿）" true
    else
        chk "负控后源逐字节还原" false; head -8 "$W/negR.log" | sed 's/^/     /'
    fi
else
    echo "  SKIP 负控（--neg-skip）"
fi

echo "── [8] 覆盖边界（如实登记）"
echo "     · 短路族 and / or / ?? / |> 不在矩阵内（对任意类型做真值性转换 ⇒ 响亮性判据无判别力）"
echo "     · 类型对取 14 对（全同类型 + 全与 int 异类型 + 与 str 异类型）；7×7 全交叉未做"
echo "     · in / not in 只覆盖 (str,str) / (list,list) 两种合法形状"
echo "     · list / dict / bool / null 的**同类型**序比较静默给 false（Python 是 TypeError）"
echo "       —— 三轨一致、非分叉，锁进基线，口径分歧另立候选"
echo "     · 一元只覆盖 - / not / ~ 三种；未覆盖链式调用与方法值"

echo ""
echo "══ 汇总：通过 $pass · 失败 $fail ══"
if [ "$fail" = 0 ]; then echo "M231-VERIFY-OK"; else echo "M231-VERIFY-FAIL"; fi
exit $((fail > 0))
