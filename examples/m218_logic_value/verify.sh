#!/usr/bin/env bash
# ============================================================
# M218 门（第 97 轮）：`and` / `or` 的**返回值**语义统一（缺陷 310）
# ------------------------------------------------------------
# 病灶：**三轨分叉** —— 解释轨返回**布尔**，编译两轨（runtime 的
#   `px_and(a,b) = truthy(a) ? b : a` / `px_or(a,b) = truthy(a) ? a : b`）返回**操作数本身**。
#   实测（修前）：
#     `1 and 2`      解释 `true`   ⇄ VM/C `2`
#     `0 or 3`       解释 `true`   ⇄ VM/C `3`
#     `"" or "x"`    解释 `true`   ⇄ VM/C `"x"`
#   而文档只规定了「**短路**」（`a and b` 的右操作数不被提前求值），
#   **没规定返回什么** ⇒ 未文档化 + 三轨分叉。
#   影响面：`x = a or default` / `x = cond and value` 是**极常见**写法
#   ⇒ 解释轨下静默拿到 `true`（错值，不报错）。
#
# 修法：解释轨 `i_eval_binary_node` 的四条返回路径**逐字对齐** runtime：
#   假 ⇒ 返回左操作数 / 真 ⇒ 返回右操作数，**短路位置不变**。
#
# 层：
#   ① 工具自证（语料漂移 · 规模下限 · 形态）
#   ② 正判据：三轨对拍（30 例）+ 与 Python 独立真值一致（Python 的 and/or 即此语义）
#   ③ 短路举证（副作用日志直接观测右操作数**未被求值**）
#   ④ 负控 A：把解释轨改回**布尔** ⇒ 必须判红
#   ⑤ 负控 B：把 **runtime** 的 px_and/px_or 改成布尔 ⇒ 必须判红（编译两轨）
#   ⑥ 负控 C：判据自伤（比对恒真）⇒ ④ 的红必须消失
#   ⑦ 覆盖边界登记（如实）
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
W=$(mktemp -d /tmp/m218_gate.XXXXXX)

IEXPR="$ROOT/selfhost/iexpr.px"
CGE="$ROOT/selfhost/cg_expr.px"
RTC="$ROOT/runtime/runtime.c"
TT="$HERE/three_tracks.py"
SNAP="$W/snap"; mkdir -p "$SNAP"
cp -a "$IEXPR" "$SNAP/iexpr.px.snap"
cp -a "$CGE"   "$SNAP/cg_expr.px.snap"
cp -a "$RTC"   "$SNAP/runtime.c.snap"
cp -a "$TT"    "$SNAP/tt.snap"

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo; echo "── $*"; }

restore_all() {
    cp -a "$SNAP/iexpr.px.snap" "$IEXPR"
    cp -a "$SNAP/cg_expr.px.snap" "$CGE"
    cp -a "$SNAP/runtime.c.snap" "$RTC"
    cp -a "$SNAP/tt.snap"        "$TT"
}
trap 'restore_all; rm -rf "$W"' EXIT

build_dev() {
    timeout 900 bash "$ROOT/selfhost/devbuild.sh" pxc pxi --vm > "$W/devbuild.log" 2>&1
}
prov() {
    echo "   溯源：rtcache=$( (cd "$ROOT" && ./tools/px rtcache 2>/dev/null | tail -1) )"
    sha256sum /tmp/pxcdev /tmp/pxcdev_vm /tmp/pxidev 2>/dev/null | awk '{printf "         %s %s\n", substr($1,1,16), $2}'
}
run_tt() {
    local d="$1"; mkdir -p "$d"
    timeout 900 python3 "$TT" --root "$ROOT" --work "$d/tt" \
        --interp /tmp/pxidev --cbin /tmp/pxcdev --vmb /tmp/pxcdev_vm > "$d/tt.out" 2>&1
    echo $? > "$d/tt.rc"
}
nprob() { sed -n 's/^对拍 [0-9]* 例 · 通过 [0-9]* · 问题 \([0-9]*\)$/\1/p' "$1"; }

echo "M218 门 · 工作目录 $W"
[ "$NEG_SKIP" = "1" ] && note "（--neg-skip：跳过负控 ④⑤⑥）"

# ①
hdr "[1/7] 工具自证：语料**漂移检测**（gen_cases.py 是单一事实源）"
if python3 "$HERE/gen_cases.py" --out "$W" > "$W/gen.log" 2>&1; then
    for f in cases.px CASES.tsv; do
        if cmp -s "$W/$f" "$HERE/$f"; then
            note "$f 重生成与入库件逐字节一致 ✅（$(wc -l < "$HERE/$f") 行）"
        else
            bad "$f 与 gen_cases.py 不一致（改了生成器没重生成？）"
            diff "$W/$f" "$HERE/$f" | head -10
        fi
    done
    N=$(grep -c . "$HERE/CASES.tsv")
    [ "$N" -ge 20 ] || bad "语料规模低于下限（$N 例 < 20）⇒ 判据可能被架空"
    note "语料：$N 例（下限 20）"
    # 覆盖：非布尔操作数（这才是分叉面）+ 短路 + 惯用法
    for pat in 'ok_and_int_int' 'ok_or_empty_str' 'ok_short_and' 'ok_short_or' 'ok_default' 'ok_chain'; do
        grep -q "^$pat" "$HERE/CASES.tsv" || bad "语料缺关键用例：$pat"
    done
    note "语料覆盖「非布尔操作数 / 短路 / 默认值惯用法」✅"
