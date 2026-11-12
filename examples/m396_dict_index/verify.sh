#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# M396 · dict 桶索引（PX-015 的处置：**纯加速，零语义变更**）
# ---------------------------------------------------------------------------
# 背景：px_dict_get / px_dict_has 线性扫，N=256→1024 四倍键 GET 1.7us→9–11us（线性成立）。
#   dict 只有一份实现（VM/解释/C 轨全汇到 runtime.c）⇒ 改一处三轨同享。
#   设计：不可变快照 + 原子换指针（get/has 不加锁）；remove 发「墓碑」整体作废；
#         重建因子 1.25（尾部补扫平均 ≤12.5% n）。
# 判据：
#   T1–T5 语义不变：100 键命中/缺键/键序/items/删除中段/删后重插/连续删除/深拷贝/无索引小 dict
#   X1   三轨（interp / vm / c）逐字节一致
#   S    尺寸判据：sizeof(LXObject) 必须与基线一致（索引是侧表，不许把对象撑大）
#   P    形状判据：GET(1024)/GET(32) < 3.0（线性扫 ≈15×，有索引 ≈1×）
#   N1   **有牙负控**：PX_DICT_NO_INDEX=1 时同一判据必须变红（⇒ 证明开关与判据都真起作用）
# ═══════════════════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"
PX="$ROOT/tools/px"
SRC="$ROOT/examples/m396_dict_index"
TMP="${TMPDIR:-/tmp}/m396_idx.$$"
rm -rf "$TMP"; mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT
cp "$SRC/shape.px" "$TMP/shape.px" || exit 1

pass=0; fail=0
ok()  { echo "  ✅ $*"; pass=$((pass+1)); }
bad() { echo "  ❌ $*"; fail=$((fail+1)); }

run_track() {
    local track="$1" src="$2" out="$3"
    # 源先拷进 $TMP：px build 把产物写到 <源目录>/build/ ⇒ 不许污染仓库工作树
    local base; base="$(basename "$src")"
    cp "$src" "$TMP/$base" || return 9
    case "$track" in
    interp) "$PX" run "$TMP/$base" > "$out" 2> "$out.err" ;;
    vm)     "$PX" build "$TMP/$base" > "$out.build" 2>&1 || return $?
            local nm; nm="$(basename "$src" .px)"
            "$TMP/build/$nm" > "$out" 2> "$out.err" ;;
    c)      "$PX" build --c "$TMP/$base" > "$out.build" 2>&1 || return $?
            local nm; nm="$(basename "$src" .px)"
            "$TMP/build/$nm" > "$out" 2> "$out.err" ;;
    *)      return 9 ;;
    esac
}

# ── T1–T5：语义不变（解释轨逐行比对期望） ─────────────────────────────────
if run_track interp "$SRC/main.px" "$TMP/i.out"; then
    exp() { if grep -qx -- "$1" "$TMP/i.out"; then ok "$1"; else bad "期望整行 [$1]，实得 [$(grep -m1 -- "^${1%% *} " "$TMP/i.out")]"; fi; }
    exp "T1 100 99 -1 n"
    exp "T2 k0,k1,k99 k99=99"
    exp "T3 50 99 49 51 n"
    exp "T3b k50 k51 500"
    exp "T3c 50 2500"
    exp "T4 1000 0 -1"
    exp "T5 2 1 2"
    exp "BAD=0"
else
    bad "interp 主体失败：$(head -3 "$TMP/i.out.err" | tr '\n' ' ')"
fi

# ── X1：三轨逐字节一致 ────────────────────────────────────────────────────
for tr in vm c; do
    if run_track "$tr" "$SRC/main.px" "$TMP/$tr.out"; then
        if cmp -s "$TMP/i.out" "$TMP/$tr.out"; then
            ok "X1-$tr 与 interp 逐字节一致（$(wc -l < "$TMP/$tr.out") 行）"
        else
            bad "X1-$tr 与 interp 不一致：$(diff "$TMP/i.out" "$TMP/$tr.out" | head -4 | tr '\n' ' ')"
        fi
    else
        bad "$tr 轨失败：$(head -3 "$TMP/$tr.out.err" 2>/dev/null | tr '\n' ' ')$(head -3 "$TMP/$tr.out.build" 2>/dev/null | tr '\n' ' ')"
    fi
