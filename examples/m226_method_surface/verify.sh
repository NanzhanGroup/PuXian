#!/usr/bin/env bash
# ============================================================
# M226 门 · **方法面「逐位置 × 错类型 + 错 arity」全量对拍**（[S10-c] · 缺陷 328–335）
# ------------------------------------------------------------
# 病灶：M190 量过「方法实参**个数**」、M199 量过「native **函数**面 × 每个位置 × 错类型」，
#   而**方法面**从来没有清单级 / 全量度量。本轮把它变成可全量度量，222 例 → **32 例分叉**：
#     · 328 `[1,2].join()`（0 参）⇒ 直接读 `args[0]` = **越界读** ⇒ VM 轨 SIGSEGV（rc=139 core）
#     · 329 `list.contains` / `dict.has|contains|remove` 判据是 `< 1` ⇒ **多余实参静默忽略**
#     · 330 解释轨缺参报 R1002「参数 1 需要 string」、编译轨报 R1005「需要 1 个参数」
#     · 331 `str.to_lower` / `to_upper` 只在编译轨（解释轨响亮「没有方法」）
#     · 332 `dict.put` 只在解释轨（编译轨响亮「没有方法」）
#     · 333 `list.join` 分隔符错类型：`join 分隔符需要 string` ⇄ `方法 join 参数 1 需要 string`
#     · 334 `dict.remove` 缺键：解释轨 R1008 ⇄ 编译轨静默返回 null
#     · 335 `dict.get` 放过多余实参（>2 参静默）
#
# 判据：
#   [1] 静态：方法清单**从源码派生**（不许手抄）⇄ `spec.tsv` 双向一致 ⇄ `UNPAIRED.tsv` 精确相等
#   [2] 动态：222 例三轨对拍 ⇒ **分叉 0**（含 34 个方法的合法调用侧）
#   [3] 崩溃回归：`list_join_n0` 三轨 rc 均 < 128（**不是被信号杀死**）且同为 R1005
#   [4] 负控 A：忠实撤回 `runtime.c` 7 处 ⇒ 分叉必须回来
#   [5] 负控 B：忠实撤回 `selfhost/icall.px` 4 处 ⇒ 分叉必须回来
#   [6] 负控 C：判据自伤（比对恒真）⇒ A 的红**消失**（证明红来自比对）
#   [7] 覆盖边界（如实登记）
# CI 用 `--neg-skip`（负控要重编驱动两轨 + 重编解释轨件）。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m226_method_surface
W=/tmp/m226_gate
rm -rf "$W"; mkdir -p "$W"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }

RT="$ROOT/runtime/runtime.c"
IC="$ROOT/selfhost/icall.px"
BAK_RT="$W/runtime.c.bak"; BAK_IC="$W/icall.px.bak"
snap()  { cp -f "$RT" "$BAK_RT"; cp -f "$IC" "$BAK_IC"; }
restore() { cp -f "$BAK_RT" "$RT"; cp -f "$BAK_IC" "$IC"; }
trap 'restore; rm -f "$BAK_RT" "$BAK_IC"' EXIT
snap

# 解释轨件：默认用**入库件**（重烘后已含本轮修复）；可用 M226_PXI 覆盖（负控 B 用开发件）
PXI="${M226_PXI:-$ROOT/bootstrap/pxi}"

