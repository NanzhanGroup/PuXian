#!/bin/bash
# M237 门 · 字段访问 / 构造 / 变体族 —— 三轨全量对拍
#
# 主题：M226（方法面）· M227（同名两门）· M228（三个门）· M229（tuple/result）·
#   M230（索引/切片）· M231（运算符矩阵）逐个推过去 —— 而 **`x.f` / `x.f = v` / `T(...)` /
#   `E.V` / `E("V")` 这一族（字段访问 · 构造 · 变体）从来没有清单级 / 全量度量**。
#
# 修前基线（三轨实测）：组 A 42 例 **30 OK / 12 FORK**；组 B 8 例**无一例三轨同命**。
#   八个编号：
#     365 C 轨 `Color("Red")` 发**野指针**（把 string 对象当 enum 实例读）⇒ **SIGSEGV core dumped**
#     366 C 轨**诊断文本逃逸到代码生成**（4 处）⇒ `_v1 = 结构体 Point 需要 2 个字段，给出 1;`
#         = 非法 C ⇒ 用户看到 gcc 报错而非 R2001
#     367 解释轨 `Point(1)` **静默构造**（缺字段填 null）⇄ 编译两轨报错
#     368 `Color.Nope` C 轨**静默造假值**；`Color("Nope")` VM 轨**静默造假值**
#     369 枚举值取字段：解释轨值级 ⇄ 编译两轨类型级（措辞分叉）
#     370 `d.a = v`（dict 字段写）解释轨 OK ⇄ 编译两轨 R1002（读支持写不支持）
#     371 `E(V)` / `E("V")` 三轨三种错误（含 365 的崩溃）
#     364 spec §3.8/§16.3 承诺 data enum，parser 只给「期望 ')'，实际得到 :」（指不到真因）
#
# 定稿一条真相（三轨同码同文，参考 = 解释轨单一 env）：
#   `T(...)` arity 必须恰等 ⇒ R2001 · `E("V")`/`E(V)` = 按变体名构造（保留能力）+ 存在性校验 ⇒ R1008
#   · `E.V` 不存在 ⇒ R1008 · 取字段：struct R1008 / enum 值级 R1007 / 不可写目标 R1002 带类型
#   · dict 字段读支持 ⇒ **写也支持**
#
# 判据
#   [1] 静态：新形态在位 + 旧形态清零 + 负控锚点自证 + 规模锚点
#   [2] 生成探针 + 构建三轨驱动器
#   [3] 组 A 对拍（42 例 × 3 轨）：跨轨一致
#   [4] 组 B/C 单文件对拍（9 例 × 3 轨）：三轨构建+运行一致
#   [5] MODEL.tsv 双向核对（漏登记 / 过期都判红）
#   [6][7][8] 负控 A/B/C（各自独立判红 + 源逐字节还原）
#   [9] 覆盖边界（如实登记）
#
# CI 用 `--neg-skip`（负控各要重编一次编译器）。
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D="$ROOT/examples/m237_field_enum"   # 绝对路径（门中 cd 到 $W/a 后相对路径会失效）
W=/tmp/m237_gate
rm -rf "$W"; mkdir -p "$W/probes" "$W/a" "$W/probe_b"
export M237_NEG_W="$W/snap"

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
PXC="${M237_PXC:-$ROOT/bootstrap/pxc}"
PXCV="${M237_PXCV:-$ROOT/bootstrap/pxc_vm}"
PXI="${M237_PXI:-$ROOT/bootstrap/pxi}"
NEGCTL() { python3 "$D/negctl.py" --root "$ROOT" --snap "$W/snap" "$@"; }
has() { grep -qF "$2" "$ROOT/$1"; }
cnt() { grep -cF "$2" "$ROOT/$1" || true; }
norm() { sed -E 's/^运行时错误 \[[^]]*\]: //; s/^运行时错误: //; s/^错误 \[R([0-9]+)\] [0-9]+:[0-9]+: /R\1: /; s/^R([0-9]+): /R\1: /' "$1"; }

