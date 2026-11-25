#!/usr/bin/env bash
# ============================================================
# M177 门 · 内置面统一（第 57 轮 · 缺陷 165 的审计发现）
# ------------------------------------------------------------
# 缺陷 165 原本登记为「GenExp 多 for/多变量的**物化路径**捕获语义未逐条核查」。
# 把它按**矩阵**展开之后（内置 × 实参形态 × 三轨）跳出来的是一整族结构性缺口：
#
#   ① **解释轨有、编译轨没有**的内置：`dict()` / `unique()` / `flatten()`
#      ⇒ 照解释轨写、一上编译轨就 `R1001 未定义变量`；而 lint 名册取的是**并集**，
#        门 ③④ 只看「⊆ 名册」⇒ **全绿直到真去编译**（实测 `let d = dict()` 编译轨 R1001）。
#   ② `min(x)` / `max(x)` 单参数：编译轨**静默返回实参本身**
#      ⇒ `min(gen)` 得到的是**生成器对象**（错值，不是报错）；解释轨则报「需要两个参数」。
#   ③ `sum`：编译轨只收 list、解释轨收 list/tuple ⇒ 同一份源码两轨不同。
#
# 修法：runtime 补 `dict/unique/flatten`；`min/max` 支持「单参数可迭代 ⇒ 元素最值」；
#   `sum` 收 list/tuple/生成器；**三处共用一条迭代语义**（`px_as_list` = `px_len`+`px_iter_at`，
#   与 `for x in xs` 同源）。解释轨对称补齐（min/max 变 variadic、sum 收生成器）。
#   名册门 `builtin_list_check.sh` 新增判据 ⑦「解释轨注册名 ⊆ runtime 可达名」——
#   把这类「两轨内置集不同」从**无人可测**变成**门能红**。
#
# 判据：
#   [1] 用例 A（20 条内置面矩阵）三轨：rc=0 · stdout **逐字节一致**（「统一」的定义）
#   [2] 用例 B/C（真正不支持的类型 / 空可迭代）三轨：rc≠0 · stdout 恰为 before ·
#       stderr 含 R1002 + **统一文案**
#   [3] 名册门 ⑦ 全绿（两轨内置集一致）
#   [4] 负控：把 `min/max` 的单参数分支改回「直接返回实参」⇒ 用例 A 的编译轨必须与
#       解释轨**不一致**（定点字符串）；sha256 逐字节还原复绿
#   [5] 外部定标（M445 试点）：用例 A 的 20 条期望值不再「照输出抄」，而是
#       oracle/truth.py（CPython 独立实现）算出的 builtins.oracle.tsv。
#       三轨（interp/vm/c）每一条都要与该表逐条相等 —— 口径从 A==B==C 升为 A==B==C==**期望**。
#       算不出的（px 特有规约）必须显式标 px-spec 并写 note 理由，数量必须对上。
#       负控：篡改 oracle 副本一个值 ⇒ 必红（否则「外部定标」是空话）。
# CI 用 --neg-skip。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m177_builtin_parity
RT="$ROOT/runtime/runtime.c"
BAK=/tmp/m177_runtime.bak
W=/tmp/m177_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }
snapshot() { cp -f "$RT" "$BAK"; }
restore_all() { [ -f "$BAK" ] && cp -f "$BAK" "$RT"; }
trap 'restore_all; rm -f "$BAK"' EXIT

# 前置不变量（M176 教训：trap 挡不住 SIGKILL，上一轮被 kill 会留下脏源码）
grep -q 'return px_minmax_iter(args\[0\], 0, "min");' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 min 单参数分支锚点（上一轮被 kill 留下脏源码？）" >&2; exit 2; }

run_tracks() {   # $1=用例名
    local n=$1 args
    "$ROOT/tools/px" run "$D/$n.px" > "$W/$n.interp.out" 2> "$W/$n.interp.err"; echo $? > "$W/$n.interp.rc"
    for t in vm c; do
        args=""; [ "$t" = "c" ] && args="--c"
        rm -rf "$D/build"
        if (cd "$ROOT" && ./tools/px build $args "$D/$n.px" > "$W/$n.$t.build.log" 2>&1) && [ -x "$D/build/$n" ]; then
            "$D/build/$n" > "$W/$n.$t.out" 2> "$W/$n.$t.err"; echo $? > "$W/$n.$t.rc"
        else
            : > "$W/$n.$t.out"; : > "$W/$n.$t.err"; echo 99 > "$W/$n.$t.rc"
        fi
        rm -rf "$D/build"
    done
}

echo "=== [1] 用例 A：内置面矩阵 20 条（三轨逐字节一致）"
run_tracks iter_builtins
for t in interp vm c; do
    chk "A/$t rc=0" "[ \"\$(cat $W/iter_builtins.$t.rc)\" = 0 ]"
    chk "A/$t 含 M177A-BUILTIN-PARITY-OK" "grep -q 'M177A-BUILTIN-PARITY-OK' $W/iter_builtins.$t.out"
done
chk "A interp==VM 逐字节" "cmp -s $W/iter_builtins.interp.out $W/iter_builtins.vm.out"
chk "A interp==C  逐字节" "cmp -s $W/iter_builtins.interp.out $W/iter_builtins.c.out"
chk "A 定点：dict() 可用" "grep -q '1 dict=dict len=0' $W/iter_builtins.interp.out"
chk "A 定点：min_gen=1（不再返回生成器对象）" "grep -q '9 min_gen=1' $W/iter_builtins.interp.out"
chk "A 定点：sum_tuple=6" "grep -q '5 sum_tuple=6' $W/iter_builtins.interp.out"
chk "A 定点：unique_gen=[0, 1, 2]" "grep -q '17 unique_gen=\[0, 1, 2\]' $W/iter_builtins.interp.out"

