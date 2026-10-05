#!/bin/bash
# M230 门 · **索引 / 切片族**三轨全量对拍（缺陷 345）
#
# 主题：M226 量过方法面、M227 量过「同名两门」、M228 量过「运算符门（in）」、
#   M229 量过 tuple/result 接收者 —— 而 **`x[i]` / `x[i]=v` / `x[a:b]` 这一族
#   从来没有清单级 / 全量度量**（它是最常用的操作）。
#
# 缺陷 345：**负索引越界的消息报「归一化后的位置」而不是用户输入的值** ——
#   `[10,20,30][-4]` 报「索引越界: -1」、`l[-5] = 9` 报「索引越界: -2」。
#   三轨一致地错 ⇒ **任何「三轨对拍」门按定义看不见**（M215/M226/M228/M229 同族立论）；
#   且违反本仓纪律「错误必须指到真因」（M185/M187）。只有**期望值判据**能看见。
#
# 判据
#   [1] 静态：消息一律传原值（不得再传归一化值）· 规模锚点 · 负控锚点自证
#   [2] 生成探针 + 构建三轨（C / VM）
#   [3] 三轨对拍：56 例 × 3 轨 = 168 次执行 ⇒ 跨轨一致 · 期望一致 · 指到真因
#   [4] MODEL.tsv 双向核对（漏登记与过期都判红）
#   [5][6][7] 负控 A/B/C（各自独立判红 + 源逐字节还原）
#   [8] 覆盖边界（如实登记）
#
# CI 用 `--neg-skip`（负控各要重构一次三轨 ≈ 3–4 min）。
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m230_index_slice
W=/tmp/m230_gate
rm -rf "$W"; mkdir -p "$W"
export M230_NEG_W="$W/snap"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
PXI="${M230_PXI:-$ROOT/bootstrap/pxi}"
NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" "$@"; }
has() { grep -qF "$2" "$ROOT/$1"; }
cnt() { grep -cF "$2" "$ROOT/$1" || true; }

echo "── [1] 静态判据"
V_OLD=$(cnt runtime/runtime.c '索引越界: %d (len=%d)", i,')
V_RAW=$(cnt runtime/runtime.c 'm230_raw')
VM_OLD=$(cnt runtime/vm.c '索引越界: %d (len=%d)", i,')
VM_RAW=$(cnt runtime/vm.c 'm230_raw')
IV_RAW=$(cnt selfhost/ival.px 'm230_raw')
N_CASES=$(( $(wc -l < "$D/cases.tsv") - 1 ))
chk "runtime：索引越界消息一律传原值（无残留归一化值）" "[ '$V_OLD' = 0 ]"
chk "runtime：原值变量 ≥ 8 处（list/tuple/str/bytes/iter/bytes_get/bytes_set）" "[ '$V_RAW' -ge 8 ]"
chk "vm.c：快路径同口径（PXOP_INDEX + PXOP_ITERAT，无残留归一化值）" "[ '$VM_OLD' = 0 ] && [ '$VM_RAW' -ge 2 ]"
chk "ival.px：解释轨同口径（i_as_index 报原值）" "[ '$IV_RAW' -ge 1 ]"
chk "规模锚点：例数 ≥ 50（实测 $N_CASES）" "[ '$N_CASES' -ge 50 ]"
chk "负控锚点自证（7 处唯一命中）" "NEGCTL --selftest >/dev/null"

echo "── [2] 生成探针 + 构建三轨驱动器"
python3 "$D/gen_probes.py" --gen >"$W/gen.log" 2>&1 && chk "探针生成" true || { chk "探针生成" false; tail -5 "$W/gen.log"; }
[ -f "$PXI" ] && chk "解释轨件存在（$PXI）" true || chk "解释轨件存在" false

build_drivers() {
    rm -rf "$W/a_c" "$W/a_vm" "$W/build"; mkdir -p "$W/a_c" "$W/a_vm" "$W/build"
    cp -f "$D/drv.px" "$W/a_c/drv.px"; cp -f "$D/drv.px" "$W/a_vm/drv.px"
    ( cd "$W/a_c" && PX_BUILD_ENGINE=c timeout 1200 "$ROOT/tools/px" build drv.px ) >"$W/b_c.log" 2>&1 || { tail -12 "$W/b_c.log"; return 1; }
    ( cd "$W/a_vm" && timeout 1200 "$ROOT/tools/px" build drv.px ) >"$W/b_vm.log" 2>&1 || { tail -12 "$W/b_vm.log"; return 1; }
    cp -f "$W/a_c/build/drv" "$W/build/drv_c" && cp -f "$W/a_vm/build/drv" "$W/build/drv_vm"
    [ -x "$W/build/drv_c" ] && [ -x "$W/build/drv_vm" ]
}
if build_drivers; then chk "两轨驱动器构建" true; else chk "两轨驱动器构建" false; echo "M230-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; fi

echo "── [3] 三轨对拍"
run3() { M230_PXI="$PXI" timeout 1200 python3 "$D/gen_probes.py" --root "$ROOT" --work "$W" --drv "$ROOT/$D/drv.px"; }
if run3 >"$W/three.log" 2>&1; then chk "三轨对拍（$((N_CASES*3)) 次执行）" "grep -q M230-THREE-TRACKS-OK '$W/three.log'"
else chk "三轨对拍（$((N_CASES*3)) 次执行）" false; head -25 "$W/three.log"; fi
grep -E '跨轨 0|差异' "$W/three.log" | head -2

