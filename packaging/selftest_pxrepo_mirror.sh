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

echo "== 结果：通过 $PASS / 失败 $FAIL =="
[ "$FAIL" = 0 ] || exit 1
