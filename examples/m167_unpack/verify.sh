#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M167 门（第 53 轮）：**解包（destructuring）统一**（缺陷 180 / 182）
# ---------------------------------------------------------------
# 本轮两件事：
#   ① **新语法**：语句形式 `for a, b[, c…] in xs:`（与推导式同语义：逐元素**解包**）。
#      Go 的 `for k, v := range m` 是最常用迭代形式，而本语言此前**只有推导式**支持解包
#      （M30）—— 语句形式 parser 直接 E2001 ⇒ 语句/推导式不对等。
#   ② **解包诊断统一**（修前**三轨四种行为**，同一份源码）：
#      · 元素非 list/tuple（`[k for k, v in d]` 的元素是字符串键）：
#          解释轨 `R1002 推导式解包需要 list/tuple…`、VM/C 轨 `R1003 字符串索引越界: 1`
#      · 元素长度不足（`[a for a, b in [[1]]]`）：
#          解释轨**静默给 null**（错值）、VM 轨 `列表索引越界: 1 (len=1)`（无 R1003 前缀）、
#          C 轨 `R1003: 列表索引越界: 1 (len=1)`
#      · 元素长度超出：三轨都**静默忽略**多出的元素（静默数据丢失）
# 语义裁定（三轨**一条真相**）：
#   `for a, b[, c…] in xs` / `[a for a, b in xs]` —— 变量个数 N ≥ 2 时，被解包值必须是
#   **list/tuple 且长度恰为 N**；否则 R1002 同码同文：
#     · 非 list/tuple → `R1002 解包需要 list/tuple，实际是 <t>`
#     · 长度 ≠ N      → `R1002 解包需要 N 个元素，实际是 M`
#   为什么严格（不做「缺位给 null」也不做「多的忽略」）：① 缺位给 null 是**静默的错值**
#   （M116/M120 静默 null 族、M163 字典键严格化、M166 长度变化报错 —— 同一哲学：响亮优于静默）；
#   ② 三轨本已四种行为，只有「严格」能收敛为**一条真相**；③ 与 Python 的解包
#   （`ValueError: not enough values to unpack` / `too many values to unpack`）同向。
#   ④ `dict` **直接迭代只产键**（字符串）⇒ `for k, v in d` 会响亮报「需要 list/tuple」；
#      要「键+值」写 **`for k, v in d.items()`**（本轮新增 `dict.items()`，
#      返回 `[[k, v], …]` 插入序**快照**，与 keys()/values() 同口径）——
#      即 Go `for k, v := range m` 的对应写法。
# 实现：
#   · parser：`parse_for` 收 `IDENT (',' IDENT)*`；AST 里**单变量仍是字符串**（既有
#     `cases/*.ast` golden 逐字节不变），多变量是**名字列表**（LAYOUT kind `vs` → astdump 渲染）
#   · runtime：**`px_unpack_ck(item, n)`**（类型 + 长度严格校验）+ **`PXOP_UNPACKCK`**(65)
#     （`vm.h` 的 `PXM_MAX` 65→66、`vm.c` 指令名表同步）+ **`dict.items()`**（px_method）
#   · VM 轨（`bc_emit`）：for 多变量分支先 `ITERAT item` → **UNPACKCK** → 逐字段 INDEX
#   · C 轨（`cg_stmt` / `cg_expr`）：for / 推导式发射 `px_unpack_ck(item, N);` 再 `px_index`；
#     多变量的 item 临时槽用**专用计数器** `_upN`（`cg_unpack_tmp`，不进 `cg_tmp` 序列）
#   · 解释轨（`istmt` / `iexpr` / `icall`）：`i_exec_for` 走 `i_unpack_bind`（推导式
#     `i_bind_comp_vars` 同源 ⇒ 两处一条真相）+ `i_dict_method` 的 `items`
#   · 顺带收口**缺陷 182**（VM 轨 · 循环变量被闭包捕获）：装箱循环变量必须走 `CELLSET`
#     而非直写槽（直写会把 cell 冲掉，闭包构造读到的就是裸值 ⇒ `R1002 此类型不支持索引: int`；
#     值为字符串时 `px_index("z",0)` 恰好返回 "z" ⇒ **静默碰对**，更难发现）。未装箱名字
#     保持直写 ⇒ 发射文本与既有 golden 逐字节一致。
# 判据（逐层可单独变红）：
#   ① 合法侧主用例 **16 断言**（list/tuple 解包、三变量、`d.items()`、嵌套、break/continue、
#      函数内循环后保留最后值、单变量与推导式回归、函数内闭包按引用捕获解包变量）；
#   ② VM 轨（用户面默认轨）与 ③ C 轨：stdout 与解释轨**逐字节一致**（`pass=16 fail=0`）；
#   ④ **严格性层**：**8 类**用例 × 三轨，每轨必须 rc≠0 + 含 R1002 + 含**统一词条**
#      （非 list/tuple ×2、长度不足 ×2、长度超出 ×2、三变量不足、dict 直接迭代）；
#   ⑤ **负控 3 道**（默认跑，`--neg-skip` 跳过）—— 每道必须判红 + 逐字节还原：
#      A VM 轨 `bc_emit` 把 UNPACKCK 换成 MOV（无校验）⇒ 长度不足落到 INDEX ⇒ 报 R1003（不是 R1002）
#      B C 轨 `cg_stmt` 去掉 `px_unpack_ck` 发射 ⇒ 同上
#      C 解释轨 `i_unpack_bind` 类型检查放行 ⇒ 解包照常进行（**静默成功**，不再报统一词条）
#        —— 即把「响亮」退回「静默」；证明**这条检查**正是严格性层判红的原因
#        （修前解释轨报的是另一条词条「推导式解包需要…」，此处取更强的反例）
# 用法：./examples/m167_unpack/verify.sh            （完整门：正判据 + 严格性 + 负控）
#       ./examples/m167_unpack/verify.sh --neg-skip （只跑正判据，CI 用）
# 退出码：0 = 绿，1 = 红。
# 备注：负控机制 = 「改 `selfhost/` 源 → 重建 dev 件（`selfhost/devbuild.sh`，产物 /tmp/pxcdev*、
#   /tmp/pxidev，不碰入库件）→ 跑 → 逐字节还原」。⚠️ 每道负控前先 `restore_all` 再 `snapshot`
#   （各自独立、干净起点；M165 教训），退出 trap 里也 `restore_all`（中途被杀也能还原）。
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
WORK=$(mktemp -d /tmp/m167.XXXXXX)
trap 'restore_all 2>/dev/null; rm -rf "$WORK"' EXIT

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

