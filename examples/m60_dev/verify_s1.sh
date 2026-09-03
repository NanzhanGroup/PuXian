#!/usr/bin/env bash
# ============================================================
# M60 S1 verify —— us 级时钟 + fd 控制（sleep_us/now_us/fcntl）
# ------------------------------------------------------------
# 验证点：
#   A. build 出静态 ELF（sleep_us/now_us/fcntl 已注册）
#   B. dev_s1.px 行为断言全过 → 打印 "M60-S1 OK" 且退出码 0
#      （sleep_us 计时 / now_us 单调 / fcntl 设 O_NONBLOCK / 非阻塞读空 PTY → EAGAIN）
#   C. 参数/类型错误 → 终止（编程契约）：sleep_us() 无参、fcntl(fd) 缺 cmd、
#      sleep_us("x") 类型错 → 各自退出码非 0 且 stderr 有提示
#   D. 行数约束 < 500（工程惯例）
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m60_dev/build
D=examples/m60_dev

fail() { echo "M60-S1 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. build（编译模式）=="
$PXC build "$D/dev_s1.px" >/dev/null 2>&1 || fail "dev_s1.px 编译失败"
[ -x "$B/dev_s1" ] || fail "产物 $B/dev_s1 不存在"
ARCH=$(file "$B/dev_s1")
echo "$ARCH"
echo "$ARCH" | grep -q "x86-64" || fail "产物不是 x86-64"
echo "$ARCH" | grep -q "statically linked" || fail "产物非静态链接"
ok "编译成功：静态 ELF"

echo "== B. dev_s1.px 行为断言 =="
OUT=$("$B/dev_s1" 2>&1) || fail "运行退出码非 0"
echo "$OUT" | grep -E "^A:|^B:|^C/D:|^E:|^F:"
echo "$OUT" | grep -q "M60-S1 OK" || fail "未输出 M60-S1 OK"
ok "行为断言全过（sleep_us/now_us/fcntl 三态）"

echo "== C. 参数/类型错误终止（编程契约）=="
mkdir -p /tmp/m60_s1_err
cat > /tmp/m60_s1_err/err1.px <<'EOF'
sleep_us()
print("不应到达这里")
EOF
cat > /tmp/m60_s1_err/err2.px <<'EOF'
var fd = open("/dev/ptmx", "rw")
var r = fcntl(fd)
print("不应到达这里")
EOF
cat > /tmp/m60_s1_err/err3.px <<'EOF'
var r = sleep_us("x")
print("不应到达这里")
EOF
for c in err1 err2 err3; do
    $PXC build "/tmp/m60_s1_err/$c.px" >/dev/null 2>&1 || fail "$c 编译失败"
    OUT2=$(/tmp/m60_s1_err/build/$c 2>&1); RC=$?
    [ "$RC" -ne 0 ] || fail "$c 应终止但退出码 0"
    echo "  $c 退出码 $RC"
done
# 提示语各自核对
O1=$(/tmp/m60_s1_err/build/err1 2>&1); echo "$O1" | grep -q "sleep_us 需要" || fail "err1 无 sleep_us 提示: $O1"
O2=$(/tmp/m60_s1_err/build/err2 2>&1); echo "$O2" | grep -q "fcntl 需要" || fail "err2 无 fcntl 提示: $O2"
O3=$(/tmp/m60_s1_err/build/err3 2>&1); echo "$O3" | grep -q "期望整数" || fail "err3 无类型提示: $O3"
ok "参数/类型错误终止且提示正确"

echo "== D. 行数约束 =="
L=$(wc -l < "$D/dev_s1.px")
echo "  dev_s1.px: $L 行"
[ "$L" -lt 500 ] || fail "dev_s1.px 超 500 行"
ok "行数约束满足"

echo "M60-S1 PASS"
