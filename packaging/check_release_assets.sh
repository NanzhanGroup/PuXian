#!/usr/bin/env bash
# ============================================================
# packaging/check_release_assets.sh —— 发布资产 ⇄ 校验清单 **双向核对**（M311 · C1/C2）
# ------------------------------------------------------------
# 为什么有它（M310 D5 实测）：
#   发布物里有两个 tarball（主包 + aarch64 引导包），而 `sha256sums.txt` 由
#   `tools/make_release.sh` 只写**主包**一行；aarch64 包只有同名 sidecar `.sha256`。
#   ⇒ 下载者用「一个清单核全部资产」时**核不到 aarch64 包**，而两套口径并存时，
#     「某个资产到底有没有被校验」这件事**没有任何判据看得见**（M277 的同族：
#     当时是 aarch64 包根本没进镜像，也无人告警）。
#
# 判据（双向，缺一不可）：
#   A1 目录里**每个** `*.tar.gz` 都必须在 `sha256sums.txt` 里，且**重算的 sha256 与清单一致**；
#   A2 清单里**每一行**都必须对应目录里存在的文件（悬空条目 ⇒ 红）；
#   A3 若存在同名 sidecar（`<tarball>.sha256`），其哈希必须与清单条目**一致**（两套口径不许互相矛盾）；
#   A4 aarch64 引导包**必须存在**（M277：它曾长期没进镜像而无人知）；
#   A5 规模锚点：tarball ≥2 · 清单行 ≥2（防「空集 ⊇ 空集」的假绿）。
#
# 用法：
#   packaging/check_release_assets.sh <含发布资产的目录> <tag>
#   packaging/check_release_assets.sh --self-test      # 离线自证（5 条，含负控）
# 退出码：0 绿 / 1 有违例 / 2 用法错 / 3 前提不成立（目录不存在 / 无清单）
# ============================================================
set -uo pipefail

usage() { sed -n '2,30p' "$0" | sed 's/^# \{0,1\}//'; }

selftest() {
    local W; W="$(mktemp -d /tmp/m311ra.XXXXXX)" || exit 3
    local p=0 f=0
    sok(){ p=$((p+1)); echo "  ✅ $*"; }
    sno(){ f=$((f+1)); echo "  ❌ $*"; }

    mkdir -p "$W/good"
    printf 'main content\n' > "$W/good/puxian-1.0.0-abc.tar.gz"
    printf 'arm content\n'  > "$W/good/puxian-bootstrap-aarch64-v1.0.0.tar.gz"
    ( cd "$W/good" && sha256sum puxian-1.0.0-abc.tar.gz puxian-bootstrap-aarch64-v1.0.0.tar.gz > sha256sums.txt )
    sha256sum "$W/good/puxian-bootstrap-aarch64-v1.0.0.tar.gz" | awk '{print $1"  puxian-bootstrap-aarch64-v1.0.0.tar.gz"}' > "$W/good/puxian-bootstrap-aarch64-v1.0.0.tar.gz.sha256"

    if bash "$0" "$W/good" v1.0.0 > "$W/s1.log" 2>&1; then sok "S1 完整资产 ⇒ 绿"
    else sno "S1 完整资产应绿：$(grep '❌' "$W/s1.log" | head -1)"; fi

    # S2 资产不在清单里（= 本轮 C1 的缺陷形态）
    cp -r "$W/good" "$W/miss"
    printf 'extra\n' > "$W/miss/puxian-bootstrap-aarch64-extra.tar.gz"
    if bash "$0" "$W/miss" v1.0.0 > "$W/s2.log" 2>&1; then sno "S2 清单漏资产应判红"
    else grep -q '不在 sha256sums.txt' "$W/s2.log" && sok "S2 清单漏资产判红且指名" || sno "S2 红但没指名"; fi

    # S3 清单哈希与实际不符
    cp -r "$W/good" "$W/badhash"
    printf 'tampered\n' >> "$W/badhash/puxian-1.0.0-abc.tar.gz"
    if bash "$0" "$W/badhash" v1.0.0 > "$W/s3.log" 2>&1; then sno "S3 哈希不符应判红"
    else grep -q 'sha256 与清单不一致' "$W/s3.log" && sok "S3 哈希不符判红且指名" || sno "S3 红但没指名"; fi

    # S4 sidecar 与清单矛盾
    cp -r "$W/good" "$W/badside"
    printf 'deadbeef  puxian-bootstrap-aarch64-v1.0.0.tar.gz\n' > "$W/badside/puxian-bootstrap-aarch64-v1.0.0.tar.gz.sha256"
    if bash "$0" "$W/badside" v1.0.0 > "$W/s4.log" 2>&1; then sno "S4 sidecar 矛盾应判红"
    else grep -q 'sidecar' "$W/s4.log" && sok "S4 sidecar 与清单矛盾判红" || sno "S4 红但没提 sidecar"; fi

    # S5 缺 aarch64 引导包（M277 形态）+ 判据自伤：拿掉该判据后必须绿吗？不 —— 这里只证它会红
    cp -r "$W/good" "$W/noarm"
    rm -f "$W/noarm"/puxian-bootstrap-aarch64-*
    ( cd "$W/noarm" && sha256sum puxian-1.0.0-abc.tar.gz > sha256sums.txt )
    if bash "$0" "$W/noarm" v1.0.0 > "$W/s5.log" 2>&1; then sno "S5 缺 aarch64 包应判红"
    else grep -q 'aarch64' "$W/s5.log" && sok "S5 缺 aarch64 引导包判红" || sno "S5 红但没指名 aarch64"; fi

    echo "══ 自证：$p 通过 / $f 失败 ══"
    rm -rf "$W"
    [ "$f" = 0 ] || exit 1
    exit 0
}

