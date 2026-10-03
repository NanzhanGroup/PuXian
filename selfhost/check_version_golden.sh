#!/usr/bin/env bash
# ============================================================
# check_version_golden.sh —— **版本字面量 ⇄ 自举基准 ⇄ 入库件** 三方一致（M250 建立 · 缺陷 435）
# ------------------------------------------------------------
# 【来历（CI 实测红）】
#   M249 引入 `bump_version.sh`：版本段每轮机械递增（`0.2.0 → 0.2.1`），改了 **7 文件 8 处**
#   字面量。其中 `selfhost/compiler.px` 的 `PXC_VER` **在自举链上** ——
#   它进 `compiler.px` 的 import 链，因此 `pxc` 编出的 C 里带这个字符串。
#   而 `selfhost/golden/compiler.c` / `compiler.bc.dump` 是**逐字节基准**，由**老版本**烘出
#   ⇒ 基准里还是 `0.2.0`，源码已是 `0.2.1` ⇒ `bootstrap_prove.sh` **必然失败**。
#   实测（CI run #484 · bdf7090）：两个 job 红 ——
#     · `原生 aarch64 自举自证 + 裸 build` ❌（step 4）
#     · `自举回归（PuXian-only）` ❌（step 17「自举证明（pxc 编译 compiler.px 与基准一致）」）
#
# 【为什么本地全量门没抓住】
#   `selfhost/gates.registry.sh` 明确记着：**`bootstrap_prove(_bc)` 属 CI 独占**（各约 7 分钟）
#   ⇒ 「本地绿 ≠ CI 绿」的第 N 次实例（M219 立过同款纪律）。本守卫是**廉价**的那一半：
#   不跑编译器，只做**三方字面量对拍**（毫秒级），把这一类整族挡在提交之前。
#
# 【判据（三方一致）】
#   ① `selfhost/compiler.px` 的 `PXC_VER` == `selfhost/golden/compiler.c` 里的版本串
#   ② 同上 == `selfhost/golden/compiler.bc.dump` 里的版本串
#   ③ 全部 `.px` 版本字面量（`tools/*.px` 的 PXLSP_VER/PXMC_VER 等）**彼此相等**
#      —— 一次发布只有一个版本号（M249 的 `bump_version.sh` 就是按这个口径写的）
#   ④ 入库件 `bootstrap/pxc --version` 与源码一致（**改了源码忘重烘**也能被抓到）
# 退出码：0=通过 · 1=判红 · 2=用法错 · 3=判据自身失效（一个版本字面量都没解析出来）
# 用法：check_version_golden.sh [--root .] [--self-test]
# ============================================================
# ── shell 契约（M251s1 · 缺陷 441）────────────────────────────
#   shebang 是 bash，且下面用了 bash 专有语义（`set -o pipefail`）。
#   ⚠️ 事故（CI run 37152650582 · 2026-10-03）：自证里用 `sh "$SELF"` 调自己，而
#   ubuntu 的 /bin/sh 是 **dash** ⇒ 不支持 `-o pipefail`；`set` 是**特殊内建**，
#   选项非法会让 dash **立即退出**（rc=2）⇒ 判据主体**一行都没跑**。
#   更糟的是自证用 `cmd && r=0 || r=1` 把 rc=2 归一化成 1 ⇒ 「脚本崩了」被误判成
#   「脚本正确判红」⇒ 三道负控**假绿**（其中 S2 的 grep 还命中了错误消息里的文件名）。
#   ⇒ 契约显式化：非 bash 调用**响亮退出**，而不是静默死在第 33 行。
if [ -z "${BASH_VERSION:-}" ]; then
    echo "❌ check_version_golden.sh 需要 bash（当前 shell 不是 bash：$0）。" >&2
    echo "   请用：bash $0 [--root .] [--self-test]" >&2
    exit 2
fi
set -uo pipefail
ROOT=.
ST=0
for a in "$@"; do
    case "$a" in
        --root) shift; ROOT="${1:-.}";;
        --root=*) ROOT="${a#--root=}";;
        --self-test) ST=1;;
    esac
done
cd "$ROOT" || { echo "❌ 无法进入 $ROOT"; exit 2; }

# 从 .px 里抽 `let XXX_VER = "a.b.c"`
ver_of() { sed -n 's/.*VER *= *"\([0-9][0-9.]*\)".*/\1/p' "$1" 2>/dev/null | head -1; }

SELF="$0"

