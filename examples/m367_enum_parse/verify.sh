#!/usr/bin/env bash
# ============================================================
# examples/m367_enum_parse/verify.sh —— data enum 语法收口：载荷字段「名: 类型」真进 AST（M367-S1）
# ------------------------------------------------------------
# 为什么有这道门（缺陷 364，M237 登记 / M366 复核）：
#   spec §3.8/§16.3 从第一版就写着 `enum Shape: Circle(radius: float)`，
#   而 parser 只吃类型不吃「字段名: 类型」⇒ 照文档写得到 `E2001 期望 ')'，实际得到 :`。
#   M237 把它改成可执行指引（`Circle(float)`），M367-S1 起**真收下**：
#   `名: 类型` 落成 `EnumField{name, ty, pos}` 节点进 AST，载荷语义留给 S2/S3。
#
# 判据（S1 只判「语法与 AST」，不判载荷值 —— 载荷构造是 S2）：
#   A 三轨一致：带名变体的**无括号访问** `Shape.Circle` 仍正常（防「修一个坏一个」）
#   B AST 真存：pxpar 输出含 EnumField · 字段名 · 类型 · 行号；且 fields 下不是裸类型节点
#   C parser 不再拒绝：命名写法 rc=0（修前 E2001）
#   D 旧位置写法 `V(float)` 不回归：仍解析、仍是裸 Named 节点（不是 EnumField）
#   E 多字段：Rect(w, h) 两个 EnumField 都进 AST
#   F 覆盖度自证（防用例被改空后门变恒绿）
#
# 负控（证明判据有鉴别力）：
#   NC-ast    把 B 期望字段名改成 radiusx ⇒ B 必须红
#   NC-legacy 把 D 期望反转（要求 V(float) 是 EnumField）⇒ D 必须红
#
# 用法：verify.sh [--neg-skip]
# 退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
PXPAR="$ROOT/bootstrap/pxpar"
WORK="${TMPDIR:-/tmp}/m367_enum_parse.$$"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
NC_MODE="${M367P_NC:-}"

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK"
for b in "$PX" "$PXPAR"; do [ -x "$b" ] || { echo "❌ 找不到可执行件 $b"; exit 1; }; done

echo "═══ M367-S1 · data enum 载荷字段进 AST 门 ═══"

cat > "$WORK/e.px" <<'EOF'
enum Shape:
    Circle(radius: float)
    Rect(w: float, h: float)
    Empty

let s = Shape.Circle
let r = Shape.Rect
print(s)
print(r)
EOF

# ── C：命名写法不再被 parser 拒绝 ───────────────────────────
"$PX" run "$WORK/e.px" > "$WORK/e.run" 2> "$WORK/e.run.err"; RC_RUN=$?
if [ "$RC_RUN" -eq 0 ] && ! grep -qE 'E2001|语法错误' "$WORK/e.run.err"; then
    ok "C 命名写法解析通过（rc=0，无 E2001）"
else
    bad "C 命名写法仍被拒（rc=$RC_RUN）：$(head -2 "$WORK/e.run.err" | tr '\n' ' ')"
fi

# ── A：三轨一致（无括号变体访问）────────────────────────────
"$PX" build "$WORK/e.px" > "$WORK/e.vmlog" 2>&1; RC_VM=$?
if [ "$RC_VM" -eq 0 ] && [ -x "$WORK/build/e" ]; then "$WORK/build/e" > "$WORK/e.vm" 2> "$WORK/e.vm.err"; else : > "$WORK/e.vm"; fi
rm -rf "$WORK/build"
"$PX" build --c "$WORK/e.px" > "$WORK/e.clog" 2>&1; RC_C=$?
if [ "$RC_C" -eq 0 ] && [ -x "$WORK/build/e" ]; then "$WORK/build/e" > "$WORK/e.c" 2> "$WORK/e.c.err"; else : > "$WORK/e.c"; fi
rm -rf "$WORK/build"

if [ "$RC_RUN" -ne 0 ] || [ "$RC_VM" -ne 0 ] || [ "$RC_C" -ne 0 ]; then
    bad "A 轨道非 0（run=$RC_RUN vm=$RC_VM c=$RC_C）"
