#!/usr/bin/env bash
# check_keyword_sync.sh —— PuXian 保留字表单一事实源校验（M419 · 缺陷 841/842）
#
# 背景（实测 2026-11-18，HEAD b38da5e）：
#   · 12 个 .px 源文件各自手抄一份 KEYWORDS，均为 41 键
#   · 行 md5 分成两组（d4ac805a / 19d94fa9），差异仅行尾 "# noqa"
#     ⇒ 判据必须比「规范化后的键集合」，不能比行指纹（否则假红）
#   · docs/spec.md §2.4 列 35 键：多 1 个假的（type），漏 7 个（and/or/not/None/extern/try/catch）
#   · 44 个 .px 里只有 12 个持有 KEYWORDS（其余是库/工具，不该有）
#     ⇒ 判据不能「要求所有 .px 都有」，那样 29 份假红
#     ⇒ 正解：清单（owners）↔ 实际持有者 双向对账，多一个少一个都红
#
# 用法: check_keyword_sync.sh [--repo DIR] [--owners FILE] [--selftest]
# 退出: 0 一致 / 1 不一致 / 3 环境问题（名单或 spec 缺失、样本面为 0）
set -uo pipefail

REPO=""
OWNERS=""
SELFTEST=0
while [ $# -gt 0 ]; do
  case "$1" in
    --repo) REPO="$2"; shift 2 ;;
    --owners) OWNERS="$2"; shift 2 ;;
    --selftest) SELFTEST=1; shift ;;
    *) echo "未知参数: $1" >&2; exit 3 ;;
  esac
done

PASS=0; FAIL=0
ok()  { PASS=$((PASS+1)); echo "  ✅ $*"; }
bad() { FAIL=$((FAIL+1)); echo "  ❌ $*"; }

# 从一个 .px 文件提取 KEYWORDS 键集合（排序、去重、换行分隔）
kw_of() {
  grep -m1 'let KEYWORDS = {' "$1" 2>/dev/null \
    | grep -o '"[A-Za-z_][A-Za-z0-9_]*":' \
    | sed 's/^"//; s/":$//' \
    | sort -u
}

# 仓库内**实际持有** KEYWORDS 的 .px 源文件（相对路径，排序）
holders_of() {
  local repo="$1"
  grep -rl 'let KEYWORDS = {' "$repo/selfhost" "$repo/tools" --include='*.px' 2>/dev/null \
    | grep -v '/build/' \
    | sed "s|^$repo/||" \
    | sort
}

# 从 docs/spec.md §2.4 代码块提取关键字（排序、换行分隔）
spec_kw() {
  awk '/^### 2\.4 关键字/{f=1; next} f && /^```/{c++; if(c==2) exit; next} f && c==1{print}' "$1" 2>/dev/null \
    | tr -s ' \t' '\n' \
    | grep -E '^[A-Za-z_][A-Za-z0-9_]*$' \
    | sort -u
}

