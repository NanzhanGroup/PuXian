#!/usr/bin/env bash
# ============================================================
# examples/m318_mem_layers/verify.sh
# ------------------------------------------------------------
# M318（第 190 轮）：**内存分层可观测性** —— 回答「这几十/几百 MB 是哪一层在涨」
#
# 来历（本机实测，本轮侦察）：
#   同一个「反代 3000 请求」装置，两种 GC 档位下的读数差两个量级：
#     · 关闭档（PX_GC_MAX_INTERVAL_MS=0）：anon 45.8MB · slab 21.5MB · 对象 64794
#     · 开启档（interval=1000）        ：anon 26.5MB · slab 4.0MB  · 对象 8147 · **live 62KB**
#   ⇒ 「RSS 涨了」这句话在修前**无法进一步分解**：是活数据？是回收了没归还的页？还是非 GC 内存
#     （线程栈 / glibc arena）？M317 的 dump 只有 `rss_obj_live` 一个数。
#   三层模型（本门与 docs/GC_MEMORY_LAYERS.md 的同一口径）：
#     ① 活对象层  live_objs / live_bytes                  —— 真在用的数据
#     ② 分配器层  slab_bytes / slab_n / free_slots / fully_empty —— 已向 OS 要、未必在用
#     ③ 触发层    alloc_since_gc / trigger_bytes / max_interval_ms / runs 族 —— 下一次 GC 何时来
#
# 本轮**刻意不做**的事（否证记录，见 docs/GC_MEMORY_LAYERS.md §四）：
#   「自适应 GC 软上限」（≈ Go 的 GOGC：触发点取 2×live）。实测 live 只有 62KB，
#   而真正在触发回收的是**时间判据**（alloc_since_gc 峰值 1.9MB，从未接近字节阈值）⇒
#   把字节阈值从 512MiB 降到 8MiB **不会有任何行为变化**。先量后改，结论是「不值得改」。
#
# 判据层次：
#   [0] 前置：语料与源码在场（缺失即判红，不静默跳过）
#   [1] 静态：实现判据在位（size 计算纯读 / 两处 sweep 都累加 / slab 读数单一实现 /
#       native 已注册 / 名册已同步 / 容器创建即 PX_KEEP）
#   [2] 构建 + **三轨一致**（gc_diagnose 的契约面：字段数 / 类型 / 单位语义）
#   [3] 服务场景分层（编译轨 · 只对**编译轨**断言绝对值，且有数量级分离）
#   [4] 负控 A（关闭档 ⇒ [3] 的分层判据必须红 —— 证明判据有牙）
#   [5] 负控 B（判据自伤 ⇒ A 的红必须消失 —— 证明红来自比对而不是恒红）
#   [6] 覆盖边界（如实）
#
# 用法：verify.sh [--neg-skip] [--keep]
#   --neg-skip 跳过负控（CI 用；负控要跑两遍服务场景，约 +60s）
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M318_GATE_W:-$(mktemp -d /tmp/m318_gate.XXXXXX)}"
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
    [ "$KEEP_W" = "1" ] || rm -rf "$W"
}
trap cleanup EXIT INT TERM

RT="$ROOT/runtime/runtime.c"
echo "=== M318 门：内存分层可观测性（gc_diagnose / live_bytes）==="
echo "    工作区 $W"

# ── [0] 前置 ────────────────────────────────────────────────
echo "── [0] 前置（源码与语料在场）"
for f in "$RT" "$HERE/diag.px" "$HERE/srv.px" "$HERE/proxy.px" "$HERE/drive.px"; do
    if [ ! -f "$f" ]; then bad "[0.1] 缺文件 $f"; echo "M318-VERIFY-FAIL"; exit 1; fi
done
ok "[0.1] 源码与四个语料在场"

# ── [1] 静态 ────────────────────────────────────────────────
echo "── [1] 静态（实现判据在位）"

# 1.1 gc_obj_size 存在且是**纯读**（不调 px_type_name —— M257：它会走 px_dict_get 互递归）
#     ⚠️ 必须**剥掉注释再判**：函数体的**行尾注释**里就写着「同一次 xmalloc」与「不调 px_type_name」
#        ⇒ 只剔整行注释不够（首版就是这么踩的：M251/M293 老坑第 N 次）。
c=$(awk '/^static long long gc_obj_size\(const LXObject\* o\) \{/,/^\}/' "$RT" \
    | sed 's://.*::' \
    | grep -c 'px_type_name\|xmalloc\|px_str(' || true)
