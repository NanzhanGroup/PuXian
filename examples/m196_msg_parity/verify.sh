#!/usr/bin/env bash
# ============================================================
# M196 门（第 74 轮）：诊断**文案**的三轨单一真相（缺陷 227 + 230）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §6.7。
# 主问题：**解释轨的守卫先于原生触发** —— 于是「解释轨自己写的那条文案」会盖住原生那条，
#   而编译轨拿到的是原生那条 ⇒ **同一坏输入三轨两文**。这类分歧**不报错、不崩**，
#   只在错误路径上显形 ⇒ 此前没有任何门看得见（M194/M195 只统一了自己动过的几处）。
# 本轮：
#   · 静态普查出 **180 个「两边都有文案」的函数**，逐条对拍；
#   · 批次 1（49 条）：解释轨**缺「参数」尾词** / 数字写法（`两个参数`→`2 个参数`）/
#     词面（`不需要参数`→`不接受参数`）/ 函数面（`(列表, 函数)`→`(list, fn)`）等 ⇒ 对齐；
#   · 批次 2（5 条）：类型文案 `不支持此类型` → `不支持类型 <t>`（带实际类型）、`trim` 的类型文案；
#   · 顺带修 **缺陷 230**：`gen_next` 的**守卫顺序** —— `let g = args[0]` 在 arity 检查之前
#     ⇒ `gen_next()` 撞 `R1003 索引越界: 0 (len=0)`，而原生给 `R1002: gen_next 需要生成器对象`。
# 判据：
#   [1] 静态 [S7]：`scan_msgs.py` —— 可比函数 180 · **当前不一致 0** · 基线**已清空**
#       （⇒ 硬判据：此后任何新增的「解释轨独有文案」立即判红）；
#       FMT_OK 2 条（range / atan2）**必须给理由**（原生侧由公共格式串模板产出，静态器看不见）。
#   [2] 动态：12 个错例 × 三轨（解释 / VM / C）—— rc≠0 + **同码** + **同文**
#   [2b] 合法侧：4 例 rc=0（收口不得改坏正常路径）
#   [3] 负控 4 道（各自独立判红、源逐字节还原）：
#       A 静态 · 把一条已对齐的解释轨文案**改回去**（chr）　B 静态 · 删掉 FMT_OK 的 range 条目
#       C 静态 · 改**原生侧**文案（两侧任一改动都要能被发现）
#       D 动态 · 改原生的 `gen_next` 文案（⇒ C 轨文案变、同文判据失效；只判 C 轨）
# 用法：bash examples/m196_msg_parity/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m196_gate
BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime.c runtime/runtime.h runtime/runtime_ws.c runtime/runtime_h3.c
       runtime/runtime_quic.c runtime/runtime_onnx.c selfhost/ibuiltin.px
       examples/m196_msg_parity/scan_msgs.py)
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
trap 'restore_all' EXIT

# ── 前置不变量：本轮改动必须在位 ──
for pat in 'i_r1002("chmod 需要 (path, mode) 参数", pos)' \
           'i_r1002("chr 需要 (码点) 参数", pos)' \
           'i_r1002("contains 需要 2 个参数", pos)' \
           'i_r1002("flush 不接受参数", pos)' \
           'i_r1002("trim 需要一个字符串参数", pos)' \
           'i_r1002("filter 需要 (list, fn)", pos)' \
           'i_r1002("md5 需要一个参数 (data)", pos)' \
           'i_r1002("os_capture 需要 (cmd, args[, opts]) 参数", pos)' \
           'i_r1002("http_unix 需要 (socket_path, url_path, method[, body[, headers[, opts]]]]) 参数", pos)'; do
    grep -qF "$pat" selfhost/ibuiltin.px || { echo "❌ 前置自查失败：ibuiltin.px 缺「$pat」" >&2; exit 2; }
done
grep -qF 'i_r1002("abs 不支持类型 " + i_type_name(args[0]), pos)' selfhost/ibuiltin.px \
    || { echo "❌ 前置自查失败：abs 的类型文案未对齐" >&2; exit 2; }
grep -qF 'i_r1002("int 不支持类型 " + i_type_name(args[0]), pos)' selfhost/ibuiltin.px \
    || { echo "❌ 前置自查失败：int 的类型文案未对齐" >&2; exit 2; }
grep -qF 'i_r1002("gen_next 需要生成器对象", pos)
        let g = args[0]' selfhost/ibuiltin.px \
    || { echo "❌ 前置自查失败：gen_next 的守卫顺序未修（缺陷 230）" >&2; exit 2; }

