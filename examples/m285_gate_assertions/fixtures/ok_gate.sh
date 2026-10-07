#!/usr/bin/env bash
set -uo pipefail
sed -i 's|PXOP_ITERLEN|PXOP_ITERLEN_X|' runtime/vm.h
grep -q 'PXOP_ITERLEN' runtime/vm.h || { echo "打桩失败"; exit 2; }
grep -q '汇总：失败 0 项' "$W/run.log" || { bad "汇总行缺失"; }
grep -q '缺陷 440 · 已登记未修' examples/m128_unlock_grow/verify.sh && bad "旧措辞仍在"
grep -q '计算两数之和' "$W/doc.out" || bad "文档缺"
