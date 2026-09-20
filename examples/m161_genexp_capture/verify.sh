#!/usr/bin/env bash
# ============================================================
# M161 门（第 47 轮）：生成器捕获（GenExp）—— 缺陷 163（VM 轨）+ 缺陷 164（C 轨）
# ------------------------------------------------------------
# 背景（M160 门第 ⑥ 层登记、本轮收口）：
#   · 缺陷 163（VM 轨 = **用户面默认轨**）：生成器 `transform` / `filter` 合成的 lambda
#     **不带捕获表** —— 体内引用外层局部（函数帧）时按全局名解析 ⇒ 运行期
#     「未定义变量: k」或错值。
#   · 缺陷 164（C 轨，本轮加强面抓到）：同帧里该局部**已被闭包按引用捕获**（⇒ 已装箱
#     为 cell）时，生成器的捕获直接**共享该 cell** ⇒ 外层之后的修改对生成器可见
#     （实测 co_live：解释/VM 轨 [7,2]，C 轨 [7,8]）。
# 修法（三轨**同一条真相：按值快照** —— 与解释轨「创建时求值」对齐）：
#   · VM 轨：`bc_genexp_caps` 求捕获表（自由变量 ∩ 本帧局部名，顺序同 cg_closure_caps）；
#     非空 ⇒ MKCLO，捕获槽由 `bc_emit_caps_snapshot` 造：先 CELLGET（cell 源取当前值）
#     / MOV（普通槽复制值）到连续临时槽，再逐个 CELLNEW **造新 cell**。空捕获 ⇒ 原 LOADK
#     路径不变（不回归）。
#   · C 轨：`cg_gen_lambda` 的 cell 源改为 `px_cell(px_cell_get(cv))`（取当前值 → 新 cell），
#     与 VM 轨的 CELLGET+CELLNEW 对称。**闭包**（cg_gen_closure）仍按引用，不动。
# 判据（逐层可单独变红）：
#   ① 解释轨主用例 pass=13 fail=0（绝对值断言）；
#   ② VM 轨（默认轨）编译+运行 ⇒ stdout 与解释轨**逐字节一致**；
#   ③ C 轨（逃生舱）⇒ stdout 与解释轨**逐字节一致**；
#   ④ 加强面（多捕获/形参来源/循环多实例独立快照/引用类型值快照/闭包共存）三轨一致；
#   ⑤ 缺口最小复现器 genexp_local.px（缺陷 163/164）三轨一致 —— M160 门第 ⑥ 层由此
#      **升格**为硬判据（原为 SKIP 报告）；
#   ⑥ 缺口登记（**不计失败**，但必须打印编号）；
#   ⑦ 负控 3 道（默认跑，`--neg-skip` 跳过）—— 每道必须判红 + 逐字节还原：
#      A VM 轨：生成器捕获表置空（恢复缺陷 163）⇒ 主用例必红
#      B VM 轨：快照退化为「把快照值槽当 cell 交给 MKCLO」⇒ 必红
#      C VM 轨：filter 的捕获表丢弃（只留 transform）⇒ 主用例必红
# 用法：./examples/m161_genexp_capture/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m161_genexp_capture/verify.sh --neg-skip （只跑正判据，CI 用）
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
CASE=$HERE/genexp_capture.px
EDGE=$HERE/genexp_capture_edge.px
LOCAL=$HERE/genexp_local.px
WORK=$(mktemp -d /tmp/m161.XXXXXX)
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

hdr "[1/4] 解释轨 + VM 轨 + C 轨：生成器捕获主用例（13 断言）"
three_tracks "$CASE" main
expect_all_three main "pass=13 fail=0"

hdr "[2/4] 加强面：多捕获/形参/循环多实例/引用类型快照/闭包共存（6 断言）"
three_tracks "$EDGE" edge
expect_all_three edge "pass=6 fail=0"

hdr "[3/4] 交叉判据：缺陷 163/164 最小复现器（genexp_local.px）三轨一致"
three_tracks "$LOCAL" local
expect_all_three local "pass=5 fail=0"

hdr "[4/4] 缺口登记（不计失败）"
note "语义登记：生成器捕获 = **按值快照**（解释轨创建时求值 / C 轨 cg_gen_lambda / VM 轨 bc_emit_caps_snapshot）"
note "缺陷 165（待下一轮）：GenExp **惰性路径**已定按值快照，但 M32 **物化路径**（多 for /"
note "  多变量）与 ListComp/DictComp 的捕获语义尚未逐条核查"

# ---------------- 负控 ----------------
run_neg() {              # $1=标签 $2=引擎(c|vm) $3=文件 $4=旧文本 $5=新文本 $6=说明 $7=用例
    local tag=$1 eng=$2 file=$3 from=$4 to=$5 what=$6 case=${7:-$CASE}
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
            PX_PXC_BIN="$WORK/pxc_neg" three_tracks "$case" neg$tag
        else
            PXC_VM_BIN="$WORK/pxc_vm_neg" three_tracks "$case" neg$tag
        fi
        local out="$WORK/neg$tag.$eng.out"
        if [ "$eng" = "c" ]; then out="$WORK/neg$tag.c.out"; else out="$WORK/neg$tag.vm.out"; fi
        local want="pass=13 fail=0"
        [ "$case" = "$EDGE" ] && want="pass=6 fail=0"
        if grep -q "^$want\$" "$out"; then
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
    run_neg A vm selfhost/bc_emit.px \
        '        if not contains(param_names, cn) and func["smap"].has(cn):
            caps.append(cn)' \
        '        if false and not contains(param_names, cn) and func["smap"].has(cn):
            caps.append(cn)' \
        "生成器捕获表置空（恢复缺陷 163）"
    # ⚠️ M164（第 50 轮）改动 bc_genexp_caps / bc_emit_genexp 后**本门两条锚点已随之更新**：
    #   A 从旧形态（`caps.append(cn)` + `i += 1`）改为新形态（`if not contains(param_names, cn) …`）；
    #   C 从 `bc_genexp_caps(pms, expr[3], func)` 改为 `bc_genexp_caps([vn], expr[3], func)`
    #   （M164 缺陷 175 把参数从 Param 节点改成平铺形参名表）。
    #   教训（R47 已记过一次，本轮再次撞上）：**改源码前先 grep 全仓的负控锚点**，否则门会
    #   「锚点失效」判红 —— 那是门坏了，不是代码坏了，但红是一样的红。
    run_neg B vm selfhost/bc_emit.px \
        '    return snap_base' \
        '    return vb' \
        "快照退化为「把快照值槽当 cell 交给 MKCLO」"
    run_neg C vm selfhost/bc_emit.px \
        '            fcaps = bc_genexp_caps([vn], expr[3], func)' \
        '            fcaps = []' \
        "filter 的捕获表丢弃（只留 transform）"
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
    echo "M161-VERIFY-OK（生成器捕获：三轨一致 · 按值快照；缺陷 163 + 164 收口）"
else
    echo "M161-VERIFY-FAIL"
fi
exit $fail