build_drivers() {   # 建 VM / C 两轨驱动器 → $W/build/{drv_vm,drv_c}
    rm -rf "$W/a_vm" "$W/a_c" "$W/build"
    mkdir -p "$W/a_vm" "$W/a_c" "$W/build"
    cp "$W/drv.px" "$W/a_vm/drv.px"; cp "$W/drv.px" "$W/a_c/drv.px"
    ( cd "$W/a_vm" && timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_vm.log" 2>&1 || { cat "$W/b_vm.log" | tail -12; return 1; }
    ( cd "$W/a_c"  && PX_BUILD_ENGINE=c timeout 900 "$ROOT/tools/px" build drv.px ) > "$W/b_c.log" 2>&1 || { tail -12 "$W/b_c.log"; return 1; }
    cp -f "$W/a_vm/build/drv" "$W/build/drv_vm" && cp -f "$W/a_c/build/drv" "$W/build/drv_c"
    [ -x "$W/build/drv_vm" ] && [ -x "$W/build/drv_c" ]
}

x3() {  # 三轨对拍 → stdout 落 $1；**只把 rc 交给调用方**（别 echo 到 stdout，会混进门日志）
    python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --pxi "$PXI" --json "$W/rows.json" \
        > "$1" 2>&1
    return $?
}
ndiv() { grep -m1 '分叉 ' "$1" | sed -n 's/.*分叉 \([0-9]*\)\*\*.*/\1/p'; }

echo "=== [1] 静态：方法清单（**源码派生**）⇄ spec.tsv ⇄ UNPAIRED.tsv"
python3 "$D/gen_probes.py" --root "$ROOT" --out "$W" --here "$D" > "$W/gen.log" 2>&1
genrc=$?
cat "$W/gen.log" | sed 's/^/  /'
chk "[1] 生成/清单/漂移判据 rc=0" "[ $genrc -eq 0 ]"
chk "[1] 清单差集 ⇄ UNPAIRED.tsv 精确相等" "grep -q '差集 ⇄ UNPAIRED.tsv 精确相等' $W/gen.log"
chk "[1] spec.tsv ⇄ 源码派生清单双向一致" "grep -q 'spec.tsv ⇄ 源码派生清单双向一致' $W/gen.log"
chk "[1] 规模下限：方法 ≥ 30" "grep -qE 'spec.tsv ⇄ 源码派生清单双向一致（3[0-9]|4[0-9]）' $W/gen.log"
chk "[1] 规模下限：用例 ≥ 200" "grep -qE '生成 2[0-9][0-9] 例|生成 [3-9][0-9][0-9] 例' $W/gen.log"
chk "[1] 负控锚点自证（11 处，逐条唯一）" "M226_ROOT=$ROOT python3 $D/negctl.py --selftest | grep -q '自证 OK'"
[ $genrc -ne 0 ] && { echo "生成失败，后续层跳过"; echo "M226-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; }

echo "=== [2] 动态：两轨驱动器 + 三轨对拍（222 例）"
if ! build_drivers; then
    chk "[2] 两轨驱动器构建" "false"
else
    chk "[2] 两轨驱动器构建" "true"
    x3 "$W/base.log"; rc=$?
    cat "$W/base.log" | sed 's/^/  /'
    chk "[2] 三轨对拍 rc=0（分叉 0）" "[ $rc -eq 0 ]"
    chk "[2] 覆盖规模 ≥ 200 例" "grep -qE '对拍：2[0-9][0-9] 例' $W/base.log"
    # 留一份**基线件**：负控 A 会重编驱动器覆盖 $W/build，负控 B 需要回到基线
    cp -f "$W/build/drv_vm" "$W/drv_vm.base"; cp -f "$W/build/drv_c" "$W/drv_c.base"
fi

echo "=== [3] 崩溃回归：list_join_n0（缺陷 328 的原始症状）"
python3 - "$W/rows.json" <<'PY' > "$W/crash.txt" 2>&1
import json, sys
rows = {r['label']: r for r in json.load(open(sys.argv[1]))}
r = rows['list_join_n0']
bad = []
for t in ('interp', 'vm', 'c'):
    d = r[t]
    if d['rc'] >= 128:
        bad.append(f'{t} rc={d["rc"]}（被信号杀死）')
    if d['code'] != 'R1005':
        bad.append(f'{t} code={d["code"]}（期望 R1005）')
print('CRASH-OK' if not bad else 'CRASH-FAIL ' + '; '.join(bad))
PY
cat "$W/crash.txt" | sed 's/^/  /'
chk "[3] join() 三轨 rc<128 且同码 R1005" "grep -q CRASH-OK $W/crash.txt"

if [ "$NEG" = "1" ]; then
    echo "=== [4] 负控 A：忠实撤回 runtime.c（7 处）"
    M226_ROOT=$ROOT python3 "$D/negctl.py" --apply A
    if build_drivers; then
        x3 "$W/ncA.log"; rc2=$?
        cat "$W/ncA.log" | sed 's/^/  /'
        a1=$(ndiv "$W/ncA.log")
        chk "[4] 负控 A 判红（分叉 > 0）" "[ \"\${a1:-0}\" -gt 0 ]"
        chk "[4] 负控 A 含 join 越界族（list_join_n0）" "grep -q 'list_join_n0' $W/ncA.log"
        chk "[4] 负控 A 含 put 清单族（dict_put_ok）" "grep -q 'dict_put_ok' $W/ncA.log"
        chk "[4] 负控 A 含缺键语义族（dict_remove_p0_s）" "grep -q 'dict_remove_p0_s' $W/ncA.log"
        chk "[4] 负控 A 含静默多参族（list_contains_n3）" "grep -q 'list_contains_n3' $W/ncA.log"
        echo "=== [6] 负控 C：判据自伤（比对恒真）⇒ A 的红必须消失"
        python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" --pxi "$PXI" --harm > "$W/ncC.log" 2>&1
        chk "[6] 自伤后 NDIV=0（红确实来自比对）" "grep -q '分叉 0' $W/ncC.log"
        restore
        chk "[6] 源码逐字节还原" "cmp -s $RT $BAK_RT && cmp -s $IC $BAK_IC"
    else
        chk "[4] 负控 A 设备构建" "false"
    fi

    echo "=== [5] 负控 B：忠实撤回 selfhost/icall.px（4 处）"
    M226_ROOT=$ROOT python3 "$D/negctl.py" --apply B
    if [ -x "$ROOT/selfhost/devbuild.sh" ] && ./selfhost/devbuild.sh pxi > "$W/dev_xi.log" 2>&1 && [ -x /tmp/pxidev ]; then
        chk "[5] 开发件 pxi 构建" "true"
        restore
        chk "[5] 源码逐字节还原" "cmp -s $RT $BAK_RT && cmp -s $IC $BAK_IC"
        cp -f /tmp/pxidev "$W/pxidev_rev"
        # 负控 A 重编过驱动器（运行时已撤回）⇒ 回到**基线件**，否则 A/B 两个变量混在一起
        cp -f "$W/drv_vm.base" "$W/build/drv_vm"; cp -f "$W/drv_c.base" "$W/build/drv_c"
        chk "[5] 恢复基线驱动器（隔离 A/B 两个变量）" "[ -x $W/build/drv_vm ] && [ -x $W/build/drv_c ]"
        python3 "$D/three_tracks.py" --root "$ROOT" --work "$W" \
            --pxi "$W/pxidev_rev" > "$W/ncB.log" 2>&1
        cat "$W/ncB.log" | sed 's/^/  /'
        b1=$(ndiv "$W/ncB.log")
        chk "[5] 负控 B 判红（分叉 > 0）" "[ \"\${b1:-0}\" -gt 0 ]"
        chk "[5] 负控 B 含 to_upper 清单族（str_toupper_ok）" "grep -q 'str_toupper_ok' $W/ncB.log"
        chk "[5] 负控 B 含缺参口径族（dict_get_n0）" "grep -q 'dict_get_n0' $W/ncB.log"
    else
        echo "  SKIP 负控 B（devbuild 不可用）"; tail -5 "$W/dev_xi.log" 2>/dev/null
    fi
else
    echo "=== [4][5][6] 负控（--neg-skip：需要重编驱动两轨 + 重编解释轨件）"
fi

echo "=== [7] 覆盖边界（如实登记）"
cat <<'TXT' | sed 's/^/  /'
  · 只判「R 码 + 消息体 + rc + 程序输出」—— **通道与行:列前缀不判**（属已登记缺陷 186 族）。
  · 接收者类型只覆盖 str / list / dict；chan / mutex / rwlock / result / tuple 未纳入
    （解释轨对前三者不按类型分派，属 MINI_SUBSET.md 的设计性缺口；result 方法本轮未探）。
  · 只覆盖「**接收者类型正确、实参错**」。接收者本身错类型（如 `(7).len()`）未纳入。
  · 未覆盖方法链式调用 / 方法值（`d.get` 作为一等值）/ impl 方法（struct 面）。
  · 合法侧只取每方法**一种**实参组合（毒值面是 6 种 × 每位置）。
TXT

echo "──────────────────────────────────────────"
if [ $fail -eq 0 ]; then echo "M226-VERIFY-OK pass=$pass fail=$fail"; else echo "M226-VERIFY-FAIL pass=$pass fail=$fail"; fi
[ $fail -eq 0 ]
