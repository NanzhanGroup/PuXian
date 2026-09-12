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
# 两个阶段（M113-S0 / Issue 57 起）：
#   阶段 1 负例：cases_bad/codegen_b*.px → 默认轨必须报错，且诊断不得混进产物通道
#   阶段 2 正例：cases_ok/*.px → **两轨都必须编译通过**，产物运行结果对拍 expected
#     （正例门补的正是「不该报的别报」这一类：Issue 57 的 `_ = expr` 误报 E3002
#       发生时，阶段 1 与 CI 其余各门全绿）
#
# 已知缺口显式登记（不得沉默放行；缺口修好后本门会主动报 XPASS 要求摘除）：
#   当前为空 —— M112-S2（Issue 52）已闭合 codegen_b04_ffi_args：VM 轨
#   bc_emit_program 补齐 cg_ffi（ExternDef → 声明表），E3004 在默认轨生效。
#   新增缺口在此登记，不要用「跳过」代替登记。
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
    # 登记格式（新增缺口时照写；返回 0 表示已登记）：
    #   codegen_bNN_xxx) echo "原因" ;;
    case "$1" in
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

# ============================================================
# 阶段 2（M113-S0 / Issue 57）：**正例**两轨一致性
# ------------------------------------------------------------
# 阶段 1 只保证「该报的报」；对「**不该报的别报**」零覆盖 —— Issue 57 正是这一类：
#   默认轨（用户面）把 `_ = expr`（弃元）判成「对不可变变量 '_' 赋值」E3002，
#   而当时**所有门全绿**（CI 无任何正例门；`bootstrap/pxc_vm` 也没被 CI 校验）。
# 契约（每例）：
#   A. 基准轨（C 轨 bootstrap/pxc）rc=0 且 stdout 非空
#   B. 默认轨（用户面 tools/px）rc=0、stdout 非空、**stderr 必须为空**
#   C. 若存在 `<case>.expected` → 实际运行默认轨产物，stdout 逐字节相符
#      （弃元右侧**仍必须求值**：`_ = f()` 不是把 f() 删掉）
# ============================================================
echo "── 阶段 2：正例（两轨都必须编译通过 + 产物运行对拍）──"
ok_pass=0; ok_bad=0
mkdir -p "$WORK/ok"
for f in selfhost/cases_ok/*.px; do
    [ -e "$f" ] || continue
    base=$(basename "$f" .px)
    exp_file="$f.expected"
    # 在 $WORK 里编译：`tools/px build` 会在源文件旁生成 build/ 目录，
    # 不能污染仓库（正例跑在 CI 上，工作区脏会连带影响后续 git diff 类门）。
    src="$WORK/ok/$base.px"
    cp -a "$f" "$src"

    "$PXC" build "$src" >"$WORK/ok_c.out" 2>"$WORK/ok_c.err"; crc=$?
    "$PXDEF" build "$src" >"$WORK/ok_d.out" 2>"$WORK/ok_d.err"; drc=$?
    cbytes=$(wc -c <"$WORK/ok_c.out"); dbytes=$(wc -c <"$WORK/ok_d.out")

    why=""
    [ "$crc" -eq 0 ] || why="基准轨 rc=$crc（$(head -1 "$WORK/ok_c.err" | tr -d '\n')）"
    [ "$cbytes" -gt 0 ] || why="${why:+$why；}基准轨无产物"
    [ "$drc" -eq 0 ] || why="${why:+$why；}默认轨 rc=$drc（$(head -1 "$WORK/ok_d.err" | tr -d '\n')）"
    [ "$dbytes" -gt 0 ] || why="${why:+$why；}默认轨无产物"
    [ ! -s "$WORK/ok_d.err" ] || why="${why:+$why；}默认轨 stderr 非空（$(head -1 "$WORK/ok_d.err" | tr -d '\n')）"

    if [ -z "$why" ] && [ -f "$exp_file" ]; then
        bin=$(cat "$WORK/ok_d.out" "$WORK/ok_d.err" \
              | sed -n 's/^编译成功: \([^ ]*\).*/\1/p' | head -1)
        if [ -n "$bin" ] && [ -x "$bin" ]; then
            "$bin" >"$WORK/ok_run.out" 2>"$WORK/ok_run.err"; rrc=$?
            if [ "$rrc" -ne 0 ]; then
                why="产物运行退出码 $rrc（$(head -1 "$WORK/ok_run.err" | tr -d '\n')）"
            elif ! diff -q "$exp_file" "$WORK/ok_run.out" >/dev/null 2>&1; then
                why="产物运行输出 ≠ expected：$(tr '\n' '|' <"$WORK/ok_run.out")"
            fi
        else
            why="未从默认轨输出解析到可执行产物路径"
        fi
    fi

    if [ -z "$why" ]; then
        echo "    ✅ $base"
        ok_pass=$((ok_pass+1))
    else
        echo "    ❌ $base"
        echo "        $why"
        ok_bad=$((ok_bad+1))
    fi
done

echo "── 小计：负例 通过 $pass · 失败 $bad · xfail $xfail · xpass $xpass ──"
echo "── 小计：正例 通过 $ok_pass · 失败 $ok_bad ──"
echo "── 基准轨诊断出口：stdout $fd_out_stdout 例 / stderr $fd_out_stderr 例（仅供参考，非判据）──"
bad=$((bad+ok_bad))
if [ "$bad" -gt 0 ]; then
    echo "❌ 引擎一致性门失败"
    echo "   提示 1：若默认轨静默通过，先确认入库二进制是否已按当前源码重烘："
    echo "           cp selfhost/build/compiler_vm bootstrap/pxc_vm"
    echo "   提示 2：若基准轨首行不符，golden/errors 与编译器源码可能不同批更新。"
    exit 1
fi
echo "✅ 引擎一致性门通过"
exit 0
