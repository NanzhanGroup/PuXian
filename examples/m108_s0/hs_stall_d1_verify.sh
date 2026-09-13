#!/bin/bash
# D1 验收台（qg-issue 66 · px_serve 握手停滞「可归因」改动）
# ---------------------------------------------------------------------------
# 被测：runtime/runtime.c
#   · px_serve_stall_log() 新增 peer=（getpeername+inet_ntop）与 reason=
#   · px_conn_tls_handshake() 停止原因分桶（deadline / shutdown / step-guard / poll-error）
#   · PX_SERVE_DIAG 摘要新增 hs_stop(deadline= shutdown= guard= pollerr=)
# 判据（**只看服务端自身 stderr**，不靠外部推断）：
#   C1 deadline    慢客户端（发 5 字节 Record 头后静默）⇒ reason=deadline + peer=127.0.0.1:*
#   C2 poll-error  RST 客户端（SO_LINGER=0）             ⇒ reason=poll-error
#                  —— 这是 F10 猜想（96 条自然 timeout 中 62 条「停滞<1s」在语义上
#                     不可能是 10s 总截止）的**本地可判据**
#   C3 shutdown    关闭期在途握手                        ⇒ reason=shutdown
#   C4 计数聚合     diag 行 hs_stop(...) 与日志条数逐项相等，且 guard=0
#   C5 兼容性      旧字段 fd= / 停滞= / extra= / hs_inflight= 仍在（不破坏既有取证口径）
# 用法：hs_stall_d1_verify.sh [port]
#   需要 go 才能编 rst_client（C2）；无 go ⇒ C2 记 SKIP（不记 FAIL）。
set -u
PORT="${1:-18447}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SRV="${SRV_BIN:-$ROOT/examples/m108_s0/build/hs_stall_server}"
WORK=/tmp/m108_d1
HS_TMO_MS="${HS_TMO_MS:-3000}"
PASS=0; FAIL=0; SKIP=0
ok(){ echo "  PASS  $*"; PASS=$((PASS+1)); }
no(){ echo "  FAIL  $*"; FAIL=$((FAIL+1)); }
sk(){ echo "  SKIP  $*"; SKIP=$((SKIP+1)); }

rm -rf "$WORK"; mkdir -p "$WORK/site"
pkill -f 'hs_stall_server' 2>/dev/null; sleep 0.3
[ -x "$SRV" ] || { echo "缺少 $SRV（先 ./tools/pxc build examples/m108_s0/hs_stall_server.px）"; exit 2; }

if [ ! -f "$WORK/cert.pem" ]; then
  openssl req -x509 -newkey rsa:2048 -keyout "$WORK/key.pem" -out "$WORK/cert.pem" \
      -days 2 -nodes -subj '/CN=localhost' -addext 'subjectAltName=DNS:localhost' >/dev/null 2>&1
fi

SRVPID=0; C1PID=0; C3PID=0
cleanup(){ [ "${SRVPID:-0}" != "0" ] && kill -9 "$SRVPID" 2>/dev/null
           [ "${C1PID:-0}" != "0" ] && kill "$C1PID" 2>/dev/null
           [ "${C3PID:-0}" != "0" ] && kill "$C3PID" 2>/dev/null; :; }
trap cleanup EXIT

# --- 可选：编 RST 客户端（C2 用）
if command -v go >/dev/null 2>&1; then
  HOME="${HOME:-/root}" go build -o "$WORK/rst_client" "$ROOT/examples/m108_s0/rst_client.go" >/dev/null 2>&1
fi

echo "=== [0] 启动（port=$PORT  tls_hs_tmo=${HS_TMO_MS}ms  PX_SERVE_DIAG=1）"
PX_TLS_HS_TMO_MS="$HS_TMO_MS" PX_SERVE_DIAG=1 "$SRV" "$PORT" "$WORK/cert.pem" "$WORK/key.pem" "$WORK/site" \
    >"$WORK/srv.log" 2>&1 &
SRVPID=$!
sleep 1.5
sed -n '1,2p' "$WORK/srv.log" | sed 's/^/  /'
if timeout 8 curl -sk --resolve "localhost:$PORT:127.0.0.1" --max-time 6 -o /dev/null \
     -w '%{http_code}' "https://localhost:$PORT/" 2>/dev/null | grep -q 200; then
  ok "[0] 基线 200（服务可用，且 diag=1 生效）"
else
  no "[0] 基线非 200 —— 后续断言不可信"
fi

wait_stall(){ local pat="$1" tmo="$2" i; for ((i=0;i<tmo*10;i++)); do grep -qE "$pat" "$WORK/srv.log" && return 0; sleep 0.1; done; return 1; }
# ⚠️ 后台子壳的 stdout/stderr 必须重定向：否则它会持有 `$(slow_client)` 的命令替换管道，
#   令赋值**阻塞到 stall 客户端退出（25s）**为止 —— C3 会因此在 kill 之前就已错过在途握手，
#   把「未出现 shutdown」误报成实现缺陷（实测踩到，第一版就是被这个坑骗过）。
slow_client(){ ( exec 3<>"/dev/tcp/127.0.0.1/$PORT" || exit 1
                 printf '\x16\x03\x01\x02\x00' >&3
                 sleep 25 ) >/dev/null 2>&1 &
               echo $!; }

