#!/bin/bash
# ============================================================
# m89_perf/bench_vm_vs_c.sh —— VM 轨 vs C 轨性能基线（默认轨切换决策数据）
# ------------------------------------------------------------
# 目标：量化「px build 默认产物从 C 文本轨切到 VM 字节码轨」的性能代价。
#   对拍实体：同一 .px 源程序的两种用户产物——
#     C 轨  = px build --c（fn_* C 文本 → gcc 机器码运行时）
#     VM 轨 = px build（M91 起 = 默认；BCModule 字节码镜像 → VM 解释执行）
#
# ⚠️ M266 修复（2026-10-05）：**本脚本原先两条命令产出的是同一轨** ——
#   M91 把 `px build` 默认轨从 C 切到 VM 后，`px build` 与 `px build --vm` 均为 VM 轨
#   ⇒ 本脚本测的是 VM vs VM（实测 1.007×），既测不出老的 1.52× 也测不出新的 3.71×，
#   而它**不被任何门跑** ⇒ 静默失真 8 个月无人发现（见 docs/PERF_BASELINE_V2.md §四）。
#   现已改为 `--c` / 默认，并新增**产物身份自检**（C 轨含 fn_* 符号、VM 轨不含）。
# ⚠️ 数字也过期了：老的 6.091s/9.283s（px 0.2.0）现为 0.246s/0.913s（px 0.2.17）
#   ⇒ 绝对值以 docs/PERF_BASELINE_V2.md 为准；本脚本仅用于**相对变化**。
# 三类负载（覆盖计算/服务两端的代价区间）：
#   1) fib_calc     纯计算热点（递归 fib，CPU 密集最坏情形）
#   2) compiler 形态 同源码编译器（compiler.px）C轨/VM轨 产物各跑 bc dump
#                   同一输入（综合真实负载：parse/codegen/bc_emit）
#   3) http_json    长跑 HTTP JSON 服务（IO/服务型，ws-approve 类负载代表）
# 用法：
#   ./bench_vm_vs_c.sh fib|http|compile|all   # 单类或全跑（默认 all）
# 前置：tools/px 可 build；selfhost/build/compiler_new + compiler_vm 存在
#   （跑过 bootstrap_prove_bc.sh）；python3。
# 产物缓存：/tmp/m89perf/bin/（缺则自动重建）。结果 stdout 汇总。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
BIN=/tmp/m89perf/bin
SRC="$ROOT/examples/m89_perf"
RUNONE="$SRC/http_bench_one.sh"
PRESS="$SRC/press_http.py"
mkdir -p "$BIN"

# ---------- 产物就绪 ----------
ensure_bins() {
    local need=0
    for b in fib_calc_c fib_calc_vm http_json_c http_json_vm; do
        [ -x "$BIN/$b" ] || need=1
    done
    [ "$need" = 0 ] && return 0
    echo "── 重建产物（C 轨 --c / VM 轨 默认）..."
    ( cd "$SRC" && "$PX" build --c fib_calc.px >/dev/null 2>&1 && cp build/fib_calc "$BIN/fib_calc_c" \
      && "$PX" build     fib_calc.px >/dev/null 2>&1 && cp build/fib_calc "$BIN/fib_calc_vm" \
      && "$PX" build --c http_json.px >/dev/null 2>&1 && cp build/http_json "$BIN/http_json_c" \
      && "$PX" build     http_json.px >/dev/null 2>&1 && cp build/http_json "$BIN/http_json_vm" ) \
      || { echo "❌ 产物重建失败"; exit 1; }
    # ★ M266 新增：产物身份自检（C 轨含 fn_* 符号；VM 轨不含）
    check_identity "$BIN/fib_calc_c" C  && check_identity "$BIN/fib_calc_vm" VM \
      && check_identity "$BIN/http_json_c" C && check_identity "$BIN/http_json_vm" VM \
      || { echo "   ⛔ 产物身份自检失败 ⇒ 两条轨可能已退化为同一轨，**本脚本的对比结论无效**"; exit 1; }
    echo "    产物就绪（身份自检 ✅）：$(ls "$BIN" | tr '\n' ' ')"
}

# ---------- 产物身份自检（M266）：C 轨含 fn_* 符号，VM 轨不含 ----------
# 没有这一步就会重犯「两条命令同一轨」的错（见文件头 ⚠️）。
check_identity() { # $1=产物路径 $2=期望轨（C|VM）
    local n
    n=$(strings -a "$1" 2>/dev/null | grep -c '^fn_')
    if [ "$2" = C ]; then
        [ "$n" -ge 1 ] || { echo "❌ 身份错：$1 期望 C 轨（应含 fn_* 符号），实测 $n 个"; return 1; }
    else
        [ "$n" -eq 0 ] || { echo "❌ 身份错：$1 期望 VM 轨（应无 fn_* 符号），实测 $n 个"; return 1; }
    fi
}

