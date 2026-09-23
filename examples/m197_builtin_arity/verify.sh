#!/usr/bin/env bash
# ============================================================
# M197 门（第 75 轮）：**全内置 0 参探针** ⇄ 原生 arity 文案（缺陷 232 + 233）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §6.7（[S7] 的**动态补面**）。
# 为什么需要（M196 的 [S7] 的**盲区**）：
#   [S7] 是**静态**对拍「解释轨的字面量」⇄「原生的字面量/模板」；但解释轨有一大类文案是
#   **用变量拼出来的**（`cname + " 需要 1 个参数"`、`"方法 " + name + " 需要 1 个参数"` …）——
#   字面量的 head 是**变量**，静态器只看到 `" 需要 1 个参数"` 这种**无 head** 的碎片 ⇒ **看不见**。
# 本轮实测（193 个内置全跑 0 参）抓到两组：
#   · **缺陷 233**（arity 文案 · 5 条）：`sqrt 需要 1 个参数`（原生 `sqrt 需要一个参数`）、
#     `sin`/`cos`/`tan` 缺「（弧度）」、`log` 缺「（自然对数 ln）」；
#   · **缺陷 232**（守卫顺序 · 3 处）：`os_spawn` / `os_kill` / `os_exec` 修前**没有 arity 守卫**、
#     直接索引 `args[0]`/`args[1]` ⇒ 0 参撞 `R1003 索引越界`（解释轨自己的越界错），
#     而原生给 `R1002: … 需要 (…参数)` ⇒ **同操作两码两文**（与 M196 的 `gen_next` 同族）。
# 判据：
#   [1] 静态 [S7]（M196 建立）仍绿 —— 可比 180 · 不一致 0
#   [2] **动态 [S8]**：**全内置 0 参探针**（名册 = `selfhost/interp.px` 的 names 列表，
#       193 个）—— ① 必须得 R1002；② 文案必须**逐字命中**原生侧该函数的 arity 文案。
#       跳过表 24 条（0 参合法 / 有副作用），**每条给理由**。
#   [3] 动态 · 8 个错例 × 三轨（解释 / VM / C）：rc≠0 + 同码 + 同文
#   [4] 负控 4 道（各自独立判红、源逐字节还原）：
#       A 静态（改**原生** sqrt 的 arity 文案 ⇒ [S8] 判红；[S8] 从**源码**读原生文案 ⇒ 不用重烘）
#       B 静态（同理改 sin）　C 静态（把 `flush` 从 SKIP 移出 ⇒ `flush()` 合法 ⇒ 判红）
#       D 动态（改原生 os_exec 的文案 ⇒ C 轨文案变、同文判据失效；只判 C 轨）
# 用法：bash examples/m197_builtin_arity/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m197_gate
BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime.c selfhost/ibuiltin.px examples/m197_builtin_arity/sweep_arity.py)
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
trap 'restore_all' EXIT

# ── 前置不变量：本轮改动必须在位 ──
grep -qF 'def i_math_arity_msg(cname):' selfhost/ibuiltin.px \
    || { echo "❌ 前置自查失败：i_math_arity_msg 不在（缺陷 233 未修）" >&2; exit 2; }
for pat in 'cname + " 需要 1 个参数（弧度）"' '"log 需要 1 个参数（自然对数 ln）"' \
           '"sqrt 需要一个参数"' \
           'i_r1002("os_spawn 需要 (cmd, args[, group|opts]) 参数", pos)' \
           'i_r1002("os_kill 需要 (pid, sig[, group]) 参数", pos)' \
           'i_r1002("os_exec 需要 (cmd, args?) 参数，cmd 为字符串", pos)'; do
    grep -qF "$pat" selfhost/ibuiltin.px || { echo "❌ 前置自查失败：ibuiltin.px 缺「$pat」" >&2; exit 2; }
done

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

echo "=== [1] 静态 [S7]（M196 建立 · 仍绿）==="
s7() { python3 "$ROOT/examples/m196_msg_parity/scan_msgs.py" --root "$ROOT" > "$W/s7.log" 2>&1; }
s7
chk "[1] [S7] 可比 ≥180 · 不一致 0（不写等式：数量随新增文案自然增长）" \
    "grep -q '当前不一致 0' '$W/s7.log' && grep -oE '可比函数 [0-9]+' '$W/s7.log' | awk '{exit !(\$2 >= 180)}'"

