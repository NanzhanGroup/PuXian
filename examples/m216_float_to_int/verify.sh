#!/usr/bin/env bash
# ============================================================
# M216 门（第 95 轮）：浮点→int 转换族 —— 平台相关 UB + 三轨分叉（缺陷 308）
# ------------------------------------------------------------
# 病灶：`(int64_t)f` 在 f 越界/非有限时是 C11 6.3.1.4p1 的**未定义行为**
#   · x86_64 `cvttsd2si` ⇒ **INT64_MIN**（不定值哨兵）· aarch64 `fcvtzs` ⇒ **饱和**
#   ⇒ 同一份源码两个架构打印不同的数（静默）；且**编译两轨**（走 `int_val` 的
#   `(int64_t)`）与**解释轨**（float 直接参与整数比较）在切片界上**三轨分叉**：
#   实测修前 `l[0:1e30]` 解释 `[1,2,3]` vs VM/C `[]`、`l[1e30:2]` 解释 `[]` vs VM/C `[1,2]`。
# 修法（4 处 + 1 处安全网）：`px_f2i` 唯一入口（越界/非有限 ⇒ R1003）· 切片界与**索引位**
#   同口径（只收 int ⇒ R1002）· `int_val` 不再静默截断 float。
#
# 层：
#   ① 工具自证（语料漂移 · 规模下限 · 形态）
#   ② 正判据：三轨对拍 + **Python 独立真值**（22 例必须全过）
#   ③ 平台相关性举证（静态：两架构指令不同 ⇒ 新形态两侧都有守卫）
#   ④ 负控 A：撤回**编译轨**的 4 处守卫 ⇒ 必须判红
#   ⑤ 负控 B：撤回**解释轨** i_slice 的界守卫 ⇒ 必须判红
#   ⑥ 负控 C：判据自伤（比对恒真）⇒ ④ 的红必须消失（证明红来自比对）
#   ⑦ 覆盖边界登记（如实）
# 用法：verify.sh [--neg-skip]     （CI 用 --neg-skip：负控要重编 runtime 两次，太贵）
# ============================================================
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1
W=$(mktemp -d /tmp/m216_gate.XXXXXX)

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

# devbuild 一次（正判据与负控都复用）；负控各自 restore_all 后再重编
#   ⚠️ 必须**显式列出 pxc**：devbuild.sh 的约定是「给了名字就**只**编那些」（`[ -n "$NAMES" ] || NAMES="pxc"`）
#   ⇒ 写成 `devbuild.sh pxi --vm` 会**不重建 /tmp/pxcdev**，于是「C 轨编译器还是上一版」而门看不出原因。
#   本门第 ① 层之外新增「溯源行」正是为此：把用到的三个件的 sha 打进日志（可读出真因）。
build_dev() {
    timeout 900 bash "$ROOT/selfhost/devbuild.sh" pxc pxi --vm > "$W/devbuild.log" 2>&1
}

prov() {
    echo "   溯源：rtcache=$( (cd "$ROOT" && ./tools/px rtcache 2>/dev/null | tail -1) )"
    sha256sum /tmp/pxcdev /tmp/pxcdev_vm /tmp/pxidev 2>/dev/null | awk '{printf "         %s %s\n", substr($1,1,16), $2}'
}

# 三轨对拍：$1=工作目录 → stdout 落 $1/tt.out，rc 落 $1/tt.rc
run_tt() {
    local d="$1"
    mkdir -p "$d"
    timeout 900 python3 "$TT" --root "$ROOT" --work "$d/tt" \
        --interp /tmp/pxidev --cbin /tmp/pxcdev --vmb /tmp/pxcdev_vm > "$d/tt.out" 2>&1
    echo $? > "$d/tt.rc"
}

echo "M216 门 · 工作目录 $W"
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
    [ "$N" -ge 20 ] || bad "语料规模低于下限（$N 例 < 20）⇒ 判据可能被架空"
    note "语料：$N 例（下限 20）"
    NOK=$(awk -F'\t' '$2=="ok"' "$HERE/CASES.tsv" | wc -l)
    NREJ=$(awk -F'\t' '$2=="rej"' "$HERE/CASES.tsv" | wc -l)
    note "合法侧 $NOK · 拒绝侧 $NREJ"
    [ "$NOK" -ge 8 ] || bad "合法侧过少（$NOK < 8）"
    [ "$NREJ" -ge 12 ] || bad "拒绝侧过少（$NREJ < 12）"
    BADKIND=$(awk -F'\t' '$2!="ok" && $2!="rej"' "$HERE/CASES.tsv" | wc -l)
    [ "$BADKIND" -eq 0 ] || bad "CASES.tsv 出现未知 kind（$BADKIND 行）"
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
hdr "[3/7] 平台相关性举证（静态：同一段转换在两个架构上生成**不同**指令）"
bash "$HERE/platform_proof.sh" "$W/plat" > "$W/plat.out" 2>&1
cat "$W/plat.out" | sed 's/^/   /'
if grep -q '^SKIP=1' "$W/plat.out"; then
    note "⚠️ 交叉工具链不可用 ⇒ 本层跳过（**响亮记录**，不静默过）"
