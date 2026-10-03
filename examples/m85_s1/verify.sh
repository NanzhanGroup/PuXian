#!/bin/bash
# M85-S1 (qg-issue 24) pxc build 细粒度模块裁剪验证
#   M86-S2 适配：默认裸 build = 自动最小 → 显式裁剪档统一加 --full（全能力）前缀保 M85 原意
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

echo "== [1/5] 全能力基线 --full（M251：改「下界防误裁 + 宽松上界防膨胀」）=="
echo "   原判据是 M84 快照 9010184±1.4% —— 164 个里程碑后体积自然增长 5.4% ⇒ 必然腐烂。"
echo "   新判据：下界 8.9M（防自动裁剪误裁掉能力）+ 上界 13M（防异常膨胀）；后续档用**相对关系**。"
$PX build --full hello.px >/tmp/m85s1.log 2>&1 || { bad "默认 build"; tail -5 /tmp/m85s1.log; }
FULL=$(stat -c %s build/hello 2>/dev/null || echo 0)
if [ "$FULL" -ge 8900000 ] && [ "$FULL" -le 13000000 ]; then ok "全能力体积 $FULL [8.9M, 13M]"; else bad "全能力体积 $FULL 越界 [8900000, 13000000]"; fi
./build/hello >/dev/null 2>&1 && ok "默认产物运行" || bad "默认产物运行"

echo "== [2/5] --full --no-quic（相对判据：裁掉 quic ⇒ 必须显著小于全能力档）=="
$PX build --full --no-quic hello.px >/dev/null 2>&1 && SZ=$(stat -c %s build/hello) || SZ=0
NOQUIC=$SZ
LIM=$((FULL * 60 / 100))
if [ "$SZ" -ge 3800000 ] && [ "$SZ" -lt "$LIM" ]; then ok "--no-quic $SZ（< 全能力 60% = $LIM）"; else bad "--no-quic $SZ 越界 [3800000, $LIM)"; fi

echo "== [3/5] --full --no-quic --no-sqlite（相对判据：再裁 sqlite ⇒ 比上一档更小）=="
$PX build --full --no-quic --no-sqlite hello.px >/dev/null 2>&1 && SZ=$(stat -c %s build/hello) || SZ=0
NOSQL=$SZ
LIM=$((NOQUIC * 90 / 100))
if [ "$SZ" -ge 2700000 ] && [ "$SZ" -lt "$LIM" ]; then ok "--no-quic --no-sqlite $SZ（< 上一档 90% = $LIM）"; else bad "--no-quic --no-sqlite $SZ 越界 [2700000, $LIM)"; fi

echo "== [4/5] 全裁（--no-quic + 10 模块开关 = --min 档 · 相对判据）=="
ALL="--no-quic --no-sqlite --no-ws --no-zip --no-xml --no-aes --no-rsa --no-ed25519 --no-route --no-zlib --no-h2"
$PX build $ALL hello.px >/dev/null 2>&1 && SZ=$(stat -c %s build/hello) || SZ=0
LIM=$((NOSQL * 105 / 100))
if [ "$SZ" -ge 2500000 ] && [ "$SZ" -lt "$LIM" ]; then ok "全裁 $SZ（≈ 上一档 · 差 <5% = $LIM）"; else bad "全裁 $SZ 越界 [2500000, $LIM)"; fi
./build/hello >/dev/null 2>&1 && ok "全裁产物运行" || bad "全裁产物运行"

echo "== [5/5] 裁剪态语义：缺 native → R1001（未定义，非崩溃）+ 核心 http 面保留 =="
$PX build --no-quic --no-sqlite sqlite_dep.px >/dev/null 2>&1 || { bad "sqlite_dep build"; }
if ./build/sqlite_dep 2>&1 | grep -qE "未定义变量: '?sqlite_open" ; then ok "no-sqlite 缺 sqlite_open → R1001"; else bad "no-sqlite 缺 sqlite_open 语义"; fi
$PX build $ALL ws_dep.px >/dev/null 2>&1 || { bad "ws_dep build"; }
if ./build/ws_dep 2>&1 | grep -qE "未定义变量: '?ws_connect" ; then ok "全裁缺 ws_connect → R1001"; else bad "全裁缺 ws_connect 语义"; fi
$PX build $ALL http_dep.px >/dev/null 2>&1 || { bad "http_dep build"; }
if ./build/http_dep 2>&1 | grep -q "Err(net:"; then ok "全裁 http_get 核心保留"; else bad "全裁 http_get 核心保留"; fi

echo
echo "M85-S1 verify: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = 0 ] || exit 1
