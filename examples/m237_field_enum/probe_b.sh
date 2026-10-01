#!/bin/bash
# M237 门 · 组 B/C 单文件三轨对拍（可被负控复用）
# 用法：probe_b.sh <probe_dir> <work_dir> <pxc> <pxcv> <pxi>
# 退出码 = 分叉例数（0 = 三轨全一致）
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
PD="$1"; W="$2"; PXC="$3"; PXCV="$4"; PXI="$5"
norm() { sed -E 's/^运行时错误 \[[^]]*\]: //; s/^运行时错误: //; s/^错误 \[R([0-9]+)\] [0-9]+:[0-9]+: /R\1: /; s/^R([0-9]+): /R\1: /' "$1"; }
mkdir -p "$W"
nfb=0
for f in "$PD"/sep/*.px; do
  cid=$(basename "$f" .px)
  DD="$W/$cid"; rm -rf "$DD"; mkdir -p "$DD"; cp "$f" "$DD/x.px"
  (
    cd "$DD" || exit 9
    PX_PXC_BIN="$PXC" PX_BUILD_ENGINE=c timeout 600 "$ROOT/tools/px" build --c x.px > bc.log 2>&1; rbc=$?
    if [ $rbc -eq 0 ] && [ -x build/x ]; then timeout 60 ./build/x > oc.txt 2>&1; rrc=$?; else rrc=998; sed -n '$p' bc.log > oc.txt; fi
    timeout 60 "$PXI" x.px > oi.txt 2>&1; ri=$?
    PXC_VM_BIN="$PXCV" timeout 600 "$ROOT/tools/px" build x.px > bv.log 2>&1; rbv=$?
    if [ $rbv -eq 0 ] && [ -x build/x ]; then timeout 60 ./build/x > ov.txt 2>&1; rv=$?; else rv=998; tail -1 bv.log > ov.txt; fi
    printf 'C|%s|%s|%s\n' "$rbc" "$rrc" "$(norm oc.txt|tr '\n' '~')" > st.txt
    printf 'I|0|%s|%s\n'   "$ri"  "$(norm oi.txt|tr '\n' '~')" >> st.txt
    printf 'V|%s|%s|%s\n' "$rbv" "$rv" "$(norm ov.txt|tr '\n' '~')" >> st.txt
  )
  # 判据 = **归一化输出**逐字节一致（构建 rc 只反映「编译期 vs 运行期」的阶段差异；
  #   阶段一致性已由本轮定稿统一，而输出才是行为真相。空输出/BUILDFAIL 一律判红防假绿）。
  nl=$(awk -F'|' '{print $4}' "$DD/st.txt" | sort -u | wc -l)
  nn=$(wc -l < "$DD/st.txt")
  first=$(awk -F'|' 'NR==1{print $4}' "$DD/st.txt")
  if [ "$nl" = "1" ] && [ "$nn" = "3" ] && [ -n "$first" ] && [ "$first" != "BUILDFAIL" ]; then
    echo "  OK   $cid"
  else
    nfb=$((nfb+1)); echo "  FORK $cid"; sed 's/^/       /' "$DD/st.txt"
  fi
done
exit $nfb
