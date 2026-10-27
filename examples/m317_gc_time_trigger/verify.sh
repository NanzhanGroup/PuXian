#!/usr/bin/env bash
# ============================================================
# examples/m317_gc_time_trigger/verify.sh
# ------------------------------------------------------------
# M317（第 188 轮 · 缺陷 551）：**长期在线服务的 GC 永不触发 ⇒ 内存线性上涨**
#
# 来历（晨曦 2026-10-20 总线 m18e03f17a5fc57eae3702a2e + 附件 f7d26044… + 2026-10-21
#   复测包 fc9c8795…）：Ma 生产 16 实例低流量节点 8.5h 涨 234MB，且「GC 跑过之后内存
#   也不还给 OS」。他这一轮自己推翻了一半表述（每请求那部分是**可回收垃圾**，阈值一调小
#   就平台化），并把根因指向两条：P0「GC 后空闲页不归还」、P1「512MiB 字节阈值在低流量下
#   等于不存在」。
#
# 本机独立复现（M317 归因，见 docs/GC_TRIGGER.md）：
#   · 反代路径 **10.2KB/请求**，6000 请求后 `runs=0`（7.4MB → 68.8MB，斜率不变）；
#   · 同负载把字节阈值调小立刻平台化（4MiB ⇒ 27MB / 256KiB ⇒ 23.6MB）；
#   ⇒ 是「该回收的没回收」，不是泄漏。而两条既有触发判据（对象数 1M / 字节 512MiB）
#     都是**批处理尺度**的旋钮：8.9 个对象/请求 ⇒ 十万级请求；7.6KB/s ⇒ 19 小时。
#
# 修复（runtime.c，只改「何时回收」，回收语义一字未动）：
#   ① 距上次 GC ≥ PX_GC_MAX_INTERVAL_MS（默认 30000）**且** 
#   ② 自上次 GC 分配 ≥ PX_GC_TIME_FLOOR_BYTES（默认 1MiB）⇒ 触发
#   ③ 或：上次 GC 后仍有「非头空 slab」待归还 ⇒ 允许**空闲时**继续推进（判据在 px_gc_poll）
#   ④ px_serve 池 worker #0 每秒醒一次（pthread_cond_timedwait）做安全点，否则流量一停
#      就再没有人调用 px_gc_poll ⇒ ISSUE28-B2 的 SLAB_FREE_STREAK(4) 轮永远到不了
#   ⑤ PX_GC_STATS dump 补 heap / slab 两行（晨曦明确提的 P1：没有 alloc/trigger 读数，
#      只能靠「首次 GC 落点」反推，而落点受其它分配影响、实测波动 ±35%）
#
# 判据层次：
#   [0] 前置：待测源码与语料在场（缺失即判红，不静默跳过）
#   [1] 静态：五处实现判据在位（时间判据 / 增量下限 / 空闲推进 / 池心跳 / dump 增行）
#   [2] 构建：devbuild pxc（用**当前源码** runtime）+ 三个语料
#   [3] 关闭档（PX_GC_MAX_INTERVAL_MS=0）：**必须复现线性增长**（证明判据有牙：
#       本门能量出「不修会怎样」，而不是永远绿）
#   [4] 开启档（interval=1s / floor=64KiB）：**后半程斜率必须塌下来** + time_runs ≥ 2
#   [5] 空闲心跳（零流量 / floor=0）：time_runs ≥ 2 且 idle_runs ≥ 2，且 0 ≤ tr-ir ≤ 1
#       （差额判据取代原「严格相等」—— 缺陷 599：time_runs 有两个生产者，见 [5] 段注释）
#       —— 这是「流量停了也要能把页还回去」的唯一机制，零流量下只有池 worker 心跳能驱动
#   [6] 页归还（数量级判据）：峰值后静置，RssAnon 必须**下降** ≥ 1MB
#   [7] 负控 A/B/C/D：撤时间判据 ⇒ [4] 红；撤心跳 ⇒ [5] 红；判据自伤 ⇒ A 的红消失；
#       注入无配对的 time_run ⇒ [5.3] 红（差额判据的牙口）
#   [8] 覆盖边界（如实）
#
# ⚠️ 判据取**服务端**的 /proc/<pid>/status（缺陷在服务端；采客户端会把两件事混在一起）。
# ⚠️ 判据用**斜率**（字节/请求）而不是绝对值 —— 绝对值随 GC 轮次抖动（实测同一装置
#   141MB → 33MB），而斜率在「关闭档」与机器快慢无关。
# ⚠️ 负控会临时改仓库里的 runtime.c ⇒ source gate_lock.sh（M276）+ trap 还原。
#
# 用法：verify.sh [--neg-skip] [--keep]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M317_GATE_W:-$(mktemp -d /tmp/m317_gate.XXXXXX)}"
NEG_SKIP=0
KEEP_W=0
for a in "$@"; do
    [ "$a" = "--neg-skip" ] && NEG_SKIP=1
    [ "$a" = "--keep" ] && KEEP_W=1
