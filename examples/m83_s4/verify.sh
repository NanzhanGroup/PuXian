#!/bin/bash
# M83-S4 (Issue 18 GAP-RSA-1) 专项验证：RSA PKCS1v15-SHA256 + PEM ↔ Go rsa.SignPKCS1v15 双向互通
#   px 用 Go 私钥 PEM（PKCS8/PKCS1）签 → Go/openssl 反向验 true；px 验 Go 签（SPKI/PKCS1 公钥）true
#   + PKCS8==PKCS1 确定性 + 超长 msg + 反例矩阵
# 依赖：go（crypto/rsa）、openssl（第三方交叉验）、pxc 工具链
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
GOBIN=./build/go_s4
mkdir -p artifacts build

echo "== [1/5] Go 生成 RSA 密钥/消息/签名 artifacts =="
go build -o "$GOBIN" go_s4.go || { echo "FAIL go build"; exit 1; }
rm -f artifacts/*.pem artifacts/*.hex artifacts/msg.txt artifacts/binmsg.bin
"$GOBIN" gen artifacts || { echo "FAIL go gen"; exit 1; }
ls artifacts/

echo "== [2/5] 编译 px =="
$PX build --no-quic s4_verify.px >/tmp/m83s4_build.log 2>&1 || { echo "FAIL build"; tail -30 /tmp/m83s4_build.log; exit 1; }

echo "== [3/5] 运行 px 验证（全断言）=="
OUT=$(./build/s4_verify 2>&1); RC=$?
echo "$OUT"
[ $RC -eq 0 ] || { echo "FAIL px 运行 rc=$RC"; exit 1; }
echo "$OUT" | grep -q 'M83-S4-VERIFY-OK' || { echo "FAIL px 断言未全过"; exit 1; }

echo "== [4/5] Go 反向验 px 签名（文本 + 二进制）=="
RV=$("$GOBIN" verify artifacts/pub_spki.pem artifacts/msg.txt artifacts/px.sig.hex)
[ "$RV" = "true" ] || { echo "FAIL Go 验 px 文本签: $RV"; exit 1; }
echo "PASS Go 验 px 文本签 = true"
RV2=$("$GOBIN" verify artifacts/pub_pkcs1.pem artifacts/binmsg.bin artifacts/px_bin.sig.hex)
[ "$RV2" = "true" ] || { echo "FAIL Go 验 px 二进制签: $RV2"; exit 1; }
echo "PASS Go 验 px 二进制签 = true"

echo "== [5/5] openssl 第三方交叉验 px 签名 =="
xxd -r -p artifacts/px.sig.hex /tmp/m83s4_px.sig.bin
OV=$(openssl dgst -sha256 -verify artifacts/pub_spki.pem -signature /tmp/m83s4_px.sig.bin artifacts/msg.txt 2>&1)
echo "$OV"
echo "$OV" | grep -q 'Verified OK' || { echo "FAIL openssl 验 px 签"; exit 1; }
rm -f /tmp/m83s4_px.sig.bin
echo "m83_s4 verify done"
exit 0
