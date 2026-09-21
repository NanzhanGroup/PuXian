#!/usr/bin/env bash
# ============================================================
# devbuild.sh —— **开发期**快速自举构建（不碰入库件）
#   M116 建立（qg-issue 71 续）：改 `selfhost/*.px` 时需要一个"只用当前源码 +
#   现役入库 pxc"就能得到**新版工具**的回路，且**不覆盖** bootstrap/*（入库件只能
#   由 rebake_bin.sh 带指纹重烘，见 Issue 55/58）。
# 用法：
#   ./selfhost/devbuild.sh                 # 只构建 /tmp/pxcdev（C 轨编译器）
#   ./selfhost/devbuild.sh pxi pxl pxpar   # 额外构建 /tmp/pxi_dev 等
#   ./selfhost/devbuild.sh --vm            # 再生成 /tmp/pxcdev_vm（VM 轨，用户面默认）
# 产物：/tmp/pxcdev · /tmp/<name>_dev（+ 日志 /tmp/devbuild_*.log）
# 备注：单件 C 轨 ≈19s；--vm 追加 ≈90s（--emit-c + 链 VM 镜像）
# ============================================================
set -u
cd "$(dirname "$0")/.."
ROOT=$(pwd)
export LC_ALL=C LANG=C
RT="$ROOT/runtime"
BASE="$ROOT/bootstrap/pxc"

ENTRIES="pxc|selfhost/compiler.px|static
pxi|selfhost/interp.px|static
pxl|selfhost/lexer.px|static
pxpar|selfhost/parser.px|static
pxfmt|tools/pxfmt.px|static
pxlint|tools/pxlint.px|static"

entry_src() { echo "$ENTRIES" | grep "^$1|" | cut -d'|' -f2; }

# 选 .rtcache（全 runtime 对象）
#   M169 修（本地实测踩中）：**按 rt_key 命中优先 + 主机架构过滤**。
#   修前只按 `runtime.o` 的 mtime 取「最新」——而 M168 起交叉编译缓存（aarch64/armv7/
#   riscv64）会落进**同一个** `.rtcache/`：最新那份可能是**别的架构**的对象
#   ⇒ 链接报 `Relocations in generic ELF (EM: 183)` / `file in wrong format`，
#   错误信息完全指不到根因（本机实测：aarch64 档把 pxc 的 devbuild 全数打死）。
#   纪律同 M168「门/工具不能依赖环境」：选料必须**可判定**，不能靠「谁最新」。
CACHE=""; best_m=0
keyed="$("$ROOT/tools/px" rtcache 2>/dev/null | tail -1)"
if [ -n "$keyed" ] && [ -d "$keyed" ]; then
    n=$(ls "$keyed"/*.o 2>/dev/null | wc -l)
    [ "$n" -ge 15 ] && CACHE="${keyed%/}"
fi
if [ -z "$CACHE" ]; then
    case "$(uname -m)" in
        x86_64)  WANT="x86-64" ;;
        aarch64) WANT="ARM aarch64" ;;
        armv7l)  WANT="ARM," ;;
        riscv64) WANT="RISC-V" ;;
        *)       WANT="" ;;
    esac
    for d in "$ROOT"/.rtcache/*/; do
        n=$(basename "$d"); [ ${#n} -eq 16 ] || continue
        [ -f "$d/runtime.o" ] || continue
        if [ -n "$WANT" ]; then
            file -b "$d/runtime.o" 2>/dev/null | grep -qF "$WANT" || continue
        fi
        m=$(stat -c %Y "$d/runtime.o" 2>/dev/null || echo 0)
        if [ "$m" -gt "$best_m" ]; then best_m="$m"; CACHE="${d%/}"; fi
    done
fi
[ -n "$CACHE" ] || { echo "❌ 无可用 .rtcache（先跑 ./tools/px build --full examples/hello.px）" >&2; exit 1; }
echo "── devbuild 选料：${CACHE#$ROOT/}（$(file -b "$CACHE/runtime.o" 2>/dev/null | cut -d, -f1-2)）"
objs=""; for f in "$CACHE"/*.o; do objs="$objs $f"; done
LIBS="$RT/third_party/sqlite3/sqlite3.o
$RT/mbedtls/lib/libmbedtls.a $RT/mbedtls/lib/libmbedx509.a $RT/mbedtls/lib/libmbedcrypto.a
$RT/third_party/ngtcp2/lib/libngtcp2.a $RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a
$RT/third_party/openssl/lib/libssl.a $RT/third_party/openssl/lib/libcrypto.a
$RT/third_party/zlib/lib/libz.a -lm -ldl -lpthread"

build_one() {   # $1=件名 → /tmp/${1}dev
    local name="$1" src out
    src=$(entry_src "$name")
    [ -n "$src" ] || { echo "❌ 未知件：$name（可用：$(echo "$ENTRIES" | cut -d'|' -f1 | tr '\n' ' '))" >&2; return 1; }
    out="/tmp/${name}dev"
    timeout 900 "$BASE" build "$ROOT/$src" > "/tmp/devbuild_$name.c" 2>"/tmp/devbuild_$name.err" || {
        echo "❌ $name：pxc build $src 失败"; tail -5 "/tmp/devbuild_$name.err" >&2; return 1; }
    gcc -c -O2 -I"$CACHE" -I"$RT" "/tmp/devbuild_$name.c" -o "/tmp/devbuild_$name.o" 2>"/tmp/devbuild_$name.cc.log" || {
        echo "❌ $name：C 编译失败"; tail -10 "/tmp/devbuild_$name.cc.log" >&2; return 1; }
    gcc -static -O2 -pthread -o "$out" "/tmp/devbuild_$name.o" $objs $LIBS 2>"/tmp/devbuild_$name.link.log" || {
        echo "❌ $name：链接失败"; tail -10 "/tmp/devbuild_$name.link.log" >&2; return 1; }
    echo "✅ $name → $out（$(stat -c %s "$out") 字节）"
}

WANT_VM=0; NAMES=""
for a in "$@"; do
    case "$a" in
        --vm) WANT_VM=1 ;;
        *) NAMES="$NAMES $a" ;;
    esac
done
[ -n "$NAMES" ] || NAMES="pxc"
echo "── rtcache: ${CACHE#$ROOT/}"
for n in $NAMES; do build_one "$n" || exit 1; done

if [ "$WANT_VM" = "1" ]; then
    echo "── VM 轨：--emit-c → /tmp/pxcdev_vm"
    timeout 1500 /tmp/pxcdev --emit-c "$ROOT/selfhost/compiler.px" > /tmp/devbuild_vm.c 2>/tmp/devbuild_vm.err || {
        echo "❌ --emit-c 失败"; tail -5 /tmp/devbuild_vm.err >&2; exit 1; }
    gcc -c -O2 -I"$CACHE" -I"$RT" /tmp/devbuild_vm.c -o /tmp/devbuild_vm.o 2>/tmp/devbuild_vm.cc.log || {
        echo "❌ VM 镜像编译失败"; tail -10 /tmp/devbuild_vm.cc.log >&2; exit 1; }
    gcc -O2 -pthread -o /tmp/pxcdev_vm /tmp/devbuild_vm.o $objs $LIBS 2>/tmp/devbuild_vm.link.log || {
        echo "❌ VM 轨链接失败"; tail -10 /tmp/devbuild_vm.link.log >&2; exit 1; }
    echo "✅ VM 轨：/tmp/pxcdev_vm（$(stat -c %s /tmp/pxcdev_vm) 字节）"
fi
