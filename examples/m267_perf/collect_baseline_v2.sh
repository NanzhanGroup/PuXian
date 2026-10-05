#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════
# examples/m267_perf/collect_baseline_v2.sh
#   —— 按 docs/PERF_BASELINE_METRICS_SPEC.md 采集「性能基线 v2」原始数据（**单次采集**）
# --------------------------------------------------------------
# 与 verify.sh 的关系：
#   · verify.sh = **门**（相对 baseline.tsv 判红，只报比值，不留原始轮次）；
#   · 本脚本   = **采集器**（照口径跑满轮次，把**每轮原始耗时**与环境/身份/锚点
#                一并留档，产出结构化 JSON，供「重定基」固化基线用）。
# 口径来源（一律照抄，不自行发明）：
#   run      : 钉核 `taskset -c 3` · 预热 1 次 · **5 轮** · 统计量 = **min**（保留每轮原值）
#   startup  : 空程序**连跑 200 次**测总墙钟 · 统计量 = 总时间/200（单位 s/次）
#   build    : **2 次**热构建预热 · 计时 **1 次**热构建 · 统计量 = 单次墙钟
#   产物身份 : C 轨 `fn_*` ≥ 1 / VM 轨 `fn_*` == 0（**强制**，防 M266 静默腐烂）
#   正确性锚 : 双轨 stdout 逐字节一致（不一致 ⇒ 本轮作废）
# 用法：
#   bash examples/m267_perf/collect_baseline_v2.sh <out.json> [tag]
# 环境变量：M267_ROOT / M267_REPS（默认5） / M267_W（工作目录） / M267_LOADS
# 说明：先落 <out>.records（可读原值），再调 baseline_v2_json.py assemble 出 JSON。
# ══════════════════════════════════════════════════════════════
set -uo pipefail

ROOT="${M267_ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
DIR="$ROOT/examples/m267_perf"
SRC="$DIR/src"
PX="$ROOT/tools/px"
SPEC="$ROOT/docs/PERF_BASELINE_METRICS_SPEC.md"
ASSEMBLE="$DIR/baseline_v2_json.py"
OUT="${1:?usage: collect_baseline_v2.sh <out.json> [tag]}"
TAG="${2:-rep1}"
REPS="${M267_REPS:-5}"
W="${M267_W:-/tmp/m267_perf_v2}"
LOADS="${M267_LOADS:-fib28 while_sum mixed jsonwb}"
PIN="taskset -c 3"
STARTUP_RUNS=200
BUILD_WARMUPS=2

command -v taskset >/dev/null 2>&1 || { echo "FATAL: 缺 taskset —— 钉核不可用，数字不得入基线" >&2; exit 3; }
[ -x "$PX" ] || { echo "FATAL: 缺 $PX" >&2; exit 3; }
mkdir -p "$W"

REC="$OUT.records"
: > "$REC"
rec() { printf '%s\n' "$(IFS=$'\t'; echo "$*")" >> "$REC"; }

T0=$(date +%s)
STARTED=$(date -u +%FT%TZ)
LA0=$(awk '{print $1" "$2" "$3}' /proc/loadavg)
GATES0=$(pgrep -fc 'verify\.sh|run_gates\.sh' 2>/dev/null || echo 0)

echo "══ collect_baseline_v2 [$TAG] ══ W=$W REPS=$REPS PIN='$PIN'"
echo "  started=$STARTED loadavg=$LA0 并发门进程=$GATES0"

# ── 1) 构建产物（VM 轨默认 / C 轨 --c）────────────────────────
for n in $LOADS; do
    mkdir -p "$W/vm_$n" "$W/c_$n"
    cp -f "$SRC/$n.px" "$W/vm_$n/"; cp -f "$SRC/$n.px" "$W/c_$n/"
    ( cd "$W/vm_$n" && "$PX" build     "$n.px" >/dev/null 2>&1 ) || { echo "FATAL: build VM $n 失败" >&2; exit 4; }
    ( cd "$W/c_$n"  && "$PX" build --c "$n.px" >/dev/null 2>&1 ) || { echo "FATAL: build C  $n 失败" >&2; exit 4; }
done
mkdir -p "$W/empty"; printf 'print(1)\n' > "$W/empty/empty.px"
( cd "$W/empty" && "$PX" build empty.px >/dev/null 2>&1 ) || { echo "FATAL: build empty 失败" >&2; exit 4; }

# ── 2) 产物身份自检（强制）────────────────────────────────────
echo "[身份自检]"
for n in $LOADS; do
    nvm=$(strings -a "$W/vm_$n/build/$n" 2>/dev/null | grep -c '^fn_' || true)
    nc=$(strings -a "$W/c_$n/build/$n"  2>/dev/null | grep -c '^fn_' || true)
    rec ident "$n" vm "$nvm"
    rec ident "$n" c  "$nc"
    printf '  %-10s VM fn_*=%s(期望0)  C fn_*=%s(期望≥1)\n' "$n" "$nvm" "$nc"
    [ "$nvm" = 0 ] || { echo "FATAL: $n VM 轨含 fn_*（=$nvm），产物身份错 ⇒ 退出" >&2; exit 5; }
    [ "$nc" -ge 1 ] || { echo "FATAL: $n C 轨无 fn_*（=$nc），产物身份错 ⇒ 退出" >&2; exit 5; }
done

