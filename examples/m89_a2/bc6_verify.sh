#!/usr/bin/env bash
# ============================================================
# M89-S3-A5: bc6_verify.sh —— FORCE(!) Ok 解包 + null 经 ? 传播
# bc6.px main: get_val()! = 7（FORCE Ok 解包）；ret_null()? → null 就地传播
#   → main RET null → driver _code=0 exit 0；若 null 未传播 → return 8 exit 8；
#   若 FORCE 遇 Err/null 错 → px_error exit 非 0。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
./examples/m89_a2/bc_run.sh selfhost/cases_bc/bc6.px > /tmp/bc6_out.txt 2> /tmp/bc6_err.txt
code=$?
if [ "$code" -ne 0 ]; then
    echo "❌ 期望退出码 0（null ? 传播 → main RET null），实际 $code"
    echo "—— 若为 8：null 未传播（bug）；若其他：FORCE/传播错"
    cat /tmp/bc6_err.txt
    exit 1
fi
echo "✅ FORCE(!) Ok 解包 + null 经 ? 传播 → 退出码 0"
echo "== A5 bc6 FORCE/null 传播 ALL PASS =="
exit 0
