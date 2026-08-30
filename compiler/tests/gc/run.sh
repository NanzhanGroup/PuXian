#!/bin/bash
# 普贤 GC 单元测试（M8）—— 编译并运行 C 级 GC 测试
# M10：链接 mbedtls（runtime.c 含 HTTPS 支持）
# 用法：bash tests/gc/run.sh
set -e
cd "$(dirname "$0")/../.."   # compiler/
MB="runtime/mbedtls"
gcc -std=c99 -O2 -Iruntime -I$MB/include tests/gc/test_gc.c runtime/runtime.c \
    $MB/lib/libmbedtls.a $MB/lib/libmbedx509.a $MB/lib/libmbedcrypto.a \
    -o /tmp/test_gc -lpthread -lm
/tmp/test_gc
echo "---"
echo "GC 演示（编译模式 + 内存对照）："
gcc -std=c99 -O2 -Iexamples/build -Iruntime -I$MB/include examples/build/gc_demo.c runtime/runtime.c \
    $MB/lib/libmbedtls.a $MB/lib/libmbedx509.a $MB/lib/libmbedcrypto.a \
    -o /tmp/gc_demo -lpthread -lm 2>/dev/null || true
echo "（用 examples/ 下 px build gc_demo.px 生成后运行 ./build/gc_demo 验证）"