echo "=== [2] 用例 B/C：真不支持的类型 / 空可迭代 ⇒ 三轨同码同文"
printf 'before\n' > "$W/expect_before"
run_tracks err_sum_int
run_tracks err_min_empty
for t in interp vm c; do
    chk "B/$t rc≠0 + stdout 恰为 before" "[ \"\$(cat $W/err_sum_int.$t.rc)\" != 0 ] && cmp -s $W/err_sum_int.$t.out $W/expect_before"
    chk "B/$t stderr 含 R1002 + 统一文案" "grep -q 'R1002' $W/err_sum_int.$t.err && grep -q 'sum 参数需要 list/tuple/生成器' $W/err_sum_int.$t.err"
    chk "C/$t rc≠0 + stdout 恰为 before" "[ \"\$(cat $W/err_min_empty.$t.rc)\" != 0 ] && cmp -s $W/err_min_empty.$t.out $W/expect_before"
    chk "C/$t stderr 含 R1002 + 统一文案" "grep -q 'R1002' $W/err_min_empty.$t.err && grep -q 'min 需要至少一个元素' $W/err_min_empty.$t.err"
done

echo "=== [3] 名册门 ⑦：两轨内置集一致"
o=$(bash "$ROOT/selfhost/builtin_list_check.sh" 2>&1); rc=$?
chk "builtin_list_check rc=0" "[ $rc -eq 0 ]"
chk "含 ⑦ 两轨内置集一致" "echo \"\$o\" | grep -q '⑦ 两轨内置集一致'"

echo "=== [5] 外部定标：用例 A 的 20 条 ⇄ oracle（CPython 独立真值，M445 试点）"
ORA=$D/oracle/builtins.oracle.tsv
PY=$D/oracle/truth.py
if ! command -v python3 >/dev/null 2>&1; then
    echo "❌ ENV rc=3 缺 python3（外部定标无法现场重算，不降级成绿）"; exit 3
fi
if [ ! -e "$ORA" ] || [ ! -e "$PY" ]; then
    chk "外部定标文件在位（builtins.oracle.tsv + truth.py）" "false"
else
    chk "外部定标文件在位（builtins.oracle.tsv + truth.py）" "true"
    if python3 "$PY" > "$W/oracle.regen" 2> "$W/oracle.regen.err"; then
        chk "[5a] truth.py 现场重算 rc=0" "true"
        chk "[5a] 现场重算 == 落盘 oracle 逐字节（落盘被改/没重生成 ⇒ 红）" "cmp -s $W/oracle.regen $ORA"
    else
        chk "[5a] truth.py 现场重算 rc=0" "false"
        tail -3 "$W/oracle.regen.err" | sed 's/^/     /'
    fi
    awk -F'\t' '!/^#/ && NF>=2 {print $1"\t"$2}' "$ORA" > "$W/oracle.exp"
    for t in interp vm c; do
        awk '!/M177A-BUILTIN-PARITY-OK/ && NF>0 {sub(/ /,"\t"); print}' "$W/iter_builtins.$t.out" > "$W/$t.oracle.out"
        chk "[5b] 用例 A/$t 输出 == 外部 oracle（$t 轨逐条等于 CPython 真值）" "cmp -s $W/oracle.exp $W/$t.oracle.out"
    done
    nspec=$(awk -F'\t' '!/^#/ && $3=="px-spec"' "$ORA" | wc -l)
    nnote=$(grep -c '^# note\[' "$ORA" || true)
    chk "[5c] px-spec 行数($nspec) == note 理由数($nnote)（无外部参照必须写明）" "[ \"$nspec\" = \"$nnote\" ]"
    sed $'s/^4\tsum_list=6\t/4\tsum_list=7\t/' "$ORA" > "$W/oracle.neg"
    if cmp -s "$W/oracle.neg" "$ORA"; then
        chk "[5d] 负控前提自证：oracle 副本已篡改" "false"
    else
        chk "[5d] 负控前提自证：oracle 副本已篡改（sum_list 6→7）" "true"
        awk -F'\t' '!/^#/ && NF>=2 {print $1"\t"$2}' "$W/oracle.neg" > "$W/oracle.neg.exp"
        chk "[5d] 判据识破 oracle 篡改（无牙则此处红）" "! cmp -s $W/oracle.neg.exp $W/interp.oracle.out"
    fi
fi

if [ $NEG -eq 1 ]; then
echo "=== [6] 负控：min/max 单参数改回「直接返回实参」⇒ 用例 A 编译轨必须与解释轨不一致"
snapshot
SHA0=$(sha256sum "$RT" | cut -c1-16)
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '''        if (args[0].type == PX_LIST || args[0].type == PX_TUPLE ||
            args[0].type == PX_GEN || args[0].type == PX_STR)
            return px_minmax_iter(args[0], 0, "min");'''
assert s.count(old) == 1, s.count(old)
open(p, 'w', encoding='utf-8').write(s.replace(old, '        if (0) return px_minmax_iter(args[0], 0, "min");   // NC'))
PYEOF
run_tracks iter_builtins
if cmp -s "$W/iter_builtins.interp.out" "$W/iter_builtins.vm.out"; then
    chk "NC 判红（VM 与解释轨不一致）" "false"
else
    chk "NC 判红（VM 与解释轨不一致）" "grep -q '9 min_gen=<gen' $W/iter_builtins.vm.out || grep -q 'min_gen=0 items' $W/iter_builtins.vm.out"
fi
restore_all
chk "NC 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
run_tracks iter_builtins
chk "NC 还原后复绿" "cmp -s $W/iter_builtins.interp.out $W/iter_builtins.vm.out && grep -q 'M177A-BUILTIN-PARITY-OK' $W/iter_builtins.vm.out"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M177-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
