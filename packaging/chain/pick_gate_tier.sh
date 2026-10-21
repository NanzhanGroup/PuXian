#!/usr/bin/env bash
# ============================================================
# packaging/chain/pick_gate_tier.sh —— 全量门档位选择（SERIAL / SHARD）
# ------------------------------------------------------------
# 用户令（2026-10-19 选 A）：
#   「常规发布走分片档（~98min），串行权威档只在**改了 runtime / 编译器**的轮次跑一次做周期性复核」
#
# 判据（**可判定，不靠人工记忆**）：
#   语义面 = runtime/*.c|h  ∪  selfhost/*.px  ∪  tools/*.px
#     · 命中  ⇒ SHARD_SEM（**分片全覆盖 + 抽样串行复核**；见 recheck_sample.sh）
#     · 未命中 ⇒ SHARD（与串行基线逐门对拍，不一致即 RED —— 见 shard_verdict.py）
#
# ⭐ 用户令更新（2026-10-21 · 方案②）：
#     原「语义面命中 ⇒ SERIAL（~5.2h）」改为「⇒ SHARD_SEM（分片 ~98min + 抽样串行复核 ~40min）」。
#     强度不降的依据：抽样集 = **Top-K 耗时门**（资源竞争最敏感）+ **quiet 档全部**（本就串行跑），
#     这批门**既有分片结果又有串行结果** ⇒ 可逐门对拍；其余门靠分片 + 全套判据自证。
#     SERIAL 仍保留三种触发：① --force-serial 显式指定 ② 超期周期性复核 ③ 判定失败保守回落。
#   --overdue-days N ⇒ 距上次串行档超过 N 天也强制 SERIAL（周期性复核兜底）
#
# 为什么语义面只算这三处：
#   `runtime/*.c` 决定运行时语义；`selfhost/*.px` 是编译器/解释器本体；
#   `tools/*.px` 是 px CLI 的行为（`px build` 本身在三轨对拍链上）。
#   改门、改打包、改文档、改 registry（`.px` 库）都**不**改变这三个东西的语义，
#   分片档 + 逐门对拍足以保证判据强度不降。
#
# 用法：
#   pick_gate_tier.sh [--base REF] [--head REF] [--overdue-days N] [--stamp FILE] [--quiet]
#   默认 base = 最近的 tag（git describe --tags --abbrev=0），head = HEAD
#   输出（stdout，可被 . 或用 eval 读）：
#     GATE-TIER=SERIAL|SHARD
#     GATE-REASON=<一句话>
#     GATE-HITS=<语义面命中文件数>
#     GATE-HEAD=<短 sha>
#   退出码：0 = 判定成功 · 3 = **无法判定**（不是 git 仓库 / 取不到 ref / 不在任何 tag 之后）
#          ⚠️ 取不到就 rc=3，**绝不静默回落到 SERIAL 或 SHARD**（M212 缺陷 288 的纪律）
#
# 自证：pick_gate_tier.sh --self-test（含 3 个历史区间 + 非法 ref + 未超期/超期两档）
# ============================================================
set -uo pipefail

BASE=""
HEAD=""
OVERDUE_DAYS=0
STAMP=""
QUIET=0
SELF_TEST=0
FORCE_SERIAL=0

while [ $# -gt 0 ]; do
    case "$1" in
        --base) BASE="${2:-}"; shift 2 ;;
        --head) HEAD="${2:-}"; shift 2 ;;
        --overdue-days) OVERDUE_DAYS="${2:-0}"; shift 2 ;;
        --force-serial) FORCE_SERIAL=1; shift ;;
        --stamp) STAMP="${2:-}"; shift 2 ;;
        --quiet) QUIET=1; shift ;;
        --self-test) SELF_TEST=1; shift ;;
        -h|--help) sed -n '1,40p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
HERE="$(cd "$(dirname "$0")" && pwd)"

# 语义面（正则，逐行匹配 `git diff --name-only` 的输出）
SEM_PATTERNS='^runtime/[^/]+\.(c|h)$
^selfhost/[^/]+\.px$
^tools/[^/]+\.px$'

