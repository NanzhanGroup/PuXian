#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# recheck_loadsensitive.sh —— M411 S2：分片失败门的「同轮低负载复跑」分类
#
# 要解决的问题（实测代价）：
#   M410 r1 一轮 9 红里 2 门是负载敏感（m128_unlock_grow 分片 pass=90 fail=4，
#   22:04 quiet 车道复跑 pass=94 fail=0；m324_tls_failclosed 负控 build 期链接失败）。
#   旧口径：分片一红就判整轮 RED ⇒ 100 分钟的链白跑，还要人工逐门定性。
#
# 本脚本只做一件事：把「分片/车道里失败的门」在**同一 HEAD、同一代码来源**下
#   低负载串行复跑一次，给出三分类：
#     复跑绿   ⇒ LOAD-SENSITIVE（不进产品红，但必须显式计数并打印门名单）
#     复跑仍红 ⇒ 产品红（照旧判整轮 RED）
#     超预算   ⇒ 未复跑（保持红，打印 NOT-RECHECKED）
#
# 防自欺（对应用户要求的判据纪律）：
#   ① 复跑前必须自证「被测物没变」：worktree HEAD == 本轮 head 且工作树干净；
#      任一不成立 ⇒ 该门不复跑，保持红（不许拿另一份代码的绿去豁免红）。
#   ② 门必须**真跑过**：判绿要求 rc=0 **且** times.tsv 里有该门且状态 OK；
#      只 rc=0 不算（run_gates.sh 对不存在门名会 rc=0 + 跳过）。
#   ③ LOAD-SENSITIVE 计数与门名单必须打印；候选数 0 也要打印（证明判据在跑）。
#   ④ 同一门**连续两轮**都 LOAD-SENSITIVE ⇒ 升级为红（ESCALATED），
#      不许长期靠复跑掩盖「门没声明自己的负载前提」。
#
# 用法：
#   recheck_loadsensitive.sh --log <链日志目录> [--repo <仓库根>] [--wt <复跑 worktree>]
#                            [--max N] [--budget 秒] [--history TSV]
# 输出文件（都落在 --log 目录）：
#   recheck-candidates.txt   候选门（去重）
#   loadsens-exempt.txt      本轮可豁免的产品红（复跑绿 · 无升级）
#   loadsens.txt             全部复跑绿的门（含被升级的，供台账）
#   recheck-red.txt          复跑仍红 / 未复跑
#   recheck/<gate>.{log,times.tsv}
#   recheck-summary.txt      key=value 汇总（供上游读取）
# 退出码：0 = 无候选或全部复跑完成（结论看文件）· 2 = 环境错（不许当绿）
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail

LOG=""; REPO=""; WT=""; MAX=8; BUDGET=1500
RECHECK_RUNNER="${PXC_RECHECK_RUNNER:-}"   # 自测注入 stub；生产留空 ⇒ 用 $WT/selfhost/run_gates.sh
HIST="${PXC_LOADSENS_HIST:-/data/autodrive/loadsens-history.tsv}"
GATEVM="${PX_GATEVM:-/data/gatevm}"

while [ $# -gt 0 ]; do
    case "$1" in
        --log)     LOG="${2:-}"; shift 2 ;;
        --repo)    REPO="${2:-}"; shift 2 ;;
        --wt)      WT="${2:-}"; shift 2 ;;
        --max)     MAX="${2:-}"; shift 2 ;;
        --budget)  BUDGET="${2:-}"; shift 2 ;;
        --history) HIST="${2:-}"; shift 2 ;;
        --selftest) SELFTEST=1; shift ;;
        -h|--help) sed -n '2,40p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done
SELFTEST="${SELFTEST:-0}"
SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
SELF_P="$SELF_DIR/$(basename "${BASH_SOURCE[0]:-$0}")"

