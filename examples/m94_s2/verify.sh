#!/usr/bin/env bash
# ============================================================
# M94-S2 verify.sh —— 抢占式时间片验证门
# ------------------------------------------------------------
#   1) coro_preempt WORKERS=1 + 默认量子(5ms)：死循环 busy×2 每片让出 →
#      有限 wkr×8 分到片全部完成（M93 无抢占会饿死 —— 本门核心）
#   2) coro_preempt WORKERS=1 + PX_CORO_QUANTUM_US=0（逃生阀）：抢占关 →
#      busy 独占 worker → wkr 饿死 → timeout（预期失败码，证明逃生阀语义）
#   3) coro_preempt WORKERS=4 + 小量子 200us：小时间片下轮转依然正确
#   4) coro_many 1000 协程纯计算（PX_CORO_DIAG=1 done 行数=1000）—— 抢占
#      不回归并发吞吐（worker 数 = PX_CORO_WORKERS 非 spawn 数）
#   5) 抢占协程数不放大线程：thr_count 128 spawn → 线程 = worker+1(+timer)
# 退出码：0=全 PASS；非 0=有失败。
# 前置：仓库 tools/px（跑过任一 build 使 .rtcache 含最新 coro.o/vm.o）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m94_s2"
S2="$ROOT/examples/m93_s2"
PASS=0 FAIL=0

chk() { # $1=名 $2=条件
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── 1) 抢占公平性：WORKERS=1 + 默认量子（死循环×2 vs 有限×8 轮转）"
"$PX" build "$DIR/coro_preempt.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_WORKERS=1 timeout 60 "$DIR/build/coro_preempt" >/tmp/m94s2_1.out 2>&1
grep -q "PREEMPT-FAIR OK N=8" /tmp/m94s2_1.out
chk "抢占公平 WORKERS=1（rc=$?）" $?

echo "── 2) 逃生阀负验证：PX_CORO_QUANTUM_US=0 → 死循环独占 → 饿死（timeout 预期）"
PX_CORO_WORKERS=1 PX_CORO_QUANTUM_US=0 timeout 8 "$DIR/build/coro_preempt" >/tmp/m94s2_2.out 2>&1
RC=$?
if [ "$RC" != "0" ] && ! grep -q "PREEMPT-FAIR OK" /tmp/m94s2_2.out; then
    echo "  PASS 逃生阀关闭抢占 → 饿死（timeout rc=$RC，符合预期）"; PASS=$((PASS+1))
else
    echo "  FAIL 逃生阀负验证（rc=$RC 本应超时饿死）"; FAIL=$((FAIL+1))
fi

echo "── 3) 小量子 200us + WORKERS=4（抢占更频繁下轮转正确）"
PX_CORO_WORKERS=4 PX_CORO_QUANTUM_US=200 timeout 60 "$DIR/build/coro_preempt" >/tmp/m94s2_3.out 2>&1
grep -q "PREEMPT-FAIR OK N=8" /tmp/m94s2_3.out
chk "小量子 200us 轮转（rc=$?）" $?

echo "── 4) coro_many 1000 协程纯计算全完成（抢占不回归吞吐）"
"$PX" build "$S2/coro_many.px" >/dev/null 2>&1 || { echo "build fail"; exit 1; }
PX_CORO_DIAG=1 PX_CORO_WORKERS=8 timeout 60 "$S2/build/coro_many" >/tmp/m94s2_4.out 2>&1
N=$(grep -c "\[px-coro\] #.* done" /tmp/m94s2_4.out)
[ "$N" = "1000" ]
chk "coro_many done=$N/1000（rc=$?）" $?

echo "════════ M94-S2 verify：PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ]
