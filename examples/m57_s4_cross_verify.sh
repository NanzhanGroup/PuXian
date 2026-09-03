#!/usr/bin/env bash
# ============================================================
# M57-S4：aarch64 交叉编译 + qemu 验证脚本
# ------------------------------------------------------------
# 验证闭环：PuXian .px → 生成 C → aarch64 交叉静态二进制 → qemu-aarch64 运行
#   A. 前置：aarch64 交叉编译器 + qemu-aarch64 + 交叉库（tools/cross_aarch64.sh）
#   B. pxc --no-quic 交叉编译 m57_s3_devctl.px（设备层 ioctl 真内核替身验证，
#      与 GPIO/I2C 完全相同的语言胶水路径——fd → ABI buffer → ioctl 就地填充）
#   C. file 确认 ELF aarch64 静态
#   D. qemu-aarch64 运行：ioctl syscall 直通宿主内核 → 断言与 x86 同结果
#      （asm-generic ioctl 码跨架构一致；边缘设备层语义跨架构成立）
# 说明：--no-quic 裁剪 QUIC/H3（ngtcp2/openssl-quictls 无 aarch64 预编译库）；
#       设备层不依赖 H3，语义不受裁剪影响（x86 裁剪版 m57_s3 已验证一致）。
# ============================================================
set -u
cd "$(dirname "$0")/.."   # 仓库根
QEMU="${QEMU:-qemu-aarch64-static}"
CC="${CC:-aarch64-linux-musl-gcc}"
TMPD=$(mktemp -d /tmp/m57s4.XXXXXX)
trap 'rm -rf "$TMPD"' EXIT

fail() { echo "M57-S4 FAIL: $*" >&2; exit 1; }

# ---- A. 前置检查 ----
command -v "$QEMU" >/dev/null 2>&1 || fail "缺 $QEMU（如 multiarch/qemu-user-static 的 qemu-aarch64-static，放 /usr/local/bin）"
command -v "$CC"   >/dev/null 2>&1 || fail "缺交叉编译器 $CC"
[ -f runtime/mbedtls/lib-aarch64/libmbedcrypto.a ] || fail "缺 aarch64 mbedtls 库（先跑 tools/cross_aarch64.sh）"
[ -f runtime/third_party/sqlite3/sqlite3-aarch64.o ] || fail "缺 aarch64 sqlite3.o（先跑 tools/cross_aarch64.sh）"
echo "A. 前置齐备：$CC + $QEMU + 交叉库"

# ---- B. 交叉编译设备层 devctl ----
cp examples/m57_s3_devctl.px "$TMPD/devctl.px"
tools/pxc build --no-quic \
    --cc "$CC" \
    --mbedtls-lib runtime/mbedtls/lib-aarch64 \
    --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o \
    "$TMPD/devctl.px" > "$TMPD/build.log" 2>&1 \
    || { echo "交叉编译失败:"; tail -20 "$TMPD/build.log"; exit 1; }
echo "B. 交叉编译成功"

# ---- C. 产物架构确认 ----
BIN="$TMPD/build/devctl"
ARCH=$(file "$BIN")
echo "$ARCH"
echo "$ARCH" | grep -q "ARM aarch64" || fail "产物不是 aarch64"
echo "$ARCH" | grep -q "statically linked" || echo "注: 产物非静态链接（musl 默认 static-pie 属正常，qemu 可跑）"

# ---- D. qemu-aarch64 运行验证 ----
OUT=$("$QEMU" "$BIN" 2>&1)
echo "== qemu-aarch64 运行输出 =="
echo "$OUT"
echo "$OUT" | grep -q "M57-S3 devctl OK" || fail "qemu 运行未达预期"
echo "== M57-S4 交叉验证 PASS =="
