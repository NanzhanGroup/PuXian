# m139 · 裸标识符语句是编译错误（qg-issue 87 缺陷 101 的门）
#
# 背景：语句位置上，**裸标识符永远没有效果** —— 调用必须带括号、赋值必须带 `=`，
#   没有第二种解释（cg_stmt 对 ExprStmt 直接 `(void)(…)` 丢弃值，istmt 同样返回 null，
#   故也不存在"隐式返回"的用法）。但旧解析器**接受**它：
#     · 名字未定义（`pass`）⇒ 运行时 R1001 **打挂进程**（第 17 轮实撞：token-cache
#       的 llm_resp.px / llm_msgs.px 共 8 处「字段为 null 就什么都不做」写成 `pass`，
#       上游返回任何 JSON null 即整进程死）
#     · 名字已定义 ⇒ **静默什么都不做**（最坏形态：编译通过、看不出错）
#   同族的"沉默"还有一处：`examples/*.px` 里 228 处 `assert <expr>`（无括号）此前被解析成
#   **两条语句**（`assert` 空转 + 表达式无效果）⇒ 这些断言**从未真正执行**。本门把
#   "至少会报错"钉住：改成 `assert(…)` 后它们才真的断言（已逐条修正 3 处失效期望）。
#
# 本门：正例（合法写法必须编译通过）+ 负例（裸标识符/`pass` 必须编译失败且给可操作提示）
set -u
cd "$(dirname "$0")"
PX=../../tools/px
FAIL=0
mkdir -p build
build_one() {  # $1=文件 → 用用户面默认轨（VM）编译
    "$PX" build "$1" > /tmp/m139_build.log 2>&1
}

check_ok() {   # $1=文件 $2=说明
    if build_one "$1" && "$PX" build "$1" > /dev/null 2>&1; then
        echo "PASS [正例] $2"
    else
        echo "FAIL [正例] $2 —— 应通过却失败"; tail -3 /tmp/m139_build.log; FAIL=$((FAIL+1))
    fi
}
check_bad() {  # $1=文件 $2=期望出现的片段 $3=说明
    if "$PX" build "$1" > /tmp/m139_bad.log 2>&1; then
        echo "FAIL [负例] $3 —— 应报错却通过了（门在自欺）"; FAIL=$((FAIL+1))
    elif grep -q "$2" /tmp/m139_bad.log; then
        echo "PASS [负例] $3"
        grep -m1 "$2" /tmp/m139_bad.log | sed 's/^/       /'
    else
        echo "FAIL [负例] $3 —— 报错了但不是期望的诊断"; tail -3 /tmp/m139_bad.log; FAIL=$((FAIL+1))
    fi
}

echo "== 正例：合法写法必须照旧通过 =="
check_ok ok_call.px        "函数调用 f()"
check_ok ok_str_stmt.px    "字符串字面量语句（只拦裸标识符）"
check_ok ok_pass_var.px    "pass 作变量名（var pass = 0 —— 故不能关键字化）"
check_ok ok_method.px      "方法调用 / 字段访问语句"
check_ok ok_assert.px      "assert(...) 带括号调用"

echo "== 负例：裸标识符 / pass 必须编译期报错 =="
check_bad bad_ident.px   "E2012" "裸标识符语句"
check_bad bad_pass.px    "`pass` 不是 PuXian 关键字" "pass（提示空语句写 0）"
check_bad bad_indent.px  "E2012" "块内缩进的裸标识符"

echo "════════════════════════════════════"
if [ "$FAIL" = "0" ]; then echo "M139-VERIFY-OK"; exit 0; fi
echo "M139-VERIFY-FAIL（$FAIL 项）"; exit 1