if [ "$ST" = 1 ]; then
    ok=0; bad=0
    chk() { if [ "$1" = 1 ]; then ok=$((ok+1)); echo "  ✅ $2"; else bad=$((bad+1)); echo "  ❌ $2"; fi; }
    T=$(mktemp -d)
    mkdir -p "$T/selfhost/golden" "$T/tools" "$T/bootstrap"
    # 夹具 1：三方一致 ⇒ rc=0
    echo 'let PXC_VER = "9.9.9"' > "$T/selfhost/compiler.px"
    echo 'let PXI_VER = "9.9.9"' > "$T/selfhost/interp.px"
    echo 'let PXLSP_VER = "9.9.9"' > "$T/tools/pxlsp.px"
    echo 'const char* s = "9.9.9";' > "$T/selfhost/golden/compiler.c"
    echo 'CONST 9.9.9' > "$T/selfhost/golden/compiler.bc.dump"
    printf '#!/bin/sh\necho "pxc 9.9.9 (fixture)"\n' > "$T/bootstrap/pxc"; chmod +x "$T/bootstrap/pxc"
    bash "$SELF" --root "$T" > "$T/o1" 2>&1; r1=$?
    chk $([ "$r1" = 0 ] && grep -qF 'VERSION-GOLDEN-OK' "$T/o1" && echo 1 || echo 0) "S1 三方一致 ⇒ rc=0（实测 rc=$r1）"
    # 夹具 2（负控①）：golden 落后一个版本 ⇒ 判红且点名
    echo 'const char* s = "9.9.8";' > "$T/selfhost/golden/compiler.c"
    bash "$SELF" --root "$T" > "$T/o2" 2>&1; r2=$?
    chk $([ "$r2" = 1 ] && grep -qF 'VERSION-GOLDEN-FAIL' "$T/o2" && echo 1 || echo 0) "S2 负控①：golden 落后 ⇒ 判红（实测 rc=$r2）"
    echo 'const char* s = "9.9.9";' > "$T/selfhost/golden/compiler.c"
    # 夹具 3（负控②）：工具字面量彼此不等 ⇒ 判红
    echo 'let PXLSP_VER = "9.9.8"' > "$T/tools/pxlsp.px"
    bash "$SELF" --root "$T" > "$T/o3" 2>&1; r3=$?
    chk $([ "$r3" = 1 ] && grep -qF 'VERSION-GOLDEN-FAIL' "$T/o3" && echo 1 || echo 0) "S3 负控②：.px 字面量彼此不等 ⇒ 判红（实测 rc=$r3）"
    echo 'let PXLSP_VER = "9.9.9"' > "$T/tools/pxlsp.px"
    # 夹具 4（负控③）：入库件版本落后 ⇒ 判红
    printf '#!/bin/sh\necho "pxc 9.9.7 (fixture)"\n' > "$T/bootstrap/pxc"
    bash "$SELF" --root "$T" > "$T/o4" 2>&1; r4=$?
    chk $([ "$r4" = 1 ] && grep -qF 'VERSION-GOLDEN-FAIL' "$T/o4" && echo 1 || echo 0) "S4 负控③：入库件版本落后 ⇒ 判红（实测 rc=$r4）"
    printf '#!/bin/sh\necho "pxc 9.9.9 (fixture)"\n' > "$T/bootstrap/pxc"
    # 夹具 5：**一个版本字面量都解析不出来** ⇒ rc=3（拒绝静默回退成「全绿」）
    : > "$T/selfhost/compiler.px"
    bash "$SELF" --root "$T" > "$T/o5" 2>&1; r5=$?
    chk $([ "$r5" = 3 ] && grep -qF '解析不出 PXC_VER' "$T/o5" && echo 1 || echo 0) "S5 解析不出字面量 ⇒ rc=3（判据自身失效，不许绿）（实测 rc=$r5）"
    # ── S0 元判据（缺陷 442）─────────────────────────────────
    #   把「脚本**根本没跑到结尾**」与「脚本**正确地判红**」区分开：
    #   前者（崩溃 / 未执行 / shell 不对）在输出里**没有**任何判据标记。
    #   ⚠️ 事故里 S2/S3/S4 就是因为只有 rc 断言（而 rc 被归一化成 1）⇒ 假绿。
    _nomark=""
    for _i in 1 2 3 4 5; do
        grep -qE 'VERSION-GOLDEN-(OK|FAIL)|解析不出 PXC_VER' "$T/o$_i" || _nomark="$_nomark $_i"
    done
    chk $([ -z "$_nomark" ] && echo 1 || echo 0) "S0 元判据：每个夹具输出都含判据标记（未含夹具编号：${_nomark:-无}）"
    # 夹具 6（契约 · 缺陷 441）：**非 bash 调用**必须 rc=2 + 响亮
    #   ⚠️ 本机 /bin/sh 是 bash（RHEL 系）⇒ 需 CVG_NONBASH_SH 指定 dash 才能真跑；
    #      ubuntu CI（/bin/sh=dash）会**真跑**这条。跳过时**响亮登记**，不静默。
    _sh="${CVG_NONBASH_SH:-sh}"
    if [ -z "$("$_sh" -c 'echo ${BASH_VERSION:-}' 2>/dev/null)" ]; then
        "$_sh" "$SELF" --root "$T" > "$T/o6" 2>&1; r6=$?
        chk $([ "$r6" = 2 ] && grep -qF '需要 bash' "$T/o6" && echo 1 || echo 0) "S6 非 bash 调用（$_sh）⇒ rc=2 + 响亮（实测 rc=$r6）"
    else
        echo "  ⏭ S6 跳过：$_sh 是 bash（本机无非 bash sh）—— CI/dash 会真跑；本地可用 CVG_NONBASH_SH=<dash 路径> 复现"
    fi
    rm -rf "$T"
    echo ""; echo "自证：$ok 通过 / $bad 失败"
    [ "$bad" = 0 ] && exit 0 || exit 1
