#!/usr/bin/env bash
# ============================================================================
# selfhost/check_emitc_hook.sh —— 提交前预警钩子的**功能自证**（M325 · 缺陷 566）
# ----------------------------------------------------------------------------
# 为什么不能只"检查文件存在"：
#   574 的教训 —— 门脚本「看起来装了 trap」，运行时一个都没挂上（bash -n 也绿）。
#   同理：`.githooks/pre-commit` 存在 + 可执行 ≠ 它真的会拦。
#   所以本脚本**真跑钩子**：临时 git 仓库里摆出 6 种现场，逐一断言 rc。
#
# 判据（每条都有反例，不是只测正路）：
#   A1 无相关文件        ⇒ rc=0   （不误伤）
#   A2 新增冻结集 .px    ⇒ rc≠0   （这是 r12 的真实事故形状）
#   A3 改冻结集老 .px    ⇒ rc≠0
#   A4 基准同交          ⇒ rc=0   （给了出路就要真的通）
#   A5 PX_SKIP_FREEZE_HOOK=1 ⇒ rc=0 且 .git/px-hook-skip.log 记了一行（跳过必须留痕）
#   A6 MERGE_HEAD 在     ⇒ rc=0   （合并提交不预警）
#   A7 install_hooks.sh --check：未装 ⇒ rc≠0；装完 ⇒ rc=0 且回读一致
#   A8 glob 同源提取自证：从 emitc_freeze.sh 提出 2 条 glob（防「提取器坏了 ⇒ 空集 ⇒ 全放行」）
#
# 用法：bash selfhost/check_emitc_hook.sh    （CI 与本地同口径）
# ============================================================================
set -uo pipefail
REPO=$(cd "$(dirname "$0")/.." && pwd)
HOOK="$REPO/.githooks/pre-commit"
INST="$REPO/tools/install_hooks.sh"
PASS=0; FAIL=0
ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

echo "=== 提交前预警钩子 · 功能自证（缺陷 566）==="

[ -f "$HOOK" ] && ok "pre-commit 存在" || { bad "缺 .githooks/pre-commit"; exit 1; }
[ -x "$HOOK" ] && ok "pre-commit 可执行" || bad "pre-commit 不可执行（chmod +x）"

# ── A8 glob 同源提取自证 ──
PAT=$(grep -oE 'selfhost/cases/\*\.px|examples/(\*|[A-Za-z0-9_]*)/\*\.px' "$REPO/selfhost/emitc_freeze.sh" 2>/dev/null | sort -u)
NP=$(printf '%s\n' "$PAT" | grep -c . || true)
if [ "$NP" -eq 2 ]; then ok "A8 glob 同源提取：2 条"; else bad "A8 只提出 $NP 条 glob（应 2 条）—— 提取器坏了，钩子会全放行"; fi
printf '%s\n' "$PAT" | sed 's/^/     /'

T=$(mktemp -d "${TMPDIR:-/tmp}/px-hooktest.XXXXXX") || exit 1
trap 'rm -rf "$T"' EXIT
W="$T/repo"
mkdir -p "$W/selfhost/cases" "$W/selfhost/golden" "$W/examples/m999_probe" "$W/tools"
cd "$W" || exit 1
git init -q . && git config user.email t@t && git config user.name t
# 给钩子一个「能被提取」的同源门脚本（形状与真门一致）
printf '%s\n' 'files() { ls selfhost/cases/*.px examples/*/*.px 2>/dev/null; }' > selfhost/emitc_freeze.sh
echo 'x' > README.md
git add -A && git commit -qm init

# A1 无关文件
echo 'a' > README.md; git add README.md
rc=$( ( cd "$W" && bash "$HOOK" >"$T/a1.txt" 2>&1 ); echo $? )
[ "$rc" -eq 0 ] && ok "A1 无关文件 ⇒ rc=0（不误伤）" || bad "A1 无关文件却 rc=$rc"
git reset -q

