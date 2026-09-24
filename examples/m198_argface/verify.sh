#!/usr/bin/env bash
# ============================================================
# M198 门（第 76 轮）：native **元数上界** ⇄ **时长族数值化**（缺陷 234 + 缺陷 229）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §6.9（本轮建立）· 判据 [S9] / [S10]
#
# 为什么需要：
#   M190 量的是「三轨 **rc 分叉**」，M197 量的是「**0 参** ⇄ arity 文案」——
#   两者都看不见**三轨一致**的宽容：`quic_close(1, 2)` 三个轨都**静默忽略**多余实参。
#   本轮普查：**81 个注册 native 没有元数上界**（62 个「固定元数只查下界」+ 19 个 0/0-1 参）。
#   同族的第二条（缺陷 229）：**时长**形参走 `px_arg_int` / `if (type == PX_INT)` ——
#     · `set_timeout(fn, 1.5)` 直接报「ms 需要整数」（而 M195 已让 `sleep(0.1)` 生效，同一族自相矛盾）；
#     · `{timeout_ms: 300.0}` / `{"timeout_ms": "300"}` 被**静默忽略** ⇒ 用户以为设了超时，其实**没有**。
#
# 判据：
#   [1] 静态 [S9]：`runtime/*.c` 全量 —— **每个**注册 native 必须（a）显式声明上界
#       （`nargs != N` / `nargs > N`）或（b）在**真变长豁免表**内（每条给理由 + 表内名字必须真注册过）。
#       硬判据：未声明 = 0 · 表漂移 = 0 · 解析不到的注册 = 0。
#   [2] 动态 [S9]：14 个错例 × **三轨**（解释/VM/C）—— rc≠0 + 同码 + 同文 + 已进运行期（`before`）。
#   [3] 时长族（缺陷 229）：`set_timeout(fn, 1.5)` **真生效** · 整数毫秒精度不变 · 取消语义不变 ·
#       `{"timeout_ms": 300.0}` **被采纳**（修前静默忽略 ⇒ 等于没有超时）· `fd_wait(0, 1.5)` 接受小数。
#   [4] 负控 4 道（各自独立判红、源逐字节还原）：
#       A 静态：把 `quic_close` 的上界退回 `nargs < 1` ⇒ [S9] 判红
#       B 静态：把 `set_timeout` 从豁免表删掉 ⇒ 表与代码漂移判红（豁免机制本身的自证）
#       C 动态（C 轨）：删掉 `now_ms` 的 0 参守卫 ⇒ p8 不再 R1002 ⇒ 判红
#       D 动态（C 轨）：把 `set_timeout` 的时长退回 `px_arg_int` ⇒ t1 编译运行报错 ⇒ 判红
# 用法：bash examples/m198_argface/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m198_gate
BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime.c runtime/runtime_quic.c runtime/runtime_h3.c runtime/vm.c
       examples/m198_argface/sweep_upper.py)
pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
trap 'restore_all' EXIT

# ── 前置不变量：本轮改动必须在位 ──
for pat in 'int64_t px_arg_dur_ms(LXValue v, const char* fn, const char* pname);' \
           'int64_t px_opt_dur_ms(LXValue opts, const char* key, const char* fn, int* has);'; do
    grep -qF "$pat" runtime/runtime.h || { echo "❌ 前置自查失败：runtime.h 缺「$pat」" >&2; exit 2; }
done
grep -qF 'if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_close 需要 (conn: int)");' runtime/runtime_quic.c \
    || { echo "❌ 前置自查失败：quic_close 的上界不在位（缺陷 234 未修）" >&2; exit 2; }
grep -qF 'if (nargs != 0) px_error("R1002: now_ms 不需要参数");' runtime/runtime.c \
    || { echo "❌ 前置自查失败：now_ms 的 0 参守卫不在位" >&2; exit 2; }
