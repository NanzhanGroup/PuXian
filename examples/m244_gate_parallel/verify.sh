#!/usr/bin/env bash
# ============================================================
# M244 门 · 「门内并行」（缺陷 412 + 一项性能改进）
# ------------------------------------------------------------
# 主题：全量门实测 **6997s / 154 门**，其中 12 个「矩阵对拍门」占 **3513s**；
#   而它们的耗时主体是**逐例 spawn 一个进程**的三重串行循环
#   （m227 = 197 例 × 2 面 × 3 轨 = **1182 次串行 spawn**）。
#   每次 spawn 之间**无依赖**（同一驱动器、不同 case 选择；独立进程、不写共享文件）
#   ⇒ 天生可并行。而门**之间**必须串行（`run_gates.sh` 的 PID 锁 —— 53 个门的负控
#   会改 `runtime.c` / `selfhost/*.px`，门间并行必然互相踩）⇒ **门内并行与那条护栏正交**。
#
# 判据：
#   [1] 静态：`targets.tsv` ⇄ **源码派生**（双向 · 漏登记/悬空都判红）· 规模锚点 ·
#             import 路径在位 · **负控锚点自证**（锚点漂了 ⇒ 负控静默失效，本仓撞过 4 次）
#             · 缺陷 412 静态侧
#   [2] 自证：`gate_par.py --selftest` 14 条（保序 / n=1⇄n=8 等价 / **真并发峰值** /
#             jobs() 非法值**响亮** / 异常**不吞** / 键重复响亮）
#   [3] 合成负载：64 任务 × 60ms ⇒ 串行 ⇄ 默认并发：**结果逐字节相同** + 加速比 ≥3.0
#   [4] 真实驱动：仓库内 `m231 drv.px` × 120 例（解释轨）⇒ **输出逐字节相同** + 加速比 ≥1.8
#   [5] 缺陷 412 动态侧：`--list` **不触碰**计时 TSV
#       ＋**对照**：真的跑门那条路径**必须**截断它（否则 [5] 是恒真的空判据）
#   [6] 负控 A（破坏保序 ⇒ [4] 必红）· B（忽略并发度 ⇒ [3] 加速比必红，而保序仍绿 ⇒ 两层独立）
#       · C（判据自伤 ⇒ A 的红必须消失）
#   [7] 覆盖边界（如实登记）
# CI 用 `--neg-skip`（负控要改 `selfhost/gate_par.py`）。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m244_gate_parallel
W=/tmp/m244_gate
rm -rf "$W"; mkdir -p "$W/neg"
cp -f selfhost/gate_par.py "$W/gp.orig"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --work "$W/neg" "$@"; }
trap 'NEGCTL --restore >/dev/null 2>&1' EXIT

echo "=== [1] 静态：清单 ⇄ 源码派生（双向）· 缺陷 412 静态侧"
python3 "$D/static_check.py" --root "$ROOT" --here "$D" > "$W/static.log" 2>&1
src=$?
sed 's/^/  /' "$W/static.log"
chk "[1] 静态判据 rc=0" "[ $src -eq 0 ]"
chk "[1] 清单 ⇄ 派生**精确相等**（漏登记 0 · 悬空 0）" "grep -q '清单 ⇄ 派生精确相等' $W/static.log"
chk "[1] 规模锚点（目标 ≥7）" "grep -q '规模锚点：目标 ≥7' $W/static.log"
chk "[1] 豁免自证（非空 + 不指向不存在的东西）" "! grep -q 'FAIL \\[A2\\]' $W/static.log"
chk "[1] 豁免自证确实被执行（日志含 [A2] 行）" "grep -q '\\[A2\\]' $W/static.log"
chk "[1] 每个目标：import + 调度调用在位" "! grep -q 'FAIL \[B\]' $W/static.log"
chk "[1] 每个目标：sys.path.insert(…selfhost)" "! grep -q 'FAIL \[C\]' $W/static.log"
chk "[1] 负控锚点自证（2 处各唯一）" "[ \$(grep -c '锚点唯一命中' $W/static.log) -eq 2 ]"
chk "[1] 缺陷 412 静态侧（截断在 --list 之后）" "grep -q 'TSV 截断在 --list 短路之后' $W/static.log"

echo "=== [2] 自证：gate_par.py --selftest"
python3 selfhost/gate_par.py --selftest > "$W/par.log" 2>&1
prc=$?
sed 's/^/  /' "$W/par.log"
chk "[2] 自证 rc=0" "[ $prc -eq 0 ]"
chk "[2] 自证 pass=14 fail=0" "grep -q 'GATE-PAR-SELFTEST-OK pass=14 fail=0' $W/par.log"

echo "=== [3][4] 动态：合成负载 + 真实驱动（串行 ⇄ 默认并发）"
python3 "$D/bench.py" --root "$ROOT" --n 120 > "$W/bench.log" 2>&1
brc=$?
sed 's/^/  /' "$W/bench.log"
chk "[3] 合成负载：结果逐字节相同（保序）" "grep -q 'PASS \[3\] 合成负载：n=1 与 n=8 结果逐字节相同' $W/bench.log"
chk "[3] 合成负载：加速比 ≥3.0" "grep -q 'PASS \[3\] 合成负载：加速比 ≥3.0' $W/bench.log"
chk "[4] 真实驱动：输出逐字节相同" "grep -q 'PASS \[4\] 真实驱动：n=1 与 n=8 输出逐字节相同' $W/bench.log"
chk "[4] 真实驱动：加速比 ≥1.8" "grep -q 'PASS \[4\] 真实驱动：加速比 ≥1.8' $W/bench.log"
chk "[3][4] bench rc=0" "[ $brc -eq 0 ]"

