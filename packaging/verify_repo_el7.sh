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
# ⚠️ **限定到本仓库**：vault.centos.org 时通时不通，若把它的失败算进"验签失败"会掩盖真因
if ! yum -y --disablerepo='*' --enablerepo=puxian-verify makecache 2>&1 | tail -6; then
    echo "❌ 步骤失败: yum makecache（双验签）" >&2; exit 1
fi
echo "== [verify-el7] 仓库可见性 =="
# yum 3.4（el7）无 dnf 风格 --repo 选项 → 用 --disablerepo/--enablerepo
yum -y --disablerepo='*' --enablerepo=puxian-verify list puxian || { echo "❌ yum list 找不到 puxian"; exit 1; }

echo "== [verify-el7] 真实安装 puxian + 运行验证 =="
if ! yum -y install puxian >/dev/null 2>&1; then
    echo "❌ 步骤失败: yum install puxian（依赖解析/下载）" >&2
    yum -y install puxian 2>&1 | tail -25 >&2
    exit 1
fi
rpm -q puxian
/usr/bin/pxc --version

# ---- M168：入库件必须**全静态**（ldd 判定）----
# 为什么补这一层：M168 前 bootstrap/pxc_vm（**用户面默认 VM 轨**）是动态件、要求
#   GLIBC_2.34，而 el7 只有 glibc 2.17 ⇒ 用户 `px build` 一执行就崩；而本脚本当时
#   只跑 `pxc --version`（走 C 轨件 pxc，恰好是静态的）⇒ **整类缺陷看不见**。
echo "== [verify-el7] 入库件静态性自检（动态件在 el7 上必崩）=="
bad=0
for b in /usr/share/puxian/bootstrap/*; do
  [ -f "$b" ] || continue
  # ⚠️ **不要写成 `ldd … | grep -q …`**：`ldd` 对静态件的退出码是 **1**（"not a dynamic
  #   executable" 走 stderr），在 `set -o pipefail` 下整条管道非零 ⇒ 会把**静态件误判为动态件**
  #   （2026-09-21 el7 终验实测：14 件全红，而每行的 ldd 原文恰好是"not a dynamic executable"）。
  #   ⇒ 先取输出、再判内容（与退出码解耦）。
  _ldd_out="$(ldd "$b" 2>&1 || true)"
  case "$_ldd_out" in
    *"not a dynamic executable"*) echo "   ✅ $(basename "$b")：静态" ;;
    *) echo "   ❌ $(basename "$b")：**动态件** —— $(printf '%s' "$_ldd_out" | head -3 | tr '\n' ' ')"
       bad=$((bad+1)) ;;
  esac
done
[ "$bad" = 0 ] || { echo "❌ 有 $bad 个动态件（分发可移植性口径：入库件必须全静态）"; exit 1; }

# ---- M168：真编译 + 真运行（默认 VM 轨）----
# 契约（**要么真跑通，要么给出可执行的指引**）：el7 自带 gcc 4.8.5 < 4.9 ⇒ 不支持 C11
#   `<stdatomic.h>`（runtime 需要）⇒ `px build` 在本容器里**必然**失败。此时我们要求失败
#   原因必须是**已登记且可执行**的那条（tools/px 的能力预检给出 devtoolset 指引）；
#   若是别的失败，一律判红。安装 / 工具链 / 入库件静态性仍是**硬判据**。
echo "== [verify-el7] 真编译 + 真运行（默认 VM 轨：pxc → bootstrap/pxc_vm）=="
mkdir -p /tmp/pxt && cd /tmp/pxt
printf 'print("el7-pkg-ok")\n' > hello.px
if /usr/bin/pxc build hello.px > /tmp/pxt/build.log 2>&1; then
    [ -x /tmp/pxt/build/hello ] || { echo "❌ 缺产物 /tmp/pxt/build/hello"; ls -lR /tmp/pxt; exit 1; }
    GOT="$(/tmp/pxt/build/hello)"
    [ "$GOT" = "el7-pkg-ok" ] || { echo "❌ 产物运行输出异常：[$GOT]"; exit 1; }
    echo "   编译产物运行 OK：$GOT"
    /usr/bin/pxc run hello.px | grep -q el7-pkg-ok || { echo "❌ px run 失败"; exit 1; }
    echo "✅ el7 正式签名仓库 yum 双验签 + 安装 + 真编译运行全部通过"
else
    if grep -q "不支持 C11 原子\|stdatomic" /tmp/pxt/build.log; then
        echo "⚠️ LIMITATION（**已登记，非静默放过**）：el7 自带 gcc 4.8.5 < 4.9 ⇒"
        echo "   无法编译 runtime（C11 <stdatomic.h>），故本容器内 px build 不可用。"
        tail -6 /tmp/pxt/build.log | sed 's/^/   | /'
        echo "   el7 用户请用 devtoolset（SCL）："
        echo "     sudo yum install -y centos-release-scl && sudo yum install -y devtoolset-9"
        echo "     scl enable devtoolset-9 bash        # 或 PX_CC=/opt/rh/devtoolset-9/root/usr/bin/gcc"
        echo "   （安装 / 工具链 / 入库件静态性三项硬判据均已通过；本项按上述契约放行）"
        echo "✅ el7：repo 双验签 + 安装 + 工具链可用 + 入库件全静态通过（px build 需 gcc ≥ 4.9，见上）"
    else
        echo "❌ px build 失败，且**不是**已知的 gcc 版本限制 ⇒ 判红"
        tail -30 /tmp/pxt/build.log | sed 's/^/   | /'
        exit 1
    fi
fi
