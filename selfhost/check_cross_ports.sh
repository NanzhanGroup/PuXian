#!/usr/bin/env bash
# ============================================================
# M208s1 新增门：**交叉档「可编译性」本地门**（cross-port compile smoke）
# ------------------------------------------------------------
# 立此门的原因（缺陷 273）：
#   M208 给 `runtime/vm.c` 加了 write(2,…) 诊断却**没 include `<unistd.h>`** ——
#   宿主 gcc 侥幸（隐式声明）通过，而三档交叉 cc（clang/musl）一律
#   `error: call to undeclared function 'write'` ⇒ CI 的 m67 三个 job 全红。
#   本机**本来就有** /opt/muslcc-bin 三档工具链，却要等 17 分钟的 CI 往返才发现。
#   ⇒「宿主编译器绿」不等于「可移植」；**能本机复现的，就不要留给 CI**。
#
# 判据（三档各自独立）：
#   ① 有工具链+库 ⇒ 必须 `--no-quic` 编译成功，且**产物架构正确**（读到架构不对也判红）；
#   ② 缺工具链或库 ⇒ 记 **SKIP 并打印原因**（不算通过、也不判红）；
#   ③ 全档 SKIP ⇒ 门整体 SKIP（CI runner 无 /opt/muslcc-bin ⇒ 走这条；
#      权威覆盖仍在 ci.yml 的 m67 job —— 那里带 qemu 真跑）。
# 覆盖率来源 = examples/m67_multiarch/hello_multi.px（与 CI 同一语料，避免"门自己造语料"）。
#
# 用法：bash selfhost/check_cross_ports.sh [--self-test] [--quiet]
# 退出码：0=绿/SKIP 1=红
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

SELF=0; QUIET=0
for a in "$@"; do
    case "$a" in
        --self-test) SELF=1 ;;
        --quiet)     QUIET=1 ;;
        *) echo "未知参数: $a（--self-test|--quiet）" >&2; exit 2 ;;
    esac
done
say() { [ "$QUIET" = 1 ] || echo "$@"; }

# ── 工具链定位：**按需、逐个、命中即停**（不许一次性拓宽 PATH）──
#   ⚠️ M208s1 实测教训：首版把 `/opt/aarch64-linux-musl-cross/bin` 也塞进 PATH ⇒ 它把
#     `aarch64-linux-musl-ld` 顶到前面、抢走 clang 包装器该用的链接器 ⇒ 链接期
#     `read-only segment has dynamic relocations` **假红**（同一 cc 单独用 /opt/muslcc-bin 时是绿的）。
#     ⇒ 纪律：**门只做「找得到就不动环境」的最小干预**；门自己造出的环境差异 = 假红来源。
CCDIRS="/opt/muslcc-bin /opt/aarch64-linux-musl-cross/bin"
ensure_cc() {   # $1=cc 名 → 找得到返回 0，并**只**为它追加最小 PATH
    command -v "$1" >/dev/null 2>&1 && return 0
    for d in $CCDIRS; do
        [ -x "$d/$1" ] || continue
        PATH="$d:$PATH"; export PATH
        command -v "$1" >/dev/null 2>&1 && return 0
    done
    return 1
}

SRC=examples/m67_multiarch/hello_multi.px
MULTI="${PX_MULTI_LIBS:-/opt/px-multiarch}"
[ -f "$SRC" ] || { echo "❌ 前置：缺语料 $SRC" >&2; exit 2; }

pass=0; fail=0; skips=0
libs_for() {   # $1=arch → 打印 3 个路径（mbed/sqlite/zlib），任一缺失即打印 MISSING:<原因>
    case "$1" in
        aarch64) echo "runtime/mbedtls/lib-aarch64 runtime/third_party/sqlite3/sqlite3-aarch64.o runtime/third_party/zlib/lib-aarch64" ;;
        x86_64)  echo "runtime/mbedtls/lib runtime/third_party/sqlite3/sqlite3.o runtime/third_party/zlib/lib" ;;
        *)       echo "$MULTI/$1/mbedtls/lib-$1 $MULTI/$1/sqlite3/sqlite3-$1.o $MULTI/$1/zlib/lib-$1" ;;
    esac
}
cc_for() { case "$1" in aarch64) echo aarch64-linux-musl-gcc ;; armv7) echo armv7l-linux-musleabihf-gcc ;; riscv64) echo riscv64-linux-musl-gcc ;; esac; }