echo "=== [2] 动态 [S8]：全内置 0 参探针 ⇄ 原生 arity 文案 ==="
s8() { python3 "$ROOT/examples/m197_builtin_arity/sweep_arity.py" --root "$ROOT" > "$W/s8.log" 2>&1; }
s8; rc=$?
chk "[2] [S8] 不一致 0（193 个内置）" "[ \$rc = 0 ] && grep -q '共 193 个' '$W/s8.log' && grep -q '不一致 \*\*0\*\*' '$W/s8.log'"
chk "[2] [S8] 真抽查过（抽查数 > 150）" "grep -oE '抽查 [0-9]+' '$W/s8.log' | head -1 | awk '{exit !(\$2 > 150)}'"
chk "[2] SKIP 表每条有理由" "grep -q 'SKIP = {' '$ROOT/examples/m197_builtin_arity/sweep_arity.py'"

echo "=== [3] 动态：8 个错例 × 三轨（同码 + 同文）==="
while IFS=$'\t' read -r b ec eb; do
    [ -n "$b" ] || continue
    run3 "$b"
    chk "[3] $b：三轨 rc≠0 + $ec + 同文「$eb」" "judge_case '$b' '$ec' '$eb'"
done < "$HERE/probe/EXPECT.tsv"

echo "=== [3b] 合法侧 ==="
while IFS=$'\t' read -r b want; do
    [ -n "$b" ] || continue
    run3 "$b"
    for t in interp vm c; do
        chk "[3b] $b ($t)：rc=0 且输出含「$want」" \
            "[ \"\$(cat '$W/$b.$t.rc')\" = 0 ] && grep -q '$want' '$W/$b.$t.out'"
    done
done < "$HERE/probe/EXPECT_OK.tsv"

if [ "$NEG" = 1 ]; then
    echo "=== [4] 负控（各自独立判红 · 源逐字节还原）==="

    # A 静态：改**原生** sqrt 的 arity 文案（[S8] 从源码读原生文案 ⇒ 不必重烘）
    restore_all; snapshot
    sed -i 's|px_error("R1002: sqrt 需要一个参数")|px_error("R1002: sqrt 需要一个参数（负控）")|' runtime/runtime.c
    s8
    chk "[4A] 改原生 sqrt 文案 ⇒ [S8] 判红" "[ \$? != 0 ] && grep -q 'sqrt' '$W/s8.log'"

    # B 静态：同理改 sin
    restore_all; snapshot
    sed -i 's|px_error("R1002: sin 需要 1 个参数（弧度）")|px_error("R1002: sin 需要 1 个参数")|' runtime/runtime.c
    s8
    chk "[4B] 改原生 sin 文案 ⇒ [S8] 判红" "[ \$? != 0 ] && grep -q 'sin' '$W/s8.log'"

    # C 静态：把 flush 从 SKIP 移出 ⇒ flush() 合法（rc=0）⇒ 判红
    restore_all; snapshot
    sed -i 's|"flush": "0 参合法", ||' examples/m197_builtin_arity/sweep_arity.py
    s8
    chk "[4C] 把 flush 移出 SKIP ⇒ [S8] 判红（0 参合法却被要求 R1002）" "[ \$? != 0 ] && grep -q 'flush' '$W/s8.log'"

    # D 动态：改原生 os_exec 的文案 ⇒ C 轨文案变、同文判据失效（只判 C 轨）
    restore_all; snapshot
    sed -i 's|px_error("R1002: os_exec 需要 (cmd, args?) 参数，cmd 为字符串")|px_error("R1002: os_exec 需要 (cmd, args?) 参数，cmd 为字符串（负控）")|' runtime/runtime.c
    run1c p3_os_exec_arity
    chk "[4D] 改原生 os_exec 文案 ⇒ p3 的 C 轨文案不符（判据失效）" \
        "[ \"\$(norm '$W/nc_p3_os_exec_arity.c.out')\" != 'R1002|os_exec 需要 (cmd, args?) 参数，cmd 为字符串' ]"

    restore_all
fi

echo ""
echo "── 汇总：通过 $pass · 失败 $fail ──"
if [ "$fail" = 0 ]; then echo "M197-VERIFY-OK"; exit 0; else echo "M197-VERIFY-FAIL"; exit 1; fi
