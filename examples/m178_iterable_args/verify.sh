#!/usr/bin/env bash
# ============================================================
# M178 门 · 可迭代实参统一（第 57 轮）—— join / sorted / reversed / contains
# ------------------------------------------------------------
# 病灶（修前同一份源码三轨分叉，全部实测）：
#   ① `join("-", (x for x in [1,2,3]))`：解释轨报一句**与 join 毫无关系**的
#      `字典索引键必须是字符串`（解释轨内部直接 `len(args[1])` / `args[1][i]`，
#      而生成器在解释轨里是 dict{"__gen__"}）；编译轨正常 `1-2-3`。
#   ② `sorted((3,1,2))` / `reversed((1,2,3))` / `contains((1,2,3), 2)`：编译轨**拒绝**
#      （只收 list）；解释轨也拒绝，但两轨**文案各说各话**。
#   ③ `reversed("中文")`：解释轨 `[文, 中]`（list of rune）vs 编译轨 **按字节反转**
#      ⇒ 得到**非法 UTF-8**（实测 `\xad\x87\xe4…`，渲染乱码）——不只是分叉，是**坏值**。
#   ④ `join("-", "abc")` / `sorted("cab")`：解释轨 reject vs 编译轨 accept
#      （str 经 `px_as_list` 悄悄混进编译轨接受面）。
#
# 一条真相（= 本门判据的定义）：
#   · 可迭代实参 = **list / tuple / 生成器 / 字符串**（str 按 **rune** 迭代，与 `len`/索引同口径）；
#   · `reversed(str)` 是**字符串操作** ⇒ 返回 **str**（rune 级反转）；其余入口返回 list；
#   · 拒绝文案统一：`<名> 参数需要 list/tuple/生成器/字符串，实际是 <t>`
#     （`contains` 沿用 `不支持类型 <t>`；字符串容器走**子串**语义，与成员判定对 str 针等价）。
#
# 判据：
#   [1] 用例 A（31 行矩阵：4 形态 × 4 入口 + 空/重复/嵌套/中文）三轨 rc=0 · stdout **逐字节一致**
#   [2] 用例 B（12 行 str 面，含 emoji 4 字节与内嵌 NUL）三轨 rc=0 · stdout **逐字节一致**
#   [3] 用例 C（4 个真不支持的类型）三轨 rc≠0 · stdout 恰为 `before` · stderr 含 R1002 + **同文案**
#   [4] 缺陷 195 的取证（**不计失败**）：`sum("12")` 的措辞分叉 —— 该缺陷**已由 M179 收口**
#       （`+ 不支持` → `无法相加`，且编译轨补上 R1002 码）；此处保留用例作为"收敛人证"，
#       正式的逐格判红在 `examples/m179_arith_diag/`（20 用例 × 三轨）
#   [5] 负控 3 道（默认跑，`--neg-skip` 跳过；各自独立判红 + sha256 逐字节还原 + 复绿）：
#       NC-A runtime：`reversed(str)` 改回**按字节**反转 ⇒ C/VM 轨必须与解释轨不一致
#       NC-B 解释轨：`reversed(str)` 改回收 rune 进 list ⇒ 解释轨必须与编译轨不一致
#       NC-C runtime：`join` 改回只收 list/tuple（拒生成器）⇒ C/VM 轨红而解释轨绿
#
# 用法：./examples/m178_iterable_args/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m178_iterable_args/verify.sh --neg-skip （只跑正判据，CI 用）
# 退出码：0 = 绿，1 = 红，2 = 门自身前置自查失败（源码被上轮 kill 留脏）。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m178_iterable_args
RT="$ROOT/runtime/runtime.c"
IB="$ROOT/selfhost/ibuiltin.px"
DEV_PXI="$ROOT/selfhost/build/interp"
BAK_RT=/tmp/m178_runtime.bak
BAK_IB=/tmp/m178_ibuiltin.bak
W=/tmp/m178_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { cp -f "$RT" "$BAK_RT"; cp -f "$IB" "$BAK_IB"; }
restore_all() {
    [ -f "$BAK_RT" ] && cp -f "$BAK_RT" "$RT"
    [ -f "$BAK_IB" ] && cp -f "$BAK_IB" "$IB"
}
trap 'restore_all; rm -f "$BAK_RT" "$BAK_IB"' EXIT

