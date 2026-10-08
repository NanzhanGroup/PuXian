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

# M238：③ tag 命名合规判据要读豁免表。夹具里刻意留了一个**非里程碑 tag**
#   （v0.2.0-m114s2，用于钉住「建议的 tag 名不得被它污染」）⇒ 默认豁免它，
#   否则 A–I 段会全部变成 rc=3（判据没错，是夹具需要一个出口）。
EXF_OK="$TMP/exempt-ok.txt"
printf 'v0.2.0-m114s2\t2026-10-01\t自测夹具：非里程碑 tag\n' > "$EXF_OK"
g() {   # g [--allow] [guard 参数...] —— 跑守卫并记下 rc/输出
    local _ex="${SELFTEST_GUARD_EXEMPT:-$EXF_OK}"
    if [ "${1:-}" = "--allow" ]; then
        shift
        out=$(TAG_GUARD_EXEMPT="$_ex" TAG_GUARD_ALLOW_MISSING='自测放行理由' bash "$GUARD" --repo "$REPO" "$@" 2>&1); RC=$?
    else
        out=$(TAG_GUARD_EXEMPT="$_ex" bash "$GUARD" --repo "$REPO" "$@" 2>&1); RC=$?
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
ckhas "A2 建议的 tag 名 = 新形态 v<次段>.<里程碑>（M274）" "v0.2.900"
cksame "A2b 建议名不得退回旧形态 -m 后缀" "v0.2.0-m900"
cksame "A3 未被非里程碑 tag v0.2.0-m114s2 污染" "m114s2"

echo "── B 有 tag 必绿（且正文里的 M901 不带偏判据）──"
git -C "$REPO" tag v0.2.900 "$C1"
g --ref "$C1"; ck "B1 tag 存在且可达 ⇒ rc=0" 0
ckhas "B1 回显命中的 tag" "v0.2.900"
cksame "B2 正文里的未来里程碑 M901 未参与判据" "M901"

echo "── C 前导零里程碑号同样认（v0.2.0900 = m900）──"
git -C "$REPO" tag -d v0.2.900 >/dev/null
git -C "$REPO" tag v0.2.0900 "$C1"
g --ref "$C1"; ck "C1 v0.2.0900（前导零）视为 m900 ⇒ rc=0" 0

echo "── D grace 窗口（刚推上来、tag 还没打）──"
git -C "$REPO" tag -d v0.2.0900 >/dev/null
g --ref "$C1" --grace-min 999999; ck "D1 提交很新 + grace ⇒ 跳过、rc=0" 0
ckhas "D1 明示跳过" "跳过检查"

echo "── E tag 存在但**不可达**（打在旁支上）⇒ 仍红 ──"
cat >> "$REPO/CHANGELOG.md" <<'EOF'

### runtime · 假里程碑乙（M902 · qg-issue 0）
EOF
git -C "$REPO" commit -aqm "feat: M902 假里程碑乙"
git -C "$REPO" checkout -q -b side
git -C "$REPO" commit -qm "side: 旁支提交" --allow-empty
git -C "$REPO" tag v0.2.902
git -C "$REPO" checkout -q main
g --ref HEAD; ck "E1 tag 在旁支 ⇒ rc=1" 1
ckhas "E1 明示不可达" "可达链"

echo "── F 显式放行（留痕）──"
g --allow --ref HEAD; ck "F1 TAG_GUARD_ALLOW_MISSING 放行 ⇒ rc=0" 0
ckhas "F1 理由进日志" "自测放行理由"

echo "── G 就地补 tag ⇒ 转绿；中间里程碑缺 tag 只提示 ──"
git -C "$REPO" tag -d v0.2.902 >/dev/null
git -C "$REPO" tag v0.2.902 HEAD
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
git -C "$REPO" tag v0.2.910 "$OLD"
g --ref "$OLD" --grace-min 45
ck "I1 老提交 + tag 可达 ⇒ rc=0" 0
ckhas "I1 打印年龄行" "距今年龄"
cksame "I1 未走跳过分支" "跳过检查"
git -C "$REPO" tag -d v0.2.910 >/dev/null
g --ref "$OLD" --grace-min 45
ck "I2 老提交 + 缺 tag ⇒ rc=1" 1
ckhas "I2 判红时也打年龄行（红能读出真因）" "距今年龄"
g --ref "$OLD" --grace-min 99999
ck "I3 grace 覆盖老提交 ⇒ 跳过、rc=0" 0
ckhas "I3 明示跳过" "跳过检查"

echo "── J tag 命名合规（M238 新增 · 用户令 2026-10-01）──"
#   背景（实测事故）：误打 v0.2.0-m237s3 ⇒
#     ① packaging/pxrepo_mirror.sh 的版本序正则（= 规则本身）把它**静默过滤**
#        ⇒ 镜像永远停在旧版、**连 ❌ 都打不出来**；
#     ② packaging/build_rpm.sh 的 MILESTONE 取短横线之后整段 ⇒ rpm 包名被污染成
#        puxian-0.2.0-1.m237s3.el9.x86_64.rpm ⇒ 与 gh-pages 的 `.mNNN.` 交叉校验失配。
#   判据：未登记的违规 tag ⇒ rc=3（指名）；登记豁免 ⇒ rc=0；豁免表过期条目 ⇒ rc=3。
git -C "$REPO" tag v0.2.0-m903s1 HEAD
g --ref HEAD; ck "J1 未登记的违规 tag ⇒ rc=3" 3
ckhas "J1 指出「未登记」" "未登记"
ckhas "J1 列出违规 tag 名" "v0.2.0-m903s1"

git -C "$REPO" tag v0.2.910 HEAD
{ printf 'v0.2.0-m114s2\t2026-10-01\t夹具\n'; printf 'v0.2.0-m903s1\t2026-10-01\t夹具\n'; } > "$TMP/ex2.txt"
SELFTEST_GUARD_EXEMPT="$TMP/ex2.txt" g --ref HEAD; ck "J2 登记豁免后 ⇒ rc=0" 0
ckhas "J2 明示已登记豁免数" "已登记豁免"

{ printf 'v0.2.0-m114s2\t2026-10-01\t夹具\n'; printf 'v0.2.0-m903s1\t2026-10-01\t夹具\n'; printf 'v0.2.0-m999s9\t2026-10-01\t不存在的 tag\n'; } > "$TMP/ex3.txt"
SELFTEST_GUARD_EXEMPT="$TMP/ex3.txt" g --ref HEAD; ck "J3 豁免表过期条目 ⇒ rc=3" 3
ckhas "J3 指名「过期条目」" "过期条目"

git -C "$REPO" tag -d v0.2.0-m903s1 >/dev/null
g --ref HEAD; ck "J4 删掉违规 tag 后 ⇒ rc=0" 0

echo
echo "── K push 竞态：本地快照缺 tag、远端已有 ⇒ 补取后必须绿（负控：禁补取必红）──"
R2="$TMP/k-remote.git"; L2="$TMP/k-src"; L3="$TMP/k-clone"; L4="$TMP/k-clone-neg"
EXF_EMPTY="$TMP/exempt-empty.txt"; : > "$EXF_EMPTY"   # K 的夹具里没有非里程碑 tag ⇒ 用空豁免表
git init -q --bare "$R2"
git init -q -b main "$L2"
cat > "$L2/CHANGELOG.md" <<'EOF'
# Changelog

## [Unreleased]

### runtime · 假里程碑乙（M950 · 自测夹具）
EOF
git -C "$L2" add -A
git -C "$L2" commit -qm "feat: M950 假里程碑乙"
git -C "$L2" remote add origin "$R2"
git -C "$L2" push -q origin main
# 关键次序：**先**做一份「没有 tag」的快照（= CI 里 checkout 的那一刻）
git clone -q -b main "$R2" "$L3"   # -b main：bare 库的 HEAD 默认是 master（未推）⇒ 必须显式指定
# ⚠️ 负控必须有自己的**干净起点**：K1 的补取会把 tag 拉进它那一份快照 ⇒ 若 K2 复用同一份，
#    就变成「已经补过了」的假绿（M213/M214 立过的纪律：每道负控各自独立、干净起点）。
git clone -q -b main "$R2" "$L4"
# **再**打 tag 并推到远端（= 紧随 main 推出去的 tag）
git -C "$L2" tag v0.2.950
git -C "$L2" push -q origin v0.2.950
# 夹具形状自证：快照里没有、远端里有 —— 否则本段什么都没证明
if [ -z "$(git -C "$L3" tag -l v0.2.950)" ] && [ -z "$(git -C "$L4" tag -l v0.2.950)" ] \
   && [ -n "$(git -C "$R2" tag -l v0.2.950)" ]; then
    echo "  ✅ K0 夹具形状成立（两份快照都缺 tag、远端已有）"; pass=$((pass+1))
else
    echo "  ❌ K0 夹具形状不成立（快照/远端的 tag 状态与预期不符）"; fail=$((fail+1))
fi

out=$(TAG_GUARD_EXEMPT="$EXF_EMPTY" bash "$GUARD" --repo "$L3" --ref HEAD 2>&1); RC=$?
ck "K1 补取兜底后 ⇒ rc=0（不是真缺）" 0
ckhas "K1 明示是 push 竞态" "push 竞态"

out=$(TAG_GUARD_EXEMPT="$EXF_EMPTY" TAG_GUARD_NO_FETCH=1 bash "$GUARD" --repo "$L4" --ref HEAD 2>&1); RC=$?
ck "K2 负控：禁补取 ⇒ rc=1（证明红确实来自《快照缺 tag》）" 1
ckhas "K2 仍如实报缺 tag" "缺发布 tag"

echo "── L ③b 最高里程碑必须存在**合规形态**的 tag（M288 事故）──"
#   ⚠️ 本段**刻意**用历史形态 v0.2.0-m900 作夹具：③b 就是为「新打 tag 误用旧形态」立的。
#     其余各段（B/C/E/G/I/J/K）已一并改用合规形态 —— 它们要测的是别的性质，
#     夹具不该因形态问题而改变本意（「改判据 ⇒ 期望值移位」的标准动作）。
#   夹具：给 M900 打**旧形态** tag（合规形态不存在）⇒ 必须 rc=3
git -C "$REPO" tag v0.2.0-m900 "$C1"
# 夹具形状自证：合规形态确实不存在、旧形态确实存在（否则本段什么都没证明）
if [ -z "$(git -C "$REPO" tag -l v0.2.900)" ] && [ -n "$(git -C "$REPO" tag -l v0.2.0-m900)" ]; then
    echo "  ✅ L0 夹具形状成立（只有旧形态 v0.2.0-m900）"; pass=$((pass+1))
else
    echo "  ❌ L0 夹具形状不成立"; fail=$((fail+1))
fi
g --ref "$C1"; ck "L1 最高里程碑只有旧形态 tag ⇒ rc=3" 3
ckhas "L1 指名「旧形态」" "旧形态"
ckhas "L1 给出整改命令（make_tag.sh --milestone 900）" "make_tag.sh --milestone 900"
cksame "L1 不再报「缺发布 tag」（形态问题不是缺失问题）" "缺发布 tag"
# 负控：补上合规形态 ⇒ 必须 rc=0（证明那道红确由「缺合规形态」引起）
git -C "$REPO" tag v0.2.900 "$C1"
g --ref "$C1"; ck "L2 负控：补上合规 v0.2.900 ⇒ rc=0（红确由缺合规形态引起）" 0
git -C "$REPO" tag -d v0.2.900 >/dev/null 2>&1 || true

echo
if [ "$fail" -eq 0 ]; then
    echo "✅ tag_guard 自测全通过（pass=$pass fail=0）"
    exit 0
fi
echo "❌ tag_guard 自测失败：pass=$pass fail=$fail"
exit "$fail"
