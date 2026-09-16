#!/usr/bin/env bash
# M124 独立复核：STW 自旋 futex 化前后 A/B（sched_yield/s + CPU + 吞吐）
# base = 4127306（M124 之前，git worktree /data/tmp/puxian-base）
# new  = main（含 M124 + M125）
set -u
BASE=/data/tmp/puxian-base
NEW=/data/code/puxian
# base 用 git worktree 取 M124 之前的提交（幂等：已存在则复用）
BASE_REV=${BASE_REV:-4127306}
if [ ! -d "$BASE/runtime" ]; then
  (cd "$NEW" && git worktree add "$BASE" "$BASE_REV") || exit 1
fi
echo "=== 编译两份 ==="
(cd "$BASE" && ./tools/px build examples/m124_stw/stw_stress.px) 2>&1 | tail -2
(cd "$NEW"  && ./tools/px build examples/m124_stw/stw_stress.px) 2>&1 | tail -2
echo "=== 校验：base 的 runtime 不含 futex，new 含 ==="
echo "base px_futex_wait=$(grep -c px_futex_wait "$BASE/runtime/runtime.c")  new=$(grep -c px_futex_wait "$NEW/runtime/runtime.c")"
echo
echo "############ A/B 1：base（4127306，自旋）############"
bash "$BASE/examples/m124_stw/probe.sh" base "$BASE/examples/m124_stw/build/stw_stress" 8 20 2>&1 | grep -E "TOTAL ticks|sched_yield|calls=|iters/s|iters=" | head -12
echo
echo "############ A/B 2：new（main，futex）############"
bash "$NEW/examples/m124_stw/probe.sh" new "$NEW/examples/m124_stw/build/stw_stress" 8 20 2>&1 | grep -E "TOTAL ticks|sched_yield|calls=|iters/s|iters=" | head -12
echo
echo "=== 原始测量文件 ==="
for f in /tmp/m124_base.txt /tmp/m124_new.txt; do echo "--- $f ---"; grep -E "TOTAL ticks|sched_yield|rate=|iters" "$f" 2>/dev/null | head -8; done
