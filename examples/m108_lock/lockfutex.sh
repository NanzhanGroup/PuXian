#!/bin/bash
# M108 ① 白盒：把 futex 等待**按地址归因**到 g_srv_hs_mu（全局握手串行锁）
#
# 黑盒的「有效并发度 E ≈ 1.0」只能证明**存在**单点串行，不能指认是**哪一把**锁。
# 本脚本用 strace -f -T 抓服务端全部 futex，按「uaddr == &g_srv_hs_mu」
# （静态非 PIE 二进制 → 固定地址，取自 nm 符号）过滤，给出：
#   该锁 FUTEX_WAIT 次数 / 累计等待秒 / 单次均值与最大 / 平均同时阻塞线程数。
# 同时打印「出现等待最多的 5 个 futex 地址」做交叉验证（防归因错人）。
# A/B 对照：plain（同栈但不握手、根本不进这把锁）→ 该地址应近乎 0。
# 附带 C 段：RSA-2048 / EC-P256 单次完整握手的服务端 CPU，及同机 openssl 基线。
set -u
cd "$(dirname "$0")/../.."
ROOT=$(pwd); DIR="$ROOT/examples/m108_lock"
BIN="$DIR/build/m108_lock_daemon"
CLI=/tmp/m108_bench
PORT=18203
CLK=$(getconf CLK_TCK)
CERT=/tmp/px_m108_cert.pem;     KEY=/tmp/px_m108_key.pem
ECCERT=/tmp/px_m108_eccert.pem; ECKEY=/tmp/px_m108_eckey.pem
MAXC=64
CONC=32
SRV_PID=0; SPID=0; MODE=0

srv_cpu() { awk '{printf "%.4f", ($14+$15)/'"$CLK"'}' /proc/$1/stat 2>/dev/null || echo 0; }
srv_thr() { awk '/^Threads:/{print $2}' /proc/$1/status 2>/dev/null || echo 0; }
clean() {
    [ "${SPID:-0}" -gt 0 ] 2>/dev/null && kill -9 "$SPID" 2>/dev/null
    [ "${SRV_PID:-0}" -gt 0 ] 2>/dev/null && kill -9 "$SRV_PID" 2>/dev/null
    wait 2>/dev/null; SRV_PID=0; SPID=0
}
trap clean EXIT

ADDR=$(nm -a "$BIN" 2>/dev/null | awk '$3=="g_srv_hs_mu"{print "0x"$1}')
echo "g_srv_hs_mu 符号地址 = ${ADDR:-<取不到>}（EXEC 非 PIE → 运行期静态固定）"
[ -z "$ADDR" ] && { echo "FAIL: 无法从符号表取地址"; exit 1; }
openssl ecparam -name prime256v1 -genkey -noout -out "$ECKEY" >/dev/null 2>&1
openssl req -new -x509 -key "$ECKEY" -out "$ECCERT" -days 1 -subj "/CN=localhost" \
    -addext "subjectAltName=DNS:localhost" >/dev/null 2>&1
echo "EC P-256 自签证书 size=$(stat -c %s "$ECCERT" 2>/dev/null)"

start_srv() {  # $1=cert $2=key $3=mode(0=tls/1=plain)
    taskset -c 0-3 "$BIN" "$PORT" "$1" "$2" "$MAXC" "$3" >/tmp/m108_fx_srv.log 2>&1 </dev/null &
    SRV_PID=$!
    local i R
    for i in $(seq 1 60); do
        if [ "$3" = "1" ]; then R=$(curl -s -m 2 "http://127.0.0.1:$PORT/index.txt" 2>/dev/null)
        else                     R=$(curl -sk -m 2 "https://127.0.0.1:$PORT/index.txt" 2>/dev/null); fi
        [ "$R" = "M108-OK" ] && return 0
        kill -0 "$SRV_PID" 2>/dev/null || { echo "  daemon 早退:"; tail -5 /tmp/m108_fx_srv.log; return 1; }
        sleep 0.2
    done
    echo "  未就绪"; return 1
}