echo "── [1] 静态判据"
chk "runtime：枚举构造统一入口 px_enum_checked 在位" "has runtime/runtime.c 'px_enum_checked'"
chk "runtime：实参个数错入口 px_enum_arity 在位" "has runtime/runtime.c 'px_enum_arity'"
chk "runtime：px_field 有枚举分支（值级措辞）" "has runtime/runtime.c 'R1007: 枚举值 %s.%s 没有字段'"
chk "runtime：px_field_set 有 dict 分支（读写对称）" "has runtime/runtime.c 'px_dict_set(obj, name, val);'"
chk "vm.h：PXOP_RAISE 在位" "has runtime/vm.h 'PXOP_RAISE   70'"
chk "vm.c：RAISE 实现在位" "has runtime/vm.c 'case PXOP_RAISE'"
V_OLD_STRUCT=$(cnt selfhost/cg_expr.px 'return "结构体 " + ')
V_OLD_ENUM=$(cnt selfhost/cg_expr.px '构造需要一个变体名"')
# ⚠️ 只查**代码行**：注释里引用了修前形态（M223 立过「判据只查代码行」，首版重犯 ⇒ 假红）
V_OLD_PTR=$(grep -v '^[[:space:]]*#' "$ROOT/selfhost/cg_expr.px" | grep -cF '.as.enum_inst.variant)' || true)
chk "C 轨：诊断逃逸已清零（struct 实测 $V_OLD_STRUCT）" "[ '$V_OLD_STRUCT' = 0 ]"
chk "C 轨：枚举构造旧文案已清零（实测 $V_OLD_ENUM）" "[ '$V_OLD_ENUM' = 0 ]"
chk "C 轨：野指针形态已清零（实测 $V_OLD_PTR）" "[ '$V_OLD_PTR' = 0 ]"
chk "C 轨：cg_err_expr 在位（≥5 处）" "[ $(cnt selfhost/cg_expr.px 'cg_err_expr') -ge 5 ]"
chk "C 轨：cg_enum_ctor 在位（≥3 处）" "[ $(cnt selfhost/cg_expr.px 'cg_enum_ctor') -ge 3 ]"
V_PANIC_S=$(cnt selfhost/bc_emit.px 'panic("结构体 "')
V_PANIC_E=$(cnt selfhost/bc_emit.px 'panic("bc_emit enum')
chk "VM 轨：构造期 panic 已清零（struct=$V_PANIC_S enum=$V_PANIC_E）" "[ '$V_PANIC_S' = 0 ] && [ '$V_PANIC_E' = 0 ]"
N_RAISE=$(cnt selfhost/bc_emit.px '"RAISE"')
chk "VM 轨：RAISE 发射在位（≥4 处，实测 $N_RAISE）" "[ '$N_RAISE' -ge 4 ]"
chk "解释轨：i_enum_ctor 在位" "has selfhost/iexpr.px 'def i_enum_ctor'"
chk "解释轨：枚举构造拦在 args 求值之前" "has selfhost/iexpr.px 'if g_enums.has(cname):'"
chk "解释轨：struct arity 响亮化（R2001）" "has selfhost/icall.px 'i_r2001'"
chk "解释轨：字段赋值带类型（对接编译轨口径）" "has selfhost/istmt.px '不支持字段赋值'"
chk "解释轨：struct 新字段不再静默（纯赋值分支）" "has selfhost/istmt.px '结构体没有字段'"
chk "parser：data enum 给出可执行指引" "has selfhost/parser.px 'data enum 计划中'"
chk "负控锚点自证（3 处唯一命中）" "NEGCTL --selftest >/dev/null"
N_ENTRY=$(grep -c '' "$D/cases.tsv" 2>/dev/null || echo 0)
chk "规模锚点：入库 cases.tsv ≥ 50 行（实测 $N_ENTRY）" "[ '$N_ENTRY' -ge 50 ]"

