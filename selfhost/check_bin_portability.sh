#!/usr/bin/env bash
# ============================================================
# selfhost/check_bin_portability.sh —— 入库/发布**二进制可移植性门**（M168）
# ------------------------------------------------------------
# 存在理由（2026-09-21 用户报障的根因，不是现象）：
#   官方 aarch64 引导包（v0.2.0-m166/m167）里的 `bootstrap/pxc` 是**在
#   ubuntu-24.04-arm 上用系统 gcc 动态链接**出来的 ⇒ 它带 `GLIBC_2.38` 动态符号需求，
#   而 openEuler 22.03 只有 glibc 2.34 ⇒ 用户"装上了、解压了，却一执行就
#   `version GLIBC_2.38 not found`"。
#   更糟的是**发布链上没有人在看这件事**：CI 的自证只跑「本机能不能跑」
#   （runner 上当然能跑 —— 它自己就是 2.38），于是发布物一路绿到用户手里。
#   ⇒ 本门把「这个二进制到底需要什么」变成**可判定的数字**，并给出基线比较。
# 判据（逐文件）：
#   · 全静态（`readelf -d` 无 DT_NEEDED）           → ✅ 零动态依赖（最可移植）
#   · 动态：取 `objdump -T` 里最高的 GLIBC_x.y
#       ≤ 基线（默认 2.34，= el9 / openEuler 22.03）→ ✅ 并**打印该数字**
#       >  基线                                     → ❌ 判红（附「哪台发行版会崩」）
#   · 指定 --arch 时，`readelf -h` 的机器必须匹配   → 不匹配判红（防"包内混架构"）
#   · 非 ELF（脚本/文本/静态库）→ 记 INFO 跳过，不进账（但会打印计数，不静默）
# 用法：
#   selfhost/check_bin_portability.sh bootstrap/*                    # 基线 2.34
#   selfhost/check_bin_portability.sh --arch aarch64 -b 2.17 dir/*   # 自定义
#   selfhost/check_bin_portability.sh --self-test                    # 自证（含负控）
# 退出码：0 = 全部达标；1 = 有判红项（逐条打印）
# ============================================================
set -uo pipefail

BASELINE="2.34"
WANT_ARCH=""
REQUIRE_STATIC=0
SELFTEST=0
FILES=()
while [ $# -gt 0 ]; do
    case "$1" in
        -b|--baseline-glibc) BASELINE="$2"; shift 2 ;;
        --arch)              WANT_ARCH="$2"; shift 2 ;;
        --require-static)    REQUIRE_STATIC=1; shift ;;
        --self-test)         SELFTEST=1; shift ;;
        -h|--help)           sed -n '2,30p' "$0"; exit 0 ;;
        *)                   FILES+=("$1"); shift ;;
    esac
done

# glibc 版本比较：sort -V 取最大
# 依赖版本符号的取法（M168 加固）：readelf 是本脚本**本来就是必需**的工具（静态性判定用它），
#   故优先 `readelf --dyn-syms`；objdump 作兜底（某些精简镜像只有其中之一）。
#   ⚠️ 两者都取不到时**不当作"没有依赖"**（M159 教训：取不到就放行 = 暗门）——
#   动态件 + 版本不可判定 ⇒ **判红**（见 check_one）。
VERS_TOOL=""
have_vers_tool() {   # 0 = 至少一种可用
    [ -n "$VERS_TOOL" ] && return 0
    if readelf --dyn-syms --wide /bin/sh >/dev/null 2>&1; then VERS_TOOL=readelf; return 0; fi
    if objdump -T /bin/sh >/dev/null 2>&1; then VERS_TOOL=objdump; return 0; fi
    return 1
}
max_glibc_of() {   # $1=ELF → 打印最高 GLIBC_x.y（取不到则空）
    case "$VERS_TOOL" in
        readelf) readelf --dyn-syms --wide "$1" 2>/dev/null \
                   | grep -o 'GLIBC_[0-9]\+\.[0-9]\+' | sed 's/GLIBC_//' | sort -Vu | tail -1 ;;
        objdump) objdump -T "$1" 2>/dev/null \
                   | grep -o 'GLIBC_[0-9]\+\.[0-9]\+' | sed 's/GLIBC_//' | sort -Vu | tail -1 ;;
        *)       printf '' ;;
    esac
}
ver_le() { [ "$1" = "$2" ] && return 0; [ "$(printf '%s\n%s\n' "$1" "$2" | sort -V | tail -1)" = "$2" ]; }
is_elf()  { head -c 4 "$1" 2>/dev/null | od -An -tx1 | tr -d ' \n' | grep -qi '^7f454c46'; }
elf_machine() { readelf -h "$1" 2>/dev/null | sed -n 's/^ *Machine: *//p' | head -1; }

