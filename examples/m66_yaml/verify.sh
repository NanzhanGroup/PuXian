#!/bin/bash
# M66-S2 m66_yaml 专项验证：stdlib/yaml.px 收编（双模式 35 PASS 一致）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
echo "== [1/2] 解释模式 pxi run =="
$PX run yaml_test.px 2>&1 | tail -3
echo "== [2/2] 编译模式 pxc build =="
$PX build --no-quic yaml_test.px >/dev/null 2>&1 || { echo "FAIL build"; exit 1; }
./build/yaml_test 2>&1 | tail -3
echo "m66_yaml verify done"
