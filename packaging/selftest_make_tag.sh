#!/usr/bin/env bash
# ============================================================
# packaging/selftest_make_tag.sh —— make_tag.sh 的**离线**自测（M248）
# ------------------------------------------------------------
# 不联网、不碰真仓库：在 mktemp 里造一个假仓库，逐条断言判据与退出码。
# 本自测的**核心**是那条事故判据：补丁后缀形态（v0.2.0-m245s1）**必须拒绝创建**，
#   且拒绝之后**仓库里不得多出任何 tag**（只判 rc 不判副作用 = 半边判据）。
#
# 用法: bash packaging/selftest_make_tag.sh
# 退出码: 0 = 全部通过；非 0 = 失败用例数
# ============================================================
set -uo pipefail

SELF_DIR="$(cd "$(dirname "$0")" && pwd)"
MT="$SELF_DIR/make_tag.sh"
TG="$SELF_DIR/tag_guard.sh"
[ -f "$MT" ] || { echo "❌ 缺 $MT"; exit 1; }
bash -n "$MT" || { echo "❌ $MT 语法错误"; exit 1; }

# 第 ① 道防线的规则必须与事后守卫（tag_guard.sh）**逐字符相同** —— 判定见 H 段。
RE_MT="$(grep -oE "TAG_NAME_RE='[^']+'" "$MT" | head -1 | sed "s/^TAG_NAME_RE='//; s/'$//")"
RE_TG="$(grep -oE "TAG_NAME_RE='[^']+'" "$TG" 2>/dev/null | head -1 | sed "s/^TAG_NAME_RE='//; s/'$//")"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

export GIT_AUTHOR_NAME=t GIT_AUTHOR_EMAIL=t@example.com
export GIT_COMMITTER_NAME=t GIT_COMMITTER_EMAIL=t@example.com

pass=0; fail=0
ok()   { pass=$((pass+1)); printf '  ✅ %s\n' "$1"; }
bad()  { fail=$((fail+1)); printf '  ❌ %s\n' "$1"; }
chk()  { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1 —— 期望 $3，实得 $2"; fi; }
chkn() { if [ "$2" != "$3" ]; then ok "$1（$2）"; else bad "$1 —— 不应等于 $3"; fi; }

# ---------- 夹具仓库 ----------
newrepo() {  # newrepo <目录名>  —— 造一个干净夹具（含 CHANGELOG 最高里程碑 M248）
    local d="$TMP/$1"
    rm -rf "$d"; mkdir -p "$d"
    git -C "$d" init -q -b main
    printf '# CHANGELOG\n\n## M247：前一轮\n\n## M248：本轮\n' > "$d/CHANGELOG.md"
    git -C "$d" add -A
    git -C "$d" commit -qm "M248：夹具初始提交"
    printf '%s' "$d"
}
run() {  # run <仓库> <参数...> —— 跑 make_tag 并记下 rc/输出（**不经管道**，避免 SIGPIPE 污染 rc）
    local d="$1"; shift
    out="$(PX_TAG_REPO="$d" bash "$MT" "$@" 2>&1)"; RC=$?
    return 0
}
tags() { git -C "$1" tag -l | sort | tr '\n' ' '; }

echo "── 判据载体自证 ──"
[ -n "$RE_MT" ] && ok "从 make_tag.sh 抽到规则: $RE_MT" || bad "make_tag.sh 里找不到 TAG_NAME_RE"
[ -n "$RE_TG" ] && ok "从 tag_guard.sh 抽到规则: $RE_TG" || bad "tag_guard.sh 里找不到 TAG_NAME_RE"
chk "H 两道防线的规则逐字符相同" "$RE_MT" "$RE_TG"
chk "H 规则不接受补丁后缀（负控自证）" \
    "$(printf 'v0.2.0-m245s1' | grep -qE "$RE_MT" && echo MATCH || echo NOMATCH)" "NOMATCH"

echo "── A 正向：构造名（--milestone）──"
R="$(newrepo a)"; run "$R" --milestone 249
chk "A rc=0" "$RC" "0"
chk "A 建出 v0.2.249" "$(tags "$R")" "v0.2.249 "
chk "A 是 annotated tag" "$(git -C "$R" cat-file -t refs/tags/v0.2.249)" "tag"
chk "A 指向 HEAD" \
    "$(git -C "$R" rev-parse 'refs/tags/v0.2.249^{commit}')" "$(git -C "$R" rev-parse HEAD)"

echo "── B 正向：里程碑自 CHANGELOG 推断 ──"
R="$(newrepo b)"; run "$R"
chk "B rc=0" "$RC" "0"
chk "B 推断出 M248 ⇒ v0.2.248" "$(tags "$R")" "v0.2.248 "
case "$out" in *"M248"*) ok "B 输出里报出推断到的里程碑";; *) bad "B 输出未提里程碑: $out";; esac

