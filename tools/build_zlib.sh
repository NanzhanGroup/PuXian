#!/usr/bin/env bash
# ============================================================
# tools/build_zlib.sh —— M61-S0：zlib 外部静态库两版构建入库
# ------------------------------------------------------------
# 背景：FFI 外部系统库绑定 proof（M61 A 线）以 zlib 打通
#   「外部 .a 入库 → pxc 链接 → 语言调用 → 跨架构」全链路。
#   保持纯静态哲学：不依赖 apt 半套/动态链接，自编两版静态 .a：
#     runtime/third_party/zlib/lib/libz.a          （x86_64，gcc 静态）
#     runtime/third_party/zlib/lib-aarch64/libz.a  （aarch64 musl 静态）
#     runtime/third_party/zlib/include/{zlib.h,zconf.h}
# 用法：
#   tools/build_zlib.sh                    # 全量：下载+两版编译+拷贝+校验
#   tools/build_zlib.sh --src <源码目录>    # 复用已有源码目录（免下载）
#   tools/build_zlib.sh --zlib-ver 1.3.1   # 指定版本
#   tools/build_zlib.sh --cc <交叉CC>      # 指定交叉编译器（缺省 aarch64-linux-musl-gcc，
#                                          #   交叉链不在 PATH 时可传绝对路径）
# 产物架构校验：ar 抽 crc32.o，file 应为 x86-64 / ARM aarch64。
# ============================================================
set -euo pipefail
PXC_HOME="$(cd "$(dirname "$0")/.." && pwd)"
ZLIB_DIR="$PXC_HOME/runtime/third_party/zlib"
ZLIB_VER="${ZLIB_VER:-1.3.1}"
CC_ARM="${CC_ARM:-aarch64-linux-musl-gcc}"
JOBS="$(nproc 2>/dev/null || echo 8)"
SRC=""
TMP=""

while [ $# -gt 0 ]; do
    case "$1" in
        --src) SRC="$2"; shift 2 ;;
        --zlib-ver) ZLIB_VER="$2"; shift 2 ;;
        --cc) CC_ARM="$2"; shift 2 ;;
        --jobs) JOBS="$2"; shift 2 ;;
        *) echo "未知参数: $1" >&2; exit 1 ;;
    esac
done

# 交叉编译器校验（缺省假设在 PATH；不在可传绝对路径）
if [ "${CC_ARM#/}" = "$CC_ARM" ]; then
    command -v "$CC_ARM" >/dev/null 2>&1 || { echo "错误: 找不到交叉编译器 $CC_ARM（加 PATH 或 --cc 传绝对路径）" >&2; exit 1; }
else
    [ -x "$CC_ARM" ] || { echo "错误: 找不到交叉编译器 $CC_ARM" >&2; exit 1; }
fi
command -v gcc >/dev/null 2>&1 || { echo "错误: 需要 gcc 编 x86 版" >&2; exit 1; }
command -v ar >/dev/null 2>&1 || { echo "错误: 需要 ar" >&2; exit 1; }

mkdir -p "$ZLIB_DIR/include" "$ZLIB_DIR/lib" "$ZLIB_DIR/lib-aarch64"

# 1) 源码：复用或下载
if [ -n "$SRC" ]; then
    [ -d "$SRC" ] || { echo "错误: --src 目录不存在 $SRC" >&2; exit 1; }
else
    TMP="$(mktemp -d)"
    echo "== 下载 zlib-$ZLIB_VER =="
    curl -sSL -o "$TMP/zlib-$ZLIB_VER.tar.gz" \
        "https://github.com/madler/zlib/releases/download/v$ZLIB_VER/zlib-$ZLIB_VER.tar.gz"
    tar xzf "$TMP/zlib-$ZLIB_VER.tar.gz" -C "$TMP"
    SRC="$TMP/zlib-$ZLIB_VER"
fi

# 2) x86_64 静态编译
echo "== x86_64 静态编译 =="
( cd "$SRC" && ./configure --static >/dev/null && make -j"$JOBS" >/dev/null )
cp "$SRC/libz.a" "$ZLIB_DIR/lib/libz.a"
cp "$SRC/zlib.h" "$ZLIB_DIR/include/zlib.h"
cp "$SRC/zconf.h" "$ZLIB_DIR/include/zconf.h"

# 3) aarch64 musl 交叉静态编译（独立副本，避免污染 x86 产物）
ARM_SRC="$SRC"
if [ -n "$TMP" ]; then
    mkdir -p "$TMP/arm"
    tar xzf "$TMP/zlib-$ZLIB_VER.tar.gz" -C "$TMP/arm" --strip-components=1
    ARM_SRC="$TMP/arm/zlib-$ZLIB_VER"
    [ -d "$ARM_SRC" ] || ARM_SRC="$TMP/arm"
fi
echo "== aarch64 交叉静态编译 =="
( cd "$ARM_SRC" && \
    CC="$CC_ARM" AR="${CC_ARM%-gcc}-ar" RANLIB="${CC_ARM%-gcc}-ranlib" \
    ./configure --static >/dev/null && \
    make -j"$JOBS" >/dev/null )
cp "$ARM_SRC/libz.a" "$ZLIB_DIR/lib-aarch64/libz.a"

# 4) 架构校验
echo "== 架构校验 =="
TMPC="$(mktemp -d)"
( cd "$TMPC" && ar x "$ZLIB_DIR/lib/libz.a" crc32.o && \
  file crc32.o | grep -q 'x86-64' && echo "x86_64 版: OK" )
( cd "$TMPC" && ar x "$ZLIB_DIR/lib-aarch64/libz.a" crc32.o && \
  file crc32.o | grep -q 'aarch64' && echo "aarch64 版: OK" )
rm -rf "$TMPC" "$TMP"
echo "== 完成：$ZLIB_DIR/{lib,lib-aarch64,include} =="
