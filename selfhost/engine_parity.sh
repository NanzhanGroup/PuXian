#!/usr/bin/env bash
# ============================================================
# 引擎一致性门（engine parity）—— Issue 51
# ------------------------------------------------------------
# 契约（三断言）：
#   A. 默认轨（tools/px，用户面引擎）对负例必须失败：rc != 0
#   B. 默认轨失败时 stdout 必须为空（Issue 45：诊断不得混进产物通道）
#   C. 默认轨 stderr 必须含 golden 诊断正文；基准轨合并流首行 == golden
#      （基准轨契约与 diffcheck.sh --errors 一致，不改既有口径）
#
# 退出码：0 = 全部门通过；1 = 有分歧
#
# 为什么要有这条门（Issue 51）：
#   CI 的 `diffcheck.sh --errors` 只跑**基准轨** bootstrap/pxc；用户面默认轨
#   （tools/px → bootstrap/pxc_vm）零覆盖。Issue 46 修复期实测：入库 pxc_vm
#   未重烘时，默认轨对 codegen_b01 静默 rc=0 / stdout 非空 / stderr 空，
#   而 CI 全绿。⇒ 「CI 绿 ≠ 用户拿到修复」。
#
# 已知缺口显式登记（不得沉默放行；缺口修好后本门会主动报 XPASS 要求摘除）：
#   codegen_b04_ffi_args —— VM 轨尚无 ffi（E3004 未覆盖）
#
# 用法：
#   ./engine_parity.sh                     # 全部 codegen_b* 负例
#   ./engine_parity.sh codegen_b01_immutable
# ============================================================
set -u
cd "$(dirname "$0")/.."
ROOT=$(pwd)
PXC="$ROOT/bootstrap/pxc"
PXDEF="$ROOT/tools/px"
GOLD="$ROOT/selfhost/golden/errors"
WORK=/tmp/px_engine_parity
mkdir -p "$WORK"

xfail_reason() {
    case "$1" in
        codegen_b04_ffi_args) echo "VM 轨无 ffi，E3004 未覆盖" ;;
        *) return 1 ;;
    esac
}

pass=0; bad=0; xfail=0; xpass=0; fd_out_stdout=0; fd_out_stderr=0

[ -x "$PXC" ] || { echo "❌ 缺少基准轨二进制 $PXC" >&2; exit 1; }
[ -x "$PXDEF" ] || { echo "❌ 缺少默认轨入口 $PXDEF" >&2; exit 1; }

echo "── 引擎一致性门：默认轨（用户面）vs 基准轨 ──"
for f in selfhost/cases_bad/codegen_b*.px; do
    [ -e "$f" ] || continue
    base=$(basename "$f" .px)
    if [ $# -gt 0 ] && [ "$base" != "$1" ]; then continue; fi
    gold=$(cat "$GOLD/codegen.$base.txt" 2>/dev/null)
    if [ -z "$gold" ]; then
        echo "    ⚠️  $base 无 golden（$GOLD/codegen.$base.txt），跳过"
        continue
    fi

    # 基准轨（与 diffcheck --errors 同契约：合并流首行）
    "$PXC" build "$f" >"$WORK/c.out" 2>"$WORK/c.err"; crc=$?
    cfirst=$(cat "$WORK/c.out" "$WORK/c.err" | head -1)
    if grep -qF "$gold" "$WORK/c.out"; then fd_out_stdout=$((fd_out_stdout+1)); else fd_out_stderr=$((fd_out_stderr+1)); fi

    # 默认轨（用户面）
    "$PXDEF" build "$f" >"$WORK/d.out" 2>"$WORK/d.err"; drc=$?
    dbytes=$(wc -c <"$WORK/d.out")

    why=""
    [ "$crc" -ne 0 ] || why="基准轨 rc=0（未报错）"
    [ "$cfirst" = "$gold" ] || why="${why:+$why；}基准轨首行 ≠ golden：[$cfirst]"

    if [ "$drc" -eq 0 ]; then
        reason=$(xfail_reason "$base") || reason=""
        if [ -n "$reason" ]; then
            echo "    ⏸  $base 已知缺口（xfail）：$reason"
            xfail=$((xfail+1))
            continue
        fi
        why="${why:+$why；}默认轨静默通过（rc=0 / stdout ${dbytes}B）"
    else
        if [ -n "$(xfail_reason "$base")" ]; then
            echo "    ⚠️  $base XPASS：登记为已知缺口，但默认轨已能报错 —— 请摘掉 xfail 登记"
            xpass=$((xpass+1)); bad=$((bad+1))
            continue
        fi
        [ "$dbytes" -eq 0 ] || why="${why:+$why；}默认轨 stdout 非空（${dbytes}B，诊断混进产物通道）"
        grep -qF "$gold" "$WORK/d.err" || why="${why:+$why；}默认轨 stderr 未含 golden 正文"
    fi

    if [ -z "$why" ]; then
        echo "    ✅ $base"
        pass=$((pass+1))
    else
        echo "    ❌ $base"
        echo "        $why"
        echo "        golden   : $gold"
        echo "        基准轨   : rc=$crc [$cfirst]"
        echo "        默认轨   : rc=$drc stdout=${dbytes}B stderr=[$(head -2 "$WORK/d.err" | tr '\n' '|')]"
        bad=$((bad+1))
    fi
done

echo "── 小计：通过 $pass · 失败 $bad · xfail $xfail · xpass $xpass ──"
echo "── 基准轨诊断出口：stdout $fd_out_stdout 例 / stderr $fd_out_stderr 例（仅供参考，非判据）──"
if [ "$bad" -gt 0 ]; then
    echo "❌ 引擎一致性门失败"
    echo "   提示 1：若默认轨静默通过，先确认入库二进制是否已按当前源码重烘："
    echo "           cp selfhost/build/compiler_vm bootstrap/pxc_vm"
    echo "   提示 2：若基准轨首行不符，golden/errors 与编译器源码可能不同批更新。"
    exit 1
fi
echo "✅ 引擎一致性门通过"
exit 0
