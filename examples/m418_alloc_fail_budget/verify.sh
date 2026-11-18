#!/usr/bin/env bash
# ============================================================
# M418（缺陷 840）门：PX_ALLOC_FAIL_MIN 的**注入预算**
# ------------------------------------------------------------
# 病灶（CI run 44292432507 @858c81d 实证）：
#   钩子注释写「注入一次失败」，实现是「每次 ≥ 阈值都注入」
#   ⇒ 隔离点内第一次失败被 longjmp 吃掉后，进程后续任何 ≥ 阈值的分配
#     （GC 扩容 / 大对象）若落在隔离点外 ⇒ px_alloc_fail 的
#     「无隔离点 → 保留致命语义」分支 _exit(1)
#   ⇒ m403 → m126 D 档在 4 核 runner 上「主循环未存活 rc=1」，本地 16 核 42/0 全绿
#     = 判据依赖「这台机器这 4 秒里恰好没有第二笔大分配」这一环境事实（M415 同族）
# 修法：PX_ALLOC_FAIL_MAX=N（默认 0/未设 = 不限次数 = 旧行为）；
#   需要确定性的门用 MAX=1 把「注入几次」变成显式契约。
# 判据：
#   T1 正控 MAX=1  ：处理器内失败被隔离 → 第二、三笔 8MB 分配成功 → EXIT-NORMAL rc=0
#   T2 负控 不设 MAX：同夹具 ⇒ 第二笔分配 _exit(1) rc≠0 且日志含「无隔离点 → 保留致命语义」
#                    （这是 CI 真因的**可判复现**，不是演示）
#   T3 前提自证     ：T1 注入计数 = 1 且隔离行 = 1；T2 注入计数 = 2
#   T4 MAX 不影响未触发场景：MIN 拉到极大 ⇒ 三笔分配全成功、无隔离行
#   T5 阈值边界     ：MIN = 载荷字节数（8388609）⇒ 处理器内分配（8388610 请求）仍被注入
#                    （证明判据踩的是请求尺寸，不是其它）
# 用法：bash examples/m418_alloc_fail_budget/verify.sh
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m418_alloc_fail_budget
PX=${PX:-$ROOT/tools/px}
BIN=$ROOT/$DIR/build/alloc_budget
PAYLOAD=/tmp/m418_payload.bin
THRESH=8388610
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M418 验证（PX_ALLOC_FAIL_MIN 注入预算 · 缺陷 840）=="
echo "[0/3] 载荷"
[ -f "$PAYLOAD" ] && [ "$(stat -c%s "$PAYLOAD")" = 8388609 ] || head -c 8388609 /dev/zero | tr '\0' 'A' > "$PAYLOAD"
echo "     payload=$(stat -c%s "$PAYLOAD") 字节  阈值=$THRESH"

echo "[1/3] 编译（仓库 tools/px ⇒ 仓库 runtime/）"
if "$PX" build "$DIR/alloc_budget.px" > /tmp/m418_build.log 2>&1 && [ -x "$BIN" ]; then
  ok "编译成功（$(stat -c%s "$BIN") 字节）"
else
  bad "编译失败"; tail -20 /tmp/m418_build.log | sed 's/^/     /'; exit 1
fi

