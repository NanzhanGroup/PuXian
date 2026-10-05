#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════
# examples/m267_perf/collect_baseline_v2_driver.sh
#   —— 「性能基线 v2 原始数据采集」主流程（t2）
#   流程：N 次独立采集（默认 3）→ 归并 + 重复性自校验 → raw_baseline_v2.json/.md/.candidate.tsv
#   ⚠️ 请在**尽量无干扰**的环境下运行（口径 §4：重定基尽量避开全量门并行）。
#      运行中若检测到并发全量门，会在 env.quiet=false 与 env_summary 中如实登记。
# 用法： bash examples/m267_perf/collect_baseline_v2_driver.sh [N] [outdir]
# ══════════════════════════════════════════════════════════════
set -uo pipefail
cd "$(cd "$(dirname "$0")/../.." && pwd)"
D=examples/m267_perf
N="${1:-3}"
OUTD="${2:-$D}"
W="${M267_W:-/tmp/m267_perf_v2}"
REPS_JSON=()
for i in $(seq 1 "$N"); do
    f="$OUTD/raw_baseline_v2.rep$i.json"
    echo "── 采集 rep$i/$N ──"
    M267_W="$W" bash "$D/collect_baseline_v2.sh" "$f" "rep$i" || { echo "采集 rep$i 失败" >&2; exit 1; }
    REPS_JSON+=("$f")
done
echo "── 归并 + 重复性自校验 ──"
python3 "$D/baseline_v2_json.py" compare "$OUTD/raw_baseline_v2.json" "${REPS_JSON[@]}"
echo "DONE -> $OUTD/raw_baseline_v2.json"
