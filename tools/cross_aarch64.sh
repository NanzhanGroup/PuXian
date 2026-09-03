#!/usr/bin/env bash
# ============================================================
# tools/cross_aarch64.sh —— M57-S4：aarch64 交叉静态库构建
# ------------------------------------------------------------
# 背景：pxc 交叉编译（--cc aarch64-...）需要目标架构的第三方静态库。
#   - ngtcp2 / openssl-quictls 仅有 x86_64 预编译 .a（且交叉构建成本高）
#     → 交叉编译须配 pxc --no-quic 裁剪 QUIC/H3（runtime 侧 -DPX_NO_QUIC），
#       不链这两个库（见 tools/pxc 与 runtime/runtime.c）。
#   - mbedtls / sqlite3 为纯 C，可交叉编译 → 本脚本一次产出：
#       runtime/mbedtls/lib-aarch64/{libmbedtls,libmbedx509,libmbedcrypto}.a
#       runtime/third_party/sqlite3/sqlite3-aarch64.o
# 用法：
#   tools/cross_aarch64.sh                    # CC=aarch64-linux-musl-gcc（需在 PATH）
#   tools/cross_aarch64.sh --cc <交叉CC>       # 指定交叉编译器
#   tools/cross_aarch64.sh --src <mbedtls源码> # 复用已有 mbedtls 源码目录（免下载）
#   tools/cross_aarch64.sh --mbedtls-ver 3.6.2
# 产物架构校验：ar 抽一个成员 file 应为 "ARM aarch64"。
# ============================================================
set -euo pipefail
PXC_HOME="$(cd "$(dirname "$0")/.." && pwd)"
RT="$PXC_HOME/runtime"
MBEDTLS_VER="${MBEDTLS_VER:-3.6.2}"
CC="${CC:-aarch64-linux-musl-gcc}"
JOBS="$(nproc 2>/dev/null || echo 8)"
SRC=""

while [ $# -gt 0 ]; do
    case "$1" in
        --cc) CC="$2"; shift 2 ;;
        --src) SRC="$2"; shift 2 ;;
        --mbedtls-ver) MBEDTLS_VER="$2"; shift 2 ;;
        --jobs) JOBS="$2"; shift 2 ;;
        *) echo "未知参数: $1" >&2; exit 1 ;;
    esac
done

command -v "$CC" >/dev/null 2>&1 || { echo "错误: 找不到交叉编译器 $CC（请装 aarch64-linux-musl-cross 并加 PATH，或 --cc 指定）" >&2; exit 1; }
command -v curl >/dev/null 2>&1 || { echo "错误: 需要 curl 下载 mbedtls 源码" >&2; exit 1; }

echo "== M57-S4 交叉库构建：CC=$CC mbedtls=$MBEDTLS_VER =="

# ---------- 1. sqlite3（amalgamation 单文件直编）----------
SQLITE_OBJ="$RT/third_party/sqlite3/sqlite3-aarch64.o"
echo "== [1/2] sqlite3 → $SQLITE_OBJ"
if [ -f "$RT/third_party/sqlite3/sqlite3.c" ]; then
    "$CC" -c -O2 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_FOREIGN_KEYS=1 \
        -o "$SQLITE_OBJ" "$RT/third_party/sqlite3/sqlite3.c"
else
    echo "错误: 缺少 $RT/third_party/sqlite3/sqlite3.c" >&2; exit 1
fi

# ---------- 2. mbedtls（源码 make CC=交叉编译器 lib）----------
echo "== [2/2] mbedtls v$MBEDTLS_VER → $RT/mbedtls/lib-aarch64/"
TMP_SRC=""
if [ -n "$SRC" ]; then
    [ -d "$SRC" ] || { echo "错误: --src 目录不存在: $SRC" >&2; exit 1; }
    TMP_SRC="$SRC"
else
    TMP_SRC="/tmp/mbedtls-$MBEDTLS_VER"
    if [ ! -f "$TMP_SRC/library/libmbedtls.a" ]; then
        if [ ! -f "/tmp/mbedtls-$MBEDTLS_VER.tar.gz" ]; then
            echo "下载 mbedtls v$MBEDTLS_VER ..."
            curl -sL -o "/tmp/mbedtls-$MBEDTLS_VER.tar.gz" \
                "https://github.com/Mbed-TLS/mbedtls/archive/refs/tags/v$MBEDTLS_VER.tar.gz"
        fi
        rm -rf "$TMP_SRC"
        tar xzf "/tmp/mbedtls-$MBEDTLS_VER.tar.gz" -C /tmp
        mv "/tmp/mbedtls-mbedtls-$MBEDTLS_VER" "$TMP_SRC" 2>/dev/null || true
    fi
    # 版本一致性校验（仓库 runtime/mbedtls/include 的 version.h）
    local_ver=$(grep -m1 'MBEDTLS_VERSION_STRING' "$RT/mbedtls/include/mbedtls/version.h" | grep -oE '"[0-9.]+"' | tr -d '"')
    src_ver=$(grep -m1 'MBEDTLS_VERSION_STRING' "$TMP_SRC/include/mbedtls/version.h" | grep -oE '"[0-9.]+"' | tr -d '"')
    if [ "$local_ver" != "$src_ver" ]; then
        echo "警告: 仓库 mbedtls 头文件版本 $local_ver ≠ 源码版本 $src_ver（建议对齐）" >&2
    fi
fi

( cd "$TMP_SRC" && make -s clean >/dev/null 2>&1 || true
  make -s -j"$JOBS" CC="$CC" lib >/dev/null 2>&1 ) \
    || { echo "错误: mbedtls 交叉编译失败（源码 $TMP_SRC）" >&2; exit 1; }

mkdir -p "$RT/mbedtls/lib-aarch64"
cp "$TMP_SRC/library/libmbedtls.a" "$TMP_SRC/library/libmbedx509.a" \
   "$TMP_SRC/library/libmbedcrypto.a" "$RT/mbedtls/lib-aarch64/"

# ---------- 3. 架构校验 ----------
chk=$(mktemp -d)
( cd "$chk" && ar x "$RT/mbedtls/lib-aarch64/libmbedcrypto.a" "$(ar t "$RT/mbedtls/lib-aarch64/libmbedcrypto.a" | head -1)" \
    && file *.o | head -1 )
rm -rf "$chk"
file "$SQLITE_OBJ"
echo "== 完成：交叉库已就绪，pxc 用法示例："
echo "  tools/pxc build --no-quic --cc $CC \\"
echo "    --mbedtls-lib runtime/mbedtls/lib-aarch64 \\"
echo "    --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o <file.px>"