# ── --selftest：1 正控 + 4 负控 + 前提自证（全离线，stub runner，不跑真门）──────────
#   判据纪律：负控必须能红（否则「豁免」就是个恒真的自欺）；每个负控要能自证扰动发生。
if [ "$SELFTEST" = 1 ]; then
  T="$(mktemp -d)"; trap 'rm -rf "$T"' EXIT
  N=0; F=0
  ok() { N=$((N+1)); echo "  ✅ $*"; }
  bad() { N=$((N+1)); F=$((F+1)); echo "  ❌ $*"; }
  mkrepo() {  # mkrepo <dir> → 干净 git 仓库 + stub runner（stub 必须在 commit 之前写好，否则树脏）
    mkdir -p "$1/selfhost"
    printf 'x\n' > "$1/selfhost/marker"
    cat > "$1/stub_runner.sh" <<'STUB'
#!/usr/bin/env bash
g=""; while [ $# -gt 0 ]; do case "$1" in --only) g="$2"; shift 2 ;; *) shift ;; esac; done
st="${STUB_STATUS:-OK}"
printf '%s\t%s\t1\t-\n' "$g" "$st" >> "${GATE_TSV:-/dev/null}"
[ "$st" = OK ]
STUB
    chmod +x "$1/stub_runner.sh"
    ( cd "$1" && git init -q \
        && git -c user.email=a@b -c user.name=t add -A >/dev/null 2>&1 \
        && git -c user.email=a@b -c user.name=t commit -qm x >/dev/null 2>&1 )
  }
  mklog() { mkdir -p "$1"; printf '%s\n' "${2:-❌ gate_one（12s）}" > "$1/shard1.log"; }
  runrc() {  # runrc <log> <repo> <hist> [wt] → 打印 rc
    local _wt="${4:-$2}"
    PXC_RECHECK_RUNNER="$2/stub_runner.sh" bash "$SELF_P" --log "$1" --repo "$2" --wt "$_wt" \
      --history "$3" --budget 60 > "$1.out" 2>&1
    echo $?
  }

  # S1 正控：复跑绿 ⇒ 豁免
  mkrepo "$T/r1"; mklog "$T/l1"
  _rc="$(runrc "$T/l1" "$T/r1" "$T/h1.tsv")"
  [ "$_rc" = 0 ] && ok "S1 rc=0" || bad "S1 rc=$_rc（应 0）"
  grep -qx 'gate_one' "$T/l1/loadsens-exempt.txt" 2>/dev/null && ok "S1 正控：复跑绿 ⇒ 进豁免名单" || bad "S1 没豁免"
  grep -q 'candidates=1' "$T/l1/recheck-summary.txt" 2>/dev/null && ok "S1 前提自证：候选真收集到 1 门" || bad "S1 候选数不对（扰动没发生）"
  grep -q 'LOAD-SENSITIVE n=1' "$T/l1.out" 2>/dev/null && ok "S1 计数与名单被打印（不静默）" || bad "S1 静默了计数"

  # S2 负控：复跑仍红 ⇒ 不许豁免
  mkrepo "$T/r2"; mklog "$T/l2"
  export STUB_STATUS=FAIL
  _rc="$(runrc "$T/l2" "$T/r2" "$T/h2.tsv")"
  unset STUB_STATUS
  [ -s "$T/l2/loadsens-exempt.txt" ] && bad "S2 负控失效：复跑仍红却被豁免" || ok "S2 负控：复跑仍红 ⇒ 不豁免"
  grep -q 'still_red=1' "$T/l2/recheck-summary.txt" 2>/dev/null && ok "S2 仍红门被计数（still_red=1）" || bad "S2 没记仍红"

  # S3 负控（前提）：worktree HEAD ≠ 本轮 HEAD ⇒ 一律不豁免
  mkrepo "$T/r3"; mklog "$T/l3"
  ( cd "$T/r3" && printf 'y\n' > other && git -c user.email=a@b -c user.name=t add -A >/dev/null 2>&1 \
      && git -c user.email=a@b -c user.name=t commit -qm y >/dev/null 2>&1 \
      && git worktree add -q --detach "$T/r3wt" HEAD~1 >/dev/null 2>&1 )
  _rc="$(runrc "$T/l3" "$T/r3" "$T/h3.tsv" "$T/r3wt")"
  [ -s "$T/l3/loadsens-exempt.txt" ] && bad "S3 负控失效：HEAD 不一致仍豁免（换代码豁免）" || ok "S3 负控：HEAD 不一致 ⇒ 不豁免"
  grep -q '被测物自证失败' "$T/l3.out" 2>/dev/null && ok "S3 报出了自证失败原因" || bad "S3 没报原因"

  # S4 负控（升级）：上一轮同门 LOAD-SENSITIVE ⇒ 本轮复跑绿也不豁免
  mkrepo "$T/r4"; mklog "$T/l4"
  printf 'chain_ts\tgate\tverdict\n20260101-000000\tgate_one\tLOAD-SENSITIVE\n' > "$T/h4.tsv"
  _rc="$(runrc "$T/l4" "$T/r4" "$T/h4.tsv")"
  [ -s "$T/l4/loadsens-exempt.txt" ] && bad "S4 负控失效：连续两轮仍豁免（允许长期掩盖）" || ok "S4 负控：连续两轮 ⇒ 升级为红"
  grep -q 'escalated=1' "$T/l4/recheck-summary.txt" 2>/dev/null && ok "S4 升级计数 =1" || bad "S4 升级没计数"

  # S5 无候选：日志里没有失败门 ⇒ 不进入复跑，且要打印候选 0（判据在跑）
  mkrepo "$T/r5"; mklog "$T/l5" "✅ gate_one（3s）"
  _rc="$(runrc "$T/l5" "$T/r5" "$T/h5.tsv")"
  [ "$_rc" = 0 ] && grep -q 'candidates=0' "$T/l5/recheck-summary.txt" 2>/dev/null \
    && ok "S5 无候选 ⇒ candidates=0 且 rc=0" || bad "S5 无候选处理不对"

  echo "── 自测：$((N-F))/$N 通过 ──"
  [ "$F" = 0 ] || exit 1
  exit 0
fi

[ -n "$LOG" ] && [ -d "$LOG" ] || { echo "❌ --log 目录不存在：${LOG:-<空>}" >&2; exit 2; }
REPO="${REPO:-$(cd "$SELF_DIR/../.." && pwd)}"
WT="${WT:-$GATEVM/wt1}"
RUNNER="${PXC_RECHECK_RUNNER:-$WT/selfhost/run_gates.sh}"
[ -f "$RUNNER" ] || { echo "❌ 找不到门运行器：$RUNNER" >&2; exit 2; }

RC_DIR="$LOG/recheck"
mkdir -p "$RC_DIR" || { echo "❌ 无法创建 $RC_DIR" >&2; exit 2; }
CAND="$LOG/recheck-candidates.txt"
EXEMPT="$LOG/loadsens-exempt.txt"
LSALL="$LOG/loadsens.txt"
RCRED="$LOG/recheck-red.txt"
SUMF="$LOG/recheck-summary.txt"
: > "$EXEMPT"; : > "$LSALL"; : > "$RCRED"

say() { echo "$*" | tee -a "$LOG/recheck.log"; }

# ── 1) 收集失败门（行格式「❌ name（…」/「⏱ name（…」；只认行首）─────────
{
  for f in "$LOG"/shard*.log "$LOG"/quiet-lane*.log "$LOG"/quiet.log; do
    [ -f "$f" ] || continue
    grep -E '^(❌|⏱)[[:space:]]+[a-zA-Z0-9_]+（' "$f" 2>/dev/null \
      | sed -E 's/^(❌|⏱)[[:space:]]+([a-zA-Z0-9_]+)（.*/\2/'
  done
} | sort -u > "$CAND" 2>/dev/null || true

# 扫描面（M410 R1/R2 口径：判绿必须打印扫描面，且扫描面要带零值守卫）
nscan=$(grep -c . "$CAND" 2>/dev/null) || nscan=0
case "$nscan" in ''|*[!0-9]*) nscan=0 ;; esac

