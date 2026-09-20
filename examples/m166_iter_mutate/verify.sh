#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M166 门（第 52 轮）：**迭代期间修改被迭代容器 ⇒ R1003**（缺陷 176）
# ---------------------------------------------------------------
# 缺陷 176（三轨**四种**行为，同一份源码）
#   · `for x in l { l.append(9) }`   解释/C 轨按**活长度**多迭代（泄漏新元素）；VM 轨按快照
#   · `for x in l { l.pop() }`       VM 轨**越界 rc=1**；解释/C 轨正常退出（静默跳过元素）
#   · `for k in d { d.set(...) }`    解释/VM 轨按**键快照**；C 轨按活长度（迭代到新键）
#   · `for k in d { d.remove(k) }`   VM 轨**越界 rc=1**；解释轨键快照；C 轨中途停
# 语义裁定（三轨**一条真相**）：for-in / 推导式在**进入循环时快照长度**；迭代期间被迭代容器
#   长度变化（**增或减**）一律
#       `R1003 迭代期间被迭代容器长度变化: n0 → n1`
#   为什么不是「活长度」（Python list 语义）或「静默跳过」：
#     · 三轨必须同一条真相 —— 活长度下「边遍历边删」的结果依赖容器实现，跨轨必然漂移；
#     · 静默跳过/越界给出的是**用户看不见的错结果**，与 R1008 严格口径、M163 键严格化同一哲学。
#   与 Python 对 dict 的 `RuntimeError: dictionary changed size during iteration` 同向，
#   只是把 list 也纳入（更安全：Python 的 list 会静默跳过元素）。
# 修法：
#   · runtime：新指令 **PXOP_ITERLEN**(64) + `px_iter_ck(obj, n0)`（长度 ≠ n0 ⇒ R1003）
#   · VM 轨：`bc_emit` 的 for / 推导式在循环体首发射 ITERLEN（nslot = 进循环时的 len 快照）
#   · C 轨：`cg_stmt` for 发射 `int _ilN = (int)px_len(it);` + 体内 `px_iter_ck(it, _ilN);`
#     （`cg_expr` 推导式走 for 初始化子句；长度局部用专用计数器 `_ilN` ⇒ 其余发射文本不变）
#   · 解释轨：`istmt`/`iexpr` 的快照长度 + `i_iter_ck`（list/dict 比较活长度）
# 判据（逐层可单独变红）：
#   ① 合法侧主用例 18 断言（含「改元素不改长度」「先快照」「values() 副本」「while」「嵌套」
#      「推导式」「break/continue」「多变量解包」）；
#   ② VM 轨（用户面默认轨）与 ③ C 轨：stdout 与解释轨**逐字节一致**（`pass=18 fail=0`）；
#   ④ 严格性：**6 类**修改用例 × 三轨，每轨必须 rc≠0 + 含 R1003 + 含**统一词条**；
#   ⑤ **负控 3 道**（默认跑，`--neg-skip` 跳过）—— 每道必须判红（该轨不再报 R1003）+ 还原：
#      A VM 轨 `bc_emit` 去掉 ITERLEN 发射（= 缺陷原样：快照长度 ⇒ list_append 静默少迭代）
#      B C 轨 `cg_stmt` 去掉 `px_iter_ck`（= 缺陷原样：活长度 ⇒ list_append 静默多迭代）
#      C 解释轨 `i_iter_ck` 直接放行（= 缺陷原样：活长度 ⇒ list_append 静默多迭代）
# 用法：./examples/m166_iter_mutate/verify.sh            （完整门：正判据 + 严格性 + 负控）
#       ./examples/m166_iter_mutate/verify.sh --neg-skip （只跑正判据，CI 用）
# 退出码：0 = 绿，1 = 红。
# 备注：负控会改 `selfhost/` 源 → 机制是「改源 → 重建 dev 件 → 跑 → 逐字节还原」；
#   dev 件走 `selfhost/devbuild.sh`（产物 /tmp/pxcdev、/tmp/pxcdev_vm、/tmp/pxidev，不碰入库件）。
#   ⚠️ 每道负控前先 `restore_all` 再 `snapshot`（各自独立、干净起点；M165 教训）；
#   退出 trap 里也 `restore_all`（中途被杀也能还原）。
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
WORK=$(mktemp -d /tmp/m166.XXXXXX)
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

# 严格性判据：三轨都必须 rc≠0 + 含 R1003 + 含统一词条
err_case() {             # $1=标签
    local tag=$1 t rc
    for t in interp vm c; do
        rc=$(cat "$WORK/$tag.$t.rc")
        if [ "$rc" = "0" ]; then
            bad "$tag $t 轨**未报错**（rc=0）—— 严格性缺失"
            sed 's/^/      /' "$WORK/$tag.$t.out" | head -4
            continue
        fi
        if ! grep -q "R1003" "$WORK/$tag.$t.out"; then
            bad "$tag $t 轨缺 R1003"; sed 's/^/      /' "$WORK/$tag.$t.out" | head -4; continue
        fi
        if ! grep -qF "迭代期间被迭代容器长度变化" "$WORK/$tag.$t.out"; then
            bad "$tag $t 轨词条不符（应为「迭代期间被迭代容器长度变化: n0 → n1」）"
            sed 's/^/      /' "$WORK/$tag.$t.out" | head -4; continue
        fi
        note "$tag $t 轨 rc=$rc · $(sed -n '1p' "$WORK/$tag.$t.out" | tr -d '\n')"
    done
}

