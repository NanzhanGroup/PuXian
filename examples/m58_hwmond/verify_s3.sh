#!/usr/bin/env bash
# ============================================================
# M58 S3 verify —— 手写 HTTP 状态页 + 阈值告警通知（Err 语义真发）
# ------------------------------------------------------------
# 验证点：
#   A-C. daemon（--port + mmap 快照）运行时，verify_s3_client.px 断言：
#     A. /healthz → 200 + Content-Type: application/json + Connection: close
#        + Content-Length 与 body 字节一致 + JSON 字段齐全（service/status/
#        heartbeat_ms/mem_total/…）【M57-S7 响应头教训自验】
#     B. /       → 200 + text/html + 快照表格（含 mem_avail/temp）
#     C. /nope   → 404
#   D. 阈值告警 + webhook 真发（Err 语义 dogfood 闭环）：
#     D1 成功路径：PXHWMON_ALERT_MEM_AVAIL_KB 巨大值必触发 → 每轮告警日志 +
#        webhook_send 真发到本地 PuXian mock（http_serve）→ mock 实收 3 条
#        POST /alert、报文 JSON 含 alert；发送日志 3 条 sent:true
#     D2 失败路径：webhook 指向 127.0.0.1:1（连接拒绝）→ daemon 不 panic、
#        3 轮跑完退出码 0、发送日志 3 条 sent:false + err（§十三 #1 修复实证）
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
WLOG2="/tmp/m58s3_webhook_fail_$$.jsonl"
MWLOG="/tmp/m58s3_mock_$$.jsonl"
MPID=""

