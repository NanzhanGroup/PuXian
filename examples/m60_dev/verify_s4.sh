#!/usr/bin/env bash
# ============================================================
# M60 S4 verify —— 双模式同步（pxi +5 内置）+ aarch64 交叉 qemu + 回归
# ------------------------------------------------------------
# 验证点：
#   A. bootstrap/pxi 已重建（静态 ELF，含 M60 5 内置）
#   B. dev_s4.px 编译模式断言全过 → "M60-S4 OK"
#   C. pxi 解释模式同文件全过，输出与编译模式逐字节一致（双模式同步）
#   D. aarch64 交叉编译 + qemu-aarch64 运行 → "M60-S4 OK"
#      （PTY/termios/poll/fcntl/sleep_us/now_us 跨架构一致）
#   E. 回归抽查：hello/fib 双模式 + m59 数学解释模式 + m57_s1（fd/ioctl）编译
#   F. 行数约束 < 500
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m60_dev/build
D=examples/m60_dev
QEMU="${QEMU:-qemu-aarch64-static}"
CCPATH=/opt/aarch64-linux-musl-cross/bin
CC="${CC:-aarch64-linux-musl-gcc}"
TMPD=$(mktemp -d /tmp/m60s4.XXXXXX)

fail() { echo "M60-S4 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }
cleanup() { rm -rf "$TMPD"; }
trap cleanup EXIT

echo "== A. pxi 已重建（含 M60 5 内置）=="
file bootstrap/pxi | grep -q "x86-64" || fail "bootstrap/pxi 非 x86-64"
file bootstrap/pxi | grep -q "statically linked" || fail "bootstrap/pxi 非静态"
$PXC run "$D/dev_s4.px" >/dev/null 2>&1 || fail "pxi 解释 dev_s4.px 失败（pxi 未含 M60 内置？）"
ok "bootstrap/pxi 静态 ELF 且可解释 dev_s4.px（M60 5 内置已进解释器）"

echo "== B. 编译模式断言 =="
$PXC build "$D/dev_s4.px" >/dev/null 2>&1 || fail "dev_s4.px 编译失败"
OUT1=$("$B/dev_s4" 2>&1) || fail "编译模式退出码非 0"
echo "$OUT1" | grep -E "^S4-[A-E]"
echo "$OUT1" | grep -q "M60-S4 OK" || fail "编译模式未输出 M60-S4 OK"
ok "编译模式断言全过（sleep_us/now_us/fcntl/tty_config/fd_wait）"

echo "== C. pxi 解释模式 + 双模式逐字节一致 =="
OUT2=$($PXC run "$D/dev_s4.px" 2>&1) || fail "解释模式退出码非 0"
echo "$OUT2" | tail -3
echo "$OUT2" | grep -q "M60-S4 OK" || fail "解释模式未输出 M60-S4 OK"
diff <(echo "$OUT1") <(echo "$OUT2") || fail "双模式输出不一致"
ok "解释模式断言全过且输出与编译模式逐字节一致"

echo "== D. aarch64 交叉编译 + qemu =="
command -v "$QEMU" >/dev/null 2>&1 || fail "缺 $QEMU"
export PATH="$CCPATH:$PATH"
command -v "$CC" >/dev/null 2>&1 || fail "缺交叉编译器 $CC"
[ -f runtime/mbedtls/lib-aarch64/libmbedcrypto.a ] || fail "缺 aarch64 mbedtls 库"
[ -f runtime/third_party/sqlite3/sqlite3-aarch64.o ] || fail "缺 aarch64 sqlite3.o"
cp "$D/dev_s4.px" "$TMPD"/
tools/pxc build --no-quic \
    --cc "$CC" \
    --mbedtls-lib runtime/mbedtls/lib-aarch64 \
    --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o \
    "$TMPD/dev_s4.px" >/tmp/m60s4_x.log 2>&1 || { echo "交叉编译失败:"; tail -20 /tmp/m60s4_x.log; exit 1; }
XBIN="$TMPD/build/dev_s4"
F=$(file "$XBIN")
echo "$F"
echo "$F" | grep -q "ARM aarch64" || fail "产物非 aarch64"
ok "交叉编译成功（静态，PTY/termios/poll 无平台依赖）"
XOUT=$("$QEMU" "$XBIN" 2>&1) || fail "qemu 退出非 0"
echo "$XOUT" | tail -3
echo "$XOUT" | grep -q "M60-S4 OK" || fail "qemu 未输出 M60-S4 OK"
ok "qemu-aarch64 断言全过（M60 5 内置跨架构一致）"

echo "== E. 回归抽查 =="
$PXC build examples/hello.px >/dev/null 2>&1 || fail "hello 编译失败"
H1=$(examples/build/hello 2>&1) || fail "hello 运行失败"
H2=$($PXC run examples/hello.px 2>&1) || fail "hello 解释运行失败"
[ "$H1" = "$H2" ] || fail "hello 双模式输出不一致"
$PXC build examples/fib.px >/dev/null 2>&1 || fail "fib 编译失败"
F1=$(examples/build/fib 2>&1) || fail "fib 运行失败"
F2=$($PXC run examples/fib.px 2>&1) || fail "fib 解释运行失败"
[ "$F1" = "$F2" ] || fail "fib 双模式输出不一致"
$PXC run examples/m59_math/math_s1.px >/dev/null 2>&1 || fail "m59 math_s1 解释模式回归失败"
$PXC build examples/m57_s1_ioctl.px >/dev/null 2>&1 || fail "m57_s1 编译失败"
ok "hello/fib 双模式 + m59 数学解释 + m57_s1 编译回归 PASS"

echo "== F. 行数约束 =="
L=$(wc -l < "$D/dev_s4.px")
echo "  dev_s4.px: $L 行"
[ "$L" -lt 500 ] || fail "dev_s4.px 超 500 行"
ok "行数约束满足"

echo "M60-S4 PASS"
