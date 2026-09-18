#!/usr/bin/env bash
# M133 验证：http_unix 的 opts.timeout_ms 与失败成因（errno）可编程获取
#   缺陷 80（语言缺口）：http_unix 无超时入口 ⇒ RCVTIMEO 固定 180s，
#     Go 的 http.Client{Timeout}（总时限）在这条通路上**表达不出**。
#   缺陷 81（语言缺口）：失败时 errno 被 close() 覆盖 ⇒ 调用方无法区分
#     "连不上"（可重试）与"超时/对端断开"（重试会放大对端 CPU）。
# 手段：真起 http_serve_unix 服务端（快/慢两档），**默认 VM 轨 + C 轨都跑**，断言集相同。
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m133_build.log
FAIL=0
rm -f /tmp/m133_fast.sock /tmp/m133_slow.sock

run_track() {   # $1 = 轨名（vm / c）
    local track="$1"
    echo "== 轨：$track =="
    rm -f /tmp/m133_fast.sock /tmp/m133_slow.sock
    local ok=1
    if [ "$track" = "c" ]; then
        env PX_BUILD_ENGINE=c "$PX" build http_unix_timeout.px > "$LOG" 2>&1 || ok=0
    else
        "$PX" build http_unix_timeout.px > "$LOG" 2>&1 || ok=0
    fi
    if [ "$ok" != "1" ]; then
        echo "FAIL 编译失败（$track）"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    ./build/http_unix_timeout > "/tmp/m133_$track.txt" 2>&1
    local rc=$?
    sed 's/^/     /' "/tmp/m133_$track.txt"
    if [ "$rc" != "0" ]; then
        echo "FAIL 自检非 0 退出（$track，rc=$rc）"; FAIL=$((FAIL+1)); return
    fi
    local n
    n=$(grep -c '^PASS ' "/tmp/m133_$track.txt" || true)
    if [ "$n" != "5" ]; then
        echo "FAIL 通过项数=$n（期望 5）"; FAIL=$((FAIL+1))
    fi
    if grep -q '^FAIL ' "/tmp/m133_$track.txt"; then
        echo "FAIL 存在失败项"; FAIL=$((FAIL+1))
    fi
}

run_track vm
run_track c

echo "---"
# 两轨输出必须一致（含 errno 与判定结果）——但**耗时数字**是时间量，先归一化
for t in vm c; do
    sed -E 's/[0-9]+ms/Nms/g' "/tmp/m133_$t.txt" > "/tmp/m133_${t}_norm.txt"
done
if diff -q /tmp/m133_vm_norm.txt /tmp/m133_c_norm.txt > /dev/null 2>&1; then
    echo "PASS 两轨（VM/C）输出一致（耗时已归一）"
else
    echo "FAIL 两轨输出不一致："; diff /tmp/m133_vm_norm.txt /tmp/m133_c_norm.txt | head -20; FAIL=$((FAIL+1))
fi

rm -f /tmp/m133_fast.sock /tmp/m133_slow.sock
if [ "$FAIL" = "0" ]; then echo "✅ M133 门：全部通过"; else echo "❌ M133 门：$FAIL 项失败"; fi
exit $FAIL
