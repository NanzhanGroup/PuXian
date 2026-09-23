#!/usr/bin/env bash
# ============================================================
# M189 门（第 67 轮）：**bytes 边界族**（bytes_get / bytes_set / b[i]）三轨同一真相
# ------------------------------------------------------------
# 本轮收的是第三方 `banshanhanfu/registry-px` 登记的 **PX-DEF-029**
# （`bytes_get` 越界返回 null），复测时又牵出同族两条（内部编号 211/212 一族）：
#
#  ① 缺陷 211（PX-DEF-029 · **静默错值**）：`bytes_get(b, 5)` → `null`，而**同一个操作**的
#     另两个入口都响亮 —— `b[5]`（M185 统一后）与 `bytes_set(b, 5, v)` 均为
#     `R1003 索引越界: i (len=n)`，`list[i]`/`str[i]` 亦然 ⇒ **同一语义两套答案**。
#     更糟的是 null 会**流到下游**：`bytes_get(b,5) < 251` 报「无法比较 null 与 int」，
#     **指不到越界点**（二进制协议解析器首当其冲 —— 对方 mysql/pg 驱动各自手写
#     `off >= bytes_len(b)` 守卫，正是此痛的证据）。⇒ 统一为 R1003。
#
#  ② 缺陷 212-a（**同族两套文案**）：`bytes_get(b,"x")` 报「期望整数，实际是 string」
#     （走 `int_val`），而 `b["x"]` 报「索引必须是整数，实际是 string」（走 `px_req_int_idx`）。
#     同一族（bytes 的索引）两个入口两套文案 ⇒ 索引校验改走 `px_req_int_idx` 同一入口。
#
#  ③ 缺陷 212-b（**静默截断**）：`int_val` 对 float 直接 `(int64_t)` 截断 ⇒
#     `bytes_get(b, 1.5)` 静默返回 `b[1]`；而 `b[1.5]` 响亮报「索引必须是整数，实际是 float」。
#     改走 `px_req_int_idx` 后同样响亮。
#
#  ④ 同族收尾：`bytes_set` 越界词条修前是「bytes_set 下标越界」（同族第三套文案）⇒ 统一。
#
# 判据（四层正判据 + 四道负控，每层/每道可独立判红）：
#  ① `bounds_ok.px`（17 行）三轨 rc=0 且 stdout **逐字节一致**；
#     含 A8/A9 = **函数面 ⇔ 索引面全范围等价**（正索引 + 负索引逐项对拍）；
#  ② 定点断言（值语义 13 条：正/负索引取值、bytes_set 函数式、守卫模式、切片 clamp）；
#  ③ `neg/*.px`（6 用例）三轨 rc≠0 + 提取词条**逐字相同** + **解释轨必须带用户侧 行:列**
#     （`错误 [R100x] <行>:`）+ `before` 已打印（证明是运行期而非编译期）；
#  ④ 负控 4 道（A/B/D 改 runtime.c、C 改 selfhost/ibuiltin.px），各自独立判红。
#
# 负控（各自独立判红，源逐字节还原）：
#  A runtime `bi_bytes_get` 越界退回 `return px_null();`      ⇒ ③ n1 红（rc 由 1 变 0）
#  B runtime `bi_bytes_set` 越界文案退回旧文               ⇒ ③ n2 红（词条与 b[i] 不同）
#  C selfhost `ibuiltin.px` 删掉 bytes_get 前置校验        ⇒ ③ n1 红（解释轨丢掉用户行号）
#  D runtime `bi_bytes_get` 索引校验退回 `int_val`          ⇒ ③ n5/n6 红（文案分叉 + 静默截断）
#
# 用法：bash examples/m189_bytes_bounds/verify.sh [--neg-skip]
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
IB="selfhost/ibuiltin.px"
BACK=/tmp/m189_gate_bak
W=/tmp/m189_gate
rm -rf "$W" "$BACK"; mkdir -p "$W" "$BACK"
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

snapshot() { cp -f "$RT" "$BACK/runtime.c"; cp -f "$IB" "$BACK/ibuiltin.px"; }
restore_all() { [ -f "$BACK/runtime.c" ] && cp -f "$BACK/runtime.c" "$RT"; [ -f "$BACK/ibuiltin.px" ] && cp -f "$BACK/ibuiltin.px" "$IB"; }
sha_rt() { sha256sum "$RT" | cut -c1-16; }
sha_ib() { sha256sum "$IB" | cut -c1-16; }
snapshot
SHA0_RT="$(sha_rt)"; SHA0_IB="$(sha_ib)"
trap 'restore_all' EXIT