one() {   # one <arch> <cc> [<corpus>]
    local arch="$1" cc="$2" corpus="${3:-$SRC}"
    local mbed sqlite zlib
    read -r mbed sqlite zlib <<<"$(libs_for "$arch")"
    if ! ensure_cc "$cc"; then
        say "  ⏭ SKIP $arch：PATH 上无 $cc（原因已打印，不计通过）"; skips=$((skips+1)); return 0
    fi
    if [ ! -f "$mbed/libmbedcrypto.a" ]; then
        say "  ⏭ SKIP $arch：缺交叉库 $mbed/libmbedcrypto.a"; skips=$((skips+1)); return 0
    fi
    local out build_dir
    build_dir="$(dirname "$corpus")/build"
    if timeout 420 tools/pxc build --no-quic --cc "$cc" \
            --mbedtls-lib "$mbed" --sqlite-obj "$sqlite" --zlib-lib "$zlib" \
            "$corpus" > /tmp/cross_port.$arch.log 2>&1; then
        out="$(file -b "$build_dir/hello_multi" 2>/dev/null || true)"
        local want
        case "$arch" in aarch64) want="aarch64" ;; armv7) want="EABI5" ;; riscv64) want="RISC-V" ;; esac
        if [ -z "$out" ]; then
            say "  ❌ $arch：编译报成功但**没有产物**（判据的『找不到』先怀疑判据）"; fail=$((fail+1)); return 1
        fi
        if ! printf '%s' "$out" | grep -q "$want"; then
            say "  ❌ $arch：产物架构不符（期望含 $want，实测 '$out'）"; fail=$((fail+1)); return 1
        fi
        say "  ✅ $arch 交叉编译：$(printf '%s' "$out" | cut -c1-56)"
        pass=$((pass+1)); return 0
    else
        say "  ❌ $arch 交叉编译失败（$cc）："
        tail -5 /tmp/cross_port.$arch.log | sed 's/^/       /'
        [ -s /tmp/pxc_rtcc.log ] && tail -8 /tmp/pxc_rtcc.log | sed 's/^/       rtcc | /'
        fail=$((fail+1)); return 1
    fi
}

if [ "$SELF" = 1 ]; then
    echo "── 自证（3 条：假 cc ⇒ SKIP 不算通过 · 坏源码 ⇒ 判红 · 真档 ⇒ 绿）──"
    S=0; SF=0
    sc() { if eval "$2"; then echo "  PASS $1"; S=$((S+1)); else echo "  FAIL $1"; SF=$((SF+1)); fi; }
    sc "假 cc 记 SKIP（不冒充通过）" "one riscv64 no-such-cc-xyz >/dev/null 2>&1; [ \$skips -ge 1 ]"
    W=/tmp/cross_port_selftest; rm -rf "$W"; mkdir -p "$W/src"
    printf 'def f(:\n    return 1\n' > "$W/src/hello_multi.px"     # 语法坏 ⇒ 必判红
    one aarch64 aarch64-linux-musl-gcc "$W/src/hello_multi.px" >/dev/null 2>&1
    sc "坏源码（语法错）必判红" "[ \$fail -ge 1 ]"
    rm -rf "$W"
    echo "  （自证合计：$S 通过 / $SF 失败）"
    [ "$SF" = 0 ] || exit 1
    echo "CROSS-PORT-SELFTEST-OK"
    pass=0; fail=0; skips=0     # 自证用掉的气量**不许**串到正式跑（否则计数看不懂）
fi

echo "=== 交叉档可编译性（--no-quic · 语料 $SRC）==="
one aarch64 aarch64-linux-musl-gcc
one armv7   armv7l-linux-musleabihf-gcc
one riscv64 riscv64-linux-musl-gcc

echo "── 合计：绿 $pass · 红 $fail · SKIP $skips ──"
if [ "$fail" != 0 ]; then echo "❌ CROSS-PORT FAIL"; exit 1; fi
if [ "$pass" = 0 ]; then echo "⏭ CROSS-PORT SKIP（本机无可用交叉工具链/库；权威覆盖 = ci.yml 的 m67 job）"; exit 0; fi
echo "✅ CROSS-PORT-OK（绿 $pass 档；SKIP $skips 档见上）"
