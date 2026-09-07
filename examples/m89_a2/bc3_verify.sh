#!/usr/bin/env bash
# ============================================================
# M89-S3-A3: bc3_verify.sh —— 控制流批端到端验证（短路/??/IfExpr/break-continue）
# ------------------------------------------------------------
# 依赖：bc_cli 已 build + bc_run.sh（含 vm.o 的 rtcache）。
# 退出码：0=ALL PASS；1=有失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
OUT="$(./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc3.px 2>&1)"
[ $? -eq 0 ] || { echo "❌ bc_run 失败"; echo "$OUT"; exit 1; }
echo "$OUT" > /tmp/bc3_out.txt
fail=0
check() { # $1=key $2=期望值
    if ! grep -q "^$1=$2$" /tmp/bc3_out.txt; then
        echo "❌ $1 期望 [$2]，实际: $(grep "^$1=" /tmp/bc3_out.txt || echo 缺失)"
        fail=1
    else
        echo "✅ $1=$2"
    fi
}
check a1 0
check a2 7
check o1 9
check o2 3
check n1 42
check n2 0
check n3 x
check ie1 yes
check ie2 no
check sum 25
check flags 5
if [ $fail -eq 0 ]; then echo "== A3 bc3 全部断言 PASS =="; exit 0; fi
echo "== A3 bc3 有断言失败 =="
exit 1
