#!/usr/bin/env bash
# ============================================================
# M181 门 · 帧内绑定「未初始化即读」三轨统一（第 59 轮 · 缺陷 193）
# ------------------------------------------------------------
# 病灶（本机实测，同一份源码三轨三个答案）：
#   `def f(): print(x); let x = 42` —— 解释轨报 `R1001 未定义变量: 'x'`，
#   而 VM/C 轨**静默给 null**：帧局部槽的初值原来是 `px_null()` / calloc 零值，
#   与「已声明且值为 null」**不可区分**。同族实测还有：
#     · 条件分支未走时的声明（`if c: let z = 1` + `return z`，c=false）
#     · 空循环的循环变量（`for i in []` 后读 i）· while 体 0 次迭代后的声明
#     · `let x = x + 1` —— 修前编译轨报的是 `R1002 无法相加: null + int`
#       （**一条与根因无关的错误**，而解释轨是 R1001）
#     · 帧内声明**遮蔽**同名模块/外层绑定（`var x = 99` + `def f(): print(x); let x = 1`）
#       —— 解释轨沿 env 链读到 99，编译两轨读到本帧未初始化的槽
#
# 一条真相（本门判据的定义）：
#   帧内（函数体 / 闭包体）读一个**本帧绑定**、而其声明尚未执行 ⇒
#   通道 stderr · 码 `R1001` · 消息体 `未定义变量: '<名>'` · rc ≠ 0（三轨一致）。
#   **初始化是单调的**：声明执行过之后的任何读都合法 ——
#   「先创建闭包、后声明、再调用」必须继续可行（解释轨给 1）。
#   为什么不做编译期静态拒绝：本仓是 Python 式**函数级作用域**（M62-L5/M169），
#   上面的闭包写法是**合法程序**，静态拒绝会误拒。
#
# 判据：
#   [1] 合法侧 7 例（`ok_side.px`）三轨 rc=0 · stdout **逐字节一致** · 含 `M181A-OK`
#   [2] 严格性层 8 例 × 三轨：rc ≠ 0 · 含码 R1001 · 含统一消息体（通道 stderr）
#   [3] 负控 3 道（默认跑，`--neg-skip` 跳过；**各自独立判红** + sha256 逐字节还原 + 复绿）：
#       NC-A runtime/vm.c 的 `PXOP_UNINIT` 变 no-op ⇒ 哨兵不写入 ⇒ [2] 的 VM 轨变红
#       NC-B selfhost/codegen.px 的 hoist 初值退回 `px_null()` ⇒ [2] 的 C 轨变红
#       NC-C selfhost/icall.px 跳过 `env_mark_unbound` ⇒ [2] 的解释轨变红（回到 env 链）
#       纪律（R50 教训）：每道负控前先 restore 再 snapshot（各自干净起点）；trap 兜底还原。
# 用法：./examples/m181_uninit_read/verify.sh [--neg-skip]
# 退出码：0 = 绿，1 = 红，2 = 门自身前置自查失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m181_uninit_read
RT="$ROOT/runtime/runtime.c"
VM_C="$ROOT/runtime/vm.c"
CG="$ROOT/selfhost/codegen.px"
IC="$ROOT/selfhost/icall.px"
DEV_PXI="$ROOT/selfhost/build/interp"
DEV_PXC="$ROOT/selfhost/build/compiler"
BAK=/tmp/m181_gate_bak
W=/tmp/m181_gate
rm -rf "$W" "$BAK"; mkdir -p "$W" "$BAK"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

snapshot() { cp -f "$RT" "$BAK/runtime.c"; cp -f "$VM_C" "$BAK/vm.c"; cp -f "$CG" "$BAK/codegen.px"; cp -f "$IC" "$BAK/icall.px"; }
restore_all() {
    [ -f "$BAK/runtime.c" ] && cp -f "$BAK/runtime.c" "$RT"
    [ -f "$BAK/vm.c" ] && cp -f "$BAK/vm.c" "$VM_C"
    [ -f "$BAK/codegen.px" ] && cp -f "$BAK/codegen.px" "$CG"
    [ -f "$BAK/icall.px" ] && cp -f "$BAK/icall.px" "$IC"
}
sha_of() { sha256sum "$1" | cut -d' ' -f1; }
trap 'restore_all' EXIT

# ── 前置不变量自查（防上轮被 SIGKILL 留下脏源码）
grep -q 'px_chk_uninit' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M181 px_chk_uninit" >&2; exit 2; }
grep -q 'case PXOP_UNINIT' "$VM_C" || { echo "❌ 前置自查失败：vm.c 缺 PXOP_UNINIT" >&2; exit 2; }
grep -q 'def cg_load_ck' "$CG" || { echo "❌ 前置自查失败：codegen.px 缺 cg_load_ck" >&2; exit 2; }
grep -q 'env_mark_unbound' "$IC" || { echo "❌ 前置自查失败：icall.px 缺 env_mark_unbound 调用" >&2; exit 2; }

