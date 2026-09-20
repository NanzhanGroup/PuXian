#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M163 门（第 49 轮）：**字典键类型严格化** —— 字面量 / 推导式非字符串键
#   从编译两轨的「静默丢弃」改为 R1002（与解释轨同码同文）。
# ---------------------------------------------------------------
# 背景（本轮实测的三处分歧 + 一处同码不同文）：
#   · 缺陷 168（字面量）：`{1: 11, 2: 22}` —— 解释轨 `R1002 字典键必须是字符串`；
#     **VM 轨 / C 轨静默丢弃**（`len=0`），`{"a":1,"b":2,3:4}` 只丢那**一项**（`len=2`）
#     ⇒ 与 R1008「静默 null」同族的**静默数据丢失**，且三轨分叉。
#   · 缺陷 169（推导式）：`{x: x+10 for x in [1,2,3]}` —— C 轨静默丢（`len=0`）；
#     VM 轨走 `set` 方法报「方法 set 参数 1 需要 string」（同码 R1002、**词条不一致**）。
#   · 缺陷 171（索引赋值文案）：`d[2] = 3` —— 解释轨「字典键必须是字符串」，
#     而文档（docs/DICT_STRICT_MIGRATION.md §1）与 VM/C 轨是「字典索引键必须是字符串」。
# 修法（三轨**同一条真相**）：构造位置上的非字符串键一律 R1002，词条逐字统一 ——
#   · 运行时 `px_dict_set_checked`（runtime.c）：非 PX_STR ⇒ `字典键必须是字符串，实际是 <t>`
#   · VM 轨：NEWDICT 指令内联同一检查；推导式改 **DICTSET** 指令（取代 CALLM set）
#   · C 轨：字面量 / 推导式发射改走 `px_dict_set_checked`（保留「先求键、再求值」显式序列）
#   · 解释轨：字面量文案补「实际是 <t>」；推导式词条去掉「推导」二字；赋值改「字典索引键…」
# 判据（逐层可单独变红）：
#   ① 解释轨主用例 pass=16 fail=0（绝对值断言）；② VM 轨（用户面默认轨）编译+运行 ⇒
#   stdout 与解释轨**逐字节一致**；③ C 轨逃生舱同上；④ 加强面 6 断言三轨一致；
#   ⑤ **严格性层（本门主职）**：8 个错误用例 × 三轨，每轨都必须 rc≠0 + 含 R1002 +
#   含**统一词条**（修前 VM/C 在 ①②③ 类用例上是 rc=0 ⇒ 本层当场判红）；
#   ⑥ **负控 3 道**（默认跑，`--neg-skip` 跳过）—— 每道必须判红 + 逐字节还原：
#      A VM 轨：NEWDICT 恢复「跳过非字符串键」⇒ 严格性层必红
#      B C 轨：`px_dict_set_checked` 恢复「静默跳过」⇒ 严格性层必红
#      C 解释轨：推导式词条退回「字典推导键必须是字符串」⇒ 词条层必红
# 用法：./examples/m163_dict_key_strict/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m163_dict_key_strict/verify.sh --neg-skip （只跑正判据，CI 用）
# 退出码：0 = 绿，1 = 红。
# 备注：负控 A/B 会改 `runtime/` 源 → `tools/px` 的 rtcache **按内容哈希另建目录**
#   （`.rtcache/<key>/`，.gitignore 已忽略；不改动既有缓存、跑完源码逐字节还原）。
# ═══════════════════════════════════════════════════════════════════════
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT"
export LC_ALL=C LANG=C

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PXI="${PXI_BIN:-./bootstrap/pxi}"   # 解释轨（开发期可用 PXI_BIN=/tmp/pxidev 预演）
PXI_PROD="${PXI_BIN:-./bootstrap/pxi}"
WORK=$(mktemp -d /tmp/m163.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

# 三轨跑一个用例：$1=源文件 $2=标签 → $WORK/<tag>.{interp,vm,c}.{out,rc}
run_three() {
    local case=$1 tag=$2
    cp "$case" "$WORK/$tag.px"
    timeout 60 "$PXI" "$WORK/$tag.px" > "$WORK/$tag.interp.out" 2>&1
    echo $? > "$WORK/$tag.interp.rc"
    rm -rf "$WORK/build"
    if timeout 900 ./tools/px build "$WORK/$tag.px" > "$WORK/$tag.vm.log" 2>&1 && [ -x "$WORK/build/$tag" ]; then
        timeout 90 "$WORK/build/$tag" > "$WORK/$tag.vm.out" 2>&1; echo $? > "$WORK/$tag.vm.rc"
    else
        : > "$WORK/$tag.vm.out"; echo 99 > "$WORK/$tag.vm.rc"
        note "VM 轨编译失败（日志 $WORK/$tag.vm.log）"
    fi
    rm -rf "$WORK/build"
    if timeout 900 ./tools/px build --c "$WORK/$tag.px" > "$WORK/$tag.c.log" 2>&1 && [ -x "$WORK/build/$tag" ]; then
        timeout 90 "$WORK/build/$tag" > "$WORK/$tag.c.out" 2>&1; echo $? > "$WORK/$tag.c.rc"
    else
        : > "$WORK/$tag.c.out"; echo 99 > "$WORK/$tag.c.rc"
        note "C 轨编译失败（日志 $WORK/$tag.c.log）"
    fi
    rm -rf "$WORK/build"
}

# 正常路径：期望三轨都出同一行 + stdout 逐字节一致
expect_all_three() {     # $1=标签 $2=期望行
    local tag=$1 want=$2 t
    if grep -q "^$want\$" "$WORK/$tag.interp.out"; then
        note "解释轨：$want"
    else
        bad "$tag 解释轨未达 $want"; sed 's/^/      /' "$WORK/$tag.interp.out" | head -6
    fi
    for t in vm c; do
        if grep -q "^$want\$" "$WORK/$tag.$t.out"; then
            note "$t 轨：$want"
        else
            bad "$tag $t 轨未达 $want（$(grep -m1 -E 'FAIL|运行时错误|错误' "$WORK/$tag.$t.out" | head -c 120)）"
        fi
        cmp -s "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" \
            || { bad "$tag $t 轨 stdout 与解释轨不一致"; diff "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" | head -6 | sed 's/^/      /'; }
    done
}

# 严格性：三轨都必须 rc≠0 + 含 R1002 + 含统一词条
err_case() {             # $1=标签 $2=期望词条子串
    local tag=$1 want=$2 t rc
    for t in interp vm c; do
        rc=$(cat "$WORK/$tag.$t.rc")
        if [ "$rc" = "0" ]; then
            bad "$tag $t 轨 rc=0 —— **应当报错**（修前 VM/C 正是静默 rc=0 的那条缺口）"
            continue
        fi
        if ! grep -q "R1002" "$WORK/$tag.$t.out"; then
            bad "$tag $t 轨未报 R1002"; sed 's/^/      /' "$WORK/$tag.$t.out" | head -3; continue
        fi
        if ! grep -qF "$want" "$WORK/$tag.$t.out"; then
            bad "$tag $t 轨词条不符（缺「$want」）"; sed 's/^/      /' "$WORK/$tag.$t.out" | head -3; continue
        fi
        note "$tag $t 轨：rc=$rc · R1002 · $want"
    done
}

# ════ 正判据 ════
hdr "[1/5] 正常路径主用例（16 断言）—— 三轨逐字节一致"
run_three "$HERE/dict_key_strict.px" main
expect_all_three main "pass=16 fail=0"

hdr "[2/5] 加强面（6 断言：函数返回键 / 方法链键 / 深层结构 / 批量 300）"
run_three "$HERE/dict_key_edge.px" edge
expect_all_three edge "pass=6 fail=0"

hdr "[3/5] 严格性（缺陷 168/169）：字面量 / 推导式非字符串键 ⇒ R1002（三轨同码同文）"
run_three "$HERE/err_lit_int.px" e1;     err_case e1 "字典键必须是字符串，实际是 int"
run_three "$HERE/err_lit_mix.px" e2;     err_case e2 "字典键必须是字符串，实际是 int"
run_three "$HERE/err_lit_list.px" e3;    err_case e3 "字典键必须是字符串，实际是 list"
run_three "$HERE/err_lit_bool.px" e4;    err_case e4 "字典键必须是字符串，实际是 bool"
run_three "$HERE/err_comp_int.px" e5;    err_case e5 "字典键必须是字符串，实际是 int"
run_three "$HERE/err_comp_float.px" e6;  err_case e6 "字典键必须是字符串，实际是 float"
run_three "$HERE/err_comp_multi.px" e7;  err_case e7 "字典键必须是字符串，实际是 int"

hdr "[4/5] 索引赋值文案对齐（缺陷 171）：d[k]=v 非字符串键 ⇒ 字典索引键必须是字符串"
run_three "$HERE/err_setitem_int.px" e8; err_case e8 "字典索引键必须是字符串"

# ════ 负控 ════
run_neg() {              # $1=标签 $2=文件 $3=旧文本 $4=新文本 $5=说明 $6=用例标签 $7=轨 $8=期望词条
    local tag=$1 file=$2 from=$3 to=$4 what=$5 ctag=$6 eng=$7 want=$8
    local bak="$WORK/$(basename "$file").bak"
    echo "── 负控 $tag：$what"
    cp "$file" "$bak"
    python3 - "$file" "$from" "$to" <<'PY'
import sys
p, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p, encoding="utf-8").read()
if s.count(a) != 1:
    print("锚点不唯一（%d）" % s.count(a)); sys.exit(1)
open(p, "w", encoding="utf-8").write(s.replace(a, b, 1))
PY
    if [ $? -ne 0 ]; then
        bad "$tag 锚点失效（与源码不同步）"; cp "$bak" "$file"; return
    fi
    run_three "$HERE/$ctag.px" "neg$tag"
    local rc; rc=$(cat "$WORK/neg$tag.$eng.rc")
    if [ "$rc" != "0" ] && grep -qF "$want" "$WORK/neg$tag.$eng.out"; then
        bad "$tag 【未被判红】—— 判据对该回归不敏感"
        sed 's/^/      /' "$WORK/neg$tag.$eng.out" | head -3
    else
        note "$tag 已判红（$eng 轨 rc=$rc）✅"
    fi
    cp "$bak" "$file"
    cmp -s "$file" "$bak" || bad "$tag 还原失败（源码与负控前不一致）"
}

