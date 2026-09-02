#!/usr/bin/env bash
# ============================================================
# M57 S3：设备控制示例 + x86 真实内核替身验证
# ------------------------------------------------------------
# 背景：pxc build 产物为静态链接（LD_PRELOAD 注入不可行），"x86 ioctl
#       mock" 改用内核自带的用户态可访问设备（loopback 网卡 + PTY）作
#       GPIO/I2C 替身——走与设备完全相同的语言胶水路径（open/socket →
#       ABI buffer → ioctl bytes/int 就地填充 → 解析内核写回）。
# 覆盖：
#   A. devctl（硬断言，x86 必跑）：网络接口 ifreq ioctl（SIOCGIFADDR /
#      SIOCGIFFLAGS / SIOCGIFHWADDR on lo）+ PTY TIOCGPTN，全部真实内核
#   B. gpio 示例（真实板子跑 OK；x86 无 /dev/gpiochip* → SKIP 放行）
#   C. i2c 示例（真实板子跑 OK；x86 无总线/无器件 → SKIP 放行；
#      I2C_SLAVE int 形态若设备存在则真实内核设置成功）
#   D. 回归 m57_s1_ioctl / m57_s2_mmap（S3 零 runtime 改动，防意外）
# ============================================================
set -u
cd "$(dirname "$0")/.."
PXC=./tools/pxc
B=examples/build
MOCK_SO=$B/libm57_s3_mock.so   # 预留：静态链接下不用；文档说明用

echo "== build =="
for px in m57_s3_devctl m57_s3_gpio m57_s3_i2c; do
    $PXC build examples/$px.px >/dev/null 2>&1 || { echo "FAIL: $px.px 编译失败"; exit 1; }
done
echo "build 3/3 OK"

echo "== A. devctl（真实内核设备 ioctl 硬断言）=="
OUT=$($B/m57_s3_devctl 2>&1); RC=$?
echo "$OUT"
if [ $RC -ne 0 ]; then echo "❌ M57-S3: devctl 退出码 $RC"; exit 1; fi
echo "$OUT" | grep -q "A1: SIOCGIFADDR lo -> family=2 ip=127.0.0.1 ok" || { echo "❌ M57-S3: A1 未出现"; exit 1; }
echo "$OUT" | grep -q "A2: SIOCGIFFLAGS lo" || { echo "❌ M57-S3: A2 未出现"; exit 1; }
echo "$OUT" | grep -q "A3: SIOCGIFHWADDR lo -> family=772" || { echo "❌ M57-S3: A3 未出现"; exit 1; }
echo "$OUT" | grep -q "B: TIOCGPTN -> pts/" || { echo "❌ M57-S3: B TIOCGPTN 未出现"; exit 1; }
echo "$OUT" | grep -q "M57-S3 devctl OK" || { echo "❌ M57-S3: 未达 OK 行"; exit 1; }
echo "✅ A: devctl PASS（网卡 ifreq + PTY 设备 ioctl 全过）"

echo "== B. gpio 示例（板子 OK / 无设备 SKIP 均放行）=="
GOUT=$($B/m57_s3_gpio 2>&1); GRC=$?
echo "$GOUT"
if [ $GRC -ne 0 ]; then echo "❌ M57-S3: gpio 退出码 $GRC"; exit 1; fi
echo "$GOUT" | grep -q "GPIO OK" || echo "$GOUT" | grep -q "SKIP" || { echo "❌ M57-S3: gpio 无 OK/SKIP"; exit 1; }
echo "✅ B: gpio 示例放行"

echo "== C. i2c 示例（板子 OK / 无器件 SKIP 均放行）=="
IOUT=$($B/m57_s3_i2c 2>&1); IRC=$?
echo "$IOUT"
if [ $IRC -ne 0 ]; then echo "❌ M57-S3: i2c 退出码 $IRC"; exit 1; fi
echo "$IOUT" | grep -q "I2C OK" || echo "$IOUT" | grep -q "SKIP" || { echo "❌ M57-S3: i2c 无 OK/SKIP"; exit 1; }
echo "✅ C: i2c 示例放行"

echo "== D. 回归 m57_s1 / m57_s2 =="
./examples/m57_s1_ioctl_verify.sh >/dev/null 2>&1 && echo "✅ m57_s1 复验 PASS" || { echo "❌ M57-S3: m57_s1 复验失败"; exit 1; }
./examples/m57_s2_mmap_verify.sh >/dev/null 2>&1 && echo "✅ m57_s2 复验 PASS" || { echo "❌ M57-S3: m57_s2 复验失败"; exit 1; }

echo "✅ M57-S3: 设备控制示例 + x86 真实内核替身验证 PASS"