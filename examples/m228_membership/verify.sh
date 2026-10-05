#!/usr/bin/env bash
# ============================================================
# M228 门 · **成员运算 `in` / `not in` 三轨落地** + **「三个门」全量对拍**
# ------------------------------------------------------------
# 主题：M226 量过 **方法面**、M227 量过 **同名两门**（函数 ⇄ 方法）。
#   本轮补上**第三个门** —— **运算符面**：
#       `x in y` / `x not in y`   ⇄   `contains(y, x)`   ⇄   `y.contains(x)`
#   实测（修前）：`x in y` 作为**表达式在普贤里根本不存在**
#       `E2001 意外的 token: in` / `期望 ')'，实际得到 in`
#     —— 语言里 `in` 只出现在 `for x in y` 的**语句位置**；文档未承诺、上游 PX-DEF 未登记
#     ⇒ 这是一个**能力缺口**（与 M202 的 base32/hmac_sha1/sorted(key) 同族）。
#
# 判据：
#   [1] 静态：解析器两分支（`in` / `not in`，后者**必须吃掉两个 token**）·
#       三轨映射（cg_expr→px_in/px_not_in · bc_emit→IN/NOTIN · ival→i_membership）·
#       runtime 的**单份判定核心**（`px_membership_probe` 7 个引用点 · `px_memmem(` 只在
#       probe 里被调 ⇒ 三个门不再各写一遍扫描）· 声明与 opcode · 规模锚点
#   [2] 构建三轨驱动器（81 例聚合 · 一次编译覆盖全量）
#   [3] 动态：81 例 × 4 门 × 3 轨 = 972 次执行 ⇒ 跨轨 0 · 跨门 0 ·
#       **与 Python 独立真值一致** · H3 词条归属 0 违规 · RCODE 双向精确相等
#   [4] 负控 A：忠实撤回 runtime 的**取反**（`not in` 退化成 `in`）⇒ 必红
#   [5] 负控 B：忠实撤回解释轨的 `not in` 取反 ⇒ 必红（且 A 未在位 = 独立）
#   [6] 负控 C：判据自伤（`--harm`，A 补丁在位）⇒ A 的红**消失**
#   [7] 源逐字节还原（快照 + cmp）
#   [8] 覆盖边界（如实登记）
# CI 用 `--neg-skip`（负控各要**完整重建一次 runtime** ≈ 6–8 min）。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m228_membership
W=/tmp/m228_gate
rm -rf "$W"; mkdir -p "$W"
export M228_NEG_W="$W/snap"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

PXI="${M228_PXI:-$ROOT/bootstrap/pxi}"
NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" "$@"; }

has() { grep -qF "$2" "$ROOT/$1"; }
cnt() { grep -cF "$2" "$ROOT/$1" || true; }

