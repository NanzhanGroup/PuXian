#!/bin/bash
# m89_perf/http_bench_one.sh —— 单档 HTTP 压测（自管理 server 生命周期）
# 用法：http_bench_one.sh <c|vm> <port> <threads> <reqs> [--json]
# 流程：起对应轨 http_json server → 预热 50 请求 → press_http.py 压测 →
#       输出结果 JSON → 杀 server。stdout 单行 JSON。
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
BIN=/tmp/m89perf/bin
TRACK="$1"; PORT="$2"; TH="$3"; REQ="$4"
[ "$TRACK" = c ] && EXE="$BIN/http_json_c" || EXE="$BIN/http_json_vm"

setsid env PERF_PORT="$PORT" "$EXE" </dev/null >/tmp/m89perf/srv_${TRACK}.log 2>&1 &
SRV=$!
sleep 1.5
# 预热 50 请求（丢弃输出）
python3 "$ROOT/examples/m89_perf/press_http.py" "$PORT" --threads 2 --reqs 25 --timeout 15 >/dev/null 2>&1
sleep 0.3
# 正式压测
python3 "$ROOT/examples/m89_perf/press_http.py" "$PORT" --threads "$TH" --reqs "$REQ" --timeout 30
kill -9 "$SRV" 2>/dev/null
wait "$SRV" 2>/dev/null
exit 0