chk "[1.1] gc_obj_size 是纯读（去注释后无 px_type_name / 分配调用）" "$c" "0"

# 1.2 gc_obj_size 覆盖了容器族（STR/BYTES/LIST/DICT/STRUCT/TUPLE/CHAN 七个 case）
c=$(awk '/^static long long gc_obj_size\(const LXObject\* o\) \{/,/^\}/' "$RT" | grep -cE '^ *case PX_(STR|BYTES|LIST|DICT|STRUCT|TUPLE|CHAN):' || true)
chk "[1.2] gc_obj_size 覆盖 7 个容器族 case" "$c" "7"

# 1.3 **两处 sweep** 都必须累加（并发路径 + 单线程路径）——
#     只加一处的后果：另一条路径上 live_bytes 永远是 0（静默假绿）
c=$(grep -c 'live_b += gc_obj_size(o);' "$RT" || true)
chk "[1.3] 两处 sweep 都累加存活字节（并发 + 单线程）" "$c" "2"
c=$(grep -c 'g_gc_live_bytes = live_b;' "$RT" || true)
chk "[1.3b] 两处出口都发布 live_bytes" "$c" "2"

# 1.4 slab 读数**单一实现**（M215 缺陷 305 / M230 缺陷 345 的老坑：同一语义两处写）
#     判据比「字符串只出现一次」更贴契约：`g_slab_range_count` 是**遍历边界**，
#     它只该出现在 gc_slab_totals 里 —— gcs_dump 里再有就是自己又写了一遍。
c=$(awk '/^static void gcs_dump\(const char\* why\) \{/,/^\}/' "$RT" | grep -c 'g_slab_range_count' || true)
chk "[1.4] gcs_dump 不再直接遍历 slab（统一走 gc_slab_totals）" "$c" "0"
c=$(awk '/^static void gc_slab_totals/,/^\}/' "$RT" | grep -c 'g_slab_range_count' || true)
chkge "[1.4b] gc_slab_totals 承担遍历" "$c" "1"
c=$(grep -c 'gc_slab_totals(&st)' "$RT" || true)
chk "[1.4c] 两个消费方都用它（gcs_dump + gc_diagnose）" "$c" "2"

# 1.5 native 注册 + 名册同步（M235s1 缺陷 355：新增 native 必须同步四个派生生成物）
c=$(grep -c 'px_set_global("gc_diagnose", px_native("gc_diagnose", bi_gc_diagnose));' "$RT" || true)
chk "[1.5] gc_diagnose 已注册" "$c" "1"
c=$(grep -c 'gc_diagnose' "$ROOT/tools/lint_core.px" || true)
chkge "[1.5b] lint 名册含 gc_diagnose" "$c" "1"
c=$(grep -c '"gc_diagnose"' "$ROOT/docs/native_index.json" || true)
chkge "[1.5c] native_index.json 含 gc_diagnose" "$c" "1"

# 1.6 M206 纪律：容器创建后**立刻** PX_KEEP（构造函数体里 px_dict() 的下一句）
c=$(awk '/^static LXValue bi_gc_diagnose/,/^}/' "$RT" | grep -A1 'LXValue d = px_dict();' | grep -c 'PX_KEEP(d);' || true)
chk "[1.6] 诊断容器创建即 PX_KEEP（M206 缺陷 248 族）" "$c" "1"

# 1.7 触发层字段齐全（14 个键 ⇒ 少一个就是契约变更）
c=$(awk '/^static LXValue bi_gc_diagnose/,/^}/' "$RT" | grep -c 'px_dict_set(d, "' || true)
chk "[1.7] 诊断输出 14 个字段" "$c" "14"

# ── [2] 构建 + 三轨一致 ─────────────────────────────────────
echo "── [2] 构建（devbuild pxc + 语料）+ 三轨一致"
PXC="/tmp/pxcdev"
if ! "$ROOT/selfhost/devbuild.sh" pxc > "$W/devb.log" 2>&1; then
    bad "[2.0] devbuild pxc 失败（尾 3 行：$(tail -3 "$W/devb.log" | tr '\n' ' ')）"
    echo "M318-VERIFY-FAIL"; exit 1
