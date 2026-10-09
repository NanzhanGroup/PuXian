#!/usr/bin/env bash
# ============================================================
# selfhost/gate_verdict.sh —— 门的**末行裁决**（M293 · 缺陷 506）
# ------------------------------------------------------------
# 为什么需要（M293 首次全量门实测）：
#   本仓有一批门的结尾是「打印 `MNNN-VERIFY-OK` 但**无条件 `exit 0`**」。
#   而 `selfhost/run_gates.sh` 的失败计数**只看 rc** ⇒ 这类门的**门内 ❌ 被静默吞掉**：
#   本地全量门报「失败 0 项」，而门的日志末行明明写着 `失败 1 项`。
#   实测（M293）：`m293_h3_robustness` 内部 `失败 1 项 · 通过 37 项`，运行器却报 ✅。
#   CI 侧本来就有这条判据（`packaging/ci_diagnose.py` 的末行裁决）⇒ 只有**本地**是盲区。
#
# 口径：**与 `packaging/ci_diagnose.py` 同一份规则**（不许发明第二套）——
#   ⚠️ 「失败优先」：末行同时含「通过」与「失败」时（`══ 通过 165 · 失败 3 ══`）判**红**。
#      这是 M216s1 记下的真盲区（旧版把 `通过` 当 OK ⇒ 判成绿）。
#
# 误报率实测（M293）：对**刚跑完的全量门 200 份 `/tmp/gate_*.log`** 施加本判据 ⇒
#   判红 **1** 份，且是**真阳性**（就是 m293 自己那一次）⇒ **误报 0/199**。
#   这次实测就是「先量，再改」的落点：**不量就上线这条规则，等于赌它不误伤**。
#
# 用法：
#   . selfhost/gate_verdict.sh          # 只取函数（run_gates.sh 这样用）
#   bash selfhost/gate_verdict.sh --self-test   # 自证（合成夹具 + 真实语料形态）
#   cat <门日志> | gate_verdict_lastline        # 0=OK · 1=BAD · 2=无法判定
# ============================================================

# ⚠️ 与 packaging/ci_diagnose.py 的 OK_LAST / BAD_LAST **逐条对齐**（改一处必须改两处）
GATE_OK_LAST_RE='(✅|VERIFY-OK|VERIFY:|ALL-OK|FAIL=0|fail=0|失败[[:space:]]*0|PASS|通过|一致|EXPECTED|SELFTEST-OK|done|[0-9]+P/0F/)'
GATE_BAD_LAST_RE='(FAIL=[1-9]|FAIL-[A-Z]|[1-9][[:space:]]*失败|失败[[:space:]]*[1-9]|问题[[:space:]]*[1-9]|fail=[1-9]|FAILED|❌|VERIFY-FAIL)'

# 从 stdin 读日志 ⇒ 0=OK · 1=BAD · 2=无法判定（**失败优先**）
# ⚠️ `grep -a`：日志里可能有 NUL（二进制语料输出）⇒ 不带 -a 会「binary file matches」而失判
gate_verdict_lastline() {
    local ll
    ll="$(grep -a -v '^[[:space:]]*$' | tail -1)"
    if printf '%s' "$ll" | grep -Eaq "$GATE_BAD_LAST_RE"; then return 1; fi
    if printf '%s' "$ll" | grep -Eaq "$GATE_OK_LAST_RE";  then return 0; fi
    return 2
}

# ───────────────────────── 自证 ─────────────────────────
if [ "${1:-}" = "--self-test" ]; then
    P=0; F=0
    chk() {   # $1=标签 $2=期望(0/1/2) $3=末行
        local got
        printf '%s\n' "$3" | gate_verdict_lastline
        got=$?
        if [ "$got" = "$2" ]; then P=$((P + 1)); echo "  ✅ $1（$got）"; else F=$((F + 1)); echo "  ❌ $1：期望 $2，实际 $got（末行 [$3]）"; fi
    }
    echo "===== gate_verdict 自证（判据 = 末行 + 失败优先）====="
    chk "M293-VERIFY-OK ⇒ OK"                    0 "M293-VERIFY-OK"
    chk "失败 1 项 · 通过 37 项 ⇒ BAD（失败优先）"  1 "===== 失败 1 项 · 通过 37 项 ====="
    chk "通过 165 · 失败 3 ⇒ BAD（同时含通过）"     1 "══ 通过 165 · 失败 3 ══"
    chk "PASS=29 FAIL=0 ⇒ OK"                    0 "PASS=29 FAIL=0"
    chk "FAIL=2 ⇒ BAD"                           1 "PASS=27 FAIL=2"
    chk "❌ 在末行 ⇒ BAD"                          1 "  ❌ 编译失败"
    chk "VERIFY-FAIL ⇒ BAD"                      1 "M225-VERIFY-FAIL（通过 12 / 失败 2）"
    chk "末尾空行被跳过 ⇒ 取最后**非空**行（OK 侧）"   0 "$(printf 'M293-VERIFY-OK\n\n\n')"
    chk "末尾空行被跳过 ⇒ 取最后**非空**行（BAD 侧）"  1 "$(printf '失败 9 项\n\n\n')"
    chk "纯中文「一致」⇒ OK"                       0 "逐字节一致"
    chk "无标记 ⇒ 无法判定(2)"                     2 "跑完了"
    chk "done ⇒ OK"                              0 "SMOKE-done"
    chk "仅「失败 0」⇒ OK（数字 0 不算坏）"          0 "===== 失败 0 项 · 通过 34 项 ====="
    echo "===== 通过 $P 项 · 失败 $F 项 ====="
    [ "$F" -eq 0 ] && echo "GATE-VERDICT-SELFTEST-OK" && exit 0
    exit 1
fi
