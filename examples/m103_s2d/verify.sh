#!/bin/bash
# M103-S2d (Issue 30 GAP-IMG) 专项验证：img_decode/scale/encode_jpeg ↔ Go image 对拍
#   [1] Go gen：scene.png→512×384 jpeg q70 (go_ref.jpg 基准) + 200×150 渐变 jpeg (go_input.jpg)
#   [2] px 断言：PNG decode/等比缩放 ≤512/JPEG q70 魔数+decode back/JPEG 输入/畸形 Err
#   [3] Go check：px_out.jpg 可被 image.Decode 解析 + 尺寸 512×384 + 体积同量级
# 依赖：go（image/png/jpeg 标准库）、pxc 工具链；用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
GOBIN=./build/go_img
mkdir -p build artifacts
rm -f artifacts/*.jpg

echo "== [1/4] Go gen（参照图 + JPEG 输入）=="
go build -o "$GOBIN" go_img.go || { echo "FAIL go build"; exit 1; }
"$GOBIN" gen || { echo "FAIL go gen"; exit 1; }

echo "== [2/4] px build =="
$PX build --no-quic img_test.px >/tmp/m103s2d_build.log 2>&1 || { echo "FAIL build"; tail -20 /tmp/m103s2d_build.log; exit 1; }

echo "== [3/4] px 运行断言（decode/scale/jpeg + Go 输入反向）=="
OUT=$(./build/img_test 2>&1)
echo "$OUT"
echo "$OUT" | grep -q 'IMG_TEST:.*0F' || { echo "FAIL px 断言未全过"; exit 1; }

echo "== [4/4] Go check（px 产出 JPEG 格式合法 + 尺寸/体积对拍）=="
"$GOBIN" check || { echo "FAIL Go check"; exit 1; }
echo "m103_s2d_img verify done"
exit 0