fi
[ -x "$PXC" ] || PXC="/tmp/pxcdev"
ok "[2.0] devbuild pxc 就绪"

BUILD_OK=1
for f in diag srv proxy drive; do
    cp "$HERE/$f.px" "$W/$f.px"
    if ! PX_PXC_BIN="$PXC" "$ROOT/tools/px" build "$W/$f.px" > "$W/build_$f.log" 2>&1; then
        bad "[2.1] 构建 $f.px 失败（尾 2 行：$(tail -2 "$W/build_$f.log" | tr '\n' ' ')）"
        BUILD_OK=0
    fi
done
if [ "$BUILD_OK" = 1 ]; then
    ok "[2.1] 四个语料构建通过"
    # 三轨：解释轨 / VM 轨（默认产物）/ C 轨
    "$ROOT/bootstrap/pxi" "$W/diag.px" > "$W/diag_interp.out" 2>&1 || true
    if [ -x /tmp/pxidev ]; then /tmp/pxidev "$W/diag.px" > "$W/diag_interp.out" 2>&1 || true; fi
    "$W/build/diag" > "$W/diag_vm.out" 2>&1 || true
    if PX_PXC_BIN="$PXC" "$ROOT/tools/px" build "$W/diag.px" --c > "$W/build_diag_c.log" 2>&1; then
        "$W/build/diag" > "$W/diag_c.out" 2>&1 || true
    else
        : > "$W/diag_c.out"
    fi
    if [ -s "$W/diag_interp.out" ] && [ -s "$W/diag_vm.out" ] && [ -s "$W/diag_c.out" ]; then
        if diff -q "$W/diag_interp.out" "$W/diag_vm.out" >/dev/null 2>&1; then
            ok "[2.2] 解释轨 == VM 轨（契约面逐字节一致）"
        else
            bad "[2.2] 解释轨 != VM 轨（diff 首行：$(diff "$W/diag_interp.out" "$W/diag_vm.out" | head -2 | tr '\n' ' ')）"
        fi
        if diff -q "$W/diag_vm.out" "$W/diag_c.out" >/dev/null 2>&1; then
            ok "[2.3] VM 轨 == C 轨（契约面逐字节一致）"
        else
            bad "[2.3] VM 轨 != C 轨（diff 首行：$(diff "$W/diag_vm.out" "$W/diag_c.out" | head -2 | tr '\n' ' ')）"
        fi
        # 判据：15 行输出（14 个字段 + 1 行 payload 自证），全部以 "D " 开头，无空行
        dcnt=$(wc -l < "$W/diag_vm.out")
        dpref=$(grep -c '^D ' "$W/diag_vm.out" || true)
        chk "[2.4] 诊断输出 15 行且全部是契约行" "$dcnt/$dpref" "15/15"
    else
        bad "[2.2] 三轨至少一轨无输出（interp=$(wc -l < "$W/diag_interp.out") vm=$(wc -l < "$W/diag_vm.out") c=$(wc -l < "$W/diag_c.out")）"
    fi
else
    bad "[2.2] 构建失败，三轨判据跳过"
fi

# ── 工具函数：跑一次服务场景，输出 KEY=VALUE ─────────────────
free_port() {   # 21000-21900（避开出站临时端口池 32768+，也与 m317 的 20000 段区分 ⇒ P1 端口互斥）
    local i p
    for i in $(seq 1 60); do
        p=$((21000 + RANDOM % 900))
        ss -ltn 2>/dev/null | grep -q ":$p " || { echo "$p"; return 0; }
    done
    echo $((21000 + RANDOM % 900))
}
statv() {  # statv <file> <key>  ⇒ 该 key 的**最后一次**读数（slab 行按 dump 锚定）
    case "$2" in
        slab)      grep -oE 'slab n=[0-9]+ map_bytes=[0-9]+KB' "$1" 2>/dev/null | tail -1 ;;
        live)      grep -oE 'live_bytes=[0-9]+KB' "$1" 2>/dev/null | tail -1 | cut -d= -f2 | tr -d 'KB' ;;
        objlive)   grep -oE 'rss_obj_live=[0-9]+' "$1" 2>/dev/null | tail -1 | cut -d= -f2 ;;
        slabkb)    grep -oE 'map_bytes=[0-9]+KB' "$1" 2>/dev/null | tail -1 | cut -d= -f2 | tr -d 'KB' ;;
        anon)      grep -oE 'anon=[0-9]+' "$1" 2>/dev/null | tail -1 | cut -d= -f2 ;;
    esac
}

