#!/usr/bin/env bash
# ============================================================
# M89-S3-C2: vm_run.sh —— VM 轨运行器（compiler.px 正主 emit-c → gcc → 跑）
# ------------------------------------------------------------
# C2 收口：bc_cli.px 姊妹壳退役，VM 发射切 compiler.px 正主（compiler_new，
#   = pxc 编 compiler.px 的 C 引擎产物；含 bc/--emit-c 子命令）。
# 流程：compiler_new --emit-c <file> → 静态 BCModule C → gcc 链 rtcache
#   （含 vm.o）→ 运行。退出码透传被编译程序的退出码。
# 用法：./examples/m89_a2/vm_run.sh <file.px> [args...]
# 前置：selfhost/build/compiler_new 已 build（跑过 bootstrap_prove_bc.sh
#   或 px build 任一程序生成含 vm.o 的 rtcache）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC_HOME="$(pwd)"
RT="$PXC_HOME/runtime"
CN="$PXC_HOME/selfhost/build/compiler_new"
SRC="${1:?用法: vm_run.sh <file.px> [args...]}"
shift || true
[ -x "$CN" ] || { echo "❌ 缺 selfhost/build/compiler_new（先跑 bootstrap_prove_bc.sh）" >&2; exit 1; }

# 选含 vm.o 的最新 rtcache
CACHE=""
for d in $(ls -dt "$PXC_HOME"/.rtcache/*/ 2>/dev/null); do
    [ -f "$d/.complete" ] && [ -f "$d/vm.o" ] && CACHE="$d" && break
done
[ -n "$CACHE" ] && [ -f "$CACHE/vm.o" ] || { echo "❌ 未找到含 vm.o 的 rtcache" >&2; exit 1; }

NAME="$(basename "$SRC" .px)"
echo "── vm_run: $SRC（compiler_new emit-c → gcc → 运行）" >&2
# emit-c（虚拟内存高水位，放宽 ulimit；本机 16G）
( ulimit -v 10000000
  "$CN" --emit-c "$SRC" > "/tmp/vm_run_${NAME}.c" 2>/tmp/vm_run_emit.log ) || {
    echo "❌ emit-c 失败" >&2; tail -5 /tmp/vm_run_emit.log >&2; exit 1; }
gcc -c -O2 -I"$CACHE" -I"$RT" "/tmp/vm_run_${NAME}.c" -o "/tmp/vm_run_${NAME}.o" 2>/tmp/vm_run_cc.log || {
    echo "❌ C 编译失败 —— 日志 /tmp/vm_run_cc.log" >&2; tail -15 /tmp/vm_run_cc.log >&2; exit 1; }
objs=""
for f in "$CACHE"/*.o; do objs="$objs $f"; done
gcc -O2 -pthread -o "/tmp/vm_run_${NAME}" "/tmp/vm_run_${NAME}.o" $objs \
    "$RT/third_party/sqlite3/sqlite3.o" \
    "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
    "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
    "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
    "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread 2>/tmp/vm_run_link.log || {
        echo "❌ 链接失败 —— 日志 /tmp/vm_run_link.log" >&2; tail -15 /tmp/vm_run_link.log >&2; exit 1; }
"/tmp/vm_run_${NAME}" "$@"
exit $?
