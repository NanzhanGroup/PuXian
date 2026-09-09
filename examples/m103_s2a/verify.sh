#!/bin/bash
# M103-S2a (Issue 29 GAP-DNS-TXT-1) 专项验证：dns_txt 手写 DNS UDP TXT 查询
#   px dns_txt 断言（公开域 SPF / 空域空 list / 非法入参 Err）+ 与 Go net.LookupTXT
#   对拍集合相等 + dns_lookup A/AAAA 零回归
# 依赖：go、pxc 工具链；断网环境 [4/4] 自动 skip
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
GOBIN=./build/go_txt
mkdir -p build artifacts

echo "== [1/4] Go 工具编译 =="
go build -o "$GOBIN" go_txt.go || { echo "FAIL go build"; exit 1; }

echo "== [2/4] px build =="
$PX build --no-quic dns_txt_test.px >/tmp/m103s2a_build.log 2>&1 || { echo "FAIL build"; tail -20 /tmp/m103s2a_build.log; exit 1; }

echo "== [3/4] px 运行断言 + dump =="
OUT=$(./build/dns_txt_test 2>&1)
echo "$OUT"
echo "$OUT" | grep -q 'DNS_TXT_TEST:.*0F' || { echo "FAIL px 断言未全过"; exit 1; }
echo "$OUT" | sed -n '/----TXT-DUMP----/,$p' | tail -n +2 | grep -v '^DNS_TXT_TEST' > artifacts/px.txt

echo "== [4/4] Go net.LookupTXT 对拍（集合相等）=="
"$GOBIN" dump qq.com > artifacts/go.txt 2>/dev/null || { echo "skip: 网络不可达（dns 对拍跳过，px 断言已过）"; exit 0; }
RV=$("$GOBIN" cmp artifacts/go.txt artifacts/px.txt)
echo "$RV"
[ "$RV" = "CMP-EQUAL" ] || { echo "FAIL Go 对拍"; exit 1; }
echo "m103_s2a_dns_txt verify done"
exit 0
