#!/bin/bash
# ============================================================
# M97-S2b verify.sh —— DEFECT-hpool-dead-conn 回归门
#   「池内满 4 条死连接（HTTP_POOL_PER_HOST）时，http_request 必须仍能拿到 200」
# ------------------------------------------------------------
# 病灶：hpool 里的死连接被**逐条取走、close、不放回** ⇒ 每条 = 一次本地立即失败（~1ms）。
#   阈值（源码推演）：Err ⟺ 池内死连接数 ≥ opts.retries + 2
#     · 默认 retries=1（max_attempts=2）⇒ 3 条死连接即失败
#     · retries=2（max_attempts=3）⇒ 满池 4 条才失败（对齐 Ma 现场）
#   修前 `dead_rebuilt` 只允许重建一次（runtime.c:18405）⇒ 越过阈值即 502。
#
# 门构造（见 m97_s2b_upstream.px 头注释）：
#   上游 HTTP/1.1 + 应答后立刻 close；对 /fill* 请求**压着应答直到 4 条连接到齐**
#   ⇒ 客户端并发 4 路蓄池，池里同时存在 4 条死连接（不是 1 条，不能被 dead_rebuilt 兜住）。
#
# 判据（三条全过才算绿）：
#   1) 蓄池两轮 bad==0（4 路并发本身健康）
#   2) 上游出现 2 次 UP:HELD4（确实每次攒满 4 条连接，构造没退化）
#   3) probe retries=1 与 retries=2 **都**拿到 200 → 打印 M97S2B_ALL_OK
#   负控：把 runtime.c 的 M1 预算改回单次（或撤 M1）⇒ probe 必报 err(~1ms) ⇒ 本门红。
#
# 依赖：tools/px（VM 轨）。端口 18891（与 m97_s2 的 18890 不冲突）。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
DIR="$ROOT/examples/m97_s2b"
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── build upstream + client（VM 轨；runtime 变更自动重建缓存）"
"$PX" build --no-quic "$DIR/m97_s2b_upstream.px" >/tmp/m97s2b_build.log 2>&1 || { echo "FAIL build upstream"; tail -5 /tmp/m97s2b_build.log; exit 1; }
"$PX" build --no-quic "$DIR/m97_s2b_client.px"   >>/tmp/m97s2b_build.log 2>&1 || { echo "FAIL build client"; tail -5 /tmp/m97s2b_build.log; exit 1; }
chk "build(VM 轨)" 0

SRV_PID=0
cleanup() { if [ -n "$SRV_PID" ] && [ "$SRV_PID" -gt 0 ] 2>/dev/null; then kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; fi; SRV_PID=0; }
trap cleanup EXIT

echo "── 池内满 4 条死连接：retries=1 / retries=2 都必须 200"
"$DIR/build/m97_s2b_upstream" >/tmp/m97s2b_up.out 2>&1 </dev/null &
SRV_PID=$!
sleep 0.6
timeout 90 "$DIR/build/m97_s2b_client" >/tmp/m97s2b_cli.out 2>&1
RC=$?
cat /tmp/m97s2b_cli.out
HELD=$(grep -c '^UP:HELD4' /tmp/m97s2b_up.out 2>/dev/null || echo 0)
chk "上游攒满 4 条连接 ×2 轮（HELD4=$HELD）" $([ "$HELD" = "2" ] && echo 0 || echo 1)
grep -q "M97S2B_ALL_OK" /tmp/m97s2b_cli.out
chk "retries=1/2 在满池死连接下都 200（rc=$RC）" $?
if grep -q "M97S2B_REPRO_502" /tmp/m97s2b_cli.out; then
    echo "  ↳ 复现到缺陷（修前预期：err ~1ms + 无 RTT）"
    grep -E 'probe retries=' /tmp/m97s2b_cli.out | sed 's/^/    /'
fi
cleanup

echo ""
echo "════════ m97_s2b: PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ] && exit 0 || exit 1
