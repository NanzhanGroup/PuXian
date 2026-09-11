#!/bin/sh
# M110-S1 验收运行器：编译 + 跑对拍/计时。BASELINE 环境变量指向改前二进制可做逐字节对拍。
set -e
cd "$(dirname "$0")"
PXC=${PXC:-../../tools/px}
[ -x "$PXC" ] || PXC=/data/code/puxian/tools/px
"$PXC" build m110_s1.px
./build/m110_s1 > out.new.txt
echo "--- 输出 ---"
cat out.new.txt
if [ -n "$BASELINE" ] && [ -x "$BASELINE" ]; then
    "$BASELINE" > out.base.txt
    grep -v '^TIME ' out.base.txt > b.txt
    grep -v '^TIME ' out.new.txt  > n.txt
    if diff -u b.txt n.txt > /dev/null; then
        echo "对拍（非计时行）: 逐字节一致 ✅"
    else
        echo "对拍: 不一致 ❌"; diff -u b.txt n.txt; exit 1
    fi
fi
