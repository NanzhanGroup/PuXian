#!/usr/bin/env bash
# M137 验证：sse_connect_ex（失败可分类 / CT 不硬要求 / IO 超时）
#   VM 轨 + C 轨跑**同一断言集**，两轨输出必须一致（除时间量）。
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m137_build.log
UP=/tmp/m137_up.log
PORT=13971
DEAD=13972
FAIL=0

rm -f "$UP"
python3 fake_up.py "$PORT" - "$UP" &
UPPID=$!
for _ in $(seq 1 50); do
    grep -q READY "$UP" 2>/dev/null && break
    sleep 0.1
done
if ! grep -q READY "$UP" 2>/dev/null; then
    echo "FAIL 上游桩未就绪"; tail -5 "$UP"; kill $UPPID 2>/dev/null; exit 1
fi

run_track() {
    local track="$1"
    echo "== 轨：$track =="
    local ok=1
    if [ "$track" = "c" ]; then
        env PX_BUILD_ENGINE=c "$PX" build sse_connect_ex.px > "$LOG" 2>&1 || ok=0
    else
        "$PX" build sse_connect_ex.px > "$LOG" 2>&1 || ok=0
    fi
    if [ "$ok" != "1" ]; then
        echo "FAIL 编译失败（$track）"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    ./build/sse_connect_ex "$PORT" /tmp/m137_no_such.sock "$DEAD" > "/tmp/m137_$track.txt" 2>&1
    local rc=$?
    sed 's/^/     /' "/tmp/m137_$track.txt"
    if [ "$rc" != "0" ]; then
        echo "FAIL 自检非 0 退出（$track，rc=$rc）"; FAIL=$((FAIL+1)); return
    fi
    if grep -q '^FAIL ' "/tmp/m137_$track.txt"; then
        echo "FAIL 存在失败项（$track）"; FAIL=$((FAIL+1))
    fi
}

run_track vm
run_track c

echo "---"
# 两轨输出必须一致（耗时断言是布尔判定，已在断言里归一；时间量不入输出）
if diff -q "/tmp/m137_vm.txt" "/tmp/m137_c.txt" > /dev/null 2>&1; then
    echo "PASS 两轨（VM/C）输出一致"
else
    echo "FAIL 两轨输出不一致："; diff /tmp/m137_vm.txt /tmp/m137_c.txt | head -20; FAIL=$((FAIL+1))
fi

n=$(grep -c '^PASS ' /tmp/m137_vm.txt || true)
if [ "$n" -lt 30 ]; then
    echo "FAIL 通过项数=$n（期望 >=30）"; FAIL=$((FAIL+1))
else
    echo "PASS 通过项数=$n"
fi
grep -q 'M137 PASS=[0-9]* FAIL=0' /tmp/m137_vm.txt || { echo "FAIL 汇总行非 FAIL=0"; FAIL=$((FAIL+1)); }

kill $UPPID 2>/dev/null
wait $UPPID 2>/dev/null
if [ "$FAIL" = "0" ]; then echo "✅ M137 门：全部通过"; else echo "❌ M137 门：$FAIL 项失败"; fi
exit $FAIL
