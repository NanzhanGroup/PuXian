#!/usr/bin/env bash
# ============================================================
# examples/m369_name_shadow/verify.sh —— 名字解析优先级：变量 > 类型名（M369 · 缺陷 635）
# ------------------------------------------------------------
# 为什么有这道门（M369 实测三轨分叉，M366 之前就存在）：
#   enum Color: Red
#   def main():
#     let Color = {"Red": 7}
#     print(Color.Red)          # 解释轨 7 · VM/C 轨 Color.Red（枚举值）
#
#   enum Shape: Circle
#   struct Box: n: int
#   impl Box:
#     def Circle(self, k: int) -> int: return k + 100
#   def main():
#     let Shape = Box(1)
#     print(Shape.Circle(2))    # 解释轨 102 · VM/C 轨 R1005
#
# 根因：解释轨把类型名当**绑定**放进 env（i_exec_* → env_define/it_typeref），
#   局部变量遮蔽它 ⇒ 值优先；编译两轨按名字查类型表（g_bcm["enums"] / cg_enums）先命中
#   ⇒ 同名变量被整段跳过。
#
# 定稿口径（写进 spec §17.12）：**同一作用域内，变量绑定（帧局部或模块级）优先于类型名**，
#   与声明先后无关；该作用域是**函数级**（spec §17.8 的 Python 式作用域，块不新建作用域）。
#   ⇒ 三处同时改：VM 轨 bc_emit 的 Field / Call-Field 守卫、C 轨 cg_expr 同两处、
#     解释轨类型名绑定不覆盖已有值绑定（istmt.px i_bind_typeref）。
#
# 判据：
#   A 三轨逐字节一致（6 正控：局部遮蔽·方法调用 / 局部遮蔽·字段 / 模块级遮蔽 / 块内声明块外使用 /
#     函数参数遮蔽 / 先变量后同名类型）
#   B 与门侧独立参考对拍（期望值门内写死，不抄实现）
#   C 错误面三轨同码同文（R1001：函数内「先用后声明」，验 CHKINIT 与解释轨 unbound 同口径）
#   D 回归：无遮蔽时枚举变体照旧、且在**另一个函数**里仍能取到枚举（防「修一个坏一个」）
#   E 覆盖度自证（正向 ≥6 组 / 错误 ≥1 类 / 必须同时含「变量胜」与「枚举仍可用」两族）
#
# 负控（证明判据有鉴别力）：
#   NC-expect  把 p2 的期望值改一个数            ⇒ B 必须红
#   NC-invert  把 A 判据反向（要求三轨不一致）    ⇒ A 必须红
#   NC-empty   把用例表清空                      ⇒ E 覆盖度必须红（防门变恒绿）
#
# 用法：verify.sh [--neg-skip]
# 退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m369_name_shadow.$$"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
NC_MODE="${M369_NC:-}"

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK"
[ -x "$PX" ] || { echo "❌ 找不到 $PX"; exit 1; }

run_lanes() {  # $1=prog $2=前缀 → $2.run/$2.vm/$2.c · $2.{run,vm,c}.err · $2.rc
    local prog="$1" pfx="$2" d name
    d="$(dirname "$prog")"; name="$(basename "$prog" .px)"
    timeout 60 "$PX" run "$prog" > "$pfx.run" 2>"$pfx.run.err"; local rc_run=$?
    rm -rf "$d/build"
    timeout 200 "$PX" build "$prog" > "$pfx.vmlog" 2>&1; local rc_vm=$?
    if [ $rc_vm -eq 0 ] && [ -x "$d/build/$name" ]; then timeout 60 "$d/build/$name" > "$pfx.vm" 2>"$pfx.vm.err"; else : > "$pfx.vm"; fi
    rm -rf "$d/build"
    timeout 300 "$PX" build --c "$prog" > "$pfx.clog" 2>&1; local rc_c=$?
    if [ $rc_c -eq 0 ] && [ -x "$d/build/$name" ]; then timeout 60 "$d/build/$name" > "$pfx.c" 2>"$pfx.c.err"; else : > "$pfx.c"; fi
    rm -rf "$d/build"
    printf '%s %s %s\n' "$rc_run" "$rc_vm" "$rc_c" > "$pfx.rc"
}

