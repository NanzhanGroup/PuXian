#!/usr/bin/env bash
# ============================================================
# examples/m65_lsp/verify.sh —— M65 LSP 里程碑验收
# ------------------------------------------------------------
# S1：双模式 jsonrpc 回环 33 PASS + os_spawn_capture 冒烟 5 PASS
# S2：pxlsp 端到端（python3 模拟标准 LSP client 双向管道）：
#     initialize → didOpen(didChange/didSave/didClose) → publishDiagnostics
#     → shutdown → exit，17 断言全绿
# S3：completion / definition / hover 增强 e2e：
#     demo_s3.px（def/struct/enum/trait/impl/var/局部/文档注释）+ 真实文件
#     selfhost/astdump.px + 错误输入不崩，39 断言全绿
# 依赖：python3 + bootstrap/pxlsp + bootstrap/pxcheck（自举产物，随仓库提交）
# ============================================================
set -u
cd "$(dirname "$0")/../.."
HERE=examples/m65_lsp
BOOT=bootstrap
FAIL=0

echo "== M65-S1 verify =="

# 1) 编译 jsonrpc_core 自测 + spawncap 冒烟
$BOOT/pxc build $HERE/jsonrpc_selftest.px >/dev/null 2>&1 || { echo "FAIL: build jsonrpc_selftest"; exit 1; }
$BOOT/pxc build $HERE/spawncap_selftest.px >/dev/null 2>&1 || { echo "FAIL: build spawncap_selftest"; exit 1; }

# 2) 双模式运行并断言
for MODE in pxi compiled; do
  if [ "$MODE" = pxi ]; then
    JR_OUT=$($BOOT/pxi $HERE/jsonrpc_selftest.px 2>&1)
    SP_OUT=$($BOOT/pxi $HERE/spawncap_selftest.px 2>&1)
  else
    JR_OUT=$($HERE/build/jsonrpc_selftest 2>&1)
    SP_OUT=$($HERE/build/spawncap_selftest 2>&1)
  fi
  echo "$JR_OUT" | grep -q "33 PASS, 0 FAIL" || { echo "FAIL[$MODE]: jsonrpc_selftest 未全绿"; echo "$JR_OUT"; FAIL=1; }
  echo "$SP_OUT" | grep -q "5 PASS, 0 FAIL" || { echo "FAIL[$MODE]: spawncap_selftest 未全绿"; echo "$SP_OUT"; FAIL=1; }
  echo "PASS[$MODE]: jsonrpc_selftest + spawncap_selftest"
done

echo ""
echo "== M65-S2 verify =="

# S2：pxlsp 端到端握手 + 诊断（需自举产物 pxlsp/pxcheck 与 python3）
if [ ! -x "$BOOT/pxlsp" ] || [ ! -x "$BOOT/pxcheck" ]; then
  echo "FAIL: 缺 bootstrap/pxlsp 或 bootstrap/pxcheck（先 tools/pxc build 后 cp）"
  FAIL=1
elif ! command -v python3 >/dev/null 2>&1; then
  echo "SKIP: 无 python3（S2 client 握手验证跳过）"
else
  LSP_OUT=$(python3 "$HERE/lsp_client_s2.py" "$BOOT/pxlsp" "$BOOT/pxcheck" 2>&1)
  echo "$LSP_OUT" | grep -q "ALL PASS" || { echo "FAIL: S2 client 未全绿"; echo "$LSP_OUT"; FAIL=1; }
  echo "PASS: S2 client 端到端握手 + 诊断全绿"
fi

echo ""
echo "== M65-S3 verify =="

# S3：completion/definition/hover e2e（demo + 真实文件 + 错误输入不崩）
if [ ! -x "$BOOT/pxlsp" ] || [ ! -x "$BOOT/pxcheck" ]; then
  echo "FAIL: 缺 bootstrap/pxlsp 或 bootstrap/pxcheck（S3 无法运行）"
  FAIL=1
elif ! command -v python3 >/dev/null 2>&1; then
  echo "SKIP: 无 python3（S3 client 验证跳过）"
else
  S3_OUT=$(python3 "$HERE/lsp_client_s3.py" "$BOOT/pxlsp" "$BOOT/pxcheck" "$PWD" 2>&1)
  echo "$S3_OUT" | grep -q "ALL PASS" || { echo "FAIL: S3 client 未全绿"; echo "$S3_OUT"; FAIL=1; }
  echo "PASS: S3 client completion/definition/hover 全绿"
fi

if [ $FAIL -eq 0 ]; then
  echo ""
  echo "== M65-S1/S2/S3 verify: ALL PASS =="
  exit 0
fi
echo ""
echo "== M65-S1/S2/S3 verify: FAILED =="
exit 1