# 归因规则：
#   entry 行含 "futex(0x…"（用 index 定位，避免 awk 把 "futex(0x" 当正则报错）
#   → 记 pend[pid]=地址；resumed 行取 $NF 的 <秒> 记入该地址。
#   地址规范化（strace 打 0x78d540，nm 打 0x000000000078d540）→ canon() 去前导零后比较。
AWK_ATTR='
function canon(s, t) { t = tolower(s); sub(/^0x/, "", t); while (length(t) > 1 && substr(t, 1, 1) == "0") t = substr(t, 2); return t }
/\(0x/ {
    p = index($0, "futex(0x")
    if (p > 0) {
        rest = substr($0, p + 8)
        split(rest, b, ","); raw = "0x" b[1]; ad = canon(raw)
        split($0, c, "\\[pid "); if (c[2] != "") { split(c[2], d, "]"); pid = d[1] }
        pend[pid] = ad
        if ($0 ~ /FUTEX_WAIT/) cnt[ad]++
        next
    }
}
/futex resumed/ {
    split($0, c, "\\[pid "); if (c[2] != "") { split(c[2], d, "]"); pid = d[1] }
    ad = pend[pid]
    f = $NF; gsub(/[<>]/, "", f)
    if (f + 0 > 0) { w[ad] += f; if (f > mx[ad]) mx[ad] = f }
}
END {
    t = canon(target)
    tc = cnt[t] + 0; tw = w[t] + 0
    printf "   目标锁 %s：FUTEX_WAIT 次数=%d 累计等待=%.3fs 单次均值=%.3fms 单次最大=%.1fms\n",
        target, tc, tw, (tc > 0 ? tw * 1000.0 / tc : 0), mx[t] * 1000
    if (el > 0) printf "   → 平均同时阻塞在该锁上的线程数 = %.1f（累计等待/墙钟）\n", tw / el
    printf "   交叉验证｜窗口内出现 FUTEX_WAIT 最多的 5 个地址：\n"
    n = 0
    for (k in cnt) { n++; ks[n] = k }
    for (i = 1; i <= n; i++) for (j = i + 1; j <= n; j++) if (cnt[ks[j]] > cnt[ks[i]]) { x = ks[i]; ks[i] = ks[j]; ks[j] = x }
    tot = 0; tw2 = 0; for (k in w) tw2 += w[k]; for (k in cnt) tot += w[k]
    for (i = 1; i <= (n < 5 ? n : 5); i++) {
        k = ks[i]
        printf "     %-20s WAIT=%d 累计=%.3fs%s\n", "0x" k, cnt[k], w[k] + 0, (k == t ? "   ← 目标锁" : "")
    }
    printf "   窗口内全部 futex 累计等待=%.1fs（含线程池空闲等待，仅作参照）\n", tw2
}'

attrib() {  # $1=标签 $2=连接数
    local tag="$1" n="$2" c0 c1 t0 t1 el sc extra="" out
    [ "$MODE" = "1" ] && extra="-plain"
    strace -f -T -e trace=futex -o /tmp/m108_fx.txt -p "$SRV_PID" 2>/dev/null &
    SPID=$!
    sleep 0.6
    c0=$(srv_cpu "$SRV_PID"); t0=$(date +%s.%N)
    out=$(timeout 60 taskset -c 4-7 "$CLI" -addr "127.0.0.1:$PORT" -n "$n" -c "$CONC" $extra 2>&1)
    t1=$(date +%s.%N); c1=$(srv_cpu "$SRV_PID")
    sleep 0.25; kill -9 "$SPID" 2>/dev/null; wait "$SPID" 2>/dev/null; SPID=0
    el=$(awk -v a="$t1" -v b="$t0" 'BEGIN{printf "%.3f", a-b}')
    sc=$(awk -v a="$c1" -v b="$c0" 'BEGIN{printf "%.4f", a-b}')
    printf "── %s：线程数=%s 墙钟 %ss 服务端CPU %ss E=%.3f\n     %s\n" "$tag" \
        "$(srv_thr "$SRV_PID" 2>/dev/null || echo '?')" "$el" "$sc" \
        "$(awk -v s="$sc" -v e="$el" 'BEGIN{printf "%.3f",(e>0)?s/e:0}')" "$out"
    awk -v target="$ADDR" -v el="$el" "$AWK_ATTR" /tmp/m108_fx.txt
}

echo; echo "=== B1. tls/RSA-2048：futex 按地址归因（max_conn=$MAXC conc=$CONC）==="
if start_srv "$CERT" "$KEY" 0; then MODE=0; attrib "tls/RSA" 400; fi
kill -9 "$SRV_PID" 2>/dev/null; wait "$SRV_PID" 2>/dev/null; SRV_PID=0; sleep 0.3

echo; echo "=== B2. plain（对照：同栈但不进这把锁）==="
if start_srv "$CERT" "$KEY" 1; then MODE=1; attrib "plain" 4000; fi
kill -9 "$SRV_PID" 2>/dev/null; wait "$SRV_PID" 2>/dev/null; SRV_PID=0; sleep 0.3

echo; echo "=== B3. tls/EC-P256：同一把锁？ ==="
if start_srv "$ECCERT" "$ECKEY" 0; then MODE=0; attrib "tls/EC" 800; fi
kill -9 "$SRV_PID" 2>/dev/null; wait "$SRV_PID" 2>/dev/null; SRV_PID=0; sleep 0.3

echo; echo "=== C. 单次完整握手的服务端 CPU（n=1200 conc=8，任务跑完再读计数）==="
cpu_per_hs() {  # $1=cert $2=key $3=mode $4=标签
    local c0 c1 sc out
    start_srv "$1" "$2" "$3" || { kill -9 "$SRV_PID" 2>/dev/null; SRV_PID=0; return 1; }
    c0=$(srv_cpu "$SRV_PID")
    out=$(timeout 120 taskset -c 4-7 "$CLI" -addr "127.0.0.1:$PORT" -n 1200 -c 8 \
        $([ "$3" = "1" ] && echo -plain) 2>&1)
    c1=$(srv_cpu "$SRV_PID")
    sc=$(awk -v a="$c1" -v b="$c0" 'BEGIN{printf "%.4f", a-b}')
    printf "  %-10s %s | 服务端CPU=%.3fs/1200 = %.3f ms/次\n" "$4" "$out" "$sc" \
        "$(awk -v s="$sc" 'BEGIN{printf "%.3f", s*1000/1200}')"
    kill -9 "$SRV_PID" 2>/dev/null; wait "$SRV_PID" 2>/dev/null; SRV_PID=0; sleep 0.3
}
cpu_per_hs "$CERT" "$KEY" 0 "RSA-2048"
cpu_per_hs "$ECCERT" "$ECKEY" 0 "EC-P256"
cpu_per_hs "$CERT" "$KEY" 1 "plain"
echo
echo "同机同核（0-3）openssl 基线（仅私钥运算，不含协议栈）："
taskset -c 0-3 openssl speed -seconds 1 rsa2048 2>/dev/null | grep -E 'rsa2048|sign/s' | sed 's/^/  /'
taskset -c 0-3 openssl speed -seconds 1 ecdsap256 2>/dev/null | grep -E 'nistp256|sign/s' | sed 's/^/  /'
