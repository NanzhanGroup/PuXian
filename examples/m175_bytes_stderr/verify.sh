#!/usr/bin/env bash
# ============================================================
# M175 门 · 台账两小项收口（缺陷 153 `len(bytes)` + 缺陷 14 `os_popen` 的 stderr 去向）
# ------------------------------------------------------------
# 153：`len(bytes)` 修前落进 `px_len` 的 default ⇒ `len 不支持类型 bytes`（只能用 `bytes_len`）。
#      而解释轨的 `i_builtin_len` **本来就写了 bytes 分支**（`len(args[0])`）—— 那一步最终
#      调到的正是同一个 runtime 函数 ⇒「同一份意图、两处实现」的又一例。定调：**每个类型按
#      自己的自然单位**（str = rune 数；bytes = 字节数）。
# 14：`os_popen` 的子进程只被 dup2 了 0/1，**fd 2 继承宿主** ⇒ 子进程 stderr 直接漏进宿主
#      stderr（同族的 os_capture / os_spawn_capture 一直是分离捕获的，只有它漏了这一环）。
#     修法：第 3 参 `opts{"stderr":"inherit"（默认）/ "pipe" / "null"}` —— **纯增量**：
#       · inherit（不传 opts）⇒ 逐字节保持旧行为（本门用**宿主 stderr 里应当出现 CHILD-ERR** 反证）
#       · pipe ⇒ 子进程 stderr 被管道接走，返回值多一个 `stderr_fd`
#       · null ⇒ 接进 /dev/null（连 fd 都不占）
#
# 判据：
#   [1] 用例 A（len(bytes) 语义矩阵 12 条）三轨：rc=0 · stdout 逐字节一致
#   [2] B1 默认 / B2 pipe / B3 null（三轨）：
#       B1 宿主 stderr **含** CHILD-ERR；B2/B3 宿主 stderr **不含**；三者 stdout 都含各自 OK 行
#   [3] B4 opts.stderr 非法取值（三轨）：rc≠0 · stdout 恰为 before · stderr 含 R1002 + 枚举文案
#   [4] 负控：去掉 `px_len` 的 PX_BYTES 分支 ⇒ 用例 A 的编译轨必须变红；去掉 stderr 开关 ⇒
#       B2 的宿主 stderr 判据必须变红（各自独立 + sha256 逐字节还原复绿）
# CI 用 --neg-skip（负控要重编 runtime，~1min×2）。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m175_bytes_stderr
RT="$ROOT/runtime/runtime.c"
BAK=/tmp/m175_runtime.bak
W=/tmp/m175_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }
snapshot() { cp -f "$RT" "$BAK"; }
restore_all() { [ -f "$BAK" ] && cp -f "$BAK" "$RT"; }
trap 'restore_all; rm -f "$BAK"' EXIT

# 单轨跑一个用例：$1=用例 $2=轨 → 落 <W>/<name>.<track>.{out,err,rc}（宿主 stderr 单独一份）
run_one() {
    local n=$1 t=$2 args=""
    [ "$t" = "c" ] && args="--c"
    rm -rf "$D/build"
    if ! (cd "$ROOT" && ./tools/px build $args "$D/$n.px" > "$W/$n.$t.build.log" 2>&1) || [ ! -x "$D/build/$n" ]; then
        : > "$W/$n.$t.out"; : > "$W/$n.$t.err"; echo 99 > "$W/$n.$t.rc"; rm -rf "$D/build"; return 0
    fi
    "$D/build/$n" > "$W/$n.$t.out" 2> "$W/$n.$t.err"; echo $? > "$W/$n.$t.rc"
    rm -rf "$D/build"
}
run_interp() {
    local n=$1
    "$ROOT/tools/px" run "$D/$n.px" > "$W/$n.interp.out" 2> "$W/$n.interp.err"; echo $? > "$W/$n.interp.rc"
}

echo "=== [1] 用例 A：len(bytes) 语义矩阵（三轨逐字节一致）"
for t in interp vm c; do
    [ "$t" = "interp" ] && run_interp len_bytes || run_one len_bytes "$t"
done
for t in interp vm c; do
    chk "A/$t rc=0" "[ \"\$(cat $W/len_bytes.$t.rc)\" = 0 ]"
    chk "A/$t 含 M175A-LEN-BYTES-OK" "grep -q 'M175A-LEN-BYTES-OK' $W/len_bytes.$t.out"
done
chk "A interp==VM 逐字节" "cmp -s $W/len_bytes.interp.out $W/len_bytes.vm.out"
chk "A interp==C  逐字节" "cmp -s $W/len_bytes.interp.out $W/len_bytes.c.out"
chk "A 定点：len(bytes)=1（单字节）" "grep -q '1 len_bytes_1=1' $W/len_bytes.interp.out"
chk "A 定点：str 是 rune 数（中文abc=5）" "grep -q '4 len_str_runes=5' $W/len_bytes.interp.out"
chk "A 定点：bytes 是字节数（中文abc=9）" "grep -q '5 bytes_of_str=9' $W/len_bytes.interp.out"

