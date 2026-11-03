#!/usr/bin/env bash
# ============================================================
# examples/m373_data_enum_nested/verify.sh
#   M373：data enum 载荷的**复合值**（数组 / 嵌套 enum）+ GC 根面
# ------------------------------------------------------------
# 为什么有这道门：
#   M372/S3 落了载荷「值语义」，但它的实测覆盖面只到**标量**（float/str）。
#   载荷的实现载体是通用 dict（键 "0"…"N-1"），因此数组/嵌套 enum 在
#   **三轨上本来就能跑** —— 但**没有任何门覆盖它**。
#   「能用但没人测」= 下次谁改 compare_values / gc_mark / px_fmt_value_n
#   都会静默改坏它，而全量门全绿。本门的职责就是把这个面钉住。
#
# 判据（每条都有牙；牙在哪写清楚了）：
#   A 三轨 nested.px 逐字节 == expected_nested.txt（17 行，覆盖复合载荷的面）
#   B GC 根面（C 轨 · PX_GC_THRESHOLD=5000 强制频繁回收）：
#       B1 输出值正确（200000）—— 功能没坏
#       B2 **不得出现** PX_GC_LIVECHK / PX_GC_UAFDET 报错
#          ⇒ 这是 UAF 的直接检测器：若 enum payload(dict) 没被标记，
#            回收后读载荷会当场响亮（不是"等着某天偶发崩"）
#       B3 stats 里 runs > 0 —— **前提自证**：证明这次真跑了 GC。
#          没有 B3，B2 可能是"GC 根本没触发所以没报错"的假绿。
#   C 循环引用打印有保护（不挂死、不无限展开）：`{self: ...}`
#   D 不支持的复合模式给**明确错误**（不是静默不匹配）：
#      `case [1, 2]:` ⇒ E2001「无效的模式: [」—— 文档化限制，防止静默
#   E 覆盖度自证：用例文件的关键行必须在（防止被改空后"全绿"）
#
# 负控（证明判据有牙）：
#   NC1 篡改 expected 副本一行 ⇒ A 必红（并自证篡改真发生）
#   NC2 把 PX_GC_THRESHOLD 设为极大值 ⇒ runs==0 ⇒ **B3 必红**
#       （证明「前提自证」不是摆设：GC 没跑时门不会假绿）
#
# 用法：verify.sh [--neg-skip]
# 退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m373_nested.$$"
NEG_SKIP=0; [ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK"

[ -x "$PX" ] || { echo "❌ 找不到 $PX（先跑 selfhost/rebake_bin.sh）"; exit 1; }
D="$(cd "$(dirname "$0")" && pwd)"
for f in nested.px expected_nested.txt gcstress.px cycle.px badpat.px; do
  [ -r "$D/$f" ] || { echo "❌ 缺 $f"; exit 1; }
done

echo "═══ M373 · data enum 复合载荷（数组 / 嵌套）+ GC 根面 ═══"

# ---------- A：三轨逐字节 ----------
run_three() {   # $1=源 $2=tag
  local src="$1" tag="$2"
  cp "$src" "$WORK/$tag.px"
  ( cd "$WORK" && "$PX" run "$tag.px" > "$WORK/$tag.interp" 2> "$WORK/$tag.interp.err" ); local ri=$?
  rm -rf "$WORK/build"
  ( cd "$WORK" && "$PX" build "$tag.px" > "$WORK/$tag.vm.log" 2>&1 ); local rb=$?
  if [ $rb -eq 0 ] && [ -x "$WORK/build/$tag" ]; then "$WORK/build/$tag" > "$WORK/$tag.vm" 2> "$WORK/$tag.vm.err"; else : > "$WORK/$tag.vm"; fi
  rm -rf "$WORK/build"
  ( cd "$WORK" && "$PX" build --c "$tag.px" > "$WORK/$tag.c.log" 2>&1 ); local rc2=$?
  if [ $rc2 -eq 0 ] && [ -x "$WORK/build/$tag" ]; then "$WORK/build/$tag" > "$WORK/$tag.c" 2> "$WORK/$tag.c.err"; else : > "$WORK/$tag.c"; fi
  rm -rf "$WORK/build"

  if [ $ri -eq 0 ] && cmp -s "$WORK/$tag.interp" "$D/expected_nested.txt"; then
    ok "A1 $tag 解释轨 == 期望（逐字节）"
  else
    bad "A1 $tag 解释轨不一致（rc=$ri）：$(diff "$D/expected_nested.txt" "$WORK/$tag.interp" 2>/dev/null | head -4 | tr '\n' ' ')"
  fi
  if [ $rb -eq 0 ] && cmp -s "$WORK/$tag.vm" "$D/expected_nested.txt"; then
    ok "A2 $tag VM 轨 == 期望"
  else
    bad "A2 $tag VM 轨不一致（rc=$rb）：$(head -2 "$WORK/$tag.vm.log" | tr '\n' ' ')"
  fi
  if [ $rc2 -eq 0 ] && cmp -s "$WORK/$tag.c" "$D/expected_nested.txt"; then
    ok "A3 $tag C 轨 == 期望"
  else
    bad "A3 $tag C 轨不一致（rc=$rc2）：$(head -2 "$WORK/$tag.c.log" | tr '\n' ' ')"
  fi
  if cmp -s "$WORK/$tag.interp" "$WORK/$tag.vm" && cmp -s "$WORK/$tag.vm" "$WORK/$tag.c"; then
    ok "A4 $tag 三轨逐字节一致"
  else
    bad "A4 $tag 三轨不一致：$(diff "$WORK/$tag.interp" "$WORK/$tag.vm" 2>/dev/null | head -3 | tr '\n' ' ')"
  fi
}

run_three "$D/nested.px" nested

# ---------- E：覆盖度自证（防止用例被改空） ----------
miss=0
while IFS= read -r pat; do
  grep -qF -- "$pat" "$D/nested.px" || { echo "    缺：$pat"; miss=$((miss+1)); }
done <<'PATS'
Circle(items: [float])
Poly(inner: Shape, tag: str)
Shape.Circle([1.0, 2.0])
case Shape.Circle(xs)
case Shape.Poly(inner, tg)
sorted(
< Shape.Circle([1.0, 3.0])
PATS
if [ $miss -eq 0 ]; then ok "E 覆盖度自证：nested.px 关键行齐（7/7）"; else bad "E 覆盖度：nested.px 缺 $miss 处关键行"; fi

# ---------- B：GC 根面 ----------
echo "  ·  B：GC 压测（PX_GC_THRESHOLD=5000 强制频繁回收 + LIVECHK + UAFDET）"
cp "$D/gcstress.px" "$WORK/gcstress.px"
rm -rf "$WORK/build"
if ( cd "$WORK" && "$PX" build --c gcstress.px > "$WORK/gc.build.log" 2>&1 ) && [ -x "$WORK/build/gcstress" ]; then
  GOUT="$( cd "$WORK" && PX_GC_STATS=1 PX_GC_THRESHOLD=5000 PX_GC_LIVECHK=1 PX_GC_UAFDET=1 \
           timeout 120 ./build/gcstress 2>&1 )"
  GRC=$?
  VAL="$(printf '%s\n' "$GOUT" | head -1)"
  RUNS="$(printf '%s\n' "$GOUT" | sed -n 's/.*runs=\([0-9]\+\).*/\1/p' | head -1)"

  if [ "$GRC" -eq 0 ] && [ "$VAL" = "200000" ]; then
    ok "B1 GC 压测输出正确（200000）"
  else
    bad "B1 GC 压测输出异常（rc=$GRC，首行=$VAL）"
  fi

  if printf '%s' "$GOUT" | grep -qE 'PX_GC_LIVECHK|PX_GC_UAFDET'; then
    bad "B2 检测到已回收对象/空闲链损坏（UAF）：$(printf '%s' "$GOUT" | grep -E 'PX_GC_LIVECHK|PX_GC_UAFDET' | head -2 | tr '\n' ' ')"
  else
    ok "B2 无 LIVECHK/UAFDET 报错（载荷对象图在回收压力下存活位正确）"
  fi

  if [ -n "${RUNS:-}" ] && [ "$RUNS" -gt 0 ]; then
    ok "B3 前提自证：GC 实跑 $RUNS 轮（>0，B2 不是「GC 没跑」的假绿）"
  else
    bad "B3 前提自证失败：GC 轮次 runs=${RUNS:-取不到} ⇒ B2 不成立"
  fi
else
  bad "B 压测编译失败：$(head -2 "$WORK/gc.build.log" 2>/dev/null | tr '\n' ' ')"
fi

# ---------- C：循环引用打印保护 ----------
cp "$D/cycle.px" "$WORK/cycle.px"
COUT="$( cd "$WORK" && timeout 15 "$PX" run cycle.px 2>&1 )"; CRC=$?
if [ $CRC -eq 0 ] && printf '%s' "$COUT" | grep -q '\.\.\.'; then
  ok "C 循环引用打印有保护（输出含 ... ，未无限展开）"
elif [ $CRC -eq 124 ]; then
  bad "C 循环引用打印**挂死**（15s 超时）"
else
  bad "C 循环引用打印异常（rc=$CRC）：$(printf '%s' "$COUT" | head -2 | tr '\n' ' ')"
fi

# ---------- D：不支持的复合模式给明确错误 ----------
cp "$D/badpat.px" "$WORK/badpat.px"
BOUT="$( cd "$WORK" && timeout 20 "$PX" run badpat.px 2>&1 )"; BRC=$?
if [ $BRC -ne 0 ] && printf '%s' "$BOUT" | grep -qE 'E2001'; then
  ok "D list 字面量模式给明确错误（E2001，未静默不匹配）"
else
  bad "D list 字面量模式未按预期报错（rc=$BRC）：$(printf '%s' "$BOUT" | head -2 | tr '\n' ' ')"
fi
note "已知限制（spec §5.4 未承诺）：list / dict 字面量作模式不支持；tuple 字面量支持。本门只钉「必须有明确错误」。"

# ---------- 负控 ----------
if [ "$NEG_SKIP" = "0" ]; then
  echo "  ·  NC1 篡改 expected 副本一行 ⇒ A 必须红"
  cp "$D/expected_nested.txt" "$WORK/exp.bak"
  sed '3s/.*/TAMPERED-BY-NC1/' "$D/expected_nested.txt" > "$WORK/exp.tampered"
  if cmp -s "$WORK/exp.bak" "$WORK/exp.tampered"; then
    bad "NC1 前提自证失败：篡改没生效（文件内容相同）"
  elif cmp -s "$WORK/nested.interp" "$WORK/exp.tampered"; then
    bad "NC1 篡改后期望仍相等 ⇒ A 判据无牙"
  else
    ok "NC1 篡改 expected 后必不相等（A 有牙，且自证篡改真发生）"
  fi

  echo "  ·  NC2 PX_GC_THRESHOLD 极大 ⇒ runs==0 ⇒ B3 必须红"
  if [ -x "$WORK/build/gcstress" ]; then :; else
    rm -rf "$WORK/build"
    ( cd "$WORK" && "$PX" build --c gcstress.px >/dev/null 2>&1 )
  fi
  NOUT="$( cd "$WORK" && PX_GC_STATS=1 PX_GC_THRESHOLD=2000000000 timeout 60 ./build/gcstress 2>&1 )"
  NRUNS="$(printf '%s' "$NOUT" | sed -n 's/.*runs=\([0-9]\+\).*/\1/p' | head -1)"
  if [ "${NRUNS:-x}" = "0" ]; then
    ok "NC2 阈值拉高后 runs=0（B3 的前提自证有牙：GC 没跑就不会被算绿）"
  else
    bad "NC2 阈值拉高后 runs=${NRUNS:-取不到}（预期 0）—— 前提自证失效，B2/B3 的可信度存疑"
  fi
  rm -rf "$WORK/build"
else
  note "负控已跳过（--neg-skip）"
fi

echo
echo "════════════════════════"
if [ $FAIL -eq 0 ]; then
  echo "✅ M373 复合载荷门全绿：PASS=$PASS FAIL=0"
  exit 0
else
  echo "❌ M373 复合载荷门有红：PASS=$PASS FAIL=$FAIL"
  exit 1
fi
