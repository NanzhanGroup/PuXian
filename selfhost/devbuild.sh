#!/usr/bin/env bash
# ============================================================
# devbuild.sh —— **开发期**快速自举构建（不碰入库件）
#   M116 建立（qg-issue 71 续）：改 `selfhost/*.px` 时需要一个"只用当前源码 +
#   现役入库 pxc"就能得到**新版工具**的回路，且**不覆盖** bootstrap/*（入库件只能
#   由 rebake_bin.sh 带指纹重烘，见 Issue 55/58）。
# 用法：
#   ./selfhost/devbuild.sh                 # 只构建 /tmp/pxcdev（C 轨编译器）
#   ./selfhost/devbuild.sh pxi pxl pxpar   # 额外构建 /tmp/pxi_dev 等
#   ./selfhost/devbuild.sh --vm            # 再生成 /tmp/pxcdev_vm（VM 轨，用户面默认）
# 产物：/tmp/pxcdev · /tmp/<name>_dev（+ 日志 /tmp/devbuild_*.log）
# 备注：单件 C 轨 ≈19s；--vm 追加 ≈90s（--emit-c + 链 VM 镜像）
# ⚠️ M219（缺陷 311）：**C 轨与 VM 轨都必须 `-static`**。本仓预置三方资产
#   （`sqlite3.o` / `libz.a` / `mbedtls/*.a`）是**非 PIC** 对象，在「默认 PIE」工具链
#   （Ubuntu / Debian gcc 的 `--enable-default-pie`）上会被 ld 拒绝：
#     relocation R_X86_64_32[S] against `.rodata' can not be used when making a PIE object
#   而 Red Hat 系 gcc 默认非 PIE ⇒ **开发机恒绿、CI 必红**（实测 CI run 36262322866）。
#   守卫：`selfhost/check_link_flags.sh`（静态扫全仓）；本机复现垫片：`selfhost/sim_pie_cc.sh`。
# ⚠️ M221（第 100 轮）：**产物指纹短路** —— 同一源码链下重复调用 ≈ 秒返回（打印 `⏭`）。
#   为什么：全量门 131 门里有 **23 门**各自 `devbuild.sh pxc pxi --vm`，实测单次（暖缓存）
#   ≈45s ⇒ **23×45 ≈ 17 分钟纯重复**，占 CI「工具自测」步（实测 23/40+/37min，上限 45min）
#   的 40–75%。而 23 门在 CI 里同一步、同一源码链、同一 .rtcache ⇒ 产物逐字节相同。
#   ⇒ 判据：产物存在 + 指纹 == 当前源码链指纹 ⇒ 复用。
#   ⚠️ 指纹**必须**含 `runtime/*.c,h` —— 门内负控会篡改它们（见 m216 门头注），
#      若只按「产物比源码新」判，负控的**重建**会被吃掉 ⇒ **假绿**（M169 老坑）。
#   ⚠️ 构建**失败**绝不写指纹；`--rebuild`（或 `DEVB_REBUILD=1`）⇒ 无条件重建。
# ============================================================
set -u
cd "$(dirname "$0")/.."
ROOT=$(pwd)
export LC_ALL=C LANG=C
RT="$ROOT/runtime"
BASE="$ROOT/bootstrap/pxc"

ENTRIES="pxc|selfhost/compiler.px|static
pxi|selfhost/interp.px|static
pxl|selfhost/lexer.px|static
pxpar|selfhost/parser.px|static
pxfmt|tools/pxfmt.px|static
pxlint|tools/pxlint.px|static"

entry_src() { echo "$ENTRIES" | grep "^$1|" | cut -d'|' -f2; }

