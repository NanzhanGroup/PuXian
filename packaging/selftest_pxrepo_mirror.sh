#!/usr/bin/env bash
# ============================================================
# packaging/selftest_pxrepo_mirror.sh —— pxrepo_mirror.sh 判据离线回归
# ------------------------------------------------------------
# 只测**离线判据**（不联网、不下载、不写站点）：按标记从 pxrepo_mirror.sh
# **逐字抽取**真实代码段执行 ⇒ 无副本漂移。当前覆盖四段：
#   [1] xcheck-rpm-tree   版本交叉校验（gh-pages 树 ⇄ tag）
#   [2] rootfiles-fresh   站点根文件指纹（版本不变也要比内容）
#   [3] tag-name-guard    tag 命名护栏（不合规 tag 必须响亮）
#   [4] rpm-tree-layout   ★镜像树布局（rpm/<dist>/x86_64/…）—— M254 补 §⑤ 的判据缺口
#   [5] registry-mirror   W3：registry/ 随发布物落到镜像（M275）
#   [6] 整脚本不变量      ★**段间**：序（赋值先于引用）+ 禁形（大产出 | grep -q）+ 同源（缺陷 488）—— M279 补「段内自洽、段间不接」这条缝
# 回归对象（晨曦 2026-09-16 干跑发现）：原判据用 gh-pages **tip 提交信息**判定，
#   tip 为「站点文件同步」提交时必然误判 ⇒ 改为以 rpm 树内 `.mNNN` 为真值。
# 回归对象（晨曦 2026-10-02 干跑发现 · 10-04 第 2 次催办）：§⑤ 丢了 arch 层 ⇒ 真实站点必 die，
#   而 §⑤ 原本**没有任何判据覆盖** ⇒ [4] 段即为此而设。
# 用法：bash packaging/selftest_pxrepo_mirror.sh   （退出码 0 = 全过）
# ============================================================
set -uo pipefail

SCRIPT="$(cd "$(dirname "$0")" && pwd)/pxrepo_mirror.sh"
[ -f "$SCRIPT" ] || { echo "❌ 找不到 $SCRIPT"; exit 2; }

W="$(mktemp -d /tmp/pxselftest.XXXXXX)"
trap 'rm -rf "$W"' EXIT

# ---- 抽取真实判据段 + 桩函数 ----
BLOCK="$W/judge.sh"
cat > "$BLOCK" <<'HDR'
log() { printf '   [log] %s\n' "$*"; }
die() { printf '   [die] %s\n' "$*"; exit 1; }
HDR
sed -n '/^# >>> xcheck-rpm-tree >>>/,/^# <<< xcheck-rpm-tree <<<$/p' "$SCRIPT" >> "$BLOCK"
grep -q 'RPM_MS=' "$BLOCK" || { echo "❌ 抽取失败：$SCRIPT 里的标记行缺失或被改"; exit 2; }

# 站点根文件指纹判据：单独抽一份（纯函数，无 log/die 依赖）
BLOCK2="$W/rootfiles.sh"
sed -n '/^# >>> rootfiles-fresh >>>/,/^# <<< rootfiles-fresh <<<$/p' "$SCRIPT" > "$BLOCK2"
grep -q 'rootfiles_stale()' "$BLOCK2" || { echo "❌ 抽取失败：$SCRIPT 里的 rootfiles-fresh 标记行缺失或被改"; exit 2; }

# M277：资产挑选块（主包 ⇄ aarch64 引导包）—— 单独抽一份（纯函数，无 log/die 依赖）
BLOCK6="$W/assets.sh"
sed -n '/^# >>> asset-select >>>/,/^# <<< asset-select <<<$/p' "$SCRIPT" > "$BLOCK6"
grep -q 'pick_release_assets()' "$BLOCK6" || { echo "❌ 抽取失败：$SCRIPT 里的 asset-select 标记行缺失或被改"; exit 2; }

PASS=0; FAIL=0
mk_repo() { # $1=名 $2..=rpm文件名列表
  local d="$W/$1" f; shift
  mkdir -p "$d/rpm/9/x86_64"
  for f in "$@"; do : > "$d/rpm/9/x86_64/$f"; done
  git -C "$d" init -q
  git -C "$d" -c user.email=t@t -c user.name=t add -A
  git -C "$d" -c user.email=t@t -c user.name=t commit -qm "c"
  printf '%s\t\tbranch gh-pages of github\n' "$(git -C "$d" rev-parse HEAD)" > "$d/.git/FETCH_HEAD"
}
tip_msg() { # $1=名 $2=提交信息（模拟「站点同步提交为 tip」）
  local d="$W/$1"
  git -C "$d" -c user.email=t@t -c user.name=t commit -q --allow-empty -m "$2"
  printf '%s\t\tbranch gh-pages of github\n' "$(git -C "$d" rev-parse HEAD)" > "$d/.git/FETCH_HEAD"
}
check() { # $1=用例名 $2=期望rc $3=期望输出关键词 $4=clone $5=TAG
  local out rc
  out="$(CLONE="$4" TAG="$5" bash "$BLOCK" 2>&1)"; rc=$?
  if [ "$rc" = "$2" ] && printf '%s' "$out" | grep -q "$3"; then
    echo "✅ $1（rc=$rc，含「$3」）"; PASS=$((PASS+1))
  else
    echo "❌ $1（rc=$rc 期望 $2；输出: $(printf '%s' "$out" | tr '\n' ' ')）"; FAIL=$((FAIL+1))
  fi
}

