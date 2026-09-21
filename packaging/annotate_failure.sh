#!/usr/bin/env bash
# ============================================================
# packaging/annotate_failure.sh —— 把「失败的日志要点」打成**一条富注解**（M168）
# ------------------------------------------------------------
# 为什么需要它（2026-09-21 实测教训）：
#   本仓 job 日志对非管理员**不可读**（API 403 "Must have admin rights"），
#   **注解是唯一对外可读的通道**（公开 check-runs API）。
#   而 GitHub 对每个 step 的注解**有数量上限**（实测只回传前 ~10 条）：
#   直接 `tail -60 | sed 's/^/::error::/'` 会被截断在"docker 拉镜像进度"这类
#   噪声行上 —— 真因在后面，看不见（首跑两次都被这个上限挡住）。
#   ⇒ 本脚本把要点收进**一条**注解：关键行（❌/错误/Error/LIMITATION/No such…）
#     优先，其后是尾部 12 行；换行按 GitHub 规定编码为 `%0A`。
# 用法：annotate_failure.sh <前缀> <日志文件> [关键行数=14] [尾行数=12]
# ============================================================
set -uo pipefail
PREFIX="${1:-job}"
LOG="${2:-}"
KEYN="${3:-14}"
TAILN="${4:-12}"
[ -n "${LOG}" ] || { echo "用法: annotate_failure.sh <前缀> <日志文件> [关键行数] [尾行数]" >&2; exit 2; }
[ -f "${LOG}" ] || { echo "::error::${PREFIX} 失败，但日志文件不存在: ${LOG}"; exit 0; }

ESC() { sed 's/%/%25/g; s/\r//g' ; }   # 先转义 %（注解里 %0A 表示换行，故 % 必须先转义）

body="$(
  {
    echo "── 关键行（❌/错误/LIMITATION/No such…）──"
    grep -aE '❌|错误|Error|error:|No such|not found|Cannot|cannot|LIMITATION|失败|Failing|conflict' "${LOG}" \
      | tail -n "${KEYN}" || true
    echo "── 日志尾部 ${TAILN} 行 ──"
    tail -n "${TAILN}" "${LOG}" || true
  } | ESC | awk 'BEGIN{ORS=""} {printf "%s%s", (NR>1 ? "%0A" : ""), $0}'
)"

# 单条注解承载全部要点（GitHub 注解 message 上限远大于此）
echo "::error::${PREFIX} 失败 · 要点合一条（原日志见 job 日志，本仓日志需管理员权限）%0A${body}"
exit 0
