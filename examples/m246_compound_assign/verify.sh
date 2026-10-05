#!/usr/bin/env bash
# ============================================================
# M246 门（第 123 轮）：**复合赋值（`Var` 目标）全量对拍**
#
# 为什么是这个面
# --------------
# M231 量过 `(二元运算符 × 左类型 × 右类型)` 的**表达式**矩阵；
# M204 量过**复合赋值**在 `Index` / `Field` 目标上的三轨同一真相（缺陷 245/246）。
# 而 **`x op= y`（`Var` 目标）的同一矩阵从来没有被清单级度量过** ——
# M204 的门头只写了一句**口头断言**：「`Var` 目标那一支走 `cg_assign_op_local`，是对的」。
# 本轮把这句断言**变成判据**。
#
# 面 = 13 个复合赋值运算符 × 14 个类型对 × **两个形态**（共 364 例）：
#   形态 A（被测）：`var x = <L>;  x <op>= <R>;  print(x)`
#   形态 B（对照）：`var x = <L>;  x = x <op> <R>;  print(x)`
#
# 判据层级
# --------
#   [1] 静态：运算符清单**源码派生**（parser.px 的复合赋值分支）⇄ ORDER **双向一致** ·
#            规模锚点（364 例）· `cg_assign_op_local` 在位 · negctl 锚点自证
#   [2] 构建两轨驱动器（C 轨 `PX_BUILD_ENGINE=c` / VM 轨默认）
#   [3][4] 三轨对拍：响亮性 / 值 / R 码 + 词条 逐字节一致 · MODEL.tsv **双向**核对
#   [5] **形态等价性**（独立真值，不依赖三轨）：`x op= y` ⇄ `x = x op y` 必须同结果
#   [6] 负控（打桩 `cg_assign_op_local` 的 `+=` 支 ⇒ 三轨必分叉）+ 判据自伤
#   [7] 覆盖边界（如实登记）
#
# 用法：bash examples/m246_compound_assign/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m246_gate
rm -rf "$W"; mkdir -p "$W"
D=examples/m246_compound_assign
PXI="${M246_PXI:-$ROOT/bootstrap/pxi}"

pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

# ── 源快照/还原（负控打桩必须回滚）──
snap_src() { cp -f selfhost/cg_stmt.px "$W/cg_stmt.bak"; }
restore_src() { [ -f "$W/cg_stmt.bak" ] && cp -f "$W/cg_stmt.bak" selfhost/cg_stmt.px; }
trap 'restore_src' EXIT
snap_src

echo "── [1] 静态判据与清单派生"
if python3 "$D/gen_probes.py" --root "$ROOT" >"$W/derive.log" 2>&1; then
    sed 's/^/     /' "$W/derive.log"
    chk "运算符清单源码派生 ⇄ ORDER 双向一致" "grep -q '派生 ⇄ ORDER 双向一致' $W/derive.log"
else
    chk "运算符清单源码派生" false; sed 's/^/     /' "$W/derive.log"
fi
N_CASES=$(( $(wc -l < "$D/cases.tsv") - 1 ))
N_REF=$(grep -c '_ref	' "$D/cases.tsv" || true)
chk "规模锚点：例数 = 364（实测 $N_CASES）" "[ '$N_CASES' = 364 ]"
chk "规模锚点：对照例 = 182（实测 $N_REF）" "[ '$N_REF' = 182 ]"
chk "MODEL.tsv 行数 == 例数 + 1" "[ \"\$(wc -l < \"$D/MODEL.tsv\")\" = \"$((N_CASES+1))\" ]"
chk "C 轨：Var 目标复合赋值发射在位（cg_assign_op_local）" \
    "grep -q 'def cg_assign_op_local' $ROOT/selfhost/cg_stmt.px"
chk "C 轨：cg_assign_op_local 覆盖 Plus/Minus/…（≥ 8 个分支）" \
    "[ \"\$(sed -n '/def cg_assign_op_local/,/^def /p' $ROOT/selfhost/cg_stmt.px | grep -c 'return \"px_')\" -ge 8 ]"
chk "负控锚点自证（唯一命中）" "python3 \"$D/negctl.py\" --root \"$ROOT\" --snap \"$W/snap\" --selftest | grep -q NEGCTL-SELFTEST-OK"

echo "── [2] 生成探针 + 构建两轨驱动器"
build_drivers() {
    local extra="${1:-}"      # 负控用：PX_PXC_BIN=/tmp/pxcdev（否则跑的是入库件 = 假绿）
    rm -rf "$W/a_c" "$W/a_vm" "$W/build"; mkdir -p "$W/a_c" "$W/a_vm" "$W/build"
    cp -f "$D/drv.px" "$W/a_c/drv.px"; cp -f "$D/drv.px" "$W/a_vm/drv.px"
    ( cd "$W/a_c" && env $extra PX_BUILD_ENGINE=c timeout 2400 "$ROOT/tools/px" build drv.px ) >"$W/b_c.log" 2>&1 \
        || { echo "     ❌ C 轨构建失败："; tail -12 "$W/b_c.log" | sed 's/^/     /'; return 1; }
    ( cd "$W/a_vm" && timeout 2400 "$ROOT/tools/px" build drv.px ) >"$W/b_vm.log" 2>&1 \
        || { echo "     ❌ VM 轨构建失败："; tail -12 "$W/b_vm.log" | sed 's/^/     /'; return 1; }
    cp -f "$W/a_c/build/drv" "$W/build/drv_c"
    cp -f "$W/a_vm/build/drv" "$W/build/drv_vm"
    [ -x "$W/build/drv_c" ] && [ -x "$W/build/drv_vm" ]
}
if build_drivers; then chk "两轨驱动器构建" true; else chk "两轨驱动器构建" false; fi

