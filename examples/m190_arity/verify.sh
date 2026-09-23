#!/usr/bin/env bash
# ============================================================
# M190 门（第 68 轮）：**方法族参数面**三轨同一真相（缺陷 213-a…213-l）
# ------------------------------------------------------------
# 本轮的主题是「**同一操作的实参个数**」这一面：
#
#   ① 缺陷 213-a `l.pop(1)`：解释轨**忽略实参**（rc=0，静默少一个元素）vs VM/C「pop 不接受参数」
#   ② 缺陷 213-b `d.set("b")`：解释轨**把值当 null 写入**（rc=0，数据污染）vs VM/C「方法 set 需要 2 个参数」
#   ③ 缺陷 213-c `Ok(1).unwrap(2)`：解释轨忽略实参 vs VM/C 响亮（同族的 `unwrap_err` 早就有检查）
#   ④ 缺陷 213-d `Ok()` / `Ok(1,2)` / `Err()` / `Some()`：解释轨 0 参给 null、多参只取第一个 vs VM/C 恰 1 参
#   ⑤ 缺陷 213-e `split("a,b")`：**解释轨窄**（只收恰 2 个）vs VM/C 收 1-2（1 参 = 单空格）
#   ⑥ 缺陷 213-f `assert(c,"m",3)`：**编译轨宽**（只查下限）vs 解释轨 1-2
#   ⑦ 缺陷 213-g `l.index(x)`：解释轨**有该方法**、编译两轨 R1007「没有方法 'index'」⇒ 补齐编译侧
#   ⑧ 缺陷 213-h `l.append(1,2)`：解释轨只查下限（静默丢弃第 2 个）vs native 精确 `nargs != 1`
#   ⑨ 缺陷 213-i 字符串方法族 `s.upper(9)` / `s.contains("a","b")` / `s.replace("a","b","c")` /
#      `s.len(9)` / `s.trim(9)` / `s.split(",",";")` / `s.starts_with("a","b")`：
#      解释轨**静默忽略多余实参**，native 侧有的不查、有的转发到内置（报**内置名**文案）⇒ 三轨分叉
#   ⑩ 缺陷 213-j `l.reverse(9)` / `l.sort(9)`：`reverse`/`sort` 解释轨有、编译轨无（R1007）；
#      且解释轨 `sort` 的比较用 `i_to_str`（**渲染串** —— 与 M162 修掉的 `sorted` 旧 bug 同族）
#   ⑪ 缺陷 213-k `t.len()`：解释轨有、编译轨无（R1007）
#   ⑫ 缺陷 213-l 同批收尾：native 侧 `pop`/`is_ok`/`unwrap`/`unwrap 失败` 等**无 R 码**，
#      解释轨带码 ⇒ 同码不同文（补码）
#   ⑬ 缺陷 213-m（**反向** · 由 708 探针的全量普查筛出）：`now(1,2,3)` / `now_sec` / `now_us` /
#      `now_ns` / `hostname` / `tz_local` / `os_self_path` / `onnx_op_names` 这 8 个 **0 参内置**
#      修前**完全不查实参**（`(void)nargs;`）⇒ **编译轨静默忽略**、解释轨响亮「X 不需要参数」
#      （与 ①…⑫ 方向相反，但同族）⇒ native 侧补 `nargs != 0`。
#
# 统一口径（一条真相）：**每个方法/构造器的实参个数是精确的**（下界 = 上界，或显式区间），
#   两侧**同码同文**；拒绝时一律 **rc≠0**（响亮优于静默，与 M163/M166/M184/M185/M189 一贯）。
#
# 判据（三层正判据 + 五道负控，每层/每道可独立判红）：
#  ① `arity_ok.px` 合法侧：三轨 rc=0 + stdout **逐字节一致** + 逐行定点断言（A1…A30）；
#  ② 拒绝侧 23 例 × 三轨：rc≠0 + **同一 R 码** + **正文逐字相同** + 三轨都指到用户行；
#  ③ 反例守卫：合法侧必须**真的**跑到（`before` 之后仍有输出 ⇒ 证明是运行期校验）；
#  ④ 负控 5 道（A/B/E 改 selfhost/icall.px、C/D 改 runtime/runtime.c），各自独立判红。
#
# 用法：bash examples/m190_arity/verify.sh [--neg-skip]
# 退出码：0 = 绿，1 = 红，2 = 门自身前置自查失败。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0

