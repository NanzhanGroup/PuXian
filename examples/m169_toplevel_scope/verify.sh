#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M169 门（第 54 轮）：**模块体 / 闭包帧的「绑定归属」统一**（缺陷 181）
# ---------------------------------------------------------------
# 语义裁定（三轨一条真相，参考 = 解释轨）：
#   ① **模块体（顶层，含嵌套块）里的所有绑定 = 模块级全局**：循环变量、if/while
#      体内的 let/var、嵌套 def、chan 一律如此 —— 模块体**没有块作用域**。
#   ② 顶层闭包/函数捕获这些绑定 ⇒ **按引用**（env 链语义）。
#   ③ **闭包体（lambda 帧）也是帧**：体内绑定提升到帧顶、体内被内层闭包捕获的局部
#      先装箱成 cell（与函数体同口径；VM 轨 M160 已有，C 轨此前两者都缺）。
#   ④ 帧内**声明式**绑定 ⇒ 帧局部（遮蔽同名模块全局）；帧内**赋值式**绑定（无声明）
#      ⇒ 命中模块绑定则写模块全局，否则也是帧局部。
# 修前实测（同一份源码三轨四种行为，逐条已复现）：
#   · `for p in [1,2,3]: 0` 后 `print(p)`            → 解释/VM ok · **C 编译失败**
#     （error: '_v3' undeclared —— 循环变量被就地声明在 C 的 for 块里）
#   · `if true: let x = 42` 后 `print(x)`            → 解释/VM ok · **C error: '_v1' undeclared**
#   · `for i in [7,8]: 0` 后 `def f(): return i`     → 解释 8 · **VM/C「未定义变量: i」**
#   · 顶层闭包捕获顶层循环变量                        → 解释/VM 20/20（引用）· **C 10/20（按值）**
#   · `if true: def h(): 41` 后由同模块函数调用 h()   → 解释 42 · **VM/C「未定义变量: h」**
#   · `fn (): var s=0; for k in …: s += k; return s + k` → 解释/VM 9 · **C error: '_v4' undeclared**
#   · `fn (): var c=0; let g = fn (): c = c+1; …`     → 解释/VM [1,2,2] · **C [1,2,0]**
#   · `var g=1` + `def f(): if true: var g=5; return g` → 解释/VM 5 · **C error: '_v2' undeclared**
#   · `fn (): xx = 5` + `def g(): return xx`          → 解释「未定义变量」· **VM/C 全局写（r2=5）**
# 实现（全部落在两轨的**帧/模块归属**判据上；不改优化、不改 runtime）：
#   · codegen.px：新增 `cg_topbody`（模块体标志）与 `cg_topnames`（模块绑定名集合）；
#     新增 `cg_collect_decl_vars`（**声明式**绑定收集，与 cg_collect_hoist_vars 同递归口径）
#   · cg_stmt.px：VarDecl / For 循环变量 / 嵌套 def / ChanDecl 在模块体一律 `px_set_global`
#     （不再分配 `_vN` 帧局部；仍推进 cg_uid ⇒ 编号与改前对齐，重定基差异只在本族）
#   · cg_expr.px：闭包体补 **hoist 预扫描 + 装箱预扫描**（对齐 VM 的 bc_lambda_hoist/bc_box_frame）
#   · bc_emit.px：`bc_emit_for` 的 `is_top` 分支改 **SETG**；`bc_emit_stmt` 的嵌套 FuncDef
#     在 `is_top` 改 SETG；新增 `g_topnames` 并把三处「帧局部判定」从惰性 `bc_is_global`
#     换成确定性的模块绑定集（顺序无关 —— 这正是 err_lambda_frame_local 的分叉根因）
# 判据（逐层可单独变红）：
#   ① 主用例 17 断言 × 三轨 stdout **逐字节一致**（`pass=17 fail=0`）；
#   ② 严格性 3 用例 × 三轨：rc≠0 + stderr 含「未定义变量」+ **三轨 stdout 逐字节一致**
#      （⚠️ 错误**文本格式**的既有差异不在本门范围：解释轨 `错误 [R1001] 行:列: 未定义变量: 'x'`
#        与编译轨 `运行时错误 [fn 行N]: 未定义变量: x` 的措辞/引号不同 —— 本门只比 stdout，
#        并要求三轨都能**响亮报错**、语义同一条；格式统一另案）
#   ③ 负控 4 道（默认跑，`--neg-skip` 跳过）—— 改源 → 重建该轨 dev 件 → 必须判红 → 逐字节还原：
#        A VM 轨 `bc_emit_for` 的 `is_top` 分支退回帧局部槽 ⇒ ⑤/⑥ 走全局读 ⇒ 未定义变量/R1002
#        B C 轨模块体循环变量退回「就地声明」⇒ ① 循环后引用 C undeclared
#        C C 轨闭包体退回「不 hoist / 不装箱」⇒ ⑧⑨ 编译失败/引用语义错
#        D C 轨帧内 hoist 判据退回旧口径（名字在 cg_globals 就不提升）⇒ ⑩ 遮蔽用例 C undeclared
# 用法：./examples/m169_toplevel_scope/verify.sh            （完整门）
#       ./examples/m169_toplevel_scope/verify.sh --neg-skip （只跑正判据+严格性，CI 用）
# 退出码：0 = 绿，1 = 红。
# 备注：负控机制 = 「改 `selfhost/` 源 → 重建 dev 件（`selfhost/devbuild.sh`，产物
#   /tmp/pxcdev*，不碰入库件）→ 跑 → 逐字节还原」。⚠️ 每道负控前先 `restore_all` 再
#   `snapshot`（各自独立、干净起点；M165/M167 教训）；退出 trap 里也 `restore_all`。
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
PXVM="${PXC_VM_BIN:-$ROOT/bootstrap/pxc_vm}"
WORK=$(mktemp -d /tmp/m169.XXXXXX)
trap 'restore_all 2>/dev/null; rm -rf "$WORK"' EXIT

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

