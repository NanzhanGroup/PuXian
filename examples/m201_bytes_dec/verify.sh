#!/usr/bin/env bash
# ============================================================
# M201 门（第 80 轮）：**精确十进制读取** `bytes_to_dec` + `bytes_to_int` 的静默 null 收口
#                        （第三方 PX-DEF-031 + 同族的"静默 null"）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §6.12
#
# 为什么需要（PX-DEF-031）：
#   第三方写 mysql/pg 驱动时发现：`bytes_to_int(hex_to_bytes("ffffffffffffffff"), "little", false)`
#   返回 **-1**（应为 18446744073709551615）—— **三轨一致但静默给错值**，
#   而语言 `int` 是 64 位**有符号**，装不下 u64 ⇒ 只能靠手写「32 位高低字长除法」
#   （`mysql/stmt.px::my_u64_dec`）绕行。
#
# 本轮的判定（两层，各自有硬判据）：
#   ① `bytes_to_dec(b[, endian[, signed]])` —— 新增**一等公民**：整段字节十进制长除法
#      （不经 double，无精度损失），宽度 1..16（u64/u128），signed 时补码取负。
#      ⇒ 第三方手写的长除法可以退休（其登记里的"官方宜考虑 bytes_to_uint64 或以字符串返回"落地）。
#   ② `bytes_to_int` 的 **8 字节 unsigned > 2^63-1 保留回绕**（= Go `int64(uint64)` /
#      Java `getLong()` / C 强制转换的**同款语义**），**不报错** —— 因为它是"读原始位模式"
#      （float64 位型 / 消息字段）的唯一可用形态（实测：msgpack 的 float64 位型、mysql 的
#      DOUBLE 列都依赖它；改成报错会**打破两个官方库**）。⇒ 文档明示这一"双模型"，
#      精确值一律走 ② 之外的 `bytes_to_dec`。
#   ③ **同族收口（静默 null）**：`bytes_to_int` 长度越界（<1 或 >8）修前**静默返回 null**
#      （与 M199 修掉的 `bytes_get` 越界 null 同族：null 流到下游只报"无法比较 null 与 int"，
#      **指不到越界点**）⇒ 改 **`R1003`**，并在消息里指明更宽的整数用 `bytes_to_dec`。
#
# 判据：
#   [1] 静态：注册点存在 · 旧静默路径消失 · 解释轨（ibuiltin.px）逐参透传（1..3 参）·
#            口径文件含 `bytes_to_dec`。
#   [2] 动态三轨（解释 / VM / C）**逐字节一致**：
#       正例 23 项（含 2^64-1 / 2^63 / u128 / 补码负数 / 两序对照）**期望值由 bignum 独立算得**；
#       拒绝侧 4 例（越界 ×2 + 类型 + 个数）判 rc≠0 + **同码** + **同文案** + 前置标记存在。
#   [3] 负控（各自独立判红 · 源逐字节还原）：
#       A 删 `bytes_to_dec` 注册 ⇒ 三轨 R1001 ⇒ [2] 红
#       B 把 `bytes_to_int` 长度守卫改回 `return px_null()` ⇒ **仅** rej_i9 红（其它仍绿 = 独立牙）
#       C 把 `bytes_to_dec` 宽度上限 16 改 8 ⇒ **仅** d16/d17 红（正例面有牙）
# 用法：bash examples/m201_bytes_dec/verify.sh [--neg-skip] [--jobs N]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
for a in "$@"; do case "$a" in --neg-skip) NEG=0 ;; esac; done

W=/tmp/m201_dec_gate; BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime.c selfhost/ibuiltin.px)
pass=0; fail=0
chk() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
chkout() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; cat "$W/$3" 2>/dev/null | sed 's/^/      | /' | head -8; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
trap 'restore_all' EXIT

# ── 前置不变量（缺了说明门自己过期）──
pre() { grep -qF "$1" "$2" || { echo "❌ 前置自查失败：$2 缺「$1」" >&2; exit 2; }; }
pre 'px_set_global("bytes_to_dec", px_native("bytes_to_dec", bi_bytes_to_dec));' runtime/runtime.c
pre 'R1003: bytes_to_int 需要 1..8 字节' runtime/runtime.c
pre 'if name == "bytes_to_dec":' selfhost/ibuiltin.px

