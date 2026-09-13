#!/usr/bin/env bash
# ============================================================
# 入库二进制重烘 / 重烘门（shipped bootstrap binaries）
#   M113-S0 建立（Issue 58）· M113-S1 扩门 + 接 CI · M113-S2 补 VM 轨门
# ------------------------------------------------------------
# 背景（Issue 58）：改了 `selfhost/*.px` 却忘了重烘入库编译器，是**结构性失明** ——
#   `bootstrap_prove*.sh` 的 golden 由**同一枚 pxc** 重定基，「老 pxc + 自己的 golden」
#   永远自洽 ⇒ 门绿 ≠ 用户拿到修复。而当时全仓**没有任何可执行的 C 轨重烘指引**
#   （`CONTRIBUTING` 第 5 步只写了 `cp compiler_vm bootstrap/pxc_vm`，C 轨
#   `bootstrap/pxc` 无人重烘 ⇒ 自 M112 起落后源码，见 Issue 58 实测）。
#
# 入库口径：
#   bootstrap/pxc     = compiler.px 的 C 镜像   → gcc **-static** + **全 runtime 对象**
#   bootstrap/pxc_vm  = compiler.px 的字节码镜像 + VM 驱动（**用户面默认轨**：
#                       `tools/px` → `bootstrap/pxc_vm`）→ 动态链
#
# 门的判据（两轨各有门，**判据不重叠**；M113-S2 起）：
#   ① 产物来源（O(1)，两轨共用，**强判据**）：重烘时把**源码链指纹**
#      （`PXSRC-xxxxxxxx`）作为 host-only 常量链进入库件；门上读回该常量与现算指纹
#      比对 —— 不等即「此件不是当前源码烘出来的」。指纹口径 = compiler.px 全 import
#      链 + runtime 的 vm/runtime 源（与 `bootstrap_prove_bc.sh` 的 SRC_CHAIN 对齐）
#      ⇒ 改编译器源码、或改 runtime，都必须重烘。
#      **无内嵌指纹 = 判红**（M113-S2 起入库件必须带指纹；不设"退化放行"的暗门）。
#   ② C 轨门：入库 `pxc` vs「用入库 pxc 现编出的 pxc」——**同轨**行为对拍
#      （rc/stdout/stderr 逐字节），覆盖 cases/ + cases_bad/ + cases_ok/ 全量探针。
#      为何不与 pxc_vm 对拍：**跨轨输出本就不逐字节相同**（实测 54 例中 14 例
#      stdout 或 stderr 有差：诊断措辞/位置）。
#   ③ VM 轨门：**字节码镜像对拍** —— `bc compiler.px` 的输出，入库 `pxc_vm` 与入库
#      `pxc` 逐字节一致（BC 轨自举证明的跨引擎契约，实测 C 轨 53s / VM 轨 36s）。
#      ⚠️ 诚实边界（负控实测定界）：镜像对拍**不是"来源"判据** —— 把 main 上
#      **未含 Issue 57 修复**的旧 `pxc_vm` 换回去，镜像仍逐字节一致（该修复不改发射
#      结果，只改语义检查）。故"是否重烘"由 ① 判，"跨引擎发射是否漂移/VM 件是否
#      能编译自身"由 ③ 判；VM 轨的**逐例行为**（rc/stdout/stderr vs 基准轨/golden、
#      正例产物运行对拍）由 `engine_parity.sh` 守（S0 起在 CI）。三门各管一件事，
#      不重复、也不假装覆盖对方的盲区。
#
# 前置：`.rtcache` 里有**全 runtime** 对象缓存
#       （若没有：`./tools/px build --full examples/hello.px` 生成）
# 用法：
#   ./selfhost/rebake_bin.sh              # 重烘两个入库件（含出厂指纹）
#   ./selfhost/rebake_bin.sh --check      # C 轨重烘门
#   ./selfhost/rebake_bin.sh --check-vm   # VM 轨重烘门（用户面默认轨）
#   ./selfhost/rebake_bin.sh --rebake-all            # 全件重烘（表内 14 件）
#   ./selfhost/rebake_bin.sh --check-all             # 全件指纹门（O(1) 逐件）
#   ./selfhost/rebake_bin.sh --entries=pxlint,pxcheck # 只重烘指定件（M114 尾）
# ============================================================
set -u
cd "$(dirname "$0")/.."
ROOT=$(pwd)
# ---- M114-S4（CI 首跑红）：指纹必须**与语言环境无关** ----
# 实锤（PR #8 首次真机 CI）：`closure | sort -u` 的**排序受 locale 影响** ——
#   本机 LANG=en_US.UTF-8（glibc 排序：`_` 基本权重可忽略）算出的 pxi 指纹是
#   PXSRC-b0e380ca15c1a775，而 CI runner（C 语言环境，逐字节序）算出的同一份源码
#   是 PXSRC-7716b5f78932a71a ⇒ 同样源码、同样脚本，两处结论不同；14 件里恰好有
#   4 件（pxi/pxi_vm：闭包含 i_err.px；pxcheck/pxlint：闭包含 `tools/../selfhost/…`
#   这类路径）踩中排序差异，门在 CI 上**假红**。
# 修法：全脚本钉死 LC_ALL=C（逐字节排序 = 可复现），并在 sort 处再显式一次。
# 教训与本门同源：**一个门若依赖环境，它给出的"红/绿"就不是事实**。
export LC_ALL=C
export LANG=C

