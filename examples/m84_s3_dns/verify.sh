#!/bin/bash
# M84-S3 (Issue 22 GAP-DNS-1) 专项验证：dns_lookup 原生域名解析
#   S1 localhost 确定性断言（/etc/hosts 语义，offline 安全）
#   S2 失败可判定：NXDOMAIN(.invalid)/空名 → Err（is_err 可判定，区别于空 list）
#   S3 公网域对拍（网络可用时）：Go net.LookupIP 现算期望 → px 交集 ≥1 + AAAA 域返回 IPv6；
#      断网自动跳过并标注（S1/S2/S4 仍全量断言）
#   S4 健壮性（重复调用稳定）
#   [4/4] native 表对账（dns_lookup 计入 → 301）
# 注：pxi 解释模式双跑留到 M84-S4 收口（relink bootstrap/pxi 后）统一抽查；
#     本批 verify 只验 pxc build 编译产物（runtime.c 自动重建）。
# 依赖：go（net.LookupIP oracle）、getent（系统解析器探测）、tools/pxc
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
GOBIN=./build/go_s3
mkdir -p artifacts build
rm -f artifacts/net_on artifacts/go_*.txt

echo "== [1/4] Go oracle（net.LookupIP）现算期望 + 网络探测 =="
if go build -o "$GOBIN" go_s3.go; then
    "$GOBIN" gen artifacts
else
    echo "WARN: go build 失败，S3 公网项将跳过"
fi
if timeout 6 getent ahostsv4 www.qq.com >/dev/null 2>&1 && [ -s artifacts/go_v4_qq.txt ]; then
    touch artifacts/net_on
    echo "网络可用：公网对拍开启"
else
    echo "网络不可用/解析失败：S3 公网项跳过（仅本地断言）"
fi
echo "artifacts: $(ls artifacts/ 2>/dev/null | tr '\n' ' ')"

echo "== [2/4] 编译 px（runtime.c 变更 → pxc 自动重建 runtime）=="
$PX build --no-quic s3_verify.px >/tmp/m84s3_build.log 2>&1 || { echo "FAIL build"; tail -30 /tmp/m84s3_build.log; exit 1; }

echo "== [3/4] 运行编译产物验证（全断言）=="
OUT=$(./build/s3_verify 2>&1); RC=$?
echo "$OUT"
[ $RC -eq 0 ] || { echo "FAIL px 运行 rc=$RC"; exit 1; }
echo "$OUT" | grep -q 'M84-S3-VERIFY-OK' || { echo "FAIL px 断言未全过"; exit 1; }

echo "== [4/4] native 表对账（dns_lookup 计入，基线 ≥301；最终精确值 S4 收口统一核）=="
cd ../..
bash tools/gen_native_table.sh >/tmp/m84s3_gentab.log 2>&1 || { echo "FAIL gen_native_table"; cat /tmp/m84s3_gentab.log; exit 1; }
N=$(python3 -c "import json;print(json.load(open('docs/native_index.json'))['count'])")
[ "$N" -ge "301" ] || { echo "FAIL native count=$N 低于基线 301"; exit 1; }
echo "m84_s3_dns verify done (native=$N)"
exit 0