# run_case <tag> [env...] → RC / LOG
run_case() {
  local tag="$1"; shift
  LOG=/tmp/m418_${tag}.log; RC=99
  pkill -f "$BIN" 2>/dev/null; sleep 0.3
  : > "$LOG"
  if [ $# -gt 0 ]; then env "$@" "$BIN" >> "$LOG" 2>&1 &
  else "$BIN" >> "$LOG" 2>&1 & fi
  local pid=$! i
  for i in $(seq 1 60); do grep -q "M126\|READY" "$LOG" && break; sleep 0.25; done
  grep -q "READY" "$LOG" || { bad "$tag 启动失败（无 READY）"; RC=99; return; }
  kill -USR1 "$pid" 2>/dev/null
  wait "$pid" 2>/dev/null; RC=$?
}

echo "[2/3] T1/T2 正控与负控（同一夹具，只差 MAX）"
run_case T1 PX_ALLOC_FAIL_MIN=$THRESH PX_ALLOC_FAIL_MAX=1
grep -q "\[px-signal\]" /tmp/m418_T1.log && ok "T1 处理器内失败已隔离（前提：信号真投递到处理器）" || bad "T1 无隔离行"
grep -q "SECOND-ALLOC-OK" /tmp/m418_T1.log && ok "T1 隔离后第二笔 8MB 分配成功（预算用完即恢复正常）" || bad "T1 第二笔分配未成功（钩子仍连坐）"
grep -q "THIRD-ALLOC-OK" "$LOG" 2>/dev/null && ok "T1 第三笔分配成功" || bad "T1 第三笔分配未成功"
grep -q "EXIT-NORMAL" /tmp/m418_T1.log && ok "T1 自然退出（进程存活到底）" || bad "T1 未自然退出"
[ "$RC" = 0 ] && ok "T1 退出码 0" || bad "T1 退出码 $RC（应为 0）"

run_case T2 PX_ALLOC_FAIL_MIN=$THRESH
[ "$RC" != 0 ] && ok "T2 负控：不设 MAX ⇒ 退出码 $RC（旧行为，进程被连坐）" || bad "T2 旧行为居然存活（负控失效，判据无牙）"
grep -q "无隔离点 → 保留致命语义" /tmp/m418_T2.log && ok "T2 死因是「无隔离点 → 保留致命语义」（= CI 现场那条路径）" || bad "T2 死因不符"
grep -qE "SECOND-ALLOC-OK" /tmp/m418_T2.log && bad "T2 第二笔分配居然成功（负控前提不成立）" || ok "T2 第二笔分配确实失败（负控前提自证）"

echo "[3/3] T3 前提自证 · T4 未触发场景 · T5 阈值边界"
n1=$(grep -c "PX_ALLOC_FAIL_MIN 注入" /tmp/m418_T1.log)
n2=$(grep -c "PX_ALLOC_FAIL_MIN 注入" /tmp/m418_T2.log)
[ "$n1" = 1 ] && ok "T3 T1 注入恰 1 次（MAX=1 生效）" || bad "T3 T1 注入 $n1 次"
[ "$n2" = 2 ] && ok "T3 T2 注入 2 次（不限次数 = 旧行为，与 MAX=1 形成对照）" || bad "T3 T2 注入 $n2 次（期望 2）"
ok1=$(grep -c "\[px-signal\] 信号处理器运行时错误已隔离" /tmp/m418_T1.log)
[ "$ok1" = 1 ] && ok "T3 T1 隔离行恰 1 次" || bad "T3 T1 隔离行 $ok1 次"

run_case T4 PX_ALLOC_FAIL_MIN=999999999999
grep -q "THIRD-ALLOC-OK" /tmp/m418_T4.log && ok "T4 MIN 未触发 ⇒ 三笔分配全成功（钩子默认关）" || bad "T4 未触发场景异常"
grep -q "\[px-signal\]" /tmp/m418_T4.log && bad "T4 未触发场景竟有隔离行（判据恒真？）" || ok "T4 未触发场景无隔离痕迹"
[ "$RC" = 0 ] && ok "T4 退出码 0" || bad "T4 退出码 $RC"

run_case T5 PX_ALLOC_FAIL_MIN=8388609 PX_ALLOC_FAIL_MAX=1
grep -q "PX_ALLOC_FAIL_MIN 注入" /tmp/m418_T5.log && ok "T5 MIN=载荷字节(8388609) 仍注入 ⇒ 判据踩的是请求尺寸 8388610" || bad "T5 阈值边界不符"

echo
echo "== 汇总：pass=$pass fail=$fail =="
[ "$fail" = 0 ] && { echo "✅ M418 verify 全绿"; exit 0; } || { echo "❌ M418 verify 有失败"; exit 1; }
