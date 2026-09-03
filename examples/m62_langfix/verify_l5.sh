#!/bin/bash
# M62-L5 块作用域验证（需新 bootstrap/pxc：含 codegen 变量提升 hoist）
# 修复前：fp_block.px 编译模式 C undeclared（build 失败）；修复后双模式一致 PASS。
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
mkdir -p build
if ! $PX run fp_block.px > /tmp/m62_l5.run 2>&1; then
  echo "❌ 解释模式运行失败"; tail -3 /tmp/m62_l5.run; exit 1
fi
if ! $PX build --no-quic fp_block.px > /tmp/m62_l5.buildlog 2>&1; then
  echo "❌ 编译失败（hoist 未生效？）"; tail -6 /tmp/m62_l5.buildlog; exit 1
fi
./build/fp_block > /tmp/m62_l5.bin 2>&1 || { echo "❌ 编译产物运行失败"; exit 1; }
echo "解释: $(tail -1 /tmp/m62_l5.run)"
echo "编译: $(tail -1 /tmp/m62_l5.bin)"
if diff -q /tmp/m62_l5.run /tmp/m62_l5.bin > /dev/null && grep -q "L5 BLOCK OK" /tmp/m62_l5.bin; then
  echo "M62-L5 PASS"
else
  echo "M62-L5 FAIL"; exit 1
fi
