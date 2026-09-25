#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M154 门：分配率第二刀（小整数文本池 + 多字节 rune 池 + join 字节精确 + 常量池/比较器/钉住表）
# ---------------------------------------------------------------
# 门做四件事：
#   ① 语义：`m154_sem.px` 在**三轨**（VM 默认轨 / C 轨 / 解释轨）逐字节一致且 0 失败
#      —— 池化与字节口径修复**不得**改变任何值语义；缺陷 149 的 NUL 断言必须有据可依。
#   ② 分配预算（硬阈值、可复现）：`budget2.px` 用**默认轨**构建后运行，
#      `PX_GC_ALLOC_STATS=1` 的 `total=` 必须 ≤ 阈值。计数完全确定 ⇒ 可做硬门；
#      时间是受负载影响的伪判据，不做门。
#   ③ 输出冻结：两个语料的 `--emit-c` 指纹必须与 `golden.sha` 一致 —— 本轮改动全部在
#      runtime 与「内部实现方式」，**发射文本一字不得变**（与仓库级 emitc_freeze 门同向）。
#   ④ 负控 3 道（必须全红）：
#        A：关掉多字节 rune 池 ⇒ **预算**门必红
#        B：关掉小整数文本池 ⇒ **预算**门必红
#        C：join 退回 strlen 口径（缺陷 149 旧行为）⇒ **语义**门必红
# 口径：
#   · 负控会改 `runtime/runtime.c`：开门先查 NEGCTL 残留，开头快照，末尾逐字节还原；
#     本门持 flock（并发跑同一个门 = 假 diff + 权限错误，第 29 轮实锤）。
#   · 负控重编 runtime 走 `tools/px build`（缓存 key 随源码变 ⇒ 自动重建）。
# 用法：bash examples/m154_alloc2/verify.sh
# 退出码：0 = M154-VERIFY-OK；1 = 有失败项
# ═══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")"
# 并发互斥：门会改 runtime.c，两个同时跑必然互相污染
exec 9>/tmp/m154_gate.lock
if ! flock -n 9; then echo "M154-VERIFY-SKIP（已有同类门在跑）"; exit 1; fi

PX=../../tools/px
RT=../../runtime/runtime.c
LOG=/tmp/m154_build.log
FAIL=0
mkdir -p build

# 预算阈值（实测：本轮三处池化**全部生效** = 401,270；阈值 = 实测 ×1.2）。
#   为什么敢贴这么近：分配**计数完全确定**（同源码 + 同实现 ⇒ 同数字），不是受负载影响的
#   伪判据。三处池化任一处退回都正好 +20 万（= 语料相位 N）：
#     关 rune 池 ⇒ 601,266 · 关整数文本池 ⇒ 1,000,270（实测）⇒ 1.2 倍留量已足够判红。
#   （第 36 轮首次开门实测：阈值 65 万时负控 A 的 601,266 **漏网** ⇒ 收紧到 48 万。）
ALLOC_MAX=480000

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
cp "$RT" /tmp/m154_rt_snapshot.c
restore_rt() { [ -f /tmp/m154_rt_snapshot.c ] && cp /tmp/m154_rt_snapshot.c "$RT" 2>/dev/null; }
trap restore_rt INT TERM HUP

build_vm() { bash "$PX" build "$1" >>"$LOG" 2>&1; }

# ── ① 三轨语义 ──
say "── ① m154_sem.px 三轨语义 ──"
build_vm m154_sem.px || { say "  ❌ m154_sem.px（VM 轨）构建失败"; FAIL=$((FAIL+1)); }
VM_OUT=$(./build/m154_sem 2>&1); VM_RC=$?
build_c() { bash "$PX" build --c "$1" >>"$LOG" 2>&1; }
build_c m154_sem.px || { say "  ❌ m154_sem.px（C 轨）构建失败"; FAIL=$((FAIL+1)); }
C_OUT=$(./build/m154_sem 2>&1); C_RC=$?
build_vm m154_sem.px >/dev/null 2>&1   # 还原 VM 轨产物（后续步骤可能回读）
IFACE_OUT=$(bash "$PX" run m154_sem.px 2>&1); I_RC=$?
chk "VM 轨退出码" "$VM_RC" "0"
chk "C 轨退出码" "$C_RC" "0"
chk "解释轨退出码" "$I_RC" "0"
chk "VM/C 两轨输出逐字节一致" "$(printf '%s' "$VM_OUT" | sha256sum | cut -c1-16)" "$(printf '%s' "$C_OUT" | sha256sum | cut -c1-16)"
chk "解释轨输出与 VM 轨一致" "$(printf '%s' "$IFACE_OUT" | sha256sum | cut -c1-16)" "$(printf '%s' "$VM_OUT" | sha256sum | cut -c1-16)"
chk "0 项失败" "$(printf '%s' "$VM_OUT" | grep -c '^FAIL' || true)" "0"
chk "全部通过" "$(printf '%s' "$VM_OUT" | grep -c '^PASS' || true)" "$(printf '%s' "$VM_OUT" | grep -o 'pass=[0-9]*' | head -1 | cut -d= -f2)"

