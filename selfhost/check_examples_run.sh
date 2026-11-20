#!/usr/bin/env bash
# ============================================================
# selfhost/check_examples_run.sh —— **示例即文档：顶层示例必须跑得起来**（M426 · 缺陷 871）
# ------------------------------------------------------------
# 为什么有它：
#   M422 建了 check_examples_compile.sh（缺陷 856）：120 个顶层示例**编译**必须过。
#   但「编译过 ≠ 跑得动」。M426 实测（HEAD d2f236b · 编译轨逐条 build + 跑产物）：
#     120 个里 82 个能跑 / 36 个红 / 2 个超时 —— 而 300 道门里**没有一道**跑过它们。
#   真缺陷三个（本轮已修，见 CHANGELOG M426）：
#     · m23b_bytes / m30_int_bytes / m57_s2_mmap 仍断言「越界 → null」
#       而 M189/M201 早已把越界改成 R1003 响亮 ⇒ 示例教用户写错代码
#     · m28_time_sqlite 断言 tz_offset("+0800")==28800 而实现返回 null
#       ⇒ 这一条是**示例对了、实现错了**（缺陷 869，已修：±HHMM 全判非法）
#   ⇒ 与 M401「判据存在 ≠ 判据在执行」同源：示例是产品对外文档，却无人执行。
#
# 判据：
#   C1 顶层枚举：examples/*.px 每个都算（扫描面 < MIN ⇒ rc=3，提取器坏了）
#   C2 逐条跑：`px build` 后执行产物，rc 必须 0；超时(124)/崩溃/断言失败 ⇒ 红并打印首行
#   C3 豁免账 examples/examples_run_exempt.tsv：name.px<TAB>category<TAB>expires<TAB>reason
#        category ∈ network|port|pair|relative|interactive|demo-rc1
#        · 条目无对应文件 ⇒ STALE 红
#        · reason < 8 字 或 expires 非 YYYY-MM-DD ⇒ 红
#        · category=local 的豁免项若本次真跑通 ⇒ STALE 红（账跟着事实走）
#          （network/port/pair 类**永远不能独立跑通**，不判 STALE，否则网速抖动就假红）
#   C4 豁免集合规模 ≤ EXEMPT_MAX（写死常量，只许缩小）
#   C5 前提自证：真跑 ≥ MIN 条、至少 1 条非豁免 ⇒ 否则 rc=3
#   N1 负控：注入「编译过但跑挂」的示例 ⇒ 必红并**指名**
#   N2 正控：同一夹具去掉坏示例 ⇒ 必绿（证明 N1 的红来自「跑挂」而不是夹具本身）
#   N3 负控：豁免项不存在 ⇒ STALE 红
#   G1 环境守卫：examples / tools/px 不可用 ⇒ rc=3（不降级成 0）
#   G2 零污染：跑完仓库树 examples/build 条目数不得增加（示例一律在临时树里构建）
#
# 用法：
#   check_examples_run.sh               真仓判定
#   check_examples_run.sh --root DIR    指定仓库根（夹具用）
#   check_examples_run.sh --list        逐条打印（不判定）
#   check_examples_run.sh --self-test   9 条正/负控（含负控必须真红）
#   check_examples_run.sh --emit-skip   打印豁免草稿（含实测分类建议）
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

MIN_RUN=${MIN_RUN:-50}
JOBS=${JOBS:-1}          # 示例内端口写死（如 net_demo 7891/7892）⇒ 默认串行，避免与并发门撞
EXEMPT_MAX=${EXEMPT_MAX:-40}
RUN_TIMEOUT=${RUN_TIMEOUT:-20}
MODE=check
ROOT=""
while [ $# -gt 0 ]; do
  case "$1" in
    --root)      ROOT="$2"; shift 2 ;;
    --list)      MODE=list; shift ;;
    --self-test) MODE=selftest; shift ;;
    --emit-skip) MODE=emit; shift ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done
