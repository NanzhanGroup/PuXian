#!/usr/bin/env bash
# ============================================================
# M89-S3-C2: 自举证明 · BC 轨（字节码权威对拍基准）
# ------------------------------------------------------------
# 规则更新（M89_PLAN S3-C C2）：自举证明基准从 C 文本（compiler.c）
# 切到 BCModule 字节码镜像（compiler.bc.dump）—— 编译器"正主"的权威
# 形态是字节码镜像：compiler.px 含 bc_emit 后，pxc 编 compiler.px 得到
# 的 BC dump（golden/compiler.bc.dump）即"含发射器自身的编译器镜像"。
#
# 本脚本证明链（VM 编译器编译自身）：
#   bootstrap/pxc build compiler.px → B.c（C 引擎产物，含 bc_emit 生态）
#     → gcc 链 rtcache（含 vm.o）→ compiler_new（C 引擎版编译器二进制）
#   compiler_new --emit-c compiler.px → compiler_vm.c（BCModule 静态 C
#     = compiler.px 的字节码镜像 + VM main，即 VM 驱动版编译器）
#     → gcc 链 rtcache → compiler_vm
#   compiler_vm bc compiler.px → dump 与 golden/compiler.bc.dump 逐字节对拍
#     （VM 编译器编译 compiler.px 自身，镜像==权威）
#
# 用法：
#   ./bootstrap_prove_bc.sh             # BC 轨自举证明（缓存有效则复用）
#   ./bootstrap_prove_bc.sh --fresh     # 强制全链重建（约 12-15 分钟）
#   ./bootstrap_prove_bc.sh --update-golden  # 有意改动后重定基 golden/compiler.bc.dump
#                                            # （基准由 C 引擎 compiler_new 生成，再由 VM 引擎
#                                            #   compiler_vm 重放逐字节自证——跨引擎才算真定基）
# 前置：bootstrap/pxc（引导编译器）；.rtcache 含 vm.o 的缓存（px build 任一
#   程序生成）；gcc。golden/compiler.c 的 C 轨同步由 bootstrap_prove.sh 守护。
# ============================================================
set -u
cd "$(dirname "$0")"
PXC="../bootstrap/pxc"
RT="../runtime"
GOLDEN_BC="golden/compiler.bc.dump"
BUILD="build"                      # selfhost/build（gitignore）
WORK=/tmp/px_bootstrap
mkdir -p "$WORK" "$BUILD"

# 源码链（compiler.px 全 import 链 + runtime vm.c 变更影响）
SRC_CHAIN="compiler.px codegen.px parser.px pxlexer.px cg_stmt.px cg_expr.px cg_module.px bc_emit.px ../runtime/vm.c ../runtime/vm.h ../runtime/runtime.c ../runtime/runtime.h"
norm_bc() { grep -vE '^# BCModule ' ; }

# 最新源码时间（compiler.px import 链）
newest_src() {
    local newest=0 m
    for f in $SRC_CHAIN; do
        [ -f "$f" ] || continue
        m=$(stat -c %Y "$f")
        [ "$m" -gt "$newest" ] && newest=$m
    done
    echo "$newest"
}

fresh_build() { [ "${1:-}" = "--fresh" ]; }

# M112/Issue 48：--update-golden（重定基 golden/compiler.bc.dump，步骤 5 的 VM 重放即自证）
UPDATE_GOLDEN=0
for a in "$@"; do [ "$a" = "--update-golden" ] && UPDATE_GOLDEN=1; done

echo "══════════ M89-S3-C2 自举证明（BC 轨 · 字节码权威）══════════"
[ -x "$PXC" ] || { echo "❌ 缺少引导编译器 bootstrap/pxc" >&2; exit 1; }
[ -f "$GOLDEN_BC" ] || { echo "❌ 缺少基准 golden/compiler.bc.dump" >&2; exit 1; }
echo "── 基准：golden/compiler.bc.dump（$(wc -l < "$GOLDEN_BC") 行字节码镜像）"

# ---- 步骤 1：B.c（pxc 编 compiler.px，C 引擎产物）----
if fresh_build "$@" || [ ! -s "$BUILD/compiler_new.c" ] \
   || [ "$(stat -c %Y "$BUILD/compiler_new.c" 2>/dev/null || echo 0)" -lt "$(newest_src)" ]; then
    echo "── 步骤 1：bootstrap/pxc 编译 compiler.px → compiler_new.c（实测约 6.5-7 分钟）"
    timeout 900 "$PXC" build compiler.px > "$BUILD/compiler_new.c" 2>/tmp/bpbc_build.log || {
        echo "❌ pxc build compiler.px 失败" >&2; tail -5 /tmp/bpbc_build.log >&2; exit 1; }