# ════ 正判据 ① ════
hdr "[1/3] 合法迭代主用例（18 断言：只读/改元素/快照/别的容器/values()/while/嵌套/推导式/解包）"
run_three "$HERE/iter_ok.px" ok
expect_all_three ok "pass=18 fail=0"

# ════ 严格性 ② ════
hdr "[2/3] 严格性：6 类「迭代期间修改」用例 × 三轨（rc≠0 + R1003 + 统一词条）"
for c in mut_list_append mut_list_pop mut_dict_add mut_dict_del mut_comp mut_nested; do
    run_three "$HERE/$c.px" "$c"
    err_case "$c"
done

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

# 负控跑法：改源 → 重建 dev 件 → 用该轨跑 mut_list_append → **应不再报 R1003**（rc=0）→ 还原
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
    cp "$HERE/mut_list_append.px" "$w/mut_list_append.px"
    local rc=99
    case "$eng" in
        vm)
            if ( cd "$w" && PXC_VM_BIN=/tmp/pxcdev_vm timeout 900 "$PX" build mut_list_append.px > "$WORK/neg_$tag.build.log" 2>&1 && [ -x build/mut_list_append ] ); then
                ( cd "$w" && timeout 120 ./build/mut_list_append > "$WORK/neg_$tag.out" 2>&1 ); rc=$?
            else
                bad "负控 $tag VM 轨构建失败"; tail -5 "$WORK/neg_$tag.build.log" | sed 's/^/      /'; restore_all; return
            fi
            ;;
        c)
            if ( cd "$w" && PX_PXC_BIN=/tmp/pxcdev PX_BUILD_ENGINE=c timeout 900 "$PX" build --c mut_list_append.px > "$WORK/neg_$tag.build.log" 2>&1 && [ -x build/mut_list_append ] ); then
                ( cd "$w" && timeout 120 ./build/mut_list_append > "$WORK/neg_$tag.out" 2>&1 ); rc=$?
            else
                bad "负控 $tag C 轨构建失败"; tail -5 "$WORK/neg_$tag.build.log" | sed 's/^/      /'; restore_all; return
            fi
            ;;
        interp)
            ( cd "$w" && timeout 120 /tmp/pxidev mut_list_append.px > "$WORK/neg_$tag.out" 2>&1 ); rc=$?
            ;;
    esac
    if [ "$rc" = "0" ] || ! grep -q "迭代期间被迭代容器长度变化" "$WORK/neg_$tag.out"; then
        note "负控 $tag 已判红（$eng 轨不再报 R1003，rc=$rc）✅"
    else
        bad "负控 $tag 【未被判红】—— 判据对该回归不敏感"
        sed 's/^/      /' "$WORK/neg_$tag.out" | head -3
    fi
    restore_all
}

if [ "$NEG_SKIP" = "1" ]; then
    hdr "[3/3] 负控 3 道（--neg-skip：跳过）"
    note "CI 模式：负控涉及 dev 件重建（pxc / pxc_vm / pxi），交给本地完整门"
else
    hdr "[3/3] 负控 3 道（每道必须判红 + 逐字节还原）"

    # A：VM 轨 bc_emit 去掉 ITERLEN 发射（= 缺陷原样：快照长度 ⇒ list_append 静默少迭代）
    run_neg A selfhost/bc_emit.px \
'    # 循环变量 = its[ctr]
    # M166（缺陷 176）：迭代期间被迭代容器长度变化 ⇒ R1003（nslot = 进入循环时的 len 快照）
    bc_emit_inst(func, "ITERLEN", its, nslot, 0)
' \
'    # 循环变量 = its[ctr]
' \
        "pxc --vm" vm "VM 轨去掉 ITERLEN 发射（缺陷 176 原样：快照长度静默少迭代）"

    # B：C 轨 cg_stmt 去掉 px_iter_ck（= 缺陷原样：活长度 ⇒ list_append 静默多迭代）
    run_neg B selfhost/cg_stmt.px \
'        s += pad + "    px_iter_ck(" + it_var + ", " + len_var + ");\n"
' \
'' \
        "pxc" c "C 轨去掉 px_iter_ck 复核（缺陷 176 原样：活长度静默多迭代）"

    # C：解释轨 i_iter_ck 直接放行（= 缺陷原样：活长度 ⇒ list_append 静默多迭代）
    run_neg C selfhost/istmt.px \
'def i_iter_ck(v, n0, pos):
    let t = i_type_name(v)
    if t != "list" and t != "dict":
        return Ok(null)
    let n1 = len(v)
    if n1 != n0:
        return Err(i_r1003("迭代期间被迭代容器长度变化: " + str(n0) + " → " + str(n1), pos))
    return Ok(null)
' \
'def i_iter_ck(v, n0, pos):
    return Ok(null)
' \
        "pxi" interp "解释轨 i_iter_ck 直接放行（缺陷 176 原样：活长度静默多迭代）"
fi

echo
if [ "$fail" = "0" ]; then
    echo "M166-VERIFY-OK"
else
    echo "M166-VERIFY-FAIL"
fi
exit "$fail"
