#!/usr/bin/env bash
# ============================================================
# tools/cross_aarch64.sh —— aarch64 交叉静态库构建（M57-S4，兼容薄包装）
# ------------------------------------------------------------
# M67-S5：泛化为 tools/cross_multiarch.sh（--arch aarch64|armv7|riscv64，
#   含 sqlite3 + mbedtls + zlib 三件套）。本脚本保留为 M57-S4 兼容入口：
#   tools/cross_aarch64.sh                 == cross_multiarch.sh --arch aarch64
#   tools/cross_aarch64.sh --cc <cc>       == cross_multiarch.sh --arch aarch64 --cc <cc>
#   产物：runtime/mbedtls/lib-aarch64/{libmbedtls,libmbedx509,libmbedcrypto}.a
#         runtime/third_party/sqlite3/sqlite3-aarch64.o（+ zlib lib-aarch64/libz.a，M67）
# ============================================================
set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/cross_multiarch.sh" --arch aarch64 "$@"
