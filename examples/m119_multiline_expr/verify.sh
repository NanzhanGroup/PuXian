# m119_multiline_expr · 括号内隐式续行（Go/Python 语义）
#
# 探针来源：文殊 supervisor 的 Issue 72 修复现场 —— 给一行日志加长后报
#   `E2001 期望 ')'，实际得到 +`，只能把整行挤成一行才编得过。
# 实测：**任何长表达式都不得在运算符处断行**（编译器自身源码亦被迫如此）：
#     print("a" +          print("a"
#           "b")      vs         + "b")
#   → 前者「E2001 意外的 token: 换行」，后者「E2001 期望 ')'，实际得到 +」
#
# 修法（M119）：括号深度 > 0 时
#   ① 期望操作数处跳过换行（parse_unary 开头）→ 覆盖「运算符在行尾」；
#   ② 检查运算符时向后看穿换行（chk_op）→ 覆盖「运算符在行首」。
#   ⚠️ 行首一元运算符（- ~ not）不参与 ②（与「新语句以一元运算符开头」歧义）。
#   ⚠️ 第 18 轮修订：**括号外（深度 0）**的边界**不再是**「一律不续行」—— M129
#      （qg-issue 87 缺陷 33）起，括号外**行尾**运算符也续行（Go 的自动分号插入语义，
#      见 docs/PUXIAN_CHEATSHEET.md 第 43 条），本门第 5 节据实改为正控 + 行首负控。
#
# 用法：bash examples/m119_multiline_expr/verify.sh
set -u
cd "$(dirname "$0")/../.."
PX=./tools/px
TMP=$(mktemp -d /tmp/m119_XXXXXX)
trap 'rm -rf "$TMP"' EXIT
FAIL=0
ok()  { echo "  ✅ $1"; }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
# 双轨跑：解释轨（px run）+ 编译轨（px build → 运行）。两轨判据必须一致。
both() {  # $1=名 $2=文件 $3=期望 stdout
    if out=$(timeout 60 $PX run "$2" 2>&1) && [ "$out" = "$3" ]; then ok "$1 解释轨"
    else bad "$1 解释轨：$out（期望：$3）"; fi
    if $PX build "$2" >/dev/null 2>&1 && out=$(timeout 60 "$TMP/build/$(basename "${2%.px}")" 2>&1) && [ "$out" = "$3" ]; then ok "$1 编译轨"
    else bad "$1 编译轨：$out（期望：$3）"; fi
}

echo "── 1. 运算符在**行尾**（括号内，最常用的续行写法）"
cat > "$TMP/t1.px" <<'PXEOF'
def main():
    var a = "x"
    print("a" +
          "b" + a)
    print(1 +
          2 + 3)
    var s = ("p" +
             "q")
    print(s)
PXEOF
both "行尾运算符" "$TMP/t1.px" "$(printf 'abx\n6\npq')"

echo "── 2. 运算符在**行首**（括号内）"
cat > "$TMP/t2.px" <<'PXEOF'
def main():
    var a = "x"
    print("a"
          + "b" + a)
    var n = (10
             + 20
             * 2)
    print(n)
    var xs = [1
              , 2, 3]
    print(len(xs))
    var d = {
        "k": "v1"
             + "v2"
    }
    print(d["k"])
PXEOF
both "行首运算符" "$TMP/t2.px" "$(printf 'abx\n50\n3\nv1v2')"

echo "── 3. 多行调用实参 + 长表达式混排（真实日志拼接形状）"
cat > "$TMP/t3.px" <<'PXEOF'
def main():
    var nm = "token-cache"
    var st = "S"
    var valve = "12h"
    print("[守护] ⚠ " + nm
          + " 已连续运行超 " + valve
          + "（状态 " + st + "，非僵尸）→ 不重拉，继续等待")
    var m = json_parse("{}")
    m.set("a",
          1)
    print(m["a"])
PXEOF
both "长日志拼接" "$TMP/t3.px" "$(printf '[守护] ⚠ token-cache 已连续运行超 12h（状态 S，非僵尸）→ 不重拉，继续等待\n1')"

echo "── 4. 回归：M118 的「调用实参里匿名函数多行体」仍可用（括号内语句上下文）"
cat > "$TMP/t4.px" <<'PXEOF'
def main():
    var ys = [1, 2, 3, 4]
    var evens = filter(ys, fn (x):
        var r = x % 2
        return r == 0)
    print(json_stringify(evens))
PXEOF
if out=$(timeout 60 $PX run "$TMP/t4.px" 2>&1) && [ "$out" = "[2,4]" ]; then ok "多行匿名函数体 解释轨"; else bad "多行匿名函数体 解释轨：$out"; fi
if $PX build "$TMP/t4.px" >/dev/null 2>&1 && out=$(timeout 60 "$TMP/build/t4" 2>&1) && [ "$out" = "[2,4]" ]; then ok "多行匿名函数体 编译轨"; else bad "多行匿名函数体 编译轨：$out"; fi

echo "── 5. 括号外：**行尾**运算符续行（M129/缺陷 33 的 Go 语义；本节原为「必须报错」的负控，"
echo "        该期望已被 M129 有意变更 → 第 18 轮提交前预检据实改写）"
cat > "$TMP/t5.px" <<'PXEOF'
def main():
    var s = "a" +
            "b"
    print(s)
    var ok = 1 == 1 and
             2 == 2
    print(ok)
PXEOF
both "括号外·行尾运算符续行" "$TMP/t5.px" "$(printf 'ab\ntrue')"

echo "── 5b. 负控：括号外**行首**运算符不续行（Go 亦不接受；见速查表第 43 条）"
cat > "$TMP/t5b.px" <<'PXEOF'
def main():
    var s = "a"
            + "b"
    print(s)
PXEOF
err=$(timeout 60 $PX run "$TMP/t5b.px" 2>&1 | tr '\n' ' ')
if echo "$err" | grep -q "E2001"; then ok "括号外·行首运算符 → E2001（与 Go 同边界）"; else bad "负控未报错：$err"; fi

echo "── 6. 负控：括号内**行首一元运算符**不参与续行（歧义保护：与新语句同形）"
cat > "$TMP/t6.px" <<'PXEOF'
def main():
    print(1
          - 2)
PXEOF
err=$(timeout 60 $PX run "$TMP/t6.px" 2>&1 | tr '\n' ' ')
if echo "$err" | grep -q "E2001"; then ok "行首 - 不续行 → E2001（写法：把运算符放行尾）"; else bad "负控未报错：$err"; fi

echo "── 7. 负控：语义未变 —— 同优先级表达式求值顺序与结合性不受影响"
cat > "$TMP/t7.px" <<'PXEOF'
def main():
    # 左结合：((10 - 3) - 2) = 5；若续行被错误折叠成右结合会得到 9
    var x = (10
             - 3
             - 2)
    # 括号内换行不得改变优先级：2 + 3 * 4 = 14
    var y = (2
             + 3
             * 4)
    print(x, y)
PXEOF
# 行首 - 属歧义保护范围 → 这里用行尾写法验证结合性（等价表达式）
cat > "$TMP/t7.px" <<'PXEOF'
def main():
    var x = (10 -
             3 -
             2)
    var y = (2 +
             3 *
             4)
    print(x, y)
PXEOF
both "结合性/优先级不变" "$TMP/t7.px" "$(printf '5 14')"

echo
if [ "$FAIL" = "0" ]; then echo "✅ m119 全部用例通过"; else echo "❌ 失败 $FAIL 项"; fi
exit $FAIL