echo "── [2] 生成探针 + 构建三轨驱动器"
python3 "$D/gen_probes.py" --gen "$W/probes" >"$W/gen.log" 2>&1 && chk "探针生成" true || { chk "探针生成" false; tail -5 "$W/gen.log"; }
python3 "$D/gen_probes.py" --model "$W/MODEL.tsv" >"$W/model.log" 2>&1 && chk "MODEL 生成（定稿口径表）" true || { chk "MODEL 生成" false; tail -3 "$W/model.log"; }
cmp -s "$D/MODEL.tsv" "$W/MODEL.tsv" && chk "MODEL.tsv 与生成器逐字节一致（防漂移）" true || chk "MODEL.tsv 与生成器逐字节一致（防漂移）" false
cmp -s "$D/cases.tsv" "$W/probes/cases.tsv" && chk "cases.tsv 与生成器逐字节一致（防漂移）" true || chk "cases.tsv 与生成器逐字节一致（防漂移）" false
cd "$W/a" || exit 9
cp "$W/probes/drv.px" .
PX_PXC_BIN="$PXC" PX_BUILD_ENGINE=c timeout 1200 "$ROOT/tools/px" build --c drv.px > bc.log 2>&1; RBC=$?
[ $RBC -eq 0 ] && [ -x build/drv ] && cp build/drv drv_c
PXC_VM_BIN="$PXCV" timeout 1200 "$ROOT/tools/px" build drv.px > bv.log 2>&1; RBV=$?
[ $RBV -eq 0 ] && [ -x build/drv ] && cp build/drv drv_vm
chk "组 A 驱动器构建（C=$RBC VM=$RBV）" "[ $RBC -eq 0 ] && [ $RBV -eq 0 ]"
[ $RBC -ne 0 ] && tail -6 bc.log
[ $RBV -ne 0 ] && tail -6 bv.log

echo "── [3] 组 A 对拍（42 例 × 3 轨）"
nf=0; nok=0
: > "$W/a_raw.tsv"
while IFS=$'\t' read -r cid recv op shape kind exp grp; do
  [ "$grp" = "A" ] || continue
  M237_CASE="$cid" timeout 60 "$PXI" drv.px > o.i 2>&1; ri=$?
  if [ -x drv_c ]; then M237_CASE="$cid" timeout 60 ./drv_c > o.c 2>&1; rc=$?; else rc=998; echo BUILDFAIL > o.c; fi
  if [ -x drv_vm ]; then M237_CASE="$cid" timeout 60 ./drv_vm > o.v 2>&1; rv=$?; else rv=998; echo BUILDFAIL > o.v; fi
  oi=$(norm o.i|tr '\n' '~'); oc=$(norm o.c|tr '\n' '~'); ov=$(norm o.v|tr '\n' '~')
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$cid" "$op" "$recv" "$shape" "$ri" "$oi" "$rc" "$oc" "$rv" >> "$W/a_raw.tsv"
  if [ "$ri" = "$rc" ] && [ "$ri" = "$rv" ] && [ "$oi" = "$oc" ] && [ "$oi" = "$ov" ]; then
    nok=$((nok+1))
  else
    nf=$((nf+1))
    printf '    FORK %-20s i(%s)=%s c(%s)=%s v(%s)=%s\n' "$cid" "$ri" "$oi" "$rc" "$oc" "$rv" "$ov"
  fi
done < "$W/probes/cases.tsv"
chk "组 A 三轨一致（$nok 例通过 / $nf 例分叉）" "[ $nf -eq 0 ]"

echo "── [4] 组 B/C 单文件对拍（9 例 × 3 轨）"
bash "$D/probe_b.sh" "$W/probes" "$W/probe_b" "$PXC" "$PXCV" "$PXI" >"$W/probe_b.log" 2>&1
NFB=$?
sed 's/^/    /' "$W/probe_b.log"
chk "组 B/C 三轨一致（分叉 $NFB 例）" "[ $NFB -eq 0 ]"

