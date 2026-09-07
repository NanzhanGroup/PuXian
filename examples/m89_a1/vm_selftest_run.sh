#!/usr/bin/env bash
# ============================================================
# M89-S3-A1: vm_selftest_run.sh —— VM 引擎 A1 自测（编译 + 链接 + 运行）
# ------------------------------------------------------------
# 用法：./examples/m89_a1/vm_selftest_run.sh [rtcache_dir]
#   rtcache_dir 缺省自动探测仓库 .rtcache 下任一 .complete 缓存目录（预编译
#   runtime .o 全家，含 quic/全模块的无裁剪 cache 优先）。
# 说明：vm.c 尚未入 rt_src_files（A1 起接入时机见 M89_PLAN），故本自测把
#   vm.c 与手写 PxBCModule selftest 单独编 .o，链接 rtcache 预编译 runtime.o，
#   验证 GETG/SETG/px_vm_run_module（Top 运行 + 函数注册 D2 trampoline）。
# 退出码：0=ALL PASS；1=编译/链接失败或断言失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC_HOME="$(pwd)"
RT="$PXC_HOME/runtime"
CACHE="${1:-}"
if [ -z "$CACHE" ] || [ ! -d "$CACHE" ]; then
    # 优先含 quic 全模块的 cache（runtime_quic.o 存在）；否则任一 .complete
    for d in "$PXC_HOME"/.rtcache/*/; do
        [ -f "$d/.complete" ] && [ -f "$d/runtime.o" ] || continue
        if [ -f "$d/runtime_quic.o" ]; then CACHE="$d"; break; fi
        [ -z "$CACHE" ] && CACHE="$d"
    done
fi
[ -n "$CACHE" ] && [ -f "$CACHE/runtime.o" ] || { echo "❌ 未找到 rtcache 预编译目录（先 px build 任一程序生成）" >&2; exit 1; }
echo "── rtcache: $CACHE"

gcc -c -O2 -I"$CACHE" -I"$RT" "$RT/vm.c" -o /tmp/vm_selftest_vm.o || { echo "❌ vm.c 编译失败" >&2; exit 1; }
gcc -c -O2 -I"$CACHE" -I"$RT" "examples/m89_a1/vm_selftest.c" -o /tmp/vm_selftest_main.o || { echo "❌ selftest 编译失败" >&2; exit 1; }

objs=""
for f in "$CACHE"/*.o; do objs="$objs $f"; done
gcc -static -O2 -pthread -o /tmp/vm_selftest /tmp/vm_selftest_main.o /tmp/vm_selftest_vm.o $objs \
    "$RT/third_party/sqlite3/sqlite3.o" \
    "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
    "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
    "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
    "$RT/third_party/zlib/lib/libz.a" -lm -ldl 2>/tmp/vm_selftest_link.log || {
        echo "❌ 链接失败 —— 日志 /tmp/vm_selftest_link.log" >&2; tail -15 /tmp/vm_selftest_link.log >&2; exit 1; }
echo "── 运行"
/tmp/vm_selftest
