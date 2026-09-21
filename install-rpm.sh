#!/usr/bin/env bash
# ============================================================
# packaging/install-rpm.sh —— PuXian RPM 仓库安装脚本（M73 dnf / M77 el7 yum / M168 openEuler）
# ------------------------------------------------------------
# 设计口径（M168 重写，**收掉一类结构性缺陷**）：
#   旧版把「发行版是否支持」写成一条 `case $ID in rhel|rocky|... ) ... esac`，
#   并**完全依赖包管理器展开 `$releasever`** —— 那是 RHEL 系的约定（7 / 9）。
#   于是 openEuler 一举踩两个坑：
#     ① 认脸就退出（`❌ 暂支持 RHEL 系 7/9`，连试都没试）；
#     ② 即使放行，`$releasever` = `22.03LTS` ⇒ 去 `rpm/22.03LTS/x86_64/` 取元数据 ⇒ 404。
#   本版改为**显式映射表**：由**脚本**决定去哪个仓库目录，不再把解释权交给各发行版
#   语义不一的 `$releasever`。
#     · RHEL 系 7/9：保留 `$releasever`（已验证、目录名恰是这个数）
#     · openEuler 22.03/24.03：写**字面**目录 `openeuler/<ver>/<arch>/`
#       （该发行版 glibc 2.34/2.38 ≥ 我们的 ABI 基线，故复用 el9 构建物；CI 在
#        openeuler:22.03 容器里**真装真跑**，不是「大概能用」）
#     · 其余（el8/el10 / Fedora / openEuler 20.03 / 非 x86_64）：**明确不支持**，
#       并给出**可执行**的替代路线（tarball / aarch64 引导包），不再只报一句"暂支持"。
# 用法：
#   sudo bash install-rpm.sh                # 添加仓库 + 导入公钥
#   sudo bash install-rpm.sh install        # 添加仓库后立即 dnf/yum install puxian
#   sudo bash install-rpm.sh remove         # 移除 PuXian 仓库
#   bash install-rpm.sh status              # 只报「本机是什么 / 会去哪个仓库」（免 root）
#   bash install-rpm.sh --dry-run           # 只打印将写入的 .repo 内容，不落盘（免 root）
# 选项：
#   -n|--dry-run        不写文件、不导公钥、不跑包管理器
#       --base URL      覆盖仓库基址（同 PUXIAN_RPM_BASE）
#       --os-release F  用 FILE 代替 /etc/os-release（**离线自测用**）
#       --repo-file F   写入 FILE 代替 /etc/yum.repos.d/puxian.repo（自测用）
#       --arch A        覆盖架构探测（同 PUXIAN_ARCH）
#   -h|--help
# 环境变量：PUXIAN_RPM_BASE / PUXIAN_OS_RELEASE / PUXIAN_REPO_FILE / PUXIAN_ARCH /
#           PUXIAN_OFFLINE=1（禁网络探测，自测用）
# 双活（两处仓库内容等价，public 资产同一发布产物）：
#   国内镜像  https://soft.xiusoft.cn/puxian/rpm     ← 默认优先（探测通过即用）
#   上游兜底  https://nanzhangroup.github.io/PuXian/rpm
# 校验链：repo_gpgcheck=1 验 repomd.xml.asc，gpgcheck=1 验每个 rpm 包签名，
#   公钥来自与仓库同源托管的 PUXIAN-GPG-KEY.asc（签名不绑定域名，两处等价）。
# ============================================================
set -euo pipefail

MIRROR_ROOT="https://soft.xiusoft.cn/puxian"
UPSTREAM_ROOT="https://nanzhangroup.github.io/PuXian"
GH_REPO="NanzhanGroup/PuXian"
GPG_KEY_FILE="PUXIAN-GPG-KEY.asc"
REPO_ID=puxian
DEFAULT_REPO_FILE=/etc/yum.repos.d/puxian.repo
# 我们构建物的 glibc 基线（el9 / openEuler 22.03 都是 2.34）。低于它的发行版
# **不能**直接用 el9 仓库 —— 那是「装得上、跑不起来」，比 404 更难查。
ABI_BASELINE_GLIBC="2.34"

ACTION=""
DRY=0
BASE_OVERRIDE="${PUXIAN_RPM_BASE:-}"
OS_RELEASE_FILE="${PUXIAN_OS_RELEASE:-/etc/os-release}"
REPO_FILE="${PUXIAN_REPO_FILE:-$DEFAULT_REPO_FILE}"
ARCH_OVERRIDE="${PUXIAN_ARCH:-}"

die()  { echo "❌ $*" >&2; exit 1; }
warn() { echo "⚠️  $*" >&2; }
say()  { echo "$*"; }

