#!/usr/bin/env bash
# ============================================================
# M89-S3-A4: hello_compare.sh —— hello.px 三轨 stdout 逐字节对拍
# ------------------------------------------------------------
# VM（emit-c）输出 vs 旧 C codegen 产物（px build）输出 vs pxi 解释输出。
# hello.px 覆盖：main 调用约定 + 局部变量 + if/and 短路 + print(native CALL)
#              + Pipe(msg |> to_upper())。无容器 → A4 即可全跑。
# 退出码：0=三轨逐字节一致；非 0=有差异。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
./tools/px build examples/hello.px >/tmp/hello_build.log 2>&1 || { echo "❌ px build hello 失败"; exit 1; }
./examples/build/hello > /tmp/hello_c.txt 2>&1
./tools/px run examples/hello.px > /tmp/hello_pxi.txt 2>&1
./examples/m89_a2/bc_run.sh examples/hello.px 2>/dev/null > /tmp/hello_vm.txt
echo "── 旧 C 产物输出："; cat /tmp/hello_c.txt
echo "── VM 输出："; cat /tmp/hello_vm.txt
if diff -q /tmp/hello_c.txt /tmp/hello_vm.txt >/dev/null && diff -q /tmp/hello_c.txt /tmp/hello_pxi.txt >/dev/null; then
    echo "== A4 hello.px 三轨 stdout 逐字节一致（VM=旧C=pxi）PASS =="
    exit 0
fi
echo "❌ stdout 有差异："
echo "--- 旧 C vs VM ---"; diff /tmp/hello_c.txt /tmp/hello_vm.txt
echo "--- 旧 C vs pxi ---"; diff /tmp/hello_c.txt /tmp/hello_pxi.txt
exit 1
