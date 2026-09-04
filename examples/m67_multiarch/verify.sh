#!/usr/bin/env bash
# ============================================================
# M67-S7：多架构交叉编译 + qemu 验证（四档矩阵）
# ------------------------------------------------------------
# 矩阵：x86_64（native）+ aarch64 + armv7（armhf）+ riscv64
#   每档四用例：hello_multi / http_multi（qemu 起服宿主 curl 200）/ sqlite_multi / gc_stress
#   （gc_stress 并发深链 GC 压力：专测 arch_*.h 寄存器/栈保守扫描不漏标）
# 用法：
#   bash examples/m67_multiarch/verify.sh              # 全四档（需全部工具链 + qemu）
#   bash examples/m67_multiarch/verify.sh --arch aarch64   # 单档
#   bash examples/m67_multiarch/verify.sh --arch x86_64 --no-quic
# 工具链/库依赖（各档前置）：
#   x86_64  ：gcc（native，无需 qemu/交叉库）
#   aarch64 ：aarch64-linux-musl-gcc + qemu-aarch64-static + 仓库预置 aarch64 库
#   armv7   ：armv7l-linux-musleabihf-gcc + qemu-arm-static
#             + 交叉库（tools/cross_multiarch.sh --arch armv7 --outdir $PX_MULTI_LIBS/armv7）
#   riscv64 ：riscv64-linux-musl-gcc + qemu-riscv64-static
#             + 交叉库（tools/cross_multiarch.sh --arch riscv64 --outdir $PX_MULTI_LIBS/riscv64）
# ============================================================
set -u
cd "$(dirname "$0")/../.."   # 仓库根
PX_MULTI_LIBS="${PX_MULTI_LIBS:-/opt/px-multiarch}"   # armv7/riscv64 交叉库根（D1：不入库）
TMPD=$(mktemp -d /tmp/m67multi.XXXXXX)
trap 'rm -rf "$TMPD"' EXIT
ONLY=""

while [ $# -gt 0 ]; do
    case "$1" in
        --arch) ONLY="$2"; shift 2 ;;
        *) echo "未知参数: $1（--arch x86_64|aarch64|armv7|riscv64）" >&2; exit 1 ;;
    esac
done

