#!/usr/bin/env bash
# ============================================================
# M172 门 · 运行期诊断的**通道**与**措辞**统一（缺陷 186）
# ------------------------------------------------------------
# 病灶（修前实测，三轨对同一份源码写出四种行为）：
#   `def f(): return zzz + 1` —— 解释轨把诊断写 **stdout**
#     （`运行时错误: 错误 [R1001] 2:12: 未定义变量: 'zzz'`），
#     编译轨写 **stderr**（`运行时错误 [f 行2]: 未定义变量: zzz`）；
#   `def main(): return Err("boom")` —— 解释轨写 **stdout**、编译轨写 **stderr**。
#   ⇒ ① 诊断混进**产物通道**（与 Issue 45/51、engine_parity 判据 B「失败时 stdout 必须为空」
#        同轴相悖）；② 三轨措辞不同（一个有 `R1001` + 引号，一个都没有）⇒ 自动化无法按码判。
#
# 判据（四层 + 负控 2 道）：
#   [1] R1001 三轨：rc≠0 · **stdout 恰为程序输出 `out-1`**（诊断不得混进产物通道）·
#       **stderr** 含 `R1001` 与 `未定义变量: 'zzz'`（同码 + 同引号）
#   [2] main→Err 三轨：rc≠0 · stdout 空 · stderr 含 `错误: boom`
#   [3] 正常程序三轨：rc=0 · **stderr 为 0 字节** · stdout 逐字节一致
#   [4] 真·合并流（`> f 2>&1`）顺序：首行必须是 `out-1`
#       （诊断走 stderr 后若不先 `fflush(stdout)`，诊断会跑到程序输出**前面**）
#   [5] 负控 2 道（各自独立判红 + 逐字节还原复绿）：
#       NC-A 通道：解释轨 print_err 改回 print   → [1] 的 stdout 判据变红
#       NC-B 措辞：i_err.px 的 `未定义变量: '` 去掉引号 → [1] 的消息体判据变红
#
# 已知边界（如实登记，不假装已统一）：**位置前缀**保留各轨最优信息 ——
#   解释轨 `错误 [R1001] 行:列:`（AST 行列），编译轨 `[函数 行N]:`（px_srcline 追踪，
#   runtime 无列号）⇒ 判据只断言「同通道 + 同错误码 + 同消息体 + rc」，**不按整行对拍**。
# ============================================================
set -u
cd "$(dirname "$0")/../.." || exit 1
ROOT=$PWD
D=examples/m172_diag_channel
PX="$ROOT/tools/px"
INTERP_SRC="$ROOT/selfhost/interp.px"
IERR_SRC="$ROOT/selfhost/i_err.px"
DEV_PXI="$ROOT/selfhost/build/interp"
BAK_I=/tmp/m172_interp.bak
BAK_E=/tmp/m172_ierr.bak
W=/tmp/m172_gate
rm -rf "$W"; mkdir -p "$W"
NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
pass=0; fail=0
chk() { if eval "$2"; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi }

snapshot() { cp -f "$INTERP_SRC" "$BAK_I"; cp -f "$IERR_SRC" "$BAK_E"; }
restore_all() {
    [ -f "$BAK_I" ] && cp -f "$BAK_I" "$INTERP_SRC"
    [ -f "$BAK_E" ] && cp -f "$BAK_E" "$IERR_SRC"
}
trap 'restore_all; rm -f "$BAK_I" "$BAK_E"' EXIT
build_dev_pxi() { (cd "$ROOT" && "$PX" build selfhost/interp.px >/dev/null 2>&1) && [ -x "$DEV_PXI" ]; }

echo "=== [1] R1001：通道 + 错误码 + 消息体（三轨）"
bash "$D/tracks.sh" "$D/undef.px" "$W/u" || echo "  (tracks.sh rc=$?)"
printf 'out-1\n' > "$W/expect_undef.out"
for t in interp vm c; do
    chk "undef/$t rc≠0" "[ \"\$(cat $W/u/undef.$t.rc)\" != 0 ]"
    chk "undef/$t stdout 只有程序输出" "cmp -s $W/u/undef.$t.out $W/expect_undef.out"
    chk "undef/$t stderr 含 R1001" "grep -q 'R1001' $W/u/undef.$t.err"
    chk "undef/$t stderr 含 未定义变量: 'zzz'" "grep -q \"未定义变量: 'zzz'\" $W/u/undef.$t.err"
    chk "undef/$t stdout 无诊断" "! grep -q '未定义变量' $W/u/undef.$t.out"
