#!/usr/bin/env bash
# ============================================================
# M126（qg-issue 83）门：**信号处理器**隔离点 + 纳入并发 GC
# ------------------------------------------------------------
# 病灶（原实现，`sig_dispatch_thread`）：
#   ⓐ 直接 `px_call(handler)`、**不装隔离点** ⇒ 处理器内任何运行时错误 / 分配失败
#      走「无隔离点 → 保留致命语义」= `exit(1)` ⇒ **一个信号带走整个进程**。
#      生产实证：mahesvara 的 SIGHUP reload 处理器（main.px:318）一错，8 站点同进程一起死
#      （journal 00:40:53 / 00:42:24 / 00:42:43 / 00:43:35 / 00:43:46 三分钟内 5 次重启）。
#   ⓑ 该线程**从未注册进 g_threads** ⇒ 并发 GC 既不暂停它也不标它的 vm_state
#      ⇒ 处理器持有的普贤对象被回收 = UAF（实测崩点 #0 px_add / #1 vm_run_loop /
#      #2 px_vm_entry / #3 sig_dispatch_thread）。
# 修法：处理器执行窗口内 `px_gc_thread_enter/leave` + `setjmp/longjmp` 隔离点（kind=2）。
# 判据：
#   A 正控：信号确实被投递、处理器真跑了（否则后面全是假绿）；
#   B 处理器内运行时错误 → 进程存活 + /health 200 + 只终止本次调用 + 退出码不变；
#   C 处理器内真·大分配（read_file 100 GB 稀疏文件）→ 同上，且文案带字节数/errno；
#   D 处理器内注入分配失败（PX_ALLOC_FAIL_MIN）→ 同上；
#   E 处理器内 5000 次分配并持有 + 后台风暴 + PX_GC_THRESHOLD=100 → 无 SIGSEGV（ⓑ）；
#   F 负控：PX_SPAWN_ISOLATE=0 逃生舱 → **必须仍 exit 非 0**（证明不是"永远吞掉"）；
#   G 多信号连发：计数递增、进程持续存活；
#   H 自然退出（无信号）：退出码 0 且无隔离痕迹。
# 注 1：必须用**仓库自带** tools/px（PXC_HOME=仓库根 ⇒ 用仓库 runtime/）；
#       /usr/bin/px 是安装版，其 runtime/ 与仓库不同步 ⇒ 会测出旧行为。
# 注 2：D 的注入阈值 = `8388610`（= 载荷 8388609 + 1）。**这不是随手取的**：实测扫过
#       5 个阈值 —— 6 MB / 8 MB 会连带打掉运行时**内部 8 MB 分配**（协程栈）⇒ 协程死亡后
#       进程**假死**（/health 不通、也不退出）；8388610 精确踩在处理器载荷之上、内部 8 MB
#       分配之下 ⇒ 只打处理器。见 README「阈值标定」。
# 用法：bash examples/m126_signal_isolate/verify.sh
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m126_signal_isolate
PX=${PX:-$ROOT/tools/px}
BIN=$ROOT/$DIR/build/sig_isolate
PORT=18098
PAYLOAD=/tmp/m126_payload.bin            # 8388609 字节（8 MB + 1）
HUGE=/tmp/m126_huge.bin                  # 100 GB 稀疏
THRESH=8388610                           # 见注 2
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

probe_health() {   # → "HTTP/1.1 200 OK" 或空（不用 curl：避免管道依赖）
  ( exec 3<>/dev/tcp/127.0.0.1/$PORT 2>/dev/null &&
    printf 'GET /health HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n' >&3 &&
    timeout 3 head -1 <&3 ) 2>/dev/null | tr -d '\r'
}

