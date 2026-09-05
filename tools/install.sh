#!/usr/bin/env bash
# ============================================================
# tools/install.sh —— PuXian 一键安装（M71-S4，B4）
# ------------------------------------------------------------
# 从 GitHub Release 下载官方 tarball → 验 sha256（sha256sums.txt）
# → 解压安装 → PATH 软链。pxc argv0 自发现（tools/pxc readlink 真实根）
# + stdlib 自动注入 → 软链后任意目录可用，免 PX_STDLIB/相对路径。
# 用法：
#   curl -fsSL https://raw.githubusercontent.com/NanzhanGroup/PuXian/main/tools/install.sh | sh
#   bash tools/install.sh                 # 安装 latest release（GitHub API）
#   bash tools/install.sh v0.1.0-m71      # 指定 tag
#   bash tools/install.sh --prefix ~/.local --version v0.1.0-m71
# 环境：PX_PREFIX（安装前缀；缺省 ~/.local，root 无参时 /usr/local）
# 产物：
#   <prefix>/share/puxian/<tag>/   包根（tarball 解压，strip 顶层目录）
#   <prefix>/bin/pxc → 软链 包根/tools/pxc
# 架构：当前官方资产 x86_64；aarch64 用户提示用 --target 交叉（见 ROADMAP M71）
# 依赖：curl + tar + sha256sum；GitHub 可达
# ============================================================
set -eu
REPO="NanzhanGroup/PuXian"
API="https://api.github.com/repos/$REPO"
REL="https://github.com/$REPO/releases/download"

usage() { sed -n '2,28p' "$0"; }

VERSION=""
PREFIX="${PX_PREFIX:-}"
PREFIX_SET=0
while [ $# -gt 0 ]; do
    case "$1" in
        --version|-v) VERSION="$2"; shift 2 ;;
        --prefix|-p) PREFIX="$2"; PREFIX_SET=1; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        -*) echo "未知参数: $1"; usage; exit 1 ;;
        *) VERSION="$1"; shift ;;
    esac
done

# ---- 架构检测 ----
MACH="$(uname -m)"
case "$MACH" in
    x86_64|amd64) ARCH="x86_64" ;;
    aarch64|arm64)
        echo "提示：当前官方 Release 资产为 x86_64（aarch64 原生包按需，见 ROADMAP M71）。"
        echo "  aarch64 部署：在 x86_64 开发机用官方包交叉 —— pxc build --target aarch64 <app.px>（产物直接跑 aarch64）。"
        exit 1 ;;
    *) echo "错误: 不支持架构 $MACH（当前仅 x86_64 资产）"; exit 1 ;;
esac

# ---- 前缀（root 未显式指定 → /usr/local；否则 ~/.local）----
[ -n "$PREFIX" ] || PREFIX="$HOME/.local"
if [ "$(id -u)" = "0" ] && [ "$PREFIX_SET" = 0 ] && [ -z "${PX_PREFIX:-}" ]; then
    PREFIX="/usr/local"
fi
BIN_DIR="$PREFIX/bin"
PKG_DIR="$PREFIX/share/puxian"

# ---- 版本解析 ----
if [ -z "$VERSION" ]; then
    echo "== 查询最新 release =="
    VERSION="$(curl -fsSL "$API/releases/latest" | grep -o '"tag_name": *"[^"]*"' | head -1 | sed 's/.*"tag_name": *"//; s/"//' || true)"
    [ -n "$VERSION" ] || { echo "错误: 无法解析 latest tag（GitHub API 不可达？）"; exit 1; }
fi
echo "== 安装 PuXian $VERSION (arch=$ARCH) =="

# ---- 资产定位 ----
META="$(curl -fsSL "$API/releases/tags/$VERSION")"
PKG_NAME="$(echo "$META" | grep -o '"name": *"[^"]*\.tar\.gz"' | head -1 | sed 's/.*"name": *"//; s/"//' || true)"
[ -n "$PKG_NAME" ] || { echo "错误: release $VERSION 无 tarball 资产"; exit 1; }

# ---- 下载 + sha256 校验 ----
echo "== 下载 $PKG_NAME =="
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
curl -fL --retry 3 -o "$TMP/pkg.tar.gz" "$REL/$VERSION/$PKG_NAME"
if curl -fsSL --retry 3 -o "$TMP/sha256sums.txt" "$REL/$VERSION/sha256sums.txt"; then
    EXPECT="$(awk -v n="$PKG_NAME" '$2 == n {print $1}' "$TMP/sha256sums.txt" | head -1)"
    GOT="$(sha256sum "$TMP/pkg.tar.gz" | awk '{print $1}')"
    if [ -n "$EXPECT" ] && [ "$EXPECT" != "$GOT" ]; then
        echo "错误: sha256 不匹配（$GOT ≠ $EXPECT），已中止" >&2
        exit 1
    fi
    echo "== sha256 校验通过（$GOT）=="
else
    echo "（无 sha256sums.txt 资产，跳过校验）"
fi

# ---- 解压安装 + 软链 ----
echo "== 解压安装 =="
DEST="$PKG_DIR/$VERSION"
rm -rf "$DEST"
mkdir -p "$DEST" "$BIN_DIR"
tar -C "$DEST" -xzf "$TMP/pkg.tar.gz" --strip-components=1
chmod +x "$DEST/tools/pxc" "$DEST/bootstrap/pxc" "$DEST/bootstrap/pxi" 2>/dev/null || true
ln -sf "$DEST/tools/pxc" "$BIN_DIR/pxc"
echo "== 安装完成 =="
echo "  包根: $DEST"
echo "  入口: $BIN_DIR/pxc"
"$BIN_DIR/pxc" --version
echo ""
echo "  用法: export PATH=\"$BIN_DIR:\$PATH\""
echo "  然后: pxc run app.px / pxc build --target aarch64 app.px / pxc mcp"