# ── 前置不变量自查（M176 教训：trap 挡不住 SIGKILL，上一轮被 kill 会留下脏源码）
grep -q 'join 第二参数需要 list/tuple/生成器/字符串' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M178 join 文案锚点（上轮被 kill 留下脏源码？）" >&2; exit 2; }
grep -q '字符串入参 → 字符串出参' "$IB" || { echo "❌ 前置自查失败：ibuiltin.px 缺 M178 reversed(str) 锚点" >&2; exit 2; }

# ── 解释轨用**从当前源码新烘**的 dev 件（否则改 runtime/interp 时解释轨是旧入库件）
build_dev() { (cd "$ROOT" && ./tools/px build selfhost/interp.px > "$W/devbuild.log" 2>&1) && [ -x "$DEV_PXI" ]; }
echo "── 烘 dev 解释器（selfhost/build/interp）"; build_dev || { echo "❌ dev 解释器构建失败（日志 $W/devbuild.log）" >&2; exit 2; }

run_tracks() {   # $1=用例名（不含 .px）
    local n=$1
    "$DEV_PXI" "$D/$n.px" > "$W/$n.interp.out" 2> "$W/$n.interp.err"; echo $? > "$W/$n.interp.rc"
    rm -rf "$D/build"
    if (cd "$ROOT" && ./tools/px build "$D/$n.px" > "$W/$n.vm.build.log" 2>&1) && [ -x "$D/build/$n" ]; then
        "$D/build/$n" > "$W/$n.vm.out" 2> "$W/$n.vm.err"; echo $? > "$W/$n.vm.rc"
    else : > "$W/$n.vm.out"; : > "$W/$n.vm.err"; echo 99 > "$W/$n.vm.rc"; fi
    rm -rf "$D/build"
    if (cd "$ROOT" && ./tools/px build --c "$D/$n.px" > "$W/$n.c.build.log" 2>&1) && [ -x "$D/build/$n" ]; then
        "$D/build/$n" > "$W/$n.c.out" 2> "$W/$n.c.err"; echo $? > "$W/$n.c.rc"
    else : > "$W/$n.c.out"; : > "$W/$n.c.err"; echo 99 > "$W/$n.c.rc"; fi
    rm -rf "$D/build"
}

echo "=== [1] 用例 A：可迭代实参矩阵 31 行（四形态 × 四入口）三轨逐字节一致"
run_tracks iter_args
for t in interp vm c; do
    chk "A/$t rc=0" "[ \"\$(cat $W/iter_args.$t.rc)\" = 0 ]"
    chk "A/$t 含 M178A-ITERABLE-ARGS-OK" "grep -q 'M178A-ITERABLE-ARGS-OK' $W/iter_args.$t.out"
done
chk "A interp==VM 逐字节" "cmp -s $W/iter_args.interp.out $W/iter_args.vm.out"
chk "A interp==C  逐字节" "cmp -s $W/iter_args.interp.out $W/iter_args.c.out"
chk "A 定点：join_gen=1-2-3" "grep -q '3  join_gen=1-2-3' $W/iter_args.interp.out"
chk "A 定点：join_gen_str=中文" "grep -q '5  join_gen_str=中文' $W/iter_args.interp.out"
chk "A 定点：sorted_gen=[1, 2, 3]" "grep -q '11 sorted_gen=\[1, 2, 3\]' $W/iter_args.interp.out"
chk "A 定点：rev_gen=[3, 2, 1]" "grep -q '17 rev_gen=\[3, 2, 1\]' $W/iter_args.interp.out"
chk "A 定点：rev_str_cjk=文中（rune 级）" "grep -q '19 rev_str_cjk=文中' $W/iter_args.interp.out"
chk "A 定点：contains_gen=true" "grep -q '26 contains_gen=true' $W/iter_args.interp.out"

