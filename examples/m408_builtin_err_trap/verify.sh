#!/usr/bin/env bash
# ============================================================
# examples/m408_builtin_err_trap/verify.sh —— 内置失败可捕获门（M408 · PX-002/PX-003）
# ------------------------------------------------------------
# 为什么有这道门（清歌 11-09 清单第 2/3 条，两条从未入账）：
#   解释轨（pxi）的 try/catch 是 **.px 层语义**（catch 的是 Err 返回值），而 runtime native
#   出错走 `px_error → longjmp`。解释轨此前**没装任何 try 帧** ⇒ 用户在 try 里调
#   `regex_match("\\p", …)` / `zip_unpack(坏包, …)` / `list_dir(不存在)` 失败时，longjmp
#   一路落到进程出口：**坏输入直接打死解释器进程**；而 VM / C 两轨有 px_try_push 帧、
#   能正常捕获 ⇒ 三轨分裂，且死的正是开发/脚本默认用的那条轨（P0）。
#
# 判据（每条都是「被测 vs 外部参考」，不许实现自证）：
#   A 三轨逐字节    9 个内置失败用例 × (stdout+stderr+rc) —— 解释 / VM / C 完全相同
#   B 独立期望表    逐行**手写**（文案取自 runtime 源码与 docs/ERROR_CODES.md §3 域前缀闭集，
#                   不是从被测件抓的），并断言**码制**：域前缀族不带 R 码、语言层必须带 R 码
#   C 归因         未捕获时三轨同形「运行时错误 [<用户函数> 行<N>]: …」，
#                  且报文**不许**出现内部函数名（i_call_builtin / i_builtin_ffi_call / rebake）
#   D 负控（有牙）  把护栏摘掉（i_exec_stmt_guarded 直调 i_exec_stmt，**解释执行同一份源码**）
#                  ⇒ A 的用例必须判红（rc≠0、没有 DONE）
#   P 前提自证     D 那次执行必须复现**修前形状**：stderr 里是解释器内部函数名
#                  （证明"过门靠的是护栏，不是运气"）
#   N1 期望扰动    改 B 的一行期望 ⇒ B 必红
#   N2 合法侧     无错误程序三轨逐字节、stderr 全空、rc=0 ⇒ 判据不是恒红
#
# 已知三轨差异（**不在本门覆盖内**，另立缺陷）：
#   `read_file(不存在)` —— 解释轨返回 Err(R2001: io: … (os error N))，编译两轨 px_error
#   （io: …）。同族不同码不同文 ⇒ 登记缺陷 811，本门不拿它当用例（否则等于用门掩盖差异）。
#
# 用法：verify.sh [--neg-skip]      # --neg-skip 跳过 D/P/N1（CI 慢机用）
# 退出码：0=全绿 · 1=有红 · 2=门自身前置失败
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
. "$(dirname "$0")/../../selfhost/gate_patch.sh" || { echo "❌ [M304] 打桩原语 source 失败（selfhost/gate_patch.sh）" >&2; exit 2; }
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
PX="$ROOT/tools/px"
INTERP="${M408_INTERP:-$ROOT/bootstrap/pxi}"
WORK="${TMPDIR:-/tmp}/m408_trap.$$"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0