# run_case <tag> <extra_env> <n>  ⇒ 写 $W/<tag>.env（live/slabkb/anondelta 三个 K=V）
run_case() {
    local tag="$1" xenv="$2" n="$3"
    local sp pp
    sp=$(free_port); pp=$(free_port)
    env PX_GC_STATS=1 PX_GC_STATS_INTERVAL=1 "$W/build/srv" "$sp" > "$W/srv_$tag.out" 2>&1 &
    SRV_PID=$!
    sleep 0.6
    # shellcheck disable=SC2086
    env PX_GC_STATS=1 PX_GC_STATS_INTERVAL=1 $xenv "$W/build/proxy" "$pp" "$sp" > "$W/pxy_$tag.out" 2>&1 &
    PXY_PID=$!
    sleep 0.9
    if ! kill -0 "$PXY_PID" 2>/dev/null; then
        echo "PROXY_DIED=1" > "$W/$tag.env"
        kill -9 "$SRV_PID" 2>/dev/null; SRV_PID=""; PXY_PID=""
        return 1
    fi
    "$W/build/drive" "$pp" "$PXY_PID" "$n" > "$W/drive_$tag.out" 2>&1
    sleep 1.3
    local live slab anon_start anon_end
    live=$(statv "$W/pxy_$tag.out" live);        live=${live:-0}
    slab=$(v=$(statv "$W/pxy_$tag.out" slabkb); echo "${v:-0}")
    anon_start=$(statv "$W/drive_$tag.out" anon)
    anon_end=$(grep -oE 'M317-DRV end +anon=[0-9]+' "$W/drive_$tag.out" 2>/dev/null | tail -1 | grep -oE '[0-9]+$')
    anon_start=${anon_start:-0}; anon_end=${anon_end:-0}
    {
        echo "LIVE_KB=$live"
        echo "SLAB_KB=$slab"
        echo "ANON_START_KB=$anon_start"
        echo "ANON_END_KB=$anon_end"
        echo "PROXY_DIED=0"
    } > "$W/$tag.env"
    kill -TERM "$PXY_PID" 2>/dev/null; wait "$PXY_PID" 2>/dev/null
    kill -TERM "$SRV_PID" 2>/dev/null; wait "$SRV_PID" 2>/dev/null
    PXY_PID=""; SRV_PID=""
    return 0
}

# 判据主体（**抽成函数** ⇒ 负控 B 可以拿它做「判据回放」对照）
#   layering_verdict <envfile> <live_max_kb> <slab_max_kb>  ⇒ 输出 "PASS n" / "FAIL n"
layering_verdict() {
    local ef="$1" lmax="$2" smax="$3"
    local passed=0 total=0
    # shellcheck disable=SC1090
    . "$ef"
    total=3
    [ "${PROXY_DIED:-1}" = "0" ] && passed=$((passed + 1))
    [ "${LIVE_KB:-999999}" -le "$lmax" ] 2>/dev/null && passed=$((passed + 1))
    [ "${SLAB_KB:-999999}" -le "$smax" ] 2>/dev/null && passed=$((passed + 1))
    if [ "$passed" = "$total" ]; then echo "PASS $passed"; else echo "FAIL $passed"; fi
}

# 第②层的「档位敏感」判据（**相对判据**，跨机器稳定 —— 绝对阈值会随机器/负载漂移，
#   首版用 slab≤16MB 做负控，实测 2000 请求的关闭档只有 14.9MB ⇒ 负控判绿，白跑一轮）。
#   closing <closed_slab_kb> <open_slab_kb> <factor>  ⇒ "PASS"/"FAIL"
slab_tier_verdict() {
    local c="$1" o="$2" f="$3"
    [ "$o" -le 0 ] 2>/dev/null && { echo "FAIL"; return; }
    if [ "$c" -ge $((o * f)) ] 2>/dev/null; then echo "PASS"; else echo "FAIL"; fi
}