RT="runtime/runtime.c"
IC="selfhost/icall.px"
BACK=/tmp/m190_gate_bak
W=/tmp/m190_gate
rm -rf "$W" "$BACK"; mkdir -p "$W" "$BACK"
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

snapshot() { cp -f "$RT" "$BACK/runtime.c"; cp -f "$IC" "$BACK/icall.px"; }
restore_all() { [ -f "$BACK/runtime.c" ] && cp -f "$BACK/runtime.c" "$RT"; [ -f "$BACK/icall.px" ] && cp -f "$BACK/icall.px" "$IC"; }
sha_rt() { sha256sum "$RT" | cut -c1-16; }
sha_ic() { sha256sum "$IC" | cut -c1-16; }
snapshot
SHA0_RT="$(sha_rt)"; SHA0_IC="$(sha_ic)"
trap 'restore_all' EXIT

# ── 前置不变量自查（防脏源码 / 防负控锚点失效）──
grep -q 'M190（缺陷 213-g）' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M190 的 list.index 补齐" >&2; exit 2; }
grep -q 'M190（缺陷 213-i）' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M190 的字符串方法族校验" >&2; exit 2; }
grep -q 'M190（缺陷 213-i）' "$IC" || { echo "❌ 前置自查失败：icall.px 缺 M190 的字符串方法族校验" >&2; exit 2; }
! grep -q 'M190-NC' "$RT" "$IC" || { echo "❌ 前置自查失败：源码里有 M190 负控残留" >&2; exit 2; }
grep -c 'if (nargs != 1) px_error("R1005: 方法 append 需要 1 个参数");' "$RT" | grep -qx 1 || { echo "❌ 前置自查失败：append 锚点不唯一" >&2; exit 2; }

# ── 三轨 runner ──
# 解释轨二进制：默认**入库件** bootstrap/pxi；开发期可 PX_INTERP_BIN=/tmp/pxidev 先验门自身
INTERP_BIN="${PX_INTERP_BIN:-$ROOT/bootstrap/pxi}"
run3() {   # run3 <源文件> <tag>
    local src="$1" tag="$2"
    local d="$W/b$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$src" "$d/"
    local b; b="$(basename "$src" .px)"
    ( cd "$d" && timeout 120 "$INTERP_BIN" "$b.px" ) > "$W/$tag.interp.out" 2>&1
    echo $? > "$W/$tag.interp.rc"
    rm -rf "$d/build"
    timeout 300 ./tools/px build "$d/$b.px" > "$W/$tag.vm.log" 2>&1
    if [ -x "$d/build/$b" ]; then timeout 60 "$d/build/$b" > "$W/$tag.vm.out" 2>&1; echo $? > "$W/$tag.vm.rc"
    else echo 999 > "$W/$tag.vm.rc"; tail -3 "$W/$tag.vm.log" > "$W/$tag.vm.out"; fi
    rm -rf "$d/build"
    PX_BUILD_ENGINE=c timeout 300 ./tools/px build "$d/$b.px" > "$W/$tag.c.log" 2>&1
    if [ -x "$d/build/$b" ]; then timeout 60 "$d/build/$b" > "$W/$tag.c.out" 2>&1; echo $? > "$W/$tag.c.rc"
    else echo 999 > "$W/$tag.c.rc"; tail -3 "$W/$tag.c.log" > "$W/$tag.c.out"; fi
    rm -rf "$d/build"
}
tri_same() { cmp -s "$W/$1.interp.out" "$W/$1.vm.out" && cmp -s "$W/$1.interp.out" "$W/$1.c.out"; }
tri_rc0() { [ "$(cat "$W/$1.interp.rc")" = 0 ] && [ "$(cat "$W/$1.vm.rc")" = 0 ] && [ "$(cat "$W/$1.c.rc")" = 0 ]; }
has() { grep -qE "$2" "$W/$1"; }

