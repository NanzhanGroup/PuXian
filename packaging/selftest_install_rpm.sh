#!/usr/bin/env bash
# ============================================================
# packaging/selftest_install_rpm.sh —— install-rpm.sh 的离线矩阵自测（M168）
# ------------------------------------------------------------
# 存在理由（2026-09-21 用户报障）：
#   旧版 install-rpm.sh **没有**任何自测，而它的"发行版认脸"是一串写死的 case：
#   openEuler 直接 `❌ 暂支持 RHEL 系 7/9` 退出。这类"认脸失败"在**发布侧不可见**
#   （CI 只跑 el7/el9 容器），只有用户撞上来才知道。
#   ⇒ 本自测把「一台机器会去哪个仓库」变成**可断言的输出**：给定 os-release 夹具，
#     断言 rc、断言 baseurl（**含"不得出现 $releasever"这类反向判据**）、断言给不出
#     支持时必须同时给出替代路线（不许只报一句"暂支持"）。
# 特点：**离线**（PUXIAN_OFFLINE=1 + 注入 base）、**免 root**（只走 status/--dry-run）、
#   **不写 /etc**（--repo-file 指向临时目录）。可用于本机与 CI。
# ⚠️ 覆盖边界（**如实登记**）：本自测只看「脚本会把本机映射到哪个仓库」这条**决策链**；
#   「写的 .repo 能不能被真包管理器消费」（验签/装包/真编译）由**容器终验**覆盖：
#   `packaging/verify_repo_el7.sh`（centos:7 + yum3.4）与 `verify_repo_openeuler.sh`
#   （openeuler:22.03-lts / 24.03-lts + dnf）。二者都在 release.yml 里跑，缺一不可。
# 用法：bash packaging/selftest_install_rpm.sh
# 退出码：0 = 全过；1 = 有用例不符（逐条打印期望/实际）
# ============================================================
set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
SCRIPT="$HERE/install-rpm.sh"
[ -f "$SCRIPT" ] || { echo "❌ 缺 $SCRIPT"; exit 1; }

WORK="$(mktemp -d /tmp/px-installrpm-selftest.XXXXXX)"
trap 'rm -rf "$WORK"' EXIT

INJ_BASE="https://mirror.invalid/puxian/rpm"      # 注入 base：避免探测真实网络
pass=0; fail=0; ncase=0

# run <夹具> <动作/选项...>  → stdout 落 $WORK/out，stderr 合流，rc 落 $WORK/rc
run() {
    local fixture="$1"; shift
    PUXIAN_RPM_BASE="$INJ_BASE" PUXIAN_OFFLINE=1 PUXIAN_ARCH="${ARCH_OVERRIDE:-x86_64}" \
    bash "$SCRIPT" "$@" --os-release "$fixture" --repo-file "$WORK/puxian.repo" \
        > "$WORK/out" 2>&1
    echo $? > "$WORK/rc"
}
has()   { grep -qF -- "$1" "$WORK/out"; }
hasnt() { ! grep -qF -- "$1" "$WORK/out"; }
check() {   # $1=描述 $2=0/1（1=通过）
    ncase=$((ncase+1))
    if [ "$2" = 1 ]; then pass=$((pass+1)); printf '  ✅ %s\n' "$1"
    else fail=$((fail+1)); printf '  ❌ %s\n' "$1"; sed 's/^/       | /' "$WORK/out" | head -14; fi
}

mk() {   # 造夹具：mk <文件名> <ID> <VERSION_ID> <ID_LIKE>
    printf 'NAME="%s"\nID="%s"\nVERSION_ID="%s"\nID_LIKE="%s"\nPRETTY_NAME="%s %s"\n' \
        "$2" "$2" "$3" "$4" "$2" "$3" > "$WORK/$1"
}
mk os-rocky9.release   rocky      9.8  "rhel centos fedora"
mk os-centos7.release  centos     7.9  "rhel fedora"
mk os-oe2203.release   openEuler  22.03 ""
mk os-oe2403.release   openEuler  24.03 ""
mk os-oe2003.release   openEuler  20.03 ""
mk os-ol8.release      ol         8.9  "rhel fedora"
mk os-fedora.release   fedora     40   ""
mk os-ubuntu.release   ubuntu     24.04 "debian"
mk os-anolis9.release  anolis     9.2  "rhel centos fedora"
mk os-clone7.release   opencloudos 7.9 "rhel centos fedora"

echo "== 自测: install-rpm.sh 发行版矩阵（M168）=="
echo "   夹具目录 $WORK · 注入 base $INJ_BASE"

