#!/usr/bin/env bash
# ============================================================
# 内置名册防漂移门（builtin name list drift gate）
#   M114-S4 建立（Issue 63）· M114 尾重写为「单一事实源」判据
# ------------------------------------------------------------
# 背景（Issue 63，PR #8 首次真机 CI 实锤）：
#   原先 `tools/pxlint.px` 与 `tools/pxcheck.px` **各自手抄一份**「内置函数名」
#   白名单，而真正的事实源是 **runtime 注册表**。手抄 ⇒ 双向漂移：
#     · 各缺 27 个真实内置（print_err / flush / round / floor / mmap / ffi_call …）
#       ⇒ M114-S1 的诊断改走 `print_err` 后，lint 立刻误报 L002（门红得对、
#         但红的原因是名册缺项，不是代码错）；
#     · 反向漏收 54 项 runtime 已注册名（quic_connect / dns_lookup /
#       img_encode_jpeg / h3_frame 实测均被误报 L002）；
#     · 残留死名 bus_new（runtime 侧已改名 event_bus）。
#
# M114 尾的**根治**：名册不再手抄，而是从 runtime 注册表**派生**——
#   生成器 `tools/gen_builtin_list.sh` → 写入 `tools/lint_core.px` 的标记块
#   （pxlint / pxcheck 共同 import 该文件）⇒ 全仓名册载体**唯一**。
#
# 本门判据（五项，任一不成立即 rc=1；解析异常 rc=2，门坏掉不许静默变绿）：
#   ① **生成器无漂移**：重跑 gen_builtin_list.sh --check（不写盘），
#     `tools/lint_core.px` 的标记块必须与 runtime 现算结果逐字节一致。
#   ② **名册载体唯一**：标记块恰在 lint_core.px 出现 1 次；pxlint/pxcheck 里
#     只允许**引用** BUILTIN_NAMES，不得再有 `* BUILTINS = split(` 手抄定义。
#   ③ **interp ⊆ 名册**：解释器 interp.px 运行期注册的名字，lint 不许误报。
#   ④ **runtime native ⊆ 名册**：`px_set_global(..., px_native)` 全量必须覆盖
#     （防生成器口径被收窄后本门「跟着变绿」——判据独立于生成器）。
#   ⑤ 解析健全性：名册 < 200 名即判 rc=2（正则失配 / 文件缺失不许静默）。
#   ⑦ 两轨内置集一致（M177）：解释轨注册名 ⊆ runtime 可达名（防「解释轨有、编译轨 R1001」）。
#   ⑥ 宿主注入全局 ⊆ 名册（M171：`px_dict_set(env, …)` 的 REQUEST/GET/POST/SERVER，
#     判据独立于生成器 —— 同 ④ 的理由）。
#
# 用法：bash selfhost/builtin_list_check.sh
# ============================================================
set -u
cd "$(dirname "$0")/.."
# 名册解析必须是**确定性**的：排序一律逐字节（locale 无关）。
# 教训见 selfhost/rebake_bin.sh 顶部（PR #8 CI 假红：`sort` 受 locale 影响）。
export LC_ALL=C
export LANG=C

INTERP=selfhost/interp.px
CORE=tools/lint_core.px
PLINT=tools/pxlint.px
PCHK=tools/pxcheck.px
GEN=tools/gen_builtin_list.sh
BEGIN_MARK='# >>> BEGIN BUILTIN_NAMES >>>'
END_MARK='# <<< END BUILTIN_NAMES <<<'

for f in "$INTERP" "$CORE" "$PLINT" "$PCHK" "$GEN"; do
    [ -f "$f" ] || { echo "❌ 缺文件：$f" >&2; exit 2; }
done

bad=0

# 名册本体：lint_core.px 标记块内那一行 split("...", " ") 的名字集。
core_names() {
    sed -n "/^${BEGIN_MARK}\$/,/^${END_MARK}\$/p" "$CORE" \
        | sed -n 's/.*split("\(.*\)", " ").*/\1/p' | tr ' ' '\n' | sed '/^$/d' | sort -u
}
# 解释器名册：interp.px 里 `let names = [ "a", "b", ... ]`（运行期注册）。
interp_names() {
    grep -o 'let names = \[[^]]*\]' "$INTERP" \
        | grep -o '"[^"]*"' | tr -d '"' | sort -u
}
# runtime native 名：与 gen_builtin_list.sh / gen_native_table.sh 同源口径。
runtime_native_names() {
    grep -h 'px_set_global("' runtime/*.c 2>/dev/null \
        | sed -n 's/.*px_set_global("\([A-Za-z_][A-Za-z0-9_]*\)", *px_native.*/\1/p' | sort -u
}
# M171 ⑥：宿主注入全局名（px_serve / px_exec 的 `px_dict_set(env, "NAME", …)`）——
#   判据**独立于生成器**（与 ④ 同理由：防「生成器口径被收窄后本门跟着变绿」）。
host_injected_names() {
    grep -h 'px_dict_set(env, "' runtime/*.c 2>/dev/null \
        | sed -n 's/.*px_dict_set(env, "\([A-Za-z_][A-Za-z0-9_]*\)".*/\1/p' | sort -u
}

n_core=$(core_names | wc -l)
n_int=$(interp_names | wc -l)
n_rt=$(runtime_native_names | wc -l)
n_host=$(host_injected_names | wc -l)
[ "$n_core" -ge 200 ] || { echo "❌ 名册解析异常：$CORE 只解析出 $n_core 个名字（标记块缺失/正则失配？）" >&2; exit 2; }
[ "$n_int"  -ge 50  ] || { echo "❌ 名册解析异常：interp.px 只解析出 $n_int 个名字（正则失配？）" >&2; exit 2; }
[ "$n_rt"   -ge 100 ] || { echo "❌ 名册解析异常：runtime/*.c 只解析出 $n_rt 个 native（正则失配？）" >&2; exit 2; }
[ "$n_host" -ge 4   ] || { echo "❌ 名册解析异常：runtime/*.c 只解析出 $n_host 个宿主注入全局（应为 REQUEST/GET/POST/SERVER 4 个）" >&2; exit 2; }