done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
chkge() { if [ "$2" -ge "$3" ] 2>/dev/null; then ok "$1（$2 ≥ $3）"; else bad "$1：期望 ≥ $3，实际 $2"; fi; }
chkle() { if [ "$2" -le "$3" ] 2>/dev/null; then ok "$1（$2 ≤ $3）"; else bad "$1：期望 ≤ $3，实际 $2"; fi; }

SRV_PID=""; PXY_PID=""
cleanup() {
    [ -n "$PXY_PID" ] && kill -9 "$PXY_PID" 2>/dev/null
    [ -n "$SRV_PID" ] && kill -9 "$SRV_PID" 2>/dev/null
    [ -f /tmp/m317_negctl_runtime.c.bak ] && python3 "$HERE/negctl.py" --restore >/dev/null 2>&1
    [ "$KEEP_W" = "1" ] || rm -rf "$W"
}
trap cleanup EXIT INT TERM

echo "=== M317 门：GC「时间 + 增量」触发 / 空闲页归还（缺陷 551）==="
echo "    工作区 $W"

# ── [0] 前置 ────────────────────────────────────────────────
echo "── [0] 前置"
MISS=0
for f in runtime/runtime.c selfhost/devbuild.sh tools/px \
         examples/m317_gc_time_trigger/srv.px \
         examples/m317_gc_time_trigger/proxy.px \
         examples/m317_gc_time_trigger/drive.px \
         examples/m317_gc_time_trigger/negctl.py; do
    [ -f "$ROOT/$f" ] || { bad "[0] 缺件：$f"; MISS=$((MISS + 1)); }
done
[ "$MISS" = 0 ] && ok "[0] 7 件前置齐备（含 devbuild / px 工具链）"

# ── [1] 静态：五处判据在位 ──────────────────────────────────
echo "── [1] 静态（五处实现判据）"
RT="$ROOT/runtime/runtime.c"

# 1.1 环境变量名在场
for v in PX_GC_MAX_INTERVAL_MS PX_GC_TIME_FLOOR_BYTES; do
    c=$(grep -c "getenv(\"$v\")" "$RT" || true)
    chk "[1.1] $v 有解析点" "$c" "1"
done
# 1.2 默认值（源码派生，不手抄数字）
defi=$(grep -oE '^#define GC_MAX_INTERVAL_MS_DEFAULT [0-9]+LL' "$RT" | awk '{print $3}' | tr -d 'L')
[ -n "$defi" ] && ok "[1.2] 时间触发的缺省值 = $defi ms" || bad "[1.2] 未找到 GC_MAX_INTERVAL_MS_DEFAULT"
chkge "[1.2b] 缺省间隔不松于 60s（越松高水位越高）" "$((defi <= 60000 ? 1 : 0))" "1"

# 1.3 need 判据（三条并列）
c=$(grep -c 'g_alloc_bytes >= g_gc_time_floor &&' "$RT" || true)
chk "[1.3] 时间 + 增量判据在 gc_register 的 need 里" "$c" "1"
c=$(grep -c 'if (!need && g_gc_max_interval_ms > 0' "$RT" || true)
chk "[1.3b] 该判据不覆盖既有的对象数/字节判据（!need 前置）" "$c" "1"

# 1.4 空闲推进（px_gc_poll）
c=$(grep -c 'g_gc_idle_reclaim, __ATOMIC_RELAXED' "$RT" || true)
chkge "[1.4] 空闲推进判据（idle_reclaim 读数）" "$c" "2"
c=$(grep -c 'g_gc_idle_reclaim, pend, __ATOMIC_RELAXED' "$RT" || true)
chk "[1.4b] idle_reclaim 的**置位点**在 slab_reclaim_empty 里" "$c" "1"

# 1.5 空闲 GC 心跳线程（两个 serve 形态都要挂）
c=$(grep -c 'pthread_create(&t, NULL, gc_tick_thread, NULL)' "$RT" || true)
chk "[1.5] 空闲 GC 心跳线程（gc_tick_thread）在场" "$c" "1"
c=$(grep -c 'px_gc_tick_start();' "$RT" || true)
chk "[1.5b] 两个 serve 入口都挂了心跳（fserve_ensure + px_serve）" "$c" "2"
c=$(grep -c 'px_io_sleep_ms(1000)' "$RT" || true)
chkge "[1.5c] 心跳按秒醒（不是忙等）" "$c" "1"

