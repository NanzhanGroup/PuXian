#!/usr/bin/env bash
# ============================================================
# M89-S3-B5: bc11_verify.sh —— 并发原语端到端验证（bc11.px stdout 断言）
# ------------------------------------------------------------
# 依赖：bc_cli 已 build（含 vm.o 的 rtcache）。
# 断言 bc11.px 运行 stdout（spawn 两 worker 各自 ch.send(id*2)，main 阻塞
#   ch.recv 两值求和 print）→ "sum: 6"（1*2 + 2*2，与接收顺序无关）。
# 退出码：0=ALL PASS；1=有失败。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
OUT="$(./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc11.px 2>&1)"
[ $? -eq 0 ] || { echo "❌ bc_run 失败"; echo "$OUT"; exit 1; }
echo "$OUT" > /tmp/bc11_out.txt
fail=0
grep -q "^sum: 6$" /tmp/bc11_out.txt && echo "✅ sum: 6" || { echo "❌ 缺 sum: 6（实际：$(grep sum /tmp/bc11_out.txt || echo 无)）"; fail=1; }
if [ $fail -eq 0 ]; then echo "== B5 bc11 并发原语 ALL PASS =="; exit 0; fi
echo "== B5 bc11 有断言失败 =="
exit 1
