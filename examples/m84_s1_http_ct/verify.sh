#!/bin/bash
# M84-S1 (Issue 23) http_request/http_unix CT/CL 解耦验证
#   单头语义：显式 Content-Type 不再追加默认 urlencoded（修复双 Content-Type bug）；
#   无头默认表单头回归；Content-Length 自动补齐（服务端按 CL 读足 body 并回 200 即证）；
#   显式 CL 保留；GET 无 body 无 CT/CL；http_unix 同步修复。
# 依赖：tools/pxc 编译模式（http_request/http_unix 非 pxi Mini 子集）+ python3
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
PORT=18107
SOCK=/tmp/m84s1_echo.sock
OUT=/tmp/m84s1_reqs
rm -rf "$OUT"; mkdir -p "$OUT" build
python3 echo_server.py --tcp-port $PORT --unix-sock $SOCK --outdir "$OUT" >/tmp/m84s1_echo_srv.log 2>&1 &
SRV=$!
trap 'kill $SRV 2>/dev/null; wait $SRV 2>/dev/null; rm -f $SOCK' EXIT
sleep 0.6

echo "== [1/5] 编译客户端（runtime.c 变更 → pxc 自动重建 runtime）=="
$PX build --no-quic http_ct_client.px >/tmp/m84s1_build.log 2>&1 || { echo "FAIL build"; tail -20 /tmp/m84s1_build.log; exit 1; }

echo "== [2/5] 客户端 6 场景回环（T1 无头表单/T2 json无CL/T3 multipart带CL/T4 GET/T5 XCustom/U1 unix json）=="
./build/http_ct_client $PORT $SOCK >/tmp/m84s1_client.log 2>&1 || { echo "FAIL client"; cat /tmp/m84s1_client.log; exit 1; }
cat /tmp/m84s1_client.log
sleep 0.5
N=$(ls "$OUT" | wc -l)
[ "$N" = "6" ] || { echo "FAIL 期望 6 请求文件，实得 $N"; cat /tmp/m84s1_echo_srv.log; exit 1; }

# check <文件> <请求行path> <期望CT数> <期望CL数> <期望CT值子串|-->
check() {
    f="$1"; path="$2"; ect="$3"; ecl="$4"; ev="$5"
    grep -q "$path" "$f" || { echo "FAIL $f 缺请求行 $path"; cat "$f"; exit 1; }
    ct=$(grep -ci '^content-type:' "$f"); cl=$(grep -ci '^content-length:' "$f")
    [ "$ct" = "$ect" ] || { echo "FAIL $f ($path) Content-Type 计数=$ct 期望 $ect"; cat "$f"; exit 1; }
    [ "$cl" = "$ecl" ] || { echo "FAIL $f ($path) Content-Length 计数=$cl 期望 $ecl"; cat "$f"; exit 1; }
    if [ "$ev" != "-" ]; then
        grep -qi "content-type: $ev" "$f" || { echo "FAIL $f ($path) CT 值缺: $ev"; cat "$f"; exit 1; }
    fi
    echo "  PASS $f ($path)：CT=$ct CL=$cl"
}

echo "== [3/5] 逐文件单头断言 =="
check "$OUT/req_1.txt" 'POST /t1' 1 1 "application/x-www-form-urlencoded"  # 无头默认表单回归
check "$OUT/req_2.txt" 'POST /t2' 1 1 "application/json"                   # 核心：显式 CT 无 CL → 单 CT + 自动 CL
check "$OUT/req_3.txt" 'POST /t3' 1 1 "multipart/form-data; boundary=x42"  # 显式 CL 保留，仍单 CT
check "$OUT/req_4.txt" 'GET /t4'  0 0 "-"                                  # GET 无 body 无 CT/CL
check "$OUT/req_5.txt" 'POST /t5' 1 1 "text/plain; charset=utf-8"          # X-Custom 相邻不干扰行首匹配
check "$OUT/req_6.txt" 'POST /u1' 1 1 "application/json"                   # http_unix 同步修复

echo "== [4/5] body 完整性（server 按 CL 读足 body 且均回 200，客户端断言全过即证）=="
echo "== [5/5] 清理 =="
echo "M84-S1-VERIFY-OK"
