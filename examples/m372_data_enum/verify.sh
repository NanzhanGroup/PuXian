#!/usr/bin/env bash
# ============================================================
# examples/m372_data_enum/verify.sh —— M372/S3：data enum 载荷三轨一致门
# ------------------------------------------------------------
# 为什么有这道门（spec §3.8 唯一「写了没兑现」的语义，缺陷 364 的后半）：
#   M367/S1 只把 `Circle(radius: float)` 的**语法**收进 AST（EnumField），载荷语义留给 S2/S3。
#   S2 落了构造与 parser；S3 落**载荷值**：构造带值、match 解构绑定、
#   载荷参与相等/比较/排序、打印形态确定。
#   三轨（解释器 / C / VM）必须**逐字节一致** —— 这是本仓对「同一程序三个后端」的硬契约。
#
# ⚠ 前置：本门跑的是 `tools/px`（bootstrap 烘死的编译器）。
#   S3 的语义只存在于源码里 ⇒ **必须先 `selfhost/rebake_bin.sh`（S4）**再跑本门。
#   未重烘时本门会红在「C 轨仍拒 R2005」上 —— 那是**预期**，不是回归（S4 的职责是重烘）。
#
# 判据：
#   A 解释轨 main.px 输出 == expected_main.txt（逐字节）
#   B VM 轨   main.px 输出 == expected_main.txt
#   C C  轨   main.px 输出 == expected_main.txt（三轨一致 = A==B==C）
#   D 解释轨 value.px 输出 == expected_value.txt
#   E VM 轨   value.px 输出 == expected_value.txt
#   F C  轨   value.px 输出 == expected_value.txt
#   G 错误格（三道，载荷契约的「不许静默」面）：
#       G1 载荷给少 ⇒ rc≠0 且 R2001「需要 1 个字段，给出 0」
#       G2 载荷给多 ⇒ rc≠0 且 R2001「需要 1 个字段，给出 2」
#       G3 无载荷变体给参数 ⇒ rc≠0 且 R1005「不接受参数」
#   H 旧写法不回归：Vec.V（无括号无载荷）与 Vec.V(1.5) 都成立且相等性正确
#   I 覆盖度自证：用例文件没被改空（关键行必须在）
#
# 负控（证明判据有牙）：
#   NC1 篡改 expected 副本一行 ⇒ A/B/C 必须红（并自证篡改真发生）
#   NC2 把载荷给少的程序当「应通过」喂给 A 判据 ⇒ 必须红（错误格鉴別力）
#
# 用法：verify.sh [--neg-skip]     退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m372_data_enum.$$"
NEG_SKIP=0; [ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK"

[ -x "$PX" ] || { echo "❌ 找不到 $PX（先跑 selfhost/rebake_bin.sh）"; exit 1; }
D="$ROOT/examples/m372_data_enum"
for f in main.px value.px expected_main.txt expected_value.txt; do
  [ -r "$D/$f" ] || { echo "❌ 缺 $f"; exit 1; }
done

echo "═══ M372/S3 · data enum 载荷三轨一致门 ═══"

# ---------- 三轨跑一个源文件 ----------
# ⚠ 必须在 $WORK 内用**相对路径** build：px build 把产物放在**源文件同目录**的 build/，
#   用绝对路径会把 build/ 扔进 examples/（实测 22:15 首次跑就是这么"全空"的 —— 判据读错通道）。
run_three() {
  local src="$1" tag="$2" exp="$3"
  cp "$src" "$WORK/$tag.px"
  local base="$tag"
  # 解释轨
  ( cd "$WORK" && "$PX" run "$base.px" > "$WORK/$tag.interp" 2> "$WORK/$tag.interp.err" ); local ri=$?
  # VM 轨
  rm -rf "$WORK/build"
  ( cd "$WORK" && "$PX" build "$base.px" > "$WORK/$tag.build.log" 2>&1 ); local rb=$?
  if [ $rb -eq 0 ] && [ -x "$WORK/build/$base" ]; then
    "$WORK/build/$base" > "$WORK/$tag.vm" 2> "$WORK/$tag.vm.err"
  else
    : > "$WORK/$tag.vm"
  fi
  # C 轨
  rm -rf "$WORK/build"
  ( cd "$WORK" && "$PX" build --c "$base.px" > "$WORK/$tag.cbuild.log" 2>&1 ); local rc2=$?
  if [ $rc2 -eq 0 ] && [ -x "$WORK/build/$base" ]; then
    "$WORK/build/$base" > "$WORK/$tag.c" 2> "$WORK/$tag.c.err"
  else
    : > "$WORK/$tag.c"
  fi
  rm -rf "$WORK/build"

  # A/B/C 或 D/E/F
  if [ $ri -eq 0 ] && cmp -s "$WORK/$tag.interp" "$exp"; then ok "$tag 解释轨 == 期望（逐字节）"
  else bad "$tag 解释轨不一致（rc=$ri）：$(diff "$exp" "$WORK/$tag.interp" 2>/dev/null | head -4 | tr '\n' ' ')"; fi

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

run_three "$D/main.px"  main  "$D/expected_main.txt"
run_three "$D/value.px" value "$D/expected_value.txt"

# ---------- G 错误格：载荷契约不许静默 ----------
neg_case() {   # $1=名 $2=源码 $3=期望错误码 $4=期望片段
  local n="$1" body="$2" code="$3" frag="$4"
  printf '%s\n' "$body" > "$WORK/neg_$n.px"
  "$PX" run "$WORK/neg_$n.px" > /dev/null 2> "$WORK/neg_$n.err"; local r=$?
  if [ $r -ne 0 ] && grep -q "$code" "$WORK/neg_$n.err" && grep -q "$frag" "$WORK/neg_$n.err"; then
    ok "G/$n rc≠0 且 $code 含「$frag」"
  else
    bad "G/$n 未按契约报错（rc=$r）：$(head -1 "$WORK/neg_$n.err")"
  fi
}

HDR='enum Shape:
    Circle(radius: float)
    Dot'
neg_case few "$HDR
def main():
    print(Shape.Circle())"                 R2001 "给出 0"
neg_case many "$HDR
def main():
    print(Shape.Circle(1.0, 2.0))"         R2001 "给出 2"
neg_case nodot "$HDR
def main():
    print(Shape.Dot(1))"                   R1005 "不接受参数"

# ---------- H 旧写法不回归（位置写法 + 无载荷变体） ----------
cat > "$WORK/legacy.px" <<'EOF'
enum Vec:
    V(float)
    W
def main():
    let a = Vec.V
    let b = Vec.V(1.5)
    print(a)
    print(b)
    print(Vec.V == a)
    print(Vec.V(1.5) == b)
EOF
"$PX" run "$WORK/legacy.px" > "$WORK/legacy.out" 2> "$WORK/legacy.err"; rl=$?
EXP_L='Vec.V
Vec.V({0: 1.5})
true
true'
if [ $rl -eq 0 ] && [ "$(cat "$WORK/legacy.out")" = "$EXP_L" ]; then
  ok "H 旧写法（位置载荷 + 无载荷变体）不回归"
else
  bad "H 旧写法回归（rc=$rl）：$(head -2 "$WORK/legacy.err" | tr '\n' ' ')"
fi

# ---------- I 覆盖度自证（防用例被改空后门恒绿） ----------
cov=0
grep -q 'match s:'                       "$D/main.px" && cov=$((cov+1))
grep -q 'case Shape.Circle(2.0):'        "$D/main.px" && cov=$((cov+1))
grep -q 'case Shape.Circle(_):'          "$D/main.px" && cov=$((cov+1))
grep -q 'Shape.Circle(1.0) != Shape.Circle(2.0)' "$D/value.px" && cov=$((cov+1))
grep -q 'sorted(\[Shape.Circle(2.0), Shape.Circle(1.0)\])' "$D/value.px" && cov=$((cov+1))
[ "$cov" -ge 5 ] && ok "I 覆盖度自证：5 个关键判据点都在用例里" || bad "I 用例被改空（命中 $cov/5）"

# ---------- 负控 ----------
if [ "$NEG_SKIP" = 1 ]; then
  note "负控跳过（--neg-skip）"
else
  echo "--- 负控 ---"
  # NC1：篡改期望副本 ⇒ 逐字节判据必须红
  cp "$D/expected_main.txt" "$WORK/xm.txt"
  sed -i 's/^12\.0$/12.5/' "$WORK/xm.txt"
  if cmp -s "$WORK/xm.txt" "$D/expected_main.txt"; then
    bad "NC1 前提不成立：篡改没生效"
  else
    note "NC1 前提自证：$(diff "$D/expected_main.txt" "$WORK/xm.txt" | grep '^[<>]' | tr '\n' ' ')"
    "$PX" run "$D/main.px" > "$WORK/nc1.out" 2>/dev/null
    if cmp -s "$WORK/nc1.out" "$WORK/xm.txt"; then
      bad "NC1 篡改期望后判据仍绿 ⇒ 判据无鉴别力"
    else
      ok "NC1 篡改期望一行 ⇒ 判据变红（有牙）"
    fi
  fi
  # NC2：把「载荷给少」的程序当应通过喂给同一比对 ⇒ 必须红
  "$PX" run "$WORK/neg_few.px" > "$WORK/nc2.out" 2>/dev/null || true
  if cmp -s "$WORK/nc2.out" "$D/expected_main.txt"; then
    bad "NC2 错误程序竟与期望一致 ⇒ 错误格无鉴别力"
  else
    ok "NC2 载荷给少的程序 ≠ 期望（错误格有牙）"
  fi
fi

echo
echo "PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = 0 ]