echo "── [3][4] 三轨对拍（$N_CASES 例）+ MODEL 双向核对"
if M246_PXI="$PXI" timeout 1800 python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" >"$W/three.log" 2>&1; then
    chk "三轨对拍 + MODEL 双向（$N_CASES 例）" "grep -q M246-THREE-TRACKS-OK '$W/three.log'"
else
    chk "三轨对拍 + MODEL 双向（$N_CASES 例）" false; head -25 "$W/three.log" | sed 's/^/     /'
fi
grep -E '^例数' "$W/three.log" | sed 's/^/     /' || true

echo "── [5] 形态等价性（独立真值 · 不依赖三轨）"
if M246_PXI="$PXI" timeout 1800 python3 "$D/pair_check.py" --root "$ROOT" --work "$W" >"$W/pairs.log" 2>&1; then
    chk "★ 形态等价性：\`x op= y\` ⇄ \`x = x op y\`（182 对 × 3 轨）" "grep -q M246-PAIRS-OK '$W/pairs.log'"
else
    chk "★ 形态等价性" false; tail -20 "$W/pairs.log" | sed 's/^/     /'
fi
tail -2 "$W/pairs.log" | sed 's/^/     /' || true

echo "── [6] 负控（$([ "$NEG" = 1 ] && echo '全量' || echo '--neg-skip 跳过')）"
negA="skip"
if [ "$NEG" = 1 ]; then
    restore_src
    if python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" --apply >"$W/negctl.log" 2>&1; then
        # ⚠️ `cg_stmt.px` 是**编译器源码** ⇒ 只打桩**不会**影响 `tools/px build`
        #    （它用入库的 `bootstrap/pxc`）⇒ 必须**重编 C 轨编译器**（devbuild → /tmp/pxcdev）
        #    并用 `PX_PXC_BIN` 注入，否则跑的还是入库件 = **假绿**
        #    —— 这条纪律 M204 门头已登记，本门首版正是踩在它上面（实测「打桩成功但门仍绿」）。
        if timeout 900 bash selfhost/devbuild.sh >"$W/negA_dev.log" 2>&1 && [ -x /tmp/pxcdev ]; then
            if build_drivers "PX_PXC_BIN=/tmp/pxcdev" >"$W/negA_build.log" 2>&1; then
                if M246_PXI="$PXI" timeout 1800 python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" >"$W/negA.log" 2>&1; then
                    negA="green"     # 打桩后仍绿 ⇒ 判据没牙
                else
                    negA="red"       # 分叉 ⇒ 正是要的红
                fi
            else
                negA="buildfail"
            fi
        else
            negA="devfail"
        fi
    else
        negA="anchor"
    fi
    restore_src
    # ⚠️ /tmp/pxcdev 是**共享**产物（`PX_PXC_BIN` 的默认注入点）⇒ 留着**打桩版**会污染
    #    后续用它的门（如 M204）。删掉 ⇒ 下一个需要它的门自己重建（devbuild 有指纹短路）。
    rm -f /tmp/pxcdev
    chk "负控 A 打桩成功（锚点唯一命中）" "[ '$negA' != 'anchor' ] && [ '$negA' != 'buildfail' ] && [ '$negA' != 'devfail' ]"
    chk "★ 负控 A：丢弃 += 运算符后**必须判红**（三轨分叉）—— 实测 $negA" "[ '$negA' = 'red' ]"
else
    echo "     （--neg-skip：跳过负控 A）"
fi

echo "── [7] 覆盖边界（如实登记）"
cat <<'EOF'
      · 本门只覆盖 **`Var` 目标**的复合赋值 —— `Index` / `Field` 目标由 **M204 门**覆盖
        （缺陷 245/246）；`<-`（追加简写）不是复合赋值运算符，不在面内。
      · 类型对沿用 M231 的 14 对（同类型对 + 与 int/str 的异类型对），**不是 7×7 全叉积**
        ⇒ 「某类型对从不出现」的组合未在面内。
      · 形态 B（对照）用 `x = x <op> y` —— 对**可变容器**（list/dict）这是**新建**语义；
        语言对 `x += y` 取同一语义（M246 实测确认，非 Python 的「原地 extend」），
        故等价性判据成立。若日后改成原地语义，**本判据会红**（这是有意的守卫）。
      · 解释轨的静态语义检查**宽松**（ECOSYSTEM_GAPS G3 已登记：`px run` 不报 E3002）
        ⇒ 本门探针一律用 `var`（不用 `let`），避免把「已知设计差异」误当分叉。
EOF

echo "────────────────────────────────────────────"
if [ "$fail" = 0 ]; then echo "M246-VERIFY-OK（$pass 通过）"; exit 0; else echo "M246-VERIFY-FAIL（$pass 通过 / $fail 失败）"; exit 1; fi
