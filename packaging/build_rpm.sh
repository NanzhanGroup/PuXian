#!/usr/bin/env bash
# ============================================================
# packaging/build_rpm.sh —— PuXian RPM 构建 + 签名 + 仓库组包（M73/M77/M78）
# ------------------------------------------------------------
# 全链路（dnf/yum 一行安装的前置产物）：
#   make_release.sh 打发布 tarball → rpmbuild -bb → rpm --addsign 包签名
#   → createrepo(_c) 组 yum/dnf 仓库 → gpg --detach-sign repomd.xml 元数据签名
#   → 导出公钥 PUXIAN-GPG-KEY.asc → rpm -K 自检
# 输出（REPO_OUT，缺省 /tmp/pxrepo）：
#   <REPO_OUT>/<dist>/<arch>/puxian-...rpm + repodata/repomd.xml(.asc)
#   <REPO_OUT>/PUXIAN-GPG-KEY.asc
# 兼容：el7（gpg2.0/createrepo-python）与 el8/9（gpg2.x/createrepo_c）双跑，
#       RPM Release 后缀自动带 .el<dist>（CI 显式传 DIST=7/9）。
# 签名密钥（两种模式）：
#   demo（缺省）：自动取本机唯一签名密钥 —— 仅链路验证，禁止正式发布
#   release：    env GPG_KEY_ID=<指纹> 显式指定（正式主/子密钥，见 packaging/README）
# 无签名构建：SKIP_SIGN=1 → 跳过密钥解析/addsign/repomd 签名/公钥导出/rpm -K，
#      仅产出未签名 rpm + 仓库元数据。M78 起 el7 在 centos:7 内无签名构建，
#      正式签名由 el9 侧 rpm4.16（loopback）代签 —— 规避 el7 rpm4.11/gpg2.0
#      无 --pinentry-mode loopback 的 headless 签名缺陷。
# 用法：
#   packaging/build_rpm.sh                            # demo 签名（本地验证链路）
#   GPG_KEY_ID=ABC123... packaging/build_rpm.sh       # 指定签名密钥（CI 用）
#   SKIP_SIGN=1 packaging/build_rpm.sh                # 仅构建不打签名（el7 预构建）
# 环境：
#   REPO_OUT   仓库输出目录（缺省 /tmp/pxrepo）
#   RPM_TOP    rpmbuild 顶层（缺省 $HOME/rpmbuild）
#   DIST       目录 = releasever 纯数字（缺省取 rpm -E %{?dist}，如 7/9）
#   GPG_PASSPHRASE  签名私钥口令（有则 --passphrase-file 无 tty 签名）
# ============================================================
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

REPO_OUT="${REPO_OUT:-/tmp/pxrepo}"
RPM_TOP="${RPM_TOP:-$HOME/rpmbuild}"
GPG_KEY_ID="${GPG_KEY_ID:-}"
DIST="${DIST:-$(rpm -E %{?dist} 2>/dev/null | grep -o '[0-9]*' || echo 9)}"
DL="${DIST//[^0-9]/}"          # 目录名里取纯数字（DIST=el9 → 9）

# ---- gpg 二进制自适应（el7 若只装 gnupg2 则无 /usr/bin/gpg → 用 gpg2）----
# --pinentry-mode loopback 仅 gpg 2.1+；gpg2.0 无此选项 → 空，靠 --passphrase-file
# SKIP_SIGN=1（纯构建不打签名）时不要求 gpg 存在
GPG_BIN="$(command -v gpg || command -v gpg2 || true)"
_LOOPBACK=""
if [ -n "$GPG_BIN" ] && "$GPG_BIN" --batch --pinentry-mode loopback --version >/dev/null 2>&1; then
    _LOOPBACK="--pinentry-mode loopback"
fi
if [ "${SKIP_SIGN:-0}" != "1" ] && [ -z "$GPG_BIN" ]; then
    echo "❌ 未找到 gpg/gpg2"; exit 1
fi