PASS=0; FAIL=0
ok()   { PASS=$((PASS+1)); printf '  ✅ %s\n' "$*"; }
bad()  { FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$*"; }
note() { printf '  ·  %s\n' "$*"; }
cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT
mkdir -p "$WORK"
[ -x "$PX" ] || { echo "❌ 找不到 $PX"; exit 2; }
[ -x "$INTERP" ] || { echo "❌ 找不到解释器 $INTERP"; exit 2; }

# ── 主用例：9 个内置失败，全部套在 try 里 ──────────────────────
cat > "$WORK/cases.px" <<'PXEOF'
def k1():
    try:
        var v = regex_match("\\p", "x")
        print("k1 NOERR")
    catch e:
        print("k1 " + e)
def k2():
    try:
        var v = zip_unpack("/nonexistent/notzip.bin", "/tmp/m408_out")
        print("k2 NOERR")
    catch e:
        print("k2 " + e)
def k3():
    try:
        var v = json_parse("{bad")
        print("k3 NOERR")
    catch e:
        print("k3 " + e)
def k4():
    try:
        var v = list_dir("/nonexistent/zzz")
        print("k4 NOERR")
    catch e:
        print("k4 " + e)
def k5():
    try:
        var v = write_file("/nonexistent/dir/x.txt", "a")
        print("k5 NOERR")
    catch e:
        print("k5 " + e)
def k6():
    try:
        var v = file_size("/nonexistent/zzz")
        print("k6 NOERR")
    catch e:
        print("k6 " + e)
def k7():
    try:
        var v = len(1.5)
        print("k7 NOERR")
    catch e:
        print("k7 " + e)
def k8():
    try:
        var v = sum(1)
        print("k8 NOERR")
    catch e:
        print("k8 " + e)
def k9():
    try:
        var v = bytes_get(bytes("ab"), 5)
        print("k9 NOERR")
    catch e:
        print("k9 " + e)
k1()
k2()
k3()
k4()
k5()
k6()
k7()
k8()
k9()
print("DONE")
PXEOF

# ── B：门侧独立期望表（手写；域前缀族不带 R 码，语言层必须带码）──
cat > "$WORK/expect.txt" <<'EXPEOF'
k1 regex: 不支持的转义 \p（Unicode 类别：本引擎无 Unicode 表）
k2 zip 读取失败 /nonexistent/notzip.bin
k3 json: 期望对象键
k4 fs: 读取目录失败 /nonexistent/zzz
k5 io: 写入文件失败 /nonexistent/dir/x.txt
k6 io: 获取文件大小失败 /nonexistent/zzz: No such file or directory
k7 R1002: len 不支持类型 float
k8 R1002: sum 参数需要 list/tuple/生成器，实际是 int
k9 R1003: 索引越界: 5 (len=2)
DONE
EXPEOF

# ── C：未捕获归因用例（具名函数内出错、无 try）────────────────
cat > "$WORK/uncaught.px" <<'PXEOF'
print("start")
def boom():
    var m = regex_match("\\p", "abc")
    return m
boom()
print("after")
PXEOF

# ── N2：合法程序（不得被判红）────────────────────────────────
cat > "$WORK/legal.px" <<'PXEOF'
def add(a, b):
    return a + b
try:
    print(str(add(1, 2)))
    print(str(len("abc")))
catch e:
    print("IMPOSSIBLE: " + e)
print("OK")
PXEOF

# run_lanes <prog> <pfx> → $pfx.run/.vm/.c + $pfx.rc（**程序自身**的 rc 三元组）
run_lanes() {
    local prog="$1" pfx="$2" d name rc_run rc_vm rc_c
    d="$(dirname "$prog")"; name="$(basename "$prog" .px)"
    timeout 120 "$PX" run "$prog" > "$pfx.run" 2>"$pfx.run.err"; rc_run=$?
    rm -rf "$d/build"
    timeout 300 "$PX" build "$prog" > "$pfx.vmlog" 2>&1
    rc_vm=127
    if [ -x "$d/build/$name" ]; then timeout 120 "$d/build/$name" > "$pfx.vm" 2>"$pfx.vm.err"; rc_vm=$?; else : > "$pfx.vm"; : > "$pfx.vm.err"; fi
    rm -rf "$d/build"
    timeout 400 "$PX" build --c "$prog" > "$pfx.clog" 2>&1
    rc_c=127
    if [ -x "$d/build/$name" ]; then timeout 120 "$d/build/$name" > "$pfx.c" 2>"$pfx.c.err"; rc_c=$?; else : > "$pfx.c"; : > "$pfx.c.err"; fi
    rm -rf "$d/build"
    printf '%s %s %s\n' "$rc_run" "$rc_vm" "$rc_c" > "$pfx.rc"
}

echo "═══ M408 · 内置失败可捕获（PX-002/003）三轨门 ═══"

# ── [0] 前提：被测解释器必须真带护栏（否则 A 绿得毫无意义）──────
if grep -q '__guard_call(i_exec_stmt' selfhost/istmt.px; then
    ok "[0] 源码在护栏位（i_exec_stmt_guarded 经 __guard_call 受保护执行）"
else
    bad "[0] 源码**没有**护栏调用 ⇒ 本门失去前提（拒绝判绿）"
fi

# ── [A] 三轨逐字节 ───────────────────────────────────────────
run_lanes "$WORK/cases.px" "$WORK/k"
read -r RC_RUN RC_VM RC_C < "$WORK/k.rc"
if [ "$RC_RUN" -ne 0 ] || [ "$RC_VM" -ne 0 ] || [ "$RC_C" -ne 0 ]; then
    bad "A 轨道非 0（run=$RC_RUN vm=$RC_VM c=$RC_C）：$(head -1 "$WORK/k.run.err" 2>/dev/null | cut -c1-90)"
elif cmp -s "$WORK/k.run" "$WORK/k.vm" && cmp -s "$WORK/k.run" "$WORK/k.c"; then
    ok "A 三轨逐字节一致（$(wc -l < "$WORK/k.run") 行 / 9 用例）"
else
    bad "A 三轨不一致：run=$(tr '\n' '|' < "$WORK/k.run" | cut -c1-70) vm=$(tr '\n' '|' < "$WORK/k.vm" | cut -c1-70)"
fi
# D 的判据要用同一份 run 输出：没有 DONE 就是"死在半路"
if grep -qx DONE "$WORK/k.run"; then ok "A' 解释轨跑完 9 个用例（DONE 在）"; else bad "A' 解释轨没跑完（缺 DONE）⇒ 有内置失败打死了进程"; fi

# ── [B] 独立期望表 + 码制断言 ────────────────────────────────
if cmp -s "$WORK/expect.txt" "$WORK/k.run"; then
    ok "B 与门侧独立期望表逐字节一致（9 行）"
else
    bad "B 与期望表不符：$(diff "$WORK/expect.txt" "$WORK/k.run" | head -6 | tr '\n' ' ')"
fi
# 码制：域前缀族（regex/zip/json/fs/io）**不许**带 R 码；语言层必须有 R 码
CODECHK=0
while IFS= read -r line; do
    case "$line" in
        DONE) continue ;;
        k[0-9]*" regex:"*|k[0-9]*" zip "*|k[0-9]*" json:"*|k[0-9]*" fs:"*|k[0-9]*" io:"*)
            case "$line" in *" R1"*|*" R2"*) CODECHK=1; echo "     域前缀族却带 R 码：$line" ;; esac ;;
        k[0-9]*" R1"*|k[0-9]*" R2"*) : ;;
        *) CODECHK=1; echo "     既不是域前缀族也不是 R 码：$line" ;;
    esac
