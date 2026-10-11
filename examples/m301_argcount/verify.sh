#!/bin/bash
# M301 门 · **函数调用「实参个数」三轨全量对拍**（第 111 轮 · 缺陷 518）
#
# 主题：M226 量过**方法面**、M227 量过「同名两门」、M230 量过索引/切片、M231 量过
#   `(运算符 × 类型)` 矩阵 —— 而**函数调用的实参个数**这一面从来没有被清单级度量过。
#   它是一个「每行代码都在用」的最基础面，却正是**三轨各自为政**的地方。
#
# 缺陷 518（修前实测 21 例 ⇒ **13 例分叉**，三轨四种行为）：
#   · **多给**：解释轨响亮 `R1005 参数过多` ⇄ **VM/C 两轨静默忽略**
#     （`f2(1,2,3)` = 3、`z0(1)` = 7、`p.scaled(3,1,9)` = 4 —— 静默错值方向）
#   · **少给（无默认值）**：解释/VM 响亮 R1005 ⇄ **C 轨静默绑 null**，
#     错误要等函数体用到该参数才在下游炸（`R1002 无法相加: int + null` ⇒ **指不到真因**）
#   · **少给（有默认值）**：三轨都对（补齐）—— 但**闭包**是个例外，见下
#   · **闭包默认值**：解释轨 6 ✓ ⇄ **VM 轨 `R1005 需 2 给 1`**（`bc_emit_push_lambda` 的
#     arity = `len(param_names)`、ndefault 恒 0）⇄ **C 轨丢默认值**（`px_null()`）
#
# 判据
#   [1] 静态：四处修复在位（C 轨两处发射点 / VM 轨判据函数 + 4 个调用点 / 闭包 arity）· 规模锚点
#   [2] 构建三轨（devbuild）+ 生成探针（21 例：12 strict + 9 legal）
#   [3] **三轨对拍**：分叉必须为 0（同 rc · 同 R 码 · 同 stdout）
#   [4] 期望判据：strict 12 例**三轨全响亮** · legal 9 例**三轨全绿**
#   [5] 负控 A（忠实停用 VM 轨判据 —— C 轨保留）⇒ 分叉必须回来
#   [6] 负控 B（忠实停用 C 轨两处发射检查 —— VM 轨保留）⇒ 分叉必须回来
#   [7] 负控 C（**判据自伤**）⇒ A 的红**必须消失**
#   [8] 源逐字节还原 + 复绿
#   [9] 覆盖边界（如实登记）
#
# CI 用 `--neg-skip`（负控各要重建一轨编译器 ≈ 1–2 min）。
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m301_argcount
W=/tmp/m301_gate
rm -rf "$W"; mkdir -p "$W" "$W/build"
export M301_ROOT="$ROOT"
export M301_SNAP="$W/snap"
NEGCTL="python3 $D/negctl.py"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
ok(){ echo "  ✅ $1"; pass=$((pass+1)); }
bad(){ echo "  ❌ $1"; fail=$((fail+1)); }
chk(){ local d="$1"; shift; if eval "$@" >/dev/null 2>&1; then ok "$d"; else bad "$d"; fi; }

build_all(){   # $1 = 标签
  ./selfhost/devbuild.sh pxc pxi --vm > "$W/devbuild_$1.log" 2>&1 || return 1
  python3 "$D/gen_probes.py" --out "$W" > "$W/gen_$1.log" 2>&1 || return 1
  PX_PXC_BIN=/tmp/pxcdev ./tools/px build --c "$W/drv.px" > "$W/cb_$1.log" 2>&1 || return 1
  cp -f "$W/build/drv" "$W/build/drv_c" || return 1
  PXC_VM_BIN=/tmp/pxcdev_vm ./tools/px build "$W/drv.px" > "$W/vmb_$1.log" 2>&1 || return 1
  cp -f "$W/build/drv" "$W/build/drv_vm" || return 1
  return 0
}
run_tt(){ python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --pxi /tmp/pxidev "$@" > "$W/tt.log" 2>&1; }

echo "── [1] 静态判据（四处修复在位）· 规模锚点"
chk "C 轨函数体发射含两处 R1005 检查" \
    "test \$(grep -c 'R1005: 调用 %s 参数不足' $ROOT/selfhost/codegen.px) -ge 1 -a \$(grep -c 'R1005: 调用 %s 参数过多' $ROOT/selfhost/codegen.px) -ge 1"
chk "C 轨闭包发射含检查 + 默认值（cl_def_c）" \
    "grep -q 'R1005: 调用 %s 参数不足' $ROOT/selfhost/cg_expr.px && test \$(grep -c 'cl_def_c' $ROOT/selfhost/cg_expr.px) -ge 2"
chk "VM 轨判据函数在位于 + 4 个调用点（2 处 entry + 2 处 CALL）" \
    "test \$(grep -c 'static void vm_check_arity' $ROOT/runtime/vm.c) -eq 1 -a \$(grep -c 'vm_check_arity(' $ROOT/runtime/vm.c) -ge 5"
