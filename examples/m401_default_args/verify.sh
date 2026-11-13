#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M401 · 默认参数**求值位置**：解释轨三轨分裂（缺陷 764 收口 · 缺陷 765 登记）
# ---------------------------------------------------------------------------
# 背景（实测 2026-11-14 00:0x）：`def g(a, b = a + 1)` 这份**合法**源码
#   （spec：docs/MINI_SUBSET §「def 支持默认参数值」；语料：examples/m90_s1/cmpsem.px）
#   `px build`（VM 轨）→ 34 · `px build --c`（C 轨）→ 34 · `px run`（解释轨）
#   → **R1001 未定义变量: 'a'**（rc=1）。三轨两种命运。
#   根因一行：`selfhost/icall.px` 把默认表达式放在 **fd["closure"]（定义处闭包）** 求值，
#   而前序形参是绑在**本帧** call_env 里的 ⇒ 查不到。VM 轨（bc_emit_default_fill 帧入口
#   逐参求值）与 C 轨（codegen 生成 (nargs > i) ? args[i] : default_c，声明序里可引用前参）
#   都是「调用时、函数帧内」求值 ⇒ 解释轨与两者同一条真相才对。
#
# 判据（每条都配「改坏了必红」的差分断言）：
#   T1 常量默认            T2 引用前序参数（含二级引用 b 引 a、c 引 b）
#   T3 引用全局            T4 多默认**部分提供**（0/1/2/3 个实参各一档）
#   T5 别名间接调用缺参（函数值调用路径，非直接 Call）
#   T6 方法 self 默认（impl 里 extra = self.x + 1）
#   T7 **每次调用求值**（改全局后同一次运行里的结果必须变）—— 判「定义时固化」为红
#   X1 三轨（interp / vm / c）stdout 逐字节一致
#   N1 判据自证：把 T2 的期望值改错 ⇒ 判据必须判红（证明判据不是恒绿）
#   P  前提自证：上一发布件（v0.2.400 的 bootstrap/pxi）必须在本用例上**失败** ——
#      证明本门测到的是这次修复，不是别的
#   K1 已知缺口（缺陷 765 · 不计失败）：默认表达式**前向引用**后序形参
#      `def f(a = b, b = 1)` 在**未提供**该形参时，C 轨/解释轨都 R1001，
#      而 VM 轨静默给出 null（rc=0）。VM 侧修法要动字节码入口哨兵序列，独立一轮。
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
TMP="${TMPDIR:-/tmp}/m401_dflt.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

cat > "$TMP/main.px" <<'PXEOF'
let G = 100

def f1(a, b = 5):
    return a + b

def f2(a, b = a + 1):
    return a * 10 + b

def f3(a, b = G + a):
    return a * 100 + b

def f4(a, b = a + 1, c = 9):
    return a * 100 + b * 10 + c

def f6(a = 1, b = a + 1, c = b + 1):
    return a * 100 + b * 10 + c

struct P:
    x: int

impl P:
    def get(self, extra = self.x + 1):
        return self.x * 10 + extra

var K = 1

def f7(a, b = a + K):
    return a * 10 + b

def main():
    print("T1 " + str(f1(1)))
    print("T2 " + str(f2(3)))
    print("T3 " + str(f3(2)))
    print("T4 " + str(f4(1)) + " " + str(f4(1, 2)) + " " + str(f4(1, 2, 3)))
    print("T5 " + str(f6()) + " " + str(f6(5)))
    let p = P(7)
    print("T6 " + str(p.get()) + " " + str(p.get(3)))
    let alias = f2
    print("T7 " + str(alias(3)) + " " + str(f2(3)))
    K = 10
    print("T8 " + str(f7(2)))
    print("ALL DONE")
main()
PXEOF

# ── 三轨执行器 ─────────────────────────────────────────────────────────────
run_track() {
    local track="$1" src="$2" out="$3" nm
    nm="$(basename "$src" .px)"
    case "$track" in
    interp) "$PX" run "$src" > "$out" 2> "$out.err" ;;
    vm)     "$PX" build "$src" > "$out.build" 2>&1 || return $?
            "$TMP/build/$nm" > "$out" 2> "$out.err" ;;
    c)      "$PX" build --c "$src" > "$out.build" 2>&1 || return $?
            "$TMP/build/$nm" > "$out" 2> "$out.err" ;;
    *)      return 9 ;;
    esac
}

# ── 期望值（由**契约**给出，不是拿某轨输出回抄）─────────────────────────────
EXPECT="T1 6
T2 34
T3 302
T4 129 129 123
T5 123 567
T6 78 73
T7 34 34
T8 32
ALL DONE"

judge() {   # $1=stdout 文件；全行逐字节命中 + 无多余行
    printf '%s\n' "$EXPECT" > "$TMP/expect.txt"
    cmp -s "$TMP/expect.txt" "$1"
}

if run_track interp "$TMP/main.px" "$TMP/i.out"; then
    if judge "$TMP/i.out"; then
        ok "T1–T8 全部断言命中（$(wc -l < "$TMP/i.out") 行逐字节一致）"
    else
        bad "断言不符：$(diff "$TMP/expect.txt" "$TMP/i.out" | head -6 | tr '\n' ' ')"
    fi