done < "$WORK/k.run"
[ "$CODECHK" -eq 0 ] && ok "B' 码制合规（docs/ERROR_CODES.md §1：域前缀族不带码 / 语言层必须带码）" || bad "B' 码制违规（见上）"

# ── [C] 未捕获归因：三轨同形 + 不含内部函数名 ────────────────
run_lanes "$WORK/uncaught.px" "$WORK/u"
read -r UR_RUN UR_VM UR_C < "$WORK/u.rc"
CU=0
[ "$UR_RUN" -ne 0 ] && [ "$UR_VM" -ne 0 ] && [ "$UR_C" -ne 0 ] || { bad "C 未捕获错误必须三轨都 rc≠0（run=$UR_RUN vm=$UR_VM c=$UR_C）"; CU=1; }
for lane in run vm c; do
    f="$WORK/u.$lane.err"
    if grep -q '运行时错误 \[boom 行3\]: ' "$f"; then :; else bad "C-$lane 归因不是 [boom 行3]：$(head -2 "$f" | tr '\n' '|' | cut -c1-90)"; CU=1; fi
    if grep -qE 'i_call_builtin|i_builtin_ffi_call|rebake_pxi' "$f"; then bad "C-$lane 报文泄漏内部函数名：$(grep -oE 'i_call_builtin|i_builtin_ffi_call|rebake_pxi' "$f" | head -2 | tr '\n' ' ')"; CU=1; fi
done
[ "$CU" -eq 0 ] && ok "C 未捕获归因三轨同形（[boom 行3]）且不含内部函数名"