echo "=== [2] 用例 B：str 作为可迭代实参的面（含 emoji / 内嵌 NUL）"
run_tracks str_args
for t in interp vm c; do
    chk "B/$t rc=0" "[ \"\$(cat $W/str_args.$t.rc)\" = 0 ]"
    chk "B/$t 含 M178B-STR-ARGS-OK" "grep -q 'M178B-STR-ARGS-OK' $W/str_args.$t.out"
done
chk "B interp==VM 逐字节" "cmp -s $W/str_args.interp.out $W/str_args.vm.out"
chk "B interp==C  逐字节" "cmp -s $W/str_args.interp.out $W/str_args.c.out"
chk "B 定点：join_str=a-b-c" "grep -q '1  join_str=a-b-c' $W/str_args.interp.out"
chk "B 定点：sorted_str_cjk=[b, 中, 文]" "grep -q '4  sorted_str_cjk=\[b, 中, 文\]' $W/str_args.interp.out"
chk "B 定点：rev_str_emoji=b😀a" "grep -q '11 rev_str_emoji=b😀a' $W/str_args.interp.out"
chk "B 定点：rev_str_nul=3（内嵌 NUL 不截断）" "grep -q '12 rev_str_nul=3' $W/str_args.interp.out"

echo "=== [3] 用例 C：真不支持的类型 ⇒ 三轨同码（R1002）+ 同文案 + stdout 为 before"
printf 'before\n' > "$W/expect_before"
declare -A MSG=(
  [err_join_int]='join 第二参数需要 list/tuple/生成器/字符串，实际是 int'
  [err_sorted_int]='sorted 参数需要 list/tuple/生成器/字符串，实际是 int'
  [err_reversed_int]='reversed 参数需要 list/tuple/生成器/字符串，实际是 int'
  [err_contains_int]='contains 不支持类型 int'
)
for c in err_join_int err_sorted_int err_reversed_int err_contains_int; do
    run_tracks "$c"
    for t in interp vm c; do
        chk "C/$c/$t rc≠0" "[ \"\$(cat $W/$c.$t.rc)\" != 0 ]"
        chk "C/$c/$t stdout 恰为 before" "cmp -s $W/$c.$t.out $W/expect_before"
        chk "C/$c/$t stderr 含 R1002 + 统一文案" "grep -q 'R1002' $W/$c.$t.err && grep -qF \"${MSG[$c]}\" $W/$c.$t.err"
    done
done

echo "=== [4] 登记项（不计失败）：缺陷 195 —— 算术/比较运算的码与措辞三轨未统一"
run_tracks err_sum_str
for t in interp vm c; do
    echo "   [195] $t rc=$(cat $W/err_sum_str.$t.rc) err=$(head -1 $W/err_sum_str.$t.err)"
done
echo "   ↑ 三轨都响亮报错（rc≠0）；措辞已在 M179 收口为 `无法相加: int + string`（此处只作人证输出）"

if [ $NEG -eq 1 ]; then
echo "=== [5] 负控 3 道（各自独立判红 + 逐字节还原）"
SHA_RT=$(sha256sum "$RT" | cut -c1-16); SHA_IB=$(sha256sum "$IB" | cut -c1-16)

echo "── NC-A runtime：reversed(str) 改回**按字节**反转 ⇒ C/VM 轨必须与解释轨不一致"
snapshot
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
o1 = '        int nr = px_unicode_len_n(s, nb);\n'
o2 = '        for (int i = 0; i < nr; i++) { tab[i] = off; off += px_utf8_step((const unsigned char*)s + off, nb - off); }\n'
assert s.count(o1) == 1 and s.count(o2) == 1, (s.count(o1), s.count(o2))
s = s.replace(o1, '        int nr = nb;   // NC-A：按字节（旧缺陷）\n')
s = s.replace(o2, '        for (int i = 0; i < nr; i++) { tab[i] = off; off += 1; }   // NC-A\n')
open(p, 'w', encoding='utf-8').write(s)
PYEOF
run_tracks iter_args
if cmp -s "$W/iter_args.interp.out" "$W/iter_args.c.out"; then
    chk "NC-A 判红（C 轨与解释轨不一致）" "false"
