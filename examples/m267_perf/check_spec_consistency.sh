#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════
# examples/m267_perf/check_spec_consistency.sh
#   —— 自校验《性能基线 v2 指标口径》(docs/PERF_BASELINE_METRICS_SPEC.md)
#      的「SPEC-PARAMS」声明与既有脚本 / 基线表 / 门注册 / 实机环境是否一致。
#
# 目的（任务 #166/t1）：口径说明必须能**机器核对**，不能只是"人读着像"。
#   判定：全部一致 ⇒ 输出 SPEC-CONSISTENCY-OK 且 rc=0；任一不符 ⇒ rc=1。
# 只读：不修改任何被检文件。
# ══════════════════════════════════════════════════════════════
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SPEC="${SPEC_FILE:-$ROOT/docs/PERF_BASELINE_METRICS_SPEC.md}"   # SPEC_FILE 可覆盖（负控自测用）
VERIFY="$ROOT/examples/m267_perf/verify.sh"
BASE="$ROOT/examples/m267_perf/baseline.tsv"
REG="$ROOT/selfhost/gates.registry.sh"
PX="$ROOT/tools/px"

PASS=0; FAIL=0
ok()  { echo "  ✅ $*"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $*"; FAIL=$((FAIL+1)); }
eq()  { # $1=项 $2=声明 $3=实际
  if [ "$2" = "$3" ]; then ok "$1 = $3（声明=$2）"; else bad "$1 不一致：声明=[$2] 实际=[$3]"; fi
}

echo "══ 口径一致性自校验 ══  spec=$SPEC"

[ -f "$SPEC" ] || { echo "❌ 缺 spec"; exit 2; }
[ -f "$VERIFY" ] || { echo "❌ 缺 verify.sh"; exit 2; }
[ -f "$BASE" ] || { echo "❌ 缺 baseline.tsv"; exit 2; }
[ -f "$REG" ] || { echo "❌ 缺 gates.registry.sh"; exit 2; }

# ── 解析 SPEC-PARAMS 块 ────────────────────────────────────────
declare -A D
while IFS='=' read -r k v; do
  [ -n "$k" ] && D["$k"]="$v"
done < <(awk '/# >>> SPEC-PARAMS >>>/{f=1;next} /# <<< SPEC-PARAMS <<</{f=0} f && /=/ && $0 !~ /^#/{print}' "$SPEC")
[ "${#D[@]}" -gt 0 ] && ok "SPEC-PARAMS 块可解析（${#D[@]} 项）" || { bad "SPEC-PARAMS 块缺失/为空"; exit 2; }

echo
echo "[A] verify.sh（门实现）"
# 钉核
A=$(grep -oE 'PIN="[^"]+"' "$VERIFY" | head -1 | sed 's/PIN="//;s/"//')
eq "pin" "${D[pin]}" "$A"
# run 轮次
A=$(grep -oE 'REPS:-[0-9]+' "$VERIFY" | head -1 | grep -oE '[0-9]+$')
eq "run_reps" "${D[run_reps]}" "$A"
# run 统计量（min：sort -n | head -1）
if grep -Fq 'sort -n | head -1' "$VERIFY"; then ok "run_stat = min（sort -n | head -1 在位）"; else bad "run_stat 未找到 min 实现"; fi
# 启动次数
if grep -Fq 'seq 200' "$VERIFY" && grep -Fq '/200' "$VERIFY"; then ok "startup_runs = 200（seq 200 且 /200）"; else bad "startup_runs != 200"; fi
# 热构建预热
if grep -Fq 'for _ in 1 2; do' "$VERIFY"; then ok "build_warmups = 2"; else bad "build_warmups != 2"; fi
# warn / fail
A=$(grep -oE 'WARN:-[0-9.]+' "$VERIFY" | head -1 | grep -oE '[0-9.]+$')
eq "warn_ratio" "${D[warn_ratio]}" "$A"
A=$(grep -oE 'FAIL:-[0-9.]+' "$VERIFY" | head -1 | grep -oE '[0-9.]+$')
eq "fail_ratio" "${D[fail_ratio]}" "$A"
# 两轮确认
if grep -Fq 'got2=$(measure_one' "$VERIFY"; then ok "confirm_rounds = 2（首轮超 fail 复测取 min）"; else bad "未找到两轮确认实现"; fi
# 门内负载
A=$(grep -oE '^LOADS="[^"]*"' "$VERIFY" | head -1 | sed 's/LOADS="//;s/"//')
eq "gate_loads" "${D[gate_loads]}" "$A"
# 身份自检
if grep -Fq "grep -c '^fn_'" "$VERIFY" && grep -Fq '[ "$n" -ge 1 ]' "$VERIFY" && grep -Fq '[ "$n" -eq 0 ]' "$VERIFY"; then
  ok "identity：C≥1 / VM=0（grep -c '^fn_' 在位）"
else
  bad "identity 自检判据与声明不符"
fi

echo
echo "[B] baseline.tsv（门用基线表）"
# 格式头
if grep -q '格式：' "$BASE" && grep -q 'run|startup|build' "$BASE"; then ok "baseline_format 表头在位（kind:run|startup|build）"; else bad "baseline 表头格式不符"; fi
# 列数（数据行 ≥4 段：name/kind/unit/base[/note]，note 含空格）
BADD=$(grep -v '^#' "$BASE" | grep -v '^$' | awk 'NF<4{n++}END{print n+0}')
[ "$BADD" = 0 ] && ok "数据行均 ≥4 段（name/kind/unit/base[/note]）" || bad "$BADD 行字段不足"
# run 行成对（应为 8 = 4 负载 × 2 轨）
RN=$(grep -v '^#' "$BASE" | awk '$2=="run"{n++}END{print n+0}')
eq "baseline run 行数" "8" "$RN"
NPAIR=$(grep -v '^#' "$BASE" | awk '$2=="run"{print $1}' | sed 's/_[vc]m\?$//' | sort | uniq -c | awk '$1!=2{n++}END{print n+0}')
[ "$NPAIR" = 0 ] && ok "每条 run 负载 vm/c 成对" || bad "$NPAIR 条 run 负载未成对"
SN=$(grep -v '^#' "$BASE" | awk '$2=="startup"{n++}END{print n+0}'); eq "startup 行数" "1" "$SN"
BN=$(grep -v '^#' "$BASE" | awk '$2=="build"{n++}END{print n+0}');   eq "build 行数"   "1" "$BN"
# 基线表头环境与声明一致
grep -q 'AMD EPYC 7K62' "$BASE" && ok "baseline 表头写 CPU 型号" || bad "baseline 表头缺 CPU 型号"
grep -q 'gcc 11.5.0' "$BASE"    && ok "baseline 表头写 gcc 11.5.0" || bad "baseline 表头缺 gcc"
grep -q 'px 0.2.17' "$BASE"     && ok "baseline 表头写 px 0.2.17" || bad "baseline 表头缺 px 版本"

echo
echo "[C] 门注册（单一注册源）"
if grep -Fxq "${D[gate_registry_line]}" "$REG"; then ok "registry: ${D[gate_registry_line]}"; else bad "registry 未登记 m267_perf"; fi

echo
echo "[D] 实机环境"
A=$(nproc);                                          eq "env_cores"   "${D[env_cores]}"   "$A"
A=$(grep -m1 'model name' /proc/cpuinfo | sed 's/.*: //' | sed 's/ *[0-9]*-Core.*//'); eq "env_cpu" "${D[env_cpu]}" "$A"
A=$(gcc --version 2>/dev/null | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1); eq "env_gcc" "${D[env_gcc]}" "$A"
A=$("$PX" --version 2>/dev/null | head -1 | awk '{print $2}'); eq "env_px" "${D[env_px]}" "$A"
A=$(grep PRETTY_NAME /etc/os-release | sed 's/.*"\(.*\)".*/\1/' | sed 's/ (.*//'); eq "env_os" "${D[env_os]}" "$A"
A=$(free -g | awk '/^Mem:/{print $2}');              eq "env_mem_gib" "${D[env_mem_gib]}" "$A"

echo
echo "[E] 声明路径与实体"
for k in baseline_file gate_script; do
  p="${D[$k]}"; [ -f "$ROOT/$p" ] && ok "$k 存在：$p" || bad "$k 不存在：$p"
done

echo
if [ "$FAIL" = 0 ]; then
  echo "══ 结果：$PASS 通过 / 0 失败 ══"
  echo "SPEC-CONSISTENCY-OK"
  exit 0
else
  echo "══ 结果：$PASS 通过 / $FAIL 失败 ══"
  exit 1
fi
