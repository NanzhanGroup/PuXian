#!/usr/bin/env bash
# ============================================================
# packaging/pxrepo_mirror.sh —— PuXian 发布镜像器（GitHub → 自建静态站点）
# ------------------------------------------------------------
# 用途：把 PuXian 的「最新发布」镜像到一个静态站点子目录，供国内用户高速下载：
#   rpm/{7,9}/x86_64/   签名 RPM 仓库（含 repodata + repomd.xml.asc）
#   rpm/openeuler/<ver>/x86_64/   openEuler 别名仓库（M168 · 同包同签名，元数据 gz）
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
ALLTAGS="$(git ls-remote --tags "$REMOTE_URL" \
        | awk '{print $2}' | sed 's#^refs/tags/##; s/\^{}$//' | sort -u || true)"
# >>> tag-name-guard >>>  （selftest_pxrepo_mirror.sh 按此标记抽取本段做离线回归，勿删改标记行）
# 规则（唯一）：v<主版本>-m<里程碑>，例 v0.2.0-m167。**没有补丁后缀**。
# M238（用户令 2026-10-01）：tag 命名**是发布规则的一部分**，不允许自由发挥。
#   事故：误打 v0.2.0-m237s3 ⇒ ① 版本序正则把它**静默过滤**（镜像永远停在旧版、无任何告警）；
#   ② build_rpm.sh 的 MILESTONE 取短横线之后整段 ⇒ rpm 包名变成
#   puxian-0.2.0-1.m237s3.el9.x86_64.rpm ⇒ 与 gh-pages 的 `.mNNN.` 交叉校验失配。
#   **违规必须响亮**：把不参与版本序的 tag 全部列出来。
# 本段**自包含**（TAG_RE / TAGS / die 都在段内）—— 离线回归按标记抽取后可直接执行。
TAG_RE='^v[0-9]+\.[0-9]+\.[0-9]+(-m[0-9]+)?$'
TAGS="$(printf '%s\n' "$ALLTAGS" | grep -E "$TAG_RE" || true)"
[ -n "$TAGS" ] || die "远端没有任何符合 v<M>.<m>.<p>[-mNNN] 的 tag"
BADTAGS="$(printf '%s\n' "$ALLTAGS" | grep -vE "$TAG_RE" | grep -v '^$' || true)"
if [ -n "$BADTAGS" ]; then
  # 豁免表：规则确立（M238）之前的**历史遗留**，只减不增 —— 见 packaging/tag_name_exempt.txt。
  # 未登记的违规 ⇒ **响亮**（默认不拦：宁可同步旧版，也不要把镜像整体停掉；
  #   需要硬拦时设 PXREPO_STRICT_TAGS=1）。
  _exf="${PXREPO_EXEMPT_FILE:-$(dirname "$0")/tag_name_exempt.txt}"
  _exl=""
  if [ -f "$_exf" ]; then
    _exl="$(grep -vE '^[[:space:]]*(#|$)' "$_exf" | awk '{print $1}' || true)"
  fi
  if [ -n "$_exl" ]; then
    _badnew="$(printf '%s\n' "$BADTAGS" | grep -vxF "$_exl" || true)"
  else
    _badnew="$BADTAGS"
  fi
  _nnew="$(printf '%s\n' "$_badnew" | grep -c . || true)"
  if [ "$_nnew" -gt 0 ]; then
    log "   ⚠ 远端有 $_nnew 个**未登记**的不合规 tag（规则应为 v<M>.<m>.<p>[-mNNN]，例 v0.2.0-m167）"
    log "      它们不参与版本序 ⇒ 对应 Release 永远不会被镜像："
    printf '%s\n' "$_badnew" | sed 's/^/        · /'
    log "      整改：把合规 tag 指向该里程碑的最终提交，再 git push origin --delete <违规 tag>"
    if [ "${PXREPO_STRICT_TAGS:-0}" = 1 ]; then
      die "PXREPO_STRICT_TAGS=1：存在未登记的不合规 tag，拒绝继续（清单见上）"
    fi
  fi
  _nall="$(printf '%s\n' "$BADTAGS" | grep -c . || true)"
  if [ "$_nall" -gt "$_nnew" ]; then
    log "   ℹ️ 另有 $((_nall - _nnew)) 个违规 tag 已登记在豁免表（历史遗留，不告警）"
  fi
