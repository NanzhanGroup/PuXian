#!/usr/bin/env bash
# ============================================================
# M45 端到端验证：registry 版本化（semver + lockfile）
# ------------------------------------------------------------
# 验证链：
#   1. pxpkg init + add（name@range / 精确 / 本地路径）→ px.toml
#   2. pxpkg install（PX_REGISTRY 本地目录）→ semver 选最高版本 → 写 px.pkg.lock
#   3. 断言：mylib 装的是 1.2.5（^1.2.0 最高）且 lock sha256 与实际一致
#   4. install --locked 二次安装一致（可复现构建）
#   5. 篡改 .px_modules 内容 → install --locked 拒绝（sha256 校验）
#   6. 恢复后 --locked 通过；registry 删除也不影响（已锁定，不重新解析）
# 运行：examples/m45_pkgdemo/verify.sh
# ============================================================
set -u
DEMO="$(cd "$(dirname "$0")" && pwd)"
PKG="${DEMO}/../../tools/pxpkg"
REG="${DEMO}/registry"
APP="${DEMO}/app"
PASS=0
FAIL=0

chk() {
    if [ "$1" = "$2" ]; then
        PASS=$((PASS+1)); echo "[PASS] $3"
    else
        FAIL=$((FAIL+1)); echo "[FAIL] $3 （期望 '$2' 实际 '$1'）"
    fi
}

echo "=== M45 registry 版本化端到端 ==="
rm -rf "$APP"; mkdir -p "$APP"; cd "$APP"
export PX_REGISTRY="$REG"
PKG_TOOL="$PKG"

# 1. init + add
$PKG_TOOL init --name demoapp >/dev/null 2>&1
$PKG_TOOL add mylib@^1.2.0 >/dev/null 2>&1
$PKG_TOOL add other@0.1.0 >/dev/null 2>&1
echo 'def local_helper(): return "local"' > "${DEMO}/local_helper.px"
$PKG_TOOL add "${DEMO}/local_helper.px" --name helper >/dev/null 2>&1
chk "$(grep -c 'mylib\|other\|helper' px.toml)" "3" "px.toml 记录 3 个依赖"

# 2. install
$PKG_TOOL install >/dev/null 2>&1
chk "$(grep -c '1.2.5' .px_modules/mylib/mylib.px 2>/dev/null)" "1" "mylib 安装的是 1.2.5（^1.2.0 最高版本）"
chk "$(test -f .px_modules/other/other.px && echo yes)" "yes" "other 精确 0.1.0 已安装"
chk "$(test -f .px_modules/helper/helper.px && echo yes)" "yes" "helper 本地路径已安装"
chk "$(test -f px.pkg.lock && echo yes)" "yes" "px.pkg.lock 已生成"

# 3. lock 断言：mylib version=1.2.5 + sha256 与内容一致
LOCK_V=$(grep -o '"mylib":{[^}]*"version":"[^"]*"' px.pkg.lock | grep -o '1\.2\.5')
chk "$LOCK_V" "1.2.5" "lock 锁定 mylib=1.2.5"
LOCK_SHA=$(grep -o '"mylib":{[^}]*"sha256":"[^"]*"' px.pkg.lock | grep -o '[0-9a-f]\{64\}')
ACT_SHA=$(sha256sum .px_modules/mylib/mylib.px | cut -d' ' -f1)
chk "$ACT_SHA" "$LOCK_SHA" "lock sha256 == 安装内容哈希"

# 4. --locked 二次安装一致（幂等 + 校验）
OUT=$($PKG_TOOL install --locked 2>&1)
chk "$(echo "$OUT" | grep -c '\[locked\]')" "3" "--locked 三个依赖全部校验通过"
chk "$(echo "$OUT" | grep -c '可复现')" "1" "--locked 可复现构建确认"

# 5. 篡改检测
echo 'def tampered(): return "hacked"' > .px_modules/mylib/mylib.px
OUT=$($PKG_TOOL install --locked 2>&1)
chk "$(echo "$OUT" | grep -c '被篡改')" "1" "--locked 拒绝篡改内容（sha256 不符）"

# 6. 恢复 + registry 删除后仍可复现（已锁定，不重新解析）
cp "${REG}/mylib/1.2.5/mylib.px" .px_modules/mylib/mylib.px
OUT=$($PKG_TOOL install --locked 2>&1)
chk "$(echo "$OUT" | grep -c '可复现')" "1" "恢复后 --locked 通过"
# 模拟 registry 不可用：换一个空 registry 目录
export PX_REGISTRY="${DEMO}/registry_empty"
mkdir -p "$PX_REGISTRY"
OUT=$($PKG_TOOL install --locked 2>&1)
chk "$(echo "$OUT" | grep -c '可复现')" "1" "registry 删除后 --locked 仍可复现（不重新解析）"
unset PX_REGISTRY

# 7. list 展示
OUT=$($PKG_TOOL list 2>&1)
chk "$(echo "$OUT" | grep -c '已安装')" "3" "list 显示 3 个依赖已安装"

echo ""
echo "========== M45 端到端汇总 =========="
echo "PASS: $PASS  FAIL: $FAIL"
[ "$FAIL" = "0" ] && echo "M45 端到端全部通过 ✅" || echo "存在失败 ❌"
exit $FAIL
