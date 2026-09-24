#!/usr/bin/env bash
# ============================================================
# selfhost/check_pkg_arch.sh —— 发布包**架构卫生门**（M199，缺陷 239）
# ------------------------------------------------------------
# 存在理由（用户报障的延伸，2026-09-24）：
#   aarch64 引导包（`puxian-bootstrap-aarch64-<tag>.tar.gz`）里**夹带 x86-64 的构建输入**：
#     runtime/third_party/sqlite3/sqlite3.o        （x86-64 ELF 对象）
#     runtime/third_party/sqlite3/sqlite3-windows.o（COFF 对象）
#     runtime/mbedtls/lib/*.a、runtime/third_party/zlib/lib/*.a（x86-64 静态库）
#     runtime/third_party/{openssl,ngtcp2}/lib/*.a（x86-64 静态库；aarch64 上 QUIC 已被裁剪）
#   —— 包内**混架构**的两重后果：
#     ① 体积白送（≈17MB 中的一部分）· ② **错架构陷阱**：用户在 aarch64 上显式指定
#        `--sqlite-obj runtime/third_party/sqlite3/sqlite3.o`（README 里就是这么写的路径家族）
#        会得到 `file in wrong format` 这类**指不到根因**的链接错误。
#   M168 的门只判**可执行件**（bootstrap/*）的静态性与架构 —— 包内**构建输入**无人看。
#
# 判据（逐文件 + 归档成员）：
#   · ELF 可执行/对象   → `readelf -h` 的 Machine 必须匹配 --arch；不匹配**判红**
#   · ar 静态库（.a）   → **逐成员**判架构（成员可能是 ELF 或文本，逐个分类）
#   · PE/COFF（Windows 对象）→ **一律判红**（跨平台构建输入；任何 Linux 发布包都不该带）
#   · 其他（脚本/文本/头文件）→ INFO 跳过（计数打印，不静默）
# 用法：
#   selfhost/check_pkg_arch.sh --arch aarch64 dist/pkg-dir/
#   selfhost/check_pkg_arch.sh --arch x86_64 --allow-foreign 'runtime/third_party/**' dir/
#   selfhost/check_pkg_arch.sh --self-test          # 自证（含 4 道负控，fixture 取自仓库，**与环境无关**）
# 退出码：0 = 达标；1 = 有判红项；2 = 用法/工具缺失
# ============================================================
set -uo pipefail

WANT_ARCH=""; SELFTEST=0; ALLOW=(); PATHS=()
while [ $# -gt 0 ]; do
    case "$1" in
        --arch)          WANT_ARCH="$2"; shift 2 ;;
        --allow-foreign) ALLOW+=("$2"); shift 2 ;;
        --self-test)     SELFTEST=1; shift ;;
        -h|--help)       sed -n '2,26p' "$0"; exit 0 ;;
        *)               PATHS+=("$1"); shift ;;
    esac
done

need_tools() {
    command -v readelf >/dev/null || { echo "❌ 缺 readelf（binutils）" >&2; exit 2; }
    command -v ar >/dev/null || { echo "❌ 缺 ar（binutils）" >&2; exit 2; }
}
need_tools

arch_of_elf() {   # $1=ELF → 打印归一化机器名（AArch64/x86-64/…）；非 ELF 打印空
    readelf -h "$1" 2>/dev/null | awk -F: '/Machine:/{gsub(/^[ \t]+/,"",$2); print $2; exit}'
}
norm_arch() {     # 归一化：aarch64/arm64 → AArch64；x86_64/amd64 → x86-64
    case "$(echo "$1" | tr 'A-Z' 'a-z')" in
        aarch64|arm64)   echo AArch64 ;;
        x86-64|x86_64|amd64) echo x86-64 ;;
        arm|armv7*|arm32) echo ARM ;;
        riscv64*)        echo RISC-V ;;
        *)               echo "$1" ;;
    esac
}
foreign_kind() {  # $1=文件 → Windows/其它平台标记（空 = 无）
    case "$(file -b "$1" 2>/dev/null)" in
        *COFF*|*PE32*) echo 'Windows(PE/COFF)' ;;
        *) echo '' ;;
    esac
}

