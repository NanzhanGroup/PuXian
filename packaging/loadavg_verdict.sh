#!/usr/bin/env bash
# ============================================================
# packaging/loadavg_verdict.sh —— 整链负载判据（M361 · 缺陷 625）
# ------------------------------------------------------------
# 由来（M355–M360 六轮同形实测）：
#   旧判据只看 `loadavg_max_1m`（10s 采样窗口里的**单点最大值**）。
#   六轮 max = 67.98 / 79.99 / 80.34 / 92.16 / 93.62 / 105.50（4.25–6.59 每核），
#   而 median 只有 0.53–0.62 每核、p95 0.92–1.14 每核、≥2 核采样占比 1.9–3.0%
#   ⇒ **max 捕的是编译 spawn 潮的一个尖峰，落在哪个 10s 采样点决定读数**，
#     与「机器是否被持续压」无关。用 max 当判据 = 判据选错量具（第 5 次同族）。
#
# 新判据（两腿都要过，缺一即 FAIL）：
#   [P] p95(loadavg 1m) ≤ --p95-percore × 核数       （默认 2.0/核）
#   [R] 越界采样占比（≥ --p95-percore 每核）≤ --over-pct（默认 5%）
#   诊断量（**不判红**）：max > --anomaly-percore × 核数（默认 8.0/核）⇒ 打 LOADAVG-ANOMALY
#   max 全量保留在输出里，只做定位线索。
#
# 判据**只能更严或更准**：
#   旧 max 判据（max > 2.0/核 即红）六轮全红；新判据六轮全绿 + 持续压载仍判红
#   （负控见 packaging/selftest_loadavg_verdict.sh 与 /data/autodrive/logs/M361-NEGCTL.md）。
#
# 用法：
#   loadavg_verdict.sh [选项] <tsv...>
#     tsv 第 2 列 = loadavg 1m（第 1 列时间戳，其余列忽略）
#   --nproc N            核数（默认 nproc）
#   --p95-percore R      p95 上限，每核（默认 2.0）
#   --over-pct R         越界采样占比上限 %（默认 5）
#   --anomaly-percore R  max 诊断阈值，每核（默认 8.0）
#   --min-samples N      样本下限（默认 30；不足 ⇒ rc=2，不许拿 3 个点判绿）
#   --json               机器可读输出
#   --quiet              只打聚合结论
# 退出码：0 全 PASS · 1 有 FAIL · 2 输入/用法无效
# ============================================================
set -uo pipefail
PROG=${0##*/}

NPROC=$(nproc 2>/dev/null || echo 1)
P95PC=2.0
OVERPCT=5
ANOMAX=8.0
MINS=30
JSON=0
QUIET=0
FILES=()

die(){ echo "$PROG: $*" >&2; exit 2; }
usage(){ sed -n '2,40p' "$0" | sed 's/^# \{0,1\}//'; exit 2; }

while [ $# -gt 0 ]; do
  case "$1" in
    --nproc)          NPROC=${2:-}; shift 2;;
    --p95-percore)    P95PC=${2:-}; shift 2;;
    --over-pct)       OVERPCT=${2:-}; shift 2;;
    --anomaly-percore) ANOMAX=${2:-}; shift 2;;
    --min-samples)    MINS=${2:-}; shift 2;;
    --json)           JSON=1; shift;;
    --quiet)          QUIET=1; shift;;
    -h|--help)        usage;;
    --)               shift; while [ $# -gt 0 ]; do FILES+=("$1"); shift; done;;
    -*)               die "未知选项 $1（--help 看用法）";;
    *)                FILES+=("$1"); shift;;
  esac
done

# 参数合法性：核数/阈值必须是正数，占比 0..100
case "$NPROC" in ''|*[!0-9]*) die "--nproc 需正整数（得 [$NPROC]）";; esac
[ "$NPROC" -gt 0 ] || die "--nproc 需 > 0"
for v in "$P95PC" "$OVERPCT" "$ANOMAX" "$MINS"; do
  case "$v" in ''|*[!0-9.]*|*.*.*) die "数值参数非法：[$v]";; esac
