#!/usr/bin/env bash
# ============================================================
# examples/m366_enum_payload/verify.sh —— 枚举变体带载荷构造：三轨报文对拍门（M366）
# ------------------------------------------------------------
# 为什么有这道门（缺陷 634，M366 实测三轨三种结果）：
#   `enum Shape: Circle` + `Shape.Circle(1.5)`
#     解释轨  R1007 类型 type 没有方法 'Circle'   ← 让人去找**方法**（方向错）
#     VM 轨   编译成功 → 运行期 R1001 未定义变量 'Shape'  ← 说 Shape 是**变量**（对象错）
#     C 轨    同上 R1001
#     spec §3.8 却早已承诺 **R1005**              ← 文档与三轨全不符
#   同族的 `Shape.Nope(1)`（变体不存在）同样分叉（R1007 / R1001 / R1001），
#   而无括号形态 `Shape.Nope` 三轨本就同码 R1008 —— 说明**分叉只在「带括号」这一格**。
#
# 定稿口径（与 Field 访问同口径）：
#   变体存在   ⇒ R1005「枚举变体 X.Y 不接受参数（data enum 未实现；请写 X.Y）」
#   变体不存在 ⇒ R1008「枚举 X 没有变体 'Y'」
#   ⚠️ 实参**不求值**：编译两轨在本处直接发报错，解释轨必须在求值前返回（否则带副作用实参会分叉）。
#
# 判据：
#   A 三轨逐字节一致（正向：无括号变体访问仍正常 —— 防「修一个坏一个」）
#   B 与门侧独立参考对拍（期望值由门写死，不抄实现）
#   C 错误面：三轨同码同文（归一化去位置/前缀后逐字比对）+ 门侧谓词（码 + 必须指名枚举与变体）
#   D 副作用实参不被求值（三轨 stdout 均不得出现标记）
#   E 覆盖度自证（正向 ≥2 形态 / 错误 ≥3 类，防用例被改空后门变恒绿）
#
# 负控（证明判据有鉴别力）：
#   NC-code  把 e1 观测码改成 R1008          ⇒ C 必须红（三轨不一致）
#   NC-text  把 e1 观测报文改一个字           ⇒ C 必须红
#   NC-pos   把正向期望值改一个字             ⇒ B 必须红
#   NC-side  把 D 判据反向（必须出现副作用）  ⇒ D 必须红
#
# 覆盖边界（如实写，不在本门判据内）：变量名**遮蔽**枚举类型名时，编译两轨按名判、
#   解释轨按值判 ⇒ 仍会分叉（缺陷 635）。M366 前即如此（此前编译轨报 R1001），
#   本轮只把报文改准，未改判据面 —— 修它要三轨统一的「名字解析优先级」，另立里程碑。
#
# 用法：verify.sh [--neg-skip]
# 退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m366_enum_payload.$$"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
NC_MODE="${M366_NC:-}"

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK"
[ -x "$PX" ] || { echo "❌ 找不到 $PX"; exit 1; }