# 三轨跑一个用例：$1=源文件 $2=标签（stdout/stderr **分开**收：错误文本格式三轨本就不全同）
run_three() {
    local case=$1 tag=$2
    cp "$case" "$WORK/$tag.px"
    ( cd "$WORK" && timeout 120 "$PXI" "$tag.px" > "$tag.interp.out" 2> "$tag.interp.err" ); echo $? > "$WORK/$tag.interp.rc"
    rm -rf "$WORK/build"
    if ( cd "$WORK" && PXC_VM_BIN="$PXVM" timeout 900 "$PX" build "$tag.px" > "$tag.vm.log" 2>&1 && [ -x "build/$tag" ] ); then
        ( cd "$WORK" && timeout 120 "./build/$tag" > "$tag.vm.out" 2> "$tag.vm.err" ); echo $? > "$WORK/$tag.vm.rc"
    else
        : > "$WORK/$tag.vm.out"; : > "$WORK/$tag.vm.err"; echo 99 > "$WORK/$tag.vm.rc"
        note "VM 轨编译失败（日志 $WORK/$tag.vm.log）"
    fi
    rm -rf "$WORK/build"
    if ( cd "$WORK" && PX_PXC_BIN="$PXC" PX_BUILD_ENGINE=c timeout 900 "$PX" build --c "$tag.px" > "$tag.c.log" 2>&1 && [ -x "build/$tag" ] ); then
        ( cd "$WORK" && timeout 120 "./build/$tag" > "$tag.c.out" 2> "$tag.c.err" ); echo $? > "$WORK/$tag.c.rc"
    else
        : > "$WORK/$tag.c.out"; : > "$WORK/$tag.c.err"; echo 99 > "$WORK/$tag.c.rc"
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
            grep -E "^FAIL|错误|error" "$WORK/$tag.$t.out" "$WORK/$tag.$t.err" | head -6 | sed 's/^/      /'
        fi
        cmp -s "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" \
            || { bad "$tag $t 轨 stdout 与解释轨不一致"; diff "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" | head -6 | sed 's/^/      /'; }
    done
}

# 严格性判据：三轨都必须 rc≠0 + 含「未定义变量」+ **程序自身输出（去掉错误行）逐字节一致**
#   ⚠️ 两个**既有**差异不在本门范围（都不影响本条语义，也不由本轮引入）：
#     ① 通道：解释轨把运行期错误写 **stdout**，编译轨写 stderr；
#     ② 措辞：解释轨 `错误 [R1001] 行:列: 未定义变量: 'x'` vs 编译轨 `运行时错误 [fn 行N]: 未定义变量: x`。
#   ⇒ 本门对「程序自身输出」比字节（把错误行滤掉），对「响亮报错」用 rc + 关键字判。
err_case() {             # $1=标签 $2=用例名（用于说明）
    local tag=$1 what=$2 t rc
    for t in interp vm c; do
        rc=$(cat "$WORK/$tag.$t.rc")
        if [ "$rc" = "0" ]; then
            bad "$tag[$what] $t 轨**未报错**（rc=0）—— 帧内绑定泄漏到模块体（静默错值）"
            sed 's/^/      /' "$WORK/$tag.$t.out" | head -4
            continue
        fi
        if ! grep -q "未定义变量" "$WORK/$tag.$t.err" "$WORK/$tag.$t.out"; then
            bad "$tag[$what] $t 轨未报「未定义变量」"; sed 's/^/      /' "$WORK/$tag.$t.err" | head -4; continue
        fi
        grep -v -e "未定义变量" -e "运行时错误" -e "^错误" "$WORK/$tag.$t.out" > "$WORK/$tag.$t.clean"
        note "$tag[$what] $t 轨 rc=$rc · $(grep -h -m1 未定义变量 "$WORK/$tag.$t.err" "$WORK/$tag.$t.out" | tr -d '\n')"
    done
    for t in vm c; do
        cmp -s "$WORK/$tag.interp.clean" "$WORK/$tag.$t.clean" \
            || { bad "$tag[$what] $t 轨**程序自身输出**与解释轨不一致"; diff "$WORK/$tag.interp.clean" "$WORK/$tag.$t.clean" | head -6 | sed 's/^/      /'; }
    done
}

