#!/usr/bin/env bash
# ============================================================
# M89-S3-B1: bc7_verify.sh —— 容器/字段/方法桥/For 端到端验证
# ------------------------------------------------------------
# 依赖：bc_cli 已 build（含 vm.o 的 rtcache）。
# 断言 bc7.px 顶层计算后各全局值（覆盖 NEWLIST/NEWTUPLE/NEWDICT、
#   INDEX/SETIDX/SLICE、GETF(dict 字段)、CALLM 方法桥、For 迭代、复合赋值）。
# 退出码：0=ALL PASS；1=有失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
OUT="$(./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc7.px 2>&1)"
[ $? -eq 0 ] || { echo "❌ bc_run 失败"; echo "$OUT"; exit 1; }
echo "$OUT" > /tmp/bc7_out.txt
fail=0
check() { # $1=key $2=期望值
    if ! grep -q "^$1=$2$" /tmp/bc7_out.txt; then
        echo "❌ $1 期望 [$2]，实际: $(grep "^$1=" /tmp/bc7_out.txt || echo 缺失)"
        fail=1
    else
        echo "✅ $1=$2"
    fi
}
check x0 1
check x1 99
check sl0 99
check sln 2
check total 112
check up HI
check c1 b
check dk 42
check dj 3
check keys_n 2
check gacc 15
check out_ok true
check out_ok2 true
if [ $fail -eq 0 ]; then echo "== B1 bc7 全部断言 PASS =="; exit 0; fi
echo "== B1 bc7 有断言失败 =="
exit 1
