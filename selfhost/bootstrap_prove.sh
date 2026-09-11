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
#   ./bootstrap_prove.sh                  # 自举证明（B.c 缓存有效则复用）
#   ./bootstrap_prove.sh --fresh           # 强制重新生成 B.c（实测约 6.5-7 分钟，8 核 16G）
#   ./bootstrap_prove.sh --update-golden   # 有意改动编译器后重定基 golden/compiler.c
#                                          # （重定基后强制重跑步骤 1 自证；改过源码/基准须同批
#                                          #   重烘入库二进制：cp selfhost/build/compiler_vm bootstrap/pxc_vm）
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

# ---- M112/Issue 48：--update-golden：把「指引」变成可执行（重定基后自证）----
UPDATE_GOLDEN=0
for a in "$@"; do [ "$a" = "--update-golden" ] && UPDATE_GOLDEN=1; done
if [ "$UPDATE_GOLDEN" = "1" ]; then
    echo "── [--update-golden] 重定基 golden/compiler.c（原 $(wc -l < "$GOLDEN") 行）"
    cp -a "$GOLDEN" "$WORK/compiler.c.bak"
    ug_rc=0
    timeout 900 "$PXC" build compiler.px > "$GOLDEN" 2>"$WORK/golden.err" || ug_rc=$?
    if [ "$ug_rc" -ne 0 ] || [ ! -s "$GOLDEN" ]; then
        echo "❌ 重定基失败（exit=$ug_rc），已回滚 golden" >&2
        cp -a "$WORK/compiler.c.bak" "$GOLDEN"
        tail -5 "$WORK/golden.err" >&2
        exit 1
    fi
    echo "    新基准 $(wc -l < "$GOLDEN") 行 / 旧 $(wc -l < "$WORK/compiler.c.bak") 行，差异 $(diff "$WORK/compiler.c.bak" "$GOLDEN" | grep -c '^[<>]') 行"
    # 自证（不靠“应该一致”）：优先与既有 B.c 缓存对拍（同一源码链）；
    # 无缓存或对不上 → 清缓存，让步骤 1 现算、步骤 2 复核。
    if [ -s "$WORK/B.c" ]; then
        if diff -q <(norm_c < "$WORK/B.c") <(norm_c < "$GOLDEN") >/dev/null 2>&1; then
            echo "    ✅ 自证：新基准 == 既有 B.c 缓存（norm_c 后逐字节一致）"
        else
            echo "    ⚠️ 新基准与 B.c 缓存不一致（缓存可能来自旧源码）→ 步骤 1 重跑复核"
            rm -f "$WORK/B.c"
        fi
    else
        echo "    无 B.c 缓存 → 步骤 1 将现算并由步骤 2 复核"
    fi
fi

# ---- 步骤 1：用引导编译器编译 compiler.px → B.c ----
if [ "${1:-}" = "--fresh" ] || ! cache_valid; then
    echo "── 步骤 1：bootstrap/pxc 编译 compiler.px → B.c（实测约 6.5-7 分钟，8 核 16G 单线程）"
    # Issue 49：产物通道与诊断通道必须分开，且必须判退出码。
    # 旧写法 `> B.c 2>&1` 会把诊断文本写进“产物”，失败时 B.c 仍非空 →
    # 步骤 1 表面通过，错误被推迟到步骤 2 报成“产物有差异”（误导）。
    step1_rc=0
    timeout 900 "$PXC" build compiler.px > "$WORK/B.c" 2>"$WORK/B.err" || step1_rc=$?
    echo "    （exit=$step1_rc，$(wc -c < "$WORK/B.c") 字节；诊断 → $WORK/B.err）"
    if [ "$step1_rc" -ne 0 ]; then
        echo "❌ 步骤 1 编译失败（exit=$step1_rc）：B.c 不是有效产物，后续 diff 无意义" >&2
        tail -5 "$WORK/B.err" >&2
        exit 1
    fi
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
    echo "     有意改动请重定基（Issue 48：旧提示指向不存在的 tools/bootstrap.sh）："
    echo "       cd selfhost && ./bootstrap_prove.sh --update-golden      # 重定基 golden/compiler.c"
    echo "       cd selfhost && ./bootstrap_prove_bc.sh --update-golden   # 重定基 golden/compiler.bc.dump"
    echo "     两条基准须与源码同批提交；改过基准后入库二进制同步重烘："
    echo "       cp selfhost/build/compiler_vm bootstrap/pxc_vm && ./selfhost/engine_parity.sh"
    exit 1
fi
