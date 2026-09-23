#!/usr/bin/env bash
# ============================================================
# M188 门（第 66 轮）：SHA1 族 + 三件「安全替代」+ 字符串方法面四别名
# ------------------------------------------------------------
# 本轮收的是第三方 `banshanhanfu/registry-px` 登记的四条（在三轨上逐条复现后收口）：
#
#  ① PX-DEF-026 **没有 `sha1`**：MySQL `mysql_native_password` 的应答是
#       SHA1(password) XOR SHA1(salt ‖ SHA1(SHA1(password)))
#     ⇒ 驱动（对方 T2 的 `mysql` 库）只能用纯 .px 手搓 SHA1。补 `sha1` / `sha1_bytes`，
#     族口径与 `sha256`/`md5` **逐条对齐**（收 str|bytes|数值、二进制安全、hex 小写）。
#
#  ② PX-DEF-016 `sha256` 只给 hex ⇒ 想再哈希一次必须 `hex_to_bytes` 往返，而
#     `sha256(sha256(x))` 实际是对 **hex 文本** 求哈希（**双哈希陷阱**，静默错值）。补 `sha256_bytes`。
#  ③ PX-DEF-012 `bytes(10)` 是字符串 "10" 的 **2 字节**（不是 10 个字节）⇒ 补 `bytes_zeros(n)`。
#  ④ PX-DEF-017 `chr(185)` 给的是 UTF-8 编码后的 **2 字节** `c2b9` ⇒ 补 `byte(n)`（原样单字节）。
#     （②③④ 都是"命名/构造器缺口"：现有名字能到，但**语义与直觉不符**且没有显式替代。）
#
#  ⑤ PX-DEF-025/027 **字符串方法面窄**：`"abc".find("b")` / `"  x ".strip()` /
#     `"ab".has_prefix("a")` 全部 `R1007 类型 string 没有方法 'find'` —— 而函数面
#     （`trim`/`starts_with`/`ends_with`）与库面（`std.strings.index_of`）都在，
#     缺的只是**最常用的调用形态**（Go 的 `strings.Index` / `s.HasPrefix` 习惯）。
#     补 `.find` / `.strip` / `.has_prefix` / `.has_suffix`，**只加路由不复制实现**：
#       .find(sub)      → `str_index_of(s, sub)`（新增 native，rune 轴）
#       .strip()        → `trim(s)`
#       .has_prefix(p)  → `starts_with(s, p)`
#       .has_suffix(p)  → `ends_with(s, p)`
#
# 判据（四层正判据 + 四道负控，每层/每道可独立判红）：
#  ① `sha1_family.px`（18 行）三轨 rc=0 且 stdout **逐字节一致** + 已知向量定点（S1/S3/S6 为 NIST 向量）；
#  ② `str_face.px`（28 行）同上，含 ①rune 轴定点（F2/E5 = **2**，字节轴会给 6）
#     ②方法面⇔函数面等价性（E1..E4）③方法面⇔stdlib `index_of` 同轴（X1..X4）；
#  ③ `neg/*.px`（5 用例）三轨 rc≠0 且**词条逐字相同**（位置前缀允许不同 —— 缺陷 186）；
#  ④ 负控 4 道（A/B/C 改 runtime.c、D 改 selfhost/icall.px）。
#
# 负控（各自独立判红，源逐字节还原）：
#  A runtime `px_method` 删掉 `find` 路由 ⇒ ② 红（回到 R1007）
#  B runtime `bi_sha1_bytes` 改成返回 16 字节 ⇒ ① 红（长度/向量定点）
#  C runtime `px_str_index_of_runes` 退化为**字节轴**（返回字节下标）⇒ ② 红（F2 由 2 变 6）
#  D selfhost `icall.px` 删掉 `strip` 分支（用当前 selfhost 源码重编解释器跑）⇒ ② 红
#
# 用法：bash examples/m188_std_face/verify.sh [--neg-skip]
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
BACK=/tmp/m188_gate_bak
W=/tmp/m188_gate
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

