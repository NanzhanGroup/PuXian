#!/usr/bin/env bash
# ============================================================
# M220 门（第 99 轮）：`?` / `!` / `?.` 三个 Result/Optional 运算符的**语义收口**
# ------------------------------------------------------------
# 为什么到这一轮才收：`and`/`or` 的返回值（M218）· 「此类型不支持 X」的词条（M203）·
#   求值**顺序**（M165）都收过，但 `?` / `!` / `?.` 全仓**几乎没有对拍用例**
#   （`??` 有 18 处，`?` 后缀与 `!` 极少）⇒ 三轨各写一份实现而**没有任何门看着**。
#
# 五个缺陷（修前实测）：
#   · 319（高 · C 轨静默重复副作用）`?.` 的接收者在发射串里写了**两遍**
#     ⇒ `f()?.a` 的副作用执行**两次**（解释/VM 轨 1 次 · C 轨 2 次）。
#   · 316（高 · 静默）顶层 `?` 遇 `null` ⇒ **VM 轨 rc=0 且零输出**（`print` 都没跑，
#     程序"成功"退出）；同一条路径的 `Err` 只印 `错误: <payload>`（丢措辞、无行号）= 317。
#   · 315（中）`!` 的失败文案三轨三种，且 C 轨**无 R 码、连 `Err` 的载荷都丢**。
#   · 320（本轮新登记 · 已 A/B 证明 **pre-existing**）：**闭包体在 C 轨不是「帧」** ——
#     顶层闭包体不压错误标签 ⇒ `len(cg_err_labels)==0` 被判成「顶层 ?」⇒ 报错（应传播）；
#     函数内闭包体 ⇒ `goto` 到**外层函数**的标签 ⇒ **C 编译失败**（`label … used but not defined`）。
#     `cg_gen_lambda`（生成器）有这套机制但**顺序错**（先生成体、后压标签）。
#   · 318（文档）：cheatsheet 写「顶层 `?` 传播 Err/None」与「`?:` 三元」，
#     而实际顶层 `?` 不可用、`?:` **不是本语言的运算符**（幽灵运算符 —— 它从文档
#     扩散进了 C 轨的产品错误消息 `错误传播 ?:`）。
#
# 层：
#   ① 工具自证（语料漂移 · 规模下限 · 关键覆盖）
#   ② 正判据：三轨对拍 26 例（合法侧逐字节一致 + 严格侧 rc≠0/同 R 码/同核心文案）
#   ③ 文档对拍（缺陷 318 的三个面）
#   ④ 负控 A：**忠实撤回** 319（`px_field(t` → `px_field(o`）⇒ 两道非 null 用例必须红
#      （⚠️ null 那例**按设计不会红** —— 见该层的注：三元只求值被选中的分支）
#   ⑤ 负控 B：撤回 320（闭包体不压错误标签）⇒ 必须判红
#   ⑥ 负控 C：撤回 316/317（VM 顶层标记判据）⇒ 必须判红
#   ⑦ 负控 D：判据自伤（比对恒真）⇒ ④ 的红必须**消失**
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
W=$(mktemp -d /tmp/m220_gate.XXXXXX)

CGE="$ROOT/selfhost/cg_expr.px"
CDG="$ROOT/selfhost/codegen.px"
BCE="$ROOT/selfhost/bc_emit.px"
VMC="$ROOT/runtime/vm.c"
TT="$HERE/three_tracks.py"
GEN="$HERE/gen_cases.py"
SNAP="$W/snap"; mkdir -p "$SNAP"

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo; echo "── $*"; }