if [ "$NEG_SKIP" = "1" ]; then
    hdr "[5/5] 负控 3 道（--neg-skip：跳过）"
    note "CI 模式：负控涉及运行时/解释器重建，交给本地完整门"
else
    hdr "[5/5] 负控 3 道（每道必须判红 + 逐字节还原）"

    # A：VM 轨 NEWDICT 恢复「跳过非字符串键」（= 修前语义）
    run_neg A runtime/vm.c \
"                    if (slots[k0].type != PX_STR)
                        px_error(\"R1002: 字典键必须是字符串，实际是 %s\", px_type_name(slots[k0]));
                    px_dict_set(d, slots[k0].as.obj->as.str.data, slots[k0 + 1]);" \
"                    if (slots[k0].type == PX_STR)
                        px_dict_set(d, slots[k0].as.obj->as.str.data, slots[k0 + 1]);" \
        "VM 轨 NEWDICT 恢复「跳过非字符串键」（缺陷 168 原样）" e1 vm "字典键必须是字符串，实际是 int"

    # B：C 轨 px_dict_set_checked 恢复「静默跳过」（= 修前语义）
    run_neg B runtime/runtime.c \
"    if (k.type != PX_STR)
        px_error(\"R1002: 字典键必须是字符串，实际是 %s\", px_type_name(k));
    px_dict_set(dict, k.as.obj->as.str.data, v);" \
"    if (k.type != PX_STR) return;
    px_dict_set(dict, k.as.obj->as.str.data, v);" \
        "C 轨 px_dict_set_checked 恢复「静默跳过」（缺陷 168/169 原样）" e5 c "字典键必须是字符串，实际是 int"

    # C：解释轨推导式词条退回旧文案（同码不同文 = 缺陷 169 的残留面）
    echo "── 负控 C 需重建解释器（/tmp/pxidev）"
    cp selfhost/iexpr.px "$WORK/iexpr.px.bak"
    python3 - selfhost/iexpr.px \
        'i_r1002("字典键必须是字符串，实际是 " + i_type_name(k), pos)' \
        'i_r1002("字典推导键必须是字符串，实际是 " + i_type_name(k), pos)' <<'PY'
import sys
p, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p, encoding="utf-8").read()
if s.count(a) != 1:
    print("锚点不唯一（%d）" % s.count(a)); sys.exit(1)
open(p, "w", encoding="utf-8").write(s.replace(a, b, 1))
PY
    if [ $? -ne 0 ]; then
        bad "负控 C 锚点失效（与源码不同步）"; cp "$WORK/iexpr.px.bak" selfhost/iexpr.px
    else
        if bash selfhost/devbuild.sh pxi > "$WORK/negC.build.log" 2>&1; then
            PXI=/tmp/pxidev
            run_three "$HERE/err_comp_int.px" negC
            local_rc=$(cat "$WORK/negC.interp.rc")
            if [ "$local_rc" != "0" ] && grep -qF "字典键必须是字符串，实际是 int" "$WORK/negC.interp.out"; then
                bad "负控 C 【未被判红】—— 词条判据不敏感"
                sed 's/^/      /' "$WORK/negC.interp.out" | head -3
            else
                note "负控 C 已判红（interp 轨 rc=$local_rc · 词条退回旧文案）✅"
            fi
            PXI=$PXI_PROD
        else
            bad "负控 C 解释器重建失败（日志 $WORK/negC.build.log）"
        fi
        cp "$WORK/iexpr.px.bak" selfhost/iexpr.px
        cmp -s selfhost/iexpr.px "$WORK/iexpr.px.bak" || bad "负控 C 还原失败"
    fi
fi

echo ""
if [ "$fail" = "0" ]; then
    echo "M163-VERIFY-OK"
else
    echo "M163-VERIFY-FAIL"
fi
exit $((fail > 0))