# 归一化首条诊断：解释轨 `运行时错误: 错误 [R1001] 5:9: 未定义变量: 'Color'`
#               编译两轨 `运行时错误 [main 行6]: R1001: 未定义变量: 'Color'`
norm_err() {
    sed -e 's/^运行时错误: 错误 \[\([A-Z][0-9]*\)\] [0-9]*:[0-9]*: /\1|/' \
        -e 's/^运行时错误 \[[^]]*\]: \([A-Z][0-9]*\): /\1|/' \
        -e 's/^[0-9]*:[0-9]*: 语法错误 \([A-Z][0-9]*\): /\1|/' "$1" 2>/dev/null \
        | grep -E '^[A-Z][0-9]*\|' | head -1
}

echo "═══ M369 · 名字解析优先级（变量 > 类型名）三轨对拍门 ═══"

# ── A/B：正向六形态 ────────────────────────────────────────
POS_SPEC=()
if [ "$NC_MODE" != "empty" ]; then
POS_SPEC=(
  'p1|局部变量遮蔽枚举名 + 方法调用|enum Shape:
  Circle
struct Box:
  n: int
impl Box:
  def Circle(self, k: int) -> int:
    return k + 100
def main():
  let Shape = Box(1)
  print(Shape.Circle(2))
|102'
  'p2|局部变量遮蔽枚举名 + 字段访问|enum Color:
  Red
def main():
  let Color = {"Red": 7}
  print(Color.Red)
|7'
  'p3|模块级变量遮蔽枚举名|enum Color:
  Red
let Color = {"Red": 9}
def main():
  print(Color.Red)
|9'
  'p4|块内声明、块外使用（函数级作用域）|enum Color:
  Red
def main():
  if true:
    let Color = {"Red": 7}
  print(Color.Red)
|7'
  'p5|函数参数遮蔽枚举名|enum Color:
  Red
def show(Color: dict) -> int:
  return Color.Red
def main():
  print(show({"Red": 5}))
|5'
  'p6|先变量后同名类型（后置类型不覆盖变量）|let Color = {"Red": 9}
enum Color:
  Red
def main():
  print(Color.Red)
|9'
)
fi
NP=${#POS_SPEC[@]}
for spec in "${POS_SPEC[@]}"; do
    pname="${spec%%|*}"; rest="${spec#*|}"; pdesc="${rest%%|*}"; rest="${rest#*|}"
    pbody="${rest%|*}"; pexp="${rest##*|}"
    f="$WORK/$pname.px"; pfx="$WORK/$pname"
    printf '%s\n' "$pbody" > "$f"
    [ "$NC_MODE" = "expect" ] && [ "$pname" = "p2" ] && pexp="8"
    run_lanes "$f" "$pfx"
    read -r RC_RUN RC_VM RC_C < "$pfx.rc"
    if [ "$RC_RUN" -ne 0 ] || [ "$RC_VM" -ne 0 ] || [ "$RC_C" -ne 0 ]; then
        bad "A-$pname ${pdesc}：轨道非 0（run=$RC_RUN vm=$RC_VM c=$RC_C）"
    elif [ "$NC_MODE" = "invert" ] && [ "$pname" = "p1" ]; then
        # NC-invert：把判据**反向**（要求不一致才 ok）⇒ 必须打红，证明 A 不是恒绿
        if cmp -s "$pfx.run" "$pfx.vm" && cmp -s "$pfx.run" "$pfx.c"; then
            bad "A-$pname 反向断言：期望三轨**不一致**，实际一致"
        else
            ok "A-$pname 三轨不一致（反向断言通过）"
        fi
    elif cmp -s "$pfx.run" "$pfx.vm" && cmp -s "$pfx.run" "$pfx.c"; then
        ok "A-$pname ${pdesc} 三轨逐字节一致（$(tr -d '\n' < "$pfx.run")）"
    else
        bad "A-$pname ${pdesc} 三轨不一致：run=[$(tr -d '\n' < "$pfx.run")] vm=[$(tr -d '\n' < "$pfx.vm")] c=[$(tr -d '\n' < "$pfx.c")]"
    fi
    for lane in run vm c; do
        got="$(tr -d '\n' < "$pfx.$lane")"
        if [ "$got" = "$pexp" ]; then ok "B-$pname-$lane 与门侧参考一致（$pexp）"
        else bad "B-$pname-$lane 期望 $pexp，实得 $got"; fi
    done
done

# ── C：错误面（函数内先用后声明 ⇒ 三轨同码同文 R1001）──────────
f="$WORK/e1.px"; pfx="$WORK/e1"
cat > "$f" <<'EOF'
enum Color:
  Red
def main():
  print(Color.Red)
  let Color = {"Red": 7}
EOF
run_lanes "$f" "$pfx"
r1="$(norm_err "$pfx.run.err")"; r2="$(norm_err "$pfx.vm.err")"; r3="$(norm_err "$pfx.c.err")"
[ -z "$r2" ] && r2="$(norm_err "$pfx.vmlog")"
[ -z "$r3" ] && r3="$(norm_err "$pfx.clog")"
if [ -z "$r1" ]; then bad "C-e1 解释轨无诊断（应报 R1001）"
elif [ "$r1" = "$r2" ] && [ "$r1" = "$r3" ]; then ok "C-e1 三轨同码同文（$r1）"
else bad "C-e1 三轨不一致：run=[$r1] vm=[$r2] c=[$r3]"; fi
if [ "${r1%%|*}" = "R1001" ] && [[ "$r1" == *"未定义变量"* ]] && [[ "$r1" == *"Color"* ]]; then
    ok "C-e1 码与关键词齐（R1001 / 未定义变量 / Color）"
else
    bad "C-e1 码或关键词不符：期望 R1001 / 未定义变量 / Color，实得「$r1」"
fi

# ── D：回归（无遮蔽时枚举照旧，且跨函数不串味）──────────────
f="$WORK/d1.px"; pfx="$WORK/d1"
cat > "$f" <<'EOF'
enum Color:
  Red
def shadowed() -> int:
  let Color = {"Red": 3}
  return Color.Red
def plain():
  print(Color.Red)
def main():
  print(shadowed())
  plain()
EOF
run_lanes "$f" "$pfx"
if cmp -s "$pfx.run" "$pfx.vm" && cmp -s "$pfx.run" "$pfx.c" \
   && [ "$(tr -d '\n' < "$pfx.run")" = "3Color.Red" ]; then
    ok "D1 遮蔽只作用于声明它的帧：shadowed=3 · plain=Color.Red（三轨一致）"
else
    bad "D1 跨函数回归失败：run=[$(tr -d '\n' < "$pfx.run")] vm=[$(tr -d '\n' < "$pfx.vm")] c=[$(tr -d '\n' < "$pfx.c")]（期望 3Color.Red）"
fi

# ── E：覆盖度自证（防空转）──────────────────────────────────
[ "$NP" -ge 6 ] && ok "E1 正向形态 $NP 组（≥6）" || bad "E1 正向形态只有 $NP 组（≥6 才够覆盖遮蔽面）"
ALL_SPEC="$(printf '%s\n' "${POS_SPEC[@]}")"
if printf '%s' "$ALL_SPEC" | grep -q 'p1' && printf '%s' "$ALL_SPEC" | grep -q 'p2' \
   && printf '%s' "$ALL_SPEC" | grep -q 'p3' && printf '%s' "$ALL_SPEC" | grep -q 'p6'; then
    ok "E2 「变量胜」四族齐（局部/方法/模块级/后置类型）"
else
    bad "E2 「变量胜」用例退化（防用例被改空后门变恒绿）"
fi
if grep -q 'plain()' "$WORK/d1.px" 2>/dev/null; then
    ok "E3 「枚举仍可用」回归族在位"
else
    bad "E3 回归族缺失"
fi

# ── 负控 ────────────────────────────────────────────────────
if [ "$NEG_SKIP" -eq 0 ] && [ -z "$NC_MODE" ]; then
    for pair in "expect:B" "invert:A" "empty:E"; do
        m="${pair%%:*}"; judge="${pair##*:}"
        out="$(M369_NC="$m" bash "$0" --neg-skip 2>&1 || true)"
        if grep -q "❌ $judge" <<< "$out"; then
            ok "NC-$m 扰动后 $judge 判据如实红"
        else
            bad "NC-$m 扰动后 $judge 没红（判据无鉴别力）"
        fi
    done
else
    note "负控跳过（--neg-skip 或负控子进程）"
fi

echo "───────────────────────────────────────────────"
# 末行不得含 ❌（M293 · 缺陷 506：GATE_BAD_LAST_RE 会把它记成红，rc=0 也判红）
echo "M369 门结果：PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