say "═══ M411 复跑分类：nscan=$nscan（候选失败门，来源 $LOG）═══"
if [ "$nscan" -eq 0 ]; then
  say "RECHECK: 无失败门 ⇒ 不进入复跑（候选数 0 已打印，判据在跑）"
  {
    echo "candidates=0"; echo "loadsensitive=0"; echo "exempt=0"; echo "still_red=0"
    echo "skipped_budget=0"; echo "escalated=0"; echo "head=$(git -C "$REPO" rev-parse --short HEAD 2>/dev/null)"
  } > "$SUMF"
  exit 0
fi

# ── 2) 被测物自证：worktree 必须存在、HEAD 与本轮一致、工作树干净 ──────────
HEAD_FULL="$(git -C "$REPO" rev-parse HEAD 2>/dev/null || echo '')"
HEAD7="$(printf '%s' "$HEAD_FULL" | cut -c1-7)"
WT_HEAD="$(git -C "$WT" rev-parse HEAD 2>/dev/null || echo '')"
WT_OK=1
if [ -z "$HEAD_FULL" ] || [ -z "$WT_HEAD" ]; then
  WT_OK=0; say "⚠️ 被测物自证失败：REPO HEAD=${HEAD_FULL:-无} WT HEAD=${WT_HEAD:-无（$WT）}"