arch_ok() {   # $1=readelf Machine 文本 $2=想要的架构名
    case "$2" in
        x86_64)  case "$1" in *X86-64*|*Advanced*Micro*Devices*X86-64*) return 0 ;; esac ;;
        aarch64) case "$1" in *AArch64*) return 0 ;; esac ;;
        armv7)   case "$1" in *ARM*) return 0 ;; esac ;;
        riscv64) case "$1" in *RISC-V*) return 0 ;; esac ;;
        *) return 0 ;;
    esac
    return 1
}

check_one() {   # $1=文件 → 0 达标 / 1 判红
    local f="$1" mach gl max
    if [ ! -f "$f" ]; then echo "  ❌ $f：文件不存在"; return 1; fi
    if ! is_elf "$f"; then echo "  ·  $f：非 ELF（脚本/文本/静态库）· 跳过"; return 0; fi
    mach="$(elf_machine "$f")"
    if [ -n "$WANT_ARCH" ] && ! arch_ok "$mach" "$WANT_ARCH"; then
        echo "  ❌ $f：架构不符 —— 需要 $WANT_ARCH，实测 [$mach]"
        echo "     （发布包里混进别的架构 ⇒ 用户那边就是 Exec format error）"
        return 1
    fi
    if ! readelf -d "$f" 2>/dev/null | grep -q 'NEEDED'; then
        echo "  ✅ $f：全静态（无 DT_NEEDED）· [$mach]"
        return 0
    fi
    max="$(max_glibc_of "$f")"
    if [ -n "$max" ] && [ "$REQUIRE_STATIC" = 1 ]; then
        echo "  ❌ $f：要求全静态（--require-static），实测动态 · 最高 GLIBC_$max · [$mach]"
        readelf -d "$f" | grep NEEDED | sed 's/^/       | /'
        echo "       ⇒ 发布资产（尤其 aarch64 引导包）必须与 glibc 版本解耦：改用 --portable 重编"
        return 1
    fi
    if [ -z "$max" ]; then
        # M168 加固（发布侧首跑实测）：**取不到版本 ≠ 没有依赖** ⇒ 动态件判红。
        #   原先此处"提醒后放行"正是 M159 记过的暗门形态（取不到就跳过 ⇒ 门红不了）。
        echo "  ❌ $f：动态链接但**依赖版本不可判定**（$VERS_TOOL 取不到 GLIBC 符号）· [$mach]"
        readelf -d "$f" | grep NEEDED | sed 's/^/       | /'
        echo "       ⇒ 判红（不放行「未知」）：请改用全静态件，或在装有 binutils 的机器上复判"
        return 1
    fi
    if ver_le "$max" "$BASELINE"; then
        echo "  ✅ $f：动态 · 最高 GLIBC_$max ≤ 基线 $BASELINE · [$mach]"
        return 0
    fi
    echo "  ❌ $f：动态 · 最高 GLIBC_$max **高于**基线 $BASELINE · [$mach]"
    readelf -d "$f" | grep NEEDED | sed 's/^/       | /'
    echo "       ⇒ 在 glibc < $max 的发行版上会 'version GLIBC_$max not found'（装得上、跑不起来）"
    echo "       ⇒ 修法：重编为静态件（selfhost/native_bootstrap.sh --portable）或换低基线工具链"
    return 1
}