echo "== 判据回归（抽取自 $SCRIPT）=="
mk_repo a "puxian-0.2.0-1.m122.el9.x86_64.rpm"
tip_msg a "site: 站点根文件改由仓库 packaging/ 同步"
check "A 站点同步提交为 tip（原判据误判处）" 0 "交叉校验：rpm 树含 m122" "$W/a" "v0.2.0-m122"

mk_repo b "puxian-0.2.0-1.m122.el9.x86_64.rpm"
tip_msg b "rpm: 发布 PuXian v0.2.0-m122（el7/el9 签名仓库）"
check "B rpm 发布提交为 tip" 0 "交叉校验：rpm 树含 m122" "$W/b" "v0.2.0-m122"

mk_repo c "puxian-0.2.0-1.m121.el9.x86_64.rpm"
tip_msg c "rpm: 发布 PuXian v0.2.0-m121（el7/el9 签名仓库）"
check "C 真不同步（树 m121 / tag m122）必须拦住" 1 "不含 m122" "$W/c" "v0.2.0-m122"

mk_repo d "puxian-0.2.0-1.m122.el9.x86_64.rpm" "puxian-0.2.0-1.m121.el9.x86_64.rpm"
tip_msg d "rpm: 发布 PuXian v0.2.0-m122"
check "D 树内多版本共存 → 放行 + 告警" 0 "多个里程碑版本" "$W/d" "v0.2.0-m122"

check "E tag 无 -mNNN 后缀 → 跳过" 0 "跳过 rpm 树版本交叉校验" "$W/a" "v0.2.0"

echo "== [6] 新形态 tag（M272 起）⇒ 交叉校验必须**真跑**（晨曦 2026-10-05 提出）=="
# 背景：M272 把 tag 换成 v<次段>.<里程碑>（**无 -mNNN 后缀**）。修前 `grep -oE 'm[0-9]+$'`
#   抓不到 ⇒ TAG_MS 为空 ⇒ 静默落到 else「跳过 rpm 树版本交叉校验」⇒ **覆盖率无声下降**
#   （M235 缺陷 355 同族）。晨曦在他站上实测到该行（2026-10-05 17:51）：
#       ⚠ tag 无 -mNNN 后缀（v0.2.272），跳过 rpm 树版本交叉校验
#   ⇒ 修复后：新形态 tag **必须**派生里程碑号并**真校验**。
#   用例逐字取自晨曦提交的 newform-xcheck-test.sh（2729 B · file_id f6aeba3b89b5211061f19ad7a）。
#   ⚠️ 本段存在的意义：产品代码已修（v0.2.272），但**判据覆盖为 0** ⇒ 没有它，
#      一次重构就能把「静默跳过」悄悄带回来，而 17 条既有断言**全部照绿**。
mk_repo n1 "puxian-0.2.272-1.m272.el9.x86_64.rpm"
check "N1 新形态 v0.2.272 ⇄ 树 .m272. → **真校验通过**（修前此处输出「跳过」）" 0 "交叉校验：rpm 树含 m272" "$W/n1" "v0.2.272"

mk_repo n2 "puxian-0.2.271-1.m271.el9.x86_64.rpm"
check "N2 负控：树只有 .m271. 而 tag v0.2.272 → 必须拦住（证明 N1 不是恒过）" 1 "不含 m272" "$W/n2" "v0.2.272"

mk_repo n3 "puxian-0.2.272-1.m272.el9.x86_64.rpm" "puxian-0.2.271-1.m271.el9.x86_64.rpm"
check "N3 树内多版本共存 → 放行 + 告警" 0 "多个里程碑版本" "$W/n3" "v0.2.272"

mk_repo n4 "puxian-0.3.1-1.m1.el9.x86_64.rpm"
check "N4 跨次段 v0.3.1（树 .m1.）→ 按 patch 段派生 m1 ⇒ 校验通过（次段升级不误判）" 0 "交叉校验：rpm 树含 m1" "$W/n4" "v0.3.1"

