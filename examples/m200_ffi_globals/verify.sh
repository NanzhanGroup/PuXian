#!/usr/bin/env bash
# ============================================================
# M200 门（第 78 轮）：**extern def（C-FFI 桥）名字的全局发布** —— 缺陷 240
#                          + 上游 registry-px 全量再引入（33 新库 + 8 就地更新）
# ------------------------------------------------------------
# 权威口径 = docs/ERROR_CODES.md §6.11（本轮建立）
#
# 为什么需要（缺陷 240 的由来 —— 由"引入官方库"这件事照出来的）：
#   `extern def` 声明的名字（例：官方 `registry/zlib` 的 `import "c/zlib"` +
#   `extern def zlib_compress(crc32/uncompress)`）：
#     · **解释轨**有运行期兜底 —— `selfhost/iexpr.px` 遇未知名/FFI 名 → `i_builtin_ffi_call`
#       → C 侧 `ffi_call` 的**双表**（ffi 注册表 → 全局 native 表）⇒ 可用；
#     · **编译轨**把该名字编译成 **GETG**（extern def 名即全局名），而运行期**从未**把它
#       发布成全局 ⇒ `运行时错误 [zl_compress 行22]: R1001: 未定义变量: 'zlib_compress'`。
#   实证：`px build`（默认档）与 `px build --full` **皆然** ⇒ 与"按引用集自动裁剪"无关，
#   是**发布缺失**；后果 = **官方 registry 的 zlib 包在编译产物里完全不可用**
#   （第三方把它记成 PX-DEF-035，且方向记反了：他们以为"编译轨可用、解释轨未注册"）。
#
# 判据：
#   [1] 静态 [S11]：`px_ffi_register` 的注册名集合非空（下限）· `px_ffi_publish_globals`
#       定义在 `runtime_ffi.c` 且**遍历整表**（`for (i = 0; i < g_ffi_n; i++)`）·
#       且在 `px_register_builtins` 内被调用（发布点唯一且在建表窗口内）。
#   [2] 动态：3 条**确定性**探针（不经环境）在**三轨**（解释/VM/C）输出**逐字节一致**：
#       f01 `zlib_crc32`（官方 zlib 包的核心）· f02 `bytes_to_hex`/`hex_to_bytes` 往返 ·
#       f03 `float32_bits`/`bits_to_float32`（IEEE 位模式）。
#       负控 A/B/C 各自独立判红（见下）。
#   [3] 上游联动：`upstream-tests/zlib_test`（官方包的真实用例）在双轨转 **PASS**
#       —— 由 `selfhost/run_upstream_tests.sh` 守（m116 全量门里跑）。
#
# 负控（各自独立判红 · 源逐字节还原）：
#   A 删掉 `px_register_builtins` 里的 `px_ffi_publish_globals();` 调用 ⇒ 编译轨 R1001 ⇒ [2] 红
#   B 把发布循环改成"跳过名字含 zlib 的" ⇒ 仅 f01 红（证明**动态面**有独立牙，静态仍绿）
#   C 把发布循环改成 `for (i = 0; i < 0; i++)`（不遍历）⇒ **静态** [S11] 红
# 用法：bash examples/m200_ffi_globals/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m200_gate; BACK=$W/bak
rm -rf "$W"; mkdir -p "$W" "$BACK"
FILES=(runtime/runtime.c runtime/runtime_ffi.c)
pass=0; fail=0
chk() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
snapshot() { for f in "${FILES[@]}"; do cp -f "$f" "$BACK/$(echo "$f" | tr / _)"; done; }
restore_all() { for f in "${FILES[@]}"; do b="$BACK/$(echo "$f" | tr / _)"; [ -f "$b" ] && cp -f "$b" "$f"; done; return 0; }
snapshot
trap 'restore_all' EXIT

# ── 前置不变量 ──
pre() { grep -qF "$1" "$2" || { echo "❌ 前置自查失败：$2 缺「$1」" >&2; exit 2; }; }
pre 'px_ffi_publish_globals(void);' runtime/runtime.h
pre 'for (i = 0; i < g_ffi_n; i++) {' runtime/runtime_ffi.c
pre 'px_ffi_publish_globals();' runtime/runtime.c

sweep_static() { python3 "$HERE/sweep_ffi.py" --root "$ROOT" > "$W/s11.log" 2>&1; }
build3() {   # $1=probe 名 → 产出 $W/<n>.{interp,vm,c}.out 与 .rc
    # ⚠️ 必须分两行（M201 顺手修）：`local n="$1" d="$W/b_$n"` 里 d 取到的是**外层** n
    #   —— bash 5.1 + set -u：外层无 n ⇒ "n: unbound variable"。本门原先靠循环留下的
    #   全局 n "恰好"没炸（本文件下方就是 `for n in …; do build3 "$n"`）⇒ 纯属巧合。
    local n="$1"
    local d="$W/b_$n"
    rm -rf "$d"; mkdir -p "$d"; cp -f "$HERE/probe/$n.px" "$d/"
    ( cd "$d" && timeout 60 "$ROOT/bootstrap/pxi" "$n.px" ) > "$W/$n.interp.out" 2>&1; echo $? > "$W/$n.interp.rc"
    ( cd "$d" && timeout 600 "$ROOT/tools/px" build "$n.px" ) > "$W/$n.vm.build" 2>&1
    if [ -x "$d/build/$n" ]; then ( cd "$d" && timeout 60 "$d/build/$n" ) > "$W/$n.vm.out" 2>&1; echo $? > "$W/$n.vm.rc"
    else echo 999 > "$W/$n.vm.rc"; tail -3 "$W/$n.vm.build" > "$W/$n.vm.out"; fi
    rm -rf "$d/build"
    ( cd "$d" && PX_BUILD_ENGINE=c timeout 600 "$ROOT/tools/px" build "$n.px" ) > "$W/$n.c.build" 2>&1
    if [ -x "$d/build/$n" ]; then ( cd "$d" && timeout 60 "$d/build/$n" ) > "$W/$n.c.out" 2>&1; echo $? > "$W/$n.c.rc"
    else echo 999 > "$W/$n.c.rc"; tail -3 "$W/$n.c.build" > "$W/$n.c.out"; fi
    rm -rf "$d/build"
}
judge3() {   # $1=probe  $2=期望 stdout（多行以 \n 连接）
    local n="$1" want="$2" t
    for t in interp vm c; do
        [ "$(cat "$W/$n.$t.rc" 2>/dev/null)" = 0 ] || return 1
        [ "$(cat "$W/$n.$t.out" 2>/dev/null | sed '/^$/d')" = "$want" ] || return 1
    done
    return 0
}

