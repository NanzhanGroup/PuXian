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
#   bash examples/m267_perf/verify.sh --neg          # 负控（4 道，各自独立判红）
#   bash examples/m267_perf/verify.sh --list         # 列出负载与基线
#   bash examples/m267_perf/verify.sh --only fib28_vm
# 环境：M267_REPS / M267_WARN / M267_FAIL / M267_W / M267_ROOT / M267_DIR
# ══════════════════════════════════════════════════════════════
set -uo pipefail

# ROOT/DIR 可用环境变量覆盖 —— 便于在 /tmp 沙箱里自测（也是门自身的可测性要求）
ROOT="${M267_ROOT:-$(cd "$(dirname "$0")/../.." && pwd)}"
DIR="${M267_DIR:-$ROOT/examples/m267_perf}"
SRC="$DIR/src"
BASE="$DIR/baseline.tsv"
PX="$ROOT/tools/px"
W="${M267_W:-/tmp/m267_perf}"
REPS="${M267_REPS:-5}"
WARN_R="${M267_WARN:-1.25}"
FAIL_R="${M267_FAIL:-2.00}"
LOADS="fib28 while_sum mixed jsonwb"

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

# ── 负载清单（单一事实源 = baseline.tsv）────────────────────────
read_baseline() { grep -v '^#' "$BASE" | grep -v '^$'; }

# ── 计时：钉核 + min of N ─────────────────────────────────────
PIN=""
command -v taskset >/dev/null 2>&1 && PIN="taskset -c 3"

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
    return $fail
}

# ── 测一个负载，返回秒（stdout）───────────────────────────────
measure_one() { # $1=name $2=kind
    local n="$1" k="$2"
    case "$k" in
      run)
        case "$n" in
          *_vm) time_run "$REPS" "$W/vm_${n%_vm}/build/${n%_vm}" ;;
          *_c)  time_run "$REPS" "$W/c_${n%_c}/build/${n%_c}" ;;
          *)    echo "-1"; return 1 ;;
        esac ;;
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
        [ -n "$only" ] && [ "$name" != "$only" ] && continue
        nenv=$((nenv+1))
        local got r
        got=$(measure_one "$name" "$kind")
        if [ "$got" = "-1" ]; then bad "$name 无法测量（未知 kind=$kind）"; continue; fi
        r=$(echo "$got $base" | awk '{if ($2+0>0) printf "%.3f", $1/$2; else print "0"}')
        local verdict="✅"
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
    echo "刷新基线（写入 $BASE）"
    { echo "# M267 性能回归基线 —— 由 verify.sh --update 生成"
      echo "# 生成机：$(uname -n) · $(grep -m1 'model name' /proc/cpuinfo | sed 's/.*: //') · $(nproc) 核"
      echo "# 工具链：$("$PX" --version 2>/dev/null | head -1) · gcc $(gcc -dumpversion 2>/dev/null) · $(date -u +%FT%TZ)"
      echo "# 格式：<name> <kind:run|startup|build> <unit> <baseline秒> <说明>"
      echo "# ⚠️ 绝对值只对**同机同工具链**有意义；判据是**相对变化**。刷新前须人工确认。"
      while read -r name kind unit base note; do
        local v; v=$(measure_one "$name" "$kind")
        printf "%-14s %-8s %-6s %-10s %s\n" "$name" "$kind" "$unit" "$v" "$note"
      done < <(read_baseline)
    } > "$BASE.new"
    mv "$BASE.new" "$BASE"
    echo "已写入。请 git diff 核对后提交。"
}

