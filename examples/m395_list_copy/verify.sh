#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M395 · 三种拷贝：别名 / 浅拷贝 / deep_copy（PX-009 的处置口径）
# ---------------------------------------------------------------------------
# 背景：清歌 2026-11-09 报「b = a 后改 b 会改 a，不报错、结果错」。用户 2026-11-12
#   决策：**保留引用语义**（与 Python/Go 同；值语义会把 O(n) 拷贝转嫁给每一轮跑门），
#   补的是「想拷一份时的明确写法」= deep_copy(x) + 文档前置。
# 判据（每条都配「改坏了必红」的差分断言）：
#   T1 浅拷贝 a[:] 断开第一层         T2 **浅拷贝边界正控**：a[:] 后改 b[0][0] 必须改到 a
#   T3 深拷贝三层全断开               T4 **别名负控**：b = a 后改 b 必须改到 a
#   T5 deep_copy 字典：键序保持 + 值断开
#   T6 标量/句柄原样（int/str/bool/null 不产生新语义）
#   N1 循环引用必须 R1002 且**不挂死**（判据带 timeout）
#   N2 判据自证：把 T3 的源换成浅拷贝，判据必须判红（证明 T3 有牙）
#   X1 三轨（interp / vm / c）逐字节一致
#   P  前提自证：上一版编译器（v0.2.394）必须不认 deep_copy
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
TMP="${TMPDIR:-/tmp}/m395_copy.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

# ── 主用例：T1–T6 ──────────────────────────────────────────────────────────
cat > "$TMP/main.px" <<'PXEOF'
# T1 浅拷贝：第一层断开
var a1 = [1, 2]
var b1 = a1[:]
b1[0] = 9
print("T1 " + str(a1[0]) + " " + str(b1[0]))

# T2 浅拷贝边界（正控）：内层仍共享 —— 这是语义本身，不是 bug
var a2 = [[1, 2], [3, 4]]
var b2 = a2[:]
b2[0][0] = 9
print("T2 " + str(a2[0][0]) + " " + str(b2[0][0]))

# T3 深拷贝：三层都断开
var a3 = [[[1, 2], [3]], [[4]]]
var c3 = deep_copy(a3)
c3[0][0][0] = 9
c3[1][0][0] = 9
print("T3 " + str(a3[0][0][0]) + " " + str(a3[1][0][0]) + " " + str(c3[0][0][0]))

# T4 别名负控：b = a 就是同一对象（不许被改成拷贝）
var a4 = [1, 2]
var b4 = a4
b4[0] = 9
print("T4 " + str(a4[0]) + " " + str(b4[0]))

# T5 字典：键序保持 + 值断开
var d5 = {"b": [1], "a": [2], "c": 3}
var e5 = deep_copy(d5)
e5["b"][0] = 9
e5["c"] = 7
var ks = []
for k in e5:
    ks.append(k)
print("T5 " + join(",", ks) + " " + str(d5["b"][0]) + " " + str(d5["c"]) + " " + str(e5["b"][0]))

# T6 标量/不可变原样
print("T6 " + str(deep_copy(5)) + " " + deep_copy("x") + " " + str(deep_copy(true)) + " " + str(deep_copy(null)))
print("ALL DONE")
PXEOF

# ── 三轨执行器 ─────────────────────────────────────────────────────────────
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

# 判据函数（T3 单列出来，供 N1 自证复用）
judge_t3() {
    grep -qx 'T3 1 4 9' "$1" && grep -q '^T3 ' "$1"
}

if run_track interp "$TMP/main.px" "$TMP/i.out"; then
    exp() { if grep -qx -- "$1" "$TMP/i.out"; then ok "$1"; else bad "期望整行 [$1]，实得 [$(grep -m1 -- "^${1%% *} " "$TMP/i.out")]"; fi; }
    exp "T1 1 9"
    exp "T2 9 9"
    exp "T4 9 9"
    exp "T5 b,a,c 1 3 9"
    exp "T6 5 x true null"
    if judge_t3 "$TMP/i.out"; then ok "T3 深拷贝三层断开（源 1/4 保持，副本 9）"; else bad "T3 深拷贝未断开：$(grep -m1 '^T3 ' "$TMP/i.out")"; fi
    grep -qx 'ALL DONE' "$TMP/i.out" && ok "ALL DONE（无提前退出）" || bad "主体未跑完"
else
    bad "interp 主体失败：$(head -3 "$TMP/i.out.err" | tr '\n' ' ')"
fi