echo "=== [1] 合法侧：arity_ok.px 三轨一致（方法族参数面 · 全部合法形态）==="
run3 "$HERE/arity_ok.px" ok
chk "[1] 三轨 rc=0" "tri_rc0 ok"
chk "[1] 三轨 stdout 逐字节一致" "tri_same ok"
chk "[1] 行数=30" "[ \"\$(wc -l < $W/ok.interp.out)\" = 30 ]"
chk "[1] A1/A2=list append+push 后 pop 末元素" "has ok.interp.out '^A1= \\[3, 1, 2, 4, 5\\]$' && has ok.interp.out '^A2= 5$'"
chk "[1] A3=list.index 返回值" "has ok.interp.out '^A3= 1$'"
chk "[1] A4/A5=contains/len" "has ok.interp.out '^A4= true$' && has ok.interp.out '^A5= 4$'"
chk "[1] A6=list.reverse 原地" "has ok.interp.out '^A6= \\[4, 2, 1, 3\\]$'"
chk "[1] A7=list.sort 原地（**值比较**，非渲染串）" "has ok.interp.out '^A7= \\[1, 2, 3, 4\\]$'"
chk "[1] A8=list.join" "has ok.interp.out '^A8= 1,2,3,4$'"
chk "[1] A9/A10/A11=dict.get 三形态" "has ok.interp.out '^A9= 2$' && has ok.interp.out '^A10= null$' && has ok.interp.out '^A11= 9$'"
chk "[1] A12/A13=dict.len/keys" "has ok.interp.out '^A12= 2$' && has ok.interp.out '^A13= \\[a, b\\]$'"
chk "[1] A14…A17=str 四个 0 参方法" "has ok.interp.out '^A14= AB$' && has ok.interp.out '^A15= ab$' && has ok.interp.out '^A16= ab|$' && has ok.interp.out '^A17= 2$'"
chk "[1] A18/A19=str.split 1 参 / **0 参**（按单空格）" "has ok.interp.out '^A18= \\[a, b\\]$' && has ok.interp.out '^A19= \\[a, b, , c\\]$'"
chk "[1] A20…A23=str contains/replace/starts_with/ends_with" "has ok.interp.out '^A20= true$' && has ok.interp.out '^A21= aXc$' && has ok.interp.out '^A22= true$' && has ok.interp.out '^A23= true$'"
chk "[1] A24…A26=result is_ok/unwrap/unwrap_err" "has ok.interp.out '^A24= true$' && has ok.interp.out '^A25= 1$' && has ok.interp.out '^A26= e$'"
chk "[1] A27=tuple.len" "has ok.interp.out '^A27= 3$'"
chk "[1] A28/A29=内置 split 2 参 / 1 参" "has ok.interp.out '^A28= \\[a,b\\]$' && has ok.interp.out '^A29= \\[a, b, , c\\]$'"
chk "[1] A30=assert(true)" "has ok.interp.out '^A30= null$'"

