#!/usr/bin/env bash
# M64-S5 doc 生成器自测验证
# 注：文档输出含 markdown 反引号 `，不能走 eval+chk（反引号会被命令替换），
# 此处直接 if/grep 断言；grep pattern 用反引号需以变量携带并 -F 字面匹配。
set -u
cd "$(dirname "$0")/../.." || exit 1
PXC="$PWD/tools/pxc"
pass=0; fail=0
ok() { echo "  PASS $1"; pass=$((pass+1)); }
no() { echo "  FAIL $1"; fail=$((fail+1)); }
BT='`'   # 反引号字面（grep -F 用）
echo "[1] sample.px 生成 Markdown 结构"
out=$("$PXC" doc examples/m64_doc/sample.px 2>&1)
echo "$out" | grep -q '# sample.px API 文档' && ok "标题 basename" || no "标题 basename"
echo "$out" | grep -q '由 .px doc. 自动生成' && ok "自动生成注记" || no "自动生成注记"
echo "$out" | grep -q '^## 函数' && ok "函数节" || no "函数节"
echo "$out" | grep -Fq "### ${BT}def add(a: int, b: int) -> int:${BT}" && ok "add 签名反引号" || no "add 签名反引号"
echo "$out" | grep -q '计算两数之和' && ok "add 文档（紧跟 ##）" || no "add 文档（紧跟 ##）"
echo "$out" | grep -q '参数 a、b：加数，返回和' && ok "参数说明合并" || no "参数说明合并"
echo "$out" | grep -q '普贤自测样本' && ok "文件头说明并入首个定义（对齐 Rust）" || no "文件头说明并入首个定义（对齐 Rust）"
echo "$out" | grep -Fq "### ${BT}def greet():${BT}" && ok "greet 列示" || no "greet 列示"
echo "$out" | grep -q '_无文档注释。_' && ok "无文档注释标记" || no "无文档注释标记"
echo "$out" | grep -q '^## 类型' && ok "类型节" || no "类型节"
echo "$out" | grep -Fq "### ${BT}struct Point:${BT}" && ok "struct Point 签名" || no "struct Point 签名"
echo "$out" | grep -Fq "### ${BT}enum Color:${BT}" && ok "enum Color 签名" || no "enum Color 签名"
echo "[2] --output 写文件"
rm -f /tmp/m64_doc_out.md
"$PXC" doc examples/m64_doc/sample.px --output /tmp/m64_doc_out.md >/dev/null 2>&1
[ -s /tmp/m64_doc_out.md ] && ok "文件生成" || no "文件生成"
grep -q '计算两数之和' /tmp/m64_doc_out.md && ok "文件内容含文档" || no "文件内容含文档"
rm -f /tmp/m64_doc_out.md
echo "[3] stdlib/collections.px 真实 dogfood（## 语料库）"
out3=$("$PXC" doc stdlib/collections.px 2>&1)
echo "$out3" | grep -q '# collections.px API 文档' && ok "stdlib 标题" || no "stdlib 标题"
echo "$out3" | grep -q '去重：返回新列表' && ok "unique 文档命中" || no "unique 文档命中"
echo "$out3" | grep -q '展平一层' && ok "flatten 文档命中" || no "flatten 文档命中"
echo "$out3" | grep -q '按键分组' && ok "group_by 文档命中" || no "group_by 文档命中"
echo
echo "结果: $pass 通过 / $fail 失败"
[ $fail -eq 0 ] || exit 1
