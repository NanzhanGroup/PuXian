#!/usr/bin/env bash
# ============================================================
# M89-S3-A4: bc4_verify.sh —— 调用闭环端到端验证（递归/深链/main 退出码）
# ------------------------------------------------------------
# 依赖：bc_cli 已 build + bc_run.sh（含 vm.o 的 rtcache）。
# bc4.px main 返回 63（add(3,4)=7 + fib(10)=55 + deep(20000)=7 - 6）
#  → emit-c 产物进程退出码 = 63（main 调用约定 + 退出码转换 driver）。
# 退出码：0=ALL PASS（退出码 63）；1=失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc4.px > /tmp/bc4_out.txt 2>&1
code=$?
echo "$code" > /tmp/bc4_exit.txt
if [ "$code" -eq 63 ]; then
    echo "✅ main 退出码 = 63（add(3,4)+fib(10)+deep(20000)-6 全链路正确）"
    echo "== A4 bc4 调用闭环 ALL PASS =="
    exit 0
fi
echo "❌ 退出码期望 63，实际 $code —— 输出见 /tmp/bc4_out.txt"
cat /tmp/bc4_out.txt
exit 1