echo "── C ★核心负控：补丁后缀形态必须被拒绝 ──"
R="$(newrepo c)"; run "$R" --name v0.2.0-m245s1
chk "C rc=3（命名不合规）" "$RC" "3"
chk "C **没有**创建任何 tag（只判 rc 不判副作用 = 半边判据）" "$(tags "$R")" ""
case "$out" in *"-mNsN"*|*"补丁后缀"*) ok "C 错误信息指到真因（补丁后缀）";; *) bad "C 信息未指真因: $out";; esac
case "$out" in *"打包 r"*|*"镜像"*) ok "C 信息说明了后果（镜像同步）";; *) bad "C 未说明后果";; esac

echo "── D 不合规名矩阵（全部必须 rc=3 且零副作用）──"
for n in v0.2.0-m245s1 v0.2.0-m245s2 v0.2.0-245 v0.2.0-M245 v0.2.0-m245x \
         m245 v0.2.0-m249 v0.2.0-m245.1 v1.2-m245; do
    R="$(newrepo "d$pass$fail$$")"
    run "$R" --name "$n"
    rc="$RC"; created="$(tags "$R")"
    if [ "$rc" = "3" ] && [ -z "$created" ]; then ok "D 拒绝 '$n'"
    else bad "D '$n' ⇒ rc=$rc tags=[$created]（应 rc=3 且无 tag）"; fi
done
# 空 --name 是**参数错**（不是命名不合规）：必须响亮 rc=2，且**绝不静默退回构造名**
R="$(newrepo dempty)"; run "$R" --name ""
chk "D --name '' ⇒ rc=2（参数错 · 不静默退回构造名）" "$RC" "2"
chk "D --name '' 零副作用" "$(tags "$R")" ""

echo "── E 已存在且指向别处、未 --move ⇒ rc=4 且**不动**原 tag ──"
# ⚠ M272：方案 B 下 **tag 名与里程碑一一对应**（patch 段 = 里程碑号）⇒ 同一里程碑
#   第二次 `--milestone 248` 会算出**同一个名字** ⇒ rc=4。本段用 `--name` 把名字钉死，
#   测的是「同名 tag 已存在、未 --move」的行为；正面的「同名即拒绝」由 **R7** 覆盖。
R="$(newrepo e)"; run "$R" --name v0.2.248; OLD="$(git -C "$R" rev-parse 'refs/tags/v0.2.248^{commit}')"
git -C "$R" commit -q --allow-empty -m "M248：第二个提交"
run "$R" --name v0.2.248
chk "E rc=4" "$RC" "4"
chk "E 原 tag 未被移动" "$(git -C "$R" rev-parse 'refs/tags/v0.2.248^{commit}')" "$OLD"
case "$out" in *"--move"*) ok "E 信息给出合规出路（--move）";; *) bad "E 未给出路";; esac

echo "── F --move 重定向 ──"
run "$R" --move --name v0.2.248
chk "F rc=0" "$RC" "0"
chk "F tag 已指向新 HEAD" \
    "$(git -C "$R" rev-parse 'refs/tags/v0.2.248^{commit}')" "$(git -C "$R" rev-parse HEAD)"