core_check() {
  local repo="$1" tag="$2"
  echo "── [$tag] repo=$repo"
  [ -d "$repo" ] || { bad "$tag 仓库目录不存在"; return 3; }

  local owners_file="${OWNERS:-$repo/selfhost/keywords_owners.txt}"
  [ -f "$owners_file" ] || { bad "$tag 名单缺失: $owners_file"; return 3; }

  local owners holders
  owners=$(grep -v '^[[:space:]]*#' "$owners_file" 2>/dev/null | grep -v '^[[:space:]]*$' | sed 's/[[:space:]]*$//' | sort -u)
  holders=$(holders_of "$repo")

  [ -n "$owners" ] || { bad "$tag 名单为空（判据失效，不是绿）"; return 3; }
  [ -n "$holders" ] || { bad "$tag 样本面为 0（找不到任何 KEYWORDS 定义）"; return 3; }

  local n_owners n_holders
  n_owners=$(printf '%s\n' "$owners" | wc -l)
  n_holders=$(printf '%s\n' "$holders" | wc -l)
  echo "     名单 $n_owners 份 / 实际持有 $n_holders 份"

  # 1) 双向对账：未登记持有者 / 名单里不存在或已无定义
  local unreg gone
  unreg=$(comm -13 <(printf '%s\n' "$owners") <(printf '%s\n' "$holders"))
  gone=$(comm -23 <(printf '%s\n' "$owners") <(printf '%s\n' "$holders"))
  if [ -n "$unreg" ]; then bad "$tag 未登记持有者 $(echo "$unreg" | wc -l) 份:"; echo "$unreg" | sed 's/^/        + /'
  else ok "$tag 无未登记持有者"; fi
  if [ -n "$gone" ]; then bad "$tag 名单项已失效 $(echo "$gone" | wc -l) 份（文件不在或已无定义）:"; echo "$gone" | sed 's/^/        - /'
  else ok "$tag 名单项均有效"; fi

  # 2) 所有持有者键集合一致（比键集合，不比行指纹）
  local base="" basefile="" n_diff=0
  while IFS= read -r f; do
    [ -n "$f" ] || continue
    local k; k=$(kw_of "$repo/$f")
    [ -n "$k" ] || continue
    if [ -z "$base" ]; then base="$k"; basefile="$f"; continue; fi
    if [ "$k" != "$base" ]; then
      n_diff=$((n_diff+1))
      echo "     ✗ 键集合不一致: $f（$(printf '%s\n' "$k" | wc -l) 键 vs 基准 $basefile）"
    fi
  done <<< "$holders"
  if [ "$n_diff" -gt 0 ]; then bad "$tag $n_diff 份键集合偏差"
  else ok "$tag 键集合语义一致（$n_holders 份，比键集合不比行指纹）"; fi

  local cnt_base; cnt_base=$(printf '%s\n' "$base" | wc -l)
  [ "$cnt_base" -gt 1 ] || { bad "$tag 基准键数异常($cnt_base)"; return 1; }
  echo "     实现键数 = $cnt_base"

  # 3) 与 spec §2.4 对账
  local spec="$repo/docs/spec.md"
  [ -f "$spec" ] || { bad "$tag spec 不存在"; return 3; }
  local sk; sk=$(spec_kw "$spec")
  [ -n "$sk" ] || { bad "$tag spec §2.4 提取为空（判据失效，不是绿）"; return 3; }
  echo "     spec 键数 = $(printf '%s\n' "$sk" | wc -l)"

  local extra missing
  extra=$(comm -23 <(printf '%s\n' "$base") <(printf '%s\n' "$sk") | tr '\n' ' ')
  missing=$(comm -13 <(printf '%s\n' "$base") <(printf '%s\n' "$sk") | tr '\n' ' ')
  if [ -n "$extra" ]; then bad "$tag spec 漏列 $(echo "$extra" | wc -w) 个: $extra"
  else ok "$tag spec 无漏列"; fi
  if [ -n "$missing" ]; then bad "$tag spec 多列 $(echo "$missing" | wc -w) 个（实现无）: $missing"
  else ok "$tag spec 无多列"; fi

  # 4) 判据自证：M392 已实现的 try/catch 必须在表内
  local w
  for w in try catch; do
    if grep -qx "$w" <<< "$base"; then ok "$tag 判据自证：$w 在实现关键字表内"
    else bad "$tag 判据自证失败：$w 不在实现关键字表内"; fi
  done
  return 0
}