done

echo "=== [2] main() 返回 Err：通道统一"
bash "$D/tracks.sh" "$D/main_err.px" "$W/m" || echo "  (tracks.sh rc=$?)"
for t in interp vm c; do
    chk "main_err/$t rc≠0" "[ \"\$(cat $W/m/main_err.$t.rc)\" != 0 ]"
    chk "main_err/$t stdout 空" "[ ! -s $W/m/main_err.$t.out ]"
    chk "main_err/$t stderr 含 错误: boom" "grep -q '错误: boom' $W/m/main_err.$t.err"
done

echo "=== [3] 正常程序：stderr 必须一个字节都没有"
bash "$D/tracks.sh" "$D/ok.px" "$W/k" || echo "  (tracks.sh rc=$?)"
for t in interp vm c; do
    chk "ok/$t rc=0" "[ \"\$(cat $W/k/ok.$t.rc)\" = 0 ]"
    chk "ok/$t stderr 为 0 字节" "[ ! -s $W/k/ok.$t.err ]"
done
chk "ok 三轨 stdout 逐字节一致" "cmp -s $W/k/ok.interp.out $W/k/ok.vm.out && cmp -s $W/k/ok.interp.out $W/k/ok.c.out"

echo "=== [4] 真·合并流（> f 2>&1）顺序：程序输出在前"
for t in interp vm c; do
    chk "undef/$t 合并流首行 == out-1" "[ \"\$(head -1 $W/u/undef.$t.merged)\" = 'out-1' ]"
    chk "undef/$t 合并流含诊断" "grep -q 'R1001' $W/u/undef.$t.merged"
done

if [ $NEG -eq 1 ]; then
echo "=== [5] 负控（各自独立判红）"
echo "--- NC-A 通道：解释轨 176 行 print_err 改回 print → [1] stdout 判据应变红"
snapshot
python3 - "$INTERP_SRC" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = 'print_err("运行时错误: " + i_err_display(e))'
assert s.count(old) == 1, s.count(old)
open(p, 'w', encoding='utf-8').write(s.replace(old, 'print("运行时错误: " + i_err_display(e))'))
PYEOF
if build_dev_pxi; then
    "$DEV_PXI" "$D/undef.px" > "$W/negA.out" 2> "$W/negA.err"; rca=$?
    chk "NC-A 判红（诊断回到 stdout）" "[ $rca -ne 0 ] && grep -q '未定义变量' $W/negA.out"
else
    chk "NC-A 重编 dev pxi" "false"
fi
restore_all; build_dev_pxi >/dev/null 2>&1
"$DEV_PXI" "$D/undef.px" > "$W/posA.out" 2> "$W/posA.err" || true
chk "NC-A 还原后复绿" "! grep -q '未定义变量' $W/posA.out && grep -q 'R1001' $W/posA.err"

echo "--- NC-B 措辞：i_err.px 的「未定义变量: '」去掉引号 → [1] 消息体判据应变红"
restore_all
python3 - "$IERR_SRC" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p, encoding='utf-8').read()
old = 'return i_err("R1001", "未定义变量: \'" + name + "\'", pos)'
assert s.count(old) == 1, s.count(old)
new = 'return i_err("R1001", "未定义变量: " + name, pos)'
open(p, 'w', encoding='utf-8').write(s.replace(old, new))
PYEOF
if build_dev_pxi; then
    "$DEV_PXI" "$D/undef.px" > "$W/negB.out" 2> "$W/negB.err" || true
    chk "NC-B 判红（消息体不再带引号）" "grep -q 'R1001' $W/negB.err && ! grep -q \"未定义变量: 'zzz'\" $W/negB.err"
else
    chk "NC-B 重编 dev pxi" "false"
fi
restore_all; build_dev_pxi >/dev/null 2>&1
"$DEV_PXI" "$D/undef.px" > "$W/posB.out" 2> "$W/posB.err" || true
chk "NC-B 还原后复绿" "grep -q \"未定义变量: 'zzz'\" $W/posB.err"
fi

echo
echo "结果: $pass 通过 / $fail 失败"
if [ $fail -eq 0 ]; then echo "M172-VERIFY-OK"; fi
[ $fail -eq 0 ] || exit 1
