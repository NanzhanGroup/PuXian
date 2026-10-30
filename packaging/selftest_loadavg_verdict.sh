#!/usr/bin/env bash
# ============================================================
# packaging/selftest_loadavg_verdict.sh —— loadavg_verdict.sh 的**离线**自证（M361 · 缺陷 625）
# ------------------------------------------------------------
# 来历（M355–M360）：整链负载判据用 `loadavg_max_1m`（单点最大值）当红绿，
#   六轮 max 4.25–6.59 每核 ⇒ 六轮全「红」，而 median 0.53–0.62、p95 0.92–1.14、越界占比 1.9–3.0%
#   ⇒ 判的是「尖峰恰落在哪个 10s 采样点」，不是「机器被持续压」。这是治假红，不是放宽：
#   判据换成 p95 + 越界占比 之后，**持续压载仍必须判红**（本自证 [3][4] + 真机 hog 负控）。
#
# 层次：
#   [1] 静默档（M35x 形态）                ⇒ PASS
#   [2] 尖峰短、均值低（M36x 真形态）      ⇒ PASS 且 **旧 max 判据必红**（证明治的是假红）
#   [3] 持续压载（每样本 2.5/核）          ⇒ FAIL（p95 腿有牙）
#   [4] 越界占比 5.16%、p95 恰在 2.0/核 边界 ⇒ FAIL（占比腿**单独**有牙）
#   [5] 单点 20/核 尖峰                    ⇒ PASS + diag=ANOMALY（诊断量不判红）
#   [6] 样本不足 / 空文件                  ⇒ rc=2 INVALID（不许拿 3 个点判绿）
#   [7] 六轮真实语料回放（本机有则跑）     ⇒ 新判据 6/6 PASS，旧 max 判据 6/6 红
#   [8] 负控 A：判据自伤（阈值放到 100/核）⇒ [3] 必须变绿（证明红来自判据侧）
#   [9] 负控 B：占比阈值压到 0%            ⇒ [2] 必须变红（证明占比腿不是摆设）
#   [10] 用法/参数错误                     ⇒ rc=2
# 退出码：0 全绿 · 1 有失败
# ============================================================
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
SH=${SH:-$HERE/loadavg_verdict.sh}
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
P=0; F=0; S=0
ok(){ P=$((P+1)); printf '  ✅ %s\n' "$1"; }
ng(){ F=$((F+1)); printf '  ❌ %s\n' "$1"; }
sk(){ S=$((S+1)); printf '  ⏭  %s\n' "$1"; }
chk(){ if [ "$2" = "$3" ]; then ok "$1"; else ng "$1（期望 [$3] 实得 [$2]）"; fi; }
chkg(){ if grep -q -- "$3" <<<"$2"; then ok "$1"; else ng "$1（[${2:0:240}] 不含 [$3]）"; fi; }
chkn(){ if grep -q -- "$3" <<<"$2"; then ng "$1（[${2:0:240}] 不该含 [$3]）"; else ok "$1"; fi; }

# 全机核数 —— 口径必须与 loadavg_verdict.sh 的 online_cpus 一致：**不能用 nproc**
#   nproc 读 sched_getaffinity；分片门 taskset 绑 2 核时返回 2。
#   缺陷 626（M361r2）只修了判据侧，本自证仍拿 nproc 当「全机」⇒ 分片里 NC=2
#   ⇒ [11a]「2 < 2」为假、[11d] 期望「核数 2」而不含 ⇒ 门自己假红（缺陷 627，2026-10-30 r2 实测）。
machine_cpus(){
  local n=''
  if [ -r /sys/devices/system/cpu/online ]; then
    n=$(awk -F, '{s=0; for(i=1;i<=NF;i++){ if($i ~ /-/){split($i,a,"-"); s+=a[2]-a[1]+1} else if($i ~ /^[0-9]+$/) s++ } print s}' \
        /sys/devices/system/cpu/online 2>/dev/null)
  fi
  case "$n" in ''|*[!0-9]*|0) n=$(getconf _NPROCESSORS_ONLN 2>/dev/null);; esac
  case "$n" in ''|*[!0-9]*|0) n=$(nproc 2>/dev/null);; esac
  case "$n" in ''|*[!0-9]*|0) n=1;; esac
  printf '%s\n' "$n"
}
NC=$(machine_cpus)
[ -x "$SH" ] || { echo "❌ 缺 $SH 或不可执行"; exit 1; }

