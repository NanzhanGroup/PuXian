#!/usr/bin/env bash
# ============================================================
# tools/cross_multiarch.sh —— M67-S5：多架构 musl 交叉静态库构建
# ------------------------------------------------------------
# 一次产出目标架构三件套（纯 C 可交叉，对齐 M57-S4 cross_aarch64.sh 先例）：
#   sqlite3-<arch>.o                                      sqlite3 直编
#   <out>/mbedtls/<lib-arch>/{libmbedtls,libmbedx509,libmbedcrypto}.a
#   <out>/zlib/<lib-arch>/libz.a                          zlib 交叉静态
# 架构目录约定（对齐 tools/pxc 自动探测 + 仓库预置）：
#   aarch64 → lib-aarch64 / sqlite3-aarch64.o   （仓库已预置，重跑覆盖）
#   armv7   → lib-armv7   / sqlite3-armv7.o     （armhf/EABI hard-float）
#   riscv64 → lib-riscv64 / sqlite3-riscv64.o
# 用法：
#   tools/cross_multiarch.sh --arch aarch64                 # 默认三件套入仓库 runtime/
#   tools/cross_multiarch.sh --arch armv7 --outdir /opt/px-multiarch/armv7
#   tools/cross_multiarch.sh --arch riscv64 --cc riscv64-linux-musl-gcc
# 可选：--cc <交叉CC> / --mbedtls-ver 3.6.2 / --zlib-ver 1.3.1 /
#       --jobs N / --mbedtls-src <目录>（复用源码免下载）
# 说明（M67_PLAN D1）：armv7/riscv64 交叉库建议 --outdir 外部目录（CI 现编 +
#   actions/cache），不入库控体积；aarch64 维持仓库预置。
# 产物架构校验：ar 抽成员 file 应为 "ARM aarch64" / "ARM" / "RISC-V"。
# ============================================================
set -euo pipefail
PXC_HOME="$(cd "$(dirname "$0")/.." && pwd)"
RT="$PXC_HOME/runtime"
ARCH="aarch64"
OUTDIR=""
MBEDTLS_VER="${MBEDTLS_VER:-3.6.2}"
ZLIB_VER="${ZLIB_VER:-1.3.1}"
JOBS="$(nproc 2>/dev/null || echo 8)"
CC=""
SRC_MBED=""
SRC_ZLIB=""

while [ $# -gt 0 ]; do
    case "$1" in
        --arch) ARCH="$2"; shift 2 ;;
        --outdir) OUTDIR="$2"; shift 2 ;;
        --cc) CC="$2"; shift 2 ;;
        --mbedtls-ver) MBEDTLS_VER="$2"; shift 2 ;;
        --zlib-ver) ZLIB_VER="$2"; shift 2 ;;
        --jobs) JOBS="$2"; shift 2 ;;
        --mbedtls-src) SRC_MBED="$2"; shift 2 ;;
        --zlib-src) SRC_ZLIB="$2"; shift 2 ;;
        -h|--help) sed -n '2,30p' "$0"; exit 0 ;;
        *) echo "未知参数: $1（--arch aarch64|armv7|riscv64 [--outdir dir] [--cc cc]）" >&2; exit 1 ;;
    esac
done

# ---- 架构 → 目录名 / 默认交叉 CC / file 断言 ----
case "$ARCH" in
    aarch64) LIB="lib-aarch64"; OBJ="sqlite3-aarch64.o"; DEF_CC="aarch64-linux-musl-gcc"; FILE_RE="aarch64" ;;
    armv7)   LIB="lib-armv7";   OBJ="sqlite3-armv7.o";   DEF_CC="armv7l-linux-musleabihf-gcc"; FILE_RE="EABI5" ;;
    riscv64) LIB="lib-riscv64"; OBJ="sqlite3-riscv64.o"; DEF_CC="riscv64-linux-musl-gcc";      FILE_RE="RISC-V" ;;
    *) echo "错误: 未知 --arch $ARCH（支持 aarch64|armv7|riscv64）" >&2; exit 1 ;;
esac
CC="${CC:-$DEF_CC}"

# ---- 目标输出目录（缺省仓库 runtime/ 对应位置）----
MBED_DIR="$RT/mbedtls/$LIB"
SQLITE_OBJ="$RT/third_party/sqlite3/$OBJ"
ZLIB_DIR="$RT/third_party/zlib/$LIB"
if [ -n "$OUTDIR" ]; then
    MBED_DIR="$OUTDIR/mbedtls/$LIB"
    SQLITE_OBJ="$OUTDIR/sqlite3/$OBJ"
    ZLIB_DIR="$OUTDIR/zlib/$LIB"
fi

command -v "$CC" >/dev/null 2>&1 || { echo "错误: 找不到交叉编译器 $CC（musl.cc $ARCH-linux-musl-cross 加 PATH，或 --cc 指定）" >&2; exit 1; }
command -v curl >/dev/null 2>&1 || { echo "错误: 需要 curl 下载源码" >&2; exit 1; }
command -v ar  >/dev/null 2>&1 || { echo "错误: 需要 ar" >&2; exit 1; }

echo "== M67-S5 交叉库构建：arch=$ARCH CC=$CC mbedtls=$MBEDTLS_VER zlib=$ZLIB_VER =="

mkdir -p "$MBED_DIR" "$(dirname "$SQLITE_OBJ")" "$ZLIB_DIR"