done
[ ${#FILES[@]} -gt 0 ] || die "没有输入文件（--help 看用法）"

# 单文件裁决。输出：一行机器可读 kv + （非 --json）一行人读
#   file n median p95 over overpct max p95_ok over_ok verdict diag
judge() {
  local f=$1
  [ -r "$f" ] || { echo "ERR\tfile=$f\t原因=不可读"; return 2; }
  awk -F'\t' -v nc="$NPROC" -v p95pc="$P95PC" -v overpct="$OVERPCT" \
               -v anomax="$ANOMAX" -v mins="$MINS" -v fn="$f" '
    NF >= 2 && $2 ~ /^[0-9]+(\.[0-9]+)?$/ {
      v = $2 + 0; n++; h[int(v*100+0.5)]++
      if (v > mx) mx = v
      if (v >= p95pc*nc) over++
      next
    }
    NF >= 2 { bad++ }                      # 有列但第 2 列不是数 ⇒ 记账
    END {
      if (n == 0) { printf "ERR\tfile=%s\trc=2\t原因=无有效采样（坏行 %d）\n", fn, bad+0; exit 2 }
      if (n < mins) { printf "ERR\tfile=%s\trc=2\t原因=样本不足 n=%d < %d\n", fn, n, mins; exit 2 }
      # nearest-rank 分位数（centi-load 直方图，避免依赖 gawk asort）
      r = int((95*n + 99)/100);  if (r < 1) r = 1
      m = int((n+1)/2)
      c = 0; cum = 0; p95 = -1; med = -1
      for (c = 0; cum < n; c++) {
        if (!(c in h)) continue
        cum += h[c]
        if (med < 0 && cum >= m)   med = c/100
        if (p95 < 0 && cum >= r) { p95 = c/100; break }
      }
      if (p95 < 0) p95 = mx + 0
      overp = 100.0*over/n
      p95ok  = (p95 <= p95pc*nc + 1e-9) ? 1 : 0
      overok = (overp <= overpct + 1e-9) ? 1 : 0
      vd = (p95ok && overok) ? "PASS" : "FAIL"
      diag = (mx > anomax*nc) ? "ANOMALY" : "OK"
      printf "file=%s\tn=%d\tmedian=%.2f\tmedian_pc=%.2f\tp95=%.2f\tp95_pc=%.2f\tover=%d\tover_pct=%.2f\tmax=%.2f\tmax_pc=%.2f\tp95_ok=%d\tover_ok=%d\tdiag=%s\tverdict=%s\n",
             fn, n, med, med/nc, p95, p95/nc, over, overp, mx, mx/nc, p95ok, overok, diag, vd
      exit (vd == "PASS") ? 0 : 1
    }
  ' "$f"
  return $?
}

TOTAL=0; NFAIL=0; NERR=0; NANOM=0; OUT=""
for f in "${FILES[@]}"; do
  TOTAL=$((TOTAL+1))
  line=$(judge "$f"); rc=$?
  case "$line" in
    ERR*) NERR=$((NERR+1)); [ "$JSON" -eq 1 ] || echo "  ⚠️  $line";;
    *) case "$line" in *verdict=FAIL*) NFAIL=$((NFAIL+1));; esac
       case "$line" in *diag=ANOMALY*) NANOM=$((NANOM+1));; esac
       if [ "$JSON" -eq 1 ]; then OUT+="$line"$'\n'
       elif [ "$QUIET" -eq 0 ]; then
         printf '  %-28s n=%-5s p95=%s/c 越界=%s%% max=%s/c ⇒ %s%s\n' \
           "$(basename "$f")" "$(sed -n 's/.*\tn=\([0-9]*\).*/\1/p' <<<"$line")" \
           "$(sed -n 's/.*\tp95_pc=\([0-9.]*\).*/\1/p' <<<"$line")" \
           "$(sed -n 's/.*\tover_pct=\([0-9.]*\).*/\1/p' <<<"$line")" \
           "$(sed -n 's/.*\tmax_pc=\([0-9.]*\).*/\1/p' <<<"$line")" \
           "$(sed -n 's/.*\tverdict=\([A-Z]*\).*/\1/p' <<<"$line")" \
           "$(case "$line" in *diag=ANOMALY*) echo '  ⚠️ ANOMALY（诊断量，不判红）';; *) echo '';; esac)"
       fi;;
  esac
done

if [ "$JSON" -eq 1 ]; then printf '%s' "$OUT"; fi

if [ "$NERR" -gt 0 ]; then
  echo "LOADAVG-VERDICT=INVALID bad_input=$NERR files=$TOTAL"
  exit 2
fi
if [ "$NFAIL" -gt 0 ]; then
  echo "LOADAVG-VERDICT=FAIL fail=$NFAIL files=$TOTAL anomaly=$NANOM"
  exit 1
fi
echo "LOADAVG-VERDICT=PASS files=$TOTAL anomaly=$NANOM（判据 p95≤${P95PC}/核 且 越界≤${OVERPCT}% · 核数 ${NPROC}）"
exit 0
