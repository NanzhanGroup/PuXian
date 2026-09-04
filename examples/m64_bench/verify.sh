#!/usr/bin/env bash
# M64-S5 bench 工具自测验证
set -u
cd "$(dirname "$0")/../.." || exit 1
PXC="$PWD/tools/pxc"
pass=0; fail=0
chk() { # name cond
    if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi
}
echo "[1] bench_target 基准（10 次 × 2 轮；pxi 解释器执行，count 取小避免超时）"
out=$("$PXC" bench examples/m64_bench/bench.px bench_target --count 10 --repeat 2 2>&1); rc=$?
chk "rc=0" "[ $rc -eq 0 ]"
chk "基准头行" "echo \"$out\" | grep -q '基准: examples/m64_bench/bench.px (函数 bench_target)'"
chk "每轮输出" "[ $(echo "$out" | grep -c '第 .* 轮: 总') -eq 2 ]"
chk "轮含 ms/次" "echo \"$out\" | grep -q '平均 .* ms/次'"
chk "汇总行" "echo \"$out\" | grep -q '汇总: min'"
chk "顶层可执行已剔除" "! echo \"$out\" | grep -q 'TOP_SHOULD_NOT_RUN'"
echo "[2] 不存在函数报错 rc=1"
out2=$("$PXC" bench examples/m64_bench/bench.px no_such_fn --count 10 2>&1); rc2=$?
chk "未找到函数 rc=1" "[ $rc2 -eq 1 ]"
chk "错误消息" "echo \"$out2\" | grep -q \"未找到函数 'no_such_fn'\""
echo
echo "结果: $pass 通过 / $fail 失败"
[ $fail -eq 0 ] || exit 1