# ── [3] 服务场景分层（编译轨 · 开启档）──────────────────────
echo "── [3] 服务场景分层（开启档 interval=1000 · 2000 请求）"
OPEN_OK=0
if [ "$BUILD_OK" = 1 ]; then
    if run_case open "PX_GC_MAX_INTERVAL_MS=1000" 2000; then
        OPEN_OK=1
        # shellcheck disable=SC1090
        . "$W/open.env"
        echo "     读数：live=${LIVE_KB}KB slab=${SLAB_KB}KB anon=${ANON_START_KB}→${ANON_END_KB}KB"
        # 判据取值依据（实测 3000 请求：live 62KB · slab 4.0MB）——阈值留 4~8× 余量：
        #   live 峰值随请求体大小而变，但「活数据是百 KB 量级」这条契约必须成立；
        #   slab 是分配器总量，3000 请求实测 4.0MB，取 16MB 上限（仍远小于关闭档的 21.5MB）。
        v=$(layering_verdict "$W/open.env" 4096 16384)
        chk "[3.1] 开启档三层判据（进程存活 · live≤4MB · slab≤16MB）" "$v" "PASS 3"
        # 「RSS 大 ≠ 活数据多」这条口径本身要成立（slab 必须显著小于 RSS 增量）
        rssdelta=$(( ANON_END_KB - ANON_START_KB ))
        if [ "$rssdelta" -gt 0 ] 2>/dev/null; then
            if [ "$SLAB_KB" -lt "$rssdelta" ] 2>/dev/null; then
                ok "[3.2] 分层有意义：slab(${SLAB_KB}KB) < RSS 增量(${rssdelta}KB) ⇒ 剩余部分是**非 GC 内存**"
            else
                bad "[3.2] slab(${SLAB_KB}KB) ≥ RSS 增量(${rssdelta}KB) —— 分层模型不成立（RSD 未涨或 slab 统计错）"
            fi
        else
            ok "[3.2] RSS 增量 ≤ 0（平台化）⇒ 分层判据由 [3.1] 承担"
        fi
        # 活对象必须**远小于**分配器（否则「分层」无意义：活数据就是全部）
        if [ "$LIVE_KB" -gt 0 ] 2>/dev/null && [ "$SLAB_KB" -gt 0 ] 2>/dev/null; then
            ratio=$(( SLAB_KB / (LIVE_KB > 0 ? LIVE_KB : 1) ))
            chkge "[3.3] slab / live ≥ 3（分配器与活数据的量级差）" "$ratio" "3"
        else
            bad "[3.3] live/slab 读数为 0（GC 未跑或 stats 未输出）"
        fi
    else
        bad "[3.0] 开启档服务场景未跑起来（proxy died）"
    fi
else
    bad "[3.0] 构建失败，服务场景跳过"
fi

# ── [4] 负控 A：关闭档 ⇒ 第②层必须显著变大 ────────────────
echo "── [4] 负控 A（关闭档 PX_GC_MAX_INTERVAL_MS=0 ⇒ 第②层读数必须显著变大）"
NEG_OK=0
OPEN_SLAB=0
if [ "$OPEN_OK" = 1 ]; then
    # shellcheck disable=SC1090
    OPEN_SLAB=$(grep '^SLAB_KB=' "$W/open.env" | cut -d= -f2)
    OPEN_SLAB=${OPEN_SLAB:-0}
fi
if [ "$NEG_SKIP" = "1" ]; then
    echo "     ⏭  --neg-skip（CI 档）：跳过"
elif [ "$BUILD_OK" = 1 ] && [ "$OPEN_OK" = 1 ]; then
    if run_case closed "PX_GC_MAX_INTERVAL_MS=0" 2000; then
        # shellcheck disable=SC1090
        . "$W/closed.env"
        echo "     读数：live=${LIVE_KB}KB slab=${SLAB_KB}KB anon=${ANON_START_KB}→${ANON_END_KB}KB"
        v=$(slab_tier_verdict "${SLAB_KB:-0}" "$OPEN_SLAB" 2)
        case "$v" in
            PASS) ok "[4.1] 关闭档 slab(${SLAB_KB}KB) ≥ 2× 开启档(${OPEN_SLAB}KB) ⇒ 判据对档位敏感" ; NEG_OK=1 ;;
            *)    bad "[4.1] 关闭档 slab(${SLAB_KB}KB) < 2× 开启档(${OPEN_SLAB}KB) —— GC 档位未影响第②层（判据无牙）" ;;
        esac
    else
        bad "[4.0] 关闭档服务场景未跑起来"
    fi
