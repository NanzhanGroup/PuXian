#!/usr/bin/env bash
# ============================================================
# M217 门（第 96 轮）：移位计数的越界语义 —— UB 收口（缺陷 309）
# ------------------------------------------------------------
# 病灶：`a << n` / `a >> n` 在 `n < 0`、`n >= 64`、或**左移结果不可表示**时，是
#   **C11 6.5.7p3/p4 的未定义行为**。实测（同一份 C，只换优化档）：
#     `f(1,64)`（运行期）  -O0 ⇒ **1**（shl 指令掩码）  **-O2 ⇒ 0**
#     常量 `1LL << 64`     两侧 ⇒ 0（gcc 折叠 + 告警 `-Wshift-count-overflow`）
#   ⇒ 「三轨一致」只是**恰好**都走 `shl` 指令，与语义无关（同 M216 的结论）。
#   而且**同族的 4 个移位只收口了 1 个**：`px_ushr`/`PXOP_SHRU` 早已显式 `& 63u`
#   （注释明写「与解释器 wrapping_shr 一致」），`<<`/`>>` 却一直是裸移位
#   ⇒ M191/M203/M206/M215 的「同一语义面只收口一处」老病再犯。
#   修前实测（干净树 HEAD）：`a << -1` 三轨**一致地静默给错值**（`8 << -1` ⇒ 8 或 0）。
#
# 修法：`px_shift_count` / `px_shl64` / `px_shr64` / `px_shru64` **唯一实现**
#   （runtime.c）—— 计数 ∈[0,63] 正常；>=64 **掩码 `n & 63`**（= Rust wrapping_*，
#   与本仓既有 `>>>` 口径一致 ⇒ **零行为变更**）；**<0 ⇒ R1003 响亮**；
#   左移**一律经 `uint64_t`**（无符号回绕良定义 ⇒ 连「结果不可表示」那半 UB 也消除）。
#   VM 轨三个快路径、解释轨三个分支全部收敛到同口径。
#
# 层：
#   ① 工具自证（语料漂移 · 规模下限 · 形态）
#   ② 正判据：三轨对拍（19 例）+ 与 Python 独立真值一致
#   ③ UB 举证（静态：三种症状 × 旧/新形态 + 反汇编掩码指令）
#   ④ 负控 A：撤回 **C/VM 轨**的负计数守卫 ⇒ 拒绝侧必须判红
#   ⑤ 负控 B：撤回 **解释轨**的负计数守卫 ⇒ 拒绝侧必须判红
#   ⑥ 负控 C：判据自伤（比对恒真）⇒ ④ 的红必须消失
#   ⑦ 覆盖边界登记（如实）
# 用法：verify.sh [--neg-skip]     （CI 用 --neg-skip：负控要重编 dev 件，太贵）
# ============================================================
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
W=$(mktemp -d /tmp/m217_gate.XXXXXX)

RTC="$ROOT/runtime/runtime.c"
RTH="$ROOT/runtime/runtime.h"
IVAL="$ROOT/selfhost/ival.px"
TT="$HERE/three_tracks.py"
SNAP="$W/snap"; mkdir -p "$SNAP"
cp -a "$RTC"  "$SNAP/runtime.c.snap"
cp -a "$RTH"  "$SNAP/runtime.h.snap"
cp -a "$IVAL" "$SNAP/ival.px.snap"
cp -a "$TT"   "$SNAP/tt.snap"

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo; echo "── $*"; }

restore_all() {
    cp -a "$SNAP/runtime.c.snap" "$RTC"
    cp -a "$SNAP/runtime.h.snap" "$RTH"
    cp -a "$SNAP/ival.px.snap"   "$IVAL"
    cp -a "$SNAP/tt.snap"        "$TT"
}
trap 'restore_all; rm -rf "$W"' EXIT

# devbuild 一次（正判据与负控都复用）
#   ⚠️ 必须**显式列出 pxc pxi**：devbuild.sh 的约定是「给了名字就**只**编那些」
#   ⇒ 写成 `devbuild.sh pxi --vm` 会不重建 /tmp/pxcdev，于是「C 轨编译器还是上一版」
#   而门看不出原因（M216 踩过）⇒ 本门在第 ② 层打**溯源行**（三个件的 sha）。
build_dev() {
    timeout 900 bash "$ROOT/selfhost/devbuild.sh" pxc pxi --vm > "$W/devbuild.log" 2>&1
}

prov() {
    echo "   溯源：rtcache=$( (cd "$ROOT" && ./tools/px rtcache 2>/dev/null | tail -1) )"
    sha256sum /tmp/pxcdev /tmp/pxcdev_vm /tmp/pxidev 2>/dev/null | awk '{printf "         %s %s\n", substr($1,1,16), $2}'
}

run_tt() {
    local d="$1"
    mkdir -p "$d"
    timeout 900 python3 "$TT" --root "$ROOT" --work "$d/tt" \
        --interp /tmp/pxidev --cbin /tmp/pxcdev --vmb /tmp/pxcdev_vm > "$d/tt.out" 2>&1
    echo $? > "$d/tt.rc"
}

