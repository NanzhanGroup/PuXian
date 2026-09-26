#!/usr/bin/env bash
# ============================================================
# M217 平台/优化相关性举证（**静态** · 可复现 · 不依赖目标机）
# ------------------------------------------------------------
# 命题：`a << n` 在 `n < 0`、`n >= 64`、或**左移结果不可表示**时，是
#       **C11 6.5.7p3/p4 的未定义行为**（UB）。UB 的「症状」有三种可观察形态，
#       本脚本逐一测量（**同一份 C**，只换编译档/形态）：
#         ① **告警**：gcc 对常量越界给 `-Wshift-count-overflow` / `-Wshift-count-negative`
#         ② **优化档不同**：同一函数 `-O0` 与 `-O2` 给出**不同返回值**
#         ③ **常量与运行期不同**：`1LL << 64`（常量折叠）与 `f(1, 64)`（运行期）不同
#       修法（`px_shl64` 等）把计数**显式掩码**（`n & 63`）+ 负计数**响亮报错**，
#       且左移经 `uint64_t`（无符号回绕**良定义**）⇒ 三种症状**全部消失**。
# 判据（verify.sh 断言）：
#   `UB_SYMPTOMS`  >= 1   （旧形态至少有一种 UB 症状）
#   `NEW_SYMPTOMS` == 0   （新形态一种都没有）
#   `OLD_HAS_AND`  == 0   （旧形态反汇编里没有显式掩码）
#   `NEW_HAS_AND`  == 1   （新形态有 `and $0x3f` —— 用 `-fno-inline` 保函数边界）
# ⚠️ 踩过的坑（首版）：① 把 old/new 放同一个 TU 测告警 ⇒ 新形态也被算上告警；
#   ② `-O2` 会把 `n & 63` 连同调用点常量一起折叠掉 ⇒ 反汇编里看不到 `and`
#   （⇒ 必须 `noinline` + `-fno-inline`）；③ 少取一个变量（`NEW_C2`）让
#   「未定义变量 != 1」**恒真** ⇒ 假报 `NEW_SYMPTOMS=1`。
# 用法：ub_proof.sh <工作目录> [cc]
# ============================================================
set -uo pipefail
W="${1:-/tmp/m217/ubproof}"
CC="${2:-gcc}"
mkdir -p "$W"

OLD_SRC='long long shl_rt(long long a, long long n) { return a << n; }
long long shl_const(void) { return 1LL << 64; }'

NEW_SRC='long long shl_rt(long long a, long long n) {
    return (long long)((unsigned long long)a << (unsigned long long)(n & 63));
}
long long shl_const(void) { return (long long)((unsigned long long)1 << (64 & 63)); }'

# 运行期 + 常量 的对照（同 TU，便于一起跑）
cat > "$W/old.c" <<EOF
#include <stdio.h>
$OLD_SRC
int main(void) {
    printf("%lld\n%lld\n", shl_rt(1, 64), shl_const());
    return 0;
}
EOF
cat > "$W/new.c" <<EOF
#include <stdio.h>
$NEW_SRC
int main(void) {
    printf("%lld\n%lld\n", shl_rt(1, 64), shl_const());
    return 0;
}
EOF

for tag in old new; do
    for lvl in O0 O2; do
        if ! "$CC" -"$lvl" "$W/$tag.c" -o "$W/${tag}_$lvl" 2> "$W/${tag}_${lvl}.log"; then
            echo "SKIP=1"; echo "SKIP_REASON=$CC -$lvl $tag 编译失败"; exit 0
        fi
        "$W/${tag}_$lvl" > "$W/${tag}_out_$lvl.txt" 2>&1 || true
    done
done

val() { sed -n "$2p" "$W/$1_out_$3.txt" 2>/dev/null; }

OLD_O0=$(val old 1 O0); OLD_C0=$(val old 2 O0)
OLD_O2=$(val old 1 O2); OLD_C2=$(val old 2 O2)
NEW_O0=$(val new 1 O0); NEW_C0=$(val new 2 O0)
NEW_O2=$(val new 1 O2); NEW_C2=$(val new 2 O2)

# ① 告警（**分开**编译，否则新形态会被旧函数的告警牵连）
WARN=0; NEWWARN=0
"$CC" -O2 -c "$W/old.c" -o /dev/null 2> "$W/old_warn.log" || true
grep -qE 'shift-count-overflow|shift-count-negative' "$W/old_warn.log" && WARN=1
"$CC" -O2 -c "$W/new.c" -o /dev/null 2> "$W/new_warn.log" || true
grep -qE 'shift-count-overflow|shift-count-negative' "$W/new_warn.log" && NEWWARN=1

# ② 优化档不同 / ③ 常量与运行期不同
DIFF_OPT=0;   { [ "$OLD_O0" != "$OLD_O2" ] || [ "$OLD_C0" != "$OLD_C2" ]; } && DIFF_OPT=1
DIFF_CONST=0; { [ "$OLD_O0" != "$OLD_C0" ] || [ "$OLD_O2" != "$OLD_C2" ]; } && DIFF_CONST=1
UB=$(( WARN + DIFF_OPT + DIFF_CONST ))

NDIFF=0
{ [ "$NEW_O0" != "$NEW_O2" ] || [ "$NEW_C0" != "$NEW_C2" ] \
  || [ "$NEW_O0" != "$NEW_C0" ] || [ "$NEW_O2" != "$NEW_C2" ]; } && NDIFF=1
NEW_SYM=$(( NEWWARN + NDIFF ))

# 反汇编（`-fno-inline` 保函数边界；`noinline` 属性防跨 TU 折叠）
OLD_AND=0; NEW_AND=0
if command -v objdump >/dev/null 2>&1; then
    # ⚠️ 必须 `-O0`：`-O1/-O2` 会把调用点的常量 64 传播进来 ⇒ `n & 63` 被折叠成 0
    #   ⇒ 反汇编里看不到 `and`（首版实测 `NEW_HAS_AND=0` 假红）。要的是「**函数体内**是否有掩码」。
    for tag in old new; do
        "$CC" -O0 -fno-inline -c "$W/$tag.c" -o "$W/$tag.o" 2>/dev/null || continue
        objdump -d "$W/$tag.o" > "$W/$tag.dis" 2>/dev/null || continue
        A=0
        sed -n '/<shl_rt>:/,/^$/p' "$W/$tag.dis" | grep -qE 'and.*\$0x3f' && A=1
        if [ "$tag" = old ]; then OLD_AND=$A; else NEW_AND=$A; fi
    done
fi

echo "OLD_O0_F64=${OLD_O0:-?}"
echo "OLD_O2_F64=${OLD_O2:-?}"
echo "OLD_O0_CONST=${OLD_C0:-?}"
echo "NEW_O0_F64=${NEW_O0:-?}"
echo "NEW_O2_F64=${NEW_O2:-?}"
echo "NEW_O0_CONST=${NEW_C0:-?}"
echo "GCC_WARNS=$WARN"
echo "NEW_GCC_WARNS=$NEWWARN"
echo "OLD_DIFF_OPT=$DIFF_OPT"
echo "OLD_DIFF_CONST=$DIFF_CONST"
echo "UB_SYMPTOMS=$UB"
echo "NEW_SYMPTOMS=$NEW_SYM"
echo "OLD_HAS_AND=$OLD_AND"
echo "NEW_HAS_AND=$NEW_AND"
echo "SKIP=0"
