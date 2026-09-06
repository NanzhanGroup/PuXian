#!/bin/bash
# M83-S3 (Issue 17 GAP-ED25519-1) 专项验证：ed25519 RFC8032 ↔ Go crypto/ed25519 双向互通
#   px 验 Go 签（PEM SPKI + hex pub + 二进制 msg）→ Go 验 px 签（PEM PKCS8/seed 路径签出的 sig）
#   + RFC8032 确定性（同 seed 同 msg → px 签 == Go 签）+ 反例矩阵
# 依赖：go（crypto/ed25519 生成密钥与反向验签）、pxc 工具链
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
GOBIN=./build/go_s3
mkdir -p artifacts build

echo "== [1/4] Go 生成密钥/消息/签名 artifacts =="
go build -o "$GOBIN" go_s3.go || { echo "FAIL go build"; exit 1; }
rm -f artifacts/*.hex artifacts/*.pem artifacts/msg.txt artifacts/binmsg.bin
"$GOBIN" gen artifacts || { echo "FAIL go gen"; exit 1; }
ls artifacts/

echo "== [2/4] 编译 px =="
$PX build --no-quic s3_verify.px >/tmp/m83s3_build.log 2>&1 || { echo "FAIL build"; tail -30 /tmp/m83s3_build.log; exit 1; }

echo "== [3/4] 运行 px 验证（全断言）=="
OUT=$(./build/s3_verify 2>&1); RC=$?
echo "$OUT"
[ $RC -eq 0 ] || { echo "FAIL px 运行 rc=$RC"; exit 1; }
echo "$OUT" | grep -q 'M83-S3-VERIFY-OK' || { echo "FAIL px 断言未全过"; exit 1; }

echo "== [4/4] Go 反向验 px 签名（文本 + 二进制）=="
RV=$("$GOBIN" verify artifacts/pub.pem artifacts/msg.txt artifacts/px.sig.hex)
[ "$RV" = "true" ] || { echo "FAIL Go 验 px 文本签: $RV"; exit 1; }
echo "PASS Go 验 px 文本签 = true"
RV2=$("$GOBIN" verify artifacts/pub.pem artifacts/binmsg.bin artifacts/px_bin.sig.hex)
[ "$RV2" = "true" ] || { echo "FAIL Go 验 px 二进制签: $RV2"; exit 1; }
echo "PASS Go 验 px 二进制签 = true"
echo "m83_s3_ed25519 verify done"
exit 0
