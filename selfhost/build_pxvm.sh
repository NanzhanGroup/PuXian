#!/usr/bin/env bash
# ============================================================
# selfhost/build_pxvm.sh —— 构建 .pxe 宿主 pxvm（M442-S2 建 · M442 补3 支持 aarch64）
# ------------------------------------------------------------
# 为什么要这个脚本：pxvm 必须与静态产物**链同一份 runtime.o**（否则「三轨等价」
#   就变成「两个不同的 runtime 碰巧输出一样」，判据会假绿）。
#   ⇒ 先让 px build 把 runtime 对象烘到 .rtcache/<semkey>/，再拿那些 .o 链接 pxvm。
#
# 为什么必须能编 aarch64（M442 补3 · 缺陷 930）：
#   bootstrap/pxvm 是**入库件**（与其余 12 件工具同级）。入库件是 x86_64 的，
#   而 CI「原生 aarch64 自举自证」与 release「aarch64 自举件」都要求
#   bootstrap/* 每一件都是 **aarch64 全静态**（M168 可移植性门）。
#   若 aarch64 侧不同批现编 pxvm ⇒ 该门判「架构不符」，真发出去就是 Exec format error。
#   ⇒ 本脚本按宿主架构选库（x86_64 用默认布局 + 保留 QUIC；aarch64 用 -aarch64 预置库
#     + -DPX_NO_QUIC，与 selfhost/native_bootstrap.sh 同口径）。
#
# 用法：build_pxvm.sh <输出路径>
# 退出码：0 成功 / 2 用法 / 3 环境不成立（缺库 / 不支持的宿主 / 对象缺失）
# ============================================================
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime"
OUT="${1:-}"
[ -n "$OUT" ] || { echo "用法: build_pxvm.sh <输出路径>" >&2; exit 2; }
[ -f "$PX" ] || { echo "缺 $PX" >&2; exit 3; }

ARCH="$(uname -m)"
case "$ARCH" in
    x86_64)        TA=x86_64 ;  NQ="" ;;
    aarch64|arm64) TA=aarch64 ; NQ="-DPX_NO_QUIC" ;;
    *) echo "build_pxvm: 不支持的宿主架构 $ARCH（已知 x86_64 / aarch64）" >&2; exit 3 ;;
esac

CC="${PX_CC:-gcc}"
# 目标架构静态库：x86_64 用仓库默认布局；aarch64 用 -aarch64 预置（同 native_bootstrap.sh）
if [ "$TA" = x86_64 ]; then
    MB="$RT/mbedtls/lib"
    ZL="$RT/third_party/zlib/lib"
    SQLITE_O="$RT/third_party/sqlite3/sqlite3.o"
    QUIC_LIBS="$RT/third_party/ngtcp2/lib/libngtcp2.a $RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a $RT/third_party/openssl/lib/libssl.a $RT/third_party/openssl/lib/libcrypto.a"
else
    MB="$RT/mbedtls/lib-$TA"
    ZL="$RT/third_party/zlib/lib-$TA"
    SQLITE_O="$RT/third_party/sqlite3/sqlite3-$TA.o"
    QUIC_LIBS=""
fi
for f in "$MB/libmbedtls.a" "$MB/libmbedx509.a" "$MB/libmbedcrypto.a" "$ZL/libz.a" "$SQLITE_O"; do
    [ -f "$f" ] || { echo "build_pxvm: 缺静态库 $f" >&2; exit 3; }
done

WORK="$(mktemp -d "${TMPDIR:-/tmp}/pxvm.XXXXXX")" || exit 3
trap 'rm -rf "$WORK"' EXIT

