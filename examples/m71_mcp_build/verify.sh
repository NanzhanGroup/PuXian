#!/usr/bin/env bash
# ============================================================
# examples/m71_mcp_build/verify.sh —— M71-S3 MCP build e2e 验收
# ------------------------------------------------------------
# 「写→验→交付」最后一条腿：MCP 第 9 工具 build（pxmcp spawn tools/pxc，
# 与 CLI 完全同语义）。client 断言 initialize/tools-list(9)/call build；
# 本脚本复核产物真实存在 + 静态链接 + 运行输出。
# 依赖：tools/pxc + bootstrap/pxmcp（9 工具重建版）+ python3
# ============================================================
set -u
cd "$(dirname "$0")/../.."
HERE=examples/m71_mcp_build
FAIL=0

echo "== M71-S3 MCP build verify =="

[ -x bootstrap/pxmcp ] || { echo "FAIL: 缺 bootstrap/pxmcp（先 tools/pxc build tools/pxmcp.px 后 cp）"; exit 1; }
[ -x tools/pxc ] || { echo "FAIL: 缺 tools/pxc"; exit 1; }

# e2e：tools/pxc mcp 由 client 驱动（注入 PX_BOOT/PX_PXI/PX_PXC）
python3 "$HERE/mcp_client.py" || { echo "== M71-S3 verify: FAILED =="; exit 1; }

# 产物真实存在 + 静态链接 + 运行输出
OUT="$HERE/build/hello_build"
if [ -x "$OUT" ]; then
    file "$OUT" | grep -q "statically linked" || { echo "FAIL: 产物非静态链接"; FAIL=1; }
    O="$("$OUT")"
    [ "$O" = "hello from mcp build" ] || { echo "FAIL: 产物运行输出=[$O]"; FAIL=1; }
    echo "  产物: $OUT（$(stat -c %s "$OUT")B, statically linked, 运行输出正确）"
else
    echo "FAIL: 产物缺失 $OUT"; FAIL=1
fi

[ "$FAIL" = 0 ] && echo "== M71-S3 verify: ALL PASS ==" || echo "== M71-S3 verify: FAILED =="
exit "$FAIL"
