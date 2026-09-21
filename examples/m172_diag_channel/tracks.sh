#!/usr/bin/env bash
# M172 门用三轨运行器：把 interp / VM / C 三轨的 rc、stdout、stderr、**真·合并流**分别落盘。
# 用法：tracks.sh <file.px> <outdir>
#   产出 <outdir>/{interp,vm,c}.{rc,out,err,merged}
#   · out/err = 分开重定向（判「诊断走哪条通道」）
#   · merged  = 真 `> f 2>&1` 单次运行（判「诊断不得跑到程序输出前面」——
#     拼接式的 out+err 测不出交错，必须用同一进程的合并重定向）
set -u
F="$1"; OUT="$2"
B=$(basename "$F" .px)
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
PX="$ROOT/tools/px"
mkdir -p "$OUT"
cp "$F" "$OUT/$B.px"

split_run() {   # $1=标签 $2...=命令
    local tag=$1; shift
    "$@" > "$OUT/$B.$tag.out" 2> "$OUT/$B.$tag.err"
    echo $? > "$OUT/$B.$tag.rc"
}
merged_run() {  # $1=标签 $2...=命令
    local tag=$1; shift
    "$@" > "$OUT/$B.$tag.merged" 2>&1
}

split_run interp "$PX" run "$OUT/$B.px"
merged_run interp "$PX" run "$OUT/$B.px"

rm -rf "$OUT/build"
if "$PX" build "$OUT/$B.px" > "$OUT/$B.vm.build.log" 2>&1 && [ -x "$OUT/build/$B" ]; then
    split_run vm "$OUT/build/$B"; merged_run vm "$OUT/build/$B"
else
    : > "$OUT/$B.vm.out"; : > "$OUT/$B.vm.err"; : > "$OUT/$B.vm.merged"; echo 99 > "$OUT/$B.vm.rc"
fi
rm -rf "$OUT/build"
if "$PX" build --c "$OUT/$B.px" > "$OUT/$B.c.build.log" 2>&1 && [ -x "$OUT/build/$B" ]; then
    split_run c "$OUT/build/$B"; merged_run c "$OUT/build/$B"
else
    : > "$OUT/$B.c.out"; : > "$OUT/$B.c.err"; : > "$OUT/$B.c.merged"; echo 99 > "$OUT/$B.c.rc"
fi
rm -rf "$OUT/build"
exit 0