echo "=== [2] 拒绝侧 23 例 × 三轨：rc≠0 + 同 R 码 + 正文逐字相同 + 指到用户行 ==="
# PAT = 错误**正文**（不含信封：缺陷 186 的三轨信封差异已单独登记）
# CODE = R 码   LNO = 调用所在用户行（源里 print("before") 在第 3 行）
declare -A PAT=(
  [n1_str_upper_arg]='方法 upper 不接受参数'
  [n2_str_lower_arg]='方法 lower 不接受参数'
  [n3_str_trim_arg]='方法 trim 不接受参数'
  [n4_str_len_arg]='方法 len 不接受参数'
  [n5_str_contains_2]='方法 contains 需要 1 个参数'
  [n6_str_replace_3]='方法 replace 需要 2 个参数'
  [n7_str_starts_2]='方法 starts_with 需要 1 个参数'
  [n8_str_split_2]='方法 split 需要 0-1 个参数'
  [n9_list_append_2]='方法 append 需要 1 个参数'
  [n10_list_push_2]='方法 push 需要 1 个参数'
  [n11_list_pop_1]='pop 不接受参数'
  [n12_list_len_arg]='方法 len 不接受参数'
  [n13_list_reverse_arg]='方法 reverse 不接受参数'
  [n14_list_sort_arg]='方法 sort 不接受参数'
  [n15_tuple_len_arg]='方法 len 不接受参数'
  [n16_dict_set_1]='方法 set 需要 2 个参数'
  [n17_dict_set_3]='方法 set 需要 2 个参数'
  [n18_ok_0]='Ok 需要 1 个参数'
  [n19_ok_2]='Ok 需要 1 个参数'
  [n20_unwrap_arg]='unwrap 不接受参数'
  [n21_assert_3]='assert 需要 1-2 个参数'
  [n22_index_miss]='list.index 未找到元素'
  [n23_index_2]='list.index 需要 1 个参数'
  [n24_now_arg]='now 不需要参数'
  [n25_hostname_arg]='hostname 不需要参数'
)
declare -A CODE=(
  # M191（第 69 轮）口径统一：**方法调用的实参个数**一律 R1005（此前字符串方法 / list.len / tuple.len
  #   是 R1002 —— 解释轨与 native 两侧同批改为 R1005，见 docs/ERROR_CODES.md §2.3）。
  [n1_str_upper_arg]=R1005 [n2_str_lower_arg]=R1005 [n3_str_trim_arg]=R1005 [n4_str_len_arg]=R1005
  [n5_str_contains_2]=R1005 [n6_str_replace_3]=R1005 [n7_str_starts_2]=R1005 [n8_str_split_2]=R1005
  [n9_list_append_2]=R1005 [n10_list_push_2]=R1005 [n11_list_pop_1]=R1005 [n12_list_len_arg]=R1005
  [n13_list_reverse_arg]=R1005 [n14_list_sort_arg]=R1005 [n15_tuple_len_arg]=R1005
  [n16_dict_set_1]=R1005 [n17_dict_set_3]=R1005 [n18_ok_0]=R1002 [n19_ok_2]=R1002
  [n20_unwrap_arg]=R1005 [n21_assert_3]=R1002 [n22_index_miss]=R1003 [n23_index_2]=R1005
  [n24_now_arg]=R1002 [n25_hostname_arg]=R1002
)
# ⚠️ 行号**不写死**：从三轨输出里各取一次，判据 = **三轨指到同一行**（本文件的行号常量
#   曾在首版写错 —— 生成器给每例加了 2 行头注释 ⇒ 实际行比手算多 1，15 项假红。
#   教训：判据常量要与**产物**对齐，不要与"我以为的行数"对齐。）
for c in "${!PAT[@]}"; do
    run3 "$HERE/neg/$c.px" "n_$c"
    ok=1
    for t in interp vm c; do
        [ "$(cat "$W/n_$c.$t.rc")" != 0 ] || ok=0
        grep -qE "${CODE[$c]}" "$W/n_$c.$t.out" || ok=0
        grep -qE "${PAT[$c]}" "$W/n_$c.$t.out" || ok=0
    done
    grep -qE '^before$' "$W/n_$c.interp.out" || ok=0          # 证明是运行期（编译期就不会打印）
    li="$(grep -oE "错误 \[${CODE[$c]}\] [0-9]+:" "$W/n_$c.interp.out" | head -1 | sed -E 's/.*\] ([0-9]+):/\1/')"
    lv="$(grep -oE "行[0-9]+\]" "$W/n_$c.vm.out" | head -1 | sed -E 's/行([0-9]+)\]/\1/')"
    lc="$(grep -oE "行[0-9]+\]" "$W/n_$c.c.out" | head -1 | sed -E 's/行([0-9]+)\]/\1/')"
    [ -n "$li" ] && [ "$li" = "$lv" ] && [ "$li" = "$lc" ] || ok=0
    e1="$(grep -oE "${PAT[$c]}" "$W/n_$c.interp.out" | head -1)"
    e2="$(grep -oE "${PAT[$c]}" "$W/n_$c.vm.out" | head -1)"
    e3="$(grep -oE "${PAT[$c]}" "$W/n_$c.c.out" | head -1)"
    [ -n "$e1" ] && [ "$e1" = "$e2" ] && [ "$e1" = "$e3" ] || ok=0
    chk "[2] $c：三轨 rc≠0 + ${CODE[$c]} + 同文 + 同行=$li「$e1」" "[ $ok = 1 ]"
