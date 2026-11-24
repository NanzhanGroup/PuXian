#!/usr/bin/env bash
# ============================================================
# selfhost/build_pxvm.sh —— 构建 .pxe 宿主 pxvm（M442-S2）
# ------------------------------------------------------------
# 为什么要这个脚本：pxvm 必须与静态产物**链同一份 runtime.o**（否则「三轨等价」
#   就变成「两个不同的 runtime 碰巧输出一样」，判据会假绿）。
#   ⇒ 先让 px build 把 runtime 对象烘到 .rtcache/<semkey>/，再拿那些 .o 链接 pxvm。
# 用法：build_pxvm.sh <输出路径>
# 退出码：0 成功 / 2 用法 / 3 环境不成立（缺库 / 非 x86_64 宿主 / 对象缺失）
# ============================================================
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime"
OUT="${1:-}"
[ -n "$OUT" ] || { echo "用法: build_pxvm.sh <输出路径>" >&2; exit 2; }
[ -f "$PX" ] || { echo "缺 $PX" >&2; exit 3; }

ARCH="$(uname -m)"
if [ "$ARCH" != "x86_64" ]; then
    echo "build_pxvm: 暂只支持 x86_64 宿主（当前 $ARCH）—— 交叉链接留到 M442-S3" >&2
    exit 3
fi

CC="${PX_CC:-gcc}"
MB="$RT/mbedtls/lib"
ZL="$RT/third_party/zlib/lib"
SQLITE_O="$RT/third_party/sqlite3/sqlite3.o"
for f in "$MB/libmbedtls.a" "$MB/libmbedx509.a" "$MB/libmbedcrypto.a" "$ZL/libz.a" "$SQLITE_O"; do
    [ -f "$f" ] || { echo "build_pxvm: 缺静态库 $f" >&2; exit 3; }
done

WORK="$(mktemp -d "${TMPDIR:-/tmp}/pxvm.XXXXXX")" || exit 3
trap 'rm -rf "$WORK"' EXIT

# 1) 用平凡程序驱动 px build ⇒ runtime 对象进入 .rtcache/<semkey>/
printf 'def main():\n    return 0\n' > "$WORK/tiny.px"
if ! "$PX" build "$WORK/tiny.px" >/dev/null 2>"$WORK/boot.log"; then
    echo "build_pxvm: px build 预热失败 —— $(tail -3 "$WORK/boot.log" | tr '\n' ' ')" >&2
    exit 3
fi

# 2) 定位 sem 目录：.rtcache 下含 runtime.o 且 mtime 最新者（= 刚被使用的那份）
SEM="$(ls -dt "$ROOT"/.rtcache/*/runtime.o 2>/dev/null | head -1)"
SEM="$(dirname "${SEM:-/nonexistent}")"
[ -f "$SEM/runtime.o" ] || { echo "build_pxvm: 找不到 runtime 对象目录" >&2; exit 3; }

# 3) 编译 pxvm 侧两件（pxe.c loader + pxvm.c 宿主）
INC="-I$SEM -I$RT/mbedtls/include -I$RT/third_party/miniz -I$RT/third_party/sqlite3 \
     -I$RT/third_party/zlib -I$RT/third_party/ngtcp2/include -I$RT/third_party/openssl/include -I$RT"
CF="-O2 -pthread -static -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_FOREIGN_KEYS=1"
for src in pxe pxvm; do
    if ! $CC -c $CF $INC -o "$WORK/$src.o" "$RT/$src.c" 2>"$WORK/$src.log"; then
        echo "build_pxvm: 编译 $src.c 失败 —— $(tail -5 "$WORK/$src.log" | tr '\n' ' ')" >&2
        exit 3
    fi
done

# 4) 链接：sem 目录里全部 .o（与静态产物同一份）+ sqlite/mbedtls/zlib/ngtcp2/openssl
OBJS="$(ls "$SEM"/*.o 2>/dev/null | grep -v '/prog\.o$')"
$CC -static -pthread -o "$OUT" "$WORK/pxvm.o" "$WORK/pxe.o" $OBJS "$SQLITE_O" \
    "$MB/libmbedtls.a" "$MB/libmbedx509.a" "$MB/libmbedcrypto.a" \
    "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
    "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
    "$ZL/libz.a" -lm -ldl 2>"$WORK/link.log" || {
    echo "build_pxvm: 链接失败 —— $(tail -5 "$WORK/link.log" | tr '\n' ' ')" >&2
    exit 3
}
[ -x "$OUT" ] || { echo "build_pxvm: 产物不可执行 $OUT" >&2; exit 3; }
echo "pxvm OK: $OUT （$(stat -c %s "$OUT") B）"
