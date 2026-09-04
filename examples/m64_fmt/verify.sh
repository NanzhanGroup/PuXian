#!/usr/bin/env bash
# ============================================================
# M64-S2 pxfmt 一键验证（PuXian 自举格式化器）
#   ① 乱格式输入 → golden 逐字节一致
#   ② golden 幂等（fmt 两次不变）
#   ③ 语义等价（格式化前后重 lex token 序列一致，Int/Float/Str 值不变）
#   ④ --check 语义（未格式 rc=1 / 已格式 rc=0）
#   ⑤ --diff 输出 unified diff（---/+++ 头 + @@ hunk）
#   ⑥ -w 写回
#   ⑦ 自举 dogfood：fmt_core.px / pxfmt.px 自身 --check 通过
# 依赖：bootstrap/pxfmt（tools/pxc fmt）已构建
# 用法：./verify.sh
# ============================================================
set -u
cd "$(dirname "$0")"
ROOT="$(cd ../.. && pwd)"
PXFMT="$ROOT/bootstrap/pxfmt"
PXC="$ROOT/tools/pxc"
IN=m64_fmt_in.px
GOLD=m64_fmt_gold.px
fail=0

chk() {
    if [ "$1" = "0" ]; then echo "  ✅ $2"; else echo "  ❌ $2"; fail=1; fi
}

echo "══════════ M64-S2 pxfmt 验证 ══════════"

[ -x "$PXFMT" ] || { echo "缺少 bootstrap/pxfmt（先 tools/pxc build tools/pxfmt.px && cp tools/build/pxfmt bootstrap/pxfmt）" >&2; exit 1; }

echo "── 版本"
v=$("$PXFMT" --version 2>&1); chk $? "pxfmt --version exit 0"
echo "$v" | grep -q "pxfmt 0.1.0" && vok=0 || vok=1; chk $vok "pxfmt --version 文本 (pxfmt 0.1.0 ...)"

echo "── ① 输入→golden 一致"
"$PXFMT" "$IN" > /tmp/m64f_out.px 2>/dev/null; o=$?
chk $o "pxfmt 输入 exit 0"
diff -q /tmp/m64f_out.px "$GOLD" >/dev/null 2>&1 && dok=0 || dok=1
chk $dok "输出与 m64_fmt_gold.px 逐字节一致"
grep -q "def fib(n: int) -> int:" /tmp/m64f_out.px && a=0 || a=1; chk $a "参数/箭头空格 (def fib(n: int) -> int:)"
grep -q "let f = 1.0" /tmp/m64f_out.px && a=0 || a=1; chk $a "整值浮点保留小数点 (1.0)"
grep -q 'let d = {"name": "px", "v": 1}' /tmp/m64f_out.px && a=0 || a=1; chk $a "dict 冒号后空格"
grep -q "let neg = -3" /tmp/m64f_out.px && a=0 || a=1; chk $a "一元负号赋值空格 (neg = -3)"
grep -q 'let interp = "val=${x}end"' /tmp/m64f_out.px && a=0 || a=1; chk $a "插值字符串保留原文 (spec)"
grep -q 'let uni = "\\u{4f60}\\u{597d}"' /tmp/m64f_out.px && a=0 || a=1; chk $a "Unicode 转义保留原文"
grep -qF "let slice = arr[0:2]" /tmp/m64f_out.px && a=0 || a=1; chk $a "切片冒号紧贴 (arr[0:2])"
grep -q "# 行首注释" /tmp/m64f_out.px && a=0 || a=1; chk $a "行首注释保留"
grep -q "let x = 1  # 行内注释" /tmp/m64f_out.px && a=0 || a=1; chk $a "行内注释保留（前补 2 空格）"

echo "── ② 幂等"
"$PXFMT" "$GOLD" > /tmp/m64f_g2.px 2>/dev/null
diff -q /tmp/m64f_g2.px "$GOLD" >/dev/null 2>&1 && iok=0 || iok=1
chk $iok "golden 幂等（二次 fmt 不变）"

echo "── ③ 语义等价（重 lex token 序列）"
for f in "$IN" /tmp/m64f_out.px; do
    "$PXC" lex "$f" 2>/dev/null | awk '{ if ($2!="换行" && $2!="缩进" && $2!="去缩进" && $2!="EOF") printf "%s %s\n", $2, (NF>=3?$3:"-") }' > "$f.sem"
done
diff -q "$IN.sem" /tmp/m64f_out.px.sem >/dev/null 2>&1 && sok=0 || sok=1
chk $sok "格式化前后语义 token 序列一致"
rm -f "$IN.sem"

echo "── ④ --check"
"$PXFMT" --check "$IN" >/dev/null 2>&1; r1=$?
[ "$r1" = "1" ] && cok=0 || cok=1; chk $cok "--check 未格式输入 rc=1（实际 $r1）"
"$PXFMT" --check "$GOLD" >/dev/null 2>&1; r0=$?
[ "$r0" = "0" ] && cok=0 || cok=1; chk $cok "--check 已格式 golden rc=0（实际 $r0）"

echo "── ⑤ --diff"
d=$("$PXFMT" --diff "$IN" 2>/dev/null)
echo "$d" | head -1 | grep -q -- "--- original" && h=0 || h=1; chk $h "--diff 以 --- original 开头"
echo "$d" | grep -q "@@" && h=0 || h=1; chk $h "--diff 含 @@ hunk"
"$PXFMT" --diff "$GOLD" 2>/dev/null | grep -q "@@" && h=1 || h=0; chk $h "--diff 已格式输出为空"

echo "── ⑥ -w 写回"
cp "$IN" /tmp/m64f_w.px
"$PXFMT" -w /tmp/m64f_w.px >/dev/null 2>&1; w=$?
chk $w "-w 写回 exit 0"
diff -q /tmp/m64f_w.px "$GOLD" >/dev/null 2>&1 && w=0 || w=1
chk $w "-w 写回内容 == golden"

echo "── ⑦ 自举 dogfood"
"$PXFMT" --check "$ROOT/tools/fmt_core.px" >/dev/null 2>&1; chk $? "fmt_core.px 自身格式正确"
"$PXFMT" --check "$ROOT/tools/pxfmt.px" >/dev/null 2>&1; chk $? "pxfmt.px 自身格式正确"
"$PXC" fmt --version >/dev/null 2>&1; chk $? "pxc fmt 子命令可用"

echo ""
if [ "$fail" = "0" ]; then
    echo "🎉 M64-S2 pxfmt 全部通过"
    exit 0
else
    echo "❌ 存在失败项"
    exit 1
fi