fi
# <<< tag-name-guard <<<

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
# 违规形态（补丁后缀，如 .m237s3.）单独抓一份 —— 否则 die 消息只能说「不同步」，
# 指不到真因（真因是上游 tag / rpm 包名不合规）。2026-10-01 实测踩到。
RPM_MS_BAD="$(git -C "$CLONE" ls-tree -r --name-only FETCH_HEAD rpm 2>/dev/null \
          | grep -oE '\.m[0-9]+s[0-9]+\.' | tr -d '.' | sort -u | tr '\n' ' ' || true)"
TAG_MS="$(printf '%s' "$TAG" | grep -oE 'm[0-9]+$' || true)"
if [ -z "$TAG_MS" ]; then
  # 新形态 v0.2.271（M272 起）：**第三段就是里程碑号** —— 没有 -m 段可抓。
  #   ⚠ 这里修前会静默落到「跳过交叉校验」分支 ⇒ 覆盖率无声下降（M235 缺陷 355 同族）。
  _p="$(printf '%s' "$TAG" | sed -nE 's/^v[0-9]+\.[0-9]+\.([0-9]+)$/\1/p')"
  _p="$(printf '%s' "$_p" | sed 's/^0*//')"   # v0.2.0 ⇒ 空 ⇒ 保持「跳过校验」（不是 m0）
  [ -n "$_p" ] && TAG_MS="m$_p"
fi
if [ -n "$TAG_MS" ]; then
  case " $RPM_MS " in
    *" $TAG_MS "*) log "   ✅ 交叉校验：rpm 树含 $TAG_MS（= tag $TAG）" ;;
    *)
      if [ -n "$RPM_MS_BAD" ]; then
        die "版本交叉校验失败：rpm 树里的版本标记不合规（${RPM_MS_BAD}）—— 上游 tag 带了补丁后缀（如 -m237s3），build_rpm.sh 把短横线之后整段当作 MILESTONE ⇒ rpm 包名被污染。整改：删违规 tag + 合规 tag 指向该里程碑最终提交 + 重发。"
      fi
      die "版本交叉校验失败：rpm 树版本（${RPM_MS:-空}）不含 $TAG_MS（gh-pages 与 Release 不同步，保持原样）" ;;
  esac
  [ "$(printf '%s' "$RPM_MS" | wc -w)" -le 1 ] \
    || log "   ⚠ rpm 树内出现多个里程碑版本（$RPM_MS）—— 疑似上游 rsync 未 --delete"
