#!/usr/bin/env bash
# ============================================================
# packaging/selftest_pxrepo_mirror.sh —— pxrepo_mirror.sh 判据离线回归
# ------------------------------------------------------------
# 只测「版本交叉校验」判据（不联网、不下载、不写站点）：
#   从 pxrepo_mirror.sh 的 `# >>> xcheck-rpm-tree >>>` ~ `# <<< xcheck-rpm-tree <<<`
#   之间**逐字抽取**真实代码段执行 ⇒ 无副本漂移。
# 回归对象（晨曦 2026-09-16 干跑发现）：原判据用 gh-pages **tip 提交信息**判定，
#   tip 为「站点文件同步」提交时必然误判 ⇒ 改为以 rpm 树内 `.mNNN` 为真值。
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

echo "== 结果：通过 $PASS / 失败 $FAIL =="
[ "$FAIL" = 0 ] || exit 1