# A2 新增冻结集 .px
echo 'fn main() {}' > examples/m999_probe/probe.px; git add -A
rc=$( ( cd "$W" && bash "$HOOK" >"$T/a2.txt" 2>&1 ); echo $? )
if [ "$rc" -ne 0 ] && grep -q '发射冻结集' "$T/a2.txt"; then
    ok "A2 新增 examples/*/probe.px ⇒ rc=$rc 且给出两条出路"
else
    bad "A2 新增冻结集 .px 未被拦（rc=$rc）"; sed 's/^/     /' "$T/a2.txt" | head -5
fi

# A3 改冻结集老 .px
git reset -q; git rm -q --cached examples/m999_probe/probe.px 2>/dev/null; rm -f examples/m999_probe/probe.px
echo 'fn main() { 1 }' > selfhost/cases/a.px; git add selfhost/cases/a.px
rc=$( ( cd "$W" && bash "$HOOK" >"$T/a3.txt" 2>&1 ); echo $? )
[ "$rc" -ne 0 ] && ok "A3 改 selfhost/cases/*.px ⇒ rc=$rc" || bad "A3 改冻结集老文件未被拦（rc=0）"

# A4 基准同交 ⇒ 放行（出路必须真的通）
echo 'sha' > selfhost/golden/emitc_freeze.sha; git add selfhost/golden/emitc_freeze.sha
rc=$( ( cd "$W" && bash "$HOOK" >"$T/a4.txt" 2>&1 ); echo $? )
[ "$rc" -eq 0 ] && ok "A4 基准同交 ⇒ rc=0（出路是真的）" || bad "A4 基准同交仍被拦（rc=$rc）"
git reset -q selfhost/golden/emitc_freeze.sha

# A5 显式跳过：必须放行 + 必须留痕
rc=$( ( cd "$W" && PX_SKIP_FREEZE_HOOK=1 bash "$HOOK" >"$T/a5.txt" 2>&1 ); echo $? )
if [ "$rc" -eq 0 ] && [ -s .git/px-hook-skip.log ] && grep -q 'staged=' .git/px-hook-skip.log; then
    ok "A5 PX_SKIP_FREEZE_HOOK=1 ⇒ rc=0 且留痕（$(wc -l < .git/px-hook-skip.log) 行）"
else
    bad "A5 跳过出口不正常（rc=$rc · 留痕=$( [ -s .git/px-hook-skip.log ] && echo 有 || echo 无 )）"
fi

# A6 合并提交不预警
git rev-parse HEAD > .git/MERGE_HEAD
rc=$( ( cd "$W" && bash "$HOOK" >"$T/a6.txt" 2>&1 ); echo $? )
[ "$rc" -eq 0 ] && ok "A6 MERGE_HEAD 在 ⇒ rc=0（合并提交不预警）" || bad "A6 合并提交仍被拦（rc=$rc）"
rm -f .git/MERGE_HEAD

# A7 install_hooks.sh：未装 ⇒ 非 0；装完 ⇒ 0 且回读一致
cp "$INST" tools/install_hooks.sh; mkdir -p .githooks; cp "$HOOK" .githooks/pre-commit; chmod +x .githooks/pre-commit
rc1=$( ( cd "$W" && bash tools/install_hooks.sh --check >/dev/null 2>&1 ); echo $? )
rc2=$( ( cd "$W" && bash tools/install_hooks.sh >/dev/null 2>&1 ); echo $? )
got=$(cd "$W" && git config --get core.hooksPath)
rc3=$( ( cd "$W" && bash tools/install_hooks.sh --check >/dev/null 2>&1 ); echo $? )
if [ "$rc1" -ne 0 ] && [ "$rc2" -eq 0 ] && [ "$got" = ".githooks" ] && [ "$rc3" -eq 0 ]; then
    ok "A7 install：未装 rc=$rc1 ⇒ 装后 rc=$rc2 · hooksPath=$got · 再查 rc=$rc3"
else
    bad "A7 install_hooks.sh 行为不符（$rc1/$rc2/$rc3 · hooksPath='$got'）"
fi

echo "── 结果：通过 $PASS · 失败 $FAIL"
[ "$FAIL" -eq 0 ] && { echo "HOOK-VERIFY-OK"; exit 0; }
echo "HOOK-VERIFY-FAIL"; exit 1