echo "=== C1 慢客户端（静默不完成握手）→ reason=deadline + peer"
C1PID=$(slow_client)
if wait_stall 'tls-handshake-timeout reason=deadline' $(( HS_TMO_MS/1000 + 4 )); then
  L=$(grep -m1 'tls-handshake-timeout reason=deadline' "$WORK/srv.log")
  echo "  $L"
  echo "$L" | grep -qE 'peer=127\.0\.0\.1:[0-9]+' \
    && ok "C1 peer= 已标识（getpeername+inet_ntop 生效）" || no "C1 peer= 缺失或非 127.0.0.1"
  echo "$L" | grep -qE 'reason=deadline' && ok "C1 reason=deadline（真·总截止）" || no "C1 reason 非 deadline"
else
  no "C1 超时未出现 reason=deadline"
fi

echo "=== C2 RST 客户端 → reason=poll-error（F10 判据）"
if [ -x "$WORK/rst_client" ]; then
  for _ in 1 2 3; do "$WORK/rst_client" "127.0.0.1:$PORT" 300 >/dev/null 2>&1; sleep 0.4; done
  if wait_stall 'reason=poll-error' 8; then
    L=$(grep -m1 'reason=poll-error' "$WORK/srv.log"); echo "  $L"
    echo "$L" | grep -qE 'peer=127\.0\.0\.1:[0-9]+' && ok "C2 peer= 已标识" || no "C2 peer= 缺失"
    echo "$L" | grep -qE '停滞=[0-9]{1,3}ms' && ok "C2 停滞<1s 也被明确标为 poll-error（不再冒充 10s 截止）" \
                                              || echo "  （注：本次 RST 的停滞未落在 <1s 桶，不影响判据）"
  else
    no "C2 未出现 reason=poll-error"
  fi
else
  sk "C2 无 rst_client（缺 go 工具链），poll-error 未被本地覆盖"
fi

echo "=== C4 计数聚合：diag 行 hs_stop(...) 与日志逐项一致"
sleep 6   # 让最后一行 diag（每 5s）包含本轮全部计数
D=$(grep 'hs_stop(' "$WORK/srv.log" | tail -1)
if [ -z "$D" ]; then
  no "C4 diag 行未见 hs_stop(（计数未暴露）"
else
  echo "  $D"
  HS=$(echo "$D" | sed -n 's/.*hs_stop(\([^)]*\)).*/\1/p')
  getv(){ echo "$HS" | tr ' ' '\n' | sed -n "s/^$1=//p"; }
  ND=$(grep -c 'reason=deadline' "$WORK/srv.log"); NP=$(grep -c 'reason=poll-error' "$WORK/srv.log")
  [ "$(getv deadline)" = "$ND" ] && ok "C4 deadline 计数一致（$ND）" || no "C4 deadline 不符：diag=$(getv deadline) log=$ND"
  [ "$(getv pollerr)"  = "$NP" ] && ok "C4 pollerr 计数一致（$NP）"  || no "C4 pollerr 不符：diag=$(getv pollerr) log=$NP"
  [ "$(getv guard)" = "0" ] && ok "C4 guard=0（防御分支未触发，符合预期）" || no "C4 guard≠0"
fi

echo "=== C5 兼容性：旧字段仍在（既有取证/告警口径不破）"
L=$(grep -m1 'tls-handshake-timeout reason=deadline' "$WORK/srv.log")
MISS=""
for f in 'fd=[0-9-]' '停滞=[0-9]+ms' 'extra=[0-9]+' 'hs_inflight=[0-9]+' 'hs_tmo=[0-9]+'; do
  echo "$L" | grep -qE "$f" || MISS="$MISS $f"
done
[ -z "$MISS" ] && ok "C5 fd=/停滞=/extra=/hs_inflight=/hs_tmo= 全在" || no "C5 丢失字段:$MISS"

echo "=== C3 关闭期仍有在途握手 → reason=shutdown"
C3PID=$(slow_client); sleep 0.8
T0=$(date +%s%N); kill -TERM $SRVPID 2>/dev/null
for ((i=0;i<80;i++)); do kill -0 $SRVPID 2>/dev/null || break; sleep 0.1; done
T1=$(date +%s%N); ELMS=$(( (T1-T0)/1000000 )); SRVPID=0
echo "  优雅关闭耗时 ${ELMS}ms"
if grep -q 'reason=shutdown' "$WORK/srv.log"; then
  echo "  $(grep -m1 'reason=shutdown' "$WORK/srv.log")"
  ok "C3 关闭期在途握手标为 shutdown（与真截止可分）"
else
  no "C3 未出现 reason=shutdown"
fi
[ "$ELMS" -lt 2000 ] && ok "C3 有界 join 未被破坏（${ELMS}ms < 2000ms）" || no "C3 优雅关闭 ${ELMS}ms ≥2s"
kill $C3PID 2>/dev/null

echo
echo "=== 结果：PASS=$PASS FAIL=$FAIL SKIP=$SKIP"
[ "$FAIL" = "0" ] && echo "M108_D1_VERIFY_OK" || echo "M108_D1_VERIFY_FAIL"
exit $FAIL
