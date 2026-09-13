#!/usr/bin/env bash
# ============================================================
# 内置名册防漂移门（builtin name list drift gate）— M114-S4
# ------------------------------------------------------------
# 背景（PR #8 首次真机 CI 实锤）：
#   `tools/pxlint.px` 与 `tools/pxcheck.px` **各自手抄了一份**「内置函数名」白名单
#   （注释自称"对齐 Rust lint.rs builtin_names"），而**权威名册**在 `selfhost/interp.px`
#   （解释器运行期注册的内置名列表）。副本 → 必然漂移：
#
#   M114-S1 把 lexer/parser/fmtlexer 的诊断从 `print` 改走 `print_err`（**真内置**，
#   interp.px 名册里有）之后，lint 立刻报：
#       tools/fmtlexer.px:66:5: E L002: 未定义变量: 'print_err'
#   —— 门红得对，但**红的原因是名册缺项，不是代码错**。实测 pxlint/pxcheck 各缺
#   **27 个**真实内置（print_err / flush / round / floor / ceil / exp / log / sin …
#   与 dict / tuple 这类构造器）。
#
# 本门判据（三项，任一不成立即红）：
#   ① interp.px 名册 ⊆ pxlint 名册（解释器认得的名字，lint 不许报未定义）
#   ② interp.px 名册 ⊆ pxcheck 名册
#   ③ pxlint 名册 == pxcheck 名册（两份工具名册本就声明"对齐"，不许各自漂移）
#   ⚠️ 反向**不判**：工具名册里有 interp.px 名册外的名字（如 tcp_listen / sqlite_open
#      这类由 runtime/FFI 注册、不经 interp 名册的内置）是**合法**的，故只判单向包含。
#
# 用法：selfhost/builtin_list_check.sh      # rc=0 一致 / rc=1 漂移（并打印缺项）
# ============================================================
set -u
cd "$(dirname "$0")/.."
# 名册解析必须是**确定性**的：排序一律逐字节（locale 无关）。
# 教训见 selfhost/rebake_bin.sh 顶部（同一轮 PR #8 CI 红：`sort` 受 locale 影响）。
export LC_ALL=C
export LANG=C

INTERP=selfhost/interp.px
PLINT=tools/pxlint.px
PCHK=tools/pxcheck.px

for f in "$INTERP" "$PLINT" "$PCHK"; do
    [ -f "$f" ] || { echo "❌ 缺文件：$f" >&2; exit 2; }
done

# 权威名册：interp.px 里 `let names = [ "a", "b", ... ]`（运行期注册的内置名）。
interp_names() {
    grep -o 'let names = \[[^]]*\]' "$INTERP" \
        | grep -o '"[^"]*"' | tr -d '"' | sort -u
}
# 工具名册：`let BUILTINS = split("a b c", " ")`（空格分隔字符串 + split 构造）。
tool_names() {
    sed -n 's/.*split("\(.*\)", " ").*/\1/p' "$1" | tr ' ' '\n' | sed '/^$/d' | sort -u
}

ni=$(interp_names | wc -l)
[ "$ni" -ge 50 ] || { echo "❌ 名册解析异常：interp.px 只解析出 $ni 个名字（正则失配？）" >&2; exit 2; }
nl=$(tool_names "$PLINT" | wc -l)
nc=$(tool_names "$PCHK" | wc -l)
[ "$nl" -ge 50 ] && [ "$nc" -ge 50 ] || {
    echo "❌ 名册解析异常：pxlint=$nl / pxcheck=$nc 个名字（正则失配？）" >&2; exit 2; }

bad=0

# ① ② interp ⊆ 工具
for pair in "pxlint|$PLINT" "pxcheck|$PCHK"; do
    tag=${pair%%|*}; file=${pair#*|}
    miss=$(comm -23 <(interp_names) <(tool_names "$file"))
    if [ -n "$miss" ]; then
        echo "❌ $tag 名册缺 $(printf '%s\n' "$miss" | wc -l) 个真实内置（interp.px 认得、$tag 会误报 L002）："
        printf '%s\n' "$miss" | sed 's/^/     /'
        bad=$((bad+1))
    else
        echo "    ✅ $tag 名册 ⊇ interp.px 内置名册（${ni} 个名字全覆盖）"
    fi
done

# ③ 两份工具名册彼此一致
if diff <(tool_names "$PLINT") <(tool_names "$PCHK") > /tmp/builtin_list_drift.txt; then
    echo "    ✅ pxlint 名册 == pxcheck 名册（${nl} 个名字逐字节一致）"
else
    echo "❌ pxlint 与 pxcheck 名册彼此漂移（两文件都声明「对齐」，应同增同减）："
    sed 's/^/     /' /tmp/builtin_list_drift.txt | head -20
    bad=$((bad+1))
fi

if [ "$bad" -gt 0 ]; then
    echo "❌ 内置名册漂移 $bad 处（权威名册在 $INTERP；工具名册是副本，必须同步）"
    exit 1
fi
echo "✅ 内置名册一致（interp.px 权威 ⊆ pxlint/pxcheck 副本，且两副本相同）"
exit 0
