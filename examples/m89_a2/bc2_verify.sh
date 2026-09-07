#!/usr/bin/env bash
# ============================================================
# M89-S3-A2: bc2_verify.sh —— 运算批端到端验证（bc2.px 顶层值断言）
# ------------------------------------------------------------
# 依赖：bc_cli 已 build + bc_run.sh 可用（含 vm.o 的 rtcache）。
# 断言 bc2.px 顶层计算后各全局值（覆盖算术/比较/位/一元/if-else/while）。
# 退出码：0=ALL PASS；1=有失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
OUT="$(./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc2.px 2>&1)"
[ $? -eq 0 ] || { echo "❌ bc_run 失败"; echo "$OUT"; exit 1; }
echo "$OUT" > /tmp/bc2_out.txt
fail=0
check() { # $1=key $2=期望值
    if ! grep -q "^$1=$2$" /tmp/bc2_out.txt; then
        echo "❌ $1 期望 [$2]，实际: $(grep "^$1=" /tmp/bc2_out.txt || echo 缺失)"
        fail=1
    else
        echo "✅ $1=$2"
    fi
}
check arith 13
check idiv 3
check modv 1
check powv 256
check bita 2
check bito 5
check bitx 4
check shlv 16
check negv -7
check notv true
check cmp1 true
check cmp2 false
check cmps true
check scon abcd
check acc 10
check res big
if [ $fail -eq 0 ]; then echo "== A2 bc2 全部断言 PASS =="; exit 0; fi
echo "== A2 bc2 有断言失败 =="
exit 1
