#!/bin/sh
# M110-S2 验收运行器：同一二进制分别以 CLI（单线程）与 serve（多线程）跑，比较放大倍率。
#   改前：serve/cli ≈ 13.5×（Issue 38）  改后：≈ 1×
set -e
cd "$(dirname "$0")"
PXC=${PXC:-../../tools/px}
BIN=./build/m110_s2
"$PXC" build m110_s2.px
echo "--- CLI（单线程进程）---"
taskset -c 3 "$BIN" cli  | tee cli.txt
echo "--- serve（多线程进程）---"
rm -f /tmp/m110_s2.sock
taskset -c 3 "$BIN" serve | tee serve.txt
echo "--- 放大倍率（serve / cli）---"
for k in alloc_loop scan_core; do
    c=$(grep "\[cli\] $k" cli.txt | sed 's/.*ms=\([0-9]*\).*/\1/')
    s=$(grep "\[worker\] $k" serve.txt | sed 's/.*ms=\([0-9]*\).*/\1/')
    echo "$k: cli=${c}ms serve=${s}ms  倍率=$(awk "BEGIN{ if ($c>0) printf \"%.2fx\", $s/$c; else print \"n/a\" }")"
done
