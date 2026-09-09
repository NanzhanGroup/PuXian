#!/bin/bash
# ============================================================
# M96-S2 verify.sh —— 客户端网络 IO 协程化（D8-① offload 执行器核心）验证门
# ------------------------------------------------------------
#   1) 不卡 worker（铁证）：PX_CORO_WORKERS=1 + 慢 http(/slow2 500ms) + 快 sleep(60ms)
#      协程 —— offload 下 sleep 在 http 完成前执行（完成时刻 <300ms）；若阻塞 native
#      卡 1 worker，sleep 饿到 http 完（≥500ms）才跑 → q 断言暴露。
#   2) offload 与直调对拍：4 协程 http_get(/data) 内容 == 主线程直调基准（逐字节一致）
#   3) 线程池上限收敛：并发 12 × 慢 http + PX_OFFLOAD_MAX=4 → 活跃期 Threads ≤ 上限+
#      worker+主+余量（不随 spawn 数涨到 12+）；空闲回收后回落（< 峰值）
#   4) 逃生舱 --c（C 轨 fn_* 文本）：spawn = pthread，http_get 直调（无 offload 路径）
#      → 语义零变化全对
#   5) VM 轨重建回归：--c 覆盖产物后重建 notblock 复跑确认
# 退出码：0=全 PASS；非 0=有失败。
# 依赖：tools/px（M91 默认 VM 轨 + --c 逃生舱）。端口 18889。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m96_s2"
PORT=18889
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── build daemon + 测试程序（VM 轨；runtime 变更自动重建缓存）"
"$PX" build --no-quic "$DIR/m96_daemon.px"   >/tmp/m96s2_build.log 2>&1 || { echo "FAIL build daemon"; tail -5 /tmp/m96s2_build.log; exit 1; }
"$PX" build --no-quic "$DIR/m96_notblock.px"  >>/tmp/m96s2_build.log 2>&1 || { echo "FAIL build notblock"; tail -5 /tmp/m96s2_build.log; exit 1; }
"$PX" build --no-quic "$DIR/m96_pool.px"      >>/tmp/m96s2_build.log 2>&1 || { echo "FAIL build pool"; tail -5 /tmp/m96s2_build.log; exit 1; }
chk "build(VM 轨)" 0

SRV_PID=0
trap 'if [ -n "$SRV_PID" ] && [ "$SRV_PID" -gt 0 ] 2>/dev/null; then kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; fi' EXIT
PX_SERVE_WORKERS=2 PX_CORO_WORKERS=2 "$DIR/build/m96_daemon" $PORT \
    >/tmp/m96s2_srv.log 2>&1 < /dev/null &
SRV_PID=$!
sleep 1.0
B=$(curl -s --max-time 3 "http://127.0.0.1:$PORT/data"); chk "慢服务就绪 /data" $([ -n "$B" ] && echo 0 || echo 1)

echo "── 1+2) 不卡 worker + 对拍：PX_CORO_WORKERS=1 慢 http(500ms)+快 sleep+4 对拍"
T0=$(date +%s.%N)
timeout 60 env PX_CORO_WORKERS=1 PX_OFFLOAD_IDLE_MS=800 "$DIR/build/m96_notblock" >/tmp/m96s2_nb.out 2>&1
RC=$?
T1=$(date +%s.%N)
WALL=$(awk -v a="$T0" -v b="$T1" 'BEGIN{printf "%.2f", b-a}')
grep -q "NOTBLOCK-ALL-OK" /tmp/m96s2_nb.out
chk "notblock 全对（wall=${WALL}s rc=$RC）" $?
# 显式复述 q 完成时刻（判 worker 不被卡的铁证行）
Q=$(grep '^NB msgs=' /tmp/m96s2_nb.out | sed 's/.*q1:/q1:/')
echo "  $Q"
# 防御：确认 q 断言在 px 内已过（NOTBLOCK-ALL-OK 已含 qbad==0），此门即闭合
grep -q "q1:[0-9]*:" /tmp/m96s2_nb.out && echo "  (q 完成时刻已由 px 断言 <300ms 把关)"

echo "── 3) 线程池上限收敛 + 空闲回收：PX_OFFLOAD_MAX=4 + 并发 12 慢 http"
env PX_CORO_WORKERS=2 PX_OFFLOAD_MAX=4 PX_OFFLOAD_IDLE_MS=600 \
    "$DIR/build/m96_pool" >/tmp/m96s2_pool.out 2>&1 < /dev/null &
POOL_PID=$!
sleep 0.45     # http 批处理活跃期（12×200ms 分 3 批在 4 外包线程，wall≈0.6-1.2s）
T1_THR=$(grep Threads /proc/$POOL_PID/status 2>/dev/null | awk '{print $2}')
# 等 POOL-OK 出现（http 全完成）→ 再等 > PX_OFFLOAD_IDLE_MS(600ms) → 采样空闲回收后
for i in $(seq 1 100); do
    grep -q "POOL-OK" /tmp/m96s2_pool.out 2>/dev/null && break
    sleep 0.1
done
sleep 0.8     # > 600ms 空闲回收窗
T2_THR=$(grep Threads /proc/$POOL_PID/status 2>/dev/null | awk '{print $2}')
wait $POOL_PID 2>/dev/null
POOLRC=$?
grep -q "POOL-OK" /tmp/m96s2_pool.out
chk "pool 12×http 全对" $?
echo "  活跃期 Threads=$T1_THR（上限 4 off + 2 worker + 主 + 服务余量，理论 ≈ 10 内）"
chk "线程池上限收敛 T1≤14" $([ -n "$T1_THR" ] && [ "$T1_THR" -le 14 ] && echo 0 || echo 1)
echo "  空闲回收后 Threads=$T2_THR（< 峰值 → 外包线程已回收）"
chk "空闲回收 T2<T1" $([ -n "$T2_THR" ] && [ -n "$T1_THR" ] && [ "$T2_THR" -lt "$T1_THR" ] && echo 0 || echo 1)

echo "── 4) 逃生舱 --c（C 轨 fn_* 文本：spawn=pthread、http_get 直调，无 offload 路径）"
"$PX" build --no-quic --c "$DIR/m96_notblock.px" >/tmp/m96s2_c.log 2>&1 || { echo "FAIL --c build"; tail -5 /tmp/m96s2_c.log; chk "--c build" 1; }
timeout 60 env "$DIR/build/m96_notblock" >/tmp/m96s2_c.out 2>&1
RC=$?
grep -q "NOTBLOCK-ALL-OK" /tmp/m96s2_c.out
chk "逃生舱直调语义不变（rc=$RC）" $?

echo "── 5) VM 轨重建回归（--c 覆盖产物后）"
"$PX" build --no-quic "$DIR/m96_notblock.px" >/tmp/m96s2_b2.log 2>&1 || { chk "VM 重建" 1; }
timeout 60 env PX_CORO_WORKERS=1 PX_OFFLOAD_IDLE_MS=800 "$DIR/build/m96_notblock" >/tmp/m96s2_nb2.out 2>&1
RC=$?
grep -q "NOTBLOCK-ALL-OK" /tmp/m96s2_nb2.out
chk "VM 重建复跑（rc=$RC）" $?

kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; SRV_PID=0

echo ""
echo "════════ m96_s2: PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ] && exit 0 || exit 1
