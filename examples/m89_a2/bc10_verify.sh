#!/usr/bin/env bash
# ============================================================
# M89-S3-B3b: bc10_verify.sh —— match 模式匹配端到端验证（bc10.px 全局断言）
# ------------------------------------------------------------
# 依赖：bc_cli 已 build（含 vm.o 的 rtcache）。
# 断言 bc10.px 顶层计算后各全局值（覆盖 match 表达式：enum 变体 PatConstructor →
#   ENUMVAR variant 比较、PatWildcard 兜底、PatLiteral 字面量匹配、match 表达式嵌套在
#   def 内 + 顶层 var 初始值）。
# 退出码：0=ALL PASS；1=有失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
OUT="$(./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc10.px 2>&1)"
[ $? -eq 0 ] || { echo "❌ bc_run 失败"; echo "$OUT"; exit 1; }
echo "$OUT" > /tmp/bc10_out.txt
fail=0
check() { # $1=key $2=期望值
    if ! grep -q "^$1=$2$" /tmp/bc10_out.txt; then
        echo "❌ $1 期望 [$2]，实际: $(grep "^$1=" /tmp/bc10_out.txt || echo 缺失)"
        fail=1
    else
        echo "✅ $1=$2"
    fi
}
check n1 red
check n2 other
check n3 green
check x1 many
check x2 one
check ok true
if [ $fail -eq 0 ]; then echo "== B3b bc10 全部断言 PASS =="; exit 0; fi
echo "== B3b bc10 有断言失败 =="
exit 1
