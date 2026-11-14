#!/usr/bin/env bash
# ============================================================
# selfhost/gate_skip_loud.sh —— **门「绿」必须承重：静默跳过判红**（M405 · 缺陷 776）
# ------------------------------------------------------------
# 为什么要它（2026-11-14 实测，M404 r3 那轮的 278 门全绿之后）：
#   把 /data/gatevm/shard-20261114-142623/gatelog 里 385 份门日志逐份扫，
#   **12 扇注册门的日志里出现行首 SKIP，其中 0 扇带 [585-SKIP-LOUD]**。
#   它们被记成「绿」，但绿的理由是**断言被跳过了**，不是被验过了。
#   与 M403 那 46 扇孤儿门是同一族病，区别更要紧：这 12 扇**就在名单里**，
#   我们一直以为它们在验。
#
# 口径（与 examples/m403_orphan_batch 的 C3 同源，落地到主运行器）：
#   ① 只认**行首**的 SKIP / ⏭ / [SKIP]（行内提及不算 —— m313 的说明文字就被这条排除）
#   ② 排除计数器与变量名：`SKIP=0` / `SKIP_NORM =` / 纯数字统计行（m216/m217/m207 靠这条）
#   ③ 无 [585-SKIP-LOUD] 标记 ⇒ 判红（静默跳过 = 零观测）
#   ④ 有标记但全篇没有任何正向断言（PASS/OK/✅）⇒ 仍判红（整门跳过也是零覆盖）
#
# 用法：bash selfhost/gate_skip_loud.sh <日志>      # 0=允许 1=静默跳过 2=用法错
#       bash selfhost/gate_skip_loud.sh --self-test # 自证 9 条（含前提自证）
# ============================================================
set -uo pipefail

MARK='585-SKIP-LOUD'

# 行首跳过行（排除计数器/变量）
skip_lines() {   # $1=日志
    grep -a -nE '^[[:space:]]*(⏭️?|\[SKIP\]|SKIP)' "$1" 2>/dev/null \
        | grep -avE '^[0-9]+:[[:space:]]*SKIP[[:space:]]*([=_]|[0-9]+[[:space:]]*$)' || true
}
positive_lines() {   # $1=日志
    grep -a -cE '(PASS|✅|VERIFY-OK|-OK（|OK（)' "$1" 2>/dev/null || true
}

check() {   # $1=日志
    local log="$1" n pos
    [ -f "$log" ] || { echo "用法错：日志不存在 $log" >&2; return 2; }
    n=$(skip_lines "$log" | wc -l)
    if [ "$n" -eq 0 ]; then
        echo "SKIP-LOUD-NA（无跳过行）"
        return 0
    fi
    pos=$(positive_lines "$log")
    if ! grep -aq "$MARK" "$log"; then
        echo "静默跳过 $n 处（缺 [$MARK] 标记）—— 绿不承重："
        skip_lines "$log" | head -3 | sed 's/^/     /'
        return 1
    fi
    if [ "${pos:-0}" -eq 0 ]; then
        echo "整门跳过（有标记但零正向断言）⇒ 零覆盖"
        return 1
    fi
    echo "SKIP-LOUD-OK（$n 处跳过已声明 · 正向断言 $pos 处）"
    return 0
}

self_test() {
    local sp=0 sf=0 d
    d=$(mktemp -d)
    ok() { echo "  ✅ $1"; sp=$((sp+1)); }
    ng() { echo "  ❌ $1 ｜$2"; sf=$((sf+1)); }

    printf 'PASS 断言一\nSKIP 真实模型（未设 env）\nPASS 断言二\n' > "$d/silent.log"
    if ! check "$d/silent.log" >/dev/null; then ok "N1 负控：行首 SKIP 无标记 ⇒ 判红"; else ng "N1 负控" "判绿了"; fi

    printf 'PASS 断言一\nSKIP [585-SKIP-LOUD] 真实模型（未设 env）\n' > "$d/loud.log"
    if check "$d/loud.log" >/dev/null; then ok "P1 正控：带标记 + 有正向断言 ⇒ 允许"; else ng "P1 正控" "判红了"; fi

    printf '                 SKIP     36\n' > "$d/counter.log"
    if check "$d/counter.log" >/dev/null; then ok "P2 正控：纯数字统计行是计数器，不算跳过"; else ng "P2 计数器" "被误判"; fi

    printf 'SKIP=0\nSKIP_NORM = 正常档不通\nPASS x\n' > "$d/var.log"
    if check "$d/var.log" >/dev/null; then ok "P3 正控：SKIP=/SKIP_XXX= 是变量，不算跳过"; else ng "P3 变量" "被误判"; fi

    printf 'PASS a\n说明：CI 上 SKIP（行内提及）\n' > "$d/inline.log"
    if check "$d/inline.log" >/dev/null; then ok "P4 正控：行内提及不算（只认行首）"; else ng "P4 行内" "被误判"; fi

    printf '[585-SKIP-LOUD] 整门跳过\nSKIP 全部\n' > "$d/allskip.log"
    if ! check "$d/allskip.log" >/dev/null; then ok "N2 负控：有标记但零正向断言 ⇒ 判红（整门跳过）"; else ng "N2 整门" "判绿了"; fi

    printf '  ⏭️  真实模型端到端（未开启）\nPASS 静态层\n' > "$d/arrow.log"
    if ! check "$d/arrow.log" >/dev/null; then ok "N3 负控：⏭ 行首也算跳过（M157 的形状）"; else ng "N3 ⏭" "漏了"; fi

    printf 'PASS a\n' > "$d/clean.log"
    if check "$d/clean.log" >/dev/null; then ok "P5 正控：无跳过行的门直接放过"; else ng "P5 clean" "被误判"; fi

    # 前提自证：确认判据真的在读日志（不是空转）
    local got; got=$(skip_lines "$d/silent.log" | wc -l)
    if [ "$got" -eq 1 ]; then ok "P 前提自证：silent.log 实测跳过行 = 1（判据在扫）"; else ng "P 前提自证" "数到 $got"; fi

    if check /nonexistent/log >/dev/null 2>&1; then ng "N4 用法错" "不存在的日志被判绿"; else ok "N4 负控：日志不存在 ⇒ rc=2（不许当绿）"; fi

    echo "── gate_skip_loud --self-test：通过 $sp · 失败 $sf"
    rm -rf "$d"
    [ "$sf" -eq 0 ] || return 2
    return 0
}

case "${1:-}" in
    --self-test) self_test ;;
    ''|-h|--help) sed -n '2,30p' "$0"; exit 2 ;;
    *) check "$1"; exit $? ;;
esac
