#!/usr/bin/env bash
# ============================================================
# M162 门（第 48 轮）：sorted 三轨一致 —— 缺陷 166（解释轨比较依据）+ 167（编译轨稳定性）
# ------------------------------------------------------------
# 背景：
#   · 缺陷 166（解释轨）：`i_builtin_sorted` 用 `i_to_str` **渲染串**比较
#       ⇒ `sorted([10, 9, 2])` = [10, 2, 9]（"10" < "2"，错）
#       ⇒ `sorted([[1,"a"],[1,"a2"]])` = [[1,a2],[1,a]]（= 第三方 registry-px 登记的 PX-DEF-011）
#   · 缺陷 167（编译轨）：`bi_sorted` 用 `compare_values`（值比较，对），但算法是**选择式**
#       ⇒ 在「比较器判相等、但值可渲染区分」的元素上**不稳定**：
#         实测 `sorted([1.0, 1, 0.5])` 旧 = [0.5, 1, 1.0]（稳定序应为 [0.5, 1.0, 1]）
# 修法（三轨同一条真相 = **值比较 + 稳定排序**）：
#   · 解释轨：新增 `i_cmp_values`（selfhost/ival.px，逐分支对齐 C `compare_values_raw` 三态，
#     沿用 i_eq 同款**环保护**）；`i_builtin_sorted` 改用它，交换条件 `> 0`（等值不换 = 稳定）
#   · 编译轨：`bi_sorted`（runtime.c）改**相邻冒泡**（仅 `> 0` 交换）⇒ 稳定
# 判据（逐层可单独变红）：
#   ① 解释轨主用例 pass=26 fail=0（**绝对值**断言，不靠两轨互比）
#   ② VM 轨（用户面默认轨）⇒ stdout 与解释轨**逐字节一致**
#   ③ C 轨（逃生舱）⇒ stdout 与解释轨**逐字节一致**
#   ④ 加强面（同键三元素/嵌套稳定/长列表/跨类型/同族 min·max 回归）三轨一致（12 断言）
#   ⑤ PX-DEF-011 最小复现器（第三方登记件直译）三轨一致
#   ⑥ 负控 3 道（默认跑，`--neg-skip` 跳过）—— 每道必须判红 + 逐字节还原：
#      A 解释轨：值比较退回**渲染串**比较（恢复缺陷 166）⇒ 主用例必红（重建 pxi）
#      B 编译轨：相邻冒泡退回**选择式**（恢复缺陷 167）⇒ 稳定性断言必红（重建 runtime）
#      C 解释轨：list **逐元素**比较退化为「只比长度」⇒ PX-DEF-011 复现器必红（重建 pxi）
# 用法：./examples/m162_sorted_order/verify.sh            （完整门：正判据 + 负控）
#       ./examples/m162_sorted_order/verify.sh --neg-skip （只跑正判据，CI 用）
# 退出码：0 = 绿，1 = 红。
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT"
export LC_ALL=C LANG=C

NEG_SKIP=0
[ "${1:-}" = "--neg-skip" ] && NEG_SKIP=1