else
    bad "gen_cases.py 执行失败"; tail -5 "$W/gen.log"
fi

# ②
hdr "[2/7] 正判据：三轨对拍（驱动器一次编译 + 逐 label 只跑不编）"
if build_dev; then
    note "devbuild 成功（/tmp/pxcdev · /tmp/pxcdev_vm · /tmp/pxidev）"; prov
else
    bad "devbuild 失败"; tail -20 "$W/devbuild.log"
fi
run_tt "$W/pos"
tail -3 "$W/pos/tt.out" | sed 's/^/   /'
if [ "$(cat "$W/pos/tt.rc" 2>/dev/null || echo 9)" = "0" ]; then
    note "三轨一致 + 与 Python 独立真值相符 ✅"
else
    bad "三轨对拍未过（见下）"
    grep -E '^▲|^    期望' "$W/pos/tt.out" | head -20
fi

# ③ 短路举证：右操作数的**副作用**不得出现（用同一份语料里的 SIDE 标记）
hdr "[3/7] 短路举证：右操作数的副作用**不得**出现"
SC_OK=1
for pair in "ok_short_and:0" "ok_short_or:1"; do
    lb=${pair%%:*}; want=${pair##*:}
    got=$(cd "$W/pos/tt" 2>/dev/null && true)
    out=$(timeout 120 /tmp/pxidev "$lb" "$HERE/cases.px" 2>&1 | head -1)
    if [ "$out" = "$want" ]; then
        note "$lb ⇒ [$out]（右操作数未求值：无 SIDE）✅"
    else
        bad "$lb ⇒ [$out]（期望 [$want]）⇒ 短路被破坏或返回值语义错"
        SC_OK=0
    fi
done
[ "$SC_OK" = "1" ] && note "短路与返回值**同时**成立 ✅"

# ④⑤⑥
if [ "$NEG_SKIP" != "1" ]; then
hdr "[4/7] 负控 A：把解释轨改回**布尔** ⇒ 必须判红"
restore_all
python3 - "$IEXPR" <<'PY'
import sys, io, re
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
n = 0
# ⚠️ 用 **re 容忍空白**：`pxfmt -w` 会把行内注释的对齐空格压掉，
#   首版写死原文 ⇒ 锚点失配（补丁点 0）⇒ 负控**假绿**。
PATS = [
    (r'return Ok\(l\)\s*# px_and: truthy\(a\) \? b : a ⇒ 假 ⇒ 返回 a', 'return Ok(false)   # NEGCTL-M218A'),
    (r'return Ok\(rr\.unwrap\(\)\)\s*# 真 ⇒ 返回 b', 'return Ok(i_truthy(rr.unwrap()))   # NEGCTL-M218A'),
    (r'return Ok\(l\)\s*# px_or: truthy\(a\) \? a : b ⇒ 真 ⇒ 返回 a', 'return Ok(true)   # NEGCTL-M218A'),
    (r'return Ok\(rr\.unwrap\(\)\)\s*# 假 ⇒ 返回 b', 'return Ok(i_truthy(rr.unwrap()))   # NEGCTL-M218A'),
]
for pat, rep in PATS:
    s2, k = re.subn(pat, rep, s, count=1)
    if k:
        s = s2; n += 1
io.open(p, 'w', encoding='utf-8').write(s)
print('NC-A 补丁点：%d（期望 4）' % n)
PY
if build_dev; then
    run_tt "$W/ncA"; prov
    NP=$(nprob "$W/ncA/tt.out")
    if [ "${NP:-0}" -ge 10 ]; then
        note "负控 A ✅ 解释轨退回布尔 ⇒ 问题 $NP 例（三轨分叉复现）"
    else
        bad "负控 A 未判红（问题 ${NP:-?} 例 ⇒ 判据无牙）"; head -20 "$W/ncA/tt.out"
    fi
else
    bad "负控 A：重编失败"
fi

hdr "[5/7] 负控 B：把 **C 轨**的 and/or 发射改成布尔 ⇒ 必须判红"
restore_all
# ⚠️ 靶心为什么不是 `px_and`/`px_or`：**它们是死代码** —— `and`/`or` 是**短路运算符**，
#   由 codegen 直接发射分支（C 轨 `cg_expr.px` 的 `?:`、VM 轨 `bc_emit.px` 的 JMPF/JMPT
#   + 覆写），**从不调用** `px_and`/`px_or`（全仓 grep 只有声明与注释）。
#   首版打它们 ⇒ 行为不变 ⇒ 负控**假绿**（实测「问题 0 例」）。
#   现靶心 = C 轨发射串外面裹一层 `px_bool(...)` ⇒ 返回布尔。
python3 - "$CGE" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
# 靶心：把 And 的「真 ⇒ 返回右操作数」改成「真 ⇒ 也返回左操作数」
#   ⚠️ 首版裹 `px_bool(...)` ⇒ **C 轨构建失败**（`px_bool` 不在发射可用的符号集里）
#      ⇒ 对拍脚本崩、`问题` 行缺失 ⇒ 门报「未判红（问题 ? 例）」——是**假红**，没意义。
#      现在只改三元表达式的**取值**，不动函数调用面 ⇒ 发射合法、行为可判。
old = 'px_is_truthy(" + t + ") ? " + r + " : " + t + "; })"'
new = 'px_is_truthy(" + t + ") ? " + t + " : " + t + "; })"   # NEGCTL-M218B'
n = s.count(old)
if n == 1:
    io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
print('NC-B 补丁点：%d（期望 1）' % n)
PY
if build_dev; then
    run_tt "$W/ncB"; prov
    NP=$(nprob "$W/ncB/tt.out")
    if [ "${NP:-0}" -ge 5 ]; then
        note "负控 B ✅ C 轨退回布尔 ⇒ 问题 $NP 例（分叉复现）"
    else
        bad "负控 B 未判红（问题 ${NP:-?} 例）"; tail -20 "$W/ncB/tt.out"
    fi
else
    bad "负控 B：重编失败"
fi

hdr "[6/7] 负控 C：判据自伤（比对/期望恒真）⇒ ④ 的红必须消失"
restore_all
python3 - "$TT" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
n = 0
if 'bad, nok = [], 0' in s:
    s = s.replace('bad, nok = [], 0',
                  'bad, nok = [], 0\n_BAD_SINK = lambda *a, **k: None', 1); n += 1
cnt = s.count('bad.append(')
s = s.replace('bad.append(', '_BAD_SINK(')
n += cnt
io.open(p, 'w', encoding='utf-8').write(s)
print('NC-C 补丁点：%d（sink 1 + bad.append %d）' % (n, cnt))
PY
python3 - "$IEXPR" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
s = s.replace('            return Ok(l)          # px_and: truthy(a) ? b : a ⇒ 假 ⇒ 返回 a',
              '            return Ok(false)   /* NEGCTL-M218A */')
s = s.replace('        return Ok(rr.unwrap())    # 真 ⇒ 返回 b',
              '        return Ok(i_truthy(rr.unwrap()))   /* NEGCTL-M218A */')
s = s.replace('            return Ok(l)          # px_or: truthy(a) ? a : b ⇒ 真 ⇒ 返回 a',
              '            return Ok(true)    /* NEGCTL-M218A */')
s = s.replace('        return Ok(rr.unwrap())    # 假 ⇒ 返回 b',
              '        return Ok(i_truthy(rr.unwrap()))   /* NEGCTL-M218A */')
io.open(p, 'w', encoding='utf-8').write(s)
PY
if build_dev; then
    run_tt "$W/ncC"
    NP=$(nprob "$W/ncC/tt.out")
    if [ "${NP:-9}" = "0" ]; then
        note "负控 C ✅ 判据自伤后不再判红 ⇒ ④ 的红确实来自比对逻辑"
    else
        bad "负控 C 失败：仍报问题 ${NP:-?} 例"
    fi
else
    bad "负控 C：重编失败"
fi
restore_all
build_dev && note "收尾：dev 件已按修复后源码重建 ✅"
else
    note "（④⑤⑥ 已跳过）"
fi

# ⑦
hdr "[7/7] 覆盖边界（如实登记）"
cat <<'EOF'
   · **本门无「拒绝侧」**：`and` / `or` 对**任意类型**都合法（不报错）⇒ 判据只有「返回值」这一面。
     「类型守卫」不在本缺陷面内（`and`/`or` 从不因类型报错）。
   · **未覆盖**：`??`（空合并）与 `?:`（三元）—— 它们也参与短路（M165 已登记同批），
     但**返回值语义**各自独立（`??` 返回非 null 侧、`?:` 返回分支值）⇒ 本次未动；
     若将来发现同款分叉，按同一口径收口。
   · **未覆盖**：`not`（一元）—— 它**本就**返回布尔（`not x` ⇒ bool），三轨一致（本门语料含回归例）。
   · **已登记的语义收紧**：解释轨的 `a and b` / `a or b` 在**非布尔操作数**上
     从「返回布尔」改为「返回操作数」⇒ `x = a or default` 这类写法在解释轨下**从错值变正确值**；
     在 `if a and b:` 这类**只关心真值**的位置上**行为不变**（布尔上下文里两者等价）。
     ⇒ 对生态是**修复**（编译两轨本来就是这个语义，而编译轨是默认档）。
EOF

echo
if [ "$fail" = "0" ]; then echo "M218-VERIFY-OK"; else echo "M218-VERIFY-FAIL"; fi
exit "$fail"