# 三轨对拍的问题数（从 tt.out 里取 `问题 N`）
nprob() { sed -n 's/^对拍 [0-9]* 例 · 通过 [0-9]* · 问题 \([0-9]*\)$/\1/p' "$1"; }

echo "M217 门 · 工作目录 $W"
[ "$NEG_SKIP" = "1" ] && note "（--neg-skip：跳过负控 ④⑤⑥）"

# ------------------------------------------------------------ ①
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
    [ "$N" -ge 15 ] || bad "语料规模低于下限（$N 例 < 15）⇒ 判据可能被架空"
    note "语料：$N 例（下限 15）"
    NOK=$(awk -F'\t' '$2=="ok"' "$HERE/CASES.tsv" | wc -l)
    NREJ=$(awk -F'\t' '$2=="rej"' "$HERE/CASES.tsv" | wc -l)
    note "合法侧 $NOK · 拒绝侧 $NREJ"
    [ "$NOK" -ge 6 ] || bad "合法侧过少（$NOK < 6）"
    [ "$NREJ" -ge 6 ] || bad "拒绝侧过少（$NREJ < 6，且必须覆盖三个移位运算符）"
    BADKIND=$(awk -F'\t' '$2!="ok" && $2!="rej"' "$HERE/CASES.tsv" | wc -l)
    [ "$BADKIND" -eq 0 ] || bad "CASES.tsv 出现未知 kind（$BADKIND 行）"
    # 三轨对比的**必要性**：移位语义若只测一轨就看不见「只收口一处」
    for op in '<<' '>>' '>>>'; do
        grep -qF " $op " "$HERE/cases.px" || bad "语料未覆盖运算符 $op"
    done
    note "语料覆盖三个移位运算符 ✅"
else
    bad "gen_cases.py 执行失败"
    tail -5 "$W/gen.log"
fi

# ------------------------------------------------------------ ②
hdr "[2/7] 正判据：三轨对拍（编译轨 **一次编译** + 逐 label 只跑不编）"
if build_dev; then
    note "devbuild 成功（/tmp/pxcdev · /tmp/pxcdev_vm · /tmp/pxidev）"
    prov
else
    bad "devbuild 失败"
    tail -20 "$W/devbuild.log"
fi
run_tt "$W/pos"
tail -3 "$W/pos/tt.out" | sed 's/^/   /'
if [ "$(cat "$W/pos/tt.rc" 2>/dev/null || echo 9)" = "0" ]; then
    note "三轨一致 + 与独立真值相符 ✅"
else
    bad "三轨对拍未过（见下）"
    grep -E '^▲|^    期望' "$W/pos/tt.out" | head -20
fi

# ------------------------------------------------------------ ③
hdr "[3/7] UB 举证（静态：三种症状 × 旧/新形态 + 反汇编掩码指令）"
bash "$HERE/ub_proof.sh" "$W/ub" > "$W/ub.out" 2>&1
sed 's/^/   /' "$W/ub.out"
if grep -q '^SKIP=1' "$W/ub.out"; then
    note "⚠️ 工具链不可用 ⇒ 本层跳过（**响亮记录**，不静默过）"
else
    UB=$(sed -n 's/^UB_SYMPTOMS=//p' "$W/ub.out")
    NS=$(sed -n 's/^NEW_SYMPTOMS=//p' "$W/ub.out")
    OA=$(sed -n 's/^OLD_HAS_AND=//p' "$W/ub.out")
    NA=$(sed -n 's/^NEW_HAS_AND=//p' "$W/ub.out")
    [ "${UB:-0}" -ge 1 ] || bad "旧形态**看不出** UB 症状（⇒ 举证失效，本门失去前提）"
    [ "${NS:-9}" = "0" ] || bad "新形态仍有 UB 症状（$NS）⇒ 掩码/无符号化不彻底"
    [ "${OA:-9}" = "0" ] || bad "旧形态反汇编里出现显式掩码（⇒ 旧形态其实不裸？）"
    [ "${NA:-0}" = "1" ] || bad "新形态反汇编里没有掩码指令（⇒ 举证看不到显式掩码）"
    [ "${UB:-0}" -ge 1 ] && [ "${NS:-0}" = "0" ] && note "旧形态 UB 症状 $UB 种 ⇄ 新形态 **0** 种 ✅"
fi

# ------------------------------------------------------------ ④⑤⑥
if [ "$NEG_SKIP" != "1" ]; then
hdr "[4/7] 负控 A：撤回 **C/VM 轨**的负计数守卫 ⇒ 拒绝侧必须判红"
restore_all
python3 - "$RTC" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = '        px_error("R1003: %s 计数不能为负数，实际是 %lld", what, (long long)n);'
new = '        (void)what;   /* NEGCTL-M217A：撤回负计数守卫 */'
n = s.count(old)
if n == 1:
    io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
