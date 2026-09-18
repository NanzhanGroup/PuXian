#!/usr/bin/env bash
# M140 验证：连接失败成因分类 + 双栈（qg-issue 87 第 21 轮 · 缺陷 108）
#   修前：px_tcp_connect_timeout 的失败路径不保留 errno ⇒ 语言层只有
#     `net: 连接 X:Y 失败`，一律被当成 connection refused；
#     且 hints.ai_family=AF_INET 强制 IPv4、hparse_url 不支持 `[::1]`。
#   手段：**不依赖任何外部服务**（127.0.0.1:1 必然无监听、.invalid 必然不解析、
#     2001:db8::/32 无路由时必然 ENETUNREACH），默认 VM 轨 + C 轨都跑，断言集相同。
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m140_build.log
FAIL=0

run_track() {   # $1 = 轨名（vm / c）
    local track="$1"
    echo "== 轨：$track =="
    local ok=1
    if [ "$track" = "c" ]; then
        env PX_BUILD_ENGINE=c "$PX" build http_conn_errno.px > "$LOG" 2>&1 || ok=0
    else
        "$PX" build http_conn_errno.px > "$LOG" 2>&1 || ok=0
    fi
    if [ "$ok" != "1" ]; then
        echo "FAIL 编译失败（$track）"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    ./build/http_conn_errno > "/tmp/m140_$track.txt" 2>&1
    local rc=$?
    sed 's/^/     /' "/tmp/m140_$track.txt"
    if [ "$rc" != "0" ]; then
        echo "FAIL 自检非 0 退出（$track，rc=$rc）"; FAIL=$((FAIL+1)); return
    fi
    local n
    n=$(grep -c '^PASS ' "/tmp/m140_$track.txt" || true)
    if [ "$n" != "9" ]; then
        echo "FAIL 通过项数=$n（期望 9）"; FAIL=$((FAIL+1))
    fi
    if grep -q '^FAIL ' "/tmp/m140_$track.txt"; then
        echo "FAIL 存在失败项"; FAIL=$((FAIL+1))
    fi
}

run_track vm
run_track c

echo "---"
if diff -q /tmp/m140_vm.txt /tmp/m140_c.txt > /dev/null 2>&1; then
    echo "PASS 两轨（VM/C）输出逐字节一致"
else
    echo "FAIL 两轨输出不一致："; diff /tmp/m140_vm.txt /tmp/m140_c.txt | head -20; FAIL=$((FAIL+1))
fi

if [ "$FAIL" = "0" ]; then echo "✅ M140 门：全部通过"; else echo "❌ M140 门：$FAIL 项失败"; fi
exit $FAIL
