#!/usr/bin/env bash
# ============================================================
# tools/gen_native_map.sh —— 生成 runtime/native_mod_map.txt
# ------------------------------------------------------------
# native_mod_map.txt：可裁模块 native 名 → 模块索引（name=mod，每行）
#   供 px build 自动裁剪（M86-S1/S2）：编译器 refs 输出被引用名集合 →
#   装配器按此 map 反推"未引用模块" → 自动折 --no-xxx。
# 数据源（权威 = runtime C 注册点）：
#   - runtime.c px_register_builtins() 内 #ifndef PX_NO_<MOD> 宏块的
#     px_set_global/px_ffi_register 注册名 → 对应模块
#   - runtime_quic.c / runtime_h3.c / runtime_h3_qpack_dyn.c 全部注册名 → quic
#   - runtime_zlib.c 全部注册名 → zlib（runtime.c 内 PX_NO_ZLIB 块只调 px_register_zlib）
# 用法：
#   bash tools/gen_native_map.sh            # stdout 输出映射（不落盘）
#   bash tools/gen_native_map.sh --update   # 直接覆盖 runtime/native_mod_map.txt
# 依赖：awk + grep + sort（零外部依赖，纯标准工具）
# ============================================================
set -eu
cd "$(dirname "$0")/.."
RT=runtime
OUT="$RT/native_mod_map.txt"

# ---- runtime.c 函数体内扫描（模块宏块栈归属；core 不输出）----
# 同时扫各模块自注册文件（quic 栈/h3_qpack_dyn + zlib + ws 模块文件内注册）
gen() {
    awk '
    function regname(line,    s) {
        # 取 px_set_global("name" / px_ffi_register("name" 的名字
        if (match(line, /px_(set_global|ffi_register)\("[^"]+"/)) {
            s = substr(line, RSTART, RLENGTH)
            sub(/.*\("/, "", s)
            sub(/".*/, "", s)
            return s
        }
        return ""
    }
    /void px_register_builtins\(void\) \{/ { infn = 1 }
    infn && /^}/ { infn = 0; exit }
    infn && /#ifndef[ \t]+PX_NO_[A-Z0-9_]+/ {
        line = $0
        sub(/.*PX_NO_/, "", line)
        sub(/[ \t].*/, "", line)
        stack[++sp] = tolower(line)
        next
    }
    infn && /#endif/ { if (sp > 0) sp--; next }
    {
        n = regname($0)
        if (n != "") {
            mod = (sp > 0 ? stack[sp] : "core")
            if (mod == "quic") mod = "core"   # quic 宏块内仅调注册函数
            if (mod != "core") print n "=" mod
        }
    }
    ' "$RT/runtime.c"
    # 模块文件全量注册名（quic 栈 / zlib / ws）
    for f in runtime_quic.c runtime_h3.c runtime_h3_qpack_dyn.c runtime_zlib.c runtime_ws.c; do
        grep -oE 'px_(set_global|ffi_register)\("[^"]+"' "$RT/$f" 2>/dev/null \
            | sed -E 's/.*\("([^"]+)".*/\1/' \
            | awk -v m="${f#runtime_}" '{ m2=m; sub(/\.c$/, "", m2); if (m2=="h3"||m2=="h3_qpack_dyn") m2="quic"; print $0 "=" m2 }' \
            || true
    done
}

MAP="$(gen | LC_ALL=C sort -u)"
echo "$MAP"

if [ "${1:-}" = "--update" ]; then
    echo "$MAP" > "$OUT"
    echo "== 已覆盖 $OUT（$(echo "$MAP" | grep -c .) 行）==" >&2
fi
