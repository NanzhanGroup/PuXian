#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════
# examples/m267_perf/verify.sh —— **性能回归门**（M267）
# ------------------------------------------------------------
# 为什么有这一门（清歌提案 P0-1；见 docs/PERF_BASELINE_V2.md）：
#   · 全仓 179 个门**全是正确性门**，无一性能判据；M105/M106/M107 三笔收益
#     （1.88× / 1.78× / 5.6×）全靠人工量化 ⇒ 后续重构可悄悄退回。
#   · M266 实证：连**生成基线的脚本**（examples/m89_perf/bench_vm_vs_c.sh）都
#     静默烂了 8 个月（两条命令都走 VM 轨）而无人知晓 —— 因为它不被任何门跑。
#
# 判据设计（按"避免假阳/假阴"取舍）：
#   ① 报**相对基线的 ×倍**，不报绝对秒（机器/负载会变，绝对值不可跨环境）
#   ② **两轮确认**：首轮超 fail 阈值 ⇒ 复测取 min；仍超才判红（抗偶发干扰）
#   ③ **产物身份自检**：C 轨含 fn_* 符号 / VM 轨不含 —— 防 M266 那类腐烂重演
#
# 阈值：超 warn(默认 1.25×) 出 ⚠️ 但仍绿；超 fail(默认 2.00×) **判红**。
#
# 用法：
#   bash examples/m267_perf/verify.sh               # 跑门（默认）
#   bash examples/m267_perf/verify.sh --update       # 刷新基线表（人工确认后提交）
#   bash examples/m267_perf/verify.sh --neg          # 负控（6 道，各自独立判红）
#   bash examples/m267_perf/verify.sh --list         # 列出负载与基线
#   bash examples/m267_perf/verify.sh --only fib28_vm
# ── M303（S1）扩展：接入 **ONNX 负载** ──────────────────────────
#   在此之前性能门**一个 ONNX 项都没有** ⇒ 谁把 MatMul 改快改慢，没有任何判据看得见。
#   两档刻意构成对照：512³ 在"无崩塌"区（1.12 GFLOPS）、1024³ 在**跨页崩塌**区
#   （本机实测 0.313 GFLOPS ⇒ 慢 3.58×；观音那台同形状慢 13×）⇒ 后者是路线 S3 的靶子。
#   ⚠️ ONNX 负载**单轨**（vm/c 同一份 C 实现）；kind 记为 `onnx`，与 `run` 区分。
# ── M305：环境感知基线档（baselines/<cpu-signature>.tsv）──────────
#   2026-10-12 宿主冷迁移（EPYC 7K62 → Xeon E5-2682 v4）暴露的结构问题：
#   单一 baseline.tsv 的比值**跨机不可比** ⇒ 门在新机上恒报 4 项红，而那不是回归
#   （C 轨原生产物同样慢 1.5–2.1× ⇒ 机器本身慢了，与 PuXian 无关）。
#   选档：本机档存在 ⇒ 相对本机档判（严格）；否则合成「各档逐负载 max」档
#         （跨机时上限判据宽松但仍有牙，**下限判据不变**）。
#   在陌生机器上想看到严格判据：先 `--update` 建本机档。
#   新增用法：`--baselines` 列出所有机器档。
# 环境：M267_REPS / M267_WARN / M267_FAIL / M267_W / M267_ROOT / M267_DIR / M267_MODELS
# ══════════════════════════════════════════════════════════════
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -uo pipefail

