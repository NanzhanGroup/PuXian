#!/usr/bin/env bash
# ============================================================
# M229 门 · **`tuple` / `result` 两个接收者的方法面全量对拍**（M226 的覆盖面补齐）
# ------------------------------------------------------------
# 主题：M226 把方法面的「逐位置 × 错类型 + 错 arity」量到 **str/list/dict**；
#   M227 补「同名两门」、M228 补「运算符门」。**`tuple` 与 `result` 这两个接收者
#   从未被度量过** —— 它们的**同族内部**是否一致，此前没有任何门看得见。
#   一量就照出两处：
#     · 缺陷 343：`ok`/`err` **完全不查 arity** ⇒ `Ok(7).ok(1)` 静默返回 7、
#       `Ok(7).err(1)` 静默返回 null；而**同一个接收者**上 is_ok/is_err/unwrap/unwrap_err
#       四个都有检查 ⇒「同族六个方法，四个拦、两个放行」（M226 缺陷 329 / M227 缺陷 336 的形状）。
#     · 缺陷 344：方法面 R1005 的**措辞三种写法并存** —— 主流 `方法 X …`、
#       result 全族 + `pop` 用 `X …`、`list.index`/`mutex.with`/`rwlock.*` 用 `类型.方法 …`。
#       本轮把前两族统一到 M226 立的 `方法 X …`；第三族**有意保留**（见覆盖边界）。
#
# 判据：
#   [1] 静态：`tuple`/`result` 的方法清单**从源码派生** ⇄ `cases.tsv` 双向一致 ＋
#       规模锚点 ＋ 措辞族普查（方法面 R1005 只允许 `方法 X …` 或登记的 `类型.方法 …`）
#   [2] 构建三轨驱动器（35 例聚合 · 一次编译）
#   [3] 动态：35 例 × 3 轨 = 105 次 ⇒ 跨轨 0 · H1 arity 0 · H2 措辞 0 · H4 合法侧一致
#   [4] 负控 A：撤回 `ok`/`err` 的 arity 检查 ⇒ H1 必红
#   [5] 负控 B：撤回措辞统一 ⇒ H2 必红
#   [6] 负控 C：判据自伤（`--harm`）⇒ A 的红消失
#   [7] 源逐字节还原（含「还原后重编 pxi 与负控前基线一致」）
#   [8] 覆盖边界（如实登记）
# CI 用 `--neg-skip`（负控各要完整重建一次 runtime ≈ 6–8 min）。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m229_result_tuple
W=/tmp/m229_gate
rm -rf "$W"; mkdir -p "$W"
export M229_NEG_W="$W/snap"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
PXI="${M229_PXI:-$ROOT/bootstrap/pxi}"
NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" "$@"; }
has() { grep -qF "$2" "$ROOT/$1"; }
cnt() { grep -cF "$2" "$ROOT/$1" || true; }

echo "── [1] 静态判据"
chk "runtime: ok/err 已补精确 arity" \
    "has runtime/runtime.c 'px_error(\"R1005: 方法 ok 不接受参数\")' && has runtime/runtime.c 'px_error(\"R1005: 方法 err 不接受参数\")'"
chk "icall.px: ok/err 同步补 arity" \
    "has selfhost/icall.px 'i_r1005(\"方法 ok 不接受参数\"' && has selfhost/icall.px 'i_r1005(\"方法 err 不接受参数\"'"
chk "措辞族统一：result 四法补「方法」前缀" \
    "has runtime/runtime.c 'R1005: 方法 is_ok 不接受参数' && has runtime/runtime.c 'R1005: 方法 unwrap_err 不接受参数'"
chk "措辞族统一：pop 补「方法」前缀（两轨）" \
    "has runtime/runtime.c 'R1005: 方法 pop 不接受参数' && has selfhost/icall.px 'i_r1005(\"方法 pop 不接受参数\"'"
# 措辞族普查：方法面 R1005 里不属于「方法 X …」的只允许登记的 类型.方法 族
chk "措辞族普查：非「方法 X」写法只剩 登记族（5 条）" \
    "[ \"\$(grep -oE '\\\"R1005: [^\\\"]*\\\"' runtime/runtime.c | grep -vc '方法 ')\" = 5 ]"
chk "  且「类型.方法」族恰 4 条" \
    "[ \"\$(grep -oE '\\\"R1005: [^\\\"]*\\\"' runtime/runtime.c | grep -v '方法 ' | grep -cE 'R1005: (list\\.index|mutex\\.with|rwlock\\.with)')\" = 4 ]"
chk "  且新增的枚举构造族恰 1 条（M237 缺陷 368：E(V) 的 arity）" \
    "[ \"\$(grep -oE '\\\"R1005: [^\\\"]*\\\"' runtime/runtime.c | grep -v '方法 ' | grep -cE 'R1005: 枚举 ')\" = 1 ]"
chk "规模锚点：例数 ≥ 30" "[ \"\$(cnt $D/cases.tsv '')\" -ge 31 ]"
chk "负控锚点自证（唯一性）" "NEGCTL --selftest >/dev/null"

echo "── [2] 生成探针 + 构建三轨驱动器"
python3 "$D/gen_probes.py" --gen >"$W/gen.log" 2>&1 && chk "探针生成" "true" || { chk "探针生成" "false"; tail -5 "$W/gen.log"; }
[ -f "$PXI" ] && chk "解释轨件存在（$PXI）" "true" || chk "解释轨件存在（$PXI）" "false"

