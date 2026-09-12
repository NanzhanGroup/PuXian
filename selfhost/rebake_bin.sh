#!/usr/bin/env bash
# ============================================================
# 入库二进制重烘（rebake shipped bootstrap binaries）—— M113-S0 / Issue 58
# ------------------------------------------------------------
# 背景（Issue 58）：改了 `selfhost/*.px` 却忘了重烘入库编译器，是**结构性失明** ——
#   `bootstrap_prove*.sh` 的 golden 由**同一枚 pxc** 重定基，「老 pxc + 自己的 golden」
#   永远自洽 ⇒ 门绿 ≠ 用户拿到修复。而当时全仓**没有任何可执行的 C 轨重烘指引**
#   （`CONTRIBUTING` 第 5 步只写了 `cp compiler_vm bootstrap/pxc_vm`，C 轨
#   `bootstrap/pxc` 无人重烘 ⇒ 自 M112 起落后源码，见 Issue 58 实测）。
#
# 本脚本把该步骤变成可执行，并且**按入库口径**链：
#   bootstrap/pxc     = compiler.px 的 C 镜像 → gcc **-static** + **全 runtime 对象**
#                       （与 bootstrap/pxi / pxfmt 等同为静态 ELF）
#   bootstrap/pxc_vm  = compiler.px 的 VM 字节码镜像 → dev 缓存产物（动态，2.6MB 档）
#
# 前置：bootstrap/pxc 已是可用的当前源码编译器；`.rtcache` 里有**全 runtime** 对象缓存
#       （若没有：`./tools/px build --full examples/hello.px` 生成）
# 用法：
#   ./selfhost/rebake_bin.sh            # 重烘两个入库件 + 打印指纹
#   ./selfhost/rebake_bin.sh --check    # 只检查「入库件是否与当前源码产物一致」
# ============================================================
set -u
cd "$(dirname "$0")/.."
ROOT=$(pwd)
CHECK=0
[ "${1:-}" = "--check" ] && CHECK=1

PXC="$ROOT/bootstrap/pxc"
BUILD="$ROOT/selfhost/build"
RT="$ROOT/runtime"
CK="$ROOT/selfhost/compiler.px"

