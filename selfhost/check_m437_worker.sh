#!/usr/bin/env bash
# ============================================================
# M437 门：`px --worker` 常驻协议（缺陷 860）
# ------------------------------------------------------------
# 病根（M423 记录现象，M437 定位到结构性原因）：
#   M25 起 runtime 侧就有进程池（PXWorker / 任务帧 / 结果帧 / 空闲回收 / 滚动重启），
#   但 worker 那一端**从来不存在** —— `px --worker` 实测只报「未知子命令」，
#   于是每次 px_exec 先派生 4 个必然失败的进程，再退化 fork+exec。
#   为什么写不出来：协议要求 worker「执行脚本期间把 stdout 重定向到捕获通道、执行完恢复」，
#   而语言层 IO 面此前**没有 dup/dup2** ⇒ 在 .px 里表达不出来。
# M437 交付：runtime 补 dup2 内置；interp.px 实现 _wt_worker_loop；runtime px_pool_spawn
#   结果通道改 fd 3（stdout 留给脚本捕获）；tools/px 转发 --worker。
#
# 判据（7 组）：
#   [1] 静态形状：四件改动在位
#   [2] 协议判据（selfhost/check_m437_worker_probe.py）：
#        P1 单帧 · P2 同 worker 连 3 帧状态重置 · P4 运行时错误传播 · P5 错误后 worker 仍服务 ·
#        N1 缺失路径不静默成功 · N2 越界帧响亮退出（rc=2 + 指名）
#   [3] 双态对拍 P3：PX_POOL_WORKERS=0 vs 2，px_exec 输出逐字节一致
#   [4] 前提自证 P7：PX_POOL_WORKERS=2 时子进程里真有 pxi --worker（不是退化路径冒充）
#   [5] 默认值口径 G1：g_px_pool_size 仍为 0（不改变默认行为）
#   [6] 名册：dup2 已进 lint_core.px 内置名册（否则 lint 误报 L002）
#   [7] 收尾：捕获文件无残留
# 用法：bash selfhost/check_m437_worker.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
cd "$(dirname "$0")/.."
ROOT=$(pwd)
PXI=$ROOT/bootstrap/pxi
PX=$ROOT/tools/px
PROBE=$ROOT/selfhost/check_m437_worker_probe.py
W=/tmp/m437_gate_$$
mkdir -p "$W"
trap 'rm -rf "$W"' EXIT
# 捕获文件判据的**基线**：worker 的捕获文件在 /tmp/.pxw_*.out。**只看「本次新增」**——
#   全局「一个都不许有」会被历史残留骗红（实测：早期用 exit(7) 做 P4 时，worker 被 exit
#   直接终止、来不及 remove(cap)，留下 6 个老文件 ⇒ 本门的收尾判据红在一个与本轮无关的现场上）。
PXW_BEFORE=$(ls /tmp/.pxw_*.out 2>/dev/null | LC_ALL=C sort | grep -v '^$' || true)
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M437 门：px --worker 常驻协议（缺陷 860） =="
echo

echo "[1/7] 静态形状：四件改动在位"
grep -q 'static LXValue bi_dup2' runtime/runtime.c \
  && ok "dup2 内置已实现（runtime.c bi_dup2）" || bad "缺 bi_dup2"
grep -q 'px_set_global("dup2", px_native("dup2", bi_dup2))' runtime/runtime.c \
  && ok "dup2 已注册到全局" || bad "dup2 未注册"
grep -q 'dup2(out_pipe\[1\], 3)' runtime/runtime.c \
  && ok "结果通道走 fd 3（stdout 留给脚本）" || bad "px_pool_spawn 未把结果通道放到 fd 3"
grep -q 'close_range(4' runtime/runtime.c \
  && ok "close_range 从 4 起（不误关 fd 3）" || bad "close_range 起点不是 4"
if grep -q 'if (direct) return -1' runtime/runtime.c; then
  bad "px_pool_spawn 仍在 direct 时拒绝（pxi worker 不可用）"
else
  ok "px_pool_spawn 允许直连解释器（pxi --worker）"
fi
grep -q 'def _wt_worker_loop' selfhost/interp.px \
  && ok "interp.px 有 _wt_worker_loop" || bad "缺 _wt_worker_loop"
grep -q 'a\[1\] == "--worker"' selfhost/interp.px \
  && ok "interp.px main 有 --worker 分支" || bad "main 缺 --worker 分支"
grep -q -- '--worker) shift; exec' tools/px \
  && ok "tools/px 转发 --worker（exec 解释器）" || bad "tools/px 缺 --worker 转发"
echo

echo "[2/7] 协议判据（P1/P2/P4/P5/N1/N2 · 装置 selfhost/check_m437_worker_probe.py）"
if [ ! -f "$PROBE" ]; then
  bad "缺测试装置 $PROBE"
else
  PROBE_OUT=$(timeout 240 python3 -u "$PROBE" "$PXI" "$W" 2>&1); PRC=$?
  echo "$PROBE_OUT" | sed 's/^/    /'
  for k in P1 P2 P4 P5 N1 N2; do
    if echo "$PROBE_OUT" | grep -q "^OK $k"; then ok "$k"; else bad "$k"; fi
  done
  [ "$PRC" = 0 ] || bad "装置整体 rc=$PRC"
fi
echo

echo "[3/7] 双态对拍 P3：PX_POOL_WORKERS=0 vs 2（px_exec 输出逐字节一致）"
cat > "$W/target.px" <<'EOF'
var s = 0
var i = 0
while i < 5:
    s = s + i
    i = i + 1
