#!/usr/bin/env bash
# ============================================================
# packaging/verify_repo_el7.sh —— el7 正式签名仓库终验（M78）
# ------------------------------------------------------------
# 对 rpm-build-9（rockylinux:9）代签并组好的 /rpm/7 仓库做真实 el7 消费验证：
#   yum 3.4 + rpm 4.11：gpgcheck=1 + repo_gpgcheck=1 双验签（repomd.xml.asc
#   与逐包签名）→ 真实安装 puxian → pxc --version。
# 执行：GitHub Actions ubuntu runner：
#   docker run --rm -v <pxrepo-all 解包目录>:/out \
#     centos:7 bash /src/packaging/verify_repo_el7.sh
#   /out = 完整仓库树（rpm/{7,9} + PUXIAN-GPG-KEY.asc），/src = 仓库（本脚本）
# ============================================================
set -euo pipefail

echo "== [verify-el7] 修正 CentOS 7 EOL 源 → vault.centos.org（https，备源 CERN vault）=="
cat > /etc/yum.repos.d/CentOS-Base.repo <<'EOF'
[base]
name=CentOS-$releasever - Base
baseurl=https://vault.centos.org/centos/$releasever/os/$basearch/
        https://linuxsoft.cern.ch/centos-vault/centos/$releasever/os/$basearch/
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7

[updates]
name=CentOS-$releasever - Updates
baseurl=https://vault.centos.org/centos/$releasever/updates/$basearch/
        https://linuxsoft.cern.ch/centos-vault/centos/$releasever/updates/$basearch/
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7

[extras]
name=CentOS-$releasever - Extras
baseurl=https://vault.centos.org/centos/$releasever/extras/$basearch/
        https://linuxsoft.cern.ch/centos-vault/centos/$releasever/extras/$basearch/
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7
EOF
yum -y makecache >/dev/null 2>&1 || true

echo "== [verify-el7] 写入 puxian 正式签名仓库（gpgcheck=1 + repo_gpgcheck=1）=="
cat > /etc/yum.repos.d/puxian-verify.repo <<'EOF'
[puxian-verify]
name=PuXian Signed Repo Verify (el7)
baseurl=file:///out/7/x86_64/
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=file:///out/PUXIAN-GPG-KEY.asc
EOF

echo "== [verify-el7] yum makecache 双验签（-y 免 tty 自动导入公钥）=="
yum -y makecache 2>&1 | tail -6 || { echo "❌ el7 yum makecache 双验签失败"; exit 1; }
echo "== [verify-el7] 仓库可见性 =="
yum -y --repo=puxian-verify list puxian || { echo "❌ yum list 找不到 puxian"; exit 1; }

echo "== [verify-el7] 真实安装 puxian + 运行验证 =="
yum -y install puxian >/dev/null
rpm -q puxian
/usr/bin/pxc --version
echo "✅ el7 正式签名仓库 yum 双验签 + 安装运行全部通过"
