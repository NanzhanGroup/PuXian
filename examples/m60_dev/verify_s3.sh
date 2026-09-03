#!/usr/bin/env bash
# ============================================================
# M60 S3 verify —— stdlib edge.px + 设备示例
# ------------------------------------------------------------
# 验证点：
#   A. dev_s3.px（库单测：GPIO V2 布局常量断言/事件解析）编译+运行 → "M60-S3 DEV OK"
#   B. m60_serial_pty.px x86 实跑：PTY 真内核串口 loopback（serial_open+fd_wait+读写）→ OK
#   C. m60_gpio / m60_i2c / m60_pwm 真板段：x86 无设备 → SKIP 放行（exit 0）
#   D. 行数约束：edge.px 及各示例 < 500
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m60_dev/build
D=examples/m60_dev

fail() { echo "M60-S3 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. 库单测 dev_s3.px（布局常量断言，x86 无设备可跑）=="
$PXC build "$D/dev_s3.px" >/dev/null 2>&1 || fail "dev_s3.px 编译失败"
OUT=$("$B/dev_s3" 2>&1) || fail "dev_s3.px 运行退出码非 0"
echo "$OUT" | grep -E "^A:|^B:|^C/D:|^E:|^F:"
echo "$OUT" | grep -q "M60-S3 DEV OK" || fail "未输出 M60-S3 DEV OK"
ok "GPIO V2 布局常量与事件解析单测全过"

echo "== B. m60_serial_pty.px（x86 实跑：PTY 串口 loopback）=="
$PXC build examples/m60_serial_pty.px >/dev/null 2>&1 || fail "m60_serial_pty.px 编译失败"
SOUT=$(./examples/build/m60_serial_pty 2>&1) || fail "m60_serial_pty 运行退出码非 0"
echo "$SOUT"
echo "$SOUT" | grep -q "M60-SERIAL-PTY OK" || fail "未输出 M60-SERIAL-PTY OK"
ok "串口全链路（serial_open 115200/9600 + 双向 loopback）实跑 PASS"

echo "== C. 真板段示例（x86 无设备 → SKIP 放行）=="
for ex in m60_gpio m60_i2c m60_pwm; do
    $PXC build "examples/$ex.px" >/dev/null 2>&1 || fail "$ex.px 编译失败"
    XOUT=$(./examples/build/$ex 2>&1); XRC=$?
    [ "$XRC" -eq 0 ] || fail "$ex 运行退出码非 0（应 SKIP 放行 exit 0）: $XOUT"
    echo "$XOUT" | grep -q "SKIP" && { echo "  $ex: SKIP（x86 无设备，真板通道）"; continue; }
    echo "$XOUT" | grep -q "OK" && { echo "  $ex: OK（本机有设备）"; continue; }
    fail "$ex 既无 SKIP 也无 OK: $XOUT"
done
ok "真板段示例 SKIP 通道通畅（编译 + SKIP exit 0）"

echo "== D. 行数约束 =="
for f in stdlib/edge.px "$D/dev_s3.px" examples/m60_serial_pty.px examples/m60_gpio.px examples/m60_i2c.px examples/m60_pwm.px; do
    L=$(wc -l < "$f")
    echo "  $f: $L 行"
    [ "$L" -lt 500 ] || fail "$f 超 500 行"
done
ok "行数约束满足"

echo "M60-S3 PASS"