echo "── [1] 静态判据"
chk "parser: 新增 chk_op(\"in\") 分支"            "has selfhost/parser.px 'elif chk_op(\"in\"):'"
chk "parser: not in 用 chk2 前瞻"                 "has selfhost/parser.px 'elif chk_op(\"not\") and chk2(\"in\"):'"
chk "parser: NotIn 必须吃掉两个 token"            "has selfhost/parser.px 'if op == \"NotIn\":'"
chk "cg_expr: In→px_in / NotIn→px_not_in"         "has selfhost/cg_expr.px 'return \"px_in\"' && has selfhost/cg_expr.px 'return \"px_not_in\"'"
chk "bc_emit: In→IN / NotIn→NOTIN"                "has selfhost/bc_emit.px 'return \"IN\"' && has selfhost/bc_emit.px 'return \"NOTIN\"'"
chk "ival: 运算符门拼写映射（op 名 ≠ 拼写）"       "has selfhost/ival.px 'if op == \"NotIn\":' && has selfhost/ival.px 'sp = \"not in\"'"
chk "ival: i_membership 措辞归运算符门"           "has selfhost/ival.px '运算符右操作数不支持类型'"
chk "runtime.h: px_in / px_not_in 声明"           "has runtime/runtime.h 'LXValue px_in(LXValue a, LXValue b);' && has runtime/runtime.h 'LXValue px_not_in(LXValue a, LXValue b);'"
chk "vm.h: PXOP_IN / PXOP_NOTIN / PXM_MAX ≥ 70"     "has runtime/vm.h '#define PXOP_IN      68' && has runtime/vm.h '#define PXOP_NOTIN   69' && [ \"\$(awk '/PXM_MAX/{print \$3}' runtime/vm.h)\" -ge 70 ]"
chk "vm.c: 指令名 + 两个 dispatch 分支"            "has runtime/vm.c '[PXOP_IN] = \"IN\", [PXOP_NOTIN] = \"NOTIN\",' && has runtime/vm.c 'slots[in.a] = px_not_in(slots[in.b], slots[in.c]);'"
chk "runtime: 判定核心单份（probe 恰 7 个调用/定义点）" "[ \"\$(cnt runtime/runtime.c 'px_membership_probe(')\" = 7 ]"
chk "runtime: px_memmem( 只在 probe 内被调（2 处=定义+调用）" "[ \"\$(cnt runtime/runtime.c 'px_memmem(')\" = 2 ]"
chk "runtime: 运算符门措辞带用户拼写"              "has runtime/runtime.c '%s 运算符左操作数需要 string，实际是 %s'"
chk "规模锚点：例数 ≥ 70"                          "[ \"\$(cnt $D/cases.tsv '')\" -ge 70 ]"
chk "负控锚点自证（唯一性）"                       "NEGCTL --selftest >/dev/null"

echo "── [2] 生成探针 + 构建三轨驱动器"
python3 "$D/gen_probes.py" >"$W/gen.log" 2>&1 && chk "探针生成" "true" || { chk "探针生成" "false"; tail -5 "$W/gen.log"; }
[ -f "$PXI" ] && chk "解释轨件存在（$PXI）" "true" || chk "解释轨件存在（$PXI）" "false"

build_drivers() {
    rm -rf "$W/a_c" "$W/a_vm" "$W/build"
    mkdir -p "$W/a_c" "$W/a_vm" "$W/build"
    cp -f "$D/drv.px" "$W/a_c/drv.px"; cp -f "$D/drv.px" "$W/a_vm/drv.px"
    ( cd "$W/a_c" && PX_BUILD_ENGINE=c timeout 1200 "$ROOT/tools/px" build drv.px ) >"$W/b_c.log" 2>&1 || { tail -12 "$W/b_c.log"; return 1; }
    ( cd "$W/a_vm" && timeout 1200 "$ROOT/tools/px" build drv.px ) >"$W/b_vm.log" 2>&1 || { tail -12 "$W/b_vm.log"; return 1; }
    cp -f "$W/a_c/build/drv" "$W/build/drv_c" && cp -f "$W/a_vm/build/drv" "$W/build/drv_vm"
    [ -x "$W/build/drv_c" ] && [ -x "$W/build/drv_vm" ]
}
if build_drivers; then chk "两轨驱动器构建" "true"; else chk "两轨驱动器构建" "false"; echo "M228-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; fi
# 负控前的 **pxi 基线**（强制重编自当前源码）—— [7] 用它证明「源还原 ⇒ 产物还原」。
#   ⚠️ 不能拿 `bootstrap/pxi` 当基线：**devbuild 与 rebake_bin 的链接参数不同**
#   （实测 10059536 vs 10059608 字节），两者本就不该逐字节相等。
( cd "$ROOT" && DEVB_REBUILD=1 timeout 1800 ./selfhost/devbuild.sh pxi ) >"$W/pre.build" 2>&1
cp -f /tmp/pxidev "$W/pxi.pre" 2>/dev/null || true
chk "pxi 基线已固化" "[ -s '$W/pxi.pre' ]"

echo "── [3] 三门 × 三轨对拍"
run3() { M228_PXI="$PXI" timeout 1800 python3 "$D/three_doors.py" --root "$ROOT" --work "$W" \
            --drv "$ROOT/$D/drv.px" --cases "$ROOT/$D/cases.tsv" --rcode "$ROOT/$D/RCODE.tsv" "$@"; }