echo "=== [1] 静态 [S11]：FFI 注册名 ⇄ 全局发布（唯一发布点 · 遍历整表）==="
sweep_static; rc=$?
chk "[1] 扫描器：注册名 ≥ 150 · 发布点唯一 · 遍历整表 · 建表窗口内调用" \
    "[ \$rc = 0 ] && grep -q '结论：不一致 0' '$W/s11.log'"
chk "[1] 静态自证数字（打印，便于人核）" "grep -qE 'px_ffi_register 名字数 *: *[0-9]+' '$W/s11.log'"

echo "=== [2] 动态：3 条确定性探针 × 三轨（解释 / VM / C）输出逐字节一致 ==="
for n in f01_zlib_crc32 f02_hex_roundtrip f03_float32_bits; do build3 "$n"; done
chk "[2] f01 zlib_crc32（官方 registry/zlib 的核心）三轨同值 891568578" \
    "judge3 f01_zlib_crc32 '891568578'"
chk "[2] f02 bytes_to_hex / hex_to_bytes 往返三轨同值" \
    "judge3 f02_hex_roundtrip '4142
2'"
chk "[2] f03 float32_bits / bits_to_float32（IEEE 位模式）三轨同值" \
    "judge3 f03_float32_bits '1069547520
1.5'"

if [ "$NEG" = 1 ]; then
    echo "=== [3] 负控（各自独立判红 · 源逐字节还原）==="

    # A：删掉发布调用 ⇒ 编译轨 R1001
    restore_all; snapshot
    python3 - <<'PY'
s = open('runtime/runtime.c', encoding='utf-8').read()
old = '    px_ffi_publish_globals();\n'
assert s.count(old) == 1
open('runtime/runtime.c', 'w', encoding='utf-8').write(s.replace(old, ''))
PY
    sweep_static >/dev/null 2>&1; rc_static=$?
    build3 f01_zlib_crc32 >/dev/null 2>&1
    chk "[3A] 删发布调用 ⇒ 静态 [S11] 红（发布点缺失）" "[ \$rc_static != 0 ]"
    chk "[3A] 删发布调用 ⇒ 编译轨 R1001（三轨不再一致）" \
        "! judge3 f01_zlib_crc32 '891568578' && grep -q 'R1001' '$W/f01_zlib_crc32.vm.out'"

    # B：跳过名字含 zlib 的 ⇒ 仅 f01 红（动态有独立牙）
    restore_all; snapshot
    python3 - <<'PY'
s = open('runtime/runtime_ffi.c', encoding='utf-8').read()
old = '''    for (i = 0; i < g_ffi_n; i++) {
        px_set_global(g_ffi_syms[i].name, px_native(g_ffi_syms[i].name, g_ffi_syms[i].fn));
    }'''
new = '''    for (i = 0; i < g_ffi_n; i++) {
        if (strstr(g_ffi_syms[i].name, "zlib") != NULL) continue;   // NEGCTL-B
        px_set_global(g_ffi_syms[i].name, px_native(g_ffi_syms[i].name, g_ffi_syms[i].fn));
    }'''
assert s.count(old) == 1
open('runtime/runtime_ffi.c', 'w', encoding='utf-8').write(s.replace(old, new))
PY
    build3 f01_zlib_crc32 >/dev/null 2>&1; build3 f02_hex_roundtrip >/dev/null 2>&1
    sweep_static >/dev/null 2>&1; rc_static=$?
    chk "[3B] 跳过 zlib 名 ⇒ **仅** f01 判红（f02 仍一致 · 静态仍绿 = 动态面有独立牙）" \
        "[ \$rc_static = 0 ] && ! judge3 f01_zlib_crc32 '891568578' && judge3 f02_hex_roundtrip '4142
2'"

    # C：循环不遍历 ⇒ 静态红
    restore_all; snapshot
    sed -i 's|for (i = 0; i < g_ffi_n; i++) {|for (i = 0; i < 0; i++) {   /* NEGCTL-C */|' runtime/runtime_ffi.c
    sweep_static; rc=$?
    chk "[3C] 发布循环不遍历整表 ⇒ 静态 [S11] 红" \
        "[ \$rc != 0 ] && grep -q '不一致' '$W/s11.log'"
    restore_all
fi

restore_all
echo ""
echo "═══ M200 门：通过 $pass · 失败 $fail ═══"
if [ "$fail" = 0 ]; then echo "M200-VERIFY-OK"; exit 0; fi
exit 1
