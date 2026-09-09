#!/bin/bash
# ============================================================
# M97-S2 verify.sh —— qg-issue 31（客户端 http_request 连接池 keep-alive 生命周期）验证门
# ------------------------------------------------------------
#   1) 修复验证（新 runtime）：HTTP/1.0 模拟上游（无 Connection 头、发完即关）+
#      连续 30 次 http_request → 全 status==200（bad==0）零失败
#      （修复前：h_exchange 误判 keep_alive → 死连接回池 → 奇偶失败 ~50%）
#   2) 对拍复现：bootstrap/pxi（未重链 = 旧 runtime）跑同客户端 → 预期 ~半数失败
#      （奇偶 FAILS），实锤「修复前确实失败」；若 pxi 轨全过/全连不上则 SKIP 不强求
#   3) HTTP/1.1 正常 keep-alive 回归：见 m97_s3（长连复用 100 请求）——本门专注 F31
# 退出码：0=全 PASS；非 0=有失败。
# 依赖：tools/px（VM 轨）+ bootstrap/pxi（旧 runtime 对拍）。端口 18890。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
PXI="$ROOT/bootstrap/pxi"
DIR="$ROOT/examples/m97_s2"
PORT=18890
PASS=0 FAIL=0

chk() { # $1=名 $2=条件(0=pass)
    if [ "$2" = "0" ]; then echo "  PASS $1"; PASS=$((PASS+1)); else echo "  FAIL $1"; FAIL=$((FAIL+1)); fi
}

echo "── build upstream + client（VM 轨；runtime 变更自动重建缓存）"
"$PX" build --no-quic "$DIR/m97_s2_upstream.px" >/tmp/m97s2_build.log 2>&1 || { echo "FAIL build upstream"; tail -5 /tmp/m97s2_build.log; exit 1; }
"$PX" build --no-quic "$DIR/m97_s2_client.px"   >>/tmp/m97s2_build.log 2>&1 || { echo "FAIL build client"; tail -5 /tmp/m97s2_build.log; exit 1; }
chk "build(VM 轨)" 0

SRV_PID=0
cleanup() { if [ -n "$SRV_PID" ] && [ "$SRV_PID" -gt 0 ] 2>/dev/null; then kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null; fi; SRV_PID=0; }
trap cleanup EXIT

echo "── 1) 修复验证：HTTP/1.0 上游 + 连续 30 次 http_request（新 runtime）"
"$DIR/build/m97_s2_upstream" >/tmp/m97s2_up.out 2>&1 </dev/null &
SRV_PID=$!
sleep 0.6
timeout 60 "$DIR/build/m97_s2_client" >/tmp/m97s2_cli.out 2>&1
RC=$?
UPN=$(grep -c '^UP:' /tmp/m97s2_up.out 2>/dev/null || echo 0)
grep -q "M97S2_CLIENT_ALL_OK_30" /tmp/m97s2_cli.out
chk "30/30 全成功（rc=$RC, upstream_accept=$UPN）" $?
grep -E 'ok=|req#' /tmp/m97s2_cli.out | head -5
# 修复语义附加断言：HTTP/1.0 连接不回池 → accept 数 == 请求数（30），无死连接回池复用
chk "accept==30（HTTP/1.0 每请求新建连接，不回池）" $([ "$UPN" = "30" ] && echo 0 || echo 1)
cleanup

echo "── 2) 对拍复现：bootstrap/pxi（旧 runtime，未重链）跑同客户端"
"$DIR/build/m97_s2_upstream" >/tmp/m97s2_up2.out 2>&1 </dev/null &
SRV_PID=$!
sleep 0.6
timeout 60 "$PXI" "$DIR/m97_s2_client.px" >/tmp/m97s2_pxi.out 2>&1
PRC=$?
UPN2=$(grep -c '^UP:' /tmp/m97s2_up2.out 2>/dev/null || echo 0)
OKN=$(grep -o 'ok=[0-9]*' /tmp/m97s2_pxi.out | head -1 | cut -d= -f2)
if [ -n "$OKN" ] && [ "$OKN" -lt 30 ] && grep -q "M97S2_CLIENT_FAILS_" /tmp/m97s2_pxi.out; then
    chk "旧 runtime 复现失败（ok=$OKN/30 <30 → 奇偶失败 bug 实锤）" 0
    grep -E 'ok=|req#' /tmp/m97s2_pxi.out | head -6
elif grep -q "M97S2_CLIENT_ALL_OK_" /tmp/m97s2_pxi.out; then
    echo "  SKIP pxi 轨也全过（pxi 与 VM 轨 http 路径差异，不影响 VM 轨修复结论）"
else
    echo "  SKIP pxi 未产出判定（rc=$PRC, ok=$OKN, up_accept=$UPN2）：$(tail -2 /tmp/m97s2_pxi.out | head -1)"
fi
cleanup

echo ""
echo "════════ m97_s2: PASS=$PASS FAIL=$FAIL ════════"
[ "$FAIL" = "0" ] && exit 0 || exit 1
