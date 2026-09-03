#!/usr/bin/env bash
# ============================================================
# M58 S4 verify —— run.sh 崩溃自愈 + aarch64 交叉 qemu + 回归
# ------------------------------------------------------------
# 验证点：
#   A. aarch64 交叉编译多文件 import 工程（--no-quic 裁剪，daemon 无 H3）
#      → file 确认 ARM aarch64 静态 → qemu-aarch64 --once 输出 SNAP 行，
#      mem_total 与 x86/MemTotal 一致（跨架构同源 /proc 实证）
#   B. run.sh 崩溃自愈：daemon 常驻 → kill -9（模拟崩溃）→ wrapper 自动
#      拉起新 daemon（存活断言 + wrapper 日志 attempt 递增）
#   C. x86 回归：m57_s1_ioctl_verify + m57_s3_verify（M57 fd 能力未动）
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m58_hwmond/build
D=examples/m58_hwmond
QEMU="${QEMU:-qemu-aarch64-static}"
CCPATH=/opt/aarch64-linux-musl-cross/bin
CC="${CC:-aarch64-linux-musl-gcc}"
PORT=$((21000 + RANDOM % 3000))
SHM="/tmp/m58s4_verify_$$.shm"
WRAP_LOG="/tmp/m58s4_wrapper_$$.log"

fail() { echo "M58-S4 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }
cleanup() {
    # 清理 wrapper 与 daemon 残留（防孤儿进程/端口占用）
    pkill -9 -f "build/main --port $PORT" 2>/dev/null
    pkill -9 -f "run.sh --port $PORT" 2>/dev/null
    rm -f "$SHM" "$WRAP_LOG"
}
trap cleanup EXIT

echo "== build x86 =="
$PXC build "$D/main.px" >/dev/null 2>&1 || fail "x86 编译失败"

echo "== A. aarch64 交叉编译 + qemu --once =="
command -v "$QEMU" >/dev/null 2>&1 || fail "缺 $QEMU"
command -v "$CC" >/dev/null 2>&1 || {
    export PATH="$CCPATH:$PATH"
    command -v "$CC" >/dev/null 2>&1 || fail "缺交叉编译器 $CC（$CCPATH）"
}
[ -f runtime/mbedtls/lib-aarch64/libmbedcrypto.a ] || fail "缺 aarch64 mbedtls 库"
[ -f runtime/third_party/sqlite3/sqlite3-aarch64.o ] || fail "缺 aarch64 sqlite3.o"
export PATH="$CCPATH:$PATH"
TMPD=$(mktemp -d /tmp/m58s4.XXXXXX)
trap 'rm -rf "$TMPD"; cleanup' EXIT
cp "$D"/*.px "$TMPD"/
tools/pxc build --no-quic \
    --cc "$CC" \
    --mbedtls-lib runtime/mbedtls/lib-aarch64 \
    --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o \
    "$TMPD/main.px" >/tmp/m58s4_x.log 2>&1 || { echo "交叉编译失败:"; tail -20 /tmp/m58s4_x.log; exit 1; }
XBIN="$TMPD/build/main"
F=$(file "$XBIN")
echo "$F"
echo "$F" | grep -q "ARM aarch64" || fail "产物非 aarch64"
ok "交叉编译成功（多文件 import 工程 + --no-quic）"
XOUT=$("$QEMU" "$XBIN" --once --no-shm 2>&1) || fail "qemu --once 退出非 0"
echo "  qemu: $XOUT"
echo "$XOUT" | grep -q '^SNAP ' || fail "qemu 输出无 SNAP"
XMT=$(echo "$XOUT" | sed 's/.* mem_total=\([^ ]*\).*/\1/')
REF_MEM=$(awk '/^MemTotal:/{print $2}' /proc/meminfo)
[ "$XMT" = "$REF_MEM" ] || fail "qemu mem_total=$XMT 应=$REF_MEM"
ok "qemu-aarch64 静态产物 --once 采集真实 /proc（mem_total=$XMT 与 MemTotal 一致，跨架构同源实证）"

echo "== B. run.sh 崩溃自愈 =="
"$D/run.sh" --port "$PORT" --shm "$SHM" >"$WRAP_LOG" 2>&1 &
WRAP_PID=$!
sleep 3
P1=$(pgrep -f "build/main --port $PORT" | head -1)
[ -n "$P1" ] || { echo "  wrapper 日志:"; cat "$WRAP_LOG"; fail "daemon 未启动（pgrep 空）"; }
ok "daemon 启动（pid $P1）"
kill -9 "$P1"
sleep 3
P2=$(pgrep -f "build/main --port $PORT" | head -1)
[ -n "$P2" ] || { echo "  wrapper 日志:"; cat "$WRAP_LOG"; fail "kill -9 后未拉起新 daemon"; }
[ "$P2" != "$P1" ] || fail "新 daemon pid 与旧相同（未重启？）"
grep -q "attempt 2" "$WRAP_LOG" || { echo "  wrapper 日志:"; cat "$WRAP_LOG"; fail "wrapper 日志无 attempt 2"; }
ok "kill -9 旧 daemon（$P1）→ wrapper 自动拉起新 daemon（$P2, attempt 2）"
# 清理本步后台（cleanup trap 兜底）
kill "$WRAP_PID" 2>/dev/null
pkill -9 -f "build/main --port $PORT" 2>/dev/null
sleep 0.5

echo "== C. x86 回归（M57 fd 能力未动）=="
bash examples/m57_s1_ioctl_verify.sh >/tmp/m58s4_reg1.log 2>&1 && echo "  m57_s1 PASS" || { tail -5 /tmp/m58s4_reg1.log; fail "m57_s1 回归失败"; }
bash examples/m57_s3_verify.sh >/tmp/m58s4_reg3.log 2>&1 && echo "  m57_s3 PASS" || { tail -5 /tmp/m58s4_reg3.log; fail "m57_s3 回归失败"; }
ok "m57_s1_ioctl + m57_s3_devctl 回归 PASS（语言面未动）"

echo "✅ M58-S4: run.sh 崩溃自愈 + aarch64 交叉 qemu PASS"