# ⚠️ 纪律（M213/M214 教训）：每道负控**各自**先 restore 再 snap —— 各自干净起点；
#   否则第二道起「无备份可还原」⇒ 改源残留（M191 老病）。
snap_all() {
    cp -a "$CGE" "$SNAP/cg_expr.px.snap"
    cp -a "$CDG" "$SNAP/codegen.px.snap"
    cp -a "$BCE" "$SNAP/bc_emit.px.snap"
    cp -a "$VMC" "$SNAP/vm.c.snap"
    cp -a "$TT"  "$SNAP/tt.snap"
}
restore_all() {
    cp -a "$SNAP/cg_expr.px.snap" "$CGE"
    cp -a "$SNAP/codegen.px.snap" "$CDG"
    cp -a "$SNAP/bc_emit.px.snap" "$BCE"
    cp -a "$SNAP/vm.c.snap" "$VMC"
    cp -a "$SNAP/tt.snap" "$TT"
}
snap_all
trap 'restore_all; rm -rf "$W"' EXIT

build_dev() {
    timeout 900 bash "$ROOT/selfhost/devbuild.sh" pxc pxi --vm > "$W/devbuild.log" 2>&1
}
prov() {
    echo "   溯源：rtcache=$( (cd "$ROOT" && ./tools/px rtcache 2>/dev/null | tail -1) )"
    sha256sum /tmp/pxcdev /tmp/pxcdev_vm /tmp/pxidev 2>/dev/null | awk '{printf "         %s %s\n", substr($1,1,16), $2}'
}
run_tt() {   # $1=outdir $2=only（可空）
    local d="$1" only="${2:-}" args=()
    mkdir -p "$d"
    python3 "$GEN" --out "$d/gen" > "$d/gen.log" 2>&1
    [ -n "$only" ] && args=(--only "$only")
    timeout 1800 python3 "$TT" --root "$ROOT" --work "$d" --cases "$d/gen" \
        --interp /tmp/pxidev --cbin /tmp/pxcdev --vmb /tmp/pxcdev_vm \
        "${args[@]}" > "$d/tt.out" 2>&1
    echo $? > "$d/tt.rc"
}
nprob() { sed -n 's/^对拍 [0-9]* 例 · 通过 [0-9]* · 问题 \([0-9]*\)$/\1/p' "$1"; }
nran()  { sed -n 's/^对拍 \([0-9]*\) 例 · 通过 [0-9]* · 问题 [0-9]*$/\1/p' "$1"; }

echo "M220 门 · 工作目录 $W"
[ "$NEG_SKIP" = "1" ] && note "（--neg-skip：跳过负控 ④⑤⑥⑦）"

# ①
hdr "[1/7] 工具自证：语料**漂移检测**（gen_cases.py 是单一事实源）"
if python3 "$GEN" --out "$W/g0" > "$W/g0.log" 2>&1; then
    if cmp -s "$W/g0/CASES.tsv" "$HERE/CASES.tsv"; then
        note "CASES.tsv 重生成与入库件逐字节一致 ✅（$(grep -c . "$HERE/CASES.tsv") 例）"
    else
        bad "CASES.tsv 与 gen_cases.py 不一致（改了生成器没重生成？）"
        diff "$W/g0/CASES.tsv" "$HERE/CASES.tsv" | head -10
    fi
    NOK=$(grep -c $'\tok\t' "$HERE/CASES.tsv")
    NER=$(grep -c $'\terr\t' "$HERE/CASES.tsv")
    NT=$((NOK + NER))
    [ "$NT" -ge 24 ] || bad "语料规模低于下限（$NT 例 < 24）⇒ 判据可能被架空"
    [ "$NOK" -ge 18 ] || bad "合法侧不足（$NOK < 18）"
    [ "$NER" -ge 4 ] || bad "严格侧不足（$NER < 4）"
    note "语料：$NT 例（合法 $NOK · 严格 $NER；下限 24/18/4）"
    for pat in ok_opt_side_effect ok_opt_side_effect_null ok_clos_toplevel_err \
               ok_clos_in_fn err_try_toplevel_null err_try_toplevel_err \
               err_force_null err_force_err; do
        grep -q "^$pat	" "$HERE/CASES.tsv" || bad "语料缺关键用例：$pat"
    done
    note "语料覆盖「319 副作用 / 320 闭包帧 / 316-317 顶层 ? / 315 ! 文案」✅"
