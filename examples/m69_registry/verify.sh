#!/usr/bin/env bash
# ============================================================
# M69-S3 端到端：registry 官库资产 → pxpkg 拉取 → import 双模式
# ------------------------------------------------------------
# 验证链：
#   1. pxpkg init + add（semver/yaml/collections @^0.1.0）→ px.toml
#   2. install（PX_REGISTRY=<仓库>/registry）→ .px_modules/ + px.pkg.lock
#   3. main.px `import semver/yaml/collections`（裸名 → .px_modules）
#      pxi run 与 pxc build 双模式输出一致（stdlib 官库 registry 化消费）
#   4. install --locked 可复现；registry 不可用后 --locked 仍过（不重新解析）
# 运行：examples/m69_registry/verify.sh
# ============================================================
set -u
DEMO="$(cd "$(dirname "$0")" && pwd)"
PKG="$(cd "${DEMO}/../../tools" && pwd)/pxpkg"
PXI="$(cd "${DEMO}/../../bootstrap" && pwd)/pxi"
PXC="$(cd "${DEMO}/../../tools" && pwd)/pxc"
REG="$(cd "${DEMO}/../../registry" && pwd)"
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

echo "=== M69-S3 registry 官库拉取闭环（import 双模式）==="
rm -rf "$APP"; mkdir -p "$APP"; cd "$APP"
export PX_REGISTRY="$REG"

# 1. init + add 三个官库
"$PKG" init --name ecoapp >/dev/null 2>&1
"$PKG" add semver@^0.1.0 >/dev/null 2>&1
"$PKG" add yaml@^0.1.0 >/dev/null 2>&1
"$PKG" add collections@^0.1.0 >/dev/null 2>&1
chk "$(grep -c 'semver\|yaml\|collections' px.toml)" "3" "px.toml 记录 3 个 registry 依赖"

# 2. install
"$PKG" install >/dev/null 2>&1
chk "$(test -f .px_modules/semver/semver.px && echo yes)" "yes" "semver 已安装 (.px_modules)"
chk "$(test -f .px_modules/yaml/yaml.px && echo yes)" "yes" "yaml 已安装"
chk "$(test -f .px_modules/collections/collections.px && echo yes)" "yes" "collections 已安装"
chk "$(test -f px.pkg.lock && echo yes)" "yes" "px.pkg.lock 已生成"
LOCK_V=$(grep -o '"semver":{[^}]*"version":"[^"]*"' px.pkg.lock | grep -o '0\.1\.0')
chk "$LOCK_V" "0.1.0" "lock 锁定 semver=0.1.0"

# 3. main.px：裸名 import（registry 包）→ 双模式一致
cat > main.px <<'EOF'
import semver
import yaml
import collections
var sv = sv_parse("1.2.3-alpha.1+b5")
assert(sv["major"] == 1 and sv["pre"] == "alpha.1", "sv_parse")
assert(sv_satisfies("1.2.5", "^1.2.0") == true, "sv_satisfies")
var ya = yaml_parse("name: eco\nver: 1\n")
assert(ya["ok"] == true, "yaml_parse")
var u = unique([3, 1, 3, 2])
assert(len(u) == 3, "unique")
print("M69-S3 import OK: semver + yaml + collections (registry 0.1.0)")
EOF

OUT=$("$PXI" run main.px 2>&1)
chk "$(echo "$OUT" | grep -c 'M69-S3 import OK')" "1" "pxi run：import registry 包可用"
if echo "$OUT" | grep -q "M69-S3 import OK"; then
    PASS=$((PASS+1)); echo "[PASS] pxi 输出正确"
else
    FAIL=$((FAIL+1)); echo "[FAIL] pxi 输出异常：$OUT"
fi

build_out=$("$PXC" build main.px 2>&1)
if echo "$build_out" | grep -q "编译成功"; then
    RUNOUT=$(./build/main 2>&1)
    chk "$(echo "$RUNOUT" | grep -c 'M69-S3 import OK')" "1" "pxc build 运行：import registry 包可用"
else
    FAIL=$((FAIL+1)); echo "[FAIL] pxc build 失败：$build_out"
fi

# 4. --locked 可复现 + registry 不可用仍可复现
OUT=$("$PKG" install --locked 2>&1)
chk "$(echo "$OUT" | grep -c '可复现')" "1" "install --locked 可复现构建确认"
export PX_REGISTRY="${DEMO}/registry_empty"
mkdir -p "$PX_REGISTRY"
OUT=$("$PKG" install --locked 2>&1)
chk "$(echo "$OUT" | grep -c '可复现')" "1" "registry 不可用后 --locked 仍可复现（不重新解析）"
unset PX_REGISTRY

echo ""
echo "========== M69-S3 汇总 =========="
echo "PASS: $PASS  FAIL: $FAIL"
[ "$FAIL" = "0" ] && echo "M69-S3 registry 拉取闭环全部通过 ✅" || echo "存在失败 ❌"
exit $FAIL
