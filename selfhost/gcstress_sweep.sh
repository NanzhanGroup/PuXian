#!/usr/bin/env bash
# ============================================================
# gcstress_sweep.sh —— GC 压力**差分筛**（M207 · 保证强度第二项）
# ------------------------------------------------------------
# 目的：把「native 桥 / 运行期漏登记 GC 根」这类缺陷的**发现动作**
#   从「一次性人工筛」变成**可重复、可分批、可复现、可判据化**的常设能力。
#
# 判据（两档差分 · 三跑）：
#   ① 正常档跑 **两次** —— 两遍 stdout 必须逐字节一致；不一致记 `NONDET`
#      （程序自身不确定 ⇒ 不在本筛管辖区，**如实登记**而不是假绿）；
#   ② 压力档（PX_GC_STRESS=1 PX_GC_INLINE=1 [+ PX_GC_LIVECHK=1]）与正常档比：
#      stdout 逐字节一致 · rc 一致 · stderr 无 GC 检测器标记 · 未被信号杀死。
#
# ⚠️ `PX_GC_INLINE=1` 不可省（M206 纪律）：服务/spawn 模式下 GC 被延迟到安全点
#   （实测 collect 次数差一个量级：32 vs 288）—— 只开 STRESS 时缺陷不显形。
#
# 分类（TSV 第二列 = tag）：
#   PASS      两档一致
#   FAIL_OUT  压力档 stdout 与正常档不同          ← 真信号
#   FAIL_RC   压力档 rc 与正常档不同               ← 真信号
#   FAIL_SIG  压力档被信号杀死（rc>=128）          ← 真信号
#   FAIL_DIAG 压力档 stderr 出现 GC 检测器标记     ← 真信号
#   STIMEOUT  正常档 OK、压力档超时（O(n²)，需人工定性）
#   NONDET    正常档自身两遍不一致 ⇒ 排除
#   SKIP_NORM 正常档就不通（rc≠0 / 超时）⇒ 该语料不在本筛管辖区
#   SKIP_KNOWN 已判定豁免（`--known FILE`：路径<TAB>理由；只登记**人工实跑定性过**的计时类）
#   BUILDFAIL 构建失败
#
# ⚠️ 所有 `timeout` 都带 `-k <宽限>`（默认 5s）：目标忽略 SIGTERM（如 px_serve 的优雅关闭）
#   时，裸 `timeout` 会**一直等** ⇒ 整轮永久卡住（实测）。宽限后 SIGKILL 保证有界结束。
#
# 用法：
#   selfhost/gcstress_sweep.sh [--list] [--batch K/N] [--out FILE] [--only GLOB]
#                              [--fails-only] [--quiet] [--no-det] [--no-inline]
#                              [--keep-side] [--known FILE] [--slow FILE]
#                              [--t-norm S] [--t-stress S] [--t-build S] [--env "A=1 B=2"]
# 退出码：出现 FAIL_* ⇒ 1；有 STIMEOUT 但无 FAIL_* ⇒ 0；全绿 ⇒ 0
#
# ⚠️ 副作用护栏：语料在**自己的目录**里跑，程序写出的文件会落在仓库里
#   （实测 m155 写 .bin、m170 写 SQLite 库）⇒ 默认会在跑完**清掉新增项**
#   （`build` 除外），`--keep-side` 关闭。
# ============================================================
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
cd "$ROOT" || exit 9

