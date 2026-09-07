#!/usr/bin/env bash
# ============================================================
# M89-S3-B1: m89_b1_parity_compare.sh —— 容器/方法/For 双轨 stdout 对拍
# ------------------------------------------------------------
# m89_b1_parity.px（list/tuple/dict 字面量、索引读写、切片、dict 字段、
#   list.push/str.to_upper 方法桥、for-in 迭代、len）——
#   旧 C codegen（px build 产物） vs VM（bc_run emit-c） stdout 逐字节一致。
# 退出码：0=一致；非 0=有差异。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
./tools/px build examples/m89_a2/m89_b1_parity.px >/tmp/pb_build.log 2>&1 || { echo "❌ px build 失败"; exit 1; }
./examples/m89_a2/build/m89_b1_parity > /tmp/pb_c.txt 2>&1
./examples/m89_a2/bc_run.sh examples/m89_a2/m89_b1_parity.px > /tmp/pb_vm.txt 2>/tmp/pb_vm_err.txt || { echo "❌ bc_run 失败"; cat /tmp/pb_vm_err.txt; exit 1; }
if diff -q /tmp/pb_c.txt /tmp/pb_vm.txt >/dev/null; then
    echo "== B1 容器双轨 stdout 逐字节一致（VM=旧C）PASS =="
    exit 0
fi
echo "❌ 双轨 stdout 有差异："
echo "--- 旧 C ---"; cat /tmp/pb_c.txt
echo "--- VM ---"; cat /tmp/pb_vm.txt
exit 1