elif [ "$HEAD_FULL" != "$WT_HEAD" ]; then
  WT_OK=0; say "⚠️ 被测物自证失败：WT($(printf '%s' "$WT_HEAD" | cut -c1-7)) ≠ HEAD($HEAD7) ⇒ 不许换代码豁免红"
fi
if [ "$WT_OK" = 1 ] && [ -n "$(git -C "$WT" status --porcelain 2>/dev/null)" ]; then
  WT_OK=0; say "⚠️ 被测物自证失败：worktree 脏（复跑测的会是另一份代码）"
fi
if [ "$WT_OK" = 1 ]; then
  # 恢复被负控改过的受管目录（与 shard_run 的 quiet 档同一手法），再确认干净
  git -C "$WT" checkout -q -- selfhost runtime tools 2>/dev/null || true
  if [ -n "$(git -C "$WT" status --porcelain 2>/dev/null)" ]; then
    WT_OK=0; say "⚠️ 被测物自证失败：checkout 恢复后 worktree 仍脏"
  fi
fi
[ "$WT_OK" = 1 ] && say "被测物自证：WT HEAD=$HEAD7 工作树干净 ⇒ 复跑有效"

# ── 3) 上一轮台账（判断「连续两轮 LOAD-SENSITIVE」）──────────────────────
PREV_TS="$(awk -F'\t' 'NR>1{print $1}' "$HIST" 2>/dev/null | sort -u | tail -1)"
PREV_LS="$(awk -F'\t' -v t="${PREV_TS:-}" 'NR>1 && $1==t && $3=="LOAD-SENSITIVE"{print $2}' "$HIST" 2>/dev/null | sort -u)"
TS="$(date +%Y%m%d-%H%M%S)"
say "台账：上一轮 ts=${PREV_TS:-无} · 上轮 LOAD-SENSITIVE 门=[$(printf '%s' "$PREV_LS" | tr '\n' ' ')]"

