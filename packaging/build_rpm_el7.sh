#!/usr/bin/env bash
# ============================================================
# packaging/build_rpm_el7.sh —— el7（CentOS 7 / RHEL 7）RPM 构建 + 运行自检（M77/M78）
# ------------------------------------------------------------
# 背景：CentOS 7 已 EOL（源迁 vault，http 强制 301→https）；el7 的 gpg2.0 无
#       --pinentry-mode loopback，rpm4.11 对 __gpg_sign_cmd 注入不稳 ——
#       headless 包签名在 el7 原生容器内不可靠（M77 实测 Bad passphrase）。
# 因此 M78 起 el7 只在此容器内【无签名构建】+【gpgcheck=0 运行自检】：
#   1) 正式签名（rpm4.16 + loopback + passphrase-file）由 rpm-build-9
#      （rockylinux:9）job 代签，与 el9 签名同一条已验证链路；
#   2) 最终【gpgcheck=1 + repo_gpgcheck=1 双验签 + 安装运行】由 rpm-verify-7
#      job 对正式签名仓库执行（centos:7 + yum 3.4）。
# 执行：GitHub Actions ubuntu runner（自带 docker）：
#   docker run --rm -v <workspace>:/src -v <out>:/out \
#     [-e GPG_*（本脚本忽略）] centos:7 bash /src/packaging/build_rpm_el7.sh
#   /src = 仓库（含 .git，build_rpm.sh 需 git describe）
#   /out = 仓库输出（host 卷，供 upload-artifact → el9 代签 job 下载）
# ============================================================
set -euo pipefail

echo "== [el7] 修正 CentOS 7 EOL 源 → vault.centos.org（https，备源 CERN vault）=="
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

echo "== [el7] yum 安装构建工具链（无需 gpg：签名由 el9 侧 rpm4.16 代签）=="
yum -y makecache >/dev/null 2>&1 || true
yum -y install rpm-build createrepo git tar >/dev/null
echo "   ✅ 工具: $(rpmbuild --version | head -1), createrepo $(createrepo --version 2>&1 | head -1)"

# rpmbuild 打 .el7 后缀依赖 redhat-rpm-config 的 %dist 宏；缺失时兜底
if ! rpm -E '%{?dist}' 2>/dev/null | grep -q 'el7'; then
    yum -y install redhat-rpm-config >/dev/null
fi
echo "   ✅ %{dist} = $(rpm -E '%{?dist}')"

echo "== [el7] 信任工作区（容器内 git safe.directory）=="
git config --global --add safe.directory '*' || true

echo "== [el7] build_rpm.sh（DIST=7, SKIP_SIGN=1 → 未签名 rpm + 仓库元数据）=="
cd /src
export DIST=7
export REPO_OUT=/out
export SKIP_SIGN=1
unset GPG_KEY_ID GPG_PASSPHRASE || true
bash packaging/build_rpm.sh

echo "== [el7] yum 本地自检（无签名：gpgcheck=0，仅验证 el7 可消费 + pxc 可运行）=="
cat > /etc/yum.repos.d/puxian-selfcheck.repo <<'EOF'
[puxian-selfcheck]
name=PuXian Selfcheck Repo (el7, unsigned)
baseurl=file:///out/7/x86_64/
enabled=1
gpgcheck=0
repo_gpgcheck=0
EOF
yum -y makecache 2>&1 | tail -3 || { echo "❌ el7 yum makecache 失败"; exit 1; }
yum -y --repo=puxian-selfcheck list puxian >/dev/null || { echo "❌ yum list 找不到 puxian"; exit 1; }
echo "== [el7] 真实安装 puxian + 运行验证 =="
yum -y install puxian >/dev/null
/usr/bin/pxc --version
rpm -q puxian
echo "✅ el7 无签名构建 + 运行自检通过（签名/双验签分别在 el9 代签 job 与 rpm-verify-7 job）"
