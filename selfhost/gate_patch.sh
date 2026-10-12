#!/usr/bin/env bash
# ============================================================
# selfhost/gate_patch.sh —— **门内「打桩」统一入口**（M304）
# ------------------------------------------------------------
# ⚠️ 本文件是**被 source** 的，不单独执行：
#     . "$(dirname "$0")/../../selfhost/gate_patch.sh"
#
# 为什么（M302 缺陷 523 + M285/M286 两个守卫的共同结论）：
#   门的负控一直用 `sed -i 's|OLD|NEW|' <目标>`，而 sed 有两类**静默失效**：
#     ① 元字符陷阱 —— 替换文本里的 & \ $ \1 与分隔符会被 sed 解释
#        （本仓实例：M302 缺陷 523，`\u` 未转义 ⇒ sed 把模式当字面 u ⇒ 打桩静默失效）
#     ② **找不到也 rc=0** —— OLD 不在文件里时 sed 什么都不做、一声不吭
#        ⇒ 负控被抽掉牙，门照样打 ✅（M285 的 A1 守卫就是为了**事后**抓它而建的）
#   ⇒ 收敛到 selfhost/patch_file.py：**字面量匹配 · 零转义 · 失败响亮 · --count 强制命中数**。
#
# 用法：
#   gp_patch <file> <old字面量> <new字面量> [count]      # count 默认 1（强制唯一）
#   gp_patch_spec <spec.json>                           # 多处 / 多行文本走 JSON
#
# ⚠️ **调用点必须检查 rc** —— 不检查就退回 sed 那种静默：
#     gp_patch runtime/runtime.c 'OLD' 'NEW' || { echo "  FAIL 打桩失败"; fail=$((fail+1)); }
#
# 退出码：0 = 已应用；1 = 失败（**文件一字未改**，stderr 已指名 file 与 OLD 片段）
#
# 环境变量：
#   GP_PYTHON    python 解释器（默认 python3）
# ============================================================

_GP_SELF="${BASH_SOURCE[0]}"
_GP_DIR="$(cd "$(dirname "$_GP_SELF")" && pwd)"
GP_PY="${GP_PY:-$_GP_DIR/patch_file.py}"
GP_PYTHON="${GP_PYTHON:-python3}"

_gp_show() {   # $1=rc  $2=file  $3=old  $4=工具输出
    printf 'GP-PATCH-FAIL rc=%s file=%s\n  old=%.90s\n' "$1" "$2" "$3" >&2
    while IFS= read -r _l; do printf '  | %s\n' "$_l" >&2; done <<< "$4"
}

gp_patch() {
    local f="${1-}" old="${2-}" new="${3-}" cnt="${4-1}"
    if [ -z "$f" ] || [ -z "$old" ]; then
        printf 'GP-PATCH-FAIL usage: gp_patch <file> <old> <new> [count]\n' >&2
        return 1
    fi
    if [ ! -f "$GP_PY" ]; then
        printf 'GP-PATCH-FATAL 找不到打桩原语：%s（M304）\n' "$GP_PY" >&2
        return 1
    fi
    local out rc
    out="$("$GP_PYTHON" "$GP_PY" --file "$f" --old "$old" --new "$new" --count "$cnt" 2>&1)"; rc=$?
    if [ "$rc" != 0 ]; then
        _gp_show "$rc" "$f" "$old" "$out"
        return 1
    fi
    return 0
}

gp_patch_spec() {
    local spec="${1-}"
    if [ -z "$spec" ]; then
        printf 'GP-PATCH-FAIL usage: gp_patch_spec <spec.json>\n' >&2
        return 1
    fi
    if [ ! -f "$GP_PY" ]; then
        printf 'GP-PATCH-FATAL 找不到打桩原语：%s（M304）\n' "$GP_PY" >&2
        return 1
    fi
    local out rc
    out="$("$GP_PYTHON" "$GP_PY" --spec "$spec" 2>&1)"; rc=$?
    if [ "$rc" != 0 ]; then
        _gp_show "$rc" "(spec)" "$spec" "$out"
        return 1
    fi
    return 0
}