else
    chk "NC-A 判红（C 轨按字节反转 ⇒ 中文行不同）" "! grep -q '19 rev_str_cjk=文中' $W/iter_args.c.out"
fi
restore_all
chk "NC-A 还原逐字节（runtime.c）" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA_RT\" ]"
run_tracks iter_args
chk "NC-A 还原后复绿" "cmp -s $W/iter_args.interp.out $W/iter_args.c.out && grep -q 'M178A-ITERABLE-ARGS-OK' $W/iter_args.c.out"

echo "── NC-B 解释轨：reversed(str) 改回收 rune 进 list ⇒ 解释轨必须与编译轨不一致"
python3 - "$IB" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
o = '''    elif t == "string":
        # M178：**字符串入参 → 字符串出参**（与编译轨同形）。
        #   修前解释轨把 rune 收进 list 返回 ⇒ 三轨分叉（`reversed("abc")`：
        #   解释轨 `[c, b, a]` / 编译轨 `cba`）。编译轨另有「按字节反转」缺陷
        #   （`reversed("中文")` 产出**非法 UTF-8**，实测 `\\xad\\x87...`），同批改 **rune 级**。
        var si = len(args[0]) - 1
        var s = ""
        while si >= 0:
            s = s + args[0][si]
            si -= 1
        return Ok(s)
'''
n = '''    elif t == "string":
        var si = 0
        while si < len(args[0]):
            items.append(args[0][si])
            si += 1
'''
assert s.count(o) == 1, s.count(o)
open(p, 'w', encoding='utf-8').write(s.replace(o, n))
PYEOF
build_dev || echo "  (dev 重建失败)"
"$DEV_PXI" "$D/iter_args.px" > "$W/ncb.interp.out" 2> "$W/ncb.interp.err"; echo $? > "$W/ncb.interp.rc"
if cmp -s "$W/ncb.interp.out" "$W/iter_args.c.out"; then
    chk "NC-B 判红（解释轨与编译轨不一致）" "false"
else
    chk "NC-B 判红（解释轨 rev_str 变 list）" "! grep -q '18 rev_str=abc' $W/ncb.interp.out"
fi
restore_all
chk "NC-B 还原逐字节（ibuiltin.px）" "[ \"\$(sha256sum $IB | cut -c1-16)\" = \"$SHA_IB\" ]"
build_dev || echo "  (dev 重建失败)"
"$DEV_PXI" "$D/iter_args.px" > "$W/ncb2.interp.out" 2>/dev/null
chk "NC-B 还原后复绿" "cmp -s $W/ncb2.interp.out $W/iter_args.c.out"

echo "── NC-C runtime：join 改回只收 list/tuple（拒生成器）⇒ C/VM 轨必须红"
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]; s = open(p, encoding='utf-8').read()
o = '    if (!px_as_list(args[1], &xs)) { px_root_pop(); px_error("R1002: join 第二参数需要 list/tuple/生成器/字符串，实际是 %s", px_type_name(args[1])); }\n'
assert s.count(o) == 1, s.count(o)
s = s.replace(o, '    if (args[1].type == PX_GEN) { px_root_pop(); px_error("R1002: NC-C join 拒绝生成器"); }\n' + o)
open(p, 'w', encoding='utf-8').write(s)
PYEOF
run_tracks iter_args
chk "NC-C 判红（C 轨 rc≠0）" "[ \"\$(cat $W/iter_args.c.rc)\" != 0 ]"
chk "NC-C 判红（C 轨 stderr 含 NC-C）" "grep -q 'NC-C join 拒绝生成器' $W/iter_args.c.err"
chk "NC-C 解释轨仍绿（判据不对称是预期的）" "[ \"\$(cat $W/iter_args.interp.rc)\" = 0 ]"
restore_all
chk "NC-C 还原逐字节（runtime.c）" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA_RT\" ]"
run_tracks iter_args
chk "NC-C 还原后复绿" "cmp -s $W/iter_args.interp.out $W/iter_args.c.out && cmp -s $W/iter_args.interp.out $W/iter_args.vm.out"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M178-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
