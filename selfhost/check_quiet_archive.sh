#!/bin/bash
# ══════════════════════════════════════════════════════════════════════
# selfhost/check_quiet_archive.sh —— 门 `quiet_archive`（M383 · 2/3）
#
# 守的是 quiet 档**归档器**（packaging/chain/quiet_archive.py）的两条契约：
#   A1 归档器自身的判据成立（--selftest）：慢门进并行档 / 快门留独占 /
#      独占白名单不被慢门顶掉 / 无实测保守留独占 / 装箱均分 —— 且**两条负控必须红**
#      （负控 A：把慢门改快 ⇒ 必须回独占；负控 B：把快门改慢 ⇒ 必须进并行）。
#   A2 现场归档不漂移（--check）：宿主有 quiet_lanes.txt 时，**先按判据挑实测源**
#      （只接受覆盖全部 quiet 门的表；残表＝限范围探针一律拒）再重算，不一致 ⇒ rc=3。
#      没有宿主数据（CI / 新机器 / 只有探针）⇒ 只跑 A1/A3，A2 明确 SKIP。
#
# 为什么「归档」要立判据（而不是人手一张表）：
#   本轮实测（2026-11-09）：quiet 尾 16 门 2428s，谁是瓶颈每轮都在变（M382 第四轮
#   top 是 m244 514s / m302 285s / m291 228s）。手拍一张表 ⇒ 下一轮实测变了没人知道，
#   表格变成历史文件。判据必须是「按实测重算 ⇒ 与落盘一致」。
#
# 负控在哪：quiet_archive.py --selftest 内置两条（改时间 ⇒ 档位必须变），
#   本门若被改成「只有正控」，selftest 会少两行 ✅ ⇒ 断言数对不上 ⇒ 判红。
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$(cd "$HERE/.." && pwd)"
ARC="$SRC/packaging/chain/quiet_archive.py"
BASE="${PX_GATEVM:-/data/gatevm}"
FAIL=0
ok(){  printf '  ✅ %s\n' "$1"; }
bad(){ printf '  ❌ %s\n' "$1"; FAIL=$((FAIL+1)); }

[ -f "$ARC" ] || { echo "❌ 找不到归档器 $ARC"; exit 1; }
command -v python3 >/dev/null 2>&1 || { echo "⚠️ 无 python3 ⇒ SKIP"; exit 0; }

echo "=== A1 归档器自证（含两条负控）==="
out="$(python3 "$ARC" --selftest 2>&1)"; rc=$?
printf '%s\n' "$out" | sed 's/^/  /'
nok=$(printf '%s\n' "$out" | grep -c '^  ✅' || true)
[ "$rc" -eq 0 ] && ok "selftest rc=0" || bad "selftest rc=$rc（归档判据不成立）"
[ "$nok" -eq 7 ] && ok "断言数 7（含 2 条负控）" || bad "断言数 $nok ≠ 7（负控被删/被改？）"

echo "=== A2 现场归档一致性（实测源先按判据挑 · 缺陷 678）==="
# 旧口径（已废）：`ls -dt shard-*/quiet.times.tsv | head -1` = 取最新 ⇒ 会把**限范围探针**的表
#   当全量实测用。2026-11-09 14:18 我跑了一次 4 门探针（gates.txt 只有 4 个 quiet 门）验证车道
#   并行性，shard_run 照常写出 4 行 quiet.times.tsv ⇒ 校验器拿它重算 16 门 ⇒ 12 门被判「无实测
#   ⇒ 保守独占」⇒ 假报「归档漂移」。同一份归档 --write 说 8 门 3422s / --check 说 17s。
# 新口径：只接受**覆盖全部 quiet 门**的表（残表一律拒），并按新→旧挑。
PICK="$(python3 "$ARC" --pick --base "$BASE" --quiet "$BASE/quiet_needed.txt" 2>&1)"; prc=$?
PICKED="$(printf '%s\n' "$PICK" | sed -n 's/^PICK-FILE=//p')"
REJN="$(printf '%s\n' "$PICK" | sed -n 's/^PICK-REJECTED-TOTAL=//p')"
STAMPED="$(printf '%s\n' "$PICK" | sed -n 's/^PICK-STAMP=//p')"
printf '%s\n' "$PICK" | sed 's/^/  /'
if [ "$prc" -ne 0 ] || [ -z "$PICKED" ]; then
  echo "  ⚠ SKIP：宿主没有「覆盖全部 quiet 门」的实测表（被拒残表 ${REJN:-0} 份）"
  echo "     限范围探针 / CI / 新机器都属此例 —— 没实测就不许断言归档"
elif [ ! -f "$BASE/quiet_lanes.txt" ]; then
  echo "  ⚠ SKIP：缺 $BASE/quiet_lanes.txt（未归档过的机器正常）"
else
  echo "  实测来源：$PICKED（完成戳 ${STAMPED:-no}）"
  if python3 "$ARC" --check --times "$PICKED" \
        --quiet "$BASE/quiet_needed.txt" --lanes "$BASE/quiet_lanes.txt" \
        --exclusive "$BASE/quiet_exclusive.txt" 2>&1 | sed 's/^/  /'; then
    ok "归档不漂移（QUIET-ARCHIVE-OK）"
  else
    bad "归档漂移 ⇒ 跑 packaging/chain/quiet_archive.py --write 落盘（并写进 CHANGELOG）"
  fi
fi

echo "=== A3 选源判据自证（缺陷 678：残表会伪造漂移）==="
out3="$(python3 "$ARC" --selftest-pick 2>&1)"; rc3=$?
printf '%s\n' "$out3" | sed 's/^/  /'
nok3=$(printf '%s\n' "$out3" | grep -c '^  ✅' || true)
[ "$rc3" -eq 0 ] && ok "selftest-pick rc=0" || bad "selftest-pick rc=$rc3（选源判据不成立）"
[ "$nok3" -eq 7 ] && ok "选源断言数 7（含 4 条负控）" || bad "选源断言数 $nok3 ≠ 7（负控被删/被改？）"

if [ "$FAIL" -ne 0 ]; then
  echo "QUIET-ARCHIVE-FAIL（$FAIL 项）"
  exit 1
fi
echo "QUIET-ARCHIVE-OK"