mk_repo n5 "puxian-0.2.272-1.m272s1.el9.x86_64.rpm"
check "N5 负控：新形态 tag 但树里违规标记 .m272s1. → 拦住且**指名**不合规" 1 "不合规" "$W/n5" "v0.2.272"

mk_repo n6 "puxian-0.2.0-1.m122.el9.x86_64.rpm"
check "N6 回归：旧形态 v0.2.0-m122 ⇄ 树 .m122. → 仍通过" 0 "交叉校验：rpm 树含 m122" "$W/n6" "v0.2.0-m122"
check "N6b 回归负控：旧形态 tag 配新形态树 → 仍拦住" 1 "不含 m122" "$W/n1" "v0.2.0-m122"
check "N7 tag v0.2.0（patch=0）→ 仍「跳过校验」，**不得**被派生成 m0" 0 "跳过 rpm 树版本交叉校验" "$W/n1" "v0.2.0"


echo "== 站点根文件指纹判据（抽取自 $SCRIPT）=="
# 背景：index.html / install-rpm.sh 与版本号无关，只比 version.json 会把「只改落地页」
# 的更新永久挡住（2026-09-16 实测）。此判据决定「版本相同时是否还要继续同步」。
RF="$W/rf"
mkdir -p "$RF/dest" "$RF/clone"
git -C "$RF/clone" init -q
printf 'NEW-HTML\n'    > "$RF/clone/index.html"
printf 'NEW-INSTALL\n' > "$RF/clone/install-rpm.sh"
git -C "$RF/clone" -c user.email=t@t -c user.name=t add -A
git -C "$RF/clone" -c user.email=t@t -c user.name=t commit -qm c
rf() { bash -c 'source "$1"; shift; rootfiles_stale "$@"' _ "$BLOCK2" "$RF/clone" HEAD "$1" index.html install-rpm.sh; }

o="$(rf "$RF/dest")"
if [ "$o" = " index.html install-rpm.sh" ]; then
  echo "✅ ① DEST 缺两份 → 两份都列出"; PASS=$((PASS+1))
else
  echo "❌ ① 期望「 index.html install-rpm.sh」，实得「$o」"; FAIL=$((FAIL+1))
fi

cp "$RF/clone/index.html" "$RF/dest/index.html"
cp "$RF/clone/install-rpm.sh" "$RF/dest/install-rpm.sh"
o="$(rf "$RF/dest")"
if [ -z "$o" ]; then
  echo "✅ ② 与 gh-pages 完全一致 → 空（幂等短路仍生效）"; PASS=$((PASS+1))
else
  echo "❌ ② 期望空，实得「$o」"; FAIL=$((FAIL+1))
fi

printf 'OLD-HTML\n' > "$RF/dest/index.html"
o="$(rf "$RF/dest")"
if [ "$o" = " index.html" ]; then
  echo "✅ ③ 只改落地页（版本不变）→ 只列出 index.html ★本次真实场景"; PASS=$((PASS+1))
else
  echo "❌ ③ 期望「 index.html」，实得「$o」"; FAIL=$((FAIL+1))
fi

echo "== tag 命名护栏（抽取自 $SCRIPT 的 tag-name-guard 段）=="
# 背景：2026-10-01 我方误打 v0.2.0-m237s3 ⇒ ① 版本序正则把它**静默过滤**（镜像停在旧版、
#   无任何告警）② build_rpm.sh 的 MILESTONE 取短横线之后整段 ⇒ rpm 包名被污染成
#   0.2.0-1.m237s3。此判据保证「不合规 tag 必须响亮」。
BLOCK3="$W/tagname.sh"
cat > "$BLOCK3" <<'HDR'
ALLTAGS="${ALLTAGS:-}"
log() { printf '   [log] %s\n' "$*"; }
die() { printf '   [die] %s\n' "$*"; exit 1; }
HDR
sed -n '/^# >>> tag-name-guard >>>/,/^# <<< tag-name-guard <<<$/p' "$SCRIPT" >> "$BLOCK3"
grep -q 'BADTAGS=' "$BLOCK3" || { echo "❌ 抽取失败：$SCRIPT 里的 tag-name-guard 标记行缺失或被改"; exit 2; }

chk_tag() { # $1=用例名 $2=ALLTAGS $3=期望rc $4=必须含(可空) $5=必须不含(可空) $6=STRICT
  local out rc ok=1
  out="$(ALLTAGS="$2" PXREPO_STRICT_TAGS="${6:-0}" bash "$BLOCK3" 2>&1)"; rc=$?
  [ "$rc" = "$3" ] || ok=0
  if [ -n "$4" ]; then printf '%s' "$out" | grep -q "$4" || ok=0; fi
  if [ -n "$5" ]; then printf '%s' "$out" | grep -q "$5" && ok=0; fi
  if [ "$ok" = 1 ]; then echo "✅ $1（rc=$rc）"; PASS=$((PASS+1))
  else echo "❌ $1（rc=$rc 期望 $3；输出: $(printf '%s' "$out" | tr '\n' ' ')）"; FAIL=$((FAIL+1)); fi
}