# ── 负控 ──────────────────────────────────────────────────────
run_neg() {
    echo "══ 负控（每道必须独立判红）══"
    local rc_all=0 na nb nd
    cp "$BASE" "$W/base.bak"
    BASE_ORIG="$W/base.bak"          # ← 交给 EXIT trap（中途被杀也能还原）
    local h0; h0=$(sha256sum "$BASE" | cut -c1-16)

    echo "── NC-A 基线被人为改坏（原值 ÷3 ⇒ 比值 ×3）⇒ 必须判红"
    awk '/^#/ {print; next} NF>=4 { $4=$4/3 } { print }' "$W/base.bak" > "$BASE"
    na=$(run_compare "" 2>&1 | grep -c '❌')
    cp "$W/base.bak" "$BASE"
    if [ "$na" -ge 1 ]; then echo "  ✅ NC-A 生效（$na 个负载判红）"; else echo "  ❌ NC-A 未生效：假基线未被判红"; rc_all=1; fi

    # ⚠️ 必须直接改 shell 变量 FAIL_R —— **不能**写 `M267_FAIL=0.5 run_compare`：
    #    FAIL_R 在脚本开头就已由 `${M267_FAIL:-2.00}` **展开成具体值**，环境变量
    #    只对子进程生效 ⇒ 覆盖无效（= M212/M209 记过的"覆盖变量式负控"老坑，本轮又踩）。
    echo "── NC-B fail 阈值调到不可能满足（0.5）⇒ 必须判红"
    local f_save="$FAIL_R"; FAIL_R=0.5
    nb=$(run_compare "" 2>&1 | grep -c '❌')
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
    nd=$(run_compare "" 2>&1 | grep -c '❌')
    FAIL_R="$f_save"
    cp "$W/base.bak" "$BASE"
    if [ "$nd" -eq 0 ]; then echo "  ✅ NC-D 生效：fail 阈值放空 ⇒ 同一假基线不再判红（红来自比对）"; else echo "  ❌ NC-D 未生效（仍有 $nd 判红）"; rc_all=1; fi

    # ── 还原核对：负控跑完 baseline.tsv 必须**逐字节回到原样** ──
    local h1; h1=$(sha256sum "$BASE" | cut -c1-16)
    if [ "$h0" = "$h1" ]; then echo "  ✅ 还原核对：baseline.tsv 逐字节还原（sha $h1）"
    else echo "  ❌ 还原核对失败：$h0 → $h1（负控残留！）"; rc_all=1; fi
    return $rc_all
}

# ── 主入口 ────────────────────────────────────────────────────
MODE="${1:-run}"
mkdir -p "$W"
echo "══ M267 性能回归门 ══  机=$(uname -n) · $("$PX" --version 2>/dev/null | head -1) · REPS=$REPS · W=$W"

case "$MODE" in
  --list)
    printf "  %-13s %-8s %-9s %s\n" "负载" "kind" "基线(s)" "说明"
    while read -r n k u b note; do printf "  %-13s %-8s %-9s %s\n" "$n" "$k" "$b" "$note"; done < <(read_baseline)
    exit 0 ;;
  --update) build_all >/dev/null 2>&1 || true; update_baseline; exit 0 ;;
  --neg)    build_all >/dev/null || { echo "❌ 构建失败，负控无法进行"; exit 1; }
            run_neg; rc=$?
            echo "══ 负控结果：$([ $rc = 0 ] && echo '4/4 各自独立判红 ✅' || echo '有未生效 ❌') ══"
            exit $rc ;;
esac
only=""; [ "$MODE" = "--only" ] && only="${2:-}"

echo
echo "[1] 静态自证"
for f in $LOADS; do
    [ -f "$SRC/$f.px" ] && ok "负载源码存在 $f.px" || bad "缺负载源码 $f.px"
done
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
    ident_bad=0
    for n in $LOADS; do
        check_identity "$W/vm_$n/build/$n" VM || { bad "VM 轨身份错 $n：$ID_MSG"; ident_bad=1; }
        check_identity "$W/c_$n/build/$n"  C  || { bad "C 轨身份错 $n：$ID_MSG"; ident_bad=1; }
    done
    [ "$ident_bad" = 0 ] && ok "8/8 产物身份正确（VM 无 fn_* / C 有 fn_*）"
else
    bad "构建失败，无法自检"
fi

echo
echo "[3] 正确性锚点（双轨 stdout 必须一致）"
for n in $LOADS; do
    ov=$($PIN "$W/vm_$n/build/$n" 2>&1); oc=$($PIN "$W/c_$n/build/$n" 2>&1)
    [ "$ov" = "$oc" ] && ok "$n 双轨 stdout 一致（$ov）" || bad "$n 双轨 stdout 不一致 VM=[$ov] C=[$oc]"
done

echo
echo "[4] 相对基线比对"
run_compare "$only"

echo
echo "[5] 覆盖边界（如实登记）"
info "只覆盖**本机单核**负载；多核/IO/HTTP 服务型负载不在面内（M89 的 http_json 形态未纳入）"
info "冷构建（清 .rtcache）刻意不纳入：会污染同机其它门/构建的缓存，代价不可控"
info "基线绝对值只对**同机同工具链**有意义 —— 换机器/换 gcc/换 px 版本须 --update 重定基"
info "C 轨只取 fib28_c/while_sum_c 作比值锚（全谱见 docs/PERF_BASELINE_V2.md §五）"
info "本门**不覆盖** px run（解释轨）面 —— 那是 M268 的活；字符串 += O(n²) 是 M270"

echo
echo "══ M267 结果：$PASS 通过 / $FAILN 失败 ══"
[ "$FAILN" = 0 ] && echo "M267-VERIFY-OK"
exit $([ "$FAILN" = 0 ] && echo 0 || echo 1)
