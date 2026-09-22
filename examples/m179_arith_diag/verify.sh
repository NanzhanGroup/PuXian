#!/usr/bin/env bash
# ============================================================
# M179 门 · 运算族的「码 + 措辞」统一 + 静默坏值收口（第 57 轮 · 缺陷 195 / 196）
# ------------------------------------------------------------
# 病灶一（**静默坏值**，本轮实测）：runtime 的数值分支直接用 `num_val(a)`，而它是
#     `return v.type == PX_INT ? (double)v.as.i : v.as.f;`
#   ⇒ 非数值实参读的是 union 的 `as.f` —— 对 PX_STR 那是**对象指针的位模式**（UB）：
#     · `1.0 * "x"` → VM 轨 `6.905411902715e-310` / C 轨 `6.95199933934835e-310`
#       （**同一台机、同一份源码，两个不同的垃圾值**）
#     · `1 / "x"` → `inf`；`1.0 + "x"` → `1.0`；`2 ** "x"` → `1.0`
#   而解释轨一直是响亮的「需要数值」⇒ 三轨分叉 **且** 编译轨静默给坏值。
# 病灶二（**静默错值**）：`px_lt/le/gt/ge` 的兜底是 `compare_values`，其默认分支为
#     `strcmp(px_type_name(a), px_type_name(b))`（"保证可比性"）
#   ⇒ `1 < "x"` 在编译轨给 **true**，解释轨报「比较不支持」。
# 病灶三（措辞/码三分）：解释轨 `+ 不支持: …` / 编译轨 `无法相加: …`（**无 R1002 码**）；
#   整数除零 runtime 无码（应为 R1006）；索引越界三轨三种文（VM 轨连码都没有）；
#   位运算/索引位置对 float **静默截断**（解释轨一直报错）。
#
# 一条真相（本门判据的定义）：
#   · 算术 + - * / // % ** 一元- ：要求数值（`str * int` 例外 = 重复），
#     否则 `R1002: 无法<运算>: <ta> <op> <tb>`；
#   · 次序比较 < <= > >= ：要求**双数值或同类型**，否则 `R1002: 无法比较: <ta> vs <tb>`；
#     而 `sorted` / `min` / `max` 用**内部比较器**（全序，跨型按类型名）—— 见 §17.13；
#   · 位运算要求 int（`R1002: <运算> 需要整数，实际是 <t>`）；索引位置要求 int
#     （`R1002: 索引必须是整数，实际是 <t>`）；越界 `R1003: 索引越界: <i> (len=<n>)`；
#   · 整数除零/取模除零 `R1006: 除零错误`（与 spec §11.2 的错误码表对齐）。
#
# 判据：
#   [1] 用例 A（`ok_matrix.px`，28 行合法侧矩阵）三轨 rc=0 · stdout **逐字节一致**
#   [2] 用例 B（20 个真类型错/越界/除零）三轨 rc≠0 · stdout 恰为 `before` ·
#       stderr 含统一错误码 + **统一消息体**
#   [3] 负控 3 道（默认跑，`--neg-skip` 跳过；各自独立判红 + sha256 逐字节还原 + 复绿）：
#       NC-A runtime `px_req_num2` 变 no-op ⇒ `1.0 * "x"` 的 C/VM 轨与解释轨不一致
#       NC-B 解释轨 `i_bin_add` 文案退回 `+ 不支持` ⇒ 消息体判据变红
#       NC-C runtime `px_req_cmp` 变 no-op ⇒ `1 < "x"` 的 C/VM 轨变 rc=0（静默 true）
# 用法：./examples/m179_arith_diag/verify.sh [--neg-skip]
# 退出码：0 = 绿，1 = 红，2 = 门自身前置自查失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m179_arith_diag
RT="$ROOT/runtime/runtime.c"
VM_C="$ROOT/runtime/vm.c"
IV="$ROOT/selfhost/ival.px"
DEV_PXI="$ROOT/selfhost/build/interp"
BAK_RT=/tmp/m179_runtime.bak
BAK_VM=/tmp/m179_vm.bak
BAK_IV=/tmp/m179_ival.bak
W=/tmp/m179_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { cp -f "$RT" "$BAK_RT"; cp -f "$VM_C" "$BAK_VM"; cp -f "$IV" "$BAK_IV"; }
restore_all() {
    [ -f "$BAK_RT" ] && cp -f "$BAK_RT" "$RT"
    [ -f "$BAK_VM" ] && cp -f "$BAK_VM" "$VM_C"
    [ -f "$BAK_IV" ] && cp -f "$BAK_IV" "$IV"
}
trap 'restore_all; rm -f "$BAK_RT" "$BAK_VM" "$BAK_IV"' EXIT

