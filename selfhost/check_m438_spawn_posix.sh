#!/usr/bin/env bash
. "$(dirname "$0")/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
# ============================================================
# M438 门 · posix_spawn 快路径（缺陷 916 · 晨曦报告）
# ------------------------------------------------------------
# 病根：os_spawn / os_spawn_capture / os_capture 的子进程一律 fork(2) 起。多线程宿主里
#   fork 要复制整个地址空间（页表 / malloc arena）。独立 C 对照（data/autodrive/M438.md）：
#     线程数    裸 fork+exec   posix_spawn   捕获形状 fork+dup2   posix_spawnp
#       0        1.60 ms        1.78 ms         4.79 ms            3.99 ms
#      64        4.14 ms        1.43 ms         7.97 ms            4.28 ms
#     264       15.18 ms        2.15 ms        20.50 ms            4.38 ms
# 修：posix_spawnp + spawnattr(SETPGROUP) + file_actions(dup2/open/close/chdir_np)；
#   需 setsid（glibc 2.34 无 POSIX_SPAWN_SETSID）或 spawn 失败 ⇒ **回落原 fork 路径**
#   （语义逐字节不变，rc=127 由子进程 _exit 给出）。
#
# 判据（5 组）：
#   [1] 静态形状：快路径函数在位 + 两个调用点接入 + fork 回落仍在
#   [2] 功能等价：解释 · VM · C 三轨跑 probes/m438_spawn_equiv.px，20 条用例全过且逐字节一致
#   [3] 前提自证：C 轨 strace 里出现 CLONE_VM|CLONE_VFORK（真走 posix_spawnp，不是 fork）
#   [4] 负控：PX_SPAWN_FORK=1（同一二进制只切一条路径）⇒ 计数归零
#   [5] 前提自证：负控 trace 里确有不带 CLONE_VM 的 clone（扰动真发生）+ 功能仍全绿
# 用法：bash selfhost/check_m438_spawn_posix.sh
# 退出码：0 = 绿 · 1 = 红 · 2 = 环境不成立（缺 strace 等）
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/.."
ROOT=$(pwd)
PXI=./bootstrap/pxi
PX=./tools/px
CASE=selfhost/probes/m438_spawn_equiv.px
export LC_ALL=C LANG=C
for f in "$PXI" "$PX" "$CASE"; do
  [ -e "$f" ] || { echo "❌ ENV rc=2 缺 $f"; exit 2; }
done
command -v strace >/dev/null 2>&1 || { echo "❌ ENV rc=2 缺 strace（本门的前提自证依赖它）"; exit 2; }
W=$(mktemp -d /tmp/m438sp.XXXXXX) || { echo "❌ ENV rc=2 mktemp 失败"; exit 2; }
trap 'rm -rf "$W"' EXIT
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M438 门 · posix_spawn 快路径（缺陷 916） =="

echo "[1/5] 静态形状"
grep -q 'static int px_spawn_posix(' runtime/runtime.c \
  && ok "快路径实现 px_spawn_posix 在位" || bad "缺 px_spawn_posix"
grep -q 'px_spawn_posix(cmd, argv, envp, in_path' runtime/runtime.c \
  && ok "os_spawn 已接入快路径" || bad "os_spawn 未接入"
grep -q 'px_spawn_posix(cmd, argv, envp, o->in_path' runtime/runtime.c \
  && ok "os_spawn_capture / os_capture 已接入快路径" || bad "捕获路径未接入"
grep -q 'pid = fork();' runtime/runtime.c \
  && ok "fork 回落路径仍在（setsid / spawn 失败时保语义）" || bad "fork 回落被删（setsid 与 rc=127 语义会变）"
grep -q 'POSIX_SPAWN_SETPGROUP' runtime/runtime.c \
  && ok "进程组用 POSIX_SPAWN_SETPGROUP 表达" || bad "未用 SETPGROUP"
grep -q 'PX_SPAWN_FORK' runtime/runtime.c \
  && ok "逃生口 PX_SPAWN_FORK 在位（供 [4] 负控）" || bad "缺逃生口 PX_SPAWN_FORK"

