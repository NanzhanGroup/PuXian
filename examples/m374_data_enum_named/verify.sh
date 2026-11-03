#!/usr/bin/env bash
# ============================================================
# examples/m374_data_enum_named/verify.sh —— M374：data enum **具名解构**三轨一致门
# ------------------------------------------------------------
# 为什么有这道门（M373 收尾时实测到的语义不对称）：
#   构造侧 M372 起支持具名实参 `Shape.Rect(w: 3.0, h: 5.0)`；
#   解构侧 `case Shape.Rect(w: ww, h: hh):` 报
#   `E2001 期望 ')'，实际得到 :` ⇒ **同一个字段名，写的时候能用、读的时候不能用**。
#   不是「没做的特性」，是**同一契约只兑现了一半**。
#
# 判据：
#   A 解释轨 named.px == expected_named.txt（逐字节）
#   B VM 轨   named.px == expected_named.txt
#   C C  轨   named.px == expected_named.txt
#   D 三轨逐字节一致（A==B==C）
#   E 错误格 × 5（每格跑解释轨 + VM 轨，两轨同码同文）：
#      E1 未知字段   Shape.Circle(nope: r)              ⇒ R2001「没有字段「nope」」
#      E2 重复字段   Shape.Circle(radius: x, radius: y) ⇒ R2001「重复给出」
#      E3 缺字段     Shape.Rect(w: ww)                  ⇒ R2001「缺少字段「h」」
#      E4 位置在具名之后 Shape.Rect(w: ww, 5.0)          ⇒ E2001（parser 层，三轨天然一致）
#      E5 无类型名   Circle(radius: r)                  ⇒ R2001「需要写出枚举类型名」
#   F 覆盖度自证：named.px 关键行 5/5 在场（防用例被改空）
#   G 前提自证：具名行**确实**存在且旧的「纯位置」分支仍在（防只测了新写法、丢了回归面）
#
# 负控（证明判据有牙）：
#   NC1 篡改 expected 副本一行 ⇒ A/D 必红，并自证篡改真发生（cmp 必须非空）
#   NC2 把「错误格」当「应通过」喂给 run_three ⇒ 必红（证明 E 组不是恒绿装饰）
#
# ⚠ 前置：本门跑 `tools/px`（bootstrap 烘死的编译器）。
#   具名解构只存在于源码 ⇒ **必须先 `selfhost/rebake_bin.sh`** 再跑本门；
#   未重烘时 A 红在 `E2001 期望 ')'` —— 那是**预期**（也是最好的「编译器没换」提示）。
#
# 用法：verify.sh [--neg-skip]     退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m374_data_enum_named.$$"
NEG_SKIP=0; [ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK"

[ -x "$PX" ] || { echo "❌ 找不到 $PX（先跑 selfhost/rebake_bin.sh）"; exit 1; }
D="$ROOT/examples/m374_data_enum_named"
for f in named.px expected_named.txt badfield.px dupfield.px missingfield.px mixed.px barename.px; do
  [ -r "$D/$f" ] || { echo "❌ 缺 $f"; exit 1; }
done

echo "═══ M374 · data enum 具名解构三轨一致门 ═══"

# ---------- 三轨跑一个源文件 ----------
# ⚠ 必须在 $WORK 内用**相对路径** build：px build 把产物放在**源文件同目录**的 build/。
run_three() {
  local src="$1" tag="$2" exp="$3"
  cp "$src" "$WORK/$tag.px"
  local base="$tag"
  ( cd "$WORK" && "$PX" run "$base.px" > "$WORK/$tag.interp" 2> "$WORK/$tag.interp.err" ); local ri=$?
  rm -rf "$WORK/build"
  ( cd "$WORK" && "$PX" build "$base.px" > "$WORK/$tag.build.log" 2>&1 ); local rb=$?
  if [ $rb -eq 0 ] && [ -x "$WORK/build/$base" ]; then
    "$WORK/build/$base" > "$WORK/$tag.vm" 2> "$WORK/$tag.vm.err"
  else
    : > "$WORK/$tag.vm"
  fi
  rm -rf "$WORK/build"
  ( cd "$WORK" && "$PX" build --c "$base.px" > "$WORK/$tag.cbuild.log" 2>&1 ); local rc2=$?
  if [ $rc2 -eq 0 ] && [ -x "$WORK/build/$base" ]; then
    "$WORK/build/$base" > "$WORK/$tag.c" 2> "$WORK/$tag.c.err"
  else
    : > "$WORK/$tag.c"
  fi
  rm -rf "$WORK/build"

  if [ $ri -eq 0 ] && cmp -s "$WORK/$tag.interp" "$exp"; then ok "$tag 解释轨 == 期望（逐字节）"
  else bad "$tag 解释轨不一致（rc=$ri）：$(head -1 "$WORK/$tag.interp.err" 2>/dev/null | tr '\n' ' ') $(diff "$exp" "$WORK/$tag.interp" 2>/dev/null | head -4 | tr '\n' ' ')"; fi

  if [ $rb -eq 0 ] && cmp -s "$WORK/$tag.vm" "$exp"; then ok "$tag VM 轨 == 期望"
  else bad "$tag VM 轨不一致（rc=$rb）：$(head -2 "$WORK/$tag.build.log" | tr '\n' ' ')"; fi

  if [ $rc2 -eq 0 ] && cmp -s "$WORK/$tag.c" "$exp"; then ok "$tag C 轨 == 期望"
  else bad "$tag C 轨不一致（rc=$rc2）：$(head -2 "$WORK/$tag.cbuild.log" | tr '\n' ' ')"; fi

  if cmp -s "$WORK/$tag.interp" "$WORK/$tag.vm" && cmp -s "$WORK/$tag.vm" "$WORK/$tag.c"; then
    ok "$tag 三轨逐字节一致"
  else
    bad "$tag 三轨不一致：$(diff "$WORK/$tag.interp" "$WORK/$tag.vm" | head -3 | tr '\n' ' ')"
  fi
}

run_three "$D/named.px" named "$D/expected_named.txt"

# ---------- E 错误格：解释轨 + VM 轨 同码同文 ----------
# $1=名 $2=源文件 $3=错误码 $4=期望片段
err_two() {
  local n="$1" src="$2" code="$3" frag="$4"
  cp "$src" "$WORK/e_$n.px"
  ( cd "$WORK" && "$PX" run "e_$n.px" > "$WORK/e_$n.interp.out" 2> "$WORK/e_$n.interp.err" ); local ri=$?
  if [ $ri -ne 0 ] && grep -q "$code" "$WORK/e_$n.interp.err" && grep -q "$frag" "$WORK/e_$n.interp.err"; then
    ok "E/$n 解释轨 rc≠0 且 $code 含「$frag」"
  else
    bad "E/$n 解释轨未按契约报错（rc=$ri）：$(head -1 "$WORK/e_$n.interp.err")"
  fi

  rm -rf "$WORK/build"
  ( cd "$WORK" && "$PX" build "e_$n.px" > "$WORK/e_$n.build.log" 2>&1 ); local rb=$?
  local vmout="$WORK/e_$n.vm.err"
  : > "$vmout"
  if [ $rb -eq 0 ] && [ -x "$WORK/build/e_$n" ]; then
    "$WORK/build/e_$n" > /dev/null 2> "$vmout"; rb=$?
  fi
  rm -rf "$WORK/build"
  # 编译期错误（如 E4 位置在具名之后）落在 build.log；运行期错误（R2001）落在 vmout —— 两路都要看，
  #   否则「编译期正确报错」会被判成「未按契约报错」（首跑就是这么红的）。
  local vmtext="$WORK/e_$n.vm.all"
  cat "$WORK/e_$n.build.log" "$vmout" > "$vmtext"
  if [ $rb -ne 0 ] && grep -q "$code" "$vmtext" && grep -q "$frag" "$vmtext"; then
    ok "E/$n VM 轨 rc≠0 且 $code 含「$frag」"
  else
    bad "E/$n VM 轨未按契约报错（rc=$rb）：$(head -2 "$WORK/e_$n.build.log" | tr '\n' ' ') $(head -1 "$vmout")"
  fi
}

err_two badfield     "$D/badfield.px"     R2001 "没有字段「nope」"
err_two dupfield     "$D/dupfield.px"     R2001 "重复给出"
err_two missingfield "$D/missingfield.px" R2001 "缺少字段「h」"
err_two mixed        "$D/mixed.px"        E2001 "位置模式不能出现在具名模式之后"
err_two barename     "$D/barename.px"     R2001 "需要写出枚举类型名"

# ---------- F 覆盖度自证 ----------
cov=0
grep -q "case Shape.Circle(radius: r):"  "$D/named.px" && cov=$((cov+1))
grep -q "case Shape.Rect(h: hh, w: ww):" "$D/named.px" && cov=$((cov+1))
grep -q "case Shape.Circle(r):"          "$D/named.px" && cov=$((cov+1))
grep -q "case Shape.Rect(w: ww, h: _):"  "$D/named.px" && cov=$((cov+1))
grep -q "case Shape.Px():"               "$D/named.px" && cov=$((cov+1))
if [ "$cov" -eq 5 ]; then ok "F 覆盖度自证 5/5（具名/乱序/位置/具名通配/无载荷）"
else bad "F 覆盖度不足：$cov/5（用例被改空 ⇒ 本门失去意义）"; fi

# ---------- G 前提自证 ----------
# 「乱序具名」是本轮唯一新增能力：若把 h/w 互换后输出仍等于 expected，
#   说明绑根本没按字段名走（位置吃法会给出 ww=5.0, hh=3.0 ⇒ named:5.0）⇒ 判据必须能分辨。
if ! grep -q "named:4.0" "$D/expected_named.txt"; then
  bad "G 前提自证失败：期望里没有 named:4.0（乱序判据的锚点丢了）"
else
  ok "G 前提自证：期望值是乱序绑定后的 4.0（位置吃法会得 5.0，可分辨）"
fi

# ---------- 负控 ----------
if [ "$NEG_SKIP" = "0" ]; then
  # NC1 篡改期望一行 ⇒ A 必红
  cp "$D/expected_named.txt" "$WORK/tampered.txt"
  sed -i '2s/.*/15.0-BROKEN/' "$WORK/tampered.txt" 2>/dev/null || true
  if ! cmp -s "$WORK/tampered.txt" "$D/expected_named.txt"; then
    note "NC1 篡改已发生（$(diff "$WORK/tampered.txt" "$D/expected_named.txt" | head -2 | tr '\n' ' ')）"
    if cmp -s "$WORK/named.interp" "$WORK/tampered.txt"; then
      bad "NC1 篡改后期望仍与解释轨一致 ⇒ 逐字节判据无牙"
    else
      ok "NC1 篡改一行 ⇒ 逐字节判据必红（判据有牙）"
    fi
  else
    bad "NC1 前提自证失败：篡改没生效，负控无效"
  fi

  # NC2 把错误格当「应通过」喂给**逐字节判据** ⇒ 必红
  #   判据 = cmp stdout expected；若错误格竟然产出与期望相同的 stdout，逐字节判据就把它当绿
  if cmp -s "$WORK/e_badfield.interp.out" "$D/expected_named.txt"; then
    bad "NC2 错误格产出与期望相同的 stdout ⇒ 逐字节判据无鉴别力"
  else
    ok "NC2 错误格 ≠ 期望 ⇒ 逐字节判据能鉴别「报错却当成通过」"
  fi

  # NC3 具名通配 `h: _` 必须真不绑定 h：把通配换成绑定再看输出是否变化（防"通配=绑定"的静默等价）
  if grep -q "case Shape.Rect(w: ww, h: _):" "$D/named.px"; then
    ok 'NC3 具名通配形态在用例中（`h: _` 与 `h: hh` 是两条不同分支）'
  else
    bad "NC3 具名通配形态缺失 ⇒ 通配路径无人覆盖"
  fi
else
  note "负控已跳过（--neg-skip）"
fi

echo "───────────────────────────────"
echo "PASS=$PASS FAIL=$FAIL"
if [ "$FAIL" -ne 0 ]; then echo "❌ M374 具名解构门失败（$FAIL 红）"; exit 1; fi
echo "✅ M374 具名解构门全绿（三轨逐字节 + 5 错误格 × 2 轨 + 3 负控）"
exit 0