# ════ 正判据 ① ════
hdr "[1/3] 主用例（17 断言：模块体循环变量 / 嵌套块 / 函数可见 / 闭包按引用 / 闭包帧 / 遮蔽）"
run_three "$HERE/toplevel_scope.px" ok
expect_all_three ok "pass=17 fail=0"
for t in interp vm c; do
    [ "$(cat "$WORK/ok.$t.rc")" = "0" ] || bad "ok $t 轨 rc≠0"
done

# ════ 严格性 ② ════
hdr "[2/3] 严格性：帧内「赋值式」绑定不得泄漏到模块体（3 用例 × 三轨）"
run_three "$HERE/err_frame_local.px" e_funclocal;  err_case e_funclocal  "函数帧局部"
run_three "$HERE/err_lambda_frame_local.px" e_lambda; err_case e_lambda "闭包帧局部"
run_three "$HERE/err_no_iteration.px" e_noiter;    err_case e_noiter    "零迭代未绑定"

if [ "$NEG_SKIP" = "1" ]; then
    echo; echo "── 负控已跳过（--neg-skip）"; echo
    [ "$fail" = "0" ] && echo "M169-VERIFY-OK（无负控）" && exit 0
    echo "M169-VERIFY-FAILED"; exit 1
fi

# ════ 负控 ③ ════
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
snapshot() {             # $1=文件 ← 每次快照前先清表（M165 教训：无备份别还原）
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

# 负控跑法：改源 → 重建 dev 件 → 用该轨跑主用例 → **必须与解释轨参考不一致（判红）** → 还原
run_neg() {              # $1=标签 $2=文件 $3=旧 $4=新 $5=devbuild 参数 $6=轨 $7=说明
    local tag=$1 file=$2 old=$3 new=$4 devargs=$5 eng=$6 what=$7
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
    cp "$HERE/toplevel_scope.px" "$w/$tag.px"
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
                # C 轨构建失败本身就是判红（缺陷原样：编译不过）
                echo "      负控结果 rc=99（C 轨构建失败 = 缺陷原样）"
                if grep -qE "undeclared|error" "$WORK/neg_$tag.build.log"; then
                    note "判红 ✅（C 轨 undeclared/error = 缺陷原样）"
                else
                    bad "负控 $tag 构建失败但不是缺陷原样的错误"; tail -5 "$WORK/neg_$tag.build.log" | sed 's/^/      /'
                fi
                restore_all; return
            fi
            ;;
    esac
    echo "      负控结果 rc=$rc · $(sed -n '1p' "$WORK/neg_$tag.out" | tr -d '\n')"
    if [ "$rc" != "0" ]; then
        note "判红 ✅（该轨 rc≠0 = 缺陷原样）"
    elif ! cmp -s "$WORK/neg_$tag.out" "$WORK/ok.interp.out"; then
        note "判红 ✅（stdout 与解释轨参考不一致 = 缺陷原样）"
        diff "$WORK/ok.interp.out" "$WORK/neg_$tag.out" | head -6 | sed 's/^/      /'
    else
        bad "负控 $tag **未判红**：修补后仍与解释轨逐字节一致 —— 该判据未真正覆盖本修复"
    fi
    restore_all
}

hdr "[3/3] 负控 4 道（改源 → 重建该轨 dev 件 → 必须判红 → 逐字节还原）"
run_neg A selfhost/bc_emit.px \
    'let top = func["is_top"]' \
    'let top = false' \
    "pxc --vm" vm \
    "VM 轨顶层 for 的循环变量退回帧局部槽 ⇒ 函数/闭包按全局读 ⇒ 未定义变量"
run_neg B selfhost/cg_stmt.px \
    'decls.append("px_set_global(\"" + nm + "\", " + fv_init + ");")' \
    'decls.append("LXValue " + cg_new_var(nm) + " = " + fv_init + ";")' \
    "pxc" c \
    "C 轨模块体循环变量退回「就地声明」⇒ 循环后引用 = C undeclared"
run_neg C selfhost/cg_expr.px \
    'if cg_var_of(lnm) == null and not (not contains(ldeclonly, lnm) and contains(cg_topnames, lnm)):' \
    'if false:' \
    "pxc" c \
    "C 轨闭包体退回「不 hoist / 不装箱」⇒ 闭包帧用例编译失败/引用语义错"
run_neg D selfhost/codegen.px \
    'if not contains(decl_names, name) and contains(cg_topnames, name):' \
    'if contains(cg_globals, name):' \
    "pxc" c \
    "C 轨帧内 hoist 判据退回旧口径（名字在 cg_globals 就不提升）⇒ 声明遮蔽模块全局时 C undeclared"

echo
if [ "$fail" = "0" ]; then
    echo "M169-VERIFY-OK"
    exit 0
fi
echo "M169-VERIFY-FAILED"
exit 1
