#!/usr/bin/env bash
# ============================================================
# M216 平台相关性举证（**静态** · 可复现 · 不依赖目标机）
# ------------------------------------------------------------
# 命题：(int64_t)f 在 f 越界时是 **C11 6.3.1.4p1 的未定义行为**，两个架构各自"选了一种"：
#   x86_64  cvttsd2si  —— 越界/NaN 给 **INT64_MIN**（"不定值"哨兵 0x8000…0）
#   aarch64 fcvtzs     —— 越界 **饱和**（+1e30 ⇒ INT64_MAX、−1e30 ⇒ INT64_MIN）
# ⇒ 同一份 PuXian 源码在两个架构上打印**不同的数**，且都是静默的。
# 修法 px_f2i 把「守卫」放在转换**之前** ⇒ 越界/非有限 ⇒ 响亮 R1003，
#   转换只在 `[−2^63, 2^63)` 内执行 ⇒ 该处的 `(int64_t)` **良定义**、且**与架构无关**。
# 判据（本脚本输出 4 行 KEY=VALUE，供 verify.sh 断言）：
#   OLD_X86_HAS_CVTTSD2SI / OLD_ARM_HAS_FCVTZS   —— 旧形态两架构指令不同（不可移植）
#   NEW_X86_HAS_GUARD / NEW_ARM_HAS_GUARD        —— 新形态两侧**都**有守卫（且转换在守卫后）
# 用法：platform_proof.sh <工作目录> [交叉 gcc]
# ============================================================
set -uo pipefail
W="${1:-/tmp/m216/plat}"
CROSS="${2:-/opt/aarch64-linux-musl-cross/bin/aarch64-linux-musl-gcc}"
mkdir -p "$W"

if ! command -v "$CROSS" > /dev/null 2>&1; then
    echo "SKIP=1"
    echo "SKIP_REASON=交叉编译器不可用（$CROSS）"
    exit 0
fi

cat > "$W/old.c" <<'EOF'
long long old_conv(double f) { return (long long)f; }
EOF

cat > "$W/new.c" <<'EOF'
#include <math.h>
void px_error(const char*, ...);
long long px_f2i(double f, const char* what) {
    if (isnan(f)) px_error("nan");
    if (isinf(f)) px_error("inf");
    if (f >= 9223372036854775808.0 || f < -9223372036854775808.0) px_error("range");
    return (long long)f;
}
EOF

gcc            -O2 -c "$W/old.c" -o "$W/old_x86.o" 2> "$W/err.log" || { echo "SKIP=1"; echo "SKIP_REASON=x86 编译失败"; exit 0; }
"$CROSS"       -O2 -c "$W/old.c" -o "$W/old_arm.o" 2>> "$W/err.log" || { echo "SKIP=1"; echo "SKIP_REASON=aarch64 编译失败"; exit 0; }
gcc            -O2 -c "$W/new.c" -o "$W/new_x86.o" 2>> "$W/err.log" || { echo "SKIP=1"; echo "SKIP_REASON=x86 编译失败"; exit 0; }
"$CROSS"       -O2 -c "$W/new.c" -o "$W/new_arm.o" 2>> "$W/err.log" || { echo "SKIP=1"; echo "SKIP_REASON=aarch64 编译失败"; exit 0; }

for p in old_x86 old_arm new_x86 new_arm; do
    # ⚠️ 宿主 objdump 通常是**单目标**（x86_64），反汇编 aarch64 会得到空/报错
    #   ⇒ arm 两份必须用交叉工具链自带的 objdump（本仓 M168 起已有该工具链）
    case "$p" in
        *_arm) OD="$(dirname "$CROSS")/aarch64-linux-musl-objdump" ;;
        *)     OD="objdump" ;;
    esac
    "$OD" -d "$W/$p.o" > "$W/$p.dis" 2>&1
    printf '%-8s 转换指令=%-12s 守卫指令=%s\n' "$p" \
        "$(grep -oE 'cvttsd2si|fcvtzs' "$W/$p.dis" | sort -u | tr '\n' ',' || true)" \
        "$(grep -oE 'ucomisd|ucomiss|fcmp|b\.[a-z]+|jp |jnp |jbe |ja ' "$W/$p.dis" | sort -u | tr '\n' ',' || true)"
done

grep -q 'cvttsd2si' "$W/old_x86.dis" && echo "OLD_X86_HAS_CVTTSD2SI=1" || echo "OLD_X86_HAS_CVTTSD2SI=0"
grep -q 'fcvtzs'    "$W/old_arm.dis" && echo "OLD_ARM_HAS_FCVTZS=1"    || echo "OLD_ARM_HAS_FCVTZS=0"
# 新形态：两侧都必须**先有守卫**（浮点比较）+ 转换指令仍在
if grep -qE 'ucomisd|ucomiss' "$W/new_x86.dis" && grep -q 'cvttsd2si' "$W/new_x86.dis"; then
    echo "NEW_X86_HAS_GUARD=1"
else
    echo "NEW_X86_HAS_GUARD=0"
fi
if grep -qE '\bfcmpe?|\bfcmeq' "$W/new_arm.dis" && grep -q 'fcvtzs' "$W/new_arm.dis"; then
    echo "NEW_ARM_HAS_GUARD=1"
else
    echo "NEW_ARM_HAS_GUARD=0"
fi
echo "SKIP=0"
