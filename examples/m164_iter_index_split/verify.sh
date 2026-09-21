#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M164 门（第 50 轮）：**迭代位置语义与用户索引分离**
#   一组四个缺陷，同一个病根 —— 「位置语义」（dict → 第 i 个键）当年为了
#   `for k in d` 能用 px_index 遍历而被塞进**用户可见的索引**里。
# ---------------------------------------------------------------
#   缺陷 170（核心）：`d[int]`（读取）
#     解释轨 R1002，而 VM/C 轨返回**第 i 个键**（M37 遗留的共用入口）
#     ⇒ 三轨分叉，且用户会把「第 i 个键」误读成「第 i 个元素」。
#   缺陷 174：惰性生成器的 seq **只认 list/gen**
#     `(c for c in "abc")` / `(e for e in (1,2,3))` / `(k for k in d)` 在 VM/C 轨
#     **静默产出空生成器**（解释轨正常）。range 看着正常只因 C 端 range 早已物化成 list。
#   缺陷 175：GenExp 合成 lambda 的**形参被双重去引号**
#     bc_genexp_caps 把已 unescape 的名字再塞回 Param 节点，而 cg_closure_caps 又
#     `rust_unescape(p[1])`（该函数假定带引号、无条件剥首尾各一字符）⇒ "c"→""，
#     被 cg_name_add 的空串判据静默丢弃 ⇒ 形参被当自由变量 ⇒ 一旦本帧有同名局部
#     （`var c = "Z"`）就被装箱捕获 ⇒ **VM 轨**读到外层值（解释/C 轨正确）。
# 修法（三轨**一条真相**）：
#   · runtime：新增 `px_iter_at`（迭代专用：dict → 第 i 个键；其余同 px_index）
#     与 `px_iter_prep` 无关；`px_index` 的 dict-int 分支**删除** ⇒ 用户索引严格
#   · VM 轨：新指令 **PXOP_ITERAT**（替换 for / 推导式迭代步里的 INDEX）
#   · C 轨：cg_stmt 的 For、cg_expr 的推导式展开改走 `px_iter_at`
#   · runtime：`px_lazy_seq_get` 补 dict/str/tuple（走同一迭代协议）
#   · bc_emit：`bc_genexp_caps` 改「体内引用名 − 形参，再 ∩ 本帧局部」（不再二次 unescape）
# 判据（逐层可单独变红）：
#   ① 解释轨主用例 pass=22 / 加强面 pass=10 / 形参遮蔽 pass=4（绝对值断言）；
#   ② VM 轨（用户面默认轨）与 ③ C 轨逃生舱：stdout 与解释轨**逐字节一致**；
#   ④ **严格性层（本门主职）**：5 个 `d[非字符串键]` 用例 × 三轨 ⇒ 每轨 rc≠0 + 含 R1002
#      + 含统一词条「字典索引键必须是字符串」（修前 VM/C 在这类用例上是 rc=0 ⇒ 当场判红）；
#   ⑤ **负控 4 道**（默认跑，`--neg-skip` 跳过）—— 每道必须判红 + 逐字节还原：
#      A runtime 恢复「dict 整数索引 → 第 i 个键」（缺陷 170 原样）⇒ 严格性层必红
#      B 两轨发射器同时退回 INDEX（cg_stmt + bc_emit_for）⇒ 正判据主用例必红
#      C runtime 惰性 seq 退回「只认 list/gen」（缺陷 174 原样）⇒ 加强面必红
#      D VM 轨 bc_genexp_caps 退回旧捕获算法（缺陷 175 原样）⇒ 形参遮蔽用例必红
# 用法：./examples/m164_iter_index_split/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m164_iter_index_split/verify.sh --neg-skip （只跑正判据，CI 用）
# 退出码：0 = 绿，1 = 红。
# 备注：负控会改 `runtime/` 与 `selfhost/` 源 → 机制是「改源 → 重建 → 跑 → 逐字节还原」。
#   runtime 走 `tools/px` 的内容哈希缓存（`.rtcache/<key>/`，.gitignore 已忽略）；
#   selfhost 走 `selfhost/devbuild.sh --vm`（产物 /tmp/pxcdev、/tmp/pxcdev_vm，不碰入库件）。
# ═══════════════════════════════════════════════════════════════════════
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT"
export LC_ALL=C LANG=C

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PXI="${PXI_BIN:-./bootstrap/pxi}"
WORK=$(mktemp -d /tmp/m164.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

# 三轨跑一个用例：$1=源文件 $2=标签 → $WORK/<tag>.{interp,vm,c}.{out,rc}
# 引擎可被 PXC_VM_BIN / PX_PXC_BIN 覆盖（负控重建 dev 件时用）
run_three() {
    local case=$1 tag=$2
    cp "$case" "$WORK/$tag.px"
    timeout 60 "$PXI" "$WORK/$tag.px" > "$WORK/$tag.interp.out" 2>&1; echo $? > "$WORK/$tag.interp.rc"
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

# 正常路径：三轨都出同一行 + stdout 逐字节一致
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
            bad "$tag $t 轨 rc=0 —— **应当报错**（修前 VM/C 正是把 d[i] 当「第 i 个键」返回的那条缺口）"
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
hdr "[1/6] 正常路径主用例（22 断言）—— 迭代仍可用 + 三轨逐字节一致"
run_three "$HERE/iter_split.px" main
expect_all_three main "pass=22 fail=0"

hdr "[2/6] 加强面（10 断言：嵌套/函数边界/推导式/生成器 3 种 seq）"
run_three "$HERE/iter_split_edge.px" edge
expect_all_three edge "pass=10 fail=0"

hdr "[3/6] 生成器形参遮蔽同名外层局部（缺陷 175，4 断言）"
run_three "$HERE/genexp_shadow.px" shadow
expect_all_three shadow "pass=4 fail=0"

hdr "[4/6] 严格性（缺陷 170）：d[非字符串键] ⇒ R1002（三轨同码同文）"
run_three "$HERE/err_d_int_lit.px" e1; err_case e1 "字典索引键必须是字符串"
run_three "$HERE/err_d_int_var.px" e2; err_case e2 "字典索引键必须是字符串"
run_three "$HERE/err_d_float.px"   e3; err_case e3 "字典索引键必须是字符串"
run_three "$HERE/err_d_neg.px"     e4; err_case e4 "字典索引键必须是字符串"
run_three "$HERE/err_d_bool.px"    e5; err_case e5 "字典索引键必须是字符串"

# ════ 负控 ════
# 改一处源码（锚点必须唯一）→ 重建 → 跑 → 判红 → 逐字节还原
patch_one() {            # $1=文件 $2=旧文本 $3=新文本 → rc≠0 表示锚点失效
    python3 - "$1" "$2" "$3" <<'PY'
import sys
p, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p, encoding="utf-8").read()
if s.count(a) != 1:
    print("锚点不唯一（%d）" % s.count(a)); sys.exit(1)
open(p, "w", encoding="utf-8").write(s.replace(a, b, 1))
PY
}

restore_all() {          # 由各负控自行登记的回滚点还原
    local i=0 f b
    while [ $i -lt ${#BAKFILES[@]} ]; do
        f="${BAKFILES[$i]}"; b="${BAKBLOB[$i]}"
        cp "$b" "$f"
        cmp -s "$f" "$b" || bad "还原失败：$f"
        i=$((i + 1))
    done
    BAKFILES=(); BAKBLOB=()
}
BAKFILES=(); BAKBLOB=()
snapshot() {             # $1=文件
    local b="$WORK/$(echo "$1" | tr '/' '_').bak"
    cp "$1" "$b"; BAKFILES+=("$1"); BAKBLOB+=("$b")
}

if [ "$NEG_SKIP" = "1" ]; then
    hdr "[5/6][6/6] 负控 4 道（--neg-skip：跳过）"
    note "CI 模式：负控涉及 runtime/发射器重建，交给本地完整门"
else
    hdr "[5/6] 负控 A/C：runtime 侧（缺陷 170 / 174 原样）"

    # A：px_index 恢复「dict 整数索引 → 第 i 个键」⇒ 严格性层必红
    echo "── 负控 A：px_index 恢复 dict-int 分支（缺陷 170 原样）"
    snapshot runtime/runtime.c
    if patch_one runtime/runtime.c \
'        //   现在迭代机制走**独立入口** px_iter_at（见下），用户索引一律严格。
        px_error("R1002: 字典索引键必须是字符串");' \
'        //   现在迭代机制走**独立入口** px_iter_at（见下），用户索引一律严格。
        if (idx.type == PX_INT) {
            LXObject* o = obj.as.obj;
            int i = (int)idx.as.i;
            if (i < 0) i += o->as.dict.len;
            if (i >= 0 && i < o->as.dict.len) return px_str(o->as.dict.keys[i]);
        }
        px_error("R1002: 字典索引键必须是字符串");'; then
        run_three "$HERE/err_d_int_lit.px" negA
        rc=$(cat "$WORK/negA.vm.rc")
        if [ "$rc" != "0" ] && grep -qF "字典索引键必须是字符串" "$WORK/negA.vm.out"; then
            bad "负控 A 【未被判红】—— 严格性判据对「位置语义回流」不敏感"
            sed 's/^/      /' "$WORK/negA.vm.out" | head -3
        else
            note "负控 A 已判红（vm 轨 rc=$rc，d[0] 又返回第 0 个键）✅"
        fi
    else
        bad "负控 A 锚点失效（与源码不同步）"
    fi
    restore_all

    # C：惰性 seq 退回「只认 list/gen」⇒ 加强面必红
    echo "── 负控 C：px_lazy_seq_get 退回只认 list/gen（缺陷 174 原样）"
    snapshot runtime/runtime.c
    if patch_one runtime/runtime.c \
'    if (seq.type == PX_DICT) {
        LXObject* o = seq.as.obj;
        if (i >= 0 && i < o->as.dict.len) {
            *has = 1;
            return px_str(o->as.dict.keys[i]);
        }
    }
    if (seq.type == PX_STR || seq.type == PX_TUPLE) {
        int n = px_len(seq);
        if (i >= 0 && i < n) {
            *has = 1;
            return px_iter_at(seq, px_int(i));
        }
    }
' ''; then
        run_three "$HERE/iter_split_edge.px" negC
        rc=$(cat "$WORK/negC.vm.rc")
        if [ "$rc" = "0" ] && grep -q "pass=10 fail=0" "$WORK/negC.vm.out"; then
            bad "负控 C 【未被判红】—— 加强面判据对「惰性 seq 类型缺口」不敏感"
        else
            note "负控 C 已判红（vm 轨 rc=$rc，生成器 seq 不再支持 str/tuple/dict）✅"
        fi
    else
        bad "负控 C 锚点失效（与源码不同步）"
    fi
    restore_all

    hdr "[6/6] 负控 B/D：发射器侧（需重建 dev 件 —— 不碰入库件）"

    # B：两轨发射器的迭代步同时退回 INDEX ⇒ 正判据主用例必红
    echo "── 负控 B：cg_stmt + bc_emit_for 的迭代步退回 px_index/INDEX"
    snapshot selfhost/cg_stmt.px
    snapshot selfhost/bc_emit.px
    okB=1
    patch_one selfhost/cg_stmt.px \
'                fv_init = "px_iter_at(" + it_var + ", px_int(" + idx_var + "))"' \
'                fv_init = "px_index(" + it_var + ", px_int(" + idx_var + "))"' || okB=0
    patch_one selfhost/bc_emit.px \
'            bc_emit_inst(func, "ITERAT", vss[0], its, ctr)' \
'            bc_emit_inst(func, "INDEX", vss[0], its, ctr)' || okB=0
    if [ "$okB" = "1" ]; then
        if bash selfhost/devbuild.sh --vm > "$WORK/negB.build.log" 2>&1; then
            ( export PXC_VM_BIN=/tmp/pxcdev_vm; export PX_PXC_BIN=/tmp/pxcdev
              run_three "$HERE/iter_split.px" negB )
            rc=$(cat "$WORK/negB.vm.rc")
            if [ "$rc" = "0" ] && grep -q "pass=22 fail=0" "$WORK/negB.vm.out"; then
                bad "负控 B 【未被判红】—— 正判据对「迭代步复用用户索引入口」不敏感"
            else
                note "负控 B 已判红（vm 轨 rc=$rc，for-in dict 又走用户索引）✅"
            fi
        else
            bad "负控 B 重建失败（日志 $WORK/negB.build.log）"
        fi
    else
        bad "负控 B 锚点失效（与源码不同步）"
    fi
    restore_all

    # D：VM 轨 bc_genexp_caps 退回旧捕获算法 ⇒ 形参遮蔽用例必红
    echo "── 负控 D：bc_genexp_caps 退回 cg_closure_caps 版（缺陷 175 原样）"
    snapshot selfhost/bc_emit.px
    if patch_one selfhost/bc_emit.px \
'    let used = []
    cg_ast_used(body_expr, used)
    let caps = []
    var ui = 0
    while ui < len(used):
        let cn = used[ui]
        if not contains(param_names, cn) and func["smap"].has(cn):
            caps.append(cn)
        ui += 1
    return caps' \
'    let pms = []
    var pi = 0
    while pi < len(param_names):
        pms.append(["Param", param_names[pi], null, null, null])
        pi += 1
    let cap_free = []
    cg_closure_caps(["Closure", pms, null, body_expr, [], null], cap_free)
    let caps = []
    var u2 = 0
    while u2 < len(cap_free):
        let cn = cap_free[u2]
        if func["smap"].has(cn):
            caps.append(cn)
        u2 += 1
    return caps'; then
        if bash selfhost/devbuild.sh --vm > "$WORK/negD.build.log" 2>&1; then
            ( export PXC_VM_BIN=/tmp/pxcdev_vm; export PX_PXC_BIN=/tmp/pxcdev
              run_three "$HERE/genexp_shadow.px" negD )
            rc=$(cat "$WORK/negD.vm.rc")
            if [ "$rc" = "0" ] && grep -q "pass=4 fail=0" "$WORK/negD.vm.out"; then
                bad "负控 D 【未被判红】—— 形参遮蔽判据对「形参被误判为自由变量」不敏感"
            else
                note "负控 D 已判红（vm 轨 rc=$rc，形参遮蔽用例红）✅"
            fi
        else
            bad "负控 D 重建失败（日志 $WORK/negD.build.log）"
        fi
    else
        bad "负控 D 锚点失效（与源码不同步）"
    fi
    restore_all
fi

echo ""
if [ "$fail" = "0" ]; then
    echo "M164-VERIFY-OK"
else
    echo "M164-VERIFY-FAIL"
fi
exit $((fail > 0))