else
    bad "gen_cases.py 执行失败"; tail -5 "$W/g0.log"
fi

# ②
hdr "[2/7] 正判据：三轨对拍（26 例**独立小程序** —— 上下文忠实，见 gen_cases.py 头注）"
if build_dev; then
    note "devbuild 成功（/tmp/pxcdev · /tmp/pxcdev_vm · /tmp/pxidev）"; prov
else
    bad "devbuild 失败"; tail -20 "$W/devbuild.log"
fi
run_tt "$W/pos"
tail -2 "$W/pos/tt.out" | sed 's/^/   /'
if [ "$(cat "$W/pos/tt.rc" 2>/dev/null || echo 9)" = "0" ]; then
    note "三轨一致 + 与期望值相符 ✅（$(nran "$W/pos/tt.out") 例）"
else
    bad "三轨对拍未过（见下）"
    grep -E '^▲|^    期望|^    [a-z]+ +rc=' "$W/pos/tt.out" | head -24
fi

# ③ 文档对拍（缺陷 318）
hdr "[3/7] 文档对拍：幽灵运算符与错误表述必须已清除"
CH="$ROOT/docs/PUXIAN_CHEATSHEET.md"
SP="$ROOT/docs/spec.md"
EC="$ROOT/docs/ERROR_CODES.md"
grep -q '顶层 `?` 传播 Err/None' "$CH" && bad "cheatsheet 仍在说「顶层 \`?\` 传播 Err/None」" \
    || note "cheatsheet：顶层 \`?\` 的旧表述已清除 ✅"
grep -q '`c ?: a : b`（三元）' "$CH" && bad "cheatsheet 仍把 \`?:\` 写成三元运算符" \
    || note "cheatsheet：\`?:\` 三元表述已清除 ✅"
grep -qE '\?\?.*`\?:`.*右操作数' "$SP" && bad "spec 短路运算列表仍含 \`?:\`" \
    || note "spec：短路列表不含 \`?:\` ✅"
grep -q '三元表达式' "$SP" && note "spec：「❌ 三元表达式」声明在位 ✅" || bad "spec 丢了「❌ 三元表达式」声明"
grep -q '强制解包' "$EC" || bad "ERROR_CODES 的 R1004 未登记 \`!\` 解包失败"
grep -q '顶层使用' "$EC" || bad "ERROR_CODES 的 R1004 未登记「顶层使用 \`?\`」"
note "ERROR_CODES R1004 已登记 \`!\` 与顶层 \`?\` ✅"
grep -q '幽灵' "$CH" || bad "cheatsheet 未记录「幽灵运算符」的更正来源"
sz=$(wc -l < "$CH"); [ "$sz" -ge 2750 ] || bad "cheatsheet 行数异常（$sz < 2750）"
note "cheatsheet $sz 行 ✅"

# ④⑤⑥⑦ 负控
if [ "$NEG_SKIP" != "1" ]; then

hdr "[4/7] 负控 A：**忠实撤回**缺陷 319 修复（px_field 的 t → o）⇒ 两道**非 null** 用例必须判红"
restore_all
# ⚠️ 首版把标记写成了 `px_field(<o>, "` **后面**再加 `# NEGCTL-M220A` —— 那等于把注释
#   塞进**被发射的字符串字面量**：生成的 C 变成 `px_field(o, "v   # NEGCTL-M220Aa")`
#   ⇒ 报的是 `R1008 字典没有键 '   # NEGCTL-M220Aa'`（**字段名被污染**），而
#   `ok_opt_side_effect_null` 更因为「null 走真分支、污染项在假分支」**根本没跑到**
#   ⇒ 实测只有 2/3 判红。**那不是「撤回修复」，那是「另造一个错」** —— 判据与被测对象没对齐。
#   现改为**忠实撤回**（`t` → `o`，一字之差），标记挪到**上一行的代码注释**里。
python3 - "$CGE" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
BS = chr(92)
old = ('        let fname = rust_unescape(expr[2])' + chr(10) +
       '        return "({ LXValue " + t + " = " + o + "; px_is_null(" + t + ") ? px_null() : px_field(" + t + ", ' + BS + '"')
