#!/usr/bin/env bash
# ============================================================
# M263 · 「响应串味」定向复现器（可参数化 · 可常设）
# ------------------------------------------------------------
# 判据：每轮起一个**全新进程**的服务端，按固定顺序打 5 个请求，
#   逐个**逐字节**核对应答体（互不相同 ⇒ 答非所问必被抓）；
#   另核对「应答体形状必须与请求路径对应」的**互斥**性
#   （例如 `/list` 的应答里不得出现 `/inv` 独有的 `first=`/`n=`）。
# 用法：bash repro.sh [轮数] [服务端二进制]
# 退出：0=全清 · 1=有异常 · 2=用法/构建问题
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m263_resp_crosstalk
PORT=18363
N=${1:-20}
BIN=${2:-$ROOT/$DIR/build/srv}
W=${M263_W:-/tmp/m263_gate}
mkdir -p "$W"
[ -x "$BIN" ] || { echo "缺二进制：$BIN"; exit 2; }

req() {  # $1=path $2=out
  : > "$2"
  ( exec 3<>/dev/tcp/127.0.0.1/$PORT 2>/dev/null &&
    printf 'GET %s HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n' "$1" >&3 &&
    timeout 25 cat <&3 ) > "$2" 2>/dev/null
  tr -d '\r' < "$2" | tail -1
}

tot=0; anom=0; rounds_ok=0
# 全部异常上报的唯一出口（**单行**：负控 C 用 sed 整行中性化）
flag_anom() { anom=$((anom+1)); fail=1; echo "轮 ${i:-?} ❌ $1"; }
for i in $(seq 1 "$N"); do
  LOG=$W/srv_$i.log
  "$BIN" >"$LOG" 2>&1 &
  PID=$!
  ready=0
  for _ in $(seq 1 80); do grep -q 'M263 READY' "$LOG" 2>/dev/null && { ready=1; break; }; sleep 0.25; done
  if [ "$ready" != 1 ]; then
    flag_anom "启动失败（无 READY）"
    kill -9 "$PID" 2>/dev/null; wait "$PID" 2>/dev/null; continue
  fi
  fail=0
  while IFS='|' read -r pth exp; do
    B=$(req "$pth" "$W/r.in"); tot=$((tot+1))
    if [ "$B" != "$exp" ]; then
      flag_anom "$pth → '$B'（期望 '$exp'）"
    fi
  done <<'CASES'
/health|alive
/list?n=20000|list=20000
/dict?n=20000|dict=20000
CASES
  # `/inv` 是不变量判据（协程追加 ⇒ 长度随调度变化）
  B=$(req "/inv" "$W/inv.in"); tot=$((tot+1))
  case "$B" in
    inv\ first=0\ n=*) : ;;
    *) flag_anom "/inv 不变量破裂：'$B'" ;;
  esac
  # 互斥判据：`/list` 的应答里不得出现 `/inv` 独有形状
  B=$(req "/list?n=20000" "$W/mx.in"); tot=$((tot+1))
  case "$B" in
    list=20000) : ;;
    *first=*|*"n="*|*inv*) flag_anom "/list 应答含 /inv 形状（**串味**）：'$B'" ;;
    *) flag_anom "/list 应答异常：'$B'" ;;
  esac
  [ "$fail" = 0 ] && { rounds_ok=$((rounds_ok+1)); echo "轮 $i ✅"; }
  kill -9 "$PID" 2>/dev/null; wait "$PID" 2>/dev/null
done
echo "──────────────────────────────"
echo "M263-REPRO：轮数=$N 断言数=$tot 全绿轮数=$rounds_ok 异常=$anom"
if [ "$anom" = 0 ]; then echo "M263-REPRO-CLEAN"; exit 0; fi
echo "M263-REPRO-ANOMALY"; exit 1