# 造语料：gen <file> <模式> [参数]
#   quiet  : 465 点，8.3–9.6（≈0.55/核）
#   spiky  : 453 点静默 + 12 点 90（真 M36x 形态：2.6% 越界）
#   hog    : 465 点 40（2.5/核 持续）
#   ratio  : 441 点 8 + 24 点 32（5.16% 越界；p95 **恰落在 2.0/核 边界** ⇒ 只有占比腿能红）
#   spike1 : 464 点 8 + 1 点 320（单点 20/核）
#   few    : 5 点
#   empty  : 空
gen(){
  local f=$1 mode=$2 n i
  : > "$f"
  case $mode in
    quiet)  n=465; for ((i=0;i<n;i++)); do printf '%s\t%.2f\t0.90\t2.90\n' $((1793200000+i*10)) $(awk -v i=$i 'BEGIN{printf "%.2f", 8.3 + (i%7)*0.2}'); done > "$f";;
    spiky)  n=465; for ((i=0;i<n;i++)); do v=8.3; (( i % 40 == 0 && i < 480 )) && v=90.0; printf '%s\t%.2f\t0.90\t2.90\n' $((1793200000+i*10)) $v; done > "$f";;
    hog)    n=465; for ((i=0;i<n;i++)); do printf '%s\t40.00\t40.00\t39.00\n' $((1793200000+i*10)); done > "$f";;
    ratio)  n=465; for ((i=0;i<n;i++)); do v=8.0; (( i >= 441 )) && v=32.0; printf '%s\t%.2f\t8.00\t8.00\n' $((1793200000+i*10)) $v; done > "$f";;
    spike1) n=465; for ((i=0;i<n;i++)); do v=8.0; (( i == 200 )) && v=320.0; printf '%s\t%.2f\t8.00\t8.00\n' $((1793200000+i*10)) $v; done > "$f";;
    few)    for ((i=0;i<5;i++)); do printf '%s\t8.00\t8.00\t8.00\n' $((1793200000+i*10)); done > "$f";;
    empty)  : > "$f";;
  esac
}

echo
echo "=== loadavg_verdict 离线自证（M361 · 判据 p95+占比，核数 ${NC}）==="

echo "── [1] 静默档 ⇒ PASS"
gen "$W/quiet.tsv" quiet
out=$("$SH" --quiet "$W/quiet.tsv"); rc=$?
chk "[1a] 退出码 0" "$rc" 0
chkg "[1b] 聚合 PASS" "$out" "LOADAVG-VERDICT=PASS"

echo "── [2] 尖峰短 + 均值低（M36x 真形态）⇒ PASS，且旧 max 判据必红"
gen "$W/spiky.tsv" spiky
outj=$("$SH" --json "$W/spiky.tsv"); rc=$?
chk "[2a] 新判据 退出码 0" "$rc" 0
chkg "[2b] p95 没被尖峰带跑（0.52/核）" "$outj" "p95_pc=0.52"
chkg "[2c] 越界占比 2.58% 在 5% 内" "$outj" "over_pct=2.58"
chkg "[2d] max 仍原样保留（诊断可追溯）" "$outj" "max_pc=5.62"
oldmax=$(awk -F'\t' 'BEGIN{m=0}{if($2+0>m)m=$2+0}END{printf "%.2f", m}' "$W/spiky.tsv")
oldpc=$(awk -v m="$oldmax" -v nc="$NC" 'BEGIN{printf "%.2f", m/nc}')
oldrc=$(awk -v m="$oldmax" -v nc="$NC" 'BEGIN{exit (m/nc > 2.0) ? 1 : 0}'; echo $?)
chk "[2e] 旧 max 判据对同料判红（max=${oldmax} = ${oldpc}/核）" "$oldrc" 1
chk "[2f] 旧 max 红 / 新判据绿 ⇒ 治的是假红" "$(case "$rc:$oldrc" in 0:1) echo fixed;; *) echo no;; esac)" "fixed"

echo "── [3] 持续压载（2.5/核）⇒ FAIL（p95 腿）"
gen "$W/hog.tsv" hog
outj=$("$SH" --json "$W/hog.tsv"); rc=$?
chk "[3a] 退出码 1" "$rc" 1
chkg "[3b] 聚合 FAIL" "$outj" "LOADAVG-VERDICT=FAIL"
chkg "[3c] p95 越过 2.0/核 被点名" "$outj" "p95_ok=0"

echo "── [4] 均值贴地但越界占比 5.16%（p95 恰在 2.0/核 边界）⇒ FAIL（占比腿单独有牙）"
gen "$W/ratio.tsv" ratio
outj=$("$SH" --json "$W/ratio.tsv"); rc=$?
chk "[4a] 退出码 1" "$rc" 1
chkg "[4b] p95 腿是过的（恰 2.0/核）" "$outj" "p95_ok=1"
chkg "[4c] 红来自占比腿（5.16% > 5%）" "$outj" "over_ok=0"

echo "── [5] 单点 20/核 尖峰 ⇒ PASS + diag=ANOMALY（诊断量不判红）"
gen "$W/spike1.tsv" spike1
outj=$("$SH" --json "$W/spike1.tsv"); rc=$?
chk "[5a] 退出码 0（不判红）" "$rc" 0
chkg "[5b] 打出 ANOMALY 诊断" "$outj" "diag=ANOMALY"
chkg "[5c] max 原样保留可定位" "$outj" "max_pc=20.00"

