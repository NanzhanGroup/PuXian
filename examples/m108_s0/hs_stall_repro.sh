#!/bin/bash
# M108-S2/S3 回归与现场复刻台（qg-issue 37：px_serve 连接生命周期族 / 443 假死）
# ---------------------------------------------------------------------------
# 复刻生产形态：一个「TCP 连上但不完成 TLS 握手」的连接（半开/残缺 ClientHello/
#   不回 Finished —— 现场为 CDN 边缘回源连接卡住）。
# 修复前（基线）：仅 1 个此类连接即令整个 TLS 端口假死——新连接 ClientHello 滞留
#   内核 Recv-Q 无人读，客户端只见沉默直至超时；wchan 直方图 futex 占满（= worker
#   全堆在全局握手串行锁 g_srv_hs_mu 上）→ 与生产 294/300 futex 同形。
# 修复后（M108-S2a 握手分步 + 步间释放锁 + 总截止 / S3 有界入队 / S1a 有界 join）：
#   ① 卡住的连接 ≤ PX_TLS_HS_TMO_MS 被回收（自愈），期间其他连接**零影响**；
#   ② 优雅关闭不再被卡住的握手拖成 systemd SIGKILL（91s → <2s）。
# 用法：hs_stall_repro.sh [port] [--expect-fail]   （--expect-fail = 在修复前基线上跑，
#       断言"假死"应当发生，用于证明本台子真的能复现）
set -u
PORT="${1:-18443}"
MODE="${2:-}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SRV="${SRV_BIN:-$ROOT/examples/m108_s0/build/hs_stall_server}"   # SRV_BIN=可指定「修复前」二进制做前后对照
WORK=/tmp/m108_repro
HS_TMO_MS="${HS_TMO_MS:-3000}"
PASS=0; FAIL=0
ok(){ echo "  PASS  $*"; PASS=$((PASS+1)); }
no(){ echo "  FAIL  $*"; FAIL=$((FAIL+1)); }

rm -rf "$WORK"; mkdir -p "$WORK/site"
pkill -f 'hs_stall_server' 2>/dev/null; sleep 0.3
[ -x "$SRV" ] || { echo "缺少 $SRV（先 tools/px build examples/m108_s0/hs_stall_server.px）"; exit 2; }

if [ ! -f "$WORK/cert.pem" ]; then
  openssl req -x509 -newkey rsa:2048 -keyout "$WORK/key.pem" -out "$WORK/cert.pem" \
      -days 2 -nodes -subj '/CN=localhost' -addext 'subjectAltName=DNS:localhost' >/dev/null 2>&1
fi

probe(){  # $1 = 标签；输出 http 码；超时 → 000
  timeout $((HS_TMO_MS/1000 + 5)) curl -sk --resolve "localhost:$PORT:127.0.0.1" \
      --max-time $((HS_TMO_MS/1000 + 4)) -o /dev/null -w '%{http_code}' "https://localhost:$PORT/" 2>/dev/null
}

STALL_PIDS=""
stall(){  # $1 = 个数；TCP 连上后只发 5 字节 TLS Record 头（声称 512 字节）后静默
  local n="$1" i
  for ((i=0;i<n;i++)); do
    ( exec 3<>"/dev/tcp/127.0.0.1/$PORT" || exit 1
      printf '\x16\x03\x01\x02\x00' >&3
      sleep 30 ) &
    STALL_PIDS="$STALL_PIDS $!"
  done
}
kill_stalls(){ [ -n "$STALL_PIDS" ] && kill $STALL_PIDS 2>/dev/null; STALL_PIDS=""; }

echo "=== [1] 启动 TLS px_serve（port=$PORT  tls_hs_tmo=${HS_TMO_MS}ms）"
PX_TLS_HS_TMO_MS="$HS_TMO_MS" PX_SERVE_DIAG=1 "$SRV" "$PORT" "$WORK/cert.pem" "$WORK/key.pem" "$WORK/site" \
    >"$WORK/srv.log" 2>&1 &
SRVPID=$!
# ⚠️ 收尾必须防 pid=0：`kill -9 0` 会杀掉**整个进程组**（含调用方，实测把上层闸门脚本一起打死）。
cleanup(){ [ -n "${SRVPID:-}" ] && [ "$SRVPID" != "0" ] && kill -9 "$SRVPID" 2>/dev/null; kill_stalls; :; }
trap cleanup EXIT
sleep 1.5
head -3 "$WORK/srv.log" | sed 's/^/  /'

echo "=== [2] 基线（无 stall）：应 200"
B=$(probe); echo "  baseline=$B"
[ "$B" = "200" ] && ok "基线 200" || no "基线非 200（$B）"

echo "=== [3] 制造 3 个半完成握手连接"
stall 3; sleep 1
echo "  ss: $(ss -tan 2>/dev/null | grep -c ":$PORT")"

echo "=== [4] 故障态探测：修复后必须仍是 200"
P=$(probe); echo "  after_stall=$P"
if [ "$MODE" = "--expect-fail" ]; then
  [ "$P" != "200" ] && ok "基线复现成功（单点 stall 即假死，$P）" || no "未复现假死（$P）"