# run_case <tag> <fx> <hold_ms> <n_signals> [env...]
# 导出：RC 最终退出码 / LOG 日志 / HEALTH 探测时的 /health 首行 / HUNG 1=看门狗超时
run_case() {
  local tag="$1" fx="$2" hold="$3" nsig="$4"; shift 4
  local log="/tmp/m126_${tag}.log"
  LOG="$log"; HEALTH=""; HUNG=0; RC=99
  pkill -f "$BIN" 2>/dev/null; sleep 0.3
  : > "$log"
  if [ $# -gt 0 ]; then env FX="$fx" HOLD_MS="$hold" "$@" "$BIN" >> "$log" 2>&1 &
  else                 env FX="$fx" HOLD_MS="$hold"     "$BIN" >> "$log" 2>&1 &
  fi
  local pid=$! i
  for i in $(seq 1 60); do grep -q "M126 READY" "$log" && break; sleep 0.25; done
  if ! grep -q "M126 READY" "$log"; then bad "$tag 启动失败（无 READY）"; RC=99; return; fi
  for i in $(seq 1 "$nsig"); do kill -USR1 "$pid" 2>/dev/null; sleep 0.6; done
  sleep 1.0
  # ★ 关键：**进程还活着时**探测（原来的错误是在 wait 之后探测 ⇒ 永远非 200）
  HEALTH=$(probe_health)
  # 看门狗：HOLD_MS 到点后应自然退出；超时 = 假死（比 exit(1) 更糟的那种）
  for i in $(seq 1 48); do kill -0 "$pid" 2>/dev/null || break; sleep 0.25; done
  if kill -0 "$pid" 2>/dev/null; then HUNG=1; kill -9 "$pid" 2>/dev/null; fi
  wait "$pid" 2>/dev/null; RC=$?
}

echo "== M126 验证（信号处理器隔离：一个信号不再带走整个进程）=="

echo "[0/4] 准备载荷"
head -c 8388609 /dev/zero | tr '\0' 'A' > "$PAYLOAD"
truncate -s 100G "$HUGE"
echo "     payload=$(stat -c%s "$PAYLOAD") 字节  huge=$(stat -c%s "$HUGE") 字节（100 GB 稀疏）"

echo "[1/4] 编译（仓库 tools/px ⇒ 仓库 runtime/）"
if "$PX" build "$DIR/sig_isolate.px" > /tmp/m126_build.log 2>&1 && [ -x "$BIN" ]; then
  ok "编译成功（$(stat -c%s "$BIN") 字节）"
else
  bad "编译失败"; tail -20 /tmp/m126_build.log | sed 's/^/     /'; exit 1
fi
if grep -qa "信号处理器运行时错误已隔离" "$BIN"; then   # grep -a：strings 默认只出 7-bit ASCII
  ok "二进制含 M126 新文案（确认用的是仓库 patched runtime）"
else
  bad "二进制不含 M126 新文案 —— runtime 未重编，本门无效"; exit 1
fi

echo
echo "[2/4] A 正控：FX=ok（处理器只打印）"
run_case A ok 4000 1
grep -q "M126 HANDLER-ENTER fx=ok" "$LOG" && ok "A 处理器被调用（信号确实投递到分发线程）" || bad "A 处理器未被调用"
grep -q "M126 HANDLER-EXIT-OK"    "$LOG" && ok "A 处理器跑到结尾（正常路径未被隔离打断）" || bad "A 处理器未跑完"
[ "$HEALTH" = "HTTP/1.1 200 OK" ] && ok "A 处理器执行期间 /health 200" || bad "A /health 非 200（$HEALTH）"
[ "$HUNG" = 0 ] && [ "$RC" = 0 ] && ok "A 自然退出码 0（无假死）" || bad "A 退出异常（HUNG=$HUNG rc=$RC）"

echo
echo "[3/4] B/C/D/E 处理器内失败：进程必须存活且可服务"
for case_spec in "B abort 4000 1" "C huge 4000 1" "D inject 4000 1 PX_ALLOC_FAIL_MIN=$THRESH" "E storm 4500 1 PX_GC_THRESHOLD=100"; do
  set -- $case_spec
  tag=$1; fx=$2; hold=$3; nsig=$4; shift 4
  echo "  --- $tag FX=$fx $* ---"
  run_case "$tag" "$fx" "$hold" "$nsig" "$@"
  case "$tag" in
    B)
      grep -q "\[px-signal\] 信号处理器运行时错误已隔离" "$LOG" && ok "B [px-signal] 隔离行" || bad "B 无 [px-signal] 行"
      grep -q "信号处理器" "$LOG" && ok "B 文案指明是**信号处理器**隔离" || bad "B 文案未指明隔离点种类"
      grep -q "M126 HANDLER-EXIT-OK" "$LOG" && bad "B 处理器居然跑完（断言未生效？）" || ok "B 本次处理器调用被终止（未到 HANDLER-EXIT-OK）"
      ;;
    C)
      grep -q "\[px-signal\]" "$LOG" && ok "C [px-signal] 隔离行" || bad "C 无 [px-signal] 行"
      grep -qE "运行时错误: (内存不足|分配尺寸非法)" "$LOG" && ok "C 报出分类真因" || bad "C 未报分类真因"
      grep -q "信号处理器隔离点内 → 只终止本次处理器调用" "$LOG" && ok "C 文案标注「处理器调用级」（区分于协程级）" || bad "C 文案未区分隔离点种类"
      grep -q "errno=" "$LOG" && ok "C 文案带 errno" || bad "C 缺 errno"
      ;;
    D)
      grep -q "\[px-signal\]" "$LOG" && ok "D [px-signal] 隔离行" || bad "D 无 [px-signal] 行"
      grep -q "PX_ALLOC_FAIL_MIN 注入" "$LOG" && ok "D 真因标注为注入（可区分于真 OOM）" || bad "D 未标注注入"
      grep -qE "px-coro" "$LOG" && bad "D 连带打掉了运行时内部协程（阈值标定失效）" || ok "D 只打中处理器，未波及运行时内部"
      ;;
    E)
      grep -q "M126 HANDLER-ALLOC-DONE len=20000" "$LOG" && ok "E 处理器内 20000 字节存活对象构造完成" || bad "E 处理器分配阶段未完成"
      grep -qE "SIGSEGV|段错误|Segmentation" "$LOG" && bad "E 出现 SIGSEGV（GC 根集仍漏信号线程）" || ok "E 无 SIGSEGV（信号线程已纳入 GC 根集）"
      grep -q "\[px-signal\]" "$LOG" && bad "E 出现隔离（非 UAF 路径）" || ok "E 无隔离（走的正是 UAF 路径且未崩）"
      ;;
  esac
  # 四个用例共用的存活判据
  [ "$HEALTH" = "HTTP/1.1 200 OK" ] && ok "$tag 失败后 /health 仍 200（服务未中断）" || bad "$tag /health 非 200（$HEALTH）"
  grep -q "M126 MAIN-EXIT-NORMAL" "$LOG" && ok "$tag 主循环存活到自然退出（旧实现此处整进程 exit(1)）" || bad "$tag 主循环未存活"
  if [ "$HUNG" = 0 ] && [ "$RC" = 0 ]; then ok "$tag 退出码 0（信号处理器失败**不计入**退出码）"; else bad "$tag HUNG=$HUNG rc=$RC"; fi
