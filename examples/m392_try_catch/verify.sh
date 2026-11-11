#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M392 · try/catch（PX-025）—— 动的是**运行期错误模型**，不是 parser
# ---------------------------------------------------------------------------
# 判据分三组（每条都配「修法回退则必红」的差分断言）：
#   T 正控（语义）   T1 捕到 R1001 且取到消息 · T2 正常路径不进 handler
#                    T3 嵌套内层先捕 · T4 catch 里 return · T5 循环内捕到后 break
#                    T6 跨函数帧（错误在深层函数抛、外层 try 捕 ⇒ VM 要丢帧）
#                    T7 catch 内再抛错被外层捕（帧已弹 ⇒ 不许死循环）
#   N 负控           N1 函数内 try+return 之后再触发错误 ⇒ 不许段错误（rc≠139）
#                    N2 未捕获的错误仍然响亮（rc≠0 且带 R 码）
#   X 三轨一致       X1 interp / VM / C 三轨同一份源的输出**逐字节一致**（T1–T7 全在里面）
#   P 前提自证       旧编译器（上一版 tag）必须拒绝本用例 ⇒ 证明用例真测到新语法
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
TMP="${TMPDIR:-/tmp}/m392_try.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

# ── 主用例：T1–T7（全部可捕获 ⇒ 正常结束 rc=0）─────────────────────────────
cat > "$TMP/main.px" <<'PXEOF'
# T1 捕到运行时错误并取到消息（三轨统一口径 = "R码: 消息"）
try:
    print(nope_undefined)
catch e:
    print("T1 " + e)

# T2 正常路径不触发 handler
try:
    print("T2 ok")
catch e:
    print("T2b BAD")

# T3 嵌套：内层先捕
try:
    try:
        print(zz_nope)
    catch e:
        print("T3 inner")
    print("T3 after-inner")
catch e:
    print("T3 outer BAD")

# T4 catch 里 return 正常返回
def f4():
    try:
        print(nope4)
    catch e:
        return "T4 ok"
    return "T4 BAD"
print(f4())

# T5 循环内 try 捕到后 break
def f5():
    var i = 0
    while i < 3:
        try:
            if i == 1:
                print(nope5)
            print("T5 i=" + str(i))
        catch e:
            print("T5 caught " + str(i))
            break
        i += 1
    return "T5 end"
print(f5())

# T6 跨函数帧：错误在深层函数抛，外层 try 捕（VM 必须丢弃压深的帧）
def deep6():
    print(nope6)
def outer6():
    try:
        deep6()
    catch e:
        return "T6 ok"
    return "T6 BAD"
print(outer6())

# T7 catch 内再抛错，被外层捕获（帧已弹 ⇒ 不许跳回同一个 jmp_buf 死循环）
def f7():
    try:
        try:
            print(nope7a)
        catch e:
            print("T7 inner")
            print(nope7b)
    catch e:
        return "T7 outer-ok"
    return "T7 BAD"
print(f7())

# T8 不带名字的 catch（丢弃错误值）
try:
    print(nope8)
catch:
    print("T8 noname")
print("ALL DONE")
PXEOF

# ── 三轨执行器：interp / vm / c ─────────────────────────────────────────────
# 统一把 stdout 写 $2，stderr 写 $2.err，回 rc（set -e 无关，显式取）
run_track() {
    local track="$1" src="$2" out="$3"
    case "$track" in
    interp) "$PX" run "$src" > "$out" 2> "$out.err" ;;
    vm)     "$PX" build "$src" > "$out.build" 2>&1 || return $?
            local nm; nm="$(basename "$src" .px)"
            "$TMP/build/$nm" > "$out" 2> "$out.err" ;;
    c)      "$PX" build --c "$src" > "$out.build" 2>&1 || return $?
            local nm; nm="$(basename "$src" .px)"
            "$TMP/build/$nm" > "$out" 2> "$out.err" ;;
    *)      return 9 ;;
    esac
}