# ============================================================
# 自证（--self-test）：**含负控** —— 门若对坏件判绿，等于没有门
# ============================================================
if [ "$SELFTEST" = 1 ]; then
    have_vers_tool || true      # 自证前先选定提取器（readelf 优先 / objdump 兜底）
    W="$(mktemp -d /tmp/px-portability.XXXXXX)"; trap 'rm -rf "$W"' EXIT
    printf '#include <stdio.h>\nint main(void){puts("ok");return 0;}\n' > "$W/t.c"
    ok=0; bad=0
    sm() {   # $1=描述 $2=期望 rc $3=命令行（余下参数）
        local d="$1" want="$2"; shift 2
        "$@" > "$W/out" 2>&1; local rc=$?
        if [ "$rc" = "$want" ]; then ok=$((ok+1)); echo "  ✅ $d（rc=$rc）"
        else bad=$((bad+1)); echo "  ❌ $d（期望 rc=$want，实际 rc=$rc）"; sed 's/^/       | /' "$W/out" | head -8; fi
    }
    echo "== 自证: check_bin_portability.sh =="
    if gcc -static -O1 -o "$W/stat" "$W/t.c" 2>/dev/null; then
        sm "正例：静态件判绿" 0 "$0" "$W/stat"
    else
        echo "  ⚠️  本机 gcc 无法 -static（缺 static libc）⇒ 跳过静态正例（**不静默**，CI 需 glibc-static/libc6-dev）"
    fi
    if gcc -O1 -o "$W/dyn" "$W/t.c" 2>/dev/null; then
        if have_vers_tool; then
            echo "  （依赖版本提取器：$VERS_TOOL）"
            sm "正例：动态件（≤基线）判绿" 0 "$0" -b "$(max_glibc_of "$W/dyn")" "$W/dyn"
        else
            echo "  ⚠️  本机 readelf/objdump 都取不到依赖版本 ⇒ **跳过**正例（动态件）"
            echo "      （**不静默**：门在缺提取器的机器上只剩「静态/架构/不可判定即红」三条判据）"
        fi
        # 负控①：把基线压到 2.17 ⇒ 同一个动态件**必须**判红（版本可判定时按版本，
        #   不可判定时按"未知即红"—— 两条路都必须红）
        sm "负控①：动态件 vs 2.17 基线判红" 1 "$0" -b 2.17 "$W/dyn"
        # 负控②：架构断言 —— 拿本机件冒充 aarch64 ⇒ 必须判红
        sm "负控②：架构不符判红" 1 "$0" --arch aarch64 "$W/dyn"
        # 负控③：--require-static 下动态件必须判红（aarch64 引导包的口径）
        sm "负控②b：--require-static 下动态件判红" 1 "$0" --require-static "$W/dyn"
    else
        echo "  ⚠️  本机 gcc 不可用 ⇒ 跳过动态用例"
    fi
    printf 'not an elf\n' > "$W/notelf"
    sm "边界：非 ELF 记 INFO 不判红" 0 "$0" "$W/notelf"
    sm "负控③：文件不存在判红" 1 "$0" "$W/nope.bin"
    echo "  自证结果：通过 $ok · 失败 $bad"
    [ "$bad" = 0 ] || exit 1
    echo "CHECK-PORTABILITY-SELFTEST-OK"
    exit 0
fi

# ============================================================
# 主判据
# ============================================================
[ "${#FILES[@]}" -gt 0 ] || { echo "用法：$0 [-b 基线glibc] [--arch 架构] <文件...>；--self-test 自证" >&2; exit 2; }
have_vers_tool || true      # 提前选定依赖版本提取器（readelf 优先，objdump 兜底）
echo "== 二进制可移植性门：${#FILES[@]} 个文件 · glibc 基线 $BASELINE ${WANT_ARCH:+· 期望架构 $WANT_ARCH} =="
bad=0
for f in "${FILES[@]}"; do
    check_one "$f" || bad=$((bad+1))
done
echo "== 判红 $bad 项 =="
[ "$bad" = 0 ] || exit 1
echo "CHECK-PORTABILITY-OK"
