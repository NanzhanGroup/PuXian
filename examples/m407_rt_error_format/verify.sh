#!/usr/bin/env bash
# ============================================================
# examples/m407_rt_error_format/verify.sh —— 运行时错误报文三轨统一（M407）
# ------------------------------------------------------------
# 为什么有这道门：
#   缺陷 186 族「诊断三轨不同」登记了很久，21 扇门里写着「属 186 族 ⇒ **不判**」。
#   M407 之前的实测（同一份源码、同一台机、三轨各一次）：
#     interp : 运行时错误: 错误 [R1002] 2:14: 无法相加: int + string
#     VM     : 运行时错误 [f 行2]: R1002: 无法相加: int + string
#     C      : 运行时错误 [helper 行6]: R1002: 无法相加: int + string   ← 名字还是**错的**
#   三种前缀 + C 轨归因错误（被调函数返回后 g_px_src_func 停在 callee）+ 顶层 C 轨无名字。
#   M407 定稿规范（docs/ERROR_CODES.md §0 · spec §16.5）：
#     运行时错误 [<fn> 行<N>]: <CODE>: <msg>
#       <fn> = 出错语句**所在**函数名；顶层 = <top>；匿名闭包 = <closure>（不带序号）；
#              具名帧内 def = 该 def 名
#       <N>  = 该语句的 .px 行号；**不打列号**（编译两轨字节码无列信息）
#
# 判据（每条都是「被测 vs 外部参考」）：
#   A 三轨逐字节   每条用例的 stdout / stderr / rc 三轨 cmp 完全相同（**不再归一化**）
#   B 门侧独立参考 首行报文按规范**手写**在 EXPECT 表里（不是从实现里抓的）
#   C 覆盖度自证   ≥6 个错误码 · 4 种归因形态（顶层 / 具名函数 / 具名嵌套 def / 匿名闭包）
#                  · 每条报文都匹配 `^运行时错误 \[[^]]+ 行[0-9]+\]: R[0-9]{4}: `
# 负控：
#   N1 ref      EXPECT 扰动 1 处 ⇒ B 必须红（判据有鉴别力）
#   N2 okform   正常程序（无错）⇒ 三轨 stderr 皆空、stdout 一致 ⇒ 判据不是恒红
#   P  前提自证 v0.2.406 的 bootstrap/pxi 在用例 1 上必须给**旧格式**（`运行时错误: 错误 [`）
#
# 用法：verify.sh [--neg-skip]
# 退出码：0=全绿 · 1=有红
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="${TMPDIR:-/tmp}/m407_rtfmt.$$"
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
NC_MODE="${M407_NC:-}"

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT
mkdir -p "$WORK/cases"
[ -x "$PX" ] || { echo "❌ 找不到 $PX"; exit 1; }

# ── 用例：名字 | 期望首行（按规范手写）─────────────────────────
CASES="top_arith fn_arith caller_attrib nested_fn closure undef_var index_oob dict_key struct_arity match_nonexh enum_field top_prop"
EXPECT_top_arith='运行时错误 [<top> 行1]: R1002: 无法相加: int + string'
EXPECT_fn_arith='运行时错误 [f 行2]: R1002: 无法相加: int + string'
EXPECT_caller_attrib='运行时错误 [main 行6]: R1002: 无法相加: int + string'
EXPECT_nested_fn='运行时错误 [inner 行3]: R1002: 无法相加: int + string'
EXPECT_closure='运行时错误 [<closure> 行3]: R1002: 无法相加: int + string'
EXPECT_undef_var='运行时错误 [f 行2]: R1001: 未定义变量: '"'"'zzz'"'"''
EXPECT_index_oob='运行时错误 [main 行3]: R1003: 索引越界: 5 (len=2)'
EXPECT_dict_key='运行时错误 [main 行3]: R1008: 字典没有键 '"'"'b'"'"''
EXPECT_struct_arity='运行时错误 [main 行4]: R2001: 结构体 P 需要 1 个字段，给出 0'
EXPECT_match_nonexh='运行时错误 [f 行2]: R1003: match 未匹配任何分支（非穷尽）：3'
EXPECT_enum_field='运行时错误 [f 行5]: R2001: 枚举变体 Shape.Circle 没有字段「radius」'
EXPECT_top_prop='运行时错误 [<top> 行4]: R1004: 顶层不能使用错误传播 ?（仅函数内可用）'

# N1：把 caller_attrib 的期望改成「被调函数名」——旧 C 轨正是这么错的，必须判红
[ "$NC_MODE" = "ref" ] && EXPECT_caller_attrib='运行时错误 [helper 行6]: R1002: 无法相加: int + string'

w() { cat > "$WORK/cases/$1.px"; }

w top_arith <<'PXEOF'
let x = 1 + "s"
def main():
    print("never")
PXEOF

w fn_arith <<'PXEOF'
def f(n: int) -> int:
    return n + "x"
def main():
    print(str(f(1)))
PXEOF