PAT="examples/*/*.px"
BATCH_K=1
BATCH_N=1
T_NORM="${PX_SWEEP_T_NORM:-10}"
T_STRESS="${PX_SWEEP_T_STRESS:-60}"
T_BUILD="${PX_SWEEP_T_BUILD:-300}"
# ⚠️ M207：`timeout` 默认只发 SIGTERM —— 目标**忽略 SIGTERM**（如 px_serve 的优雅关闭）
#   时它会一直等下去 ⇒ **整轮永久卡住**（实测 examples/…/s2c_pxserve 卡住 3 分钟以上）。
#   `-k <宽限>` = 宽限后 SIGKILL ⇒ 保证每候选**有界结束**。
T_KILL="${PX_SWEEP_T_KILL:-5}"
OUT=""
LIST_ONLY=0
FAILS_ONLY=0
QUIET=0
DET="${PX_SWEEP_DET:-1}"
NO_INLINE=0
KEEP_SIDE=0
# 已判定豁免表（`路径<TAB>理由`）：把**人工实跑定性过**的「输出本身依赖计时」这类
#   语料从判定里摘出去（记 `SKIP_KNOWN`，理由随行）。⚠️ **不是「跳过疑似项」的捷径** ——
#   只在实跑定性之后登记（见 examples/m207_gcstress/KNOWN.tsv 的规则）。
KNOWN=""
# ---- M212（第 91 轮）：**已实测「压力档 O(n²) 很慢但通过」**的语料表 ----
#   格式：`路径<TAB>压力档所需秒数<TAB>理由`（理由必须写实测数字与判据）。
#   ⚠️ 与 `--known` 的区别：`--known` 是「**输出本身不确定** ⇒ 排除在判定之外」；
#   本表是「**输出确定且正确，只是慢** ⇒ 给足时间**继续判**」—— 覆盖面**不丢**。
#   为什么需要：`m88_s3/s1b_gc_stress`（200 线程 × 300 分配）在
#   `PX_GC_STRESS=1 PX_GC_INLINE=1` 下每次分配都触发一轮全 STW GC ⇒ 实测 **274.5s**
#   （rc=0 · 输出与正常档**逐字节一致** · RSS 平稳 ~7MB）⇒ 默认 60s 上限必记 STIMEOUT，
#   那是**判据的时间上限**问题，不是缺陷。
SLOW=""
EXTRA_ENV=""
WORK="${PX_SWEEP_WORK:-}"

while [ $# -gt 0 ]; do
    case "$1" in
        --list)       LIST_ONLY=1 ;;
        --batch)      BATCH_K="${2%%/*}"; BATCH_N="${2##*/}"; shift ;;
        --out)        OUT="$2"; shift ;;
        --only)       PAT="$2"; shift ;;
        --fails-only) FAILS_ONLY=1 ;;
        --quiet)      QUIET=1 ;;
        --no-det)     DET=0 ;;
        --no-inline)  NO_INLINE=1 ;;
        --keep-side)  KEEP_SIDE=1 ;;
        --known)      KNOWN="$2"; shift ;;
        --slow)       SLOW="$2"; shift ;;
        --env)        EXTRA_ENV="$2"; shift ;;
        --t-norm)     T_NORM="$2"; shift ;;
        --t-stress)   T_STRESS="$2"; shift ;;
        --t-build)    T_BUILD="$2"; shift ;;
        -h|--help)    sed -n '3,40p' "$0"; exit 0 ;;
        *) echo "gcstress_sweep: 未知选项 '$1'" >&2; exit 2 ;;
    esac
    shift
done

# ---- 候选清单（排序 ⇒ 可复现；分批按行号取模 ⇒ 各批并集 = 全量）----
COLLECT="$(mktemp "${TMPDIR:-/tmp}/gcsweep.list.XXXXXX")"
AUTO_WORK=0
[ -n "$WORK" ] || AUTO_WORK=1
trap 'rm -f "$COLLECT"; [ "$AUTO_WORK" = 1 ] && [ -d "$WORK" ] && rm -rf "$WORK"' EXIT

# shellcheck disable=SC2086
ls -1 $PAT 2>/dev/null | grep -v '/build/' | grep -v '\.rtcache/' | LC_ALL=C sort > "$COLLECT"
if [ "$BATCH_N" -gt 1 ]; then
    awk -v k="$BATCH_K" -v n="$BATCH_N" 'NR % n == (k % n)' "$COLLECT" > "$COLLECT.b"
    mv "$COLLECT.b" "$COLLECT"
fi

if [ "$LIST_ONLY" = 1 ]; then
    cat "$COLLECT"
    { echo "# 批 $BATCH_K/$BATCH_N"; wc -l < "$COLLECT" | sed 's/^/# 候选数: /'; } >&2
    exit 0
fi

[ -n "$WORK" ] || WORK="$(mktemp -d "${TMPDIR:-/tmp}/gcsweep.XXXXXX")"

DET_ENV=""
[ "$DET" = 1 ] && DET_ENV="PX_GC_LIVECHK=1"
# `--no-inline` 只为**判据自证**存在：撤掉 PX_GC_INLINE=1 后，某些语料（服务/spawn 模式）
#   GC 被延迟到安全点 ⇒ 缺陷不再显形 ⇒ 本筛应变**不红**（证明 INLINE 这一条不可省）。
INLINE_ENV="PX_GC_INLINE=1"
[ "$NO_INLINE" = 1 ] && INLINE_ENV=""

