#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M165 门（第 51 轮）：**C 轨求值顺序 = 词法左→右**（缺陷 178 / 179）
# ---------------------------------------------------------------
# 缺陷 178（C 轨 · 实参求值顺序）
#   C 标准**不规定**函数实参求值顺序，gcc 实测为**右→左**；解释轨（AST 序）与 VM 轨
#   （字节码序）都是左→右 ⇒ 一个 C 表达式里 ≥2 个带副作用的操作数就三轨分叉：
#     `f() + g()`                  → 解释/VM 轨 F,G；**C 轨 G,F**
#     `"${f()}${g()}"`             → 同上
#     `str(l.pop()) + str(len(l))` → C 轨 len 先求 ⇒ 打印 `len=3`（解释/VM 轨 `len=2`）
# 缺陷 179（解释轨 · 赋值求值顺序）
#   `o[k] = v`：解释轨**先算右值**（V,O,K），VM 轨 O,K,V；`o.f = v`：解释轨 V,O，VM 轨 O,V。
# 修法（对齐 **Go 规范**：表达式 / 赋值 / 实参按**词法左→右**求值；C 未定义）：
#   · C 轨：`cg_side_effect` + `cg_seq_join` + `cg_seq_wrap` —— 一个 C 表达式里 ≥2 个
#     含副作用的操作数按序提升为临时变量（语句表达式 `({ LXValue _tN = …; <式>; })`），
#     纯操作数内联 ⇒ 求值顺序**由语法保证**，不再依赖编译器实现。接线点：Binary / Index /
#     Slice / List / Tuple / Call（普通、方法、impl 静态、FFI、结构体、枚举） / Pipe /
#     赋值（Index / Field / `<-`） / spawn。
#     ⚠️ 短路运算（`and` / `or` / `??` / `?:`）**不接线** ⇒ 右操作数仍不提前求值。
#   · 解释轨：`i_assign` 改收**右值 AST 节点**，按「目标对象 → 键 → 右值」求值。
# 判据（逐层可单独变红）：
#   ① 解释轨 24 条断言（副作用日志直接观测求值顺序；全为绝对值断言）；
#   ② VM 轨（用户面默认轨）与 ③ C 轨：stdout 与解释轨**逐字节一致**（`pass=24 fail=0`）；
#   ④ **负控 4 道**（默认跑，`--neg-skip` 跳过）—— 每道必须判红 + 逐字节还原：
#      A `cg_seq_operands` 直接返回 `[null, null]`（关闭全部序列化 = 缺陷 178 原样）⇒ 11 项红
#      B Binary 站点退回不序列化 ⇒ 6 项红（add/sub/cmp/interp/concat/genexp）
#      C 方法站点退回不序列化 ⇒ `14-method-recv-arg` 红
#      D `cg_stmt` 索引赋值站点退回不序列化 ⇒ `13/18-index-set-order` 红
# 用法：./examples/m165_eval_order/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m165_eval_order/verify.sh --neg-skip （只跑正判据，CI 用）
# 退出码：0 = 绿，1 = 红。
# 备注：负控会改 `selfhost/` 源 → 机制是「改源 → 重建 dev 件 → 跑 → 逐字节还原」；
#   dev 件走 `selfhost/devbuild.sh`（产物 /tmp/pxcdev，不碰入库件）。
# ═══════════════════════════════════════════════════════════════════════
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT"
export LC_ALL=C LANG=C

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PXI="${PXI_BIN:-$ROOT/bootstrap/pxi}"
PX="$ROOT/tools/px"
PXC="${PX_PXC_BIN:-$ROOT/bootstrap/pxc}"
WORK=$(mktemp -d /tmp/m165.XXXXXX)
# 退出时清临时目录；若负控中途被杀，**顺带把源码还原**（防「改源未还原」残留）
trap 'restore_all 2>/dev/null; rm -rf "$WORK"' EXIT

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