# 用 .rtcache 全 runtime 对象把一份 C 链成可执行（门内负控共用）
rcache=$("$ROOT/tools/px" rtcache 2>/dev/null | tail -1)
cbuild() {   # $1=输入 C，$2=输出二进制
    gcc -static -O1 -I"$rcache" -I"$ROOT/runtime" "$1" -o "$2" \
        "$rcache"/*.o \
        "$ROOT/runtime/third_party/sqlite3/sqlite3.o" \
        "$ROOT/runtime/mbedtls/lib/libmbedtls.a" "$ROOT/runtime/mbedtls/lib/libmbedx509.a" "$ROOT/runtime/mbedtls/lib/libmbedcrypto.a" \
        "$ROOT/runtime/third_party/ngtcp2/lib/libngtcp2.a" "$ROOT/runtime/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
        "$ROOT/runtime/third_party/openssl/lib/libssl.a" "$ROOT/runtime/third_party/openssl/lib/libcrypto.a" \
        "$ROOT/runtime/third_party/zlib/lib/libz.a" -lm -ldl -lpthread
}

run_tracks() {   # $1=用例名
    local n=$1
    "$ROOT/bootstrap/pxi" "$D/$n.px" > "$W/$n.interp.out" 2> "$W/$n.interp.err"; echo $? > "$W/$n.interp.rc"
    rm -rf "$D/build"
    if (cd "$ROOT" && ./tools/px build "$D/$n.px" > "$W/$n.vm.log" 2>&1) && [ -x "$D/build/$n" ]; then
        "$D/build/$n" > "$W/$n.vm.out" 2> "$W/$n.vm.err"; echo $? > "$W/$n.vm.rc"
    else : > "$W/$n.vm.out"; : > "$W/$n.vm.err"; echo 99 > "$W/$n.vm.rc"; fi
    rm -rf "$D/build"
    if (cd "$ROOT" && ./tools/px build --c "$D/$n.px" > "$W/$n.c.log" 2>&1) && [ -x "$D/build/$n" ]; then
        "$D/build/$n" > "$W/$n.c.out" 2> "$W/$n.c.err"; echo $? > "$W/$n.c.rc"
    else : > "$W/$n.c.out"; : > "$W/$n.c.err"; echo 99 > "$W/$n.c.rc"; fi
    rm -rf "$D/build"
}

echo "=== [1] 合法侧 7 例：三轨 rc=0 + stdout 逐字节一致"
run_tracks ok_side
for t in interp vm c; do
    chk "A/$t rc=0" "[ \"$(cat $W/ok_side.$t.rc)\" = 0 ]"
    chk "A/$t 含 M181A-OK" "grep -q 'M181A-OK' $W/ok_side.$t.out"
done
chk "A interp==VM 逐字节" "cmp -s $W/ok_side.interp.out $W/ok_side.vm.out"
chk "A interp==C  逐字节" "cmp -s $W/ok_side.interp.out $W/ok_side.c.out"
chk "A 定点：闭包后声明后调用=42" "grep -q '^A1 42$' $W/ok_side.interp.out"
chk "A 定点：非空循环后读循环变量=3" "grep -q '^A2 3$' $W/ok_side.interp.out"
chk "A 定点：分支都声明同一名=7" "grep -q '^A3 7$' $W/ok_side.interp.out"
chk "A 定点：参数可读=42" "grep -q '^A4 42$' $W/ok_side.interp.out"
chk "A 定点：无初值声明=null" "grep -q '^A7 null$' $W/ok_side.interp.out"

echo "=== [2] 严格性层 8 例 × 三轨：rc≠0 + R1001 + 统一消息体"
declare -A NAME=(
  [e1_read_before_decl]=x       [e2_closure_call_before_decl]=x
  [e3_empty_loop_var]=i         [e4_branch_not_taken]=z
  [e5_self_ref_init]=x          [e6_shadow_module]=x
  [e7_shadow_outer_frame]=y     [e8_while_zero_iter]=u
)
for c in e1_read_before_decl e2_closure_call_before_decl e3_empty_loop_var e4_branch_not_taken \
         e5_self_ref_init e6_shadow_module e7_shadow_outer_frame e8_while_zero_iter; do
    run_tracks "$c"
    for t in interp vm c; do
        chk "B/$c/$t rc≠0" "[ \"$(cat $W/$c.$t.rc)\" != 0 ]"
        chk "B/$c/$t 含 R1001 + 消息体" "grep -qF 'R1001' $W/$c.$t.err && grep -qF \"未定义变量: '${NAME[$c]}'\" $W/$c.$t.err"
        chk "B/$c/$t 诊断走 stderr（stdout 空）" "[ ! -s $W/$c.$t.out ]"
    done
done
chk "B e1 VM 与 C 的 stderr 逐字节一致（同一 runtime 出口）" "cmp -s $W/e1_read_before_decl.vm.err $W/e1_read_before_decl.c.err"
chk "B e1 三轨消息体同形（未定义变量: 'x'）" "grep -qF \"未定义变量: 'x'\" $W/e1_read_before_decl.interp.err && grep -qF \"未定义变量: 'x'\" $W/e1_read_before_decl.vm.err && grep -qF \"未定义变量: 'x'\" $W/e1_read_before_decl.c.err"

echo "=== [3] 回归哨兵：p4 形态（闭包先创建、后声明、再调用）必须仍可行（在 [1] 的 A1）"
chk "[3] 最小复现器即 e1（缺陷 193 原文形态）" "[ -f $D/e1_read_before_decl.px ]"

if [ "$NEG" = "1" ]; then
    echo "=== [4] 负控 3 道（各自独立判红 + 还原复绿）"

    # NC-A：VM 的 UNINIT 变 no-op ⇒ 哨兵不写入 ⇒ e1 的 VM 轨应回到 rc=0
    restore_all; snapshot
    perl -0pi -e 's/(case PXOP_UNINIT:[^\n]*\n)\s*slots\[in\.a\] = px_uninit\(\);\n\s*break;/$1            \/* M181-NC-A no-op *\/ break;/' "$VM_C"
    grep -q 'M181-NC-A no-op' "$VM_C" || { echo "  SKIP NC-A（锚点未命中）"; }
    run_tracks e1_read_before_decl
    chk "NC-A VM 轨变红（不再报 R1001 ⇒ rc=0）" "[ \"$(cat $W/e1_read_before_decl.vm.rc)\" = 0 ]"
    restore_all
    chk "NC-A 还原后 vm.c 逐字节一致" "cmp -s $VM_C $BAK/vm.c"

    # NC-B：C 轨 hoist 初值退回 px_null() ⇒ 重编 dev 编译器 ⇒ e1 的 C 轨应回到 rc=0
    restore_all; snapshot
    perl -0pi -e 's/LXValue \" \+ hd\[0\] \+ \" = px_uninit\(\);\\n\"/LXValue \" + hd[0] + \" = px_null();\\n\"/' "$CG"
    if (cd "$ROOT" && ./tools/px build selfhost/compiler.px > "$W/ncb_build.log" 2>&1) && [ -x "$DEV_PXC" ]; then
        "$DEV_PXC" build "$D/e1_read_before_decl.px" > "$W/ncb.c" 2>/dev/null
        if cbuild "$W/ncb.c" "$W/ncb.bin" > "$W/ncb.link.log" 2>&1; then
            "$W/ncb.bin" > "$W/ncb.out" 2> "$W/ncb.err"; echo $? > "$W/ncb.rc"
            chk "NC-B C 轨变红（不再报 R1001 ⇒ rc=0）" "[ \"$(cat $W/ncb.rc)\" = 0 ]"
        else
            echo "  SKIP NC-B（链接失败，见 $W/ncb.link.log）"
        fi
    else
        echo "  SKIP NC-B（dev 编译器构建失败，见 $W/ncb_build.log）"
    fi
    restore_all
    chk "NC-B 还原后 codegen.px 逐字节一致" "cmp -s $CG $BAK/codegen.px"

    # NC-C：解释轨跳过 env_mark_unbound ⇒ 重编 dev 解释器 ⇒ e1/e6 应回到 rc=0
    restore_all; snapshot
    perl -0pi -e 's/^(\s*)env_mark_unbound\(call_env, decls\[di\]\)\s*$/$1\/* M181-NC-C: 不登记 *\//m' "$IC"
    grep -q 'M181-NC-C' "$IC" || echo "  ⚠️ NC-C 锚点未命中（icall.px 形态变了？）"
    if (cd "$ROOT" && ./tools/px build selfhost/interp.px > "$W/ncc_build.log" 2>&1) && [ -x "$DEV_PXI" ]; then
        "$DEV_PXI" "$D/e1_read_before_decl.px" > "$W/ncc1.out" 2> "$W/ncc1.err"; r1=$?
        "$DEV_PXI" "$D/e6_shadow_module.px" > "$W/ncc6.out" 2> "$W/ncc6.err"; r6=$?
        chk "NC-C 解释轨变红（e1 rc=0）" "[ $r1 = 0 ]"
        chk "NC-C 解释轨变红（e6 遮蔽例回到 99）" "[ $r6 = 0 ] && grep -q '^99$' $W/ncc6.out"
    else
        echo "  SKIP NC-C（dev 解释器构建失败，见 $W/ncc_build.log）"
    fi
    restore_all
    chk "NC-C 还原后 icall.px 逐字节一致" "cmp -s $IC $BAK/icall.px"

    # 还原后复绿（三道负控都还原 ⇒ 三轨必须全部回到 R1001）
    run_tracks e1_read_before_decl
    chk "负控后复绿：e1 三轨 rc≠0 且含 R1001" "[ \"$(cat $W/e1_read_before_decl.interp.rc)\" != 0 ] && [ \"$(cat $W/e1_read_before_decl.vm.rc)\" != 0 ] && [ \"$(cat $W/e1_read_before_decl.c.rc)\" != 0 ]"
else
    echo "=== [4] 负控已跳过（--neg-skip）"
fi

rm -rf "$D/build"
echo
echo "M181 门结果：PASS=$pass FAIL=$fail"
[ "$fail" = "0" ] && echo "M181-VERIFY-OK" || echo "M181-VERIFY-FAIL"
exit $([ "$fail" = "0" ] && echo 0 || echo 1)