# ROOT/DIR 可用环境变量覆盖 —— 便于在 /tmp 沙箱里自测（也是门自身的可测性要求）
ROOT="${M267_ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
DIR="${M267_DIR:-$ROOT/examples/m267_perf}"
SRC="$DIR/src"
# ⭐ M305（环境感知基线档）：门不再直接读单一 baseline.tsv，而是按**本机 CPU 签名**选档。
#    2026-10-12 宿主冷迁移（AMD EPYC 7K62 → Intel Xeon E5-2682 v4）后，单一基线的比值
#    **跨机不可比** ⇒ 门恒报 4 项红（jsonwb×2.0 / startup×2.34 / hot_build×2.23），
#    而那**不是**回归（C 轨原生产物同样慢 1.5–2.1× ⇒ 是机器本身慢了）。
#    选档规则：baselines/<sig>.tsv 存在 ⇒ 用它；否则用「各档逐负载 max」合成档
#    （跨机时仍保留「比最慢参考机还慢 N 倍」这条有牙判据，而不是把上限判据整个放空）。
BASELINES="$DIR/baselines"
BASE="${M267_BASE:-}"            # ← 由 select_baseline() 在主入口填（见下）
PX="$ROOT/tools/px"
W="${M267_W:-/tmp/m267_perf}"
REPS="${M267_REPS:-5}"
WARN_R="${M267_WARN:-1.25}"
FAIL_R="${M267_FAIL:-2.00}"
LOADS="fib28 while_sum mixed jsonwb"
# ⭐ M305：负控的**代表性负载子集**。
#   为什么要收敛：M303 加 ONNX 两行后，4 道负控（各跑一次 run_compare）的耗时
#   从 ≈3 分钟涨到 ≈13 分钟 —— 门总耗时贴到全量门的 `GATE_TIMEOUT_DEFAULT=900s` 上限，
#   在 4 路分片（负载更高）下会**超时**（而超时是静默失败，最坏的一种）。
#   负控的目的是「证明判据有牙」—— 代表三条不同形态（纯计算 / 容器 / ONNX 单轨）已足够；
#   **正判据仍遍历全部 12 个负载**（那是主判据，不受影响）。
#   要回全量：`M267_NEG_ONLY=""`（空值 = 不过滤）。
NEG_ONLY="${M267_NEG_ONLY-fib28_vm mixed_vm onnx_mm512}"
# ⭐ M303（S1）新增：**单轨 ONNX 负载**（只有 VM 轨一份产物 —— ONNX 执行全在 runtime 的
#    C 代码里，两轨跑的是同一个 onnx_exec_run ⇒ vm/c 对照对本类负载无意义）。
#    它们在**选定档**里的 kind 是 `onnx`，因此 [1] 层"vm/c 成对登记"那条判据
#    （只筛 kind==run）不会误伤它们。
# ⭐ M310_W2_MARK（丙2 W2）：ONNX 负载清单 = **单一事实源**（`onnx_loads.tsv`）。
#   为什么改（W0-① 实测）：原先负载名在 `reps_of` 与 `measure_one` **两处硬编码**，
#   模型文件名又各写一遍 ⇒ 加一个算子要改三处 + 档；漏一处就是「账本里静默少一项」。
#   现在：清单给 <负载名> <模型文件> <覆盖算子> <说明>，本脚本从清单派生：
#     · ONNX_LOADS（构建 / 身份自检 / 静态自证的遍历面）
#     · 每个负载的模型文件名（measure_one 的派发）
#   ⚠️ 清单 ≠ 可选装饰：check_load_coverage.sh 双向核对「注册表 ⇄ 清单 ⇄ 源码」。
ONNX_MANIFEST="$DIR/onnx_loads.tsv"
onnx_manifest_rows() { grep -v '^[[:space:]]*#' "$ONNX_MANIFEST" 2>/dev/null | grep -v '^[[:space:]]*$'; }
ONNX_LOADS="$(onnx_manifest_rows | awk '{print $1}' | tr '\n' ' ')"
ONNX_LOADS="${ONNX_LOADS% }"
onnx_model_of() { onnx_manifest_rows | awk -v n="$1" '$1==n{print $2; exit}'; }
MODELS_DIR="${M267_MODELS:-$W/models}"

# ── 负控期间的「基线还原护栏」（M213 教训：门被中途杀掉会留下负控残留）──
#   本轮实测踩过：--neg 跑到 NC-A 时被我这边 120s 工具上限杀掉 ⇒ baseline.tsv
#   留下"÷3"的假基线（下次跑门就会假红）。⇒ 必须 trap 还原，不能只靠顺序执行。
BASE_ORIG=""
restore_base() {
    if [ -n "$BASE_ORIG" ] && [ -f "$BASE_ORIG" ]; then cp "$BASE_ORIG" "$BASE" 2>/dev/null; fi
    return 0
}
trap 'restore_base' EXIT
trap 'restore_base; exit 130' INT TERM HUP


PASS=0; FAILN=0
ok()   { echo "  ✅ $*"; PASS=$((PASS+1)); }
bad()  { echo "  ❌ $*"; FAILN=$((FAILN+1)); }
info() { echo "  · $*"; }

# ── ⭐ M305：环境感知基线档（machine signature → baseline file）──────
#   machine_sig() 抽到 machine_sig.sh —— **单一实现**（check_spec_consistency.sh 也用）
. "$DIR/machine_sig.sh" || { echo "❌ 缺 machine_sig.sh（examples/m267_perf/）" >&2; exit 2; }

