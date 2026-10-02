#!/usr/bin/env bash
# ============================================================
# selfhost/run_gates.sh —— 「本地全量门」运行器（M243 建立）
# ------------------------------------------------------------
# 门的**清单**在 `selfhost/gates.registry.sh`（单一注册源）；本文件只管**机制**：
#   · PID 锁（同一时刻只允许一个全量门）—— M191 事故后立的护栏
#   · 进出脏树检查（门内负控的 snapshot/restore 会**盖掉**未提交改动）—— M191
#   · NEGCTL 残留预检 —— 缺陷 139
#   · 逐门 `timeout -k 20`（目标忽略 SIGTERM 时裸 timeout 会一直等 —— M207 实测）
#     + 逐门计时 TSV（「哪扇门贵」从此是实测数据 —— M221）
#   · 汇总 + devbuild 复用台账（M222 补）
#   · **M243 新增**：--only / --skip / --list / --fail-fast（迭代档）
#
# 用法：
#   selfhost/run_gates.sh                     # 全量（发布档）
#   selfhost/run_gates.sh --only m242,m235    # 只跑指定门（迭代档：改完立刻验回归）
#   selfhost/run_gates.sh --skip m227         # 排除指定门
#   selfhost/run_gates.sh --list              # 只列门名（供清单↔实现一致性校验）
#   selfhost/run_gates.sh --fail-fast         # 首败即停（写错了不必等满全程）
#   selfhost/run_gates.sh --allow-dirty       # 接受脏树（危险，仅排障）
#
# 环境变量：GATE_TSV（计时输出）· GATE_TIMEOUT_DEFAULT · GATE_TIMEOUT_<门名>
#           GATE_ONLY / GATE_SKIP / GATE_FAIL_FAST（与同名参数等价，便于 CI）
# ============================================================
set -uo pipefail
cd "$(cd "$(dirname "$0")/.." && pwd)"
export LC_ALL=C LANG=C
FAIL=0
SKIPPED=0
REG="${GATE_REGISTRY:-selfhost/gates.registry.sh}"

# ── M243：参数解析（**必须在 PID 锁之前** —— `--list` 不执行任何门，
#    既不需要锁、也不该被脏树检查挡住）────────────────────────────
GATE_ONLY="${GATE_ONLY:-}"
GATE_SKIP="${GATE_SKIP:-}"
GATE_LIST=0
GATE_FAIL_FAST="${GATE_FAIL_FAST:-0}"
ALLOW_DIRTY=0
while [ $# -gt 0 ]; do
    case "$1" in
        --allow-dirty) ALLOW_DIRTY=1 ;;
        --only)        shift; GATE_ONLY="${1:-}" ;;
        --skip)        shift; GATE_SKIP="${1:-}" ;;
        --list)        GATE_LIST=1 ;;
        --fail-fast)   GATE_FAIL_FAST=1 ;;
        -h|--help)     sed -n '2,26p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数: $1（--help 看用法）"; exit 2 ;;
    esac
    shift
done

# 「,a,b,」包含判定（门名的逗号分隔表）
_in_list() { case ",$1," in *",$2,"*) return 0 ;; *) return 1 ;; esac; }

step() { [ "$GATE_LIST" = 1 ] && return 0; echo ""; echo "══ $* ══"; }

# ── M221（第 100 轮）：逐门计时 + 每门独立 timeout ──────────────
#   为什么：CI「工具自测」步实测 23/40+/37 分钟（上限 45），而**整步只有一个 timeout**、
#   507 行里 `timeout` 仅出现 2 次 ⇒ 一个门挂住就吃掉全部预算，其后的门**一次都不跑**；
#   诊断器的判据是「最后写入的日志 = 失败门」⇒ 挂住的门看起来像"没跑"，**根因被误导**。
#   ⇒ ① 每门独立 `timeout -k 20`（默认 $GATE_TIMEOUT_DEFAULT，可用 GATE_TIMEOUT_<门名> 覆盖）；
#      ② 逐门耗时写入 TSV（GATE_TSV）⇒ 「哪扇门贵」从此是实测数据，不再是猜。
#      ⚠️ `-k 20` 必须有：目标忽略 SIGTERM 时（如 px_serve 的优雅关闭）裸 timeout 会一直等
#         （M207 实测 s2c_pxserve 卡死 3 分钟）。
GATE_TSV="${GATE_TSV:-/tmp/m116_gates.times.tsv}"
GATE_TIMEOUT_DEFAULT="${GATE_TIMEOUT_DEFAULT:-900}"
: > "$GATE_TSV"
GATE_ALL0=$SECONDS

