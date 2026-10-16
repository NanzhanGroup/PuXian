#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────
# packaging/chain/shard_accel.sh —— 全量门**分片加速档**（M309 C1 · 加速器，非权威）
#
# 定位（与 gate_verdict.sh 的关系）：
#   · 权威档恒为**串行** `selfhost/run_gates.sh` —— `gate_verdict.sh` 裁决的那一档。
#   · 本脚本只是**加速器**：N 路 worktree 并行跑**同一份**门清单（取自 HEAD），
#     且**必须**与串行基线逐门 (状态, rc) 对拍（`shard_verdict.py`）。
#     对拍不过 ⇒ RED —— **不许拿"跑得快"当结论**（判据强度零让步）。
#   · 默认口径：夹住门内并行度（每路 `GATE_JOBS=2` + 按 N 自动切核区间绑核）
#     —— 实测 118 min → 87 min（1.36×），依据与覆盖边界见 `docs/GATE_ISOLATION.md` §十。
#     退回旧行为：`SHARD_NO_CLAMP=1`（不夹并行度 / 不绑核）。
#
# ⚠️ 为什么 `--base` 必须由调用方给（**不许写死**）：
#   本仓守卫 `selfhost/check_gate_paths.sh` 的纪律 —— 仓库代码不许出现开发机绝对路径
#   （缺陷 300：写死 /data/code/puxian ⇒ 本机全绿、CI 红）。分片调度器住在**宿主**上、
#   不在仓里 ⇒ 路径只能**参数化**；缺参数就 `rc=3` 拒绝，**不许猜**。
#
# 用法：
#   packaging/chain/shard_accel.sh --base <gatevm 目录> --baseline <串行 times.tsv> \
#                                 [--times <实测负载 times.tsv>] [--n 4] [--dry] [--quiet]
# 输出（第一行恒为裁决，便于调用方 grep）：
#   SHARD-VERDICT=GREEN|RED|INCONSISTENT|PREMISE-FAIL
# 退出码：0 绿 / 1 红（含对拍不一致/有红门） / 2 没跑完（找不到日志目录） / 3 前提不成立
# ─────────────────────────────────────────────────────────────────────
set -uo pipefail

BASE=""; BL=""; TIMES=""; N=4; DRY=0; QUIET=0
while [ $# -gt 0 ]; do
  case "$1" in
    --base)     BASE="${2:-}";     shift 2 ;;
    --baseline) BL="${2:-}";       shift 2 ;;
    --times)    TIMES="${2:-}";    shift 2 ;;
    --n)        N="${2:-}";        shift 2 ;;
    --dry)      DRY=1; shift ;;
    --quiet)    QUIET=1; shift ;;
    -h|--help)  sed -n '2,32p' "$0"; exit 0 ;;
    *) echo "未知参数: $1" >&2; exit 3 ;;
  esac
done

die_premise() { echo "SHARD-VERDICT=PREMISE-FAIL"; printf '  ⛔ %s\n' "$*"; exit 3; }
say() { [ "$QUIET" = 1 ] || printf '%s\n' "$*"; }

[ -n "$BASE" ] || die_premise "缺 --base（分片调度器所在目录）—— 本脚本**不猜**宿主路径"
[ -d "$BASE" ] || die_premise "目录不存在：$BASE"
[ -f "$BASE/shard_run.sh" ]    || die_premise "缺分片调度器：$BASE/shard_run.sh"
[ -f "$BASE/shard_verdict.py" ]|| die_premise "缺逐门裁决器：$BASE/shard_verdict.py"
case "$N" in ''|*[!0-9]*) die_premise "--n 必须是数字（给的是「$N」）";; esac
[ "$N" -ge 2 ] || die_premise "--n 必须 ≥2（1 路并行不是加速档，请用串行档）"

if [ "$DRY" = 1 ]; then
  # --dry：只让调度器打规划（不跑、不裁决）—— 用于发布前确认门清单/均衡
  say "── dry：仅规划（不跑门、不裁决）──"
  bash "$BASE/shard_run.sh" "$N" ${TIMES:+--times "$TIMES"} --dry
  exit $?
fi

[ -n "$BL" ]  || die_premise "缺 --baseline（串行权威档的 times.tsv）—— 没有基线就没有逐门对拍"
[ -f "$BL" ]  || die_premise "串行基线不存在：$BL"
if [ -n "$TIMES" ]; then [ -f "$TIMES" ] || die_premise "耗时依据不存在：$TIMES"; fi

# 前提：同一时刻不许有两份全量门（门锁是宿主级；分片内部另有自己的锁命名空间）
if [ -f /tmp/.m116_gates.lock ] && kill -0 "$(cat /tmp/.m116_gates.lock 2>/dev/null)" 2>/dev/null; then
  die_premise "已有全量门在跑（PID $(cat /tmp/.m116_gates.lock)）⇒ 拒绝启动（否则净加速被别的门吃掉、结论失真）"
fi

TMPD="$(mktemp -d /tmp/shard_accel.XXXXXX)" || die_premise "mktemp 失败"
RUNLOG="$TMPD/shard_run.log"

say "── 分片加速档：N=$N · 基线 $(basename "$BL")${TIMES:+ · 耗时依据 $(basename "$TIMES")} ──"
START=$(date +%s)
bash "$BASE/shard_run.sh" "$N" ${TIMES:+--times "$TIMES"} > "$RUNLOG" 2>&1
SRC=$?
WALL=$(( $(date +%s) - START ))
say "  调度器 rc=$SRC · 墙钟 ${WALL}s（$((WALL/60)) min）· 日志 $RUNLOG"

LOG="$(grep -oE "$BASE/shard-[0-9]{8}-[0-9]{6}" "$RUNLOG" | head -1)"
if [ -z "$LOG" ] || [ ! -d "$LOG" ]; then
  echo "SHARD-VERDICT=INCONSISTENT"
  printf '  ⚠️ 找不到分片日志目录（调度器没跑到建目录那一步？）—— 看 %s\n' "$RUNLOG"
  tail -5 "$RUNLOG" 2>/dev/null | sed 's/^/    /'
  exit 2
fi

MERGED="$TMPD/par.tsv"
cat "$LOG"/shard*.times.tsv "$LOG"/quiet.times.tsv 2>/dev/null | sort > "$MERGED"
if [ ! -s "$MERGED" ]; then
  echo "SHARD-VERDICT=INCONSISTENT"
  printf '  ⚠️ 分片 times.tsv 全空（空跑？）—— 看 %s\n' "$LOG"
  exit 2
fi

python3 "$BASE/shard_verdict.py" "$BL" "$MERGED" > "$TMPD/verdict.txt" 2>&1
VRC=$?
say "── 逐门裁决（串行基线 ⇄ 分片）──"
sed 's/^/  /' "$TMPD/verdict.txt"

case "$VRC" in
  0) echo "SHARD-VERDICT=GREEN" ;;
  1) echo "SHARD-VERDICT=RED" ;;
  *) echo "SHARD-VERDICT=INCONSISTENT" ;;
esac
say "  出处：分片日志 $LOG · 合并 times $MERGED · 裁决 $TMPD/verdict.txt"
exit "$VRC"