[ "${1:-}" = "--self-test" ] && selftest
[ $# -ge 2 ] || { usage; exit 2; }
DIR="$1"; TAG="$2"
[ -d "$DIR" ] || { echo "❌ 资产目录不存在：$DIR" >&2; exit 3; }
SUMS="$DIR/sha256sums.txt"
[ -f "$SUMS" ] || { echo "❌ 缺 sha256sums.txt（$SUMS）—— 没有清单就无从核对" >&2; exit 3; }

PASS=0; FAILN=0
ok(){ PASS=$((PASS+1)); echo "  ✅ $*"; }
bad(){ FAILN=$((FAILN+1)); echo "  ❌ $*"; }

echo "══ 发布资产核对（$TAG · $DIR）══"
NT=$(find "$DIR" -maxdepth 1 -name '*.tar.gz' | wc -l)
NL=$(grep -c . "$SUMS" || true)
echo "  目录 tarball=$NT · 清单行=$NL"

# A5 规模锚点
if [ "$NT" -ge 2 ] && [ "$NL" -ge 2 ]; then ok "规模锚点（tarball ≥2 · 清单行 ≥2）"
else bad "规模锚点不成立（tarball=$NT · 清单行=$NL）—— 提取器坏了或资产确实不全"; fi

# A1 每个 tarball 必须在清单里且哈希一致
while IFS= read -r f; do
    b="$(basename "$f")"
    exp="$(awk -v n="$b" '$2 == n || $2 == "*"n {print $1}' "$SUMS" | head -1)"
    if [ -z "$exp" ]; then
        bad "**$b** 不在 sha256sums.txt 里（下载者用清单核不到它）"
        continue
    fi
    got="$(sha256sum "$f" | awk '{print $1}')"
    if [ "$got" = "$exp" ]; then ok "$b 哈希与清单一致"
    else bad "**$b** sha256 与清单不一致（清单 $exp ⇄ 实际 $got）"; fi

    # A3 sidecar 与清单不许矛盾
    sc="$f.sha256"
    if [ -f "$sc" ]; then
        se="$(awk '{print $1}' "$sc" | head -1)"
        if [ "$se" = "$exp" ]; then ok "$b 的 sidecar 与清单一致"
        else bad "**$b** 的 sidecar（$se）与清单（$exp）不一致 —— 两套口径互相矛盾"; fi
    fi
done < <(find "$DIR" -maxdepth 1 -name '*.tar.gz' | sort)

# A2 清单每行都要有对应文件
while IFS= read -r line; do
    [ -n "$line" ] || continue
    n="$(printf '%s' "$line" | awk '{print $2}')"
    if [ ! -f "$DIR/$n" ]; then bad "清单条目**悬空**：$n 不存在于资产目录"; fi
done < "$SUMS"

# A4 aarch64 引导包必须存在
if find "$DIR" -maxdepth 1 -name 'puxian-bootstrap-aarch64-*.tar.gz' | grep -q .; then
    ok "aarch64 引导包在位（M277：它曾长期没进镜像而无人知）"
else
    bad "缺 aarch64 引导包（puxian-bootstrap-aarch64-*.tar.gz）"
fi

echo ""
echo "══ 汇总：$PASS 通过 / $FAILN 失败 ══"
[ "$FAILN" = 0 ] && { echo "RELEASE-ASSETS-OK"; exit 0; } || { echo "RELEASE-ASSETS-FAIL"; exit 1; }
