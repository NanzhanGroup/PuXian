#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# 口径 19 的第二通道：判据里的「生产者 | grep -q」棘轮守卫
#
# 为什么：pipefail 下 `producer | grep -q PAT` 是**假否定**制造机 ——
#   grep 一命中就退出 ⇒ 生产者后续 write 拿 EPIPE ⇒ 生产者 rc=141 ⇒
#   管线被 pipefail 判成失败 ⇒ 「命中」被读成「不含」。
#   出处：缺陷 621（m278）· 622（selftest_latest_guard）· 624（m286）。
#   机制实测：bash 的 printf '%s' 对 672 字节输出做了 19 次 write（strace）
#   ⇒ 「小输出不会撞 SIGPIPE」是错的，任何生产者都可能中招。
#
# 扫描口径（M356 修 · 缺陷「注释也算残留」）：**整行注释不计入**。
#   原实现 `grep -cE '| grep -q'` 不排除 `^[[:space:]]*#` ⇒ M355 基线 120 条里
#   有 24 条是注释里的历史说明（如「缺陷 487 tar -tzf … | grep -q」）——
#   后果 twofold：① 残留数虚高 20%（把文档当债务）；② **棘轮可被规避**：
#   删一行注释就能降指标，门变「更绿」而判据没修（口径 19 的反面：假绿）。
#   现 scan_one/scan_one_file/--list 三处同口径（不得两套算法），自证夹具 E 守它。
#   仍未覆盖（待判，不猜）：单引号字符串里的**夹具文本**（如 printf '… | grep -q …'）
#
# 正确形态：`grep -q PAT <<< "$var"`（here-string，无写端）、
#           或 `[ -n "$(producer)" ]` / `grep -q PAT <file>`。
#
# 本守卫是**棘轮**：基线 selfhost/pipe_grep_baseline.txt 记「每个文件允许的
# 残留条数」；只许往下减，不许新增/增加。修好一个就 `--emit-baseline` 重生成。
#
# 用法：
#   check_pipe_grep.sh                 扫全仓（基线比对）· rc=0 通过 / 1 有新违规
#   check_pipe_grep.sh --self-test     三档夹具自证（检定/不检定/文件输入）
#   check_pipe_grep.sh --emit-baseline 打印当前分布（重生成基线用）
#   check_pipe_grep.sh --list          逐条列出残留（含行号）
# 注意：本脚本自身也必须守规矩 —— 里面**不得**出现生产者接 grep -q。
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BASE="$ROOT/selfhost/pipe_grep_baseline.txt"
# 只看会被 pipefail 咬到的文件：含 pipefail 的 .sh
scan_one() {   # $1=文件路径（相对 ROOT）→ 输出条数
  local f="$1" n=0
  grep -q 'pipefail' "$ROOT/$f" 2>/dev/null || { echo 0; return 0; }
  n=$(grep -vE '^[[:space:]]*#' "$ROOT/$f" 2>/dev/null | grep -cE '\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q' || true)
  echo "${n:-0}"
}
list_files() {
  cd "$ROOT" || exit 9
  { find examples packaging selfhost tools -name '*.sh' -type f 2>/dev/null; } | LC_ALL=C sort
}

emit_baseline() {
  cd "$ROOT" || exit 9
  local total=0 f n
  printf '# 口径 19 棘轮基线：每个 .sh 允许残留的「生产者 | grep -q」条数（只许减少）\n'
  printf '# 生成：selfhost/check_pipe_grep.sh --emit-baseline · 里程碑 M355\n'
  while IFS= read -r f; do
    n=$(scan_one "$f")
    [ "$n" -gt 0 ] && { printf '%s %s\n' "$n" "$f"; total=$((total+n)); }
  done < <(list_files)
  printf '# 残留合计 %s 条\n' "$total"
}