MODE=rebake
ENTRY_ONLY=""
for a in "$@"; do
    case "$a" in
        --rebake)     MODE=rebake ;;
        --check)      MODE=check ;;
        --check-vm)   MODE=check-vm ;;
        --rebake-all) MODE=rebake-all ;;   # M114-S2（Issue 55）：重烘表内全部入库件
        --check-all)  MODE=check-all ;;    # M114-S2（Issue 55）：全件指纹门（O(1) 逐件断言来源）
        --entries=*)  MODE=entry; ENTRY_ONLY="${a#--entries=}" ;;
                                           # M114 尾：**只重烘指定件**（逗号分隔），
                                           # 不动其余件 —— 改单个工具（如 pxlint）时不必
                                           # 重烘全部 14 件。验收仍走 --check-all。
        -h|--help)  sed -n '2,60p' "$0"; exit 0 ;;
        *) echo "未知参数：$a（可用：--check / --check-vm / --rebake-all / --check-all / --entries=<件名,件名>）" >&2; exit 2 ;;
    esac
done

PXC="$ROOT/bootstrap/pxc"
PXVM="$ROOT/bootstrap/pxc_vm"
BUILD="$ROOT/selfhost/build"
RT="$ROOT/runtime"
CK="$ROOT/selfhost/compiler.px"
FP_OBJ=/tmp/rebake_fp.o
FP_TAG=PXSRC
FP=""
PARITY_CNT=0

# ---- 源码链指纹（口径与 bootstrap_prove_bc.sh 的 SRC_CHAIN 对齐）----
FP_FILES="compiler.px codegen.px parser.px pxlexer.px cg_stmt.px cg_expr.px cg_module.px bc_emit.px"
FP_FILES="$FP_FILES ../runtime/vm.c ../runtime/vm.h ../runtime/runtime.c ../runtime/runtime.h"
# ⚠️ M114-S2（Issue 55）起**已不再被调用**：指纹口径统一由下方 entry_fp()（import 闭包）
#   承担 —— 它比这份 8 文件清单更严（实测多含 astdump.px）。保留仅供对照/考古。
src_fingerprint() {
    ( cd "$ROOT/selfhost" && cat $FP_FILES 2>/dev/null ) | sha256sum | cut -c1-16
}
artifact_fp() {                       # $1=二进制 → 打印内嵌指纹（无则空）
    grep -ao "${FP_TAG}-[0-9a-f]\{16\}" "$1" 2>/dev/null | head -1 | cut -d- -f2
}
emit_fp_object() {                    # $1=指纹 → 编成 host-only 对象，供链接
    cat > /tmp/rebake_fp.c <<EOF
/* M113-S2（Issue 58）：出厂源码链指纹 —— host-only 常量，唯一用途是回答
   「这枚入库件是不是当前源码烘出来的」。不参与 .px 语义，不进任何 golden。 */
const char px_build_srcfp[] = "${FP_TAG}-${1}";
EOF
    gcc -c -O2 /tmp/rebake_fp.c -o "$FP_OBJ" 2>/tmp/rebake_fp_cc.log || {
        echo "❌ 指纹对象编译失败" >&2; tail -5 /tmp/rebake_fp_cc.log >&2; return 1; }
}
fp_gate() {                           # $1=二进制 $2=标签 → 0 一致 / 1 不一致或缺指纹
    local got; got=$(artifact_fp "$1")
    if [ -z "$got" ]; then
        echo "    ❌ $2：无内嵌指纹 ⇒ 无法判定是否由当前源码烘出"
        echo "       （M113-S2 起入库件必须带源码链指纹；本门不设「退化放行」的暗门）"
        return 1
    fi
    if [ "$got" != "$FP" ]; then
        echo "    ❌ $2：内嵌指纹 ${FP_TAG}-${got} ≠ 当前源码链 ${FP_TAG}-${FP}"
        return 1
    fi
    echo "    ✅ $2：内嵌指纹 ${FP_TAG}-${got} == 当前源码链"
    return 0
}
rebake_hint() {
    echo "   ⇒ 重烘：./selfhost/rebake_bin.sh   （改过 selfhost/*.px 或 runtime/ 下的 vm/runtime 源就必须重烘，否则用户拿到的是旧引擎）"
}

