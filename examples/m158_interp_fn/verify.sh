#!/usr/bin/env bash
# ============================================================
# M158 门（第 40 轮）：解释轨函数值 → runtime native 桥（缺陷 114 根治）
#                      + type() 函数值口径三轨一致（缺陷 161 根治）
# ------------------------------------------------------------
# 背景（缺陷 114）：解释轨把用户函数包装成 dict（__ufn__ / __builtin__），而 runtime
#   native 按 PX_FUNC/PX_NATIVE 校验函数参数 ⇒ 直接转发会 px_error（**不可捕获**）
#   终止进程 ⇒ 解释轨用户代码里「把函数值传给 runtime native」整族不可用
#   （`set_interval(fn (): …)` → `R1002: set_interval: 第一个参数必须是函数`）。
#   M158 建立双向桥：interp_bridge_install 注册调度器 + bi_ffi_call 自动桥接。
#
# 门的判据（逐层可单独变红）：
#   ① 解释轨跑 interp_fn_test.px ⇒ pass=9 fail=0（用例内含绝对值断言）；
#   ② VM 轨（px build 默认）编译 + 运行 ⇒ **stdout 与解释轨逐字节一致**；
#   ③ C 轨（px build --c）编译 + 运行 ⇒ **stdout 与解释轨逐字节一致**；
#   ④ type() 口径守卫：三轨 `type(函数值) == "function"`（缺陷 161 的回归守卫，
#      改回 "fn" 会让 ①-③ 的断言计数变化 ⇒ 必红）；
#   ⑤ 闭包三轨一致（**M160 起由"缺口 SKIP"升格为硬判据**）：interp_fn_closure.px
#      在 VM 轨 / C 轨都要 pass=5 fail=0 且 stdout 与解释轨逐字节一致
#      —— 缺陷 159（VM 轨 upvalue/函数体内 FuncDef）、缺陷 160（C 轨嵌套 def 捕获）。
#   ⑥ 负控 3 道（默认跑，`--neg-skip` 可跳过）：关闭自动桥接（runtime）/
#      不安装调度器（interp.px）/ 调度器忽略函数值（interp.px）—— **三道必须判红**，
#      且每道都要逐字节还原并复跑全绿。
#
# 用法：./examples/m158_interp_fn/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m158_interp_fn/verify.sh --neg-skip （只跑正判据，CI 用）
# 退出码：0 = 绿，1 = 红。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT"
export LC_ALL=C LANG=C

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PXI=./bootstrap/pxi
CASE=$HERE/interp_fn_test.px
CLOSURE=$HERE/interp_fn_closure.px
WORK=$(mktemp -d /tmp/m158.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }

echo "── [1/5] 解释轨：$CASE"
timeout 60 "$PXI" "$CASE" > "$WORK/interp.out" 2> "$WORK/interp.err"
rc_i=$?
cat "$WORK/interp.out" | sed 's/^/   /'
[ -s "$WORK/interp.err" ] && sed 's/^/   stderr: /' "$WORK/interp.err"
[ $rc_i -eq 0 ] || bad "解释轨退出码 $rc_i（期望 0）"
grep -q '^pass=9 fail=0$' "$WORK/interp.out" || bad "解释轨未输出 pass=9 fail=0（绝对值断言未全过）"

# ---- 把用例复制到干净目录再编译（tools/px build 输出到 <src_dir>/build/） ----
cp "$CASE" "$WORK/case.px"

echo "── [2/5] VM 轨（用户面默认轨）"
if timeout 400 ./tools/px build "$WORK/case.px" > "$WORK/vm.build.log" 2>&1; then
    cp "$WORK/build/case" "$WORK/case_vm"
    timeout 60 "$WORK/case_vm" > "$WORK/vm.out" 2> "$WORK/vm.err"
    rc_v=$?
    sed 's/^/   /' "$WORK/vm.out"
    [ -s "$WORK/vm.err" ] && sed 's/^/   stderr: /' "$WORK/vm.err"
    [ $rc_v -eq 0 ] || bad "VM 轨退出码 $rc_v（期望 0）"
    cmp -s "$WORK/interp.out" "$WORK/vm.out" || { bad "VM 轨 stdout 与解释轨不一致"; diff "$WORK/interp.out" "$WORK/vm.out" | head -6 | sed 's/^/   /'; }