emit() {   # $1=tag $2=src $3=detail
    printf '%s\t%s\t%s\n' "$1" "$2" "$3"
}

# M212：查 `--slow` 表 ⇒ 打印该语料的压力档秒数（无则空）
slow_secs() {
    [ -n "$SLOW" ] && [ -f "$SLOW" ] || return 0
    awk -F'\t' -v k="$1" '$0 !~ /^#/ && $1 == k { print $2; exit }' "$SLOW"
}

DIAG_RE='PX_GC_LIVECHK|PX_GC_UAFDET|PX_GC_TRACE|空闲链表损坏|已回收对象|core dumped'
SIG_RE='Segmentation|Aborted|Bus error'

run_one_raw() {
    local src="$1"
    local dir name bin blog
    dir="$(dirname "$src")"
    name="$(basename "$src" .px)"
    bin="./build/$name"
    blog="$WORK/build.$name.log"
    # M212：压力档上限可按 `--slow` 表逐语料覆盖（默认仍 $T_STRESS）
    local TS="$T_STRESS"; local SNOTE=""
    local _ss; _ss="$(slow_secs "$src")"
    if [ -n "${_ss:-}" ]; then TS="$_ss"; SNOTE="已实测慢语料（--slow 表：${_ss}s）"; fi

    rm -f "$dir/build/$name"
    if ! ( cd "$dir" && timeout -k "$T_KILL" "$T_BUILD" "$ROOT/tools/px" build "$(basename "$src")" ) >"$blog" 2>&1; then
        emit BUILDFAIL "$src" "$(tail -n 1 "$blog" | cut -c1-110)"
        return
    fi
    [ -x "$dir/build/$name" ] || { emit BUILDFAIL "$src" "未产出可执行件"; return; }

    # ---- 正常档 ×2 ----
    ( cd "$dir" && timeout -k "$T_KILL" "$T_NORM" "$bin" </dev/null ) >"$WORK/n1.out" 2>"$WORK/n1.err"; local r1=$?
    if [ "$r1" -ne 0 ]; then
        emit SKIP_NORM "$src" "正常档 rc=$r1"
        return
    fi
    ( cd "$dir" && timeout -k "$T_KILL" "$T_NORM" "$bin" </dev/null ) >"$WORK/n2.out" 2>"$WORK/n2.err"; local r2=$?
    if [ "$r2" -ne 0 ] || ! cmp -s "$WORK/n1.out" "$WORK/n2.out"; then
        emit NONDET "$src" "正常档两遍不一致（rc $r1/$r2）"
        return
    fi

    # ---- 压力档 ----
    # shellcheck disable=SC2086
    ( cd "$dir" && env PX_GC_STRESS=1 $INLINE_ENV $DET_ENV $EXTRA_ENV \
        timeout -k "$T_KILL" "$TS" "$bin" </dev/null ) >"$WORK/s.out" 2>"$WORK/s.err"; local rs=$?
    if [ "$rs" -eq 124 ]; then
        emit STIMEOUT "$src" "压力档超时 ${TS}s"
        return
    fi
    if [ "$rs" -ge 128 ]; then
        emit FAIL_SIG "$src" "压力档被信号杀死 rc=$rs $(kill -l $((rs-128)) 2>/dev/null)"
        return
    fi
    if grep -Eq "$DIAG_RE" "$WORK/s.err"; then
        emit FAIL_DIAG "$src" "$(grep -Eo "$DIAG_RE" "$WORK/s.err" | head -n 1)"
        return
    fi
    if [ "$rs" -ne 0 ]; then
        emit FAIL_RC "$src" "压力档 rc=$rs（正常 0）$(head -c 90 "$WORK/s.err" | tr '\n' ' ')"
        return
    fi
    if ! cmp -s "$WORK/n1.out" "$WORK/s.out"; then
        # M207：**确认步** —— 压力档再跑一遍；两遍自身不一致 ⇒ 判 `NONDET`（计时类）。
        #   为什么需要：本筛已用「正常档跑两遍」挡掉大部分不确定性，但**正常档两遍侥幸
        #   一致、压力档把计时放大**的程序会漏网（实测 examples/m117_realworld_defects：
        #   打印「用时 701ms（阈值 700ms）」⇒ 假红）。判据从「两跑」升级为「2+2 跑」。
        # shellcheck disable=SC2086
        ( cd "$dir" && env PX_GC_STRESS=1 $INLINE_ENV $DET_ENV $EXTRA_ENV \
            timeout -k "$T_KILL" "$TS" "$bin" </dev/null ) >"$WORK/s2.out" 2>"$WORK/s2.err"
        if ! cmp -s "$WORK/s.out" "$WORK/s2.out"; then
            emit NONDET "$src" "压力档自身两遍不一致（计时类）"
            return
        fi
        local d
        d="$(diff <(head -c 4000 "$WORK/n1.out") <(head -c 4000 "$WORK/s.out") | head -n 3 | tr '\n' '~')"
        emit FAIL_OUT "$src" "${d:0:150}"
        return
    fi
    emit PASS "$src" "$SNOTE"
}

