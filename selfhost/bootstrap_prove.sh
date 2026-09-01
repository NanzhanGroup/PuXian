#!/usr/bin/env bash
# ============================================================
# M-B9a: 自举证明（PuXian-only 版，Rust 已退役）
# ------------------------------------------------------------
# 引导链：bootstrap/pxc（入库的 PuXian 版编译器二进制，M-B8 自举产物）
# 证明：pxc 编译 compiler.px 的当前产物（B.c）与入库基准（golden/compiler.c，
#       = pxc 自身的 C 产物 A.c）逐字节一致 → 编译器源码 ↔ 二进制对应，
#       源码改动若破坏一致性，此证明立即失败。
#
# 用法：
#   ./bootstrap_prove.sh            # 自举证明（B.c 缓存有效则复用）
#   ./bootstrap_prove.sh --fresh    # 强制重新生成 B.c（约 3.5 分钟）
# ============================================================
set -u
cd "$(dirname "$0")"
PXC="../bootstrap/pxc"
GOLDEN="golden/compiler.c"
WORK=/tmp/px_bootstrap
mkdir -p "$WORK"

norm_c() {
    grep -vE '^/\* 由普贤' | sed -E 's/[[:space:]]+$//'
}

# B.c 缓存校验：源码链（compiler.px + import 链）比缓存新 → 失效
cache_valid() {
    [ -s "$WORK/B.c" ] || return 1
    local newest_src=0
    for f in compiler.px codegen.px parser.px pxlexer.px cg_stmt.px cg_expr.px cg_module.px; do
        [ -f "$f" ] || continue
        local m; m=$(stat -c %Y "$f")
        [ "$m" -gt "$newest_src" ] && newest_src=$m
    done
    local bcache; bcache=$(stat -c %Y "$WORK/B.c" 2>/dev/null || echo 0)
    [ "$bcache" -ge "$newest_src" ]
}

echo "══════════ M-B9a 自举证明（PuXian-only）══════════"

# ---- 前置检查 ----
[ -x "$PXC" ] || { echo "❌ 缺少引导编译器 bootstrap/pxc" >&2; exit 1; }
[ -f "$GOLDEN" ] || { echo "❌ 缺少基准 golden/compiler.c（= 引导编译器自身的 C 产物）" >&2; exit 1; }
echo "── 基准：golden/compiler.c（$(wc -l < "$GOLDEN") 行 C 源码，引导编译器自身产物）"

# ---- 步骤 1：用引导编译器编译 compiler.px → B.c ----
if [ "${1:-}" = "--fresh" ] || ! cache_valid; then
    echo "── 步骤 1：bootstrap/pxc 编译 compiler.px → B.c（约 3.5 分钟）"
    timeout 900 "$PXC" build compiler.px > "$WORK/B.c" 2>&1
    echo "    （exit=$?，$(wc -c < "$WORK/B.c") 字节）"
else
    echo "── 步骤 1：B.c 缓存有效，复用（--fresh 强制重跑）"
fi
[ -s "$WORK/B.c" ] || { echo "❌ B.c 为空（编译失败）" >&2; exit 1; }

# ---- 步骤 2：diff B.c vs golden（A.c）----
echo "── 步骤 2：产物 diff（norm_c 后逐字节）"
norm_c < "$WORK/B.c" > "$WORK/B.n.c"
norm_c < "$GOLDEN" > "$WORK/A.n.c"
if diff -q "$WORK/A.n.c" "$WORK/B.n.c" >/dev/null 2>&1; then
    echo "    ✅ B.c 与基准完全一致（$(wc -l < "$WORK/A.n.c") 行 C 源码）"
    echo ""
    echo "══════════ 🎉 自举成立（PuXian-only）══════════"
    echo "引导编译器（bootstrap/pxc）重新编译 compiler.px，产物与自身逐字节一致。"
    echo "Rust 编译器已退役：工具链完全由 PuXian 自身驱动。"
    exit 0
else
    echo "    ❌ B.c 与基准有差异："
    diff "$WORK/A.n.c" "$WORK/B.n.c" | head -20
    echo ""
    echo "提示：若刚改了编译器源码，需先确认改动是有意的；"
    echo "     有意改动请重新生成基准：tools/bootstrap.sh --update-golden"
    exit 1
fi
