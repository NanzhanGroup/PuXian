#!/usr/bin/env bash
# ============================================================
# selfhost/native_bootstrap.sh —— **只用 gcc 的原生/交叉自举**（M159 / R45 · 缺口 G1+G2+G4）
# ------------------------------------------------------------
# 存在理由（第三方仓库照出来的一等缺口）：
#   入库的 bootstrap/* 全是 **x86_64** 二进制（pxc/pxi/pxl/…，14 件静态 + pxc_vm 动态）。
#   ⇒ 在 aarch64 / riscv64 / armv7 **原生**上，`tools/px` 连第一步都跑不起来（Exec format
#     error），用户只能自己从源码搓一套自举链 —— banshanhanfu/px-openEuler-bootstrap
#     就是为此诞生的第三方仓。
# 本脚本把那条链**变成官方能力**（且不复用第三方脚本）：只用目标机自带的 gcc，
#   ① 编译全部 runtime 源（QUIC 裁剪：aarch64/armv7/riscv64 无 ngtcp2/openssl-quictls 静态库）
#   ② 用 **入库基准 selfhost/golden/compiler.c** 编出 pxc0（= 编译器的 C 产物）
#   ③ **自证**：pxc0 编译 selfhost/compiler.px → 与基准逐字节一致（归一化后）
#      —— 该步**只在目标架构 == 本机架构时可跑**；交叉档跳过（由 CI 的 native-arm64 真机
#      job 负责自证，见 .github/workflows/ci.yml 的 native-arm64）
#   ④ 交付：<out>/pxc（目标架构编译器）；--install 可装成 bootstrap/pxc-<arch> 或本机 pxc
# 用法：
#   ./selfhost/native_bootstrap.sh                          # 原生全流程（约 1-3 分钟）
#   ./selfhost/native_bootstrap.sh --outdir DIR             # 产物目录（默认 /tmp/px-native-bootstrap）
#   ./selfhost/native_bootstrap.sh --no-prove               # 跳过步骤③（仅求可用编译器）
#   ./selfhost/native_bootstrap.sh --jobs N                 # 并行编译任务数（默认 nproc）
#   ./selfhost/native_bootstrap.sh --target-arch aarch64    # **交叉档**：本机产出 aarch64 自举件
#                                                           # （自动找 /opt/aarch64-linux-musl-cross
#                                                           #   或 PATH 上的 musl 交叉 gcc；musl=静态）
#   ./selfhost/native_bootstrap.sh --install                # 装成 bootstrap/pxc-<target>（非本机架构时）
#                                                           # 本机架构则**拒绝**覆盖入库件（那份由
#                                                           # selfhost/rebake_bin.sh 重烘，带出厂指纹）
#   CC=aarch64-linux-gnu-gcc ./selfhost/native_bootstrap.sh --target-arch aarch64 --no-quic   # 手工交叉
# 退出码：0 = 成功（含自证）；非 0 = 失败（打印首个失败日志尾部）
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/.."
ROOT="$(pwd)"
RT="$ROOT/runtime"
GOLDEN="$ROOT/selfhost/golden/compiler.c"
SRC="$ROOT/selfhost/compiler.px"
OUTDIR="${PX_NATIVE_OUT:-/tmp/px-native-bootstrap}"
CC="${CC:-}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"
PROVE=1
TA=""                 # 目标架构（空 = 本机）
FORCE_NQ=""           # 1 = 强制 --no-quic（--no-quic 显式给定）
INSTALL=0
while [ $# -gt 0 ]; do
    case "$1" in
        --outdir) OUTDIR="$2"; shift 2 ;;
        --no-prove) PROVE=0; shift ;;
        --jobs) JOBS="$2"; shift 2 ;;
        --target-arch) TA="$2"; shift 2 ;;
        --no-quic) FORCE_NQ=1; shift ;;
        --install) INSTALL=1; shift ;;
        -h|--help) sed -n '2,42p' "$0"; exit 0 ;;
        *) echo "未知参数: $1" >&2; exit 2 ;;
    esac