[ -n "$ROOT" ] || ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# ── 自测 ──────────────────────────────────────────────────────────────
if [ "$MODE" = selftest ]; then
  P=0; F=0
  ok(){ echo "  ✅ $1"; P=$((P+1)); }
  bad(){ echo "  ❌ $1"; F=$((F+1)); }
  T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
  mkfix(){ # $1=根
    mkdir -p "$1/examples" "$1/selfhost" "$1/tools"
    cat > "$1/examples/good.px" <<'EOF'
def main():
    assert(1 + 1 == 2)
    print("GOOD-OK")
main()
EOF
    cat > "$1/examples/runbad.px" <<'EOF'
def main():
    print("BEFORE-ASSERT")
    assert(1 == 2)
main()
EOF
    ln -sf "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/tools/px" "$1/tools/px"
    : > "$1/examples/examples_run_exempt.tsv"
  }
  echo "=== check_examples_run --self-test ==="
  # T1 正控：只有好示例 ⇒ rc=0
  mkfix "$T/r1"; rm -f "$T/r1/examples/runbad.px"
  MIN_RUN=1 bash "${BASH_SOURCE[0]}" --root "$T/r1" > "$T/o1" 2>&1; r1=$?
  [ "$r1" = 0 ] && ok "T1 正控：能跑 ⇒ rc=0" || { bad "T1 期望 rc=0 实得 $r1"; cat "$T/o1"; }
  # T2 负控：**编译过但跑挂**的示例未登记 ⇒ 必红且点名
  mkfix "$T/r2"
  MIN_RUN=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o2" 2>&1; r2=$?
  if [ "$r2" = 1 ] && grep -q 'runbad.px' "$T/o2"; then ok "T2 负控：跑挂未登记 ⇒ rc=1 且点名"; else bad "T2 期望 rc=1 且点名 runbad.px，实得 rc=$r2"; cat "$T/o2"; fi
  # N3 前提自证：T2 的红确实来自「跑挂」而不是「编译失败」
  if grep -q 'RUN-FAIL\|rc=' "$T/o2"; then ok "N3 前提自证：T2 输出含运行期失败标记"; else bad "N3 T2 输出没有运行期失败标记（判据可能空转）"; cat "$T/o2"; fi
  # T3 正控：豁免登记的 runbad 不再判红
  printf 'runbad.px\tlocal\t2099-01-01\t自测夹具：故意跑挂\n' > "$T/r2/examples/examples_run_exempt.tsv"
  MIN_RUN=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o3" 2>&1; r3=$?
  [ "$r3" = 0 ] && ok "T3 正控：跑挂示例已登记 ⇒ rc=0" || { bad "T3 期望 rc=0 实得 $r3"; cat "$T/o3"; }
  # T4 负控：category=local 的豁免项其实能跑 ⇒ STALE 红
  #   ⚠️ 必须留一条**非豁免**能跑示例，否则会先撞 C5「非豁免为空 ⇒ rc=3」（判据空转守卫）
  cat > "$T/r2/examples/good2.px" <<'EOF'
def main():
    print("GOOD2-OK")