# ---------- 1) RHEL 系：保留 $releasever（既有用户路径不能改）----------
echo "── [1] RHEL 系（必须**保持** \$releasever 语义，不回归）"
run "$WORK/os-rocky9.release" --dry-run
check "el9：rc=0" "$([ "$(cat "$WORK/rc")" = 0 ] && echo 1 || echo 0)"
RELVER_PAT='$releasever/$basearch/'    # 单引号：保持字面量，勿展开
check "el9：baseurl 用 \$releasever/\$basearch" "$(has "baseurl=${INJ_BASE}/${RELVER_PAT}" && echo 1 || echo 0)"
check "el9：仓库目录报 9/x86_64" "$(has '仓库目录: 9/x86_64' && echo 1 || echo 0)"

run "$WORK/os-centos7.release" --dry-run
check "el7：rc=0" "$([ "$(cat "$WORK/rc")" = 0 ] && echo 1 || echo 0)"
check "el7：baseurl 用 \$releasever" "$(has "${RELVER_PAT}" && echo 1 || echo 0)"
check "el7：提示 yum 3.4/glibc 2.17" "$(has 'glibc 2.17' && echo 1 || echo 0)"

# ---------- 2) openEuler：本次报障的核心（必须认脸 + 字面目录）----------
echo "── [2] openEuler（M168 新增；核心判据 = **不得**出现 \$releasever）"
run "$WORK/os-oe2203.release" --dry-run
check "oe22.03：rc=0（旧版在此直接退出）" "$([ "$(cat "$WORK/rc")" = 0 ] && echo 1 || echo 0)"
check "oe22.03：baseurl = <base>/openeuler/22.03/x86_64/" \
      "$(has 'baseurl='"$INJ_BASE"'/openeuler/22.03/x86_64/' && echo 1 || echo 0)"
check "oe22.03：**不含** \$releasever（否则必然 404）" "$(hasnt '$releasever' && echo 1 || echo 0)"
check "oe22.03：name 里标出 openEuler 22.03" "$(has 'name=PuXian Repository (openEuler 22.03' && echo 1 || echo 0)"

run "$WORK/os-oe2403.release" --dry-run
check "oe24.03：rc=0" "$([ "$(cat "$WORK/rc")" = 0 ] && echo 1 || echo 0)"
check "oe24.03：目录 openeuler/24.03" "$(has '/openeuler/24.03/x86_64/' && echo 1 || echo 0)"

# ---------- 3) 不支持组合：必须**给出替代路线**，不许只报一句"暂支持" ----------
echo "── [3] 不支持组合（判据：rc≠0 **且** 同时给出可执行替代路线）"
for c in "os-oe2003.release:openEuler 20.03" "os-ol8.release:el8" "os-fedora.release:Fedora" "os-ubuntu.release:Ubuntu"; do
    fx="${c%%:*}"; label="${c#*:}"
    run "$WORK/$fx" --dry-run
    check "$label：rc≠0（明确不支持）" "$([ "$(cat "$WORK/rc")" != 0 ] && echo 1 || echo 0)"
    check "$label：说明原因（不是空话）" "$(has '原因：' && echo 1 || echo 0)"
    check "$label：给出 tarball 路线" "$(has '官方发布 tarball' && echo 1 || echo 0)"
    check "$label：给出支持矩阵" "$(has '当前支持的组合' && echo 1 || echo 0)"
done
run "$WORK/os-ol8.release" --dry-run
check "el8：明确指出**不能**混用 el9 仓库" "$(has '不能' && echo 1 || echo 0)"
run "$WORK/os-oe2003.release" --dry-run
check "oe20.03：点明 glibc 2.28 < 基线（装得上跑不起来）" "$(has 'glibc 2.28' && echo 1 || echo 0)"

# ---------- 4) 未验证的 EL 系克隆：放行但**必须**标注未验证 ----------
echo "── [4] EL 系克隆（anolis/opencloudos：放行 + 显式「未验证」标签）"
run "$WORK/os-anolis9.release" --dry-run
check "anolis 9：rc=0" "$([ "$(cat "$WORK/rc")" = 0 ] && echo 1 || echo 0)"
check "anolis 9：标注未验证" "$(has '未验证' && echo 1 || echo 0)"
run "$WORK/os-clone7.release" --dry-run
check "opencloudos 7：rc=0 且标注未验证" "$([ "$(cat "$WORK/rc")" = 0 ] && has '未验证' && echo 1 || echo 0)"

