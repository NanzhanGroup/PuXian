#!/usr/bin/env bash
# ============================================================
# M409 门 · **容器生命周期**（缺陷 267：mark 之后仍在执行的对象不得被本轮回收）
# ------------------------------------------------------------
# 病灶形态（/data/autodrive/M409-S3-EVIDENCE.md）：某线程**提前挣脱暂停**（handler 6s 自旋上限
#   到点 break）后恢复用户态、继续持有/改写自己的容器，而本轮 mark 的根面快照里没有它
#   ⇒ sweep 把**仍存活的容器**回收复用 ⇒ `[M257-CTR]` / 崩溃。
# 处置（S3b/S3d）：判据 =「自上一次 sweep 以来发生过挣脱」⇒ 本轮**放弃 sweep**（只清 mark 位）。
#
# 本门三层牙（缺一即为假绿）：
#   N1 **前提自证**：修后档必须打出 `[M409-STAT] 提前挣脱>0` **且** `因挣脱放弃 sweep>0`
#      ——否则「注入/挣脱根本没发生」，后面的绿毫无意义（PRE-MISE-FAIL，响亮但不算产品绿）。
#   N2 **负控**：`PX_GC_EARLY_ABORT=0`（只关处置，其余参数逐字相同）⇒ **必须红**
#      （`[M257-CTR]` / BAD>0 / 未 DONE）。若负控也绿 ⇒ 装置没牙 ⇒ 判红（不是绿）。
#   N3 **反假绿**：修后档 `M409-PROBE-DONE` + `BAD=0` + 无 `[M257-CTR]` + 无信号终止，
#      且**连跑 2 次**（判据不能是一次性 flake）。
#
# 驱动器：examples/m409_gc_container_lifetime/probe_sig.px（信号处理器线程 = 真注册 OS 线程，
#   处理器内的容器只挂在 ti->vm_state / TLS 根栈上 —— 只有「本轮 paused」才被扫）。
#   ⚠️ v3 协程版（probe_lifetime.px）**证不了**这事：`px_coro_gc_mark_roots` 无条件标全部
#      存活协程帧槽 ⇒ 未暂停也被标 ⇒ 负控恒绿（假绿）。文件头有实测记录。
#
# 用法：verify.sh [--neg-skip] [--green-runs N]
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.." || exit 1
ROOT=$(pwd)
DIR=examples/m409_gc_container_lifetime
BIN=$DIR/build/probe_sig
NEG_SKIP=0
GREEN_RUNS=2
while [ $# -gt 0 ]; do
  case "$1" in
    --neg-skip) NEG_SKIP=1 ;;
    --green-runs) shift; GREEN_RUNS=${1:-2} ;;
    *) echo "未知参数：$1" >&2; exit 2 ;;
  esac
  shift
done

fails=0
say()  { echo "── $*"; }
ok()   { echo "  PASS  $*"; }
bad()  { echo "  FAIL  $*"; fails=$((fails+1)); }

say "[0] 构建探针（$DIR/probe_sig.px）"
if ! tools/px build "$DIR/probe_sig.px" >/tmp/m409_gate_build.log 2>&1; then
  bad "编译失败：tail -5 /tmp/m409_gate_build.log"
  sed -n '1,5p' /tmp/m409_gate_build.log
  echo; echo "M409-VERIFY-FAIL（编译）"; exit 1
fi
ok "编译成功"
[ -x "$BIN" ] || { bad "$BIN 不存在"; echo "M409-VERIFY-FAIL"; exit 1; }

# 公共参数：只差 PX_GC_EARLY_ABORT（N2 的自伤检查口径）
COMMON_ENV=(PX_GC_THRESHOLD=100000000 PX_GC_FORCE_EARLY_RESUME=1
            PX_GC_M409_HOLD_US=50000 PX_GC_M409_HOLD_N=6
            M409_PROBE_ROUNDS=6)

run_probe() {  # $1=output file, 其余=额外 env
  local out=$1; shift
  timeout 120 env "${COMMON_ENV[@]}" "$@" "$BIN" >"$out" 2>&1
  echo $?
}

say "[1] 修后档 ×$GREEN_RUNS（处置开 = 默认）+ N1 前提自证"
g=0
while [ $g -lt "$GREEN_RUNS" ]; do
  g=$((g+1))
  out=/tmp/m409_gate_green$g.out
  rc=$(run_probe "$out")
  txt=$(tr -d '\000' <"$out")
  if [ "$rc" != 0 ]; then bad "green#$g rc=$rc（期望 0）"; fi
  if ! grep -q 'M409-PROBE-DONE' <<<"$txt"; then bad "green#$g 未打 M409-PROBE-DONE"; fi
  if ! grep -q 'M409-PROBE-BAD=0' <<<"$txt"; then bad "green#$g BAD≠0（容器被改写）"; fi
  if grep -q 'M257-CTR' <<<"$txt"; then bad "green#$g 出现 [M257-CTR]"; fi
  stat=$(grep -a 'M409-STAT' <<<"$txt" | tail -1)
  er=$(sed -n 's/.*提前挣脱暂停=\([0-9]*\) 次.*/\1/p' <<<"$stat")
  ab=$(sed -n 's/.*因挣脱放弃 sweep=\([0-9]*\) 轮.*/\1/p' <<<"$stat")
  if [ -z "${er:-}" ] || [ "$er" -le 0 ]; then
    bad "green#$g PREMISE-FAIL：提前挣脱=$er（注入/挣脱没发生 ⇒ 这扇门在空转）"
  elif [ -z "${ab:-}" ] || [ "$ab" -le 0 ]; then
    bad "green#$g PREMISE-FAIL：放弃 sweep=$ab（处置没被触发 = 窗口没造出来）"
  else
    ok "green#$g 提前挣脱=$er · 放弃 sweep=$ab · BAD=0 · DONE"
  fi
done

if [ "$NEG_SKIP" = 1 ]; then
  say "[2] 负控 —— **已跳过**（--neg-skip；CI 用）"
else
  say "[2] 负控 PX_GC_EARLY_ABORT=0（关处置，其余参数逐字相同）⇒ **必须红**"
  out=/tmp/m409_gate_neg.out
  rc=$(run_probe "$out" PX_GC_EARLY_ABORT=0)
  txt=$(tr -d '\000' <"$out")
  red=0
  if grep -q 'M257-CTR' <<<"$txt"; then red=1; why="[M257-CTR] 存储已失效"; fi
  if ! grep -q 'M409-PROBE-DONE' <<<"$txt"; then red=1; why="${why:-未 DONE}"; fi
  if grep -q 'M409-PROBE-BAD=[1-9]' <<<"$txt"; then red=1; why="${why:-BAD>0}"; fi
  if [ "$rc" -ge 124 ]; then red=1; why="${why:-超时/被信号杀}"; fi
  if [ "$red" = 1 ]; then
    ok "负控按预期判红（$why）⇒ 装置有牙"
  else
    bad "负控**也绿**（rc=$rc，无 CTR、BAD=0、DONE）⇒ 装置没牙，这扇门的绿不算数"
    grep -a -E 'M409-PROBE|M409-STAT' <<<"$txt" | tail -3
  fi
fi

echo
if [ "$fails" -eq 0 ]; then
  echo "M409-VERIFY-OK（修后绿 + 前提自证 + 负控必红）"
  exit 0
else
  echo "M409-VERIFY-FAIL（$fails 处）"
  exit 1
fi
