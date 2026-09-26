#!/usr/bin/env bash
# ============================================================
# packaging/selftest_tag_guard.sh —— 发布 tag 守卫的**离线**自测（qg-issue 86）
# ------------------------------------------------------------
# 不联网、不碰真仓库：在 mktemp 里造一个假仓库，逐条断言判据与退出码。
# 判据来源 = 设计文档 §判据（最高里程碑必须有可达的 v*-m<NNN> tag）。
# 用法: bash packaging/selftest_tag_guard.sh
# 退出码: 0 = 全部通过；非 0 = 失败用例数
# ============================================================
set -uo pipefail

GUARD="$(cd "$(dirname "$0")" && pwd)/tag_guard.sh"
[ -f "$GUARD" ] || { echo "❌ 缺 $GUARD"; exit 1; }
bash -n "$GUARD" || { echo "❌ $GUARD 语法错误"; exit 1; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
REPO="$TMP/repo"
mkdir -p "$REPO"

export GIT_AUTHOR_NAME=t GIT_AUTHOR_EMAIL=t@example.com
export GIT_COMMITTER_NAME=t GIT_COMMITTER_EMAIL=t@example.com
git -C "$REPO" init -q -b main

pass=0; fail=0
RC=0; out=""

g() {   # g [--allow] [guard 参数...] —— 跑守卫并记下 rc/输出
    if [ "${1:-}" = "--allow" ]; then
        shift
        out=$(TAG_GUARD_ALLOW_MISSING='自测放行理由' bash "$GUARD" --repo "$REPO" "$@" 2>&1); RC=$?
    else
        out=$(bash "$GUARD" --repo "$REPO" "$@" 2>&1); RC=$?
    fi
}
ck() {   # ck <用例名> <期望 rc>
    if [ "$2" = "$RC" ]; then echo "  ✅ $1（rc=$RC）"; pass=$((pass+1))
    else echo "  ❌ $1：期望 rc=$2，实际 rc=$RC"; printf '%s\n' "$out" | sed 's/^/       | /'; fail=$((fail+1)); fi
}
ckhas() {   # ckhas <用例名> <关键词>
    if printf '%s\n' "$out" | grep -q -- "$2"; then echo "  ✅ $1（输出含「$2」）"; pass=$((pass+1))
    else echo "  ❌ $1：输出不含「$2」"; printf '%s\n' "$out" | sed 's/^/       | /'; fail=$((fail+1)); fi
}
cksame() {  # cksame <用例名> <不期望出现的词>
    if printf '%s\n' "$out" | grep -q -- "$2"; then
        echo "  ❌ $1：不该出现「$2」"; printf '%s\n' "$out" | sed 's/^/       | /'; fail=$((fail+1))
    else echo "  ✅ $1（未受「$2」干扰）"; pass=$((pass+1)); fi
}

# ── 夹具 C1：标题行 M900；**正文**写未来里程碑 M901（不得参与判据）──
cat > "$REPO/CHANGELOG.md" <<'EOF'
# Changelog

## [Unreleased]

### runtime · 假里程碑甲（M900 · qg-issue 0）

- 正文里写「⇒ 建议 M901」——未来里程碑，**不得**参与判据
EOF
git -C "$REPO" add -A
git -C "$REPO" commit -qm "feat: M900 假里程碑甲"
C1="$(git -C "$REPO" rev-parse HEAD)"
# 真实仓库里同时存在「非里程碑 tag」（v0.2.0-m114s2）与更早的里程碑 tag ⇒
# 一并放进夹具，钉住「建议的 tag 名不得被它们污染」（首跑实测踩到过）
git -C "$REPO" tag v0.2.0-m800 "$C1"
git -C "$REPO" tag v0.2.0-m114s2 "$C1"

echo "── A 缺 tag 必红 ──"
g --ref "$C1"; ck "A1 无 tag ⇒ rc=1" 1
ckhas "A1 指出最高里程碑 M900" "M900"
ckhas "A1 给出补打 tag 的处置" "git tag -a"
ckhas "A1 标出名册缺 tag" "❌ 无 tag"
ckhas "A2 建议的 tag 名沿用最新里程碑 tag 的版本段" "v0.2.0-m900"
cksame "A3 未被非里程碑 tag v0.2.0-m114s2 污染" "m114s2"

echo "── B 有 tag 必绿（且正文里的 M901 不带偏判据）──"
git -C "$REPO" tag v0.2.0-m900 "$C1"
g --ref "$C1"; ck "B1 tag 存在且可达 ⇒ rc=0" 0
ckhas "B1 回显命中的 tag" "v0.2.0-m900"
cksame "B2 正文里的未来里程碑 M901 未参与判据" "M901"

echo "── C 前导零里程碑号同样认（-m0900 = m900）──"
git -C "$REPO" tag -d v0.2.0-m900 >/dev/null
git -C "$REPO" tag v0.2.0-m0900 "$C1"
g --ref "$C1"; ck "C1 v0.2.0-m0900 视为 m900 ⇒ rc=0" 0

echo "── D grace 窗口（刚推上来、tag 还没打）──"
git -C "$REPO" tag -d v0.2.0-m0900 >/dev/null
g --ref "$C1" --grace-min 999999; ck "D1 提交很新 + grace ⇒ 跳过、rc=0" 0
ckhas "D1 明示跳过" "跳过检查"

echo "── E tag 存在但**不可达**（打在旁支上）⇒ 仍红 ──"
cat >> "$REPO/CHANGELOG.md" <<'EOF'

### runtime · 假里程碑乙（M902 · qg-issue 0）
EOF
git -C "$REPO" commit -aqm "feat: M902 假里程碑乙"
git -C "$REPO" checkout -q -b side
git -C "$REPO" commit -qm "side: 旁支提交" --allow-empty
git -C "$REPO" tag v0.2.0-m902
git -C "$REPO" checkout -q main
g --ref HEAD; ck "E1 tag 在旁支 ⇒ rc=1" 1
ckhas "E1 明示不可达" "可达链"

echo "── F 显式放行（留痕）──"
g --allow --ref HEAD; ck "F1 TAG_GUARD_ALLOW_MISSING 放行 ⇒ rc=0" 0
ckhas "F1 理由进日志" "自测放行理由"

echo "── G 就地补 tag ⇒ 转绿；中间里程碑缺 tag 只提示 ──"
git -C "$REPO" tag -d v0.2.0-m902 >/dev/null
git -C "$REPO" tag v0.2.0-m902 HEAD
g --ref HEAD; ck "G1 main 上补 tag ⇒ rc=0" 0
ckhas "G1 中间里程碑缺 tag 只提示" "ℹ️ 名册里以下里程碑无 tag"

echo "── H 参数/环境错误 ⇒ rc=2 ──"
g --ref no-such-ref; ck "H1 ref 不存在 ⇒ rc=2" 2
printf '# Changelog\n\n## [Unreleased]\n' > "$REPO/CHANGELOG.md"
git -C "$REPO" commit -aqm "docs: 标题行里没有里程碑"
g --ref HEAD; ck "H2 标题行无里程碑 ⇒ rc=2" 2
rm -f "$REPO/CHANGELOG.md"
git -C "$REPO" commit -aqm "chore: 删掉 CHANGELOG"
g --ref HEAD; ck "H3 提交里没有 CHANGELOG.md ⇒ rc=2" 2
g --repo "$TMP/不存在"; ck "H4 仓库目录不存在 ⇒ rc=2" 2

echo "── I 年龄行**无条件**打印 + grace 边界（M215 收尾 · 实测事故回归）──"
#   事故（2026-09-26）：commit 16:45 CST → 跑完全量门 `m116_gates.sh`（~70 分钟）
#   → push 18:08 CST ⇒ 年龄 **83 分钟** > grace 45 ⇒ push 触发的 run **误红**；
#   而红条只说「缺发布 tag」，**读不出真因是「窗口太小」**。
#   ⇒ 修法：① grace 45 → **180**（覆盖「提交 → 跑全门 → 推送」的正常窗口）；
#           ② 年龄行**无条件打印**（判红时 run 摘要里能直接读到真值）。
cat > "$REPO/CHANGELOG.md" <<'EOF'
# Changelog

## [Unreleased]

### runtime · 老提交夹具（M910）
EOF
#   夹具日期取 **3 小时前**（不是"很久以前"）：要的是"年龄 > 45 但 < 大 grace"这个边界，
#   首版写成 2020 年 ⇒ 年龄 354 万分钟，连 `--grace-min 99999` 都盖不住 ⇒ I3 自伤。
OLDSTAMP=$(date -u -d '3 hours ago' +%Y-%m-%dT%H:%M:%SZ)
export GIT_AUTHOR_DATE="$OLDSTAMP" GIT_COMMITTER_DATE="$OLDSTAMP"
git -C "$REPO" add -A
git -C "$REPO" commit -qm "feat: M910（3 小时前的夹具）"
unset GIT_AUTHOR_DATE GIT_COMMITTER_DATE
OLD=$(git -C "$REPO" rev-parse HEAD)
git -C "$REPO" tag v0.2.0-m910 "$OLD"
g --ref "$OLD" --grace-min 45
ck "I1 老提交 + tag 可达 ⇒ rc=0" 0
ckhas "I1 打印年龄行" "距今年龄"
cksame "I1 未走跳过分支" "跳过检查"
git -C "$REPO" tag -d v0.2.0-m910 >/dev/null
g --ref "$OLD" --grace-min 45
ck "I2 老提交 + 缺 tag ⇒ rc=1" 1
ckhas "I2 判红时也打年龄行（红能读出真因）" "距今年龄"
g --ref "$OLD" --grace-min 99999
ck "I3 grace 覆盖老提交 ⇒ 跳过、rc=0" 0
ckhas "I3 明示跳过" "跳过检查"

echo
if [ "$fail" -eq 0 ]; then
    echo "✅ tag_guard 自测全通过（pass=$pass fail=0）"
    exit 0
fi
echo "❌ tag_guard 自测失败：pass=$pass fail=$fail"
exit "$fail"