# 归因回归：main 里**先调用 helper**，再在 main 第 6 行出错 ⇒ 必须报 main（旧 C 轨报 helper）
w caller_attrib <<'PXEOF'
def helper() -> int:
    return 41

def main():
    let v = helper()
    let w = v + "s"
    print(str(w))
PXEOF

w nested_fn <<'PXEOF'
def outer() -> int:
    def inner(n: int) -> int:
        return n + "s"
    return inner(1)

def main():
    print(str(outer()))
PXEOF

w closure <<'PXEOF'
def main():
    let f = fn(n: int) -> int:
        return n + "s"
    print(str(f(1)))
PXEOF

w undef_var <<'PXEOF'
def f():
    return zzz
def main():
    print(str(f()))
PXEOF

w index_oob <<'PXEOF'
def main():
    let l = [1, 2]
    print(str(l[5]))
PXEOF

w dict_key <<'PXEOF'
def main():
    let d = {"a": 1}
    print(str(d["b"]))
PXEOF

w struct_arity <<'PXEOF'
struct P:
    a: int
def main():
    let p = P()
    print(str(p.a))
PXEOF

w match_nonexh <<'PXEOF'
def f(n: int) -> str:
    match n:
        case 1:
            return "one"
        case 2:
            return "two"
    return "none"

def main():
    print(f(3))
PXEOF

w enum_field <<'PXEOF'
enum Shape:
    Circle(r: float)

def f(s: Shape):
    match s:
        case Shape.Circle(radius: r):
            print(str(r))
        case _:
            print("other")

def main():
    f(Shape.Circle(1.5))
PXEOF

w top_prop <<'PXEOF'
def f() -> Result[int]:
    return Err("x")

let v = f()?
print(str(v))
PXEOF

# ── 三轨执行：stdout / stderr / rc 全落文件 ───────────────────
run3() {
    local name="$1" prog="$WORK/cases/$1.px" pfx="$WORK/$1"
    "$PX" run "$prog" > "$pfx.run" 2> "$pfx.run.err"; local rc_run=$?
    rm -rf "$WORK/cases/build"
    "$PX" build "$prog" > "$pfx.vmlog" 2>&1; local rc_vm_build=$?
    if [ $rc_vm_build -eq 0 ] && [ -x "$WORK/cases/build/$name" ]; then
        "$WORK/cases/build/$name" > "$pfx.vm" 2> "$pfx.vm.err"; local rc_vm=$?
    else
        : > "$pfx.vm"; : > "$pfx.vm.err"; local rc_vm="BUILD-FAIL"
    fi
    rm -rf "$WORK/cases/build"
    "$PX" build --c "$prog" > "$pfx.clog" 2>&1; local rc_c_build=$?
    if [ $rc_c_build -eq 0 ] && [ -x "$WORK/cases/build/$name" ]; then
        "$WORK/cases/build/$name" > "$pfx.c" 2> "$pfx.c.err"; local rc_c=$?
    else
        : > "$pfx.c"; : > "$pfx.c.err"; local rc_c="BUILD-FAIL"
    fi
    rm -rf "$WORK/cases/build"
    printf '%s %s %s\n' "$rc_run" "$rc_vm" "$rc_c" > "$pfx.rc"
}

echo "═══ M407 · 运行时错误报文三轨统一 门 ═══"

CODES=""
SHAPES=""
for c in $CASES; do
    run3 "$c"
    read -r RC_RUN RC_VM RC_C < "$WORK/$c.rc"
    exp_var="EXPECT_$c"
    want=$(eval "printf '%s' \"\${$exp_var}\"")
    got_run=$(head -1 "$WORK/$c.run.err" 2>/dev/null)
    got_vm=$(head -1 "$WORK/$c.vm.err" 2>/dev/null)
    got_c=$(head -1 "$WORK/$c.c.err" 2>/dev/null)

    # A 三轨逐字节（rc / stdout / stderr 三项全等）
    if [ "$RC_RUN" = "$RC_VM" ] && [ "$RC_RUN" = "$RC_C" ] \
       && cmp -s "$WORK/$c.run" "$WORK/$c.vm" && cmp -s "$WORK/$c.run" "$WORK/$c.c" \
       && cmp -s "$WORK/$c.run.err" "$WORK/$c.vm.err" && cmp -s "$WORK/$c.run.err" "$WORK/$c.c.err"; then
        ok "A[$c] 三轨逐字节一致（rc=$RC_RUN）"
    else
        bad "A[$c] 三轨不一致：rc=$RC_RUN/$RC_VM/$RC_C"
        printf '        i: %s\n        v: %s\n        c: %s\n' "$got_run" "$got_vm" "$got_c"
    fi

    # B 与门侧手写期望逐字相同
    if [ "$got_run" = "$want" ] && [ "$got_vm" = "$want" ] && [ "$got_c" = "$want" ]; then
        ok "B[$c] 三轨首行 = 规范手写期望"
    else
        bad "B[$c] 期望：$want"
        printf '        实得 i: %s\n        实得 v: %s\n        实得 c: %s\n' "$got_run" "$got_vm" "$got_c"
    fi

    # C 报文形状：前缀规范 + 码 + 消息体非空
    if printf '%s' "$got_run" | grep -Eq '^运行时错误 \[[^]]+ 行[0-9]+\]: R[0-9]{4}: .+'; then
        :
    else
        bad "C[$c] 报文不匹配规范形状：$got_run"
    fi
    CODES="$CODES $(printf '%s' "$got_run" | sed -n 's/.*行[0-9]*\]: \(R[0-9]\{4\}\): .*/\1/p')"
    case "$c" in
        top_arith|top_prop) SHAPES="$SHAPES top" ;;
        nested_fn) SHAPES="$SHAPES named-nested" ;;
        closure) SHAPES="$SHAPES closure" ;;
        *) SHAPES="$SHAPES named" ;;
    esac
