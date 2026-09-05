#!/usr/bin/env bash
# ============================================================
# packaging/install-rpm.sh —— PuXian RPM 仓库安装脚本（M73）
# ------------------------------------------------------------
# 目标：RHEL 9 系（RHEL / Rocky / Alma / CentOS Stream 9 / Oracle 9）
#   Fedora 与 openEuler 的 $releasever 语义不同，目录未铺（见末尾待办）
# 用法：
#   sudo bash install-rpm.sh             # 仅添加 PuXian 仓库 + 导入公钥
#   sudo bash install-rpm.sh install     # 添加仓库后立即 dnf install puxian
#   sudo bash install-rpm.sh remove      # 移除 PuXian 仓库
# 原理：repo 文件内保留 $releasever/$basearch 字面量，由 dnf 展开为
#   9/x86_64 → https://nanzhangroup.github.io/PuXian/rpm/9/x86_64/
# 校验链：repo_gpgcheck=1 验 repomd.xml.asc，gpgcheck=1 验每个 rpm 包签名，
#   公钥来自 GitHub Pages 同源托管的 PUXIAN-GPG-KEY.asc。
# ============================================================
set -euo pipefail

BASE="https://nanzhangroup.github.io/PuXian/rpm"
REPO_FILE=/etc/yum.repos.d/puxian.repo
REPO_ID=puxian
ACTION="${1:-}"

# ---- 权限与系统探测 ----
[ "$(id -u)" = 0 ] || { echo "❌ 请用 root 或 sudo 运行"; exit 1; }
command -v dnf >/dev/null 2>&1 || { echo "❌ 未找到 dnf（本脚本面向 RHEL 9 系）"; exit 1; }
. /etc/os-release
case "$ID" in
  rhel|rocky|almalinux|centos|ol)
    case "${VERSION_ID%%.*}" in
      9) RV=9 ;;
      *) echo "❌ 暂只支持 9 系（当前 $ID $VERSION_ID），Fedora/openEuler 仓库见 packaging/README 待办"; exit 1 ;;
    esac ;;
  *) echo "❌ 暂只支持 RHEL 9 系（当前 $ID），本脚本面向 dnf/yum"; exit 1 ;;
esac

if [ "$ACTION" = "remove" ]; then
  rm -f "$REPO_FILE"
  echo "✅ 已移除 $REPO_FILE"
  exit 0
fi

# ---- 写入 repo 文件（$releasever/$basearch 需 dnf 展开 → 单引号 heredoc 再拼 BASE）----
cat > "$REPO_FILE" <<EOF
[$REPO_ID]
name=PuXian Repository ($RV)
baseurl=${BASE}/\$releasever/\$basearch/
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=${BASE}/PUXIAN-GPG-KEY.asc
EOF
echo "✅ 仓库已写入 $REPO_FILE"

# ---- 导入公钥（避免 dnf 首次交互询问）----
rpm --import "${BASE}/PUXIAN-GPG-KEY.asc" 2>/dev/null \
  && echo "✅ PuXian 公钥已导入" \
  || echo "⚠️ 公钥导入失败，可忽略（dnf 首次安装时会从 gpgkey URL 自动拉取并提示确认）"

# ---- 验证仓库可访问 ----
if dnf -q makecache >/dev/null 2>&1; then
  echo "✅ 仓库元数据已缓存并通过验签（repo_gpgcheck）"
else
  echo "⚠️ makecache 未完全通过（网络/验签），安装时 dnf 会再尝试"
fi

if [ "$ACTION" = "install" ]; then
  echo "== 安装 puxian =="
  dnf -y install puxian
  echo
  echo "✅ 安装完成：$(pxc --version 2>/dev/null || echo 'pxc 已装（新开 shell 生效 PATH）')"
else
  echo
  echo "下一步："
  echo "  sudo dnf install puxian        # 或 sudo bash $0 install"
  echo "升级（新里程碑发布后自动）："
  echo "  sudo dnf upgrade puxian"
fi