done
grep -q "收尾提示：本次运行有 1 次信号处理器错误被隔离" /tmp/m126_B.log && ok "B 收尾提示给出隔离次数（证据主渠道）" || bad "B 缺收尾提示"

echo
echo "[4/4] F 负控（逃生舱）+ G 多信号 + H 无信号"
echo "  --- F PX_SPAWN_ISOLATE=0：必须回退旧的「一错即退」致命语义 ---"
run_case F abort 4000 1 PX_SPAWN_ISOLATE=0
[ "$RC" != 0 ] && ok "F 逃生舱下进程以非 0 退出（rc=$RC）——隔离不是「永远吞掉」" || bad "F 逃生舱未生效（rc=$RC）"
grep -q "\[px-signal\]" "$LOG" && bad "F 逃生舱下仍出现隔离行" || ok "F 逃生舱下无隔离行"
grep -q "M126 MAIN-EXIT-NORMAL" "$LOG" && bad "F 逃生舱下主循环居然存活" || ok "F 逃生舱下主循环未存活（= 修复前行为）"

echo "  --- G 连发 3 个信号：计数递增、进程持续存活 ---"
run_case G abort 6000 3
grep -q "第 3 次" "$LOG" && ok "G 第 3 次隔离被记录（每次信号独立隔离）" || bad "G 未记录到第 3 次"
grep -q "收尾提示：本次运行有 3 次信号处理器错误被隔离" "$LOG" && ok "G 收尾提示计数为 3" || bad "G 收尾计数不为 3"
[ "$HEALTH" = "HTTP/1.1 200 OK" ] && ok "G 连发期间 /health 200" || bad "G /health 非 200（$HEALTH）"
[ "$HUNG" = 0 ] && [ "$RC" = 0 ] && ok "G 退出码仍为 0" || bad "G HUNG=$HUNG rc=$RC"

echo "  --- H 自然退出（无信号）---"
run_case H ok 2000 0
[ "$RC" = 0 ] && [ "$HUNG" = 0 ] && ok "H 退出码 0" || bad "H HUNG=$HUNG rc=$RC"
grep -q "收尾提示" "$LOG" && bad "H 无信号却出现收尾提示" || ok "H 无信号 → 无隔离痕迹"
grep -q "M126 MAIN-EXIT-NORMAL" "$LOG" && ok "H 正常走到自然退出" || bad "H 未自然退出"

pkill -f "$BIN" 2>/dev/null
echo
echo "== 汇总：pass=$pass fail=$fail =="
[ "$fail" = 0 ] && echo "✅ M126 verify 全绿" || echo "❌ M126 verify 有失败"
exit $fail