norm() {
    grep -E '^运行时错误' "$1" 2>/dev/null | head -1 \
      | sed -E -e 's/^运行时错误: 错误 \[(R[0-9]{4})\] [0-9]+:[0-9]+: /\1|/' \
               -e 's/^运行时错误 \[[^]]*行[0-9]+\]: (R[0-9]{4}): /\1|/'
}
run3() {
    local b="$1" d="$W/d_$b"
    rm -rf "$d"; mkdir -p "$d"
    cp "$HERE/probe/$b.px" "$d/"
    ( cd "$d" && timeout 60 "$ROOT/bootstrap/pxi" "$b.px" ) > "$W/$b.interp.out" 2>&1; echo $? > "$W/$b.interp.rc"
    rm -rf "$d/build"
    timeout 200 ./tools/px build "$d/$b.px" > "$W/$b.vm.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/$b.vm.out" 2>&1; echo $? > "$W/$b.vm.rc"
    else echo 999 > "$W/$b.vm.rc"; tail -3 "$W/$b.vm.build.log" > "$W/$b.vm.out"; fi
    rm -rf "$d/build"
    PX_BUILD_ENGINE=c timeout 260 ./tools/px build "$d/$b.px" > "$W/$b.c.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/$b.c.out" 2>&1; echo $? > "$W/$b.c.rc"
    else echo 999 > "$W/$b.c.rc"; tail -3 "$W/$b.c.build.log" > "$W/$b.c.out"; fi
    rm -rf "$d/build"
}
run1c() {
    local b="$1" d="$W/nc_$b"
    rm -rf "$d"; mkdir -p "$d"; cp "$HERE/probe/$b.px" "$d/"
    PX_BUILD_ENGINE=c timeout 260 ./tools/px build "$d/$b.px" > "$W/nc_$b.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 60 "$d/build/$b" ) > "$W/nc_$b.c.out" 2>&1; echo $? > "$W/nc_$b.c.rc"
    else echo 999 > "$W/nc_$b.c.rc"; tail -3 "$W/nc_$b.build.log" > "$W/nc_$b.c.out"; fi
    rm -rf "$d/build"
}
judge_case() {
    local b="$1" ec="$2" eb="$3" t ok=1
    for t in interp vm c; do
        [ "$(cat "$W/$b.$t.rc")" != 0 ] || ok=0
        [ "$(norm "$W/$b.$t.out")" = "$ec|$eb" ] || ok=0
        grep -q '^before$' "$W/$b.$t.out" || ok=0
    done
    [ $ok = 1 ]
}

echo "=== [1] 静态判据：文案对拍 [S7]（§6.7）==="
s7() { python3 "$ROOT/examples/m196_msg_parity/scan_msgs.py" --root "$ROOT" > "$W/s7.log" 2>&1; }
s7; rc=$?
chk "[1] [S7] 全绿（可比 180 · 不一致 0）" "[ \$rc = 0 ] && grep -q '可比函数 180 · 当前不一致 0' '$W/s7.log'"
chk "[1] 基线已清空 ⇒ 硬判据（任何新增立即判红）" "grep -q '基线函数 0 · 新增 0/0' '$W/s7.log'"
chk "[1] FMT_OK 两条带理由（range / atan2）且真跑过" "grep -q 'FMT_OK' '$ROOT/examples/m196_msg_parity/scan_msgs.py' && grep -c 'why' '$ROOT/examples/m196_msg_parity/scan_msgs.py' | grep -q '^2$'"

echo "=== [2] 动态判据：12 个错例 × 三轨（同码 + 同文）==="
while IFS=$'\t' read -r b ec eb; do
    [ -n "$b" ] || continue
    run3 "$b"
    chk "[2] $b：三轨 rc≠0 + $ec + 同文「$eb」" "judge_case '$b' '$ec' '$eb'"
done < "$HERE/probe/EXPECT.tsv"

echo "=== [2b] 合法侧 ==="
while IFS=$'\t' read -r b want; do
    [ -n "$b" ] || continue
    run3 "$b"
    for t in interp vm c; do
        chk "[2b] $b ($t)：rc=0 且输出含「$want」" \
            "[ \"\$(cat '$W/$b.$t.rc')\" = 0 ] && grep -q '$want' '$W/$b.$t.out'"
    done
done < "$HERE/probe/EXPECT_OK.tsv"

if [ "$NEG" = 1 ]; then
    echo "=== [3] 负控（各自独立判红 · 源逐字节还原）==="

    # A 静态：把一条已对齐的**解释轨**文案改回去
    restore_all; snapshot
    sed -i 's|i_r1002("chr 需要 (码点) 参数", pos)|i_r1002("chr 需要 (码点)", pos)|' selfhost/ibuiltin.px
    s7
    chk "[3A] 解释轨文案改回旧形态（chr）⇒ [S7] 判红" "[ \$? != 0 ] && grep -q 'chr' '$W/s7.log'"

    # B 静态：删掉 FMT_OK 的 range 条目（证明 FMT_OK 有牙：不是"白名单兜底"）
    restore_all; snapshot
    python3 - "$ROOT" <<'PYEOF'
import sys
p = sys.argv[1] + "/examples/m196_msg_parity/scan_msgs.py"
s = open(p, encoding="utf-8").read()
i = s.index('    "range": {')
j = s.index('    "atan2": {')
open(p, "w", encoding="utf-8").write(s[:i] + s[j:])
PYEOF
    s7
    chk "[3B] 删掉 FMT_OK 的 range 条目 ⇒ [S7] 判红" "[ \$? != 0 ] && grep -q 'range' '$W/s7.log'"

    # C 静态：改**原生侧**文案（两侧任一改动都必须被发现）
    restore_all; snapshot
    sed -i 's|px_error("R1002: chmod 需要 (path, mode) 参数")|px_error("R1002: chmod 需要 (path, mode) 参数x")|' runtime/runtime.c
    s7
    chk "[3C] 改原生侧文案 ⇒ [S7] 判红" "[ \$? != 0 ] && grep -q 'chmod' '$W/s7.log'"

    # D 动态：改原生 `gen_next` 文案 ⇒ C 轨文案变、同文判据失效（只判 C 轨）
    restore_all; snapshot
    sed -i 's|px_error("R1002: gen_next 需要生成器对象")|px_error("R1002: gen_next 需要生成器对象（负控改文）")|' runtime/runtime.c
    run1c m9_gen_next_type
    chk "[3D] 改原生 gen_next 的类型文案 ⇒ m9 的 C 轨文案不符（判据失效）" \
        "[ \"\$(norm '$W/nc_m9_gen_next_type.c.out')\" != 'R1002|gen_next 需要生成器对象' ]"

    restore_all
fi

echo ""
echo "── 汇总：通过 $pass · 失败 $fail ──"
if [ "$fail" = 0 ]; then echo "M196-VERIFY-OK"; exit 0; else echo "M196-VERIFY-FAIL"; exit 1; fi
