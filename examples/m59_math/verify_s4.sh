#!/usr/bin/env bash
# ============================================================
# M59 S4 verify —— 双模式同步 + aarch64 交叉 qemu + 回归抽查
# ------------------------------------------------------------
# 验证点：
#   A. bootstrap/pxi 已重建（静态 ELF，含 M59 数学内置）
#   B. math_s4.px 编译模式断言全过 → "M59-S4 OK" 退出码 0
#   C. pxi 解释模式同文件全过，且输出与编译模式逐字节一致（双模式同步）
#   D. aarch64 交叉编译（--no-quic --cc aarch64-linux-musl-gcc）+ qemu 运行
#      → "M59-S4 OK"（静态 libm 跨架构断言 PASS；splitmix 确定性复现）
#   E. 回归抽查：hello/fib 编译+解释双模式 + sqrt 可用
#   F. 行数约束 < 500
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m59_math/build
D=examples/m59_math
QEMU="${QEMU:-qemu-aarch64-static}"
CCPATH=/opt/aarch64-linux-musl-cross/bin
CC="${CC:-aarch64-linux-musl-gcc}"
TMPD=$(mktemp -d /tmp/m59s4.XXXXXX)

fail() { echo "M59-S4 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }
cleanup() { rm -rf "$TMPD"; }
trap cleanup EXIT

echo "== A. pxi 已重建 =="
file bootstrap/pxi | grep -q "x86-64" || fail "bootstrap/pxi 非 x86-64"
file bootstrap/pxi | grep -q "statically linked" || fail "bootstrap/pxi 非静态"
$PXC run "$D/math_s4.px" >/dev/null 2>&1 || fail "pxi 解释 math_s4.px 失败（pxi 未含数学内置？）"
ok "bootstrap/pxi 静态 ELF 且可解释 math_s4.px（M59 数学已进解释器）"

echo "== B. 编译模式断言 =="
$PXC build "$D/math_s4.px" >/dev/null 2>&1 || fail "math_s4.px 编译失败"
OUT1=$("$B/math_s4" 2>&1) || fail "编译模式退出码非 0"
echo "$OUT1" | tail -3
echo "$OUT1" | grep -q "M59-S4 OK" || fail "编译模式未输出 M59-S4 OK"
ok "编译模式断言全过"

echo "== C. pxi 解释模式 + 双模式逐字节一致 =="
OUT2=$($PXC run "$D/math_s4.px" 2>&1) || fail "解释模式退出码非 0"
echo "$OUT2" | tail -3
echo "$OUT2" | grep -q "M59-S4 OK" || fail "解释模式未输出 M59-S4 OK"
diff <(echo "$OUT1") <(echo "$OUT2") || fail "双模式输出不一致"
ok "解释模式断言全过且输出与编译模式逐字节一致"

echo "== D. aarch64 交叉编译 + qemu =="
command -v "$QEMU" >/dev/null 2>&1 || fail "缺 $QEMU"
export PATH="$CCPATH:$PATH"
command -v "$CC" >/dev/null 2>&1 || fail "缺交叉编译器 $CC"
[ -f runtime/mbedtls/lib-aarch64/libmbedcrypto.a ] || fail "缺 aarch64 mbedtls 库"
[ -f runtime/third_party/sqlite3/sqlite3-aarch64.o ] || fail "缺 aarch64 sqlite3.o"
cp "$D/math_s4.px" "$TMPD"/
tools/pxc build --no-quic \
    --cc "$CC" \
    --mbedtls-lib runtime/mbedtls/lib-aarch64 \
    --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o \
    "$TMPD/math_s4.px" >/tmp/m59s4_x.log 2>&1 || { echo "交叉编译失败:"; tail -20 /tmp/m59s4_x.log; exit 1; }
XBIN="$TMPD/build/math_s4"
F=$(file "$XBIN")
echo "$F"
echo "$F" | grep -q "ARM aarch64" || fail "产物非 aarch64"
ok "交叉编译成功（静态 libm + splitmix64 无浮点平台依赖）"
XOUT=$("$QEMU" "$XBIN" 2>&1) || fail "qemu 退出非 0"
echo "$XOUT" | tail -3
echo "$XOUT" | grep -q "M59-S4 OK" || fail "qemu 未输出 M59-S4 OK"
echo "$XOUT" | grep -q "rand_seq=" || fail "qemu 无 rand_seq（splitmix 确定性）"
# splitmix64 纯整数运算 → aarch64 与 x86 序列应完全一致
RANDX86=$(echo "$OUT1" | grep "^rand_seq=" | head -1)
RANDARM=$(echo "$XOUT" | grep "^rand_seq=" | head -1)
[ "$RANDX86" = "$RANDARM" ] || fail "aarch64 随机序列与 x86 不一致: $RANDX86 vs $RANDARM"
ok "qemu-aarch64 断言全过 + splitmix64 序列与 x86 逐位一致"

echo "== E. 回归抽查 =="
$PXC build examples/hello.px >/dev/null 2>&1 || fail "hello 编译失败"
H1=$(examples/build/hello 2>&1) || fail "hello 运行失败"
H2=$($PXC run examples/hello.px 2>&1) || fail "hello 解释运行失败"
[ "$H1" = "$H2" ] || fail "hello 双模式输出不一致"
$PXC build examples/fib.px >/dev/null 2>&1 || fail "fib 编译失败"
F1=$(examples/build/fib 2>&1) || fail "fib 运行失败"
F2=$($PXC run examples/fib.px 2>&1) || fail "fib 解释运行失败"
[ "$F1" = "$F2" ] || fail "fib 双模式输出不一致"
ok "hello/fib 编译+解释双模式一致 PASS"

echo "== F. 行数约束 =="
L=$(wc -l < "$D/math_s4.px")
echo "  math_s4.px: $L 行"
[ "$L" -lt 500 ] || fail "math_s4.px 超 500 行"
ok "行数约束满足"

echo "M59-S4 PASS"