else
    bad "VM 轨编译失败"; tail -3 "$WORK/vm.build.log" | sed 's/^/   /'
fi

echo "── [3/5] C 轨逃生舱"
rm -rf "$WORK/build"
if timeout 500 ./tools/px build --c "$WORK/case.px" > "$WORK/c.build.log" 2>&1; then
    cp "$WORK/build/case" "$WORK/case_c"
    timeout 60 "$WORK/case_c" > "$WORK/c.out" 2> "$WORK/c.err"
    rc_c=$?
    sed 's/^/   /' "$WORK/c.out"
    [ -s "$WORK/c.err" ] && sed 's/^/   stderr: /' "$WORK/c.err"
    [ $rc_c -eq 0 ] || bad "C 轨退出码 $rc_c（期望 0）"
    cmp -s "$WORK/interp.out" "$WORK/c.out" || { bad "C 轨 stdout 与解释轨不一致"; diff "$WORK/interp.out" "$WORK/c.out" | head -6 | sed 's/^/   /'; }
else
    bad "C 轨编译失败"; tail -3 "$WORK/c.build.log" | sed 's/^/   /'
fi

echo "── [4/5] type() 函数值口径守卫（缺陷 161）"
if [ -f "$WORK/case_vm" ] && [ -f "$WORK/case_c" ]; then
    for t in vm c; do
        if grep -q 'FAIL 函数值类型是 function' "$WORK/$t.out" 2>/dev/null; then
            bad "$t 轨 type(函数值) != \"function\"（缺陷 161 回归）"
        fi
    done
    note "三轨 type(函数值) == \"function\"（口径一致）"
fi

echo "── [5/5] 闭包三轨一致（M160：缺陷 159/160 已收口，硬判据）"
timeout 60 "$PXI" "$CLOSURE" > "$WORK/clo.interp.out" 2>&1
if grep -q '^pass=5 fail=0$' "$WORK/clo.interp.out"; then
    note "解释轨闭包捕获（upvalue）经桥正确：pass=5 fail=0"
else
    bad "解释轨闭包捕获用例未全过"; sed 's/^/   /' "$WORK/clo.interp.out" | head -6
fi
cp "$CLOSURE" "$WORK/clo.px"
# 判据以**运行**结果为准：编译成功但运行时语义错（VM 轨 null+int / C 轨未定义变量）
# 同样是缺口，必须报 SKIP 而非"已修"。
rm -rf "$WORK/build"
if timeout 400 ./tools/px build "$WORK/clo.px" > "$WORK/clo.vm.log" 2>&1 && [ -x "$WORK/build/clo" ]; then
    cp "$WORK/build/clo" "$WORK/clo_vm"
    timeout 60 "$WORK/clo_vm" > "$WORK/clo.vm.out" 2>&1
    if grep -q '^pass=5 fail=0$' "$WORK/clo.vm.out"; then
        note "✅ VM 轨闭包用例运行全过（缺陷 159 已收口 · M160）"
    else
        bad "缺陷 159 回归：VM 轨闭包**运行**未通过 —— $(grep -m1 -E '运行时错误|FAIL' "$WORK/clo.vm.out" | head -c 100)"
    fi
    cmp -s "$WORK/clo.interp.out" "$WORK/clo.vm.out" || bad "VM 轨闭包 stdout 与解释轨不一致（缺陷 159 回归）"
else
    bad "VM 轨闭包用例编译失败（缺陷 159 回归：bc_emit 不支持函数体内 FuncDef / 无 cell 捕获）"
fi
rm -rf "$WORK/build"
if timeout 500 ./tools/px build --c "$WORK/clo.px" > "$WORK/clo.c.log" 2>&1 && [ -x "$WORK/build/clo" ]; then
    cp "$WORK/build/clo" "$WORK/clo_c"
    timeout 60 "$WORK/clo_c" > "$WORK/clo.c.out" 2>&1
    if grep -q '^pass=5 fail=0$' "$WORK/clo.c.out"; then
        note "✅ C 轨闭包用例运行全过（缺陷 160 已收口 · M160）"
    else
        bad "缺陷 160 回归：C 轨闭包**运行**未通过 —— $(grep -m1 -E '运行时错误|FAIL' "$WORK/clo.c.out" | head -c 100)"
    fi
    cmp -s "$WORK/clo.interp.out" "$WORK/clo.c.out" || bad "C 轨闭包 stdout 与解释轨不一致（缺陷 160 回归）"