chk_tag "F1 全合规 tag → 无告警" 'v0.2.0-m236
v0.2.0-m237' 0 '' '不符合命名规则'
chk_tag "F2 含 v0.2.0-m237s3 → 响亮列出（默认不拦）" 'v0.2.0-m236
v0.2.0-m237
v0.2.0-m237s3' 0 'v0.2.0-m237s3' ''
chk_tag "F3 同上 + PXREPO_STRICT_TAGS=1 → 拒绝继续" 'v0.2.0-m236
v0.2.0-m237s3' 1 '拒绝继续' '' 1
chk_tag "F4 全合规 + STRICT=1 → 仍放行" 'v0.2.0-m236' 0 '' '拒绝继续'

mk_repo e "puxian-0.2.0-1.m237s3.el9.x86_64.rpm"
tip_msg e "rpm: 发布 PuXian v0.2.0-m237"
check "G rpm 树含违规标记(.m237s3.) → 拦住且**指名**不合规" 1 "不合规" "$W/e" "v0.2.0-m237"

echo "== [4] rpm 树布局判据（抽取自 $SCRIPT 的 rpm-tree-layout 段）=="
# 背景：M168 把 §⑤ 的 arch 层丢了（find "$STAGING/rpm/$d"、…/repodata/repomd.xml.asc 都缺 /x86_64）
#   ⇒ 任何按本脚本部署的镜像器都在 §⑤ 直接 die，而且是在**已经拉完数百 MB 资产之后**。
#   晨曦 2026-10-02 干跑发现（上游原件 sha256 76a3579…，我 2026-10-04 核对：与当时 HEAD 逐字节相同）、
#   2026-10-04 第 2 次催办。长期没被拦住的原因 = §⑤ **没有任何判据覆盖**（既有三段都不执行 §⑤）。
BLOCK4="$W/rpmtree.sh"
cat > "$BLOCK4" <<'HDR'
log() { printf '   [log] %s\n' "$*"; }
die() { printf '   [die] %s\n' "$*"; exit 1; }
HDR
sed -n '/^# >>> rpm-tree-layout >>>/,/^# <<< rpm-tree-layout <<<$/p' "$SCRIPT" >> "$BLOCK4"
grep -q 'rpm_tree_dir()' "$BLOCK4" || { echo "❌ 抽取失败：$SCRIPT 里的 rpm-tree-layout 标记行缺失或被改"; exit 2; }

# fixture = **真实镜像树布局**（dist 层 + arch 层 + repodata + 签名文件），4 个 dist 覆盖 x86 与 openEuler
RT="$W/rt"; RMBIN="$W/rmbin"; mkdir -p "$RMBIN"
rmk() { # $1=dist 名
  mkdir -p "$RT/rpm/$1/x86_64/repodata"
  : > "$RT/rpm/$1/x86_64/puxian-0.9.9-1.m999.x86_64.rpm"
  : > "$RT/rpm/$1/x86_64/repodata/repomd.xml.asc"
}
rmk 7; rmk 9; rmk openeuler/22.03; rmk openeuler/24.03
: > "$RT/rpm/PUXIAN-GPG-KEY.asc"
printf '#!/bin/sh\nexit 0\n' > "$RMBIN/rpm"; chmod +x "$RMBIN/rpm"   # 桩：只为让 --import/-Kv 通过

rt_run() { STAGING="$RT" PATH="$RMBIN:$PATH" bash "$BLOCK4" 2>&1; }

# ④ 正例：真实布局必须**全过**（保证判据不是恒绿）
o="$(rt_run)"; rtc=$?
ok=1
[ "$rtc" = 0 ] || ok=0
printf '%s' "$o" | grep -q '目录集合：7 9 openeuler/22.03 openeuler/24.03' || ok=0
[ "$(printf '%s' "$o" | grep -c '✅')" = 4 ] || ok=0
if [ "$ok" = 1 ]; then
  echo "✅ ④ 真实布局（4 dist × arch 层）→ 全过"; PASS=$((PASS+1))
else
  echo "❌ ④ rc=$rtc；输出: $(printf '%s' "$o" | tr '\n' ' ')"; FAIL=$((FAIL+1))
fi

