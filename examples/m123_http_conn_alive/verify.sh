#!/bin/bash
# M123 门（qg-issue 78）：http_serve/http_serve_unix「对端断开感知」原语验证
#   语义断言：非 handler 上下文 → 0 · handler 内在线 → 1 · 断连重连不串扰 → 1
#   定量对照：1MB 请求客户端发完即断，用原语提前收尾的总 CPU 必须 ≤ 完整处理 30%（验收线）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/px
SOCK=/tmp/m123.sock
LOG=/tmp/m123_gate.log
SRV=0
fail=0
ok()  { echo "PASS $1"; }
bad() { echo "FAIL $1"; fail=1; }

echo "== [1/3] 编译（VM 轨）=="
$PX build srv.px >/dev/null 2>&1 || { echo "FAIL build"; exit 1; }
ok "build"

echo "== [2/3] 起服务 + 语义断言 =="
rm -f "$SOCK"
./build/srv >"$LOG" 2>&1 &
SRV=$!
for i in $(seq 1 50); do [ -S "$SOCK" ] && break; sleep 0.1; done
[ -S "$SOCK" ] || { echo "FAIL sock 未出现（见 $LOG）"; kill $SRV 2>/dev/null; exit 1; }
sleep 0.3

grep -q "outside_handler alive=0" "$LOG" && ok "非 handler 上下文 → 0" || bad "非 handler 上下文 → 0"

P=$(curl -s --unix-socket "$SOCK" http://x/probe)
[ "$P" = "alive=1" ] && ok "handler 内、对端在线 → 1" || bad "handler 内、对端在线 → 1（实得 '$P'）"

P2=$(curl -s --unix-socket "$SOCK" http://x/probe)
[ "$P2" = "alive=1" ] && ok "断连重连后仍 → 1（fd 复用不串扰）" || bad "断连重连后仍 → 1（实得 '$P2'）"

H=$(curl -s --unix-socket "$SOCK" http://x/health)
[ "$H" = "ok" ] && ok "常规请求不受影响（/health）" || bad "常规请求（/health 实得 '$H'）"

echo "== [3/3] 断连放大定量对照（1MB，客户端完整送达后立即断开）=="
python3 bigdisc.py "$SOCK" "$SRV" /off 1024 1 | tee /tmp/m123_gate_off.txt
python3 bigdisc.py "$SOCK" "$SRV" /on  1024 1 | tee /tmp/m123_gate_on.txt
OFF=$(awk '/CPU_SECONDS/{print $2}' /tmp/m123_gate_off.txt)
ON=$(awk '/CPU_SECONDS/{print $2}' /tmp/m123_gate_on.txt)
python3 - "$OFF" "$ON" <<'PY'
import sys
off, on = float(sys.argv[1]), float(sys.argv[2])
r = on / off * 100.0
print("对照：完整处理 %.2fs → 原语提前收尾 %.2fs = %.2f%%（验收线 ≤ 30%%）" % (off, on, r))
sys.exit(0 if r <= 30.0 else 1)
PY
[ $? -eq 0 ] && ok "断连后服务端总 CPU ≤ 完整处理 30%" || bad "断连后 CPU 超验收线"

kill $SRV 2>/dev/null
if [ "$fail" -eq 0 ]; then echo "m123_http_conn_alive verify done"; else echo "m123_http_conn_alive verify FAILED"; exit 1; fi
