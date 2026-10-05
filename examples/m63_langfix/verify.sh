#!/usr/bin/env bash
# ============================================================
# M63 语言面欠账 L8–L11 一键验证
#   L9  float→str 最短 roundtrip 全精度（m63_fp）
#   L10 编译期浮点字面量全精度（m63_prec）
#   L8  pxi 网络补白名单（m63_net 双模式对拍 + m63_net_err pxi 单测）
#   L11 pxc --version
# mock：**随门自带**（m63_mock.px · 普贤写 · M249 起）—— 监听 127.0.0.1:18080，EXIT 收尾。
#   此前依赖 /tmp/m63_mock.py（易失的外部脚本），而本门长期不在任何运行器里
#   ⇒ 该文件被清理后无人发现，门实际早已跑不起来（M249 缺陷 428）。
# 用法：./verify.sh
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
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

# ---- 自带 mock（M249 · 缺陷 428）：起在 127.0.0.1:18080，EXIT 收尾 ----
MOCKLOG=/tmp/m63_mock.log
rm -f "$MOCKLOG"
"$PXC" build --no-quic m63_mock.px >/dev/null 2>&1 || { echo "❌ m63_mock.px 编译失败"; exit 1; }
# ⚠ 必须 exec：否则 $! 是子 shell，服务端活着占端口，下一轮起不来（M201 教训）
( exec ./build/m63_mock > "$MOCKLOG" 2>&1 ) &
MOCK_PID=$!
trap 'kill "$MOCK_PID" 2>/dev/null; wait "$MOCK_PID" 2>/dev/null' EXIT
for _i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50; do
    grep -q 'm63mock: listening' "$MOCKLOG" 2>/dev/null && break
    sleep 0.2
done
if ! grep -q 'm63mock: listening' "$MOCKLOG" 2>/dev/null; then
    echo "❌ mock 未能监听 18080 —— 日志："; cat "$MOCKLOG"; exit 1
fi
echo "  ✅ mock 已监听 127.0.0.1:18080（随门自带）"

echo "══════════ M63 L8-L11 验证 ══════════"

echo "── L11: pxc/pxi --version"
v=$("$BPXC" --version 2>&1); chk $? "bootstrap/pxc --version exit 0"
echo "$v" | grep -qE "pxc [0-9]+\.[0-9]+\.[0-9]+" && vok=0 || vok=1; chk $vok "bootstrap/pxc --version 文本 (pxc <semver> ...)"
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
