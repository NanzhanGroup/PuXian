#!/usr/bin/env bash
# ============================================================
# packaging/selftest_publish_gh_pages.sh —— gh-pages 发布脚本的**离线**自测（M288s1）
# ------------------------------------------------------------
# 用**本地 bare 远端**（`PX_PUBLISH_REPO_URL`）复现三类形态，不联网：
#   ① 正常发布：新版本更高 ⇒ rc=0，远端 rpm 树被替换（旧版被 --delete 清掉）
#   ② 版本回退：新版本更低 ⇒ **单调性守卫拒绝**（rc≠0），且**远端保持原样**（不静默覆盖）
#   ③ **远端在 clone 之后前进**（并发推送 / 手工提交）⇒ `push` 被拒 ⇒ rc≠0
#      ⭐ 这一条是 2026-10-08 事故的候选成因之一：内联时**只报 exit code 1**，读不出是哪条。
#        本自测断言「它能自己说清失败在哪一段」（⏱ 分段标记在场）。
# 用法：bash packaging/selftest_publish_gh_pages.sh
# 退出码：0 = 全通过；非 0 = 失败用例数
# ============================================================
set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
SCRIPT="$HERE/publish_gh_pages.sh"
[ -f "$SCRIPT" ] || { echo "❌ 缺 $SCRIPT"; exit 1; }
bash -n "$SCRIPT" || { echo "❌ $SCRIPT 语法错误"; exit 1; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
export GIT_AUTHOR_NAME=t GIT_AUTHOR_EMAIL=t@example.com
export GIT_COMMITTER_NAME=t GIT_COMMITTER_EMAIL=t@example.com

pass=0; fail=0
ck() {
    if [ "$2" = "$3" ]; then echo "  ✅ $1（$2）"; pass=$((pass+1))
    else echo "  ❌ $1：期望 $3，实际 $2"; fail=$((fail+1)); fi
}
ckhas() {
    if printf '%s' "$2" | grep -q -- "$3"; then echo "  ✅ $1（含「$3」）"; pass=$((pass+1))
    else echo "  ❌ $1：不含「$3」"; printf '%s\n' "$2" | sed 's/^/       | /'; fail=$((fail+1)); fi
}

# ── 造一个裸远端 + 初始 gh-pages（含 0.2.286 的 rpm 树）──
mk_remote() {   # $1=目录
    git init -q --bare "$1"
    local w="$TMP/seed"; rm -rf "$w"; git init -q -b gh-pages "$w"
    mkdir -p "$w/rpm/9/x86_64"
    echo old > "$w/rpm/9/x86_64/puxian-0.2.286-1.m286.el9.x86_64.rpm"
    git -C "$w" add -A; git -C "$w" commit -qm "seed 286"
    git -C "$w" push -q "$1" gh-pages
}
mk_tree() {     # $1=目录  $2=ver  $3=ms
    rm -rf "$1"; mkdir -p "$1/9/x86_64"
    echo "payload-$2" > "$1/9/x86_64/puxian-$2-1.$3.el9.x86_64.rpm"
}

run_it() {      # $1=newtree  $2=remote  $3=ws  → out/rc
    out="$(PAGES_DIR="$TMP/pages" PXREPO_DIR="$1" PX_PUBLISH_REPO_URL="$2" \
           GITHUB_WORKSPACE="$3" GITHUB_REF_NAME=v0.2.288 \
           bash "$SCRIPT" 2>&1)"; rc=$?
}

echo "── ① 正常发布（新 0.2.288 > 旧 0.2.286）──"
mk_remote "$TMP/r1.git"
mk_tree "$TMP/t1" 0.2.288 m288
run_it "$TMP/t1" "$TMP/r1.git" "$ROOT"
ck "① rc=0" "$rc" "0"
ckhas "① 分段计时在场（①探测 / ②clone / ③守卫 / ④rsync / ⑥push）" "$out" "⏱ +"
ckhas "① 守卫放行" "$out" "守卫通过"
ckhas "① 推送到 gh-pages" "$out" "已推送到 gh-pages/rpm/"
GOT="$(git -C "$TMP/r1.git" ls-tree -r --name-only gh-pages | grep '\.rpm$' | tr '\n' ' ')"
ck "① 远端只剩新版（--delete 生效）" "$GOT" "rpm/9/x86_64/puxian-0.2.288-1.m288.el9.x86_64.rpm "

echo "── ② 版本回退（新 0.2.280 < 旧 0.2.286）⇒ 守卫拒绝、远端不动 ──"
mk_tree "$TMP/t2" 0.2.280 m280
run_it "$TMP/t2" "$TMP/r1.git" "$ROOT"
ck "② rc≠0" "$([ "$rc" != "0" ] && echo YES || echo NO)" "YES"
ckhas "② 指名「版本回退」" "$out" "低于"
ckhas "② 失败落在③守卫段（分段可读）" "$out" "③ 发布单调性守卫"
GOT2="$(git -C "$TMP/r1.git" ls-tree -r --name-only gh-pages | grep '\.rpm$' | tr '\n' ' ')"
ck "② 远端未被覆盖（仍是 0.2.288）" "$GOT2" "rpm/9/x86_64/puxian-0.2.288-1.m288.el9.x86_64.rpm "

echo "── ③ ⭐ 远端在 clone 之后前进 ⇒ push 被拒 ⇒ 失败必须能读出「卡在哪一段」──"
mk_remote "$TMP/r3.git"
mk_tree "$TMP/t3" 0.2.288 m288
run_it "$TMP/t3" "$TMP/r3.git" "$ROOT"
ck "③ 基线：正常发布 ⇒ rc=0" "$rc" "0"

# 竞态器：**等到脚本 clone 完成**（PAGES_DIR/.git 出现）再往远端推一个竞争提交
#   ⇒ 与「并发发布 / 手工推送」同一形状，且**不依赖脆弱的 sleep 时序**
cat > "$TMP/racer.sh" <<'RACER'
#!/usr/bin/env bash
RURL="$1"; PDIR="$2"
for _ in $(seq 1 500); do [ -d "$PDIR/.git" ] && break; sleep 0.02; done
sleep 0.10
W="$(mktemp -d)"; git clone -q --depth 1 --branch gh-pages "$RURL" "$W" 2>/dev/null || exit 0
echo racer >> "$W/rpm/9/x86_64/puxian-0.2.288-1.m288.el9.x86_64.rpm" 2>/dev/null || \
    echo racer > "$W/rpm/9/x86_64/puxian-0.2.288-1.m288.el9.x86_64.rpm"
git -C "$W" add -A; git -C "$W" commit -qm "racer（并发发布）" 2>/dev/null
git -C "$W" push -q origin gh-pages 2>/dev/null
RACER
chmod +x "$TMP/racer.sh"
mk_tree "$TMP/t3b" 0.2.290 m290
bash "$TMP/racer.sh" "$TMP/r3.git" "$TMP/pages" &
RACERPID=$!
out="$(PAGES_DIR="$TMP/pages" PXREPO_DIR="$TMP/t3b" PX_PUBLISH_REPO_URL="$TMP/r3.git" \
       GITHUB_WORKSPACE="$ROOT" GITHUB_REF_NAME=v0.2.290 bash "$SCRIPT" 2>&1)"; rc=$?
wait "$RACERPID" 2>/dev/null
ck "③ rc≠0（push 被拒）" "$([ "$rc" != "0" ] && echo YES || echo NO)" "YES"
ckhas "③ 分段线索在场（能读出卡在哪一段）" "$out" "⏱ +"
ckhas "③ 冒出「⑥ 已 commit」段 ⇒ 说明走到了 push" "$out" "⑥ 已 commit"

echo
if [ "$fail" -eq 0 ]; then
    echo "✅ publish_gh_pages 自测全通过（pass=$pass fail=0）"
    exit 0
fi
echo "❌ publish_gh_pages 自测失败：pass=$pass fail=$fail"
exit "$fail"
