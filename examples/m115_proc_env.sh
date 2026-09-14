#!/usr/bin/env bash
# ============================================================
# M115 验证脚本 —— 服务进程/环境原语（双模式 + 负控）
#   ① 双模式跑 examples/m115_proc_env.px，两轨都必须 PASS 且输出末行 == M115_PROC_ENV_OK
#   ② 负控：故意把断言值改错 → **必须失败**（防门自欺）
# 用法：bash examples/m115_proc_env.sh
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/.."
PX=./tools/px
SRC=examples/m115_proc_env.px
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M115 验证：$SRC =="

echo "[1/3] 解释轨（px run）"
$PX run "$SRC" > "$TMP/interp.log" 2>&1; rc=$?
if [ $rc -eq 0 ] && tail -1 "$TMP/interp.log" | grep -q M115_PROC_ENV_OK; then
  ok "解释轨通过（$(grep -c '^\[m115\]' "$TMP/interp.log") 组断言）"
else
  bad "解释轨失败 rc=$rc"; tail -8 "$TMP/interp.log"
fi

echo "[2/3] 编译轨（px build）"
$PX build "$SRC" > "$TMP/build.log" 2>&1 && ./examples/build/m115_proc_env > "$TMP/compiled.log" 2>&1; rc=$?
if [ $rc -eq 0 ] && tail -1 "$TMP/compiled.log" | grep -q M115_PROC_ENV_OK; then
  ok "编译轨通过（$(grep -c '^\[m115\]' "$TMP/compiled.log") 组断言）"
else
  bad "编译轨失败 rc=$rc"; tail -8 "$TMP/compiled.log"
fi

echo "[3/3] 负控（把 diff<=2 改成 diff<0 → 必须红）"
sed 's/assert(diff <= 2/assert(diff < 0/' "$SRC" > "$TMP/neg.px"
if $PX run "$TMP/neg.px" > "$TMP/neg.log" 2>&1; then
  bad "负控未生效：故意错值仍然通过（门在自欺）"
elif grep -q "now_sec 应与" "$TMP/neg.log"; then
  ok "负控生效（因 now_sec 断言而红，非其他原因）"
else
  bad "负控红了但**不是**目标断言失败（门判据不纯）：$(tail -2 "$TMP/neg.log" | tr '\n' ' ')"
fi

echo "== 结果：通过 $pass · 失败 $fail =="
[ "$fail" -eq 0 ]
