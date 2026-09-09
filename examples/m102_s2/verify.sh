#!/bin/bash
# ============================================================
# M102-S2 verify.sh —— .px 子进程池协程化验证门（D1 语言层 px_exec offload）
# ------------------------------------------------------------
#   铁证：PX_CORO_WORKERS=1 —— 3 并发 px_exec（各 sleep400）协程 wall ≈ 单次基准
#   （外包并行：3 个 px_exec 在外包线程池并行 + coro worker 让出取下一协程）；
#   若 px_exec 同步阻塞卡 1 coro worker → 3 协程串行 ≈ 3×单次 → 暴露。
#   判据：once ≈ 500ms；conc < 2×once 且 3 结果全对。
# 退出码：0=全 PASS；非 0=有失败。
# 依赖：tools/px（VM 轨 build）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m102_s2"
PASS=0 FAIL=0
chk() { if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi }

echo "── build s2a_probe（VM 轨；runtime 变更自动重建缓存）"
"$PX" build --no-quic "$DIR/s2a_probe.px" >/tmp/m102s2_build.log 2>&1 || { echo "FAIL build"; tail -5 /tmp/m102s2_build.log; exit 1; }
chk "build s2a_probe" 0

echo "── 1) px_exec 协程 ctx 外包：PX_CORO_WORKERS=1 单次基准 vs 3 并发"
timeout 90 env PX_CORO_WORKERS=1 "$DIR/build/s2a_probe" 2>/dev/null >/tmp/m102s2.out
RC=$?
chk "s2a_probe rc=0" $RC
ONCE=$(grep '^S2A once_ms=' /tmp/m102s2.out | sed 's/^S2A once_ms=//' | sed 's/ ok=.*//')
CONC=$(grep '^S2A conc_ms=' /tmp/m102s2.out | sed 's/.*=//')
echo "  once_ms=$ONCE conc_ms=$CONC"
grep -q 'S2A-PXEXEC-OFFLOAD-OK' /tmp/m102s2.out
chk "S2A-PXEXEC-OFFLOAD-OK（并发 ≈ 单次 → 外包并行）" $?
grep -q 'S2A ok=3' /tmp/m102s2.out
chk "协程 px_exec 结果 3/3 对拍 slow-ok" $?
if [ -n "$ONCE" ] && [ -n "$CONC" ] && [ "$CONC" -lt $((ONCE * 2)) ]; then
    chk "并发 wall < 2×单次（外包并行定量）" 0
else
    chk "并发 wall < 2×单次（外包并行定量）" 1
fi

echo "======================================"
echo "M102-S2 结果: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = "0" ]
