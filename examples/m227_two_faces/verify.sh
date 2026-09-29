#!/usr/bin/env bash
# ============================================================
# M227 门 · **「同名两门」全量对拍**（函数面 ⇄ 方法面 · 缺陷 336/337/338/339/340/341）
# ------------------------------------------------------------
# 主题：**「三轨一致」看不见的缺陷，「两门对拍」才照得出来。**
#
#   `len(x)` 与 `x.len()` 是**同一个操作的两个门**。M199 量过 **native 函数面** 的
#   「逐位置 × 错类型」、M226 量过 **方法面** 的同款 —— 但**没人量过「同一个操作的两个门」**。
#   M199 缺陷 238（`replace(d,…)` 从函数面进却报 `R1007 类型 dict 没有方法 'replace'`）
#   正是这个形状的**第一例**，但那是**偶然撞上**的。
#
# 判据（期望集**从源码派生**）：
#   [1] 静态：`G ∩ ∪M`（G = `px_set_global("N", px_native("N"…)` 名字集；
#       M = `px_method` strcmp 链 ∪ `icall.px` 的 `name ==` 链）⇄ `xface.tsv` **精确相等**
#       ＋ 规模锚点 ＋ `pair/homonym` 分类（假朋友必须写明理由）＋ 负控锚点自证
#   [2] 构建两轨驱动器（含本轮 runtime.c 修复）
#   [3] 动态：197 例 × 两面 × 三轨 ⇒ **跨轨 0 · 跨面硬分叉 0**（H1 响亮性 / H2 合法输出 /
#       H3 词条归属）· R 码分歧 ⇄ `RCODE.tsv` 精确相等 · 豁免 ⇄ `XALLOW.tsv` 双向
#   [4] 负控 A：忠实撤回 `runtime.c` 4 处 ⇒ 重编驱动器 ⇒ **H1 必须红**
#   [5] 负控 B：忠实撤回 解释轨 3 处（`icall.px`/`ibuiltin.px`）⇒ 重编 pxi ⇒ **H1 必须红**
#   [6] 负控 C：判据自伤（`--harm`，A 补丁在位）⇒ A 的红**消失**
#   [7] 源逐字节还原（快照 + `cmp`）
#   [8] 覆盖边界（如实登记）
# CI 用 `--neg-skip`（负控要重编驱动器与解释轨件）。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m227_two_faces
W=/tmp/m227_gate
rm -rf "$W"; mkdir -p "$W"
export M227_NEG_W="$W/snap"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

PXI="${M227_PXI:-$ROOT/bootstrap/pxi}"
NEGCTL() { M227_ROOT="$ROOT" M227_NEG_W="$W/snap" python3 "$D/negctl.py" "$@"; }
NEGCTL --snapshot > /dev/null
trap 'NEGCTL --restore > /dev/null 2>&1' EXIT