PXI="${PX_PXI_BIN:-./bootstrap/pxi}"
CASE=$HERE/sorted_order.px
EDGE=$HERE/sorted_edge.px
DEF011=$HERE/sorted_pxdef011.px
WORK=$(mktemp -d /tmp/m162.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

fail=0
note() { echo "   $*"; }
bad()  { echo "❌ $*"; fail=1; }
hdr()  { echo "── $*"; }

# 三轨跑一个用例：$1=用例 $2=标签 → 落 $WORK/<tag>.{interp,vm,c}.out
three_tracks() {
    local case=$1 tag=$2 pxc="./tools/px"
    cp "$case" "$WORK/$tag.px"
    timeout 120 "$PXI" "$WORK/$tag.px" > "$WORK/$tag.interp.out" 2>&1 || true

    rm -rf "$WORK/build"
    if timeout 900 $pxc build "$WORK/$tag.px" > "$WORK/$tag.vm.log" 2>&1 && [ -x "$WORK/build/$tag" ]; then
        timeout 120 "$WORK/build/$tag" > "$WORK/$tag.vm.out" 2>&1 || true
    else
        : > "$WORK/$tag.vm.out"; note "VM 轨编译失败（日志 $WORK/$tag.vm.log）"
    fi

    rm -rf "$WORK/build"
    local cok=0
    if [ -n "${PX_PXC_BIN:-}" ]; then
        timeout 900 env PX_PXC_BIN="$PX_PXC_BIN" $pxc build --c "$WORK/$tag.px" > "$WORK/$tag.c.log" 2>&1 && cok=1
    else
        timeout 900 $pxc build --c "$WORK/$tag.px" > "$WORK/$tag.c.log" 2>&1 && cok=1
    fi
    if [ $cok -eq 1 ] && [ -x "$WORK/build/$tag" ]; then
        timeout 120 "$WORK/build/$tag" > "$WORK/$tag.c.out" 2>&1 || true
    else
        : > "$WORK/$tag.c.out"; note "C 轨编译失败（日志 $WORK/$tag.c.log）"
    fi
    rm -rf "$WORK/build"
}

expect_all_three() {     # $1=标签 $2=期望行（如 "pass=26 fail=0"）
    local tag=$1 want=$2
    if grep -q "^$want\$" "$WORK/$tag.interp.out"; then
        note "解释轨：$want"
    else
        bad "$tag 解释轨未达 $want"; sed 's/^/      /' "$WORK/$tag.interp.out" | head -6
    fi
    for t in vm c; do
        if grep -q "^$want\$" "$WORK/$tag.$t.out"; then
            note "$t 轨：$want"
        else
            bad "$tag $t 轨未达 $want（$(grep -m1 -E 'FAIL|运行时错误|错误' "$WORK/$tag.$t.out" | head -c 120)）"
        fi
        cmp -s "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" \
            || { bad "$tag $t 轨 stdout 与解释轨不一致"; diff "$WORK/$tag.interp.out" "$WORK/$tag.$t.out" | head -6 | sed 's/^/      /'; }
    done
}

hdr "[1/4] 解释轨 + VM 轨 + C 轨：sorted 主用例（26 断言）"
three_tracks "$CASE" main
expect_all_three main "pass=26 fail=0"

hdr "[2/4] 加强面：同键三元素/嵌套稳定/长列表/跨类型/min·max 回归（12 断言）"
three_tracks "$EDGE" edge
expect_all_three edge "pass=12 fail=0"

hdr "[3/4] 交叉判据：PX-DEF-011 最小复现器（第三方登记件）三轨一致"
three_tracks "$DEF011" def011
expect_all_three def011 "pass=4 fail=0"

hdr "[4/4] 语义登记（不计失败）"
note "语义基准：sorted = **按值比较**（runtime compare_values 三态）+ **稳定**（等值保持原序）"
note "PXI=$PXI（PX_PXI_BIN 可覆盖）· 编译轨 PX_PXC_BIN=${PX_PXC_BIN:-<入库 pxc>}"

# ---------------- 负控 ----------------
patch_one() {            # $1=文件 $2=旧文本 $3=新文本 → 唯一性校验后替换
    python3 - "$1" "$2" "$3" <<'PY'
import sys
p, a, b = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p, encoding="utf-8").read()
if s.count(a) != 1:
    print("锚点不唯一（%d）" % s.count(a)); sys.exit(1)
open(p, "w", encoding="utf-8").write(s.replace(a, b, 1))
PY
}