# 1.6 dump 增行（晨曦 P1）
c=$(grep -c 'heap alloc_since_gc=' "$RT" || true)
chk "[1.6] PX_GC_STATS 增 heap 行（alloc/trigger 可读）" "$c" "1"
# M319 收尾修：M318 把 slab 行从 %llu 改成了 %d、map_bytes 单位标 KB ⇒ 字面量锚点漂移（假红）。
#   判据口径不变：PX_GC_STATS 必须有一行 slab 读数（n / map_bytes / free_slots / fully_empty 四项齐）。
c=$(grep -cE 'slab n=%[a-z]+ map_bytes=%[a-z]+KB free_slots=%[a-z]+ fully_empty=%[a-z]+' "$RT" || true)
chk "[1.6b] PX_GC_STATS 增 slab 行（归还判据可读）" "$c" "1"

# 1.7 语义红线：只改「何时」，不改「回收什么」——S和T两处回收实现的入口一字未动
c=$(grep -c '^static void px_gc_collect_inline\|^void px_gc_collect' "$RT" || true)
chkge "[1.7] 回收实现入口仍在（本门不覆盖其内部语义）" "$c" "1"

# ── [2] 构建 ────────────────────────────────────────────────
echo "── [2] 构建（devbuild pxc 用当前源码 + 三个语料）"
PXC="$W/pxcdev"
if ! "$ROOT/selfhost/devbuild.sh" pxc > "$W/devb.log" 2>&1; then
    bad "[2.0] devbuild pxc 失败（尾 3 行：$(tail -3 "$W/devb.log" | tr '\n' ' ')）"
    echo "M317-VERIFY-FAIL"; exit 1
fi
[ -x "$PXC" ] || PXC="/tmp/pxcdev"
ok "[2.0] devbuild pxc 就绪（$PXC）"
BUILD_OK=1
for f in srv proxy drive; do
    cp "$HERE/$f.px" "$W/$f.px"
    if ! PX_PXC_BIN="$PXC" "$ROOT/tools/px" build "$W/$f.px" > "$W/build_$f.log" 2>&1; then
        bad "[2.1] 构建 $f.px 失败（尾 2 行：$(tail -2 "$W/build_$f.log" | tr '\n' ' ')）"
        BUILD_OK=0
    fi
done
[ "$BUILD_OK" = 1 ] && ok "[2.1] 三个语料构建通过（srv / proxy / drive）"

# ── 工具函数 ────────────────────────────────────────────────
free_port() {  # 20000-29999 内挑一个当前没在听的端口
    local i p
    for i in $(seq 1 60); do
        p=$((20000 + RANDOM % 9000))
        ss -ltn 2>/dev/null | grep -q ":$p " || { echo "$p"; return 0; }
    done
    echo "$((20000 + RANDOM % 9000))"
}

statv() {  # statv <file> <key> ⇒ 文件里该 key 的**最后一次**读数
    # ⚠️ `runs=` 是 `time_runs=` / `idle_runs=` 的子串 ⇒ 必须按 dump 行锚定，
    #    否则 `grep -oE 'runs=[0-9]+'` 会取到 idle_runs 的数字（判据会静默指错）。
    case "$2" in
        runs) grep -oE 'dump why=[a-z]+ runs=[0-9]+' "$1" 2>/dev/null | tail -1 | sed 's/.* runs=//' ;;
        *)    grep -oE "$2=[0-9]+" "$1" 2>/dev/null | tail -1 | cut -d= -f2 ;;
    esac
}

# pair_of <file> ⇒ "tr ir"：取最后一条**完整** dump 行里的两个计数
#   缺陷 599：不得用两次独立 grep 分别取 tr / ir —— 进程每秒还在写一行 dump，
#   两次读数可能来自不同行、甚至是半行 ⇒ 判据会拿「两次观测」当「一次观测」。
pair_of() {
    awk '{ if (match($0, /time_runs=[0-9]+ idle_runs=[0-9]+/)) {
             s = substr($0, RSTART, RLENGTH); split(s, a, " ");
             tr = substr(a[1], 11); ir = substr(a[2], 11); got = 1 } }
         END { if (got) printf "%s %s\n", tr, ir }' "$1" 2>/dev/null
}