else
  log "   ⚠ tag 形态无法派生里程碑号（$TAG），跳过 rpm 树版本交叉校验"
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
SHA="$(git ls-remote "$REMOTE_URL" "refs/tags/$TAG^{}" | awk '{print $1}' | head -1 || true)"
[ -n "$SHA" ] || SHA="$(git ls-remote "$REMOTE_URL" "refs/tags/$TAG" | awk '{print $1}' | head -1 || true)"
[ -n "$SHA" ] || die "取不到 $TAG 的 commit"
SHORT="$(printf '%s' "$SHA" | cut -c1-8)"
# 资产名**以 GitHub API 为真值**（不猜短 SHA 位数）。
#   2026-10-01 实测：自 v0.2.0-m235 起资产名里的短 SHA 由 7 位变 8 位（git 随仓库对象数自动加长）
#   ⇒ 猜名必 404；而**原来的回退分支**用 grep 匹配不到 GitHub API 的 `"name": "…"`
#   （冒号后带空格），叠加 set -euo pipefail ⇒ **命令替换非零 ⇒ 脚本静默退出，
#   连 die 的 ❌ 都打不出来** ⇒ 镜像同步自 09-30 起每 30 分钟失败一轮、无人知晓
#   （晨曦 2026-10-01 报障）。故：**先 API（真值）**，API 不可用才退回猜（8/7 位各试）。
REL_JSON="$WORK/rel-$TAG.json"
TARBALL=""
A64_TARBALL=""
# >>> asset-select >>>
# 从一个 GitHub Release JSON 里挑出**两类**资产名（M277 · 用户报障）：
#   · main = 主发布包（`puxian-<ver>-<sha>.tar.gz`）
#   · a64  = aarch64 官方引导包（`puxian-bootstrap-aarch64-<tag>.tar.gz`）
# ⚠️ 修前只取「**第一个** .tar.gz」⇒ aarch64 包**从未进过镜像**
#    （用户 2026-10-06 报障：openEuler aarch64 上提示「aarch64 包不在镜像上」）。
# 本段被 `packaging/selftest_pxrepo_mirror.sh` **抽取**做离线回归（勿删标记行）。
pick_release_assets() {   # $1=release JSON 路径 → 两行 "main\t<名>" / "a64\t<名>"
  grep -oE '"name"[[:space:]]*:[[:space:]]*"[^"]*\.tar\.gz"' "$1" 2>/dev/null \
    | sed -E 's/^"name"[[:space:]]*:[[:space:]]*"//; s/"$//' \
    | awk '
        /^puxian-bootstrap-aarch64-/ { if (a64 == "") a64 = $0; next }
        /\.tar\.gz$/                { if (main == "") main = $0 }
        END { printf "main\t%s\na64\t%s\n", main, a64 }'
}
# <<< asset-select <<<
if curl -fsSL -o "$REL_JSON" "https://api.github.com/repos/$REPO_SLUG/releases/tags/$TAG" 2>/dev/null; then
  _assets="$(pick_release_assets "$REL_JSON")"
  TARBALL="$(printf '%s\n' "$_assets" | awk -F'\t' '$1=="main"{print $2}')"
  A64_TARBALL="$(printf '%s\n' "$_assets" | awk -F'\t' '$1=="a64"{print $2}')"
  [ -n "$TARBALL" ] || die "GitHub API 响应里没有主 .tar.gz 资产（tag=$TAG，原始响应留在 $REL_JSON）"
  log "   资产名（API 真值）= $TARBALL"
  if [ -n "$A64_TARBALL" ]; then
    log "   aarch64 引导包（API 真值）= $A64_TARBALL"
  else
    log "   ℹ️ 该 Release 无 aarch64 引导包资产（老 tag 可能确实没有）"
  fi
else
  log "   ⚠ GitHub API 不可用（限流/离线）⇒ 退回按短 SHA 猜资产名（8 位 / 7 位各试一次）"
  for _n in 8 7; do
    _c="puxian-${TAG#v}-$(printf '%s' "$SHA" | cut -c1-$_n).tar.gz"
    if curl -fsS -L -r 0-0 -o /dev/null "$DL_BASE/$TAG/$_c" 2>/dev/null; then TARBALL="$_c"; break; fi
  done
  [ -n "$TARBALL" ] || die "取不到 $TAG 的 tarball 资产名（API 不可用，短 SHA 8/7 位猜测均 404）"
  log "   猜中资产名 = $TARBALL"
  # aarch64 引导包**按 tag 命名**（不含短 SHA）⇒ 猜法固定；取不到只记 ℹ️（老 tag 可能没有）
  _a64="puxian-bootstrap-aarch64-${TAG}.tar.gz"
  if curl -fsS -L -r 0-0 -o /dev/null "$DL_BASE/$TAG/$_a64" 2>/dev/null; then
    A64_TARBALL="$_a64"; log "   aarch64 引导包（按 tag 猜中）= $_a64"
  else
    log "   ℹ️ aarch64 引导包探测 404（老 tag 可能没有）"
  fi
fi
TAR_URL="$DL_BASE/$TAG/$TARBALL"
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