run_neg() {              # $1=标签 $2=轨(interp|vm|c) $3=文件 $4=旧 $5=新 $6=说明 $7=用例 $8=期望行
    local tag=$1 track=$2 file=$3 from=$4 to=$5 what=$6 case=$7 want=$8
    echo "── 负控 $tag：$what"
    cp "$file" "$WORK/$(basename "$file").bak"
    patch_one "$file" "$from" "$to" || { bad "$tag 锚点失效（与源码不同步）"; cp "$WORK/$(basename "$file").bak" "$file"; return; }

    if [ "$track" = "interp" ]; then
        # ⚠️ 必须改 **PXI 变量本身**：`PX_PXI_BIN=... three_tracks` 无效 —— PXI 在本脚本
        #    开头就已展开为具体路径，环境变量只对子进程生效 ⇒ 负控会跑入库件、假绿。
        local old_pxi="$PXI"
        if timeout 900 ./selfhost/devbuild.sh pxi > "$WORK/neg$tag.build.log" 2>&1; then
            PXI=/tmp/pxidev
            three_tracks "$case" neg$tag
        else
            bad "$tag 解释器重建失败"; tail -3 "$WORK/neg$tag.build.log" | sed 's/^/      /'
        fi
        PXI="$old_pxi"
        rm -f /tmp/pxidev        # 不留篡改版（下次重烘再生成）
    else
        # 编译轨：改 runtime ⇒ 强制新 rtcache（用入库 pxc_vm 现编 runtime 对象）
        if timeout 1200 ./tools/px build --full examples/hello.px > "$WORK/neg$tag.rt.log" 2>&1; then
            three_tracks "$case" neg$tag
        else
            bad "$tag runtime 重建失败"; tail -3 "$WORK/neg$tag.rt.log" | sed 's/^/      /'
        fi
    fi

    local out="$WORK/neg$tag.$track.out"
    if grep -q "^$want\$" "$out" 2>/dev/null; then
        bad "$tag 负控**未判红**（篡改后仍全绿 ⇒ 门的判别力不足）"
    else
        local why
        why=$(grep -m1 -E 'FAIL|运行时错误|未定义变量|错误' "$out" 2>/dev/null | head -c 120)
        [ -n "$why" ] || why="（无 stdout 差异，见 $WORK/neg$tag.*）"
        note "✅ $tag 负控判红：$why"
    fi
    cp "$WORK/$(basename "$file").bak" "$file"
    cmp -s "$WORK/$(basename "$file").bak" "$file" || bad "$tag 还原失败（源码与备份不一致）"
}

if [ $NEG_SKIP -eq 0 ]; then
    run_neg A interp selfhost/ibuiltin.px \
        'if i_cmp_values(items[j], items[j + 1]) > 0:' \
        'if i_to_str(items[j]) > i_to_str(items[j + 1]):' \
        "值比较退回渲染串比较（恢复缺陷 166）" "$CASE" "pass=26 fail=0"
    run_neg C interp selfhost/ival.px \
        '            let c = i_cmp_values(a[i], b[i])' \
        '            let c = 0' \
        "list 逐元素比较退化为只比长度（PX-DEF-011 面）" "$DEF011" "pass=4 fail=0"
    run_neg B vm runtime/runtime.c \
        '        for (int j = 0; j + 1 < ro->as.list.len - i; j++) {
            if (compare_values(ro->as.list.items[j], ro->as.list.items[j + 1]) > 0) {
                LXValue t = ro->as.list.items[j];
                ro->as.list.items[j] = ro->as.list.items[j + 1];
                ro->as.list.items[j + 1] = t;
            }
        }' \
        '        for (int j = i + 1; j < ro->as.list.len; j++) {
            if (compare_values(ro->as.list.items[j], ro->as.list.items[i]) < 0) {
                LXValue t = ro->as.list.items[i];
                ro->as.list.items[i] = ro->as.list.items[j];
                ro->as.list.items[j] = t;
            }
        }' \
        "相邻冒泡退回选择式（恢复缺陷 167：不稳定）" "$CASE" "pass=26 fail=0"
fi

echo
if [ $fail -eq 0 ]; then
    echo "✅ M162-VERIFY-OK（sorted 三轨一致：值比较 + 稳定排序）"
else
    echo "❌ M162-VERIFY-FAIL"
fi
exit $fail
