#!/usr/bin/env bash
# ============================================================
# M70-S2 pxfmt 多行表达式验证（表达式跨行 / G2 收口）
#  ① 多行输入 → golden：跨行结构保留（不压平）+ 空格/缩进规范化
#  ② golden 幂等（fmt 两次不变）
#  ③ fmt 输出语义等价：重 lex token 序列一致（忽略 换行/缩进/去缩进/EOF）
#  ④ fmt 输出可被新 parser 往返：pxi 解释运行输出 == 期望值
#  ⑤ --check：多行已格式 golden rc=0 / 未格式输入 rc=1
# 依赖：bootstrap/pxfmt（M64a）+ bootstrap/pxi（M70-S1 重建）
# 用法：./verify_fmt_multiline.sh
# ============================================================
set -u
cd "$(dirname "$0")"
ROOT="$(cd ../.. && pwd)"
PXFMT="$ROOT/bootstrap/pxfmt"
PXI="$ROOT/bootstrap/pxi"
PXC="$ROOT/tools/pxc"
IN=m70_fmt_multiline_in.px
GOLD=m70_fmt_multiline_gold.px
fail=0

chk() {
    if [ "$1" = "0" ]; then echo "  ✅ $2"; else echo "  ❌ $2"; fail=1; fi
}

echo "══════════ M70-S2 pxfmt 多行表达式验证 ══════════"

echo "── ① 多行输入 → golden"
"$PXFMT" "$IN" > /tmp/m70f_out.px 2>/dev/null; o=$?
chk $o "pxfmt 多行输入 exit 0"
diff -q /tmp/m70f_out.px "$GOLD" >/dev/null 2>&1 && dok=0 || dok=1
chk $dok "输出与 m70_fmt_multiline_gold.px 逐字节一致"
# 跨行结构保留（未压平）
grep -q 'let a = \[' /tmp/m70f_out.px && a=0 || a=1; chk $a "多行 list 结构保留"
grep -q '"x": 10,' /tmp/m70f_out.px && a=0 || a=1; chk $a "多行 dict 结构保留"
grep -q 'let s = max(' /tmp/m70f_out.px && a=0 || a=1; chk $a "多行调用结构保留"
grep -q 'x for x in \[1, 2, 3, 4, 5, 6\]' /tmp/m70f_out.px && a=0 || a=1; chk $a "多行 listcomp 结构保留"
grep -q 'let a = \[' /tmp/m70f_out.px && a=0 || a=1; chk $a "赋值号两侧空格规范化"

echo "── ② 幂等"
"$PXFMT" "$GOLD" > /tmp/m70f_g2.px 2>/dev/null
diff -q /tmp/m70f_g2.px "$GOLD" >/dev/null 2>&1 && iok=0 || iok=1
chk $iok "golden 幂等（二次 fmt 不变）"

echo "── ③ 语义等价（重 lex token 序列，忽略布局 token）"
for f in "$IN" /tmp/m70f_out.px; do
    "$PXC" lex "$f" 2>/dev/null | awk '{ if ($2!="换行" && $2!="缩进" && $2!="去缩进" && $2!="EOF") printf "%s %s\n", $2, (NF>=3?$3:"-") }' > "$f.sem"
done
diff -q "$IN.sem" /tmp/m70f_out.px.sem >/dev/null 2>&1 && sok=0 || sok=1
chk $sok "格式化前后语义 token 序列一致"
rm -f "$IN.sem"

echo "── ④ fmt 输出可往返执行（pxi 双模式语义）"
exp=$("$PXI" "$IN" 2>/dev/null)
got=$("$PXI" /tmp/m70f_out.px 2>/dev/null)
[ "$exp" = "$got" ] && eok=0 || eok=1
chk $eok "fmt 输出 pxi 执行结果与原始一致"
echo "$got" | grep -q "30" && eok=0 || eok=1; chk $eok "多行 dict 求值正确 (30)"

echo "── ⑤ --check"
"$PXFMT" --check "$IN" >/dev/null 2>&1; r1=$?
[ "$r1" = "1" ] && cok=0 || cok=1; chk $cok "--check 未格式输入 rc=1（实际 $r1）"
"$PXFMT" --check "$GOLD" >/dev/null 2>&1; r0=$?
[ "$r0" = "0" ] && cok=0 || cok=1; chk $cok "--check 已格式 golden rc=0（实际 $r0）"

echo ""
if [ "$fail" = "0" ]; then
    echo "🎉 M70-S2 pxfmt 多行验证全部通过"
    exit 0
else
    echo "❌ 存在失败项"
    exit 1
fi
