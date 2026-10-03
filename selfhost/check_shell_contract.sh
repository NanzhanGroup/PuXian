#!/usr/bin/env bash
# ============================================================
# check_shell_contract.sh —— shell 契约守卫（M251s1 建立 · 缺陷 441）
# ------------------------------------------------------------
# 【来历 · 真事故】CI run 37152650582（2026-10-03）：
#   `selfhost/check_version_golden.sh` 的**自证**用 `sh "$SELF"` 调自己，而
#   ubuntu 的 /bin/sh 是 **dash** ⇒ dash 不支持 `set -o pipefail`；`set` 是
#   **特殊内建（special built-in）**，选项非法 ⇒ dash **立即退出**（rc=2）
#   ⇒ 判据主体**一行都没跑**。而自证用 `cmd && r=0 || r=1` 把 rc 归一化成 0/1
#   ⇒ 「脚本崩了」被误判成「脚本正确判红」⇒ 三道负控**假绿**。
#   ⚠️ 本机 /bin/sh 是 bash（RHEL 系）⇒ **本地永远绿**，只有 CI 才发作。
#
# 【判据】两条 —— 关键：**跨平台**，在本机（sh=bash）也能查出 CI/dash 才发作的问题
#   A 用 `sh` 调用脚本 —— 本仓脚本**普遍使用 bash 语义**（62 个含 bashism，
#     且全部 `#!/usr/bin/env bash`）⇒ 用 `sh` 调 = **结构性错误**，在 dash 上必崩。
#     豁免：同行含 `# sh-call-ok: <理由>`。
#   B 含 bashism 但 shebang **不是** bash —— 直接 `./x.sh` 会走 shebang ⇒ 崩。
#
# ⚠️ 本脚本**跳过自身**（其模式串里必然含 `sh `）—— 同 M213 缺陷 300 的做法。
# 用法：check_shell_contract.sh [--root .] [--self-test]
# ============================================================
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
# ⚠️ 自身**绝对路径**：自证要 cd 到夹具目录，相对路径 $0 会失效（M243/M247 老坑）
SELF_ABS="$(cd "$(dirname "$0")" 2>/dev/null && pwd)/$(basename "$0")"

SELF_BASE="check_shell_contract.sh"
# 判据 A 模式：`sh <脚本>`，要求 `sh` 是**独立命令词** —— 前面必须是行首/空白/分隔符，
#   ⚠️ 首版用 `[^A-Za-z0-9_-]` 导致**高假阳**：`devbuild.sh $x`、`a.sh b.sh`、`x.sh examples/`
#   里的 `sh` 是**文件名后缀**，却被当成命令 ⇒ 实测 6 处假阳（真仓库）。改为只认空白/分隔。
#   `bash x.sh` 因 `sh` 前是 `a` 而排除；`sh -c "…"` 因 `-c` 后不是路径/变量而排除。
PAT_SH_CALL='(^|[[:space:];&|(]|`|\$\()[[:space:]]*sh[[:space:]]+(-[a-zA-Z]+[[:space:]]+)*("?\$[A-Za-z_{]|[A-Za-z0-9_./+-]+\.sh)'
# 判据 B 模式：bash 专有语法
PAT_BASHISM='(set -o pipefail|set -uo pipefail|declare -A|\[\[ )'