fi

# ── 判据主体 ──
PXC_VER=$(ver_of selfhost/compiler.px)
PXI_VER=$(ver_of selfhost/interp.px)
if [ -z "${PXC_VER:-}" ]; then
    echo "❌ 判据自身失效：从 selfhost/compiler.px 解析不出 PXC_VER（格式变了？）"; exit 3
fi

rc=0
echo "── 版本字面量三方对拍（M250 · 缺陷 435）──"
echo "   源码：selfhost/compiler.px PXC_VER=$PXC_VER · selfhost/interp.px PXI_VER=${PXI_VER:-（无）}"

# ① ② golden
for g in selfhost/golden/compiler.c selfhost/golden/compiler.bc.dump; do
    [ -f "$g" ] || { echo "   ⚠️ 缺 $g（跳过）"; continue; }
    if grep -qF "$PXC_VER" "$g"; then
        echo "   ✅ $g 含 $PXC_VER"
    else
        got=$(grep -oE '[0-9]+\.[0-9]+\.[0-9]+' "$g" | sort -u | head -3 | tr '\n' ' ')
        echo "   ❌ $g 里**没有** $PXC_VER（现含：${got:-无}）"
        echo "      修法：cd selfhost && ./bootstrap_prove.sh --update-golden（约 7 分钟）"
        echo "            ./bootstrap_prove_bc.sh --update-golden"
        rc=1
    fi
done

# ③ 全部 .px 字面量彼此相等
declare -A SEEN=()
for f in $(ls selfhost/*.px tools/*.px 2>/dev/null); do
    v=$(ver_of "$f")
    [ -n "${v:-}" ] && SEEN["$v"]="$f"
done
if [ "${#SEEN[@]}" -gt 1 ]; then
    echo "   ❌ .px 里的版本字面量**彼此不等**（一次发布应当只有一个版本号）："
    for v in "${!SEEN[@]}"; do echo "        $v  ← ${SEEN[$v]}"; done
    rc=1
elif [ "${#SEEN[@]}" -eq 1 ]; then
    echo "   ✅ 全部 .px 版本字面量一致（${#SEEN[@]} 个取值 · ${!SEEN[*]}）"
fi

# ④ 入库件
for b in bootstrap/pxc bootstrap/pxi; do
    [ -x "$b" ] || continue
    got=$("$b" --version 2>&1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
    if [ "$got" = "$PXC_VER" ]; then
        echo "   ✅ $b --version = $got"
    else
        echo "   ❌ $b --version = ${got:-（取不到）} ≠ 源码 $PXC_VER ⇒ **改了源码没重烘**"
        echo "      修法：bash selfhost/rebake_bin.sh --rebake-all"
        rc=1
    fi
done

[ "$rc" = 0 ] && echo "VERSION-GOLDEN-OK" || echo "VERSION-GOLDEN-FAIL"
exit "$rc"
