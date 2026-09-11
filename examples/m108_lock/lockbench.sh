#!/bin/bash
# M108 ①「握手锁争用量化」——黑盒并发度扫描 + 白盒 futex 计时
#
# 指标定义（关键）：
#   服务端 CPU 秒 = (utime+stime)/CLK_TCK   ← /proc/<pid>/stat 第 14/15 字段
#   有效并发度 E  = 服务端 CPU 秒 / 墙钟秒
#     E ≈ 1.0  → 服务端在任意并发下仍被**单点串行**（候选：g_srv_hs_mu 全局握手锁）
#     E → 4.0  → 在分配到的 4 核上真并行
#   无锁理论上限 = min(分配核数, 并发)；全机 8 核上限 = min(8, 并发)
#
# 隔离：服务端 taskset 0-3（4 核），客户端 taskset 4-7（4 核），互不抢核。
# 对照：plain 模式 = 同一套 serve 栈但不握手 → 剥掉「池/事件循环/HTTP 解析」自身成本，
#       与 tls 模式的差额才是「握手 + 握手锁」的净代价。
#
# 输出：/tmp/m108_lockbench.out 摘要 + CSV（label,conc,mode,n,srv_cpu_s,elapsed_s,E,rc）
set -u
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR="$ROOT/examples/m108_lock"
BIN="$DIR/build/m108_lock_daemon"
CLI=/tmp/m108_bench
PORT=18202
CLK=$(getconf CLK_TCK)
CERT=/tmp/px_m108_cert.pem
KEY=/tmp/px_m108_key.pem
SRV_PIN=0-3
CLI_PIN=4-7
N=2400
CONCS="1 2 4 8 16 32 64"
MAXC=256
CSV=/tmp/m108_lock_$(date +%H%M%S).csv
SRV_PID=0

srv_cpu() { awk '{printf "%.4f", ($14+$15)/'"$CLK"'}' /proc/$1/stat 2>/dev/null || echo 0; }

kill_srv() {
    if [ "$SRV_PID" -gt 0 ] 2>/dev/null; then
        kill -9 "$SRV_PID" 2>/dev/null
        wait "$SRV_PID" 2>/dev/null
        SRV_PID=0
    fi
}
trap kill_srv EXIT

echo "=== 准备：证书 + 构建 daemon + Go 客户端 ==="
openssl req -x509 -newkey rsa:2048 -nodes -keyout "$KEY" -out "$CERT" -days 1 \
    -subj "/CN=localhost" -addext "subjectAltName=DNS:localhost" >/dev/null 2>&1
echo "  [1/3] RSA-2048 自签证书 rc=$? size=$(stat -c %s "$CERT" 2>/dev/null)"

"$ROOT/tools/px" build --no-quic "$DIR/m108_lock_daemon.px" >/tmp/m108_build.log 2>&1 \
    || { echo "  FAIL build daemon"; tail -20 /tmp/m108_build.log; exit 1; }
echo "  [2/3] daemon 编译 rc=0 size=$(stat -c %s "$BIN" 2>/dev/null)  $(file -b "$BIN" | cut -c1-70)"

CGO_ENABLED=0 go build -o "$CLI" "$DIR/go_bench.go" >/tmp/m108_gobuild.log 2>&1 \
    || { echo "  FAIL go build"; cat /tmp/m108_gobuild.log; exit 1; }
echo "  [3/3] 客户端静态构建 rc=0 size=$(stat -c %s "$CLI" 2>/dev/null)  $(file -b "$CLI" | cut -c1-70)"

start_srv() {  # $1 = mode (0=tls / 1=plain)
    taskset -c "$SRV_PIN" "$BIN" "$PORT" "$CERT" "$KEY" "$MAXC" "$1" \
        >/tmp/m108_srv.log 2>&1 </dev/null &
    SRV_PID=$!
    local i R
    for i in $(seq 1 60); do
        if [ "$1" = "1" ]; then
            R=$(curl -s -m 2 "http://127.0.0.1:$PORT/index.txt" 2>/dev/null)
        else
            R=$(curl -sk -m 2 "https://127.0.0.1:$PORT/index.txt" 2>/dev/null)
        fi
        [ "$R" = "M108-OK" ] && return 0
        kill -0 "$SRV_PID" 2>/dev/null || { echo "  daemon 早退:"; tail -5 /tmp/m108_srv.log; return 1; }
        sleep 0.2
    done
    echo "  未就绪（60×0.2s 超时）"
    return 1
}