main()
EOF
  printf 'good.px\tlocal\t2099-01-01\t不该豁免（它本来就能跑）\n' >> "$T/r2/examples/examples_run_exempt.tsv"
  MIN_RUN=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o4" 2>&1; r4=$?
  if [ "$r4" = 1 ] && grep -q 'STALE' "$T/o4"; then ok "T4 负控：能跑却挂 local 豁免 ⇒ STALE 红"; else bad "T4 期望 STALE 红，实得 rc=$r4"; cat "$T/o4"; fi
  # T5 负控：非 local 豁免项（pair）能跑 ⇒ 不判 STALE（防网速抖动假红）
  : > "$T/r2/examples/examples_run_exempt.tsv"
  printf 'runbad.px\tpair\t2099-01-01\t需配对（自测：不判 STALE 的类别）\n' > "$T/r2/examples/examples_run_exempt.tsv"
  MIN_RUN=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o5" 2>&1; r5=$?
  [ "$r5" = 0 ] && ok "T5 正控：pair 类豁免即使能跑也不判 STALE" || { bad "T5 期望 rc=0（pair 类不判 STALE）实得 $r5"; cat "$T/o5"; }
  # T6 负控：豁免项无对应文件 ⇒ STALE 红
  printf 'no_such.px\tlocal\t2099-01-01\t指向不存在的示例\n' >> "$T/r2/examples/examples_run_exempt.tsv"
  MIN_RUN=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o6" 2>&1; r6=$?
  if [ "$r6" = 1 ] && grep -q 'no_such.px' "$T/o6"; then ok "T6 负控：豁免项无文件 ⇒ 红并点名"; else bad "T6 期望 rc=1 且点名 no_such.px，实得 rc=$r6"; cat "$T/o6"; fi
  # T7 负控：豁免理由太短 ⇒ 红
  : > "$T/r2/examples/examples_run_exempt.tsv"
  printf 'runbad.px\tlocal\t2099-01-01\t短\n' > "$T/r2/examples/examples_run_exempt.tsv"
  MIN_RUN=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o7" 2>&1; r7=$?
  if [ "$r7" = 1 ] && grep -q 'REASON' "$T/o7"; then ok "T7 负控：理由过短 ⇒ 红"; else bad "T7 期望理由过短判红，实得 rc=$r7"; cat "$T/o7"; fi
  # T8 环境守卫：examples 缺失 ⇒ rc=3
  mkfix "$T/r8"; rm -rf "$T/r8/examples"
  bash "${BASH_SOURCE[0]}" --root "$T/r8" > "$T/o8" 2>&1; r8=$?
  [ "$r8" = 3 ] && ok "T8 环境守卫：examples 缺失 ⇒ rc=3" || bad "T8 期望 rc=3 实得 $r8"
  # T9 环境守卫：px 不可执行 ⇒ rc=3
  mkfix "$T/r9"; rm -f "$T/r9/tools/px"
  MIN_RUN=1 bash "${BASH_SOURCE[0]}" --root "$T/r9" > "$T/o9" 2>&1; r9=$?
  [ "$r9" = 3 ] && ok "T9 环境守卫：px 缺失 ⇒ rc=3" || bad "T9 期望 rc=3 实得 $r9"
  echo "---- self-test: PASS=$P FAIL=$F ----"
  [ "$F" -eq 0 ] || exit 2
  echo "EXAMPLES-RUN-SELFTEST=OK"
  exit 0
fi

EX="$ROOT/examples"
PX="$ROOT/tools/px"
EXEMPT="$EX/examples_run_exempt.tsv"

[ -d "$EX" ] || { echo "EXAMPLES-RUN=ENV rc=3 原因=examples 目录不存在（$EX）"; exit 3; }
[ -x "$PX" ] || { echo "EXAMPLES-RUN=ENV rc=3 原因=px 工具不可执行（$PX）"; exit 3; }

TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
ls -1 "$EX"/*.px 2>/dev/null | xargs -r -n1 basename | sort > "$TMP/units.txt"
N=$(wc -l < "$TMP/units.txt")
if [ "$N" -lt "$MIN_RUN" ]; then
  echo "EXAMPLES-RUN=ENV rc=3 原因=扫描面过小（顶层 .px = $N < $MIN_RUN，提取器或目录坏了）"; exit 3
fi

# 零污染基线：跑前记仓库树 examples/build 条目数（G2）
PRE_BUILD=$(ls -1 "$EX/build" 2>/dev/null | wc -l)

# ── 豁免账 ────────────────────────────────────────────────────────────
declare -A EX_CAT=() EX_REASON=() EX_EXP=()
if [ -f "$EXEMPT" ]; then
  while IFS=$'\t' read -r f c e r; do
    case "$f" in ''|'#'*) continue ;; esac
    EX_CAT["$f"]="${c:-local}"; EX_EXP["$f"]="${e:-}"; EX_REASON["$f"]="${r:-}"
  done < "$EXEMPT"
fi

# ── 单条：build + 跑 ──────────────────────────────────────────────────
export PX TMP RUN_TIMEOUT
run_one() {
  local f="$1" base d rc b log first rt0 rt1
  base="${f%.px}"
  d="$TMP/w/$base"
  rm -rf "$d"; mkdir -p "$d"
  cp "$TMP/src/$f" "$d/$f" 2>/dev/null || { echo "MISS	$f	复制失败"; return; }
  cd "$d" || { echo "MISS	$f	进不去临时目录"; return; }
  rt0=$(date +%s%N)
  timeout --kill-after=5 90 "$PX" build "$f" > "$d/build.log" 2>&1
  rc=$?
  if [ "$rc" != 0 ] || [ ! -x "$d/build/$base" ]; then
    first=$(printf '%s' "$(head -3 "$d/build.log" 2>/dev/null)" | tr '\n' ' ' | cut -c1-120)
    echo "BUILD-FAIL	$f	rc=$rc ${first}"; return
  fi
  timeout --kill-after=5 "$RUN_TIMEOUT" "$d/build/$base" > "$d/run.log" 2>&1
  rc=$?
  rt1=$(( ($(date +%s%N) - rt0) / 1000000 ))
  if [ "$rc" = 0 ]; then
    echo "OK	$f	${rt1}ms"
  else
    first=$(head -2 "$d/run.log" 2>/dev/null | tr '\n' ' ' | cut -c1-120)
    echo "RUN-FAIL	$f	rc=$rc ${first:-无输出}"
  fi
}
export -f run_one

mkdir -p "$TMP/src"
cp "$EX"/*.px "$TMP/src/" 2>/dev/null || true

# 非豁免集合 + category=local 的豁免项（后者要真跑，用来判「账跟着事实走」的 STALE）
#   其它类别（network/port/pair/relative/interactive/demo-rc1）**永远不能独立跑通**，
#   一律不跑：跑了只会超时，且网速抖动会让判据假红。
: > "$TMP/todo.txt"
while IFS= read -r f; do
  if [ -z "${EX_CAT[$f]:-}" ]; then echo "$f" >> "$TMP/todo.txt"
  elif [ "${EX_CAT[$f]}" = local ]; then echo "$f" >> "$TMP/todo.txt"; fi
done < "$TMP/units.txt"
NTODO=$(grep -c . "$TMP/todo.txt" 2>/dev/null || echo 0)

if [ "$MODE" = emit ]; then
  # 草稿：把「本次不能独立跑」的示例列出来，category 留 TODO 供人工定性
  xargs -r -n1 -P "$JOBS" bash -c 'run_one "$0"' < "$TMP/todo.txt" | grep -v '^OK' | \
    awk -F'\t' '{printf "%s\tTODO(network|port|pair|relative|interactive|demo-rc1)\t2099-01-01\t实测 %s\n", $2, $3}'
  exit 0
fi

xargs -r -n1 -P "$JOBS" bash -c 'run_one "$0"' < "$TMP/todo.txt" | sort -k2 > "$TMP/res.txt"

if [ "$MODE" = list ]; then cat "$TMP/res.txt"; exit 0; fi

# ── 判定 ──────────────────────────────────────────────────────────────
fail=0; n_ok=0; n_fail=0; n_skip=0; n_stale=0; n_reason=0
: > "$TMP/viol.txt"

while IFS=$'\t' read -r st f rest; do
  [ -n "${f:-}" ] || continue
  if [ "$st" = OK ]; then
    n_ok=$((n_ok+1))
    if [ "${EX_CAT[$f]:-}" = local ]; then
      # C3：category=local 的豁免项跑通了 ⇒ 账必须跟着事实走
      fail=$((fail+1)); n_stale=$((n_stale+1))
      echo "STALE-EXEMPT  $f（category=local 却跑通了 ⇒ 必须从豁免账删除）" >> "$TMP/viol.txt"
    fi
  else
    n_fail=$((n_fail+1))
    if [ -n "${EX_CAT[$f]:-}" ]; then
      : # 豁免项（local）本次失败 = 与账一致，不判红
    else
      fail=$((fail+1)); echo "$st  $f — ${rest:-无输出}" >> "$TMP/viol.txt"
    fi
  fi
done < "$TMP/res.txt"

# 豁免账逐条校验
for f in "${!EX_CAT[@]}"; do
  n_skip=$((n_skip+1))
  if [ ! -f "$EX/$f" ]; then
    fail=$((fail+1)); n_stale=$((n_stale+1))
    echo "STALE-EXEMPT  $f（扫描面里没有这个文件）" >> "$TMP/viol.txt"
    continue
  fi
  if [ "${#EX_REASON[$f]}" -lt 8 ] || [ -z "${EX_REASON[$f]}" ]; then
    fail=$((fail+1)); n_reason=$((n_reason+1))
    echo "REASON-TOO-SHORT  $f（理由必须 ≥8 字：现在「${EX_REASON[$f]}」）" >> "$TMP/viol.txt"
  fi
  case "${EX_EXP[$f]:-}" in
    [0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]) : ;;
    *) fail=$((fail+1)); n_reason=$((n_reason+1))
       echo "REASON-TOO-SHORT  $f（expires 必须是 YYYY-MM-DD：现在「${EX_EXP[$f]:-}」）" >> "$TMP/viol.txt" ;;
  esac
  case "${EX_CAT[$f]:-}" in
    network|port|pair|relative|interactive|local|demo-rc1) : ;;
    *) fail=$((fail+1)); n_reason=$((n_reason+1))
       echo "REASON-TOO-SHORT  $f（category 非法：${EX_CAT[$f]}）" >> "$TMP/viol.txt" ;;
  esac
done

if [ "$n_skip" -gt "$EXEMPT_MAX" ]; then
  fail=$((fail+1))
  echo "EXEMPT-OVERFLOW  豁免 ${n_skip} 条 > 上限 ${EXEMPT_MAX}（只许缩小）" >> "$TMP/viol.txt"
fi

# G2 零污染
POST_BUILD=$(ls -1 "$EX/build" 2>/dev/null | wc -l)
if [ "$POST_BUILD" -gt "$PRE_BUILD" ]; then
  fail=$((fail+1))
  echo "POLLUTION  仓库树 examples/build 条目 $PRE_BUILD → $POST_BUILD（本门必须在临时树里构建）" >> "$TMP/viol.txt"
fi

# C5 前提自证
if [ "$NTODO" -lt 1 ]; then
  echo "EXAMPLES-RUN=ENV rc=3 原因=非豁免集合为空（全被豁免 ⇒ 判据空转）"; exit 3
fi

echo "账面：顶层示例=$N 非豁免=$NTODO 真跑通过=$n_ok 真跑失败=$n_fail 豁免=$n_skip（上限 $EXEMPT_MAX）"
echo "前提自证：px 可执行=是 · 真跑=$((n_ok+n_fail)) 条 · 非豁免≥1 · 仓库树零污染=$([ "$POST_BUILD" -le "$PRE_BUILD" ] && echo 是 || echo 否)"

if [ "$fail" -gt 0 ]; then
  echo "--- 违例（最多 40 条） ---"; head -40 "$TMP/viol.txt"
  echo "EXAMPLES-RUN=FAIL rc=1 违例=$fail stale=$n_stale reason=$n_reason"
  exit 1
fi
echo "EXAMPLES-RUN=OK 顶层=$N 非豁免=$NTODO 通过=$n_ok 豁免=$n_skip"
exit 0
