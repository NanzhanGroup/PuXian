#!/bin/bash
# M84-S2 (Issue 21 GAP-HMAC-1) 专项验证：hmac_sha256 native + sha256 bytes/含 NUL 增强
#   S1 RFC4231 固定向量（TC1-TC4/TC6，Go crypto/hmac 期望 + TC1/TC2 官方向量双印证）
#   S2 文本/二进制 key × 文本/二进制 msg（含 NUL）与 Go 逐字节互通
#   S3 sha256(bytes 含 NUL)==Go sha256.Sum256（旧 strlen 截断修复证明）+ ASCII/空串回归
#   S4 腾讯云 TC3-HMAC-SHA256 4 级链端到端对拍 Go（kDate→kService→kSigning→signature）
#   S5 篡改敏感性
# 依赖：go（crypto/hmac+sha256 权威对拍）、tools/pxc
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
GOBIN=./build/go_s2
mkdir -p artifacts build

echo "== [1/4] Go 生成 artifacts（RFC4231 向量期望 + TC3 链 + sha256 期望）=="
go build -o "$GOBIN" go_s2.go || { echo "FAIL go build"; exit 1; }
rm -f artifacts/*.hex artifacts/*.txt artifacts/*.bin
"$GOBIN" gen artifacts || { echo "FAIL go gen"; exit 1; }
ls artifacts/ | tr '\n' ' '; echo

echo "== [2/4] 编译 px（runtime.c 变更 → pxc 自动重建 runtime）=="
$PX build --no-quic s2_verify.px >/tmp/m84s2_build.log 2>&1 || { echo "FAIL build"; tail -30 /tmp/m84s2_build.log; exit 1; }

echo "== [3/4] 运行 px 验证（全断言）=="
OUT=$(./build/s2_verify 2>&1); RC=$?
echo "$OUT"
[ $RC -eq 0 ] || { echo "FAIL px 运行 rc=$RC"; exit 1; }
echo "$OUT" | grep -q 'M84-S2-VERIFY-OK' || { echo "FAIL px 断言未全过"; exit 1; }

echo "== [4/4] native 表对账（hmac_sha256 计入 → 300）=="
cd ../..
bash tools/gen_native_table.sh >/tmp/m84s2_gentab.log 2>&1 || { echo "FAIL gen_native_table"; cat /tmp/m84s2_gentab.log; exit 1; }
N=$(python3 -c "import json;print(json.load(open('docs/native_index.json'))['count'])")
[ "$N" = "300" ] || { echo "FAIL native count=$N 期望 300"; exit 1; }
echo "m84_s2_hmac verify done (native=$N)"
exit 0
