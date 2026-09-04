#!/usr/bin/env bash
# M64-S5 test 运行器自测验证
set -u
cd "$(dirname "$0")/../.." || exit 1
PXC="$PWD/tools/pxc"
pass=0; fail=0
chk() { # name cond
    if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi
}
echo "[1] 全量运行 sample.px：4 测试 3 过 1 败 → rc=1"
out=$("$PXC" test examples/m64_test/sample.px 2>&1); rc=$?
chk "rc=1（有失败）" "[ $rc -eq 1 ]"
chk "PASS 3 个" "[ $(echo "$out" | grep -c '\[PASS\]') -eq 3 ]"
chk "FAIL test_fail" "echo \"$out\" | grep -q '\[FAIL\] test_fail'"
chk "结果汇总 3/4" "echo \"$out\" | grep -q '结果: 3/4 通过, 1/4 失败'"
chk "顶层可执行语句已剔除" "! echo \"$out\" | grep -q 'TOP_LEVEL_SHOULD_NOT_RUN'"
chk "def main 已剔除" "! echo \"$out\" | grep -q 'MAIN_SHOULD_NOT_RUN'"
chk "断言错误详情透传（boom fail）" "echo \"$out\" | grep -q 'boom fail'"
echo "[2] filter 只跑匹配测试"
out2=$("$PXC" test examples/m64_test/sample.px helper 2>&1); rc2=$?
chk "filter rc=0" "[ $rc2 -eq 0 ]"
chk "filter 仅 1 个 PASS（test_helper）" "[ $(echo "$out2" | grep -c '\[PASS\]') -eq 1 ]"
echo "[3] --list 列出测试"
out3=$("$PXC" test --list examples/m64_test/sample.px 2>&1)
chk "list 4 个 test_*（test_with_args 有参不算）" "[ $(echo "$out3" | grep -c '^test_') -eq 4 ]"
echo "[4] 无测试函数文件 → 提示 + rc=0"
out4=$("$PXC" test examples/m64_bench/bench.px 2>&1); rc4=$?
chk "无测试提示" "echo \"$out4\" | grep -q '未发现测试函数'"
chk "rc4=0" "[ $rc4 -eq 0 ]"
echo
echo "结果: $pass 通过 / $fail 失败"
[ $fail -eq 0 ] || exit 1