else
    bad "[4.0] 构建或开启档失败，负控跳过"
fi

# ── [5] 负控 B：判据回放 ⇒ 用「开启档自己的读数」冒充关闭档，必须判红 ──
#     判据回放 > 「判据自伤」：它证明的是**同一条判据**在「档位确实没变化」时会判红，
#     而不是「把阈值改坏之后判据失效」（后者只能证明阈值是可改的）。
echo "── [5] 负控 B（判据回放：把开启档自己的 slab 当关闭档喂给 [4.1] ⇒ 必须判红）"
if [ "$NEG_SKIP" = "1" ]; then
    echo "     ⏭  --neg-skip（CI 档）：跳过"
elif [ "${OPEN_SLAB:-0}" -gt 0 ] 2>/dev/null; then
    v=$(slab_tier_verdict "$OPEN_SLAB" "$OPEN_SLAB" 2)
    case "$v" in
        FAIL) ok "[5.1] 回放判红（倍数 1 < 2）⇒ [4.1] 的红确实来自比对，不是恒绿" ;;
        *)    bad "[5.1] 回放竟判绿 ⇒ [4.1] 的判据不可信" ;;
    esac
    # 反向：把关闭档读数放大 3 倍喂给它 ⇒ 必须判绿（证明它不恒红）
    v=$(slab_tier_verdict $((OPEN_SLAB * 3)) "$OPEN_SLAB" 2)
    case "$v" in
        PASS) ok "[5.2] 反向回放判绿（3×）⇒ [4.1] 不恒红" ;;
        *)    bad "[5.2] 反向回放仍判红 ⇒ [4.1] 恒红（判据写死）" ;;
    esac
else
    bad "[5.0] 无开启档读数，负控 B 跳过"
fi

# ── [6] 覆盖边界 ────────────────────────────────────────────
echo "── [6] 覆盖边界（如实登记）"
cat <<'EOF'
  · **只对编译轨断言绝对值**：解释轨自带 AST 堆（实测 live 646KB vs 编译轨 103KB），
    绝对值不可跨实现比较 ⇒ [2] 层只比契约面（布尔判据），[3] 层只比编译轨。
  · `live_bytes` 是**上一轮 GC 结束时**的快照（不是当前瞬时值）—— 两条触发路径都可能让它
    滞后于 `live_objs`（后者在 dump 时实时读）。这是 Go 的 GOGC 同款语义（用上一次 live 定下次触发点）。
  · **GC 从未跑过时 `live_bytes = 0`**（不是「活数据为 0」）—— 本轮负控实测：关闭档
    （`PX_GC_MAX_INTERVAL_MS=0` + 2000 请求）`live=0KB` 而 `slab=12988KB`。
    ⇒ 消费方必须把 0 读作「未知」，不能读作「没有数据」。这也是为什么 [4.1] 用第②层
    （slab）做档位判据，而不是用第①层。
  · `gc_obj_size` 对 dict/struct **不计键内容**（避免 sweep 从 O(对象数) 变成 O(总键数)）⇒
    live_bytes 对大键集场景是**下界**（实测偏差在 10% 内，本门不依赖该精度）。
  · 第 ③ 层「非 GC 内存」本门只**间接**度量（RSS 增量 − slab），不逐项拆解线程栈 / arena。
  · 本门不覆盖 QUIC/H3 连接的内存（那族由 m293/m294/m296 守着）。
EOF
ok "[6.1] 覆盖边界已登记"

# ── 汇总 ────────────────────────────────────────────────────
echo
if [ "$FAIL" = 0 ]; then
    echo "M318-VERIFY-OK（$PASS 通过 / 0 失败）"
    exit 0
else
    echo "M318-VERIFY-FAIL（$PASS 通过 / $FAIL 失败）"
    exit 1
fi