print("target-sum=" + str(s))
print("target-argv=" + str(args()))
EOF
cat > "$W/driver.px" <<EOF
var r = px_exec("$W/target.px")
print("EXEC[" + r + "]")
print("LEN=" + str(len(r)))
EOF
if "$PX" build "$W/driver.px" >"$W/build1.log" 2>&1; then
  ok "driver 构建"
  D="$W/build/driver"
  # ⚠️ 必须显式 PX_BIN=$PXI：driver 在 /tmp 下，px_interp_bin 的「自身路径推导」找不到仓库
  #    ⇒ 会退回 PATH 里的 px（本机是 /usr/bin/px 0.2.436，**不认识 --worker**）⇒ 池全部 spawn
  #    失败再退化，双态对拍会「假绿」（两条都走退化）。这条实测踩中过。
  PX_BIN="$PXI" PX_POOL_WORKERS=0 timeout 90 "$D" > "$W/pool0.out" 2> "$W/pool0.err"; R0=$?
  PX_BIN="$PXI" PX_POOL_WORKERS=2 timeout 90 "$D" > "$W/pool2.out" 2> "$W/pool2.err"; R2=$?
  [ "$R0" = 0 ] && ok "池关闭（PX_POOL_WORKERS=0）rc=0" || { bad "池关闭 rc=$R0"; tail -3 "$W/pool0.err" | sed 's/^/     /'; }
  [ "$R2" = 0 ] && ok "池开启（PX_POOL_WORKERS=2）rc=0" || { bad "池开启 rc=$R2"; tail -3 "$W/pool2.err" | sed 's/^/     /'; }
  if [ "$R0" = 0 ] && [ "$R2" = 0 ]; then
    if diff -q "$W/pool0.out" "$W/pool2.out" >/dev/null 2>&1; then
      ok "双态输出逐字节一致（$(head -1 "$W/pool0.out")）"
    else
      bad "双态输出不一致"; diff "$W/pool0.out" "$W/pool2.out" | head -6 | sed 's/^/     /'
    fi
  else
    bad "双态对拍前置失败（至少一侧 rc≠0）"
  fi
else
  bad "driver 构建失败"; tail -5 "$W/build1.log" | sed 's/^/     /'
fi
echo

echo "[4/7] 前提自证 P7：PX_POOL_WORKERS 开启时子进程里真有 pxi --worker"
cat > "$W/slow.px" <<'EOF'
sleep(600)
print("slow-done")
EOF
cat > "$W/driver_slow.px" <<EOF
var r = px_exec("$W/slow.px")
print("SLOW[" + r + "]")
EOF
if "$PX" build "$W/driver_slow.px" >"$W/build2.log" 2>&1; then
  PX_BIN="$PXI" PX_POOL_WORKERS=1 timeout 60 "$W/build/driver_slow" > "$W/slow.out" 2> "$W/slow.err" &
  BG=$!
  FOUND=0
  for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
    sleep 0.2
    # 进程名口径：runtime 的 execlp 用 argv[0]="px"（历史口径，见 px_pool_spawn），
    # 所以 ps 里显示 `px --worker` 或 `pxi --worker`（tools/px 转发时 exec 后可能保留 pxi 名）。
    if ps -ef | grep -qE '[p]xi? --worker'; then FOUND=1; break; fi
  done
  wait $BG 2>/dev/null
  if [ "$FOUND" = 1 ]; then
    ok "观测到 pxi --worker 常驻进程（池路径真被执行）"
  else
    bad "未观测到 pxi --worker 进程（可能退化到 fork+exec）"
    ps -ef | grep "[p]x" | head -3 | sed 's/^/     /'
  fi
  grep -q "slow-done" "$W/slow.out" && ok "慢脚本经池执行输出正确" || { bad "慢脚本输出异常"; cat "$W/slow.out" "$W/slow.err" 2>/dev/null | head -3 | sed 's/^/     /'; }
else
  bad "driver_slow 构建失败"; tail -5 "$W/build2.log" | sed 's/^/     /'
fi
echo

echo "[5/7] 默认值口径 G1：池默认仍关闭（不改变默认行为）"
if grep -q 'static int g_px_pool_size = 0;' runtime/runtime.c; then
  ok "g_px_pool_size 默认 0（池需显式 PX_POOL_WORKERS=N 开启）"
else
  bad "默认值被改动（需同步更新 CHANGELOG 与本门）"
fi
echo

echo "[6/7] 名册：dup2 已进 lint_core.px 内置名册"
# 名册是**一行** split("…")，用词边界匹配（`grep '"dup2"'` 会假红 —— 实测踩过）
grep -qw 'dup2' tools/lint_core.px \
  && ok "lint_core.px 名册含 dup2" || bad "名册缺 dup2（跑 tools/gen_builtin_list.sh）"
echo

echo "[7/7] 收尾：本次无新增捕获文件残留"
if [ "$NEG_SKIP" = 1 ]; then
  ok "（--neg-skip：跳过残留检查）"
else
  sleep 0.3
  PXW_AFTER=$(ls /tmp/.pxw_*.out 2>/dev/null | LC_ALL=C sort | grep -v '^$' || true)
  NEWPXW=$(comm -13 <(printf '%s\n' "$PXW_BEFORE" | grep -v '^$' || true) \
                     <(printf '%s\n' "$PXW_AFTER" | grep -v '^$' || true) | grep -c . || true)
  [ "${NEWPXW:-0}" = 0 ] && ok "本次无新增捕获文件（基线 $(printf '%s\n' "$PXW_BEFORE" | grep -c . || true) 个历史文件不计）" \
                         || bad "本次新增 $NEWPXW 个捕获文件"
fi
echo

echo "── 小计：通过 $pass · 失败 $fail ──"
if [ "$fail" -gt 0 ]; then
  echo "❌ M437 worker 协议门失败"
  exit 1
fi
echo "✅ M437 worker 协议门通过"
exit 0