# ---- 版本自 tag 派生（与 make_release.sh 同构）----
TAG="$(git describe --tags --abbrev=0 2>/dev/null || echo v0.1.0-m72)"
TVER="${TAG#v}"                        # 0.1.0-m72
VER="${TVER%%-*}"                      # 0.1.0
MILESTONE="${TVER#*-}"                 # m72
SHA="$(git rev-parse --short HEAD)"    # 2e6ac8d
[ "$VER" = "0.1.0" ] || { echo "❌ spec 固定 Version 0.1.0，tag=$TAG 不一致"; exit 1; }
echo "== RPM 构建: puxian-$VER-$MILESTONE-$SHA (dist=$DL, repo_dir=$DIST) =="

# ---- 1) 发布 tarball（make_release.sh --no-check，打包不冒烟）----
chmod +x tools/pxc tools/make_release.sh bootstrap/pxc bootstrap/pxi 2>/dev/null || true
TARBALL="$(bash tools/make_release.sh --no-check 2>&1 | grep -o '/tmp/puxian-[^ ]*\.tar\.gz' | head -1)"
TARBALL="${TARBALL:-$(ls -t /tmp/puxian-$VER-$MILESTONE-*.tar.gz | head -1)}"
[ -f "$TARBALL" ] || { echo "❌ tarball 生成失败"; exit 1; }
echo "   tarball: $TARBALL ($(du -h "$TARBALL" | cut -f1))"

# ---- 2) rpmbuild -bb（Release 后缀带 .el<dist>，与仓库目录 dist 对齐）----
mkdir -p "$RPM_TOP"/{SPECS,SOURCES,RPMS,SRPMS,BUILD,BUILDROOT}
cp "$TARBALL" "$RPM_TOP/SOURCES/$(basename "$TARBALL")"
RPM_ARGS=(--define "_topdir $RPM_TOP" --define "pxtag $MILESTONE" --define "pxsha $SHA")
[ -n "$DL" ] && RPM_ARGS+=(--define "dist .el$DL")
rpmbuild -bb "${RPM_ARGS[@]}" packaging/puxian.spec
RPM="$(ls -t "$RPM_TOP"/RPMS/*/puxian-$VER-1.$MILESTONE*.rpm | head -1)"
[ -f "$RPM" ] || { echo "❌ rpm 构建失败"; exit 1; }
echo "   rpm: $RPM ($(du -h "$RPM" | cut -f1))"

# ---- 3) 组仓库目录 + 包签名（SKIP_SIGN=1 → 仅产出未签名仓库）----
mkdir -p "$REPO_OUT/$DIST/$(uname -m)"
cp "$RPM" "$REPO_OUT/$DIST/$(uname -m)/"
RPK="$(ls "$REPO_OUT/$DIST/$(uname -m)/"puxian-*.rpm | head -1)"

GPG_KEY="${GPG_KEY_ID:-}"
_PASSFILE=""
if [ "${SKIP_SIGN:-0}" = "1" ]; then
    echo "== 包签名: SKIP_SIGN=1 —— 跳过签名（未签名 rpm，待外部 el9 侧代签）=="
