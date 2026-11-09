#!/bin/bash
# ══════════════════════════════════════════════════════════════════════
# packaging/chain/keep_gate_logs.sh —— 把一轮门日志**按链**留存（M383 · 3/3）
#
# 病灶（缺陷 679）：
#   run_gates.sh 把每个门的日志写在 `/tmp/gate_<name>.log`；分片侧 /tmp 被
#   bind 成 `$BASE/tmp$i`，而 tmp$i **每轮 rm -rf 重建**（见 shard_run.sh
#   prep_worktrees 的锁文件注释）⇒ 上一轮失败门的现场被下一轮覆盖。
#   M382 第三轮 m256 偶发红的现场就是这样被第四轮抹掉的，无法回查。
#
# 口径：
#   · 每链（chain-ts）一条留存目录 `$LOG/gatelog/tmpN` 与 `$LOG/gatelog/quiet`；
#   · 红 / 超时门再单独拷一份到 `$LOG/red/<label>_gate_<name>.log`（一眼能捞）；
#   · 门名与状态**以 times.tsv 为准**（不解析人类可读日志）：status ∉ {OK,SKIP}
#     即视为现场，必须留。
#   · 本脚本只拷贝，不改任何门行为；源目录不存在/零日志 ⇒ **响亮报错**（rc=3），
#     不许静默成功 —— 590 族纪律「判据通道没了就当默认值」的反面。
#
# 用法：
#   keep_gate_logs.sh <src_dir> <dest_dir> <times.tsv> <red_dir> <label>
# 退出码：0 = 已留存（≥1 份）· 3 = 零份（现场已丢，需查）· 4 = 参数错
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail

SRC="${1:-}"; DEST="${2:-}"; TSV="${3:-}"; RED="${4:-}"; LABEL="${5:-}"
if [ -z "$SRC" ] || [ -z "$DEST" ] || [ -z "$TSV" ] || [ -z "$RED" ] || [ -z "$LABEL" ]; then
  echo "用法：keep_gate_logs.sh <src_dir> <dest_dir> <times.tsv> <red_dir> <label>" >&2
  exit 4
fi

mkdir -p "$DEST" || { echo "❌ 留存目标不可建：$DEST" >&2; exit 3; }

N=0
for f in "$SRC"/gate_*.log; do
  [ -f "$f" ] || continue
  cp -f "$f" "$DEST/" && N=$((N+1))
done

# ── 红/超时门：以 times.tsv 的状态列为准 ──
N_RED=0; N_RED_MISS=0; REDLIST=""
if [ -f "$TSV" ]; then
  while IFS=$'\t' read -r g st _rest; do
    [ -n "${g:-}" ] || continue
    case "$st" in OK|SKIP) continue ;; esac
    src="$DEST/gate_$g.log"
    if [ -f "$src" ]; then
      mkdir -p "$RED" && cp -f "$src" "$RED/${LABEL}_gate_$g.log" && N_RED=$((N_RED+1))
      REDLIST="$REDLIST $g($st)"
    else
      N_RED_MISS=$((N_RED_MISS+1))
      REDLIST="$REDLIST $g($st:无日志)"
    fi
  done < "$TSV"
fi

echo "  门日志留存[$LABEL]：$N 份 → $DEST"
if [ -n "${REDLIST// /}" ]; then
  echo "    ⚠ 非绿门${REDLIST} ⇒ 另存 $RED/${LABEL}_gate_*.log（缺日志 $N_RED_MISS 个）"
fi

if [ "$N" -eq 0 ]; then
  echo "  ❌ 门日志留存[$LABEL]：0 份（源 $SRC 里没有 gate_*.log ⇒ 本轮现场无从查）" >&2
  exit 3
fi
exit 0
