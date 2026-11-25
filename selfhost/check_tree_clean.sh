#!/usr/bin/env bash
# ============================================================
# check_tree_clean.sh —— 「树净」不许被 .gitignore 掩盖（M446 · 缺陷 873）
# ------------------------------------------------------------
# 病根：`git status --porcelain` 只报**未忽略**的文件。构建残渣、临时产物、以及
#   一条过粗的 .gitignore 规则（把本该入库的目录吞掉），都不会让树看起来脏 ⇒
#   发布链的「树净」判据是假绿。
#
# 口径（M446 定；用户授权自主决策）：
#   C1 跟踪文件干净（git status --porcelain 空）
#   C2 被忽略条目必须**已在账上**（selfhost/tree_clean_baseline.txt）——
#      新增的忽略项 ⇒ 红。为什么用账而不是正则白名单：实测 git 会把「只含被忽略内容」
#      的目录折叠成 `!! examples/`，正则白名单要么误放（过宽）要么误红（过窄）。
#      账只许收缩；新增一条必须在 CHANGELOG 写明「为什么它该被忽略」。
#   C3 --release 发布闸：examples/*/build 这类构建残渣必须为 0（日常开发允许）
#   N1 自测：账里没有的忽略项 ⇒ 红
#   N2 自测：账里有的 ⇒ 不红（证明「账」真的在起作用，不是永远红/永远绿）
#   N3 自测：未跟踪文件 ⇒ C1 红
#   N4 自测：--release 拦住 examples 构建残渣
#
# 用法：
#   check_tree_clean.sh [--repo DIR] [--release] [--self-test] [--emit-baseline]
# 退出码：0 干净 / 1 违例 / 2 自测失败 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE=check
while [ $# -gt 0 ]; do
  case "$1" in
    --repo)    ROOT="$2"; shift 2 ;;
    --release) MODE=release; shift ;;
    --self-test) MODE=selftest; shift ;;
    --emit-baseline) MODE=emit; shift ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done
BASELINE="$ROOT/selfhost/tree_clean_baseline.txt"

ignored_of() {   # $1=repo
  ( cd "$1" && git status --ignored --porcelain 2>/dev/null | sed -n 's/^!! //p' | sort -u )
}

check_repo() {
  local R="$1" release="$2" bl="$3" ok_n=0 bad_n=0
  ok()  { echo "✅ $*"; ok_n=$((ok_n+1)); }
  bad() { echo "❌ $*"; bad_n=$((bad_n+1)); }
  ( cd "$R" && git rev-parse --git-dir >/dev/null 2>&1 ) || { echo "ENV: $R 不是 git 仓"; return 3; }

  local dirty
  dirty=$( cd "$R" && git status --porcelain 2>/dev/null | grep -v '^!!' || true )
  if [ -z "$dirty" ]; then ok "C1 跟踪文件干净"; else
    bad "C1 跟踪文件有未提交改动（$(printf '%s\n' "$dirty" | wc -l) 条）"
    printf '%s\n' "$dirty" | head -6 | sed 's/^/     /'
  fi

  local cur new
  cur=$(ignored_of "$R")
  if [ ! -f "$bl" ]; then
    bad "C2 缺基线账 $bl（用 --emit-baseline 生成后提交）"
  else
    new=$(comm -23 <(printf '%s\n' "$cur") <(sort -u "$bl") | grep -v '^$' || true)
    # ── M446（缺陷 945）：把「.gitignore 明示的示例构建残渣」与「被藏的真文件」分开 ──
    #   病灶（2026-11-25 第三轮实测）：同一个 HEAD、同一本账，主树只多 1 条
    #   （examples/m65_lsp/build/），8 分片那轮多 7 条 —— 差集取决于**这一轮跑了哪些
    #   示例门**（verify.sh 就地 `px build $HERE/x.px`）。⇒ 账本式判据在这里
    #   结构性不稳定：它测的是「跑过哪些门」，不是「有没有被藏的真文件」。
    #   口径：只对 `examples/<名>/(build|artifacts)/` 这类 .gitignore **明示**的构建
    #   输出目录不判红（并在 C3 --release 里要求为 0，打 tag 前必须清）；
    #   其余账外忽略项一律照旧判红（含折叠形态 `!! examples/` —— 它可能藏真文件）。
    local vol_re='^examples/[^/]+/(build|artifacts)/$'
    local vol real_new
    vol=$(printf '%s\n' "$new" | grep -E "$vol_re" || true)
    real_new=$(printf '%s\n' "$new" | grep -vE "$vol_re" | grep -v '^$' || true)
    if [ -z "$real_new" ]; then
      if [ -n "$vol" ]; then
        ok "C2 被忽略条目未越账（账外 $(printf '%s\n' "$vol" | grep -c .) 条均为 .gitignore 明示的示例构建残渣，--release 拦）"
      else
        ok "C2 被忽略条目未越账（当前 $(printf '%s\n' "$cur" | grep -c .) 条）"
      fi
    else
      bad "C2 出现 $(printf '%s\n' "$real_new" | wc -l) 条**账外**忽略项（可能被 .gitignore 藏了真文件）"
      printf '%s\n' "$real_new" | head -8 | sed 's/^/     /'
    fi
    local gone
    gone=$(comm -13 <(printf '%s\n' "$cur") <(sort -u "$bl") | grep -v '^$' || true)
    [ -n "$gone" ] && echo "ℹ️  账上有 $(printf '%s\n' "$gone" | wc -l) 条已消失（可以收缩基线，非失败）"
  fi

  if [ "$release" = 1 ]; then
    local art
    # 折叠形态（git 把「只含被忽略内容」的目录折成 examples/）同样算残渣
    art=$(printf '%s\n' "$cur" | grep -E '^examples/(([^/]+/)?build/?)?$' || true)
    if [ -z "$art" ]; then ok "C3 发布闸：examples 构建残渣 = 0"; else
      bad "C3 发布闸：examples 下还有 $(printf '%s\n' "$art" | wc -l) 个 build 目录（打 tag 前须清）"
    fi
  fi

  echo "TREE-CLEAN-OK pass=$ok_n fail=$bad_n"
  [ "$bad_n" = 0 ] && return 0 || return 1
}