self_test() {
  local tmp rc fails=0
  tmp=$(mktemp -d) || exit 9
  trap 'rm -rf "${tmp:-/nonexistent-m355}"' EXIT
  # 夹具 A：生产者接 grep -q ⇒ 必须被检出
  printf '#!/bin/bash\nset -uo pipefail\nprintf "%%s" "$X" | grep -q PAT\n' > "$tmp/a.sh"
  # 夹具 B：here-string ⇒ 不得检出
  printf '#!/bin/bash\nset -uo pipefail\ngrep -q PAT <<< "$X"\n' > "$tmp/b.sh"
  # 夹具 C：文件输入（无生产者）⇒ 不得检出
  printf '#!/bin/bash\nset -uo pipefail\ngrep -q PAT /etc/hostname\n' > "$tmp/c.sh"
  # 夹具 D：无 pipefail 的文件 ⇒ 不纳入扫描面（pipefail 未开时无此假否定）
  printf '#!/bin/bash\necho x | grep -q PAT\n' > "$tmp/d.sh"
  # 夹具 E（负控 · M356）：整行注释里的同一形态 ⇒ **不得**检出（否则删注释就能降棘轮）
  printf '#!/bin/bash\nset -uo pipefail\n# 历史说明：缺陷 487 `tar -tzf x | grep -q y` 会撞 SIGPIPE\n  #   缩进注释 echo hi | grep -q hi\necho ok\n' > "$tmp/e.sh"
  local ra rb rc rd re
  ra=$(scan_one_file "$tmp/a.sh"); rb=$(scan_one_file "$tmp/b.sh")
  rc=$(scan_one_file "$tmp/c.sh"); rd=$(scan_one_file "$tmp/d.sh")
  re=$(scan_one_file "$tmp/e.sh")
  [ "$ra" -eq 1 ] && echo "  ✅ A 生产者接 grep -q ⇒ 检出 1" || { echo "  ❌ A 期望 1，实际 $ra"; fails=$((fails+1)); }
  [ "$rb" -eq 0 ] && echo "  ✅ B here-string ⇒ 不检出" || { echo "  ❌ B 期望 0，实际 $rb"; fails=$((fails+1)); }
  [ "$rc" -eq 0 ] && echo "  ✅ C 文件输入 ⇒ 不检出" || { echo "  ❌ C 期望 0，实际 $rc"; fails=$((fails+1)); }
  [ "$rd" -eq 0 ] && echo "  ✅ D 无 pipefail ⇒ 不在扫描面" || { echo "  ❌ D 期望 0，实际 $rd"; fails=$((fails+1)); }
  [ "$re" -eq 0 ] && echo "  ✅ E 整行注释 ⇒ 不检出（防「删注释降指标」的规避）" || { echo "  ❌ E 期望 0，实际 $re"; fails=$((fails+1)); }
  echo "  自证：通过 $((5-fails)) / 失败 $fails"
  [ "$fails" -eq 0 ]
}

scan_one_file() {   # 自证用：绝对路径
  local f="$1" n
  grep -q 'pipefail' "$f" 2>/dev/null || { echo 0; return 0; }
  n=$(grep -vE '^[[:space:]]*#' "$f" 2>/dev/null | grep -cE '\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q' || true)
  echo "${n:-0}"
}

main() {
  case "${1:-}" in
    --self-test) self_test; exit $? ;;
    --gate)
      # 门用入口：先装置自证，再棘轮扫描（两段都过才算绿）
      self_test || { echo "PIPE-GREP-VERDICT=PREMISE-FAIL（装置自证红，非产品红）"; exit 3; }
      shift; main_scan; exit $? ;;
    --emit-baseline) emit_baseline; exit 0 ;;
    --list)
      cd "$ROOT" || exit 9
      while IFS= read -r f; do
        n=$(scan_one "$f"); [ "$n" -gt 0 ] || continue
        grep -nE '\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q' "$ROOT/$f" | grep -vE '^[0-9]+:[[:space:]]*#' | sed "s|^|$f:|"
      done < <(list_files)
      exit 0 ;;
  esac
  main_scan
}

main_scan() {
  [ -f "$BASE" ] || { echo "❌ 缺基线 $BASE"; exit 1; }
  cd "$ROOT" || exit 9
  local np=0 bad=0 total=0 f n allow
  while IFS= read -r f; do
    n=$(scan_one "$f"); [ "$n" -gt 0 ] || continue
    total=$((total+n)); np=$((np+1))
    allow=$(awk -v k="$f" '$2==k{print $1; exit}' "$BASE"); allow=${allow:-0}
    if [ "$n" -gt "$allow" ]; then
      printf '  ❌ %s：%s 条 > 基线 %s（新增 %s）\n' "$f" "$n" "$allow" "$((n-allow))"
      bad=$((bad+1))
    fi
  done < <(list_files)
  printf '  扫描面：含 pipefail 的 .sh，命中文件 %s 个 · 残留合计 %s 条\n' "$np" "$total"
  if [ "$bad" -eq 0 ]; then
    echo "  棘轮：无新增（基线内）"
    echo "PIPE-GREP-VERDICT=OK remaining=$total files=$np"
    exit 0
  fi
  echo "PIPE-GREP-VERDICT=REGRESS files_over_baseline=$bad"
  exit 1
}
main "$@"