else
    grep -q '^OLD_X86_HAS_CVTTSD2SI=1' "$W/plat.out" || bad "旧形态未在 x86 生成 cvttsd2si（举证失效）"
    grep -q '^OLD_ARM_HAS_FCVTZS=1'    "$W/plat.out" || bad "旧形态未在 aarch64 生成 fcvtzs（举证失效）"
    grep -q '^NEW_X86_HAS_GUARD=1'     "$W/plat.out" || bad "新形态 x86 侧缺守卫"
    grep -q '^NEW_ARM_HAS_GUARD=1'     "$W/plat.out" || bad "新形态 aarch64 侧缺守卫"
    grep -q '^OLD_ARM_HAS_FCVTZS=1' "$W/plat.out" && grep -q '^OLD_X86_HAS_CVTTSD2SI=1' "$W/plat.out" \
        && note "旧形态两架构指令不同（cvttsd2si ⇄ fcvtzs）⇒ **UB 的两副面孔** ✅"
    note "新形态两侧都在转换前有守卫 ✅（越界/非有限走 R1003，转换点良定义）"
fi

# ------------------------------------------------------------ ④
if [ "$NEG_SKIP" != "1" ]; then
hdr "[4/7] 负控 A：撤回**编译轨**的 4 处守卫 ⇒ 必须判红"
restore_all
python3 - "$RTC" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
n = 0
pairs = [
    ('px_int(px_f2i(a.as.f, "int()"))',              'px_int((int64_t)a.as.f)'),
    ('px_int(px_f2i(floor(num_val(a) / d), "整除"))', 'px_int((int64_t)floor(num_val(a) / d))'),
    ('px_req_slice_idx(step)',      'int_val(step)'),
    ('px_req_slice_idx(start)',     'int_val(start)'),
    ('px_req_slice_idx(end)',       'int_val(end)'),
    ('px_req_slice_idx(args[1])',   'int_val(args[1])'),
    ('px_req_slice_idx(args[2])',   'int_val(args[2])'),
]
for a, b in pairs:
    if a in s:
        s = s.replace(a, b); n += 1
old = '''static int64_t int_val(LXValue v) {
    if (v.type == PX_INT) return v.as.i;'''
new = '''static int64_t int_val(LXValue v) {
    if (v.type == PX_INT) return v.as.i;
    if (v.type == PX_FLOAT) return (int64_t)v.as.f;'''
if old in s:
    s = s.replace(old, new, 1); n += 1
io.open(p, 'w', encoding='utf-8').write(s)
print('NC-A 补丁点：%d（期望 8）' % n)
PY
NCA=$(build_dev && echo ok || echo fail)
prov
if [ "$NCA" = "ok" ]; then
    run_tt "$W/ncA"
    NPROB=$(sed -n 's/^对拍 \([0-9]*\) 例 · 通过 \([0-9]*\) · 问题 \([0-9]*\)$/\3/p' "$W/ncA/tt.out")
    if [ "${NPROB:-0}" -ge 5 ]; then
        note "负控 A ✅ 撤回守卫 ⇒ 问题 $NPROB 例（修前形态复现）"
    else
        bad "负控 A 未判红（问题 ${NPROB:-?} 例 ⇒ 判据无牙）"
        head -20 "$W/ncA/tt.out"
    fi
else
    bad "负控 A：重编失败（补丁把源码改坏了？）"
fi

# ------------------------------------------------------------ ⑤
hdr "[5/7] 负控 B：撤回**解释轨** i_slice 的界守卫 ⇒ 必须判红"
restore_all
python3 - "$IVAL" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
blk = '''    var bounds = [start, end, step]
    var bi = 0
    while bi < 3:
        if bounds[bi] != null and i_type_name(bounds[bi]) != "int":
            return Err(i_r1002("切片索引必须是整数，实际是 " + i_type_name(bounds[bi]), pos))
        bi += 1
'''
if blk in s:
    s = s.replace(blk, '', 1)
    io.open(p, 'w', encoding='utf-8').write(s)
    print('NC-B 补丁点：1（期望 1）')