# ── 4b. aarch64 官方引导包（M277 · 用户报障「aarch64 包不在镜像上」）──────────
#   存在 ⇒ **必须**下载并校验（判定不了不许放行）；不存在 ⇒ ℹ️ 记一行继续。
A64_SUM=""; A64_SIZE=""
if [ -n "$A64_TARBALL" ]; then
  curl -fsSL --retry 3 --retry-delay 5 -o "$STAGING/releases/$A64_TARBALL" "$DL_BASE/$TAG/$A64_TARBALL" \
    || die "下载 aarch64 引导包 $A64_TARBALL 失败（资产存在但取不下来）"
  A64_SUM="$(sha256sum "$STAGING/releases/$A64_TARBALL" | awk '{print $1}')"
  A64_SIZE="$(stat -c%s "$STAGING/releases/$A64_TARBALL" 2>/dev/null || echo 0)"
  # 校验值来源**两选一**：优先同名 .sha256 资产；否则查 sha256sums.txt
  A64_GOT=""
  if curl -fsSL --retry 2 -o "$WORK/$A64_TARBALL.sha256" "$DL_BASE/$TAG/$A64_TARBALL.sha256" 2>/dev/null; then
    A64_GOT="$(awk 'NF>=1{print $1; exit}' "$WORK/$A64_TARBALL.sha256" 2>/dev/null || true)"
  fi
  [ -n "$A64_GOT" ] || A64_GOT="$(grep -F "$A64_TARBALL" "$STAGING/releases/sha256sums.txt" | awk '{print $1}' | head -1)"
  [ -n "$A64_GOT" ] || die "aarch64 引导包**在发布里**却**找不到校验值**（既无 .sha256 资产、也不在 sha256sums.txt）⇒ 拒绝放行"
  [ "$A64_SUM" = "$A64_GOT" ] || die "aarch64 引导包校验失败：实测 $A64_SUM ≠ 登记 $A64_GOT"
  log "   ✅ aarch64 引导包 sha256 = $A64_SUM（$(( A64_SIZE / 1048576 )) MB）"
fi

