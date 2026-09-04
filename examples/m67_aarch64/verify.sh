#!/usr/bin/env bash
# ============================================================
# M67-S3：aarch64 交叉编译 + qemu 验证（阶段一 · 用例 1/3 hello / 2/3 HTTP / 3/3 SQLite）
# ------------------------------------------------------------
# 验证闭环：PuXian .px → 生成 C → aarch64 musl 静态交叉二进制 → qemu-aarch64 运行
#   A. 前置：aarch64-linux-musl-gcc（musl.cc）+ qemu-aarch64-static + 预置 aarch64 库
#      （runtime/mbedtls/lib-aarch64 + sqlite3-aarch64.o + zlib lib-aarch64）
#   B. 三用例 pxc --no-quic 交叉编译（hello / http / sqlite）
#   C. file 断言产物 ELF ARM aarch64（static-pie 属 musl 正常形态，qemu 可跑）
#   D. qemu 逐例运行断言：
#        hello  直接运行 → "PX_ARCH_CROSS_OK"
#        http   qemu 后台起服 → 宿主 curl 127.0.0.1:18991/ → 200 + body
#        sqlite 直接运行 → "sqlite on aarch64 OK"
# 说明：--no-quic 裁剪 QUIC/H3（ngtcp2/openssl-quictls 无 aarch64 预编译库）；
#       三用例均不依赖 H3，语义不受裁剪影响（对齐 M57-S4 先例）。
# 用法：CC=<交叉gcc> QEMU=<qemu> bash examples/m67_aarch64/verify.sh
# ============================================================
set -u
cd "$(dirname "$0")/../.."   # 仓库根
QEMU="${QEMU:-qemu-aarch64-static}"
CC="${CC:-aarch64-linux-musl-gcc}"
PORT="${PORT:-18991}"
TMPD=$(mktemp -d /tmp/m67a64.XXXXXX)
trap 'rm -rf "$TMPD"' EXIT

fail() { echo "M67-S3 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

# ---- A. 前置检查 ----
command -v "$QEMU" >/dev/null 2>&1 || fail "缺 $QEMU（multiarch/qemu-user-static，放 /usr/local/bin）"
command -v "$CC"   >/dev/null 2>&1 || fail "缺交叉编译器 $CC（musl.cc aarch64-linux-musl-cross）"
[ -f runtime/mbedtls/lib-aarch64/libmbedcrypto.a ] || fail "缺 aarch64 mbedtls 库"
[ -f runtime/third_party/sqlite3/sqlite3-aarch64.o ] || fail "缺 aarch64 sqlite3.o"
[ -f runtime/third_party/zlib/lib-aarch64/libz.a ] || fail "缺 aarch64 zlib 库"
echo "A. 前置齐备：$CC + $QEMU + 预置 aarch64 交叉库"

# ---- B. 三用例交叉编译 ----
cp examples/m67_aarch64/*.px "$TMPD/"
for ex in hello_a64 http_a64 sqlite_a64; do
    tools/pxc build --no-quic \
        --cc "$CC" \
        --mbedtls-lib runtime/mbedtls/lib-aarch64 \
        --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o \
        "$TMPD/$ex.px" > "$TMPD/$ex.build.log" 2>&1 \
        || { echo "交叉编译失败 $ex:"; tail -20 "$TMPD/$ex.build.log"; fail "$ex 编译失败"; }
    ok "$ex 交叉编译成功"
done

# ---- C. 产物架构断言 ----
for ex in hello_a64 http_a64 sqlite_a64; do
    ARCH=$(file "$TMPD/build/$ex")
    echo "  $ARCH"
    echo "$ARCH" | grep -q "ARM aarch64" || fail "$ex 产物不是 aarch64: $ARCH"
done
echo "C. 三产物均为 ELF ARM aarch64"

# ---- D. qemu 逐例运行断言 ----
echo "== D1. hello_a64 =="
OUT=$("$QEMU" "$TMPD/build/hello_a64" 2>&1)
echo "$OUT"
echo "$OUT" | grep -q "PX_ARCH_CROSS_OK" || fail "hello qemu 运行未达预期"

echo "== D2. http_a64（qemu 起服 + 宿主 curl 200）=="
"$QEMU" "$TMPD/build/http_a64" > "$TMPD/http.log" 2>&1 &
SRV=$!
sleep 1
grep -q "READY" "$TMPD/http.log" || { cat "$TMPD/http.log"; kill "$SRV" 2>/dev/null; fail "http 服务未就绪"; }
RESP=$(curl -s -o "$TMPD/http.body" -w "%{http_code}" "http://127.0.0.1:$PORT/" 2>&1)
kill "$SRV" 2>/dev/null
[ "$RESP" = "200" ] || fail "curl 状态码 $RESP ≠ 200"
grep -q "m67-aarch64-http-ok" "$TMPD/http.body" || fail "响应 body 不符"
echo "  HTTP 200 + body=m67-aarch64-http-ok"
ok "http qemu 起服宿主请求通过"

echo "== D3. sqlite_a64 =="
OUT=$("$QEMU" "$TMPD/build/sqlite_a64" 2>&1)
echo "$OUT"
echo "$OUT" | grep -q "sqlite on aarch64 OK" || fail "sqlite qemu 运行未达预期"

echo "== M67-S3 aarch64 交叉验证 PASS（hello / http / sqlite 全绿）=="
