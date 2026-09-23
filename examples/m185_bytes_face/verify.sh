#!/usr/bin/env bash
# ============================================================
# M185 门（第 63 轮）：`bytes` 三面统一 + 严格解析判定器 + Result 对偶 + `px_to_string` 收口
# ------------------------------------------------------------
# 四组缺陷（前三条来自第三方 `banshanhanfu/registry-px` 的登记/复测，第四条是本轮顺带照出的）：
#
#  ① **缺陷 203 · `bytes` 的「索引 / 切片 / 迭代」三面在三轨上四种行为**（修前实测）：
#       · `b[a:b]` 切片：解释轨 `R1002 此类型不支持切片`（VM/C 轨按字节可用）
#       · `b[i]`   索引：**三轨都缺**（`R1002 此类型不支持索引: bytes`）
#       · `for x in b` 迭代：解释轨 `R1002 此类型不可迭代` / VM·C 轨 `不支持索引: bytes`
#     ⇒ **同一份源码三种报错词条**；而速查表事实 22/35 却宣称 `b[i]` 可用（文档与实现不符）。
#     统一语义（对齐 Python `bytes` / Go `[]byte`）：`len(b)`=字节数、`b[i]`=**int 字节值**（0..255）、
#     `b[a:b]`=**bytes**（按字节）、`for x in b`=逐**字节值** int；越界/负索引/非整数索引
#     一律与 list/string **同码同文**（R1003 / R1002）。
#
#  ② **`is_int_str(s)` / `is_float_str(s)`**（M184 严格解析的**配套**缺失）：
#     M184 把 `int()/float()` 收紧为"整体合法"之后，**任何容错解析都必须先判断**，
#     而语言层没有判定器 ⇒ 第三方 `cli` 库自写 `cli_is_int_str`、我方 `cookiejar` 也写了一遍。
#     语义 = **与 `int()`/`float()` 同一个谓词**（转发同一份 `px_str_to_i64`/`px_str_to_f64`
#     ⇒ 判定与转换不可能漂移）；非 string ⇒ `false`（谓词语义；false 是**拒绝**方向）。
#
#  ③ **`unwrap_err()`**（第三方 PX-DEF-003）：`unwrap` 的**对偶** —— 断言必为 Err 并取值；
#     Ok 上调用 ⇒ 响亮 `R1004`（不静默返回 null）。`ok()/err()` 是**查询**语义（另一侧给 null），
#     写库要的是「断言 + 取值」这一个动作。
#
#  ④ **缺陷 205 · `px_to_string` 的四重病灶**（错误路径/边界类型，长期没被发现）：
#     ① 容器一律返回硬编码 `"<object>"` ⇒ `join(",", [[1,2],[3,4]])` = `"<object>,<object>"`
#        （**静默错值**），而解释轨 `join` 走 `i_to_str` = `"[1, 2],[3, 4]"` ⇒ **三轨分叉**；
#        同一条路还喂着 write_file/append/env/HTTP 体/unwrap 消息（五处）；
#     ② 每次调用把值**打印到 stdout**（`px_print_value` 写的是 stdout，不是那个 tmpfile）
#        ⇒ **未经请求的副作用**（上例往程序输出插了 4 段容器文本）；
#     ③ 每次调用 `tmpfile()` **从不 fclose** ⇒ **fd 泄漏**（两元素 join 漏 4 个 fd）；
#     ④ `px_vhost_normalize` 对返回值 `xfree(s)` ⇒ 可能是**字符串字面量** = UB。
#     修法：容器走 `px_fmt_value_n`（与 `str()`/`print` 同一渲染器）+ **线程局部单槽**，
#     删 tmpfile、删非法 xfree；`route_normalize` 另备 TLS 缓冲（body 要活到发送前）。
#
# 判据（四层正判据 + 四道负控，每层/每道可独立判红）：
#  ① `bytes_face.px`（25 行）三轨 rc=0 且 stdout **逐字节一致**；
#  ② `pred_sem.px`（60 行）同上，且含 `is_int_str(s) ⇒ int(s)` 的等价性定点；
#  ③ `render_sem.px`（12 行）同上 + **fd 增量 = 0** + **stdout 行数恰为 12**（证明无副作用）；
#  ④ `neg/*.px`（4 用例）三轨 rc≠0 且提取出的词条**逐字相同**（位置前缀允许不同 —— 缺陷 186）。
#
# 负控（各自独立判红，源逐字节还原）：
#  A runtime `px_index` 删掉 bytes 分支 ⇒ ① 红（VM/C 回到"不支持索引"）
#  B runtime `px_tostr_n` 容器分支退回硬编码 "<object>" ⇒ ③ 红
#  C runtime `bi_is_int_str` 退化为恒真 ⇒ ② 红
#  D selfhost `ival.px` 删掉 `i_iter` 的 bytes 分支 ⇒ 重编 dev 解释器后 ① 红
#
# 用法：bash examples/m185_bytes_face/verify.sh [--neg-skip]
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
IV="selfhost/ival.px"
BACK=/tmp/m185_gate_bak
W=/tmp/m185_gate
rm -rf "$W" "$BACK"; mkdir -p "$W" "$BACK"
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

