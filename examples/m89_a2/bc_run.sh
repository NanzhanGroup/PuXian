#!/usr/bin/env bash
# ============================================================
# M89-S3-A2: bc_run.sh —— VM 端到端运行（发射器 emit-c → gcc 链接 → 跑 Top）
# ------------------------------------------------------------
# 用法：./examples/m89_a2/bc_run.sh <file.px> [rtcache_dir]
#   rtcache_dir 缺省自动探测仓库 .rtcache 下任一 .complete 缓存目录。
# 前置：selfhost/build/bc_cli 已 build（px build selfhost/bc_cli.px ——
#   tools/px rt_src_files 已含 vm.c/vm.h，rtcache 自动重建含 vm.o）。
# 流程：bc_cli --emit-c <file> → 静态 BCModule C → gcc 链接 rtcache .o →
#   PX_BC_DUMP=1 运行（打印非函数全局 name=value，A 阶段对拍用）。
# 退出码：0=运行成功；非 0=编译/链接/运行失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC_HOME="$(pwd)"
RT="$PXC_HOME/runtime"
BC_CLI="$PXC_HOME/selfhost/build/bc_cli"
SRC="${1:?用法: bc_run.sh <file.px> [rtcache_dir]}"
CACHE="${2:-}"
[ -x "$BC_CLI" ] || { echo "❌ 缺 selfhost/build/bc_cli（先 px build selfhost/bc_cli.px）" >&2; exit 1; }
if [ -z "$CACHE" ] || [ ! -d "$CACHE" ]; then
    # 选含 vm.o 的最新 rtcache（mtime 新者优先，避免命中改动前旧 vm.o）
    for d in $(ls -dt "$PXC_HOME"/.rtcache/*/ 2>/dev/null); do
        [ -f "$d/.complete" ] && [ -f "$d/runtime.o" ] || continue
        if [ -f "$d/vm.o" ]; then CACHE="$d"; break; fi
        [ -z "$CACHE" ] && CACHE="$d"
    done
fi
[ -n "$CACHE" ] && [ -f "$CACHE/runtime.o" ] || { echo "❌ 未找到含 vm.o 的 rtcache（先 px build 任一程序重建缓存）" >&2; exit 1; }
[ -f "$CACHE/vm.o" ] || { echo "❌ rtcache 无 vm.o（tools/px 已加 vm.c，需重建缓存）" >&2; exit 1; }
echo "── emit-c: $SRC" >&2
echo "── rtcache: $CACHE" >&2
NAME="$(basename "$SRC" .px)"
"$BC_CLI" --emit-c "$SRC" > "/tmp/bc_run_${NAME}.c" || { echo "❌ emit-c 失败" >&2; exit 1; }
gcc -c -O2 -I"$CACHE" -I"$RT" "/tmp/bc_run_${NAME}.c" -o "/tmp/bc_run_${NAME}.o" 2>/tmp/bc_run_cc.log || {
    echo "❌ C 编译失败 —— 日志 /tmp/bc_run_cc.log" >&2; tail -15 /tmp/bc_run_cc.log >&2; exit 1; }
objs=""
for f in "$CACHE"/*.o; do objs="$objs $f"; done
gcc -static -O2 -pthread -o "/tmp/bc_run_${NAME}" "/tmp/bc_run_${NAME}.o" $objs \
    "$RT/third_party/sqlite3/sqlite3.o" \
    "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
    "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
    "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
    "$RT/third_party/zlib/lib/libz.a" -lm -ldl 2>/tmp/bc_run_link.log || {
        echo "❌ 链接失败 —— 日志 /tmp/bc_run_link.log" >&2; tail -15 /tmp/bc_run_link.log >&2; exit 1; }
echo "── 运行（PX_BC_DUMP=1 打印非函数全局）" >&2
PX_BC_DUMP=1 "/tmp/bc_run_${NAME}"