BAD=0; NELF=0; NAR=0; NINFO=0; NSKIP=0
allowed() {   # $1=相对路径；命中 --allow-foreign 模式则跳过
    local p="$1" pat
    for pat in "${ALLOW[@]:-}"; do
        [ -n "$pat" ] || continue
        case "$p" in $pat) return 0 ;; esac
    done
    return 1
}
check_file() {   # $1=文件路径 $2=显示名 $3=容器(空|。a 成员)
    local f="$1" name="$2" cont="${3:-}"
    local w; w="$(foreign_kind "$f")"
    if [ -n "$w" ]; then
        if allowed "$name"; then NSKIP=$((NSKIP+1)); return 0; fi
        echo "  ❌ [$w] $name${cont} —— 跨平台构建输入，任何 Linux 发布包都不该带"; BAD=$((BAD+1)); return 1
    fi
    local m; m="$(arch_of_elf "$f")"
    if [ -z "$m" ]; then NINFO=$((NINFO+1)); return 0; fi
    NELF=$((NELF+1))
    if [ -n "$WANT_ARCH" ] && [ "$(norm_arch "$m")" != "$(norm_arch "$WANT_ARCH")" ]; then
        if allowed "$name"; then NSKIP=$((NSKIP+1)); return 0; fi
        echo "  ❌ [架构不符] $name${cont} —— 实际 $m，期望 $WANT_ARCH"; BAD=$((BAD+1)); return 1
    fi
    return 0
}
check_ar() {   # $1=.a 路径 $2=显示名 → 逐成员判
    local a="$1" name="$2" tmp d
    tmp="$(mktemp -d)"; ( cd "$tmp" && ar x "$a" 2>/dev/null ) || { rm -rf "$tmp"; NINFO=$((NINFO+1)); return 0; }
    NAR=$((NAR+1))
    for d in "$tmp"/*; do
        [ -f "$d" ] || continue
        check_file "$d" "$name" " › 成员 $(basename "$d")" || true
    done
    rm -rf "$tmp"
    return 0
}
scan() {   # $1=路径
    local p="$1" f
    if [ -d "$p" ]; then
        while IFS= read -r f; do
            case "$(file -b "$f" 2>/dev/null)" in
                *"ar archive"*) check_ar "$f" "${f#"$ROOTDIR"/}" ;;
                *)              check_file "$f" "${f#"$ROOTDIR"/}" ;;
            esac
        done < <(find "$p" -type f | LC_ALL=C sort)
    else
        case "$(file -b "$p" 2>/dev/null)" in
            *"ar archive"*) check_ar "$p" "$p" ;;
            *)              check_file "$p" "$p" ;;
        esac
    fi
}

run_scan() {
    ROOTDIR="${1:-.}"
    echo "── 架构卫生门：期望架构 ${WANT_ARCH:-（不检查）} · 根 $ROOTDIR"
    [ ${#ALLOW[@]} -gt 0 ] && echo "   豁免模式：${ALLOW[*]}"
    scan "$ROOTDIR"
    echo "── 统计：ELF 文件 $NELF · 静态库 $NAR · 非二进制（INFO）$NINFO · 豁免 $NSKIP · **判红 $BAD**"
    [ "$BAD" = 0 ] || return 1
    return 0
}

if [ "$SELFTEST" = 1 ]; then
    # ── 自证：fixture **取自仓库**（x86-64 / aarch64 / COFF 各一）⇒ 判据与环境无关 ──
    #   ⚠️ M168 教训：负控若写死 \"本机架构\"，在对应架构的 runner 上会**恒绿**（假绿）。
    #   ⚠️ v0.2.0-m167 实测：官方 aarch64 包内 sqlite3.o 是 x86-64 —— 本自证把这条固化下来。
    R="$(cd "$(dirname "$0")/.." && pwd)"
    F_X86="$R/runtime/third_party/sqlite3/sqlite3.o"
    F_A64="$R/runtime/third_party/sqlite3/sqlite3-aarch64.o"
    F_WIN="$R/runtime/third_party/sqlite3/sqlite3-windows.o"
    L_X86="$R/runtime/third_party/zlib/lib/libz.a"
    L_A64="$R/runtime/third_party/zlib/lib-aarch64/libz.a"
    for f in "$F_X86" "$F_A64" "$F_WIN" "$L_X86" "$L_A64"; do
        [ -f "$f" ] || { echo "❌ 自证 fixture 缺失：$f" >&2; exit 2; }
    done
    T="$(mktemp -d)"; ok=0; bad=0
    mkdir -p "$T/clean_a64" "$T/with_x86obj" "$T/with_win" "$T/with_x86lib" "$T/text_only"
    cp -f "$F_A64" "$T/clean_a64/sqlite3-aarch64.o"; cp -f "$L_A64" "$T/clean_a64/libz.a"
    cp -f "$T/clean_a64/"* "$T/with_x86obj/"; cp -f "$F_X86" "$T/with_x86obj/sqlite3.o"
    cp -f "$T/clean_a64/"* "$T/with_win/";    cp -f "$F_WIN" "$T/with_win/sqlite3-windows.o"
    cp -f "$T/clean_a64/"* "$T/with_x86lib/"; cp -f "$L_X86" "$T/with_x86lib/libz.a"
    printf '# aarch64 包说明\n' > "$T/text_only/README-aarch64.md"
    t() {  # $1=目录 $2=期望(green|red) $3=描述
        local out rc exp="$2"
        out="$(WANT_ARCH=aarch64 ALLOW=() ; run_scan "$1" 2>&1)"; rc=$?
        if { [ "$exp" = green ] && [ $rc = 0 ]; } || { [ "$exp" = red ] && [ $rc != 0 ]; }; then
            echo "  ✅ 自证 $3（期望 $exp · 实际 rc=$rc）"; ok=$((ok+1))
        else
            echo "  ❌ 自证 $3（期望 $exp · 实际 rc=$rc）"; echo "$out" | sed 's/^/       /'; bad=$((bad+1))
        fi
    }
    # 每例独立重置计数
    run_scan() { BAD=0; NELF=0; NAR=0; NINFO=0; NSKIP=0
        ROOTDIR="${1:-.}"; scan "$ROOTDIR"
        echo "── 统计：ELF $NELF · 静态库 $NAR · INFO $NINFO · 豁免 $NSKIP · 判红 $BAD"
        [ "$BAD" = 0 ] || return 1; return 0; }
    t "$T/clean_a64"  green "全 aarch64 目录 ⇒ 绿（正控）"
    t "$T/with_x86obj" red  "夹带 x86-64 .o ⇒ 红（负控 A：**这正是 m183 aarch64 包的真实缺陷**）"
    t "$T/with_win"    red  "夹带 COFF/Win 对象 ⇒ 红（负控 B）"
    t "$T/with_x86lib" red  "夹带 x86-64 .a（逐成员判）⇒ 红（负控 C）"
    t "$T/text_only"  green "纯文本 ⇒ 绿（INFO 不判红）"
    rm -rf "$T"
    echo "── 自证小计：$ok 通过 · $bad 失败"
    [ "$bad" = 0 ] && { echo "ARCH-GATE-SELFTEST-OK"; exit 0; }
    exit 1
fi

[ ${#PATHS[@]} -gt 0 ] || { echo "用法：$0 --arch <arch> <path...>（或 --self-test）" >&2; exit 2; }
rc=0
for p in "${PATHS[@]}"; do run_scan "$p" || rc=1; done
exit $rc