done

if [ "$NEG" = 1 ]; then
    echo "=== [3] 负控（各自独立；源逐字节还原）==="
    neg() {   # neg <名> <判据函数> <打桩函数> <源文件> <期望 sha 变量名>
        local name="$1" fn="$2" patcher="$3" src="$4"
        restore_all
        $patcher || { echo "  FAIL 负控 $name：打桩失败"; fail=$((fail+1)); return; }
        if [ "$(sha256sum "$src" | cut -c1-16)" = "$( [ "$src" = "$RT" ] && echo "$SHA0_RT" || echo "$SHA0_IC" )" ]; then
            echo "  FAIL 负控 $name：打桩未生效（锚点未命中）"; fail=$((fail+1)); restore_all; return
        fi
        if $fn; then
            echo "  PASS 负控 $name：判据已判红（符合预期）"; pass=$((pass+1))
        else
            echo "  FAIL 负控 $name：判据仍为绿（负控没有牙）"; fail=$((fail+1))
        fi
        restore_all
    }
    # ⚠️ 负控靶在 `selfhost/icall.px`（解释器源码）⇒ 必须**用当前源码重编解释器**再跑，
    #   否则跑的仍是入库件 `bootstrap/pxi`（修复后的行为）⇒ 负控"没有牙"（首版就此假绿 3 道）。
    #   判据函数约定：返回 0 = **判据已判红**（负控生效）。
    NCNAME=unknown
    rebuild_interp() {
        rm -rf "$ROOT/selfhost/build"
        ( cd "$ROOT" && timeout 600 ./tools/px build selfhost/interp.px ) > "$W/nc_interp_build_$NCNAME.log" 2>&1
        [ -x "$ROOT/selfhost/build/interp" ] || { echo "    ⚠ 重编解释器失败（见 $W/nc_interp_build_$NCNAME.log）"; return 1; }
    }
    run_dev_interp() {   # run_dev_interp <工作目录> <源文件> <输出文件>
        ( cd "$1" && timeout 120 "$ROOT/selfhost/build/interp" "$2" ) > "$3" 2>&1
        local rc=$?
        rm -rf "$ROOT/selfhost/build"
        return $rc
    }
    # A：解释轨 pop 退回"忽略实参" ⇒ n11 必须红（解释轨由 rc=1 变 rc=0）
    patch_A() { python3 - "$IC" <<'PY'
import sys
p=sys.argv[1]; s=open(p,encoding='utf-8').read()
a='''        if len(args) != 0:
            return Err(i_r1005("pop 不接受参数", pos))'''
assert s.count(a)==1
open(p,'w',encoding='utf-8').write(s.replace(a,'        # M190-NC：负控 A 打桩（退回忽略实参）',1))
PY
    }
    fn_A() { NCNAME=A; rebuild_interp || return 1; run_dev_interp "$W/bn_n11_list_pop_1" n11_list_pop_1.px "$W/ncA.out"; [ $? = 0 ]; }
    # B：解释轨 str.upper 退回"不查实参" ⇒ n1 必须红
    patch_B() { python3 - "$IC" <<'PY'
import sys
p=sys.argv[1]; s=open(p,encoding='utf-8').read()
a='''        if len(args) != 0:
            return Err(i_r1002("方法 upper 不接受参数", pos))
        return Ok(s.upper())'''
assert s.count(a)==1
open(p,'w',encoding='utf-8').write(s.replace(a,'        # M190-NC：负控 B 打桩（退回不查实参）\n        return Ok(s.upper())',1))
PY
    }
    fn_B() { NCNAME=B; rebuild_interp || return 1; run_dev_interp "$W/bn_n1_str_upper_arg" n1_str_upper_arg.px "$W/ncB.out"; [ $? = 0 ]; }
    # C：native `list.index` 补齐退回（R1007）⇒ n22 必须红
    patch_C() { python3 - "$RT" <<'PY'
import sys
p=sys.argv[1]; s=open(p,encoding='utf-8').read()
a='''        if (strcmp(name, "index") == 0) {
            if (nargs != 1) px_error("R1005: list.index 需要 1 个参数");'''
assert s.count(a)==1
open(p,'w',encoding='utf-8').write(s.replace(a,'        // M190-NC：负控 C 打桩（退回无 index）\n        if (0) {',1))
PY
    }
    fn_C() { run3 "$HERE/neg/n22_index_miss.px" ncC; grep -q 'R1007' "$W/ncC.vm.out"; }
    # D：native `list.reverse` 补齐退回 ⇒ 合法侧 A6 必须红
    patch_D() { python3 - "$RT" <<'PY'
import sys
p=sys.argv[1]; s=open(p,encoding='utf-8').read()
a='''        if (strcmp(name, "reverse") == 0) {
            if (nargs != 0) px_error("R1005: 方法 reverse 不接受参数");'''
assert s.count(a)==1
open(p,'w',encoding='utf-8').write(s.replace(a,'        // M190-NC：负控 D 打桩（退回无 reverse）\n        if (0) {',1))
PY
    }
    fn_D() { run3 "$HERE/arity_ok.px" ncD; [ "$(cat "$W/ncD.vm.rc")" != 0 ]; }
    # E：解释轨 sort 退回**渲染串比较** ⇒ 解释轨输出必须变成 `[10, 2, 9]`
    patch_E() { python3 - "$IC" <<'PY'
import sys
p=sys.argv[1]; s=open(p,encoding='utf-8').read()
a='                if i_cmp_values(l[j], l[j + 1]) > 0:'
assert s.count(a)==1
open(p,'w',encoding='utf-8').write(s.replace(a,'                # M190-NC：负控 E 打桩（退回渲染串比较）\n                if i_to_str(l[j]) > i_to_str(l[j + 1]):',1))
PY
    }
    fn_E() {
        NCNAME=E;
        # 用**能区分两种比较口径**的输入：`[10, 9, 2]` —— 渲染串比较给 `[10, 2, 9]`，值比较给 `[2, 9, 10]`
        cat > "$W/sortprobe.px" <<'EOF'
def main():
    var l = [10, 9, 2]
    l.sort()
    print(l)
EOF
        run3 "$W/sortprobe.px" ncE_norm                       # 先建立工作目录
        rebuild_interp || return 1
        run_dev_interp "$W/bncE_norm" sortprobe.px "$W/ncE.out"
        grep -qE '^\[10, 2, 9\]$' "$W/ncE.out"
    }
    neg A fn_A patch_A "$IC"
    neg B fn_B patch_B "$IC"
    neg C fn_C patch_C "$RT"
    neg D fn_D patch_D "$RT"
    neg E fn_E patch_E "$IC"
fi

rm -rf "$HERE/build" "$ROOT/selfhost/build"    # 负控重编解释器时产生的产物
echo
echo "M190 门：PASS=$pass FAIL=$fail"
if [ "$fail" = 0 ]; then echo "M190-VERIFY-OK"; exit 0; fi
exit 1