# robust_pair <file> <n> <gap> ⇒ 末次「单调不减」的 "tr ir"
#   · 两个计数必须取自**同一条** dump 行（pair_of 保证）；
#   · 计数天然单调递增 ⇒ 「连续两次相等」永远不成立（首版就是这么错的：自旋导致永远拿不到）；
#     ⇒ 正确的稳定性口径是「多次采样均**不递减**」，递减即说明读到撕裂/旧行，丢弃该次。
#   · 要求 ≥ 2 次有效采样（只有一次读数不足以排除「读到旧行」）。
robust_pair() {
    local f="$1" n="${2:-5}" gap="${3:-0.4}" i cur tr ir ptr="" pir="" acc=0
    for i in $(seq 1 "$n"); do
        cur=$(pair_of "$f")
        sleep "$gap"
        [ -z "$cur" ] && continue
        tr=${cur% *}; ir=${cur#* }
        if [ -n "$ptr" ] && { [ "$tr" -lt "$ptr" ] || [ "$ir" -lt "$pir" ]; }; then
            continue
        fi
        ptr=$tr; pir=$ir; acc=$((acc + 1))
    done
    [ "$acc" -ge 2 ] && printf '%s %s\n' "$ptr" "$pir"
}

# run_case <tag> <extra_env> <n> <hold_secs>
#   起 srv+proxy → 采样前/后 → drive → 可选静置 → 输出 KEY=VALUE 到 $W/<tag>.env
run_case() {
    local tag="$1" xenv="$2" n="$3" hold="$4"
    local sp pp
    sp=$(free_port); pp=$(free_port)
    while [ "$pp" = "$sp" ]; do pp=$(free_port); done
    env PX_GC_STATS=1 PX_GC_STATS_INTERVAL=2 $xenv "$W/build/srv" "$sp" > "$W/srv_$tag.out" 2>&1 &
    SRV_PID=$!
    sleep 0.5
    env PX_GC_STATS=1 PX_GC_STATS_INTERVAL=2 $xenv "$W/build/proxy" "$pp" "$sp" > "$W/pxy_$tag.out" 2>&1 &
    PXY_PID=$!
    sleep 0.8
    # 就绪判据：端口真在听（不是只看日志）
    if ! ss -ltn 2>/dev/null | grep -q ":$pp "; then
        echo "M317-ERR $tag 服务未就绪（端口 $pp 未监听）" >&2
        echo "$tag-failed" > "$W/$tag.state"
        return 1
    fi
    local before after drop
    before=$(awk '/RssAnon/{print $2}' /proc/$PXY_PID/status 2>/dev/null)
    "$W/build/drive" "$pp" "$PXY_PID" "$n" > "$W/drv_$tag.log" 2>&1
    after=$(awk '/RssAnon/{print $2}' /proc/$PXY_PID/status 2>/dev/null)
    local res
    res=$(grep 'M317-DRV RESULT' "$W/drv_$tag.log" | tail -1)
    echo "$res" > "$W/$tag.res"
    if [ "$hold" -gt 0 ]; then
        sleep "$hold"
        drop=$(awk '/RssAnon/{print $2}' /proc/$PXY_PID/status 2>/dev/null)
        echo "M317-DRV hold anon=$drop" >> "$W/drv_$tag.log"
    fi
    # 收尾：先备份 dump（进程仍活着才能拿到内存里最新的统计）
    sleep 2.2   # 让周期性 dump 再打一次，确保末次读数包含收尾阶段的 GC
    cp "$W/pxy_$tag.out" "$W/pxy_$tag.snap"
    kill -9 "$PXY_PID" "$SRV_PID" 2>/dev/null
    PXY_PID=""; SRV_PID=""
    echo "before=$before after=$after hold_anon=${drop:-0}" > "$W/$tag.env"
    return 0
}

slope_of() {  # slope_of <tag> ⇒ slope2_bpr
    grep -oE 'slope2_bpr=[0-9-]+' "$W/$1.res" 2>/dev/null | cut -d= -f2
}

# ── [3] 关闭档：必须复现线性增长（判据有牙的前提）────────────
echo "── [3] 关闭档 PX_GC_MAX_INTERVAL_MS=0（必须线性增长 = 复现缺陷）"
if [ "$BUILD_OK" = 1 ] && run_case closed "PX_GC_MAX_INTERVAL_MS=0" 3000 0; then
    s=$(slope_of closed)
    chkge "[3.1] 关闭档后半程斜率 ≥ 4096 B/请求（实测修复前 ~10KB）" "${s:--1}" 4096
    runs=$(statv "$W/pxy_closed.snap" 'runs')
    chk "[3.2] 关闭档 GC 轮次 = 0（证明「低流量下两条判据都打不到」）" "${runs:--1}" "0"
    bad_=$(grep -oE 'bad=[0-9]+' "$W/closed.res" | cut -d= -f2)
    chk "[3.3] 关闭档请求全部 200（装置自身无错）" "${bad_:--1}" "0"
fi

# ── [4] 开启档：后半程斜率必须塌下来 ────────────────────────
echo "── [4] 开启档 interval=1s / floor=64KiB（后半程斜率必须塌下来）"
if [ "$BUILD_OK" = 1 ] && run_case open "PX_GC_MAX_INTERVAL_MS=1000 PX_GC_TIME_FLOOR_BYTES=65536" 3000 0; then
    s=$(slope_of open)
    chkle "[4.1] 开启档后半程斜率 ≤ 1024 B/请求" "${s:--1}" 1024
    tr=$(statv "$W/pxy_open.snap" 'time_runs')
    chkge "[4.2] 时间触发轮次 ≥ 1（GC 真的被时间判据驱动）" "${tr:--1}" 1
    # 与前档对比：必须显著更低（同一机器同一装置，只差一个 env）
    sc=$(slope_of closed); so=$(slope_of open)
    if [ -n "$sc" ] && [ -n "$so" ] && [ "$sc" -gt 0 ] 2>/dev/null; then
        ratio=$((so * 100 / sc))
        chkle "[4.3] 开启档斜率 ≤ 关闭档的 50%（$so ⇄ $sc B/请求 ⇒ ${ratio}%）" "$ratio" 50
    else
        bad "[4.3] 无法比较斜率（closed=$sc open=$so）"
    fi
fi

# ── [5] 空闲心跳：零流量下也要能推进 ───────────────────────
# ⚠️ 缺陷 599（判据不稳定族第 7 例）：原 [5.3] 断言「time_runs == idle_runs（严格相等）」，
#    但这两个计数**不是同一个事件的两面**：
#      · idle_runs 的生产者只有一处：px_gc_poll 的空闲心跳分支；
#      · time_runs 的生产者有两处：同一心跳分支 + gc_register 的「时间 + 增量」分配路径
#        （runtime.c 两处 __atomic_fetch_add(&g_gcs_time_runs, …)）。
#    ⇒ 只要启动期发生过一次「分配路径」的时间触发（500ms 间隔下需慢启动），tr 就**永久**
#      比 ir 大 1，严格相等必红 —— 与产品行为无关。4 分片并发下启动变慢正是这条的触发面。
#    实证（2026-10-27 本机）：
#      · 同装置把 interval 调到 1ms ⇒ 首条 dump 即 tr=2 ir=1（差额 1，机制确实存在）；
#      · 3x CPU 过载（48 hog / 16 核）跑 10 次 ⇒ 全 tr==ir（说明单靠过载不触发，需慢启动）。
#    ⇒ 根因定性到「两生产者 + 严格相等」，具体这一次的触发时刻**未证**（不再硬说）。
#    另：原判据的**读数方式**也违反「同一事实只读一次」：两次独立 grep，中间进程还在写。
#    处置：① 两个读数取自**同一条** dump 行；② 连续两次采样一致才算数；
#          ③ 判 0 ≤ tr-ir ≤ 1（允许 1 轮非心跳发起的 time_run）；tr < ir 直接判红
#             （同步递增的计数不可能反超 ⇒ 只可能是撕裂读）。
#    牙口：负控 D 注入「无心跳配对的 time_run」⇒ 差额每秒 +1 ⇒ [5.3] 必须红。
echo "── [5] 空闲心跳 floor=0 / interval=500ms（零流量，只有池 worker 心跳能驱动）"
if [ "$BUILD_OK" = 1 ]; then
    p=$(free_port)
    env PX_GC_STATS=1 PX_GC_STATS_INTERVAL=1 PX_GC_MAX_INTERVAL_MS=500 PX_GC_TIME_FLOOR_BYTES=0 \
        "$W/build/proxy" "$p" 1 > "$W/pxy_idle.out" 2>&1 &
    PXY_PID=$!
    sleep 4
    idpair=$(robust_pair "$W/pxy_idle.out" 5 0.4)
    tr=""; ir=""
    if [ -z "$idpair" ]; then
        bad "[5.0] 拿不到 ≥2 次「单调不减」的 dump 读数（同一条行内取值）—— 判据无法判定，按红处理"
    else
        tr=${idpair% *}; ir=${idpair#* }
        ok "[5.0] 采样有效（同一条 dump 行取值 · 多次单调不减）：time_runs=$tr idle_runs=$ir"
    fi
    kill -9 "$PXY_PID" 2>/dev/null; PXY_PID=""
    chkge "[5.1] 零流量下时间触发 ≥ 2 轮（4s / 0.5s）" "${tr:--1}" 2
    chkge "[5.2] 其中由**空闲心跳线程**发起 ≥ 2 轮（修复前无此线程：无 job 就无限期 cond_wait）" "${ir:--1}" 2
    if [ -n "$tr" ] && [ -n "$ir" ]; then
        dif=$(( tr - ir ))
        if [ "$dif" -lt 0 ]; then
            bad "[5.3] time_runs($tr) < idle_runs($ir)：同步递增的两个计数不可能反超 ⇒ 撕裂读或语义被改"
        elif [ "$dif" -le 1 ]; then
            ok "[5.3] 零流量下 time_runs 只由心跳驱动（tr=$tr · ir=$ir · 差额 $dif ≤ 1，允许 1 轮分配路径/在途）"
        else
            bad "[5.3] tr-ir = $dif > 1（tr=$tr ir=$ir）⇒ 有 $dif 轮时间触发不是心跳发起的"
        fi
    fi
fi

# ── [6] 页归还：峰值后静置，整块 slab 必须被 munmap 归还 ─────────────
echo "── [6] 页归还（峰值后静置 24s：整块空 slab 必须被归还）"
# 装置说明：interval=5s / floor=0 ⇒ 每 5s 一轮 GC；静置 24s 覆盖 SLAB_FREE_STREAK(4) 轮。
# ⚠️ 判据**不用 RssAnon 绝对值**：它包含 256 条线程栈（实测 13.7MB，且每个 worker 的
#    栈会被调用深度撑大 ~30KB）——那部分不可回收，会把「页归还」的信号淹没
#    （本轮实测：slab 从 7000+ 块降到 77 块，而 RssAnon 反而涨了 16MB）。
#    直接判**分配器自己的读数**：slab 块数 n 与 map_bytes（向 OS 要的总量）。
#    这两项只在 munmap 时下降 ⇒ 是「页真的还回去了」的直接证据。
if [ "$BUILD_OK" = 1 ] && run_case reclaim "PX_GC_MAX_INTERVAL_MS=5000 PX_GC_TIME_FLOOR_BYTES=0" 3000 24; then
    nmax=$(grep -oE 'slab n=[0-9]+' "$W/pxy_reclaim.snap" | cut -d= -f2 | sort -n | tail -1)
    nend=$(grep -oE 'slab n=[0-9]+' "$W/pxy_reclaim.snap" | cut -d= -f2 | tail -1)
    mbmax=$(grep -oE 'map_bytes=[0-9]+' "$W/pxy_reclaim.snap" | cut -d= -f2 | sort -n | tail -1)
    mbend=$(grep -oE 'map_bytes=[0-9]+' "$W/pxy_reclaim.snap" | cut -d= -f2 | tail -1)
    chkge "[6.1] 装置造出了可回收峰值（slab 块数峰值 ≥ 100）" "${nmax:-0}" 100
    dn=$(( ${nmax:-0} - ${nend:-0} ))
    dkb=$(( ${mbmax:-0} - ${mbend:-0} ))   # ⚠️ dump 里的 map_bytes 已经是 **KB**（runtime.c 的 sbytes/1024），不要再除 1024
    chkge "[6.2] 静置后整块 slab 被归还（块数下降 ≥ 20）" "$dn" 20
    chkge "[6.3] 归还量 ≥ 512KB（map_bytes 下降，即页真的还给 OS）" "$dkb" 512
    before=$(grep -oE 'before=[0-9]+' "$W/reclaim.env" | cut -d= -f2)
    hold=$(grep -oE 'hold_anon=[0-9]+' "$W/reclaim.env" | cut -d= -f2)
    echo "      ℹ️ slab 块数 ${nmax:-?} → ${nend:-?} · map_bytes ${mbmax:-?}KB → ${mbend:-?}KB · RssAnon ${before:-?}KB → ${hold:-?}KB（含不可回收的线程栈）"
fi

# ── [7] 负控 ────────────────────────────────────────────────
echo "── [7] 负控"
if [ "$NEG_SKIP" = 1 ]; then
    echo "  ⏭  负控已跳过（--neg-skip）"
else
    # 自证：锚点在场且唯一 + 拒绝重复打桩 + 还原逐字节一致
    if python3 "$HERE/negctl.py" --selftest > "$W/neg_self.log" 2>&1; then
        ok "[7.0] 打桩器自证（$(grep -oE 'ok=[0-9]+ fail=[0-9]+' "$W/neg_self.log")）"
    else
        bad "[7.0] 打桩器自证失败：$(tail -2 "$W/neg_self.log" | tr '\n' ' ')"
    fi

    rebuild_dev() {
        "$ROOT/selfhost/devbuild.sh" --rebuild pxc > "$W/devb_neg.log" 2>&1 || return 1
        for f in srv proxy drive; do
            PX_PXC_BIN="$PXC" "$ROOT/tools/px" build "$W/$f.px" >> "$W/negbuild.log" 2>&1 || return 1
        done
        return 0
    }

    # 负控 A：撤时间判据 ⇒ [4] 必须红（开启档退回线性增长）
    echo "   · 负控 A：撤「时间 + 增量」判据"
    python3 "$HERE/negctl.py" --apply A > "$W/negA_apply.log" 2>&1 || bad "[7.A0] 打桩失败：$(tail -1 "$W/negA_apply.log")"
    if rebuild_dev && run_case negA "PX_GC_MAX_INTERVAL_MS=1000 PX_GC_TIME_FLOOR_BYTES=65536" 3000 0; then
        s=$(slope_of negA)
        chkge "[7.A1] 撤判据后开启档斜率 ≥ 4096 B/请求（缺陷复现；A 撤**两条**触发路径）" "${s:--1}" 4096
        tr=$(statv "$W/pxy_negA.snap" 'time_runs')
        chk "[7.A2] 撤判据后 time_runs = 0（判据真的被撤掉了）" "${tr:--1}" "0"
    else
        bad "[7.A] 负控档运行失败（构建或就绪）"
    fi
    python3 "$HERE/negctl.py" --restore > /dev/null 2>&1

    # 负控 B：撤空闲心跳 ⇒ [5] 必须红（零流量下不再推进）
    echo "   · 负控 B：撤池 worker 空闲心跳"
    python3 "$HERE/negctl.py" --apply B > "$W/negB_apply.log" 2>&1 || bad "[7.B0] 打桩失败：$(tail -1 "$W/negB_apply.log")"
    if rebuild_dev; then
        p=$(free_port)
        env PX_GC_STATS=1 PX_GC_STATS_INTERVAL=1 PX_GC_MAX_INTERVAL_MS=500 PX_GC_TIME_FLOOR_BYTES=0 \
            "$W/build/proxy" "$p" 1 > "$W/pxy_negB.out" 2>&1 &
        PXY_PID=$!
        sleep 4
        ir=$(statv "$W/pxy_negB.out" 'idle_runs')
        tk=$(statv "$W/pxy_negB.out" 'tick')
        kill -9 "$PXY_PID" 2>/dev/null; PXY_PID=""
        # ⚠️ 撤心跳后「零流量」既可能读到 0，也可能**一条 dump 都没有**（-1）——
        #    后者同样是证据（dump 只由 gc_register 或 px_gc_poll 触发，两者都不跑）。
        if [ "${ir:--1}" -le 0 ] 2>/dev/null; then
            ok "[7.B1] 撤心跳后 idle_runs 无正数（读到 ${ir:--1}；零流量不再推进）"
        else
            bad "[7.B1] 撤心跳后 idle_runs 仍有正数（$ir）—— 心跳没被撤掉"
        fi
        if [ "${tk:--1}" -le 0 ] 2>/dev/null; then
            ok "[7.B2] 撤心跳后 tick 无正数（线程确实没建：${tk:--1}）"
        else
            bad "[7.B2] 撤心跳后 tick 仍有正数（$tk）—— 线程还在"
        fi
    else
        bad "[7.B] 负控档重建失败"
    fi
    python3 "$HERE/negctl.py" --restore > /dev/null 2>&1

    # 负控 D：[5.3] 的牙口 —— 注入「无心跳配对的 time_run」⇒ 差额 > 1 ⇒ [5.3] 必须红
    #   （缺陷 599 的配套：改判据必须同时证明「弱化后的判据还能红」）
    echo "   · 负控 D：[5.3] 判据牙口（每次心跳多算一轮无配对的 time_run）"
    python3 "$HERE/negctl.py" --apply D > "$W/negD_apply.log" 2>&1 || bad "[7.D0] 打桩失败：$(tail -1 "$W/negD_apply.log")"
    if rebuild_dev; then
        p=$(free_port)
        env PX_GC_STATS=1 PX_GC_STATS_INTERVAL=1 PX_GC_MAX_INTERVAL_MS=500 PX_GC_TIME_FLOOR_BYTES=0 \
            "$W/build/proxy" "$p" 1 > "$W/pxy_negD.out" 2>&1 &
        PXY_PID=$!
        sleep 4
        dpr=$(robust_pair "$W/pxy_negD.out" 5 0.4)
        kill -9 "$PXY_PID" 2>/dev/null; PXY_PID=""
        if [ -z "$dpr" ]; then
            bad "[7.D1] 负控 D 采样无效（拿不到 ≥2 次单调不减的 dump 读数）"
        else
            dtr=${dpr% *}; dir=${dpr#* }
            ddi=$(( dtr - dir ))
            if [ "$ddi" -gt 1 ]; then
                ok "[7.D1] 注入后差额 $ddi > 1（tr=$dtr ir=$dir）⇒ [5.3] 必红（判据有牙）"
            else
                bad "[7.D1] 注入后差额仍 = $ddi ≤ 1（tr=$dtr ir=$dir）⇒ [5.3] 没有牙"
            fi
            if [ "$dtr" -ge 2 ] && [ "$dir" -ge 2 ]; then
                ok "[7.D2] 同档 [5.1]/[5.2] 仍成立（tr=$dtr ir=$dir）⇒ 红只可能来自 [5.3]（隔离性）"
            else
                bad "[7.D2] 负控 D 档前提不成立（tr=$dtr ir=$dir，要求均 ≥ 2）"
            fi
        fi
    else
        bad "[7.D] 负控 D 档重建失败"
    fi
    python3 "$HERE/negctl.py" --restore > /dev/null 2>&1

    # 还原后：重建 + 关键判据必须回绿（证明还原保真、不是「一红到底」）
    if rebuild_dev && run_case back "PX_GC_MAX_INTERVAL_MS=1000 PX_GC_TIME_FLOOR_BYTES=65536" 3000 0; then
        s=$(slope_of back)
        chkle "[7.C1] 还原后开启档斜率回归 ≤ 1024 B/请求（还原保真）" "${s:--1}" 1024
    else
        bad "[7.C] 还原档运行失败"
    fi

    # 负控 C：判据自伤 —— 同一份「撤了判据」的二进制，换成**只看装置跑通**的宽松判据必须不再判红
    #   ⇒ 证明负控 A 的那个红来自「斜率比对」，不是来自崩溃 / 构建失败 / 就绪失败。
    python3 "$HERE/negctl.py" --apply A > /dev/null 2>&1
    if rebuild_dev && run_case harm "PX_GC_MAX_INTERVAL_MS=1000 PX_GC_TIME_FLOOR_BYTES=65536" 3000 0; then
        s=$(slope_of harm)
        bd=$(grep -oE 'bad=[0-9]+' "$W/harm.res" | cut -d= -f2)
        if [ "${s:--1}" -ge 4096 ] 2>/dev/null && [ "${bd:--1}" = "0" ]; then
            ok "[7.C2] 判据自伤：同一份线性增长（$s B/请求）在「只看装置跑通」的判据下不再判红（bad=0）"
            ok "[7.C3] ⇒ 负控 A 的红确实来自「斜率比对」"
        else
            bad "[7.C2] 自伤档前提不成立（slope=${s:-?} bad=${bd:-?}）"
        fi
    else
        bad "[7.C2] 自伤档运行失败"
    fi
    python3 "$HERE/negctl.py" --restore > /dev/null 2>&1
fi

# ── [8] 覆盖边界（如实）────────────────────────────────────
echo "── [8] 覆盖边界（如实登记）"
echo "  · 本门判的是**服务端**代理路径（http_serve 内 http_request）——即 Ma proxy_routes 的形状；"
echo "    纯入站 http_serve（handler 直接 200）在修前修后都是零增长，不在本门判据面内。"
echo "  · 只判「时间 + 增量」触发与「空 slab 归还」；**不判** slab 内部碎片（部分使用的 slab 无法"
echo "    munmap —— 实测平台态 ~10MB 属于这一类，属分配器形态问题，见 docs/GC_TRIGGER.md §5）。"
echo "  · [6] 判的是**分配器读数**（slab 块数 / map_bytes），不是 RssAnon —— 后者含 256 条线程栈，\n    会把页归还的信号淹没（实测 slab 7000→77 块而 RssAnon 反涨 16MB）。"
echo "  · 心跳覆盖两种 serve 形态（fserve_ensure / px_serve 各挂一次，判据 [1.5b] 守着）；"
echo "    未覆盖：**不 serve** 的长期运行形态（纯事件循环 / 自定义线程池）—— 它们没有心跳，"
echo "    空 slab 仍要等「下一次有分配」才会被推进（内存不会涨，但也不会主动回落）。"
echo "  · 未覆盖：GC 自身耗时随存活对象数增长（实测 ~500k 对象/120MB 时单轮 >100ms，"
echo "    PX_GC_STATS 的 slow 计数会涨）—— 属既有性能面，本门不设阈值。"

echo
if [ "$FAIL" = 0 ]; then
    echo "M317-VERIFY-OK（通过 $PASS 项）"
    exit 0
else
    echo "M317-VERIFY-FAIL（通过 $PASS · 失败 $FAIL）"
    exit 1
fi