# ④b 反向判据：摘掉一个 dist 的 repomd.xml.asc ⇒ **必须**响亮（否则「一律通过」也能让 ④ 变绿）
mv "$RT/rpm/openeuler/24.03/x86_64/repodata/repomd.xml.asc" "$W/kept.asc"
o="$(rt_run)"; rtc=$?
if [ "$rtc" != 0 ] && printf '%s' "$o" | grep -q 'openeuler/24.03/x86_64 缺少 repomd.xml.asc'; then
  echo "✅ ④b 缺 repomd.xml.asc → 响亮，且路径**含 arch 层**"; PASS=$((PASS+1))
else
  echo "❌ ④b rc=$rtc；输出: $(printf '%s' "$o" | tr '\n' ' ')"; FAIL=$((FAIL+1))
fi
mv "$W/kept.asc" "$RT/rpm/openeuler/24.03/x86_64/repodata/repomd.xml.asc"

# ④c 缺陷真实性（直接证据，无打桩）：**旧形态**（缺 arch）在真实布局上**必然**找不到 .rpm
oldf="$(find "$RT/rpm/9" -maxdepth 1 -name '*.rpm' | head -1)"
if [ -z "$oldf" ]; then
  echo "✅ ④c 旧形态（find rpm/9，缺 arch）在真实布局上必然为空 ⇒ M168 的 §⑤ 必 die"; PASS=$((PASS+1))
else
  echo "❌ ④c 旧形态竟找到了「$oldf」—— fixture 不是真实布局"; FAIL=$((FAIL+1))
fi

# ④d 静态：路径拼接**不许散落回调用点** + version.json 的 rpm_repo 必须含 arch 层
# ⚠️ 只查**代码行**：§⑤ 的注释里**故意保留**缺陷原形（`find "$STAGING/rpm/$d"`）当历史说明，
#    把注释算进来会**假红**（本仓 M221/M223 同款教训 —— 判据的输入 ≠ 人读的文本）。
_hits="$(grep -nE '\$\{?STAGING\}?/rpm/\$d(/|"|$)' "$SCRIPT" | grep -vE '^[0-9]+:[[:space:]]*#' | grep -vc '/x86_64' || true)"
_repo="$(grep -cF 'rpm/$d/x86_64/' "$SCRIPT" || true)"   # 不带引号：§6 里是转义后的 \" … \"
_dec="$(grep -c 'rpm_tree_dir "\$STAGING"' "$SCRIPT" || true)"
if [ "$_hits" = 0 ] && [ "$_repo" -ge 1 ] && [ "$_dec" -ge 1 ]; then
  echo "✅ ④d 静态：无「缺 arch」拼接 · find 经 rpm_tree_dir · rpm_repo 含 /x86_64/"; PASS=$((PASS+1))
else
  echo "❌ ④d 缺 arch 拼接 $_hits 处 · rpm_repo 命中 $_repo · 经访问器 $_dec"; FAIL=$((FAIL+1))
fi

# ---- [5] registry 镜像段（W3）----
#   本 selftest 不用 ok/bad 函数（全内联），这里补一对局部助手（与 PASS/FAIL 同作用域）
ok_r()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad_r() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
echo "== [5] registry 镜像段（W3：抽取自 $SCRIPT 的 registry-mirror 段）=="
# 判据：① 标记段可抽取（结构未破）② 桩环境下**真的把 registry/ 提取出来**
#       ③ tarball 不含 registry 时**不 die**（向后兼容，且 rsync 要带 --exclude 保护现役）
BLOCK5="$W/block5.sh"
sed -n '/^# >>> registry-mirror >>>/,/^# <<< registry-mirror <<<$/p' "$SCRIPT" > "$BLOCK5"
if grep -q 'REG_FOUND=' "$BLOCK5"; then
    ok_r "registry-mirror 段可抽取（结构未破）"
else
    bad_r "抽取失败：$SCRIPT 里的 registry-mirror 标记行缺失或被改"
fi

# ② 动态：造一个小 tarball（含 registry/README.md），桩掉 die/log，跑抽取段
W5="$W/reg"; mkdir -p "$W5/src/px-t/registry/sub" "$W5/staging/releases"
printf 'r\n' > "$W5/src/px-t/registry/README.md"
printf 'x\n' > "$W5/src/px-t/registry/sub/a.px"
tar -czf "$W5/staging/releases/fake.tar.gz" -C "$W5/src" px-t
cat > "$W5/run.sh" <<'RUNEOF'
set -uo pipefail
STAGING="$1/staging"; TARBALL=fake.tar.gz
WORK="$1/work"; mkdir -p "$WORK"   # M279：registry 段「清单落盘后判」需要 WORK
die(){ echo "DIE: $*" >&2; exit 9; }
log(){ echo "$*"; }
RUNEOF
sed -n '/^# >>> registry-mirror >>>/,/^# <<< registry-mirror <<<$/p' "$SCRIPT" >> "$W5/run.sh"
echo 'echo "REG_FOUND=$REG_FOUND REG_N=$REG_N"' >> "$W5/run.sh"
OUT5="$(bash "$W5/run.sh" "$W5" 2>&1)"; RC5=$?
case "$OUT5" in
    *"REG_FOUND=1 REG_N=2"*) ok_r "含 registry 的 tarball ⇒ 提取 2 件（REG_FOUND=1）" ;;
    *) bad_r "提取结果异常（rc=$RC5）：$(printf '%s' "$OUT5" | tail -2 | tr '\n' ' ')" ;;