# 合成档：各档逐负载取 max（= 最宽松但仍可判「比最慢参考机慢 N 倍」）
synth_baseline() {
    echo "# M267 性能回归基线 —— **合成档**（本机无档时自动生成，勿提交）"
    echo "# 合成规则：baselines/*.tsv 逐负载取 **max**"
    echo "# 用途：在陌生机器（如 CI runner）上仍保留「比最慢参考机还慢 N 倍」这条有牙判据"
    echo "# 档数：$(ls "$BASELINES"/*.tsv 2>/dev/null | wc -l)"
    echo "# ⚠️ 跨机绝对值不可比 —— 要在本机做性能回归判据，请先跑：bash verify.sh --update"
    echo "# 格式：<name> <kind:run|startup|build> <unit> <baseline秒> <说明>"
    echo "# 来历：M267（合成 · M305 档位机制）"
    awk '
      /^#/ { next }
      NF >= 4 {
        name=$1; kind=$2; unit=$3; val=$4+0
        note=""; for (i=5; i<=NF; i++) note = note (i>5 ? " " : "") $i
        if (!(name in mx) || val > mx[name]) { mx[name]=val; kd[name]=kind; un[name]=unit; nt[name]=note }
        if (!(name in seen)) { seen[name]=1; ord[++n]=name }
      }
      END { for (i=1; i<=n; i++) { nm=ord[i]; printf "%-14s %-8s %-6s %-10s %s\n", nm, kd[nm], un[nm], mx[nm], nt[nm] } }
    ' "$BASELINES"/*.tsv 2>/dev/null
}

