#!/usr/bin/env bash
# ============================================================
# M174 门 · `d.get(k[, default])` 语义统一（缺陷 194）+ 缺键/坏键口径回归
# ------------------------------------------------------------
# 定调（文档与解释轨一致）：`d.get(k, def)` 的默认值**只覆盖「键不存在」**；
#   键存在但值为 null ⇒ 返回 null（= Go 两值语义 / Python dict.get）。
#   出处：docs/DICT_STRICT_MIGRATION.md 与 docs/PUXIAN_CHEATSHEET.md。
# 病灶：runtime 侧用「取出的值是不是 null」判存在性 ⇒ VM/C 轨在「键存在但值 null +
#   给了默认值」时返回默认值，解释轨返回 null ⇒ 三轨分叉。
#
# 判据：
#   [1] 用例 A（语义矩阵 12 条）三轨：rc=0 · stdout 逐字节一致 · 含 OK 行
#   [2] 用例 B（缺键下标读）三轨：rc≠0 · stdout 恰为 "before" · stderr 含 R1008 + 统一词条
#   [3] 用例 C（get 非字符串键）三轨：rc≠0 · stdout 恰为 "before" · stderr 含 R1002 + 统一词条
#   [4] 负控：把 runtime 的 has 判定改回 null 判定 ⇒ 用例 A 的 VM 轨必须与解释轨**不一致**
#       （各自独立判红 + sha256 逐字节还原复绿）
# CI 用 --neg-skip（负控要重编 runtime，~1min）。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m174_dict_get
RT="$ROOT/runtime/runtime.c"
BAK=/tmp/m174_runtime.bak
W=/tmp/m174_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }
snapshot() { cp -f "$RT" "$BAK"; }
restore_all() { [ -f "$BAK" ] && cp -f "$BAK" "$RT"; }
trap 'restore_all; rm -f "$BAK"' EXIT

run_tracks() {   # $1=用例名（产出 <W>/<name>.{interp,vm,c}.{out,err,rc}）
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

echo "=== [1] 用例 A：d.get 语义矩阵（三轨逐字节一致）"
run_tracks get_matrix
for t in interp vm c; do
    chk "A/$t rc=0" "[ \"\$(cat $W/get_matrix.$t.rc)\" = 0 ]"
    chk "A/$t 含 M174A-GET-MATRIX-OK" "grep -q 'M174A-GET-MATRIX-OK' $W/get_matrix.$t.out"
    chk "A/$t stderr 空" "[ ! -s $W/get_matrix.$t.err ]"
done
chk "A interp==VM 逐字节" "cmp -s $W/get_matrix.interp.out $W/get_matrix.vm.out"
chk "A interp==C  逐字节" "cmp -s $W/get_matrix.interp.out $W/get_matrix.c.out"
# 缺陷 194 的**定点判据**：键存在 + 值 null + 给了默认值 ⇒ 必须是 null（不是默认值）
chk "A 定点：exist_null_with_default=null" "grep -q '1 exist_null_with_default=null:null' $W/get_matrix.interp.out"

echo "=== [2] 用例 B：缺键下标读 ⇒ R1008（三轨）"
run_tracks err_missing_key
printf 'before\n' > "$W/expect_before"
for t in interp vm c; do
    chk "B/$t rc≠0" "[ \"\$(cat $W/err_missing_key.$t.rc)\" != 0 ]"
    chk "B/$t stdout 恰为 before" "cmp -s $W/err_missing_key.$t.out $W/expect_before"
    chk "B/$t stderr 含 R1008" "grep -q 'R1008' $W/err_missing_key.$t.err"
    chk "B/$t stderr 含统一词条" "grep -q \"字典没有键 'nope'\" $W/err_missing_key.$t.err"
done

echo "=== [3] 用例 C：get 非字符串键 ⇒ R1002（三轨）"
run_tracks err_get_badkey
for t in interp vm c; do
    chk "C/$t rc≠0" "[ \"\$(cat $W/err_get_badkey.$t.rc)\" != 0 ]"
    chk "C/$t stdout 恰为 before" "cmp -s $W/err_get_badkey.$t.out $W/expect_before"
    chk "C/$t stderr 含 R1002" "grep -q 'R1002' $W/err_get_badkey.$t.err"
    chk "C/$t stderr 含统一词条" "grep -q '方法 get 参数 1 需要 string' $W/err_get_badkey.$t.err"
done

if [ $NEG -eq 1 ]; then
echo "=== [4] 负控：runtime 的 has 判定改回 null 判定 ⇒ 用例 A 的 VM 轨必须与解释轨不一致"
snapshot
SHA0=$(sha256sum "$RT" | cut -c1-16)
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '''            const char* gk = args[0].as.obj->as.str.data;
            if (!px_dict_has(obj, gk)) {
                if (nargs >= 2) return args[1];
                return px_null();
            }
            return px_dict_get(obj, gk);'''
assert s.count(old) == 1, s.count(old)
new = '''            LXValue gv = px_dict_get(obj, args[0].as.obj->as.str.data);
            if (px_is_null(gv) && nargs >= 2) return args[1];
            return gv;'''
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PYEOF
run_tracks get_matrix
cp -f "$W/get_matrix.vm.out" "$W/nc.vm.out"
cp -f "$W/get_matrix.vm.err" "$W/nc.vm.err"
cp -f "$W/get_matrix.vm.rc" "$W/nc.vm.rc"
cp -f "$W/get_matrix.vm.build.log" "$W/nc.vm.build.log"
if cmp -s "$W/get_matrix.interp.out" "$W/get_matrix.vm.out"; then
    chk "NC 判红（VM 与解释轨不一致）" "false"
else
    chk "NC 判红（VM 与解释轨不一致）" "grep -q 'exist_null_with_default=.*:DEF' $W/get_matrix.vm.out"
fi
restore_all
chk "NC 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
run_tracks get_matrix
chk "NC 还原后复绿" "cmp -s $W/get_matrix.interp.out $W/get_matrix.vm.out && grep -q 'M174A-GET-MATRIX-OK' $W/get_matrix.vm.out"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M174-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