# ── N1：循环引用必须 R1002，且不许挂死 ────────────────────────────────────
#   ⚠️ 超时口径：这里**同时包住编译**（vm/c 轨要 px build 一次），所以不能用 20s。
#     首跑实测（2026-11-12 15:45）：rebake 之后 devbuild 缓存刚失效，同一份源首编
#     >20s ⇒ 判据把「在编译」读成「挂死」（假红）。90s 只留作「真死循环」的兜底。
N1_TIMEOUT=${N1_TIMEOUT:-90}
cat > "$TMP/n1.px" <<'PXEOF'
var a = []
a.append(a)
print("before")
var c = deep_copy(a)
print("BAD reached " + str(len(c)))
PXEOF
for tr in interp vm c; do
    timeout "$N1_TIMEOUT" bash -c "$(declare -f run_track); PX='$PX'; TMP='$TMP'; run_track '$tr' '$TMP/n1.px' '$TMP/n1.$tr.out'" >/dev/null 2>&1
    rc=$?
    if [ "$rc" = 124 ]; then
        bad "N1-$tr 挂死（timeout ${N1_TIMEOUT}s）—— 循环引用没被检测"
    elif [ "$rc" = 0 ]; then
        bad "N1-$tr 循环引用被静默复制（rc=0）—— 判据没有牙"
    elif grep -q 'R1002' "$TMP/n1.$tr.out.err" && grep -qx 'before' "$TMP/n1.$tr.out"; then
        ok "N1-$tr 循环引用报 R1002 且不挂死（rc=$rc）"
    else
        bad "N1-$tr 未报 R1002：rc=$rc err=$(head -2 "$TMP/n1.$tr.out.err" | tr '\n' ' ')"
    fi
done

# ── N2：判据自证 —— 把 T3 换成浅拷贝，jam 判据必须红 ──────────────────────
cat > "$TMP/n2.px" <<'PXEOF'
var a3 = [[[1, 2], [3]], [[4]]]
var c3 = a3[:]
c3[0][0][0] = 9
c3[1][0][0] = 9
print("T3 " + str(a3[0][0][0]) + " " + str(a3[1][0][0]) + " " + str(c3[0][0][0]))
PXEOF
if "$PX" run "$TMP/n2.px" > "$TMP/n2.out" 2>"$TMP/n2.err"; then
    if judge_t3 "$TMP/n2.out"; then
        bad "N2 判据自证失败：浅拷贝也让 T3 判据通过 ⇒ 判据没牙（实得 $(grep -m1 '^T3 ' "$TMP/n2.out")）"
    else
        ok "N2 判据自证：浅拷贝版必红（实得 $(grep -m1 '^T3 ' "$TMP/n2.out")）"
    fi
else
    bad "N2 自证用例本身跑不起来：$(head -2 "$TMP/n2.err" | tr '\n' ' ')"
fi

# ── X1：三轨逐字节一致（T1–T6 全在主用例里）────────────────────────────────
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

# ── P：前提自证 —— 上一版**运行时**不认 deep_copy ──────────────────────────
#   ⚠️ 判据必须对准改动类型（2026-11-12 15:46 实测教训）：
#     首版我照 m392 的写法取 v0.2.394 的 bootstrap/pxc，判「旧编译器必须拒绝」——
#     结果**判红**：旧 pxc 照样编译通过。原因：deep_copy 不是**语法**，是**运行时内建**
#     （源码里只是一个普通标识符调用，旧编译器发射 px_get_global("deep_copy")，
#      而链接的是**当前** runtime ⇒ 照样能跑）。
#     ⇒ 对「新增内建」这类改动，前提自证只能拿**旧运行时**（v0.2.394 的 bootstrap/pxi，
#       它内嵌旧 runtime）来判：它必须报「未定义变量: 'deep_copy'」。
BASE_TAG="${PX_BASE_TAG:-v0.2.394}"
if git cat-file -e "$BASE_TAG:bootstrap/pxi" 2>/dev/null; then
    git show "$BASE_TAG:bootstrap/pxi" > "$TMP/pxi_base" 2>/dev/null
    chmod +x "$TMP/pxi_base"
    if "$TMP/pxi_base" "$TMP/main.px" > "$TMP/p.out" 2> "$TMP/p.err"; then
        bad "P 前提自证失败：$BASE_TAG 的运行时也认 deep_copy ⇒ 本门没测到新内建"
    elif grep -q 'deep_copy' "$TMP/p.err"; then
        ok "P 前提自证：$BASE_TAG 的运行时拒绝 deep_copy（$(head -1 "$TMP/p.err" | cut -c1-80)）"
    else
        bad "P 前提自证：$BASE_TAG 运行时失败了，但不是因为 deep_copy（$(head -1 "$TMP/p.err" | cut -c1-80)）"
    fi
else
    echo "  ⏭  P 前提自证跳过（本地无标签 $BASE_TAG 的 bootstrap/pxi）"
fi

echo "── m395_list_copy：通过 $pass · 失败 $fail"
[ "$fail" = 0 ] || exit 1
