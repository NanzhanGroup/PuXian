#!/usr/bin/env bash
# 三轨对拍：interp（px run） / VM（px build） / C（px build --c）逐字节比 stdout
# 用法：three_tracks <file.px>  → 全部一致时打印输出，否则 rc=1 并打印差异
set -u
F="$1"
B=$(basename "$F" .px)
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
PX=${PX_BIN:-$ROOT/tools/px}
[ -x "$PX" ] || PX=/data/code/puxian/tools/px
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
"$PX" run "$F" > "$W/interp.out" 2>"$W/interp.err"; ri=$?
cp "$F" "$W/$B.px"
(cd "$W" && "$PX" build "$B.px" >/dev/null 2>"$W/vm.build.err"); rv=$?
if [ $rv -eq 0 ]; then "$W/build/$B" > "$W/vm.out" 2>"$W/vm.err"; rv=$?; fi
(cd "$W" && "$PX" build --c "$B.px" >/dev/null 2>"$W/c.build.err"); rc=$?
if [ $rc -eq 0 ]; then "$W/build/$B" > "$W/c.out" 2>"$W/c.err"; rc=$?; fi
ok=1
if [ $ri -ne 0 ] || [ $rv -ne 0 ] || [ $rc -ne 0 ]; then
    echo "  ✗ rc 不一致：interp=$ri vm=$rv c=$rc"; ok=0
fi
cmp -s "$W/interp.out" "$W/vm.out" || { echo "  ✗ interp vs VM stdout 不同"; diff "$W/interp.out" "$W/vm.out" | head -10; ok=0; }
cmp -s "$W/interp.out" "$W/c.out"  || { echo "  ✗ interp vs C  stdout 不同"; diff "$W/interp.out" "$W/c.out" | head -10; ok=0; }
if [ $ok -eq 1 ]; then cat "$W/interp.out"; fi
exit $((1 - ok))