# ---- M114-S2（Issue 55）：入库件表 + 全件指纹口径 ----
# 为什么（Issue 55）：Issue 58 只治了 pxc/pxc_vm 两件，而 `bootstrap/` 里实际有 14 件
#   —— 其余件「是否由当前源码烘出」既**不可判定**（无内嵌指纹）也**无门**。
#   更糟的是**用户面**：`tools/px run` 走 `bootstrap/pxi`（解释器），它一直停在 M110
#   ⇒ Issue 54 的僵尸回收修复在解释轨不生效（实测 `px run` 仍留 5 个 <defunct>）。
#
# 表格式：件名|入口源（相对 $ROOT）|生成轨（c=C 文本 / vm=字节码镜像）|链接（static/dynamic）
# 口径来源：实测 `ldd bootstrap/*`（13 件静态 + pxc_vm 动态）+ `selfhost/build/interp_vm.c`
#   的存在（pxi_vm 与 pxc_vm 同为 `--emit-c` 产物，只是历史链接方式不同 —— 按实测保留，
#   不在本切片统一它，避免顺手改掉用户面件的链接形态）。
ENTRIES="pxc|selfhost/compiler.px|c|static
pxc_vm|selfhost/compiler.px|vm|dynamic
pxi|selfhost/interp.px|c|static
pxi_vm|selfhost/interp.px|vm|static
pxl|selfhost/lexer.px|c|static
pxpar|selfhost/parser.px|c|static
pxfmt|tools/pxfmt.px|c|static
pxbench|tools/pxbench.px|c|static
pxcheck|tools/pxcheck.px|c|static
pxdoc|tools/pxdoc.px|c|static
pxlint|tools/pxlint.px|c|static
pxlsp|tools/pxlsp.px|c|static
pxmcp|tools/pxmcp.px|c|static
pxtest|tools/pxtest.px|c|static"

entry_field() {   # $1=件名 $2=字段号(2=源 3=轨 4=链接)
    local n s t l
    while IFS='|' read -r n s t l; do
        if [ "$n" = "$1" ]; then
            case "$2" in
                2) printf '%s' "$s" ;;
                3) printf '%s' "$t" ;;
                4) printf '%s' "$l" ;;
            esac
            break
        fi
    done <<< "$ENTRIES"
}
entry_src()   { entry_field "$1" 2; }
entry_gen()   { entry_field "$1" 3; }
entry_link()  { entry_field "$1" 4; }
entry_list()  { printf '%s\n' "$ENTRIES" | cut -d'|' -f1; }

# import 闭包（递归、含自身、绝对路径）。递归展开 import "x.px" 到不动点；
#   `sort -u` 去重。比 pxc 专用的 8 文件清单**更严**（实测多含 astdump.px）——
#   宁可要求重烘，不可漏（Issue 58 的成因正是"口径比真实依赖窄"）。
closure() {
    local f="$1" d m
    d=$(dirname "$f")
    printf '%s\n' "$f"
    grep -o 'import "[^"]*\.px"' "$f" 2>/dev/null | sed 's/import "//;s/"//' | while read -r m; do
        [ -f "$d/$m" ] && closure "$d/$m"
    done
}
# 件的源码链指纹 = 该件 import 闭包内容 + runtime vm/runtime 源
entry_fp() {
    local src; src=$(entry_src "$1")
    [ -n "$src" ] || return 1
    { closure "$ROOT/$src" | LC_ALL=C sort -u | while read -r p; do cat "$p"; done
      cat "$ROOT/runtime/vm.c" "$ROOT/runtime/vm.h" \
          "$ROOT/runtime/runtime.c" "$ROOT/runtime/runtime.h" 2>/dev/null; } \
      | sha256sum | cut -c1-16
}

