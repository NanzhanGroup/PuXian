#!/usr/bin/env bash
# ============================================================
# M60 S2 verify —— 设备组 tty_config / fd_wait
# ------------------------------------------------------------
# 验证点：
#   A. build 出静态 ELF（tty_config/fd_wait 已注册）
#   B. dev_s2.px 行为断言全过 → "M60-S2 OK"
#      （tty_config raw/波特率 / raw 无行缓冲 / fd_wait 就绪/超时/HUP/单int与list输入）
#   C. 参数错误 → 终止：tty_config 无效波特率、fd_wait 负 timeout、tty_config 缺参
#   D. 行数约束 < 500
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根
PXC=./tools/pxc
B=examples/m60_dev/build
D=examples/m60_dev

fail() { echo "M60-S2 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. build（编译模式）=="
$PXC build "$D/dev_s2.px" >/dev/null 2>&1 || fail "dev_s2.px 编译失败"
[ -x "$B/dev_s2" ] || fail "产物 $B/dev_s2 不存在"
ARCH=$(file "$B/dev_s2")
echo "$ARCH"
echo "$ARCH" | grep -q "x86-64" || fail "产物不是 x86-64"
echo "$ARCH" | grep -q "statically linked" || fail "产物非静态链接"
ok "编译成功：静态 ELF"

echo "== B. dev_s2.px 行为断言 =="
OUT=$("$B/dev_s2" 2>&1) || fail "运行退出码非 0"
echo "$OUT" | grep -E "^A:|^B:|^C:|^D:|^E:|^F:|^G:"
echo "$OUT" | grep -q "M60-S2 OK" || fail "未输出 M60-S2 OK"
ok "行为断言全过（tty_config raw/波特率 + fd_wait 就绪/超时/HUP/双形态）"

echo "== C. 参数错误终止（编程契约）=="
mkdir -p /tmp/m60_s2_err
cat > /tmp/m60_s2_err/err1.px <<'EOF'
var fd = open("/dev/ptmx", "rw")
var r = tty_config(fd, 12345, true)
print("不应到达这里")
EOF
cat > /tmp/m60_s2_err/err2.px <<'EOF'
var r = fd_wait(1, -1)
print("不应到达这里")
EOF
cat > /tmp/m60_s2_err/err3.px <<'EOF'
var fd = open("/dev/ptmx", "rw")
var r = tty_config(fd, 9600)
print("不应到达这里")
EOF
for c in err1 err2 err3; do
    $PXC build "/tmp/m60_s2_err/$c.px" >/dev/null 2>&1 || fail "$c 编译失败"
    OUT2=$(/tmp/m60_s2_err/build/$c 2>&1); RC=$?
    [ "$RC" -ne 0 ] || fail "$c 应终止但退出码 0"
    echo "  $c 退出码 $RC"
done
O1=$(/tmp/m60_s2_err/build/err1 2>&1); echo "$O1" | grep -q "不支持的波特率" || fail "err1 无波特率提示: $O1"
O2=$(/tmp/m60_s2_err/build/err2 2>&1); echo "$O2" | grep -q "timeout_ms 需要 >= 0" || fail "err2 无 timeout 提示: $O2"
O3=$(/tmp/m60_s2_err/build/err3 2>&1); echo "$O3" | grep -q "tty_config 需要 (fd, baud, raw)" || fail "err3 无缺参提示: $O3"
ok "参数错误终止且提示正确"

echo "== D. 行数约束 =="
L=$(wc -l < "$D/dev_s2.px")
echo "  dev_s2.px: $L 行"
[ "$L" -lt 500 ] || fail "dev_s2.px 超 500 行"
ok "行数约束满足"

echo "M60-S2 PASS"