else
    bad "interp 主体失败：$(head -2 "$TMP/i.out.err" | tr '\n' ' ')"
fi

# ── X1：三轨逐字节一致（VM 是用户面默认轨，C 轨是发布轨）──────────────────
for tr in vm c; do
    if run_track "$tr" "$TMP/main.px" "$TMP/$tr.out"; then
        if cmp -s "$TMP/i.out" "$TMP/$tr.out"; then
            ok "X1-$tr 与 interp 逐字节一致（$(wc -l < "$TMP/$tr.out") 行）"
        else
            bad "X1-$tr 与 interp 不一致：$(diff "$TMP/i.out" "$TMP/$tr.out" | head -4 | tr '\n' ' ')"
        fi
    else
        bad "$tr 轨失败：$(head -3 "$TMP/$tr.out.err" 2>/dev/null | tr '\n' ' ')$(head -3 "$TMP/$tr.out.build" 2>/dev/null | tr '\n' ' ')"
    fi
done

# ── N1：判据自证 —— 期望值改错一位，judge 必须判红 ────────────────────────
printf '%s\n' "$EXPECT" | sed 's/^T2 34$/T2 35/' > "$TMP/expect_bad.txt"
if cmp -s "$TMP/expect_bad.txt" "$TMP/expect.txt"; then
    bad "N1 判据自证失败：改期望值没改到（夹具坏了）"
elif [ -f "$TMP/i.out" ] && cmp -s "$TMP/expect_bad.txt" "$TMP/i.out"; then
    bad "N1 判据自证失败：错期望值也判绿 ⇒ 判据没牙"
else
    ok "N1 判据自证：期望值改错一位即判红（判据有牙）"
fi

# ── P：前提自证 —— 上一发布件的运行时必须在本用例上失败 ───────────────────
#   判据对准「求值位置」这一改动：旧件的编译器/运行时都不认这个写法（R1001）。
BASE_TAG="${PX_BASE_TAG:-v0.2.400}"
if git cat-file -e "$BASE_TAG:bootstrap/pxi" 2>/dev/null; then
    git show "$BASE_TAG:bootstrap/pxi" > "$TMP/pxi_base" 2>/dev/null
    chmod +x "$TMP/pxi_base"
    "$TMP/pxi_base" "$TMP/main.px" > "$TMP/p.out" 2> "$TMP/p.err"
    rc=$?
    if [ "$rc" = 0 ]; then
        bad "P 前提自证失败：$BASE_TAG 的运行时也跑通了 ⇒ 本门没测到修复"
    elif grep -q 'R1001' "$TMP/p.err" && ! grep -q '^T2 ' "$TMP/p.out"; then
        ok "P 前提自证：$BASE_TAG 运行时在 T2 处报 R1001（$(head -1 "$TMP/p.err" | cut -c1-72)）"
    else
        bad "P 前提自证：$BASE_TAG 运行时失败了，但不是 R1001/也不是在 T2 前（rc=$rc · $(head -1 "$TMP/p.err" | cut -c1-72)）"
    fi
else
    echo "  ⏭  P 前提自证跳过（本地无标签 $BASE_TAG 的 bootstrap/pxi）"
fi

# ── K1：已知缺口（缺陷 765）—— 前向引用默认值，三轨未统一 ─────────────────
#   本项**不计失败**：VM 侧要动字节码入口哨兵序列（独立一轮）。但必须**响亮报告**，
#   不能静默放过 —— 口径同 m158 的「已知缺口报告（SKIP 不计失败）」。
cat > "$TMP/fwd.px" <<'PXEOF'
def fw(a = b, b = 1):
    return a

def main():
    print(fw())
main()
PXEOF
FWD_IP=0; FWD_C=0; FWD_VM=0
"$PX" run "$TMP/fwd.px" > "$TMP/fwd.i.out" 2> "$TMP/fwd.i.err" || FWD_IP=$?
run_track c "$TMP/fwd.px" "$TMP/fwd.c.out" 2>/dev/null; FWD_C=$?
run_track vm "$TMP/fwd.px" "$TMP/fwd.vm.out" 2>/dev/null; FWD_VM=$?
if [ "$FWD_IP" != 0 ] && [ "$FWD_C" != 0 ] && [ "$FWD_VM" = 0 ]; then
    echo "  ⏭  K1 已知缺口（缺陷 765）：前向引用默认值 —— interp/C 报错(rc=$FWD_IP/$FWD_C)，"
    echo "      VM 静默给 null(rc=0，输出 $(head -1 "$TMP/fwd.vm.out" | cut -c1-20))。VM 侧独立一轮修。"
elif [ "$FWD_IP" = 0 ] || [ "$FWD_C" = 0 ]; then
    bad "K1 形态变了：interp/C 至少一轨接受了前向引用（rc=$FWD_IP/$FWD_C）—— 缺陷 765 定性要重做"
elif [ "$FWD_VM" != 0 ]; then
    ok "K1 缺陷 765 已被修掉（三轨同报错 rc=$FWD_IP/$FWD_C/$FWD_VM）—— 请把本项升级为硬判据"
fi

echo "── m401_default_args：通过 $pass · 失败 $fail"
[ "$fail" = 0 ] || exit 1
