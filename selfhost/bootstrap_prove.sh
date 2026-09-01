#!/usr/bin/env bash
# ============================================================
# M-B8: 自举证明（经典三步）
# ------------------------------------------------------------
# 1. Rust 版编译 PuXian 编译器源码（selfhost/compiler.px）→ 编译器 A
# 2. 编译器 A 编译同一份源码 → 编译器 B（C 产物）
# 3. A 与 B 的产物 diff 一致 → 自举成立
#
# 产物对比：
#   A.c = Rust 版 px build compiler.px 生成的 C（build/compiler.c）
#   B.c = 编译器 A 运行 build compiler.px 输出的 C
#   norm_c（去生成注释头/行尾空白）后逐字节 diff
#
# 用法：
#   ./bootstrap_prove.sh          # 自举证明（B.c 缓存有效则复用）
#   ./bootstrap_prove.sh --fresh  # 强制重新生成 B.c（约 3.5 分钟）
# ============================================================
set -u
cd "$(dirname "$0")"
PX="../compiler/target/release/px"
WORK=/tmp/px_bootstrap
mkdir -p "$WORK"

norm_c() {
    grep -vE '^/\* 由普贤' | sed -E 's/[[:space:]]+$//'
}

# B.c 缓存校验：源码链（compiler.px + import 链）比缓存新 → 失效
cache_valid() {
    [ -s "$WORK/B.c" ] || return 1
    local newest_src=0
    for f in compiler.px codegen.px parser.px pxlexer.px astdump.px cg_stmt.px cg_expr.px cg_module.px; do
        [ -f "$f" ] || continue
        local m; m=$(stat -c %Y "$f")
        [ "$m" -gt "$newest_src" ] && newest_src=$m
    done
    local bcache; bcache=$(stat -c %Y "$WORK/B.c" 2>/dev/null || echo 0)
    [ "$bcache" -ge "$newest_src" ]
}

echo "══════════ M-B8 自举证明 ══════════"

# ---- 步骤 1：Rust 版编译 compiler.px → 编译器 A ----
echo "── 步骤 1：Rust 版编译 compiler.px → 编译器 A"
"$PX" build compiler.px || { echo "❌ compiler.px 编译失败"; exit 1; }
echo "    ✅ 编译器 A = build/compiler"
cp build/compiler.c "$WORK/A.c"

# ---- 步骤 2：编译器 A 编译自己 → B.c ----
if [ "${1:-}" = "--fresh" ] || ! cache_valid; then
    echo "── 步骤 2：编译器 A 编译 compiler.px 自身 → B.c（约 3.5 分钟）"
    timeout 900 ./build/compiler build compiler.px > "$WORK/B.c" 2>&1
    echo "    （exit=$?，$(wc -c < "$WORK/B.c") 字节）"
else
    echo "── 步骤 2：B.c 缓存有效，复用（--fresh 强制重跑）"
fi

# ---- 步骤 3：diff A.c vs B.c ----
echo "── 步骤 3：产物 diff（norm_c 后逐字节）"
norm_c < "$WORK/A.c" > "$WORK/A.n.c"
norm_c < "$WORK/B.c" > "$WORK/B.n.c"
if diff -q "$WORK/A.n.c" "$WORK/B.n.c" >/dev/null 2>&1; then
    echo "    ✅ A.c 与 B.c 完全一致（$(wc -l < "$WORK/A.n.c") 行 C 源码）"
    echo ""
    echo "══════════ 🎉 自举成立！══════════"
    echo "PuXian 版编译器（compiler.px）能编译自己，且产物与 Rust 版逐字节一致。"
    echo "Rust 编译器退役条件满足（M-B9）。"
    exit 0
else
    echo "    ❌ A.c 与 B.c 有差异："
    diff "$WORK/A.n.c" "$WORK/B.n.c" | head -20
    exit 1
fi