fail() { echo "M58-S3 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }
cleanup() {
    [ -n "$MPID" ] && kill "$MPID" 2>/dev/null
    rm -f "$SHM" "$ALOG" "$WLOG" "$MWLOG" "$WLOG2"
}
trap cleanup EXIT

echo "== build =="
$PXC build "$D/main.px" >/dev/null 2>&1 || fail "main.px 编译失败"
$PXC build "$D/verify_s3_client.px" >/dev/null 2>&1 || fail "client 编译失败"
$PXC build "$D/webhook_mock.px" >/dev/null 2>&1 || fail "webhook_mock.px 编译失败"

echo "== A-C. HTTP 状态页（daemon --port $PORT）=="
"$B/main" --n 20 --interval 1 --port "$PORT" --shm "$SHM" >/tmp/m58s3_daemon.log 2>&1 &
DPID=$!
# 就绪等待：HTTP 端口可连（daemon 冷启动偶发 >2.5s，用探测重试代替固定 sleep）
for _ in $(seq 1 15); do
    if (exec 3<>"/dev/tcp/127.0.0.1/$PORT") 2>/dev/null; then
        exec 3>&- 3<&- 2>/dev/null
        break
    fi
    sleep 0.5
done
# client 断言（偶发连接抖动 → 最多重试 3 次）
COUT=""
CRC=1
for _ in $(seq 1 3); do
    COUT=$(PXHWMON_VERIFY_PORT=$PORT "$B/verify_s3_client" 2>&1)
    CRC=$?
    [ "$CRC" -eq 0 ] && break
    sleep 1
done
echo "$COUT"
[ "$CRC" -eq 0 ] || fail "HTTP 客户端断言失败（退出码 $CRC）"
echo "$COUT" | grep -q "S3-CLIENT OK" || fail "client 未达 OK"
ok "A/B/C: /healthz JSON + / HTML + /nope 404，显式响应头全部正确"
wait "$DPID" || true

echo "== D1. 阈值告警 + webhook 真发（本地 PuXian mock 实收）=="
rm -f "$ALOG"
MPORT=$((26000 + RANDOM % 4000))
MWLOG="/tmp/m58s3_mock_$$.jsonl"
WLOG="/tmp/m58s3_webhook_$$.jsonl"
rm -f "$MWLOG" "$WLOG"
PXHWMON_MOCK_PORT=$MPORT PXHWMON_MOCK_LOG="$MWLOG" "$B/webhook_mock" >/tmp/m58s3_mock.log 2>&1 &
MPID=$!
# 就绪判定：/dev/tcp 端口探测（mock 的 print 到文件是全缓冲，不能等 MOCK-READY 行）
READY=0
for _ in $(seq 1 25); do
    if (exec 3<>"/dev/tcp/127.0.0.1/$MPORT") 2>/dev/null; then
        exec 3>&- 3<&- 2>/dev/null
        READY=1
        break
    fi
    sleep 0.2
done
[ "$READY" -eq 1 ] || fail "webhook mock 未就绪（$(cat /tmp/m58s3_mock.log)）"
PXHWMON_ALERT_MEM_AVAIL_KB=999999999 \
PXHWMON_WEBHOOK="http://127.0.0.1:$MPORT/alert" \
PXHWMON_ALERT_LOG="$ALOG" \
PXHWMON_WEBHOOK_LOG="$WLOG" \
"$B/main" --n 3 --interval 1 --no-http --no-shm >/tmp/m58s3_alerts_run.log 2>&1 || fail "告警冒烟退出非 0"
N=$(wc -l < "$ALOG")
[ "$N" -eq 3 ] || fail "告警日志应 3 行得 $N"
grep -q "type=mem_avail" "$ALOG" || fail "告警日志无 mem_avail 类型"
ok "告警日志 3 行（mem_avail < 999999999 触发）"
MN=$(wc -l < "$MWLOG")
[ "$MN" -eq 3 ] || fail "mock 实收应 3 条得 $MN"
grep -q '"method":"POST"' "$MWLOG" || fail "mock 收到的不是 POST"
grep -q '"path":"/alert"' "$MWLOG" || fail "mock 路径非 /alert"
# mock 把整个请求 body 存为 JSON 字符串字段 → alert 键在文件里是转义形式 \"alert\":\"ALERT
grep -Fq 'alert\":\"ALERT' "$MWLOG" || fail "mock 报文 body 缺 alert（$(head -1 "$MWLOG")）"
ok "mock 实收 3 条 POST /alert（body 含 alert）— 网络真发打通"
WN=$(wc -l < "$WLOG")
[ "$WN" -eq 3 ] || fail "发送日志应 3 行得 $WN"
grep -q '"sent":true' "$WLOG" || fail "发送日志无 sent:true"
ok "发送结果日志 3 条 sent:true（http 2xx 判定正确）"
kill "$MPID" 2>/dev/null; wait "$MPID" 2>/dev/null; MPID=""

echo "== D2. webhook 失败路径：连接拒绝 → daemon 存活（Err 语义）=="
WLOG2="/tmp/m58s3_webhook_fail_$$.jsonl"
rm -f "$WLOG2"
PXHWMON_ALERT_MEM_AVAIL_KB=999999999 \
PXHWMON_WEBHOOK="http://127.0.0.1:1/alert" \
PXHWMON_ALERT_LOG="$ALOG" \
PXHWMON_WEBHOOK_LOG="$WLOG2" \
"$B/main" --n 3 --interval 1 --no-http --no-shm >/tmp/m58s3_alerts_fail.log 2>&1 || fail "失败路径 daemon 被 panic 杀死（退出非 0）"
FN=$(wc -l < "$WLOG2")
[ "$FN" -eq 3 ] || fail "失败路径发送日志应 3 行得 $FN"
grep -q '"sent":false' "$WLOG2" || fail "失败路径无 sent:false"
grep -q '"err":"net:' "$WLOG2" || fail "失败路径缺 err 明细"
ok "连接拒绝 3 次 → 全部 sent:false + err 落盘，daemon 未被 panic 杀（§十三 #1 实证）"

echo "✅ M58-S3: 手写 HTTP 状态页 + 阈值告警通知（webhook 真发 Err 语义）PASS"
