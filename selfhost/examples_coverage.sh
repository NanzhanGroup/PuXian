#!/usr/bin/env bash
# ============================================================
# selfhost/examples_coverage.sh —— examples 覆盖账（M421 · 缺陷 846）
# ------------------------------------------------------------
# 为什么有本文件：
#   缺陷 845（M420）是 examples/m28_route.px —— 一个生产级 Web 路由示例，
#   常规构建必崩 SEGV，而 292 门全绿。原因不是门不严，是**这门对账从不存在**：
#   该示例既没有 verify.sh、也没注册进 registry ⇒ 全量门天然看不见它。
#   ⇒ 「examples 里哪个单元有断言、哪个只是示例、哪个在裸奔」必须成为机器判据。
#
# 判据（三条，都必须有牙）：
#   L1 单元枚举：examples/ 下每个顶层 *.px 与每个含 *.px 的目录 = 一个单元
#   L2 覆盖判定：
#        COVERED  该单元的路径被「已注册的 run 入口」或其直属 verify.sh 引用
#        ORPHAN   有 verify.sh 但不在任何执行路径上（看着有覆盖，其实早就不跑了）
#        BLACKBOX 没有 verify.sh（连断言都没有）
#   L3 豁免账 selfhost/examples_exempt.tsv：
#        未登记 / 登记过期 / kind 不符 / 已覆盖却仍挂着（STALE-EXEMPT）⇒ 一律判红
#
# 局限（如实写在这里，不藏着）：
#   覆盖判定是**一跳可达**（registry 全文 + 已注册 verify.sh 正文），不做深度递归。
#   深度 >1 的调用链会被判 ORPHAN ⇒ 要么补注册，要么进豁免表写清理由。
#   这是刻意选择：宁可假红一次让人看一眼，也不做「猜它可能被跑过」的假绿。
#
# 用法：
#   examples_coverage.sh               真仓对账（默认）
#   examples_coverage.sh --root DIR    指定仓库根（自测夹具用）
#   examples_coverage.sh --list        打印逐条分类
#   examples_coverage.sh --emit-exempt 打印豁免 TSV 草稿（不判定）
# 退出码：0 全部有账 · 1 有未登记/过期/不符 · 3 环境不成立（不降级成 0）
# ============================================================
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE=check
while [ $# -gt 0 ]; do
  case "$1" in
    --root)        ROOT="$2"; shift 2 ;;
    --list)        MODE=list; shift ;;
    --emit-exempt) MODE=emit; shift ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done

EX="$ROOT/examples"
REG="$ROOT/selfhost/gates.registry.sh"
EXEMPT="$ROOT/selfhost/examples_exempt.tsv"
TODAY="$(date +%Y-%m-%d)"

# ── 环境守卫：任何一条不成立都 rc=3，绝不静默降级成「绿」 ──────────────
[ -d "$EX" ]  || { echo "EXAMPLES-COVERAGE=ENV rc=3 原因=examples 目录不存在（$EX）"; exit 3; }
[ -f "$REG" ] || { echo "EXAMPLES-COVERAGE=ENV rc=3 原因=registry 不存在（$REG）"; exit 3; }

# ── L1 单元枚举 ────────────────────────────────────────────────
#   unit 标识：不带 examples/ 前缀的相对名
#     顶层文件 → "m28_route.px"
#     目录     → "m43_webapp"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT

