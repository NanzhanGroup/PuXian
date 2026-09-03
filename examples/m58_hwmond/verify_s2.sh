#!/usr/bin/env bash
# ============================================================
# M58 S2 verify —— 温度降级 + mmap 快照共享（双向活映射）
# ------------------------------------------------------------
# 验证点：
#   A. temp 条件探测：无 hwmon/thermal temp 时降级 temp=na 不崩
#      （有真实温度环境则断言数字；本容器无 → na 分支）
#   B. net 非 lo 汇总：net_rx/net_tx 非负整数（真实内核计数）
#   C. mmap MAP_SHARED 活映射双向：
#      C1. daemon 后台采样写共享 → 外部 --dump mmap 活读（读到 SNAP 首行）
#      C2. ts 递增（两轮 dump 时间戳前进 → 快照持续更新，非陈旧）
#      C3. 外部写控制区（--ctl）→ daemon 下一轮读到并回显到快照
#          ctl=<msg>（外部写 → daemon 可见：双向活映射闭环）
#   D. daemon 有限轮次退出码 0（不裸奔验证）
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m58_hwmond/build
D=examples/m58_hwmond
SHM="/tmp/m58s2_verify_$$.shm"

fail() { echo "M58-S2 FAIL: $*" >&2; rm -f "$SHM"; exit 1; }
ok()   { echo "  ✅ $*"; }

cleanup() { rm -f "$SHM"; }
trap cleanup EXIT

echo "== build =="
$PXC build "$D/main.px" >/dev/null 2>&1 || fail "编译失败"

echo "== A. 温度条件探测（降级不崩）+ B. net 采集 =="
OUT=$("$B/main" --once --no-shm 2>&1) || fail "--once --no-shm 退出码非 0"
echo "$OUT"
echo "$OUT" | grep -q '^SNAP ' || fail "无 SNAP 行"
echo "$OUT" | grep -q "temp=" || fail "缺 temp 字段"
# 环境是否有真实温度：有 → 期望数字；无 → 期望 na（当前容器即此态）
HAS_TEMP=$(find /sys/class/hwmon /sys/class/thermal -name 'temp*_input' -o -name 'temp' 2>/dev/null | head -1)
TEMP=$(echo "$OUT" | sed 's/.* temp=\([^ ]*\).*/\1/')
if [ -n "$HAS_TEMP" ]; then
    echo "$TEMP" | grep -qE '^[0-9]+$' || fail "有真实温度但 temp 非数字: $TEMP"
    ok "temp=$TEMP（真实温度 m°C）"
else
    [ "$TEMP" = "na" ] || fail "无温度设备但 temp 非 na: $TEMP"
    ok "temp=na（无 hwmon/thermal temp，降级分支不崩）"
fi
NRX=$(echo "$OUT" | sed 's/.* net_rx=\([^ ]*\).*/\1/')
NTX=$(echo "$OUT" | sed 's/.* net_tx=\([^ ]*\).*/\1/')
echo "$NRX" | grep -qE '^[0-9]+$' || fail "net_rx 非数字: $NRX"
echo "$NTX" | grep -qE '^[0-9]+$' || fail "net_tx 非数字: $NTX"
ok "net_rx=$NRX net_tx=$NTX（非负整数，真实内核计数）"

echo "== C. mmap MAP_SHARED 双向活映射 =="
"$B/main" --n 12 --interval 1 --shm "$SHM" >/tmp/m58s2_daemon.log 2>&1 &
DPID=$!
echo "  daemon pid=$DPID"
sleep 2.2
D1=$("$B/main" --dump --shm "$SHM" 2>&1) || fail "dump#1 失败"
echo "  dump#1: $D1"
echo "$D1" | grep -q '^SNAP ' || fail "dump#1 未读到快照"
ok "C1. 外部 --dump mmap 活读到 daemon 快照"
sleep 1.2
D2=$("$B/main" --dump --shm "$SHM" 2>&1) || fail "dump#2 失败"
echo "  dump#2: $D2"
T1=$(echo "$D1" | sed 's/.*ts=\([0-9]*\).*/\1/')
T2=$(echo "$D2" | sed 's/.*ts=\([0-9]*\).*/\1/')
[ -n "$T1" ] && [ -n "$T2" ] || fail "ts 提取失败"
[ "$T2" -gt "$T1" ] || fail "dump ts 未递增（$T1 → $T2）：快照非活更新"
ok "C2. ts 递增（$T1 → $T2）：daemon 持续写，外部活读可见"

"$B/main" --ctl "HELLO-S2" --shm "$SHM" >/dev/null 2>&1 || fail "--ctl 写控制区失败"
echo "  外部写控制区 ctl=HELLO-S2"
FOUND=""
for i in 1 2 3 4 5 6; do
    sleep 0.8
    D=$("$B/main" --dump --shm "$SHM" 2>/dev/null || true)
    if echo "$D" | grep -q "ctl=HELLO-S2"; then
        FOUND="poll$i: $D"
        break
    fi
done
if [ -n "$FOUND" ]; then
    echo "  $FOUND"
    ok "C3. 外部写控制区 → daemon 下轮快照回显 ctl=HELLO-S2（双向可见）"
else
    fail "未观察到 ctl 回显（外部写未达 daemon）"
fi

echo "== D. daemon 正常退出 =="
wait "$DPID"
RC=$?
[ "$RC" -eq 0 ] || fail "daemon 退出码 $RC"
ok "daemon --n 12 有限轮次退出码 0"
echo "✅ M58-S2: 温度降级 + mmap 快照共享（双向活映射）PASS"
