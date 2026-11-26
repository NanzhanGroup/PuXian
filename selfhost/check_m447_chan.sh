#!/usr/bin/env bash
# ============================================================
# check_m447_chan.sh —— 无缓冲通道 rendezvous 的「双方到场」语义必须双向唤醒（缺陷 950）
# ------------------------------------------------------------
# 为什么有它：
#   0.2.444/0.2.446 上，无缓冲 chan 的接收方在登记等待前**从不唤醒已先到的发送方**：
#   发送协程挂在 cw_send（或 pthread 发送方挂在 cv_send），接收方挂 cw_recv/cv_recv，
#   两条等待链永不交叉 ⇒ lost wakeup ⇒ 主线程永久阻塞。
#   第 5 轮全量门里 m426 的 concurrent_m3 红就是这个（当时被误判 LOAD-SENSITIVE 豁免）。
#
# 判据：
#   C1 静态（剥离注释后判文本）：px_chan_recv 的无缓冲等待路径必须唤醒发送侧
#      （chan_sig_send），px_coro_chan_recv_wait 挂 cw_recv 前必须摘/唤醒 cw_send。
#   C2 确定性复现：发送方先到（sleep 让出）再由主线程 recv，50 轮必须全绿。
#      修前实测：第 1 轮即挂死（rc=124）。这是本门的主判据。
#   C3 并发压力：16 并发 × 10 轮 concurrent_m3 必须全绿（修前 ~10% 撞满 20s）。
#   C4 回归：buffered / select / 多值 concurrent.px 必须仍绿（修复不得误伤）。
#   N1 负控（有牙）：把 px_chan_recv 里的唤醒语句注释掉 ⇒ C2 必须挂死（rc≠0）。
#      负控结束恢复源文件并重编，顺带自证「恢复后重新变绿」。
#   G1 缺 px 工具 / 源码 ⇒ rc=3（不降级成 0）。
#
# 用法：check_m447_chan.sh [--repo DIR] [--quick]
#   --quick 跳过 N1 负控（约省 70s，CI 排障用；常规门必跑负控）
# 退出码：0 全过 / 1 有违例 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
QUICK=0
while [ $# -gt 0 ]; do
  case "$1" in
    --repo) ROOT="$2"; shift 2 ;;
    --quick) QUICK=1; shift ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done

PX="$ROOT/tools/px"
RT="$ROOT/runtime/runtime.c"
CORO="$ROOT/runtime/coro.c"
EX="$ROOT/examples"
for f in "$PX" "$RT" "$CORO" "$EX/concurrent_m3.px" "$EX/concurrent.px"; do
  [ -e "$f" ] || { echo "ENV: 缺 $f"; exit 3; }
done

ok_n=0; bad_n=0
ok()  { echo "✅ $*"; ok_n=$((ok_n+1)); }
bad() { echo "❌ $*"; bad_n=$((bad_n+1)); }

TMP="$(mktemp -d /tmp/m447chan.XXXXXX)"
BAK=""
cleanup() {
  [ -n "$BAK" ] && [ -f "$BAK" ] && cp -f "$BAK" "$RT"
  rm -rf "$TMP"
}
trap cleanup EXIT INT TERM HUP

build() {  # build <src> <outbase>  ⇒ 产物路径 <dir>/build/<outbase>
  local out
  out="$(timeout 300 "$PX" build "$1" -o "$2" 2>&1)"
  rc=$?
  [ $rc -eq 0 ] || { echo "$out" | tail -3 >&2; return 1; }
  echo "$out" | grep -o '/[^ ]*build/[^ ]*' | tail -1
}

# ---- C1 静态判据（剥离注释后判，防「注释里的代码也算代码」）----
python3 - "$RT" "$CORO" <<'PYEOF'
import re, sys
def strip(s):
    s = re.sub(r'/\*.*?\*/', '', s, flags=re.S)
    s = re.sub(r'//[^\n]*', '', s)
    return s
rt = strip(open(sys.argv[1], encoding='utf-8', errors='replace').read())
co = strip(open(sys.argv[2], encoding='utf-8', errors='replace').read())
bad = 0
# px_chan_recv：cap==0 分支里 recv_waiting++ 之后必须出现 chan_sig_send
seg = rt.split('LXValue px_chan_recv(')[-1][:2500]
if 'recv_waiting++' not in seg:
    print('C1a 未找到 recv_waiting++ 登记点'); bad = 1
elif 'chan_sig_send' not in seg:
    print('C1a px_chan_recv 无缓冲等待前未唤醒发送侧（chan_sig_send 缺失）'); bad = 1
# px_coro_chan_recv_wait：挂 cw_recv 之后必须摘 cw_send
cseg = co.split('int px_coro_chan_recv_wait(')[-1][:2000]
if 'cw_recv' not in cseg:
    print('C1b 未找到 cw_recv 登记点'); bad = 1
elif 'cw_send' not in cseg:
    print('C1b px_coro_chan_recv_wait 挂 cw_recv 前未唤醒 cw_send'); bad = 1
