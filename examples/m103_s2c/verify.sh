#!/bin/bash
# M103-S2c (Issue 30 GAP-YAML-SER) 专项验证：yaml_stringify ↔ Go yaml.v3 双向互认
#   [1] Go gen：yaml.Marshal 写 go_out.yaml（供 px parse 读，验证 Go 写→px 读）
#   [2] px 断言：roundtrip deep-equal（api-server 形状 / 特殊字符串矩阵 / 标量保型）
#       + 读 go_out.yaml（Go 写→px 读）+ dump px_out.yaml
#   [3] Go check：yaml.Unmarshal 读 px_out.yaml 断言（px 写→Go 读）
# 依赖：go（gopkg.in/yaml.v3，go.mod 已固定）、pxc 工具链；用法：bash verify.sh
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
GOBIN=./build/go_s2c
# M404：**自备 go 构建环境** —— 分片/无人值守环境里 HOME 可能未设、GOCACHE 不可写，
#   那时 `go build` 会以「neither GOCACHE nor HOME is defined」失败，
#   而这不是门的判据红、是环境缺口（M403 实测：同一门本机绿、分片红）。⇒ 门内自给自足。
# M404（缺陷 773）：GOPATH 指到 /tmp 后 GOMODCACHE 是空的 ⇒ `go build` 必须**联网**下 yaml.v3，
#   而分片是 netns 隔离的 ⇒ 必红（实测 M404 全量门 shard3：0s 报 FAIL go build）。
#   ⇒ 模块源码入仓（vendor/）：`go build` 自动 -mod=vendor，离线可跑；更新依赖走 `go mod vendor`。
export GOCACHE="${GOCACHE:-${TMPDIR:-/tmp}/m404-gocache-s2c}"
export GOPATH="${GOPATH:-${TMPDIR:-/tmp}/m404-gopath-s2c}"
export HOME="${HOME:-${TMPDIR:-/tmp}}"
mkdir -p "$GOCACHE" "$GOPATH"
mkdir -p build artifacts
rm -f artifacts/*.yaml

echo "== [1/4] Go gen（yaml.Marshal → go_out.yaml，供 px 反向读）=="
go build -o build/go_s2c . || { echo "FAIL go build"; exit 1; }
./build/go_s2c gen || { echo "FAIL go gen"; exit 1; }

echo "== [2/4] px build =="
$PX build --no-quic yaml_stringify_test.px >/tmp/m103s2c_build.log 2>&1 || { echo "FAIL build"; tail -20 /tmp/m103s2c_build.log; exit 1; }

echo "== [3/4] px 运行断言（roundtrip + Go 写→px 读 + dump）=="
OUT=$(./build/yaml_stringify_test 2>&1)
echo "$OUT"
echo "$OUT" | grep -q 'YAML_STRINGIFY_TEST:.*0F' || { echo "FAIL px 断言未全过"; exit 1; }

echo "== [4/4] Go check（px 写 → Go yaml 读回互认）=="
./build/go_s2c check || { echo "FAIL Go check"; exit 1; }
echo "m103_s2c_yaml_stringify verify done"
exit 0