if [ "$ST" = 1 ]; then
    ok=0; bad=0
    chk() { if [ "$1" = 1 ]; then ok=$((ok+1)); echo "  ✅ $2"; else bad=$((bad+1)); echo "  ❌ $2"; fi; }
    T=$(mktemp -d)
    mkdir -p "$T/selfhost" "$T/.github/workflows" "$T/examples/x"
    printf '#!/usr/bin/env bash\nset -uo pipefail\necho hi\n' > "$T/selfhost/ok.sh"
    printf '#!/usr/bin/env bash\nset -uo pipefail\nsh selfhost/ok.sh\n' > "$T/selfhost/bad_shcall.sh"
    printf '#!/bin/sh\nset -uo pipefail\necho hi\n' > "$T/selfhost/bad_shebang.sh"
    printf '#!/usr/bin/env bash\nset -uo pipefail\nsh selfhost/ok.sh  # sh-call-ok: 有意测试非 bash 契约\n' > "$T/selfhost/ok_exempt.sh"
    printf '#!/usr/bin/env bash\nset -uo pipefail\nbash selfhost/ok.sh\n' > "$T/selfhost/ok_bash.sh"
    printf 'jobs:\n  x:\n    steps:\n      - run: sh selfhost/ok.sh\n' > "$T/.github/workflows/w.yml"
    printf '#!/usr/bin/env bash\nset -uo pipefail\nsh -c "echo nested"\n' > "$T/examples/x/ok_shc.sh"

    run() { bash "$SELF_ABS" --root "$T" > "$T/out" 2>&1; echo $?; }

    # ① 违规集非空 ⇒ 判红，且**精确指名 bad_* 而不指名 ok_***
    r=$(run)
    chk $([ "$r" = 1 ] && echo 1 || echo 0) "T1 有违规 ⇒ rc=1（实测 rc=$r）"
    hit=0; miss=""
    for f in bad_shcall.sh bad_shebang.sh; do grep -qF "selfhost/$f" "$T/out" || miss="$miss $f"; done
    grep -qF 'workflows/w.yml' "$T/out" || miss="$miss w.yml"
    [ -z "$miss" ] && hit=1
    chk $hit "T2 三个违规都被指名（漏：${miss:-无}）"
    fp=""
    for f in ok.sh ok_exempt.sh ok_bash.sh ok_shc.sh; do grep -qF "selfhost/$f\|examples/x/$f" "$T/out" && fp="$fp $f"; done
    chk $([ -z "$fp" ] && echo 1 || echo 0) "T3 无假阳（被误报：${fp:-无}）"

    # ② 清掉全部违规 ⇒ 必须绿（证明判据不是恒红）
    rm -f "$T/selfhost/bad_shcall.sh" "$T/selfhost/bad_shebang.sh" "$T/.github/workflows/w.yml"
    r=$(run)
    chk $([ "$r" = 0 ] && echo 1 || echo 0) "T4 违规清零 ⇒ rc=0（实测 rc=$r）"

    # ③ 豁免失效：把 ok_exempt 的标记去掉 ⇒ 必须红（豁免不是"白名单文件"）
    sed -i 's|  # sh-call-ok.*||' "$T/selfhost/ok_exempt.sh"
    r=$(run)
    chk $([ "$r" = 1 ] && echo 1 || echo 0) "T5 去掉豁免标记 ⇒ 复判红（实测 rc=$r）"

    rm -rf "$T"
    echo ""; echo "自证：$ok 通过 / $bad 失败"
    [ "$bad" = 0 ] && exit 0 || exit 1
fi

echo "── shell 契约守卫（M251s1 · 缺陷 441）──"

mapfile -t SHF < <(find . -name '*.sh' -not -path './.git/*' 2>/dev/null | sort)
mapfile -t YMF < <(find .github/workflows -name '*.yml' 2>/dev/null | sort)
NSH=${#SHF[@]}; NYM=${#YMF[@]}

rc=0
vA=0; vB=0; nBash=0

for f in "${SHF[@]}" "${YMF[@]}"; do
    [ -f "$f" ] || continue
    case "$(basename "$f")" in "$SELF_BASE") continue;; esac   # 跳过自身（模式串自匹配）
    # ── 判据 A：用 sh 调用脚本 ──
    while IFS= read -r line; do
        ln="${line%%:*}"
        txt="${line#*:}"
        case "$txt" in *'#'*'sh-call-ok:'*) continue;; esac     # 豁免（带理由）
        echo "   ❌ A 用 sh 调用脚本：$f:$ln"
        echo "        $txt"
        echo "        ⇒ 本仓脚本普遍用 bash 语义；dash 下必崩（缺陷 441）。改用 bash，或加 '# sh-call-ok: <理由>'"
        vA=$((vA+1)); rc=1
    done < <(grep -nE "$PAT_SH_CALL" "$f" 2>/dev/null | grep -vE ':[[:space:]]*#')
    # ── 判据 B：bashism 但 shebang 不是 bash（**只对 .sh 生效** ——
    #    workflow 没有 shebang，对它查 B 是**作用域错误**，会恒判红）
    case "$f" in *.sh) ;; *) continue;; esac
    if grep -qE "$PAT_BASHISM" "$f" 2>/dev/null; then
        nBash=$((nBash+1))
        h=$(head -1 "$f")
        case "$h" in
            *bash*) ;;
            *) echo "   ❌ B 含 bashism 但 shebang 非 bash：$f（shebang='$h'）"
               echo "        ⇒ 直接 ./ 执行会走 shebang ⇒ 崩。改 shebang 或去掉 bashism"
               vB=$((vB+1)); rc=1;;
        esac
    fi
done

echo "   扫描：脚本 $NSH 个 · workflow $NYM 个"
echo "   判据 A（sh 调用脚本）：违例 $vA 处"
echo "   判据 B（bashism × shebang）：含 bashism 的脚本 $nBash 个 · 违例 $vB 处"

# ── 锚点自证：扫描面不得塌陷（**只在真仓库跑**；夹具目录里文件天然很少）──
if [ "$ROOT" = "." ]; then
    if [ "$NSH" -lt 150 ]; then
        echo "   ❌ 锚点自证失败：脚本数 $NSH < 150 ⇒ 扫描面塌陷（判据会静默变瞎）"; rc=1
    fi
    if [ "$nBash" -lt 30 ]; then
        echo "   ❌ 锚点自证失败：含 bashism 的脚本仅 $nBash < 30 ⇒ 模式失效？"; rc=1
    fi
fi

[ "$rc" = 0 ] && echo "SHELL-CONTRACT-OK" || echo "SHELL-CONTRACT-FAIL"
exit "$rc"