echo "=== [5] 缺陷 412 动态侧：--list 不清计时 TSV（＋对照）"
printf 'sentinel\n' > "$W/tsv_list"
GATE_TSV="$W/tsv_list" ./selfhost/run_gates.sh --list > "$W/list.log" 2>&1
lrc=$?
nlist=$(wc -l < "$W/list.log")
chk "[5] --list rc=0" "[ $lrc -eq 0 ]"
chk "[5] --list 规模锚点（门数 ≥150，实测 $nlist）" "[ $nlist -ge 150 ]"
chk "[5] --list **未**触碰 TSV（内容仍是 sentinel）" "grep -qx sentinel $W/tsv_list"
printf 'sentinel\n' > "$W/tsv_run"
GATE_TSV="$W/tsv_run" ./selfhost/run_gates.sh --only __m244_none__ --allow-dirty > "$W/only.log" 2>&1
chk "[5] 对照：真跑门那条路径**会**截断 TSV（证明上面不是恒真）" "! grep -qx sentinel $W/tsv_run"

if [ $NEG -eq 1 ]; then
echo "=== [6a] 负控 A：破坏保序（pmap 反序）⇒ [4] 的「逐字节相同」必须红"
NEGCTL --apply A | sed 's/^/  /'
python3 "$D/bench.py" --root "$ROOT" --n 120 > "$W/ncA.log" 2>&1; arc=$?
grep -E '^  (PASS|FAIL)' "$W/ncA.log" | sed 's/^/  /'
chk "[6a] A：bench rc≠0（必须红）" "[ $arc -ne 0 ]"
chk "[6a] A：红在 [4] 等价判据" "grep -q 'FAIL \[4\] 真实驱动：n=1 与 n=8 输出逐字节相同' $W/ncA.log"
NEGCTL --restore | sed 's/^/  /'
chk "[6a] A：gate_par.py 逐字节还原" "cmp -s $W/gp.orig $ROOT/selfhost/gate_par.py"

echo "=== [6b] 负控 B：忽略并发度（jobs() 恒 1）⇒ [3] 加速比必须红"
NEGCTL --apply B | sed 's/^/  /'
python3 "$D/bench.py" --root "$ROOT" --n 120 > "$W/ncB.log" 2>&1; brc2=$?
grep -E '^  (PASS|FAIL)' "$W/ncB.log" | sed 's/^/  /'
chk "[6b] B：bench rc≠0（必须红）" "[ $brc2 -ne 0 ]"
chk "[6b] B：红在 [3] 加速比" "grep -q 'FAIL \[3\] 合成负载：加速比 ≥3.0' $W/ncB.log"
chk "[6b] B：**保序仍 PASS**（证明两组判据互相独立）" "grep -q 'PASS \[3\] 合成负载：n=1 与 n=8 结果逐字节相同' $W/ncB.log"
NEGCTL --restore > /dev/null
chk "[6b] B：gate_par.py 逐字节还原" "cmp -s $W/gp.orig $ROOT/selfhost/gate_par.py"

echo "=== [6c] 负控 C：判据自伤（A 补丁在位 + bench --harm）⇒ A 的红必须消失"
NEGCTL --apply A > /dev/null
python3 "$D/bench.py" --root "$ROOT" --n 120 --harm > "$W/ncC.log" 2>&1; crc=$?
grep -E '^  (PASS|FAIL|ℹ)' "$W/ncC.log" | sed 's/^/  /'
chk "[6c] C：rc=0（证明 A 的红来自那条比较本身）" "[ $crc -eq 0 ]"
chk "[6c] C：等价判据报 PASS" "grep -q 'PASS \[4\] 真实驱动：n=1 与 n=8 输出逐字节相同' $W/ncC.log"
NEGCTL --restore > /dev/null
chk "[6c] C：gate_par.py 逐字节还原" "cmp -s $W/gp.orig $ROOT/selfhost/gate_par.py"
else
echo "=== [6] 负控：--neg-skip（CI 模式）"
fi

echo "=== [7] 覆盖边界（如实登记）"
cat <<'EOF' | sed 's/^/  /'
  · **只覆盖 7 个「矩阵对拍门」执行器**（清单见 targets.tsv）。其余门仍是串行，理由各异：
    ① `upstream_tests`（241s）跑第三方套件，**部分用例绑端口 / 起服务** ⇒ 并发会互踩；
    ② `m216`（223s）/ `m220`（212s）是**逐候选编译**形状（`tools/px build`），
       而 `.rtcache` 与 `devbuild` 都是**共享可变状态** ⇒ 不能门内并发；
    ③ `m223`（229s）本身就是 devbuild 指纹门 —— 串行是它的**语义**；
    ④ `m217`（182s）含 `ub_proof.sh` 逐档重编（同上）。
  · **本门不重跑那 7 个门**：它们的**正确性**由各自 verify.sh（自带判据 + 负控）在
    **同一次全量门**里覆盖；本门只证明「机制正确 + 真的接上了 + 真的更快」。
    ⇒ 若哪个门的并行化接错了，**那扇门自己的判据**会红（这正是它存在的意义）。
  · 加速比是**墙钟**：CI 上机器负载可能让 [4] 的 1.8x 抖动（已留余量）；
    [3] 是合成负载（64 × 60ms），几乎不受外部影响。
  · `pmap` 用**线程**不用进程：`subprocess.run` 释放 GIL ⇒ 真并行，且结果对象直接传回
    （免序列化）。**不适用于 CPU-bound 的 Python 任务**（本仓门里没有这种）。
  · 门**之间**仍串行（PID 锁）—— 53 个门的负控改源码，门间并行必然互踩（M191 的护栏）。
EOF

echo
echo "M244-VERIFY-$( [ $fail -eq 0 ] && echo OK || echo FAIL ) pass=$pass fail=$fail"
[ $fail -eq 0 ]