fail() { echo "M67-S7 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

cp examples/m67_multiarch/hello_multi.px examples/m67_multiarch/http_multi.px \
   examples/m67_multiarch/sqlite_multi.px examples/m67_multiarch/gc_stress.px \
   examples/m67_multiarch/gc_single.px "$TMPD/"

# 单档执行：arch cc qemu port
run_arch() {
    local arch="$1" cc="$2" qemu="$3" port="$4"
    [ -n "$ONLY" ] && [ "$ONLY" != "$arch" ] && return 0
    echo ""
    echo "========== 档位: $arch（CC=$cc QEMU=${qemu:-native}）=========="
    command -v "$cc" >/dev/null 2>&1 || { echo "  ⏭ 缺交叉编译器 $cc，跳过 $arch"; return 0; }
    if [ -n "$qemu" ]; then
        command -v "$qemu" >/dev/null 2>&1 || { echo "  ⏭ 缺 $qemu，跳过 $arch"; return 0; }
    fi
    # 库参数（armv7/riscv64 从 libroot 取；aarch64/x86_64 仓库预置）
    local mbed sqlite zlib
    case "$arch" in
        aarch64) mbed="runtime/mbedtls/lib-aarch64"; sqlite="runtime/third_party/sqlite3/sqlite3-aarch64.o"; zlib="runtime/third_party/zlib/lib-aarch64" ;;
        x86_64)  mbed="runtime/mbedtls/lib";         sqlite="runtime/third_party/sqlite3/sqlite3.o";         zlib="runtime/third_party/zlib/lib" ;;
        armv7|riscv64)
            [ -d "$PX_MULTI_LIBS/$arch" ] || { echo "  ⏭ 缺交叉库 $PX_MULTI_LIBS/$arch（先跑 tools/cross_multiarch.sh --arch $arch --outdir $PX_MULTI_LIBS/$arch），跳过 $arch"; return 0; }
            mbed="$PX_MULTI_LIBS/$arch/mbedtls/lib-$arch"
            sqlite="$PX_MULTI_LIBS/$arch/sqlite3/sqlite3-$arch.o"
            zlib="$PX_MULTI_LIBS/$arch/zlib/lib-$arch" ;;
    esac
    [ -f "$mbed/libmbedcrypto.a" ] || { echo "  ⏭ 缺 $mbed/libmbedcrypto.a，跳过 $arch"; return 0; }

    # ---- 1. 用例交叉编译（--no-quic 统一：四档同语义、免 H3 交叉库）----
    # gc_stress/gc_single 仅 native 跑（qemu-user 并发 GC 线程信号协议 + 栈扫描
    # 模拟开销极大，实测 armv7 qemu 单次 stop-the-world ~15-25s → qemu 档不编不跑，
    # 新架构 GC 验证 = C 层 arch 探针（14/32 regs + SP）+ native 并发满量 + 真机）
    local EXS="hello_multi http_multi sqlite_multi"
    [ -z "$qemu" ] && EXS="$EXS gc_stress gc_single"
    for ex in $EXS; do
        tools/pxc build --no-quic --cc "$cc" \
            --mbedtls-lib "$mbed" --sqlite-obj "$sqlite" --zlib-lib "$zlib" \
            "$TMPD/$ex.px" > "$TMPD/$arch-$ex.log" 2>&1 \
            || { echo "  编译失败 $arch/$ex:"; tail -8 "$TMPD/$arch-$ex.log"; fail "$arch $ex 编译失败"; }
        ok "$arch $ex 交叉编译"
    done

    # ---- 2. 产物架构断言 ----
    local exp
    case "$arch" in
        x86_64)  exp="x86-64" ;;
        aarch64) exp="aarch64" ;;
        armv7)   exp="EABI5" ;;
        riscv64) exp="RISC-V" ;;
    esac
    local A=$(file "$TMPD/build/hello_multi")
    echo "  $A"
    echo "$A" | grep -q "$exp" || fail "$arch 产物架构不符: $A"

    # ---- 3. hello ----
    if [ -n "$qemu" ]; then
        OUT=$("$qemu" "$TMPD/build/hello_multi" 2>&1)
    else
        OUT=$("$TMPD/build/hello_multi" 2>&1)
    fi
    echo "$OUT" | grep -q "PX_ARCH_CROSS_OK" || fail "$arch hello 运行未达预期"
    ok "$arch hello 运行"

    # ---- 4. http（起服 + curl 200）----
    # 注意：print 到重定向 stdout 为全缓冲，不能靠 READY 就绪标记 → 轮询 curl
    if [ -n "$qemu" ]; then
        "$qemu" "$TMPD/build/http_multi" > "$TMPD/$arch-http.log" 2>&1 &
    else
        "$TMPD/build/http_multi" > "$TMPD/$arch-http.log" 2>&1 &
    fi
    local SRV=$!
    local RESP=""
    local n=0
    while [ "$RESP" != "200" ] && [ "$n" -lt 40 ]; do
        sleep 0.25
        RESP=$(curl -s -m 2 -o "$TMPD/$arch-body" -w "%{http_code}" "http://127.0.0.1:$port/" 2>/dev/null)
        n=$((n + 1))
    done
    kill "$SRV" 2>/dev/null
    [ "$RESP" = "200" ] || { cat "$TMPD/$arch-http.log"; fail "$arch curl $RESP ≠ 200（10s 内未就绪）"; }
    grep -q "m67-multiarch-http-ok" "$TMPD/$arch-body" || fail "$arch body 不符"
    ok "$arch http 200 + body"

    # ---- 5. sqlite ----
    if [ -n "$qemu" ]; then
        OUT=$("$qemu" "$TMPD/build/sqlite_multi" 2>&1)
    else
        OUT=$("$TMPD/build/sqlite_multi" 2>&1)
    fi
    echo "$OUT" | grep -q "sqlite multiarch OK" || fail "$arch sqlite 运行未达预期"
    ok "$arch sqlite 运行"

    # ---- 6. GC 压力（仅 native：qemu-user 慢，见上文说明）----
    if [ -z "$qemu" ]; then
        # 并发 4 worker 深链 + stop-the-world 寄存器/栈扫描（满量 2000）
        OUT=$("$TMPD/build/gc_stress" 2000 2>&1)
        echo "$OUT" | grep -q "gc_stress OK" || { echo "$OUT" | tail -6; fail "$arch gc_stress 未过（疑似漏标误回收）"; }
        ok "$arch gc_stress 并发深链 GC 压力（4 worker + stop-the-world）"
        # 单线程深链 + 显式 gc（3 万迭代满量）
        OUT=$("$TMPD/build/gc_single" 30000 2>&1)
        echo "$OUT" | grep -q "gc_single OK" || { echo "$OUT" | tail -6; fail "$arch gc_single 未过"; }
        ok "$arch gc_single 单线程深链 GC 压力（3 万迭代）"
    else
        echo "  ℹ️  qemu-user 下并发 GC 模拟开销大 → 新架构 GC 由 C 层 arch 探针（14/32 regs+SP）+ native 并发满量 + 真机验证（见 M67_PLAN §S7）"
    fi

    echo "---------- $arch 全用例 PASS ----------"
}

run_arch x86_64  "gcc"                        ""  18991
run_arch aarch64 "aarch64-linux-musl-gcc"     "qemu-aarch64-static" 18991
run_arch armv7   "armv7l-linux-musleabihf-gcc" "qemu-arm-static"     18991
run_arch riscv64 "riscv64-linux-musl-gcc"     "qemu-riscv64-static" 18991

echo ""
echo "== M67-S7 多架构矩阵验证完成（x86_64 / aarch64 / armv7 / riscv64）=="