# 三轨跑一个用例：$1=源文件 $2=标签 → $WORK/<tag>.{interp,vm,c}.{out,rc}
run_three() {
    local case=$1 tag=$2
    cp "$case" "$WORK/$tag.px"
    ( cd "$WORK" && timeout 120 "$PXI" "$tag.px" > "$tag.interp.out" 2>&1 ); echo $? > "$WORK/$tag.interp.rc"
    rm -rf "$WORK/build"
    if ( cd "$WORK" && timeout 900 "$PX" build "$tag.px" > "$tag.vm.log" 2>&1 && [ -x "build/$tag" ] ); then
        ( cd "$WORK" && timeout 120 "./build/$tag" > "$tag.vm.out" 2>&1 ); echo $? > "$WORK/$tag.vm.rc"
    else
        : > "$WORK/$tag.vm.out"; echo 99 > "$WORK/$tag.vm.rc"
        note "VM 轨编译失败（日志 $WORK/$tag.vm.log）"
    fi
    rm -rf "$WORK/build"
    if ( cd "$WORK" && PX_PXC_BIN="$PXC" timeout 900 "$PX" build --c "$tag.px" > "$tag.c.log" 2>&1 && [ -x "build/$tag" ] ); then
        ( cd "$WORK" && timeout 120 "./build/$tag" > "$tag.c.out" 2>&1 ); echo $? > "$WORK/$tag.c.rc"
    else
        : > "$WORK/$tag.c.out"; echo 99 > "$WORK/$tag.c.rc"
        note "C 轨编译失败（日志 $WORK/$tag.c.log）"
    fi
    rm -rf "$WORK/build"
}

expect_all_three() {     # $1=标签 $2=期望行
    local tag=$1 want=$2 t
    if grep -q "^$want\$" "$WORK/$tag.interp.out"; then
        note "解释轨：$want"
    else
        bad "$tag 解释轨未达 $want"; sed 's/^/      /' "$WORK/$tag.interp.out" | head -8
    fi
    for t in vm c; do
        if grep -q "^$want\$" "$WORK/$tag.$t.out"; then
            note "$t 轨：$want"
        else
            bad "$tag $t 轨未达 $want"
            grep -E "^FAIL|错误" "$WORK/$tag.$t.out" | head -6 | sed 's/^/      /'
        fi
        cmp -s "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" \
            || { bad "$tag $t 轨 stdout 与解释轨不一致"; diff "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" | head -6 | sed 's/^/      /'; }
    done
}

# ════ 正判据 ════
hdr "[1/2] 求值顺序主用例（24 断言：二元链/调用/插值/字面量/索引/切片/赋值/方法/结构体/短路/推导式）"
run_three "$HERE/eval_order.px" main
expect_all_three main "pass=24 fail=0"

# ════ 负控 ════
patch_one() {            # $1=文件 $2=旧文本 $3=新文本 → rc≠0 表示锚点失效
    python3 - "$1" "$2" "$3" <<'PY'
import sys
p, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p, encoding="utf-8").read()
if s.count(a) != 1:
    print("锚点不唯一（%d）" % s.count(a)); sys.exit(1)
open(p, "w", encoding="utf-8").write(s.replace(a, b, 1))
PY
}