if run3 >"$W/three.log" 2>&1; then chk "三门对拍（972 次执行）" "grep -q M228-THREE-DOORS-OK '$W/three.log'"
else chk "三门对拍（972 次执行）" "false"; head -20 "$W/three.log"; fi

if [ "$NEG" = 1 ]; then
    echo "── [4] 负控 A：撤回 runtime 的取反"
    NEGCTL --restore >/dev/null 2>&1
    if NEGCTL --apply A >"$W/ncA.patch" 2>&1 && build_drivers >"$W/ncA.build" 2>&1; then
        run3 >"$W/ncA.log" 2>&1; rcA=$?
        chk "负控 A 必红" "[ $rcA -ne 0 ] && grep -q 'M228-THREE-DOORS-FAIL' '$W/ncA.log'"
        grep -oE '差异 [0-9]+ 项' "$W/ncA.log" | head -1
    else
        chk "负控 A 必红" "false"; tail -5 "$W/ncA.patch" "$W/ncA.build"
    fi

    echo "── [6] 负控 C：判据自伤（A 在位 + --harm ⇒ A 的红消失）"
    if run3 --harm >"$W/ncC.log" 2>&1; then chk "负控 C 判据自伤" "grep -q M228-THREE-DOORS-OK '$W/ncC.log'"
    else chk "负控 C 判据自伤" "false"; head -6 "$W/ncC.log"; fi

    echo "── [5] 负控 B：撤回解释轨的 not in 取反"
    NEGCTL --restore >/dev/null 2>&1
    if NEGCTL --apply B >"$W/ncB.patch" 2>&1; then
        ( cd "$ROOT" && timeout 1800 ./selfhost/devbuild.sh pxi ) >"$W/ncB.build" 2>&1
        chk "负控 B 重编 pxi 与入库件不同（补丁确实生效）" "! cmp -s /tmp/pxidev '$ROOT/bootstrap/pxi'"
        M228_PXI=/tmp/pxidev run3 >"$W/ncB.log" 2>&1; rcB=$?
        chk "负控 B 必红" "[ $rcB -ne 0 ] && grep -q 'M228-THREE-DOORS-FAIL' '$W/ncB.log'"
        grep -oE '差异 [0-9]+ 项' "$W/ncB.log" | head -1
    else
        chk "负控 B 必红" "false"; cat "$W/ncB.patch"
    fi
else
    echo "── [4][5][6] 负控跳过（--neg-skip）"
fi

echo "── [7] 源逐字节还原"
NEGCTL --restore >"$W/restore.log" 2>&1; chk "还原" "NEGCTL --restore >/dev/null 2>&1"
chk "源码无负控残留" "! grep -rq 'NEGCTL-228' '$ROOT/runtime' '$ROOT/selfhost'" 
if [ "$NEG" = 1 ]; then
    ( cd "$ROOT" && DEVB_REBUILD=1 timeout 1800 ./selfhost/devbuild.sh pxi ) >>"$W/restore.log" 2>&1
    cp -f /tmp/pxidev "$W/pxi.restored" 2>/dev/null || true
    chk "还原后重编 pxi 与**负控前基线**逐字节一致" "cmp -s '$W/pxi.restored' '$W/pxi.pre'"
fi

echo "── [8] 覆盖边界（如实登记）"
echo "     · 只覆盖 4 个受支持集合（str/list/tuple/dict）；gen（生成器）不在矩阵内"
echo "       —— 三门对 gen 的分支由 M178/M226 覆盖，本门不重复。"
echo "     · 只覆盖「命中/未命中」与「元素或集合类型错」两类，不覆盖 arity（运算符无 arity）。"
echo "     · 通道（stdout vs stderr）与行:列前缀**不判** —— 属已登记缺陷 186，按 M186 归一化。"
echo "     · 方法门 .contains 在**不受支持接收者**上报 R1007（取方法层失败），"
echo "       与 op/func 门的 R1002 不同 ⇒ 已登记在 RCODE.tsv 并双向核对（非放水）。"
echo "     · 未覆盖：成员运算出现在解包/推导式/for 目标位置（parser 层面不可达）。"

echo "M228-VERIFY-OK pass=$pass fail=$fail" 
[ "$fail" = 0 ] || { echo "M228-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; }