build3() {   # $1=probe 名 → $W/$1.{interp,vm,c}.{out,rc}
    # ⚠️ 必须分两行：`local n="$1" d="$W/b_$n"` 里 **d 取到的是外层 n**
    #   （bash 5.1 + set -u 实测：外层无 n ⇒ "n: unbound variable"；
    #    m200 门里靠 `for n in …` 留了个全局 n 才没炸 ⇒ 那只是巧合）
    local n="$1"
    local d="$W/b_$n"
    rm -rf "$d"; mkdir -p "$d"; cp -f "$HERE/probe/$n.px" "$d/"
    ( cd "$d" && timeout 60 "$ROOT/bootstrap/pxi" "$n.px" ) > "$W/$n.interp.out" 2>&1; echo $? > "$W/$n.interp.rc"
    ( cd "$d" && timeout 900 "$ROOT/tools/px" build "$n.px" ) > "$W/$n.vm.build" 2>&1
    if [ -x "$d/build/$n" ]; then ( cd "$d" && timeout 60 "$d/build/$n" ) > "$W/$n.vm.out" 2>&1; echo $? > "$W/$n.vm.rc"
    else echo 999 > "$W/$n.vm.rc"; tail -3 "$W/$n.vm.build" > "$W/$n.vm.out"; fi
    rm -rf "$d/build"
    ( cd "$d" && PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build "$n.px" ) > "$W/$n.c.build" 2>&1
    if [ -x "$d/build/$n" ]; then ( cd "$d" && timeout 60 "$d/build/$n" ) > "$W/$n.c.out" 2>&1; echo $? > "$W/$n.c.rc"
    else echo 999 > "$W/$n.c.rc"; tail -3 "$W/$n.c.build" > "$W/$n.c.out"; fi
    rm -rf "$d/build"
}
judge3() {   # $1=probe  $2=期望 stdout（多行 \n 连接）
    local n="$1" want="$2" t
    for t in interp vm c; do
        [ "$(cat "$W/$n.$t.rc" 2>/dev/null)" = 0 ] || return 1
        [ "$(cat "$W/$n.$t.out" 2>/dev/null | sed '/^$/d')" = "$want" ] || return 1
    done
    return 0
}
judge_rej() {   # $1=probe $2=期望 R 码 $3=期望消息子串 —— 三轨 rc≠0 + 码 + 文案 + 前置标记
    local n="$1" code="$2" msg="$3" t
    for t in interp vm c; do
        [ "$(cat "$W/$n.$t.rc" 2>/dev/null)" != 0 ] || return 1
        grep -q '^before$' "$W/$n.$t.out" 2>/dev/null || return 1
        grep -qF "$code" "$W/$n.$t.out" 2>/dev/null || return 1
        grep -qF "$msg" "$W/$n.$t.out" 2>/dev/null || return 1
    done
    return 0
}

echo "=== [1] 静态：注册点 / 旧静默路径消失 / 解释轨透传 / 口径文件 ==="
chk "[1] bytes_to_dec 已注册（native + 解释轨入口）" \
    "grep -q 'bi_bytes_to_dec' runtime/runtime.c && grep -q 'bytes_to_dec(args\\[0\\], args\\[1\\], args\\[2\\])' selfhost/ibuiltin.px"
chk "[1] 旧静默路径已消失（bytes_to_int 不再 return px_null()）" \
    "! grep -q 'if (len < 1 || len > 8) return px_null();' runtime/runtime.c"
chk "[1] 两个越界都走 R1003 且消息指向 bytes_to_dec" \
    "grep -qF '需要 1..8 字节，实际是 %d（更宽的整数用 bytes_to_dec）' runtime/runtime.c && grep -qF 'R1003: bytes_to_dec 需要 1..16 字节' runtime/runtime.c"
chk "[1] 口径文件 docs/ERROR_CODES.md 登记 bytes_to_dec（§6.12）" \
    "grep -q 'bytes_to_dec' docs/ERROR_CODES.md && grep -q '6.12' docs/ERROR_CODES.md"

