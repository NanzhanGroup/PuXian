#!/usr/bin/env bash
# ============================================================
# M59 S2 verify —— floor/ceil/round + log/log10/exp + e 常量
# ------------------------------------------------------------
# 验证点：
#   A. 编译模式 build 出静态 ELF
#   B. math_s2.px 数值断言全过 → "M59-S2 OK" 且退出码 0
#   C. 参数个数错误终止（round() 无参 → exit 1）
#   D. 行数约束 < 500
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m59_math/build
D=examples/m59_math

fail() { echo "M59-S2 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. build（编译模式）=="
$PXC build "$D/math_s2.px" >/dev/null 2>&1 || fail "math_s2.px 编译失败"
[ -x "$B/math_s2" ] || fail "产物 $B/math_s2 不存在"
file "$B/math_s2" | grep -q "x86-64" || fail "产物不是 x86-64"
file "$B/math_s2" | grep -q "statically linked" || fail "产物非静态链接"
ok "编译成功：静态 ELF"

echo "== B. math_s2.px 数值断言 =="
OUT=$("$B/math_s2" 2>&1) || fail "运行退出码非 0"
echo "$OUT" | tail -4
echo "$OUT" | grep -q "M59-S2 OK" || fail "未输出 M59-S2 OK"
echo "$OUT" | grep -q "log(-1) = .*nan" || fail "log(-1) 未透传 NaN"
echo "$OUT" | grep -q "exp(1000) = .*inf" || fail "exp(1000) 未透传 +inf"
ok "数值断言全过（floor/ceil/round/log/log10/exp/e + NaN/inf 透传）"

echo "== C. 参数个数错误终止（编程契约）=="
mkdir -p /tmp/m59_s2_err
cat > /tmp/m59_s2_err/err.px <<'EOF'
var r = round()
print("不应到达这里")
EOF
$PXC build /tmp/m59_s2_err/err.px >/dev/null 2>&1 || fail "err 用例编译失败"
OUT2=$(/tmp/m59_s2_err/build/err 2>&1); RC=$?
[ "$RC" -ne 0 ] || fail "round() 应终止但退出码 0"
echo "$OUT2" | grep -q "round 需要 1 个参数" || fail "stderr 无参数错误提示: $OUT2"
ok "round() 终止且提示正确（退出码 $RC）"

echo "== D. 行数约束 =="
L=$(wc -l < "$D/math_s2.px")
echo "  math_s2.px: $L 行"
[ "$L" -lt 500 ] || fail "math_s2.px 超 500 行"
ok "行数约束满足"

echo "M59-S2 PASS"