done

# ── C 覆盖度自证（防用例退化 ⇒ 门变空转）───────────────────────
ncode=$(printf '%s\n' $CODES | sort -u | grep -c '^R[0-9]' || true)
if [ "${ncode:-0}" -ge 6 ]; then
    ok "C 错误码覆盖 $ncode 种：$(printf '%s\n' $CODES | sort -u | tr '\n' ' ')"
else
    bad "C 错误码覆盖不足（$ncode < 6）⇒ 用例退化"
fi
nshape=$(printf '%s\n' $SHAPES | sort -u | grep -c . || true)
if [ "${nshape:-0}" -ge 4 ]; then
    ok "C 归因形态覆盖 $nshape 种：$(printf '%s\n' $SHAPES | sort -u | tr '\n' ' ')"
else
    bad "C 归因形态覆盖不足（$nshape < 4）⇒ 归因判据会退化成只测一个函数"
fi
if grep -q '行0' <<<"$(for c in $CASES; do eval "printf '%s\n' \"\$EXPECT_$c\""; done)"; then
    bad "C 期望表含「行0」（假位置 ⇒ 未取到真实行号）"
else
    ok "C 期望表无「行0」假位置"
fi

# ── N2 非恒红：正常程序三轨 stderr 皆空、stdout 一致 ─────────────
if [ "$NEG_SKIP" = 0 ]; then
    cat > "$WORK/cases/okprog.px" <<'PXEOF'
struct P:
    a: int = 7

def main():
    let p = P()
    let l = [1, 2, 3]
    print(str(p.a), str(len(l)), str(1 + 2))
PXEOF
    run3 okprog
    if [ -s "$WORK/okprog.run.err" ] || [ -s "$WORK/okprog.vm.err" ] || [ -s "$WORK/okprog.c.err" ]; then
        bad "N2 合法程序产生 stderr（判据恒红）：$(head -1 "$WORK/okprog.run.err")"
    elif cmp -s "$WORK/okprog.run" "$WORK/okprog.vm" && cmp -s "$WORK/okprog.run" "$WORK/okprog.c"; then
        ok "N2 合法程序三轨 stdout 一致、stderr 皆空（判据不是恒红）"
    else
        bad "N2 合法程序三轨 stdout 不一致"
    fi
else
    note "N2 已跳过（--neg-skip）"
fi

# ── N1 判据自证：EXPECT 扰动 1 处 ⇒ B 必红 ─────────────────────
if [ "$NEG_SKIP" = 0 ]; then
    if [ "$NC_MODE" = "ref" ]; then
        ok "N1 判据自证：EXPECT[caller_attrib] 已扰动为「helper 行6」（旧 C 轨的错误答案）"
        note "  ⇒ 若上面 B[caller_attrib] 判红，则本门对该缺陷有鉴别力；若仍判绿，则门无牙"
    else
        note "N1 负控需 M407_NC=ref 重跑（由 registry 的负控钩子触发）"
    fi
fi

# ── P 前提自证：v0.2.406 必须给旧格式 ──────────────────────────
P_TAG="${PX_RTFMT_BASE_TAG:-v0.2.406}"
if git -C "$ROOT" cat-file -e "$P_TAG:bootstrap/pxi" 2>/dev/null; then
    git -C "$ROOT" show "$P_TAG:bootstrap/pxi" > "$WORK/pxi_old" 2>/dev/null
    chmod +x "$WORK/pxi_old"
    out_old=$("$WORK/pxi_old" "$WORK/cases/fn_arith.px" 2>&1 | head -1)
    if printf '%s' "$out_old" | grep -q '^运行时错误: 错误 \[R'; then
        ok "P 前提自证：$P_TAG 给旧格式（$(printf '%s' "$out_old" | cut -c1-40)…）⇒ 本轮改动可见"
    else
        bad "P 前提自证：$P_TAG 未给旧格式（$out_old）⇒ 门可能在测改动前的东西"
    fi
else
    note "P 前提自证跳过（本地无标签 $P_TAG 的 bootstrap/pxi）"
fi

echo
echo "── m407_rt_error_format：通过 $PASS · 失败 $FAIL"
[ "$FAIL" = 0 ] || exit 1