# ---- 全 runtime 档缓存（重烘/现编 用；纯镜像对拍不需要）----
select_cache() {
    # M114-S3（Issue 62）：**先让 tools/px 按 rt_key 命中/生成「当前 runtime 源」的缓存**，
    #   再选**最新写入**的全量档目录（而不是「.o 数最多的」）。
    # 为什么（实锤）：`.rtcache/<key>` 按 key（runtime 源内容 × cc × 裁剪集）隔离 —— 改
    #   runtime 后老目录成为**孤儿**，而**新旧目录的全量档 .o 数相同**（本机都是 23 个）
    #   ⇒ 「选 .o 最多」会按遍历顺序**随机命中旧 runtime 的 .o**，把「缺最新 runtime 修复」
    #   的件烘进入库件，而**所有门都看不见**（门测编译器行为，不测 runtime 层修复）。
    #   实测：`190308f2…`（23 个 .o）的 `runtime.o` **不含** `PX_NO_CHILD_REAP`（Issue 54 修复），
    #   却是历轮重烘实际选中的那一份；正解是 `e210498567f2ca64`（含该标记）。
    local d n m best=0 best_m=0
    CACHE=""
    echo "── 先按 rt_key 命中/生成「当前 runtime 源」缓存（tools/px build --full）"
    "$ROOT/tools/px" build --full "$ROOT/examples/hello.px" >/tmp/rebake_cache_gen.log 2>&1 || \
        echo "⚠️  tools/px build --full 失败（见 /tmp/rebake_cache_gen.log）；回退按 mtime 选已有缓存" >&2
    FULL_MIN=15
    # 重烘/重烘门要求**全量档**；--check/--check-vm 容忍裁剪档（判据与"链进多少 runtime"无关）
    local min=1
    case "$MODE" in rebake|rebake-all|check-all|entry) min=$FULL_MIN ;; esac
    for d in "$ROOT"/.rtcache/*/; do
        [ -d "$d" ] || continue
        [ -f "$d/.complete" ] || continue
        n=$(ls "$d"/*.o 2>/dev/null | wc -l)
        [ "$n" -ge "$min" ] || continue
        m=$(stat -c %Y "$d")
        if [ "$m" -gt "$best_m" ]; then best_m=$m; best=$n; CACHE="$d"; fi
    done
    [ -n "$CACHE" ] || {
        echo "❌ 未找到可用 .rtcache（先跑：./tools/px build --full examples/hello.px）" >&2
        return 1
    }
    if [ "$best" -ge "$FULL_MIN" ]; then
        echo "── runtime 缓存：$CACHE（${best} 个 .o = 全量档）"
    else
        echo "── runtime 缓存：$CACHE（${best} 个 .o = 裁剪档）"
        if [ "$MODE" = "rebake" ] || [ "$MODE" = "rebake-all" ]; then
            echo "❌ 只有裁剪档缓存，重烘会把入库件链成 runtime 能力不全的件。" >&2
            echo "   先生成全量缓存：./tools/px build --full examples/hello.px" >&2
            return 1
        fi
    fi
    return 0
}

LINK_FP=0            # 1 = 链接时带上指纹对象（仅重烘入库件）
link_c_track() {     # $1=compiler_new.c  $2=输出（静态 + 全 runtime）
    local cfile="$1" out="$2" objs="" f fp=""
    [ "$LINK_FP" = "1" ] && fp="$FP_OBJ"
    gcc -c -O2 -I"$CACHE" -I"$RT" "$cfile" -o /tmp/rebake_cn.o 2>/tmp/rebake_cc.log || {
        echo "❌ compiler_new.c 编译失败" >&2; tail -10 /tmp/rebake_cc.log >&2; return 1; }
    for f in "$CACHE"*.o; do objs="$objs $f"; done
    gcc -static -O2 -pthread -o "$out" /tmp/rebake_cn.o $objs $fp \
        "$RT/third_party/sqlite3/sqlite3.o" \
        "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
        "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
        "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
        "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread 2>/tmp/rebake_link.log || {
        echo "❌ 链接失败：$out" >&2; tail -10 /tmp/rebake_link.log >&2; return 1; }
    return 0
}
link_vm_track() {    # $1=compiler_vm.c  $2=输出（动态 = 用户面默认轨口径）
    local cfile="$1" out="$2" objs="" f fp=""
    [ "$LINK_FP" = "1" ] && fp="$FP_OBJ"
    gcc -c -O2 -I"$CACHE" -I"$RT" "$cfile" -o /tmp/rebake_vm.o 2>/tmp/rebake_vmcc.log || {
        echo "❌ compiler_vm.c 编译失败" >&2; tail -10 /tmp/rebake_vmcc.log >&2; return 1; }
    for f in "$CACHE"*.o; do objs="$objs $f"; done
    gcc -O2 -pthread -o "$out" /tmp/rebake_vm.o $objs $fp \
        "$RT/third_party/sqlite3/sqlite3.o" \
        "$RT/mbedtls/lib/libmbedtls.a" "$RT/mbedtls/lib/libmbedx509.a" "$RT/mbedtls/lib/libmbedcrypto.a" \
        "$RT/third_party/ngtcp2/lib/libngtcp2.a" "$RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a" \
        "$RT/third_party/openssl/lib/libssl.a" "$RT/third_party/openssl/lib/libcrypto.a" \
        "$RT/third_party/zlib/lib/libz.a" -lm -ldl -lpthread 2>/tmp/rebake_vmlink.log || {
        echo "❌ compiler_vm 链接失败" >&2; tail -10 /tmp/rebake_vmlink.log >&2; return 1; }
    return 0
}

# ---- M114-S2：通用链接（轨/链接方式参数化）与单件重烘 ----
# 与 link_c_track/link_vm_track 同一库列表与同一口径，仅 static/dynamic 可选 ——
# 旧函数保留给 --check/--check-vm（已验证的路径不动），本条服务全件表。
LINK_LIBS_STATIC() {
    printf '%s' \
      "$RT/third_party/sqlite3/sqlite3.o \
       $RT/mbedtls/lib/libmbedtls.a $RT/mbedtls/lib/libmbedx509.a $RT/mbedtls/lib/libmbedcrypto.a \
       $RT/third_party/ngtcp2/lib/libngtcp2.a $RT/third_party/ngtcp2/lib/libngtcp2_crypto_quictls.a \
       $RT/third_party/openssl/lib/libssl.a $RT/third_party/openssl/lib/libcrypto.a \
       $RT/third_party/zlib/lib/libz.a"
}
link_entry() {       # $1=标签 $2=C 源 $3=输出 $4=static|dynamic
    local tag="$1" cfile="$2" out="$3" how="$4" objs="" f fp="" st="" libs=""
    [ "$LINK_FP" = "1" ] && fp="$FP_OBJ"
    [ "$how" = "static" ] && st="-static"
    libs=$(LINK_LIBS_STATIC)
    gcc -c -O2 -I"$CACHE" -I"$RT" "$cfile" -o "/tmp/rebake_$tag.o" 2>"/tmp/rebake_$tag.cc.log" || {
        echo "❌ $tag：gcc -c 失败" >&2; tail -10 "/tmp/rebake_$tag.cc.log" >&2; return 1; }
    for f in "$CACHE"*.o; do objs="$objs $f"; done
    # shellcheck disable=SC2086
    gcc $st -O2 -pthread -o "$out" "/tmp/rebake_$tag.o" $objs $fp $libs -lm -ldl -lpthread \
        2>"/tmp/rebake_$tag.link.log" || {
        echo "❌ $tag：链接失败 $out" >&2; tail -10 "/tmp/rebake_$tag.link.log" >&2; return 1; }
    return 0
}
rebake_one() {       # $1=件名 → 0 成功 / 1 失败（含指纹自证）
    local name="$1" src gen lnk out fp got
    src=$(entry_src "$name"); gen=$(entry_gen "$name"); lnk=$(entry_link "$name")
    out="$ROOT/bootstrap/$name"
    [ -n "$src" ] || { echo "❌ 未知件：$name" >&2; return 1; }
    [ -f "$ROOT/$src" ] || { echo "❌ $name：缺入口源 $src" >&2; return 1; }
    fp=$(entry_fp "$name") || return 1
    echo "── [$name] $src（轨 $gen/$lnk · 源码链 PXSRC-$fp）"
    emit_fp_object "$fp" || return 1      # 每件用**自己的**指纹对象（不是全局 FP 那份）
    mkdir -p "$BUILD"
    if [ "$gen" = "c" ]; then
        timeout 900 "$PXC" build "$ROOT/$src" > "$BUILD/rebake_$name.c" 2>"/tmp/rebake_$name.build.err" || {
            echo "❌ $name：pxc build $src 失败" >&2; tail -5 "/tmp/rebake_$name.build.err" >&2; return 1; }
        LINK_FP=1
        link_entry "$name" "$BUILD/rebake_$name.c" "$out" "$lnk" || return 1
    else
        [ -x "$BUILD/compiler_new" ] || {
            echo "❌ $name：VM 轨需要 $BUILD/compiler_new（先重烘 pxc）" >&2; return 1; }
        timeout 1500 "$BUILD/compiler_new" --emit-c "$ROOT/$src" > "$BUILD/rebake_$name.vm.c" 2>"/tmp/rebake_$name.vmerr" || {
            echo "❌ $name：--emit-c $src 失败" >&2; tail -5 "/tmp/rebake_$name.vmerr" >&2; return 1; }
        LINK_FP=1
        link_entry "$name" "$BUILD/rebake_$name.vm.c" "$out" "$lnk" || return 1
    fi
    got=$(artifact_fp "$out")
    [ "$got" = "$fp" ] || { echo "❌ $name：内嵌指纹读回失败（got=${got:-空} ≠ $fp）" >&2; return 1; }
    echo "    ✅ $name  PXSRC-$got  $(stat -c%s "$out")B  sha256 $(sha256sum "$out" | cut -c1-16)"
    return 0
}

probe_files() {      # 全量探针（跳过 M70-S3 夹具 modstate.px）
    local f
    for f in "$ROOT"/selfhost/cases/*.px "$ROOT"/selfhost/cases_bad/*.px "$ROOT"/selfhost/cases_ok/*.px; do
        [ -e "$f" ] || continue
        case "$(basename "$f")" in modstate.px) continue;; esac
        printf '%s\n' "$f"
    done
}
parity() {           # $1=参照件 $2=被测件 $3=标签：逐例 rc/stdout/stderr 逐字节
    local ref="$1" tst="$2" label="$3" bad=0 cnt=0 f s t
    while IFS= read -r f; do
        cnt=$((cnt+1))
        "$ref" build "$f" > /tmp/rb_ref.out 2>/tmp/rb_ref.err; s=$?
        "$tst" build "$f" > /tmp/rb_tst.out 2>/tmp/rb_tst.err; t=$?
        if [ "$s" != "$t" ] || ! cmp -s /tmp/rb_ref.out /tmp/rb_tst.out \
           || ! cmp -s /tmp/rb_ref.err /tmp/rb_tst.err; then
            echo "    ❌ $(basename "$f")：$label 行为不同（rc $s vs $t）"
            bad=$((bad+1))
        fi
    done < <(probe_files)
    PARITY_CNT=$cnt
    if [ "$bad" -gt 0 ]; then
        echo "❌ $label：$cnt 例中 $bad 例不同" >&2
        return 1
    fi
    echo "    ✅ $label：$cnt 例 rc/stdout/stderr 逐字节一致"
    return 0
}
dump_parity() {      # 两轨字节码镜像（bc compiler.px）逐字节对拍
    local a=/tmp/rb_c.bc b=/tmp/rb_vm.bc
    echo "── 镜像对拍：bc compiler.px（实测 C 轨 ≈53s / VM 轨 ≈36s）"
    timeout 900 "$PXC" bc "$CK" > "$a" 2>/tmp/rb_c_dump.err || {
        echo "❌ C 轨 $PXC bc compiler.px 失败" >&2; tail -3 /tmp/rb_c_dump.err >&2; return 1; }
    ( ulimit -v 10000000; timeout 900 "$PXVM" bc "$CK" > "$b" 2>/tmp/rb_vm_dump.err ) || {
        echo "❌ VM 轨 $PXVM bc compiler.px 失败（panic/超时/内存）" >&2; tail -3 /tmp/rb_vm_dump.err >&2; return 1; }
    [ -s "$a" ] && [ -s "$b" ] || { echo "❌ dump 为空（C $(wc -c <"$a")B / VM $(wc -c <"$b")B）" >&2; return 1; }
    # 归一化口径与 bootstrap_prove_bc.sh 的 norm_bc 一致（去掉 '# BCModule ' 头行）
    grep -vE '^# BCModule ' "$a" > /tmp/rb_c.n
    grep -vE '^# BCModule ' "$b" > /tmp/rb_vm.n
    if cmp -s /tmp/rb_c.n /tmp/rb_vm.n; then
        echo "    ✅ 两轨字节码镜像逐字节一致（$(wc -l < /tmp/rb_c.n) 行）"
        return 0
    fi
    echo "❌ 两轨字节码镜像不一致（C $(wc -l < /tmp/rb_c.n) 行 / VM $(wc -l < /tmp/rb_vm.n) 行）" >&2
    diff /tmp/rb_c.n /tmp/rb_vm.n | head -10 >&2
    return 1
}
build_fresh_c() {    # 用入库 pxc 现编「当前源码的 C 轨件」→ $BUILD/rebake_new
    # 每次**重新现编**（不复用旧产物）：复用会让「入库 pxc 已被换掉」时拿到上一轮的
    # 参照件，等于门自己给自己发通行证。成本实测 ≈20s（CI 新检出同量级）。
    mkdir -p "$BUILD"
    echo "── 用入库 pxc 现编当前源码 → $BUILD/rebake_new.c"
    timeout 900 "$PXC" build "$CK" > "$BUILD/rebake_new.c" 2>/tmp/rebake_build.log || {
        echo "❌ 入库 pxc 无法编译当前源码（已失效）" >&2; tail -5 /tmp/rebake_build.log >&2; return 1; }
    link_c_track "$BUILD/rebake_new.c" "$BUILD/rebake_new" || return 1
    return 0
}

# ============================================================
# 门 ①/②：C 轨
# ============================================================
if [ "$MODE" = "check" ]; then
    echo "── [--check] C 轨重烘门（入库 bootstrap/pxc）"
    FP=$(entry_fp pxc)
    echo "── 当前源码链指纹：${FP_TAG}-${FP}"
    fp_gate "$PXC" "入库 bootstrap/pxc"; fprc=$?
    if [ "$fprc" = "1" ]; then echo "❌ 入库 pxc 不是当前源码烘出的"; rebake_hint; exit 1; fi
    select_cache || exit 1
    build_fresh_c || exit 1
    parity "$PXC" "$BUILD/rebake_new" "入库 pxc vs 现编 pxc" || {
        echo "❌ 入库 bootstrap/pxc 与当前源码不一致（${PARITY_CNT} 例探针）"; rebake_hint; exit 1; }
    echo "✅ 入库 bootstrap/pxc 与当前源码一致（指纹 + ${PARITY_CNT} 例行为对拍）"
    exit 0
fi

# ============================================================
# 门 ①/③：VM 轨（用户面默认轨）
# ============================================================
if [ "$MODE" = "check-vm" ]; then
    echo "── [--check-vm] VM 轨重烘门（用户面默认轨：tools/px → bootstrap/pxc_vm）"
    [ -x "$PXVM" ] || { echo "❌ 缺少可执行 bootstrap/pxc_vm" >&2; exit 1; }
    "$PXVM" --version >/dev/null 2>&1 || {
        echo "❌ bootstrap/pxc_vm --version 失败（件已损或不可执行）" >&2; exit 1; }
    FP=$(entry_fp pxc)
    echo "── 当前源码链指纹：${FP_TAG}-${FP}"
    fp_gate "$PXVM" "入库 bootstrap/pxc_vm"; r1=$?
    [ "$r1" = "1" ] && { echo "❌ 入库 pxc_vm 不是当前源码烘出的"; rebake_hint; exit 1; }
    # 对拍基准（C 轨件）也必须是当前源码烘出的，否则"基准"不可信
    fp_gate "$PXC" "入库 bootstrap/pxc（对拍基准）"; r2=$?
    [ "$r2" = "1" ] && { echo "❌ 对拍基准 bootstrap/pxc 不是当前源码烘出的（先过 --check）"; rebake_hint; exit 1; }
    dump_parity || {
        echo "❌ VM 轨入库件与当前源码不一致（字节码镜像对拍失败）"; rebake_hint; exit 1; }
    echo "✅ 入库 bootstrap/pxc_vm 与当前源码一致（指纹 + 字节码镜像对拍）"
    exit 0
fi

# ============================================================
# 门 ①（全件）：源码链指纹门 —— O(1) 逐件断言「此件是否当前源码烘出」
# ============================================================
if [ "$MODE" = "check-all" ]; then
    echo "── [--check-all] 全件源码链指纹门（bootstrap/ 共 $(entry_list | wc -l) 件）"
    n=0; bad=0; miss=0
    for name in $(entry_list); do
        n=$((n+1))
        out="$ROOT/bootstrap/$name"
        if [ ! -x "$out" ]; then echo "    ❌ $name：缺件或不可执行"; bad=$((bad+1)); continue; fi
        fp=$(entry_fp "$name") || { echo "    ❌ $name：无法计算源码链"; bad=$((bad+1)); continue; }
        got=$(artifact_fp "$out")
        if [ -z "$got" ]; then
            echo "    ❌ $name：无内嵌指纹（引入指纹前烘的旧件 / 从未重烘）"; miss=$((miss+1)); bad=$((bad+1))
        elif [ "$got" != "$fp" ]; then
            echo "    ❌ $name：内嵌 PXSRC-$got ≠ 当前源码链 PXSRC-$fp"; bad=$((bad+1))
        else
            echo "    ✅ $name：PXSRC-$got"
        fi
    done
    echo "── 小计：$((n-bad))/$n 件与当前源码一致（其中无指纹 $miss 件）"
    if [ "$bad" -gt 0 ]; then
        echo "❌ 有 $bad 件不是当前源码烘出的（用户拿到的是旧引擎）"
        echo "   ⇒ 全件重烘：./selfhost/rebake_bin.sh --rebake-all"
        exit 1
    fi
    echo "✅ 全件源码链一致"
    exit 0
fi

# ============================================================
# 重烘
# ============================================================
FP=$(entry_fp pxc)
echo "── 源码链指纹：${FP_TAG}-${FP}"
select_cache || exit 1

# ---- M114 尾：单件重烘（--entries=<件名,...>）----
# 只重烘指定件：不动 pxc/pxc_vm，也不动其余 12 件。适用「改了某个工具源文件」
# 这类收口（逐件自证内嵌指纹；整体验收仍由 --check-all 守）。
if [ "$MODE" = "entry" ]; then
    ok=0; fail=0; failed=""
    for name in $(printf '%s' "$ENTRY_ONLY" | tr ',' ' '); do
        if rebake_one "$name"; then ok=$((ok+1)); else fail=$((fail+1)); failed="$failed $name"; fi
    done
    echo "── 单件重烘小计：成功 $ok · 失败 $fail"
    if [ -n "$failed" ]; then
        echo "   ❌ 失败件：$failed" >&2
        exit 1
    fi
    echo "   ⇒ 验收：./selfhost/rebake_bin.sh --check-all"
    exit 0
fi

emit_fp_object "$FP" || exit 1

echo "── 步骤 1/4：入库 pxc 编译 compiler.px → compiler_new.c"
mkdir -p "$BUILD"
timeout 900 "$PXC" build "$CK" > "$BUILD/compiler_new.c" 2>/tmp/rebake_build.log || {
    echo "❌ pxc build compiler.px 失败" >&2; tail -5 /tmp/rebake_build.log >&2; exit 1; }

echo "── 步骤 2/4：gcc -static 链 bootstrap/pxc（全 runtime + 指纹）"
cp -a "$PXC" "/tmp/pxc.bak-$(date +%Y%m%d-%H%M%S)"      # 备份到 /tmp，不污染仓库
LINK_FP=1
link_c_track "$BUILD/compiler_new.c" "$PXC" || exit 1

echo "── 步骤 3/4：--emit-c 生成字节码镜像 → 链 bootstrap/pxc_vm（动态）"
# 发射用「刚链出的 C 轨件」（与 bootstrap_prove_bc.sh 同一链路口径）
LINK_FP=0
link_c_track "$BUILD/compiler_new.c" "$BUILD/compiler_new" || exit 1
LINK_FP=1
if [ ! -s "$BUILD/compiler_vm.c" ] || [ "$BUILD/compiler_vm.c" -ot "$PXC" ]; then
    timeout 1500 "$BUILD/compiler_new" --emit-c "$CK" > "$BUILD/compiler_vm.c" 2>/tmp/rebake_emitc.log || {
        echo "❌ --emit-c compiler.px 失败" >&2; tail -5 /tmp/rebake_emitc.log >&2; exit 1; }
fi
link_vm_track "$BUILD/compiler_vm.c" "$BUILD/compiler_vm" || exit 1
cp -f "$BUILD/compiler_vm" "$PXVM"

echo "── 步骤 4/4：读回内嵌指纹（自证链进去了）"
for b in "$PXC" "$PXVM"; do
    got=$(artifact_fp "$b")
    [ "$got" = "$FP" ] || { echo "❌ $b 内嵌指纹读回失败（got=${got:-空}）" >&2; exit 1; }
    echo "    ✅ $(basename "$b")：${FP_TAG}-${got}"
done
file "$PXC" "$PXVM" | sed 's/^/    /'
echo "    pxc    sha256 $(sha256sum "$PXC" | cut -c1-16)"
echo "    pxc_vm sha256 $(sha256sum "$PXVM" | cut -c1-16)"
echo "✅ 重烘完成 —— 接下来必须跑：./selfhost/rebake_bin.sh --check && ./selfhost/rebake_bin.sh --check-vm"
echo "   以及：./selfhost/engine_parity.sh && ./selfhost/diffcheck.sh --errors"

# ============================================================
# M114-S2（Issue 55）：全件重烘（表内其余件；pxc/pxc_vm 上面已烘）
# ============================================================
if [ "$MODE" = "rebake-all" ]; then
    echo ""
    echo "══ 全件重烘（表内 $(entry_list | wc -l) 件；pxc/pxc_vm 已在上一步完成）══"
    ok=0; fail=0; failed=""
    for name in $(entry_list); do
        case "$name" in pxc|pxc_vm) continue;; esac
        if rebake_one "$name"; then ok=$((ok+1)); else fail=$((fail+1)); failed="$failed $name"; fi
    done
    echo "── 全件重烘小计：成功 $ok · 失败 $fail"
    if [ -n "$failed" ]; then
        echo "   ❌ 失败件：$failed"
        echo "   （失败件保持原样、不影响已成功件；逐件日志见 /tmp/rebake_<件名>.*）"
    fi
    echo "   ⇒ 验收：./selfhost/rebake_bin.sh --check-all"
fi
