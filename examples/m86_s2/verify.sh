#!/bin/bash
# M86-S2 (qg-issue 25) 自动按需裁剪验证（裸 px build = 引用集自动最小）
#   实现：pxc build C 产物提取 px_get_global → native_mod_map 反推被引用模块 →
#         未引用模块自动补裁（--no-xxx 复用 M85 链路）；--full/--max 逃生舱全能力；
#         显式 flag 优先级 > 自动；解析失败退全量。
#   基线：裸 hello ≈ 2.7M（M85 --min 档）/ --full ≈ 9.0M / sqlite 保留 ≈ 3.76M
#   依赖：tools/px（M86-S2 版）+ bootstrap/pxc + runtime/native_mod_map.txt
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/px
rm -rf build; mkdir -p build
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
SZ() { stat -c %s "$1" 2>/dev/null || echo 0; }

echo "== [1/7] 裸 px build hello = 自动最小（引用集只 core → 11 模块全裁，M85 --min 档 2713472）=="
$PX build hello.px >/tmp/m86s2.log 2>&1 || { bad "裸 build"; tail -3 /tmp/m86s2.log; }
S=$(SZ build/hello)
if [ "$S" -ge 2500000 ] && [ "$S" -le 2900000 ]; then ok "裸 build $S（自动最小档）"; else bad "裸 build $S 期望 ~2713472"; fi
./build/hello >/dev/null 2>&1 && ok "自动最小产物运行" || bad "自动最小产物运行"

echo "== [2/7] --full hello = 全能力（9.0M 基线）=="
rm -rf build
$PX build --full hello.px >/dev/null 2>&1 && S=$(SZ build/hello) || S=0
if [ "$S" -ge 8900000 ] && [ "$S" -le 9150000 ]; then ok "--full $S"; else bad "--full $S 期望 ~9010184"; fi

echo "== [3/7] --max hello = 同 --full（逃生舱别名）=="
rm -rf build
$PX build --max hello.px >/dev/null 2>&1 && S=$(SZ build/hello) || S=0
if [ "$S" -ge 8900000 ] && [ "$S" -le 9150000 ]; then ok "--max $S"; else bad "--max $S 期望 ~9010184"; fi

echo "== [4/7] sqlite_dep 裸 = sqlite 保留可运行（引用集命中 sqlite 模块）=="
rm -rf build
$PX build sqlite_dep.px >/dev/null 2>&1 && S=$(SZ build/sqlite_dep) || S=0
if [ "$S" -ge 3000000 ] && [ "$S" -le 4500000 ]; then ok "sqlite 保留 $S"; else bad "sqlite 保留 $S 期望 ~3759248"; fi
O=$(./build/sqlite_dep 2>&1); [ $? = 0 ] && echo "$O" | grep -qv "未定义变量" && ok "sqlite_dep 运行无 R1001（$O）" || bad "sqlite_dep 运行: $O"

echo "== [5/7] 显式 flag 优先：--no-sqlite sqlite_dep → 裁 sqlite（R1001 未定义）=="
rm -rf build
$PX build --no-sqlite sqlite_dep.px >/dev/null 2>&1 && S=$(SZ build/sqlite_dep) || S=0
if [ "$S" -le 2900000 ]; then ok "--no-sqlite 体积 $S（显式裁剪覆盖自动保留）"; else bad "--no-sqlite 体积 $S 应裁 sqlite"; fi
if ./build/sqlite_dep 2>&1 | grep -q "未定义变量: sqlite_open"; then ok "显式裁剪 → R1001"; else bad "显式裁剪语义"; fi

echo "== [6/7] 引用 9 模块的 allmod 裸 = 保留被引用模块（≈9M，route/h2 未引用裁掉）=="
rm -rf build
$PX build allmod.px >/dev/null 2>&1 && S=$(SZ build/allmod) || S=0
if [ "$S" -ge 8500000 ]; then ok "allmod 裸保留引用模块 $S"; else bad "allmod 裸 $S 应 ~9.0M"; fi
# 运行：sqlite_open 存在（第 1 行成功），zip_pack 参数错（native 在，非 R1001）
O=$(./build/allmod 2>&1); echo "$O" | grep -q "zip_pack 需要" && ok "allmod native 均在（zip_pack 参数校验，非未定义）" || bad "allmod 运行: $O"

echo "== [7/7] native_mod_map 与生成器一致（gen_native_map.sh 重生成 diff 空）=="
MAP=../../runtime/native_mod_map.txt
GEN=../../tools/gen_native_map.sh
bash "$GEN" > /tmp/m86s2_map.txt 2>/dev/null
if diff -q /tmp/m86s2_map.txt "$MAP" >/dev/null 2>&1; then ok "map 与生成器一致（$(wc -l < "$MAP") 行）"; else bad "map 与生成器不一致"; fi

echo
echo "M86-S2 verify: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = 0 ] || exit 1