# ---------- 5. 校验 RPM 仓库（签名 + 元数据） ----------
# M168：目录集合由「写死 7/9」改为「7/9 + 实际存在的 openeuler/<ver>」——
#   镜像侧若只认 RHEL 系，新铺的 openEuler 目录会被**静默漏掉**（用户装到 404 元数据）。
log "④ 校验 rpm 仓库"
# >>> rpm-tree-layout >>>  （selftest_pxrepo_mirror.sh 按此标记抽取本段做离线回归，勿删改标记行）
# ★ 镜像树布局（**唯一事实源**）：
#       rpm/<dist>/x86_64/<文件>
#         dist ∈ {7, 9, openeuler/<ver>}   ← 发行版维度
#         arch  = x86_64                   ← **必须显式拼出**，不许省略
#   ★ 事故（M168 引入 · 晨曦 2026-10-02 干跑发现 · 2026-10-04 第 2 次催办）：
#     M168 改造时丢了 arch 层 ⇒ find "$STAGING/rpm/$d" 与 …/repodata/repomd.xml.asc
#     都指向**不存在**的路径 ⇒ 任何按本脚本部署的镜像器都在 §⑤ 直接 die，
#     而且是在「已经拉完数百 MB 资产之后」才死。
#     对照：M168 之前的版本（4127306）此处写的是 "$STAGING/rpm/$d/x86_64" ⇒ 是**回退**，不是设计。
#   ★ 为什么长期没被拦住：§⑤ **没有任何判据覆盖** —— selftest 只造 rpm/9/x86_64 树，
#     但它抽取的三段里没有一段执行 §⑤。⇒ 下面两个访问器 + selftest 的 [4] 段补上这个缺口。
#   ★ 纪律：路径拼接**只在这里**（访问器是唯一入口）⇒ 调用点不许自己拼路径。
rpm_dist_dirs() {   # $1=站点根 ⇒ 输出 dist 名列表（空格分隔，顺序稳定）
  local root="$1" _d _v out="7 9"
  for _d in "$root"/rpm/openeuler/*/x86_64; do
    [ -d "$_d" ] || continue
    _v="$(basename "$(dirname "$_d")")"
    out="$out openeuler/$_v"
  done
  printf '%s\n' "$out"
}
rpm_tree_dir() {    # $1=站点根 $2=dist 名 ⇒ 输出该 dist 的 **arch 层**路径
  printf '%s/rpm/%s/x86_64\n' "$1" "$2"
}
RPM_DIRS="$(rpm_dist_dirs "$STAGING")"
log "   目录集合：$RPM_DIRS"
for d in $RPM_DIRS; do
  _rdir="$(rpm_tree_dir "$STAGING" "$d")"
  rpmf="$(find "$_rdir" -maxdepth 1 -name '*.rpm' | head -1)"
  [ -n "$rpmf" ] || die "rpm/$d/x86_64 下没有 .rpm"
  rpm --import "$STAGING/rpm/PUXIAN-GPG-KEY.asc" 2>/dev/null || true
  rpm -Kv "$rpmf" >/dev/null 2>&1 || die "$(basename "$rpmf") 签名校验失败"
  [ -f "$_rdir/repodata/repomd.xml.asc" ] || die "rpm/$d/x86_64 缺少 repomd.xml.asc"
  log "   ✅ $d：$(basename "$rpmf") 验签通过"
done
# <<< rpm-tree-layout <<<

# ---------- 6.5 registry/（W3：镜像侧 …/puxian/registry/ 可达） ----------
# 令源：用户 2026-10-05「pxpkg sync 的 A / W3，做」· docs/PXPKG_SYNC_PLAN.md §五 W3。
# 背景（晨曦实测）：发布段是 `rsync -a --delete --exclude=/rpm/` ⇒ DEST 下**除 rpm/ 外的
#   内容**都会被 STAGING 覆盖；registry/ 若不在 STAGING 里，上一轮放进去的会被**静默删掉**
#   （覆盖率无声下降 —— M235 缺陷 355 同族）。
# 口径：**以发布 tarball 为唯一事实源**（不另取 gh-pages，避免双源漂移）。
# >>> registry-mirror >>>  （selftest_pxrepo_mirror.sh 按此标记抽取本段做离线回归，勿删改标记行）
REG_FOUND=0
REG_N=0
# ⚠ M279 缺陷 487：**不能**写成 `tar -tzf … | grep -q X` —— 本脚本 set -o pipefail，
#   而 grep -q 命中即退出 ⇒ tar 收 SIGPIPE（141）⇒ 整条管道非零 ⇒ 判据**恒假**
#   （实测：grep -q 单独 rc=0，带 pipefail 整条管道 rc=141）
#   ⇒ registry/ 永远不被提取，正是本段注释声称要避免的「覆盖率无声下降」。
#   口径：**先把清单落盘，再对文件判**（同 packaging/selftest_make_release.sh:17 的既有教训）。
_TARLIST="$WORK/tarlist.txt"
tar -tzf "$STAGING/releases/$TARBALL" 2>/dev/null > "$_TARLIST" || true
if grep -q '/registry/README\.md$' "$_TARLIST"; then
  if tar -xzf "$STAGING/releases/$TARBALL" -C "$STAGING" --strip-components=1 \
        --wildcards '*/registry/*' 2>/dev/null; then
    REG_N="$(find "$STAGING/registry" -type f 2>/dev/null | wc -l)"
    if [ "$REG_N" -gt 0 ]; then
      REG_FOUND=1
      log "⑤ registry/ 已提取：$REG_N 件（镜像侧将提供 …/puxian/registry/）"
    else
      die "registry/ 提取后为空（tarball 结构可能变了）"
    fi
  else
    die "registry/ 提取失败（tarball=$TARBALL）"
  fi
else
  log "   ℹ tarball 内不含 registry/（老版本资产）⇒ 本轮不提供，且**保护** DEST 现役内容"
fi
# <<< registry-mirror <<<


# ---------- 6. 镜像自证文件 ----------
# M168：rpm_repo 按**实际目录集合**生成（新增 openEuler 后版本对账口径同步）
RPM_REPO_JSON=""
for d in $RPM_DIRS; do
  case "$d" in
    7)  k=el7 ;;
    9)  k=el9 ;;
    openeuler/*) k="openeuler_${d#openeuler/}" ;;
    *)  k="$d" ;;
  esac
  [ -n "$RPM_REPO_JSON" ] && RPM_REPO_JSON="$RPM_REPO_JSON, "
  # M254：仓库基址必须含 arch 层（与 §⑤ 的树布局同口径）——
  #   M168 后曾退化为 "rpm/$d/"，任何按 version.json 的 rpm_repo 配 baseurl 的第三方都会 404
  #   （实测 https://soft.xiusoft.cn/puxian/rpm/9/repodata/repomd.xml → 404；真实在 rpm/9/x86_64/ 下）。
  #   现网客户端不受影响（/etc/yum.repos.d/puxian.repo 用 $releasever/$basearch 自己拼）。
  RPM_REPO_JSON="$RPM_REPO_JSON\"$k\": \"rpm/$d/x86_64/\""
done
cat > "$STAGING/version.json" <<JSON
{
  "version": "$TAG",
  "tag": "$TAG",
  "commit": "$SHORT",
  "synced_at": "$(date -Iseconds)",
  "source": "https://github.com/$REPO_SLUG",
  "tarball": "releases/$TARBALL",
  "tarball_sha256": "$SUM",
  "rpm_repo": { $RPM_REPO_JSON },
  "registry": { "included": $REG_FOUND, "files": $REG_N, "base": "registry/" },
  "bootstrap_aarch64_tarball": "$A64_TARBALL",
  "bootstrap_aarch64_sha256": "$A64_SUM",
  "bootstrap_aarch64_size": ${A64_SIZE:-0}
}
JSON
log "⑤ version.json 就绪（rpm_repo: $RPM_REPO_JSON）"

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
# registry/：STAGING 有时**随流同步**；无时**护住 DEST 现役**（--delete 会删掉它）
REG_EXCL=()
if [ "$REG_FOUND" = 1 ]; then
  log "   registry/ 随 STAGING 同步（不排除）"
else
  REG_EXCL+=(--exclude=/registry/)
  log "   registry/ 不在 STAGING ⇒ --exclude=/registry/ 保护 DEST 现役内容"
fi
rsync -a --delete "${REG_EXCL[@]}" --exclude=/rpm/ "$STAGING/" "$DEST/"

# ---------- 8. 落地复核 ----------
log "⑦ 落地复核"
[ "$(sha256sum "$DEST/releases/$TARBALL" | awk '{print $1}')" = "$SUM" ] || die "落地后 tarball sha256 不一致"
grep -q "\"$TAG\"" "$DEST/version.json" || die "落地后 version.json 不含 $TAG"
# M277：aarch64 引导包若本轮同步了，**落地后**必须真在 DEST 上且 sha256 一致
#   （只报「同步成功」不够 —— M275 的教训：判定不了不许放行）
if [ -n "$A64_TARBALL" ]; then
  [ -f "$DEST/releases/$A64_TARBALL" ] || die "aarch64 引导包未落到 DEST：$A64_TARBALL"
  [ "$(sha256sum "$DEST/releases/$A64_TARBALL" | awk '{print $1}')" = "$A64_SUM" ] \
    || die "落地后 aarch64 引导包 sha256 不一致"
  grep -q "\"bootstrap_aarch64_tarball\": \"releases/$A64_TARBALL\"" "$DEST/version.json" \
    || die "落地后 version.json 未登记 aarch64 引导包"
fi
log "✅ 完成：$DEST ← $TAG（rpm 7/9 + tarball + version.json）"