snapshot() { cp -f "$RT" "$BACK/runtime.c"; cp -f "$IV" "$BACK/ival.px"; }
restore_all() { [ -f "$BACK/runtime.c" ] && cp -f "$BACK/runtime.c" "$RT"; [ -f "$BACK/ival.px" ] && cp -f "$BACK/ival.px" "$IV"; }
sha_rt() { sha256sum "$RT" | cut -c1-16; }
sha_iv() { sha256sum "$IV" | cut -c1-16; }
snapshot
SHA0_RT="$(sha_rt)"; SHA0_IV="$(sha_iv)"
trap 'restore_all' EXIT

# ── 前置不变量自查（防上一轮被强杀留下的脏源码 / 防本门锚点失效）──
grep -q 'M185（第 63 轮 · 缺陷 203）：\*\*bytes 整数索引' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 M185 bytes 索引分支" >&2; exit 2; }
grep -q 'bi_is_int_str' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 bi_is_int_str" >&2; exit 2; }
grep -q 'unwrap_err' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 unwrap_err" >&2; exit 2; }
grep -q 'px_tostr_n' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 px_tostr_n" >&2; exit 2; }
grep -q 'g_tostr_buf' "$RT" || { echo "❌ 前置自查失败：runtime.c 缺 TLS 渲染槽" >&2; exit 2; }
grep -q 'bytes \*\*迭代\*\*' "$IV" || { echo "❌ 前置自查失败：ival.px 缺 bytes 迭代分支" >&2; exit 2; }
grep -q 'bytes 整数索引' "$IV" || { echo "❌ 前置自查失败：ival.px 缺 bytes 索引分支" >&2; exit 2; }
grep -q 'bytes 切片' "$IV" || { echo "❌ 前置自查失败：ival.px 缺 bytes 切片分支" >&2; exit 2; }