grep -qF 'long long ns = px_arg_dur_ns(args[1], "set_timeout", "ms", 1000000.0);' runtime/runtime.c \
    || { echo "❌ 前置自查失败：set_timeout 的时长数值化不在位（缺陷 229 未修）" >&2; exit 2; }

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
    else
        echo 999 > "$W/$b.c.rc"; tail -3 "$W/$b.c.build.log" > "$W/$b.c.out"
        # M201：**构建失败**（rc=999）必须在门输出里看得见 —— 否则判据只报"行为不符"，
        #   让人去查行为面（本轮全量门里 t3 就是这么假红过一次：真因是 C 轨构建没产出）。
        echo "   ⚠️ [$b] C 轨构建失败（rc=999）—— 构建日志尾："; tail -3 "$W/$b.c.build.log" | sed 's/^/      | /'
    fi
    rm -rf "$d/build"
}
run1c() {
    local b="$1" d="$W/nc_$b"
    rm -rf "$d"; mkdir -p "$d"; cp "$HERE/probe/$b.px" "$d/"
    PX_BUILD_ENGINE=c timeout 260 ./tools/px build "$d/$b.px" > "$W/nc_$b.build.log" 2>&1
    if [ -x "$d/build/$b" ]; then ( cd "$d" && timeout 120 "$d/build/$b" ) > "$W/nc_$b.c.out" 2>&1; echo $? > "$W/nc_$b.c.rc"
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
s9static() { python3 "$ROOT/examples/m198_argface/sweep_upper.py" --root "$ROOT" > "$W/s9.log" 2>&1; }

echo "=== [1] 静态 [S9]：每个 native 必须声明元数上界或在豁免表 ==="
s9static; rc=$?
chk "[1] [S9] 未声明 0 · 表漂移 0 · 解析不到 0（核对豁免表每条理由）" \
    "[ \$rc = 0 ] && grep -qE '未声明（必须为 0）.*: 0' '$W/s9.log' && grep -q '结论：不一致 0' '$W/s9.log'"
chk "[1] 显式声明上界 ≥ 300（扫描器有效性下限，不写等式 —— 新增 native 自然增长）" \
    "grep -oE '显式声明上界（!= N / > N）  : [0-9]+' '$W/s9.log' | awk '{exit !(\$NF >= 300)}'"

echo "=== [2] 动态 [S9]：14 个错例 × 三轨（同码 + 同文）==="
while IFS=$'\t' read -r b ec eb; do
    [ -n "$b" ] || continue
    run3 "$b"
    chk "[2] $b：三轨 rc≠0 + $ec + 同文「$eb」" "judge_case '$b' '$ec' '$eb'"
done < "$HERE/probe/EXPECT.tsv"

echo "=== [3] 时长族（缺陷 229）==="
for b in t1_timer_float_fires t2_timer_precision t3_opts_float_timeout t4_fdwait_float t5_timer_int_still_ok; do
    run3 "$b"
done
for t in interp vm c; do
    chk "[3] t1 小数定时器**真生效**（$t）：fired=1" \
        "[ \"\$(cat '$W/t1_timer_float_fires.$t.rc')\" = 0 ] && grep -q 'fired=1' '$W/t1_timer_float_fires.$t.out'"
    chk "[3] t2 整数毫秒精度保持（$t）：delta ∈ [20,120]" \
        "[ \"\$(cat '$W/t2_timer_precision.$t.rc')\" = 0 ] && awk -F= '/^delta=/{exit !(\$2>=20 && \$2<=120)}' '$W/t2_timer_precision.$t.out'"
    chk "[3] t3 opts 的 **float** 超时被采纳（$t）：fast=true（判据自证：打印 elapsed）" \
        "[ \"\$(cat '$W/t3_opts_float_timeout.$t.rc')\" = 0 ] && grep -q 'fast=true' '$W/t3_opts_float_timeout.$t.out'"
    chk "[3] t4 fd_wait 接受小数（$t）：ok=list" \
        "[ \"\$(cat '$W/t4_fdwait_float.$t.rc')\" = 0 ] && grep -q 'ok=list' '$W/t4_fdwait_float.$t.out'"
    chk "[3] t5 取消语义不变（$t）：cancelled=true" \
        "[ \"\$(cat '$W/t5_timer_int_still_ok.$t.rc')\" = 0 ] && grep -q 'cancelled=true' '$W/t5_timer_int_still_ok.$t.out'"
done

if [ "$NEG" = 1 ]; then
    echo "=== [4] 负控（各自独立判红 · 源逐字节还原）==="

    # A 静态：quic_close 的上界退回 nargs < 1 ⇒ [S9] 判红
    restore_all; snapshot
    sed -i 's|if (nargs != 1 \|\| args\[0\].type != PX_INT) px_error("R1002: quic_close 需要 (conn: int)");|if (nargs < 1 \|\| args[0].type != PX_INT) px_error("R1002: quic_close 需要 (conn: int)");|' runtime/runtime_quic.c
    s9static; rc=$?
    chk "[4A] quic_close 退回下界 ⇒ [S9] 判红" "[ \$rc != 0 ] && grep -q 'quic_close' '$W/s9.log'"

    # B 静态：把 set_timeout 从豁免表删掉 ⇒ 表与代码漂移判红
    restore_all; snapshot
    sed -i '/"set_timeout": "≥2 参/d' examples/m198_argface/sweep_upper.py
    s9static; rc=$?
    chk "[4B] 豁免表删 set_timeout ⇒ 判红（未声明）" "[ \$rc != 0 ] && grep -q 'set_timeout' '$W/s9.log'"

    # C 动态：删 now_ms 的 0 参守卫 ⇒ p8 的 C 轨不再 R1002
    restore_all; snapshot
    sed -i 's|    if (nargs != 0) px_error("R1002: now_ms 不需要参数");||' runtime/runtime.c
    run1c p8_now_ms_extra
    chk "[4C] 删 now_ms 守卫 ⇒ p8 C 轨判据失效" \
        "[ \"\$(norm '$W/nc_p8_now_ms_extra.c.out')\" != 'R1002|now_ms 不需要参数' ]"

    # D 动态：set_timeout 退回 px_arg_int ⇒ t1 判红
    restore_all; snapshot
    python3 - <<'PY'
import re
p = "/data/code/puxian/runtime/runtime.c"
s = open(p, encoding="utf-8").read()
old = 'long long ns = px_arg_dur_ns(args[1], "set_timeout", "ms", 1000000.0);'
assert old in s
s = s.replace(old, 'long long ns = (long long)px_arg_int(args[1], "set_timeout", "ms") * 1000000LL;')
open(p, "w", encoding="utf-8").write(s)
PY
    run1c t1_timer_float_fires
    chk "[4D] set_timeout 退回只收整数 ⇒ t1 C 轨判据失效" \
        "! grep -q 'fired=1' '$W/nc_t1_timer_float_fires.c.out'"

    restore_all
fi

echo ""
echo "── 汇总：通过 $pass · 失败 $fail ──"
if [ "$fail" = 0 ]; then echo "M198-VERIFY-OK"; exit 0; else echo "M198-VERIFY-FAIL"; exit 1; fi
