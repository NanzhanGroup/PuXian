#!/usr/bin/env bash
# M141 验证：墙钟纳秒 + unix socket 服务端 remote 值（qg-issue 87 第 22 轮 · 缺陷 109/110）
#   缺陷 109：语言里没有**墙钟纳秒**（now_us/now_ms 是 CLOCK_MONOTONIC、now_sec 只到秒）
#             ⇒ Go 的 time.Now().UnixNano() / Format("...000000000") 表达不出来。
#   缺陷 110：http_serve_unix 的 req["remote"] 对 AF_UNIX 给 "unix"，Go 给 "@"。
#   两轨（VM / C）断言集相同，输出逐字节一致。
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/px
FAIL=0

run_track() {   # $1 = 轨名（vm / c）
    local track="$1"
    echo "== 轨：$track =="
    for prog in now_ns remote_unix; do
        local log="/tmp/m141_${track}_${prog}_build.log"
        local ok=1
        if [ "$track" = "c" ]; then
            env PX_BUILD_ENGINE=c "$PX" build "$prog.px" > "$log" 2>&1 || ok=0
        else
            "$PX" build "$prog.px" > "$log" 2>&1 || ok=0
        fi
        if [ "$ok" != "1" ]; then
            echo "FAIL 编译失败（$track / $prog）"; tail -20 "$log"; FAIL=$((FAIL+1)); continue
        fi
        "./build/$prog" > "/tmp/m141_${track}_${prog}.txt" 2>&1
        local rc=$?
        sed "s/^/     [$prog] /" "/tmp/m141_${track}_${prog}.txt"
        if [ "$rc" != "0" ]; then
            echo "FAIL 自检非 0 退出（$track / $prog，rc=$rc）"; FAIL=$((FAIL+1)); continue
        fi
        if grep -q '^FAIL ' "/tmp/m141_${track}_${prog}.txt"; then
            echo "FAIL 存在失败项（$track / $prog）"; FAIL=$((FAIL+1)); continue
        fi
    done
}

run_track vm
run_track c

echo "---"
for prog in now_ns remote_unix; do
    if diff -q "/tmp/m141_vm_${prog}.txt" "/tmp/m141_c_${prog}.txt" > /dev/null 2>&1; then
        echo "PASS 两轨（VM/C）输出逐字节一致（$prog）"
    else
        echo "FAIL 两轨输出不一致（$prog）："; diff "/tmp/m141_vm_${prog}.txt" "/tmp/m141_c_${prog}.txt" | head -20; FAIL=$((FAIL+1))
    fi
done

# 断言条数下限（防"门自己空转"）
for f in /tmp/m141_vm_now_ns.txt /tmp/m141_vm_remote_unix.txt; do
    n=$(grep -c '^PASS ' "$f" || true)
    if [ "$n" = "0" ]; then echo "FAIL $f 无 PASS 行"; FAIL=$((FAIL+1)); fi
done

if [ "$FAIL" = "0" ]; then echo "✅ M141 门：全部通过"; else echo "❌ M141 门：$FAIL 项失败"; fi
exit $FAIL