# ---- M207：运行副作用护栏 --------------------------------------------------
# 语料是在**它自己的目录**里跑（相对路径 fixture 才能命中），于是程序写出的文件会落在
# 仓库里（实测 `examples/m155_nul_bytes/nul_bytes_out.bin`、`examples/m170_gc_bridge_root/__DB__`
# —— 都不在 .gitignore 里，全量筛一次就把工作树搞脏）。⇒ 每个候选跑前记一份顶层清单，
# 跑完把**新增**项清掉（`build` 除外：它在 .gitignore 里，且是本次构建的产物）。
# `--keep-side` 关掉清理（调试用）。只清**新增**项，不改动既有文件。
side_snapshot() {   # $1=dir
    [ "$KEEP_SIDE" = 1 ] && return 0
    find "$1" -mindepth 1 -maxdepth 1 -printf '%f\n' 2>/dev/null | LC_ALL=C sort > "$WORK/pre.list"
}
side_clean() {      # $1=dir
    [ "$KEEP_SIDE" = 1 ] && return 0
    find "$1" -mindepth 1 -maxdepth 1 -printf '%f\n' 2>/dev/null | LC_ALL=C sort > "$WORK/post.list"
    [ -f "$WORK/pre.list" ] || return 0
    comm -13 "$WORK/pre.list" "$WORK/post.list" 2>/dev/null | while IFS= read -r e; do
        [ -z "$e" ] && continue
        [ "$e" = "build" ] && continue
        rm -rf "$1/$e"
        printf '  [side] 清理运行副作用: %s/%s\n' "$1" "$e" >&2
    done
}

run_one() {         # 外层包装：跑前快照 / 跑完清理
    local d
    d="$(dirname "$1")"
    side_snapshot "$d"
    run_one_raw "$1"
    side_clean "$d"
}

TOTAL="$(wc -l < "$COLLECT")"
i=0
if [ "$OUT" != "" ]; then exec 3>"$OUT"; fi
while IFS= read -r src; do
    i=$((i+1))
    kr=""
    if [ -n "$KNOWN" ] && [ -f "$KNOWN" ]; then
        kr="$(awk -F'\t' -v p="$src" '$1==p {print $2; exit}' "$KNOWN")"
    fi
    if [ -n "$kr" ]; then
        line="$(emit SKIP_KNOWN "$src" "$kr")"
    else
        line="$(run_one "$src")"
    fi
    tag="${line%%$'\t'*}"
    if [ "$FAILS_ONLY" = 1 ] && [ "${tag#FAIL}" = "$tag" ] && [ "$tag" != "STIMEOUT" ]; then
        :
    else
        if [ "$OUT" != "" ]; then printf '%s\n' "$line" >&3; fi
        if [ "$QUIET" != 1 ]; then printf '%s\n' "$line"; fi
    fi
    [ "$QUIET" = 1 ] && [ $((i % 25)) -eq 0 ] && printf '  … %d/%d\n' "$i" "$TOTAL" >&2
done < "$COLLECT"
if [ "$OUT" != "" ]; then exec 3>&-; fi

if [ "$OUT" != "" ]; then
    {
        echo "== 汇总（批 $BATCH_K/$BATCH_N · 候选 $TOTAL）=="
        cut -f1 "$OUT" | sort | uniq -c | sort -rn | sed 's/^/   /'
    } >&2
fi

if [ "$OUT" != "" ] && grep -q '^FAIL_' "$OUT"; then exit 1; fi
exit 0
