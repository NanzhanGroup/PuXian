#!/usr/bin/env bash
# ============================================================
# M186 门（第 64 轮）：解释轨 native「透传完整性」收口（第三方 PX-DEF-018/019/020）
# ------------------------------------------------------------
# 缺陷形状：解释器转发 C 层 native 时**只透传前 1~3 个实参**，而 C 层签名更长：
#   · `int_to_bytes(n, size[, endian[, signed]])`（2-4 参）
#   · `bytes_to_int(bytes[, endian[, signed]])`（1-3 参）
# ⇒ 三种后果（同一"静默/晦涩"族，与 M116/M120/M163/M184 同纪律：响亮优于静默）：
#   ① **signed 静默丢弃**（PX-DEF-020）：`int_to_bytes(-5, 2, "big", true)`
#      解释轨 `302e30`（"0.0"）vs VM/C 轨 `fffb`；
#   ② **endian 静默丢弃**（PX-DEF-019）：`bytes_to_int(0102, "little")`
#      解释轨 `258`（大端）vs VM/C 轨 `513`（小端）；
#   ③ **实参不足时 args[1] 越界**（PX-DEF-018）：`int_to_bytes(1)`
#      解释轨 `R1003: 索引越界: 1 (len=1)` vs VM/C 轨 `R1002: int_to_bytes 需要 (n, size[, endian[, signed]]) 参数`；
#      且 `bytes_to_int(b,"big",false,1)` 的多余实参被**静默忽略**（编译轨 R1002 参数错）。
# 修法：解释轨两处一律**全量透传 + 参数个数前置校验**（错误构造走 `i_r1002` ⇒ 与 C 层同码同文）。
#
# 判据（两层正判据 + 三道负控，每层/每道可独立判红）：
#  ① `conv_parity.px`（33 行）三轨 rc=0 且 stdout **逐字节一致**，且关键定点钉死
#     （A2/A4=513 小端 · B1=-1 · B3=-128 · C1=fffb · C2=fbff · C5=ff · E1/E2=-1234 往返 · 越界=null）；
#  ② `neg/*.px`（4 例）三轨 rc≠0 + 词条**逐字相同**（位置前缀允许不同 —— 缺陷 186）。
#
# 负控（各自独立判红；解释轨改动 ⇒ 用 **dev 解释器**（`selfhost/build/interp`）跑，安装件不受影响）：
#  A `ibuiltin.px` 的 `bytes_to_int` 退回"只透传 args[0]" ⇒ ① 红（A2 变 258）
#  B `ibuiltin.px` 的 `int_to_bytes` 退回"最多 3 参"     ⇒ ① 红（C1 变非 fffb）
#  C `ibuiltin.px` 的 `int_to_bytes` 参数下限退回 `< 1`  ⇒ ② 红（i2b_missing 变 R1003）
#
# 用法：bash examples/m186_native_passthrough/verify.sh [--neg-skip]
# 退出码：0 = 绿，1 = 红，2 = 门自身前置自查失败。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0

IB="selfhost/ibuiltin.px"
BACK=/tmp/m186_gate_bak
W=/tmp/m186_gate
rm -rf "$W" "$BACK"; mkdir -p "$W" "$BACK"
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

snapshot() { cp -f "$IB" "$BACK/ibuiltin.px"; }
restore_all() { [ -f "$BACK/ibuiltin.px" ] && cp -f "$BACK/ibuiltin.px" "$IB"; }
sha_ib() { sha256sum "$IB" | cut -c1-16; }
snapshot
SHA0_IB="$(sha_ib)"
trap 'restore_all' EXIT

# ── 前置不变量自查（防上一轮被强杀留下的脏源码 / 防本门锚点失效）──
grep -q 'M186 修 PX-DEF-018/020' "$IB" || { echo "❌ 前置自查失败：ibuiltin.px 缺 int_to_bytes 全量透传修复（M186）" >&2; exit 2; }
grep -q 'M186 修 PX-DEF-019' "$IB" || { echo "❌ 前置自查失败：ibuiltin.px 缺 bytes_to_int 全量透传修复（M186）" >&2; exit 2; }
grep -q 'int_to_bytes 需要 (n, size\[, endian\[, signed\]\]) 参数' runtime/runtime.c || { echo "❌ 前置自查失败：runtime.c 缺 int_to_bytes 参数错词条（C 层锚点）" >&2; exit 2; }

