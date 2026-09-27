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
# ⚠️ M219（缺陷 311）：**C 轨与 VM 轨都必须 `-static`**。本仓预置三方资产
#   （`sqlite3.o` / `libz.a` / `mbedtls/*.a`）是**非 PIC** 对象，在「默认 PIE」工具链
#   （Ubuntu / Debian gcc 的 `--enable-default-pie`）上会被 ld 拒绝：
#     relocation R_X86_64_32[S] against `.rodata' can not be used when making a PIE object
#   而 Red Hat 系 gcc 默认非 PIE ⇒ **开发机恒绿、CI 必红**（实测 CI run 36262322866）。
#   守卫：`selfhost/check_link_flags.sh`（静态扫全仓）；本机复现垫片：`selfhost/sim_pie_cc.sh`。
# ⚠️ M221（第 100 轮）：**产物指纹短路** —— 同一源码链下重复调用 ≈ 秒返回（打印 `⏭`）。
#   为什么：全量门 131 门里有 **23 门**各自 `devbuild.sh pxc pxi --vm`，实测单次（暖缓存）
#   ≈45s ⇒ **23×45 ≈ 17 分钟纯重复**，占 CI「工具自测」步（实测 23/40+/37min，上限 45min）
#   的 40–75%。而 23 门在 CI 里同一步、同一源码链、同一 .rtcache ⇒ 产物逐字节相同。
#   ⇒ 判据：产物存在 + 指纹 == 当前源码链指纹 ⇒ 复用。
#   ⚠️ 指纹**必须**含 `runtime/*.c,h` —— 门内负控会篡改它们（见 m216 门头注），
#      若只按「产物比源码新」判，负控的**重建**会被吃掉 ⇒ **假绿**（M169 老坑）。
#   ⚠️ 构建**失败**绝不写指纹；`--rebuild`（或 `DEVB_REBUILD=1`）⇒ 无条件重建。
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
    local name="$1" src out fp
    src=$(entry_src "$name")
    [ -n "$src" ] || { echo "❌ 未知件：$name（可用：$(echo "$ENTRIES" | cut -d'|' -f1 | tr '\n' ' '))" >&2; return 1; }
    out="/tmp/${name}dev"; fp="/tmp/devbuild_${name}.fp"
    if [ "$FORCE_REBUILD" = 0 ] && [ -f "$out" ] && [ -f "$fp" ] && [ "$(cat "$fp" 2>/dev/null)" = "$DEVB_KEY" ]; then
        echo "⏭  $name：源码链未变（key=$DEVB_KEY）⇒ 复用 $out"
        return 0
    fi
    timeout 900 "$BASE" build "$ROOT/$src" > "/tmp/devbuild_$name.c" 2>"/tmp/devbuild_$name.err" || {
        echo "❌ $name：pxc build $src 失败"; tail -5 "/tmp/devbuild_$name.err" >&2; return 1; }
    gcc -c -O2 -I"$CACHE" -I"$RT" "/tmp/devbuild_$name.c" -o "/tmp/devbuild_$name.o" 2>"/tmp/devbuild_$name.cc.log" || {
        echo "❌ $name：C 编译失败"; tail -10 "/tmp/devbuild_$name.cc.log" >&2; return 1; }
    gcc -static -O2 -pthread -o "$out" "/tmp/devbuild_$name.o" $objs $LIBS 2>"/tmp/devbuild_$name.link.log" || {
        echo "❌ $name：链接失败"; tail -10 "/tmp/devbuild_$name.link.log" >&2; return 1; }
    echo "$DEVB_KEY" > "$fp"
    echo "✅ $name → $out（$(stat -c %s "$out") 字节）"
}

WANT_VM=0; NAMES=""; FORCE_REBUILD=0
[ "${DEVB_REBUILD:-0}" = "1" ] && FORCE_REBUILD=1
for a in "$@"; do
    case "$a" in
        --vm) WANT_VM=1 ;;
        --rebuild) FORCE_REBUILD=1 ;;
        *) NAMES="$NAMES $a" ;;
    esac
done
[ -n "$NAMES" ] || NAMES="pxc"

# ── M221：源码链指纹（见文件头注）────────────────────────────────
#   纳入：selfhost/tools/stdlib 的 .px + runtime 顶层 .c/.h + 预置三方资产
#         + 入库 pxc 的 sha + .rtcache 目录名。
#   ⚠️ runtime/*.c,h 必须在内（门内负控会改它们）；入库 pxc 在内（自举起点变了产物就变）。
src_line() {
    for f in selfhost/*.px runtime/*.c runtime/*.h tools/*.px stdlib/*.px; do
        [ -f "$f" ] && stat -c '%n %s %Y' "$f"
    done
    for f in "$RT"/third_party/sqlite3/sqlite3.o "$RT"/mbedtls/lib/*.a "$RT"/third_party/*/lib/*.a; do
        [ -f "$f" ] && stat -c '%n %s %Y' "$f"
    done
    sha256sum "$BASE" 2>/dev/null | cut -c1-16
    echo "rtcache=$(basename "$CACHE")"
}
DEVB_KEY="$(src_line | sha256sum | cut -c1-16)"
echo "── 源码链指纹：$DEVB_KEY$([ "$FORCE_REBUILD" = 1 ] && echo "（--rebuild：忽略缓存）")"
echo "── rtcache: ${CACHE#$ROOT/}"
for n in $NAMES; do build_one "$n" || exit 1; done

if [ "$WANT_VM" = "1" ]; then
    # ⚠️ VM 轨的**输入含 /tmp/pxcdev 本身** ⇒ 指纹要把它的 sha 也算进来：
    #    `devbuild pxi --vm`（不带 pxc）时 pxcdev 可能是**别的**源码链的产物，
    #    拿它 emit-c 会得到与实际源码不符的 VM 件（m216 门头注记过这个坑）。
    VM_KEY="$DEVB_KEY/$(sha256sum /tmp/pxcdev 2>/dev/null | cut -c1-16)"
    if [ "$FORCE_REBUILD" = 0 ] && [ -f /tmp/pxcdev_vm ] && [ -f /tmp/devbuild_vm.fp ] && [ "$(cat /tmp/devbuild_vm.fp 2>/dev/null)" = "$VM_KEY" ]; then
        echo "⏭  VM 轨：源码链未变（key=$VM_KEY）⇒ 复用 /tmp/pxcdev_vm"
    else
    echo "── VM 轨：--emit-c → /tmp/pxcdev_vm"
    timeout 1500 /tmp/pxcdev --emit-c "$ROOT/selfhost/compiler.px" > /tmp/devbuild_vm.c 2>/tmp/devbuild_vm.err || {
        echo "❌ --emit-c 失败"; tail -5 /tmp/devbuild_vm.err >&2; exit 1; }
    gcc -c -O2 -I"$CACHE" -I"$RT" /tmp/devbuild_vm.c -o /tmp/devbuild_vm.o 2>/tmp/devbuild_vm.cc.log || {
        echo "❌ VM 镜像编译失败"; tail -10 /tmp/devbuild_vm.cc.log >&2; exit 1; }
    gcc -static -O2 -pthread -o /tmp/pxcdev_vm /tmp/devbuild_vm.o $objs $LIBS 2>/tmp/devbuild_vm.link.log || {
        echo "❌ VM 轨链接失败"; tail -10 /tmp/devbuild_vm.link.log >&2; exit 1; }
    echo "✅ VM 轨：/tmp/pxcdev_vm（$(stat -c %s /tmp/pxcdev_vm) 字节）"
    echo "$VM_KEY" > /tmp/devbuild_vm.fp
    fi
fi