# ── 三轨 runner ──
run3() {   # run3 <源文件> <tag>
    local src="$1" tag="$2"
    local d="$W/b$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$src" "$d/"
    local b; b="$(basename "$src" .px)"
    rm -rf "$d/build"
    ( cd "$d" && timeout 60 "$ROOT/bootstrap/pxi" "$b.px" ) > "$W/$tag.interp.out" 2>&1
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

echo "=== [1] bytes 三面（索引/切片/迭代）：三轨 rc=0 + stdout 逐字节一致"
run3 "$HERE/bytes_face.px" face
chk "[1] 三轨 rc=0" "tri_rc0 face"
chk "[1] 三轨 stdout 逐字节一致" "tri_same face"
chk "[1] 行数=25" "[ \"\$(wc -l < $W/face.interp.out)\" = 25 ]"
chk "[1] 定点 i0=97" "has face.interp.out '^i0= 97$'"
chk "[1] 定点 im1=100（负索引）" "has face.interp.out '^im1= 100$'"
chk "[1] 定点 s13=bc" "has face.interp.out '^s13= bc$'"
chk "[1] 定点 srev=dcba（反向步长）" "has face.interp.out '^srev= dcba$'"
chk "[1] 定点迭代=字节值" "has face.interp.out '^iter= \[97, 98, 99, 100\]$'"
chk "[1] 定点中文按字节迭代" "has face.interp.out '^zhiter= \[228, 189, 160, 229, 165, 189\]$'"
chk "[1] 定点原始字节 255" "has face.interp.out '^ridx= 255$'"
chk "[1] 索引与切片同轴" "has face.interp.out '^same= true$'"
chk "[1] 负索引与切片同轴" "has face.interp.out '^same2= true$'"

echo "=== [2] 严格解析判定器：与 int()/float() 同谓词 + 三轨一致"
run3 "$HERE/pred_sem.px" pred
chk "[2] 三轨 rc=0" "tri_rc0 pred"
chk "[2] 三轨 stdout 逐字节一致" "tri_same pred"
chk "[2] 行数=60" "[ \"\$(wc -l < $W/pred.interp.out)\" = 60 ]"
chk "[2] 定点 int(12)=true" "has pred.interp.out '^I 0 12 true$'"
chk "[2] 定点 int(12ab)=false" "has pred.interp.out '^I 7 12ab false$'"
chk "[2] 定点前导零=true" "has pred.interp.out '^I 13 00000000000000000000000123 true$'"
chk "[2] 定点溢出=false" "has pred.interp.out '^I 15 9223372036854775808 false$'"
chk "[2] 定点 float(1e3)=true" "has pred.interp.out '^F 3 1e3 true$'"
chk "[2] 定点 float(1.2.3)=false" "has pred.interp.out '^F 6 1.2.3 false$'"
chk "[2] 定点 infinity" "has pred.interp.out '^F 14 infinity true$'"
chk "[2] 非 string ⇒ false" "has pred.interp.out '^N1= false$'"
chk "[2] 非 string(list) ⇒ false" "has pred.interp.out '^N6= false$'"
chk "[2] 等价性 E1" "has pred.interp.out '^E1= true$'"
chk "[2] 等价性 E4（float）" "has pred.interp.out '^E4= true$'"
chk "[2] 等价性 E6（含空白）" "has pred.interp.out '^E6= true$'"
chk "[2] unwrap_err 在 Err 上取值" "has pred.interp.out '^U2= true$'"
chk "[2] err() 查询语义在 Ok 上给 null" "has pred.interp.out '^U3= true true$'"
chk "[2] 判定器 ↔ 转换 等价性齐（E1..E6）" "grep -cE '^E[1-6]= true$' $W/pred.interp.out | grep -qx 6"

echo "=== [3] px_to_string 收口（缺陷 205）：容器渲染 + 无 stdout 副作用 + 无 fd 泄漏"
run3 "$HERE/render_sem.px" rend
chk "[3] 三轨 rc=0" "tri_rc0 rend"
chk "[3] 三轨 stdout 逐字节一致" "tri_same rend"
chk "[3] stdout 行数恰为 12（无未经请求的容器打印）" "[ \"\$(wc -l < $W/rend.interp.out)\" = 12 ]"
has rend.interp.out '^A1 join= \[1, 2\],\[3, 4\]$' || chk "[3] join 容器=list 渲染" false
has rend.interp.out '^A2 join= \{a: 1\}-\{b: 2\}$' || chk "[3] join 容器=dict 渲染" false
has rend.interp.out '^A3 join= 1\|2.5\|true\|null$' || chk "[3] join 标量族" false
has rend.interp.out '^A4 join= <bytes 2>,x$' || chk "[3] join bytes=<bytes 2>" false
has rend.interp.out '^B1 strlist= \[1, 2\]$' || chk "[3] str(list) 同渲染器" false
has rend.interp.out '^B3 strbytes= <bytes 2>$' || chk "[3] str(bytes) 同渲染器" false
chk "[3] 无 fd 泄漏（fd 增量 = 0）" "grep -qE '^C2 fddelta= 0\$' $W/rend.interp.out && grep -qE '^C2 fddelta= 0\$' $W/rend.vm.out && grep -qE '^C2 fddelta= 0\$' $W/rend.c.out"
chk "[3] 200 次 join 结果仍正确（len=13）" "grep -qE '^C1 len= 13\$' $W/rend.interp.out && grep -qE '^C1 len= 13\$' $W/rend.c.out"

echo "=== [4] 拒绝侧 4 例 × 三轨：rc≠0 + 词条逐字相同"
declare -A PAT=(
  [oob]='索引越界: 5 \(len=2\)'
  [oobneg]='索引越界: -3 \(len=2\)'
  [badidx]='索引必须是整数，实际是 string'
  [ueok]='unwrap_err 失败: Ok\(\{a: 1\}\)'
)
for c in oob oobneg badidx ueok; do
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
    chk "[4] $c：三轨 rc≠0 + 词条一致「$e1」" "[ $ok = 1 ]"
done

echo
if [ "$NEG" = 1 ]; then
    echo "=== [5] 负控（各自独立；源逐字节还原）"
    neg() {   # neg <名> <判据函数> <打桩函数> <源文件>
        local name="$1" fn="$2" patcher="$3" src="$4"
        restore_all
        $patcher || { echo "  FAIL 负控 $name：打桩失败"; fail=$((fail+1)); return; }
        # 打桩后源必须变（防"锚点未命中却以为在跑"）
        if [ "$(sha256sum "$src" | cut -c1-16)" = "$( [ "$src" = "$RT" ] && echo "$SHA0_RT" || echo "$SHA0_IV" )" ]; then
            echo "  FAIL 负控 $name：打桩未生效（锚点未命中）"; fail=$((fail+1)); restore_all; return
        fi
        # 约定：检查函数返回 **0 = 负控生效（判据已判红）**，非零 = 负控没有牙。
        # ⚠️ M185 本节第一版把这两个分支写反了（把"负控没牙"报成 PASS）—— 门自身
        #    的负控也会骗人，所以本节每一道都要求"打桩后源确实变了" + 判据必须**由绿转红**，
        #    两条缺一不可。
        if $fn; then
            echo "  PASS 负控 $name：判据已判红（符合预期）"; pass=$((pass+1))
        else
            echo "  FAIL 负控 $name：判据仍为绿（负控没有牙）"; fail=$((fail+1))
        fi
        restore_all
    }

    patch_A() {   # px_index 的 bytes 分支退回 px_error
        python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
a = '        return px_int((int64_t)(unsigned char)obj.as.obj->as.str.data[i]);\n    }\n    if (obj.type == PX_DICT) {'
assert s.count(a) == 1, 'A anchor'
s = s.replace(a, '        (void)i; px_error("R1002: 此类型不支持索引: bytes");   /* M185-NC-A */\n    }\n    if (obj.type == PX_DICT) {')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_B() {   # px_tostr_n 容器分支退回硬编码 "<object>"
        python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
a = '    int n = 0;\n    char* s = px_fmt_value_n(v, &n);\n    if (n + 1 > g_tostr_cap) {'
assert s.count(a) == 1, 'B anchor'
s = s.replace(a, '    *out_len = 8; return "<object>";   /* M185-NC-B */\n    int n = 0;\n    char* s = px_fmt_value_n(v, &n);\n    if (n + 1 > g_tostr_cap) {')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_C() {   # 判定器恒真
        python3 - <<'PY'
p = 'runtime/runtime.c'
s = open(p, encoding='utf-8').read()
a = 'static LXValue bi_is_int_str(LXValue* args, int nargs, void* ctx) {\n    (void)ctx;\n    if (nargs != 1) px_error("R1002: is_int_str 需要一个参数");'
assert s.count(a) == 1, 'C anchor'
s = s.replace(a, a + '\n    return px_bool(1);   /* M185-NC-C */')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_D() {   # 解释轨删掉 bytes 迭代分支
        python3 - <<'PY'
p = 'selfhost/ival.px'
s = open(p, encoding='utf-8').read()
a = '''    if t == "bytes":
        # M185（第 63 轮 · 缺陷 203）：bytes **迭代** ⇒ 逐字节 int（0..255），'''
assert s.count(a) == 1, 'D anchor'
s = s.replace(a, '''    if t == "bytes_m185nc_dummy":
        # M185-NC-D：这条分支永不命中（等于删掉 bytes 迭代支持）
        # M185（第 63 轮 · 缺陷 203）：bytes **迭代** ⇒ 逐字节 int（0..255），''')
open(p, 'w', encoding='utf-8').write(s)
PY
    }

    # ⚠️ 这三个判据函数**绝不能**先调 `restore_all` —— 那会把刚打好的桩还原掉，
    #    于是"负控生效"永远测不出来（M185 初版就是这个错：A/B/C 全报"没有牙"）。
    #    `neg()` 已经负责"还原 → 打桩 → 判据 → 还原"的完整循环。
    chk_face_red()   { run3 "$HERE/bytes_face.px" ncA; if tri_same ncA && tri_rc0 ncA; then return 1; else return 0; fi }
    chk_rend_red()   { run3 "$HERE/render_sem.px" ncB; if tri_same ncB && has ncB.interp.out '^A1 join= \[1, 2\],\[3, 4\]$'; then return 1; else return 0; fi }
    chk_pred_red()   { run3 "$HERE/pred_sem.px" ncC; if tri_same ncC && has ncC.interp.out '^I 7 12ab false$'; then return 1; else return 0; fi }
    chk_iter_red()   {   # 解释轨必须回到"不可迭代"（用 dev 解释器；安装件不受 selfhost 改动影响）
        ( cd "$ROOT" && ./tools/px build selfhost/interp.px ) > "$W/ncd_build.log" 2>&1
        [ -x "$ROOT/selfhost/build/interp" ] || return 1
        ( cd "$W/bface" && timeout 60 "$ROOT/selfhost/build/interp" bytes_face.px ) > "$W/ncd.out" 2>&1
        local rc=$?
        # 预期：dev 解释器在该用例上 rc≠0 且词条是"不可迭代"
        [ "$rc" != 0 ] && grep -q '不可迭代' "$W/ncd.out"
    }

    neg A chk_face_red patch_A "$RT"
    neg B chk_rend_red patch_B "$RT"
    neg C chk_pred_red patch_C "$RT"
    neg D chk_iter_red patch_D "$IV"
    restore_all
    [ "$(sha_rt)" = "$SHA0_RT" ] || { echo "  FAIL runtime.c 未逐字节还原"; fail=$((fail+1)); }
    [ "$(sha_iv)" = "$SHA0_IV" ] || { echo "  FAIL ival.px 未逐字节还原"; fail=$((fail+1)); }
else
    echo "=== [5] 负控已跳过（--neg-skip）"
fi

rm -rf "$HERE/build"
echo
echo "M185 门结果：PASS=$pass FAIL=$fail"
[ "$fail" = 0 ] && echo "M185-VERIFY-OK" || echo "M185-VERIFY-FAILED"
exit $([ "$fail" = 0 ] && echo 0 || echo 1)
