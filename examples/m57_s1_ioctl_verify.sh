#!/usr/bin/env bash
# ============================================================
# M57 S1：ioctl 胶水内建端到端验证
#   语言新内建：open/close（fd 通道）+ ioctl(fd, request[, arg])
#              + os_errno()（失败 errno 查询）
# 验证点（全部在 .px 内 assert，失败即非 0 退出）：
#   A. open/close 设备文件 fd 通路 + 失败路径 errno（ENOENT/ENOTTY）
#   B. ioctl 于真实 TCP fd：FIONREAD 数值正确（bytes 就地填充 5/0）、
#      FIONBIO 设置生效（非阻塞 recv 立即返回）、类型不匹配/非法 fd
#      的 -1 + errno（ENOTTY/EBADF）、无 arg/int arg 形态
#   C. 真实设备探测（/dev/gpiochip*、/dev/i2c-*，存在则 open/close）
# ============================================================
set -u
cd "$(dirname "$0")/.."
PXC=./tools/pxc
B=examples/build

echo "== build =="
$PXC build examples/m57_s1_ioctl.px >/dev/null 2>&1 || { echo "FAIL: m57_s1_ioctl.px 编译失败"; exit 1; }

echo "== run =="
OUT=$($B/m57_s1_ioctl 2>&1)
RC=$?
echo "$OUT"
echo "== result =="
if [ $RC -ne 0 ]; then
    echo "❌ M57-S1: ioctl 自检退出码 $RC"
    exit 1
fi
echo "$OUT" | grep -q "B: FIONREAD=5" || { echo "❌ M57-S1: FIONREAD=5 断言未出现"; exit 1; }
echo "$OUT" | grep -q "B: FIONREAD drained=0" || { echo "❌ M57-S1: drained=0 未出现"; exit 1; }
echo "$OUT" | grep -q "B: FIONBIO 设置生效" || { echo "❌ M57-S1: FIONBIO 生效未出现"; exit 1; }
echo "$OUT" | grep -q "M57-S1 ioctl OK" || { echo "❌ M57-S1: 未达 OK 行"; exit 1; }
echo "✅ M57-S1: ioctl 胶水内建 PASS"
