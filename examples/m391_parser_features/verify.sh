#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M391 · parser 三件（缺陷 733–736）
# ---------------------------------------------------------------------------
# 判据分四组，每组都配「修法回退则必红」的差分断言（不靠固定行窗口）：
#   A 三元表达式      cond ? a : b（含嵌套右结合 / 出现在实参中 / 混合比较）
#   B 链式折行        行首 `.` 续行（`s` 换行 `.to_upper()`）
#   C 模块限定访问    import 后 `helper.f()` / `helper.VAR`，且**局部同名不劫持**
#   D 负控            ① `f()?` 仍是 Try（不是三元）② `f()? - 1` 不是三元
#                     ③ 未导入的别名不得被当成模块（.px 里写 xxx.yyy 仍走字段语义）
# 前提自证 P：本轮语法在**旧编译器**上必须失败 —— 证明用例真的走到了新语法
#            （否则「编译成功」可能只是用例本身被老语法接受，判据就白测了）。
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
TMP="${TMPDIR:-/tmp}/m391_pfeat.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

# ── 被测模块 ────────────────────────────────────────────────────────────────
cat > "$TMP/helper.px" <<'PXEOF'
let K = 5
G = 9
def addk(v):
    return v + K
def addg(v):
    return v + G
PXEOF

# ── A/B/C 主体 ─────────────────────────────────────────────────────────────
#   ⚠️ heredoc 必须**加引号**（'PXEOF'）：不加引号时 `? :` / `$(...)` 会被 shell 展开，
#   实测报 `?: command not found` 且用例被改写（判据静默失真）。路径用占位符替换。
cat > "$TMP/main.px" <<'PXEOF'
import "__HELPER__"

def okf():
    return Ok(10)

# A 三元
let n = 7
print("A1 " + (n > 5 ? "big" : "small"))
print("A2 " + (n < 5 ? "big" : "small"))
print("A3 " + str(1 > 2 ? 1 : 2 > 1 ? 10 : 20))
def g(x):
    return x + 1
print("A4 " + str(g(n > 3 ? 10 : 20)))

# B 链式折行
let s = "abc"
let up = s
    .to_upper()
print("B1 " + up)

# C 模块限定访问（扁平名必须照旧可用）
print("C1 " + str(helper.addk(1)))
print("C2 " + str(addk(1)))
print("C3 " + str(helper.K))
print("C4 " + str(helper.G))
def shadow():
    let K = 100
    return helper.K + K
print("C5 " + str(shadow()))

# D1 Try 后缀仍是 Try（Ok 解包）
print("D1 " + str(okf()?))
# D2 `? -` 不是三元（Try 后接减号）
print("D2 " + str(okf()? - 1))
PXEOF
sed -i "s|__HELPER__|$TMP/helper.px|" "$TMP/main.px"

OUT="$TMP/out.txt"
if ! "$PX" run "$TMP/main.px" > "$OUT" 2> "$TMP/err.txt"; then
    bad "主体用例编译/运行失败：$(head -3 "$TMP/err.txt" | tr '\n' ' ')"
else
    got() { grep -c "^$1 " "$OUT" >/dev/null 2>&1; grep "^$1 " "$OUT" | head -1 | sed "s/^$1 //"; }
    exp() { # $1=标签 $2=期望
        local v; v="$(got "$1")"
        if [ "$v" = "$2" ]; then ok "$1 = $2"; else bad "$1 期望 [$2] 实得 [$v]"; fi
    }
    exp A1 big
    exp A2 small
    exp A3 10
    exp A4 11
    exp B1 ABC
    exp C1 6
    exp C2 6
    exp C3 5
    exp C4 9
    exp C5 105
    exp D1 10
    exp D2 9
fi

# ── D3 负控：未导入的别名不是模块（字段语义不变，取不到属性就报错，不能静默变全局）──
cat > "$TMP/nons.px" <<'PXEOF'
let d = {"x": 1}
print(zzz.x)
PXEOF
if "$PX" run "$TMP/nons.px" > "$TMP/nons.out" 2> "$TMP/nons.err"; then
    if grep -q "^1$" "$TMP/nons.out"; then
        bad "D3 未导入的别名被当成模块（zzz.x 静默取到 1）—— 命名空间判据过宽"
    else
        ok "D3 未导入别名不被当成模块（输出非 1）"
    fi
else
    ok "D3 未导入别名报错（不静默）"
fi

# ── 前提自证 P：旧编译器必须拒绝本用例语法（证明用例真的测到了新语法）─────
#   基线件从 git 标签取（v0.2.390 是 M391 之前的最后一版），不额外入库大文件。
#   取不到标签 ⇒ 明确 skip，不假装测过（CRITERIA：负控必须有前提自证）。
BASE_TAG="${PX_BASE_TAG:-v0.2.390}"
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

echo "── m391_parser_features：通过 $pass · 失败 $fail"
[ "$fail" = 0 ] || exit 1
