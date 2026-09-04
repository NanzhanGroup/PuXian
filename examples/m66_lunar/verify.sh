#!/bin/bash
# M66-S4 m66_lunar 专项验证：stdlib/lunar.px 收编 std.lunar（双模式 36 PASS 一致）
# 锚点覆盖：春节/除夕/闰月/月末/边界 1900-2100 + 往返 + ws-todo lunar:M-D 落点
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
echo "== [1/2] 解释模式 pxi run =="
timeout 120 $PX run lunar_test.px 2>&1 | tail -1
echo "== [2/2] 编译模式 pxc build =="
timeout 300 $PX build --no-quic lunar_test.px >/dev/null 2>&1 || { echo "FAIL build"; exit 1; }
timeout 60 ./build/lunar_test 2>&1 | tail -1
echo "== [3/3] fmt/lint 全绿 =="
for f in lunar_test.px; do
    echo "$f lint: $($PX lint $f 2>&1 | tail -1 | sed 's/.*: //')"
    echo "$f fmt:  $($PX fmt --check $f 2>&1 | tail -1 | sed 's/.*: //')"
done
echo "m66_lunar verify done"