echo "── 内置名册防漂移门（单一事实源：runtime 注册表）──"

# ① 生成器无漂移
if out=$(bash "$GEN" --check 2>&1); then
    echo "    ✅ ① 名册 == runtime 注册表现算结果（$n_core 名，生成器 --check 无差异）"
else
    echo "❌ ① 名册漂移（重跑 bash $GEN 同步）："
    printf '%s\n' "$out" | sed 's/^/     /'
    bad=$((bad+1))
fi

# ② 名册载体唯一
nb=$(grep -cF "$BEGIN_MARK" "$CORE" || true)
if [ "$nb" -ne 1 ]; then
    echo "❌ ② $CORE 的标记块出现 $nb 次（应为 1 次：名册载体必须唯一）"
    bad=$((bad+1))
fi
dup=$(grep -n 'BUILTINS *= *split(' "$PLINT" "$PCHK" 2>/dev/null || true)
if [ -n "$dup" ]; then
    echo "❌ ② pxlint/pxcheck 又出现手抄名册定义（应只 import lint_core 的 BUILTIN_NAMES）："
    printf '%s\n' "$dup" | sed 's/^/     /'
    bad=$((bad+1))
fi
ref=0
for f in "$PLINT" "$PCHK"; do
    grep -q 'BUILTIN_NAMES' "$f" && ref=$((ref+1))
done
if [ "$ref" -ne 2 ]; then
    echo "❌ ② pxlint/pxcheck 未都引用 BUILTIN_NAMES（引用数=$ref，应为 2）"
    bad=$((bad+1))
else
    [ "$nb" -eq 1 ] && [ -z "$dup" ] && echo "    ✅ ② 名册载体唯一（$CORE 生成块 1 处；pxlint/pxcheck 仅引用）"
fi

# ③ interp ⊆ 名册
miss=$(comm -23 <(interp_names) <(core_names))
if [ -n "$miss" ]; then
    echo "❌ ③ interp.px 名册有 $(printf '%s\n' "$miss" | wc -l) 个名字不在内置名册（lint 会误报 L002）："
    printf '%s\n' "$miss" | sed 's/^/     /'
    bad=$((bad+1))
else
    echo "    ✅ ③ interp.px 名册 ⊆ 内置名册（$n_int 名全覆盖）"
fi

# ④ runtime native ⊆ 名册
miss4=$(comm -23 <(runtime_native_names) <(core_names))
if [ -n "$miss4" ]; then
    echo "❌ ④ runtime/*.c 有 $(printf '%s\n' "$miss4" | wc -l) 个 native 不在内置名册（lint 会误报 L002）："
    printf '%s\n' "$miss4" | sed 's/^/     /'
    bad=$((bad+1))
else
    echo "    ✅ ④ runtime native ⊆ 内置名册（$n_rt 名全覆盖）"
fi

# ⑥ 宿主注入全局 ⊆ 名册（M171：`px_dict_set(env, …)` 的 REQUEST/GET/POST/SERVER）——
#   判据**独立于生成器**（同 ④ 的理由：防「生成器口径被收窄后本门跟着变绿」）。
miss6=$(comm -23 <(host_injected_names) <(core_names))
if [ -n "$miss6" ]; then
    echo "❌ ⑥ runtime/*.c 有 $(printf '%s\n' "$miss6" | wc -l) 个**宿主注入全局**不在内置名册（lint 会误报 L002）："
    printf '%s\n' "$miss6" | sed 's/^/     /'
    bad=$((bad+1))
else
    echo "    ✅ ⑥ 宿主注入全局 ⊆ 内置名册（$n_host 名全覆盖：REQUEST/GET/POST/SERVER）"
fi

# ⑦ 两轨内置集一致（M177 加）：**解释轨有、runtime 没有**的名字 = 用户照解释轨写、
#   一上编译轨就 `R1001 未定义变量`。判据独立于生成器（生成器取的是**并集**，天生看不见这类差）。
#   实测盲区（M177 发现）：`dict` / `unique` / `flatten` 三个名字只在解释轨有，
#   而门 ③④ 都只看「⊆ 名册」⇒ 全绿，直到真去编译才炸。
interp_only=$(comm -23 <(interp_names) <( { grep -h 'px_set_global("' runtime/*.c 2>/dev/null \
    | sed -n 's/.*px_set_global("\([A-Za-z_][A-Za-z0-9_]*\)".*/\1/p'; \
    grep -h 'px_ffi_register("' runtime/*.c 2>/dev/null \
    | sed -n 's/.*px_ffi_register("\([A-Za-z_][A-Za-z0-9_]*\)".*/\1/p'; } | grep -v '^__' | sort -u))
if [ -n "$interp_only" ]; then
    echo "❌ ⑦ 解释轨有、runtime 没有的内置（编译轨会 R1001 未定义变量）："
    printf '%s\n' "$interp_only" | sed 's/^/     /'
    bad=$((bad+1))
else
    echo "    ✅ ⑦ 两轨内置集一致（解释轨注册名全部在 runtime 里可达）"
fi

if [ "$bad" -gt 0 ]; then
    echo "❌ 内置名册门失败 $bad 处"
    exit 1
fi
echo "✅ 内置名册一致（唯一载体 $CORE；源 = runtime 注册表，生成器 + 门双守）"
exit 0