else
    echo "── 步骤 1：compiler_new.c 缓存有效，复用"
fi
[ -s "$BUILD/compiler_new.c" ] || { echo "❌ compiler_new.c 为空" >&2; exit 1; }

# ---- 步骤 2：C 引擎版编译器二进制 compiler_new（若旧则重建）----
CACHE=""
for d in $(ls -dt ../.rtcache/*/ 2>/dev/null); do
    [ -f "$d/.complete" ] && [ -f "$d/vm.o" ] && CACHE="$d" && break
done
[ -n "$CACHE" ] || { echo "❌ 未找到含 vm.o 的 rtcache（先 px build 任一程序）" >&2; exit 1; }

need_new=0
fresh_build "$@" && need_new=1
[ -x "$BUILD/compiler_new" ] || need_new=1
[ "$(stat -c %Y "$BUILD/compiler_new" 2>/dev/null || echo 0)" -lt "$(stat -c %Y "$BUILD/compiler_new.c")" ] && need_new=1
if [ "$need_new" = "1" ]; then
    echo "── 步骤 2：gcc 编译+链接 compiler_new（C 引擎版编译器）"
    gcc -c -O2 -I"../$CACHE" -I"$RT" "$BUILD/compiler_new.c" -o /tmp/bpbc_cn.o 2>/tmp/bpbc_cn_cc.log || {
        echo "❌ compiler_new.c 编译失败" >&2; tail -15 /tmp/bpbc_cn_cc.log >&2; exit 1; }
    objs=""; for f in "$CACHE"/*.o; do objs="$objs $f"; done
    gcc -O2 -pthread -o "$BUILD/compiler_new" /tmp/bpbc_cn.o $objs \
        "$RT/third_party/sqlite3/sqlite3.o" \
        "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
        "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
        "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
        "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread 2>/tmp/bpbc_cn_link.log || {
        echo "❌ compiler_new 链接失败" >&2; tail -15 /tmp/bpbc_cn_link.log >&2; exit 1; }
else
    echo "── 步骤 2：compiler_new 缓存有效，复用"
fi
[ -x "$BUILD/compiler_new" ] || { echo "❌ compiler_new 不存在" >&2; exit 1; }

# ---- M112/Issue 48：--update-golden：用 C 引擎重定基 BC 基准（步骤 5 由 VM 引擎重放自证）----
if [ "$UPDATE_GOLDEN" = "1" ]; then
    echo "── [--update-golden] 重定基 golden/compiler.bc.dump（原 $(wc -l < "$GOLDEN_BC") 行）"
    cp -a "$GOLDEN_BC" "$WORK/compiler.bc.dump.bak"
    ug_rc=0
    ( ulimit -v 10000000; timeout 1500 "$BUILD/compiler_new" bc compiler.px \
        > "$WORK/new_golden.bc.dump" 2>/tmp/bpbc_golden.log ) || ug_rc=$?
    if [ "$ug_rc" -ne 0 ] || [ ! -s "$WORK/new_golden.bc.dump" ]; then
        echo "❌ 重定基失败（exit=$ug_rc），golden 未改动" >&2
        tail -5 /tmp/bpbc_golden.log >&2
        exit 1
    fi
    cp -a "$WORK/new_golden.bc.dump" "$GOLDEN_BC"
    echo "    新基准 $(wc -l < "$GOLDEN_BC") 行 / 旧 $(wc -l < "$WORK/compiler.bc.dump.bak") 行"
fi

# ---- 步骤 3：VM 驱动版编译器 compiler_vm（compiler.px 的字节码镜像 + VM）----
need_vm=0
fresh_build "$@" && need_vm=1
[ -s "$BUILD/compiler_vm.c" ] || need_vm=1
# compiler_vm.c 依赖 compiler_new 二进制（发射器）+ compiler.px 源码链
if [ "$(stat -c %Y "$BUILD/compiler_vm.c" 2>/dev/null || echo 0)" -lt "$(stat -c %Y "$BUILD/compiler_new")" ]; then need_vm=1; fi
if [ "$(stat -c %Y "$BUILD/compiler_vm.c" 2>/dev/null || echo 0)" -lt "$(stat -c %Y compiler.px)" ]; then need_vm=1; fi
if [ "$need_vm" = "1" ]; then
    echo "── 步骤 3：compiler_new --emit-c compiler.px → compiler_vm.c（约 4-5 分钟）"
    ( ulimit -v 10000000; timeout 1500 "$BUILD/compiler_new" --emit-c compiler.px \
        > "$BUILD/compiler_vm.c" 2>/tmp/bpbc_emitc.log ) || {
        echo "❌ emit-c compiler.px 失败" >&2; tail -5 /tmp/bpbc_emitc.log >&2; exit 1; }
else
    echo "── 步骤 3：compiler_vm.c 缓存有效，复用"
fi
[ -s "$BUILD/compiler_vm.c" ] || { echo "❌ compiler_vm.c 为空" >&2; exit 1; }

# ---- 步骤 4：gcc 链 compiler_vm ----
need_vmb=0
fresh_build "$@" && need_vmb=1
[ -x "$BUILD/compiler_vm" ] || need_vmb=1
[ "$(stat -c %Y "$BUILD/compiler_vm" 2>/dev/null || echo 0)" -lt "$(stat -c %Y "$BUILD/compiler_vm.c")" ] && need_vmb=1
if [ "$need_vmb" = "1" ]; then
    echo "── 步骤 4：gcc 编译+链接 compiler_vm（VM 驱动版编译器）"
    gcc -c -O2 -I"../$CACHE" -I"$RT" "$BUILD/compiler_vm.c" -o /tmp/bpbc_vm.o 2>/tmp/bpbc_vm_cc.log || {
        echo "❌ compiler_vm.c 编译失败" >&2; tail -15 /tmp/bpbc_vm_cc.log >&2; exit 1; }
    objs=""; for f in "$CACHE"/*.o; do objs="$objs $f"; done
    gcc -O2 -pthread -o "$BUILD/compiler_vm" /tmp/bpbc_vm.o $objs \
        "$RT/third_party/sqlite3/sqlite3.o" \
        "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
        "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
        "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
        "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread 2>/tmp/bpbc_vm_link.log || {
        echo "❌ compiler_vm 链接失败" >&2; tail -15 /tmp/bpbc_vm_link.log >&2; exit 1; }
else
    echo "── 步骤 4：compiler_vm 缓存有效，复用"
fi
[ -x "$BUILD/compiler_vm" ] || { echo "❌ compiler_vm 不存在" >&2; exit 1; }

# ---- 步骤 5：VM 编译器重放 compiler.px → BC dump，与 golden 对拍 ----
echo "── 步骤 5：compiler_vm bc compiler.px 重放（较慢，约 4-6 分钟）"
( ulimit -v 10000000; timeout 1500 "$BUILD/compiler_vm" bc compiler.px \
    > "$WORK/vm_replay.bc.dump" 2>/tmp/bpbc_replay.log ) || {
    echo "❌ VM 重放失败" >&2; tail -5 /tmp/bpbc_replay.log >&2; exit 1; }
[ -s "$WORK/vm_replay.bc.dump" ] || { echo "❌ VM 重放 dump 为空" >&2; exit 1; }
echo "    （dump $(wc -l < "$WORK/vm_replay.bc.dump") 行）"

norm_bc < "$WORK/vm_replay.bc.dump" > "$WORK/vm_replay.n"
norm_bc < "$GOLDEN_BC" > "$WORK/golden_bc.n"
if diff -q "$WORK/golden_bc.n" "$WORK/vm_replay.n" >/dev/null 2>&1; then
    echo "    ✅ VM 编译器重放 == golden/compiler.bc.dump（$(wc -l < "$WORK/golden_bc.n") 行）逐字节一致"
    echo ""
    echo "══════════ 🎉 BC 轨自举成立 ══════════"
    echo "compiler.px 的字节码镜像编译器（compiler_vm）重新编译 compiler.px，"
    echo "BCModule dump 与权威基线逐字节一致。"
    exit 0
else
    echo "    ❌ VM 重放 != golden/compiler.bc.dump："
    diff "$WORK/golden_bc.n" "$WORK/vm_replay.n" | head -20
    echo ""
    echo "提示：若刚改了编译器源码，需先确认改动是有意的；"
    echo "     有意改动请重定基：cd selfhost && ./bootstrap_prove_bc.sh --update-golden"
    echo "     （基准由 C 引擎 compiler_new 生成，再由 VM 引擎 compiler_vm 重放自证；"
    echo "       重烘入库二进制：cp selfhost/build/compiler_vm bootstrap/pxc_vm）"
    exit 1
fi