echo "── [4] 口径表 MODEL.tsv 双向核对"
if [ -f "$D/MODEL.tsv" ] && [ -f "$W/model_actual.tsv" ]; then
    if diff <(sort "$D/MODEL.tsv") <(sort "$W/model_actual.tsv") >"$W/model.diff" 2>&1; then
        chk "MODEL 双向一致（实测 ⇄ 表 精确相等）" true
    else
        chk "MODEL 双向一致（漏登记与过期都判红）" false; head -12 "$W/model.diff"
    fi
else chk "MODEL 双向一致" false; fi

if [ "$NEG" = 1 ]; then
    NEGCTL --snapshot >/dev/null
    echo "── [5] 负控 A：撤回 runtime 的原值（回到缺陷 345 修前）"
    if NEGCTL --apply A >"$W/ncA.patch" 2>&1 && build_drivers >"$W/ncA.build" 2>&1; then
        run3 >"$W/ncA.log" 2>&1; rcA=$?
        chk "负控 A 必红" "[ $rcA -ne 0 ] && grep -q 'M230-THREE-TRACKS-FAIL' '$W/ncA.log'"
        grep -oE '差异 [0-9]+ 项' "$W/ncA.log" | head -1
    else chk "负控 A 必红" false; tail -5 "$W/ncA.patch" "$W/ncA.build"; fi
    NEGCTL --restore >/dev/null

    echo "── [6] 负控 B：撤回解释轨的原值（只动 ival.px）"
    ( cd "$ROOT" && DEVB_REBUILD=1 timeout 1800 ./selfhost/devbuild.sh pxi ) >"$W/ncB.dev" 2>&1
    cp -f /tmp/pxidev "$W/pxi.base" 2>/dev/null || true
    if NEGCTL --apply B >"$W/ncB.patch" 2>&1 && ( cd "$ROOT" && DEVB_REBUILD=1 timeout 1800 ./selfhost/devbuild.sh pxi ) >"$W/ncB.dev2" 2>&1; then
        run3 >"$W/ncB.log" 2>&1; rcB=$?
        chk "负控 B 必红（解释轨 4 例）" "[ $rcB -ne 0 ] && grep -q 'M230-THREE-TRACKS-FAIL' '$W/ncB.log'"
        grep -oE '差异 [0-9]+ 项' "$W/ncB.log" | head -1
    else chk "负控 B 必红" false; tail -5 "$W/ncB.patch" "$W/ncB.dev2"; fi
    NEGCTL --restore >/dev/null
    ( cd "$ROOT" && DEVB_REBUILD=1 timeout 1800 ./selfhost/devbuild.sh pxi ) >"$W/ncB.dev3" 2>&1
    if [ -f "$W/pxi.base" ]; then
        chk "还原后重编 pxi 与负控前基线逐字节一致" "cmp -s '$W/pxi.base' /tmp/pxidev"
    else chk "还原后重编 pxi 与负控前基线逐字节一致" false; fi

    echo "── [7] 负控 C：判据自伤（施加 A 的同时关掉期望判据）⇒ A 的红必须消失"
    if NEGCTL --apply A >"$W/ncC.patch" 2>&1 && build_drivers >"$W/ncC.build" 2>&1; then
        M230_HARM=1 run3 >"$W/ncC.log" 2>&1; rcC=$?
        chk "负控 C：同一故障不再判红（证明红来自比对）" "[ $rcC -eq 0 ] && grep -q 'M230-THREE-TRACKS-OK' '$W/ncC.log'"
    else chk "负控 C" false; tail -5 "$W/ncC.patch" "$W/ncC.build"; fi
    NEGCTL --restore >/dev/null
    for f in runtime/runtime.c runtime/vm.c selfhost/ival.px; do
        b=$(echo "$f" | sed 's|/|__|g')
        chk "源逐字节还原：$f" "cmp -s '$ROOT/$f' '$W/snap/$b'"
    done
fi

echo "── [8] 覆盖边界（如实登记）"
echo "     · 容器：list / dict / string / tuple + 非容器（int/null/bool/result）"
echo "     · 操作：索引读 x[i] · 索引写 x[i]=v · 切片 x[a:b]（含步长 [::2]）"
echo "     · 形状：合法（正/负/边界）· 越界正 · 越界负 · 缺键 · 索引或键错类型 · 接收者错类型"
echo "     · **未覆盖**：bytes 的 b[i]（M185/M189 已单独收口）· 生成器物化后的索引"
echo "       · 嵌套切片链（m[a][b:c]）· 迭代位置语义（M164 已与用户索引分离，另门覆盖）"
echo "     · 通道与行:列前缀**不判**（已登记缺陷 186，按 M186 归一化）"
echo "     · ⚠️ 负索引越界的**消息**是本轮主判据；正索引越界的消息两侧本来就一致"

[ "$fail" = 0 ] || { echo "M230-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; }
echo "M230-VERIFY-OK pass=$pass fail=$fail"
