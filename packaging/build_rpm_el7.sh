#!/usr/bin/env bash
# ============================================================
# packaging/build_rpm_el7.sh —— el7（CentOS 7 / RHEL 7）签名 RPM 构建（M77）
# ------------------------------------------------------------
# 背景：el7 的 gpg1/gpg2.0 无 --pinentry-mode loopback、无 createrepo_c、
#       CentOS 7 已 EOL（源迁 vault，且 http 强制 301→https）——故必须在
#       真实 el7 容器内构建 + 用 yum 3.4 真实验签/安装。
# 执行：GitHub Actions ubuntu runner（自带 docker）：
#   docker run --rm -v <workspace>:/src -v <out>:/out \
#     -e GPG_PRIVATE_KEY -e GPG_PASSPHRASE -e GPG_KEY_ID \
#     centos:7 bash /src/packaging/build_rpm_el7.sh
#   /src = 仓库（含 .git，build_rpm.sh 需 git describe）
#   /out = 仓库输出（host 卷，供 upload-artifact）
# 签名：用 gnupg(gpg1.4) —— gpg1 无 agent，--passphrase-file 直接本地解密，
#       无 tty 最稳（rpm4.11 默认 __gpg=/usr/bin/gpg 即 gpg1）。
# 流程：修 EOL 源(vault https) → yum 装工具 → 导入签名子密钥 → build_rpm.sh(DIST=7)
#       → yum file:// 仓库双验签 + 真实安装 pxc --version
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

echo "== [el7] yum 安装构建工具链（gnupg=gpg1.4，无 agent，headless 签名最稳）=="
yum -y makecache >/dev/null 2>&1 || true
yum -y install rpm-build rpm-sign createrepo gnupg git tar >/dev/null
echo "   ✅ 工具: $(rpmbuild --version | head -1), $(rpmsign --version | head -1), createrepo $(createrepo --version 2>&1 | head -1), $(gpg --version | head -1)"

# rpmbuild 打 .el7 后缀依赖 redhat-rpm-config 的 %dist 宏；缺失时兜底
if ! rpm -E '%{?dist}' 2>/dev/null | grep -q 'el7'; then
    yum -y install redhat-rpm-config >/dev/null
fi
echo "   ✅ %{dist} = $(rpm -E '%{?dist}')，签名 gpg = $(readlink -f "$(command -v gpg)")"

echo "== [el7] 信任工作区（容器内 git safe.directory）=="
git config --global --add safe.directory '*' || true

echo "== [el7] 导入签名私钥（gpg1 无 loopback → 直接 --import，签名走 --passphrase-file）=="
mkdir -p ~/.gnupg && chmod 700 ~/.gnupg
echo "$GPG_PRIVATE_KEY" | gpg --batch --yes --import
gpg --list-secret-keys --keyid-format LONG | grep -E '^(sec|ssb)' || { echo "❌ 私钥导入失败"; exit 1; }

echo "== [el7] 口令预检（detach-sign 一次，确认 passphrase 正确）=="
if [ -n "${GPG_PASSPHRASE:-}" ] && [ -n "${GPG_KEY_ID:-}" ]; then
    printf '%s' "$GPG_PASSPHRASE" > /tmp/.gpg-pass.pre
    chmod 600 /tmp/.gpg-pass.pre
    echo "puxian el7 ci sign preflight" > /tmp/sigtest
    gpg --batch --yes --no-tty --passphrase-file /tmp/.gpg-pass.pre \
        --local-user "$GPG_KEY_ID" --detach-sign -o /tmp/sigtest.sig /tmp/sigtest
    gpg --verify /tmp/sigtest.sig /tmp/sigtest >/dev/null 2>&1 \
        || { echo "❌ preflight 验签失败"; exit 1; }
    rm -f /tmp/.gpg-pass.pre /tmp/sigtest /tmp/sigtest.sig
    echo "   ✅ passphrase 校验通过（detach-sign + verify）"
fi

echo "== [el7] build_rpm.sh 全链路（DIST=7 → rpm/7/x86_64）=="
cd /src
export DIST=7
export REPO_OUT=/out
export GPG_KEY_ID="${GPG_KEY_ID:-}"
export GPG_PASSPHRASE="${GPG_PASSPHRASE:-}"
bash packaging/build_rpm.sh

echo "== [el7] yum 本地仓库自检（file:// 双验签 makecache + 真实安装）=="
cat > /etc/yum.repos.d/puxian-selfcheck.repo <<'EOF'
[puxian-selfcheck]
name=PuXian Selfcheck Repo (el7)
baseurl=file:///out/7/x86_64/
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=file:///out/PUXIAN-GPG-KEY.asc
EOF
yum -y makecache 2>&1 | tail -3 || { echo "❌ el7 yum makecache 验签失败"; exit 1; }
yum -y --repo=puxian-selfcheck list puxian >/dev/null || { echo "❌ yum list 找不到 puxian"; exit 1; }
echo "== [el7] 真实安装 puxian（含 gcc 依赖链）+ 运行验证 =="
yum -y install puxian >/dev/null
/usr/bin/pxc --version
rpm -q puxian
echo "✅ el7 全链路通过：vault https 源 + gpg1 签名 + createrepo(gzip) + yum 双验签 + 安装运行"