# 三轨跑一个用例：$1=源文件 $2=标签
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

# 严格性判据：三轨都必须 rc≠0 + 含 R1002 + 含统一词条 $2（统一词条在**每轨**都要出现）
err_case() {             # $1=标签 $2=词条
    local tag=$1 text=$2 t rc
    for t in interp vm c; do
        rc=$(cat "$WORK/$tag.$t.rc")
        if [ "$rc" = "0" ]; then
            bad "$tag $t 轨**未报错**（rc=0）—— 严格性缺失（静默错值）"
            sed 's/^/      /' "$WORK/$tag.$t.out" | head -4
            continue
        fi
        if ! grep -q "R1002" "$WORK/$tag.$t.out"; then
            bad "$tag $t 轨缺 R1002"; sed 's/^/      /' "$WORK/$tag.$t.out" | head -4; continue
        fi
        if ! grep -qF "$text" "$WORK/$tag.$t.out"; then
            bad "$tag $t 轨词条不符（应含「$text」）"
            sed 's/^/      /' "$WORK/$tag.$t.out" | head -4; continue
        fi
        note "$tag $t 轨 rc=$rc · $(sed -n '1p' "$WORK/$tag.$t.out" | tr -d '\n')"
    done
}

# ════ 正判据 ① ════
hdr "[1/3] 合法侧主用例（16 断言：解包 / 三变量 / items / 嵌套 / break-continue / 循环后保留 / 闭包）"
run_three "$HERE/unpack_ok.px" ok
expect_all_three ok "pass=16 fail=0"

# ════ 严格性 ④ ════
hdr "[2/3] 严格性：8 类用例 × 三轨（rc≠0 + R1002 + 统一词条）"
run_three "$HERE/err_notype.px" e_notype;      err_case e_notype      "解包需要 list/tuple，实际是 string"
run_three "$HERE/err_short.px" e_short;        err_case e_short       "解包需要 2 个元素，实际是 1"
run_three "$HERE/err_long.px" e_long;          err_case e_long        "解包需要 2 个元素，实际是 3"
run_three "$HERE/err_three.px" e_three;        err_case e_three       "解包需要 3 个元素，实际是 2"
run_three "$HERE/err_dict.px" e_dict;          err_case e_dict        "解包需要 list/tuple，实际是 string"
run_three "$HERE/err_comp_notype.px" e_cn;     err_case e_cn          "解包需要 list/tuple，实际是 string"
run_three "$HERE/err_comp_short.px" e_cs;      err_case e_cs          "解包需要 2 个元素，实际是 1"
run_three "$HERE/err_comp_long.px" e_cl;       err_case e_cl          "解包需要 2 个元素，实际是 3"

if [ "$NEG_SKIP" = "1" ]; then
    echo; echo "── 负控已跳过（--neg-skip）"; echo
    [ "$fail" = "0" ] && echo "M167-VERIFY-OK（无负控）" && exit 0
    echo "M167-VERIFY-FAILED"; exit 1