# ── 前置不变量自查（防上一轮被强杀留下的脏源码 / 防本门锚点失效）──
grep -q 'M188-SHA1-FAMILY' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M188 SHA1 族" >&2; exit 2; }
grep -q 'M188-STR-FACE' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M188 方法面" >&2; exit 2; }
grep -q 'bi_str_index_of' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 bi_str_index_of" >&2; exit 2; }
grep -q 'M188-FWD' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M188 前向声明" >&2; exit 2; }
grep -q 'M188-STR-FACE' "$IC" || { echo "❌ 前置自查失败：icall.px 缺 M188 方法面" >&2; exit 2; }
grep -q 'M188（第 66 轮 · PX-DEF-026/012/016/017/025）' selfhost/ibuiltin.px || { echo "❌ 前置自查失败：ibuiltin.px 缺 M188 转发" >&2; exit 2; }
grep -q '"sha1", "sha1_bytes", "sha256_bytes", "bytes_zeros", "byte", "str_index_of",' selfhost/interp.px || { echo "❌ 前置自查失败：interp.px 名册缺 M188 六名" >&2; exit 2; }

# ── 三轨 runner ──
run3() {   # run3 <源文件> <tag>
    local src="$1" tag="$2"
    local d="$W/b$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$src" "$d/"
    local b; b="$(basename "$src" .px)"
    rm -rf "$d/build"
    ( cd "$d" && timeout 120 "$ROOT/bootstrap/pxi" "$b.px" ) > "$W/$tag.interp.out" 2>&1
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

echo "=== [1] SHA1 族 + 安全替代：三轨 rc=0 + stdout 逐字节一致 + 已知向量"
run3 "$HERE/sha1_family.px" sha
chk "[1] 三轨 rc=0" "tri_rc0 sha"
chk "[1] 三轨 stdout 逐字节一致" "tri_same sha"
chk "[1] 行数=18" "[ \"\$(wc -l < $W/sha.interp.out)\" = 18 ]"
chk "[1] S1=SHA1(abc) NIST 向量" "has sha.interp.out '^S1= a9993e364706816aba3e25717850c26c9cd0d89d\$'"
chk "[1] S2=SHA1(\"\") 向量" "has sha.interp.out '^S2= da39a3ee5e6b4b0d3255bfef95601890afd80709\$'"
chk "[1] S3=SHA1(quick brown fox) 向量" "has sha.interp.out '^S3= 2fd4e1c67a2d28fced849ee1bb76e7391b93eb12\$'"
chk "[1] S5=sha1_bytes 给 20 字节" "has sha.interp.out '^S5= 20\$'"
chk "[1] S6=SHA256(abc) NIST 向量" "has sha.interp.out '^S6= ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\$'"
chk "[1] S7=sha256_bytes 给 32 字节" "has sha.interp.out '^S7= 32\$'"
chk "[1] S8=bytes_zeros(4)" "has sha.interp.out '^S8= 00000000\$'"
chk "[1] S9=bytes_zeros(0) 长度 0" "has sha.interp.out '^S9= 0\$'"
chk "[1] S10=bytes_zeros(1024) 长度 1024" "has sha.interp.out '^S10= 1024\$'"
chk "[1] S13=byte(185) 单字节 b9（PX-DEF-017）" "has sha.interp.out '^S13= b9\$'"
chk "[1] S14=len(chr(185)) rune 轴 =1" "has sha.interp.out '^S14= 1\$'"
chk "[1] S15=bytes_len(bytes(chr(185))) 字节轴 =2" "has sha.interp.out '^S15= 2\$'"
chk "[1] S16=bytes(chr(185)) = c2b9（UTF-8 2 字节）" "has sha.interp.out '^S16= c2b9\$'"
chk "[1] S17=SHA1(SHA1(pw)) 可用" "has sha.interp.out '^S17= [0-9a-f]{40}\$'"
chk "[1] S18=sha1(s) == hex(sha1_bytes(s))" "has sha.interp.out '^S18= true\$'"

echo "=== [2] 字符串方法面四别名：三轨一致 + rune 轴 + 等价性"
run3 "$HERE/str_face.px" str
chk "[2] 三轨 rc=0" "tri_rc0 str"
chk "[2] 三轨 stdout 逐字节一致" "tri_same str"
chk "[2] 行数=28" "[ \"\$(wc -l < $W/str.interp.out)\" = 28 ]"
chk "[2] F1=find 基本" "has str.interp.out '^F1= 6\$'"
chk "[2] F2=find **rune 轴**（中文各占 1）" "has str.interp.out '^F2= 2\$'"
chk "[2] F3=find 首字符" "has str.interp.out '^F3= 0\$'"
chk "[2] F4=find 不存在 ⇒ -1" "has str.interp.out '^F4= -1\$'"
chk "[2] F5=空子串 ⇒ 0" "has str.interp.out '^F5= 0\$'"
chk "[2] F7=空串找非空 ⇒ -1" "has str.interp.out '^F7= -1\$'"
chk "[2] F8=首次出现优先" "has str.interp.out '^F8= 0\$'"
chk "[2] F9=首次出现优先(2)" "has str.interp.out '^F9= 1\$'"
chk "[2] F10=strip 去两端空白" "has str.interp.out '^F10= x\$'"
chk "[2] F11=strip 去 \\t\\n\\r" "has str.interp.out '^F11= y\$'"
chk "[2] F12=全空白 ⇒ 空串" "has str.interp.out '^F12= \$'"
chk "[2] F14=has_prefix 真" "has str.interp.out '^F14= true\$'"
chk "[2] F15=has_prefix 假" "has str.interp.out '^F15= false\$'"
chk "[2] F16=has_suffix 真" "has str.interp.out '^F16= true\$'"
chk "[2] F17=has_suffix 假" "has str.interp.out '^F17= false\$'"
chk "[2] F18=比自身长 ⇒ 假" "has str.interp.out '^F18= false\$'"
chk "[2] 方法⇔函数面等价性齐（E1..E6）" "grep -cE '^E[1-6]= true\$' $W/str.interp.out | grep -qx 6"
chk "[2] 与 stdlib index_of 同轴（X1..X4）" "grep -cE '^X[1-4]= true\$' $W/str.interp.out | grep -qx 4"

echo "=== [3] 拒绝侧 5 例 × 三轨：rc≠0 + 词条逐字相同"
declare -A PAT=(
  [find_arg]='方法 find 参数 1 需要 string'
  [strip_arg]='方法 strip 不接受参数'
  # M191（第 69 轮）口径变更：**缺参**是「实参个数」错 ⇒ 文案改为「需要 1 个参数」并带 R1005
  #   （此前解释轨把个数错复用了类型文案「参数 1 需要 string」，与 native 拆开后不一致）。
  [has_prefix_arg]='方法 has_prefix 需要 1 个参数'
  [sha1_arg]='sha1 需要一个参数'
  [byte_range]='byte 取值范围 0\.\.255，实际是 300'
)
for c in find_arg strip_arg has_prefix_arg sha1_arg byte_range; do
    run3 "$HERE/neg/$c.px" "n_$c"
    ok=1
    for t in interp vm c; do
        [ "$(cat "$W/n_$c.$t.rc")" != 0 ] || ok=0
        grep -qE "${PAT[$c]}" "$W/n_$c.$t.out" || ok=0
    done
    grep -qE '^before$' "$W/n_$c.interp.out" || ok=0
    e1="$(grep -oE "${PAT[$c]}" "$W/n_$c.interp.out" | head -1)"
    e2="$(grep -oE "${PAT[$c]}" "$W/n_$c.vm.out" | head -1)"
    e3="$(grep -oE "${PAT[$c]}" "$W/n_$c.c.out" | head -1)"
    [ -n "$e1" ] && [ "$e1" = "$e2" ] && [ "$e1" = "$e3" ] || ok=0
    chk "[3] $c：三轨 rc≠0 + 词条一致「$e1」" "[ $ok = 1 ]"
done

echo
if [ "$NEG" = 1 ]; then
    echo "=== [4] 负控（各自独立；源逐字节还原）"
    neg() {   # neg <名> <判据函数> <打桩函数> <源文件>
        local name="$1" fn="$2" patcher="$3" src="$4"
        restore_all
        $patcher || { echo "  FAIL 负控 $name：打桩失败"; fail=$((fail+1)); return; }
        # 打桩后源必须变（防"锚点未命中却以为在跑"）
        if [ "$(sha256sum "$src" | cut -c1-16)" = "$( [ "$src" = "$RT" ] && echo "$SHA0_RT" || echo "$SHA0_IC" )" ]; then
            echo "  FAIL 负控 $name：打桩未生效（锚点未命中）"; fail=$((fail+1)); restore_all; return
        fi
        # 约定：检查函数返回 **0 = 负控生效（判据已判红）**，非零 = 负控没有牙。
        if $fn; then
            echo "  PASS 负控 $name：判据已判红（符合预期）"; pass=$((pass+1))
        else
            echo "  FAIL 负控 $name：判据仍为绿（负控没有牙）"; fail=$((fail+1))
        fi
        restore_all
    }

    patch_A() {   # px_method 删掉 find 路由
        python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
# M191：锚点更新 —— 该守卫已按 §2.3 拆成「个数(R1005) + 类型(R1002)」两条
a = '''        if (strcmp(name, "find") == 0) {
            if (nargs != 1) px_error("R1005: 方法 find 需要 1 个参数");
            if (args[0].type != PX_STR) px_error("R1002: 方法 find 参数 1 需要 string");
            return px_int(px_str_index_of_runes(obj.as.obj, args[0].as.obj));
        }
'''
assert s.count(a) == 1, 'A anchor'
s = s.replace(a, '        /* M188-NC-A：find 路由被移除 */\n')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_B() {   # bi_sha1_bytes 给 16 字节（错长度）
        python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
a = '''    if (mbedtls_sha1(d, (size_t)n, digest) != 0) px_error("sha1 计算失败");
    return px_bytes_len(digest, 20);'''
assert s.count(a) == 1, 'B anchor'
s = s.replace(a, '''    if (mbedtls_sha1(d, (size_t)n, digest) != 0) px_error("sha1 计算失败");
    return px_bytes_len(digest, 16);   /* M188-NC-B：长度写错 */''')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_C() {   # px_str_index_of_runes 退化为字节轴
        python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
a = '''        if (w == ublen && memcmp(s + sbi, u, (size_t)w) == 0) return i;'''
assert s.count(a) == 1, 'C anchor'
s = s.replace(a, '''        if (w == ublen && memcmp(s + sbi, u, (size_t)w) == 0) return sbi;   /* M188-NC-C：字节轴 */''')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_D() {   # 解释轨删掉 strip 分支
        python3 - <<'PY'
p = 'selfhost/icall.px'
s = open(p, encoding='utf-8').read()
a = '''    if name == "strip":
        if len(args) != 0:
            return Err(i_r1005("方法 strip 不接受参数", pos))
        return Ok(s.trim())
'''
assert s.count(a) == 1, 'D anchor'
s = s.replace(a, '    # M188-NC-D：strip 分支被移除\n')
open(p, 'w', encoding='utf-8').write(s)
PY
    }

    # ⚠️ 判据函数**绝不能**先调 restore_all（会把刚打好的桩还原掉 ⇒ 负控永远"没牙"）。
    chk_find_red() { run3 "$HERE/str_face.px" ncA; if tri_same ncA && tri_rc0 ncA; then return 1; else return 0; fi }
    chk_sha_red()  { run3 "$HERE/sha1_family.px" ncB; if tri_same ncB && tri_rc0 ncB && has ncB.interp.out '^S5= 20$'; then return 1; else return 0; fi }
    chk_axis_red() { run3 "$HERE/str_face.px" ncC; if tri_same ncC && tri_rc0 ncC && has ncC.interp.out '^F2= 2$'; then return 1; else return 0; fi }
    chk_strip_red() {   # 解释轨必须回到"没有方法 'strip'"（用当前 selfhost 源码重编解释器）
        ( cd "$ROOT" && ./tools/px build selfhost/interp.px ) > "$W/ncd_build.log" 2>&1
        [ -x "$ROOT/selfhost/build/interp" ] || return 1
        ( cd "$W/bstr" && timeout 120 "$ROOT/selfhost/build/interp" str_face.px ) > "$W/ncd.out" 2>&1
        local rc=$?
        rm -rf "$ROOT/selfhost/build"
        [ "$rc" != 0 ] && grep -q "没有方法 'strip'" "$W/ncd.out"
    }

    neg A chk_find_red patch_A "$RT"
    neg B chk_sha_red  patch_B "$RT"
    neg C chk_axis_red patch_C "$RT"
    neg D chk_strip_red patch_D "$IC"
    restore_all
    [ "$(sha_rt)" = "$SHA0_RT" ] || { echo "  FAIL runtime.c 未逐字节还原"; fail=$((fail+1)); }
    [ "$(sha_ic)" = "$SHA0_IC" ] || { echo "  FAIL icall.px 未逐字节还原"; fail=$((fail+1)); }
else
    echo "=== [4] 负控已跳过（--neg-skip）"
fi

rm -rf "$HERE/build"
echo
echo "M188 门结果：PASS=$pass FAIL=$fail"
[ "$fail" = 0 ] && echo "M188-VERIFY-OK" || echo "M188-VERIFY-FAILED"
exit $([ "$fail" = 0 ] && echo 0 || echo 1)
