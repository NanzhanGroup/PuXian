#!/bin/bash
# M103-S2b (Issue 29 GAP-ED25519-2) 专项验证：ed25519_keygen ↔ Go crypto/ed25519 + x509
#   px 断言（keygen dict/hex/PEM 两路签名验签 + Go SPKI 反向验签）+ Go 字节级互通：
#     SPKI PEM pub == pk_hex / PKCS8 seed 派生 pub == pk_hex / Go 验 px 签
# 依赖：go、pxc 工具链；用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
GOBIN=./build/go_s2b
mkdir -p build artifacts
rm -f artifacts/*

echo "== [1/4] Go gen（密钥对 + 消息 + 签名，供 px 反向验）=="
go build -o "$GOBIN" go_s2b.go || { echo "FAIL go build"; exit 1; }
"$GOBIN" gen || { echo "FAIL go gen"; exit 1; }

echo "== [2/4] px build =="
$PX build --no-quic keygen_test.px >/tmp/m103s2b_build.log 2>&1 || { echo "FAIL build"; tail -20 /tmp/m103s2b_build.log; exit 1; }

echo "== [3/4] px 运行断言（keygen 自洽 + Go 反向验）=="
OUT=$(./build/keygen_test 2>&1)
echo "$OUT"
echo "$OUT" | grep -q 'ED25519_KEYGEN_TEST:.*0F' || { echo "FAIL px 断言未全过"; exit 1; }

echo "== [4/4] Go 反向字节级互通（读 px 产物）=="
"$GOBIN" verify || { echo "FAIL Go verify"; exit 1; }
echo "m103_s2b_ed25519_keygen verify done"
exit 0