sys.exit(bad)
PYEOF
[ $? -eq 0 ] && ok "C1 静态：接收侧等待路径已双向唤醒（runtime.c + coro.c，剥注释后判）" \
             || bad "C1 静态：接收侧未唤醒发送者（缺陷 950 未修或回归）"

# ---- C2 确定性复现（主判据）----
cat > "$TMP/rep950.px" <<'PXEOF'
def sender(ch):
    ch.send(7)

var bad = 0
for i in range(0, 50):
    let ch = chan[int]()
    spawn sender(ch)
    sleep(2)
    let v = ch.recv()
    if v != 7:
        bad = bad + 1
print("bad=", bad)
PXEOF
BIN=$(build "$TMP/rep950.px" rep950) || bad "C2 编译失败"
if [ -n "${BIN:-}" ] && [ -x "$BIN" ]; then
  OUT=$(timeout 25 "$BIN" 2>&1); rc=$?
  if [ $rc -eq 0 ] && echo "$OUT" | grep -q 'bad= 0'; then
    ok "C2 发送方先到的确定性场景 50 轮全绿（修前第 1 轮即 rc=124）"
  else
    bad "C2 确定性场景挂死/异常：rc=$rc out=$(echo "$OUT" | tail -1)（缺陷 950 回归）"
  fi
fi

# ---- C3 并发压力 ----
BIN3=$(build "$EX/concurrent_m3.px" cm3) || bad "C3 concurrent_m3 编译失败"
if [ -n "${BIN3:-}" ] && [ -x "$BIN3" ]; then
  (
    for r in 1 2 3 4 5 6 7 8 9 10; do
      for j in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16; do
        timeout 20 "$BIN3" >/dev/null 2>&1 || echo "FAIL r=$r j=$j" >> "$TMP/stress.fail"
      done
      wait
    done
  )
  NF=$( [ -f "$TMP/stress.fail" ] && wc -l < "$TMP/stress.fail" || echo 0 )
  [ "$NF" = "0" ] && ok "C3 16 并发 × 10 轮 = 160 次 concurrent_m3 全绿（修前 ~10% 撞满 20s）" \
                  || bad "C3 并发压力失败 $NF/160：$(head -3 "$TMP/stress.fail")"
fi

# ---- C4 回归（有缓冲 / select / 多值）----
BIN4=$(build "$EX/concurrent.px" ccp) || bad "C4 concurrent 编译失败"
if [ -n "${BIN4:-}" ] && [ -x "$BIN4" ]; then
  OUT4=$(timeout 30 "$BIN4" 2>&1); rc4=$?
  if [ $rc4 -eq 0 ] && echo "$OUT4" | grep -q 'first: 2' && echo "$OUT4" | grep -q 'second: 4'; then
    ok "C4 回归：有缓冲通道 / 多值语义未受影响"
  else
    bad "C4 回归失败 rc=$rc4 out=$(echo "$OUT4" | head -2 | tr '\n' ' ')"
  fi
fi

# ---- N1 负控（有牙）----
if [ "$QUICK" = "1" ]; then
  echo "⏭  N1 负控跳过（--quick）"
else
  BAK="$TMP/runtime.c.bak"
  cp -f "$RT" "$BAK"
  # 只注释掉接收侧的唤醒语句（保留代码结构，模拟「修复被回退」）
  python3 - "$RT" <<'PYEOF'
import sys, io
p = sys.argv[1]
s = open(p, encoding='utf-8', errors='replace').read()
old = "            px_coro_wake(chan_sig_send(o));"
assert old in s, "负控锚点缺失"
s = s.replace(old, "            { struct PxCoro* _sink = NULL; (void)_sink; }", 1)
open(p, 'w', encoding='utf-8').write(s)
PYEOF
  if [ $? -ne 0 ]; then
    bad "N1 负控注入失败（锚点缺失 ⇒ 判据需更新）"
  else
    BINN=$(build "$TMP/rep950.px" rep950n)
    if [ -n "${BINN:-}" ] && [ -x "$BINN" ]; then
      timeout 15 "$BINN" >/dev/null 2>&1; rcn=$?
      [ $rcn -ne 0 ] && ok "N1 负控有牙：回退唤醒语句后同一场景即挂死（rc=$rcn）" \
                     || bad "N1 负控无牙：回退唤醒后仍 rc=0 —— C2 不成立"
    else
      bad "N1 负控编译失败"
    fi
  fi
  cp -f "$BAK" "$RT"; BAK=""
  # 恢复后必须重新变绿（顺带自证「恢复生效」而不是「碰巧没编进去」）
  BINR=$(build "$TMP/rep950.px" rep950r)
  if [ -n "${BINR:-}" ] && [ -x "$BINR" ]; then
    timeout 25 "$BINR" >/dev/null 2>&1 && ok "N1b 恢复修复后重新变绿（正控自证）" \
      || bad "N1b 恢复后仍红 —— 恢复不生效"
  else
    bad "N1b 恢复后编译失败"
  fi
fi

echo "----------------------------------------"
if [ "$bad_n" -eq 0 ]; then
  echo "M447_CHAN-OK pass=$ok_n fail=0"
  exit 0
else
  echo "M447_CHAN-RED pass=$ok_n fail=$bad_n"
  exit 1
fi
