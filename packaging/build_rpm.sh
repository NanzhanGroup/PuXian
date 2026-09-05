#!/usr/bin/env bash
# ============================================================
# packaging/build_rpm.sh —— PuXian RPM 构建 + 签名 + 仓库组包（M73）
# ------------------------------------------------------------
# 全链路（dnf/yum 一行安装的前置产物）：
#   make_release.sh 打发布 tarball → rpmbuild -bb → rpm --addsign 包签名
#   → createrepo_c 组 yum/dnf 仓库 → gpg --detach-sign repomd.xml 元数据签名
#   → 导出公钥 PUXIAN-GPG-KEY.asc → rpm -K 自检
# 输出（REPO_OUT，缺省 /tmp/pxrepo）：
#   <REPO_OUT>/<arch>/<puxian-...>.rpm
#   <REPO_OUT>/<arch>/repodata/repomd.xml(.asc)
#   <REPO_OUT>/PUXIAN-GPG-KEY.asc
# 签名密钥（两种模式）：
#   demo（缺省）：自动取本机唯一签名密钥 —— 仅链路验证，禁止正式发布
#   release：    env GPG_KEY_ID=<指纹> 显式指定（正式主/子密钥，见 packaging/README）
# 用法：
#   packaging/build_rpm.sh                  # demo 签名（本地验证链路）
#   GPG_KEY_ID=ABC123... packaging/build_rpm.sh   # 指定签名密钥（CI 用）
# 环境：
#   REPO_OUT   仓库输出目录（缺省 /tmp/pxrepo）
#   RPM_TOP    rpmbuild 顶层（缺省 $HOME/rpmbuild）
# ============================================================
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

REPO_OUT="${REPO_OUT:-/tmp/pxrepo}"
RPM_TOP="${RPM_TOP:-$HOME/rpmbuild}"
GPG_KEY_ID="${GPG_KEY_ID:-}"
DIST="${DIST:-el$(rpm -E %{?dist} 2>/dev/null | grep -o '[0-9]*' || echo 9)}"

# ---- 版本自 tag 派生（与 make_release.sh 同构）----
TAG="$(git describe --tags --abbrev=0 2>/dev/null || echo v0.1.0-m72)"
TVER="${TAG#v}"                        # 0.1.0-m72
VER="${TVER%%-*}"                      # 0.1.0
MILESTONE="${TVER#*-}"                 # m72
SHA="$(git rev-parse --short HEAD)"    # 2e6ac8d
[ "$VER" = "0.1.0" ] || { echo "❌ spec 固定 Version 0.1.0，tag=$TAG 不一致"; exit 1; }
echo "== RPM 构建: puxian-$VER-$MILESTONE-$SHA (dist=$DIST) =="

# ---- 1) 发布 tarball（make_release.sh 默认输出 /tmp/puxian-<ver>-<ms>-<sha>.tar.gz）----
chmod +x tools/pxc tools/make_release.sh bootstrap/pxc bootstrap/pxi
TARBALL="$(bash tools/make_release.sh --no-check 2>&1 | tee /dev/stderr | grep -o '/tmp/puxian-[^ ]*\.tar\.gz' | head -1)"
TARBALL="${TARBALL:-$(ls -t /tmp/puxian-$VER-$MILESTONE-*.tar.gz | head -1)}"
[ -f "$TARBALL" ] || { echo "❌ tarball 生成失败"; exit 1; }
echo "   tarball: $TARBALL ($(du -h "$TARBALL" | cut -f1))"

# ---- 2) rpmbuild -bb ----
mkdir -p "$RPM_TOP"/{SPECS,SOURCES,RPMS,SRPMS,BUILD,BUILDROOT}
cp "$TARBALL" "$RPM_TOP/SOURCES/$(basename "$TARBALL")"
rpmbuild -bb \
    --define "_topdir $RPM_TOP" \
    --define "pxtag $MILESTONE" \
    --define "pxsha $SHA" \
    packaging/puxian.spec
RPM="$(ls -t "$RPM_TOP"/RPMS/*/puxian-$VER-1.$MILESTONE*.rpm | head -1)"
[ -f "$RPM" ] || { echo "❌ rpm 构建失败"; exit 1; }
echo "   rpm: $RPM ($(du -h "$RPM" | cut -f1))"

# ---- 3) 组仓库目录 + 包签名 ----
mkdir -p "$REPO_OUT/$DIST/$(uname -m)"
cp "$RPM" "$REPO_OUT/$DIST/$(uname -m)/"
RPK="$(ls "$REPO_OUT/$DIST/$(uname -m)/"puxian-*.rpm | head -1)"

# 签名 key 解析：release 用 GPG_KEY_ID（指纹），demo 自动取本机唯一签名密钥指纹
if [ -n "$GPG_KEY_ID" ]; then
    GPG_KEY="$GPG_KEY_ID"
    echo "== 包签名 (release key: $GPG_KEY) =="
else
    GPG_KEY="$(gpg --list-secret-keys --with-colons 2>/dev/null | awk -F: '/^(pub|sec)/{print $5; exit}')"
    if [ -z "$GPG_KEY" ]; then
        echo "❌ 本机无签名密钥：请 gpg --full-generate-key 生成，或用 GPG_KEY_ID=<指纹> 显式指定"
        exit 1
    fi
    echo "== 包签名 (demo key: $GPG_KEY) =="
fi
rpm --define "_gpg_name $GPG_KEY" --addsign "$RPK"

# ---- 4) 仓库元数据 + repomd.xml 签名 + 公钥导出 ----
echo "== createrepo_c =="
createrepo_c --pretty "$REPO_OUT/$DIST/$(uname -m)/" >/dev/null
echo "== repomd.xml 签名 =="
gpg --batch --yes --armor --detach-sign --local-user "$GPG_KEY" \
    -o "$REPO_OUT/$DIST/$(uname -m)/repodata/repomd.xml.asc" \
    "$REPO_OUT/$DIST/$(uname -m)/repodata/repomd.xml"
echo "== 公钥导出 =="
gpg --batch --armor --export "$GPG_KEY" > "$REPO_OUT/PUXIAN-GPG-KEY.asc"

# ---- 5) 自检 ----
echo "== rpm -Kv 自检 =="
rpm -Kv "$RPK" | tail -3
echo
echo "✅ 仓库就绪: $REPO_OUT"
echo "   包:       $RPK"
echo "   baseurl:  file://$REPO_OUT/$DIST/$(uname -m)/"
echo "   gpgkey:   file://$REPO_OUT/PUXIAN-GPG-KEY.asc"
echo "   （正式托管后替换 file:// → https://nanzhangroup.github.io/PuXian/rpm/...）"