BASE_LABEL=""
select_baseline() {
    SIG="${M267_SIG:-$(machine_sig)}"
    OWN_BASE="$BASELINES/$SIG.tsv"
    if [ -n "${M267_BASE:-}" ]; then
        BASE="$M267_BASE"; BASE_LABEL="显式指定（M267_BASE）· sig=$SIG"; return 0
    fi
    if [ -f "$OWN_BASE" ]; then
        BASE="$OWN_BASE"; BASE_LABEL="本机档（$SIG）· 判据 = 相对本机基线"
    else
        local n; n=$(ls "$BASELINES"/*.tsv 2>/dev/null | wc -l)
        if [ "$n" -eq 0 ]; then
            echo "❌ 基线档缺失：$BASELINES/*.tsv 一份都没有 ⇒ 无法比对（M305 档位机制）" >&2
            BASE="$BASELINES/__missing__.tsv"
        else
            BASE="$W/baseline.synth.tsv"
            synth_baseline > "$BASE"
            BASE_LABEL="**合成档**（本机无 $SIG 档）· 上限判据 = 相对各档 max ⇒ 跨机宽松，仅防失控"
        fi
    fi
}

# ── 负载清单（单一事实源 = 选定档）────────────────────────────
read_baseline() { grep -v '^#' "$BASE" | grep -v '^$'; }

# ── 计时：钉核 + min of N ─────────────────────────────────────
PIN=""
command -v taskset >/dev/null 2>&1 && PIN="taskset -c 3"

# ── 每个负载的**外层轮数**（ONNX 负载的"单次"成本差两个数量级，不能用同一个 REPS）──
#   onnx_mm1024 单次就 6.9 s ⇒ 2 轮即可；onnx_mm512 单次 1.3 s ⇒ 3 轮；
#   纯语言负载沿用全局 REPS（默认 5）。
reps_of() {
    case "$1" in
      onnx_mm1024) echo 2 ;;   # 单次 ≈4.4 s（本机档）⇒ 2 轮即够（M303 实测口径）
      onnx_mm512)  echo 3 ;;   # 单次 ≈2.1 s ⇒ 3 轮
      onnx_*)      echo 3 ;;          # M310_W2：新负载单次 1–2.5 s（内层 repeat 摊薄短时抖动）⇒ 3 轮
      *)           echo "$REPS" ;;
    esac
}

time_run() { # $1=轮数 $2...=命令
    local r="$1"; shift
    local -a all=()
    for _ in $(seq 1 "$r"); do
        local s e d
        s=$(date +%s.%N); $PIN "$@" >/dev/null 2>&1; e=$(date +%s.%N)
        d=$(echo "$e $s" | awk '{printf "%.4f", $1-$2}')
        all+=("$d")
    done
    printf '%s\n' "${all[@]}" | sort -n | head -1
}

# ── 产物身份自检（M266 教训）──────────────────────────────────
# 判据：C 轨产物含 fn_* 符号；VM 轨不含。
# 消息经全局 ID_MSG 回传（函数只写 stderr，避免污染 $() 捕获）。
ID_MSG=""
check_identity() { # $1=产物 $2=C|VM
    local n
    n=$(strings -a "$1" 2>/dev/null | grep -c '^fn_')
    ID_MSG=""
    if [ "$2" = C ]; then
        [ "$n" -ge 1 ] || { ID_MSG="期望 C 轨（应含 fn_* 符号），实测 $n 个"; return 1; }
    else
        [ "$n" -eq 0 ] || { ID_MSG="期望 VM 轨（应无 fn_* 符号），实测 $n 个"; return 1; }
    fi
    return 0
}

# ── 构建产物（产物落在 work 目录，**不写进仓库**）──────────────
build_all() {
    local fail=0
    # ⚠️ px build 的产物落点是**源文件所在目录**的 build/ ⇒ 必须先把源拷进 work 目录，
    #    否则会往仓库里写 examples/m267_perf/src/build/（本轮实测踩过）。
    for n in $LOADS; do
        mkdir -p "$W/vm_$n" "$W/c_$n"
        cp "$SRC/$n.px" "$W/vm_$n/"; cp "$SRC/$n.px" "$W/c_$n/"
        ( cd "$W/vm_$n" && "$PX" build     "$n.px" >/dev/null 2>&1 ) || { echo "  构建失败 VM $n"; fail=1; }
        ( cd "$W/c_$n"  && "$PX" build --c "$n.px" >/dev/null 2>&1 ) || { echo "  构建失败 C  $n"; fail=1; }
    done
    mkdir -p "$W/empty"; printf 'print(1)\n' > "$W/empty/empty.px"
    ( cd "$W/empty" && "$PX" build "empty.px" >/dev/null 2>&1 ) || { echo "  构建失败 empty"; fail=1; }
    # ── ⭐ M303（S1）：ONNX 负载 = 先**生成模型**（不入库，见 tools/gen_perf_models.py 头注），
    #    再编译负载程序。模型每次重新生成（约 2.8 s）——刻意不缓存：陈旧模型会让
    #    "账本"失去意义，而 2.8 s 不值得换那类风险。
    if [ -n "$ONNX_LOADS" ]; then
        mkdir -p "$MODELS_DIR" "$W/onnx"
        if ! python3 "$DIR/tools/gen_perf_models.py" "$MODELS_DIR" >/dev/null 2>&1; then
            echo "  模型生成失败（$DIR/tools/gen_perf_models.py）"; fail=1
        fi
        for n in $ONNX_LOADS; do
            cp "$SRC/$n.px" "$W/onnx/"
            ( cd "$W/onnx" && "$PX" build "$n.px" >/dev/null 2>&1 ) || { echo "  构建失败 ONNX $n"; fail=1; }
        done
    fi
    return $fail
}

# ── 测一个负载，返回秒（stdout）───────────────────────────────
measure_one() { # $1=name $2=kind
    local n="$1" k="$2"
    case "$k" in
      run)
        case "$n" in
          *_vm) time_run "$(reps_of "$n")" "$W/vm_${n%_vm}/build/${n%_vm}" ;;
          *_c)  time_run "$(reps_of "$n")" "$W/c_${n%_c}/build/${n%_c}" ;;
          *)    echo "-1"; return 1 ;;
        esac ;;
      onnx)
        # ONNX 负载：单轨，需把**模型路径**作为参数传给负载程序。
        # M310_W2：**按清单派发**（模型文件名来自 onnx_loads.tsv，不再逐个硬编码）。
        local _mdl; _mdl="$(onnx_model_of "$n")"
        if [ -z "$_mdl" ]; then echo "-1"; return 1; fi
        time_run "$(reps_of "$n")" "$W/onnx/build/$n" "$MODELS_DIR/$_mdl" ;;
      startup)
        local s e
        s=$(date +%s.%N); for _ in $(seq 200); do "$W/empty/build/empty" >/dev/null 2>&1; done; e=$(date +%s.%N)
        echo "$e $s" | awk '{printf "%.6f", ($1-$2)/200}' ;;
      build)
        local s e
        for _ in 1 2; do ( cd "$W/empty" && "$PX" build "empty.px" >/dev/null 2>&1 ); done
        s=$(date +%s.%N); ( cd "$W/empty" && "$PX" build "empty.px" >/dev/null 2>&1 ); e=$(date +%s.%N)
        echo "$e $s" | awk '{printf "%.4f", $1-$2}' ;;
      *) echo "-1"; return 1 ;;
    esac
}

# ── 主比对 ────────────────────────────────────────────────────
run_compare() {
    local only="${1:-}" nenv=0
    printf "  %-13s %-9s %-10s %-8s %-5s %s\n" "负载" "基线(s)" "实测(s)" "倍数" "判定" "阈值 warn/fail"
    while read -r name kind unit base note; do
        # ⭐ M305：`only` 支持**空格分隔的多值**（负控用代表性子集 —— 见 NEG_ONLY 的理由）
        if [ -n "$only" ] && [[ " $only " != *" $name "* ]]; then continue; fi
        nenv=$((nenv+1))
        local got r
        got=$(measure_one "$name" "$kind")
        if [ "$got" = "-1" ]; then bad "$name 无法测量（未知 kind=$kind）"; continue; fi
        r=$(echo "$got $base" | awk '{if ($2+0>0) printf "%.3f", $1/$2; else print "0"}')
        local verdict="✅"
        # ⭐ M303 **下限判据**：实测比基线**快 10 倍以上** ⇒ 负载疑似**根本没执行**
        #    （图被空转 / 提前退出 / 输入为空）。比值判据只看得见"变慢"这一侧，
        #    这条补上"快得不可能"那一侧 —— 否则一张坏掉的模型会以"性能大幅提升"
        #    的面目**通过**性能门（比漏报退化更危险：它会把 S3 的基线污染掉）。
        if [ "$(echo "$r 0.10" | awk '{print ($1 < $2) ? 1 : 0}')" = 1 ]; then
            printf "  %-13s %-9s %-10s %-8s %-5s %s\n" "$name" "$base" "$got" "${r}x" "❌" "$WARN_R/$FAIL_R"
            bad "$name 实测 ${r}x —— 比基线快 10 倍以上，负载疑似未真执行（M303 下限判据）"
            continue
        fi
        if [ "$(echo "$r $FAIL_R" | awk '{print ($1 > $2) ? 1 : 0}')" = 1 ]; then
            local got2 r2
            got2=$(measure_one "$name" "$kind")
            if [ "$(echo "$got2 $got" | awk '{print ($1 < $2) ? 1 : 0}')" = 1 ]; then got="$got2"; fi
            r=$(echo "$got $base" | awk '{printf "%.3f", $1/$2}')
            [ "$(echo "$r $FAIL_R" | awk '{print ($1 > $2) ? 1 : 0}')" = 1 ] && verdict="❌"
        fi
        if [ "$verdict" = "✅" ] && [ "$(echo "$r $WARN_R" | awk '{print ($1 > $2) ? 1 : 0}')" = 1 ]; then
            verdict="⚠️"
        fi
        printf "  %-13s %-9s %-10s %-8s %-5s %s\n" "$name" "$base" "$got" "${r}x" "$verdict" "$WARN_R/$FAIL_R"
        case "$verdict" in
          "❌") bad "$name 退化 ${r}x（> fail ${FAIL_R}x）—— $note" ;;
          "⚠️") ok "$name 偏离 ${r}x（> warn ${WARN_R}x，未超判红线）" ;;
          *)    ok "$name ${r}x" ;;
        esac
    done < <(read_baseline)
    [ "$nenv" -gt 0 ] || bad "基线表为空或 --only 未命中"
}

update_baseline() {
    echo "刷新基线（写入 $OWN_BASE）"
    mkdir -p "$BASELINES"
    { echo "# M267 性能回归基线 —— 由 verify.sh --update 生成"
      echo "# machine-sig: $SIG"
      echo "# 生成机：$(uname -n) · $(machine_model) · $(nproc) 核"
      echo "# 工具链：$("$PX" --version 2>/dev/null | head -1) · gcc $(gcc -dumpversion 2>/dev/null) · $(date -u +%FT%TZ)"
      echo "# 格式：<name> <kind:run|startup|build> <unit> <baseline秒> <说明>"
      echo "# ⚠️ 绝对值只对**同机同工具链**有意义；判据是**相对变化**。刷新前须人工确认。"
      while read -r name kind unit base note; do
        local v; v=$(measure_one "$name" "$kind")
        printf "%-14s %-8s %-6s %-10s %s\n" "$name" "$kind" "$unit" "$v" "$note"
      done < <(read_baseline)
    } > "$OWN_BASE.new"
    mv "$OWN_BASE.new" "$OWN_BASE"
    echo "已写入 $OWN_BASE。请 git diff 核对后提交。"
}

# ── 负控 ──────────────────────────────────────────────────────
run_neg() {
    echo "══ 负控（每道必须独立判红）══"
    local rc_all=0 na nb nd ne
    cp "$BASE" "$W/base.bak"
    BASE_ORIG="$W/base.bak"          # ← 交给 EXIT trap（中途被杀也能还原）
    local h0; h0=$(sha256sum "$BASE" | cut -c1-16)

    echo "── NC-A 基线被人为改坏（原值 ÷3 ⇒ 比值 ×3）⇒ 必须判红"
    awk '/^#/ {print; next} NF>=4 { $4=$4/3 } { print }' "$W/base.bak" > "$BASE"
    na=$(run_compare "$NEG_ONLY" 2>&1 | grep -c '❌')
    cp "$W/base.bak" "$BASE"
    if [ "$na" -ge 1 ]; then echo "  ✅ NC-A 生效（$na 个负载判红）"; else echo "  ❌ NC-A 未生效：假基线未被判红"; rc_all=1; fi

    # ⚠️ 必须直接改 shell 变量 FAIL_R —— **不能**写 `M267_FAIL=0.5 run_compare`：
    #    FAIL_R 在脚本开头就已由 `${M267_FAIL:-2.00}` **展开成具体值**，环境变量
    #    只对子进程生效 ⇒ 覆盖无效（= M212/M209 记过的"覆盖变量式负控"老坑，本轮又踩）。
    echo "── NC-B fail 阈值调到不可能满足（0.5）⇒ 必须判红"
    local f_save="$FAIL_R"; FAIL_R=0.5
    nb=$(run_compare "$NEG_ONLY" 2>&1 | grep -c '❌')
    FAIL_R="$f_save"
    if [ "$nb" -ge 1 ]; then echo "  ✅ NC-B 生效（$nb 个负载判红）"; else echo "  ❌ NC-B 未生效"; rc_all=1; fi

    echo "── NC-C 产物身份自检能抓「标签反」（= M266 的腐烂形态）"
    if check_identity "$W/vm_fib28/build/fib28" C; then
        echo "  ❌ NC-C 未生效：把 VM 产物当 C 轨却通过了"; rc_all=1
    else
        echo "  ✅ NC-C 生效：VM 产物按 C 轨校验 ⇒ 判红（$ID_MSG）"
    fi

    echo "── NC-D 判据自伤：NC-A 的红必须来自比对本身"
    awk '/^#/ {print; next} NF>=4 { $4=$4/3 } { print }' "$W/base.bak" > "$BASE"
    FAIL_R=99
    nd=$(run_compare "$NEG_ONLY" 2>&1 | grep -c '❌')
    FAIL_R="$f_save"
    cp "$W/base.bak" "$BASE"
    if [ "$nd" -eq 0 ]; then echo "  ✅ NC-D 生效：fail 阈值放空 ⇒ 同一假基线不再判红（红来自比对）"; else echo "  ❌ NC-D 未生效（仍有 $nd 判红）"; rc_all=1; fi

    # ── NC-E（M303 新增）**下限判据**必须有牙：基线放大 20 倍 ⇒ 比值 <0.10 ⇒ 必须判红 ──
    #    与 NC-A 方向相反：NC-A 证明"变慢"看得见，NC-E 证明"快得不可能"也看得见。
    #    缺了这条，一张坏掉的 ONNX 模型会以"性能大幅提升"的面目**通过**性能门。
    echo "── NC-E 下限判据：基线放大 20 倍 ⇒ 比值 <0.10 ⇒ 必须判红"
    awk '/^#/ {print; next} NF>=4 { $4=$4*20 } { print }' "$W/base.bak" > "$BASE"
    ne=$(run_compare "$NEG_ONLY" 2>&1 | grep -c '负载疑似未真执行')
    cp "$W/base.bak" "$BASE"
    if [ "$ne" -ge 1 ]; then echo "  ✅ NC-E 生效（$ne 个负载触发下限判据）"; else echo "  ❌ NC-E 未生效：快得不可能却没判红"; rc_all=1; fi

    # ── NC-F（M305 新增）档位回退：伪造签名 ⇒ 必须走「合成档」 ──
    #    为什么用 `--list` + PX_GATE_LOCK_HELD=1：门内**不能重入**加锁（gate_lock 是门级互斥）。
    #    `--list` 只读，不构建、不改档 ⇒ 安全；与 mount 无任何冲突。
    echo "── NC-F 档位回退：伪造签名 ⇒ 必须走「合成档」（而非无声拿错档）"
    PX_GATE_LOCK_HELD=1 M267_SIG=zzz-no-such-cpu M267_W="$W/ncf" bash "$0" --list > "$W/ncf.log" 2>&1
    if grep -q '合成档' "$W/ncf.log"; then
        echo "  ✅ NC-F 生效：无本机档 ⇒ 走合成档（上限判据退为「各档 max」，不再恒红）"
    else
        echo "  ❌ NC-F 未生效：无本机档却没有走合成档"; rc_all=1
    fi

    # ── 还原核对：负控跑完**选定档**必须**逐字节回到原样** ──
    local h1; h1=$(sha256sum "$BASE" | cut -c1-16)
    if [ "$h0" = "$h1" ]; then echo "  ✅ 还原核对：选定档逐字节还原（sha $h1）"
    else echo "  ❌ 还原核对失败：$h0 → $h1（负控残留！）"; rc_all=1; fi
    return $rc_all
}

# ── 主入口 ────────────────────────────────────────────────────
MODE="${1:-run}"
mkdir -p "$W"
select_baseline
echo "══ M267 性能回归门 ══  机=$(uname -n) · $("$PX" --version 2>/dev/null | head -1) · REPS=$REPS · W=$W"
echo "   基线档：$BASE_LABEL"

case "$MODE" in
  --list)
    printf "  %-13s %-8s %-9s %s\n" "负载" "kind" "基线(s)" "说明"
    while read -r n k u b note; do printf "  %-13s %-8s %-9s %s\n" "$n" "$k" "$b" "$note"; done < <(read_baseline)
    exit 0 ;;
  --baselines)
    echo "   本机签名：$(machine_sig)"
    echo "   档目录：$BASELINES"
    for f in "$BASELINES"/*.tsv; do
        [ -f "$f" ] || continue
        printf "     %-28s %s\n" "$(basename "$f")" "$(grep -m1 '生成机' "$f" | sed 's/^# //')"
    done
    exit 0 ;;
  --update) build_all >/dev/null 2>&1 || true; update_baseline; exit 0 ;;
  --neg)    build_all >/dev/null || { echo "❌ 构建失败，负控无法进行"; exit 1; }
            run_neg; rc=$?
            echo "══ 负控结果：$([ $rc = 0 ] && echo '6/6 各自独立判红 ✅' || echo '有未生效 ❌') ══"
            exit $rc ;;
esac
only=""; [ "$MODE" = "--only" ] && only="${2:-}"

echo
echo "[1] 静态自证"
for f in $LOADS $ONNX_LOADS; do
    [ -f "$SRC/$f.px" ] && ok "负载源码存在 $f.px" || bad "缺负载源码 $f.px"
done
[ -f "$DIR/tools/gen_perf_models.py" ] && ok "ONNX 模型生成器存在（tools/gen_perf_models.py）" \
    || bad "缺 ONNX 模型生成器 tools/gen_perf_models.py"
# ⭐ M310_W2：**负载清单在位且可解析**（单一事实源；三列非空、负载名不重复）
if [ -f "$ONNX_MANIFEST" ]; then
    _badrows=$(onnx_manifest_rows | awk 'NF<3 || $2 !~ /\.onnx$/ {print $1}' | tr '\n' ' ')
    _dup=$(onnx_manifest_rows | awk '{print $1}' | sort | uniq -d | tr '\n' ' ')
    if [ -z "$_badrows" ] && [ -z "$_dup" ]; then
        ok "ONNX 负载清单可解析（$(onnx_manifest_rows | wc -l) 行 · onnx_loads.tsv）"
    else
        bad "ONNX 负载清单有坏行/重名：坏行=[$_badrows] 重名=[$_dup]"
    fi
else
    bad "缺 ONNX 负载清单 examples/m267_perf/onnx_loads.tsv（W2 单一事实源）"
fi
# ⭐ M305：**档位机制在位**（防“档目录被删/头注掉字段 ⇒ 门无声走错档”）
[ -f "$DIR/machine_sig.sh" ] && ok "机器签名实现存在（machine_sig.sh · 单一实现）" || bad "缺 machine_sig.sh"
_nbtsv=$(ls "$BASELINES"/*.tsv 2>/dev/null | wc -l)
[ "$_nbtsv" -ge 1 ] && ok "基线档 ≥1（当前 $_nbtsv 档）" || bad "基线档缺失（$BASELINES/*.tsv）"
if grep -q '^# machine-sig: ' "$BASE" 2>/dev/null; then
    ok "选定档头注含 machine-sig"
elif grep -q '合成档' "$BASE" 2>/dev/null; then
    ok "选定的是**合成档**（本机无档 ⇒ 上限判据退为「各档 max」）"
else
    bad "选定档头注既无 machine-sig 也不是合成档：$BASE"
fi
# ONNX 负载必须以 kind=onnx 登记 —— 若误登成 run，[1] 下面那条"vm/c 成对"判据会误伤它们
nb_onnx=$(read_baseline | awk '$2=="onnx"' | wc -l)
_nman=$(onnx_manifest_rows | wc -l)
[ "$nb_onnx" -ge "$_nman" ] && [ "$nb_onnx" -ge 2 ] && ok "ONNX 负载登记 $nb_onnx 行（清单 $_nman 个 · kind=onnx）" \
    || bad "ONNX 负载登记 $nb_onnx 行 / 清单 $_nman 个（登记数必须 ≥ 清单数；kind 必须写 onnx）"
# ⭐ M310_W4：清单里的每个负载**必须在选定档里有一行** —— 否则它永不进测量面（W0-① 的病）。
#    合成档（CI）从 5 个档取 max ⇒ 任一档缺行都会让该负载在合成档里消失。
_missbase=""
for n in $ONNX_LOADS; do
    grep -qE "^$n[[:space:]]" <<< "$(read_baseline)" || _missbase="$_missbase $n"
done
if [ -z "$_missbase" ]; then ok "清单负载全部登记进选定档（$BASE_LABEL 的测量面覆盖 $ONNX_LOADS）"
else bad "清单负载未登记进选定档（**永不测量**）：$_missbase"; fi
nb2=$(read_baseline | wc -l)
[ "$nb2" -ge 4 ] && ok "基线表 $nb2 行（下限 4）" || bad "基线表只有 $nb2 行（下限 4）"
# 每条 run 负载必须有 vm 与 c 两行（防"只登记一半"）
read_baseline | awk '$2=="run"{print $1}' | sed 's/_[vc]m\?$//' | sort | uniq -c | awk '$1<2{print $2}' > "$W/half.txt"
if [ -s "$W/half.txt" ]; then bad "以下 run 负载只登记一条轨（应 vm+c 成对）：$(tr '\n' ' ' < "$W/half.txt")"; else ok "每条 run 负载 vm/c 成对登记"; fi
grep -q '^#' "$BASE" && ok "基线表带头部说明（人可读 + 换机指引）" || bad "基线表缺头部说明"
grep -q 'M267' "$BASE" && ok "基线表标注来历（M267）" || bad "基线表未标注来历"

echo
echo "[2] 产物身份自检（M266 那类腐烂的守卫）"
if build_all; then
    ident_bad=0; nident=0
    for n in $LOADS; do
        check_identity "$W/vm_$n/build/$n" VM || { bad "VM 轨身份错 $n：$ID_MSG"; ident_bad=1; }
        check_identity "$W/c_$n/build/$n"  C  || { bad "C 轨身份错 $n：$ID_MSG"; ident_bad=1; }
        nident=$((nident+2))
    done
    # ONNX 负载是**单轨**（ONNX 执行在 runtime 的 C 代码里，vm/c 跑同一份实现）⇒ 只查 VM 身份
    for n in $ONNX_LOADS; do
        check_identity "$W/onnx/build/$n" VM || { bad "ONNX 负载身份错 $n：$ID_MSG"; ident_bad=1; }
        nident=$((nident+1))
    done
    [ "$ident_bad" = 0 ] && ok "$nident/$nident 产物身份正确（VM 无 fn_* / C 有 fn_*）"
else
    bad "构建失败，无法自检"
fi

echo
echo "[3] 正确性锚点（双轨 stdout 必须一致 + ONNX 输出尺寸）"
for n in $LOADS; do
    ov=$($PIN "$W/vm_$n/build/$n" 2>&1); oc=$($PIN "$W/c_$n/build/$n" 2>&1)
    [ "$ov" = "$oc" ] && ok "$n 双轨 stdout 一致（$ov）" || bad "$n 双轨 stdout 不一致 VM=[$ov] C=[$oc]"
done
# ONNX 通路锚点：证明"模型真被解析 / 图真被执行 / 输出尺寸与声明一致"。
# ⚠️ 只对 mm512 做（约 1.3 s）。mm1024 由 [4] 的**下限判据**覆盖 —— 图没真跑的话耗时
#    会塌到 1/10 以下而当场判红，不必再花 7.5 s 去"证明它跑了"。
exp512=$((512 * 512 * 4))
og=$($PIN "$W/onnx/build/onnx_mm512" "$MODELS_DIR/onnx_mm512.onnx" 2>&1)
[ "$og" = "OK $exp512" ] && ok "onnx_mm512 输出尺寸锚点（期望 OK $exp512）" \
    || bad "onnx_mm512 锚点失败：[$og]（期望 OK $exp512）"

echo
echo "[4] 相对基线比对"
run_compare "$only"

echo
echo "[5] 覆盖边界（如实登记）"
info "只覆盖**本机单核**负载；多核/IO/HTTP 服务型负载不在面内（M89 的 http_json 形态未纳入）"
info "冷构建（清 .rtcache）刻意不纳入：会污染同机其它门/构建的缓存，代价不可控"
info "基线绝对值只对**同机同工具链**有意义 —— 换机器/换 gcc/换 px 版本须 --update 重定基"
info "⭐ M305：门按**本机 CPU 签名**选档（baselines/<sig>.tsv）；本机无档时用「各档 max」合成档"
info "   ⇒ 跨机跑（如 CI runner）时上限判据自动宽松（跨机比值本就不可比），但下限判据（<0.10x）不变"
info "   当前档：$BASE_LABEL"
info "C 轨只取 fib28_c/while_sum_c 作比值锚（全谱见 docs/PERF_BASELINE_V2.md §五）"
info "本门**不覆盖** px run（解释轨）面 —— 那是 M268 的活；字符串 += O(n²) 是 M270"
info "ONNX 负载：MatMul（512³ 常规区 + 1024³ 跨页崩塌区）+ M310 接入的 5 族（量化 / decoder FFN 一步 / ScatterND KV 写入 / Cos+Sin / Trilu）"
info "  ⚠️ Conv / 多线程 GEMM / GPU 后端**仍不在面内**（M303 §五 S4/S5）"
info "ONNX 负载是**单轨**（vm 与 c 跑的是同一份 runtime C 实现 onnx_exec_run）⇒ 只登记一行，无 vm/c 对照"
info "ONNX 模型**不入库**（512³ 2 MiB / 1024³ 8 MiB）：由 tools/gen_perf_models.py **每次生成**到 work 目录（刻意不缓存）"
info "ONNX 负载测的是**单线程内核**：多线程 GEMM / GPU 后端不在面内（M303 §五 S5 另立基准机流程）"

echo
echo "══ M267 结果：$PASS 通过 / $FAILN 失败 ══"
[ "$FAILN" = 0 ] && echo "M267-VERIFY-OK"
exit $([ "$FAILN" = 0 ] && echo 0 || echo 1)
