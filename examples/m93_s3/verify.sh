#!/usr/bin/env bash
# ============================================================
# M93-S3 verify.sh —— 帧协程阻塞原语让出（chan/mutex/rwlock/sleep 协程化）验证门
# ------------------------------------------------------------
# 内容（全部 spawn VM 函数 → 协程；阻塞点 try+让出，worker 不阻塞）：
#   1) coro_mutex.px     4 协程 × 1000 mutex lock/unlock 临界区计数（让出密集）
#   2) coro_chan.px      8 对 sender/recver × cap=1 chan 乒乓（满/空让出）累计和
#   3) coro_sleep_par.px 40 协程并发 sleep 40..120ms → 总墙钟 ≈ max(120ms)（非累加 3.2s）
#   4) coro_with.px      mutex.with / rwlock.with_write/with_read 展开（压帧+帧弹解锁）
#   5) coro_gc_block.px 60 协程 chan 乒乓 × 对象分配 × PX_GC_THRESHOLD=4000 并发 GC
#                       （BLOCKED 协程帧槽 = precise GC 精确根，低阈值零崩/UAF）
#   6) 逃生舱：--c build coro_chan → pthread spawn（语义零变化）
# 退出码：0=全 PASS；非 0=有失败。
# 前置：仓库 tools/px + 最新 rtcache（含 coro.o，跑过任一 px build）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m93_s3"
PASS=0 FAIL=0

chk() { # $1=名 $2=条件
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── 1) mutex 互斥计数（4×1000 临界区，lock 让出密集）"
"$PX" build "$DIR/coro_mutex.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_WORKERS=8 timeout 40 "$DIR/build/coro_mutex" >/tmp/m93s3_1.out 2>&1
grep -q "CORO-MUTEX OK counter=4000" /tmp/m93s3_1.out
chk "coro_mutex 计数精确 4000 (rc=$?)" $?

echo "── 2) chan 乒乓 8 对 × 300（cap=1 满/空让出）累计和"
"$PX" build "$DIR/coro_chan.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_WORKERS=8 timeout 40 "$DIR/build/coro_chan" >/tmp/m93s3_2.out 2>&1
grep -q "CHAN-PINGPONG OK total=8758800" /tmp/m93s3_2.out
chk "coro_chan 乒乓累计和正确 (rc=$?)" $?

echo "── 3) 并发 sleep 40 协程（总时延 ≈ max 120ms 而非累加 3.2s）"
"$PX" build "$DIR/coro_sleep_par.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_WORKERS=8 timeout 40 "$DIR/build/coro_sleep_par" >/tmp/m93s3_3.out 2>&1
grep -q "CORO-SLEEP-PAR OK" /tmp/m93s3_3.out
chk "coro_sleep_par 并发 sleep（wall≈max）" $?

echo "── 4) with 系列展开（mutex.with ×4 + rwlock with_write ×2 + with_read）"
"$PX" build "$DIR/coro_with.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_WORKERS=8 timeout 40 "$DIR/build/coro_with" >/tmp/m93s3_4.out 2>&1
grep -q "CORO-WITH OK g_cnt=800 g_wcnt=400" /tmp/m93s3_4.out
chk "coro_with 展开计数正确 (rc=$?)" $?

echo "── 5) 让出 × 并发 GC（60 协程乒乓 × 分配 × PX_GC_THRESHOLD=4000）"
"$PX" build "$DIR/coro_gc_block.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_GC_THRESHOLD=4000 PX_CORO_WORKERS=8 timeout 90 "$DIR/build/coro_gc_block" >/tmp/m93s3_5.out 2>&1
RC=$?
grep -q "CORO-GC-BLOCK OK" /tmp/m93s3_5.out
chk "coro_gc_block 零崩/UAF（rc=$RC）" $?

echo "── 6) 逃生舱（--c 产物 spawn → pthread，语义零变化）"
"$PX" build --c "$DIR/coro_chan.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
timeout 40 "$DIR/build/coro_chan" >/tmp/m93s3_6.out 2>&1
grep -q "CHAN-PINGPONG OK total=8758800" /tmp/m93s3_6.out
chk "C轨逃生舱 coro_chan（pthread 对拍）" $?

echo "════════ M93-S3 verify：PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ]