chk "F 仍是 annotated（不是轻量 tag）" "$(git -C "$R" cat-file -t refs/tags/v0.2.248)" "tag"

echo "── G --dry-run 是**只读**入口（M244 缺陷 412 的纪律）──"
R="$(newrepo g)"; run "$R" --milestone 250 --dry-run
chk "G rc=0" "$RC" "0"
chk "G **没有**创建 tag" "$(tags "$R")" ""

echo "── H 续：创建出的名字必须被事后守卫的正则接受 ──"
R="$(newrepo h)"; run "$R" --milestone 251
T="$(tags "$R" | tr -d ' ')"
chk "I 事后守卫的规则接受它" "$(printf '%s' "$T" | grep -qE "$RE_TG" && echo ACCEPT || echo REJECT)" "ACCEPT"

echo "── J/K 入口健壮性 ──"
R="$(newrepo j)"; run "$R" --help
chk "J --help rc=0" "$RC" "0"
case "$out" in *"用法"*) ok "J --help 给出用法";; *) bad "J --help 无用法";; esac
run "$R" --不存在的选项
chk "K 未知选项 rc=2" "$RC" "2"
run "$R" --at 不存在的提交
chk "K 无效 --at rc=2" "$RC" "2"
run "$R" --milestone 12x3
chk "K 非数字里程碑 rc=2" "$RC" "2"

echo "── M --at 指定提交 ──"
R="$(newrepo m)"; FIRST="$(git -C "$R" rev-parse HEAD)"
git -C "$R" commit -q --allow-empty -m "M248：新提交"
run "$R" --milestone 252 --at "$FIRST"
chk "M rc=0" "$RC" "0"
chk "M tag 指向指定提交（不是 HEAD）" \
    "$(git -C "$R" rev-parse 'refs/tags/v0.2.252^{commit}')" "$FIRST"

echo "── N 顺序判据：校验必须在创建**之前** ──"
LN_CHK="$(grep -n '拒绝创建不合规 tag' "$MT" | head -1 | cut -d: -f1)"
LN_MK="$(grep -n 'git tag -a "\$TAG"' "$MT" | head -1 | cut -d: -f1)"
if [ -n "$LN_CHK" ] && [ -n "$LN_MK" ] && [ "$LN_CHK" -lt "$LN_MK" ]; then
    ok "N 合规校验（行 $LN_CHK）先于创建（行 $LN_MK）"
else bad "N 校验/创建次序不对: chk=$LN_CHK mk=$LN_MK"; fi

echo "── O --push 无 remote ⇒ 响亮失败（不静默）──"
R="$(newrepo o)"; run "$R" --milestone 253 --push
chk "O rc=5（推送失败）" "$RC" "5"

echo "── P ★负控：拆掉合规校验 ⇒ 必须真的建出不合规 tag（证明判据有牙）──"
STUB1="$TMP/mt_stub1.sh"; STUB2="$TMP/mt_stub2.sh"
# P1 只摘掉 §4（创建前的合规闸门）—— 预期 §8 的「自证」会接住（纵深防御）
sed 's/^    exit 3$/    : # NEGCTL-P1/' "$MT" > "$STUB1"
# P2 连 §8 的自证一起摘掉 —— 预期**完全放行**（这才是「没有判据」的世界）
sed -e 's/^    exit 3$/    : # NEGCTL-P1/' \
    -e 's/^    EXIT_CODE=3 die "自证失败.*$/    : # NEGCTL-P2/' "$MT" > "$STUB2"
chk "P1 打桩点命中" "$(grep -c 'NEGCTL-P1' "$STUB1")" "1"
chk "P2 两个打桩点都命中" "$(grep -c 'NEGCTL-P[12]' "$STUB2")" "2"
bash -n "$STUB1" || bad "P1 打桩后语法错误"
bash -n "$STUB2" || bad "P2 打桩后语法错误"

