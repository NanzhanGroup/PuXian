#!/usr/bin/env bash
# ============================================================
# M61 S3 verify —— raw 终端可玩贪吃蛇（B 线输入）
# ------------------------------------------------------------
# 验证点：
#   A. pxc build m61_snake.px
#   B. SNAKE_AUTO=1 无头自动剧本：EAT（吃食增长）/ SELF（撞身判定）/
#      WALL（撞墙判定）三断言 + 初始帧渲染（含 O/#/@）
#   C. PTY 交互 smoke（python3 pty 真内核）：子进程 stdin 挂 PTY slave，
#      tty_config raw + fd_wait 读键；喂 'q' → 进程退出且 stdout 含 "QUIT score"
#      （证明 tty_config/fd_wait/read 应用层链路通 = M60 设备组 dogfood）
#   D. 行数约束 < 500
# ============================================================
set -u
cd "$(dirname "$0")/../.."
PXC=./tools/pxc
D=examples/m61_gfx
B=$D/build
fail() { echo "M61-S3 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. 编译 =="
$PXC build "$D/m61_snake.px" >/dev/null 2>&1 || fail "m61_snake.px 编译失败"
ok "m61_snake.px 编译"

echo "== B. 无头自动剧本 =="
OUT=$(SNAKE_AUTO=1 "$B/m61_snake" 2>&1) || fail "auto 模式退出码非 0"
echo "$OUT" | grep -q "AUTO EAT OK len 4 score 1" || fail "EAT 断言缺失: $(echo "$OUT" | grep AUTO)"
echo "$OUT" | grep -q "AUTO SELF OK" || fail "SELF 断言缺失"
echo "$OUT" | grep -q "AUTO WALL OK died at x=20" || fail "WALL 断言缺失"
echo "$OUT" | grep -q "@" || fail "帧缺食物 @"
echo "$OUT" | grep -q "O" || fail "帧缺蛇头 O"
echo "$OUT" | grep -q "#" || fail "帧缺蛇身 #"
echo "$OUT" | grep -q '^+--------------------+' || fail "帧缺边框"
ok "EAT/SELF/WALL 三断言 + 帧渲染全过"
echo "$OUT" | grep -E "AUTO (EAT|SELF|WALL)" 

echo "== C. PTY 交互 smoke（真终端 raw 读键链路）=="
if command -v python3 >/dev/null 2>&1; then
    python3 - "$B/m61_snake" <<'PY'
import os, pty, subprocess, time, sys
bin_ = sys.argv[1]
m, s = pty.openpty()
p = subprocess.Popen([bin_], stdin=s, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                     env={**os.environ, "SNAKE_AUTO": "0"})
os.close(s)
time.sleep(0.4)          # 等蛇完成 tty_config raw + 首帧
os.write(m, b"q")        # 喂退出键
try:
    out, err = p.communicate(timeout=5)
except subprocess.TimeoutExpired:
    p.kill()
    print("✗ 蛇未响应 q 退出（超时）")
    sys.exit(1)
os.close(m)
ok_quit = b"QUIT score" in out
print("  退出输出尾:", (out.decode(errors="replace").strip().splitlines() or [""])[-1][:60])
if not ok_quit:
    print("✗ stdout 无 QUIT score：raw 读键链路未通")
    print("stderr:", err.decode(errors="replace")[:300])
    sys.exit(1)
print("  ✅ PTY 喂 q → QUIT：tty_config/fd_wait/read 应用层链路通")
PY
    if [ $? -ne 0 ]; then
        fail "PTY 交互 smoke 失败"
    fi
else
    echo "  ⚠️ 无 python3，跳过 PTY smoke"
fi

echo "== D. 行数约束 =="
LC=$(wc -l < "$D/m61_snake.px")
[ "$LC" -lt 500 ] || fail "m61_snake.px 超 500 行: $LC"
ok "m61_snake.px $LC 行 < 500"

echo "M61-S3 verify PASS"