# ---- 参数解析（位置动作 install/remove/status + 选项，顺序无关）----
while [ $# -gt 0 ]; do
  case "$1" in
    install|remove|status) ACTION="$1"; shift ;;
    -n|--dry-run)          DRY=1; shift ;;
    --base)                BASE_OVERRIDE="$2"; shift 2 ;;
    --os-release)          OS_RELEASE_FILE="$2"; shift 2 ;;
    --repo-file)           REPO_FILE="$2"; shift 2 ;;
    --arch)                ARCH_OVERRIDE="$2"; shift 2 ;;
    -h|--help)             sed -n '2,40p' "$0"; exit 0 ;;
    *) die "未知参数：$1（-h 看用法）" ;;
  esac
done
: "${ACTION:=repo}"   # 缺省：只加仓库

# ============================================================
# 一 发行版认脸 —— 显式映射，不猜
# ============================================================
# 输出（4 字段，`|` 分隔）：FAMILY|DIR_MODE|DIR|NOTE
#   FAMILY    el / oe / other（分支用）
#   DIR_MODE  releasever（交给包管理器展开） / literal（脚本定死目录）
#   DIR       literal 时的目录名
#   NOTE      人类可读的 ABI 说明（打印给用户，也是自测的判据之一）
classify() {   # $1=ID $2=ID_LIKE $3=VERSION_ID
  local id il ver major is_el verified
  id="$(printf '%s' "$1" | tr 'A-Z' 'a-z')"
  il="$(printf '%s' "$2" | tr 'A-Z' 'a-z')"
  ver="$3"
  major="${ver%%.*}"

  if [ "$id" = openeuler ]; then
    case "$ver" in
      22.03*) echo "oe|literal|openeuler/22.03|openEuler $ver（glibc 2.34 = el9 ABI 基线）"; return 0 ;;
      24.03*) echo "oe|literal|openeuler/24.03|openEuler $ver（glibc 2.38 ≥ 基线）"; return 0 ;;
      20.03*) echo "other|literal||openEuler $ver 的 glibc 2.28 **低于**基线 $ABI_BASELINE_GLIBC（装得上、跑不起来）" ; return 0 ;;
      *)      echo "other|literal||未识别的 openEuler 版本 $ver（只验证过 22.03 / 24.03）"; return 0 ;;
    esac
  fi
  [ "$id" = fedora ] && { echo "other|literal||Fedora 未铺仓库（打包链与 EL 不同，未验证）"; return 0; }

  is_el=0; verified=0
  case "$id" in
    rhel|centos|rocky|almalinux|ol|oracle|oraclelinux|scientific) is_el=1; verified=1 ;;
  esac
  if [ "$is_el" = 0 ]; then
    case " $il " in
      *rhel*|*centos*|*ol*) is_el=1 ;;   # 未验证的 EL 系克隆（anolis/opencloudos/…）
    esac
  fi
  if [ "$is_el" = 0 ]; then
    echo "other|literal||未识别的发行版（ID=$id，ID_LIKE='${2}'）"; return 0
  fi

  case "$major" in
    7) if [ "$verified" = 1 ]; then echo "el|releasever|7|RHEL 系 el7（glibc 2.17 · yum 3.4）"
       else echo "el|releasever|7|$id $ver（EL 系克隆，未验证 · glibc 2.17）"; fi ;;
    9) if [ "$verified" = 1 ]; then echo "el|releasever|9|RHEL 系 el9（glibc 2.34）"
       else echo "el|releasever|9|$id $ver（EL 系克隆，未验证 · 按 el9 ABI 装）"; fi ;;
    8|10) echo "other|literal||el$major 目录未发布（el$major 的 glibc 与 el9 不同，**不能**混用 el9 仓库）" ;;
    *) echo "other|literal||RHEL 系版本 $ver 未识别（只验证过 el7 / el9）" ;;
  esac
}

# ---- .repo 的 baseurl：RHEL 系交给 $releasever；其余脚本定死目录 ----
repo_baseurl() {   # $1=BASE $2=MODE $3=DIR $4=ARCH
  if [ "$2" = releasever ]; then printf '%s/$releasever/$basearch/' "$1"
  else printf '%s/%s/%s/' "$1" "$3" "$4"; fi
}

