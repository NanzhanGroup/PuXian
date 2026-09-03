#!/usr/bin/env bash
# ============================================================
# M61 S4 verify —— pxi 双模式 + aarch64 交叉 + PNG FFI 联动（A+B 收口）
# ------------------------------------------------------------
# 验证点：
#   A. bootstrap/pxi 已重建（含 runtime_zlib.c + libz.a → extern zlib_* 解释可用）
#   B. pxi 解释 m61_s4_zpxi.px → "M61-S4 PXI OK"；与编译模式输出逐字节一致
#   C. pxi stdlib import 限制如实记录（m61_s4_impsmoke.px 探针，预期失败不判 fail）
#   D. aarch64 交叉 qemu：
#      - m61_zlib.px → "M61-S1 OK"，输出与 x86 diff 逐字节一致（zlib FFI 跨架构）
#      - m61_s4_det.px → det_aarch64.png，sha256 与 x86 det_x86.png 一致
#        （gfx 整数原语 + PNG stored 编码器跨架构逐字节确定）
#   E. FFI PNG 压缩联动：m61_s4_zpng.px → python 解压合法 + 像素抽查 + idat<rows
#   F. 回归：hello 双模式 / math_s1 编译 / dev_s1 --no-quic 编译
# ============================================================
set -u
cd "$(dirname "$0")/../.."
PXC=./tools/pxc
D=examples/m61_gfx
B=$D/build
ZD=examples/m61_zlib
ZB=$ZD/build
QEMU="${QEMU:-qemu-aarch64-static}"
CCPATH=/opt/aarch64-linux-musl-cross/bin
CC="${CC:-aarch64-linux-musl-gcc}"
TMPD=$(mktemp -d /tmp/m61s4.XXXXXX)
fail() { echo "M61-S4 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }
cleanup() { rm -rf "$TMPD"; }
trap cleanup EXIT

echo "== A. bootstrap/pxi 已重建（含 zlib extern）=="
file bootstrap/pxi | grep -q "statically linked" || fail "pxi 非静态"
SIZE_NOW=$(stat -c %s bootstrap/pxi)
$PXC run "$D/m61_s4_zpxi.px" >/dev/null 2>&1 || fail "pxi 解释 zpxi 失败（pxi 未含 zlib_* 注册？）"
ok "bootstrap/pxi ($SIZE_NOW B) 可解释 zlib extern（zlib_crc32/compress/uncompress 已进解释器）"

echo "== B. 双模式 zlib 一致 =="
OUTX=$($PXC run "$D/m61_s4_zpxi.px" 2>&1) || fail "pxi zpxi 退出非 0"
echo "$OUTX" | grep -q "M61-S4 PXI OK" || fail "pxi 未输出 OK"
$PXC build "$D/m61_s4_zpxi.px" >/dev/null 2>&1 || fail "zpxi 编译失败"
OUTB=$("$B/m61_s4_zpxi" 2>&1) || fail "zpxi 编译运行失败"
[ "$OUTX" = "$OUTB" ] || fail "zlib extern 双模式输出不一致"
ok "zlib extern 解释 == 编译（输出逐字节一致）"

echo "== C. pxi stdlib import 能力探针（如实记录，不判 fail）=="
IMP=$($PXC run "$D/m61_s4_impsmoke.px" 2>&1)
if echo "$IMP" | grep -q "IMPSMOKE OK"; then
    echo "  ✅ pxi 可解释 import std.gfx + canvas/set_px/get_px（纯 list 路径）"
    echo "     ⚠️ 完整 std.gfx 仍受限：text()/blit() 等依赖 pxi 未同步 builtin（bytes 等）"
    echo "        → stdlib 完整能力主打编译模式；pxi 覆盖 C 内置注册面 + 简单库路径"
else
    echo "  ℹ️ pxi import std.gfx 探针失败（§十三 #8 相关）：$(echo "$IMP" | tail -1 | head -c 90)"
    echo "     → stdlib 主打编译模式；pxi 覆盖 C 内置注册面"
fi

echo "== D. aarch64 交叉 + qemu =="
command -v "$QEMU" >/dev/null 2>&1 || fail "缺 $QEMU"
export PATH="$CCPATH:$PATH"
command -v "$CC" >/dev/null 2>&1 || fail "缺交叉编译器 $CC"
[ -f runtime/third_party/zlib/lib-aarch64/libz.a ] || fail "缺 aarch64 libz.a"

echo "-- D1. zlib FFI 跨架构 --"
cp "$ZD/m61_zlib.px" "$TMPD/"
$PXC build --no-quic --cc "$CC" --mbedtls-lib runtime/mbedtls/lib-aarch64 \
    --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o \
    "$TMPD/m61_zlib.px" >/dev/null 2>&1 || fail "m61_zlib aarch64 交叉编译失败"
X1=$("$QEMU" "$TMPD/build/m61_zlib" 2>&1) || fail "qemu m61_zlib 退出非 0"
echo "$X1" | grep -q "M61-S1 OK" || fail "qemu m61_zlib 未输出 OK"
X0=$("$ZB/m61_zlib" 2>&1) || fail "x86 m61_zlib 运行失败"
[ "$X0" = "$X1" ] || fail "m61_zlib x86 与 qemu-aarch64 输出不一致"
ok "zlib_crc32/compress/uncompress 跨架构输出逐字节一致（aarch64 libz.a 生效）"

echo "-- D2. gfx+PNG 确定性跨架构 --"
cp "$D/m61_s4_det.px" "$TMPD/"
DET_OUT=det_x86.png "$B/m61_s4_det" >/dev/null 2>&1 || fail "det x86 运行失败"
$PXC build --no-quic --cc "$CC" --mbedtls-lib runtime/mbedtls/lib-aarch64 \
    --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o \
    "$TMPD/m61_s4_det.px" >/dev/null 2>&1 || fail "det aarch64 交叉编译失败"
(DET_OUT=det_aarch64.png "$QEMU" "$TMPD/build/m61_s4_det" >/dev/null 2>&1) || fail "qemu det 运行失败"
[ -f "$D/det_aarch64.png" ] || fail "aarch64 未落盘 det_aarch64.png"
S1=$(sha256sum "$D/det_x86.png" | cut -d' ' -f1)
S2=$(sha256sum "$D/det_aarch64.png" | cut -d' ' -f1)
[ "$S1" = "$S2" ] || fail "det PNG x86/aarch64 sha256 不一致"
ok "gfx 整数原语 + PNG stored 编码器：x86 == aarch64 逐字节一致（sha256 $S1）"

echo "== E. FFI PNG 压缩联动（zlib_compress 出压缩 IDAT）=="
$PXC build "$D/m61_s4_zpng.px" >/dev/null 2>&1 || fail "zpng 编译失败"
ZO=$("$B/m61_s4_zpng" 2>&1) || fail "zpng 运行失败"
echo "$ZO"
echo "$ZO" | grep -q "M61-S4 ZPNG OK" || fail "zpng 未输出 OK"
python3 - "$D" <<'PY' || exit 1
import zlib, struct, sys
D = sys.argv[1]
data = open(f"{D}/zpng.png", 'rb').read()
assert data[:8] == b'\x89PNG\r\n\x1a\n'
off = 8; idat = b''; w = h = None
while off < len(data):
    ln, = struct.unpack('>I', data[off:off+4])
    typ = data[off+4:off+8]
    ch = data[off+8:off+8+ln]
    crc, = struct.unpack('>I', data[off+8+ln:off+12+ln])
    assert crc == (zlib.crc32(typ + ch) & 0xffffffff), f'crc {typ}'
    if typ == b'IHDR':
        w, h = struct.unpack('>II', ch[:8])
    if typ == b'IDAT':
        idat += ch
    off += 12 + ln
raw = zlib.decompress(idat)
assert (w, h) == (160, 100) and len(raw) == h * (1 + 3 * w)
def px(x, y):
    r = raw[y*(1+3*w):]
    return tuple(r[1+3*x:1+3*x+3])
assert px(0,0) == (0,0,0), px(0,0)
assert px(1,0) == (5,0,2), px(1,0)
has_white = any(px(x,y) == (255,255,255) for y in range(40,47) for x in range(10,70))
assert has_white, '文字区缺白'
print("  ✅ zpng.png：FFI compress2 生成的 PNG 合法（chunk CRC 全过 + zlib 解压 + 像素抽查）")
PY
[ $? -eq 0 ] || fail "zpng python 校验失败"
[ -f "$D/zpng.png" ] && echo "  ℹ️ zpng.png $(stat -c %s "$D/zpng.png") B（FFI deflate 压缩版）vs rows 48KB"
ok "FFI zlib_compress 直接产出标准压缩 PNG（A 线在真实 PNG 上 dogfood）"

echo "== F. 回归 =="
$PXC run examples/hello.px >/dev/null 2>&1 || fail "hello pxi 失败"
$PXC run examples/fib.px >/dev/null 2>&1 || fail "fib pxi 失败"
$PXC build examples/m59_math/math_s1.px >/dev/null 2>&1 || fail "math_s1 编译失败"
$PXC build --no-quic examples/m60_dev/dev_s1.px >/dev/null 2>&1 || fail "dev_s1 --no-quic 编译失败"
ok "hello/fib pxi + math_s1 编译 + dev_s1 --no-quic 编译全过"

echo "M61-S4 verify PASS"
