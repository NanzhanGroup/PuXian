#!/bin/bash
# M85-S1 (qg-issue 24) pxc build 细粒度模块裁剪验证
#   模块开关集 --no-sqlite/--no-ws/--no-zip/--no-xml/--no-aes/--no-rsa/--no-ed25519/
#   --no-route/--no-zlib/--no-h2（与 --no-quic 正交可组合，M85-S1）
#   验证：默认零漂移 / 各组合体积断言 / 裁剪态缺 native R1001 / 核心 http 面保留
#   依赖：tools/pxc（M85-S1 版）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
rm -rf build; mkdir -p build
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }

echo "== [1/5] 默认全能力基线（宏包裹零漂移，M84 基线 9010184）=="
$PX build hello.px >/tmp/m85s1.log 2>&1 || { bad "默认 build"; tail -5 /tmp/m85s1.log; }
SZ=$(stat -c %s build/hello 2>/dev/null || echo 0)
if [ "$SZ" -ge 8900000 ] && [ "$SZ" -le 9150000 ]; then ok "默认体积 $SZ（9.0M 基线内）"; else bad "默认体积 $SZ 偏离基线"; fi
./build/hello >/dev/null 2>&1 && ok "默认产物运行" || bad "默认产物运行"

echo "== [2/5] --no-quic（基线 3929808）=="
$PX build --no-quic hello.px >/dev/null 2>&1 && SZ=$(stat -c %s build/hello) || SZ=0
if [ "$SZ" -ge 3800000 ] && [ "$SZ" -le 4050000 ]; then ok "--no-quic $SZ"; else bad "--no-quic $SZ 期望 ~3929808"; fi

echo "== [3/5] --no-quic --no-sqlite（去 sqlite3.o 1.3M，基线 2884072）=="
$PX build --no-quic --no-sqlite hello.px >/dev/null 2>&1 && SZ=$(stat -c %s build/hello) || SZ=0
if [ "$SZ" -ge 2700000 ] && [ "$SZ" -le 3050000 ]; then ok "--no-quic --no-sqlite $SZ"; else bad "--no-quic --no-sqlite $SZ 期望 ~2884072"; fi

echo "== [4/5] 全裁（--no-quic + 10 模块开关，基线 2713472）=="
ALL="--no-quic --no-sqlite --no-ws --no-zip --no-xml --no-aes --no-rsa --no-ed25519 --no-route --no-zlib --no-h2"
$PX build $ALL hello.px >/dev/null 2>&1 && SZ=$(stat -c %s build/hello) || SZ=0
if [ "$SZ" -ge 2500000 ] && [ "$SZ" -le 2900000 ]; then ok "全裁 $SZ"; else bad "全裁 $SZ 期望 ~2713472"; fi
./build/hello >/dev/null 2>&1 && ok "全裁产物运行" || bad "全裁产物运行"

echo "== [5/5] 裁剪态语义：缺 native → R1001（未定义，非崩溃）+ 核心 http 面保留 =="
$PX build --no-quic --no-sqlite sqlite_dep.px >/dev/null 2>&1 || { bad "sqlite_dep build"; }
if ./build/sqlite_dep 2>&1 | grep -q "未定义变量: sqlite_open"; then ok "no-sqlite 缺 sqlite_open → R1001"; else bad "no-sqlite 缺 sqlite_open 语义"; fi
$PX build $ALL ws_dep.px >/dev/null 2>&1 || { bad "ws_dep build"; }
if ./build/ws_dep 2>&1 | grep -q "未定义变量: ws_connect"; then ok "全裁缺 ws_connect → R1001"; else bad "全裁缺 ws_connect 语义"; fi
$PX build $ALL http_dep.px >/dev/null 2>&1 || { bad "http_dep build"; }
if ./build/http_dep 2>&1 | grep -q "Err(net:"; then ok "全裁 http_get 核心保留"; else bad "全裁 http_get 核心保留"; fi

echo
echo "M85-S1 verify: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = 0 ] || exit 1