R="$(newrepo p1)"; out="$(PX_TAG_REPO="$R" bash "$STUB1" --name v0.2.0-m245s1 2>&1)"; RC=$?
chk "P1 摘掉创建前闸门后，§8 自证仍把 rc 判成 3（纵深防御真的在）" "$RC" "3"

R="$(newrepo p2)"; out="$(PX_TAG_REPO="$R" bash "$STUB2" --name v0.2.0-m245s1 2>&1)"; RC=$?
chk "P2 两道都摘掉 ⇒ rc=0（= 没有判据的世界）" "$RC" "0"
chk "P2 不合规 tag 真的被建出来了（= 原判据确实拦住了它）" "$(tags "$R")" "v0.2.0-m245s1 "

echo "── Q ★第 ② 道防线（build_rpm.sh / make_release.sh 的「响亮拒绝」）──"
MS_RE='^m[0-9]+$'
RPM_SH="$SELF_DIR/build_rpm.sh"; MREL="$SELF_DIR/../tools/make_release.sh"
for f in "$RPM_SH" "$MREL"; do
    [ -f "$f" ] && bash -n "$f" && ok "Q 语法 OK: $(basename "$f")" || bad "Q 缺/语法错: $f"
done

# Q1 规则一致性 + 反向自证（`^m[0-9]+$` 必须真的锚定；首版用 glob `m[0-9]*` 就栽在这）
for f in "$RPM_SH" "$MREL"; do
    chk "Q1 $(basename "$f") 含里程碑判据 $MS_RE" \
        "$(grep -cF "$MS_RE" "$f")" "1"
done
chk "Q1 判据接受 m248"      "$(printf 'm248'   | grep -qE "$MS_RE" && echo ACCEPT || echo REJECT)" "ACCEPT"
chk "Q1 判据拒绝 m245s1"    "$(printf 'm245s1' | grep -qE "$MS_RE" && echo ACCEPT || echo REJECT)" "REJECT"
chk "Q1 判据拒绝 m245s"     "$(printf 'm245s'  | grep -qE "$MS_RE" && echo ACCEPT || echo REJECT)" "REJECT"
chk "Q1 判据拒绝 m245."     "$(printf 'm245.'  | grep -qE "$MS_RE" && echo ACCEPT || echo REJECT)" "REJECT"
chk "Q1 判据拒绝 M245（大写）" "$(printf 'M245'  | grep -qE "$MS_RE" && echo ACCEPT || echo REJECT)" "REJECT"

# Q2 顺序：判据必须在**关键动作**之前（否则等于没有）
#   ⚠ 锚点要落在**真的动作行**上，不能落在文件头注释里（本轮首版就匹配到注释，报假红）。
L_MS="$(grep -nF "$MS_RE" "$MREL" | head -1 | cut -d: -f1)"
L_BL="$(grep -n '^echo "== 发布包构建' "$MREL" | head -1 | cut -d: -f1)"
if [ -n "$L_MS" ] && [ -n "$L_BL" ] && [ "$L_MS" -lt "$L_BL" ]; then
    ok "Q2 make_release.sh 判据（行 $L_MS）先于打包（行 $L_BL）"
else bad "Q2 make_release.sh 次序不对: ms=$L_MS build=$L_BL"; fi
L_MS2="$(grep -nF "$MS_RE" "$RPM_SH" | head -1 | cut -d: -f1)"
L_RB="$(grep -n '^rpmbuild -bb' "$RPM_SH" | head -1 | cut -d: -f1)"
if [ -n "$L_MS2" ] && [ -n "$L_RB" ] && [ "$L_MS2" -lt "$L_RB" ]; then
    ok "Q2 build_rpm.sh 判据（行 $L_MS2）先于 rpmbuild（行 $L_RB）"
else bad "Q2 build_rpm.sh 次序不对: ms=$L_MS2 rpmbuild=$L_RB"; fi