# ── 4) 逐门复跑（串行 · 低负载）────────────────────────────────────────
NLS=0; NEX=0; NRED=0; NSKIP=0; NESC=0; T0=$SECONDS; NLOOP=0
for g in $(cat "$CAND"); do
  NLOOP=$((NLOOP+1))
  if [ "$NLOOP" -gt "$MAX" ]; then
    echo "$g" >> "$RCRED"; NSKIP=$((NSKIP+1))
    say "  ⏭ $g：超过 --max=$MAX ⇒ 未复跑（保持红）"
    continue
  fi
  if [ $((SECONDS-T0)) -ge "$BUDGET" ]; then
    echo "$g" >> "$RCRED"; NSKIP=$((NSKIP+1))
    say "  ⏭ $g：超预算 ${BUDGET}s ⇒ 未复跑（保持红）"
    continue
  fi
  if [ "$WT_OK" != 1 ]; then
    echo "$g" >> "$RCRED"; NRED=$((NRED+1))
    say "  ❌ $g：被测物自证不成立 ⇒ 必须按红（不复跑）"
    continue
  fi

  _tsv="$RC_DIR/$g.times.tsv"
  _log="$RC_DIR/$g.log"
  rm -f "$_tsv"
  _t0=$SECONDS
  ( cd "$WT" && GATE_TSV="$_tsv" GATE_TIMEOUT_DEFAULT=900 GATE_LOG_DIR_HINT="$RC_DIR/$g" \
      bash "$RUNNER" --only "$g" ) > "$_log" 2>&1
  _rc=$?
  _sec=$((SECONDS-_t0))
  _st="$(awk -F'\t' -v g="$g" '$1==g{print $2; exit}' "$_tsv" 2>/dev/null || echo '')"
  case "$_st" in ''|*[!A-Za-z]*) _st="${_st:-NONE}" ;; esac

  if [ "$_rc" = 0 ] && [ "$_st" = "OK" ]; then
    NLS=$((NLS+1))
    echo "$g" >> "$LSALL"
    # 连续两轮 ⇒ 升级（不许长期靠复跑掩盖）
    if grep -qxF "$g" <<< "$PREV_LS"; then
      NESC=$((NESC+1))
      echo "$g" >> "$RCRED"
      say "  ⚠️ $g：低负载复跑绿（${_sec}s）但**上一轮也是 LOAD-SENSITIVE** ⇒ ESCALATED（仍判红）"
    else
      echo "$g" >> "$EXEMPT"; NEX=$((NEX+1))
      say "  ✅ $g：低负载复跑绿（${_sec}s）⇒ LOAD-SENSITIVE（本轮豁免）"
    fi
  else
    NRED=$((NRED+1)); echo "$g" >> "$RCRED"
    say "  ❌ $g：复跑仍红（rc=$_rc 状态=$_st ${_sec}s）⇒ 产品红"
  fi
  # 每门复跑后恢复受管目录，避免上一门的负控痕迹影响下一门
  git -C "$WT" checkout -q -- selfhost runtime tools 2>/dev/null || true
done

# ── 5) 汇总（计数与门名单都必须出现在链日志里：静默吞 = 自欺）──────────────
NLS=$(grep -c . "$LSALL" 2>/dev/null) || NLS=0
NEX=$(grep -c . "$EXEMPT" 2>/dev/null) || NEX=0
NRED=$(grep -c . "$RCRED" 2>/dev/null) || NRED=0
case "$NLS" in ''|*[!0-9]*) NLS=0 ;; esac
case "$NEX" in ''|*[!0-9]*) NEX=0 ;; esac
case "$NRED" in ''|*[!0-9]*) NRED=0 ;; esac

say "LOAD-SENSITIVE n=$NLS 门=$(tr '\n' ',' < "$LSALL" | sed 's/,$//')"
say "RECHECK-EXEMPT n=$NEX 门=$(tr '\n' ',' < "$EXEMPT" | sed 's/,$//')"
say "RECHECK-RED n=$NRED 门=$(tr '\n' ',' < "$RCRED" | sed 's/,$//')"
say "RECHECK-ESCALATED n=$NESC（连续两轮 LOAD-SENSITIVE ⇒ 不豁免）"
say "RECHECK-SUMMARY nscan=$nscan 复跑绿=$NLS 豁免=$NEX 仍红=$NRED 超预算或超上限=$NSKIP 用时=$((SECONDS-T0))s"

{
  echo "nscan=$nscan"; echo "candidates=$nscan"; echo "loadsensitive=$NLS"; echo "exempt=$NEX"
  echo "still_red=$NRED"; echo "skipped_budget=$NSKIP"; echo "escalated=$NESC"
  echo "head=$HEAD7"; echo "wt_selfproof=$WT_OK"
} > "$SUMF"

# 台账：每门一行（chain_ts · gate · verdict）
if [ "$NLS" -gt 0 ]; then
  [ -f "$HIST" ] || printf 'chain_ts\tgate\tverdict\n' > "$HIST"
  for g in $(cat "$LSALL"); do printf '%s\t%s\t%s\n' "$TS" "$g" "LOAD-SENSITIVE" >> "$HIST"; done
fi
exit 0
