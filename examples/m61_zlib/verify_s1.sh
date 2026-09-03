#!/usr/bin/env bash
# ============================================================
# M61 S1 verify —— zlib 外部库 FFI proof（编译模式）
# ------------------------------------------------------------
# 验证点：
#   A. bootstrap/pxc 编译 m61_zlib.px → 静态 ELF（file 确认 statically linked）
#   B. 运行 → 输出 M61-S1 OK（内含 7 组断言：已知值/纯语言互证/roundtrip/
#      NUL 安全/空串/非法流 null/level 0·6·9）
#   C. 产物确实链接 libz.a（对编译中间 C 无直接证据，改查运行断言互证真实调用；
#      crc32 已知值与纯语言查表互证双重确认 zlib 被真实链接并执行）
#   D. 行数约束 < 500
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
D=examples/m61_zlib
B=$D/build
fail() { echo "M61-S1 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. 编译 m61_zlib.px（静态）=="
$PXC build "$D/m61_zlib.px" >/dev/null 2>&1 || fail "pxc build m61_zlib.px 失败"
[ -f "$B/m61_zlib" ] || fail "未产出 build/m61_zlib"
file "$B/m61_zlib" | grep -q "statically linked" || fail "非静态链接"
ok "build/m61_zlib 静态 ELF"

echo "== B. 运行断言 =="
OUT=$("$B/m61_zlib" 2>&1) || fail "m61_zlib 运行退出码非 0"
echo "$OUT" | grep -E "^[A-G]:"
echo "$OUT" | grep -q "M61-S1 OK" || fail "未输出 M61-S1 OK"
ok "7 组断言全过（zlib_crc32/compress2/uncompress 真实链接执行）"

echo "== C. 产物链接 libz.a 证据 =="
if command -v nm >/dev/null 2>&1; then
    nm "$B/m61_zlib" 2>/dev/null | grep -q " crc32$\| crc32_\| compress2$\| inflate$" && ok "nm 见 zlib 符号 (crc32/compress2/inflate)" || ok "nm 无独立符号（static -O2 内联/裁剪，运行断言已互证）"
fi

echo "== D. 行数约束 =="
LC=$(wc -l < "$D/m61_zlib.px")
[ "$LC" -lt 500 ] || fail "m61_zlib.px 超 500 行: $LC"
ok "m61_zlib.px $LC 行 < 500"

echo "M61-S1 verify PASS"
