#!/usr/bin/env bash
# ============================================================
# packaging/pxrepo_mirror.sh —— PuXian 发布镜像器（GitHub → 自建静态站点）
# ------------------------------------------------------------
# 用途：把 PuXian 的「最新发布」镜像到一个静态站点子目录，供国内用户高速下载：
#   rpm/{7,9}/x86_64/   签名 RPM 仓库（含 repodata + repomd.xml.asc）
#   install-rpm.sh      安装脚本（与 gh-pages 同源）
#   index.html          落地页
#   releases/*.tar.gz   发布 tarball + sha256sums.txt
#   version.json        ★ 镜像自证（version/tag/synced_at/sha256），供监控与对账
#
# 权威版本序（不一致即中止，绝不猜、绝不降级）：
#   ① 远端 tag 里版本最高者（v<major>.<minor>.<patch>[-m<NNN>]）
#   ② gh-pages 的 rpm/ 树内必须含同一里程碑版本（.mNNN）—— 以**树内容**为真值，
#      不信提交信息（gh-pages 上前有「rpm 发布」提交、后有「站点文件同步」提交，
#      以 tip 提交信息判定会在站点提交为 tip 时必然误判 —— 2026-09-16 晨曦干跑实测）
#   ③ 单调守卫：不得低于目标目录现役 version.json 的版本（除非 PXREPO_ALLOW_REGRESSION=<理由>）
# 校验：tarball sha256 == sha256sums.txt；每个 rpm `rpm -Kv` 验签；repomd.xml.asc 必须存在
# 发布顺序（消除「元数据与包不匹配」窗口）：新 rpm（不删旧）→ repodata → 删旧 rpm
# 原子性：全部在 WORK 暂存并通过校验后才写入 DEST；任一步失败 ⇒ DEST 保持原样
# 站点根文件指纹：index.html / install-rpm.sh **不随版本号变化** ⇒ 版本相同也要比内容，
#   否则「只改落地页、版本不变」的更新会被下面的幂等短路永久挡住（2026-09-16 实测踩到）
#
# 用法：
#   pxrepo_mirror.sh --dest <站点子目录>
#   pxrepo_mirror.sh --dest /data/www/soft_wsai_chat/puxian --work /var/tmp/pxrepo-mirror
#   pxrepo_mirror.sh --dest ... --dry-run          # 只解析与校验，不写 DEST
# 退出码：0 = 成功（含「已是最新，无变化」）；1 = 失败（DEST 未改）；2 = 用法/环境错
# 环境变量：PXREPO_REPO（默认 NanzhanGroup/PuXian）· PXREPO_KEEP_STAGING=1 保留暂存
# ============================================================
set -euo pipefail

REPO_SLUG="${PXREPO_REPO:-NanzhanGroup/PuXian}"
REMOTE_URL="https://github.com/${REPO_SLUG}.git"
DL_BASE="https://github.com/${REPO_SLUG}/releases/download"

DEST=""; WORK=""; DRY=0
while [ $# -gt 0 ]; do
  case "$1" in
    --dest) DEST="${2:-}"; shift 2 ;;
    --work) WORK="${2:-}"; shift 2 ;;
    --dry-run) DRY=1; shift ;;
    -h|--help) sed -n '2,30p' "$0"; exit 0 ;;
    *) echo "❌ 未知参数: $1" >&2; exit 2 ;;
  esac