# ── 前置不变量自查（防上一轮被强杀留下的脏源码 / 防本门锚点失效）──
grep -q 'M189（第 67 轮 · 缺陷 211' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M189 越界收口" >&2; exit 2; }
grep -q 'int64_t i = px_req_int_idx(args\[1\]);' "$RT" || { echo "❌ 前置自查失败：runtime.c 索引校验未走 px_req_int_idx" >&2; exit 2; }
grep -q 'M189（第 67 轮 · 缺陷 211）' "$IB" || { echo "❌ 前置自查失败：ibuiltin.px 缺 M189 前置校验" >&2; exit 2; }
# 防「负控打桩残留」（M182 那次自伤）：本轮不该出现任何 NC 标记
! grep -q 'M189-NC' "$RT" "$IB" || { echo "❌ 前置自查失败：源码里有 M189 负控残留" >&2; exit 2; }
# 负控锚点必须唯一（改代码时若撞了旧门负控的锚点，这里会先报出来）
grep -c 'if (idx < 0 || idx >= len) px_error("R1003: 索引越界: %d (len=%d)", (int)idx, len);' "$RT" | grep -qx 2 || { echo "❌ 前置自查失败：越界锚点不再是 2 处（bytes_get/bytes_set）" >&2; exit 2; }

# ── 三轨 runner（interp 可指定二进制，供负控 C 用当前源码重编的解释器）──
# 解释轨二进制：默认入库件 bootstrap/pxi；开发期可用 PX_INTERP_BIN 指到 dev 件
# （例如 `/tmp/pxidev`），以便在重烘**之前**先验证门自身。
INTERP_BIN="${PX_INTERP_BIN:-$ROOT/bootstrap/pxi}"
run3() {   # run3 <源文件> <tag>
    local src="$1" tag="$2"
    local d="$W/b$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$src" "$d/"
    local b; b="$(basename "$src" .px)"
    rm -rf "$d/build"
    ( cd "$d" && timeout 120 "$INTERP_BIN" "$b.px" ) > "$W/$tag.interp.out" 2>&1
    echo $? > "$W/$tag.interp.rc"
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

echo "=== [1] 正判据：bytes 边界族三轨一致（含函数面 ⇔ 索引面全范围等价）"
run3 "$HERE/bounds_ok.px" ok
chk "[1] 三轨 rc=0" "tri_rc0 ok"
chk "[1] 三轨 stdout 逐字节一致" "tri_same ok"
chk "[1] 行数=17" "[ \"\$(wc -l < $W/ok.interp.out)\" = 17 ]"
chk "[1] A1/A2=bytes_get 首末字节" "has ok.interp.out '^A1= 0\$' && has ok.interp.out '^A2= 255\$'"
chk "[1] A3/A4=负索引（-1/-3）" "has ok.interp.out '^A3= 255\$' && has ok.interp.out '^A4= 0\$'"
chk "[1] A5/A6/A7=b[i] 与函数面同值" "has ok.interp.out '^A5= 0\$' && has ok.interp.out '^A6= 255\$' && has ok.interp.out '^A7= 255\$'"
chk "[1] A8=函数面 ⇔ 索引面 正索引全范围等价" "has ok.interp.out '^A8= true\$'"
chk "[1] A9=函数面 ⇔ 索引面 负索引全范围等价" "has ok.interp.out '^A9= true\$'"
chk "[1] A10/A11=bytes_set 函数式（含负索引）" "has ok.interp.out '^A10= 007fff\$' && has ok.interp.out '^A11= 001100\$'"
chk "[1] A12=原对象未被改动" "has ok.interp.out '^A12= 0011ff\$'"
chk "[1] A13/A14=守卫模式仍可用（越界由调用方判）" "has ok.interp.out '^A13= false\$' && has ok.interp.out '^A14= false\$'"
chk "[1] A15/A16/A17=切片 clamp 语义未变" "has ok.interp.out '^A15= 0011ff\$' && has ok.interp.out '^A16= 0011\$' && has ok.interp.out '^A17= 3\$'"

echo "=== [3] 拒绝侧 6 例 × 三轨：rc≠0 + 词条逐字相同 + 三轨都带用户 行（号）"
# 词条 = **错误消息正文**（不含 R10xx 码的包裹形态 —— 解释轨写 `错误 [R1003] 4:16: msg`、
#   编译轨写 `运行时错误 [<top> 行4]: R1003: msg`，这是**缺陷 186**（诊断通道/措辞三轨不同，
#   已单独登记、非本轮范围）。故此处分三条判据：① 三轨同一 R10xx 码 ② 正文逐字相同
#   ③ 三轨都要**指到用户行**（解释轨 `错误 [Rxxx] <行>:`；编译轨 `行<行>`）。
declare -A PAT=(
  [n1_get_oob]='索引越界: 3 \(len=3\)'
  [n2_set_oob]='索引越界: 3 \(len=3\)'
  [n3_neg_oob]='索引越界: -7 \(len=2\)'
  [n4_type]='bytes_get 需要 bytes，实际是 int'
  [n5_badidx]='索引必须是整数，实际是 string'
  [n6_floatidx]='索引必须是整数，实际是 float'
)
declare -A CODE=(
  [n1_get_oob]=R1003 [n2_set_oob]=R1003 [n3_neg_oob]=R1003
  [n4_type]=R1002 [n5_badidx]=R1002 [n6_floatidx]=R1002
)
declare -A LNO=(
  [n1_get_oob]=4 [n2_set_oob]=4 [n3_neg_oob]=4 [n4_type]=3 [n5_badidx]=4 [n6_floatidx]=4
)
for c in n1_get_oob n2_set_oob n3_neg_oob n4_type n5_badidx n6_floatidx; do
    run3 "$HERE/neg/$c.px" "n_$c"
    ok=1
    for t in interp vm c; do
        [ "$(cat "$W/n_$c.$t.rc")" != 0 ] || ok=0
        grep -qE "${CODE[$c]}" "$W/n_$c.$t.out" || ok=0
        grep -qE "${PAT[$c]}" "$W/n_$c.$t.out" || ok=0
    done
    grep -qE '^before$' "$W/n_$c.interp.out" || ok=0
    # 三轨都必须能从输出里读到**用户侧行号**
    grep -qE "错误 \[${CODE[$c]}\] ${LNO[$c]}:" "$W/n_$c.interp.out" || ok=0
    grep -qE "行${LNO[$c]}\]" "$W/n_$c.vm.out" || ok=0
    grep -qE "行${LNO[$c]}\]" "$W/n_$c.c.out" || ok=0
    e1="$(grep -oE "${PAT[$c]}" "$W/n_$c.interp.out" | head -1)"
    e2="$(grep -oE "${PAT[$c]}" "$W/n_$c.vm.out" | head -1)"
    e3="$(grep -oE "${PAT[$c]}" "$W/n_$c.c.out" | head -1)"
    [ -n "$e1" ] && [ "$e1" = "$e2" ] && [ "$e1" = "$e3" ] || ok=0
    chk "[3] $c：三轨 rc≠0 + ${CODE[$c]} + 词条一致 + 行号=${LNO[$c]}「$e1」" "[ $ok = 1 ]"
done

if [ "$NEG" = 1 ]; then
    echo "=== [4] 负控（各自独立；源逐字节还原）"
    neg() {   # neg <名> <判据函数> <打桩函数> <源文件>
        local name="$1" fn="$2" patcher="$3" src="$4"
        restore_all
        $patcher || { echo "  FAIL 负控 $name：打桩失败"; fail=$((fail+1)); return; }
        if [ "$(sha256sum "$src" | cut -c1-16)" = "$( [ "$src" = "$RT" ] && echo "$SHA0_RT" || echo "$SHA0_IB" )" ]; then
            echo "  FAIL 负控 $name：打桩未生效（锚点未命中）"; fail=$((fail+1)); restore_all; return
        fi
        # 约定：判据函数返回 0 = 负控生效（判据已判红）。
        # ⚠️ M185 的教训：这两个分支写反过（"没牙"被报成 PASS）⇒ 本节每道都要求
        #    「打桩后源确实变了」+「判据必须由绿转红」，两条缺一不可。
        if $fn; then
            echo "  PASS 负控 $name：判据已判红（符合预期）"; pass=$((pass+1))
        else
            echo "  FAIL 负控 $name：判据仍为绿（负控没有牙）"; fail=$((fail+1))
        fi
        restore_all
    }

    patch_A() {   # bytes_get 越界退回 null
        python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
a = '''    if (idx < 0 || idx >= len) px_error("R1003: 索引越界: %d (len=%d)", (int)idx, len);
    return px_int((unsigned char)args[0].as.obj->as.str.data[idx]);'''
assert s.count(a) == 1, 'A anchor'
s = s.replace(a, '''    if (idx < 0 || idx >= len) return px_null();   /* M189-NC-A */
    return px_int((unsigned char)args[0].as.obj->as.str.data[idx]);''')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_B() {   # bytes_set 越界文案退回旧文
        python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
a = '''    if (idx < 0 || idx >= len) px_error("R1003: 索引越界: %d (len=%d)", (int)idx, len);
    const char* src = args[0].as.obj->as.str.data;'''
assert s.count(a) == 1, 'B anchor'
s = s.replace(a, '''    if (idx < 0 || idx >= len) px_error("R1003: bytes_set 下标越界");   /* M189-NC-B */
    const char* src = args[0].as.obj->as.str.data;''')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_C() {   # 解释轨删掉 bytes_get 前置校验
        python3 - <<'PY'
p = 'selfhost/ibuiltin.px'
s = open(p, encoding='utf-8').read()
a = '''        if type(args[0]) != "bytes":
            return Err(i_r1002("bytes_get 需要 bytes，实际是 " + i_type_name(args[0]), pos))
        let bgr = i_as_index(args[1], bytes_len(args[0]), pos)
        if bgr.is_err():
            return Err(bgr.err())
        return Ok(bytes_get(args[0], bgr.unwrap()))'''
assert s.count(a) == 1, 'C anchor'
s = s.replace(a, '''        # M189-NC-C：前置校验被移除（应退化到解释器内部帧，丢掉用户行号）
        return Ok(bytes_get(args[0], args[1]))''')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_D() {   # bytes_get 索引校验退回 int_val
        python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
a = '''    int64_t i = px_req_int_idx(args[1]);
    int len = args[0].as.obj->as.str.len;'''
assert s.count(a) == 1, 'D anchor'
s = s.replace(a, '''    int64_t i = int_val(args[1]);   /* M189-NC-D */
    int len = args[0].as.obj->as.str.len;''')
open(p, 'w', encoding='utf-8').write(s)
PY
    }

    # ⚠️ 判据函数**绝不能**先调 restore_all（会把刚打好的桩还原掉 ⇒ 负控永远"没牙"）。
    chk_A_red() {   # n1 由「rc=1 + R1003」退化成「rc=0 + null」（且 null 会往下流）
        # 只判**编译轨两轨**：解释轨的前置校验在 .px 里，与 runtime 的这条分支无关
        # （那是负控 C 的靶子）—— 两道控制各有各的靶心，不重叠。
        run3 "$HERE/neg/n1_get_oob.px" ncA
        ! grep -q 'R1003' "$W/ncA.vm.out" && ! grep -q 'R1003' "$W/ncA.c.out"
    }
    chk_B_red() {   # n2 词条与 b[i] 分叉
        run3 "$HERE/neg/n2_set_oob.px" ncB
        ! grep -q '索引越界: 3 (len=3)' "$W/ncB.vm.out"
    }
    chk_C_red() {   # 解释轨丢掉用户行号（用当前 selfhost 源码重编解释器）
        ( cd "$ROOT" && ./tools/px build selfhost/interp.px ) > "$W/ncc_build.log" 2>&1
        [ -x "$ROOT/selfhost/build/interp" ] || return 1
        ( cd "$W/bn_n1_get_oob" && timeout 120 "$ROOT/selfhost/build/interp" n1_get_oob.px ) > "$W/ncc.out" 2>&1
        local rc=$?
        rm -rf "$ROOT/selfhost/build"
        [ "$rc" != 0 ] && ! grep -qE '错误 \[R100[23]\] 4:' "$W/ncc.out"
    }
    chk_D_red() {   # n5/n6：文案分叉 + float 静默截断
        run3 "$HERE/neg/n5_badidx.px" ncD
        ! grep -q '索引必须是整数' "$W/ncD.vm.out" || return 1
        run3 "$HERE/neg/n6_floatidx.px" ncE
        ! grep -q 'R1002' "$W/ncE.vm.out"
    }

    neg A chk_A_red patch_A "$RT"
    neg B chk_B_red patch_B "$RT"
    neg C chk_C_red patch_C "$IB"
    neg D chk_D_red patch_D "$RT"
    restore_all
    [ "$(sha_rt)" = "$SHA0_RT" ] || { echo "  FAIL runtime.c 未逐字节还原"; fail=$((fail+1)); }
    [ "$(sha_ib)" = "$SHA0_IB" ] || { echo "  FAIL ibuiltin.px 未逐字节还原"; fail=$((fail+1)); }
else
    echo "=== [4] 负控已跳过（--neg-skip）"
fi

rm -rf "$HERE/build"
echo
echo "M189 门结果：PASS=$pass FAIL=$fail"
[ "$fail" = 0 ] && echo "M189-VERIFY-OK" || echo "M189-VERIFY-FAILED"
exit $([ "$fail" = 0 ] && echo 0 || echo 1)
