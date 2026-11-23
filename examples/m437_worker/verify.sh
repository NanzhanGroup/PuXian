#!/usr/bin/env bash
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
# ============================================================
# examples/m437_worker/verify.sh —— dup2 语料冒烟（M437 · 缺陷 912 的语料面）
# ------------------------------------------------------------
# 为什么有本文件：
#   8d3db8f 提交了 examples/m437_worker/dup2_smoke.px 作为 dup2 的**仓库可见语料**
#   （缺陷 912：语料只存在于运行期生成器里 ⇒ check_native_coverage 判「新增未覆盖」）。
#   但语料只有 .px、没有 verify.sh ⇒ m421 examples 覆盖账把它记成 BLACKBOX，
#   真仓 rc=1（UNLISTED m437_worker）⇒ M437 重跑链 1 红。
#   ⇒ 本文件把该语料**真跑起来**（三轨），而不是往豁免表里塞一条 blackbox。
#
# 判据（逐层可单独变红）：
#   ① 解释轨：bootstrap/pxi 跑 dup2_smoke.px ⇒ stdout 与 POSIX 口径逐字节一致
#      （dup2(1,1)=1 · dup2(-1,1)=-1）
#   ② VM 轨：tools/px build 编译 + 运行 ⇒ stdout 与解释轨逐字节一致
#   ③ C 轨：tools/px build --c 编译 + 运行 ⇒ stdout 与解释轨逐字节一致
#   ④ 负控：拿一个错误期望去比 ⇒ 比对器必须判不同（判据不是空转）
#   ⑤ 前提自证：解释轨输出恰好 2 行非空（语料真被执行过，不是空文件）
# 用法：bash examples/m437_worker/verify.sh
# 退出码：0 = 绿 · 1 = 红 · 2 = 环境不成立（不降级成绿）
# ============================================================
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
cd "$ROOT" || { echo "ENV rc=2 无法进入仓库根"; exit 2; }
export LC_ALL=C LANG=C

PXI=./bootstrap/pxi
PX=./tools/px
CASE=examples/m437_worker/dup2_smoke.px

[ -x "$PXI" ] || { echo "❌ ENV rc=2 缺 $PXI"; exit 2; }
[ -f "$CASE" ] || { echo "❌ ENV rc=2 缺 $CASE"; exit 2; }

W=$(mktemp -d /tmp/m437ex.XXXXXX) || { echo "❌ ENV rc=2 mktemp 失败"; exit 2; }
trap 'rm -rf "$W"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

printf 'dup2(1,1)=1\ndup2(-1,1)=-1\n' > "$W/expect.txt"

echo "── [1] 解释轨：$CASE"
timeout 60 "$PXI" "$CASE" > "$W/interp.out" 2> "$W/interp.err"
rc=$?
[ "$rc" = 0 ] && ok "解释轨 rc=0" || bad "解释轨 rc=$rc"
[ -s "$W/interp.err" ] && sed 's/^/     stderr: /' "$W/interp.err"
cmp -s "$W/expect.txt" "$W/interp.out" \
  && ok "解释轨输出 = 期望（dup2(1,1)=1 · dup2(-1,1)=-1）" \
  || { bad "解释轨输出与期望不符"; sed 's/^/     /' "$W/interp.out"; }

# 复制到干净目录再编译（px build 输出到 <src_dir>/build/）
cp "$CASE" "$W/smoke.px"

echo "── [2] VM 轨（用户面默认轨）"
if timeout 400 "$PX" build "$W/smoke.px" > "$W/vm.build.log" 2>&1 && [ -x "$W/build/smoke" ]; then
  timeout 60 "$W/build/smoke" > "$W/vm.out" 2> "$W/vm.err"
  rc=$?
  [ "$rc" = 0 ] && ok "VM 轨 rc=0" || bad "VM 轨 rc=$rc"
  [ -s "$W/vm.err" ] && sed 's/^/     stderr: /' "$W/vm.err"
  cmp -s "$W/interp.out" "$W/vm.out" \
    && ok "VM 轨 stdout 与解释轨逐字节一致" \
    || { bad "VM 轨 stdout 与解释轨不一致"; diff "$W/interp.out" "$W/vm.out" | head -6 | sed 's/^/     /'; }
else
  bad "VM 轨编译失败"; tail -3 "$W/vm.build.log" | sed 's/^/     /'
fi

echo "── [3] C 轨逃生舱"
rm -rf "$W/build"
if timeout 500 "$PX" build --c "$W/smoke.px" > "$W/c.build.log" 2>&1 && [ -x "$W/build/smoke" ]; then
  timeout 60 "$W/build/smoke" > "$W/c.out" 2> "$W/c.err"
  rc=$?
  [ "$rc" = 0 ] && ok "C 轨 rc=0" || bad "C 轨 rc=$rc"
  [ -s "$W/c.err" ] && sed 's/^/     stderr: /' "$W/c.err"
  cmp -s "$W/interp.out" "$W/c.out" \
    && ok "C 轨 stdout 与解释轨逐字节一致" \
    || { bad "C 轨 stdout 与解释轨不一致"; diff "$W/interp.out" "$W/c.out" | head -6 | sed 's/^/     /'; }
else
  bad "C 轨编译失败"; tail -3 "$W/c.build.log" | sed 's/^/     /'
fi

echo "── [4] 负控：比对器有牙（错误期望必被判不同）"
printf 'dup2(1,1)=0\n' > "$W/bogus.txt"
cmp -s "$W/bogus.txt" "$W/interp.out" \
  && bad "负控失效：错误期望竟与实测一致（比对器在空转）" \
  || ok "负控：错误期望被比对器识别"

echo "── [5] 前提自证：语料真被执行（2 行非空输出）"
n=$(grep -c . "$W/interp.out" || true)
[ "${n:-0}" = 2 ] && ok "解释轨输出 $n 行非空（语料非空转）" || bad "解释轨输出行数=${n:-0}（期望 2）"

echo
echo "── 小计：通过 $pass · 失败 $fail ──"
if [ "$fail" -gt 0 ]; then
  echo "❌ dup2 语料冒烟失败"
  exit 1
fi
echo "✅ dup2 语料冒烟通过（三轨一致）"
exit 0
