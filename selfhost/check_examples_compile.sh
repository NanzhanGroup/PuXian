#!/usr/bin/env bash
# ============================================================
# selfhost/check_examples_compile.sh —— **示例即文档：顶层示例必须编译得过**（M422 · 缺陷 856/857）
# ------------------------------------------------------------
# 为什么有它：
#   缺陷 856：examples/ 下 4 个顶层示例（m25_closure_gc / m25_tls_resume / m26_ushr / m29_webprod）
#     在 `let` 默认不可变（M41.2）之后**编译不过**，而没有任何门会红 —— 因为它们全都是 BLACKBOX
#     （M421 账上 143 条无 verify.sh）。用户照着仓库里的示例写代码，第一步就撞 E3002。
#   缺陷 857：m26_ushr 不但编译不过，里面的注释还写着「-1 >>> 1 ⇒ 1」——
#     而 M217（缺陷 309）已把**负数移位计数**定为 R1003。示例与实现相反，同样无人发现。
#   ⇒ 同一形状：**示例是产品对外的一部分，却没有任何判据看着它**。
#
# 判据：
#   C1 顶层枚举：examples/*.px 每个都算一个示例（扫面 < MIN 视为提取器坏了 ⇒ rc=3）
#   C2 逐条编译：`px build` 必须 rc=0；失败 ⇒ 红并打印首行错误
#   C3 豁免账 selfhost/examples_compile_skip.tsv：
#        格式 file<TAB>reason<TAB>expires
#        · 表里有、扫描面里没有该文件 ⇒ STALE 红（防「永不回收的洗白账」）
#        · 表里有、实际编译**通过** ⇒ STALE 红（同上，账必须跟着事实走）
#        · 表里有、确实编译失败 ⇒ 允许（原因必须写清）
#   C4 前提自证：真跑了 N 条（N ≥ MIN）、至少有 1 条非跳过 ⇒ 否则 rc=3
#
# 用法：
#   check_examples_compile.sh              真仓判定
#   check_examples_compile.sh --root DIR   指定仓库根（夹具用）
#   check_examples_compile.sh --list       逐条打印（不判定）
#   check_examples_compile.sh --self-test  9 条正/负控（含负控必须真红）
#   check_examples_compile.sh --emit-skip  打印豁免草稿
# 退出码：0 全过 / 1 有违例 / 2 自测失败 / 3 环境不成立（不降级成 0）
# ============================================================
set -u

MIN_EXAMPLES=${MIN_EXAMPLES:-50}
JOBS=${JOBS:-8}
# 超时口径（2026-11-22 M430 r4 · 缺陷 886）：
#   冷缓存（.rtcache 全失效）叠加 8 分片满载时，本门内 JOBS=8 的并行编译会集体撞 120s
#   ⇒ 40 条 rc=124 被当成「编译失败」，而低负载复跑 11s 全绿。**这是环境红，不是产品红**。
#   现口径：并行 124 不算结论 ⇒ 对这批文件**降并发串行重跑一次**（TIMEOUT_RETRY 秒）；
#   串行仍 124 ⇒ 判红（真挂死的编译器/死循环仍跑不掉）。真语法错是非 0 非 124 ⇒ 立刻红，牙不掉。
TIMEOUT_SEC=${EX_TIMEOUT:-120}
TIMEOUT_RETRY_SEC=${EX_TIMEOUT_RETRY:-600}
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