build_drivers() {
    rm -rf "$W/a_c" "$W/a_vm" "$W/build"
    mkdir -p "$W/a_c" "$W/a_vm" "$W/build"
    cp -f "$D/drv.px" "$W/a_c/drv.px"; cp -f "$D/drv.px" "$W/a_vm/drv.px"
    ( cd "$W/a_c" && PX_BUILD_ENGINE=c timeout 1200 "$ROOT/tools/px" build drv.px ) >"$W/b_c.log" 2>&1 || { tail -12 "$W/b_c.log"; return 1; }
    ( cd "$W/a_vm" && timeout 1200 "$ROOT/tools/px" build drv.px ) >"$W/b_vm.log" 2>&1 || { tail -12 "$W/b_vm.log"; return 1; }
    cp -f "$W/a_c/build/drv" "$W/build/drv_c" && cp -f "$W/a_vm/build/drv" "$W/build/drv_vm"
    [ -x "$W/build/drv_c" ] && [ -x "$W/build/drv_vm" ]
}
if build_drivers; then chk "两轨驱动器构建" "true"; else chk "两轨驱动器构建" "false"; echo "M229-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; fi

echo "── [3] 三轨对拍"
run3() { M229_PXI="$PXI" timeout 1200 python3 "$D/gen_probes.py" --root "$ROOT" --work "$W" \
            --drv "$ROOT/$D/drv.px" "$@"; }
if run3 >"$W/three.log" 2>&1; then chk "三轨对拍（105 次执行）" "grep -q M229-THREE-TRACKS-OK '$W/three.log'"
else chk "三轨对拍（105 次执行）" "false"; head -20 "$W/three.log"; fi

if [ "$NEG" = 1 ]; then
    # 负控前基线（[7] 的还原自证用；devbuild 与 rebake_bin 的链接参数不同 ⇒ 只能比基线）
    ( cd "$ROOT" && DEVB_REBUILD=1 timeout 1800 ./selfhost/devbuild.sh pxi ) >"$W/pre.build" 2>&1
    cp -f /tmp/pxidev "$W/pxi.pre" 2>/dev/null || true

    echo "── [4] 负控 A：撤回 ok/err 的 arity 检查"
    NEGCTL --restore >/dev/null 2>&1
    if NEGCTL --apply A >"$W/ncA.patch" 2>&1 && build_drivers >"$W/ncA.build" 2>&1; then
        run3 >"$W/ncA.log" 2>&1; rcA=$?
        chk "负控 A 必红" "[ $rcA -ne 0 ] && grep -q 'M229-THREE-TRACKS-FAIL' '$W/ncA.log'"
        grep -oE '差异 [0-9]+ 项' "$W/ncA.log" | head -1
    else chk "负控 A 必红" "false"; tail -5 "$W/ncA.patch" "$W/ncA.build"; fi

    echo "── [6] 负控 C：判据自伤（A 在位 + --harm ⇒ A 的红消失）"
    if run3 --harm >"$W/ncC.log" 2>&1; then chk "负控 C 判据自伤" "grep -q M229-VERIFY-OK '$W/ncC.log'"
    else chk "负控 C 判据自伤" "false"; head -6 "$W/ncC.log"; fi

    echo "── [5] 负控 B：撤回措辞统一"
    NEGCTL --restore >/dev/null 2>&1
    if NEGCTL --apply B >"$W/ncB.patch" 2>&1 && build_drivers >"$W/ncB.build" 2>&1; then
        run3 >"$W/ncB.log" 2>&1; rcB=$?
        chk "负控 B 必红" "[ $rcB -ne 0 ] && grep -q 'M229-THREE-TRACKS-FAIL' '$W/ncB.log'"
        grep -oE '差异 [0-9]+ 项' "$W/ncB.log" | head -1
    else chk "负控 B 必红" "false"; tail -5 "$W/ncB.patch" "$W/ncB.build"; fi
else
    echo "── [4][5][6] 负控跳过（--neg-skip）"
fi

echo "── [7] 源逐字节还原"
NEGCTL --restore >"$W/restore.log" 2>&1; chk "还原" "NEGCTL --restore >/dev/null 2>&1"
chk "源码无负控残留" "! grep -rq 'NEGCTL-229' '$ROOT/runtime' '$ROOT/selfhost'"
if [ "$NEG" = 1 ]; then
    ( cd "$ROOT" && DEVB_REBUILD=1 timeout 1800 ./selfhost/devbuild.sh pxi ) >>"$W/restore.log" 2>&1
    cp -f /tmp/pxidev "$W/pxi.restored" 2>/dev/null || true
    chk "还原后重编 pxi 与负控前基线逐字节一致" "cmp -s '$W/pxi.restored' '$W/pxi.pre'"
fi

echo "── [8] 覆盖边界（如实登记）"
echo "     · 只覆盖 tuple / result 两个接收者（+ list.pop 的同族抽查）；str/list/dict 由 M226 覆盖。"
echo "     · mutex.with / rwlock.with_read / rwlock.with_write / list.index 的「类型.方法」措辞"
echo "       **有意保留** —— 它们刻意点名「哪个类型的方法」，且前三个属 MINI_SUBSET 设计件"
echo "       （解释轨不支持）⇒ 改措辞的收益低于破坏面。已在 [1] 的普查里计数。"
echo "     · unwrap / unwrap_err 的**值错误**文案（R1004）不在本门口径内："
echo "       值错误族（如 R1008 字典没有键）本就不带「方法」前缀。"
echo "     · 通道与行:列前缀**不判**（已登记缺陷 186，按 M186 归一化）。"
echo "     · 未覆盖：result 作为**函数参数/返回值**位置的类型面（属类型系统，不在方法面）。"
echo "     ⚠️ 本段一律用单引号 + 不用反引号 —— 双引号里的反引号会触发命令替换（本仓已踩 6 次）。"

echo "M229-VERIFY-OK pass=$pass fail=$fail"
[ "$fail" = 0 ] || { echo "M229-VERIFY-FAIL pass=$pass fail=$fail"; exit 1; }
