#!/usr/bin/env bash
# ============================================================
# M89-S3-B2: bc8_verify.sh —— 构造/类型批端到端验证（bc8.px 全局断言）
# ------------------------------------------------------------
# 依赖：bc_cli 已 build（含 vm.o 的 rtcache）。
# 断言 bc8.px 顶层计算后各全局值（覆盖 NEWSTRUCT struct 构造 + GETF/SETF
#   字段读写、impl 方法注册 "Point.sum" + CALLM struct 方法桥、NEWENUM enum
#   变体折叠 + px_eq enum 比较、TypeConst const enum 值内联折叠）。
# 退出码：0=ALL PASS；1=有失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
OUT="$(./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc8.px 2>&1)"
[ $? -eq 0 ] || { echo "❌ bc_run 失败"; echo "$OUT"; exit 1; }
echo "$OUT" > /tmp/bc8_out.txt
fail=0
check() { # $1=key $2=期望值
    if ! grep -q "^$1=$2$" /tmp/bc8_out.txt; then
        echo "❌ $1 期望 [$2]，实际: $(grep "^$1=" /tmp/bc8_out.txt || echo 缺失)"
        fail=1
    else
        echo "✅ $1=$2"
    fi
}
check psum 15
check p2sum 11
check out_x 10
check out_y 4
check out_xy 15
check is_red true
check not_red false
check lv 3
check out_lv 3
check lw 2
check out_ok true
if [ $fail -eq 0 ]; then echo "== B2 bc8 全部断言 PASS =="; exit 0; fi
echo "== B2 bc8 有断言失败 =="
exit 1