done
[ -n "$DEST" ] || { echo "❌ 缺少 --dest" >&2; exit 2; }
WORK="${WORK:-/var/tmp/pxrepo-mirror}"
# --dest 必须与 --work 分离（否则暂存目录会被软镜像一起带走 / 互相递归）
case "$DEST/" in "$WORK"/*) echo "❌ --dest 不能位于 --work（$WORK）内" >&2; exit 2 ;; esac

log() { printf '[%s] %s\n' "$(date '+%F %T')" "$*"; }
die() { log "❌ $*"; exit 1; }

for c in git curl sha256sum rsync rpm tar; do
  command -v "$c" >/dev/null 2>&1 || die "缺少命令: $c"
done

STAGING="$WORK/staging"
CLONE="$WORK/repo"
LOCK="$WORK/lock"
mkdir -p "$WORK"
exec 9>"$LOCK"
flock -n 9 || die "另一个 pxrepo_mirror 正在运行（$LOCK）"

cleanup() { [ "${PXREPO_KEEP_STAGING:-0}" = 1 ] || rm -rf "$STAGING"; }
trap cleanup EXIT

# ---------- 1. 权威版本：远端 tag 中最高者 ----------
log "① 解析远端 tag（$REPO_SLUG）"
TAGS="$(git ls-remote --tags "$REMOTE_URL" \
        | awk '{print $2}' | sed 's#^refs/tags/##; s/\^{}$//' \
        | grep -E '^v[0-9]+\.[0-9]+\.[0-9]+(-m[0-9]+)?$' | sort -u || true)"
[ -n "$TAGS" ] || die "远端没有任何符合 v<M>.<m>.<p>[-mNNN] 的 tag"

vkey() { printf '%s' "$1" | sed -nE 's/^v([0-9]+)\.([0-9]+)\.([0-9]+)(-m([0-9]+))?$/\1 \2 \3 \5/p'; }
BEST=""; BESTKEY=""
while IFS= read -r t; do
  [ -n "$t" ] || continue
  k="$(vkey "$t")"; [ -n "$k" ] || continue
  IFS=' ' read -r ma mi pa ms <<<"$k"
  key="$(printf '%04d.%04d.%04d.%06d' "$ma" "$mi" "$pa" "${ms:-0}")"
  if [ -z "$BESTKEY" ] || [ "$key" \> "$BESTKEY" ]; then BESTKEY="$key"; BEST="$t"; fi
done <<< "$TAGS"
[ -n "$BEST" ] || die "版本排序失败"
TAG="$BEST"
log "   权威版本 = $TAG"

# ---------- 1b. 公用：浅取 gh-pages / 站点根文件指纹 ----------
# fetch_pages 幂等：无变化时是秒级空转；跑完 FETCH_HEAD = gh-pages tip。
fetch_pages() {
  if [ -d "$CLONE/.git" ]; then
    git -C "$CLONE" fetch -q --depth 1 origin gh-pages
  else
    mkdir -p "$CLONE"; git -C "$CLONE" init -q
    git -C "$CLONE" remote add origin "$REMOTE_URL" 2>/dev/null || true
    git -C "$CLONE" fetch -q --depth 1 origin gh-pages
  fi
}

# >>> rootfiles-fresh >>>  （selftest_pxrepo_mirror.sh 按此标记抽取本段做离线回归，勿删改标记行）
# 站点根文件（index.html / install-rpm.sh）与版本号无关：只比 version.json 会把
# 「只改落地页」的更新永久挡住 ⇒ 直接比内容，打印「与 gh-pages 不一致的文件名」。
# 用法：rootfiles_stale <clone> <ref> <dest> <file>...   → 输出 " a b"（空 = 一致）
rootfiles_stale() {
  local cl="$1" ref="$2" dest="$3"; shift 3
  local f out=""
  for f in "$@"; do
    if [ ! -f "$dest/$f" ]; then out="$out $f"; continue; fi
    git -C "$cl" show "$ref:$f" 2>/dev/null | cmp -s - "$dest/$f" || out="$out $f"
  done
  printf '%s' "$out"
}
# <<< rootfiles-fresh <<<

# ---------- 2. 单调守卫（相对 DEST 现役） ----------
if [ -f "$DEST/version.json" ]; then
  CUR="$(grep -o '"version"[[:space:]]*:[[:space:]]*"[^"]*"' "$DEST/version.json" | head -1 | sed 's/.*"\([^"]*\)"$/\1/')"
  if [ -n "$CUR" ] && [ "$CUR" != "$TAG" ]; then
    ck="$(vkey "$CUR")"
    if [ -n "$ck" ]; then
      IFS=' ' read -r ma mi pa ms <<<"$ck"
      curkey="$(printf '%04d.%04d.%04d.%06d' "$ma" "$mi" "$pa" "${ms:-0}")"
      if [ "$BESTKEY" \< "$curkey" ]; then
        [ -n "${PXREPO_ALLOW_REGRESSION:-}" ] \
          || die "拒绝降级：现役 $CUR > 待发布 $TAG（确需覆盖请设 PXREPO_ALLOW_REGRESSION=<理由>）"
        log "   ⚠ 人工放行降级：$CUR → $TAG（理由：$PXREPO_ALLOW_REGRESSION）"
      fi
    fi
  fi
  if [ "${CUR:-}" = "$TAG" ]; then
    # ⚠ 幂等短路前先看**站点根文件**：index.html / install-rpm.sh 与版本号无关。
    #   2026-09-16 实测踩到：落地页链接修复推上 gh-pages 后，本轮同步因「版本相同」
    #   直接 exit 0 ⇒ 修复永远到不了站点。故按内容指纹判定。
    fetch_pages
    STALE_ROOT="$(rootfiles_stale "$CLONE" FETCH_HEAD "$DEST" index.html install-rpm.sh)"
    if [ -z "$STALE_ROOT" ]; then
      log "✅ DEST 已是 $TAG 且站点根文件一致 —— 无变化，退出"
      exit 0
    fi
    log "⚠ 版本未变（$TAG）但站点根文件有更新：$STALE_ROOT ⇒ 继续同步（本次会重走完整校验）"
  fi
fi

# ---------- 3. 取 gh-pages 树（rpm/ + 站点根文件）并交叉校验版本 ----------
log "② 取 gh-pages 树"
fetch_pages
PAGES_MSG="$(git -C "$CLONE" log -1 --format=%s FETCH_HEAD)"
log "   gh-pages tip: $PAGES_MSG"
# 交叉校验真值 = rpm 树内的实际里程碑版本（.mNNN），不是提交信息。
# >>> xcheck-rpm-tree >>>  （selftest_pxrepo_mirror.sh 按此标记抽取本段做离线回归，勿删改标记行）
RPM_MS="$(git -C "$CLONE" ls-tree -r --name-only FETCH_HEAD rpm 2>/dev/null \
          | grep -oE '\.m[0-9]+\.' | tr -d '.' | sort -u | tr '\n' ' ' || true)"
TAG_MS="$(printf '%s' "$TAG" | grep -oE 'm[0-9]+$' || true)"
if [ -n "$TAG_MS" ]; then
  case " $RPM_MS " in
    *" $TAG_MS "*) log "   ✅ 交叉校验：rpm 树含 $TAG_MS（= tag $TAG）" ;;
    *) die "版本交叉校验失败：rpm 树版本（${RPM_MS:-空}）不含 $TAG_MS（gh-pages 与 Release 不同步，保持原样）" ;;
  esac
  [ "$(printf '%s' "$RPM_MS" | wc -w)" -le 1 ] \
    || log "   ⚠ rpm 树内出现多个里程碑版本（$RPM_MS）—— 疑似上游 rsync 未 --delete"
else
  log "   ⚠ tag 无 -mNNN 后缀（$TAG），跳过 rpm 树版本交叉校验"
fi
# <<< xcheck-rpm-tree <<<

rm -rf "$STAGING"; mkdir -p "$STAGING"
git -C "$CLONE" archive --format=tar FETCH_HEAD rpm install-rpm.sh index.html 2>/dev/null \
  | tar -x -C "$STAGING" || die "gh-pages 归档解包失败"
[ -d "$STAGING/rpm/7/x86_64" ] && [ -d "$STAGING/rpm/9/x86_64" ] || die "gh-pages 缺少 rpm/{7,9}/x86_64"
[ -f "$STAGING/rpm/PUXIAN-GPG-KEY.asc" ] || die "缺少 rpm/PUXIAN-GPG-KEY.asc"

# ---------- 4. 取 Release 资产（tarball + sha256sums.txt） ----------
log "③ 下载 Release 资产（tag=$TAG）"
# 注：本仓库的 tag 是 annotated（refs/tags/X = tag 对象，X^{} = commit）⇒ 必须取 peeled
SHA7="$(git ls-remote "$REMOTE_URL" "refs/tags/$TAG^{}" | awk '{print $1}' | head -1 | cut -c1-7)"
[ -n "$SHA7" ] || SHA7="$(git ls-remote "$REMOTE_URL" "refs/tags/$TAG" | awk '{print $1}' | head -1 | cut -c1-7)"
[ -n "$SHA7" ] || die "取不到 $TAG 的 commit"
TARBALL="puxian-${TAG#v}-${SHA7}.tar.gz"
TAR_URL="$DL_BASE/$TAG/$TARBALL"
# 资产名以「实际可下载」为准：猜错（或命名规则变更）则回退 GitHub API 取真实资产名
if ! curl -fsS -L -r 0-0 -o /dev/null "$TAR_URL" 2>/dev/null; then
  log "   ⚠ 猜测资产名不可用（$TARBALL），改用 GitHub API 取实际资产名"
  TARBALL="$(curl -fsSL "https://api.github.com/repos/$REPO_SLUG/releases/tags/$TAG" \
             | grep -o '"name":"[^"]*\.tar\.gz"' | head -1 | sed 's/^"name":"//; s/"$//')"
  [ -n "$TARBALL" ] || die "取不到 $TAG 的 tarball 资产名"
  TAR_URL="$DL_BASE/$TAG/$TARBALL"
  log "   实际资产名 = $TARBALL"
fi
mkdir -p "$STAGING/releases"
curl -fsSL --retry 3 --retry-delay 5 -o "$STAGING/releases/sha256sums.txt" "$DL_BASE/$TAG/sha256sums.txt" \
  || die "下载 sha256sums.txt 失败"
curl -fsSL --retry 3 --retry-delay 5 -o "$STAGING/releases/$TARBALL" "$TAR_URL" \
  || die "下载 $TARBALL 失败"

SUM="$(sha256sum "$STAGING/releases/$TARBALL" | awk '{print $1}')"
GOT="$(grep -F "$TARBALL" "$STAGING/releases/sha256sums.txt" | awk '{print $1}' | head -1)"
[ -n "$GOT" ] || die "sha256sums.txt 中没有 $TARBALL 的登记值"
[ "$SUM" = "$GOT" ] || die "tarball 校验失败：实测 $SUM ≠ 登记 $GOT"
log "   ✅ tarball sha256 = $SUM（与 sha256sums.txt 一致）"

# ---------- 5. 校验 RPM 仓库（签名 + 元数据） ----------
log "④ 校验 rpm 仓库"
for d in 7 9; do
  rpmf="$(find "$STAGING/rpm/$d/x86_64" -maxdepth 1 -name '*.rpm' | head -1)"
  [ -n "$rpmf" ] || die "rpm/$d/x86_64 下没有 .rpm"
  rpm --import "$STAGING/rpm/PUXIAN-GPG-KEY.asc" 2>/dev/null || true
  rpm -Kv "$rpmf" >/dev/null 2>&1 || die "$(basename "$rpmf") 签名校验失败"
  [ -f "$STAGING/rpm/$d/x86_64/repodata/repomd.xml.asc" ] || die "rpm/$d 缺少 repomd.xml.asc"
  log "   ✅ el$d：$(basename "$rpmf") 验签通过"
done

# ---------- 6. 镜像自证文件 ----------
cat > "$STAGING/version.json" <<JSON
{
  "version": "$TAG",
  "tag": "$TAG",
  "commit": "$SHA7",
  "synced_at": "$(date -Iseconds)",
  "source": "https://github.com/$REPO_SLUG",
  "tarball": "releases/$TARBALL",
  "tarball_sha256": "$SUM",
  "rpm_repo": { "el7": "rpm/7/x86_64/", "el9": "rpm/9/x86_64/" }
}
JSON
log "⑤ version.json 就绪"

if [ "$DRY" = 1 ]; then
  log "🧪 --dry-run：解析与校验全部通过，未写 DEST"
  exit 0
fi

# ---------- 7. 发布（先新 rpm，再 repodata，最后删旧） ----------
log "⑥ 发布到 $DEST"
mkdir -p "$DEST/rpm" "$DEST/releases"
rsync -a --exclude='repodata/' "$STAGING/rpm/" "$DEST/rpm/"
rsync -a                "$STAGING/rpm/" "$DEST/rpm/"
rsync -a --delete       "$STAGING/rpm/" "$DEST/rpm/"
rsync -a --delete --exclude=/rpm/ "$STAGING/" "$DEST/"

# ---------- 8. 落地复核 ----------
log "⑦ 落地复核"
[ "$(sha256sum "$DEST/releases/$TARBALL" | awk '{print $1}')" = "$SUM" ] || die "落地后 tarball sha256 不一致"
grep -q "\"$TAG\"" "$DEST/version.json" || die "落地后 version.json 不含 $TAG"
log "✅ 完成：$DEST ← $TAG（rpm 7/9 + tarball + version.json）"