done
[ -f "$GOLDEN" ] || { echo "❌ 缺基准 $GOLDEN" >&2; exit 1; }
[ -f "$SRC" ] || { echo "❌ 缺 $SRC" >&2; exit 1; }

# ---- 架构名归一 ----
norm_arch() {
    case "$1" in
        x86_64|amd64)     echo x86_64 ;;
        aarch64|arm64)    echo aarch64 ;;
        armv7l|armv6l|arm) echo armv7 ;;
        riscv64)          echo riscv64 ;;
        *)                echo "$1" ;;
    esac
}
HOST_A="$(norm_arch "$(uname -m)")"
[ -n "$TA" ] || TA="$HOST_A"
TA="$(norm_arch "$TA")"
CROSS=0; [ "$TA" != "$HOST_A" ] && CROSS=1

# ---- 交叉档自动找编译器（musl 交叉，产物静态零依赖）----
if [ -z "$CC" ]; then
    if [ "$CROSS" = 0 ]; then
        CC=gcc
    else
        for cand in \
            "/opt/$TA-linux-musl-cross/bin/$TA-linux-musl-gcc" \
            "/opt/$TA-linux-musl-cross/bin/$TA-linux-musl-cc" \
            "/opt/$TA-linux-musl-cross/bin/$TA-linux-gnu-gcc"
        do
            [ -x "$cand" ] && { CC="$cand"; break; }
        done
        if [ -z "$CC" ]; then
            CC="$(command -v "$TA-linux-musl-gcc" 2>/dev/null || true)"
        fi
        if [ -z "$CC" ]; then
            echo "❌ 交叉档缺 $TA 的 C 编译器 —— 试：CC=$TA-linux-musl-gcc $0 --target-arch $TA" >&2
            echo "   获取工具链：https://musl.cc/$TA-linux-musl-cross.tgz（解到 /opt/ 即可被自动发现）" >&2
            exit 1
        fi
    fi
fi