# ── 三轨 runner ──
run3() {   # run3 <源文件> <tag> [解释器覆盖]
    local src="$1" tag="$2" pxi="${3:-$ROOT/bootstrap/pxi}"
    local d="$W/b$tag"
    rm -rf "$d"; mkdir -p "$d"
    cp "$src" "$d/"
    local b; b="$(basename "$src" .px)"
    ( cd "$d" && timeout 60 "$pxi" "$b.px" ) > "$W/$tag.interp.out" 2>&1
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

echo "=== [1] 透传值语义（endian × signed）：三轨 rc=0 + stdout 逐字节一致 + 定点"
run3 "$HERE/conv_parity.px" conv
chk "[1] 三轨 rc=0" "tri_rc0 conv"
chk "[1] 三轨 stdout 逐字节一致" "tri_same conv"
chk "[1] 行数=33" "[ \"\$(wc -l < $W/conv.interp.out)\" = 33 ]"
chk "[1] 定点 A2=513（小端，修前解释轨 258）" "has conv.interp.out '^A2 = 513$'"
chk "[1] 定点 A4=513（le 别名）" "has conv.interp.out '^A4 = 513$'"
chk "[1] 定点 A1=258（大端默认）" "has conv.interp.out '^A1 = 258$'"
chk "[1] 定点 B1=-1（signed 负）" "has conv.interp.out '^B1 = -1$'"
chk "[1] 定点 B3=-128（0x80 符号位）" "has conv.interp.out '^B3 = -128$'"
chk "[1] 定点 B4=128（同字节 unsigned）" "has conv.interp.out '^B4 = 128$'"
chk "[1] 定点 B5=-2" "has conv.interp.out '^B5 = -2$'"
chk "[1] 定点 C1=fffb（-5 补码，修前解释轨 0.0）" "has conv.interp.out '^C1 = fffb$'"
chk "[1] 定点 C2=fbff（小端补码）" "has conv.interp.out '^C2 = fbff$'"
chk "[1] 定点 C3=0201（小端 258）" "has conv.interp.out '^C3 = 0201$'"
chk "[1] 定点 C5=ff（-1 单字节）" "has conv.interp.out '^C5 = ff$'"
chk "[1] 定点 C7=fffffffffffffffe（-2 八字节）" "has conv.interp.out '^C7 = fffffffffffffffe$'"
chk "[1] 定点 D1..D3=null（域外）" "[ \"\$(grep -cE '^D[123] = null$' $W/conv.interp.out)\" = 3 ]"
chk "[1] 定点 D4=null（长度>8）" "has conv.interp.out '^D4 = null$'"
chk "[1] 定点 E1/E2=-1234（往返）" "[ \"\$(grep -cE '^E[12] = -1234$' $W/conv.interp.out)\" = 2 ]"
chk "[1] 定点 E5=8000（-32768）" "has conv.interp.out '^E5 = 8000$'"
chk "[1] 相邻族护栏 F1..F6 齐" "[ \"\$(grep -cE '^F[1-6] = ' $W/conv.interp.out)\" = 6 ]"

echo "=== [2] 拒绝侧 4 例 × 三轨：rc≠0 + 词条逐字相同"
declare -A PAT=(
  [i2b_missing]='int_to_bytes 需要 \(n, size\[, endian\[, signed\]\]\) 参数'
  [i2b_extra]='int_to_bytes 需要 \(n, size\[, endian\[, signed\]\]\) 参数'
  [b2i_extra]='bytes_to_int 需要 \(bytes\[, endian\[, signed\]\]\) 参数'
  [b2i_type]='bytes_to_int 需要 bytes，实际是 string'
)
for c in i2b_missing i2b_extra b2i_extra b2i_type; do
    run3 "$HERE/neg/$c.px" "n_$c"
    ok=1
    for t in interp vm c; do
        [ "$(cat "$W/n_$c.$t.rc")" != 0 ] || ok=0
        grep -qE "${PAT[$c]}" "$W/n_$c.$t.out" || ok=0
    done
    grep -q '^before$' "$W/n_$c.interp.out" || ok=0
    e1="$(grep -oE "${PAT[$c]}" "$W/n_$c.interp.out" | head -1)"
    e2="$(grep -oE "${PAT[$c]}" "$W/n_$c.vm.out" | head -1)"
    e3="$(grep -oE "${PAT[$c]}" "$W/n_$c.c.out" | head -1)"
    [ -n "$e1" ] && [ "$e1" = "$e2" ] && [ "$e1" = "$e3" ] || ok=0
    chk "[2] $c：三轨 rc≠0 + 词条一致「$e1」" "[ $ok = 1 ]"
done
chk "[2] 拒绝侧不得出现 R1003（索引越界泄漏）" "! grep -q 'R1003' \"$W/n_i2b_missing.interp.out\""

echo
if [ "$NEG" = 1 ]; then
    echo "=== [3] 负控（各自独立；源逐字节还原；解释轨改动 ⇒ 用 dev 解释器）"
    devbuild() {   # 用当前 selfhost 源码构建 dev 解释器（与安装件隔离）
        rm -rf "$ROOT/selfhost/build"
        ( cd "$ROOT" && ./tools/px build selfhost/interp.px ) > "$W/devbuild.log" 2>&1
        [ -x "$ROOT/selfhost/build/interp" ]
    }
    neg() {   # neg <名> <判据函数> <打桩函数>
        local name="$1" fn="$2" patcher="$3"
        restore_all
        $patcher || { echo "  FAIL 负控 $name：打桩失败"; fail=$((fail+1)); return; }
        if [ "$(sha_ib)" = "$SHA0_IB" ]; then
            echo "  FAIL 负控 $name：打桩未生效（锚点未命中）"; fail=$((fail+1)); restore_all; return
        fi
        if ! devbuild; then
            echo "  FAIL 负控 $name：dev 解释器构建失败"; fail=$((fail+1)); restore_all; return
        fi
        # 约定：判据函数返回 **0 = 负控生效（判据已判红）**，非零 = 负控没有牙。
        if $fn; then
            echo "  PASS 负控 $name：判据已判红（符合预期）"; pass=$((pass+1))
        else
            echo "  FAIL 负控 $name：判据仍为绿（负控没有牙）"; fail=$((fail+1))
        fi
        restore_all
    }

    patch_A() {   # bytes_to_int 退回"只透传 args[0]"（= 缺陷 019 原文）
        python3 - <<'PY'
p = 'selfhost/ibuiltin.px'
s = open(p, encoding='utf-8').read()
a = '''        if len(args) == 1:
            return Ok(bytes_to_int(args[0]))
        if len(args) == 2:
            return Ok(bytes_to_int(args[0], args[1]))
        return Ok(bytes_to_int(args[0], args[1], args[2]))'''
assert s.count(a) == 1, 'A anchor'
s = s.replace(a, '        # _m186_negA\n        return Ok(bytes_to_int(args[0]))')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_B() {   # int_to_bytes 退回"最多 3 参"（= 缺陷 020 原文）
        python3 - <<'PY'
p = 'selfhost/ibuiltin.px'
s = open(p, encoding='utf-8').read()
a = '''        if len(args) == 2:
            return Ok(int_to_bytes(args[0], args[1]))
        if len(args) == 3:
            return Ok(int_to_bytes(args[0], args[1], args[2]))
        return Ok(int_to_bytes(args[0], args[1], args[2], args[3]))'''
assert s.count(a) == 1, 'B anchor'
s = s.replace(a, '''        # _m186_negB
        if len(args) >= 3:
            return Ok(int_to_bytes(args[0], args[1], args[2]))
        return Ok(int_to_bytes(args[0], args[1]))''')
open(p, 'w', encoding='utf-8').write(s)
PY
    }
    patch_C() {   # 参数下限退回 < 1（= 缺陷 018 原文）
        python3 - <<'PY'
p = 'selfhost/ibuiltin.px'
s = open(p, encoding='utf-8').read()
a = '        if len(args) < 2 or len(args) > 4:\n            return Err(i_r1002("int_to_bytes 需要 (n, size[, endian[, signed]]) 参数", pos))'
assert s.count(a) == 1, 'C anchor'
s = s.replace(a, '        # _m186_negC\n        if len(args) < 1 or len(args) > 4:\n            return Err(i_r1002("int_to_bytes 需要 (n, size[, endian[, signed]]) 参数", pos))')
open(p, 'w', encoding='utf-8').write(s)
PY
    }

    # 判据函数**不得**先调 restore_all（那会把刚打好的桩还原掉）；dev 解释器路径由 devbuild 产出。
    DEV="$ROOT/selfhost/build/interp"
    chk_A_red() { run3 "$HERE/conv_parity.px" ncA "$DEV"; ! ( tri_same ncA && tri_rc0 ncA && has ncA.interp.out '^A2 = 513$' ); }
    chk_B_red() { run3 "$HERE/conv_parity.px" ncB "$DEV"; ! ( tri_same ncB && tri_rc0 ncB && has ncB.interp.out '^C1 = fffb$' ); }
    chk_C_red() {   # 打桩后：解释轨不再给出**与编译轨一致**的参数错 ⇒ "同码同文"判据判红
        run3 "$HERE/neg/i2b_missing.px" ncC "$DEV"
        [ "$(cat "$W/ncC.interp.rc")" != 0 ] || return 1
        grep -qE 'int_to_bytes 需要 \(n, size\[, endian\[, signed\]\]\) 参数' "$W/ncC.vm.out" || return 1
        ! grep -qE 'int_to_bytes 需要 \(n, size\[, endian\[, signed\]\]\) 参数' "$W/ncC.interp.out"
    }

    neg A chk_A_red patch_A
    neg B chk_B_red patch_B
    neg C chk_C_red patch_C
    restore_all
    [ "$(sha_ib)" = "$SHA0_IB" ] || { echo "  FAIL ibuiltin.px 未逐字节还原"; fail=$((fail+1)); }
else
    echo "=== [3] 负控已跳过（--neg-skip）"
fi

rm -rf "$ROOT/selfhost/build"
echo
echo "M186 门结果：PASS=$pass FAIL=$fail"
[ "$fail" = 0 ] && echo "M186-VERIFY-OK" || echo "M186-VERIFY-FAILED"
exit $([ "$fail" = 0 ] && echo 0 || echo 1)