done

# ── S：尺寸判据 —— 索引是侧表，LXObject 不许被撑大 ───────────────────────
cat > "$TMP/sz.c" <<'CEOF'
#include <stdio.h>
#include "runtime.h"
int main(void) { printf("%zu\n", sizeof(LXObject)); return 0; }
CEOF
# -static：本仓链接口径（M219 全仓链接 flag 卫生扫的判据；Red Hat 宿主默认非 PIE、
#   Ubuntu CI 默认 PIE ⇒ 不写 -static 会出现「本机绿、CI 红」）。探针只打印 sizeof，静态链接无副作用。
if gcc -static -I "$ROOT/runtime" -I "$ROOT/runtime/third_party/miniz" -o "$TMP/sz" "$TMP/sz.c" 2> "$TMP/sz.err"; then
    sz="$("$TMP/sz")"
    if [ "$sz" = "192" ]; then
        ok "S sizeof(LXObject)=$sz（与 M395 基线一致 ⇒ 索引零额外对象内存）"
    else
        bad "S sizeof(LXObject)=$sz，基线为 192 ⇒ 索引把对象撑大了"
    fi
else
    bad "S 尺寸探针编译失败：$(head -2 "$TMP/sz.err" | tr '\n' ' ')"
fi

# ── P / N1：形状判据 + 有牙负控 ───────────────────────────────────────────
#   判据只看**比值**（不看绝对 ns）⇒ 8 分片并发跑门时不因机器忙而假红。
if "$PX" build "$TMP/shape.px" > "$TMP/shape.build" 2>&1; then
    SHAPEBIN="$TMP/build/shape"

    # 取 3 次里最小的 RATIO（噪声只会让它变大）
    best_on=""; best_off=""
    for _ in 1 2 3; do
        v=$($SHAPEBIN 2>/dev/null | sed -n 's/.*RATIO=\([0-9.]*\).*/\1/p')
        [ -n "$v" ] && best_on=$(awk -v a="$best_on" -v b="$v" 'BEGIN{ if (a=="") print b; else print (b<a?b:a) }')
        v=$(PX_DICT_NO_INDEX=1 $SHAPEBIN 2>/dev/null | sed -n 's/.*RATIO=\([0-9.]*\).*/\1/p')
        [ -n "$v" ] && best_off=$(awk -v a="$best_off" -v b="$v" 'BEGIN{ if (a=="") print b; else print (b<a?b:a) }')
    done

    if [ -z "$best_on" ] || [ -z "$best_off" ]; then
        bad "P/N1 装置跑不出 RATIO（on=[$best_on] off=[$best_off]）"
    else
        # P：有索引 ⇒ ratio 必须小
        if awk -v r="$best_on" 'BEGIN{ exit !(r < 3.0) }'; then
            ok "P 形状判据：RATIO(on)=$best_on < 3.0（线性扫约 15×）"
        else
            bad "P 形状判据：RATIO(on)=$best_on ≥ 3.0 ⇒ 索引没生效（或退化成线性扫）"
        fi
        # N1：关掉索引 ⇒ 同一判据必须变红（判据有牙 + 开关真起作用）
        if awk -v r="$best_off" 'BEGIN{ exit !(r >= 3.0) }'; then
            ok "N1 有牙负控：RATIO(off)=$best_off ≥ 3.0 ⇒ 关掉索引判据必红（开关与判据都真起作用）"
        else
            bad "N1 负控失败：RATIO(off)=$best_off < 3.0 ⇒ 这条判据没牙（关掉索引也照样绿）"
        fi
        # 前提自证：开关确实改变了行为（on 必须显著快于 off）
        if awk -v a="$best_on" -v b="$best_off" 'BEGIN{ exit !(b > a * 1.5) }'; then
            ok "P2 前提自证：off/on = $best_off/$best_on > 1.5×（索引确实在省时间）"
        else
            bad "P2 前提自证失败：off/on = $best_off/$best_on ≤ 1.5× ⇒ 没测到索引的收益"
        fi
    fi
else
    bad "shape.px 编译失败：$(head -3 "$TMP/shape.build" | tr '\n' ' ')"
fi

echo "── m396_dict_index：通过 $pass · 失败 $fail"
[ "$fail" = 0 ] || exit 1