new = ('        let fname = rust_unescape(expr[2])   # NEGCTL-M220A' + chr(10) +
       '        return "({ LXValue " + t + " = " + o + "; px_is_null(" + t + ") ? px_null() : px_field(" + o + ", ' + BS + '"')
n = s.count(old)
if n == 1:
    io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
print('NC-A 补丁点：%d（期望 1）' % n)
PY
if build_dev; then
    run_tt "$W/ncA" "ok_opt_side_effect,ok_opt_side_effect_null,ok_opt_side_effect_chain"
    prov
    NP=$(nprob "$W/ncA/tt.out")
    # ⚠️ 阈值是 **2/3**，不是 3/3 —— 这是**被测对象的性质**，不是判据放水：
    #   `?.` 的发射形态是 `px_is_null(t) ? px_null() : px_field(<接收者>, f)`，
    #   接收者为 **null** 时走**真**分支 ⇒ 假分支里那第二次求值**根本不会执行**
    #   ⇒ `ok_opt_side_effect_null` 在缺陷下**也**得到 `calls= 1`（与解释轨相同）。
    #   能暴露缺陷的只有**接收者非 null** 的那两例 ⇒ 判据 = 「那两道必须红」。
    RA=$(grep -c '^▲ DIVERGE ok_opt_side_effect$'       "$W/ncA/tt.out")
    RC=$(grep -c '^▲ DIVERGE ok_opt_side_effect_chain$' "$W/ncA/tt.out")
    if [ "${RA:-0}" -ge 1 ] && [ "${RC:-0}" -ge 1 ]; then
        note "负控 A ✅ 两道**非 null 接收者**用例判红（问题 ${NP:-?} 例 / 3）；null 那例按上述性质**不会**红 ✅"
    else
        bad "负控 A 未判红（非 null 用例 side_effect=$RA chain=$RC ⇒ 判据无牙或撤回不忠实）"
        tail -20 "$W/ncA/tt.out"
    fi
else
    bad "负控 A：重编失败"
fi

hdr "[5/7] 负控 B：撤回**缺陷 320**（闭包体不压错误标签）⇒ 必须判红"
restore_all
python3 - "$CGE" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = '    let cerr_tag = cg_cerr_tag()' + chr(10) + '    cg_err_labels.append(cerr_tag)' + chr(10)
new = '    let cerr_tag = cg_cerr_tag()   # NEGCTL-M220B：撤回压栈' + chr(10)
n = s.count(old)
if n == 1:
    io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
print('NC-B 补丁点：%d（期望 1）' % n)
PY
if build_dev; then
    run_tt "$W/ncB" "ok_clos_toplevel_err,ok_clos_toplevel_null,ok_clos_in_fn,ok_clos_nested"
    prov
    NP=$(nprob "$W/ncB/tt.out")
    if [ "${NP:-0}" -ge 2 ]; then
        note "负控 B ✅ 闭包体退回非帧 ⇒ 问题 $NP 例 / 4（顶层误判 + 函数内 C 编译失败复现）"
    else
        bad "负控 B 未判红（问题 ${NP:-?} 例）"; tail -20 "$W/ncB/tt.out"
    fi
else
    bad "负控 B：重编失败"
fi

hdr "[6/7] 负控 C：撤回**缺陷 316/317**（VM 轨顶层标记判据）⇒ 必须判红"
restore_all
python3 - "$VMC" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = '                if (in.b) px_error("R1004: 顶层不能使用错误传播 ?（仅函数内可用）");' + chr(10)
new = '                /* NEGCTL-M220C：撤回顶层标记判据 */' + chr(10)
n = s.count(old)
if n == 1:
    io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