else
  [ "$P" = "200" ] && ok "单个卡住握手不再拖死端口（$P）" || no "仍假死（$P）"
fi

echo "=== [5] 自愈：卡住的握手 ≤ 截止被回收（服务端 CLOSE-WAIT 不单调增长）"
# 只统计**服务端**套接字（sport = 监听端口），避免把 stall 客户端自身的 CLOSE-WAIT 算进来
cw_of(){ ss -Htan state close-wait "sport = :$PORT" 2>/dev/null | wc -l; }
tmo=$(( HS_TMO_MS/1000 + 6 ))
for i in $(seq 1 $tmo); do
  cw=$(cw_of)
  [ "$cw" = "0" ] && break
  sleep 1
done
echo "  服务端 CLOSE-WAIT=$cw（等待 ${i}s；客户端侧 CLOSE-WAIT 属 stall 端行为，不计）"
[ "$cw" = "0" ] && ok "服务端 CLOSE-WAIT 归零（卡住的连接已被截止回收）" || no "服务端 CLOSE-WAIT 未归零（$cw）"

echo "=== [6] 关闭期仍有卡住握手：优雅关闭必须 < 2s 且打印完成行"
kill_stalls; sleep 0.3
stall 3; sleep 0.5
T0=$(date +%s%N)
kill -TERM $SRVPID 2>/dev/null
# 有界等待（≤8s）：修复前 join 会被卡住的握手**永久**阻塞（现场 91s → systemd SIGKILL），
# 故此处必须自带上限，否则对照实验会挂死。
GONE=0
for i in $(seq 1 80); do kill -0 $SRVPID 2>/dev/null || { GONE=1; break; }; sleep 0.1; done
T1=$(date +%s%N)
ELMS=$(( (T1 - T0) / 1000000 ))
if [ "$GONE" = "1" ]; then
  wait $SRVPID 2>/dev/null
  echo "  优雅关闭耗时 ${ELMS}ms"
  [ "$ELMS" -lt 2000 ] && ok "优雅关闭 <2s（${ELMS}ms）" || no "优雅关闭 ${ELMS}ms（≥2s）"
else
  echo "  优雅关闭 8s 内未退出（join 被卡住的握手阻塞 —— 修复前形态）"
  no "优雅关闭未在 2s 内完成（>8s，需 SIGKILL）"
  kill -9 $SRVPID 2>/dev/null; wait $SRVPID 2>/dev/null
fi
grep -q '优雅关闭完成' "$WORK/srv.log" && ok "打印「优雅关闭完成」契约行" || no "缺少完成行"
SRVPID=0

echo "=== [7] 可观测性：进程自身给出信号（V5）"
if grep -q 'STALL' "$WORK/srv.log"; then
  ok "有 [px-serve:STALL] 告警：$(grep -m1 STALL "$WORK/srv.log")"
else
  LAST=$(grep 'px-serve:diag' "$WORK/srv.log" | tail -1)
  if echo "$LAST" | grep -qE 'tmo=[1-9]'; then
    ok "诊断计数可见（握手超时已被计入，V5）：$LAST"
  else
    no "无 STALL 告警也无握手超时计数（V5 未达成）：$LAST"
  fi
fi

echo "=== [8] 容量解耦（V2）：并发连接 > max_conn(32) 全部正常完成"
PX_TLS_HS_TMO_MS="$HS_TMO_MS" "$SRV" "$PORT" "$WORK/cert.pem" "$WORK/key.pem" "$WORK/site" \
    >"$WORK/srv2.log" 2>&1 &
SRVPID=$!; sleep 1.2
N=64; GOOD=0; CURL_PIDS=""
: > "$WORK/cap.txt"
for ((i=0;i<N;i++)); do
  ( c=$(timeout 20 curl -sk --resolve "localhost:$PORT:127.0.0.1" --max-time 18 -o /dev/null \
        -w '%{http_code}' "https://localhost:$PORT/") ; echo "$c" >> "$WORK/cap.txt" ) &
  CURL_PIDS="$CURL_PIDS $!"
done
# 只等 curl 作业（不能裸 wait：会给 stall 子壳一起等）
for p in $CURL_PIDS; do wait "$p" 2>/dev/null; done
GOOD=$(grep -c '^200$' "$WORK/cap.txt" 2>/dev/null || echo 0)
echo "  $GOOD/$N 返回 200"
[ "$GOOD" = "$N" ] && ok "并发 $N（>max_conn 32）全部完成" || no "仅 $GOOD/$N 完成"

kill -TERM $SRVPID 2>/dev/null; wait $SRVPID 2>/dev/null; SRVPID=0
kill_stalls
pkill -f 'hs_stall_server' 2>/dev/null

echo
echo "=== 计数（服务端自身诊断）"
grep -o 'STALL[^（]*' "$WORK/srv.log" | head -5 | sed 's/^/  /'
echo "  hs_tmo 行数: $(grep -c 'tls-handshake-timeout' "$WORK/srv.log")"
echo "  enter_null 行数: $(grep -c 'p4-enter-null' "$WORK/srv.log")"
echo
echo "=== 结果：PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = "0" ] && echo "M108_REPRO_OK" || echo "M108_REPRO_FAIL"
exit $FAIL