run() {  # $1=名 $2..=命令
    local name="$1"; shift
    # ── M243：--list 只报门名（check_gate_registry.sh 靠它做清单↔实现对拍）──
    if [ "$GATE_LIST" = 1 ]; then printf '%s\n' "$name"; return 0; fi
    # ── M243：门选择（迭代档）—— 过滤发生在 run() 里 ⇒ **清单仍是纯 bash 片段**，
    #    无需解析任何自定义格式（零格式转换风险，这是刻意的设计选择）──
    if [ -n "$GATE_ONLY" ] && ! _in_list "$GATE_ONLY" "$name"; then
        printf '%s\tSKIP\t0\t-\n' "$name" >> "$GATE_TSV"; SKIPPED=$((SKIPPED+1)); return 0
    fi
    if [ -n "$GATE_SKIP" ] && _in_list "$GATE_SKIP" "$name"; then
        printf '%s\tSKIP\t0\t-\n' "$name" >> "$GATE_TSV"; SKIPPED=$((SKIPPED+1)); return 0
    fi
    local t0=$SECONDS
    local tmo_var="GATE_TIMEOUT_${name}"
    local tmo="${!tmo_var:-$GATE_TIMEOUT_DEFAULT}"
    if timeout -k 20 "$tmo" "$@" > "/tmp/gate_$name.log" 2>&1; then
        local el=$((SECONDS-t0))
        printf '%s\tOK\t%d\t-\n' "$name" "$el" >> "$GATE_TSV"
        echo "✅ $name（${el}s）"
    else
        local rc=$?; local el=$((SECONDS-t0))
        printf '%s\tFAIL\t%d\t%d\n' "$name" "$el" "$rc" >> "$GATE_TSV"
        if [ "$rc" = 124 ] || [ "$rc" = 137 ]; then
            echo "⏱  $name（**超时**：${el}s ≥ 上限 ${tmo}s，rc=$rc）"
        else
            echo "❌ $name（rc=$rc, ${el}s）"
        fi
        tail -12 "/tmp/gate_$name.log" | sed 's/^/     /'; FAIL=$((FAIL+1))
        # ── M243：快速失败（迭代期 —— 写错了不必等满全程）──
        if [ "$GATE_FAIL_FAST" = 1 ]; then
            echo ""
            echo "⛔ --fail-fast：首败即停（已过 $((SECONDS-GATE_ALL0))s · **其余门未跑**）"
            gate_summary
            exit 1
        fi
    fi
}

# ── 汇总（M243 抽成函数：`--fail-fast` 提前退出时也要打）──
gate_summary() {
    echo ""
    if [ "$SKIPPED" -gt 0 ]; then
        echo "══ 汇总：失败 $FAIL 项 · 跳过 $SKIPPED 门（总耗时 $((SECONDS-GATE_ALL0))s）══"
    else
        echo "══ 汇总：失败 $FAIL 项（总耗时 $((SECONDS-GATE_ALL0))s）══"
    fi
    echo "   逐门时序（name<TAB>状态<TAB>秒<TAB>rc）：$GATE_TSV"
    # M222 补（第 100 轮）：**devbuild 复用台账** —— 产物指纹短路到底省了多少？
    #   台账由 devbuild.sh 追加（DEVB_STATS 可覆盖），这里只汇总，**不参与判据**。
    if [ -f "${DEVB_STATS:-/tmp/devbuild_stats.tsv}" ]; then
        DS="${DEVB_STATS:-/tmp/devbuild_stats.tsv}"
        DSB=$(grep -c $'\trebuild\t' "$DS" 2>/dev/null || true); DSB=${DSB:-0}
        DSR=$(grep -c $'\treuse\t'   "$DS" 2>/dev/null || true); DSR=${DSR:-0}
        echo "   devbuild 台账：重建 ${DSB} 次 / 复用 ${DSR} 次（$DS）"
    fi
}

# ── `--list`：短路（不执行门 ⇒ 不需锁、不查脏树）──
if [ "$GATE_LIST" = 1 ]; then
    [ -f "$REG" ] || { echo "❌ 注册源不存在：$REG"; exit 3; }
    source "$REG"
    exit 0
fi

LOCK=/tmp/.m116_gates.lock
if [ -f "$LOCK" ] && kill -0 "$(cat "$LOCK" 2>/dev/null)" 2>/dev/null; then
    echo "❌ 已有 m116 全门在跑（PID $(cat "$LOCK")）—— 并发跑会互相踩掉负控还原，拒绝启动"
    exit 1
fi
echo $$ > "$LOCK"
# ⚠️ M243：`ALLOW_DIRTY` 由上面的参数解析**唯一**决定 ——
#   此处**不得**再赋值/覆盖（M209/M214 撞过的「模块级默认值覆盖 CLI」老坑，第三次）。
DIRTY0="$(git status --porcelain -- selfhost runtime tools 2>/dev/null)"
if [ -n "$DIRTY0" ] && [ "$ALLOW_DIRTY" = 0 ]; then
    echo "❌ 工作树不干净（selfhost/ runtime/ tools/ 有未提交改动）"
    echo "$DIRTY0" | sed 's/^/     /'
    echo "   ⇒ 门内负控的 snapshot/restore 会**盖掉**这些改动：先提交/stash，或用 --allow-dirty 明确接受风险"
    rm -f "$LOCK"; exit 1
fi
trap 'rm -f "$LOCK"' EXIT


# ── 执行：门的清单全部来自注册源（单一权威名单）──
[ -f "$REG" ] || { echo "❌ 注册源不存在：$REG"; exit 3; }
source "$REG"

# ── 收尾 ──
DIRTY1="$(git status --porcelain -- selfhost runtime tools 2>/dev/null)"
if [ "$DIRTY1" != "$DIRTY0" ]; then
    echo ""
    echo "⚠️ 门有副作用：selfhost//runtime//tools/ 的工作树状态与进门时不同（负控还原不彻底？）"
    diff <(echo "$DIRTY0") <(echo "$DIRTY1") | sed 's/^/     /' || true
    FAIL=$((FAIL+1))
fi

gate_summary
exit $((FAIL > 0))
