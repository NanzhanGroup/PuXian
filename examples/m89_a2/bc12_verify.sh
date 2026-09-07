#!/usr/bin/env bash
# ============================================================
# M89-S3-B5: bc12_verify.sh —— select 语句端到端验证（bc12.px stdout 断言）
# ------------------------------------------------------------
# 依赖：bc_cli 已 build（含 vm.o 的 rtcache）。
# 断言 bc12.px stdout：worker spawn 发 6 → select 命中绑定 x → "sel: 6"；
#   随后 "done"。
# 退出码：0=ALL PASS；1=有失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
OUT="$(timeout 30 ./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc12.px 2>&1)"
[ $? -eq 0 ] || { echo "❌ bc_run 失败/超时"; echo "$OUT"; exit 1; }
echo "$OUT" > /tmp/bc12_out.txt
fail=0
grep -q "^sel: 6$" /tmp/bc12_out.txt && echo "✅ sel: 6" || { echo "❌ 缺 sel: 6（实际：$(grep sel /tmp/bc12_out.txt || echo 无)）"; fail=1; }
grep -q "^done$" /tmp/bc12_out.txt && echo "✅ done" || { echo "❌ 缺 done"; fail=1; }
if [ $fail -eq 0 ]; then echo "== B5 bc12 select ALL PASS =="; exit 0; fi
echo "== B5 bc12 select 有断言失败 =="
exit 1