# Q3 动态 · 真仓库 · **只跑拒绝路径**（安全：闸门生效时不会打包）
out="$(timeout 30 bash "$MREL" m245s1 2>&1)"; RC=$?
chk "Q3 make_release.sh m245s1 ⇒ rc=1" "$RC" "1"
case "$out" in *"里程碑不合规"*) ok "Q3 报出真因";; *) bad "Q3 未报真因: $out";; esac
case "$out" in *"发布包构建"*) bad "Q3 **竟然开始打包**（闸门没拦住）";; *) ok "Q3 在任何打包动作之前就被拦下";; esac

# Q4 动态 · build_rpm.sh · 夹具里放一个坏 tag（**绝不在真仓库造坏 tag**）
R="$(newrepo q4)"; mkdir -p "$R/packaging"; cp "$RPM_SH" "$R/packaging/"
git -C "$R" tag -a v0.2.0-m245s1 -m "夹具：坏 tag"
out="$(cd "$R" && SKIP_SIGN=1 timeout 60 bash packaging/build_rpm.sh 2>&1)"; RC=$?
chk "Q4 build_rpm.sh（坏 tag）⇒ rc=1" "$RC" "1"
case "$out" in *"tag 命名不合规"*) ok "Q4 报出真因";; *) bad "Q4 未报真因: $out";; esac
case "$out" in *"rpmbuild"*) bad "Q4 **竟然走到 rpmbuild**";; *) ok "Q4 未走到 rpmbuild";; esac
chk "Q4 夹具里零 rpm/仓库产物" \
    "$(find "$R" -name '*.rpm' -o -name 'repodata' 2>/dev/null | wc -l)" "0"

# Q5 ★负控：拆掉两处判据 ⇒ Q3/Q4 的「拦下」必须消失（证明判据有牙）
MR_STUB="$TMP/mrel_stub.sh"; RPM_STUB="$TMP/brpm_stub.sh"
sed 's|^if \[ "\$MILESTONE" != "dev" \] && ! printf .*|if false; then : # NEGCTL-Q5|' "$MREL" > "$MR_STUB"
sed 's|^    if ! printf .*|    if false; then : # NEGCTL-Q5|' "$RPM_SH" > "$RPM_STUB"
chk "Q5 make_release 打桩点命中" "$(grep -c 'NEGCTL-Q5' "$MR_STUB")" "1"
chk "Q5 build_rpm 打桩点命中"    "$(grep -c 'NEGCTL-Q5' "$RPM_STUB")" "1"
bash -n "$MR_STUB" || bad "Q5 make_release 打桩后语法错"
bash -n "$RPM_STUB" || bad "Q5 build_rpm 打桩后语法错"
# Q5a ★负控（make_release.sh）：拆掉判据 ⇒ **不再出现拦下的措辞**
#   ⚠ 这里**不**跑到打包完成：stub 的 PXC_HOME 会落到不存在的位置，且真跑会产出 72MB tarball。
#     判据只问「拦下的那句话还在不在」——与 Q3 的「在」构成一对。
out="$(timeout 12 bash "$MR_STUB" m245s1 2>&1)"; :
case "$out" in
    *"里程碑不合规"*) bad "Q5a 打桩后仍在拦（打桩无效）";;
    *) ok "Q5a 打桩后不再拦（= 原判据确实是拦它的那条）";;
esac
rm -f /tmp/puxian-0.2.0-m245s1-*.tar.gz /tmp/sha256sums.txt
rm -rf /tmp/puxian-0.2.0-m245s1-*.check /tmp/puxian-0.2.0-m245s1-*.stage
R2="$(newrepo q5b)"; mkdir -p "$R2/packaging"; cp "$RPM_STUB" "$R2/packaging/build_rpm.sh"
git -C "$R2" tag -a v0.2.0-m245s1 -m "夹具：坏 tag"
out="$(cd "$R2" && SKIP_SIGN=1 timeout 60 bash packaging/build_rpm.sh 2>&1)"; :
case "$out" in *"tag 命名不合规"*) bad "Q5 build_rpm 打桩后仍在报错（打桩无效）";; *) ok "Q5 build_rpm 打桩后不再拦（= 原判据确实在拦）";; esac

