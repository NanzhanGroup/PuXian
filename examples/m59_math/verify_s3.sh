#!/usr/bin/env bash
# ============================================================
# M59 S3 verify —— splitmix64 PRNG：random/random_int/random_seed
# ------------------------------------------------------------
# 验证点：
#   A. 编译模式 build 出静态 ELF
#   B. math_s3.px 断言全过（区间/类型/同 seed 复现/边界）→ "M59-S3 OK" 退出码 0
#   C. 参数错误终止：random_int(0)（n 必须>0）、random("x")（不要参数）→ exit 1
#   D. pi/e %g 可读性（3.14159 / 2.71828）
#   E. 行数约束 < 500
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m59_math/build
D=examples/m59_math

fail() { echo "M59-S3 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. build（编译模式）=="
$PXC build "$D/math_s3.px" >/dev/null 2>&1 || fail "math_s3.px 编译失败"
[ -x "$B/math_s3" ] || fail "产物 $B/math_s3 不存在"
file "$B/math_s3" | grep -q "x86-64" || fail "产物不是 x86-64"
file "$B/math_s3" | grep -q "statically linked" || fail "产物非静态链接"
ok "编译成功：静态 ELF"

echo "== B. math_s3.px 断言 =="
OUT=$("$B/math_s3" 2>&1) || fail "运行退出码非 0"
echo "$OUT" | tail -4
echo "$OUT" | grep -q "M59-S3 OK" || fail "未输出 M59-S3 OK"
ok "断言全过（区间/类型/同 seed 序列复现/random_int 边界）"

echo "== C. 参数错误终止（编程契约）=="
mkdir -p /tmp/m59_s3_err
cat > /tmp/m59_s3_err/err.px <<'EOF'
var r = random_int(0)
print("不应到达这里")
EOF
$PXC build /tmp/m59_s3_err/err.px >/dev/null 2>&1 || fail "err1 用例编译失败"
OUT2=$(/tmp/m59_s3_err/build/err 2>&1); RC=$?
[ "$RC" -ne 0 ] || fail "random_int(0) 应终止但退出码 0"
echo "$OUT2" | grep -q "n 必须 > 0" || fail "random_int(0) stderr 无提示: $OUT2"

cat > /tmp/m59_s3_err/err2.px <<'EOF'
var r = random("x")
print("不应到达这里")
EOF
$PXC build /tmp/m59_s3_err/err2.px >/dev/null 2>&1 || fail "err2 用例编译失败"
OUT3=$(/tmp/m59_s3_err/build/err2 2>&1); RC3=$?
[ "$RC3" -ne 0 ] || fail "random(\"x\") 应终止但退出码 0"
echo "$OUT3" | grep -q "random 不需要参数" || fail "random 带参 stderr 无提示: $OUT3"
ok "random_int(0)/random(\"x\") 均终止且提示正确"

echo "== D. pi/e %g 可读性 =="
echo "$OUT" | grep -q "pi = 3.14159" || fail "pi %g 打印异常: $(echo "$OUT" | grep pi)"
echo "$OUT" | grep -q "e = 2.71828" || fail "e %g 打印异常: $(echo "$OUT" | grep '^e')"
ok "pi≈3.14159 e≈2.71828（%g 6 位可读）"

echo "== E. 行数约束 =="
L=$(wc -l < "$D/math_s3.px")
echo "  math_s3.px: $L 行"
[ "$L" -lt 500 ] || fail "math_s3.px 超 500 行"
ok "行数约束满足"

echo "M59-S3 PASS"