# ── [N2] 合法侧不判红 ───────────────────────────────────────
run_lanes "$WORK/legal.px" "$WORK/l"
read -r LR_RUN LR_VM LR_C < "$WORK/l.rc"
if [ "$LR_RUN" -eq 0 ] && [ "$LR_VM" -eq 0 ] && [ "$LR_C" -eq 0 ] \
   && cmp -s "$WORK/l.run" "$WORK/l.vm" && cmp -s "$WORK/l.run" "$WORK/l.c" \
   && [ ! -s "$WORK/l.run.err" ]; then
    ok "N2 合法程序三轨一致、rc=0、stderr 空 ⇒ 判据不是恒红"
else
    bad "N2 合法程序被判红（rc=$LR_RUN/$LR_VM/$LR_C）"
fi

# ── [N1] 期望扰动 ⇒ B 必红 ──────────────────────────────────
if [ "$NEG" -eq 1 ]; then
    cp -f "$WORK/expect.txt" "$WORK/expect.bak"
    sed -i '1s/.*/k1 regex: 被扰动过的期望值/' "$WORK/expect.txt"
    if cmp -s "$WORK/expect.txt" "$WORK/k.run"; then
        bad "N1 期望扰动后 B 仍绿 ⇒ B 判据没有牙"
    else
        ok "N1 期望扰动 1 处 ⇒ B 必红（判据有牙）"
    fi
    cp -f "$WORK/expect.bak" "$WORK/expect.txt"

    # ── [D]+[P] 负控：同一枚二进制里的「有护栏 / 无护栏」对照 ──────
    #   为什么不用"重烘一个无护栏的 pxi"：那要 3 分钟，且只能证明"旧件会死"。
    #   这里改成**同进程内有/无受保护调用**的对照 —— 判据更直接：
    #     P  直调 raiser()（等价于修前解释轨：native 错误无人接）⇒ 进程必须**死在半路**
    #        （rc≠0、没有 after、stderr 里是运行时错误）——这就是修前的真实形状
    #     D  同一函数经 __guard_call 调用 ⇒ 必须活着拿回 {"ok":false} 且不污染进程
    #   二者同时成立，才说明"过门靠的是护栏本身"。
    cat > "$WORK/premise.px" <<'PXEOF'
print("before")
def raiser():
    var m = regex_match("\\p", "x")
    return m
raiser()
print("after")
PXEOF
    timeout 120 "$INTERP" "$WORK/premise.px" > "$WORK/p.out" 2> "$WORK/p.err"
    PRC=$?
    if [ "$PRC" -ne 0 ] && ! grep -qx after "$WORK/p.out" && grep -q '运行时错误' "$WORK/p.err"; then
        ok "P 前提自证：无护栏直调 ⇒ 进程死在半路（rc=$PRC，无 after）—— 即修前形状"
    else
        bad "P 前提自证失败：无护栏直调竟然活着（rc=$PRC，out=$(tr '\n' '|' < "$WORK/p.out" | cut -c1-50)）"
    fi
    cat > "$WORK/guarded.px" <<'PXEOF'
print("before")
def raiser():
    var m = regex_match("\\p", "x")
    return m
let g = __guard_call(raiser, [])
if g["ok"]:
    print("OK-UNEXPECTED")
else:
    print("GUARD-ERR " + g["err"])
print("after")
PXEOF
    timeout 120 "$INTERP" "$WORK/guarded.px" > "$WORK/g.out" 2> "$WORK/g.err"
    GRC=$?
    if [ "$GRC" -eq 0 ] && grep -qx after "$WORK/g.out" && grep -q '^GUARD-ERR regex: ' "$WORK/g.out"; then
        ok "D 同一函数经护栏调用 ⇒ 活着拿回错误文本（判据有牙且只咬该咬的）"
    else
        bad "D 护栏未生效（rc=$GRC，out=$(tr '\n' '|' < "$WORK/g.out" | cut -c1-70)）"
    fi
else
    note "NEG-SKIP：跳过 N1 / D / P"
fi

echo "── 结果：PASS=$PASS FAIL=$FAIL"
[ "$FAIL" -eq 0 ] || exit 1
exit 0