# ───────── 自测 ─────────
selftest() {
  T=$(mktemp -d /tmp/m419_sel.XXXXXX)   # 故意不用 local：trap 在函数返回后求值
  trap 'rm -rf "$T"' EXIT
  local TP=0 TF=0
  echo "=== 自测（fixture 在 $T）==="
  assert() { if [ "$2" -eq 0 ]; then TP=$((TP+1)); echo "  ✅ $1"; else TF=$((TF+1)); echo "  ❌ $1"; fi; }

  KW='{"let": "let", "try": "try", "catch": "catch"}'
  SPEC='let  try  catch'

  # mkfix $1=dir [$2=是否写名单(默认1)] [$3=名单内容]
  mkfix() {
    local d="$1" wl="${2:-1}" own="${3:-a/lexer.px}"
    mkdir -p "$d/selfhost" "$d/tools" "$d/docs"
    printf 'x\nlet KEYWORDS = %s\n' "$KW" > "$d/selfhost/lexer.px"
    printf 'x\nlet KEYWORDS = %s\n' "$KW" > "$d/tools/pxcheck.px"
    printf 'x\ndef foo(): return 1\n' > "$d/tools/pxpkg.px"   # 不持有关键字表的普通文件
    [ "$wl" = 1 ] && printf '%s\n' "$own" > "$d/selfhost/keywords_owners.txt"
    { printf '### 2.4 关键字（保留字）\n```\n'; printf '%s\n' "$SPEC"; printf '```\n\n### 2.5 字面量\n'; } > "$d/docs/spec.md"
  }

  # T1 正控：名单与实际持有者一致 ⇒ 无违例
  FAIL=0; PASS=0
  mkfix "$T/t1"
  printf 'selfhost/lexer.px\ntools/pxcheck.px\n' > "$T/t1/selfhost/keywords_owners.txt"
  core_check "$T/t1" T1 > "$T/t1.log" 2>&1
  assert "T1 正控：名单↔持有者一致 ⇒ 无违例" $([ "$FAIL" -eq 0 ]; echo $?)

  # T2 负控：未登记的新持有者 ⇒ 必红
  FAIL=0; PASS=0
  mkfix "$T/t2"
  printf 'selfhost/lexer.px\n' > "$T/t2/selfhost/keywords_owners.txt"   # pxcheck.px 未登记
  core_check "$T/t2" T2 > "$T/t2.log" 2>&1
  { [ "$FAIL" -gt 0 ] && grep -q '未登记持有者' "$T/t2.log"; }; assert "T2 负控：多一个未登记持有者 ⇒ 判红" $?
  { [ "$(holders_of "$T/t2" | wc -l)" -eq 2 ] && [ "$(grep -c . "$T/t2/selfhost/keywords_owners.txt")" -eq 1 ]; }; assert "T2 前提自证：实际持有 2 份 vs 名单 1 份（扰动真发生）" $?

  # T3 负控：名单项已失效（文件删了）⇒ 必红
  FAIL=0; PASS=0
  mkfix "$T/t3"
  printf 'selfhost/lexer.px\ntools/pxcheck.px\nselfhost/gone.px\n' > "$T/t3/selfhost/keywords_owners.txt"
  core_check "$T/t3" T3 > "$T/t3.log" 2>&1
  { [ "$FAIL" -gt 0 ] && grep -q '名单项已失效' "$T/t3.log"; }; assert "T3 负控：名单含已消失文件 ⇒ 判红" $?

  # T4 负控：两份键集合不一致 ⇒ 必红
  FAIL=0; PASS=0
  mkfix "$T/t4"
  printf 'selfhost/lexer.px\ntools/pxcheck.px\n' > "$T/t4/selfhost/keywords_owners.txt"
  printf 'x\nlet KEYWORDS = {"let": "let", "try": "try"}\n' > "$T/t4/tools/pxcheck.px"
  core_check "$T/t4" T4 > "$T/t4.log" 2>&1
  { [ "$FAIL" -gt 0 ] && grep -q '键集合不一致' "$T/t4.log"; }; assert "T4 负控：副本缺键 ⇒ 判红" $?

  # T5 负控：spec 漏列 / 多列 ⇒ 各必红
  FAIL=0; PASS=0
  mkfix "$T/t5"
  printf 'selfhost/lexer.px\ntools/pxcheck.px\n' > "$T/t5/selfhost/keywords_owners.txt"
  { printf '### 2.4 关键字（保留字）\n```\nlet  try\n```\n\n### 2.5\n'; } > "$T/t5/docs/spec.md"
  core_check "$T/t5" T5 > "$T/t5.log" 2>&1
  { [ "$FAIL" -gt 0 ] && grep -q 'spec 漏列' "$T/t5.log"; }; assert "T5a 负控：spec 漏列 ⇒ 判红" $?
  FAIL=0; PASS=0
  { printf '### 2.4 关键字（保留字）\n```\nlet  try  catch  type\n```\n\n### 2.5\n'; } > "$T/t5/docs/spec.md"
  core_check "$T/t5" T5b > "$T/t5b.log" 2>&1
  { [ "$FAIL" -gt 0 ] && grep -q 'spec 多列' "$T/t5b.log"; }; assert "T5b 负控：spec 多列 type（实现没有）⇒ 判红" $?

  # T6 行指纹陷阱：键相同、行尾注释不同 ⇒ 必须判绿（防「比 md5」的假红）
  FAIL=0; PASS=0
  mkfix "$T/t6"
  printf 'selfhost/lexer.px\ntools/pxcheck.px\n' > "$T/t6/selfhost/keywords_owners.txt"
  printf 'x\nlet KEYWORDS = %s  # noqa\n' "$KW" > "$T/t6/tools/pxcheck.px"
  local m1 m2; m1=$(grep -m1 'let KEYWORDS' "$T/t6/selfhost/lexer.px" | md5sum | cut -c1-8); m2=$(grep -m1 'let KEYWORDS' "$T/t6/tools/pxcheck.px" | md5sum | cut -c1-8)
  core_check "$T/t6" T6 > "$T/t6.log" 2>&1
  { [ "$FAIL" -eq 0 ] && [ "$m1" != "$m2" ]; }; assert "T6 行指纹陷阱：md5 不同($m1≠$m2) 但键集合相同 ⇒ 判绿" $?

  # T7 环境问题：无名单 / 无持有者 ⇒ 报环境问题，不判绿
  FAIL=0; PASS=0
  mkfix "$T/t7" 0
  core_check "$T/t7" T7 > "$T/t7.log" 2>&1
  { [ "$FAIL" -gt 0 ] && grep -q '名单缺失' "$T/t7.log"; }; assert "T7 负控：无名单文件 ⇒ 报环境问题（不判绿）" $?

  echo ""
  echo "== 自测汇总：pass=$TP fail=$TF =="
  [ "$TF" -eq 0 ]
}

if [ "$SELFTEST" -eq 1 ]; then selftest; exit $?; fi

[ -n "$REPO" ] || REPO="$(cd "$(dirname "$0")/.." 2>/dev/null && pwd)"
core_check "$REPO" MAIN
RC=$?

echo ""
# 环境问题优先（缺名单 / spec 提取为空 / 样本面为 0）：这类「查不了」绝不能降级成
# 「查出来有一处不一致」—— 前者要人修环境，后者要人改代码，混在一起会误导处置。
if [ "$RC" -eq 3 ]; then echo "KEYWORD-SYNC=ENVFAIL pass=$PASS fail=$FAIL rc=3"; exit 3; fi
if [ "$FAIL" -gt 0 ]; then
  echo "KEYWORD-SYNC=FAIL pass=$PASS fail=$FAIL"
  exit 1
fi
if [ "$RC" -ne 0 ]; then echo "KEYWORD-SYNC=ENVFAIL pass=$PASS fail=$FAIL rc=$RC"; exit 3; fi
echo "KEYWORD-SYNC=OK pass=$PASS fail=0"
exit 0
