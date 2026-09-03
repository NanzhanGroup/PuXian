#!/usr/bin/env bash
# ============================================================
# M58 S3 verify —— 手写 HTTP 状态页 + 阈值告警通知
# ------------------------------------------------------------
# 验证点：
#   A-C. daemon（--port + mmap 快照）运行时，verify_s3_client.px 断言：
#     A. /healthz → 200 + Content-Type: application/json + Connection: close
#        + Content-Length 与 body 字节一致 + JSON 字段齐全（service/status/
#        heartbeat_ms/mem_total/…）【M57-S7 响应头教训自验】
#     B. /       → 200 + text/html + 快照表格（含 mem_avail/temp）
#     C. /nope   → 404
#   D. 阈值告警：PXHWMON_ALERT_MEM_AVAIL_KB 巨大值必触发 → 每轮告警日志
#      + webhook dry-run 报文落盘（url/alert/ts JSON 格式正确）
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m58_hwmond/build
D=examples/m58_hwmond
SHM="/tmp/m58s3_verify_$$.shm"
PORT=$((20000 + RANDOM % 5000))
ALOG="/tmp/m58s3_alerts_$$.jsonl"
WLOG="/tmp/m58s3_webhook_$$.jsonl"

fail() { echo "M58-S3 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }
cleanup() { rm -f "$SHM" "$ALOG" "$WLOG"; }
trap cleanup EXIT

echo "== build =="
$PXC build "$D/main.px" >/dev/null 2>&1 || fail "main.px 编译失败"
$PXC build "$D/verify_s3_client.px" >/dev/null 2>&1 || fail "client 编译失败"

echo "== A-C. HTTP 状态页（daemon --port $PORT）=="
"$B/main" --n 15 --interval 1 --port "$PORT" --shm "$SHM" >/tmp/m58s3_daemon.log 2>&1 &
DPID=$!
sleep 2.5
COUT=$(PXHWMON_VERIFY_PORT=$PORT "$B/verify_s3_client" 2>&1)
CRC=$?
echo "$COUT"
[ "$CRC" -eq 0 ] || fail "HTTP 客户端断言失败（退出码 $CRC）"
echo "$COUT" | grep -q "S3-CLIENT OK" || fail "client 未达 OK"
ok "A/B/C: /healthz JSON + / HTML + /nope 404，显式响应头全部正确"
wait "$DPID" || true

echo "== D. 阈值告警 + webhook dry-run =="
rm -f "$ALOG" "$WLOG"
PXHWMON_ALERT_MEM_AVAIL_KB=999999999 \
PXHWMON_WEBHOOK="http://webhook.invalid/alert" \
PXHWMON_ALERT_LOG="$ALOG" \
PXHWMON_WEBHOOK_DRYRUN_LOG="$WLOG" \
"$B/main" --n 3 --interval 1 --no-http --no-shm >/tmp/m58s3_alerts_run.log 2>&1 || fail "告警冒烟退出非 0"
N=$(wc -l < "$ALOG")
[ "$N" -eq 3 ] || fail "告警日志应 3 行得 $N"
grep -q "type=mem_avail" "$ALOG" || fail "告警日志无 mem_avail 类型"
grep -q "threshold=999999999" "$ALOG" || fail "告警日志阈值不符"
ok "告警日志 3 行（mem_avail < 999999999 触发）"
WN=$(wc -l < "$WLOG")
[ "$WN" -eq 3 ] || fail "webhook dryrun 应 3 行得 $WN"
grep -q '"url":"http://webhook.invalid/alert"' "$WLOG" || fail "dryrun 报文缺 url"
grep -q '"alert":"ALERT' "$WLOG" || fail "dryrun 报文缺 alert"
ok "webhook dry-run 3 行（url + alert 报文 JSON 正确）"

echo "✅ M58-S3: 手写 HTTP 状态页 + 阈值告警通知 PASS"
