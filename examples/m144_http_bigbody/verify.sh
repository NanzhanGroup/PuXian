#!/usr/bin/env bash
# M144 验证：HTTP 客户端的大请求体（缺陷 120/121/122）+ `opts.chunked` 分帧（特性）
#   · 缺陷 120：http_unix 的体 memcpy 进 req[16384] ⇒ >16KB **段错误**（实测 20000 字节）
#   · 缺陷 121：http_post（px_http_once 的 req[4096]）⇒ >3.9KB 段错误
#   · 缺陷 122：s3_put（px_s3_exec 的 pbody[4096]）⇒ >4KB 段错误 + 体静默丢弃
#   · 特性：http_unix(... opts {"chunked": true}) → Transfer-Encoding: chunked（32768/块），
#           与 Go net/http（未知长度 Reader = io.Copy 32KB 缓冲）逐字节同形
# 手段：真起 http_serve_unix + http_serve（进程内 spawn），客户端自调用并**逐字节**校验
#       （服务端回 body 的长度 + sha256，客户端与本地比对）。
# 默认 VM 轨 + C 轨都跑，断言集相同。
set -u
cd "$(dirname "$0")"
PX=../../tools/px
LOG=/tmp/m144_build.log
FAIL=0
rm -f /tmp/m144_puxian.sock

run_track() {   # $1 = 轨名（vm / c）
    local track="$1"
    echo "== 轨：$track =="
    rm -f /tmp/m144_puxian.sock
    local ok=1
    if [ "$track" = "c" ]; then
        env PX_BUILD_ENGINE=c "$PX" build http_bigbody.px > "$LOG" 2>&1 || ok=0
    else
        "$PX" build http_bigbody.px > "$LOG" 2>&1 || ok=0
    fi
    if [ "$ok" != "1" ]; then
        echo "FAIL 编译失败（$track）"; tail -20 "$LOG"; FAIL=$((FAIL+1)); return
    fi
    ./build/http_bigbody > "/tmp/m144_$track.txt" 2>&1
    local rc=$?
    sed 's/^/     /' "/tmp/m144_$track.txt"
    if [ "$rc" != "0" ]; then
        echo "FAIL 自检非 0 退出（$track，rc=$rc —— 段错误会是 139/134）"
        FAIL=$((FAIL+1)); return
    fi
    if ! grep -q '^M144-VERIFY-OK$' "/tmp/m144_$track.txt"; then
        echo "FAIL 缺 M144-VERIFY-OK（$track）"; FAIL=$((FAIL+1)); return
    fi
    local n
    n=$(grep -o 'PASS=[0-9]*' "/tmp/m144_$track.txt" | head -1 | cut -d= -f2)
    if [ "${n:-0}" != "115" ]; then
        echo "FAIL 通过项数=${n:-0}（期望 115）"; FAIL=$((FAIL+1)); return
    fi
    if grep -q '^FAIL ' "/tmp/m144_$track.txt"; then
        echo "FAIL 存在失败项（$track）"; FAIL=$((FAIL+1)); return
    fi
    # 进程必须自己退出（spawn 的服务端不能拖住）
    if pgrep -f "build/http_bigbody" >/dev/null 2>&1; then
        echo "FAIL 仍有残留进程（$track）"; FAIL=$((FAIL+1))
    fi
}

run_track vm
run_track c

rm -f /tmp/m144_puxian.sock
if [ "$FAIL" = "0" ]; then
    echo "M144-ALL-OK（VM + C 双轨）"
    exit 0
fi
echo "M144-FAIL（$FAIL 项）"
exit 1