esac

# ③ 反向：tarball 不含 registry ⇒ 必须**不 die**（老资产向后兼容）
W5b="$W/regb"; mkdir -p "$W5b/src/px-t/stdlib" "$W5b/staging/releases"
printf 'y\n' > "$W5b/src/px-t/stdlib/s.px"
tar -czf "$W5b/staging/releases/fake.tar.gz" -C "$W5b/src" px-t
sed "s#$W5#$W5b#" "$W5/run.sh" > "$W5b/run.sh"
OUT5b="$(bash "$W5b/run.sh" "$W5b" 2>&1)"; RC5b=$?
if [ "$RC5b" = 0 ]; then
    case "$OUT5b" in
        *"REG_FOUND=0"*) ok_r "不含 registry 的 tarball ⇒ 不 die 且 REG_FOUND=0（向后兼容）" ;;
        *) bad_r "未 die 但 REG_FOUND 非 0：$(printf '%s' "$OUT5b" | tail -1)" ;;
    esac
else
    bad_r "不含 registry 的 tarball ⇒ die 了（rc=$RC5b），老资产重放会整条红"
fi

# ④ 静态：rsync 必须有**条件**排除（无条件排除 ⇒ 正常的 registry 也同步不出去；无排除 ⇒ 会删现役）
case "$(cat "$SCRIPT")" in
    *'REG_EXCL+=(--exclude=/registry/)'*) ok_r "rsync 条件排除到位（保护 DEST 现役）" ;;
    *) bad_r "rsync 缺 --exclude=/registry/ 分支 ⇒ 老资产轮次会删掉 DEST 现役 registry" ;;
esac
case "$(cat "$SCRIPT")" in
    *'"registry": { "included"'*) ok_r "version.json 登记 registry 状态（下游可判定）" ;;
    *) bad_r "version.json 未登记 registry ⇒ 下游无法判定镜像是否含 registry" ;;
esac

# ══════════════════════════════════════════════════════════════
# M277 · 资产挑选（用户 2026-10-06 报障「aarch64 包不在镜像上」）
#   修前：`grep ... | head -1` 只取**第一个** .tar.gz ⇒ aarch64 包**从未进过镜像**。
#   本段用两个夹具证明「两类资产被分开挑出」，并**反向**断言 a64 ≠ main（旧行为必判红）。
# ══════════════════════════════════════════════════════════════
mk_json() {   # $1=输出文件；$2..=资产名
  local f="$1"; shift
  { printf '{\n  "tag_name": "v0.2.277",\n  "assets": [\n'
    local first=1 n
    for n in "$@"; do
      [ "$first" = 1 ] || printf ',\n'; first=0
      printf '    { "name": "%s", "size": 1 }' "$n"
    done
    printf '\n  ]\n}\n'
  } > "$f"
}
as_ok() {   # $1=用例名 $2=0/1（**缺省 1 = 通过** —— `set -u` 下漏传 $2 会直接炸，本仓第 N 次踩）
  if [ "${2:-1}" = 1 ]; then echo "✅ $1"; PASS=$((PASS+1)); else echo "❌ $1"; FAIL=$((FAIL+1)); fi
}
# 夹具 1：真实发布（主包 + aarch64 引导包 + sha256，且 aarch64 排在**后面**）
mk_json "$W/rel1.json" "puxian-0.2.277-b2c4c7c.tar.gz" "puxian-bootstrap-aarch64-v0.2.277.tar.gz.sha256" "puxian-bootstrap-aarch64-v0.2.277.tar.gz" "sha256sums.txt"
OUT1="$(bash -c ". \"$BLOCK6\"; pick_release_assets \"$W/rel1.json\"")"
M1="$(printf '%s\n' "$OUT1" | awk -F'\t' '$1=="main"{print $2}')"
A1="$(printf '%s\n' "$OUT1" | awk -F'\t' '$1=="a64"{print $2}')"
[ "$M1" = "puxian-0.2.277-b2c4c7c.tar.gz" ] && as_ok "主包挑中（未被 aarch64 顶掉）" 1 || as_ok "主包挑中（实得「$M1」）" 0
[ "$A1" = "puxian-bootstrap-aarch64-v0.2.277.tar.gz" ] && as_ok "aarch64 引导包挑中（**.sha256 不被误选**）" 1 || as_ok "aarch64 挑中（实得「$A1」）" 0
[ -n "$A1" ] && [ "$A1" != "$M1" ] && as_ok "A ≠ MAIN（旧行为 head -1 必判红）" 1 || as_ok "A ≠ MAIN（旧行为未修）" 0
# 夹具 2：老 tag（只有主包）⇒ a64 必须为空且**不 die**
mk_json "$W/rel2.json" "puxian-0.2.0-m122-abc1234.tar.gz" "sha256sums.txt"
OUT2="$(bash -c ". \"$BLOCK6\"; pick_release_assets \"$W/rel2.json\"")"
A2="$(printf '%s\n' "$OUT2" | awk -F'\t' '$1=="a64"{print $2}')"
[ -z "$A2" ] && as_ok "老 tag（无 aarch64 资产）⇒ a64 为空（不 die）" 1 || as_ok "老 tag ⇒ a64 空（实得「$A2」）" 0

