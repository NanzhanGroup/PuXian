#!/usr/bin/env bash
# ============================================================
# gen_builtin_list.sh —— 内置名册生成器（M114 尾 · Issue 63 根治）
# ------------------------------------------------------------
# 单一事实源 = **runtime 注册表**（与 tools/gen_native_table.sh 同一口径）：
#   ① runtime/*.c 的 `px_set_global("name", ...)`（`__` 前缀的内部名排除）
#   ② runtime/*.c 的 `px_ffi_register("name", ...)`
#   ③ selfhost/interp.px 的 `let names = [...]`（解释器运行期注册的 Mini 子集）
#   ④ runtime/*.c 的 `px_dict_set(env, "name", ...)`（px_serve/px_exec 注入的
#      **宿主全局** REQUEST/GET/POST/SERVER；M171 补，原先误报 L002）
# 产出**排序去重**（LC_ALL=C，逐字节序）的一份名册，写入 tools/lint_core.px
# 的标记块；pxlint / pxcheck 共同 import 该文件 ⇒ 全仓**只有一份**名册载体。
#
# 为什么要有它（Issue 63，PR #8 首次真机 CI 实锤）：
#   原先 pxlint.px / pxcheck.px **各手抄一份**名册，且与 runtime **双向漂移**：
#     · 缺 27 个真实内置（print_err / flush / round / floor / mmap / ffi_call …）
#       ⇒ M114-S1 把诊断改走 print_err 后，lint 立刻误报 `E L002: 未定义变量: 'print_err'`；
#     · 反向**漏收** runtime 已注册的名字（实测 quic_connect / dns_lookup /
#       img_encode_jpeg / h3_frame 均被误报 L002 —— 名册小 54 项）；
#     · 残留 1 个**死名** bus_new（runtime 侧已改名 event_bus，名册未跟）。
#   手抄 ⇒ 必然漂移；派生 ⇒ 漂移不可能（门只剩「生成器是否跑过」这一件事可查）。
#
# 用法：
#   bash tools/gen_builtin_list.sh          # 写入 tools/lint_core.px（幂等）
#   bash tools/gen_builtin_list.sh --check  # 只比不写：有漂移 → 打印 diff + rc=1
# ============================================================
set -eu
cd "$(dirname "$0")/.."
# 名册必须是**确定性**的：排序一律逐字节（locale 无关）。
# 教训见 selfhost/rebake_bin.sh 顶部（PR #8 CI 假红：`sort` 受 locale 影响）。
export LC_ALL=C
export LANG=C

TARGET=tools/lint_core.px
BEGIN='# >>> BEGIN BUILTIN_NAMES >>>'
END='# <<< END BUILTIN_NAMES <<<'
MODE=write
[ "${1:-}" = "--check" ] && MODE=check

tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT

grep -h 'px_set_global("' runtime/*.c 2>/dev/null \
    | sed -n 's/.*px_set_global("\([A-Za-z_][A-Za-z0-9_]*\)".*/\1/p' \
    | grep -v '^__' > "$tmp"
grep -h 'px_ffi_register("' runtime/*.c 2>/dev/null \
    | sed -n 's/.*px_ffi_register("\([A-Za-z_][A-Za-z0-9_]*\)".*/\1/p' >> "$tmp"
# ④ 宿主注入全局（M171）：px_serve 的 .px 脚本分支 / px_exec 用
#    `px_dict_set(env, "NAME", ...)` 注入的**全局变量**（REQUEST/GET/POST/SERVER）——
#    它们在 .px 程序里是合法可见名（runtime 自陈「Web 风格脚本可读全局变量」），
#    但既不经过 px_set_global 也不经过 px_ffi_register ⇒ 原先 lint 对
#    `examples/webapp/*.px` 报 3 条假 L002。此处按**同一口径**派生（勿手抄）。
grep -h 'px_dict_set(env, "' runtime/*.c 2>/dev/null \
    | sed -n 's/.*px_dict_set(env, "\([A-Za-z_][A-Za-z0-9_]*\)".*/\1/p' >> "$tmp"
grep -o 'let names = \[[^]]*\]' selfhost/interp.px \
    | grep -o '"[^"]*"' | tr -d '"' >> "$tmp"
sort -u "$tmp" -o "$tmp"

n=$(wc -l < "$tmp")
[ "$n" -ge 200 ] || { echo "❌ 名册解析异常：只抽到 $n 个名字（runtime 抽取正则失配？）" >&2; exit 2; }

joined=$(tr '\n' ' ' < "$tmp" | sed 's/ $//')
# 生成的行（单行；PuXian list 字面量不能跨行 ⇒ 空格分隔串 + split；# noqa 抑制 L007 长行）
newline="const BUILTIN_NAMES = split(\"$joined\", \" \")  # noqa"

gen=$(mktemp)
awk -v b="$BEGIN" -v e="$END" -v line="$newline" '
    $0 == b { print; print line; skip = 1; next }
    $0 == e { skip = 0 }
    skip != 1 { print }
' "$TARGET" > "$gen"

if ! grep -qF "$BEGIN" "$gen" || ! grep -qF "$newline" "$gen"; then
    echo "❌ $TARGET 缺标记块（$BEGIN / $END）" >&2
    exit 2
fi

if [ "$MODE" = check ]; then
    if diff -u "$TARGET" "$gen" > /tmp/builtin_list.diff; then
        echo "    ✅ 名册与 runtime 注册表一致（$n 名）"
    else
        echo "❌ 名册漂移：$TARGET 与 runtime 注册表现算结果不一致（重跑 bash tools/gen_builtin_list.sh）"
        sed 's/^/     /' /tmp/builtin_list.diff | head -20
        exit 1
    fi
else
    cp "$gen" "$TARGET"
    echo "written $TARGET ($n 内置名)"
fi