echo "[2/5] 功能等价（解释 · VM · C 三轨）"
timeout 120 "$PXI" "$CASE" > "$W/interp.out" 2> "$W/interp.err"
rc=$?
[ "$rc" = 0 ] && ok "解释轨 rc=0（20 条用例全过）" || { bad "解释轨 rc=$rc"; tail -4 "$W/interp.out" | sed 's/^/     /'; }
cp "$CASE" "$W/s.px"
if timeout 500 "$PX" build "$W/s.px" > "$W/vm.log" 2>&1 && [ -x "$W/build/s" ]; then
  timeout 120 "$W/build/s" > "$W/vm.out" 2>&1
  cmp -s "$W/interp.out" "$W/vm.out" && ok "VM 轨与解释轨逐字节一致" \
    || { bad "VM 轨与解释轨不一致"; diff "$W/interp.out" "$W/vm.out" | head -6 | sed 's/^/     /'; }
else
  bad "VM 轨编译失败"; tail -3 "$W/vm.log" | sed 's/^/     /'
fi
rm -rf "$W/build"
if timeout 600 "$PX" build --c "$W/s.px" > "$W/c.log" 2>&1 && [ -x "$W/build/s" ]; then
  timeout 120 "$W/build/s" > "$W/c.out" 2>&1
  cmp -s "$W/interp.out" "$W/c.out" && ok "C 轨与解释轨逐字节一致" \
    || { bad "C 轨与解释轨不一致"; diff "$W/interp.out" "$W/c.out" | head -6 | sed 's/^/     /'; }
else
  bad "C 轨编译失败"; tail -3 "$W/c.log" | sed 's/^/     /'
fi
BIN="$W/build/s"

echo "[3/5] 前提自证：真走 posix_spawnp"
if [ -x "$BIN" ]; then
  strace -f -e trace=clone,clone3,fork,vfork -o "$W/fast.trace" timeout 120 "$BIN" > /dev/null 2>&1
  nfast=$(grep -c 'CLONE_VFORK' "$W/fast.trace" || true)
  echo "     CLONE_VM|CLONE_VFORK 次数 = $nfast"
  [ "${nfast:-0}" -ge 10 ] && ok "默认路径真走 posix_spawnp（CLONE_VFORK 出现 $nfast 次 ≥10）" \
    || bad "默认路径 CLONE_VFORK 仅 ${nfast:-0} 次（快路径没生效）"
else
  bad "缺 C 轨产物，无法做前提自证"
fi

echo "[4/5] 负控：PX_SPAWN_FORK=1（同一二进制，只切一条路径）"
if [ -x "$BIN" ]; then
  PX_SPAWN_FORK=1 timeout 180 "$BIN" > "$W/slow.out" 2>&1
  rc=$?
  [ "$rc" = 0 ] && ok "逃生口下功能仍全绿（rc=0）" || bad "逃生口下 rc=$rc（回落路径被破坏）"
  cmp -s <(grep -v '^PASS=' "$W/interp.out") <(grep -v '^PASS=' "$W/slow.out") \
    && ok "逃生口输出与解释轨逐字节一致（回落语义未变）" || bad "逃生口输出与解释轨不一致"
  strace -f -e trace=clone,clone3,fork,vfork -o "$W/slow.trace" env PX_SPAWN_FORK=1 timeout 180 "$BIN" > /dev/null 2>&1
  nslow=$(grep -c 'CLONE_VFORK' "$W/slow.trace" || true)
  echo "     负控 CLONE_VFORK 次数 = ${nslow:-0}"
  [ "${nslow:-1}" -eq 0 ] && ok "负控下 CLONE_VFORK 归零（判据有牙，不是空转）" \
    || bad "负控下 CLONE_VFORK 仍有 ${nslow:-0} 次（判据在空转）"
else
  bad "缺 C 轨产物，负控无法执行"
fi

echo "[5/5] 前提自证：负控的 fork 真的发生了"
if [ -f "$W/slow.trace" ]; then
  nfork=$(awk '/CLONE_VM/ {next} /clone3?\(|fork\(|vfork\(/ {n++} END {print n+0}' "$W/slow.trace")
  echo "     负控非 CLONE_VM 的 clone/fork 次数 = ${nfork:-0}"
  [ "${nfork:-0}" -ge 5 ] && ok "负控里确有 fork 型 clone ${nfork} 次（扰动真发生）" \
    || bad "负控里几乎没有 fork 记录（${nfork:-0} 次）⇒ 负控不成立"
else
  bad "缺负控 trace"
fi

echo
echo "── 小计：通过 $pass · 失败 $fail ──"
[ "$fail" -gt 0 ] && { echo "❌ M438 posix_spawn 门失败"; exit 1; }
echo "✅ M438 posix_spawn 门通过"
exit 0