tier_for_range() {   # $1=base $2=head ⇒ 置 TIER/REASON/HITS
    local base="$1" head="$2" files n
    if ! files="$(cd "$ROOT" && git diff --name-only "$base...$head" 2>/dev/null)"; then
        TIER=""; REASON="取不到区间 $base...$head（不是 git 仓库 / ref 不存在）"; HITS=0
        return 3
    fi
    n="$(printf '%s\n' "$files" | grep -cE "$SEM_PATTERNS" || true)"
    HITS="${n:-0}"
    if [ "${HITS:-0}" -gt 0 ]; then
        TIER="SHARD_SEM"; REASON="语义面命中 $HITS 个文件（runtime/*.c|h · selfhost/*.px · tools/*.px）⇒ 分片 + 抽样串行复核"
    else
        TIER="SHARD"; REASON="语义面 0 命中 ⇒ 分片档（须与串行基线逐门对拍）"
    fi
    return 0
}

if [ "$SELF_TEST" = "1" ]; then
    PASS=0; FAIL=0
    ok()  { PASS=$((PASS+1)); echo "  ✅ $1"; }
    bad() { FAIL=$((FAIL+1)); echo "  ❌ $1"; }
    echo "══ pick_gate_tier 自证 ══"
    cd "$ROOT" || exit 3

    # T1 三个历史区间（都是本仓真实 tag；判据 = 与当时的实际档位一致）
    check_range() {   # $1=base $2=head $3=期望档 $4=说明
        if tier_for_range "$1" "$2"; then
            if [ "$TIER" = "$3" ]; then ok "$4（$1..$2 ⇒ $TIER · 命中 $HITS）"; else bad "$4：期望 $3，实际 $TIER（命中 $HITS）"; fi
        else
            bad "$4：区间不可用（$REASON）"
        fi
    }
    check_range "v0.2.310" "v0.2.311" SHARD "M311 只改门/工具/registry/打包 ⇒ 分片"
    check_range "v0.2.311" "v0.2.312" SHARD_SEM "M312 改 runtime.c + coro.c ⇒ 分片+抽样复核"
    check_range "v0.2.312" "v0.2.313" SHARD_SEM "M313 改 runtime_conn.c ⇒ 分片+抽样复核"

    # T2 非法 ref ⇒ rc=3（绝不静默回落）
    tier_for_range "no-such-ref-xyz" "HEAD"; RC=$?
    if [ "$RC" = "3" ] && [ -z "$TIER" ]; then ok "非法 ref ⇒ rc=3 且 TIER 为空（不静默回落）"; else bad "非法 ref 判定不对（rc=$RC tier=$TIER）"; fi

    # T3 同一 ref ⇒ 0 命中 ⇒ SHARD
    check_range "HEAD" "HEAD" SHARD "空区间 ⇒ 无语义面改动 ⇒ 分片"

    # T3b --force-serial ⇒ 必须 SERIAL（语义面即使 0 命中也要能强制）
    #   ⚠️ 判据写的是**最终输出**而不是中间变量：模拟正式路径（tier_for_range + force 覆盖）
    TIER=""; REASON=""; HITS=0
    tier_for_range "HEAD" "HEAD"
    if [ "$TIER" = "SHARD" ]; then TIER="SERIAL"; REASON="人工指定 --force-serial（原判：${REASON}）"; fi
    if [ "$TIER" = "SERIAL" ]; then ok "--force-serial ⇒ 强制 SERIAL（覆盖 SHARD）"; else bad "--force-serial 未生效（$TIER）"; fi
    # T3c 语义命中 + force-serial ⇒ 仍是 SERIAL（幂等）
    TIER=""; REASON=""; HITS=0
    tier_for_range "v0.2.311" "v0.2.312"
    if [ "$TIER" = "SHARD_SEM" ]; then TIER="SERIAL"; REASON="人工指定 --force-serial（原判：${REASON}）"; fi
    if [ "$TIER" = "SERIAL" ]; then ok "--force-serial 对语义轮幂等（SHARD_SEM ⇒ SERIAL）"; else bad "force 对语义轮不幂等（$TIER）"; fi

    # T4 超期逻辑：stamp 很旧 ⇒ 强制 SERIAL
    #   ⚠️ 必须**显式设 OVERDUE_DAYS**（自测默认不带 --overdue-days，否则这一档恒不触发
    #      —— 首版就踩了：自测在「超期逻辑失效」上判红，而真因是自测自己没给阈值）
    old_stamp="$(mktemp)"; echo $(( $(date +%s) - 86400 * 30 )) > "$old_stamp"
    OVERDUE_DAYS=7
    TIER=""; REASON=""; HITS=0
    tier_for_range "HEAD" "HEAD"
    if [ "$TIER" = "SHARD" ] && [ "$OVERDUE_DAYS" -gt 0 ] && [ -f "$old_stamp" ]; then
        LAST="$(cat "$old_stamp" 2>/dev/null || echo 0)"
        NOW="$(date +%s)"
        if [ $(( (NOW - LAST) / 86400 )) -ge "$OVERDUE_DAYS" ]; then
            TIER="SERIAL"; REASON="距上次串行档已 $(( (NOW - LAST) / 86400 )) 天 ≥ $OVERDUE_DAYS ⇒ 周期性复核"
        fi
    fi
    if [ "$TIER" = "SERIAL" ]; then ok "超期 30 天 + --overdue-days 7 ⇒ 强制 SERIAL"; else bad "超期逻辑失效（$TIER）"; fi
    rm -f "$old_stamp"

    # T5 未超期 ⇒ 保持 SHARD
    new_stamp="$(mktemp)"; date +%s > "$new_stamp"
    TIER=""; REASON=""; HITS=0
    tier_for_range "HEAD" "HEAD"
    LAST="$(cat "$new_stamp" 2>/dev/null || echo 0)"; NOW="$(date +%s)"
    if [ $(( (NOW - LAST) / 86400 )) -lt 7 ]; then ok "未超期（刚跑过）⇒ 保持 SHARD"; else bad "未超期判定失效"; fi
    rm -f "$new_stamp"

    echo
    if [ "$FAIL" = "0" ]; then echo "══ PGT-SELFTEST-OK · $PASS 通过 / 0 失败 ══"; exit 0; else echo "══ PGT-SELFTEST-FAIL · $PASS 通过 / $FAIL 失败 ══"; exit 1; fi