echo "── [5] MODEL.tsv 双向核对"
python3 - "$W/MODEL.tsv" "$W/a_raw.tsv" <<'PY' > "$W/model_check.log" 2>&1
import re, sys
model, raw = sys.argv[1], sys.argv[2]
def code_of(out):
    m = re.search(r"R[0-9]{4}", out)
    return (m.group(0) if m else "OK")
want = {}
for ln in open(model).read().splitlines()[1:]:
    f = ln.split('\t')
    if len(f) >= 5:
        want[(f[0], f[1], f[2])] = (f[3], f[4])
got = {}
for ln in open(raw).read().splitlines():
    f = ln.split('\t')
    if len(f) < 9:
        continue
    cid, op, recv, shape, ri, oi = f[0], f[1], f[2], f[3], f[4], f[5]
    beh = "OK" if ri == "0" else "ERR"
    got[(op, recv, shape)] = (beh, code_of(oi))
missing = [k for k in got if k not in want]
stale = [k for k in want if k not in got]
mism = [(k, got[k], want[k]) for k in got if k in want and got[k] != want[k]]
print("表 %d 条 · 实测 %d 条 · 漏登记 %d · 过期 %d · 不符 %d" % (len(want), len(got), len(missing), len(stale), len(mism)))
for k in missing: print("  漏登记(实测有表里无): %s" % (k,))
for k in stale: print("  过期(表里有实测无): %s" % (k,))
for k, g, w in mism: print("  不符: %s 实测=%s 表=%s" % (k, g, w))
sys.exit(1 if (missing or stale or mism) else 0)
PY
MODEL_RC=$?
sed 's/^/    /' "$W/model_check.log"
chk "MODEL.tsv 双向核对（漏登记/过期/不符 皆判红）" "[ $MODEL_RC -eq 0 ]"

echo "── [6-8] 负控（各自独立判红 + 源逐字节还原）"
if [ "$NEG" = "1" ]; then
  NEGCTL --snapshot >/dev/null
  # ── A：撤回 C 轨枚举构造/变体的校验 ⇒ `Color("Nope")` 必须静默（判红）
  SHA_PXC_BEFORE=$(sha256sum /tmp/pxcdev 2>/dev/null | cut -c1-16)
  NEGCTL --neg-a >"$W/nega.log" 2>&1
  bash "$ROOT/selfhost/devbuild.sh" pxc >"$W/nega_build.log" 2>&1
  SHA_PXC_AFTER=$(sha256sum /tmp/pxcdev 2>/dev/null | cut -c1-16)
  chk "负控 A：dev 件确已重建（sha $SHA_PXC_BEFORE → $SHA_PXC_AFTER）" "[ \"$SHA_PXC_BEFORE\" != \"$SHA_PXC_AFTER\" ]"
  bash "$D/probe_b.sh" "$W/probes" "$W/nega_b" /tmp/pxcdev "$PXCV" "$PXI" >"$W/nega_run.log" 2>&1
  RC_A=$?
  chk "负控 A（撤 C 轨校验）⇒ 必须判红（分叉 $RC_A 例）" "[ $RC_A -ne 0 ]"
  grep -E '^  FORK' "$W/nega_run.log" | head -3 | sed 's/^/       /'
  #   ── C（判据自伤）：把 probe_b 的比对改成恒真 ⇒ A 的红**必须消失**
  cp -f "$D/probe_b.sh" "$W/probe_b_bak.sh"
  # ⚠️ 判据行在 [4] 段被改过（加了 first/非空判据）⇒ sed 模式必须跟着改，否则
  #   「自伤」没生效 ⇒ 负控 C 假红（首版实战：sed 模式停留在旧文本）。
  python3 - "$D/probe_b.sh" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p).read()