# ---------- 1. sqlite3（amalgamation 单文件直编）----------
echo "== [1/3] sqlite3 → $SQLITE_OBJ"
SQLITE_SRC="$RT/third_party/sqlite3/sqlite3.c"
[ -f "$SQLITE_SRC" ] || { echo "错误: 缺少 $SQLITE_SRC" >&2; exit 1; }
"$CC" -c -O2 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_FOREIGN_KEYS=1 \
    -o "$SQLITE_OBJ" "$SQLITE_SRC"

# ---------- 2. mbedtls（独立解压副本交叉编，避免共享目录旧产物污染）----------
echo "== [2/3] mbedtls v$MBEDTLS_VER → $MBED_DIR"
TMP_SRC=""
if [ -n "$SRC_MBED" ]; then
    # 用户显式提供源码目录（假设干净，make clean 后交叉编）
    [ -d "$SRC_MBED" ] || { echo "错误: --mbedtls-src 目录不存在: $SRC_MBED" >&2; exit 1; }
    TMP_SRC="$SRC_MBED"
else
    TB="/tmp/mbedtls-$MBEDTLS_VER.tar.gz"
    if [ ! -f "$TB" ]; then
        echo "下载 mbedtls v$MBEDTLS_VER ..."
        curl -sL -o "$TB" \
            "https://github.com/Mbed-TLS/mbedtls/archive/refs/tags/v$MBEDTLS_VER.tar.gz"
    fi
    # 每架构独立副本：避免上次（另一架构）编译残留 .o 被 make 复用
    TMP_SRC="/tmp/mbedtls-multiarch-$ARCH-$MBEDTLS_VER"
    rm -rf "$TMP_SRC"
    mkdir -p "$TMP_SRC"
    tar xzf "$TB" -C "$TMP_SRC" --strip-components=1
fi
( cd "$TMP_SRC" && make -s clean >/dev/null 2>&1 || true
  make -s -j"$JOBS" CC="$CC" lib ) \
    || { echo "错误: mbedtls 交叉编译失败（源码 $TMP_SRC）" >&2; exit 1; }
cp "$TMP_SRC/library/libmbedtls.a" "$TMP_SRC/library/libmbedx509.a" \
   "$TMP_SRC/library/libmbedcrypto.a" "$MBED_DIR/"

# ---------- 3. zlib（独立副本交叉静态，避免污染）----------
echo "== [3/3] zlib v$ZLIB_VER → $ZLIB_DIR"
ZSRC=""
if [ -n "$SRC_ZLIB" ]; then
    [ -d "$SRC_ZLIB" ] || { echo "错误: --zlib-src 目录不存在: $SRC_ZLIB" >&2; exit 1; }
    ZSRC="$SRC_ZLIB"
else
    ZTMP="$(mktemp -d)"
    if [ ! -f "/tmp/zlib-$ZLIB_VER.tar.gz" ]; then
        echo "下载 zlib v$ZLIB_VER ..."
        curl -sSL -o "/tmp/zlib-$ZLIB_VER.tar.gz" \
            "https://github.com/madler/zlib/releases/download/v$ZLIB_VER/zlib-$ZLIB_VER.tar.gz"
    fi
    tar xzf "/tmp/zlib-$ZLIB_VER.tar.gz" -C "$ZTMP" --strip-components=1
    ZSRC="$ZTMP"
fi
( cd "$ZSRC" && make -s distclean >/dev/null 2>&1 || true
  CC="$CC" AR="${CC%-gcc}-ar" RANLIB="${CC%-gcc}-ranlib" \
  ./configure --static >/dev/null 2>&1 && make -s -j"$JOBS" >/dev/null 2>&1 ) \
    || { echo "错误: zlib 交叉编译失败（源码 $ZSRC）" >&2; exit 1; }
cp "$ZSRC/libz.a" "$ZLIB_DIR/libz.a"
[ -n "${ZTMP:-}" ] && rm -rf "$ZTMP" || true

# ---------- 4. 架构校验 ----------
echo "== 架构校验（file 断言 $FILE_RE）=="
CHK="$(mktemp -d)"
ok=1
( cd "$CHK" && ar x "$MBED_DIR/libmbedcrypto.a" "$(ar t "$MBED_DIR/libmbedcrypto.a" | head -1)" \
    && file *.o | grep -qE "$FILE_RE" ) || { echo "mbedtls 架构不符"; ok=0; }
( cd "$CHK" && ar x "$ZLIB_DIR/libz.a" crc32.o \
    && file crc32.o | grep -qE "$FILE_RE" ) || { echo "zlib 架构不符"; ok=0; }
file "$SQLITE_OBJ" | grep -qE "$FILE_RE" || { echo "sqlite3 架构不符"; ok=0; }
rm -rf "$CHK"
[ "$ok" = 1 ] || { echo "错误: 产物架构校验失败" >&2; exit 1; }
echo "== 完成：三件套就绪（arch=$ARCH）=="
echo "  mbedtls: $MBED_DIR"
echo "  sqlite3: $SQLITE_OBJ"
echo "  zlib:    $ZLIB_DIR"
echo "  pxc 用法: tools/pxc build --no-quic --cc $CC \\"
echo "    --mbedtls-lib $MBED_DIR --sqlite-obj $SQLITE_OBJ --zlib-lib $ZLIB_DIR <file.px>"
