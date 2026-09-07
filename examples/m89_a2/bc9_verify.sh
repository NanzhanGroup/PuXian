#!/usr/bin/env bash
# ============================================================
# M89-S3-B3: bc9_verify.sh —— 推导式/match/闭包 端到端验证（bc9.px 全局断言）
# ------------------------------------------------------------
# 依赖：bc_cli 已 build（含 vm.o 的 rtcache）。
# 断言 bc9.px 顶层计算后各全局值（覆盖 ListComp/DictComp 循环展开 + cond 过滤、
#   GenExp NEWGEN 惰性（transform 闭包）、for-in 迭代生成器物化、Closure 无捕获
#   闭包 LOADK PXK_FUNC、Block 块表达式）。
# 退出码：0=ALL PASS；1=有失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
OUT="$(./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc9.px 2>&1)"
[ $? -eq 0 ] || { echo "❌ bc_run 失败"; echo "$OUT"; exit 1; }
echo "$OUT" > /tmp/bc9_out.txt
fail=0
check() { # $1=key $2=期望值
    if ! grep -q "^$1=$2$" /tmp/bc9_out.txt; then
        echo "❌ $1 期望 [$2]，实际: $(grep "^$1=" /tmp/bc9_out.txt || echo 缺失)"
        fail=1
    else
        echo "✅ $1=$2"
    fi
}
check sqn 4
check sq0 1
check sq3 16
check evn 3
check ev0 2
check d1n 3
check d1a 'a!'
check ggn 3
check gg0 10
check gg2 30
check cr 12
check ok true
if [ $fail -eq 0 ]; then echo "== B3 bc9 全部断言 PASS =="; exit 0; fi
echo "== B3 bc9 有断言失败 =="
exit 1