build_drivers() {   # 建 VM / C 两轨驱动器 → $W/build/{drv_vm,drv_c}
    rm -rf "$W/a_vm" "$W/a_c" "$W/build"
    mkdir -p "$W/a_vm" "$W/a_c" "$W/build"
    cp "$W/drv.px" "$W/a_vm/drv.px"; cp "$W/drv.px" "$W/a_c/drv.px"
    ( cd "$W/a_vm" && timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_vm.log" 2>&1 || { tail -12 "$W/b_vm.log"; return 1; }
    ( cd "$W/a_c" && PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_c.log" 2>&1 || { tail -12 "$W/b_c.log"; return 1; }
    cp -f "$W/a_vm/build/drv" "$W/build/drv_vm" && cp -f "$W/a_c/build/drv" "$W/build/drv_c"
    [ -x "$W/build/drv_vm" ] && [ -x "$W/build/drv_c" ]
}

x2() {  # 对拍 → stdout 落 $1（⚠️ $1 是**日志路径**，不能跟着 "$@" 传给 python —— 首版漏 shift
        #   导致 python 收到未知参数、对拍根本没跑，而门却报「分叉不为 0」（判据读的是 usage 文本）
        #   ⇒ 又是「判据读不到真因」的老坑）
    local out="$1"
    shift
    python3 "$D/two_faces.py" --root "$ROOT" --work "$W" --pxi "$PXI" \
        --here "$D" --json "$W/rows.json" "$@" > "$out" 2>&1
    return $?
}
nsub() { sed -n "s/.*$2 \([0-9]*\) .*/\1/p" "$1" | head -1; }

echo "=== [1] 静态：xface.tsv ⇄ **源码派生**的交集（G ∩ ∪M）"
python3 "$D/gen_probes.py" --root "$ROOT" --out "$W" --here "$D" > "$W/gen.log" 2>&1
genrc=$?
sed 's/^/  /' "$W/gen.log"
chk "[1] 派生/清单/漂移判据 rc=0" "[ $genrc -eq 0 ]"
chk "[1] 同名两门 12 个（规模锚点）" "grep -q '同名两门 12 个' $W/gen.log"
chk "[1] xface.tsv ⇄ 派生交集精确相等" "grep -q 'xface.tsv ⇄ 派生交集精确相等' $W/gen.log"
chk "[1] 规模锚点自证（全局名≥300 · 方法≥30 · 交集=12）" "grep -q '\[锚点\] 规模 ok' $W/gen.log"
chk "[1] 探针规模下限：用例 ≥ 180" "grep -qE '生成 1[89][0-9] 例|生成 [2-9][0-9][0-9] 例' $W/gen.log"
chk "[1] 负控锚点自证（7 处，逐条唯一）" "NEGCTL --selftest | grep -q '锚点自证 OK'"
[ $genrc -ne 0 ] && { echo "生成失败，后续层跳过"; echo "M227-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; }

echo "=== [2] 构建两轨驱动器（含本轮 runtime.c 修复）"
if build_drivers; then chk "[2] 两轨驱动器构建" "true"; else chk "[2] 两轨驱动器构建" "false"; echo "M227-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; fi

echo "=== [3] 动态：197 例 × 两面 × 三轨 ⇒ 跨轨 0 · 跨面硬分叉 0"
x2 "$W/p.log"; prc=$?
grep -E '^  \[|^  ℹ|跨面硬分叉合计' "$W/p.log" | sed 's/^/  /'
chk "[3] 对拍 rc=0（修后全绿）" "[ $prc -eq 0 ]"
chk "[3] 跨轨分叉 = 0" "grep -q '同一面三轨分叉：0' $W/p.log"
chk "[3] H1 响亮性 = 0" "grep -q 'H1 响亮性 0' $W/p.log"
chk "[3] H2 合法输出 = 0" "grep -q 'H2 合法输出 0' $W/p.log"
chk "[3] H3 词条归属 = 0" "grep -q 'H3 词条归属 0' $W/p.log"
chk "[3] R 码分歧 ⇄ RCODE.tsv 精确相等（漏登记 0 · 过期 0）" "grep -q '漏登记 0 · 登记过期 0' $W/p.log"
chk "[3] 豁免表双向自洽（XALLOW 无过期）" "! grep -q 'XALLOW.tsv 有、实测已无此分叉' $W/p.log"
chk "[3] 豁免确有内容（≥1 条 INFO）" "grep -q 'ℹ H1 \[豁免\]' $W/p.log"

if [ $NEG -eq 1 ]; then
echo "=== [4] 负控 A：忠实撤回 runtime.c 4 处 ⇒ H1 必须红"
NEGCTL --revert A > "$W/ncA.rev" 2>&1; cat "$W/ncA.rev" | sed 's/^/  /'
build_drivers > /dev/null 2>&1 && chk "[4] 撤回后驱动器重建" "true" || chk "[4] 撤回后驱动器重建" "false"
x2 "$W/ncA.log"; arc=$?
grep -E '^  \[Ⅱ|^H1标签' "$W/ncA.log" | sed 's/^/  /'
chk "[4] A：H1 分叉回来（必须红）" "[ $arc -ne 0 ]"
chk "[4] A：含 contains 子串/字典族" "grep -q 'm227_contains_str_p1_' $W/ncA.log"
chk "[4] A：含 dict.len 静默族" "grep -q 'm227_len_dict_nextra' $W/ncA.log"
NEGCTL --restore > /dev/null; NEGCTL --snapshot > /dev/null
chk "[4] A：源逐字节还原" "cmp -s $W/snap/runtime/runtime.c $ROOT/runtime/runtime.c"

echo "=== [5] 负控 B：忠实撤回 解释轨 3 处 ⇒ H1 必须红（重编 pxi）"
NEGCTL --revert B > "$W/ncB.rev" 2>&1; cat "$W/ncB.rev" | sed 's/^/  /'
( cd "$ROOT" && timeout 1200 ./selfhost/devbuild.sh pxi ) > "$W/ncB.build" 2>&1
[ -x /tmp/pxidev ] && cp -f /tmp/pxidev "$W/pxi_neg"
chk "[5] 撤回后解释轨件构建" "[ -x $W/pxi_neg ]"
PXI_SAVE="$PXI"
PXI="$W/pxi_neg"
x2 "$W/ncB.log"; brc=$?
PXI="$PXI_SAVE"
grep -E '^  \[Ⅱ|^H1标签' "$W/ncB.log" | sed 's/^/  /'
chk "[5] B：H1 分叉回来（必须红）" "[ $brc -ne 0 ]"
chk "[5] B：含 dict.len 静默族" "grep -q 'm227_len_dict_nextra' $W/ncB.log"

echo "=== [6] 负控 C：判据自伤（--harm，B 补丁仍在位）⇒ B 的红消失"
PXI="$W/pxi_neg"
x2 "$W/ncC.log" --harm; crc=$?
PXI="$PXI_SAVE"
grep -E '^  \[Ⅱ' "$W/ncC.log" | sed 's/^/  /'
chk "[6] C：判据自伤后 rc=0（证明 B 的红来自比对本身）" "[ $crc -eq 0 ]"
chk "[6] C：自伤后 H1 报 0（与 B 的 >0 对照）" "grep -q 'H1 响亮性 0' $W/ncC.log"
NEGCTL --restore > /dev/null; NEGCTL --snapshot > /dev/null
chk "[6] 源逐字节还原（3 文件）" \
    "cmp -s $W/snap/runtime/runtime.c $ROOT/runtime/runtime.c && cmp -s $W/snap/selfhost/icall.px $ROOT/selfhost/icall.px && cmp -s $W/snap/selfhost/ibuiltin.px $ROOT/selfhost/ibuiltin.px"
else
echo "=== [4][5][6] 负控：--neg-skip（CI 模式）"
fi

echo "=== [7] 覆盖边界（如实登记）"
cat <<'EOF' | sed 's/^/  /'
  · 「假朋友」(close/remove) 不参与对拍 —— 同名不同操作（写文件 vs chan / 删文件 vs 删键），
    各自的逐位置对拍由 M199（函数面）与 M226（方法面）覆盖。
  · `join` 的**字符串序列**：函数面 `join("-","abc")` 支持（== Python `"-".join("abc")`），
    方法面 `str.join` **刻意不补** —— 接收者角色会与 Python 约定相反 ⇒ 静默算错。
    已在 XALLOW.tsv 登记并写明理由（唯一豁免）。
  · `gen`（生成器）无方法（设计性：M34 起只有 `gen_next`）⇒ 函数面独有的类型域，不对拍。
  · R 码分歧（3 种形状）**不硬判**，逐一登记在 RCODE.tsv（两面处于不同检查层）。
  · 只比对 (rc 是否 0 / R 码 / 第一行词条 / 第一行输出)；**通道与行:列前缀不判**（已登记缺陷 186）。
  · 只覆盖 10 个 pair 名字 × 声明的接收者类型（str/list/dict，合法实参取自 M226 spec.tsv）。
EOF

echo
echo "M227-VERIFY-$( [ $fail -eq 0 ] && echo OK || echo FAIL ) pass=$pass fail=$fail"
[ $fail -eq 0 ]
