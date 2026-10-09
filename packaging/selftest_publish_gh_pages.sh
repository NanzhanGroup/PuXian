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
#      ⚠️ M293（缺陷 503）：形态**必须确定性构造**，不能靠时序蒙 ——
#        旧版用「racer 线程等 `PAGES_DIR/.git` 出现」做同步，而该目录**上一轮 run_it 就已存在**
#        ⇒ 等待循环第一次检查就 break（插桩实测 `loop_iters=break`）⇒ 同步失效，
#        退化成纯竞态；CI 负载下必然偶发误报（CI #521 `pass=12 fail=1`）。
#        现改为 **rsync 垫片施法**（④ 段严格在 ②clone 之后、⑥push 之前）+ **前提判据**（P4）。
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
has() {   # 子串判据（不用管道 ⇒ 与退出码解耦；本仓 P3 禁形只针对 `git … | grep -q`）
    case "$1" in *"$2"*) echo YES ;; *) echo NO ;; esac
}
# 前提判据（P4「前提可得」· M283）：形态**真的构造出来了**才算数 ——
#   否则「没构造出来」会被误读成「脚本行为不对」，正是缺陷 503 的形状。
#   判据 = ①垫片确实施法 ②远端 sha 确实变了（且两次都读得到）
premise_ok() { [ "$1" = "YES" ] && [ -n "$2" ] && [ -n "$3" ] && [ "$2" != "$3" ]; }

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

# ── 形态构造（确定性）：把「远端前进」钉在「②clone 之后 / ⑥push 之前」的窗口里 ──
# 手法 = **rsync 垫片**：发布脚本恰在 ④ 段调 rsync（严格晚于 ②clone、早于 ⑥push）
#   ⇒ 与时序、与机器负载无关（旧版的等待式同步为什么失效，见文件头 M293 说明）。
SHIM="$TMP/shim"; rm -rf "$SHIM"; mkdir -p "$SHIM"
REAL_RSYNC="$(command -v rsync)"
cat > "$SHIM/rsync" <<SHIM
#!/usr/bin/env bash
if [ ! -f "$TMP/racer.fired" ]; then
    : > "$TMP/racer.fired"
    git -C "$TMP/r3.git" rev-parse gh-pages > "$TMP/racer.before" 2>/dev/null || true
    W="\$(mktemp -d)"
    if git clone -q --depth 1 --branch gh-pages "$TMP/r3.git" "\$W" 2>/dev/null; then
        echo racer >> "\$W/rpm/9/x86_64/puxian-0.2.288-1.m288.el9.x86_64.rpm" 2>/dev/null || true
        git -C "\$W" add -A 2>/dev/null || true
        git -C "\$W" commit -qm "racer（并发发布）" 2>/dev/null || true
        git -C "\$W" push -q origin gh-pages 2>/dev/null || true
    fi
    git -C "$TMP/r3.git" rev-parse gh-pages > "$TMP/racer.after" 2>/dev/null || true
fi
exec "$REAL_RSYNC" "\$@"
SHIM
chmod +x "$SHIM/rsync"
mk_tree "$TMP/t3b" 0.2.290 m290
out="$(PATH="$SHIM:$PATH" PAGES_DIR="$TMP/pages" PXREPO_DIR="$TMP/t3b" \
       PX_PUBLISH_REPO_URL="$TMP/r3.git" GITHUB_WORKSPACE="$ROOT" GITHUB_REF_NAME=v0.2.290 \
       bash "$SCRIPT" 2>&1)"; rc=$?

# ── 先判「前提」，再判「脚本反应」（P4）──
FIRED="$([ -f "$TMP/racer.fired" ] && echo YES || echo NO)"
BEF="$(cat "$TMP/racer.before" 2>/dev/null || echo)"
AFT="$(cat "$TMP/racer.after" 2>/dev/null || echo)"
ck "③ 前提可得：形态确已构造（垫片施法 + 远端 sha 变）" \
   "$(premise_ok "$FIRED" "$BEF" "$AFT" && echo YES || echo NO)" "YES"
ck "③ 前提·细分①：垫片确已施法" "$FIRED" "YES"
ck "③ 前提·细分②：远端 sha 确已改变" "$([ -n "$BEF" ] && [ -n "$AFT" ] && [ "$BEF" != "$AFT" ] && echo YES || echo NO)" "YES"
ck "③ 前提·细分③：② clone 确已完成（施法在 clone 之后）" "$(has "$out" '② clone 完成')" "YES"
ck "③ rc≠0（push 被拒）" "$([ "$rc" != "0" ] && echo YES || echo NO)" "YES"
ckhas "③ 分段线索在场（能读出卡在哪一段）" "$out" "⏱ +"
ckhas "③ 冒出「⑥ 已 commit」段 ⇒ 说明走到了 push" "$out" "⑥ 已 commit"
ck "③ 未走到 push 成功（不含「⑥ push 完成」）" "$(has "$out" '⑥ push 完成')" "NO"
ck "③ 未谎报发布成功（不含「✅ 已推送到 gh-pages」）" "$(has "$out" '✅ 已推送到 gh-pages')" "NO"
ck "③ 远端未被覆盖（仍是垫片那笔提交）" "$(git -C "$TMP/r3.git" rev-parse gh-pages)" "$AFT"
# 判据自证（M223 纪律）：把「没构造出来」的三种状态喂给**同一个**前提判据 ⇒ 必须判红（否则它没有牙）
ck "③ 自证·未施法 ⇒ 前提不成立" "$(premise_ok NO aaa bbb && echo YES || echo NO)" "NO"
ck "③ 自证·施法但 sha 未变 ⇒ 前提不成立" "$(premise_ok YES aaa aaa && echo YES || echo NO)" "NO"
ck "③ 自证·施法但读不到 sha ⇒ 前提不成立" "$(premise_ok YES '' '' && echo YES || echo NO)" "NO"

echo
echo "── 覆盖边界（如实登记）──"
echo "  · 用例③ 构造的是「远端在 ②clone 与 ⑥push 之间前进」这一**形状**；手段是 rsync 垫片（④ 段）。"
echo "    对被测脚本而言与「真有两个进程并发推送」等价（都只表现为 push 被拒）。"
echo "  · 未覆盖：发布脚本的 else 分支（本地无 remote-tracking ⇒ 先 fetch 再 push -u）——"
echo "    该分支只在「远端有 gh-pages 但 clone 走了 orphan」时出现，本自测三种形态都不触发。"
echo "  · 未覆盖：凭证写入路径（PX_PUBLISH_REPO_URL 覆盖时**刻意跳过**）⇒ 只测发布逻辑，不测鉴权。"
echo "  · 判据基础：用例③ 的「前提可得」（P4）必须先成立 —— 判据自证 3 条保证它不空转。"

if [ "$fail" -eq 0 ]; then
    echo "✅ publish_gh_pages 自测全通过（pass=$pass fail=0）"
    exit 0
fi
echo "❌ publish_gh_pages 自测失败：pass=$pass fail=$fail"
exit "$fail"