else
    # 签名 key 解析：release 用 GPG_KEY_ID（指纹），demo 自动取本机唯一签名密钥指纹
    if [ -z "$GPG_KEY" ]; then
        GPG_KEY="$("$GPG_BIN" --list-secret-keys --with-colons 2>/dev/null | awk -F: '/^(pub|sec)/{print $5; exit}')"
        if [ -z "$GPG_KEY" ]; then
            echo "❌ 本机无签名密钥：请 gpg --full-generate-key 生成，或用 GPG_KEY_ID=<指纹> 显式指定"
            exit 1
        fi
        echo "== 包签名 (demo key: $GPG_KEY) =="
    else
        echo "== 包签名 (release key: $GPG_KEY) =="
    fi

    # 带口令私钥在无 tty 下 rpm --addsign 拿不到口令 → 覆写 __gpg_sign_cmd：
    #   headless 标准做法 —— gpg2.1+ 加 --pinentry-mode loopback；gpg2.0 无 loopback
    #   仅靠 --passphrase-file（el7 CI 已切 el9 侧 rpm4.16 代签，此处主要服务 el8/9）。
    MACROS_FILE="$HOME/.rpmmacros"
    _GPG_SIGN_CMD=""
    SIGN_ARGS=(--define "_gpg_name $GPG_KEY")
    if [ -n "${GPG_PASSPHRASE:-}" ]; then
        _PASSFILE="$RPM_TOP/.gpg-passfile.$$"
        printf '%s' "$GPG_PASSPHRASE" > "$_PASSFILE"
        chmod 600 "$_PASSFILE"
        _GPG_SIGN_CMD="%{__gpg} gpg --batch --yes $_LOOPBACK --passphrase-file $_PASSFILE --no-armor --no-secmem-warning -u %{_gpg_name} -sbo %{__signature_filename} %{__plaintext_filename}"
        SIGN_ARGS+=(--define "__gpg_sign_cmd $_GPG_SIGN_CMD")
    fi
    command -v rpmsign >/dev/null 2>&1 || { echo "❌ 未找到 rpmsign：请先安装 rpm-sign（dnf/yum install -y rpm-sign）"; exit 1; }
    # ~/.rpmmacros 双保险（rpmsign 兼容通道）
    {
        echo "%_gpg_name $GPG_KEY"
        [ -x /usr/bin/gpg ] || echo "%__gpg $GPG_BIN"
        [ -n "$_GPG_SIGN_CMD" ] && echo "%__gpg_sign_cmd $_GPG_SIGN_CMD"
    } > "$MACROS_FILE"
    chmod 600 "$MACROS_FILE"
    rpm "${SIGN_ARGS[@]}" --addsign "$RPK"
    rm -f "$MACROS_FILE"
fi

# ---- 4) 仓库元数据 + repomd.xml 签名 + 公钥导出 ----
# createrepo_c（el8/9）与 createrepo（el7 python）自适应；el7 侧无需签名场景。
CREATEREPO_BIN="$(command -v createrepo_c || command -v createrepo || true)"
[ -n "$CREATEREPO_BIN" ] || { echo "❌ 未找到 createrepo_c/createrepo"; exit 1; }
echo "== $CREATEREPO_BIN =="
"$CREATEREPO_BIN" "$REPO_OUT/$DIST/$(uname -m)/" >/dev/null

if [ "${SKIP_SIGN:-0}" != "1" ]; then
    echo "== repomd.xml 签名 =="
    REPO_SIGN=("$GPG_BIN" --batch --yes --armor --detach-sign --local-user "$GPG_KEY")
    [ -n "$_PASSFILE" ] && REPO_SIGN+=(--passphrase-file "$_PASSFILE")
    "${REPO_SIGN[@]}" -o "$REPO_OUT/$DIST/$(uname -m)/repodata/repomd.xml.asc" \
        "$REPO_OUT/$DIST/$(uname -m)/repodata/repomd.xml"
    echo "== 公钥导出 =="
    "$GPG_BIN" --batch --armor --export "$GPG_KEY" > "$REPO_OUT/PUXIAN-GPG-KEY.asc"
fi
[ -n "$_PASSFILE" ] && rm -f "$_PASSFILE"

# ---- 5) 自检 ----
if [ "${SKIP_SIGN:-0}" = "1" ]; then
    echo "✅ 未签名仓库就绪: $REPO_OUT/$DIST/$(uname -m)（rpm 未签名，待代签后重建元数据）"
else
    echo "== rpm 导入公钥（rpm -Kv 需 rpm 密钥库内有公钥，否则 NOKEY 误报失败）=="
    rpm --import "$REPO_OUT/PUXIAN-GPG-KEY.asc"
    echo "== rpm -Kv 自检 =="
    rpm -Kv "$RPK" | tail -6
    echo
    echo "✅ 仓库就绪: $REPO_OUT"
    echo "   包:       $RPK"
    echo "   baseurl:  file://$REPO_OUT/$DIST/$(uname -m)/"
    echo "   gpgkey:   file://$REPO_OUT/PUXIAN-GPG-KEY.asc"
    echo "   （正式托管后替换 file:// → https://nanzhangroup.github.io/PuXian/rpm/...）"
fi