echo "── [6] 样本不足 / 空文件 ⇒ rc=2 INVALID（不许拿少数点判绿）"
gen "$W/few.tsv" few; gen "$W/empty.tsv" empty
out=$("$SH" "$W/few.tsv"); rc=$?
chk "[6a] 5 个样本 rc=2" "$rc" 2
chkg "[6b] 报 INVALID 而非 PASS/FAIL" "$out" "VERDICT=INVALID"
out=$("$SH" "$W/empty.tsv"); rc=$?
chk "[6c] 空文件 rc=2" "$rc" 2

echo "── [7] 六轮真实语料回放（M355–M360）"
real=(); for m in 355 356 357 358 359 360; do f=/data/autodrive/logs/M${m}-loadavg.tsv; [ -r "$f" ] && real+=("$f"); done
if [ ${#real[@]} -eq 6 ]; then
  out=$("$SH" --quiet "${real[@]}"); rc=$?
  chk "[7a] 新判据 6/6 PASS" "$rc" 0
  chkg "[7b] 聚合行" "$out" "files=6"
  oldred=0
  for f in "${real[@]}"; do
    awk -F'\t' -v nc="$NC" 'BEGIN{m=0}{if($2+0>m)m=$2+0}END{exit (m/nc > 2.0)?1:0}' "$f" || oldred=$((oldred+1))
  done
  chk "[7c] 旧 max 判据 6/6 红（对照）" "$oldred" 6
else
  sk "[7] 本机无 M355–M360 语料（${#real[@]}/6）⇒ 跳过真实语料回放（CI 常态）"
fi

echo "── [8] 负控 A：判据自伤（阈值放到 100/核、占比 100%）⇒ [3] 必须变绿"
out=$("$SH" --p95-percore 100 --over-pct 100 --anomaly-percore 1000 "$W/hog.tsv"); rc=$?
chk "[8a] 自伤后持续压载变绿 ⇒ [3] 的红来自判据侧" "$rc" 0

echo "── [9] 负控 B：占比阈值压到 0% ⇒ [2] 必须变红（占比腿有牙）"
outj=$("$SH" --json --over-pct 0 "$W/spiky.tsv"); rc=$?
chk "[9a] 退出码 1" "$rc" 1
chkg "[9b] 红点名为占比腿" "$outj" "over_ok=0"

echo "── [10] 用法 / 参数错误 ⇒ rc=2"
"$SH" --nope "$W/quiet.tsv" >/dev/null 2>&1; chk "[10a] 未知选项 rc=2" "$?" 2
"$SH" >/dev/null 2>&1; chk "[10b] 无输入 rc=2" "$?" 2
"$SH" --p95-percore abc "$W/quiet.tsv" >/dev/null 2>&1; chk "[10c] 非数值阈值 rc=2" "$?" 2
"$SH" --nproc 0 "$W/quiet.tsv" >/dev/null 2>&1; chk "[10d] 核数 0 rc=2" "$?" 2

# ── [11] 绑核回归（M361 首跑真红现场 · 缺陷 625 复发）──────────────────────
# 现场：分片门用 taskset 把每路绑在 2 核上跑，判据默认核数取 nproc ⇒ 得 2 ⇒ 每核值虚高 8 倍
#       ⇒ 本应 PASS 的 [1][2][5][7] 全判红（11 条断言塌）。根因在**量具分母与分子不同界**：
#       分子 /proc/loadavg 是全系统负载，分母必须是全机核数。
# 本回归：在 taskset -c 0,1 下重跑整个自证，必须同样全绿；并验证子跑里 nproc 确实变小（证明本回归有牙）。
if [ -z "${LVA_SELFTEST_SUB:-}" ] && command -v taskset >/dev/null 2>&1 && taskset -c 0,1 true 2>/dev/null; then
  echo "── [11] 绑核回归：taskset -c 0,1 下整份自证仍须全绿"
  subpc=$(taskset -c 0,1 nproc 2>/dev/null); subpc=${subpc:-0}
  chk "[11a] 绑核生效（子跑 nproc=${subpc} < 全机 ${NC}）" "$([ "$subpc" -gt 0 ] && [ "$subpc" -lt "$NC" ] && echo yes || echo no)" "yes"
  sub=$(LVA_SELFTEST_SUB=1 taskset -c 0,1 bash "$0" 2>&1); subrc=$?
  chk "[11b] 绑核下自证 rc=0" "$subrc" 0
  chkg "[11c] 绑核下全绿" "$sub" "SELFTEST PASS"
  # 反向牙口：判据在绑核环境里报告的分母必须是全机核数，不能是子集核数
  chkg "[11d] 绑核下分母仍是全机核数" "$(LVA_SELFTEST_SUB=1 taskset -c 0,1 bash "$SH" --quiet "$W/quiet.tsv" 2>&1)" "核数 ${NC}"
else
  sk "[11] 无 taskset / 0,1 核不可用 / 已处于子跑 ⇒ 跳过绑核回归"
fi

echo
tot=$((P+F))
if [ "$F" -eq 0 ]; then
  echo "SELFTEST PASS（${P}/${tot} 断言 · 跳过 ${S}）"
  exit 0
fi
echo "SELFTEST FAIL（${F} 条失败 / ${tot}）"
exit 1
