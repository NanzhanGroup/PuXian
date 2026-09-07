#!/usr/bin/env bash
# ============================================================
# M89-S3-A5: bc5_verify.sh —— Err 经 ? 链式传播（退出码 1 + stderr 载荷）
# bc5.px main 第二个 use_div(1,0)? 触发 Err("除零") 两跳就地传播
# → driver stderr "错误: 除零" + 退出码 1。若传播断链/文案错 → 断言失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc5.px > /tmp/bc5_out.txt 2> /tmp/bc5_err.txt
code=$?
if [ "$code" -ne 1 ]; then echo "❌ 期望退出码 1，实际 $code（Err 未传播到进程？）"; cat /tmp/bc5_err.txt; exit 1; fi
if ! grep -q "除零" /tmp/bc5_err.txt; then echo "❌ stderr 未含 Err 载荷「除零」"; cat /tmp/bc5_err.txt; exit 1; fi
echo "✅ Err 两跳 ? 传播 → 退出码 1 + stderr 含「除零」"
grep "除零" /tmp/bc5_err.txt | head -1
echo "== A5 bc5 Err 传播 ALL PASS =="
exit 0
