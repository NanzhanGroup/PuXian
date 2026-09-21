#!/usr/bin/env bash
# ============================================================
# packaging/verify_repo_openeuler.sh —— openEuler 正式签名仓库终验（M168）
# ------------------------------------------------------------
# 存在理由（2026-09-21 用户报障）：
#   官方 install-rpm.sh **根本不认 openEuler**（`❌ 暂支持 RHEL 系 7/9` 直接退出），
#   而仓库里也没有 openEuler 的目录（`$releasever`=22.03LTS ⇒ 必然 404）。
#   这类"缺少一条分发链"在 CI 里**完全不可见** —— 因为 CI 只跑 centos:7 / rockylinux:9。
#   ⇒ 本脚本在**真 openEuler 容器**里把整条链走一遍：写 repo 文件 → 双验签 makecache
#     → 真实安装 → `pxc --version` → **真编译一个程序并运行**（不是只看 --version）。
# 为什么必须"真编译"：RPM 里的 VM 轨编译器（bootstrap/pxc_vm）历史上是**动态件**，
#   在 glibc 较老的发行版上"装得上、跑不起来"；只跑 `--version` 的验证**永远发现不了**。
# 执行（GitHub Actions ubuntu runner）：
#   docker run --rm -v <pxrepo-all 解包目录>:/out \
#     openeuler/openeuler:22.03-lts bash /src/packaging/verify_repo_openeuler.sh
#   /out = 完整仓库树（rpm/{7,9} + rpm/openeuler/<ver> + PUXIAN-GPG-KEY.asc），/src = 仓库
# ============================================================
set -euo pipefail

OE_VER="${OE_VER:-22.03}"          # 仓库目录版本段（对应 rpm/openeuler/<ver>/x86_64）
REPO_DIR="/out/openeuler/$OE_VER/x86_64"

echo "== [verify-oe] 环境自述（取证用）=="
. /etc/os-release
echo "   发行版: ${PRETTY_NAME:-$ID $VERSION_ID} · 架构 $(uname -m)"
echo "   glibc:  $(ldd --version 2>/dev/null | head -1)"
echo "   dnf:    $(dnf --version 2>/dev/null | head -1)"
echo "   rpm:    $(rpm --version)"

echo "== [verify-oe] 仓库目录自检 =="
[ -d "$REPO_DIR" ] || { echo "❌ 缺仓库目录 $REPO_DIR（CI 未铺 openEuler 别名仓库？）"; ls -R /out/rpm 2>/dev/null | head -20; exit 1; }
ls -l "$REPO_DIR" | head -8
ls "$REPO_DIR/repodata" | head -8

echo "== [verify-oe] 写入 PuXian 签名仓库（gpgcheck=1 + repo_gpgcheck=1）=="
cat > /etc/yum.repos.d/puxian-verify.repo <<EOF
[puxian-verify]
name=PuXian Signed Repo Verify (openEuler $OE_VER)
baseurl=file://$REPO_DIR/
enabled=1
gpgcheck=1
repo_gpgcheck=1
gpgkey=file:///out/PUXIAN-GPG-KEY.asc
EOF
cat /etc/yum.repos.d/puxian-verify.repo

echo "== [verify-oe] dnf makecache 双验签（-y 免 tty 自动导入公钥）=="
# ⚠️ **限定到本仓库**（--repo=puxian-verify）：不带上容器自带源 —— 否则"镜像源在境外不可达"
#   会把本仓库的验签结论污染成"验签失败"（实为网络问题）。自带源在本步之后才需要（装 gcc）。
if ! dnf -y --repo=puxian-verify makecache 2>&1 | tail -8; then
    echo "❌ 步骤失败: makecache（双验签）—— 见上" >&2
    dnf -y --repo=puxian-verify makecache 2>&1 | tail -20 >&2
    exit 1
fi

echo "== [verify-oe] 仓库可见性 =="
# ⚠️ 用 `dnf list`（核心功能）而**不用** `dnf repoquery` —— 后者在 dnf-plugins-core 里，
#   精简镜像（含 openeuler 容器）常常没有 ⇒ 会以 "No such command: repoquery" 收场，
#   看起来像"仓库不可见"，实则是**工具缺件**（2026-09-21 首跑即栽此）。
if ! dnf -q --repo=puxian-verify list --available puxian 2>&1 | tee /tmp/oe_list.log | grep -q '^puxian'; then
    echo "❌ dnf list 看不到 puxian（仓库可见性检查失败）"
    echo "── 诊断：repo 配置 ──"; cat /etc/yum.repos.d/puxian-verify.repo
    echo "── 诊断：repodata 实体 ──"; ls -l "$REPO_DIR/repodata" | head
    echo "── 诊断：makecache 复跑 ──"; dnf -y makecache 2>&1 | tail -15
    exit 1
fi

echo "== [verify-oe] 真实安装 puxian（会自动拉 gcc 依赖）=="
if ! dnf -y install puxian 2>&1 | tail -12; then
    echo "❌ 步骤失败: dnf install puxian（依赖解析/下载）" >&2
    exit 1
fi
rpm -q puxian
/usr/bin/pxc --version

echo "== [verify-oe] 入库件静态性自检（ldd；动态件会在老 glibc 上崩）=="
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

echo "== [verify-oe] 真编译 + 真运行（默认 VM 轨 —— 本步才照得到『装得上跑不起来』）=="
mkdir -p /tmp/pxt && cd /tmp/pxt
printf 'print("openeuler-pkg-ok")\n' > hello.px
/usr/bin/pxc build hello.px
[ -x /tmp/pxt/build/hello ] || { echo "❌ 缺产物 /tmp/pxt/build/hello"; ls -lR /tmp/pxt; exit 1; }
GOT="$(/tmp/pxt/build/hello)"
[ "$GOT" = "openeuler-pkg-ok" ] || { echo "❌ 产物运行输出异常：[$GOT]"; exit 1; }
echo "   编译产物运行 OK：$GOT"

echo "== [verify-oe] 解释轨（px run）=="
/usr/bin/pxc run hello.px | grep -q openeuler-pkg-ok || { echo "❌ px run 失败"; exit 1; }

echo "✅ openEuler $OE_VER 正式签名仓库：dnf 双验签 + 安装 + 真编译运行 全部通过"
