#!/usr/bin/env bash
# ============================================================
# M160 门（第 46 轮）：词法闭包 / 函数体内 `def` —— 缺陷 159（VM 轨）+ 缺陷 160（C 轨）
# ------------------------------------------------------------
# 背景（M158 登记、本轮收口）：
#   · 缺陷 159（VM 轨 = **用户面默认轨**）：`bc_emit` 不支持函数体内 `FuncDef`
#     （发射期 panic），且**没有捕获机制** —— 闭包体引用外层局部一律按全局名查
#     ⇒ 运行期「未定义变量」/「无法相加: null + int」。
#   · 缺陷 160（C 轨）：函数体内 `def` 的闭包捕获在运行期「未定义变量: hits」；
#     更基本的是 `cg_ast_bound` 把 **Assign 目标当"绑定"** ⇒ 与体里的引用相消 ⇒
#     自由变量集漏掉它 ⇒ 外层不装箱、闭包把外层局部当全局名。
# 修法（两轨同一条真相：**按引用捕获**，与解释轨 env 链语义一致）：
#   · C 轨：Assign 目标改判为"使用"；函数体内 `def` 提升为本帧局部 + 闭包发射抽成
#     `cg_gen_closure(params, body, dispname)`（匿名 fn / 具名 def 共用）。
#   · VM 轨：新增 cell 四指令（CELLGET/CELLSET/CELLNEW/MKCLO）+ `PxVMFunc.upvals/nup`
#     + `PxFrame.env`；发射器做同一份自由变量分析，装 cell 入 env，帧入口按 upnames 绑槽。
# 判据（逐层可单独变红）：
#   ① 解释轨闭包用例 pass=13 fail=0（绝对值断言）；
#   ② VM 轨（默认轨）编译+运行 ⇒ stdout 与解释轨**逐字节一致**；
#   ③ C 轨（逃生舱）⇒ stdout 与解释轨**逐字节一致**；
#   ④ 加强面（跨两层/三层捕获 + 闭包经 runtime native 回调）三轨一致；
#   ⑤ M158 缺口复现器（interp_fn_closure.px）三轨一致 —— M158 门第 ⑤ 层由此**升格**
#      为硬判据（原为 SKIP 报告）；
#   ⑥ 缺口登记（**不计失败**，但必须打印编号）：生成器 `transform/filter` 引用外层局部
#      时 VM 轨仍按全局名解析（缺陷 163，待下一轮）；
#   ⑦ 负控 3 道（默认跑，`--neg-skip` 跳过）—— 每道必须判红 + 逐字节还原：
#      A C 轨：把 Assign 目标重新当「绑定」（恢复缺陷 160 的绑定口径）⇒ C 轨必红
#      B VM 轨：关掉本帧装箱（不发 CELLNEW）⇒ VM 轨必红
#      C VM 轨：MKCLO 的捕获槽基址 +1（装错 cell）⇒ VM 轨必红
# 用法：./examples/m160_closure/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m160_closure/verify.sh --neg-skip （只跑正判据，CI 用）
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
CASE=$HERE/closure_test.px
EDGE=$HERE/closure_edge.px
M158CASE=examples/m158_interp_fn/interp_fn_closure.px
WORK=$(mktemp -d /tmp/m160.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

# 三轨跑一个用例：$1=用例 $2=标签 → 落 $WORK/<tag>.{interp,vm,c}.out
three_tracks() {
    local case=$1 tag=$2 pxc="./tools/px"
    cp "$case" "$WORK/$tag.px"
    timeout 60 "$PXI" "$WORK/$tag.px" > "$WORK/$tag.interp.out" 2>&1 || true
    rm -rf "$WORK/build"
    if timeout 600 $pxc build "$WORK/$tag.px" > "$WORK/$tag.vm.log" 2>&1 && [ -x "$WORK/build/$tag" ]; then
        timeout 90 "$WORK/build/$tag" > "$WORK/$tag.vm.out" 2>&1 || true
    else
        : > "$WORK/$tag.vm.out"; note "VM 轨编译失败（日志 $WORK/$tag.vm.log）"
    fi
    rm -rf "$WORK/build"
    if timeout 600 $pxc build --c "$WORK/$tag.px" > "$WORK/$tag.c.log" 2>&1 && [ -x "$WORK/build/$tag" ]; then
        timeout 90 "$WORK/build/$tag" > "$WORK/$tag.c.out" 2>&1 || true
    else
        : > "$WORK/$tag.c.out"; note "C 轨编译失败（日志 $WORK/$tag.c.log）"
    fi
    rm -rf "$WORK/build"
}

expect_all_three() {     # $1=标签 $2=期望行（如 "pass=13 fail=0"）
    local tag=$1 want=$2
    if grep -q "^$want\$" "$WORK/$tag.interp.out"; then
        note "解释轨：$want"
    else
        bad "$tag 解释轨未达 $want"; sed 's/^/      /' "$WORK/$tag.interp.out" | head -6
    fi
    for t in vm c; do
        if grep -q "^$want\$" "$WORK/$tag.$t.out"; then
            note "$t 轨：$want"
        else
            bad "$tag $t 轨未达 $want（$(grep -m1 -E 'FAIL|运行时错误|错误' "$WORK/$tag.$t.out" | head -c 120)）"
        fi
        cmp -s "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" \
            || { bad "$tag $t 轨 stdout 与解释轨不一致"; diff "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" | head -6 | sed 's/^/      /'; }
    done
}

hdr "[1/4] 解释轨 + VM 轨 + C 轨：闭包/函数体内 def 主用例（13 断言）"
three_tracks "$CASE" main
expect_all_three main "pass=13 fail=0"

hdr "[2/4] 加强面：跨两层/三层捕获 + 闭包经 runtime native 回调（5 断言）"
three_tracks "$EDGE" edge
expect_all_three edge "pass=5 fail=0"

hdr "[3/4] 交叉判据：M158 缺口复现器（缺陷 159/160）三轨一致"
three_tracks "$M158CASE" m158
expect_all_three m158 "pass=5 fail=0"

hdr "[4/4] 缺口登记（不计失败）"
note "缺陷 163（待下一轮）：生成器 transform/filter 引用**外层局部**时，VM 轨按全局名解析"

# ---------------- 负控 ----------------
run_neg() {              # $1=标签 $2=引擎(c|vm) $3=文件 $4=旧文本 $5=新文本 $6=说明
    local tag=$1 eng=$2 file=$3 from=$4 to=$5 what=$6
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
    local ok=0
    if [ "$eng" = "c" ]; then
        timeout 900 ./tools/px build "$ROOT/selfhost/compiler.px" > "$WORK/neg$tag.build.log" 2>&1 \
            && cp "$ROOT/selfhost/build/compiler" "$WORK/pxc_neg" && ok=1
    else
        timeout 1500 bash selfhost/devbuild.sh --vm > "$WORK/neg$tag.build.log" 2>&1 \
            && cp /tmp/pxcdev_vm "$WORK/pxc_vm_neg" && ok=1
    fi
    rm -rf "$ROOT/selfhost/build"
    if [ $ok -eq 0 ]; then
        bad "$tag 负控编译器构建失败"; tail -3 "$WORK/neg$tag.build.log" | sed 's/^/      /'
    else
        if [ "$eng" = "c" ]; then
            PX_PXC_BIN="$WORK/pxc_neg" three_tracks "$CASE" neg$tag
        else
            PXC_VM_BIN="$WORK/pxc_vm_neg" three_tracks "$CASE" neg$tag
        fi
        local out="$WORK/neg$tag.$eng.out"
        if [ "$eng" = "c" ]; then out="$WORK/neg$tag.c.out"; else out="$WORK/neg$tag.vm.out"; fi
        if grep -q '^pass=13 fail=0$' "$out"; then
            bad "$tag 负控**未判红**（篡改后仍全绿 ⇒ 门的判别力不足）"
        else
            local why
            why=$(grep -m1 -E 'FAIL|运行时错误|未定义变量|错误' "$out" 2>/dev/null | head -c 120)
            [ -n "$why" ] || why="（判红在**编译期**：$(tail -1 "$WORK/neg$tag.$eng.log" 2>/dev/null | head -c 120)）"
            note "✅ $tag 负控判红：$why"
        fi
    fi
    cp "$WORK/$(basename "$file").bak" "$file"
    cmp -s "$WORK/$(basename "$file").bak" "$file" || bad "$tag 还原失败（源码与备份不一致）"
}

if [ $NEG_SKIP -eq 0 ]; then
    run_neg A c selfhost/codegen.px \
        '        if h == "VarDecl":
            cg_name_add(out, rust_unescape(x[2]))' \
        '        if h == "VarDecl":
            cg_name_add(out, rust_unescape(x[2]))
        elif h == "Assign" and type(x[1]) == "list" and len(x[1]) > 0 and x[1][0] == "Var":
            cg_name_add(out, rust_unescape(x[1][1]))' \
        "恢复缺陷 160：把 Assign 目标重新当「绑定」（cg_ast_bound）"
    run_neg B vm selfhost/bc_emit.px \
        '            bc_emit_inst(func, "CELLNEW", func["smap"][cn], func["smap"][cn], 0)' \
        '            let _m160_negB = func["smap"][cn]' \
        "关掉 VM 轨本帧装箱（不发 CELLNEW）"
    run_neg C vm selfhost/bc_emit.px \
        '    let base = func["next_slot"]' \
        '    let base = func["next_slot"] + 1' \
        "MKCLO 捕获槽基址 +1（装错 cell）"
    # 负控跑完后源码已逐字节还原；入库件不受影响（负控只用 /tmp 现场件），仍复核指纹门。
    echo "── 负控收尾：复核入库件指纹门 + 复跑正判据"
    bash selfhost/rebake_bin.sh --check-all > "$WORK/checkall.log" 2>&1 \
        || bad "负控收尾后入库件指纹门未过"
    grep -q "全件源码链一致" "$WORK/checkall.log" || bad "负控收尾后入库件指纹门未过"
    three_tracks "$CASE" after
    expect_all_three after "pass=13 fail=0"
fi
rm -rf "$ROOT/selfhost/build" "$WORK/build"

echo
if [ $fail -eq 0 ]; then
    echo "M160-VERIFY-OK（词法闭包 / 函数体内 def：三轨一致；缺陷 159 + 160 收口）"
else
    echo "M160-VERIFY-FAIL"
fi
exit $fail