old = 'if [ "$nl" = "1" ] && [ "$nn" = "3" ] && [ -n "$first" ] && [ "$first" != "BUILDFAIL" ]; then'
assert s.count(old) == 1, s.count(old)
open(p, 'w').write(s.replace(old, 'if true; then'))
PYEOF
  bash "$D/probe_b.sh" "$W/probes" "$W/negc_b" /tmp/pxcdev "$PXCV" "$PXI" >"$W/negc_run.log" 2>&1
  RC_C=$?
  cp -f "$W/probe_b_bak.sh" "$D/probe_b.sh"
  cmp -s "$D/probe_b.sh" "$W/probe_b_bak.sh" && chk "负控 C 后 probe_b.sh 逐字节还原" true || chk "负控 C 后 probe_b.sh 逐字节还原" false
  chk "负控 C（判据自伤）⇒ A 的红消失（证明红来自比对）" "[ $RC_C -eq 0 ]"
  NEGCTL --restore >"$W/restore1.log" 2>&1 && chk "负控 A 源逐字节还原" true || { chk "负控 A 源逐字节还原" false; cat "$W/restore1.log"; }
  # ── B：撤回解释轨枚举构造拦截 ⇒ 解释轨报 R1004（判红）
  SHA_PXI_BEFORE=$(sha256sum /tmp/pxidev 2>/dev/null | cut -c1-16)
  NEGCTL --neg-b >"$W/negb.log" 2>&1
  bash "$ROOT/selfhost/devbuild.sh" pxi >"$W/negb_build.log" 2>&1
  SHA_PXI_AFTER=$(sha256sum /tmp/pxidev 2>/dev/null | cut -c1-16)
  chk "负控 B：dev 件确已重建（sha $SHA_PXI_BEFORE → $SHA_PXI_AFTER）" "[ \"$SHA_PXI_BEFORE\" != \"$SHA_PXI_AFTER\" ]"
  bash "$D/probe_b.sh" "$W/probes" "$W/negb_b" "$PXC" "$PXCV" /tmp/pxidev >"$W/negb_run.log" 2>&1
  RC_B=$?
  chk "负控 B（撤解释轨拦截）⇒ 必须判红（分叉 $RC_B 例）" "[ $RC_B -ne 0 ]"
  grep -E '^  FORK' "$W/negb_run.log" | head -3 | sed 's/^/       /'
  NEGCTL --restore >"$W/restore2.log" 2>&1 && chk "负控 B 源逐字节还原" true || { chk "负控 B 源逐字节还原" false; cat "$W/restore2.log"; }
  # 还原后重编回基线件，避免污染后续门
  bash "$ROOT/selfhost/devbuild.sh" pxc pxi >"$W/rebuild.log" 2>&1
  chk "负控后 dev 件重建回基线" "[ -x /tmp/pxcdev ] && [ -x /tmp/pxidev ]"
else
  echo "  SKIP 负控（--neg-skip）"
fi

echo "── [9] 覆盖边界（如实登记）"
cat <<'EOF'
  · 组 B/C 的**构建 rc** 参与比对（`st.txt` 第 2 字段）—— 本轮定稿是「全部统一到运行期」
    ⇒ 三轨的构建 rc 都应是 0、运行 rc 都应是 1，故可比。
  · data enum 的**完整特性**（命名参数构造 + match 解构）**未实现**，本轮只做
    「parser 给可执行指引 + spec §3.8/§16.3 标注状态」⇒ `Circle(radius: float)` 仍不可编译。
  · `E.V(args)`（变体携带数据）同理不在面内。
  · 结构体**嵌套赋值回读**（`p.inner.x = 1`）只覆盖一层；深层链未展开。
  · `?.` 的**接收者求值次数**由 M220（缺陷 319）单独覆盖，本门只覆盖 `?.` 的字段存在性。
  · `result` 接收者的字段访问只取 `Ok(1)` 一例（M229 已覆盖其方法面）。
EOF

echo
echo "══ M237 门汇总：通过 $pass / 失败 $fail ══"
[ $fail -eq 0 ] && echo "M237-VERIFY-OK" || echo "M237-VERIFY-FAIL"
exit $fail