# ── 前置不变量自查（防上轮被 SIGKILL 留下脏源码）
grep -q 'static void px_req_num2' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M179 px_req_num2 锚点" >&2; exit 2; }
grep -q 'static void px_req_cmp' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M179 px_req_cmp 锚点" >&2; exit 2; }
grep -q '无法相加: " + tl' "$IV" || { echo "❌ 前置自查失败：ival.px 缺 M179 文案锚点" >&2; exit 2; }

build_dev() { (cd "$ROOT" && ./tools/px build selfhost/interp.px > "$W/devbuild.log" 2>&1) && [ -x "$DEV_PXI" ]; }
echo "── 烘 dev 解释器（selfhost/build/interp）"; build_dev || { echo "❌ dev 解释器构建失败（$W/devbuild.log）" >&2; exit 2; }

run_tracks() {   # $1=用例名
    local n=$1
    "$DEV_PXI" "$D/$n.px" > "$W/$n.interp.out" 2> "$W/$n.interp.err"; echo $? > "$W/$n.interp.rc"
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

echo "=== [1] 用例 A：合法侧矩阵 28 行（三轨必须逐字节一致）"
run_tracks ok_matrix
for t in interp vm c; do
    chk "A/$t rc=0" "[ \"\$(cat $W/ok_matrix.$t.rc)\" = 0 ]"
    chk "A/$t 含 M179A-ARITH-OK" "grep -q 'M179A-ARITH-OK' $W/ok_matrix.$t.out"
done
chk "A interp==VM 逐字节" "cmp -s $W/ok_matrix.interp.out $W/ok_matrix.vm.out"
chk "A interp==C  逐字节" "cmp -s $W/ok_matrix.interp.out $W/ok_matrix.c.out"
chk "A 定点：strmul=ababab" "grep -q '13 strmul=ababab' $W/ok_matrix.interp.out"
chk "A 定点：strmuln=[]（负数 ⇒ 空串）" "grep -q '15 strmuln=\[\]' $W/ok_matrix.interp.out"
chk "A 定点：minmix=1 / maxmix=a（全序）" "grep -q '25 minmix=1 a' $W/ok_matrix.interp.out"
chk "A 定点：sortedmix=[1, 2, a]" "grep -q '26 sortedmix=\[1, 2, a\]' $W/ok_matrix.interp.out"
chk "A 定点：not 走真值（not 0=true）" "grep -q '22 not=false true true false' $W/ok_matrix.interp.out"

echo "=== [2] 用例 B：20 个错误用例（同码 + 同消息体 + stdout 恰为 before）"
printf 'before\n' > "$W/expect_before"
declare -A CODE=(
  [err_add_str]=R1002 [err_sub_str]=R1002 [err_mul_str]=R1002 [err_div_str]=R1002
  [err_idiv_str]=R1002 [err_mod_str]=R1002 [err_pow_str]=R1002 [err_neg_str]=R1002
  [err_fadd_str]=R1002 [err_fmul_str]=R1002 [err_lt_int_str]=R1002 [err_ge_int_str]=R1002
  [err_bit_float]=R1002 [err_bitnot_float]=R1002 [err_idx_float]=R1002 [err_listmul]=R1002
  [err_idx_oob]=R1003 [err_str_oob]=R1003 [err_idiv0]=R1006 [err_mod0]=R1006
)
declare -A MSG=(
  [err_add_str]='无法相加: int + string'
  [err_sub_str]='无法相减: int - string'
  [err_mul_str]='无法相乘: int * string'
  [err_div_str]='无法相除: int / string'
  [err_idiv_str]='无法整除: int // string'
  [err_mod_str]='无法取模: int % string'
  [err_pow_str]='无法幂运算: int ** string'
  [err_neg_str]='无法取负: -string'
  [err_fadd_str]='无法相加: float + string'
  [err_fmul_str]='无法相乘: float * string'
  [err_lt_int_str]='无法比较: int vs string'
  [err_ge_int_str]='无法比较: int vs string'
  [err_bit_float]='按位与 需要整数，实际是 float'
  [err_bitnot_float]='按位取反 需要整数，实际是 float'
  [err_idx_float]='索引必须是整数，实际是 float'
  [err_idx_oob]='索引越界: 5 (len=2)'
  [err_str_oob]='索引越界: 9 (len=3)'
  [err_idiv0]='除零错误'
  [err_mod0]='除零错误'
  [err_listmul]='无法相乘: list * int'
)
for c in err_add_str err_sub_str err_mul_str err_div_str err_idiv_str err_mod_str err_pow_str err_neg_str \
         err_fadd_str err_fmul_str err_lt_int_str err_ge_int_str err_bit_float err_bitnot_float \
         err_idx_float err_idx_oob err_str_oob err_idiv0 err_mod0 err_listmul; do
    run_tracks "$c"
    for t in interp vm c; do
        chk "B/$c/$t rc≠0" "[ \"\$(cat $W/$c.$t.rc)\" != 0 ]"
        chk "B/$c/$t stdout 恰为 before" "cmp -s $W/$c.$t.out $W/expect_before"
        chk "B/$c/$t 含码 ${CODE[$c]} + 消息体" "grep -qF '${CODE[$c]}' $W/$c.$t.err && grep -qF \"${MSG[$c]}\" $W/$c.$t.err"
    done
done

if [ $NEG -eq 1 ]; then
echo "=== [3] 负控 3 道（各自独立判红 + 逐字节还原）"
SHA_RT=$(sha256sum "$RT" | cut -c1-16); SHA_IV=$(sha256sum "$IV" | cut -c1-16)

echo "── NC-A runtime：px_req_num2 变 no-op ⇒ \`1.0 * \"x\"\` 的 C/VM 轨必须与解释轨不一致"
snapshot
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
o = '''    if (!px_num_ok(a) || !px_num_ok(b))
        px_error("R1002: 无法%s: %s %s %s", opname, px_type_name(a), sym, px_type_name(b));'''
assert s.count(o) == 1, s.count(o)
s = s.replace(o, '''    if (0 && !px_num_ok(a) && !px_num_ok(b))   // NC-A：恢复「读 union as.f」的 UB 路径
        px_error("R1002: 无法%s: %s %s %s", opname, px_type_name(a), sym, px_type_name(b));''')
open(p, 'w', encoding='utf-8').write(s)
PYEOF
run_tracks err_fmul_str
if cmp -s "$W/err_fmul_str.interp.out" "$W/err_fmul_str.c.out"; then
    chk "NC-A 判红（C 轨与解释轨不一致）" "false"
else
    chk "NC-A 判红（C 轨静默给坏值 ⇒ 输出不同）" "[ \"\$(cat $W/err_fmul_str.c.rc)\" = 0 ] || ! grep -qF '无法相乘: float * string' $W/err_fmul_str.c.err"
fi
restore_all
chk "NC-A 还原逐字节（runtime.c）" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA_RT\" ]"
run_tracks err_fmul_str
chk "NC-A 还原后复绿" "cmp -s $W/err_fmul_str.interp.out $W/err_fmul_str.c.out && grep -qF '无法相乘: float * string' $W/err_fmul_str.c.err"

echo "── NC-B 解释轨：i_bin_add 文案退回 \`+ 不支持\` ⇒ 消息体判据必须变红"
python3 - "$IV" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
o = '    return Err(i_r1002("无法相加: " + tl + " + " + tr, pos))'
assert s.count(o) == 1, s.count(o)
s = s.replace(o, '    return Err(i_r1002("+ 不支持: " + tl + " + " + tr, pos))   # NC-B')
open(p, 'w', encoding='utf-8').write(s)
PYEOF
build_dev || echo "  (dev 重建失败)"
"$DEV_PXI" "$D/err_add_str.px" > "$W/ncb.out" 2> "$W/ncb.err"; echo $? > "$W/ncb.rc"
chk "NC-B 判红（解释轨消息体不同）" "! grep -qF '无法相加: int + string' $W/ncb.err"
restore_all
chk "NC-B 还原逐字节（ival.px）" "[ \"\$(sha256sum $IV | cut -c1-16)\" = \"$SHA_IV\" ]"
build_dev || echo "  (dev 重建失败)"
"$DEV_PXI" "$D/err_add_str.px" > "$W/ncb2.out" 2> "$W/ncb2.err" || true
chk "NC-B 还原后复绿" "grep -qF '无法相加: int + string' $W/ncb2.err"

echo "── NC-C runtime：px_req_cmp 变 no-op ⇒ \`1 < \"x\"\` 的 C/VM 轨必须变 rc=0（静默 true）"
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
o = '''    if (px_num_ok(a) && px_num_ok(b)) return;   // int/float 互通
    if (a.type == b.type) return;               // 同类型（str/bytes/bool/list/dict…）
    px_error("R1002: 无法比较: %s vs %s", px_type_name(a), px_type_name(b));'''
assert s.count(o) == 1, s.count(o)
s = s.replace(o, '''    (void)a; (void)b; return;   // NC-C：恢复「按类型名给确定性顺序」'''
              + '\n' + '''    px_error("R1002: 无法比较: %s vs %s", px_type_name(a), px_type_name(b));''')
open(p, 'w', encoding='utf-8').write(s)
PYEOF
run_tracks err_lt_int_str
chk "NC-C 判红（C 轨 rc=0 = 静默 true）" "[ \"\$(cat $W/err_lt_int_str.c.rc)\" = 0 ]"
chk "NC-C 判红（C 轨 stderr 无 R1002）" "! grep -qF '无法比较: int vs string' $W/err_lt_int_str.c.err"
restore_all
chk "NC-C 还原逐字节（runtime.c）" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA_RT\" ]"
run_tracks err_lt_int_str
chk "NC-C 还原后复绿" "cmp -s $W/err_lt_int_str.interp.out $W/err_lt_int_str.vm.out && grep -qF '无法比较: int vs string' $W/err_lt_int_str.c.err"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M179-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