# 静态：镜像脚本必须真的**下载 + 校验 + 登记** aarch64
case "$(cat "$SCRIPT")" in
  *'aarch64 引导包'*'sha256 = $A64_SUM'*) as_ok "镜像脚本：aarch64 下载后**校验** sha256" ;;
  *) as_ok "镜像脚本：缺 aarch64 sha256 校验" 0 ;;
esac
case "$(cat "$SCRIPT")" in
  *'"bootstrap_aarch64_tarball"'*) as_ok "version.json 登记 aarch64 坐标（下游可判定）" ;;
  *) as_ok "version.json 未登记 aarch64 坐标" 0 ;;
esac
case "$(cat "$SCRIPT")" in
  *'落地后 aarch64 引导包 sha256 不一致'*) as_ok "落地后**再验一次**（DEST 上真在且一致）" ;;
  *) as_ok "缺落地后 aarch64 自证" 0 ;;
esac


# ══════════════════════════════════════════════════════════════
# M279 · [6] 整脚本不变量（**段间**，不是段内）
#   背景（用户 2026-10-06 报障链条的第二层）：上面 [1]–[5] 全部是**按标记抽取段**离线跑
#   ⇒ 段内自洽，但**段间不接**看不见。两个真缺陷都躲过了 37/0：
#     缺陷 486 前向引用：§6.5 的 REG_FOUND 被 §6 的 heredoc 引用，赋值却在 §6 之后
#                       ⇒ set -euo pipefail 下「unbound variable」⇒ 同步 100% 失败。
#                       ⚠️ dry-run 在 §6 之后、§6.5 之前就 exit 0 ⇒「干跑通过」≠「同步能过」。
#     缺陷 487 `tar -tzf … | grep -q X` + pipefail ⇒ grep 命中即退出 ⇒ tar 收 SIGPIPE(141)
#                       ⇒ 整条管道非零 ⇒ 判据**恒假** ⇒ registry/ 永远不被提取（静默降级）。
#                       实测：grep -q 单独 rc=0；带 pipefail 整条管道 rc=141。
#   ⇒ 本段补「**整脚本层**」静态不变量，各配一条**反向判据**（证明判据有牙）。
# ══════════════════════════════════════════════════════════════
echo "== [6] 整脚本不变量（序 · 禁形 · 段间）=="
iv_ok()  { echo "  ✅ $1"; PASS=$((PASS+1)); }
iv_bad() { echo "  ❌ $1"; FAIL=$((FAIL+1)); }

iv_order() {   # $1=脚本 → 0 = 序正确（REG_* 赋值早于 version.json 的引用）
  local f="$1" a u
  a="$(grep -nE '^[[:space:]]*REG_(FOUND|N)=0' "$f" 2>/dev/null | head -1 | cut -d: -f1)"
  u="$(grep -nE '"included":[[:space:]]*\$REG_FOUND' "$f" 2>/dev/null | head -1 | cut -d: -f1)"
  [ -n "$a" ] && [ -n "$u" ] && [ "$a" -lt "$u" ]
}
iv_nopipeq() { # $1=脚本 → 0 = 无「大产出 | grep -q」禁形（**只查代码行**，注释里允许留历史说明）
  local f="$1" n
  n="$(grep -nE '(tar|find|cat|dnf)[^|]*\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q' "$f" 2>/dev/null \
       | grep -vE '^[0-9]+:[[:space:]]*#' | wc -l)"
  [ "${n:-0}" = 0 ]
}

_ra="$(grep -nE '^[[:space:]]*REG_(FOUND|N)=0' "$SCRIPT" 2>/dev/null | head -1 | cut -d: -f1)"
_ru="$(grep -nE '"included":[[:space:]]*\$REG_FOUND' "$SCRIPT" 2>/dev/null | head -1 | cut -d: -f1)"
if iv_order "$SCRIPT"; then
  iv_ok "⑥-1 序不变量：REG_* 赋值（行 ${_ra:-?}）早于 version.json 引用（行 ${_ru:-?}）"