# ============================================================
# 二 替代路线提示（**失败必须给出可执行的下一步**）
# ============================================================
# 读站点自证文件 version.json（含 tag / tarball / sha256）⇒ 打印**精确**命令；
# 取不到（离线/镜像不可达）则退回 Releases 页面指引。
fallback_hint() {   # $1=arch
  local arch="$1" vj="" tag="" rel="" sha=""
  say ""
  say "── 替代路线（本组合暂无 rpm 仓库，但 PuXian 有两条官方装机路径）──"
  if [ "${PUXIAN_OFFLINE:-0}" != 1 ] && command -v curl >/dev/null 2>&1; then
    vj="$(curl -fsS -m 5 "$MIRROR_ROOT/version.json" 2>/dev/null || true)"
  fi
  if [ -n "$vj" ]; then
    tag="$(printf '%s' "$vj" | sed -n 's/.*"tag" *: *"\([^"]*\)".*/\1/p')"
    rel="$(printf '%s' "$vj" | sed -n 's/.*"tarball" *: *"\([^"]*\)".*/\1/p')"
    sha="$(printf '%s' "$vj" | sed -n 's/.*"tarball_sha256" *: *"\([^"]*\)".*/\1/p')"
  fi
  if [ "$arch" = aarch64 ]; then
    say "  ① aarch64 官方引导包（**包内工具链为静态件、零 glibc 依赖**，解压即用）："
    if [ -n "$tag" ]; then
      say "       curl -fsSLO https://github.com/$GH_REPO/releases/download/${tag}/puxian-bootstrap-aarch64-${tag}.tar.gz"
      say "       sha256sum -c puxian-bootstrap-aarch64-${tag}.tar.gz.sha256   # 可选校验"
      say "       tar xzf puxian-bootstrap-aarch64-${tag}.tar.gz && cd puxian-bootstrap-aarch64-${tag}"
    else
      say "       打开 https://github.com/$GH_REPO/releases/latest 下载 puxian-bootstrap-aarch64-<tag>.tar.gz"
    fi
    say "       export PATH=\$PWD/tools:\$PATH && px build hello.px && ./build/hello"
    say "  ② 或者用本机 gcc 自举一套原生工具链（只需 gcc，约 1-3 分钟）："
    say "       ./selfhost/native_bootstrap.sh --portable --install"
  else
    say "  ① 官方发布 tarball（任何 x86_64 发行版通用，不挑 dnf/glibc 版本）："
    if [ -n "$rel" ]; then
      say "       curl -fsSL -o $(basename "$rel") $MIRROR_ROOT/$rel"
      if [ -n "$sha" ]; then say "       echo '$sha  $(basename "$rel")' | sha256sum -c"; fi
      say "       tar xzf $(basename "$rel") && cd \$(basename $(basename "$rel") .tar.gz)"
    else
      say "       打开 https://github.com/$GH_REPO/releases/latest 取 puxian-<ver>-<里程碑>-<sha>.tar.gz"
    fi
    say "       export PATH=\$PWD/tools:\$PATH && px build hello.px && ./build/hello"
    say "  ② 或者用本机 gcc 自举：./selfhost/native_bootstrap.sh --portable"
  fi
  say "  （站点自证/版本对账：$MIRROR_ROOT/version.json）"
}

support_matrix() {
  say ""
  say "── 当前支持的组合（RPM 仓库）──"
  say "  RHEL / Rocky / Alma / CentOS / Oracle / Scientific …  el7 · el9    x86_64"
  say "  openEuler 22.03 LTS · 24.03 LTS                                   x86_64"
  say "  （el8/el10 目录未发布；aarch64 无 rpm 仓库 ⇒ 走上面的 aarch64 引导包）"
}

# ============================================================
# 三 主流程
# ============================================================
[ -r "$OS_RELEASE_FILE" ] || die "读不到 $OS_RELEASE_FILE（可用 --os-release <file> 指定）"
# shellcheck disable=SC1090
. "$OS_RELEASE_FILE"
ID="${ID:-unknown}"; VERSION_ID="${VERSION_ID:-}"; ID_LIKE="${ID_LIKE:-}"

ARCH="${ARCH_OVERRIDE:-$(uname -m)}"
case "$ARCH" in x86_64|amd64) ARCH=x86_64 ;; aarch64|arm64) ARCH=aarch64 ;; esac

IFS='|' read -r FAMILY DIR_MODE DIR NOTE <<<"$(classify "$ID" "$ID_LIKE" "$VERSION_ID")"

# 仓库基址选择（双活：默认国内镜像；探测失败自动回退上游）
pick_base() {
  if [ -n "$BASE_OVERRIDE" ]; then printf '%s|指定' "$BASE_OVERRIDE"; return 0; fi
  if [ "${PUXIAN_OFFLINE:-0}" = 1 ] || ! command -v curl >/dev/null 2>&1; then
    printf '%s|未探测（离线/无 curl）' "${MIRROR_ROOT}/rpm"; return 0
  fi
  if curl -fsS -m 3 -o /dev/null "${MIRROR_ROOT}/version.json" 2>/dev/null; then
    printf '%s|国内镜像探测通过' "${MIRROR_ROOT}/rpm"
  else
    printf '%s|国内镜像探测失败，回退上游 GitHub Pages' "${UPSTREAM_ROOT}/rpm"
  fi
}
IFS='|' read -r BASE WHY <<<"$(pick_base)"

say "== 系统: ${PRETTY_NAME:-$ID $VERSION_ID}（架构 $ARCH）=="
say "== 仓库基址: $BASE（$WHY）=="

# ---- 支持性判定：先架构，后发行版 ----
if [ "$FAMILY" = el ] || [ "$FAMILY" = oe ]; then TARGET_SUPPORTED=1; else TARGET_SUPPORTED=0; fi

if [ "$ARCH" != x86_64 ]; then
  # RPM 仓库目前只铺 x86_64（aarch64 的重活是引导包，不是 rpm）
  say "== 组合: $ARCH —— 无 rpm 仓库（rpm 仓库只铺 x86_64）=="
  if [ "$ACTION" = remove ]; then
    rm -f "$REPO_FILE"; say "✅ 已移除 $REPO_FILE（若存在）"; exit 0
  fi
  fallback_hint "$ARCH"
  exit 1
fi

if [ "$TARGET_SUPPORTED" != 1 ]; then
  say "== 组合: $ID $VERSION_ID —— **暂无 rpm 仓库** =="
  say "   原因：$NOTE"
  if [ "$ACTION" = remove ]; then
    rm -f "$REPO_FILE"; say "✅ 已移除 $REPO_FILE（若存在）"; exit 0
  fi
  support_matrix
  fallback_hint "$ARCH"
  exit 1
fi
say "== 仓库目录: ${DIR}/$ARCH（$NOTE）=="

# ---- status：只看不写（免 root）----
if [ "$ACTION" = status ]; then
  say ""
  say "会写入:  $REPO_FILE"
  say "baseurl: $(repo_baseurl "$BASE" "$DIR_MODE" "$DIR" "$ARCH")"
  say "gpgkey:  $BASE/$GPG_KEY_FILE"
  say "（--dry-run 可看完整 .repo 内容；sudo bash $0 才真装）"
  exit 0
fi

# ---- remove ----
if [ "$ACTION" = remove ]; then
  [ "$(id -u)" = 0 ] || die "请用 root 或 sudo 运行（要删 $REPO_FILE）"
  rm -f "$REPO_FILE"
  say "✅ 已移除 $REPO_FILE"
  exit 0
fi

# ---- 真实安装路径：需要 root + 包管理器（dry-run 免）----
PM=""
if command -v dnf >/dev/null 2>&1; then PM=dnf; elif command -v yum >/dev/null 2>&1; then PM=yum; fi

URL="$(repo_baseurl "$BASE" "$DIR_MODE" "$DIR" "$ARCH")"
REPO_CONTENT="[$REPO_ID]
name=PuXian Repository ($ID $VERSION_ID · ${DIR}/$ARCH)
baseurl=$URL
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=$BASE/$GPG_KEY_FILE
"

if [ "$DRY" = 1 ]; then
  say ""
  say "── [dry-run] 将写入 $REPO_FILE 的内容 ──"
  printf '%s' "$REPO_CONTENT"
  say "── 还将执行：rpm --import $BASE/$GPG_KEY_FILE ──"
  if [ "$ACTION" = install ]; then say "── 还将执行：${PM:-dnf} -y install puxian ──"; fi
  exit 0
fi

[ "$(id -u)" = 0 ] || die "请用 root 或 sudo 运行（要写 $REPO_FILE 并导入公钥）"
[ -n "$PM" ] || die "未找到 dnf/yum —— 本脚本面向 rpm 系；非 rpm 系请走上文 tarball 路线"

printf '%s' "$REPO_CONTENT" > "$REPO_FILE"
say "✅ 仓库已写入 $REPO_FILE"

# ---- 导入公钥（避免包管理器首次交互询问）----
if rpm --import "$BASE/$GPG_KEY_FILE" 2>/dev/null; then
  say "✅ PuXian 公钥已导入"
else
  warn "公钥导入失败，可忽略（安装时会从 gpgkey URL 自动拉取并提示确认）"
fi

# ---- 验证仓库可访问（repo_gpgcheck 同样校验 repomd.xml.asc）----
if $PM -q makecache >/dev/null 2>&1; then
  say "✅ 仓库元数据已缓存并通过验签（repo_gpgcheck）"
else
  warn "makecache 未完全通过（网络/验签），安装时会再尝试"
fi

if [ "$ACTION" = install ]; then
  say "== 安装 puxian =="
  $PM -y install puxian
  say ""
  say "✅ 安装完成：$(pxc --version 2>/dev/null || echo 'pxc 已装（新开 shell 生效 PATH）')"
else
  say ""
  say "下一步："
  say "  sudo $PM install puxian        # 或 sudo bash $0 install"
  say "升级（新里程碑发布后自动）："
  say "  sudo $PM upgrade puxian"
  if [ "$PM" = yum ]; then say "  sudo yum update puxian"; fi
fi
