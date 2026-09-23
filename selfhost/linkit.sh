#!/usr/bin/env bash
# linkit.sh <file.c> <out> [static|dynamic] —— 用最新 .rtcache 链接 PuXian C 产物
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"   # M188-STDLIB：去掉硬编码仓库路径
export LC_ALL=C LANG=C
RT="$ROOT/runtime"
CF="$1"; OUT="$2"; MODE="${3:-static}"
CACHE=""; best_m=0
for d in "$ROOT"/.rtcache/*/; do
    n=$(basename "$d"); [ ${#n} -eq 16 ] || continue
    [ -f "$d/runtime.o" ] || continue
    m=$(stat -c %Y "$d/runtime.o" 2>/dev/null || echo 0)
    if [ "$m" -gt "$best_m" ]; then best_m=$m; CACHE="${d%/}"; fi
done
objs=""; for f in "$CACHE"/*.o; do objs="$objs $f"; done
gcc -c -O2 -I"$CACHE" -I"$RT" "$CF" -o /tmp/linkit.o 2>/tmp/linkit_cc.log || { echo "❌ C 编译失败"; tail -10 /tmp/linkit_cc.log; exit 1; }
FLAGS=""; [ "$MODE" = "static" ] && FLAGS="-static"
gcc $FLAGS -O2 -pthread -o "$OUT" /tmp/linkit.o $objs \
    "$RT/third_party/sqlite3/sqlite3.o" \
    "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
    "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
    "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
    "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread 2>/tmp/linkit_link.log || { echo "❌ 链接失败"; tail -10 /tmp/linkit_link.log; exit 1; }
echo "✅ $OUT（$(stat -c %s "$OUT") 字节）"