else
  iv_bad "⑥-1 序不变量破：赋值「${_ra:-无}」/ 引用「${_ru:-无}」⇒ set -u 下必 die（缺陷 486）"
fi
# ⑥-2 反向判据：把 registry 段搬回文件末尾（= 修前形态）⇒ **必须判红**
python3 - "$SCRIPT" "$W/pre486.sh" <<'PYPRE'
import sys, pathlib
s = pathlib.Path(sys.argv[1]).read_text()
B, E = "# >>> registry-mirror >>>", "# <<< registry-mirror <<<"
i, j = s.find(B), s.find(E)
assert i > 0 and j > i, "registry-mirror 标记缺失"
blk = s[i:j + len(E)]
pathlib.Path(sys.argv[2]).write_text(s[:i] + s[j + len(E):] + "\n" + blk + "\n")
PYPRE
if iv_order "$W/pre486.sh"; then
  iv_bad "⑥-2 反向判据失效：还原「段在 §6 之后」竟仍判绿（判据无牙）"
else
  iv_ok "⑥-2 反向判据：还原修前形态 ⇒ 判红（有牙）"
fi

if iv_nopipeq "$SCRIPT"; then
  iv_ok "⑥-3 禁形：无「大产出 | grep -q」（SIGPIPE ⇒ 判据恒假）"
else
  iv_bad "⑥-3 仍存在「大产出 | grep -q」：$(grep -nE '(tar|find|cat|dnf)[^|]*\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q' "$SCRIPT" 2>/dev/null | grep -vE '^[0-9]+:[[:space:]]*#' | head -2 | tr '\n' ' ')"
fi
# ⑥-4 反向判据：注入旧形态 ⇒ **必须**被发现
{ printf 'if tar -tzf /dev/null 2>/dev/null | grep -q x; then\n  :\nfi\n'; } > "$W/pre487.sh"
cat "$SCRIPT" >> "$W/pre487.sh"
if iv_nopipeq "$W/pre487.sh"; then
  iv_bad "⑥-4 反向判据失效：注入 tar|grep -q 竟未被发现（判据无牙）"
else
  iv_ok "⑥-4 反向判据：注入 tar|grep -q ⇒ 判红（有牙）"
fi

# ⑥-5 同源（缺陷 488）：写侧与 ⑦ 复核侧必须引用**同一个**表达式，且定义含 releases/ 前缀
#   （下游 packaging/install-rpm.sh:166 按 `$MIRROR_ROOT/$a64` 取 ⇒ 前缀是契约）
iv_a64same() {  # $1=脚本 → 0 = 同源
  python3 - "$1" <<'PYA64'
import re, sys
s = open(sys.argv[1], encoding="utf-8").read()
w = re.search(r'^\s*"bootstrap_aarch64_tarball":\s*("[^"]*")\s*,\s*$', s, re.M)
v = re.search(r'grep -q "\\"bootstrap_aarch64_tarball\\":\s*\\"([^"\\]*)\\""', s)
if not w or not v:
    sys.exit(1)
if w.group(1) != '"%s"' % v.group(1):
    sys.exit(1)
sys.exit(0 if 'A64_REL="releases/$A64_TARBALL"' in s else 1)
PYA64
}
if iv_a64same "$SCRIPT"; then
  iv_ok "⑥-5 同源：写侧与 ⑦ 复核侧表达式逐字节相等，且定义含 releases/ 前缀（下游 install 口径）"
else
  iv_bad "⑥-5 不同源：写侧/复核侧表达式不一致，或缺 releases/ 前缀（缺陷 488）"
fi
# ⑥-6 反向判据：写侧退回裸名 ⇒ **必须判红**
python3 - "$SCRIPT" "$W/pre488.sh" <<'PYPRE488'
import re, sys, pathlib
s = pathlib.Path(sys.argv[1]).read_text(encoding="utf-8")
s2 = re.sub(r'^(\s*"bootstrap_aarch64_tarball":\s*)"[^"]*"', r'\1"$A64_TARBALL"', s, count=1, flags=re.M)
assert s2 != s, "写侧替换失败"
pathlib.Path(sys.argv[2]).write_text(s2, encoding="utf-8")
PYPRE488
if iv_a64same "$W/pre488.sh"; then
  iv_bad "⑥-6 反向判据失效：写侧退回裸名竟仍判绿（判据无牙）"
else
  iv_ok "⑥-6 反向判据：写侧退回裸名 ⇒ 判红（有牙）"
fi

echo "== 结果：通过 $PASS / 失败 $FAIL =="

[ "$FAIL" = 0 ] || exit 1
