#!/usr/bin/env bash
# ============================================================================
# chain_red_lib.sh —— 链「判红写戳」的唯一实现（缺陷 884 / M430 补3）
#
# 为什么单独成文件：这段逻辑必须能被**负控**验证
#   （selfhost/check_chain_red_keeps_stamp.sh 直接 source 本文件、断言戳的内容），
#   而 run_release_chain.sh 跑一次就是 2 小时（前置自证 → 305 门 → 戳 → 发布），
#   拿它当测试夹具等于不可测。
#
# 事故（2026-11-21 M430 r2）：chain_stamp.sh 已写出详细戳
#   `CHAIN-VERDICT=RED reason=红 2 门 green=303 red=2 listed=305 expect=305 timeout=0 head=e086279`
#   随后 run_release_chain.sh 的 red() 用 `echo "CHAIN-VERDICT=RED reason=$1" > $DONE` 整行覆盖
#   ⇒ 只剩 `CHAIN-VERDICT=RED reason=no-go` ⇒ m430_pipeline.sh 解析 green= 失败
#   ⇒ 派生台账 m430-pipeline-criteria-empty（rc=4 criteria-empty），
#      把「2 门真红」误报成「判据没写清为什么红」，白查一轮。
#
# 调用方必须提供：$DONE $MS $DRY $NOTIFY_SH $LOG 与 say()；
#                $STAMP_FROM_THIS_RUN=1 表示戳是本轮 chain_stamp.sh 刚写的。
# ============================================================================

chain_red() {  # chain_red <reason> <台账键> <标题>
    local reason="$1" key="$2" title="$3"

    if [ "${STAMP_FROM_THIS_RUN:-0}" = "1" ] && [ -f "$DONE" ] \
       && grep -q 'green=' "$DONE" 2>/dev/null; then
        # 本轮详细戳在 ⇒ 只在尾部追加原因行，逐字保留 green=/red=/listed=/head=
        printf 'CHAIN-RED-REASON=%s\n' "$reason" >> "$DONE"
        say "RED $reason（保留详细戳，只追加 CHAIN-RED-REASON）"
    else
        # 前置自证阶段判红（戳还没写）⇒ 写短戳；此时 $STAMP_FROM_THIS_RUN=0
        printf 'CHAIN-VERDICT=RED reason=%s\n' "$reason" > "$DONE"
        say "RED $reason"
    fi

    [ "$DRY" = "1" ] || DY_NOTIFY_MUTATE=publish "$NOTIFY_SH" p1 "$key" "$title" \
        "M${MS} head=$(git rev-parse --short HEAD) · 证据 $LOG · 修后重起 launch_release_chain.sh" || true
}