echo "── R：版本段 = <次段>.<里程碑>（M272 · 用户令 2026-10-05 方案 B）──"
#   方案 B 语义：**patch 段就是里程碑号** ⇒ ① 版本号自带进展；② 同一里程碑的 tag 名**唯一**
#   （不再「每轮递增 patch」，也不再有「patch 到 100 进位 minor」那条——它会让 0.2.x 一百轮耗尽）。
#   次段只在**大改进**时手工升（`--minor N`）。
#   判据形态：`--dry-run` 只读入口（无副作用 · M244 缺陷 412 的纪律）⇒ 把 tag 名读回来比。
bump_probe() {  # bump_probe <仓库> <里程碑> <期望tag> [额外参数...]
    local d="$1"
    local ms="$2"
    local exp="$3"
    shift 3
    out="$(PX_TAG_REPO="$d" bash "$MT" --milestone "$ms" --dry-run "$@" 2>&1)"; RC=$?
    local got
    got="$(printf '%s\n' "$out" | sed -n 's/^  tag 名   : //p' | head -1)"
    chk "R m$ms ⇒ $exp" "$got" "$exp"
}

# R0 无 tag ⇒ 次段取默认 0.2
R0="$(newrepo r0)"; bump_probe "$R0" 249 "v0.2.249"

# R1 旧形态基线 v0.2.0-m246 ⇒ 次段继承 0.2，patch 段 = 里程碑号
R1="$(newrepo r1)"; git -C "$R1" tag -a v0.2.0-m246 -m x
bump_probe "$R1" 247 "v0.2.247"

# R2 旧形态 v0.2.98-m344 ⇒ 次段仍继承 0.2
RB2="$(newrepo r2)"; git -C "$RB2" tag -a v0.2.98-m344 -m x
bump_probe "$RB2" 345 "v0.2.345"

# R3 **不再「到 100 进位」**：v0.2.99-m345 ⇒ v0.2.346（旧规则会算成 0.3.0-m346）
R3="$(newrepo r3)"; git -C "$R3" tag -a v0.2.99-m345 -m x
bump_probe "$R3" 346 "v0.2.346"

# R3b 新形态基线 v0.2.271 ⇒ v0.2.272
R3B="$(newrepo r3b)"; git -C "$R3B" tag -a v0.2.271 -m x
bump_probe "$R3B" 272 "v0.2.272"

# R3c `--minor 3`（大改进升次段）⇒ v0.3.272（patch 段**仍是**里程碑号）
bump_probe "$R3B" 272 "v0.3.272" --minor 3

# R4 多 tag 乱序 ⇒ 次段取**版本序最高**那个（不是「最近可达」）
R4="$(newrepo r4)"; git -C "$R4" tag -a v0.2.0-m246 -m x; git -C "$R4" tag -a v0.2.1-m249 -m x
bump_probe "$R4" 250 "v0.2.250"

# R5 `--move` 不改变命名（重定向同一 tag 换提交 ≠ 新版本）
R5="$(newrepo r5)"; git -C "$R5" tag -a v0.2.248 -m x
git -C "$R5" commit -q --allow-empty -m "second"
bump_probe "$R5" 248 "v0.2.248" --move

# R7 **同名即拒绝**（方案 B 的直接后果）：同一里程碑连打两次 ⇒ 第二次 rc=4（提示 --move）
#   旧规则下两次会得到两个名字（递增）⇒ 这条判据正是「tag 名字与里程碑一一对应」的守护。
R7="$(newrepo r7)"
run "$R7" --milestone 800
chk "R7 第一次 rc=0" "$RC" "0"
# 换一个提交再打同一个里程碑 ⇒ 名字相同、指向不同 ⇒ 必须 rc=4（提示 --move）
git -C "$R7" commit -q --allow-empty -m "M800：第二个提交"
run "$R7" --milestone 800
chk "R7 第二次 rc=4（同名已存在 ⇒ 需 --move）" "$RC" "4"
chk "R7 只留一个 tag（不产生第二个名字）" "$(tags "$R7")" "v0.2.800 "

