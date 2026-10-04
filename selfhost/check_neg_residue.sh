#!/usr/bin/env bash
# ============================================================
# selfhost/check_neg_residue.sh —— 负控残留守卫（工作树卫生）
# ------------------------------------------------------------
# 背景（缺陷 448 · M254 实测）：
#   `rebake_bin.sh` 的 `check_neg_residue()` 扫描面写死
#   `--include='*.px' --include='*.c'` ⇒ **漏掉 `.h` / `.py` / `.sh` / `.go`**。
#   而门内并行执行器 `selfhost/gate_par.py` 正是 `m244_gate_parallel` 负控的**打桩目标**
#   ⇒ 门被中断（SIGKILL / 工具层终止 ⇒ trap 不执行）时残留**逃过自检**；
#   若此时重烘，就会把「恒串行」的执行器行为带进后续判据。
#   本轮实测：工作树里真的有 `gate_par.py` 的 `NEGCTL-M244-B` 残留，
#   是我用 `git status` 肉眼发现的 —— **说明守卫有覆盖缺口**。
#
# 判据（四条）：
#   ① **扫描面**（selfhost/ runtime/ tools/ stdlib/ 的 .px .c .h .py .sh .go）
#      出现负控标记（`_m<N>_neg` / `NEGCTL` / `__NEG`）⇒ 判红并**指名文件**；
#   ② **白名单**：**会写下标记的东西**（运行器 / 守卫自身，注释里要写标记名）允许含标记 ——
#      但必须**逐条登记理由**；
#   ③ **过期判据**：白名单里每一条**必须仍命中**（文件被删/改名/标记被清 ⇒ 判红）
#      —— 防「豁免表越积越大、没人复核」；
#   ④ **规模锚点**：白名单 ≤ 8 条 · 扫描面文件数 ≥ 200 ⇒ 防「扫描器静默失效」。
#
# ⚠️ 单一事实源：`rebake_bin.sh` 的 `check_neg_residue()` **调用本脚本**，不再各写一份。
# 用法：
#   bash selfhost/check_neg_residue.sh              # 检查当前仓库
#   bash selfhost/check_neg_residue.sh --self-test  # 判据自证（fixture，不碰仓库）
#   bash selfhost/check_neg_residue.sh --root DIR --exempt FILE [--min-files N] [--max-exempt N]
# 退出码：0 = 干净（或自证全过）；1 = 有残留/过期；2 = 环境错
# ============================================================
set -uo pipefail

SELF_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT=""; EXEMPT=""; SELFTEST=0; MIN_FILES=200; MAX_EXEMPT=8
while [ $# -gt 0 ]; do
  case "$1" in
    --root)       ROOT="${2:-}"; shift 2 ;;
    --exempt)     EXEMPT="${2:-}"; shift 2 ;;
    --min-files)  MIN_FILES="${2:-}"; shift 2 ;;
    --max-exempt) MAX_EXEMPT="${2:-}"; shift 2 ;;
    --self-test)  SELFTEST=1; shift ;;
    -h|--help) sed -n '2,32p' "$0"; exit 0 ;;
    *) echo "❌ 未知参数: $1" >&2; exit 2 ;;
  esac
done
ROOT="${ROOT:-$(cd "$SELF_DIR/.." && pwd)}"
EXEMPT="${EXEMPT:-$SELF_DIR/neg_residue_exempt.txt}"

MARK_RE='_m[0-9]+_neg|NEGCTL|__NEG'
DIRS="selfhost runtime tools stdlib"
EXTS=(px c h py sh go)

# 扫描一次，输出「命中文件的相对路径」（每行一条）
scan_hits() { # $1=root
  local r="$1" e d
  local incs=() dirs=()
  for e in "${EXTS[@]}"; do incs+=("--include=*.${e}"); done
  for d in $DIRS; do [ -d "$r/$d" ] && dirs+=("$r/$d"); done
  [ "${#dirs[@]}" -gt 0 ] || return 2
  grep -rlE "$MARK_RE" "${incs[@]}" "${dirs[@]}" 2>/dev/null | sed "s#^$r/##" | sort || true
}

count_files() { # $1=root
  local r="$1" e d n=0 c
  for d in $DIRS; do
    [ -d "$r/$d" ] || continue
    for e in "${EXTS[@]}"; do
      c="$(find "$r/$d" -type f -name "*.$e" 2>/dev/null | wc -l || true)"
      n=$((n + c))
    done
  done
  printf '%d\n' "$n"
}