# ── M222 补：复用台账（**让「短路到底有没有生效」可观测**）─────────────────
#   背景：M221 的产物指纹短路在**本机微测**上确证有效（重建 17s → 复用 0s），
#   但 CI 的「工具自测」步时长 1337s → 1657s（同时新增了两个门）⇒ **净收益无法从
#   单次样本里隔离**。CI 的 job 日志非管理员 403 ⇒ 唯一可读通道是注解。
#   ⇒ 每次 devbuild 调用把「重建 / 复用」记一行台账；`devb_summary` 打印累计，
#     再由 ci.yml 的一个 `if: always()` step 合成 `::notice::`。
#   ⚠️ 台账只**追加**、不影响任何判据；`DEVB_STATS` 可覆盖路径。
DEVB_STATS_FILE="${DEVB_STATS:-/tmp/devbuild_stats.tsv}"
devb_stat() {   # $1=reuse|rebuild  $2=件名  $3=key  [$4=选料来源]
    printf '%s\t%s\t%s\t%s\t%s\n' "$(date +%s)" "$1" "$2" "$3" "${4:-${CACHE_SRC:-?}}" \
        >>"$DEVB_STATS_FILE" 2>/dev/null || true
}
# ── M223（缺陷 325）：汇总**必须给出 key 的取值** ──────────────────────
#   修前只报「出现过的 key N 个」⇒ CI 上实测「3 个」，但**哪 3 个、为什么是 3 个**
#   无从得知（本仓 job 日志 403，注解是唯一通道 ⇒ 注解里必须自带证据）。
#   ⚠️ 计数口径也有坑：vm 的 key 是「<pxc_key>/<pxcdev_sha>」= pxc key 的**派生**，
#      按字符串去重会把 A 与 A/B 算成**两个独立 key** ⇒ 数字被虚增。
#      ⇒ 这里**按件名分组列出取值**，一眼能看出"是漂移，还是派生"。
devb_summary() {
    [ -f "$DEVB_STATS_FILE" ] || return 0
    local r b
    r=$(grep -c $'\treuse\t'   "$DEVB_STATS_FILE" 2>/dev/null || true)
    b=$(grep -c $'\trebuild\t' "$DEVB_STATS_FILE" 2>/dev/null || true)
    echo "── devbuild 累计：重建 ${b:-0} 次 / 复用 ${r:-0} 次（台账 $DEVB_STATS_FILE）"
    # ── M250（缺陷 434）：**汇总必须有上限** ──────────────────────────────
    #   台账是**只追加**的长期文件（本机实测 4712 行 ⇒ 相异 (件,key) 组上百）。
    #   修前这里对每一组打印一行 ⇒ 每次 devbuild 调用吐 2000+ 行 ⇒
    #   ① 门日志被淹没（真信号被埋）；② ci.yml 把它合成 `::notice::` ⇒ 注解逼近长度上限。
    #   现在：打印 **Top-N**（按出现次数降序）+ 明确报出「共几组、略去几组」。
    #   ⚠️ 一律不静默截断 —— 略去的**数量**必须打出来（否则读者会以为这就是全部）。
    local topn="${DEVB_SUMMARY_TOP:-12}"
    local all tot
    all=$(cut -f3,4 "$DEVB_STATS_FILE" 2>/dev/null | sort | uniq -c | sort -k1,1nr -k2,2)
    tot=$(printf '%s\n' "$all" | grep -c . || true)
    echo "── 源码链 key（按件分组 · 取值 ⇒ 可归因）· 共 ${tot:-0} 组，列出前 ${topn} 组"
    printf '%s\n' "$all" | head -n "$topn" | while read -r cnt nm kv; do
        echo "     $nm  $kv  ×$cnt"
    done
    if [ "${tot:-0}" -gt "$topn" ]; then
        echo "     …（略去 $((tot - topn)) 组 —— 完整清单见台账；本行**不许**被当成全部）"
    fi
    local cs
    cs=$(cut -f5 "$DEVB_STATS_FILE" 2>/dev/null | sort | uniq -c | awk '{printf "%s×%s ", $2, $1}')
    echo "── 选料来源：${cs:-（本台账无第 5 列 —— 旧版 devbuild 写的）}"
}