# R6 ★负控：**拆掉「patch 段 = 里程碑号」** ⇒ R1 的结论必须不再成立
#   （锚点 = TAG 构造那一行；它变了说明构造方式改了，判据要跟着改）
MT_STUB="$TMP/make_tag_nopatch.sh"
cp "$MT" "$MT_STUB"
python3 - "$MT_STUB" <<'PYEOF'
import sys, pathlib
p = pathlib.Path(sys.argv[1]); s = p.read_text(encoding="utf-8")
OLD = '    TAG="v${VER_BASE}.${MILESTONE}"\n'
NEW = '    TAG="v${VER_BASE}.0-m${MILESTONE}"\n'
if s.count(OLD) != 1:
    sys.stderr.write("NEGCTL-R6-MISS\n"); sys.exit(1)
p.write_text(s.replace(OLD, NEW, 1), encoding="utf-8")
PYEOF
if [ $? != 0 ]; then
    bad "R6 ★负控：打桩失败（TAG 构造锚点没命中 —— make_tag.sh 的写法变了，判据要跟着改）"
else
    bash -n "$MT_STUB" || bad "R6 打桩后语法错"
    out="$(PX_TAG_REPO="$R1" bash "$MT_STUB" --milestone 247 --dry-run 2>&1)"
    got="$(printf '%s\n' "$out" | sed -n 's/^  tag 名   : //p' | head -1)"
    chkn "R6 ★负控：拆掉后**不再**给出 v0.2.247" "$got" "v0.2.247"
fi

# ── V 段：packaging 工具链的执行位一致性（M254 · 缺陷 450）──
#   来历：M249 建 make_tag.sh 时漏了执行位 ⇒ `packaging/make_tag.sh --milestone N` 报
#   Permission denied，而 bump_version.sh / pxrepo_mirror.sh 是 755、能直接跑 —— **同类工具不一致**
#   （文档里也是以 `packaging/xxx.sh` 不带 bash 的形式写的）。
#   范围刻意限定 packaging/：examples/ 下的门语料惯例是 `bash <file>` 调用，**不要求**执行位。
# ⚠️ **两处都查**（缺陷 451 教训）：只查 git index 会在「index 与工作树不同步」时**假绿** ——
#   实测：`git update-index --chmod=+x` 之后任何 `git add` 会**按文件系统 mode 覆盖**它
#   ⇒ index 又回到 644；而只查 index 的那版判据在**本地是绿的**（CI 才红）。
_n="$(git -C "$SELF_DIR/.." ls-files -s 'packaging/*.sh' 2>/dev/null | wc -l | tr -d ' ')"
_gitm="$(git -C "$SELF_DIR/.." ls-files -s 'packaging/*.sh' 2>/dev/null | awk '$1!="100755"{print $4}' | tr '\n' ' ')"
_fsm="$(find "$SELF_DIR" -maxdepth 1 -name '*.sh' ! -perm -u+x -printf '%f ' 2>/dev/null)"
if [ "${_n:-0}" -lt 10 ]; then
  bad "V1 判据自伤：git ls-files 只取到 $_n 个 packaging/*.sh（应有 18 个）—— 判据静默失效"
elif [ -n "$_gitm" ] || [ -n "$_fsm" ]; then
  bad "V1 有 packaging 脚本缺执行位：git[${_gitm:-无}] fs[${_fsm:-无}]"
else
  ok "V1 packaging/*.sh 全部 100755（git+fs 双查 · $_n 个）"
fi

echo
echo "══ selftest_make_tag: 通过 $pass / 失败 $fail ══"
[ "$fail" = 0 ] || exit "$fail"
exit 0
