#!/usr/bin/env bash
# ============================================================
# M59 S1 verify —— 三角族 sin/cos/tan/atan2 + pi 常量
# ------------------------------------------------------------
# 验证点：
#   A. 编译模式 build 出静态 ELF（sin/cos/tan/atan2/pi 已注册）
#   B. math_s1.px 数值断言全过 → 打印 "M59-S1 OK" 且退出码 0
#   C. 类型错误 → 终止（编程契约）：sin("x") 退出码 1 且 stderr 有 "参数必须是数字"
#   D. 行数约束 < 500（工程惯例）
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m59_math/build
D=examples/m59_math

fail() { echo "M59-S1 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. build（编译模式）=="
$PXC build "$D/math_s1.px" >/dev/null 2>&1 || fail "math_s1.px 编译失败"
[ -x "$B/math_s1" ] || fail "产物 $B/math_s1 不存在"
ARCH=$(file "$B/math_s1")
echo "$ARCH"
echo "$ARCH" | grep -q "x86-64" || fail "产物不是 x86-64"
echo "$ARCH" | grep -q "statically linked" || fail "产物非静态链接"
ok "编译成功：静态 ELF"

echo "== B. math_s1.px 数值断言 =="
OUT=$("$B/math_s1" 2>&1) || fail "运行退出码非 0"
echo "$OUT" | tail -3
echo "$OUT" | grep -q "M59-S1 OK" || fail "未输出 M59-S1 OK"
ok "数值断言全过（sin/cos/tan/atan2/pi）"

echo "== C. 类型错误终止（编程契约）=="
mkdir -p /tmp/m59_s1_err
cat > /tmp/m59_s1_err/err.px <<'EOF'
var r = sin("x")
print("不应到达这里")
EOF
$PXC build /tmp/m59_s1_err/err.px >/dev/null 2>&1 || fail "err 用例编译失败"
OUT2=$(/tmp/m59_s1_err/build/err 2>&1); RC=$?
[ "$RC" -ne 0 ] || fail "sin(\"x\") 应终止但退出码 0"
echo "$OUT2" | grep -q "参数必须是数字" || fail "stderr 无类型错误提示: $OUT2"
ok "sin(\"x\") 终止且提示正确（退出码 $RC）"

echo "== D. 行数约束 =="
L=$(wc -l < "$D/math_s1.px")
echo "  math_s1.px: $L 行"
[ "$L" -lt 500 ] || fail "math_s1.px 超 500 行"
ok "行数约束满足"

echo "M59-S1 PASS"