else
    bad "C 轨闭包用例编译失败（缺陷 160 回归）"
fi
rm -rf "$WORK/build"

# ---------------- 负控 ----------------
run_neg() {
    local tag=$1 file=$2 from=$3 to=$4 what=$5
    echo "── 负控 $tag：$what"
    cp "$file" "$WORK/$(basename "$file").bak"
    python3 - "$file" "$from" "$to" <<'PY'
import sys
p, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p, encoding="utf-8").read()
if s.count(a) != 1:
    print("锚点不唯一（%d）" % s.count(a)); sys.exit(1)
open(p, "w", encoding="utf-8").write(s.replace(a, b, 1))
PY
    [ $? -eq 0 ] || { bad "$tag 锚点失效（与源码不同步）"; cp "$WORK/$(basename "$file").bak" "$file"; return; }
    # 现场编一个临时解释器（比整轮重烘快，且只影响本门）
    local ok=0
    if timeout 900 ./tools/px build "$ROOT/selfhost/interp.px" > "$WORK/neg$tag.log" 2>&1; then
        cp "$ROOT/selfhost/build/interp" "$WORK/pxi_neg" 2>/dev/null && ok=1
    fi
    rm -f "$ROOT/selfhost/build/interp"
    if [ $ok -eq 0 ]; then
        bad "$tag 负控解释器构建失败"; tail -3 "$WORK/neg$tag.log" | sed 's/^/   /'
    else
        timeout 60 "$WORK/pxi_neg" "$CASE" > "$WORK/neg$tag.out" 2>&1
        if grep -q '^pass=9 fail=0$' "$WORK/neg$tag.out"; then
            bad "$tag 负控**未判红**（篡改后仍全绿 ⇒ 门的判别力不足）"
        else
            note "✅ $tag 负控判红：$(grep -m1 -E 'FAIL|运行时错误|未定义变量' "$WORK/neg$tag.out" | head -c 120)"
        fi
    fi
    cp "$WORK/$(basename "$file").bak" "$file"
    # 还原后必须逐字节一致
    cmp -s "$WORK/$(basename "$file").bak" "$file" || bad "$tag 还原失败（源码与备份不一致）"
}

if [ $NEG_SKIP -eq 0 ]; then
    run_neg A runtime/runtime_ffi.c \
        '    for (i = 0; i < n; i++) { if (px_interp_is_fnwrap(items[i])) { need = 1; break; } }' \
        '    for (i = 0; i < n; i++) { if (0 && px_interp_is_fnwrap(items[i])) { need = 1; break; } }' \
        "关闭 bi_ffi_call 自动桥接（runtime）"
    run_neg B selfhost/interp.px \
        '    interp_bridge_install(i_bridge_dispatch)' \
        '    let _m158_unused = 1' \
        "解释器不安装桥调度器（interp.px）"
    run_neg C selfhost/interp.px \
        '    let r = i_call_value(fnvalue, args, 0)' \
        '    let r = i_call_value(args[0], [], 0)' \
        "调度器忽略真实函数值（interp.px）"
    # 负控跑完后解释器要回到正版：重建 pxi/pxi_vm 并复跑全绿
    echo "── 负控收尾：重建入库解释器 + 复跑正判据"
    bash selfhost/rebake_bin.sh --entries=pxi > "$WORK/rebake.log" 2>&1 || bad "负控收尾重烘失败"
    bash selfhost/rebake_bin.sh --check-all > "$WORK/checkall.log" 2>&1 || bad "负控收尾后入库件指纹门未过"
    grep -q "全件源码链一致" "$WORK/checkall.log" || bad "负控收尾后入库件指纹门未过（详见 $WORK 日志）"
    timeout 60 "$PXI" "$CASE" > "$WORK/after.out" 2>&1
    grep -q '^pass=9 fail=0$' "$WORK/after.out" || bad "负控收尾后解释轨未恢复全绿"
    note "✅ 还原后复跑：$(cat "$WORK/after.out" | head -1)"
fi
rm -rf "$ROOT/selfhost/build" "$WORK/build"

echo
if [ $fail -eq 0 ]; then
    echo "M158-VERIFY-OK（解释轨函数值 → runtime native 桥；三轨一致）"
else
    echo "M158-VERIFY-FAIL"
fi
exit $fail
