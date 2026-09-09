#!/bin/bash
# M103-S2c (Issue 30 GAP-YAML-SER) 专项验证：yaml_stringify ↔ Go yaml.v3 双向互认
#   [1] Go gen：yaml.Marshal 写 go_out.yaml（供 px parse 读，验证 Go 写→px 读）
#   [2] px 断言：roundtrip deep-equal（api-server 形状 / 特殊字符串矩阵 / 标量保型）
#       + 读 go_out.yaml（Go 写→px 读）+ dump px_out.yaml
#   [3] Go check：yaml.Unmarshal 读 px_out.yaml 断言（px 写→Go 读）
# 依赖：go（gopkg.in/yaml.v3，go.mod 已固定）、pxc 工具链；用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
GOBIN=./build/go_s2c
mkdir -p build artifacts
rm -f artifacts/*.yaml

echo "== [1/4] Go gen（yaml.Marshal → go_out.yaml，供 px 反向读）=="
(cd "$(dirname "$0")" && go build -o build/go_s2c .) || { echo "FAIL go build"; exit 1; }
(cd "$(dirname "$0")" && ./build/go_s2c gen) || { echo "FAIL go gen"; exit 1; }

echo "== [2/4] px build =="
$PX build --no-quic yaml_stringify_test.px >/tmp/m103s2c_build.log 2>&1 || { echo "FAIL build"; tail -20 /tmp/m103s2c_build.log; exit 1; }

echo "== [3/4] px 运行断言（roundtrip + Go 写→px 读 + dump）=="
OUT=$(./build/yaml_stringify_test 2>&1)
echo "$OUT"
echo "$OUT" | grep -q 'YAML_STRINGIFY_TEST:.*0F' || { echo "FAIL px 断言未全过"; exit 1; }

echo "== [4/4] Go check（px 写 → Go yaml 读回互认）=="
(cd "$(dirname "$0")" && ./build/go_s2c check) || { echo "FAIL Go check"; exit 1; }
echo "m103_s2c_yaml_stringify verify done"
exit 0
