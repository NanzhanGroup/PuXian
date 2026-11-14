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
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
# （M404：原 CN=compiler_new 前提已移除，改用 tools/px build 自备构建）
RT="$ROOT/runtime"
DIR="$ROOT/examples/m89_s3d"
OUT=/tmp/m89_s3d_verify
mkdir -p "$OUT"
# M404（缺陷 768）：不再要求 compiler_new / 手工 rtcache —— 构建前提由 tools/px build 自备。
#   旧版在缺 CN 时 `exit 0`（SKIP 判绿 = 零覆盖）；本机有残留产物时才真跑，属于「依赖残留」。
# （M404：$CN 变量已随 compiler_new 前提一并移除 —— 本门不再引用任何非仓库产物）   # 保留变量名以免其它段引用报 unbound（set -u）

# ── M404（缺陷 768）：**自备构建前提** ──────────────────────────────────────
#   病史：本门原先要求 selfhost/build/compiler_new（M89/M90 期**手工重链**的 VM 轨编译器，
#   不在仓库里、devbuild.sh 也不生成）⇒ 缺它就 SKIP（exit 0）；本机恰好有残留产物时才真跑，
#   而那个残留是 06:14 编译留下的 —— 正是 M304「门绿不绿取决于谁的机器上有什么残留」同款。
#   现改为**仓库自带工具链**：tools/px build --no-quic（VM 轨）⇒ 前提自备、可复现。
emit_and_link() { # $1=px文件 $2=输出名 → $OUT/<输出名>（VM 轨：tools/px build）
    local src="$1" name="$2" bin
    "$PX" build --no-quic "$src" > "$OUT/${name}.build.log" 2>&1 || {
        echo "  ❌ build 失败: $src"; tail -5 "$OUT/${name}.build.log"; return 1; }
    bin="$(dirname "$src")/build/$(basename "$src" .px)"
    [ -x "$bin" ] || { echo "  ❌ 产物不存在: $bin"; return 1; }
    cp "$bin" "$OUT/${name}" || return 1
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

echo "── 4) VM 轨堆回落（4 波垃圾 + 空转 gc() → 最小 RSS 回落基线；对齐 issue28-B2 断言）"
emit_and_link "$DIR/wave_settle.px" wave || { FAIL=$((FAIL+1)); }
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
# M404（缺陷 769）：判据从「某瞬间的 RSS」改成「窗口内**最小** RSS」。
#   实测（16 核本机，8 波轨迹）：gc() 之后 RSS 在两波之间 22MB ⇄ 272MB 交替，
#   旧版在 W3-DONE 后固定 sleep 0.3s 采一次 ⇒ 采到 272MB 就判「未回落」，
#   这是**相位运气**，不是产品行为：同一进程在最近 2.5s 内确实降到过 22MB。
#   契约是「gc() 后空 slab 页归还 OS」⇒ 该判「有没有降下来」，不该判「某一刻有多高」。
rss_min_window() { # $1=窗口秒数 → stdout=窗口内最小 VmRSS(KB)；无有效读数 → 0
    local dur="$1" n i v min=99999999 cnt=0
    n=$(awk -v d="$dur" 'BEGIN{printf "%d", (d/0.05)+1}')
    for i in $(seq 1 "$n"); do
        v=$(rss_kb)
        if [ -n "$v" ]; then cnt=$((cnt+1)); [ "$v" -lt "$min" ] && min=$v; fi
        sleep 0.05
    done
    if [ "$cnt" -eq 0 ]; then echo 0; else echo "$min"; fi
}
ok=1
wait_marker W-BASELINE || { echo "  FAIL wave 未启动"; ok=0; }
sleep 1.0; BASE=$(rss_kb)
for w in 1 2 3 4; do
    wait_marker W${w}-DONE || { echo "  FAIL W${w}"; ok=0; }
done
# 判定点 = 末尾「空转 gc()」之后（RSS 相位见 wave_settle.px 头注释：奇数波后仍高，属相位不是泄漏）
wait_marker W-GC2 || { echo "  FAIL 未到 W-GC2（夹具未跑完）"; ok=0; }
IDLE=$(rss_min_window 2.0)
wait_marker W-END || true
kill $WPID 2>/dev/null; wait $WPID 2>/dev/null
# M404（缺陷 772）：**等采样器把峰值落盘**。原版紧跟 `wait $WPID` 就读 wave.peak，
#   而采样器是在 `kill -0` 循环里发现自己被杀后才写盘（≥ 一个 0.04s 周期）⇒ 读到空文件 ⇒ peak=0
#   ⇒ 条件②（min < 2/3 peak）恒假 ⇒ 假红。实测：M404 全量门 shard3 与隔离复现各一次，
#   形态完全一致（base≈4MB · min≈21.7MB · peak=0）。poll 到文件非空再读（3s 上限），不靠时序运气。
SPIN=0
while [ ! -s "$OUT/wave.peak" ] && [ "$SPIN" -lt 150 ]; do sleep 0.02; SPIN=$((SPIN + 1)); done
wait $SPID 2>/dev/null || true
PEAKV=$(cat "$OUT/wave.peak" 2>/dev/null || echo 0)
echo "  RSS: base=${BASE}KB min_after_gc=${IDLE}KB peak=${PEAKV}KB"
[ "${PEAKV:-0}" -gt 0 ] || echo "  ❌ 测量失败：峰值采样未落盘（wave.peak 空/为 0）—— 这是采样问题，不是产品判据"
# 双条件：① 最小值回到基线 +40MB 内（页确实归还了 OS）；
#         ② 最小值显著低于峰值（< 2/3）—— 负控：若 munmap 失效，min ≈ peak，此项必红。
if [ $ok = 1 ] && [ -n "${BASE:-}" ] && [ -n "${IDLE:-}" ] && [ "$IDLE" -lt $((BASE + 40960)) ] \
   && [ "${PEAKV:-0}" -gt 0 ] && [ "$IDLE" -lt $((PEAKV * 2 / 3)) ]; then
    echo "  PASS VM 堆回落（窗口最小 RSS 距基线 < 40MB，且 < 峰值 2/3）"; PASS=$((PASS+1));
else
    echo "  FAIL VM 堆回落（base=$BASE min=$IDLE peak=$PEAKV）"; FAIL=$((FAIL+1));
fi

echo "── 5) VM 并发 GC × 生成器/容器 混合压测（D1+D2a 组合 ×3，须全 PASS）"
emit_and_link "$DIR/vm_conc_gen_stress.px" cgengc || { FAIL=$((FAIL+1)); }
for i in 1 2 3; do
    timeout 90 env PX_GC_THRESHOLD=20000 "$OUT/cgengc" >"$OUT/cgengc.$i.out" 2>&1
    rc=$?
    if [ $rc -eq 0 ] && grep -q "VM-CONC-GEN-STRESS PASSED" "$OUT/cgengc.$i.out" \
       && ! grep -q " FAILED bad=" "$OUT/cgengc.$i.out"; then
        echo "  PASS 并发GC×生成器混合 第${i}轮"; PASS=$((PASS+1));
    else
        echo "  FAIL 并发GC×生成器混合 第${i}轮（rc=$rc）"; tail -3 "$OUT/cgengc.$i.out"; FAIL=$((FAIL+1));
    fi
done

echo "════════ M89-S3-D verify：PASS=$PASS FAIL=$FAIL ════════"
[ $FAIL -eq 0 ]