# 1) 用平凡程序驱动 px build ⇒ runtime 对象进入 .rtcache/<semkey>/
#    （非 x86_64 宿主上 tools/px 会自动落 C 轨 + no_quic —— M159 自适应，CI 有断言）
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
# M442-S3：版本号由 tools/px 注入（发布自检 ⑥ 用 `pxvm --version` 判「件是否可执行」）
PXVM_VER="$("$PX" --version 2>/dev/null | awk '{print $2}')"
[ -n "$PXVM_VER" ] || PXVM_VER="0.0.0-dev"
CF="-O2 -pthread -static -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_FOREIGN_KEYS=1 -DPXVM_VER=\"$PXVM_VER\" $NQ"
for src in pxe pxvm; do
    if ! $CC -c $CF $INC -o "$WORK/$src.o" "$RT/$src.c" 2>"$WORK/$src.log"; then
        echo "build_pxvm: 编译 $src.c 失败 —— $(tail -5 "$WORK/$src.log" | tr '\n' ' ')" >&2
        exit 3
    fi
done

# 4) 指纹对象（M442-S3）：入库件必须能回答「我是哪份源码/哪份 runtime 烘的」
#   PXRT-<rtkey>  = 链进产物的 runtime 源语义指纹（与 bootstrap 14 件同一口径）
#   PXVMFP-<sem>  = .pxe 宿主自身源码（pxe.c/pxe.h/pxvm.c）的语义指纹（注释不计入）
RTKEY="$("$PX" rtkey 2>/dev/null | tail -1)"
[ -n "$RTKEY" ] || { echo "build_pxvm: 取不到 rt_key（tools/px rtkey 返回空）" >&2; exit 3; }
VFP=""
if VFP="$("$PX" norm --join "$RT/pxe.c" "$RT/pxe.h" "$RT/pxvm.c" 2>/dev/null)" && [ -n "$VFP" ]; then
    :
else
    VFP="$(cat "$RT/pxe.c" "$RT/pxe.h" "$RT/pxvm.c" | sha256sum | cut -c1-16)"
fi
cat > "$WORK/fp.c" <<EOF
/* M442-S3：入库 pxvm 的来源指纹（host-only 常量，不进任何语义） */
const char px_pxvm_rtfp[]  = "PXRT-${RTKEY}";
const char px_pxvm_srcfp[] = "PXVMFP-${VFP}";
EOF
$CC -c -O2 -o "$WORK/fp.o" "$WORK/fp.c" 2>"$WORK/fp.log" || {
    echo "build_pxvm: 指纹对象编译失败 —— $(tail -3 "$WORK/fp.log" | tr '\n' ' ')" >&2
    exit 3
}

# 5) 链接：sem 目录里全部 .o（与静态产物同一份）+ sqlite/mbedtls/zlib（+ x86_64 的 QUIC 四库）
OBJS="$(ls "$SEM"/*.o 2>/dev/null | grep -v '/prog\.o$')"
$CC -static -pthread -o "$OUT" "$WORK/pxvm.o" "$WORK/pxe.o" "$WORK/fp.o" $OBJS "$SQLITE_O" \
    "$MB/libmbedtls.a" "$MB/libmbedx509.a" "$MB/libmbedcrypto.a" \
    $QUIC_LIBS "$ZL/libz.a" -lm -ldl 2>"$WORK/link.log" || {
    echo "build_pxvm: 链接失败 —— $(tail -5 "$WORK/link.log" | tr '\n' ' ')" >&2
    exit 3
}
[ -x "$OUT" ] || { echo "build_pxvm: 产物不可执行 $OUT" >&2; exit 3; }
# 自证：指纹真的链进去了（否则 --check 会判「无内嵌指纹」）
grep -aq "PXRT-${RTKEY}" "$OUT" 2>/dev/null || { echo "build_pxvm: 产物内缺 PXRT 指纹（链接被丢？）" >&2; exit 3; }
grep -aq "PXVMFP-${VFP}" "$OUT" 2>/dev/null || { echo "build_pxvm: 产物内缺 PXVMFP 指纹" >&2; exit 3; }
echo "pxvm OK: $OUT （$(stat -c %s "$OUT") B · $TA · PXRT-${RTKEY} · PXVMFP-${VFP}）"