# ── 3) 正确性锚点（双轨 stdout 逐字节一致）────────────────────
echo "[正确性锚点]"
for n in $LOADS; do
    ov=$($PIN "$W/vm_$n/build/$n" 2>/dev/null)
    oc=$($PIN "$W/c_$n/build/$n"  2>/dev/null)
    [ "$ov" = "$oc" ] && m=true || m=false
    rec anchor "$n" "$ov" "$oc" "$m"
    printf '  %-10s %s  VM=[%s] C=[%s]\n' "$n" "$([ "$m" = true ] && echo 一致✅ || echo 不一致❌)" "$ov" "$oc"
    [ "$m" = true ] || { echo "FATAL: $n 双轨 stdout 不一致 ⇒ 语义问题，本轮作废" >&2; exit 6; }
done

# ── 4) run 类：每负载双轨，预热 1 + N 轮，保留每轮原值 ────────
run_rounds() { # $1=reps  $2...=cmd ; stdout = 逗号分隔的每轮秒
    local r="$1"; shift
    local -a a=()
    local i s e d
    for i in $(seq 1 "$r"); do
        s=$(date +%s.%N); $PIN "$@" >/dev/null 2>&1; e=$(date +%s.%N)
        d=$(echo "$e $s" | awk '{printf "%.4f", $1-$2}')
        a+=("$d")
    done
    local IFS=,; printf '%s' "${a[*]}"
}

echo "[run 类 · 钉核 $PIN · 预热1 + ${REPS} 轮 · 取 min]"
for n in $LOADS; do
    $PIN "$W/vm_$n/build/$n" >/dev/null 2>&1                 # 预热（丢弃）
    rvm=$(run_rounds "$REPS" "$W/vm_$n/build/$n")
    mvm=$(echo "$rvm" | tr ',' '\n' | sort -n | head -1)
    $PIN "$W/c_$n/build/$n"  >/dev/null 2>&1                 # 预热（丢弃）
    rc=$(run_rounds "$REPS" "$W/c_$n/build/$n")
    mc=$(echo "$rc" | tr ',' '\n' | sort -n | head -1)
    rec run "${n}_vm" "$rvm"
    rec run "${n}_c"  "$rc"
    printf '  %-13s rounds=[%s] min=%s\n' "${n}_vm" "$rvm" "$mvm"
    printf '  %-13s rounds=[%s] min=%s\n' "${n}_c"  "$rc"  "$mc"
done

# ── 5) startup 类：空程序连跑 200 次 / 200（不钉核，与门一致）─
echo "[startup 类 · 空程序连跑 ${STARTUP_RUNS} 次 · 均值]"
ss=$(date +%s.%N); for _ in $(seq 1 "$STARTUP_RUNS"); do "$W/empty/build/empty" >/dev/null 2>&1; done; se=$(date +%s.%N)
su_total=$(echo "$se $ss" | awk '{printf "%.6f", $1-$2}')
su_per=$(echo "$su_total" | awk '{printf "%.6f", $1/'"$STARTUP_RUNS"'}')
rec startup startup_exec "$su_total" "$su_per" "$STARTUP_RUNS"
printf '  startup_exec total=%ss /%d = %s s/次\n' "$su_total" "$STARTUP_RUNS" "$su_per"

# ── 6) build 类：2 次预热后单次热构建 ─────────────────────────
echo "[build 类 · ${BUILD_WARMUPS} 次预热 + 单次热构建]"
for _ in $(seq 1 "$BUILD_WARMUPS"); do ( cd "$W/empty" && "$PX" build empty.px >/dev/null 2>&1 ); done
bs=$(date +%s.%N); ( cd "$W/empty" && "$PX" build empty.px >/dev/null 2>&1 ); be=$(date +%s.%N)
hb=$(echo "$be $bs" | awk '{printf "%.4f", $1-$2}')
rec build hot_build "$hb" "$BUILD_WARMUPS"
printf '  hot_build = %s s（%d 次预热后单次）\n' "$hb" "$BUILD_WARMUPS"

# ── 7) 环境信息 ───────────────────────────────────────────────
LA1=$(awk '{print $1" "$2" "$3}' /proc/loadavg)
GATES1=$(pgrep -fc 'verify\.sh|run_gates\.sh' 2>/dev/null || echo 0)
rec env host        "$(uname -n)"
rec env cpu_model   "$(grep -m1 'model name' /proc/cpuinfo | sed 's/.*: //; s/ *[0-9]*-Core.*//')"
rec env nproc       "$(nproc)"
rec env mem_gib     "$(free -g | awk '/^Mem:/{print $2}')"
rec env os          "$(grep PRETTY_NAME /etc/os-release | sed 's/.*"\(.*\)".*/\1/; s/ (.*//')"
rec env kernel      "$(uname -r)"
rec env gcc         "$(gcc --version 2>/dev/null | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
rec env px          "$("$PX" --version 2>/dev/null | head -1 | awk '{print $2}')"
rec env px_path     "tools/px"
rec env pin         "$PIN"
rec env loadavg_before "$LA0"
rec env loadavg_after  "$LA1"
rec env gates_before   "$GATES0"
rec env gates_after    "$GATES1"
[ "$GATES0" = 0 ] && [ "$GATES1" = 0 ] && rec env quiet true || rec env quiet false
rec meta tag "$TAG"
rec meta started_at "$STARTED"
rec meta ended_at "$(date -u +%FT%TZ)"
rec meta duration_s "$(( $(date +%s) - T0 ))"
rec meta spec_sha256 "$(sha256sum "$SPEC" | awk '{print $1}')"
rec meta reps "$REPS"

echo "  loadavg_after=$LA1 并发门进程=$GATES1 duration=$(( $(date +%s) - T0 ))s"

# ── 8) 组装 JSON ──────────────────────────────────────────────
python3 "$ASSEMBLE" assemble "$REC" "$OUT" "${LOADS}"
rc=$?
echo "  records=$REC  json=$OUT"
exit $rc