# ── X1：三轨跑主用例，先判 interp 语义，再逐字节对拍 ────────────────────────
if run_track interp "$TMP/main.px" "$TMP/i.out"; then
    # 判据必须**整行精确**（grep -x）：M392 首跑实测，`grep -m1 "^T5 "` 会抓到
    #   "T5 i=0"（前缀相同）⇒ 判据把「通过了」读成「没过」。这正是 CRITERIA 的「判据找不到先怀疑判据」。
    exp() { if grep -qx -- "$1 $2" "$TMP/i.out"; then ok "$1 = $2"; else bad "$1 期望整行 [$1 $2]，实得 [$1 $(grep -m1 -- "^$1 " "$TMP/i.out" | sed "s/^$1 //")]"; fi; }
    exp T1 "R1001: 未定义变量: 'nope_undefined'"
    exp T2 ok
    exp T3 inner
    exp T4 ok
    exp T5 end
    exp T6 ok
    exp T7 outer-ok
    grep -qx 'T3 after-inner' "$TMP/i.out" && ok "T3 after-inner 存在（内层捕后继续执行）" || bad "T3 内层捕后没有继续执行"
    grep -q '^T5 i=0$' "$TMP/i.out" && ok "T5 循环体内正常路径执行" || bad "T5 循环体正常路径缺失"
    grep -qx 'T5 caught 1' "$TMP/i.out" && ok "T5 捕获后 break 生效" || bad "T5 break 未生效（可能继续迭代）"
    grep -qx 'T2b BAD' "$TMP/i.out" && bad "T2 正常路径误进 handler" || ok "T2 正常路径未进 handler"
    grep -qx 'T8 noname' "$TMP/i.out" && ok "T8 无名 catch 可用" || bad "T8 无名 catch 失败"
    grep -qx 'ALL DONE' "$TMP/i.out" && ok "ALL DONE（无提前退出）" || bad "主体未跑完"
else
    bad "interp 主体失败：$(head -3 "$TMP/i.out.err" | tr '\n' ' ')"
fi

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

# ── N1：函数内 try + return，之后再触发错误 ⇒ 必须正常报错，不许段错误 ──────
cat > "$TMP/n1.px" <<'PXEOF'
def n1(flag):
    try:
        if flag:
            return 1
        print(nope_n1)
    catch e:
        return 2
    return 3
print("N1a " + str(n1(true)))
print("N1b " + str(n1(false)))
print(nope_after)
PXEOF
for tr in interp vm c; do
    run_track "$tr" "$TMP/n1.px" "$TMP/n1.$tr.out"; rc=$?
    if [ "$rc" = 139 ] || [ "$rc" = 134 ]; then
        bad "N1-$tr 段错误/中止（rc=$rc）—— 退出路径没弹帧"
    elif [ "$rc" = 0 ]; then
        bad "N1-$tr 未捕获错误却没报错（rc=0）—— 错误被吞了"
    elif ! grep -qx 'N1a 1' "$TMP/n1.$tr.out" || ! grep -qx 'N1b 2' "$TMP/n1.$tr.out"; then
        bad "N1-$tr 前两行不对（rc=$rc）：$(head -2 "$TMP/n1.$tr.out" | tr '\n' ' ')"
    elif ! grep -q 'R1001' "$TMP/n1.$tr.out.err"; then
        bad "N1-$tr 报错缺 R1001：$(head -2 "$TMP/n1.$tr.out.err" | tr '\n' ' ')"
    else
        ok "N1-$tr rc=$rc 正常报错、无段错误、前两行正确"
    fi
done

# ── N2：未捕获的错误仍然响亮（同 N1 的 rc/stderr 判据的另一形态）──────────
cat > "$TMP/n2.px" <<'PXEOF'
print("before")
print(nope_only)
PXEOF
for tr in interp vm c; do
    run_track "$tr" "$TMP/n2.px" "$TMP/n2.$tr.out"; rc=$?
    if [ "$rc" != 0 ] && grep -q 'R1001' "$TMP/n2.$tr.out.err" && grep -qx 'before' "$TMP/n2.$tr.out"; then
        ok "N2-$tr 未捕获错误响亮（rc=$rc · stderr 带 R1001）"
    else
        bad "N2-$tr 未捕获错误不响亮（rc=$rc）：$(head -2 "$TMP/n2.$tr.out.err" | tr '\n' ' ')"
    fi
done

# ── P：前提自证 —— 旧编译器必须拒绝本用例（否则本门没测到新语法）──────────
BASE_TAG="${PX_BASE_TAG:-v0.2.391}"
if git cat-file -e "$BASE_TAG:bootstrap/pxc" 2>/dev/null; then
    git show "$BASE_TAG:bootstrap/pxc" > "$TMP/pxc_base" 2>/dev/null
    chmod +x "$TMP/pxc_base"
    if "$TMP/pxc_base" "$TMP/main.px" >/dev/null 2>&1; then
        bad "P 前提自证失败：$BASE_TAG 的编译器也能编译本用例 ⇒ 本门没有测到新语法"
    else
        ok "P 前提自证：$BASE_TAG 拒绝新语法（判据有牙）"
    fi
else
    echo "  ⏭  P 前提自证跳过（本地无标签 $BASE_TAG 的 bootstrap/pxc —— 未测「旧编译器会拒」）"
fi

echo "── m392_try_catch：通过 $pass · 失败 $fail"
[ "$fail" = 0 ] || exit 1
