#!/usr/bin/env bash
# ============================================================
# packaging/selftest_rpm_monotonic_guard.sh —— 单调性守卫的**离线**自测（qg-issue 41）
# ------------------------------------------------------------
# 不联网、不需 rpm、不碰 gh-pages：在 mktemp 目录里造"假仓库树"断言判据。
# 覆盖 ISSUE.md §5 验收判据 1（旧版本后发布 ⇒ 必须失败且给出明确原因）
# 与 2 的"正常新版仍成功"部分。
# 用法: bash packaging/selftest_rpm_monotonic_guard.sh
# 退出码: 0 = 全部通过；非 0 = 失败用例数
# ============================================================
set -uo pipefail

GUARD="$(cd "$(dirname "$0")" && pwd)/rpm_monotonic_guard.sh"
[ -f "$GUARD" ] || { echo "❌ 缺 $GUARD"; exit 1; }
bash -n "$GUARD" || { echo "❌ $GUARD 语法错误"; exit 1; }

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

pass=0; fail=0

# mk <树根> <rpm 文件名...> —— 依次放到 <树根>/7/x86_64、<树根>/9/x86_64、...
mk() {
    local root="$1"; shift
    local i=7
    for f in "$@"; do
        mkdir -p "$root/$i/x86_64"
        : > "$root/$i/x86_64/$f"
        i=$((i+2))
    done
}

# run <old> <new> [env...] → 回显 rc
run() {
    local old="$1" new="$2"; shift 2
    env "$@" bash "$GUARD" "$old" "$new" >"$TMP/out.txt" 2>&1
    echo $?
}

# check <用例名> <期望 rc> <实际 rc>
check() {
    if [ "$2" = "$3" ]; then
        echo "  ✅ $1（rc=$3）"; pass=$((pass+1))
    else
        echo "  ❌ $1：期望 rc=$2，实际 rc=$3"; sed 's/^/       | /' "$TMP/out.txt"; fail=$((fail+1))
    fi
}

# check_has <用例名> <模式>
check_has() {
    if grep -q -- "$2" "$TMP/out.txt"; then
        echo "  ✅ $1（输出含「$2」）"; pass=$((pass+1))
    else
        echo "  ❌ $1：输出未含「$2」"; sed 's/^/       | /' "$TMP/out.txt"; fail=$((fail+1))
    fi
}

echo "== 自测: rpm 发布单调性守卫 =="

# ---- 1. 无基线（gh-pages 尚无 rpm/，old 目录不存在）→ 允许 ----
mk "$TMP/n1" puxian-0.2.0-1.m112.el9.x86_64.rpm
check "1 无基线（old 目录不存在）⇒ 允许发布" 0 "$(run "$TMP/no-such" "$TMP/n1")"
check "1b 无基线（old 为空目录）⇒ 允许发布" 0 "$(run "$TMP/n1-empty" "$TMP/n1")"
check_has "1c 提示" "无基线"

# ---- 2. 新版 > 旧版（m112 > m111）→ 允许 ----
mk "$TMP/o2" puxian-0.2.0-1.m111.el9.x86_64.rpm
mk "$TMP/n2" puxian-0.2.0-1.m112.el9.x86_64.rpm
check "2 m112 > m111 ⇒ 允许发布" 0 "$(run "$TMP/o2" "$TMP/n2")"
check_has "2b 判词" "允许发布（单调不减）"

# ---- 3. 同版本重发 → 允许（幂等）----
mk "$TMP/n3" puxian-0.2.0-1.m112.el9.x86_64.rpm
check "3 同版本重发 ⇒ 允许发布" 0 "$(run "$TMP/n2" "$TMP/n3")"
check_has "3b 判词" "同版本重发"

# ---- 4. ★核心：旧覆盖新（m110 vs m112）→ 拒绝 ----
mk "$TMP/n4" puxian-0.2.0-1.m110.el9.x86_64.rpm
check "4 旧覆盖新 m110 < m112 ⇒ 拒绝发布" 1 "$(run "$TMP/n2" "$TMP/n4")"
check_has "4b 判词" "拒绝发布"
check_has "4c 给出明确原因" "0.2.0-1.m110"
check_has "4d 指出后果" "整体替换为旧版"

# ---- 5. 主版本回退（0.1.0-m999 vs 0.2.0-m100）→ 拒绝 ----
mk "$TMP/n5" puxian-0.1.0-1.m999.el9.x86_64.rpm
check "5 主版本回退 0.1.0-m999 < 0.2.0-m100 ⇒ 拒绝" 1 "$(run "$TMP/n2" "$TMP/n5")"

# ---- 6. 多 dist 混装：基线取 max（old={7:m111, 9:m112}），待发布全 m111 → 拒绝 ----
mk "$TMP/o6" puxian-0.2.0-1.m111.el7.x86_64.rpm puxian-0.2.0-1.m112.el9.x86_64.rpm
mk "$TMP/n6" puxian-0.2.0-1.m111.el7.x86_64.rpm puxian-0.2.0-1.m111.el9.x86_64.rpm
check "6 旧树混装取 max=m112，待发布 m111 ⇒ 拒绝" 1 "$(run "$TMP/o6" "$TMP/n6")"
check_has "6b 基线取最大" "0.2.0-1.m112"

# ---- 7. aarch64 文件名同样解析 ----
mk "$TMP/o7" puxian-0.2.0-1.m112.el9.aarch64.rpm
mk "$TMP/n7" puxian-0.2.0-1.m111.el9.aarch64.rpm
check "7 aarch64 文件名：m111 < m112 ⇒ 拒绝" 1 "$(run "$TMP/o7" "$TMP/n7")"

# ---- 8. 待发布树里没有 rpm → 环境错误 rc=2 ----
mkdir -p "$TMP/n8/9/x86_64"
check "8 待发布树无 rpm ⇒ rc=2（拒绝，防推空仓库）" 2 "$(run "$TMP/n2" "$TMP/n8")"

# ---- 9. 参数缺失 → rc=2 ----
bash "$GUARD" >"$TMP/out.txt" 2>&1; rc=$?
check "9 无参数 ⇒ rc=2" 2 "$rc"

# ---- 10. 逃生舱：RPM_ALLOW_REGRESSION 非空 ⇒ 允许，但必须留痕 ----
check "10 显式覆盖 RPM_ALLOW_REGRESSION=... ⇒ 允许发布" 0 \
      "$(run "$TMP/n2" "$TMP/n4" RPM_ALLOW_REGRESSION=紧急撤回坏版本)"
check_has "10b 覆盖留痕" "人工显式覆盖"
check_has "10c 记入日志的理由" "紧急撤回坏版本"

# ---- 11. 逃生舱为空字符串 ⇒ 仍拒绝（不允许"设了空值绕过"）----
check "11 RPM_ALLOW_REGRESSION= 空 ⇒ 仍拒绝" 1 \
      "$(run "$TMP/n2" "$TMP/n4" RPM_ALLOW_REGRESSION=)"

# ---- 12. 正常新版 + 混合提示：new={m112,m113} 且基线 m111 → 允许 ----
mk "$TMP/n12" puxian-0.2.0-1.m112.el7.x86_64.rpm puxian-0.2.0-1.m113.el9.x86_64.rpm
check "12 全 ≥ 基线 ⇒ 允许" 0 "$(run "$TMP/o2" "$TMP/n12")"

echo "== 自测结果: $pass 通过 / $fail 失败 =="
exit "$fail"