# 选 .rtcache（全 runtime 对象）
#   M169 修（本地实测踩中）：**按 rt_key 命中优先 + 主机架构过滤**。
#   修前只按 `runtime.o` 的 mtime 取「最新」——而 M168 起交叉编译缓存（aarch64/armv7/
#   riscv64）会落进**同一个** `.rtcache/`：最新那份可能是**别的架构**的对象
#   ⇒ 链接报 `Relocations in generic ELF (EM: 183)` / `file in wrong format`，
#   错误信息完全指不到根因（本机实测：aarch64 档把 pxc 的 devbuild 全数打死）。
#   纪律同 M168「门/工具不能依赖环境」：选料必须**可判定**，不能靠「谁最新」。
CACHE=""; best_m=0; CACHE_SRC=""; CACHE_WHY=""
keyed="$("$ROOT/tools/px" rtcache 2>/dev/null | tail -1)"
if [ -n "$keyed" ] && [ -d "$keyed" ]; then
    n=$(ls "$keyed"/*.o 2>/dev/null | wc -l)
    # ── M223（缺陷 324）：判据由「.o 数 ≥ 15」改为「**rt_ensure 的 .complete 标记**」。
    #   那个 15 是**无解释的魔法数**。本机 `.rtcache`（3149 目录）的 .o 数实测分布：
    #     13×2255 · 14×415 · **15×33** · 17×228 · 18×39 · 22×34 · 26×2 · 29×143
    #   ⇒ 阈值正好卡在 14/15 之间（33 个目录**恰在边界上**），而 13/14 的 2670 个是
    #   **裁剪版**（cuts 不同）。判「个数」而不判「是哪一份」⇒ 只要某个合法配置只产出
    #   14 个 .o，主路径就被**静默毙掉**、改走下面的 mtime 回退（挑到**别的**目录）
    #   ⇒ 指纹漂移，且错误信息指不到根因（M169 同族：「不能靠谁最新」）。
    #   `.complete` 是 `rt_cache_compile` **成功才 touch** 的 ⇒ 本就是"完整"的定义。
    if [ -f "$keyed/.complete" ] && [ "$n" -ge 1 ]; then
        CACHE="${keyed%/}"; CACHE_SRC="rt_key"
    else
        CACHE_WHY="目录不完整（.complete=$([ -f "$keyed/.complete" ] && echo 有 || echo 无) · .o=$n）"
    fi
else
    CACHE_WHY="tools/px rtcache 无输出（未命中且现编失败）"
fi
if [ -z "$CACHE" ]; then
    CACHE_SRC="fallback"
    echo "⚠️  devbuild 选料：主路径失效 ⇒ **回退**到「按 mtime 取最新」（$CACHE_WHY）" >&2
    case "$(uname -m)" in
        x86_64)  WANT="x86-64" ;;
        aarch64) WANT="ARM aarch64" ;;
        armv7l)  WANT="ARM," ;;
        riscv64) WANT="RISC-V" ;;
        *)       WANT="" ;;
    esac
    for d in "$ROOT"/.rtcache/*/; do
        n=$(basename "$d"); [ ${#n} -eq 16 ] || continue
        [ -f "$d/runtime.o" ] || continue
        if [ -n "$WANT" ]; then
            file -b "$d/runtime.o" 2>/dev/null | grep -qF "$WANT" || continue
        fi
        m=$(stat -c %Y "$d/runtime.o" 2>/dev/null || echo 0)
        if [ "$m" -gt "$best_m" ]; then best_m="$m"; CACHE="${d%/}"; fi
    done
fi
[ -n "$CACHE" ] || { echo "❌ 无可用 .rtcache（先跑 ./tools/px build --full examples/hello.px）" >&2; exit 1; }
# ── M223（缺陷 324 续）：回退是**不可判定**的选料（M169 原话「不能靠谁最新」）
#   ⇒ 必须说清挑中了谁、以及它是不是**当前源码**对应的那一份。
#   `tools/px rtkey`（M153）就是为这种"只算 key 不编译"的核对建的。
if [ "$CACHE_SRC" = "fallback" ]; then
    cur_rtkey="$("$ROOT/tools/px" rtkey 2>/dev/null | tail -1)"
    picked="$(basename "$CACHE" 2>/dev/null)"
    if [ -n "$cur_rtkey" ]; then
        if [ "$picked" = "$cur_rtkey" ]; then
            echo "   ✓ 回退挑中的 $picked **正好是**当前源码 rt_key（本次无实质影响）" >&2
        else
            echo "   ⚠️ 回退挑中的 $picked ≠ 当前源码 rt_key $cur_rtkey" >&2
            echo "      ⇒ **链接的 runtime 对象可能与 runtime/ 当前源码不符**（M169 同族）" >&2
        fi
    else
        echo "   ⚠️ 取不到当前 rt_key（tools/px rtkey 失败）⇒ 无法核对挑中的 runtime 与源码是否相符" >&2
    fi
fi
echo "── devbuild 选料：${CACHE#$ROOT/}（$(file -b "$CACHE/runtime.o" 2>/dev/null | cut -d, -f1-2)）"
objs=""; for f in "$CACHE"/*.o; do objs="$objs $f"; done
LIBS="$RT/third_party/sqlite3/sqlite3.o
$RT/mbedtls/lib/libmbedtls.a $RT/mbedtls/lib/libmbedx509.a $RT/mbedtls/lib/libmbedcrypto.a
$RT/third_party/ngtcp2/lib/libngtcp2.a $RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a
$RT/third_party/openssl/lib/libssl.a $RT/third_party/openssl/lib/libcrypto.a
$RT/third_party/zlib/lib/libz.a -lm -ldl -lpthread"

build_one() {   # $1=件名 → /tmp/${1}dev
    local name="$1" src out fp
    src=$(entry_src "$name")
    [ -n "$src" ] || { echo "❌ 未知件：$name（可用：$(echo "$ENTRIES" | cut -d'|' -f1 | tr '\n' ' '))" >&2; return 1; }
    out="/tmp/${name}dev"; fp="/tmp/devbuild_${name}.fp"; mf="/tmp/devbuild_${name}.src"
    if [ "$FORCE_REBUILD" = 0 ] && [ -f "$out" ] && [ -f "$fp" ] && [ "$(cat "$fp" 2>/dev/null)" = "$DEVB_KEY" ]; then
        echo "⏭  $name：源码链未变（key=$DEVB_KEY）⇒ 复用 $out"
        devb_stat reuse "$name" "$DEVB_KEY"
        return 0
    fi
    # ── M223（缺陷 325）：**为什么重建** —— 只报「key 变了」等于没报（本仓纪律：
    #   「门的红必须能读出真因」）。key 不匹配时 diff 逐文件清单 ⇒ **指名变了哪个文件**。
    #   动机（实测）：M222 补的台账在 CI 上报「出现过的 key 3 个」，而「3 个」不告诉你
    #   **为什么**变了 —— 归因能力必须先于下一次排查存在。
    if [ "$FORCE_REBUILD" = 0 ] && [ -f "$fp" ]; then
        _oldkv="$(cat "$fp" 2>/dev/null)"
        if [ "$_oldkv" != "$DEVB_KEY" ]; then
            echo "── $name：源码链指纹变化 $_oldkv → $DEVB_KEY（选料=$CACHE_SRC）"
            if [ -f "$mf" ]; then
                src_manifest > "${mf}.now"
                diff "$mf" "${mf}.now" 2>/dev/null | grep -E '^[<>]' | head -8 | sed 's/^/   /'
                rm -f "${mf}.now"
            else
                echo "   （无上次清单可比 —— M223 起才落盘）"
            fi
        fi
    fi
    timeout 900 "$BASE" build "$ROOT/$src" > "/tmp/devbuild_$name.c" 2>"/tmp/devbuild_$name.err" || {
        echo "❌ $name：pxc build $src 失败"; tail -5 "/tmp/devbuild_$name.err" >&2; return 1; }
    gcc -c -O2 -I"$CACHE" -I"$RT" "/tmp/devbuild_$name.c" -o "/tmp/devbuild_$name.o" 2>"/tmp/devbuild_$name.cc.log" || {
        echo "❌ $name：C 编译失败"; tail -10 "/tmp/devbuild_$name.cc.log" >&2; return 1; }
    gcc -static -O2 -pthread -o "$out" "/tmp/devbuild_$name.o" $objs $LIBS 2>"/tmp/devbuild_$name.link.log" || {
        echo "❌ $name：链接失败"; tail -10 "/tmp/devbuild_$name.link.log" >&2; return 1; }
    echo "$DEVB_KEY" > "$fp"
    src_manifest > "$mf"
    devb_stat rebuild "$name" "$DEVB_KEY"
    echo "✅ $name → $out（$(stat -c %s "$out") 字节）"
}

WANT_VM=0; NAMES=""; FORCE_REBUILD=0
[ "${DEVB_REBUILD:-0}" = "1" ] && FORCE_REBUILD=1
for a in "$@"; do
    case "$a" in
        --vm) WANT_VM=1 ;;
        --rebuild) FORCE_REBUILD=1 ;;
        *) NAMES="$NAMES $a" ;;
    esac
done
[ -n "$NAMES" ] || NAMES="pxc"

# ── M221：源码链指纹（见文件头注）────────────────────────────────
#   纳入：selfhost/tools/stdlib 的 .px + runtime 顶层 .c/.h + 预置三方资产
#         + 入库 pxc 的 sha + .rtcache 目录名。
#   ⚠️ runtime/*.c,h 必须在内（门内负控会改它们）；入库 pxc 在内（自举起点变了产物就变）。
# ── M223（缺陷 323）：**源码内容哈希，不是 mtime** ─────────────────────
#   修前用 `stat -c '%n %s %Y'`（**含 mtime**）：内容一字未改、只是 mtime 变
#   （`touch` / `cp` / 从归档解包 / 编辑器"重写但没改"）就换一个 key
#   ⇒ 23 门里后续每一次都白付一轮冷重建。
#   本机实测（/tmp/m223/exp1.sh）：mtime 版 **152ms** ⇄ 内容哈希版 **34ms**
#   —— 逐文件 `stat` 要 fork 115 次，而 `sha256sum <glob>` 一个进程处理全部
#   ⇒ **更严、且更快**（旧口径连"性能"这个理由都不成立）。
#   三方资产（预置、跨轮不变的大件）用 `%n %s`（名字+大小）**去掉 mtime**：
#   内容变了大小几乎必然变；mtime 却会因为解包/复制而变。
SRC_PATTERNS="selfhost/*.px runtime/*.c runtime/*.h tools/*.px stdlib/*.px"
ASSET_GLOBS="$RT/third_party/sqlite3/sqlite3.o $RT/mbedtls/lib/*.a $RT/third_party/*/lib/*.a"
src_manifest() {   # stdout：逐文件「名 哈希」清单（**已排序** ⇒ 可 diff、可归因）
    {
        sha256sum $SRC_PATTERNS 2>/dev/null | awk '{print $2, substr($1,1,16)}'
        for f in $ASSET_GLOBS; do
            [ -f "$f" ] && stat -c '%n %s' "$f"
        done
        echo "base=$(sha256sum "$BASE" 2>/dev/null | cut -c1-16)"
        echo "rtcache=$(basename "$CACHE")"
    } | LC_ALL=C sort
}
src_line() { src_manifest; }    # 兼容旧调用（`src_line | sha256sum | cut -c1-16`）
DEVB_KEY="$(src_line | sha256sum | cut -c1-16)"
echo "── 源码链指纹：$DEVB_KEY$([ "$FORCE_REBUILD" = 1 ] && echo "（--rebuild：忽略缓存）")"
echo "── rtcache: ${CACHE#$ROOT/}"
for n in $NAMES; do build_one "$n" || exit 1; done

if [ "$WANT_VM" = "1" ]; then
    # ⚠️ VM 轨的**输入含 /tmp/pxcdev 本身** ⇒ 指纹要把它的 sha 也算进来：
    #    `devbuild pxi --vm`（不带 pxc）时 pxcdev 可能是**别的**源码链的产物，
    #    拿它 emit-c 会得到与实际源码不符的 VM 件（m216 门头注记过这个坑）。
    VM_KEY="$DEVB_KEY/$(sha256sum /tmp/pxcdev 2>/dev/null | cut -c1-16)"
    if [ "$FORCE_REBUILD" = 0 ] && [ -f /tmp/pxcdev_vm ] && [ -f /tmp/devbuild_vm.fp ] && [ "$(cat /tmp/devbuild_vm.fp 2>/dev/null)" = "$VM_KEY" ]; then
        echo "⏭  VM 轨：源码链未变（key=$VM_KEY）⇒ 复用 /tmp/pxcdev_vm"
        devb_stat reuse vm "$VM_KEY"
    else
    if [ -f /tmp/devbuild_vm.fp ] && [ "$(cat /tmp/devbuild_vm.fp 2>/dev/null)" != "$VM_KEY" ]; then
        echo "── vm：VM 轨指纹变化 $(cat /tmp/devbuild_vm.fp 2>/dev/null) → $VM_KEY"
    fi
    echo "── VM 轨：--emit-c → /tmp/pxcdev_vm"
    timeout 1500 /tmp/pxcdev --emit-c "$ROOT/selfhost/compiler.px" > /tmp/devbuild_vm.c 2>/tmp/devbuild_vm.err || {
        echo "❌ --emit-c 失败"; tail -5 /tmp/devbuild_vm.err >&2; exit 1; }
    gcc -c -O2 -I"$CACHE" -I"$RT" /tmp/devbuild_vm.c -o /tmp/devbuild_vm.o 2>/tmp/devbuild_vm.cc.log || {
        echo "❌ VM 镜像编译失败"; tail -10 /tmp/devbuild_vm.cc.log >&2; exit 1; }
    gcc -static -O2 -pthread -o /tmp/pxcdev_vm /tmp/devbuild_vm.o $objs $LIBS 2>/tmp/devbuild_vm.link.log || {
        echo "❌ VM 轨链接失败"; tail -10 /tmp/devbuild_vm.link.log >&2; exit 1; }
    echo "✅ VM 轨：/tmp/pxcdev_vm（$(stat -c %s /tmp/pxcdev_vm) 字节）"
    echo "$VM_KEY" > /tmp/devbuild_vm.fp
    devb_stat rebuild vm "$VM_KEY"
    fi
fi

devb_summary