print('NC-A 补丁点：%d（期望 1）' % n)
PY
NCA=0
if build_dev; then
    run_tt "$W/ncA"
    prov
    NP=$(nprob "$W/ncA/tt.out")
    # 拒绝侧 9 例 + 合法侧 0 例 ⇒ 至少 9 例判红（负计数现在静默给掩码值）
    if [ "${NP:-0}" -ge 6 ]; then
        note "负控 A ✅ 撤回守卫 ⇒ 问题 $NP 例（拒绝侧静默给错值复现）"
    else
        bad "负控 A 未判红（问题 ${NP:-?} 例 ⇒ 判据无牙）"
        head -20 "$W/ncA/tt.out"
    fi
else
    bad "负控 A：重编失败"
fi

hdr "[5/7] 负控 B：撤回 **解释轨**的负计数守卫 ⇒ 拒绝侧必须判红"
restore_all
python3 - "$IVAL" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
old = '''            if r < 0:
                return Err(i_r1003(i_bit_opname(op) + " 计数不能为负数，实际是 " + i_to_str(r), pos))
'''
new = '''            # NEGCTL-M217B：撤回解释轨负计数守卫（r 继续走掩码）
'''
n = s.count(old)
if n == 1:
    io.open(p, 'w', encoding='utf-8').write(s.replace(old, new, 1))
print('NC-B 补丁点：%d（期望 1）' % n)
PY
if build_dev; then
    run_tt "$W/ncB"
    prov
    NP=$(nprob "$W/ncB/tt.out")
    if [ "${NP:-0}" -ge 6 ]; then
        note "负控 B ✅ 撤回解释轨守卫 ⇒ 问题 $NP 例（三轨分叉复现）"
    else
        bad "负控 B 未判红（问题 ${NP:-?} 例）"
        head -20 "$W/ncB/tt.out"
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
                  'bad, nok = [], 0\n_BAD_SINK = lambda *a, **k: None', 1)
    n += 1
cnt = s.count('bad.append(')
s = s.replace('bad.append(', '_BAD_SINK(')
n += cnt
io.open(p, 'w', encoding='utf-8').write(s)
print('NC-C 补丁点：%d（sink 1 + bad.append %d）' % (n, cnt))
PY
# 再打一遍 NC-A（撤回 C/VM 轨守卫）
python3 - "$RTC" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
s = s.replace('        px_error("R1003: %s 计数不能为负数，实际是 %lld", what, (long long)n);',
              '        (void)what;   /* NEGCTL-M217A */')
io.open(p, 'w', encoding='utf-8').write(s)
PY
if build_dev; then
    run_tt "$W/ncC"
    NP=$(nprob "$W/ncC/tt.out")
    if [ "${NP:-9}" = "0" ]; then
        note "负控 C ✅ 判据自伤后，同一份「修前源码」不再被判红 ⇒ ④ 的红确实来自比对逻辑"
    else
        bad "负控 C 失败：判据自伤后仍报问题 ${NP:-?} 例（⇒ ④ 的红不是来自比对？）"
    fi
else
    bad "负控 C：重编失败"
fi
restore_all
build_dev && note "收尾：dev 件已按修复后源码重建 ✅"
else
    note "（④⑤⑥ 已跳过）"
fi

# ------------------------------------------------------------ ⑦
hdr "[7/7] 覆盖边界（如实登记）"
cat <<'EOF'
   · **未覆盖**：`px_ushr` 之外的**其它位运算**（`&`/`|`/`^`/`~`）—— 它们对 int64 是全宽的，
     无计数参数 ⇒ 不在本缺陷面内（本门第 ① 层仍留了 `ok_bitops_regress` 做回归）。
   · **未覆盖**：`i_shr_u`（逻辑右移）内部的 `b % 64` 仍保留 —— 与新的 `r & 63` **互为幂等**；
     保留它是为了让「解释器自身被更老的运行时解释」时也安全（双保险，不是冗余 bug）。
   · **未覆盖**：native/FFI 桥里若有人直接 `args[i].as.i << args[j].as.i` —— 那是**桥内代码**，
     不经过语言运算符面；本轮只收口「语言运算符」这一面（`px_shl/px_shr/px_ushr` + VM 三快路径
     + 解释轨三分支）。⇒ 下一轮候选：把桥内裸移位也扫一遍。
   · **已登记的语义收紧**：**负计数**从「静默掩码」（`1 << -1` ⇒ INT64_MIN）改为
     **响亮 R1003**。生态扫描（本仓 212 处移位用法人工抽看）**未发现负计数**；
     常见形态是 `x >> 6` / `x >> 12` / `(0x10000 >> m)`（m 为月份 1..12）等**小常量**。
     ⇒ 与 M216 同向（「转换/计数越界是错误 ⇒ 响亮」，Python `ValueError` / Go panic 同向）。
   · **零行为变更**的部分：`n >= 64` 仍是**掩码**（`1 << 64` == `1 << 0` == 1）——
     与本仓既有的 `>>>`/`px_ushr` 口径、Rust `wrapping_shl/shr` 一致；改的只是
     **把「靠 UB 碰巧」变成「显式良定义」**。
EOF

echo
if [ "$fail" = "0" ]; then
    echo "M217-VERIFY-OK"
else
    echo "M217-VERIFY-FAIL"
fi
exit "$fail"