# ── 三轨产出 ────────────────────────────────────────────────
run_lanes() {  # $1=prog $2=前缀 → $2.run/$2.vm/$2.c（stdout）· $2.{run,vm,c}.err · $2.rc
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

# 归一化首条诊断：解释轨 `运行时错误: 错误 [R1005] 5:21: 体`
#               编译两轨 `运行时错误 [<top> 行5]: R1005: 体` / `[行5]: R1005: 体`
norm_err() {  # $1=文件 → "码|体"
    sed -e 's/^运行时错误: 错误 \[\([A-Z][0-9]*\)\] [0-9]*:[0-9]*: /\1|/' \
        -e 's/^运行时错误 \[[^]]*\]: \([A-Z][0-9]*\): /\1|/' \
        -e 's/^[0-9]*:[0-9]*: 语法错误 \([A-Z][0-9]*\): /\1|/' "$1" 2>/dev/null \
        | grep -E '^[A-Z][0-9]*\|' | head -1
}

PRE='enum Shape:
  Circle
  Square
'

echo "═══ M366 · 枚举变体带载荷构造三轨对拍门 ═══"

# ── A/B：正向（无括号变体访问不得被本次改动弄坏）────────────────
POS_SPEC=(
  'p1|let s = Shape.Circle|Shape.Circle'
  'p2|let s = Shape.Square|Shape.Square'
)
NP=${#POS_SPEC[@]}
for spec in "${POS_SPEC[@]}"; do
    IFS='|' read -r pname pline pexp <<< "$spec"
    f="$WORK/$pname.px"; pfx="$WORK/$pname"
    printf '%s\n%s\nprint(s)\n' "$PRE" "$pline" > "$f"
    exp="$pexp"
    [ "$NC_MODE" = "pos" ] && [ "$pname" = "p1" ] && exp="Shape.Square"
    run_lanes "$f" "$pfx"
    read -r RC_RUN RC_VM RC_C < "$pfx.rc"
    if [ "$RC_RUN" -ne 0 ] || [ "$RC_VM" -ne 0 ] || [ "$RC_C" -ne 0 ]; then
        bad "A-$pname 轨道非 0（run=$RC_RUN vm=$RC_VM c=$RC_C）"
    elif cmp -s "$pfx.run" "$pfx.vm" && cmp -s "$pfx.run" "$pfx.c"; then
        ok "A-$pname 三轨逐字节一致（$(tr -d '\n' < "$pfx.run")）"
    else
        bad "A-$pname 三轨不一致：run=$(tr -d '\n' < "$pfx.run") vm=$(tr -d '\n' < "$pfx.vm") c=$(tr -d '\n' < "$pfx.c")"
    fi
    for lane in run vm c; do
        got="$(tr -d '\n' < "$pfx.$lane")"
        if [ "$got" = "$exp" ]; then ok "B-$pname-$lane 与门侧参考一致（$exp）"
        else bad "B-$pname-$lane 期望 $exp，实得 $got"; fi
    done
done

# ── C：错误面三轨同码同文（含不带括号的既有口径，防回归）──────────
ERR_SPEC=(
  'e1_payload|let s = Shape.Circle(1.5)|R1005|不接受参数|Shape.Circle'
  'e2_zero|let s = Shape.Circle()|R1005|不接受参数|Shape.Circle'
  'e3_novariant|let s = Shape.Nope(1.5)|R1008|没有变体|Nope'
  'e4_field|let s = Shape.Nope|R1008|没有变体|Nope'
)
NE=${#ERR_SPEC[@]}
for spec in "${ERR_SPEC[@]}"; do
    IFS='|' read -r ename eline ecode ekey ekey2 <<< "$spec"
    f="$WORK/$ename.px"; pfx="$WORK/$ename"
    printf '%s\n%s\nprint(s)\n' "$PRE" "$eline" > "$f"
    run_lanes "$f" "$pfx"
    r1="$(norm_err "$pfx.run.err")"; r2="$(norm_err "$pfx.vm.err")"; r3="$(norm_err "$pfx.c.err")"
    [ -z "$r2" ] && r2="$(norm_err "$pfx.vmlog")"
    [ -z "$r3" ] && r3="$(norm_err "$pfx.clog")"
    [ "$NC_MODE" = "code" ] && [ "$ename" = "e1_payload" ] && r1="R1008|${r1#*|}"
    [ "$NC_MODE" = "text" ] && [ "$ename" = "e1_payload" ] && r1="${r1%%|*}|${r1#*|}改"
    if [ -z "$r1" ]; then bad "C-$ename 解释轨无诊断（应报 $ecode）"; continue; fi
    if [ "$r1" = "$r2" ] && [ "$r1" = "$r3" ]; then
        ok "C-$ename 三轨同码同文（$r1）"
    else
        bad "C-$ename 三轨不一致：run=[$r1] vm=[$r2] c=[$r3]"
    fi
    code_only="${r1%%|*}"
    if [ "$code_only" = "$ecode" ] && [[ "$r1" == *"$ekey"* ]] && { [ -z "$ekey2" ] || [[ "$r1" == *"$ekey2"* ]]; }; then
        ok "C-$ename 码与关键词齐（$ecode / $ekey / $ekey2）"
    else
        bad "C-$ename 码或关键词不符：期望 $ecode / $ekey / $ekey2，实得 $r1"
    fi
done

# ── D：副作用实参不被求值 ───────────────────────────────────
f="$WORK/d_side.px"; pfx="$WORK/d_side"
{
    printf '%s\n' "$PRE"
    echo 'def eff() -> float:'
    echo '    print("SIDE-EFFECT-MARK")'
    echo '    return 1.5'
    echo ''
    echo 'let s = Shape.Circle(eff())'
    echo 'print(s)'
} > "$f"
run_lanes "$f" "$pfx"
for lane in run vm c; do
    if [ "$NC_MODE" = "side" ]; then
        # NC-side：把断言**反向**（要求出现标记）⇒ D 必须判红。
        #   实测第 1 版写成「命中即 ok」，负控因此打不出 "❌ D" ⇒ 判据无鉴别力假象，
        #   改成「未命中即 bad」才真红（教训同 M365 的 C 判据 if/else）。
        if grep -q 'SIDE-EFFECT-MARK' "$pfx.$lane"; then
            ok "D-$lane 副作用实参未被求值"
        else
            bad "D-$lane 反向断言：期望出现 SIDE-EFFECT-MARK，未出现"
        fi
    elif grep -q 'SIDE-EFFECT-MARK' "$pfx.$lane"; then
        bad "D-$lane 副作用实参被求值了（stdout 出现 SIDE-EFFECT-MARK）"
    else
        ok "D-$lane 副作用实参未被求值"
    fi
done

# ── E：覆盖度自证（防空转）──────────────────────────────────
[ "$NP" -ge 2 ] && ok "E1 正向形态 $NP 组（≥2）" || bad "E1 正向形态只有 $NP 组"
[ "$NE" -ge 3 ] && ok "E2 错误类 $NE 种（≥3）" || bad "E2 错误类只有 $NE 种"
ALL="$(printf '%s\n' "${ERR_SPEC[@]}")"
if grep -q 'R1005' <<< "$ALL" && grep -q 'R1008' <<< "$ALL" && grep -q 'e3_novariant' <<< "$ALL"; then
    ok "E3 码面齐（R1005 载荷 + R1008 无变体）"
else
    bad "E3 码面不全（防用例退化后门变恒绿）"
fi

# ── 负控 ────────────────────────────────────────────────────
if [ "$NEG_SKIP" -eq 0 ] && [ -z "$NC_MODE" ]; then
    for pair in "code:C" "text:C" "pos:B" "side:D"; do
        m="${pair%%:*}"; judge="${pair##*:}"
        out="$(M366_NC="$m" bash "$0" --neg-skip 2>&1 || true)"
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
echo "M366 门结果：PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