echo "=== [2] os_popen 的 stderr 去向（三轨 + 宿主 stderr 侧判据）"
for n in popen_default popen_pipe popen_null; do
    for t in interp vm c; do
        [ "$t" = "interp" ] && run_interp "$n" || run_one "$n" "$t"
    done
done
for t in interp vm c; do
    chk "B1/$t rc=0 + 含 OK" "[ \"\$(cat $W/popen_default.$t.rc)\" = 0 ] && grep -q 'M175B1-DEFAULT-OK' $W/popen_default.$t.out"
    # 反向判据：默认（=旧行为）子进程 stderr **必须**漏进宿主
    chk "B1/$t 宿主 stderr 含 CHILD-ERR（旧行为不变）" "grep -q 'CHILD-ERR' $W/popen_default.$t.err"
    chk "B2/$t rc=0 + 含 OK" "[ \"\$(cat $W/popen_pipe.$t.rc)\" = 0 ] && grep -q 'M175B2-PIPE-OK' $W/popen_pipe.$t.out"
    chk "B2/$t 宿主 stderr 干净（不再漏）" "! grep -q 'CHILD-ERR' $W/popen_pipe.$t.err"
    chk "B3/$t rc=0 + 含 OK" "[ \"\$(cat $W/popen_null.$t.rc)\" = 0 ] && grep -q 'M175B3-NULL-OK' $W/popen_null.$t.out"
    chk "B3/$t 宿主 stderr 干净" "! grep -q 'CHILD-ERR' $W/popen_null.$t.err"
done

echo "=== [3] B4：opts.stderr 非法取值 ⇒ R1002（三轨同码 + 枚举文案）"
printf 'before\n' > "$W/expect_before"
for t in interp vm c; do
    [ "$t" = "interp" ] && run_interp popen_badopt || run_one popen_badopt "$t"
done
for t in interp vm c; do
    chk "B4/$t rc≠0" "[ \"\$(cat $W/popen_badopt.$t.rc)\" != 0 ]"
    chk "B4/$t stdout 恰为 before" "cmp -s $W/popen_badopt.$t.out $W/expect_before"
    chk "B4/$t stderr 含 R1002" "grep -q 'R1002' $W/popen_badopt.$t.err"
    chk "B4/$t stderr 含枚举文案" "grep -q 'inherit.*pipe.*null' $W/popen_badopt.$t.err"
done

if [ $NEG -eq 1 ]; then
echo "=== [4] 负控 A：去掉 px_len 的 PX_BYTES 分支 ⇒ 用例 A 编译轨必须变红"
snapshot
SHA0=$(sha256sum "$RT" | cut -c1-16)
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '''        case PX_BYTES: return v.as.obj->as.str.len;
        case PX_LIST: return v.as.obj->as.list.len;'''
assert s.count(old) == 1, s.count(old)
open(p, 'w', encoding='utf-8').write(s.replace(old, '        case PX_LIST: return v.as.obj->as.list.len;'))
PYEOF
run_one len_bytes vm
chk "NC-A 判红（len(bytes) 又报不支持）" "[ \"\$(cat $W/len_bytes.vm.rc)\" != 0 ] || grep -q 'len 不支持类型 bytes' $W/len_bytes.vm.err"
restore_all
chk "NC-A 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"

echo "=== [4] 负控 B：去掉 stderr 开关（恒继承）⇒ B2 的宿主 stderr 判据必须变红"
python3 - "$RT" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = '''        if (err_mode == 1) {
            close(perr[0]);
            dup2(perr[1], 2);
            close(perr[1]);
        } else if (err_mode == 2) {'''
assert s.count(old) == 1, s.count(old)
new = '''        if (0) {
            close(perr[0]);
            dup2(perr[1], 2);
            close(perr[1]);
        } else if (0) {'''
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PYEOF
run_one popen_pipe vm
chk "NC-B 判红（宿主 stderr 又出现 CHILD-ERR）" "grep -q 'CHILD-ERR' $W/popen_pipe.vm.err"
restore_all
chk "NC-B 还原逐字节" "[ \"\$(sha256sum $RT | cut -c1-16)\" = \"$SHA0\" ]"
run_one popen_pipe vm
chk "NC-B 还原后复绿" "grep -q 'M175B2-PIPE-OK' $W/popen_pipe.vm.out && ! grep -q 'CHILD-ERR' $W/popen_pipe.vm.err"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M175-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