BAKFILES=(); BAKBLOB=()
snapshot() {             # $1=文件
    local b="$WORK/$(echo "$1" | tr '/' '_').bak"
    cp "$1" "$b"; BAKFILES+=("$1"); BAKBLOB+=("$b")
}
restore_all() {
    local i=0 f b
    while [ $i -lt ${#BAKFILES[@]} ]; do
        f="${BAKFILES[$i]}"; b="${BAKBLOB[$i]}"
        cp "$b" "$f"
        cmp -s "$f" "$b" || bad "还原失败：$f"
        i=$((i + 1))
    done
    BAKFILES=(); BAKBLOB=()
}

# 负控跑法：改源 → 重建 dev C 轨件 → 跑 C 轨 → 判红 → 逐字节还原
run_neg() {              # $1=标签 $2=文件 $3=旧 $4=新 $5=期望变红的断言数下限
    local tag=$1 file=$2 old=$3 new=$4 minhit=$5
    echo "── 负控 $tag"
    restore_all                      # 保证每道负控都从**干净源码**出发（各自独立）
    snapshot "$file"
    if ! patch_one "$file" "$old" "$new"; then
        bad "负控 $tag 锚点失效（与源码不同步）"; restore_all; return
    fi
    if ! ./selfhost/devbuild.sh > "$WORK/neg_$tag.dev.log" 2>&1; then
        bad "负控 $tag devbuild 失败"; tail -5 "$WORK/neg_$tag.dev.log" | sed 's/^/      /'; restore_all; return
    fi
    local w="$WORK/w_$tag"
    rm -rf "$w"; mkdir -p "$w"
    cp "$HERE/eval_order.px" "$w/eval_order.px"
    if ! ( cd "$w" && PX_PXC_BIN=/tmp/pxcdev timeout 900 "$PX" build --c eval_order.px > "$WORK/neg_$tag.c.log" 2>&1 && [ -x build/eval_order ] ); then
        bad "负控 $tag C 轨构建失败"; tail -5 "$WORK/neg_$tag.c.log" | sed 's/^/      /'; restore_all; return
    fi
    ( cd "$w" && timeout 120 ./build/eval_order > "$WORK/neg_$tag.out" 2>&1 )
    local nfail
    nfail=$(grep -c '^FAIL ' "$WORK/neg_$tag.out" || true)
    if grep -q '^fail=0$' "$WORK/neg_$tag.out"; then
        bad "负控 $tag 【未被判红】—— 本门判据对该接线点不敏感"
    elif [ "$nfail" -lt "$minhit" ]; then
        bad "负控 $tag 只红了 $nfail 项（期望 ≥$minhit）"
        grep '^FAIL ' "$WORK/neg_$tag.out" | head -6 | sed 's/^/      /'
    else
        note "负控 $tag 已判红（$nfail 项断言变红，例：$(grep -m2 '^FAIL ' "$WORK/neg_$tag.out" | tr '\n' ' '))✅"
    fi
    restore_all
}

if [ "$NEG_SKIP" = "1" ]; then
    hdr "[2/2] 负控 4 道（--neg-skip：跳过）"
    note "CI 模式：负控需重建 dev 件，交给本地完整门 selfhost/m116_gates.sh"
else
    hdr "[2/2] 负控 4 道（改源 → 重建 dev 件 → 跑 C 轨 → 判红 → 逐字节还原）"
    run_neg A selfhost/cg_expr.px 'def cg_seq_operands(srcs, imps):
    var n = 0' 'def cg_seq_operands(srcs, imps):
    return [null, null]
    var n = 0' 8
    run_neg B selfhost/cg_expr.px \
        '        let sj = cg_seq_join([expr[2], expr[3]], [l, r])' \
        '        let sj = [null, [l, r]]' 4
    run_neg C selfhost/cg_expr.px \
        '            let sj = cg_seq_join(nodes, parts)
            return cg_seq_wrap(sj[0], "px_method(' \
        '            let sj = [null, parts]
            return cg_seq_wrap(sj[0], "px_method(' 1
    run_neg D selfhost/cg_stmt.px \
        '            let sj = cg_seq_join([tgt[1], tgt[2], stmt[3]], [o, i, rhs])' \
        '            let sj = [null, [o, i, rhs]]' 2
    restore_all
    # 负控跑完后源码已逐字节还原；入库件不受影响（负控只用 /tmp 现场件），仍复核指纹门
    ./selfhost/rebake_bin.sh --check-all > "$WORK/rebake_check.log" 2>&1 \
        && note "入库件源码链指纹门复核 ✅（$(grep -c '✅' "$WORK/rebake_check.log") 件）" \
        || bad "入库件源码链指纹门复核失败（见 $WORK/rebake_check.log）"
fi

echo ""
if [ "$fail" = "0" ]; then
    echo "M165-VERIFY-OK"
else
    echo "M165-VERIFY-FAIL"
fi
exit $fail
