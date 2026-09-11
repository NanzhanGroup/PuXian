#!/bin/sh
# M110-S2 协作式安全点验证：同一压测在「软屏蔽（默认）」与「真屏蔽（PX_GS_HARD=1）」下
#   结果与耗时应同档。若安全点失效 → 软屏蔽侧会反复命中 5s 兜底跳过 → 耗时会爆炸。
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$(pwd)
OUT=/tmp/m110/s2verify
mkdir -p "$OUT"
SRC=examples/m89_s3d/vm_conc_gc_stress.px
BIN=$OUT/vm_conc_gc_stress
echo "── 构建压测（VM 轨）"
"$ROOT/tools/px" build "$SRC" > "$OUT/build.log" 2>&1 || { echo "构建失败"; tail -5 "$OUT/build.log"; exit 1; }
GEN=examples/m89_s3d/build/vm_conc_gc_stress
if [ -f "$GEN" ] && [ "$GEN" -nt "$BIN" ]; then cp "$GEN" "$BIN"; fi
[ -x "$BIN" ] || { echo "缺产物"; ls -la "$ROOT/examples/m89_s3d/build" 2>/dev/null; exit 1; }

run_one() {
    tag=$1; shift
    t0=$(date +%s.%N)
    env "$@" PX_GC_THRESHOLD=15000 "$BIN" > "$OUT/$tag.out" 2>&1
    rc=$?
    t1=$(date +%s.%N)
    dt=$(awk "BEGIN{printf \"%.2f\", $t1-$t0}")
    echo "  $tag: rc=$rc wall=${dt}s out=[$(tail -1 "$OUT/$tag.out" | head -c 80)]"
}
echo "── 软屏蔽（默认，M110-S2）"
run_one soft
echo "── 真屏蔽（PX_GS_HARD=1，旧路径）"
run_one hard PX_GS_HARD=1
echo "── 并发 GC 观测（PX_GC_DEBUG=1，看兜底跳过）"
env PX_GC_DEBUG=1 PX_GC_THRESHOLD=15000 "$BIN" > "$OUT/dbg.out" 2>&1
echo "  兜底跳过次数（g_gc_skips 累计，应为 0）: $(grep -c '跳过 [1-9]' "$OUT/dbg.out")"
echo "  单轮最大暂停耗时(ms): $(grep -o '暂停+标记+扫栈耗时[0-9]*' "$OUT/dbg.out" | sed 's/.*耗时//' | sort -n | tail -1)"
echo "── 结果一致性"
if diff -q "$OUT/soft.out" "$OUT/hard.out" > /dev/null; then echo "  soft vs hard 输出逐字节一致 ✅"; else echo "  ❌ 输出不一致"; diff "$OUT/soft.out" "$OUT/hard.out"; fi