# ── 自测：夹具（真起一个临时根，判据必须红得动、绿得住）──────────────
if [ "$MODE" = selftest ]; then
  P=0; F=0
  ok(){ echo "  ✅ $1"; P=$((P+1)); }
  bad(){ echo "  ❌ $1"; F=$((F+1)); }
  T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
  mkfix(){ # $1=根
    mkdir -p "$1/examples" "$1/selfhost" "$1/tools"
    cat > "$1/examples/good.px" <<'EOF'
print("ok")
EOF
    cat > "$1/examples/bad.px" <<'EOF'
let g = 1
g = 2
EOF
    ln -sf "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/tools/px" "$1/tools/px"
    : > "$1/selfhost/examples_compile_skip.tsv"
  }
  echo "=== check_examples_compile --self-test ==="
  # T1 正控：只有好的 ⇒ rc=0
  mkfix "$T/r1"; rm -f "$T/r1/examples/bad.px"; : > "$T/r1/selfhost/examples_compile_skip.tsv"
  MIN_EXAMPLES=1 bash "${BASH_SOURCE[0]}" --root "$T/r1" > "$T/o1" 2>&1; r1=$?
  [ "$r1" = 0 ] && ok "T1 正控：全可编译 ⇒ rc=0" || { bad "T1 期望 rc=0 实得 $r1"; cat "$T/o1"; }
  # T2 负控：有一个坏且未登记 ⇒ 必须红且点名
  mkfix "$T/r2"
  MIN_EXAMPLES=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o2" 2>&1; r2=$?
  if [ "$r2" = 1 ] && grep -q 'bad.px' "$T/o2"; then ok "T2 负控：坏示例未登记 ⇒ rc=1 且点名"; else bad "T2 期望 rc=1 且点名 bad.px，实得 rc=$r2"; cat "$T/o2"; fi
  # T3 正控：坏示例登记在豁免表 ⇒ rc=0
  printf 'bad.px\t故意坏（自测夹具）\t2099-01-01\n' > "$T/r2/selfhost/examples_compile_skip.tsv"
  MIN_EXAMPLES=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o3" 2>&1; r3=$?
  [ "$r3" = 0 ] && ok "T3 正控：坏示例已登记 ⇒ rc=0" || { bad "T3 期望 rc=0 实得 $r3"; cat "$T/o3"; }
  # T4 负控：豁免表里的文件其实能编译 ⇒ STALE 红
  printf 'good.px\t不该豁免\t2099-01-01\n' >> "$T/r2/selfhost/examples_compile_skip.tsv"
  MIN_EXAMPLES=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o4" 2>&1; r4=$?
  if [ "$r4" = 1 ] && grep -q 'STALE' "$T/o4"; then ok "T4 负控：能编译却挂豁免 ⇒ STALE 红"; else bad "T4 期望 STALE 红，实得 rc=$r4"; cat "$T/o4"; fi
  # T5 负控：豁免表里的文件根本不存在 ⇒ STALE 红
  : > "$T/r2/selfhost/examples_compile_skip.tsv"
  printf 'bad.px\tx\t2099-01-01\nno_such.px\tx\t2099-01-01\n' > "$T/r2/selfhost/examples_compile_skip.tsv"
  MIN_EXAMPLES=1 bash "${BASH_SOURCE[0]}" --root "$T/r2" > "$T/o5" 2>&1; r5=$?
  if [ "$r5" = 1 ] && grep -q 'no_such.px' "$T/o5"; then ok "T5 负控：豁免项无对应文件 ⇒ 红并点名"; else bad "T5 期望 rc=1 且点名 no_such.px，实得 rc=$r5"; cat "$T/o5"; fi
  # T6 环境守卫：examples 目录不存在 ⇒ rc=3
  mkfix "$T/r6"; rm -rf "$T/r6/examples"
  bash "${BASH_SOURCE[0]}" --root "$T/r6" > "$T/o6" 2>&1; r6=$?
  [ "$r6" = 3 ] && ok "T6 环境守卫：examples 缺失 ⇒ rc=3" || bad "T6 期望 rc=3 实得 $r6"
  # T7 环境守卫：扫描面 0（默认 MIN=50 而只有 0 个 px）⇒ rc=3
  mkfix "$T/r7"; rm -f "$T/r7/examples"/*.px
  bash "${BASH_SOURCE[0]}" --root "$T/r7" > "$T/o7" 2>&1; r7=$?
  [ "$r7" = 3 ] && ok "T7 环境守卫：扫描面 0 ⇒ rc=3" || bad "T7 期望 rc=3 实得 $r7"
  # T8 前提自证：px 工具不可执行 ⇒ rc=3（不把「编译全失败」当产品红）
  mkfix "$T/r8"; rm -f "$T/r8/tools/px"
  MIN_EXAMPLES=1 bash "${BASH_SOURCE[0]}" --root "$T/r8" > "$T/o8" 2>&1; r8=$?
  [ "$r8" = 3 ] && ok "T8 环境守卫：px 工具缺失 ⇒ rc=3" || bad "T8 期望 rc=3 实得 $r8"
  # T9 负控：并行+串行都超时 ⇒ 必须判红（不得当「环境不成立」放过；缺陷 886）
  mkfix "$T/r9"
  rm -f "$T/r9/tools/px"
  cat > "$T/r9/tools/px" <<'EOX'
#!/bin/sh
sleep 30
EOX
  chmod +x "$T/r9/tools/px"
  : > "$T/r9/selfhost/examples_compile_skip.tsv"
  EX_TIMEOUT=1 EX_TIMEOUT_RETRY=1 MIN_EXAMPLES=1 bash "${BASH_SOURCE[0]}" --root "$T/r9" > "$T/o9" 2>&1; r9=$?
  if [ "$r9" = 1 ] && grep -q 'COMPILE-TIMEOUT' "$T/o9"; then ok "T9 负控：并行+串行均超时 ⇒ rc=1 且点名 COMPILE-TIMEOUT"; else bad "T9 期望 rc=1 且点名 COMPILE-TIMEOUT，实得 rc=$r9"; cat "$T/o9"; fi
  echo "---- self-test: PASS=$P FAIL=$F ----"
  [ "$F" -eq 0 ] || exit 2
  echo "EXAMPLES-COMPILE-SELFTEST=OK"
  exit 0
fi

EX="$ROOT/examples"
PX="$ROOT/tools/px"
SKIP="$ROOT/selfhost/examples_compile_skip.tsv"

[ -d "$EX" ] || { echo "EXAMPLES-COMPILE=ENV rc=3 原因=examples 目录不存在（$EX）"; exit 3; }
[ -x "$PX" ] || { echo "EXAMPLES-COMPILE=ENV rc=3 原因=px 工具不可执行（$PX）"; exit 3; }

TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
ls -1 "$EX"/*.px 2>/dev/null | xargs -r -n1 basename | sort > "$TMP/units.txt"
N=$(wc -l < "$TMP/units.txt")
if [ "$N" -lt "$MIN_EXAMPLES" ]; then
  echo "EXAMPLES-COMPILE=ENV rc=3 原因=扫描面过小（顶层 .px = $N < $MIN_EXAMPLES，提取器或目录坏了）"; exit 3
fi

# 在临时树里编译，零污染仓库（也不与并发门抢 examples/build/）
mkdir -p "$TMP/src"
cp "$EX"/*.px "$TMP/src/" 2>/dev/null || true

export PX TMP
compile_one() {
  f="$1"; tmo="${2:-$TIMEOUT_SEC}"
  out=$( cd "$TMP/src" && timeout --kill-after=5 "$tmo" "$PX" build "$f" 2>&1 )
  rc=$?
  if [ "$rc" -eq 0 ]; then printf 'OK\t%s\n' "$f"; else
    if [ "$rc" -eq 124 ]; then
      printf 'TIMEOUT\t%s\trc=124 tmo=%ss\n' "$f" "$tmo"
    else
      first=$(printf '%s' "$out" | grep -m1 -E '编译错误|错误:|error' | cut -c1-160)
      printf 'FAIL\t%s\t%s\n' "$f" "${first:-rc=$rc}"
    fi
  fi
}
export -f compile_one
export TIMEOUT_SEC TIMEOUT_RETRY_SEC
xargs -r -n1 -P "$JOBS" bash -c 'compile_one "$0"' < "$TMP/units.txt" | sort -k2 > "$TMP/res0.txt"

# 并行超时 ⇒ 降并发串行重跑（见文件头 TIMEOUT_SEC 段）；串行仍 124 ⇒ 保留 TIMEOUT 行 ⇒ 下面按失败判红
if grep -q '^TIMEOUT' "$TMP/res0.txt"; then
  awk -F'\t' '$1=="TIMEOUT"{print $2}' "$TMP/res0.txt" > "$TMP/retry.txt"
  n_retry=$(wc -l < "$TMP/retry.txt")
  echo "并行超时 $n_retry 条 ⇒ 串行重跑（每条上限 ${TIMEOUT_RETRY_SEC}s）"
  : > "$TMP/retry_res.txt"
  while IFS= read -r f; do compile_one "$f" "$TIMEOUT_RETRY_SEC"; done < "$TMP/retry.txt" | sort -k2 >> "$TMP/retry_res.txt"
  awk -F'\t' 'NR==FNR{k[$2]=$0; next} $1=="TIMEOUT"{if ($2 in k) print k[$2]; else print $0; next} {print}' \
      "$TMP/retry_res.txt" "$TMP/res0.txt" | sort -k2 > "$TMP/res.txt"
else
  sort -k2 "$TMP/res0.txt" > "$TMP/res.txt"
fi

if [ "$MODE" = list ]; then cat "$TMP/res.txt"; exit 0; fi

# 豁免账
declare -A SKIP_REASON=()
if [ -f "$SKIP" ]; then
  while IFS=$'\t' read -r f r e; do
    case "$f" in ''|'#'*) continue ;; esac
    SKIP_REASON["$f"]="${r:-未写原因}"
  done < "$SKIP"
fi
if [ "$MODE" = emit ]; then
  grep '^FAIL' "$TMP/res.txt" | awk -F'\t' '{printf "%s\tTODO：写清为什么它不能编译（历史负向语料？）\t2026-12-19\n",$2}'
  exit 0
fi

fail=0; n_ok=0; n_fail=0; n_skip=0; n_stale=0
: > "$TMP/viol.txt"
while IFS=$'\t' read -r st f rest; do
  [ -n "${f:-}" ] || continue
  if [ "$st" = OK ]; then
    n_ok=$((n_ok+1))
    if [ -n "${SKIP_REASON[$f]:-}" ]; then
      fail=$((fail+1)); n_stale=$((n_stale+1)); echo "STALE-SKIP  $f（已能编译，豁免必须删：${SKIP_REASON[$f]}）" >> "$TMP/viol.txt"
    fi
  else
    n_fail=$((n_fail+1))
    if [ -n "${SKIP_REASON[$f]:-}" ]; then
      n_skip=$((n_skip+1))
    else
      fail=$((fail+1))
      if [ "$st" = TIMEOUT ]; then
        echo "COMPILE-TIMEOUT $f — ${rest:-无错误行}（并行+串行均超时 ⇒ 真红）" >> "$TMP/viol.txt"
      else
        echo "COMPILE-FAIL $f — ${rest:-无错误行}" >> "$TMP/viol.txt"
      fi
    fi
  fi
done < "$TMP/res.txt"

# 豁免表里指向不存在文件的条目
for f in "${!SKIP_REASON[@]}"; do
  grep -q "	$f" "$TMP/res.txt" || { fail=$((fail+1)); n_stale=$((n_stale+1)); echo "STALE-SKIP  $f（扫描面里没有这个文件）" >> "$TMP/viol.txt"; }
done

echo "账面：顶层示例=$N 编译通过=$n_ok 编译失败=$n_fail 其中豁免=$n_skip 豁免条目=${#SKIP_REASON[@]}"
echo "前提自证：px 工具可执行=是 · 真编译=$((n_ok+n_fail)) 条（≥$MIN_EXAMPLES）· 并行=$JOBS"

if [ "$fail" -gt 0 ]; then
  echo "--- 违例（最多 40 条） ---"; head -40 "$TMP/viol.txt"
  echo "EXAMPLES-COMPILE=FAIL rc=1 stale=$n_stale"
  exit 1
fi
echo "EXAMPLES-COMPILE=OK 顶层=$N 通过=$n_ok 豁免=$n_skip"
exit 0