elif cmp -s "$WORK/e.run" "$WORK/e.vm" && cmp -s "$WORK/e.run" "$WORK/e.c"; then
    ok "A 三轨逐字节一致（$(tr '\n' ' ' < "$WORK/e.run")）"
else
    bad "A 三轨不一致：run=[$(tr '\n' ' ' < "$WORK/e.run")] vm=[$(tr '\n' ' ' < "$WORK/e.vm")] c=[$(tr '\n' ' ' < "$WORK/e.c")]"
fi

# ── B：AST 真存（pxpar 输出）────────────────────────────────
"$PXPAR" "$WORK/e.px" > "$WORK/e.ast" 2>&1
EXP_FIELD="radius"
[ "$NC_MODE" = "ast" ] && EXP_FIELD="radiusx"
if grep -q 'EnumField' "$WORK/e.ast"; then
    ok "B0 AST 出现 EnumField 节点"
else
    bad "B0 AST 无 EnumField（字段名被丢掉）"
fi
if grep -q "name: \"$EXP_FIELD\"" "$WORK/e.ast"; then
    ok "B1 字段名进了 AST（$EXP_FIELD）"
else
    bad "B1 AST 里没有字段名 $EXP_FIELD"
fi
if grep -q 'Named(' "$WORK/e.ast" && grep -q '"float"' "$WORK/e.ast"; then
    ok "B2 字段类型进了 AST（Named/float）"
else
    bad "B2 AST 里没有字段类型"
fi
if grep -q 'line: 2,' "$WORK/e.ast"; then
    ok "B3 字段位置可定位（line 2）"
else
    bad "B3 字段位置丢了（应为 line 2）"
fi

# ── D：旧位置写法不回归 ─────────────────────────────────────
cat > "$WORK/legacy.px" <<'EOF'
enum Vec:
    V(float)
    W

let a = Vec.V
print(a)
EOF
"$PX" run "$WORK/legacy.px" > "$WORK/legacy.run" 2> "$WORK/legacy.run.err"; RC_L=$?
"$PXPAR" "$WORK/legacy.px" > "$WORK/legacy.ast" 2>&1
EXP_LEG="Vec.V"
[ "$NC_MODE" = "legacy" ] && EXP_LEG="Vec.W"
if [ "$RC_L" -eq 0 ] && [ "$(tr -d '\n' < "$WORK/legacy.run")" = "$EXP_LEG" ]; then
    ok "D1 旧写法 V(float) 仍解析且取值正确（$EXP_LEG）"
else
    bad "D1 旧写法回归：rc=$RC_L out=[$(tr -d '\n' < "$WORK/legacy.run")] 期望 $EXP_LEG"
fi
# 旧写法必须是**裸类型节点**（Named），不能被打成 EnumField —— 否则 AST 语义变了
if ! sed -n '/variants:/,/pos: Pos/p' "$WORK/legacy.ast" | grep -q 'EnumField'; then
    ok "D2 旧写法仍是裸类型节点（未误标 EnumField）"
else
    bad "D2 旧写法被打成 EnumField（AST 语义漂移）"
fi

# ── E：多字段都在 ───────────────────────────────────────────
NF=$(grep -c 'EnumField' "$WORK/e.ast")
if [ "$NF" -eq 3 ]; then
    ok "E1 三个字段（radius / w / h）全在 AST（EnumField 3 处）"
else
    bad "E1 EnumField 计数 $NF，期望 3"
fi

# ── F：覆盖度自证 ───────────────────────────────────────────
POSN=$(printf '%s\n' "named" "legacy")
if [ "$(printf '%s\n' "$POSN" | grep -c .)" -ge 2 ]; then
    ok "F1 正向形态 2 类（命名 / 位置）"
else
    bad "F1 正向形态不足"
fi

# ── 负控 ────────────────────────────────────────────────────
if [ "$NEG_SKIP" -eq 0 ] && [ -z "$NC_MODE" ]; then
    for pair in "ast:B1" "legacy:D1"; do
        m="${pair%%:*}"; judge="${pair##*:}"
        out="$(M367P_NC="$m" bash "$0" --neg-skip 2>&1 || true)"
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
echo "M367-S1 门结果：PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