chk "VM 轨闭包识别默认参数（bc_emit_push_lambda 的 ndefault）" \
    "grep -qF 'params_full = null' $ROOT/selfhost/bc_emit.px && grep -qF 'func[\"ndefault\"] = ndefault' $ROOT/selfhost/bc_emit.px"

echo "── [2] 构建三轨 + 生成探针"
if build_all init; then ok "devbuild 三轨 + 探针编译（drv_c / drv_vm）"; else
  bad "构建失败"; tail -8 "$W/devbuild_init.log" "$W/cb_init.log" "$W/vmb_init.log" 2>/dev/null; fi
grep -E '^生成 drv' "$W/gen_init.log" 2>/dev/null | sed 's/^/     /'

echo "── [3] 三轨对拍（分叉必须为 0）"
run_tt; TT_RC=$?
head -1 "$W/tt.log" | sed 's/^/     /'
grep -E '^(统计|分叉)' "$W/tt.log" | sed 's/^/     /'
chk "三轨对拍：分叉为 0" "[ $TT_RC -eq 0 ]"

echo "── [4] 期望判据"
chk "strict 12 例三轨**全响亮**（rc≠0）" "grep -q '统计：strict 12/12 全响亮' $W/tt.log"
chk "legal 9 例三轨**全绿**（rc=0）" "grep -q 'legal 9/9 全绿' $W/tt.log"

if [ "$NEG" -eq 1 ]; then
  $NEGCTL snapshot > "$W/neg_snap.log" 2>&1
  chk "负控快照建立" "grep -q 'M301-NEG-SNAPSHOT-OK' $W/neg_snap.log"

  echo "── [5] 负控 A：忠实停用 VM 轨判据 ⇒ 分叉必须回来"
  $NEGCTL negA_on > "$W/negA_on.log" 2>&1 || bad "负控 A 打桩失败"
  if build_all negA; then
    run_tt; NEG_A_RC=$?
    grep -E '^分叉' "$W/tt.log" | sed 's/^/     /'
    chk "负控 A：三轨对拍必须判红" "[ $NEG_A_RC -ne 0 ]"
  else bad "负控 A 构建失败"; fi
  $NEGCTL restore >/dev/null 2>&1

  echo "── [6] 负控 B：忠实停用 C 轨两处发射检查 ⇒ 分叉必须回来"
  $NEGCTL negB_on > "$W/negB_on.log" 2>&1 || bad "负控 B 打桩失败"
  if build_all negB; then
    run_tt; NEG_B_RC=$?
    grep -E '^分叉' "$W/tt.log" | sed 's/^/     /'
    chk "负控 B：三轨对拍必须判红" "[ $NEG_B_RC -ne 0 ]"
  else bad "负控 B 构建失败"; fi
  $NEGCTL restore >/dev/null 2>&1

  echo "── [7] 负控 C：判据自伤（分叉判据恒报 0）⇒ B 的红必须消失"
  if build_all negC; then
    run_tt --harm; NEG_C_RC=$?
    grep -E '^分叉' "$W/tt.log" | sed 's/^/     /'
    chk "负控 C：判据关掉后必须不再红（证明红来自比对）" "[ $NEG_C_RC -eq 0 ]"
  else bad "负控 C 构建失败"; fi

  echo "── [8] 源逐字节还原 + 复绿"
  $NEGCTL check > "$W/restore.log" 2>&1
  grep -E 'M301-NEG-RESTORED' "$W/restore.log" | sed 's/^/     /'
  chk "负控后源码逐字节还原" "grep -q 'M301-NEG-RESTORED-OK' $W/restore.log"
  if build_all final; then
    run_tt
    chk "还原后对拍复绿（分叉 0）" "grep -q '^分叉 0 / 21' $W/tt.log"
  else bad "还原后构建失败"; fi
fi

echo "── [9] 覆盖边界（如实登记）"
cat <<'EOF'
     · **生成器 lambda**（合成 transform/filter 的形参）**不加实参个数检查** —— 它的实参
       由运行时按固定个数给出，编译期无法判定；本门 h1 只覆盖「生成器可正常使用」。
     · **默认参数不在尾部**（`def f(a = 1, b)`）不在本门面内：M90-S1 已定「默认参数在尾部」
       语义（required = 首个带默认值的形参下标），非尾部形态不在契约内。
     · 错误**文案**三轨仍不同（解释轨指名 `缺少参数 'b'`；VM/C 轨给「需 N 给 M」方向词）——
       这是已登记的**缺陷 186 族**（通道与措辞不一），本门只判 **R 码 + 响亮性 + stdout**，
       **不按整行文本对拍**。
     · `chan.send` / `mutex.with` 等**方法面**的 arity 属 M226 门面；本门只覆盖**函数调用**
       与**用户 impl 方法**（后者在 C 轨是直调、在 VM 轨经 `px_method → px_call → px_vm_entry`）。
     · 原生（native）函数的 arity 属 M199 门面。
EOF

echo
echo "══ 汇总：通过 $pass · 失败 $fail ══"
[ "$fail" -eq 0 ] && { echo "M301-VERIFY-OK"; exit 0; }
echo "M301-VERIFY-FAIL"; exit 1