print('NC-C 补丁点：%d（期望 1）' % n)
PY
if build_dev; then
    run_tt "$W/ncC" "err_try_toplevel_null,err_try_toplevel_err"
    prov
    NP=$(nprob "$W/ncC/tt.out")
    if [ "${NP:-0}" -ge 2 ]; then
        note "负控 C ✅ 顶层标记撤回 ⇒ 问题 $NP 例 / 2（null 静默 / Err 丢措辞复现）"
    else
        bad "负控 C 未判红（问题 ${NP:-?} 例）"; tail -20 "$W/ncC/tt.out"
    fi
else
    bad "负控 C：重编失败"
fi

hdr "[7/7] 负控 D：**判据自伤**（比对恒真）⇒ 负控 A 的红必须**消失**"
restore_all
python3 - "$CGE" "$TT" <<'PY'
import sys, io
cg, tt = sys.argv[1], sys.argv[2]
s = io.open(cg, encoding='utf-8').read()
BS = chr(92)
old = ('        let fname = rust_unescape(expr[2])' + chr(10) +
       '        return "({ LXValue " + t + " = " + o + "; px_is_null(" + t + ") ? px_null() : px_field(" + t + ", ' + BS + '"')
new = ('        let fname = rust_unescape(expr[2])   # NEGCTL-M220D' + chr(10) +
       '        return "({ LXValue " + t + " = " + o + "; px_is_null(" + t + ") ? px_null() : px_field(" + o + ", ' + BS + '"')
n = s.count(old)
if n == 1:
    io.open(cg, 'w', encoding='utf-8').write(s.replace(old, new, 1))
t = io.open(tt, encoding='utf-8').read()
old2 = '    # --- 判据 ---' + chr(10)
new2 = ('    if True:   # NEGCTL-M220D：判据自伤（全部算通过）' + chr(10) +
        '        nok += 1' + chr(10) +
        '        continue' + chr(10) +
        '    # --- 判据 ---' + chr(10))
k = t.count(old2)
if k == 1:
    io.open(tt, 'w', encoding='utf-8').write(t.replace(old2, new2, 1))
print('NC-D 补丁点：cg=%d tt=%d（各期望 1）' % (n, k))
PY
if build_dev; then
    run_tt "$W/ncD" "ok_opt_side_effect,ok_opt_side_effect_null,ok_opt_side_effect_chain"
    NP=$(nprob "$W/ncD/tt.out")
    if [ "${NP:-9}" = "0" ]; then
        note "负控 D ✅ 判据自伤后**同一条故障不再判红**（问题 0 例）⇒ ④ 的红确来自比对"
    else
        bad "负控 D 未生效（问题 ${NP:-?} 例 ⇒ 说明 ④ 的红另有来源，判据与被测对象没对齐）"
        tail -20 "$W/ncD/tt.out"
    fi
else
    bad "负控 D：重编失败"
fi
fi   # NEG_SKIP

hdr "[覆盖边界登记（如实）]"
note "① 三轨错误的**通道前缀**（解释轨写 stderr 但格式为 \`运行时错误: 错误 [R####] 行:列:\`，"
note "   编译两轨为 \`运行时错误 [<fn> 行N]: R####:\`）**不判** —— 属已登记的**缺陷 186** 族；"
note "   本门只比「R 码 + 剥掉前缀的核心文案」。"
note "② 顶层 \`?\` 在解释轨报的**行:列是 0:0**（\`i_run_program\` 用 [0,0]），"
note "   编译两轨给真实行号 ⇒ 同属缺陷 186 族，不判。"
note "③ 生成器（\`cg_gen_lambda\`）在本轮只被**静态**覆盖（顺序错已修）："
note "   要动态覆盖需 generator + transform/filter 语料，本轮未构造 ⇒ 记为覆盖缺口。"
note "④ 可选链在**方法调用**（\`a?.b()\`）上的形态未测。"

echo
if [ "$fail" = "0" ]; then
    echo "M220-VERIFY-OK（全部门通过）"
else
    echo "M220-VERIFY-FAIL"
fi
exit "$fail"
