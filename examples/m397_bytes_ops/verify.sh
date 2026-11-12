#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M397 · bytes 运算符面（PX-021 收口 · 缺陷 760）：`bytes + bytes` / `x in bytes`
# ---------------------------------------------------------------------------
# 背景（PX-021 原文）：「bytes 不能 +」。生态库写 crypto / 协议编码时第一个撞的就是它，
#   当时的绕行是 bytes_concat(b1, b2) —— 有内建、但运算符面缺口让代码读起来不像话。
#   `in` 同理：`b".." in some_bytes` 报告「右操作数不支持类型 bytes」。
#
# 设计（三轨同轮，语义只有一份写法）：
#   · `bytes + bytes` ⇒ **bytes**（不是 string）。单次分配、data 紧随 LXObject、
#     内嵌 NUL 原样、长度按 as.str.len（绝不用 strlen）。刻意**不**接受 bytes + string ——
#     “str→bytes 无损、bytes→str 有损”，谁转谁说不清 ⇒ 响亮报错，让用户自己写 bytes_concat。
#   · `x in bytes` ⇒ **二进制安全子串**判定，契约与 `bytes_find(b, sub[, start])` 对齐：
#     needle 收 string 或 bytes（同内存表示），按字节长度取边界（内嵌 NUL 不截断）。
#   · 实现落点：编译轨 runtime.c 的 px_add / px_membership_probe（VM 与 C 轨共用），
#     解释轨 selfhost/ival.px 的 i_bin_add / i_membership。**没有第二份语义**。
#
# 判据：
#   T1–T8 正向：拼接 / 类型 / NUL / 空边界 / 两种 needle / NUL needle / 回归锚点 / 多参版
#   X1    三轨（interp / vm / c）**逐字节**一致
#   N1    负控：`bytes + string` 三轨都必须 R1002 且文案含「无法相加: bytes + string」
#         （这条是**防复发**判据：谁把它改成隐式转换，静默类型漂移就会在这里变红）
#   N2    负控：`1 in bytes` 三轨都必须 R1002 且文案含「需要 string 或 bytes」（措辞要与契约一致）
#   P     前提自证：上一版运行时（v0.2.396 的 bootstrap/pxi，内嵌旧 runtime + 旧解释器）
#         跑同一份 main.px **必须失败** —— 否则说明本门没测到任何新东西
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
SRC="$ROOT/examples/m397_bytes_ops"
TMP="${TMPDIR:-/tmp}/m397_bops.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

run_track() {
    local track="$1" src="$2" out="$3"
    # 源先拷进 $TMP：px build 把产物写到 <源目录>/build/ ⇒ 不许污染仓库工作树
    local base; base="$(basename "$src")"
    cp "$src" "$TMP/$base" || return 9
    case "$track" in
    interp) "$PX" run "$TMP/$base" > "$out" 2> "$out.err" ;;
    vm)     "$PX" build "$TMP/$base" > "$out.build" 2>&1 || return $?
            local nm; nm="$(basename "$src" .px)"
            "$TMP/build/$nm" > "$out" 2> "$out.err" ;;
    c)      "$PX" build --c "$TMP/$base" > "$out.build" 2>&1 || return $?
            local nm; nm="$(basename "$src" .px)"
            "$TMP/build/$nm" > "$out" 2> "$out.err" ;;
    *)      return 9 ;;
    esac
}

# ── T1–T8：正向语义（解释轨逐行比对期望） ─────────────────────────────────
if run_track interp "$SRC/main.px" "$TMP/i.out"; then
    exp() {
        if grep -qx -- "$1" "$TMP/i.out"; then ok "$1"
        else bad "期望整行 [$1]，实得 [$(grep -m1 -- "^${1%% *} " "$TMP/i.out")]"; fi
    }
    exp "T1 abcd bytes 4"
    exp "T2 6 eAB5AHoh"
    exp "T3 2 2 bytes"
    exp "T4 true true false"
    exp "T5 true true"
    exp "T6 true true"
    exp "T7 true 4 97 YWI="
    exp "T8 YWJjZGVm YWJjZGVm"
else
    bad "interp 主体失败：$(head -3 "$TMP/i.out.err" 2>/dev/null | tr '\n' ' ')"
fi

# ── X1：三轨逐字节一致 ────────────────────────────────────────────────────
for tr in vm c; do
    if run_track "$tr" "$SRC/main.px" "$TMP/$tr.out"; then
        if cmp -s "$TMP/i.out" "$TMP/$tr.out"; then
            ok "X1-$tr 与 interp 逐字节一致（$(wc -l < "$TMP/$tr.out") 行）"
        else
            bad "X1-$tr 与 interp 不一致：$(diff "$TMP/i.out" "$TMP/$tr.out" | head -4 | tr '\n' ' ')"
        fi
    else
        bad "$tr 轨失败：$(head -3 "$TMP/$tr.out.err" 2>/dev/null | tr '\n' ' ')$(head -3 "$TMP/$tr.out.build" 2>/dev/null | tr '\n' ' ')"
    fi
done

# ── N1 / N2：负控（三轨都得响亮，且文案对得上） ────────────────────────────
neg() {
    local file="$1" want="$2" label="$3"
    for tr in interp vm c; do
        if run_track "$tr" "$SRC/$file" "$TMP/$label.$tr.out"; then
            bad "$label-$tr 未报错（期望 R1002）⇒ 静默通过"
        elif grep -q 'R1002' "$TMP/$label.$tr.out.err" && grep -qF -- "$want" "$TMP/$label.$tr.out.err"; then
            ok "$label-$tr 响亮（R1002 · $want）"
        else
            bad "$label-$tr 报错但文案不符：$(head -2 "$TMP/$label.$tr.out.err" | tr '\n' ' ')"
        fi
    done
}
neg neg_add.px "无法相加: bytes + string" N1
neg neg_in.px  "需要 string 或 bytes"      N2

# ── P：前提自证 —— 上一版运行时必须不认这两个运算符 ───────────────────────
#   为什么拿 bootstrap/pxi（解释器）而不是 pxc：pxi 内嵌**旧 runtime + 旧解释器**，
#   一次覆盖两条轨；pxc 只证明「旧编译器能编译」（运算符是运行期语义，编译照样过）。
BASE_TAG="${PX_BASE_TAG:-v0.2.396}"
if git cat-file -e "$BASE_TAG:bootstrap/pxi" 2>/dev/null; then
    git show "$BASE_TAG:bootstrap/pxi" > "$TMP/pxi_base" 2>/dev/null
    chmod +x "$TMP/pxi_base"
    cp "$SRC/main.px" "$TMP/pmain.px"
    if "$TMP/pxi_base" "$TMP/pmain.px" > "$TMP/p.out" 2> "$TMP/p.err"; then
        bad "P 前提自证失败：$BASE_TAG 的运行时**也**认 bytes 运算符 ⇒ 本门没测到新东西"
    elif grep -q '无法相加: bytes + bytes' "$TMP/p.err"; then
        ok "P 前提自证：$BASE_TAG 运行时拒绝 bytes + bytes（$(head -1 "$TMP/p.err" | cut -c1-70)）"
    else
        bad "P 前提自证：$BASE_TAG 运行时失败了，但不是因为 bytes 运算符（$(head -1 "$TMP/p.err" | cut -c1-70)）"
    fi
else
    echo "  ⏭  P 前提自证跳过（本地无标签 $BASE_TAG 的 bootstrap/pxi）"
fi

echo "── m397_bytes_ops：通过 $pass · 失败 $fail"
[ "$fail" = 0 ] || exit 1