# ---------- 计时助手：run 3 轮取中位 ----------
median3() { # $1=标签 $2=执行命令(env 前缀 + 可执行 + args)
    local tag="$1"; shift
    local vals=()
    for i in 1 2 3; do
        local s e d
        s=$(date +%s.%N); "$@" >/dev/null 2>&1; e=$(date +%s.%N)
        d=$(echo "$e $s" | awk '{printf "%.3f", $1-$2}')
        vals+=("$d")
    done
    # 冒泡取中位（vals 已 3 元素）
    local a="${vals[0]}" b="${vals[1]}" c="${vals[2]}" med
    med=$(echo -e "$a\n$b\n$c" | sort -n | sed -n 2p)
    echo "$tag 三轮=[$a $b $c] 中位=${med}s"
}

# ---------- 1) fib 纯计算 ----------
bench_fib() {
    echo ""
    echo "══════════ 基准 1：fib_calc（纯计算热点，fib(28)×5）══════════"
    ensure_bins
    echo "── C 轨产物（px build --c）"
    BENCH_REPS=5 "$BIN/fib_calc_c" >/dev/null   # warmup
    median3 "C轨" env BENCH_REPS=5 "$BIN/fib_calc_c"
    echo "── VM 轨产物（px build，M91 起默认）"
    BENCH_REPS=5 "$BIN/fib_calc_vm" >/dev/null  # warmup
    median3 "VM轨" env BENCH_REPS=5 "$BIN/fib_calc_vm"
}

# ---------- 3) HTTP JSON 服务 ----------
bench_http() {
    echo ""
    echo "══════════ 基准 3：http_json（长跑 HTTP JSON 服务，8 并发×200×3 轮）══════════"
    ensure_bins
    echo "── C 轨产物"
    bash "$RUNONE" c 18091 8 200 2>&1 | tail -1
    bash "$RUNONE" c 18091 8 200 2>&1 | tail -1
    bash "$RUNONE" c 18091 8 200 2>&1 | tail -1
    echo "── VM 轨产物"
    bash "$RUNONE" vm 18092 8 200 2>&1 | tail -1
    bash "$RUNONE" vm 18092 8 200 2>&1 | tail -1
    bash "$RUNONE" vm 18092 8 200 2>&1 | tail -1
}

# ---------- 2) compiler 形态（同源码编译器双轨产物跑同一输入）----------
bench_compile() {
    echo ""
    echo "══════════ 基准 2：compiler 形态（compiler.px 双轨产物 bc dump 同一输入 ×5 轮）══════════"
    local F="$ROOT/selfhost/astdump.px"
    local CN="$ROOT/selfhost/build/compiler_new" CV="$ROOT/selfhost/build/compiler_vm"
    [ -x "$CN" ] && [ -x "$CV" ] || { echo "❌ 缺 compiler_new/compiler_vm（先跑 bootstrap_prove_bc.sh）"; return 1; }
    echo "── C 轨编译器（compiler_new，compiler.px→C 机器码）"
    "$CN" bc "$F" >/dev/null 2>&1   # warmup
    for i in 1 2 3 4 5; do
        s=$(date +%s.%N); "$CN" bc "$F" >/dev/null 2>&1; e=$(date +%s.%N)
        echo "C轨 轮$i: $(echo "$e $s"|awk '{printf "%.3f",$1-$2}')s"
    done
    echo "── VM 轨编译器（compiler_vm，compiler.px→字节码→VM 解释）"
    "$CV" bc "$F" >/dev/null 2>&1   # warmup
    for i in 1 2 3 4 5; do
        s=$(date +%s.%N); "$CV" bc "$F" >/dev/null 2>&1; e=$(date +%s.%N)
        echo "VM轨 轮$i: $(echo "$e $s"|awk '{printf "%.3f",$1-$2}')s"
    done
}

# ---------- 主入口 ----------
MODE="${1:-all}"
case "$MODE" in
    fib)     bench_fib ;;
    http)    bench_http ;;
    compile) bench_compile ;;
    all)
        bench_fib
        bench_http
        bench_compile
        ;;
    *) echo "用法: $0 fib|http|compile|all"; exit 1 ;;
esac
