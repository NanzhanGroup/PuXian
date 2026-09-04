#!/usr/bin/env bash
# ============================================================
# examples/m65_mcp/verify.sh —— M65-S4 MCP 里程碑验收
# ------------------------------------------------------------
# S4：pxmcp 端到端（python3 模拟标准 MCP client 双向管道）：
#     initialize → tools/list（8 工具 + inputSchema）→ tools/call 每工具
#     各一次成功回包（run/fmt/lint 走 code 临时文件；test/bench/doc 走
#     demo_mcp.px file；ast 走 code）+ 错误参数/未知工具/未知方法 → exit
# 依赖：python3 + bootstrap/pxmcp + bootstrap/pxi pxfmt pxlint pxtest
#       pxbench pxdoc pxpar（自举产物，随仓库提交）
# ============================================================
set -u
cd "$(dirname "$0")/../.."
HERE=examples/m65_mcp
BOOT=bootstrap
FAIL=0

echo "== M65-S4 verify =="

if [ ! -x "$BOOT/pxmcp" ]; then
  echo "FAIL: 缺 bootstrap/pxmcp（先 tools/pxc build tools/pxmcp.px 后 cp）"
  exit 1
fi
for b in pxi pxfmt pxlint pxtest pxbench pxdoc pxpar; do
  [ -x "$BOOT/$b" ] || { echo "FAIL: 缺 bootstrap/$b"; FAIL=1; }
done
if [ $FAIL -ne 0 ]; then
  echo "== M65-S4 verify: FAILED（缺工具产物）=="
  exit 1
fi
if ! command -v python3 >/dev/null 2>&1; then
  echo "SKIP: 无 python3（MCP client 握手验证跳过）"
  exit 0
fi

OUT=$(python3 "$HERE/mcp_client_s4.py" "$BOOT/pxmcp" "$PWD/$BOOT" "$PWD/$HERE/demo_mcp.px" 2>&1)
echo "$OUT" | grep -q "ALL PASS" || { echo "FAIL: MCP client 未全绿"; echo "$OUT"; exit 1; }
echo "$OUT" | grep -q "FAIL" && { echo "FAIL: MCP client 存在失败项"; echo "$OUT"; exit 1; }
echo "PASS: MCP client 端到端（initialize/list/call 全工具）全绿"
echo ""
echo "== M65-S4 verify: ALL PASS =="
exit 0