selftest() {
  local T rc=0
  T=$(mktemp -d /tmp/treeclean.XXXXXX) || return 3
  ( cd "$T" && git init -q . && printf 'junk/\n.buildcache/\nexamples/*/build/\n' > .gitignore \
    && printf 'x\n' > keep.txt && git add .gitignore keep.txt \
    && git -c user.email=a@b -c user.name=t commit -qm init ) || { rm -rf "$T"; echo "ENV: 临时仓建不起来"; return 3; }
  # 注意：git 不跟踪空目录 ⇒ 必须放文件，否则 status --ignored 不列
  mkdir -p "$T/.buildcache/k" "$T/examples/x/build"
  : > "$T/.buildcache/k/o"; : > "$T/examples/x/build/out.bin"

  local BL; BL=$(mktemp /tmp/treebase.XXXXXX)   # 不能放仓内：它自己会变成未跟踪文件

  # N1：基线为空 ⇒ 当前被忽略项全部「账外」⇒ 必红
  : > "$BL"
  if check_repo "$T" 0 "$BL" >/dev/null 2>&1; then echo "❌ N1 空账未见红"; rc=2
  else echo "✅ N1 账外忽略项 ⇒ 红（C2 有牙）"; fi

  # N2：把当前忽略项入账 ⇒ 不红
  ignored_of "$T" > "$BL"
  if check_repo "$T" 0 "$BL" >/dev/null 2>&1; then echo "✅ N2 已入账 ⇒ 不红（账生效）"
  else echo "❌ N2 入账后仍红"; check_repo "$T" 0 "$BL" | sed 's/^/     /'; rc=2; fi

  # N3：未跟踪文件 ⇒ C1 红
  : > "$T/untracked.txt"
  if check_repo "$T" 0 "$BL" >/dev/null 2>&1; then echo "❌ N3 未跟踪文件未见红"; rc=2
  else echo "✅ N3 未跟踪文件 ⇒ C1 红"; fi
  rm -f "$T/untracked.txt"

  # N4：--release ⇒ examples 构建残渣必红
  if check_repo "$T" 1 "$BL" >/dev/null 2>&1; then echo "❌ N4 --release 未拦住"; rc=2
  else echo "✅ N4 --release 拦住 examples 构建残渣"; fi

  # ── M446（缺陷 945）新增两条负控：豁免必须**既生效又不过宽** ──
  # N5：账外但属 .gitignore 明示的示例构建残渣 ⇒ 日常档不红、--release 仍红
  local T2 BL2; T2=$(mktemp -d /tmp/treeclean2.XXXXXX); BL2=$(mktemp /tmp/treebase2.XXXXXX)
  ( cd "$T2" && git init -q . && printf 'examples/*/build/\n' > .gitignore \
    && mkdir -p examples/x && printf 'src\n' > examples/x/a.px \
    && git add .gitignore examples/x/a.px \
    && git -c user.email=a@b -c user.name=t commit -qm init \
    && mkdir -p examples/x/build && : > examples/x/build/out.bin ) >/dev/null 2>&1 \
    || { echo "ENV: N5 临时仓建不起来"; rm -rf "$T2" "$BL2"; return 3; }
  : > "$BL2"
  if check_repo "$T2" 0 "$BL2" >/dev/null 2>&1; then
    echo "✅ N5 明示的示例构建残渣未入账 ⇒ 日常档不红（豁免生效）"
  else
    echo "❌ N5 构建残渣仍被判红（豁免没生效）"; check_repo "$T2" 0 "$BL2" | sed 's/^/     /'; rc=2
  fi
  if check_repo "$T2" 1 "$BL2" >/dev/null 2>&1; then
    echo "❌ N5b --release 未拦住残渣（豁免漏到发布档）"; rc=2
  else
    echo "✅ N5b --release 仍拦住残渣（豁免只放宽日常档）"
  fi
  # N6：账外且**非**构建残渣 ⇒ 仍红（证明豁免没有把真文件放行）
  ( cd "$T2" && printf 'junk/\n' >> .gitignore && mkdir -p junk && : > junk/o \
    && git add .gitignore && git -c user.email=a@b -c user.name=t commit -qm gitignore ) >/dev/null 2>&1
  if check_repo "$T2" 0 "$BL2" >/dev/null 2>&1; then
    echo "❌ N6 非残渣账外项未见红（豁免过宽）"; rc=2
  else
    echo "✅ N6 非残渣账外项 ⇒ 仍红（豁免不过宽）"
  fi
  rm -rf "$T2"; rm -f "$BL2"

  rm -rf "$T"; rm -f "$BL"
  [ $rc = 0 ] && echo "SELFTEST-OK" || echo "SELFTEST-FAIL"
  return $rc
}

case "$MODE" in
  selftest) selftest; exit $? ;;
  emit)     ignored_of "$ROOT" > "$BASELINE"; echo "已写 $BASELINE（$(wc -l < "$BASELINE") 条）"; exit 0 ;;
  release)  check_repo "$ROOT" 1 "$BASELINE"; exit $? ;;
  *)        check_repo "$ROOT" 0 "$BASELINE"; exit $? ;;
esac
