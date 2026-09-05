# ============================================================
# packaging/puxian.spec —— PuXian RPM 打包定义（M73：dnf/yum 分发）
# ------------------------------------------------------------
# 用法（由 tools/build_rpm.sh 驱动，勿手动裸跑 rpmbuild）：
#   rpmbuild -bb \
#     --define "_sourcedir <dir>" \
#     --define "pxtag m72"          # 里程碑（Release 字段）
#     --define "pxsha 2e6ac8d"      # commit short sha（Source 名拼接）
#     packaging/puxian.spec
# 布局（对齐 tools/pxc argv0 自发现：软链 /usr/bin/pxc → 包根/tools/pxc，
#   任意 cwd 可用，免 PX_STDLIB）：
#   /usr/share/puxian/     包树（tools bootstrap stdlib runtime LICENSE RELEASE.md ...）
#   /usr/bin/pxc -> /usr/share/puxian/tools/pxc
# 依赖：仅 gcc（pxc build 需要 cc）。bootstrap/* 均 statically linked，
#   零动态库依赖，跨发行版通用（RHEL/Fedora/Rocky/Alma/openEuler/CentOS...）。
# 版本：主版本固定 0.1.0（与 tag v0.1.0-mXX 对齐）；Release=1.m<里程碑>
#   → RPM 版本序 1.m72 < 1.m73，dnf upgrade 自动升级路径正确。
# ============================================================

Name:           puxian
Version:        0.1.0
Release:        1.%{pxtag}%{?dist}
Summary:        PuXian programming language compiler and toolchain

License:        Apache-2.0
URL:            https://github.com/NanzhanGroup/PuXian
Source0:        puxian-%{version}-%{pxtag}-%{pxsha}.tar.gz

# pxc build 需要 C 编译器（gcc 静态链接 .px → ELF）
Requires:       gcc
Requires:       bash
Requires:       tar

# bootstrap/* 静态 ELF，无需动态库；工具全为 bash + 自举二进制
AutoReqProv:    no

# 免 debuginfo/debugsource 子包噪音 + 关闭 brp-strip：发布物含跨架构静态库
# （runtime/mbedtls/lib-aarch64/*.a）与 not-stripped 自举二进制，x86 构建机
# strip 会误伤；且保持 rpm 内文件与发布 tarball 逐字节一致（sha256 可对账）
%define         debug_package %{nil}
%global         _build_id_links none
%global         __os_install_post %(echo '%{nil}')

%description
PuXian (普贤) is a self-hosted, AI-first programming language whose compiler
is written in itself. This package installs the complete toolchain:
  pxc build / run / fmt / lint / test / bench / doc / lsp / mcp / version

All bootstrap binaries are statically linked (zero dynamic deps), and pxc
locates its package root via argv[0] resolution, so it works from any
directory without PX_STDLIB.

%prep
%setup -q -n puxian-%{version}-%{pxtag}-%{pxsha}

%install
rm -rf %{buildroot}
# 包树平铺到 /usr/share/puxian（= 发布 tarball 内容，strip 顶层目录）
install -d %{buildroot}%{_datadir}/puxian
cp -a tools bootstrap stdlib runtime \
      LICENSE RELEASE.md \
      %{buildroot}%{_datadir}/puxian/
# 可执行位兜底（cp -a 已保留，防个别环境 umask）
find %{buildroot}%{_datadir}/puxian/bootstrap -type f -exec chmod +x {} \;
chmod +x %{buildroot}%{_datadir}/puxian/tools/pxc \
        %{buildroot}%{_datadir}/puxian/tools/*.sh \
        %{buildroot}%{_datadir}/puxian/tools/pxpkg \
        %{buildroot}%{_datadir}/puxian/tools/routegen 2>/dev/null || true
# PATH 软链（argv0 自发现解析真实位置 → 包根）
install -d %{buildroot}%{_bindir}
ln -s %{_datadir}/puxian/tools/pxc %{buildroot}%{_bindir}/pxc

%files
%{_datadir}/puxian/
%{_bindir}/pxc

%changelog
* Sun Sep 06 2026 The PuXian Authors - 0.1.0-m72
- 首个 RPM 打包（M73：dnf/yum 分发），内容对齐发布 tarball v0.1.0-m72
