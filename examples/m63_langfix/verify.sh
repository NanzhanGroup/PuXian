#!/usr/bin/env bash
# ============================================================
# M63 语言面欠账 L8–L11 一键验证
#   L9  float→str 最短 roundtrip 全精度（m63_fp）
#   L10 编译期浮点字面量全精度（m63_prec）
#   L8  pxi 网络补白名单（m63_net 双模式对拍 + m63_net_err pxi 单测）
#   L11 pxc --version
# 依赖：本地 mock HTTP server 已起在 :18080（/tmp/m63_mock.py）
# 用法：./verify.sh
# ============================================================
set -u
cd "$(dirname "$0")"
ROOT="$(cd ../.. && pwd)"
PXC="$ROOT/tools/pxc"
PXI="$ROOT/bootstrap/pxi"
BPXC="$ROOT/bootstrap/pxc"
MOCK="http://127.0.0.1:18080/api"
fail=0

chk() {  # chk <条件> <描述>
    if [ "$1" = "0" ]; then echo "  ✅ $2"; else echo "  ❌ $2"; fail=1; fi
}

echo "══════════ M63 L8-L11 验证 ══════════"

echo "── L11: pxc/pxi --version"
v=$("$BPXC" --version 2>&1); chk $? "bootstrap/pxc --version exit 0"
echo "$v" | grep -q "pxc 0.1.0" && vok=0 || vok=1; chk $vok "bootstrap/pxc --version 文本 (pxc 0.1.0 ...)"
v=$("$PXI" --version 2>&1); chk $? "pxi --version exit 0"

echo "── L9: float roundtrip 全精度（双模式）"
"$PXI" m63_fp.px > /tmp/m63_fp.i 2>&1; iok=$?
"$PXC" build --no-quic m63_fp.px >/dev/null 2>&1 && ./build/m63_fp > /tmp/m63_fp.c 2>&1; cok=$?
chk $iok "pxi m63_fp 全 PASS"
chk $cok "编译 m63_fp 全 PASS"
grep -q "0.30000000000000004" /tmp/m63_fp.i && gok=0 || gok=1; chk $gok "pxi 0.1+0.2 → 0.30000000000000004（非 0.3）"
diff -q /tmp/m63_fp.i /tmp/m63_fp.c >/dev/null 2>&1 && dok=0 || dok=1; chk $dok "m63_fp stdout 双模式逐字节一致"

echo "── L10: 编译期字面量全精度（双模式）"
"$PXI" m63_prec.px > /tmp/m63_prec.i 2>&1; iok=$?
"$PXC" build --no-quic m63_prec.px >/dev/null 2>&1 && ./build/m63_prec > /tmp/m63_prec.out 2>&1; cok=$?
chk $iok "pxi m63_prec 全 PASS"
chk $cok "编译 m63_prec 全 PASS"
# 检查编译中间 C 产物（pxc 生成的 build/m63_prec.c）字面量已全精度
grep -q "3.141592653589793" build/m63_prec.c && gok=0 || gok=1; chk $gok "编译产物字面量全精度（非 3.14159）"
diff -q /tmp/m63_prec.i /tmp/m63_prec.out >/dev/null 2>&1 && dok=0 || dok=1; chk $dok "m63_prec stdout 双模式逐字节一致"

echo "── L8: pxi 网络白名单（mock :18080）"
"$PXI" m63_net.px > /tmp/m63_net.i 2>&1; iok=$?
"$PXC" build --no-quic m63_net.px >/dev/null 2>&1 && ./build/m63_net > /tmp/m63_net.c 2>&1; cok=$?
chk $iok "pxi m63_net 全 PASS（http_post/http_request 真请求 + 失败 Err 透传）"
chk $cok "编译 m63_net 全 PASS"
diff -q /tmp/m63_net.i /tmp/m63_net.c >/dev/null 2>&1 && dok=0 || dok=1; chk $dok "m63_net stdout 双模式逐字节一致"
"$PXI" m63_net_err.px > /tmp/m63_net_err.i 2>&1; eok=$?
chk $eok "pxi m63_net_err（网络失败 Err 值单测）"
grep -c "^PASS" /tmp/m63_net_err.i | grep -q "^4$" && gok=0 || gok=1; chk $gok "m63_net_err 4 断言全过"
# 参数个数错误 → 解释器传播错误 exit 1 + 函数名消息（同编译模式 px_error 报错退出）
echo 's3_put("ep", "b")' > /tmp/m63_arerr.px
"$PXI" /tmp/m63_arerr.px > /tmp/m63_arerr.out 2>&1; aok=$?
[ "$aok" != "0" ] && grep -q "s3_put 需要" /tmp/m63_arerr.out && gok=0 || gok=1
chk $gok "s3_put 参数错误 → 报错退出 + 消息（解释器不杀进程）"

echo "════════════════════════════════════"
[ "$fail" = "0" ] && echo "M63 verify ALL OK" || { echo "M63 verify FAILED"; exit 1; }
