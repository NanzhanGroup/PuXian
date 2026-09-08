#!/usr/bin/env bash
# ============================================================
# M89-S3-D verify.sh —— VM 并发 GC 根面 + 生成器标记 压测
# ------------------------------------------------------------
# 内容：
#   1) vm_spawn_smoke.px       VM 轨 spawn 最小冒烟（前置门）
#   2) vm_conc_gc_stress.px    VM 并发 GC 压测（S3-D-1：跨线程帧根）：
#        PX_GC_THRESHOLD=20000（低阈值高频并发 GC 放大窗口）。
#        修复前（并发 GC 不标跨线程 VM 帧槽）3/3 segfault；修复后稳定 PASS。
#   3) gen_gc_stress.px        生成器子对象标记压测（S3-D-2：PX_GEN 递归标记 +
#        显式 gc() 回归护栏；保守扫栈掩盖下 A/B 不区分，作防回归用）
# 退出码：0=全 PASS；非 0=有失败。
# 前置：selfhost/build/compiler_new + 最新 rtcache（含 vm.o，跑过 px build）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
CN="$ROOT/selfhost/build/compiler_new"
RT="$ROOT/runtime"
DIR="$ROOT/examples/m89_s3d"
OUT=/tmp/m89_s3d_verify
mkdir -p "$OUT"

[ -x "$CN" ] || { echo "❌ 缺 selfhost/build/compiler_new" >&2; exit 1; }
CACHE=""
for d in $(ls -dt "$ROOT"/.rtcache/*/ 2>/dev/null); do
    [ -f "$d/.complete" ] && [ -f "$d/vm.o" ] && CACHE="$d" && break
done
[ -n "$CACHE" ] || { echo "❌ 未找到含 vm.o 的 rtcache" >&2; exit 1; }
echo "── rtcache: $CACHE"

emit_and_link() { # $1=px文件 $2=输出名 → /tmp/<out>（VM 轨：compiler_new emit-c → gcc 链 rtcache）
    local src="$1" name="$2"
    ( ulimit -v 10000000; "$CN" --emit-c "$src" > "$OUT/${name}.c" 2>"$OUT/${name}.emit.log" ) || {
        echo "  ❌ emit-c 失败: $src"; tail -3 "$OUT/${name}.emit.log"; return 1; }
    gcc -c -O2 -I"$CACHE" -I"$RT" "$OUT/${name}.c" -o "$OUT/${name}.o" 2>"$OUT/${name}.cc.log" || {
        echo "  ❌ C 编译失败: $src"; tail -5 "$OUT/${name}.cc.log"; return 1; }
    local objs="" f
    for f in "$CACHE"/*.o; do objs="$objs $f"; done
    gcc -O2 -pthread -o "$OUT/${name}" "$OUT/${name}.o" $objs \
        "$RT/third_party/sqlite3/sqlite3.o" \
        "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
        "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
        "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
        "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread || return 1
    return 0
}

PASS=0; FAIL=0
chk() { # $1=结果描述 $2=实际rc $3=期望rc(0=成功)
    if [ "$2" = "$3" ] && [ "$2" = "0" ]; then PASS=$((PASS+1)); echo "  PASS $1";
    else FAIL=$((FAIL+1)); echo "  FAIL $1（rc=$2 期望 $3）"; fi
}

echo "── 1) VM spawn 冒烟"
emit_and_link "$DIR/vm_spawn_smoke.px" smoke && timeout 60 "$OUT/smoke" >"$OUT/smoke.out" 2>&1
chk "vm_spawn_smoke（rc=0 且 PASSED）" "$?" "0"
grep -q "VM-SPAWN-SMOKE PASSED" "$OUT/smoke.out" 2>/dev/null || { FAIL=$((FAIL+1)); echo "  FAIL smoke 输出缺 PASSED"; }

echo "── 2) VM 并发 GC 压测（PX_GC_THRESHOLD=20000 ×3，须全 PASS）"
emit_and_link "$DIR/vm_conc_gc_stress.px" concgc || { FAIL=$((FAIL+1)); }
for i in 1 2 3; do
    timeout 90 env PX_GC_THRESHOLD=20000 "$OUT/concgc" >"$OUT/concgc.$i.out" 2>&1
    rc=$?
    if [ $rc -eq 0 ] && grep -q "VM-CONCURRENT-GC-STRESS PASSED" "$OUT/concgc.$i.out"; then
        echo "  PASS 并发GC压测 第${i}轮"; PASS=$((PASS+1));
    else
        echo "  FAIL 并发GC压测 第${i}轮（rc=$rc）"; tail -2 "$OUT/concgc.$i.out"; FAIL=$((FAIL+1));
    fi
done

echo "── 3) 生成器 GC 压测（显式 gc() 回归护栏）"
emit_and_link "$DIR/gen_gc_stress.px" gengc || { FAIL=$((FAIL+1)); }
timeout 60 "$OUT/gengc" >"$OUT/gengc.out" 2>&1
rc=$?
if [ $rc -eq 0 ] && grep -q "GEN-GC-STRESS PASSED" "$OUT/gengc.out"; then
    echo "  PASS gen_gc_stress"; PASS=$((PASS+1));
else
    echo "  FAIL gen_gc_stress（rc=$rc）"; tail -3 "$OUT/gengc.out"; FAIL=$((FAIL+1));
fi

echo "── 4) VM 轨堆回落（3 波垃圾 → RSS 回落基线；对齐 issue28-B2 断言）"
emit_and_link "$ROOT/examples/issue28_b2/wave.px" wave || { FAIL=$((FAIL+1)); }
LOG="$OUT/wave.log"; rm -f "$LOG"
"$OUT/wave" >"$LOG" 2>&1 &
WPID=$!
rss_kb() { awk '/VmRSS/{print $2}' "/proc/$WPID/status" 2>/dev/null; }
PEAK=0
( while kill -0 $WPID 2>/dev/null; do
      V=$(rss_kb)
      [ -n "$V" ] && [ "$V" -gt "$PEAK" ] 2>/dev/null && PEAK=$V
      sleep 0.04
  done; echo "$PEAK" > "$OUT/wave.peak" ) &
SPID=$!
wait_marker() { for i in $(seq 1 300); do grep -q "$1" "$LOG" 2>/dev/null && return 0; sleep 0.1; done; return 1; }
ok=1
wait_marker W-BASELINE || { echo "  FAIL wave 未启动"; ok=0; }
sleep 1.0; BASE=$(rss_kb)
for w in 1 2 3; do
    wait_marker W${w}-DONE || { echo "  FAIL W${w}"; ok=0; }
    sleep 0.3; eval R${w}=$(rss_kb)
done
wait_marker W-END || true
kill $WPID 2>/dev/null; wait $WPID 2>/dev/null
PEAKV=$(cat "$OUT/wave.peak" 2>/dev/null || echo 0)
IDLE=$R3
echo "  RSS: base=${BASE}KB R1=$R1 R2=$R2 R3=$IDLE peak=${PEAKV}KB"
if [ $ok = 1 ] && [ -n "$BASE" ] && [ -n "$IDLE" ] && [ "$IDLE" -lt $((BASE + 40960)) ]; then
    echo "  PASS VM 堆回落（3 波后 RSS 距基线 < 40MB）"; PASS=$((PASS+1));
else
    echo "  FAIL VM 堆回落（base=$BASE idle=$IDLE）"; FAIL=$((FAIL+1));
fi

echo "════════ M89-S3-D verify：PASS=$PASS FAIL=$FAIL ════════"
[ $FAIL -eq 0 ]
