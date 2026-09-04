#!/bin/bash
# M66-S3 m66_pxml 专项验证：stdlib/pxml.px 收编 + PXML 规范 + Dogfood 闭环
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
echo "== [1/4] pxml_test 双模式（解释 68 PASS + 编译 68 PASS 一致）=="
timeout 90 $PX run pxml_test.px 2>&1 | tail -1
$PX build --no-quic pxml_test.px >/dev/null 2>&1 || { echo "FAIL build pxml_test"; exit 1; }
timeout 30 ./build/pxml_test 2>&1 | tail -1
echo "== [2/4] Dogfood demo（加载 app.pxml，decrypt=false）双模式 =="
timeout 90 $PX run demo.px 2>&1 | tail -2
$PX build --no-quic demo.px >/dev/null 2>&1 || { echo "FAIL build demo"; exit 1; }
timeout 30 ./build/demo 2>&1 | tail -2
echo "== [3/4] Dogfood demo_enc（ENC 加解密还原，decrypt=true，需 PXML_MASTER_KEY）=="
PXML_MASTER_KEY=0123456789abcdef0123456789abcdef $PX build --no-quic demo_enc.px >/dev/null 2>&1 || { echo "FAIL build demo_enc"; exit 1; }
PXML_MASTER_KEY=0123456789abcdef0123456789abcdef timeout 30 ./build/demo_enc 2>&1 | tail -2
echo "== [4/4] fmt/lint 全绿 =="
for f in pxml_test.px demo.px demo_enc.px config_loader.px; do
    echo "$f lint: $($PX lint $f 2>&1 | tail -1 | sed 's/.*: //')"
    echo "$f fmt:  $($PX fmt --check $f 2>&1 | tail -1 | sed 's/.*: //')"
done
echo "m66_pxml verify done"
