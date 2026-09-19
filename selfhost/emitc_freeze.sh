#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# 发射冻结门（emit-c freeze）—— M153（第 35 轮）建立
# ---------------------------------------------------------------
# 为什么要有：`--check` / `--check-vm` / `bootstrap_prove` 判的都是 **compiler.px 自己**
#   的产物；而"改了 runtime/优化了发射路径，却**顺手动摇了别的程序的发射结果**"这一类
#   回归，没有任何门看得见（`selfhost/cases/*.px` + `examples/` 的产物没人守）。
#   第 34 轮起用临时脚本手查 253 个文件；本轮把它**变成常驻门**。
# 判据：对 `selfhost/cases/*.px` + `examples/*/*.px` 逐个跑 `bootstrap/pxc_vm --emit-c`，
#   sha256 必须与 `selfhost/golden/emitc_freeze.sha` 逐行一致（rc/输出都进指纹）。
# 口径：
#   · 纯性能/纯 runtime 改动 ⇒ 本门必须**全绿**（M153 就是这种情况：253/253 不变）。
#   · 有意改发射 ⇒ 用 `--freeze` 重定基并**在同一提交里说明为什么**。
# 用法：
#   bash selfhost/emitc_freeze.sh --check     # 门（默认）
#   bash selfhost/emitc_freeze.sh --freeze    # 重定基（需人工确认发射差异是预期的）
# 退出码：0 = 一致（或冻结完成）；1 = 有差异
# ═══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")/.."
export LC_ALL=C LANG=C
GOLDEN=selfhost/golden/emitc_freeze.sha
MODE="${1:---check}"
TMP=/tmp/emitc_freeze_now.sha

files() { ls selfhost/cases/*.px examples/*/*.px 2>/dev/null; }

gen() {
    : > "$TMP"
    for f in $(files); do
        out=$(timeout 60 ./bootstrap/pxc_vm --emit-c "$f" 2>/tmp/emitc_freeze_err.txt)
        rc=$?
        h=$(printf '%s' "$out" | sha256sum | cut -c1-32)
        eh=$(sha256sum < /tmp/emitc_freeze_err.txt | cut -c1-16)
        echo "$f rc=$rc out=$h err=$eh" >> "$TMP"
    done
}

case "$MODE" in
--freeze)
    gen
    cp "$TMP" "$GOLDEN"
    echo "✅ 已重定基：$GOLDEN（$(wc -l < "$GOLDEN") 个文件）"
    ;;
--check)
    [ -f "$GOLDEN" ] || { echo "❌ 缺基准文件 $GOLDEN（先 --freeze）"; exit 1; }
    gen
    if diff -u "$GOLDEN" "$TMP" > /tmp/emitc_freeze_diff.txt; then
        echo "✅ 发射冻结：$(wc -l < "$TMP") 个文件的 --emit-c 输出与基准逐字节一致"
        exit 0
    fi
    echo "❌ 发射冻结被打破（$(grep -c '^[<>]' /tmp/emitc_freeze_diff.txt) 行差异）："
    head -20 /tmp/emitc_freeze_diff.txt | sed 's/^/   /'
    echo "   ⇒ 若差异是**有意的**发射改动：bash selfhost/emitc_freeze.sh --freeze（并在提交信息里说明）"
    exit 1
    ;;
*)
    echo "用法：$0 [--check|--freeze]"; exit 2 ;;
esac