run_case() {  # $1=mode $2=conc $3=label
    local mode="$1" conc="$2" label="$3"
    local c0 c1 t0 t1 scpu el E rc out extra=""
    start_srv "$mode" || { kill_srv; return 1; }
    c0=$(srv_cpu "$SRV_PID")
    [ "$mode" = "1" ] && extra="-plain"
    t0=$(date +%s.%N)
    out=$(timeout 120 taskset -c "$CLI_PIN" "$CLI" -addr "127.0.0.1:$PORT" -n "$N" -c "$conc" $extra 2>&1)
    rc=$?
    t1=$(date +%s.%N)
    c1=$(srv_cpu "$SRV_PID")
    el=$(awk -v a="$t1" -v b="$t0" 'BEGIN{printf "%.3f", a-b}')
    scpu=$(awk -v a="$c1" -v b="$c0" 'BEGIN{printf "%.4f", a-b}')
    E=$(awk -v s="$scpu" -v e="$el" 'BEGIN{printf "%.3f", (e>0)? s/e : 0}')
    printf "  %-5s conc=%-3s | %s | srv_cpu=%.3fs elapsed=%.3fs E=%.3f\n" \
        "$label" "$conc" "$out" "$scpu" "$el" "$E"
    echo "$label,$conc,$mode,$N,$scpu,$el,$E,$rc" >> "$CSV"
    kill_srv
    sleep 0.3
}

echo "label,conc,mode,n,srv_cpu_s,elapsed_s,E,rc" > "$CSV"

echo
echo "=== A. tls（完整握手，待测路径；服务端限定 0-3 共 4 核）==="
for c in $CONCS; do run_case 0 "$c" "tls"; done

echo
echo "=== B. plain（同栈无握手，对照基线）==="
for c in $CONCS; do run_case 1 "$c" "plain"; done

echo
echo "=== C. 白盒：服务端 futex 系统调用计时（conc=16，约 4s 窗口）==="
if start_srv 0; then
    taskset -c "$CLI_PIN" "$CLI" -addr "127.0.0.1:$PORT" -n 400000 -c 16 >/tmp/m108_white_cli.out 2>&1 &
    CLIPID=$!
    timeout 8 strace -c -f -p "$SRV_PID" -e trace=futex -o /tmp/m108_futex.txt 2>/tmp/m108_strace.err
    SRC=$?
    kill -9 "$CLIPID" 2>/dev/null; wait "$CLIPID" 2>/dev/null
    if [ -s /tmp/m108_futex.txt ]; then
        echo "  （strace rc=$SRC）服务端 futex 统计："
        sed 's/^/    /' /tmp/m108_futex.txt
    else
        echo "  strace 不可用/被拒（rc=$SRC）：$(head -2 /tmp/m108_strace.err | tr '\n' ' ')"
    fi
    echo "  客户端（该窗内被打断，仅作参照）：$(head -1 /tmp/m108_white_cli.out)"
    kill_srv
fi

echo
echo "=== 汇总 CSV ($CSV) ==="
column -t -s, "$CSV" 2>/dev/null || cat "$CSV"
echo
echo "=== 判读 ==="
awk -F, 'NR>1 { if ($1=="tls" && $2>=8) { if ($7+0 > maxt) maxt=$7+0 } if ($1=="plain" && $2>=8) { if ($7+0 > maxp) maxp=$7+0 } } END {
  printf "  高并发段（conc>=8）峰值有效并发度： tls E=%.2f   plain E=%.2f\n", maxt, maxp
  if (maxt > 0) printf "  → 若 tls 明显受限于 1.0 而 plain 能到多核，则握手（锁）是唯一串行点；\n     无锁理论上限 = min(4, conc)（服务端 4 核），即潜在加速比 ≈ %.1fx（4核内）/ 全机 8 核更高\n", 4.0/maxt
}' "$CSV"