mkdir -p "$OUTDIR/obj"
rm -f "$OUTDIR"/obj/*.o "$OUTDIR"/compiler_golden.o
LOG="$OUTDIR/build.log"
: > "$LOG"
say() { echo "$*" | tee -a "$LOG"; }
tail_log() { [ -s "$1" ] && tail -12 "$1" | sed 's/^/     /' >&2; }

say "══ 自举（M159）══ 本机=$HOST_A 目标=$TA $([ "$CROSS" = 1 ] && echo '（交叉档）' || echo '（原生档）') cc=$CC jobs=$JOBS outdir=$OUTDIR"
say "   编译器：$($CC --version 2>&1 | head -1)"
say "   内核：$(uname -sr) · libc：$(ldd --version 2>&1 | head -1)"
say "── 基准：golden/compiler.c（$(wc -l < "$GOLDEN") 行）"

# ---- QUIC 裁剪判定：aarch64/armv7/riscv64（及任何非 x86_64 目标）无 ngtcp2/openssl-quictls
#      预编译静态库 ⇒ 一律 -DPX_NO_QUIC。x86_64 上保留 QUIC（与入库件口径一致）。
NQ=""
case "$TA" in
    x86_64|amd64) : ;;
    *) NQ="-DPX_NO_QUIC" ;;
esac
[ -n "$FORCE_NQ" ] && NQ="-DPX_NO_QUIC"
# 交叉档产物一律静态（musl 默认静态，显式写出避免 glibc 交叉链出动态件）
LINK_EXTRA=""
[ "$CROSS" = 1 ] && LINK_EXTRA="-static"
[ -n "$NQ" ] && say "── 裁剪 QUIC/H3（$TA 无预编译 ngtcp2/openssl-quictls 静态库）"

# ---- 步骤 1：runtime 目标文件（**原地编译**：源取自 runtime/，头路径按 include 语义全给）
say "── 步骤 1/4：编译 runtime（$(ls "$RT"/*.c | wc -l) 个 .c + miniz 3 个）"
DEFS="-O2 -pthread -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_DEFAULT_FOREIGN_KEYS=1"
INC="-I$RT -I$RT/mbedtls/include -I$RT/third_party/sqlite3 -I$RT/third_party/zlib/include -I$RT/third_party/miniz -I$RT/third_party/stb"
[ -n "$NQ" ] || INC="$INC -I$RT/third_party/ngtcp2/include -I$RT/third_party/openssl/include"
mkdir -p "$OUTDIR/obj"
rm -f "$OUTDIR"/obj/*.o "$OUTDIR"/compiler_golden.o
SRCS=""
for f in $(ls "$RT"/*.c); do
    # 裁 QUIC 时**剔除 QUIC/H3 四件**：它们无 PX_NO_QUIC 守卫，且 runtime_h3.c 依赖
    #   runtime_quic.c 的 px_quic_* 符号（实测链接期 undefined reference）；而 runtime.c 里
    #   的 quic/h3 注册与绑定已被 PX_NO_QUIC 守卫，不需要它们。口径与 tools/px
    #   `--no-quic`（rt_src_files 的 quic 裁剪）**逐文件一致**。
    case "$(basename "$f")" in
        runtime_quic.c|runtime_h3.c|runtime_h3_qpack.c|runtime_h3_qpack_dyn.c)
            [ -n "$NQ" ] && continue ;;
    esac
    SRCS="$SRCS $f"
done
SRCS="$SRCS $RT/third_party/miniz/miniz.c $RT/third_party/miniz/miniz_tinfl.c $RT/third_party/miniz/miniz_tdef.c"
pids=0
for f in $SRCS; do
    b="$(basename "$f" .c)"
    ( $CC -c $DEFS $NQ $INC "$f" -o "$OUTDIR/obj/$b.o" > "$OUTDIR/cc_$b.log" 2>&1 ) &
    pids=$((pids+1))
    if [ "$pids" -ge "$JOBS" ]; then wait -n 2>/dev/null || wait; pids=$((pids-1)); fi
done
wait
fail=0
for f in $SRCS; do
    b="$(basename "$f" .c)"
    [ -f "$OUTDIR/obj/$b.o" ] || { say "❌ 编译失败：$b.c"; tail_log "$OUTDIR/cc_$b.log"; fail=1; }
done
[ "$fail" = 0 ] || { say "❌ 步骤 1 失败"; exit 1; }
say "   ✅ runtime 对象 $(ls "$OUTDIR/obj"/*.o | wc -l) 个"

# ---- 步骤 2：golden/compiler.c → pxc0（链 runtime 对象 + sqlite/mbedtls/zlib）
say "── 步骤 2/4：编译基准编译器 → pxc0（目标 $TA）"
CPX_LOG="$OUTDIR/pxc0.log"
$CC -c -O2 $DEFS $NQ $INC "$GOLDEN" -o "$OUTDIR/compiler_golden.o" > "$CPX_LOG" 2>&1 || { say "❌ golden 编译失败"; tail_log "$CPX_LOG"; exit 1; }
# 目标架构静态库：x86_64 用仓库默认布局；aarch64 用 lib-aarch64；armv7/riscv64 需先交叉编库
libs=""
if [ "$TA" = "x86_64" ]; then
    libs="$RT/third_party/sqlite3/sqlite3.o"
    libs="$libs $RT/mbedtls/lib/libmbedtls.a $RT/mbedtls/lib/libmbedx509.a $RT/mbedtls/lib/libmbedcrypto.a"
    libs="$libs $RT/third_party/zlib/lib/libz.a"
    if [ -z "$NQ" ]; then
        libs="$libs $RT/third_party/ngtcp2/lib/libngtcp2.a $RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a"
        libs="$libs $RT/third_party/openssl/lib/libssl.a $RT/third_party/openssl/lib/libcrypto.a"
    fi
else
    libs="$RT/third_party/sqlite3/sqlite3-$TA.o $RT/mbedtls/lib-$TA/libmbedtls.a $RT/mbedtls/lib-$TA/libmbedx509.a $RT/mbedtls/lib-$TA/libmbedcrypto.a $RT/third_party/zlib/lib-$TA/libz.a"
    [ -n "$NQ" ] || libs="$libs $RT/third_party/ngtcp2/lib-$TA/libngtcp2.a"
fi
cnt=0
for p in $libs; do [ -f "$p" ] || { cnt=$((cnt+1)); say "   ⚠ 缺库：$p"; }; done
[ "$cnt" = 0 ] || { say "❌ 步骤 2 缺目标架构静态库（aarch64 仓库预置 lib-aarch64；armv7/riscv64 先跑 tools/cross_multiarch.sh）"; exit 1; }
$CC -O2 -pthread $LINK_EXTRA -o "$OUTDIR/pxc0" "$OUTDIR/compiler_golden.o" "$OUTDIR"/obj/*.o $libs -lm -ldl -lpthread > "$CPX_LOG" 2>&1 || { say "❌ 链接 pxc0 失败"; tail_log "$CPX_LOG"; exit 1; }
say "   ✅ $OUTDIR/pxc0（$(stat -c %s "$OUTDIR/pxc0") 字节）· $(file -b "$OUTDIR/pxc0" 2>/dev/null | cut -c1-60)"

# ---- 步骤 3：自证（pxc0 编译 compiler.px 与基准逐字节一致）—— 仅同架构可跑
if [ "$PROVE" = 1 ] && [ "$CROSS" = 0 ]; then
    say "── 步骤 3/4：自证（pxc0 编译 compiler.px ↔ golden/compiler.c）"
    norm() { grep -vE '^/\* 由普贤' "$1" | sed -E 's/[[:space:]]+$//'; }
    if ! timeout 1800 "$OUTDIR/pxc0" build "$SRC" > "$OUTDIR/B.c" 2> "$OUTDIR/B.err"; then
        say "❌ 自证失败：pxc0 编译 compiler.px 报错（**这正是第三方报告的 R1002 现场**）"
        tail_log "$OUTDIR/B.err"
        exit 1
    fi
    norm "$OUTDIR/B.c" > "$OUTDIR/B.norm"
    norm "$GOLDEN" > "$OUTDIR/G.norm"
    if cmp -s "$OUTDIR/B.norm" "$OUTDIR/G.norm"; then
        say "   ✅ 自证通过：与基准逐字节一致（$(wc -l < "$OUTDIR/B.norm") 行）"
    else
        say "❌ 自证失败：产物与基准不一致（首个差异行）"
        diff "$OUTDIR/G.norm" "$OUTDIR/B.norm" | head -8 | sed 's/^/     /' >&2
        exit 1
    fi
elif [ "$CROSS" = 1 ]; then
    say "── 步骤 3/4：自证**跳过**（交叉档 $TA 产物无法在本机执行；由 CI native-arm64 真机 job 自证）"
    "$CC" --version >/dev/null 2>&1 && say "   （已产出的 pxc0 可用 file/readelf 断言架构与静态性）"
fi

# ---- 步骤 4：交付
say "── 步骤 4/4：交付"
cp -f "$OUTDIR/pxc0" "$OUTDIR/pxc"
say "   ✅ 目标架构编译器：$OUTDIR/pxc"
say "   用法：$OUTDIR/pxc build <file.px>   （编译产物落 <源目录>/build/<name>）"
say "   注意：pxi/pxl/pxfmt/pxlint 等其余入库件仍可从源码现编："
say "         $OUTDIR/pxc build selfhost/interp.px（其余同理）"
if [ "$INSTALL" = 1 ]; then
    if [ "$CROSS" = 1 ]; then
        cp -f "$OUTDIR/pxc" "$ROOT/bootstrap/pxc-$TA"
        say "   ✅ 已装成 bootstrap/pxc-$TA（$TA 用；本机 $HOST_A 不受影响）"
    else
        say "   ⚠ 本机架构（$HOST_A）的入库件 bootstrap/pxc 由 selfhost/rebake_bin.sh 重烘（带出厂"
        say "     源码链指纹 + 行为对拍）—— 本脚本**不覆盖**它。若要手动顶上：cp $OUTDIR/pxc bootstrap/pxc"
    fi
fi
say "NATIVE-BOOTSTRAP-OK host=$HOST_A target=$TA cross=$CROSS no_quic=$([ -n "$NQ" ] && echo 1 || echo 0) static=$([ -n "$LINK_EXTRA" ] && echo 1 || echo 0)"
