#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M153 门：常量池 / 短串池化（缺陷 145 的第一刀）
# ---------------------------------------------------------------
# 门做四件事：
#   ① 语义：`pool.px` 在**三轨**（VM 默认轨 / C 轨 / 解释轨）逐字节一致且 0 失败
#      —— 池化**不得**改变任何值语义（相等/序/长度/索引/容器/跨 GC 存活）。
#   ② 分配预算（硬阈值、可复现）：`alloc_corpus.px` 用**默认轨**构建后运行，
#      `PX_GC_ALLOC_STATS=1` 打印的 `total=` 必须 ≤ 阈值。
#      为什么用计数而不是时间：计数**完全确定**（同源码+同实现 ⇒ 同数字），
#      时间是受负载影响的伪判据。阈值取自实测（见 NOTES）。
#   ③ 输出冻结：语料与 pool.px 的 `--emit-c` 指纹必须与 `golden.sha` 一致
#      —— 池化是**纯性能**改动，任何发射差异都是回归。
#   ④ 负控 3 道（必须全红）：
#        A：`px_char1` 返回相邻字节的字符串 ⇒ **语义**门必红
#        B：`px_str_const`/`px_char1` 都不池化（退回每次新建）⇒ **预算**门必红
#        C：空串单例退回每次新建 ⇒ **预算**门必红
# 口径：
#   · 负控会改 `runtime/runtime.c`：开门先查 NEGCTL 残留，开头快照，末尾逐字节还原。
#   · 负控重编 runtime 走 `tools/px build`（缓存 key 随源码变 ⇒ 自动重建）。
# 用法：bash examples/m153_alloc/verify.sh
# 退出码：0 = M153-VERIFY-OK；1 = 有失败项
# ═══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")"
PX=../../tools/px
RT=../../runtime/runtime.c
LOG=/tmp/m153_build.log
FAIL=0
mkdir -p build

# 预算阈值（实测：池化前 2,840,232 · 池化后 483,635；此处给 1.6 倍余量仍能拦住退回）
ALLOC_MAX=800000

say() { echo "$@"; }
chk() { # chk <描述> <实际> <期望>
  if [ "$2" = "$3" ]; then echo "  ✅ $1"; else echo "  ❌ $1 实际=[$2] 期望=[$3]"; FAIL=$((FAIL+1)); fi
}

# ── 负控残留检查 + 快照 ──
for _f in ../../runtime/runtime.c ../../runtime/vm.c; do
    if [ -f "$_f" ] && grep -q 'NEGCTL' "$_f" 2>/dev/null; then
        echo "FAIL 负控残留：$_f 仍含 NEGCTL 标记（上一轮门被中断？先还原再跑）"
        exit 1
    fi
done
cp "$RT" /tmp/m153_rt_snapshot.c
restore_rt() { [ -f /tmp/m153_rt_snapshot.c ] && cp /tmp/m153_rt_snapshot.c "$RT" 2>/dev/null; }
trap restore_rt INT TERM HUP

build_vm() { bash "$PX" build "$1" >>"$LOG" 2>&1; }
build_c()  { bash "$PX" build --c "$1" >>"$LOG" 2>&1; }
run_vm()   { "$(dirname "$1")/build/$(basename "$1" .px)"; }

# ── ① 三轨语义 ──
say "── ① pool.px 三轨语义 ──"
build_vm pool.px || { say "  ❌ pool.px（VM 轨）构建失败"; FAIL=$((FAIL+1)); }
VM_OUT=$(./build/pool 2>&1); VM_RC=$?
build_c  pool.px || { say "  ❌ pool.px（C 轨）构建失败"; FAIL=$((FAIL+1)); }
C_OUT=$(./build/pool 2>&1); C_RC=$?
cp build/pool build/pool_c
IFACE_OUT=$(bash "$PX" run pool.px 2>&1); I_RC=$?
chk "VM 轨退出码" "$VM_RC" "0"
chk "C 轨退出码" "$C_RC" "0"
chk "解释轨退出码" "$I_RC" "0"
chk "VM/C 两轨输出逐字节一致" "$(printf '%s' "$VM_OUT" | sha256sum | cut -c1-16)" "$(printf '%s' "$C_OUT" | sha256sum | cut -c1-16)"
chk "解释轨输出与 VM 轨一致" "$(printf '%s' "$IFACE_OUT" | sha256sum | cut -c1-16)" "$(printf '%s' "$VM_OUT" | sha256sum | cut -c1-16)"
chk "0 项失败" "$(printf '%s' "$VM_OUT" | grep -c '^FAIL' || true)" "0"
chk "全部通过" "$(printf '%s' "$VM_OUT" | grep -c '^PASS' || true)" "$(printf '%s' "$VM_OUT" | grep -o 'pass=[0-9]*' | head -1 | cut -d= -f2)"

# ── ② 分配预算 ──
say "── ② 分配预算（alloc_corpus.px · 默认轨 · PX_GC_ALLOC_STATS=1）──"
build_vm alloc_corpus.px || { say "  ❌ alloc_corpus.px 构建失败"; FAIL=$((FAIL+1)); }
CORP_OUT=$(./build/alloc_corpus 2>/dev/null); CORP_RC=$?
chk "语料退出码" "$CORP_RC" "0"
STATS=$(PX_GC_ALLOC_STATS=1 ./build/alloc_corpus 2>&1 >/dev/null)
TOTAL=$(printf '%s' "$STATS" | grep -o 'total=[0-9]*' | head -1 | cut -d= -f2)
STRN=$(printf '%s' "$STATS" | grep -o 'type=4 n=[0-9]*' | head -1 | cut -d= -f3)
echo "     total=$TOTAL （阈值 ≤ $ALLOC_MAX） · 字符串类 n=$STRN"
if [ -n "$TOTAL" ] && [ "$TOTAL" -le "$ALLOC_MAX" ]; then
  echo "  ✅ 分配总数在预算内"
