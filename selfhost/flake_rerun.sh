#!/usr/bin/env bash
# ============================================================================
# selfhost/flake_rerun.sh —— 把「偶发」变成频次数字（M341 · 台账第 3 步）
# ----------------------------------------------------------------------------
# ⚠️ 这不是门：不进 gates.registry.sh、不进 ci.yml、不判红。它是**离线诊断工具**。
#
# 为什么要有它（M340 台账第五节那条纪律的反面）：
#   门红时最常见的两个错误处置是「重跑一遍就绿了 ⇒ 收工」和「改判据让它别红」。
#   两者都在掩盖：前者没留下任何数字，后者把真问题藏起来。
#   正确做法只有一条 —— **在同样的树、同样的环境下反复跑，把命中次数记下来**。
#   有了「x/N 命中」这个数字，才有资格谈「这条判据是不是够稳」。
#
# 用法
#   selfhost/flake_rerun.sh -n 10 examples/m245_tls_cert_reload/verify.sh
#   selfhost/flake_rerun.sh -n 5 -t 900 -- examples/m128_unlock_grow/verify.sh --neg-skip
#
# 输出
#   · 逐轮一行：run# rc 秒 首个失败行
#   · 汇总：PASS/FAIL 计数 + 命中率；命中率 0 时**不代表判据没问题**（可能 N 太小）
#   · 现场留存：$OUT/run-K.log（每轮完整日志）+ $OUT/summary.txt
#
# 诚实边界
#   ① N 轮全绿 ≠ 判据稳定，只等于「这 N 次没抽中」；频次低的现象要 N 大才看得见。
#   ② 本工具串行执行 —— 它测的是「单跑下的稳定性」，**不是**分片并发下的稳定性
#      （分片才暴露锁/端口/共享 tmp 类问题；那种要在 shard_run.sh 里跑）。
# ============================================================================
set -u

N=10
TIMEOUT=1800
OUT=""
while [ $# -gt 0 ]; do
  case "$1" in
    -n) N="$2"; shift 2 ;;
    -t) TIMEOUT="$2"; shift 2 ;;
    -o) OUT="$2"; shift 2 ;;
    --) shift; break ;;
    -h|--help) sed -n '2,30p' "$0"; exit 0 ;;
    *) break ;;
  esac
done

if [ $# -eq 0 ]; then
  echo "用法：$0 [-n 轮数] [-t 单轮超时] -- <门脚本> [门参数...]" >&2
  exit 2
fi

GATE="$1"; shift
if [ ! -f "$GATE" ]; then
  echo "❌ 找不到门脚本：$GATE" >&2
  exit 2
fi

# 门要在仓库根跑（多数门假定 CWD=仓库根）
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT" || exit 2

TS="$(date +%Y%m%d-%H%M%S)"
OUT="${OUT:-/data/gatevm/flake_rerun/$TS-$(basename "$(dirname "$GATE")")}"
mkdir -p "$OUT" || exit 2

echo "══ 重复执行：$GATE × $N 轮（单轮超时 ${TIMEOUT}s）══"
echo "   现场：$OUT"
pass=0; fail=0; to=0
declare -a rc_list=()

for k in $(seq 1 "$N"); do
  log="$OUT/run-$k.log"
  t0=$(date +%s)
  timeout "$TIMEOUT" bash "$GATE" "$@" > "$log" 2>&1
  rc=$?
  t1=$(date +%s)
  dur=$((t1 - t0))
  rc_list+=("$rc")
  if [ "$rc" -eq 0 ]; then
    pass=$((pass + 1)); verdict="PASS"
  elif [ "$rc" -eq 124 ]; then
    to=$((to + 1)); verdict="TIMEOUT"
  else
    fail=$((fail + 1)); verdict="FAIL"
  fi
  # 首个失败行（便于一眼看出红在哪）
  first_bad="$(grep -m1 -E '❌|FAIL|失败' "$log" 2>/dev/null | cut -c1-140)"
  printf 'run %-3s rc=%-3s %4ss  %-7s %s\n' "$k" "$rc" "$dur" "$verdict" "${first_bad:-}"
done

{
  echo "gate=$GATE"
  echo "rounds=$N timeout=$TIMEOUT"
  echo "pass=$pass fail=$fail timeout=$to"
  echo "rc_list=${rc_list[*]}"
} | tee "$OUT/summary.txt"

if [ "$fail" -eq 0 ] && [ "$to" -eq 0 ]; then
  echo "⇒ $N/$N 全绿：**这 $N 次里没抽中**（不是「判据稳定」的证明；要更大 N 或并发条件才有结论）"
else
  echo "⇒ 命中 $(($fail + $to))/$N 轮：这就是频次，可作为定性依据（现场在 $OUT）"
fi
exit 0