# ── ② 分配预算 ──
say "── ② 分配预算（budget2.px · 默认轨 · PX_GC_ALLOC_STATS=1）──"
build_vm budget2.px || { say "  ❌ budget2.px 构建失败"; FAIL=$((FAIL+1)); }
CORP_OUT=$(./build/budget2 2>/dev/null); CORP_RC=$?
chk "语料退出码" "$CORP_RC" "0"
say "     语料输出: $CORP_OUT"
STATS=$(PX_GC_ALLOC_STATS=1 ./build/budget2 2>&1 >/dev/null)
TOTAL=$(printf '%s' "$STATS" | grep -o 'total=[0-9]*' | head -1 | cut -d= -f2)
STRN=$(printf '%s' "$STATS" | grep -o 'type=4 n=[0-9]*' | head -1 | cut -d= -f3)
LSTN=$(printf '%s' "$STATS" | grep -o 'type=6 n=[0-9]*' | head -1 | cut -d= -f3)
echo "     total=$TOTAL （阈值 ≤ $ALLOC_MAX） · 字符串类 n=$STRN · 列表类 n=$LSTN"
if [ -n "$TOTAL" ] && [ "$TOTAL" -le "$ALLOC_MAX" ]; then
  echo "  ✅ 分配总数在预算内"
else
  echo "  ❌ 分配总数超预算：[$TOTAL] > $ALLOC_MAX"; FAIL=$((FAIL+1))
fi
echo "$TOTAL" > /tmp/m154_alloc_total.txt

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
  cp "$RT" /tmp/m154_rt_before.c
  if ! python3 "$py" "$RT"; then
    # M209：锚点失配**必须算失败** —— 修前只打 ⚠️ 就 return 1，而三处调用点都写的是
    #   `if negctl_patch ...; then`（失配即整道负控被跳过）⇒ 门仍然报 VERIFY-OK。
    #   实测（M209）：负控 C 的锚点自 `bi_join_item` 重构后**早已失配**，这道负控
    #   长期没跑过而没人知道（= 缺陷 275 同族：负控的「静默跳过」）。
    say "  ❌ $desc：补丁锚点失配（负控未真正执行 ⇒ 记失败）"
    cp /tmp/m154_rt_before.c "$RT"
    FAIL=$((FAIL+1))
    return 1
  fi
  return 0
}
negctl_restore() { cp /tmp/m154_rt_before.c "$RT"; }

# A：关掉多字节 rune 池 ⇒ 预算必红
if negctl_patch "A" negctl_a.py; then
  build_vm budget2.px >/dev/null 2>&1
  A_STATS=$(PX_GC_ALLOC_STATS=1 ./build/budget2 2>&1 >/dev/null)
  A_TOTAL=$(printf '%s' "$A_STATS" | grep -o 'total=[0-9]*' | head -1 | cut -d= -f2)
  if [ -n "$A_TOTAL" ] && [ "$A_TOTAL" -gt "$ALLOC_MAX" ]; then
    echo "  ✅ 负控 A 判红（total=$A_TOTAL > $ALLOC_MAX）"
  else
    echo "  ❌ 负控 A 未判红（关掉 rune 池后 total=[$A_TOTAL] 仍在预算内）"; FAIL=$((FAIL+1))
  fi
  negctl_restore
fi
build_vm budget2.px >/dev/null 2>&1

# B：关掉小整数文本池 ⇒ 预算必红
if negctl_patch "B" negctl_b.py; then
  build_vm budget2.px >/dev/null 2>&1
  B_STATS=$(PX_GC_ALLOC_STATS=1 ./build/budget2 2>&1 >/dev/null)
  B_TOTAL=$(printf '%s' "$B_STATS" | grep -o 'total=[0-9]*' | head -1 | cut -d= -f2)
  if [ -n "$B_TOTAL" ] && [ "$B_TOTAL" -gt "$ALLOC_MAX" ]; then
    echo "  ✅ 负控 B 判红（total=$B_TOTAL > $ALLOC_MAX）"
  else
    echo "  ❌ 负控 B 未判红（关掉整数文本池后 total=[$B_TOTAL] 仍在预算内）"; FAIL=$((FAIL+1))
  fi
  negctl_restore
fi
build_vm budget2.px >/dev/null 2>&1

# C：join 退回 strlen 口径（缺陷 149 旧行为）⇒ 语义必红
if negctl_patch "C" negctl_c.py; then
  build_vm m154_sem.px >/dev/null 2>&1
  C2_OUT=$(./build/m154_sem 2>&1); C2_RC=$?
  C2_FAILN=$(printf '%s' "$C2_OUT" | grep -c '^FAIL' || true)
  if [ "$C2_RC" != "0" ] || [ "$C2_FAILN" != "0" ]; then
    echo "  ✅ 负控 C 判红（RC=$C2_RC · FAIL=$C2_FAILN）"
  else
    echo "  ❌ 负控 C 未判红（join 退回 strlen 后 NUL 断言仍全绿）"; FAIL=$((FAIL+1))
  fi
  negctl_restore
fi
build_vm m154_sem.px >/dev/null 2>&1
build_vm budget2.px >/dev/null 2>&1

# ── 收尾 ──
if grep -q 'NEGCTL' "$RT"; then say "  ❌ runtime.c 仍有 NEGCTL 负控标记"; FAIL=$((FAIL+1)); fi
if ! cmp -s /tmp/m154_rt_snapshot.c "$RT"; then say "  ❌ runtime.c 未逐字节还原到门开始时的状态"; FAIL=$((FAIL+1)); fi
# 还原后必须重新全绿（否则"还原"本身是假的）
build_vm m154_sem.px >/dev/null 2>&1
FINAL_OUT=$(./build/m154_sem 2>&1)
if [ "$(printf '%s' "$FINAL_OUT" | grep -c '^FAIL' || true)" = "0" ] && printf '%s' "$FINAL_OUT" | grep -q 'fail=0'; then
  echo "  ✅ 还原后语义门重新全绿"
else
  echo "  ❌ 还原后语义门非全绿"; FAIL=$((FAIL+1))
fi

if [ "$FAIL" = "0" ]; then echo "M154-VERIFY-OK"; else echo "M154-VERIFY-FAIL（$FAIL 项）"; fi
exit $FAIL