# ---- 选「对象最多」的缓存 = 全 runtime 档（自动裁剪档对象少，链出来的 pxc 能力不全）----
CACHE=""
best=0
for d in "$ROOT"/.rtcache/*/; do
    [ -d "$d" ] || continue
    [ -f "$d/.complete" ] || continue
    n=$(ls "$d"/*.o 2>/dev/null | wc -l)
    if [ "$n" -gt "$best" ]; then best=$n; CACHE="$d"; fi
done
[ -n "$CACHE" ] || {
    echo "── 无 .rtcache，先用 tools/px 生成【全 runtime】缓存（--full）"
    "$ROOT/tools/px" build --full "$ROOT/examples/hello.px" >/dev/null 2>&1 || true
    best=0
    for d in "$ROOT"/.rtcache/*/; do
        [ -d "$d" ] || continue
        [ -f "$d/.complete" ] || continue
        n=$(ls "$d"/*.o 2>/dev/null | wc -l)
        if [ "$n" -gt "$best" ]; then best=$n; CACHE="$d"; fi
    done
}
[ -n "$CACHE" ] || {
    echo "❌ 未找到可用 .rtcache（先跑：./tools/px build --full examples/hello.px）" >&2
    exit 1
}
# 缓存完整度：全量档 = runtime 源集合全编（本机 23 个 .o）；裁剪档 = 自动裁剪（本机 7 个）。
#   该数字由 runtime 源决定、跨机器稳定，故用阈值判档。
#   M113-S1：**重烘必须用全量档** —— 用裁剪档链出来的 bootstrap/pxc 会缺 runtime 能力
#   （S0 时正是这么误装过一枚 2.81MB 的"动态+裁剪"件，属必须挡住的坑）。
#   --check 则容忍裁剪档：它只比对「同一份源码在两枚编译器下的 C 产物」，与链进多少 runtime 无关。
FULL_MIN=15
if [ "$best" -ge "$FULL_MIN" ]; then
    echo "── runtime 缓存：$CACHE（${best} 个 .o = 全量档）"
else
    echo "── runtime 缓存：$CACHE（${best} 个 .o = 裁剪档）"
    if [ "$CHECK" != "1" ]; then
        echo "❌ 只有裁剪档缓存，重烘会把入库 pxc 链成 runtime 能力不全的件。" >&2
        echo "   先生成全量缓存：./tools/px build --full examples/hello.px（或清掉 .rtcache 让本脚本自动生成）" >&2
        exit 1
    fi
fi

src_fingerprint() {
    # 源码链指纹（口径与 bootstrap_prove_bc.sh 的 SRC_CHAIN 一致 + 运行头）
    local files="compiler.px codegen.px parser.px pxlexer.px cg_stmt.px cg_expr.px cg_module.px bc_emit.px"
    ( cd "$ROOT/selfhost" && cat $files 2>/dev/null ) | sha256sum | cut -c1-16
}
FP=$(src_fingerprint)
echo "── 源码链指纹：$FP"

link_c_track() {   # $1 = compiler_new.c  $2 = 输出
    local cfile="$1" out="$2"
    gcc -c -O2 -I"$CACHE" -I"$RT" "$cfile" -o /tmp/rebake_cn.o 2>/tmp/rebake_cc.log || {
        echo "❌ compiler_new.c 编译失败" >&2; tail -10 /tmp/rebake_cc.log >&2; return 1; }
    local objs="" f
    for f in "$CACHE"*.o; do objs="$objs $f"; done
    gcc -static -O2 -pthread -o "$out" /tmp/rebake_cn.o $objs \
        "$RT/third_party/sqlite3/sqlite3.o" \
        "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
        "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
        "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
        "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread 2>/tmp/rebake_link.log || {
        echo "❌ 链接失败：$out" >&2; tail -10 /tmp/rebake_link.log >&2; return 1; }
    return 0
}

if [ "$CHECK" = "1" ]; then
    # 「是否与当前源码产物一致」= 行为判据而非 md5（跨机器 gcc 不可复现）：
    #   用入库 pxc 与「现编 pxc」对同一组探针编译，rc / stdout / stderr 必须逐字节一致。
    #
    # M113-S1：探针集合从「cases_ok 正例 + codegen 负例」（S0 时仅 10 例）扩到**全套用例**
    #   （cases/ + cases_bad/ + cases_ok/，跳过 import 夹具 modstate.px）。
    #   旧集合过窄：源码改动落在集合之外就漏判（假绿），而本门的存在意义正是"别漏"。
    #   成本实测可忽略：55 例 × 2 枚编译器 ≈ 1.5s（只编到 C，不过 gcc）。
    echo "── [--check] 入库 pxc vs 现编 pxc 行为对拍"
    mkdir -p "$BUILD"
    timeout 900 "$PXC" build "$CK" > "$BUILD/rebake_new.c" 2>/tmp/rebake_build.log || {
        echo "❌ 入库 pxc 无法编译当前源码（已失效）" >&2; tail -5 /tmp/rebake_build.log >&2; exit 1; }
    link_c_track "$BUILD/rebake_new.c" "$BUILD/rebake_new" || exit 1
    bad=0; cnt=0
    for f in "$ROOT"/selfhost/cases/*.px "$ROOT"/selfhost/cases_bad/*.px "$ROOT"/selfhost/cases_ok/*.px; do
        [ -e "$f" ] || continue
        case "$(basename "$f")" in modstate.px) continue;; esac   # M70-S3 夹具，非用例
        cnt=$((cnt+1))
        "$PXC" build "$f" > /tmp/rb_shipped.out 2>/tmp/rb_shipped.err; s=$?
        "$BUILD/rebake_new" build "$f" > /tmp/rb_fresh.out 2>/tmp/rb_fresh.err; t=$?
        if [ "$s" != "$t" ] || ! cmp -s /tmp/rb_shipped.out /tmp/rb_fresh.out \
           || ! cmp -s /tmp/rb_shipped.err /tmp/rb_fresh.err; then
            echo "    ❌ $(basename "$f")：入库 pxc 与现编 pxc 行为不同（rc $s vs $t）"
            bad=$((bad+1))
        fi
    done
    if [ "$bad" -gt 0 ]; then
        echo "❌ 入库 bootstrap/pxc 与当前源码不一致（$cnt 例中 $bad 例不同）"
        echo "   ⇒ 重烘：./selfhost/rebake_bin.sh   （改过 selfhost/*.px 就必须重烘，否则用户拿到的是旧语义）"
        exit 1
    fi
    echo "✅ 入库 bootstrap/pxc 与当前源码产物行为一致（$cnt 例探针）"
    exit 0
fi

# ---- 重烘 ----
echo "── 步骤 1/3：入库 pxc 编译 compiler.px → compiler_new.c"
timeout 900 "$PXC" build "$CK" > "$BUILD/compiler_new.c" 2>/tmp/rebake_build.log || {
    echo "❌ pxc build compiler.px 失败" >&2; tail -5 /tmp/rebake_build.log >&2; exit 1; }
echo "── 步骤 2/3：gcc -static 链 bootstrap/pxc（全 runtime）"
cp -a "$PXC" "/tmp/pxc.bak-$(date +%Y%m%d-%H%M%S)"   # 备份到 /tmp，不污染仓库
link_c_track "$BUILD/compiler_new.c" "$PXC" || exit 1
echo "── 步骤 3/3：重烘 bootstrap/pxc_vm（VM 字节码镜像）"
link_c_track "$BUILD/compiler_new.c" "$BUILD/compiler_new" || exit 1
if [ ! -s "$BUILD/compiler_vm.c" ] || [ "$BUILD/compiler_vm.c" -ot "$PXC" ]; then
    timeout 1500 "$BUILD/compiler_new" --emit-c "$CK" > "$BUILD/compiler_vm.c" 2>/tmp/rebake_emitc.log || {
        echo "❌ compiler_new --emit-c compiler.px 失败" >&2; tail -5 /tmp/rebake_emitc.log >&2; exit 1; }
fi
gcc -c -O2 -I"$CACHE" -I"$RT" "$BUILD/compiler_vm.c" -o /tmp/rebake_vm.o 2>/tmp/rebake_vmcc.log || {
    echo "❌ compiler_vm.c 编译失败" >&2; tail -10 /tmp/rebake_vmcc.log >&2; exit 1; }
objs=""; for f in "$CACHE"*.o; do objs="$objs $f"; done
gcc -O2 -pthread -o "$BUILD/compiler_vm" /tmp/rebake_vm.o $objs \
    "$RT/third_party/sqlite3/sqlite3.o" \
    "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
    "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
    "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
    "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread 2>/tmp/rebake_vmlink.log || {
    echo "❌ compiler_vm 链接失败" >&2; tail -10 /tmp/rebake_vmlink.log >&2; exit 1; }
cp -f "$BUILD/compiler_vm" "$ROOT/bootstrap/pxc_vm"

echo "── 指纹（源码链 $FP）"
file "$ROOT/bootstrap/pxc" "$ROOT/bootstrap/pxc_vm" | sed 's/^/    /'
echo "    pxc    sha256 $(sha256sum "$ROOT/bootstrap/pxc" | cut -c1-16)"
echo "    pxc_vm sha256 $(sha256sum "$ROOT/bootstrap/pxc_vm" | cut -c1-16)"
echo "✅ 重烘完成 —— 接下来必须跑：./selfhost/engine_parity.sh && ./selfhost/diffcheck.sh --errors"
