# ============================================================
# packaging/puxian.spec —— PuXian RPM 打包定义（M73：dnf/yum 分发）
# ------------------------------------------------------------
# 用法（由 packaging/build_rpm.sh 驱动，勿手动裸跑 rpmbuild）：
#   rpmbuild -bb \
#     --define "_sourcedir <dir>" \
#     --define "pxtag m72"          # 里程碑（Release 字段）
#     --define "pxsha 2e6ac8d"      # commit short sha（Source 名拼接）
#     packaging/puxian.spec
# 布局（对齐 tools/px argv0 自发现：软链 /usr/bin/px → 包根/tools/px，
#   /usr/bin/pxc → px（兼容别名，M86-S0），任意 cwd 可用，免 PX_STDLIB）：
#   /usr/share/puxian/     包树（tools bootstrap stdlib runtime LICENSE RELEASE.md ...）
#   /usr/bin/px  -> /usr/share/puxian/tools/px
#   /usr/bin/pxc -> px（同目录相对软链，等价别名）
# 依赖：仅 gcc（pxc build 需要 cc）。bootstrap/* 均 statically linked，
#   零动态库依赖，跨发行版通用（RHEL/Fedora/Rocky/Alma/openEuler/CentOS...）。
# 版本：主版本 %{pxver} 由 build_rpm.sh 自 git tag 派生并 --define 注入
#   （tag v<ver>[-m<里程碑>]，如 v0.2.0 / v0.1.0-m72 → pxver=0.2.0 / 0.1.0），
#   禁止写死 —— 版本升格只需打新 tag，spec 自动跟随。
#   Release=1.<里程碑>.el<dist>（无 -m 段时 build_rpm.sh 兜底 dev 或 m<最近里程碑>）
#   → RPM 版本序递增正确（1.m72 < 1.m73；0.2.0 < 0.2.1），dnf upgrade 自动升级。
# ============================================================

Name:           puxian
Version:        %{pxver}
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
  px build / run / fmt / lint / test / bench / doc / lsp / mcp / version
  （pxc 为兼容别名，等价可用）

All bootstrap binaries are statically linked (zero dynamic deps), and px
locates its package root via argv[0] resolution, so it works from any
directory without PX_STDLIB.

%prep
%setup -q -n puxian-%{version}-%{pxtag}-%{pxsha}

%install
rm -rf %{buildroot}
# 包树平铺到 /usr/share/puxian（= 发布 tarball 内容，strip 顶层目录）
install -d %{buildroot}%{_datadir}/puxian
cp -a tools bootstrap stdlib runtime \
      LICENSE RELEASE.md VERSION \
      %{buildroot}%{_datadir}/puxian/
# 可执行位兜底（cp -a 已保留，防个别环境 umask）
find %{buildroot}%{_datadir}/puxian/bootstrap -type f -exec chmod +x {} \;
chmod +x %{buildroot}%{_datadir}/puxian/tools/px \
        %{buildroot}%{_datadir}/puxian/tools/pxc \
        %{buildroot}%{_datadir}/puxian/tools/*.sh \
        %{buildroot}%{_datadir}/puxian/tools/pxpkg \
        %{buildroot}%{_datadir}/puxian/tools/routegen 2>/dev/null || true
# PATH 软链（argv0 自发现解析真实位置 → 包根；px 官方名 + pxc 兼容别名 M86-S0）
install -d %{buildroot}%{_bindir}
ln -s %{_datadir}/puxian/tools/px %{buildroot}%{_bindir}/px
ln -s px %{buildroot}%{_bindir}/pxc

%files
%{_datadir}/puxian/
%{_bindir}/px
%{_bindir}/pxc

%changelog
* Mon Sep 07 2026 The PuXian Authors - 0.2.0
- 打包链版本宏化：spec Version 由 build_rpm.sh 按 git tag 注入 %{pxver}，适配语义版本 tag v0.2.0（修复 el7 RPM job 因 spec 固定 0.1.0 与 tag 不一致而失败）

* Sun Sep 06 2026 The PuXian Authors - 0.1.0-m72
- 首个 RPM 打包（M73：dnf/yum 分发），内容对齐发布 tarball v0.1.0-m72