fi

# ════ 负控 ⑤ ════
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
        cmp -s "$f" "$b" || echo "❌ 还原失败：$f"
        i=$((i + 1))
    done
    BAKFILES=(); BAKBLOB=()
}

# 负控跑法：改源 → 重建 dev 件 → 用该轨跑指定用例 → **必须判红**（不再 R1002 同码同文）→ 还原
run_neg() {              # $1=标签 $2=文件 $3=旧 $4=新 $5=devbuild 参数 $6=轨 $7=用例 $8=说明
    local tag=$1 file=$2 old=$3 new=$4 devargs=$5 eng=$6 uc=$7 what=$8
    echo "── 负控 $tag：$what"
    restore_all                      # 每道负控都从**干净源码**出发（各自独立）
    snapshot "$file"
    if ! patch_one "$file" "$old" "$new"; then
        bad "负控 $tag 锚点失效（与源码不同步）"; restore_all; return
    fi
    if ! ./selfhost/devbuild.sh $devargs > "$WORK/neg_$tag.dev.log" 2>&1; then
        bad "负控 $tag devbuild 失败"; tail -5 "$WORK/neg_$tag.dev.log" | sed 's/^/      /'; restore_all; return
    fi
    local w="$WORK/w_$tag"
    rm -rf "$w"; mkdir -p "$w"
    cp "$uc" "$w/$tag.px"
    local rc=99
    case "$eng" in
        vm)
            if ( cd "$w" && PXC_VM_BIN=/tmp/pxcdev_vm timeout 900 "$PX" build "$tag.px" > "$WORK/neg_$tag.build.log" 2>&1 && [ -x "build/$tag" ] ); then
                ( cd "$w" && timeout 120 "./build/$tag" > "$WORK/neg_$tag.out" 2>&1 ); rc=$?
            else
                bad "负控 $tag VM 轨构建失败"; tail -5 "$WORK/neg_$tag.build.log" | sed 's/^/      /'; restore_all; return
            fi
            ;;
        c)
            if ( cd "$w" && PX_PXC_BIN=/tmp/pxcdev PX_BUILD_ENGINE=c timeout 900 "$PX" build --c "$tag.px" > "$WORK/neg_$tag.build.log" 2>&1 && [ -x "build/$tag" ] ); then
                ( cd "$w" && timeout 120 "./build/$tag" > "$WORK/neg_$tag.out" 2>&1 ); rc=$?
            else
                bad "负控 $tag C 轨构建失败"; tail -5 "$WORK/neg_$tag.build.log" | sed 's/^/      /'; restore_all; return
            fi
            ;;
        interp)
            ( cd "$w" && timeout 120 /tmp/pxidev "$tag.px" > "$WORK/neg_$tag.out" 2>&1 ); rc=$?
            ;;
    esac
    echo "      负控结果 rc=$rc · $(sed -n '1p' "$WORK/neg_$tag.out" | tr -d '\n')"
    if [ "$rc" = "0" ]; then
        note "判红 ✅（$eng 轨**静默成功** = 缺陷原样）"
    elif grep -qF "解包需要" "$WORK/neg_$tag.out"; then
        bad "负控 $tag **未判红**：仍报告统一词条「解包需要…」—— 该轨的修复未生效/未编译进去"
    else
        note "判红 ✅（$eng 轨改报其它错误 = 缺陷原样）"
    fi
    restore_all
}

hdr "[3/3] 负控 3 道（改源 → 重建该轨 dev 件 → 必须判红 → 逐字节还原）"
run_neg A selfhost/bc_emit.px \
    'bc_emit_inst(func, "UNPACKCK", item, len(names), 0)' \
    'bc_emit_inst(func, "MOV", item, item, 0)' \
    "pxc --vm" vm "$HERE/err_short.px" \
    "VM 轨去掉 UNPACKCK 校验（长度不足落到 INDEX ⇒ R1003 越界，不再是 R1002）"
run_neg B selfhost/cg_stmt.px \
    's += pad + "    px_unpack_ck(" + item_var + ", " + str(upn) + ");\n"' \
    's += pad + ""' \
    "pxc" c "$HERE/err_short.px" \
    "C 轨去掉 px_unpack_ck 发射（同 A）"
run_neg C selfhost/iexpr.px \
    'if t != "list" and t != "tuple":' \
    'if false:' \
    "pxi" interp "$HERE/err_comp_notype.px" \
    "解释轨 i_unpack_bind 类型检查放行（元素是字符串也照常解包 ⇒ 静默成功，不再报统一词条）"

echo
if [ "$fail" = "0" ]; then
    echo "M167-VERIFY-OK"
    exit 0
fi
echo "M167-VERIFY-FAILED"
exit 1