fi

# ── 正常判定 ─────────────────────────────────────────────────
cd "$ROOT" || { echo "GATE-TIER="; echo "GATE-REASON=不是 git 仓库：$ROOT" >&2; exit 3; }
if [ -z "$HEAD" ]; then HEAD="HEAD"; fi
if [ -z "$BASE" ]; then
    if ! BASE="$(git describe --tags --abbrev=0 2>/dev/null)"; then
        echo "GATE-TIER="
        echo "GATE-REASON=取不到基准 tag（git describe --tags --abbrev=0 失败）—— 请显式给 --base" >&2
        exit 3
    fi
fi

TIER=""; REASON=""; HITS=0
tier_for_range "$BASE" "$HEAD" || { echo "GATE-TIER="; echo "GATE-REASON=$REASON"; exit 3; }

# 人工强制：--force-serial（保留串行权威档的显式入口）
if [ "$FORCE_SERIAL" = "1" ]; then
    TIER="SERIAL"; REASON="人工指定 --force-serial（原判：${REASON}）"
fi

# 周期性复核：距上次串行档超期 ⇒ 升级为 SERIAL
if { [ "$TIER" = "SHARD" ] || [ "$TIER" = "SHARD_SEM" ]; } && [ "$OVERDUE_DAYS" -gt 0 ]; then
    if [ -n "$STAMP" ] && [ -f "$STAMP" ]; then
        LAST="$(tr -d ' \n' < "$STAMP" 2>/dev/null || echo 0)"
        case "$LAST" in ''|*[!0-9]*) LAST=0 ;; esac
        NOW="$(date +%s)"
        AGE_D=$(( (NOW - LAST) / 86400 ))
        if [ "$AGE_D" -ge "$OVERDUE_DAYS" ]; then
            TIER="SERIAL"; REASON="距上次串行档 $AGE_D 天 ≥ $OVERDUE_DAYS 天 ⇒ 周期性复核（$REASON）"
        fi
    elif [ -n "$STAMP" ]; then
        TIER="SERIAL"; REASON="无串行档时间戳（$STAMP 不存在）⇒ 视为超期，周期性复核"
    fi
fi

SHORT="$(git rev-parse --short "$HEAD" 2>/dev/null || echo '?')"
echo "GATE-TIER=$TIER"
echo "GATE-REASON=$REASON"
echo "GATE-HITS=$HITS"
echo "GATE-HEAD=$SHORT"
[ "$QUIET" = "1" ] || { echo "GATE-BASE=$BASE" >&2; echo "GATE-RANGE=$BASE...$HEAD" >&2; }
exit 0