else
  echo "  ❌ 分配总数超预算：[$TOTAL] > $ALLOC_MAX"; FAIL=$((FAIL+1))
fi
echo "$TOTAL" > /tmp/m153_alloc_total.txt

# ── ③ 输出冻结 ──
say "── ③ 输出冻结（golden.sha）──"
FREEZE_OK=1
while IFS=' ' read -r f want; do
  [ -z "$f" ] && continue
  got=$(../../bootstrap/pxc_vm --emit-c "$f" 2>/dev/null | sha256sum | cut -c1-32)
  if [ "$got" = "$want" ]; then echo "  ✅ $f"; else echo "  ❌ $f 指纹变了：[${got:0:12}] ≠ [${want:0:12}]"; FREEZE_OK=0; fi
done < golden.sha
[ "$FREEZE_OK" = "1" ] || FAIL=$((FAIL+1))

# ── ④ 负控 ──
say "── ④ 负控（必须全红）──"
negctl_patch() { # negctl_patch <描述> <python 补丁文件>
  local desc="$1" py="$2"
  cp "$RT" /tmp/m153_rt_before.c
  if ! python3 "$py" "$RT"; then
    say "  ⚠️ $desc：锚点失配（补丁未生效）"; cp /tmp/m153_rt_before.c "$RT"; return 1
  fi
  return 0
}
negctl_restore() { cp /tmp/m153_rt_before.c "$RT"; }

# A：px_char1 返回相邻字节 ⇒ 语义必红
if negctl_patch "A" negctl_a.py; then
  build_vm pool.px >/dev/null 2>&1
  A_OUT=$(./build/pool 2>&1); A_RC=$?
  if [ "$A_RC" != "0" ] || [ "$(printf '%s' "$A_OUT" | grep -c '^FAIL' || true)" != "0" ]; then
    echo "  ✅ 负控 A 判红（RC=$A_RC · FAIL=$(printf '%s' "$A_OUT" | grep -c '^FAIL' || true)）"
  else
    echo "  ❌ 负控 A 未判红（改坏单字符串表后语义仍全绿）"; FAIL=$((FAIL+1))
  fi
  negctl_restore
fi
# 重建基线（负控 A 的篡改态已还原 ⇒ 重新烘正件）
build_vm pool.px >/dev/null 2>&1
build_vm alloc_corpus.px >/dev/null 2>&1

# B：不池化（常量池 + 单字符串表都退回新建）⇒ 预算必红
if negctl_patch "B" negctl_b.py; then
  build_vm alloc_corpus.px >/dev/null 2>&1
  B_STATS=$(PX_GC_ALLOC_STATS=1 ./build/alloc_corpus 2>&1 >/dev/null)
  B_TOTAL=$(printf '%s' "$B_STATS" | grep -o 'total=[0-9]*' | head -1 | cut -d= -f2)
  if [ -n "$B_TOTAL" ] && [ "$B_TOTAL" -gt "$ALLOC_MAX" ]; then
    echo "  ✅ 负控 B 判红（total=$B_TOTAL > $ALLOC_MAX）"
  else
    echo "  ❌ 负控 B 未判红（不池化后 total=[$B_TOTAL] 仍在预算内）"; FAIL=$((FAIL+1))
  fi
  negctl_restore
fi
build_vm alloc_corpus.px >/dev/null 2>&1

# C：空串单例退回每次新建 ⇒ 预算必红
if negctl_patch "C" negctl_c.py; then
  build_vm alloc_corpus.px >/dev/null 2>&1
  C_STATS=$(PX_GC_ALLOC_STATS=1 ./build/alloc_corpus 2>&1 >/dev/null)
  C_TOTAL=$(printf '%s' "$C_STATS" | grep -o 'total=[0-9]*' | head -1 | cut -d= -f2)
  if [ -n "$C_TOTAL" ] && [ "$C_TOTAL" -gt "$ALLOC_MAX" ]; then
    echo "  ✅ 负控 C 判红（total=$C_TOTAL > $ALLOC_MAX）"
  else
    echo "  ❌ 负控 C 未判红（空串不单例后 total=[$C_TOTAL] 仍在预算内）"; FAIL=$((FAIL+1))
  fi
  negctl_restore
fi
build_vm pool.px >/dev/null 2>&1
build_vm alloc_corpus.px >/dev/null 2>&1

# ── 收尾 ──
if grep -q 'NEGCTL' "$RT"; then say "  ❌ runtime.c 仍有 NEGCTL 负控标记"; FAIL=$((FAIL+1)); fi
if ! cmp -s /tmp/m153_rt_snapshot.c "$RT"; then say "  ❌ runtime.c 未逐字节还原到门开始时的状态"; FAIL=$((FAIL+1)); fi

if [ "$FAIL" = "0" ]; then echo "M153-VERIFY-OK"; else echo "M153-VERIFY-FAIL（$FAIL 项）"; fi
exit $FAIL