# ---------- 自证 ----------
if [ "$SELFTEST" = 1 ]; then
  W="$(mktemp -d /tmp/negr.XXXXXX)"; trap 'rm -rf "$W"' EXIT
  P=0; F=0
  mk() { mkdir -p "$W/$(dirname "$1")"; printf '%s\n' "$2" > "$W/$1"; }
  for i in $(seq 1 30); do mk "selfhost/f$i.px" "let x$i = 1"; done
  mk "runtime/runtime.c" "int main(void){return 0;}"
  mk "tools/px" "#!/bin/sh"
  : > "$W/empty.txt"
  chk() { # $1=名 $2=期望rc $3=必须含(可空) $4=root $5=exempt ...额外参数
    local name="$1" want="$2" key="$3" r="$4" ex="$5"; shift 5
    local out rc
    out="$(bash "$0" --root "$r" --exempt "$ex" "$@" 2>&1)"; rc=$?
    if [ "$rc" = "$want" ] && { [ -z "$key" ] || printf '%s' "$out" | grep -q "$key"; }; then
      echo "✅ $name（rc=$rc）"; P=$((P+1))
    else
      echo "❌ $name（rc=$rc 期望 $want；输出: $(printf '%s' "$out" | tr '\n' ' ')）"; F=$((F+1))
    fi
  }
  chk "S1 干净树 → 绿" 0 "NEG-RESIDUE-OK" "$W" "$W/empty.txt" --min-files 5

  # 缺陷 448 的四个「逃逸面」逐个注入（旧扫描面只看 .px/.c ⇒ 这四个全漏）
  for e in py sh h go; do
    mk "selfhost/inj.$e" "# NEGCTL-M244-B：注入"
    chk "S2 注入 .$e 残留 → 判红并指名" 1 "selfhost/inj.$e" "$W" "$W/empty.txt" --min-files 5
    rm -f "$W/selfhost/inj.$e"
  done
  mk "selfhost/inj2.px" "let _m160_negB = 1"
  chk "S3 .px 的 _m<N>_neg 形态 → 判红" 1 "selfhost/inj2.px" "$W" "$W/empty.txt" --min-files 5
  rm -f "$W/selfhost/inj2.px"

  mk "selfhost/wl.sh" "# NEGCTL 说明"
  printf 'selfhost/wl.sh\t运行器注释\n' > "$W/ex1.txt"
  chk "S4 白名单命中 → 绿（豁免生效）" 0 "NEG-RESIDUE-OK" "$W" "$W/ex1.txt" --min-files 5
  printf 'selfhost/nope.sh\t不存在的文件\n' > "$W/ex2.txt"
  chk "S5 白名单登记却不命中 → 判红（过期）" 1 "过期" "$W" "$W/ex2.txt" --min-files 5
  printf 'selfhost/wl.sh\n' > "$W/ex3.txt"
  chk "S6 白名单条目**缺理由** → 判红" 1 "缺理由" "$W" "$W/ex3.txt" --min-files 5
  rm -f "$W/selfhost/wl.sh"

  # 规模锚点自己有牙（否则「扫描器静默变空」无人发现）
  chk "S7 扫描面文件数不足 → 判红（规模锚点）" 1 "扫描器可能失效" "$W" "$W/empty.txt" --min-files 99999
  mk "selfhost/many.sh" "# NEGCTL"
  printf 'selfhost/many.sh\t理由\nselfhost/many2.sh\t理由\n' > "$W/ex4.txt"
  chk "S8 豁免表超限 → 判红" 1 "规模超限" "$W" "$W/ex4.txt" --min-files 5 --max-exempt 1

  echo "== 自证：通过 $P / 失败 $F =="
  [ "$F" = 0 ] || exit 1
  exit 0
fi

# ---------- 主判据 ----------
[ -d "$ROOT" ] || { echo "❌ --root 不存在: $ROOT" >&2; exit 2; }
[ -f "$EXEMPT" ] || { echo "❌ 找不到豁免表: $EXEMPT" >&2; exit 2; }

mapfile -t RAW < <(scan_hits "$ROOT")

declare -A EX=()
exn=0; exp_bad=0
while IFS= read -r line; do
  case "$line" in ''|'#'*) continue ;; esac
  f="${line%%$'\t'*}"
  reason="${line#*$'\t'}"
  [ "$reason" = "$line" ] && reason=""
  f="${f%%[[:space:]]*}"
  [ -n "$f" ] || continue
  if [ -z "$reason" ]; then
    echo "❌ 豁免表条目缺理由（TAB 分隔）：$f" >&2; exp_bad=1; continue
  fi
  EX["$f"]="$reason"; exn=$((exn+1))
done < "$EXEMPT"

RES=(); EXHIT=()
for p in "${RAW[@]:-}"; do
  [ -n "$p" ] || continue
  if [ -n "${EX[$p]:-}" ]; then EXHIT+=("$p"); else RES+=("$p"); fi
done

nf="$(count_files "$ROOT")"
fail=0

[ "$exp_bad" = 1 ] && fail=1

if [ "${#RES[@]}" -gt 0 ]; then
  echo "❌ 源码里发现**负控残留标记**（$MARK_RE）："
  printf '%s\n' "${RES[@]}" | sed 's/^/     /'
  echo "   ⇒ 处置：git checkout -- <文件> 还原；确认无残留再重烘/提交。"
  fail=1
fi

for f in "${!EX[@]}"; do
  hit=0
  for h in "${EXHIT[@]:-}"; do [ "$h" = "$f" ] && hit=1; done
  if [ "$hit" = 0 ]; then
    echo "❌ 豁免表**过期**：$f 已不再命中（文件被删/改名，或标记已被清）"
    fail=1
  fi
done

if [ "$exn" -gt "$MAX_EXEMPT" ]; then
  echo "❌ 豁免表规模超限：$exn > $MAX_EXEMPT（豁免必须少而显式，请复核）"; fail=1
fi
if [ "$nf" -lt "$MIN_FILES" ]; then
  echo "❌ 扫描面文件数 $nf < $MIN_FILES ⇒ 扫描器可能失效（判据不许静默变空）"; fail=1
fi

[ "$fail" = 0 ] || { echo "   （豁免表 $EXEMPT：$exn 条 · 扫描面 $nf 文件）"; exit 1; }
echo "✅ NEG-RESIDUE-OK（无残留 · 豁免 $exn/$MAX_EXEMPT · 扫描面 $nf 文件）"
