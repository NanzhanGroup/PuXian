#!/usr/bin/env bash
# ============================================================
# examples/m65_lsp/verify.sh —— M65-S1 验收（jsonrpc 底座 + runtime 补丁）
# ------------------------------------------------------------
# 断言：双模式 jsonrpc 回环 33 PASS + os_spawn_capture 冒烟 4 PASS
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

if [ $FAIL -eq 0 ]; then
  echo "== M65-S1 verify: ALL PASS =="
  exit 0
fi
echo "== M65-S1 verify: FAILED =="
exit 1