else:
    print('NC-B 补丁点：0（锚点未命中 ⇒ 判据失效）')
PY
if build_dev; then
    run_tt "$W/ncB"
    prov
    NPROB=$(sed -n 's/^对拍 \([0-9]*\) 例 · 通过 \([0-9]*\) · 问题 \([0-9]*\)$/\3/p' "$W/ncB/tt.out")
    if [ "${NPROB:-0}" -ge 3 ]; then
        note "负控 B ✅ 撤回解释轨守卫 ⇒ 问题 $NPROB 例（三轨分叉复现）"
    else
        bad "负控 B 未判红（问题 ${NPROB:-?} 例）"
        head -20 "$W/ncB/tt.out"
    fi
else
    bad "负控 B：重编失败"
fi

# ------------------------------------------------------------ ⑥
hdr "[6/7] 负控 C：判据自伤（比对/期望恒真）⇒ ④ 的红必须消失"
restore_all
python3 - "$TT" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
# M216：判据自伤要**彻底** —— 首版只把「三轨一致性」那条改成恒假，
#   而拒绝侧的 `code`/`body` 两条比较仍在 ⇒ 依旧报 10 例，NC-C 假红（以为"红不来自比对"）。
#   正确做法：把**所有** `bad.append(` 换成 sink（含 3 处分歧 + 4 处期望）。
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
# 再打一遍 NC-A（撤回编译轨守卫）
python3 - "$RTC" <<'PY'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8').read()
s = s.replace('px_int(px_f2i(a.as.f, "int()"))', 'px_int((int64_t)a.as.f)')
s = s.replace('px_int(px_f2i(floor(num_val(a) / d), "整除"))', 'px_int((int64_t)floor(num_val(a) / d))')
s = s.replace('px_req_slice_idx(step)', 'int_val(step)')
s = s.replace('px_req_slice_idx(start)', 'int_val(start)')
s = s.replace('px_req_slice_idx(end)', 'int_val(end)')
s = s.replace('px_req_slice_idx(args[1])', 'int_val(args[1])')
s = s.replace('px_req_slice_idx(args[2])', 'int_val(args[2])')
io.open(p, 'w', encoding='utf-8').write(s)
PY
if build_dev; then
    run_tt "$W/ncC"
    NPROB=$(sed -n 's/^对拍 \([0-9]*\) 例 · 通过 \([0-9]*\) · 问题 \([0-9]*\)$/\3/p' "$W/ncC/tt.out")
    if [ "${NPROB:-9}" = "0" ]; then
        note "负控 C ✅ 判据自伤后，同一份「修前源码」不再被判红 ⇒ ④ 的红确实来自比对逻辑"
    else
        bad "负控 C 失败：判据自伤后仍报问题 ${NPROB:-?} 例（⇒ ④ 的红不是来自比对？）"
    fi
else
    bad "负控 C：重编失败"
fi
restore_all
# 收尾：把 dev 件重建回**修复后**的源码状态（否则门跑完留下一套"已撤回守卫"的
#   /tmp/pxidev，下一次手工跑会拿到它 —— 本门首版就是这么自伤的）
build_dev && note "收尾：dev 件已按修复后源码重建 ✅"
else
    note "（④⑤⑥ 已跳过）"
fi

# ------------------------------------------------------------ ⑦
hdr "[7/7] 覆盖边界（如实登记）"
cat <<'EOF'
   · **未覆盖**：VM 轨 `PXOP_IDIV`/`PXOP_MOD` 的快路径 —— 它只处理 INT-INT，float 一律回落
     `px_idiv`/`px_mod`（M215 已收敛）⇒ 不在本缺陷面内，无需改。
   · **未覆盖**：native/FFI 桥里若有把 `args[i].as.f` 直接当 int 读的站点 —— 那是 M194/M195
     的「参数位」面（已全量收紧，且有独立门），本门只覆盖**转换**面。
   · **未覆盖**：aarch64 **运行期**行为（本机无 qemu）—— 第 ③ 层用**指令级静态举证**替代，
     并已在门内断言两侧都有守卫；行为侧由第 ② 层的三轨对拍（同一份源码）覆盖。
   · **已登记的语义收紧**：切片界从「float 静默参与」改为「必须整数」⇒ 同时消灭了
     解释轨/编译轨的分叉。若生态里有 `xs[0:len(xs)/2]`（float 上界）会由**静默错值**
     变为**响亮报错**（Python 3 同样拒绝 float 切片界）。
EOF

echo
if [ "$fail" = "0" ]; then
    echo "M216-VERIFY-OK"
else
    echo "M216-VERIFY-FAIL"
fi
exit "$fail"