: > "$TMP/units.txt"
for f in "$EX"/*.px; do [ -f "$f" ] && basename "$f" >> "$TMP/units.txt"; done
while IFS= read -r d; do
  [ -d "$d" ] || continue
  if find "$d" -name '*.px' -print -quit 2>/dev/null | grep -q .; then
    basename "$d" >> "$TMP/units.txt"
  fi
done < <(find "$EX" -maxdepth 1 -mindepth 1 -type d | sort)
sort -u "$TMP/units.txt" -o "$TMP/units.txt"
N_UNITS=$(wc -l < "$TMP/units.txt")
if [ "$N_UNITS" -eq 0 ]; then
  echo "EXAMPLES-COVERAGE=ENV rc=3 原因=扫描面为 0（examples 下没有任何 .px 单元）"; exit 3
fi

# 预取：全部 .px 文件（一次 find）+ high 风险文件（一次 grep）—— 逐单元 find 会超时
find "$EX" -name '*.px' | sort > "$TMP/px_files.txt"
grep -rlE 'serve\(|route\(|cron|WebServer|listen\(' "$EX" --include='*.px' 2>/dev/null | sort > "$TMP/high_files.txt"

# 每个单元是否自带 verify.sh
: > "$TMP/has_verify.txt"
while IFS= read -r u; do
  case "$u" in
    *.px) [ -f "$EX/${u%.px}/verify.sh" ] && echo "$u" >> "$TMP/has_verify.txt" ;;
    *)    [ -f "$EX/$u/verify.sh" ] && echo "$u" >> "$TMP/has_verify.txt" ;;
  esac
done < "$TMP/units.txt"

# ── L2 覆盖判定（一跳） ────────────────────────────────────────
# 覆盖证据池 = registry 全文 + 每个「被 registry 直接 run 的 verify.sh」正文
#   （只用已注册入口，不把孤儿 verify.sh 也算进来 —— 否则孤儿引用孤儿会互相洗白）
: > "$TMP/pool.txt"
cat "$REG" >> "$TMP/pool.txt"
while IFS= read -r v; do
  [ -f "$ROOT/examples/$v" ] && cat "$ROOT/examples/$v" >> "$TMP/pool.txt"
done < <(grep -oE 'examples/[A-Za-z0-9_.-]+/verify\.sh' "$REG" | sed -E 's#^examples/##' | sort -u)

# 把证据池里的 examples 路径提成 token（只取 examples/ 之后的相对部分）
grep -oE 'examples/[A-Za-z0-9_./-]+' "$TMP/pool.txt" 2>/dev/null \
  | sed -E 's#^examples/##' | sort -u > "$TMP/tokens.txt"

is_covered() {  # $1=unit
  local u="$1"
  case "$u" in
    *.px) grep -qxF "$u" "$TMP/tokens.txt" && return 0
          # 兼容「不带扩展名引用」（如 examples/hello）与「带扩展名」
          grep -qxF "${u%.px}" "$TMP/tokens.txt" && return 0
          return 1 ;;
    *)    # 目录：任一 token 以 "<unit>/" 开头即算被引用
          grep -qE "^${u}/" "$TMP/tokens.txt" && return 0
          grep -qxF "$u/verify.sh" "$TMP/tokens.txt" && return 0
          return 1 ;;
  esac
}

: > "$TMP/class.tsv"   # unit<TAB>class<TAB>px数
while IFS= read -r u; do
  case "$u" in
    *.px) px=1 ;;
    *)    px=$(awk -v p="$EX/$u/" 'index($0,p)==1' "$TMP/px_files.txt" | wc -l) ;;
  esac
  if grep -qxF "$u" "$TMP/has_verify.txt"; then
    if is_covered "$u"; then cls=COVERED; else cls=ORPHAN; fi
  else
    cls=BLACKBOX
  fi
  printf '%s\t%s\t%s\n' "$u" "$cls" "$px" >> "$TMP/class.tsv"
done < "$TMP/units.txt"

# high 风险：单元内含服务端/路由/定时能力（缺陷 845 的形态）
: > "$TMP/high.txt"
while IFS=$'\t' read -r u cls px; do
  case "$u" in
    *.px) grep -qxF "$EX/$u" "$TMP/high_files.txt" && echo "$u" >> "$TMP/high.txt" ;;
    *)    awk -v p="$EX/$u/" 'index($0,p)==1{found=1; exit} END{exit !found}' "$TMP/high_files.txt" \
            && echo "$u" >> "$TMP/high.txt" ;;
  esac
done < "$TMP/class.tsv"

# ── L3 豁免账 ─────────────────────────────────────────────────
# 格式：unit<TAB>kind<TAB>owner<TAB>expires(YYYY-MM-DD)<TAB>reason
#   kind ∈ blackbox | orphan | build-artifact
declare -A EX_KIND EX_EXP EX_OWNER
if [ -f "$EXEMPT" ]; then
  while IFS=$'\t' read -r u k o e r; do
    case "$u" in ''|'#'*) continue ;; esac
    EX_KIND["$u"]="$k"; EX_EXP["$u"]="$e"; EX_OWNER["$u"]="$o"
  done < "$EXEMPT"
fi

emit_rows() {
  while IFS=$'\t' read -r u cls px; do
    case "$cls" in
      COVERED) k=covered ;;
      ORPHAN)  k=orphan ;;
      *)       k=blackbox ;;
    esac
    h="-"; grep -qxF "$u" "$TMP/high.txt" && h=high
    printf '%s\t%s\t[%s]\t%s\tpx=%s\n' "$u" "$cls" "$h" "$k" "$px"
  done < "$TMP/class.tsv"
}

if [ "$MODE" = list ]; then emit_rows; exit 0; fi

if [ "$MODE" = emit ]; then
  echo "# examples 豁免账（M421）· unit<TAB>kind<TAB>owner<TAB>expires<TAB>reason"
  emit_rows | awk -F'\t' '$2!="COVERED"{printf "%s\t%s\t东月\t2026-12-19\tTODO：补断言或说明为何无需断言（%s）\n",$1,$4,$2}'
  exit 0
fi

# ── 判定 ──────────────────────────────────────────────────────
fail=0; n_unlisted=0; n_expired=0; n_kind=0; n_stale=0
: > "$TMP/fails.txt"
while IFS=$'\t' read -r u cls px; do
  want=""
  case "$cls" in
    ORPHAN)  want=orphan ;;
    BLACKBOX) want=blackbox ;;
    COVERED) want=covered ;;
  esac
  if [ -z "${EX_KIND[$u]:-}" ]; then
    if [ "$want" != covered ]; then
      fail=$((fail+1)); n_unlisted=$((n_unlisted+1))
      echo "UNLISTED   $u  ($cls)" >> "$TMP/fails.txt"
    fi
    continue
  fi
  k="${EX_KIND[$u]}"; e="${EX_EXP[$u]:-}"
  # 已覆盖却仍挂豁免 ⇒ 烂账（防 M410 那类「豁免永不回收」）
  if [ "$want" = covered ] && [ "$k" != build-artifact ]; then
    fail=$((fail+1)); n_stale=$((n_stale+1)); echo "STALE-EXEMPT $u (已 $cls)" >> "$TMP/fails.txt"; continue
  fi
  # kind 与实际不符
  if [ "$want" != covered ] && [ "$k" != "$want" ] && [ "$k" != build-artifact ]; then
    fail=$((fail+1)); n_kind=$((n_kind+1)); echo "KIND-MISMATCH $u (实际 $want，登记 $k)" >> "$TMP/fails.txt"; continue
  fi
  # 到期
  if [ -z "$e" ] || [[ "$e" < "$TODAY" ]]; then
    fail=$((fail+1)); n_expired=$((n_expired+1)); echo "EXPIRED    $u (到期=$e)" >> "$TMP/fails.txt"; continue
  fi
done < "$TMP/class.tsv"

n_cov=$(awk -F'\t' '$2=="COVERED"' "$TMP/class.tsv" | wc -l)
n_orp=$(awk -F'\t' '$2=="ORPHAN"'  "$TMP/class.tsv" | wc -l)
n_blk=$(awk -F'\t' '$2=="BLACKBOX"' "$TMP/class.tsv" | wc -l)
n_high=$(wc -l < "$TMP/high.txt")
n_ex=$(wc -l < <(awk -F'\t' '$1!~/^#/ && NF>=4' "$EXEMPT" 2>/dev/null))

echo "账面：units=$N_UNITS covered=$n_cov orphan=$n_orp blackbox=$n_blk high=$n_high 豁免=$n_ex"
echo "前提自证：registry 可读=$([ -s "$REG" ] && echo 是) · 证据 token=$(wc -l < "$TMP/tokens.txt") 条 · 扫描面非空=$([ "$N_UNITS" -gt 0 ] && echo 是)"

if [ "$fail" -gt 0 ]; then
  echo "--- 违例（最多 40 条） ---"
  head -40 "$TMP/fails.txt"
  echo "EXAMPLES-COVERAGE=FAIL rc=1 unlisted=$n_unlisted expired=$n_expired kind=$n_kind stale=$n_stale"
  exit 1
fi
echo "EXAMPLES-COVERAGE=OK units=$N_UNITS covered=$n_cov orphan=$n_orp blackbox=$n_blk high=$n_high exempt=$n_ex"
exit 0