# ---------- 5) 架构维度：aarch64 无 rpm 仓库 ⇒ 指路引导包 ----------
echo "── [5] aarch64（无 rpm 仓库 ⇒ 必须指路 aarch64 引导包）"
ARCH_OVERRIDE=aarch64 run "$WORK/os-oe2203.release" --dry-run
check "aarch64+oe22.03：rc≠0" "$([ "$(cat "$WORK/rc")" != 0 ] && echo 1 || echo 0)"
check "aarch64：指路 puxian-bootstrap-aarch64 引导包" "$(has 'puxian-bootstrap-aarch64' && echo 1 || echo 0)"
check "aarch64：点明包内为静态件" "$(has '静态件' && echo 1 || echo 0)"
check "aarch64：同时给自举路线（native_bootstrap --portable）" "$(has 'native_bootstrap.sh --portable' && echo 1 || echo 0)"
ARCH_OVERRIDE=riscv64 run "$WORK/os-rocky9.release" --dry-run
check "riscv64：同样走替代路线（不假装支持）" "$([ "$(cat "$WORK/rc")" != 0 ] && echo 1 || echo 0)"

# ---------- 6) status / remove 的免 root 与安全性 ----------
echo "── [6] status/remove 与「不落盘」保证"
run "$WORK/os-oe2203.release" status
check "status：rc=0（免 root）" "$([ "$(cat "$WORK/rc")" = 0 ] && echo 1 || echo 0)"
check "status：报出 baseurl" "$(has 'baseurl: '"$INJ_BASE"'/openeuler/22.03/x86_64/' && echo 1 || echo 0)"
run "$WORK/os-rocky9.release" --dry-run
check "--dry-run：不写仓库文件" "$([ ! -e "$WORK/puxian.repo" ] && echo 1 || echo 0)"
run "$WORK/os-oe2203.release" --repo-file "$WORK/other.repo" --dry-run
check "未知参数被拒（-h 有指引）" "$(bash "$SCRIPT" --nope >/dev/null 2>&1; [ $? != 0 ] && echo 1 || echo 0)"

# ---------- 7) 负控：门必须能红（否则"全绿"不构成证据）----------
# 做法：把 install-rpm.sh 复制一份**改坏**，断言本门赖以判定的那条性质确实被破坏。
#   （本门不是"跑一遍看它没崩"，而是"能区分对与错"—— 故负控是它的核心证据。）
echo "── [7] 负控（把 openEuler 支持改坏 ⇒ 判据必须能区分）"
mut="$WORK/mut1.sh"
# 负控①：把 openEuler 22.03 改回「不支持」（= 用户报障时的旧行为）
sed 's#oe|literal|openeuler/22.03#other|literal||旧版行为：不支持#' "$SCRIPT" > "$mut"
PUXIAN_RPM_BASE="$INJ_BASE" PUXIAN_OFFLINE=1 bash "$mut" --dry-run \
    --os-release "$WORK/os-oe2203.release" --repo-file "$WORK/mut.repo" > "$WORK/mout" 2>&1
mrc=$?
cp "$WORK/mout" "$WORK/out"      # 让 has/hasnt 看**变体**的输出（勿沿用上一用例的缓存）
check "负控①：unsupported 变体 rc≠0（本门会判红）" "$([ "$mrc" != 0 ] && echo 1 || echo 0)"
check "负控①：变体里没有 openeuler/22.03 目录" "$(hasnt '/openeuler/22.03/' && echo 1 || echo 0)"

mut2="$WORK/mut2.sh"
# 负控②：把 openEuler 改回「靠 $releasever 展开」（= 404 的成因）⇒ 本门那条反向判据必须能抓
sed 's#oe|literal|openeuler/22.03#el|releasever|9#' "$SCRIPT" > "$mut2"
PUXIAN_RPM_BASE="$INJ_BASE" PUXIAN_OFFLINE=1 bash "$mut2" --dry-run \
    --os-release "$WORK/os-oe2203.release" --repo-file "$WORK/mut2.repo" > "$WORK/mout2" 2>&1
cp "$WORK/mout2" "$WORK/out"     # 让 has/hasnt 看这一份
check "负控②：\$releasever 变体**确实**出现 \$releasever（反向判据有区分力）" "$(has "${RELVER_PAT%/}" && echo 1 || echo 0)"

echo
echo "== 结果：用例 $ncase · 通过 $pass · 失败 $fail =="
[ "$fail" = 0 ] || exit 1
echo "SELFTEST-INSTALL-RPM-OK"
