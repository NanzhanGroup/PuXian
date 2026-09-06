#!/bin/bash
# M85-S2 (qg-issue 24) --min profile + 裁剪组合 + target 折叠路径验证
#   --min = 聚合 --no-quic + sqlite/ws/zip/xml/aes/rsa/ed25519/route/zlib/h2 全裁（~2.7M）
#   验证：--min 体积断言 / 与手写全裁组合等价 / --target x86_64 + 裁剪叠加 /
#         --min 态缺 native R1001 / 核心 HTTP native 保留
#   aarch64/armv7/riscv64 真机交叉组合由 CI 覆盖（本机无 musl 交叉工具链）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
rm -rf build; mkdir -p build
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
ALL="--no-quic --no-sqlite --no-ws --no-zip --no-xml --no-aes --no-rsa --no-ed25519 --no-route --no-zlib --no-h2"

echo "== [1/5] --min profile 体积断言（基线 2713472）=="
$PX build --min hello.px >/dev/null 2>&1 && SZ=$(stat -c %s build/hello) || SZ=0
if [ "$SZ" -ge 2500000 ] && [ "$SZ" -le 2900000 ]; then ok "--min $SZ"; else bad "--min $SZ 期望 ~2713472"; fi
./build/hello >/dev/null 2>&1 && ok "--min 产物运行" || bad "--min 产物运行"

echo "== [2/5] --min == 手写全裁组合（同体积）=="
$PX build $ALL hello.px >/dev/null 2>&1 && SZ2=$(stat -c %s build/hello) || SZ2=0
[ "$SZ" = "$SZ2" ] && ok "--min($SZ) == 全裁($SZ2)" || bad "--min($SZ) != 全裁($SZ2)"

echo "== [3/5] --target x86_64 折叠 + 裁剪叠加（quic 开 + 去 sqlite，实测 7968768 < 全能力 9010184）=="
$PX build --target x86_64 --no-sqlite hello.px >/dev/null 2>&1 && SZ3=$(stat -c %s build/hello) || SZ3=0
if [ "$SZ3" -ge 7400000 ] && [ "$SZ3" -lt 8300000 ]; then ok "--target x86_64 --no-sqlite $SZ3（<9.0M 全能力基线，裁剪生效）"; else bad "--target x86_64 --no-sqlite $SZ3 期望 ~7.97M"; fi
./build/hello >/dev/null 2>&1 && ok "target+裁剪产物运行" || bad "target+裁剪产物运行"

echo "== [4/5] --min 态缺 native → R1001（未定义，非崩溃）=="
$PX build --min sqlite_dep.px >/dev/null 2>&1
if ./build/sqlite_dep 2>&1 | grep -q "未定义变量: sqlite_open"; then ok "--min 缺 sqlite_open → R1001"; else bad "--min 缺 sqlite_open 语义"; fi
$PX build --min ws_dep.px >/dev/null 2>&1
if ./build/ws_dep 2>&1 | grep -q "未定义变量: ws_connect"; then ok "--min 缺 ws_connect → R1001"; else bad "--min 缺 ws_connect 语义"; fi

echo "== [5/5] --min 核心 HTTP native 保留 =="
$PX build --min http_dep.px >/dev/null 2>&1
if ./build/http_dep 2>&1 | grep -q "Err(net:"; then ok "--min http_get 核心保留"; else bad "--min http_get 核心保留"; fi

echo
echo "M85-S2 verify: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = 0 ] || exit 1