echo "=== [2] 动态：正例 23 项 + 拒绝侧 4 例 × 三轨（解释 / VM / C）==="
build3 dec_pos
WANT='d01=255
d02=255
d03=258
d04=513
d05=0
d06=0
d07=81985529216486895
d08=17279655951921914625
d09=18446744073709551615
d10=9223372036854775808
d11=9223372036854775807
d12=-1
d13=-9223372036854775808
d14=-129
d15=-1
d16=340282366920938463463374607431768211455
d17=-1
d18=1
i01=-1
i02=-9223372036854775808
i03=513
x01=true
x02=true'
# 期望值来源：bignum 独立算得（2^64-1 / 2^63 / 2^63-1 / 2^128-1 / 补码 -(2^63) / -129…），
#   不是"跑一遍抄下来"。d07/d08 为字节序对照（0x0123456789abcdef 两序）。
chkout "[2] dec_pos：23 项三轨逐字节一致（期望值由 bignum 独立算得）" "judge3 dec_pos \"\$WANT\"" "dec_pos.interp.out"
for e in "rej_i9:R1003:bytes_to_int 需要 1..8 字节，实际是 9" \
         "rej_d17:R1003:bytes_to_dec 需要 1..16 字节，实际是 17" \
         "rej_dtype:R1002:bytes_to_dec 需要 bytes，实际是 string" \
         "rej_dnum:R1002:bytes_to_dec 需要 (bytes[, endian[, signed]]) 参数"; do
    n="${e%%:*}"; rest="${e#*:}"; code="${rest%%:*}"; msg="${rest#*:}"
    build3 "$n"
    chkout "[2] $n：三轨 rc≠0 · 同码 $code · 同文案" \
        "judge_rej $n '$code' '$msg'" "$n.interp.out"
done

if [ "$NEG" = 1 ]; then
    echo "=== [3] 负控（各自独立判红 · 源逐字节还原）==="
    # A：删注册 ⇒ 三轨 R1001
    restore_all; snapshot
    sed -i 's|^    px_set_global("bytes_to_dec", px_native("bytes_to_dec", bi_bytes_to_dec));|    /* NEGCTL-A */|' runtime/runtime.c
    grep -q 'NEGCTL-A' runtime/runtime.c || { echo "  FAIL [3A] 打桩失败"; fail=$((fail+1)); }
    build3 dec_pos >/dev/null 2>&1
    chk "[3A] 删注册 ⇒ 正例三轨判红" "! judge3 dec_pos \"\$WANT\""
    # B：长度守卫改回静默 null ⇒ 仅 rej_i9 红
    restore_all; snapshot
    perl -0pi -e 's{if \(len < 1 \|\| len > 8\)\n        px_error\("R1003: bytes_to_int 需要 1\.\.8 字节，实际是 %d（更宽的整数用 bytes_to_dec）", len\);}{if (len < 1 || len > 8) return px_null();   /* NEGCTL-B */}' runtime/runtime.c
    if grep -q 'NEGCTL-B' runtime/runtime.c; then
        build3 rej_i9 >/dev/null 2>&1; build3 dec_pos >/dev/null 2>&1
        chk "[3B] 恢复静默 null ⇒ **仅** rej_i9 判红（正例仍绿 = 独立牙）" \
            "! judge_rej rej_i9 'R1003' '需要 1..8 字节' && judge3 dec_pos \"\$WANT\""
    else
        echo "  FAIL [3B] 打桩失败（锚点未命中）"; fail=$((fail+1))
    fi
    # C：宽度上限 16 → 8 ⇒ 仅 d16/d17 红
    restore_all; snapshot
    sed -i 's|if (len < 1 \|\| len > 16) px_error("R1003: bytes_to_dec 需要 1..16 字节|if (len < 1 \|\| len > 8) px_error("R1003: bytes_to_dec 需要 1..8 字节|' runtime/runtime.c
    if grep -q 'bytes_to_dec 需要 1..8 字节' runtime/runtime.c; then
        build3 dec_pos >/dev/null 2>&1
        chk "[3C] 宽度上限改 8 ⇒ 正例判红（d16/d17 = u128 用例）" "! judge3 dec_pos \"\$WANT\""
        chk "[3C] 且失败原因**是**宽度上限（消息读得到）" "grep -qF 'bytes_to_dec 需要 1..8 字节，实际是 16' '$W/dec_pos.vm.out'"
    else
        echo "  FAIL [3C] 打桩失败（锚点未命中）"; fail=$((fail+1))
    fi
    restore_all
fi

echo "── 结果 ──"
echo "  通过 $pass · 失败 $fail"
[ "$fail" = 0 ] && { echo "M201-DEC-VERIFY-OK"; exit 0; }
echo "M201-DEC-VERIFY-FAIL"; exit 1
