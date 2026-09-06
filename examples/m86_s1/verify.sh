#!/bin/bash
# M86-S1 (qg-issue 25) 引用集收集（px refs）验证
#   实现：pxc build 编译 C 产物（stdout）→ 提取 px_get_global("name") 引用名集合
#         （M86-S1 实现选择：C 产物静态提取，零编译器改动、零自举风险）
#   验证：hello 核心集 / 条件分支静态收集 / import 递归（多文件 + stdlib）/ 异常退全量 /
#         9 可裁模块代表 native 全命中（native_mod_map 侧 S2 过滤）
#   依赖：tools/px + bootstrap/pxc（仓库自带，零改动）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/px
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
refs() { "$PX" refs "$1" 2>/dev/null | LC_ALL=C sort -u; }

echo "== [1/6] hello：核心 native（print/to_upper）被收集 =="
R=$(refs hello.px)
echo "$R" | grep -qx "print" && ok "refs 含 print" || bad "refs 缺 print: $(echo "$R" | tr '\n' ' ')"
echo "$R" | grep -qx "to_upper" && ok "refs 含 to_upper" || bad "refs 缺 to_upper"

echo "== [2/6] 条件分支引用（if 内 ws_connect 静态收集，不管运行分支）=="
R=$(refs ws_if.px)
echo "$R" | grep -qx "ws_connect" && ok "refs 含 ws_connect（条件分支静态收集）" || bad "refs 缺 ws_connect"
echo "$R" | grep -qx "print" && ok "refs 含 print" || bad "refs 缺 print"

echo "== [3/6] import 递归（多文件：imp_a → imp_b 用 aes native）=="
R=$(refs imp_a.px)
echo "$R" | grep -qx "aes_encrypt_ecb" && ok "refs 含 aes_encrypt_ecb（import 递归）" || bad "refs 缺 aes_encrypt_ecb"
echo "$R" | grep -qx "imp_b_go" && ok "refs 含用户函数名 imp_b_go（map 侧会过滤）" || bad "refs 缺 imp_b_go"

echo "== [4/6] stdlib 调用链（import std.collections → 库内部 native 递归收集）=="
R=$(refs stdlib_dep.px)
echo "$R" | grep -qx "len" && ok "refs 含 len（stdlib 库内部 native 递归收集）" || bad "refs 缺 len"
echo "$R" | grep -qx "zip_lists" && ok "refs 含 zip_lists（库函数名）" || bad "refs 缺 zip_lists"

echo "== [5/6] 解析异常源码 → 退全量（非 0 / 空输出）=="
"$PX" refs bad.px >/tmp/m86s1_bad.txt 2>/dev/null
if [ $? -ne 0 ] || [ ! -s /tmp/m86s1_bad.txt ]; then ok "bad.px → 非0/空输出（退全量标记）"; else bad "bad.px 应失败: $(cat /tmp/m86s1_bad.txt | head -2 | tr '\n' ' ')"; fi

echo "== [6/6] 可裁模块代表 native 全命中（allmod 引用面：sqlite/ws/zip/xml/aes/rsa/ed25519/zlib/quic 9 模块）=="
R=$(refs allmod.px)
C_MOD=0; TOTAL=9
for pair in sqlite_open:sqlite ws_connect:ws zip_pack:zip xml_parse:xml aes_encrypt_ecb:aes rsa_gen_key:rsa ed25519_sign:ed25519 zlib_compress:zlib quic_listen:quic; do
    n="${pair%%:*}"; m="${pair##*:}"
    if echo "$R" | grep -qx "$n"; then ok "refs 含 $n（$m）"; C_MOD=$((C_MOD+1)); else bad "refs 缺 $n（$m）"; fi
done
[ "$C_MOD" = "$TOTAL" ] && ok "9 模块代表 native 全命中" || bad "模块代表 $C_MOD/$TOTAL"
# route 模块 native 名 "route" 与用户函数撞名风险 → 不在此断言（S2 用 webroute 库场景语义验证）

echo
echo "M86-S1 verify: PASS=$PASS FAIL=$FAIL"
[ "$FAIL" = 0 ] || exit 1
