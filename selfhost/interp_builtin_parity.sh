#!/usr/bin/env bash
# ============================================================
# 解释器轨 ↔ 编译轨 行为一致性门（内置转发「丢参」类缺陷，qg-issue 69）
# ------------------------------------------------------------
# 背景（实锤）：解释器（`bootstrap/pxi`）的内置分发是 `selfhost/ibuiltin.px` 里
#   **手写转发层**（`if name == "x": return Ok(x(args[0], args[1]))`）。它逐条列参，
#   **可选尾参一旦漏写就静默丢弃** —— 解释器不校验实参个数，调用方完全看不出异常：
#     · write_file(p, s, 0o600)   → 解释器落 0644（编译轨 0600）⇒ 密钥类文件被悄悄放宽权限
#     · append_file(p, s, 0o600)  → 同上
#     · os_spawn(cmd, argv, true) → 解释器不成进程组 ⇒ 后续 kill(-pgid) 语义失效
#   三者只在**解释器轨**出现，编译轨正常 ⇒ 典型「双轨静默分叉」，且既有门全看不见
#   （CI 的示例/工具自测都只跑编译产物，跑解释器的只有 --interp 对拍，覆盖不到 os_/io 面）。
#
# 门的判据（直接比可观测事实，不依赖 golden）：
#   ① 同一份用例 `examples/m66_proc/proc_test.px` 分别经**解释器轨**与**编译轨**执行；
#   ② 两轨都必须 `fail=0` 且 `pass` 相同（分叉即红）；
#   ③ 用例本身已含绝对值断言（mode=600 / append mode=600 / os_spawn 组杀成功），
#      防「两轨一起错」被 ② 放过。
#
# 用法：./selfhost/interp_builtin_parity.sh     （退出码 0=绿，1=红）
# ============================================================
set -u
cd "$(dirname "$0")/.."
export LC_ALL=C LANG=C

PX_I=./bootstrap/pxi          # 解释器轨
PX_C=./tools/pxc              # 编译轨（用户面默认轨）
CASE=examples/m66_proc/proc_test.px

for b in "$PX_I" "$PX_C"; do
    [ -x "$b" ] || { echo "❌ 缺可执行件 $b"; exit 1; }
done
[ -f "$CASE" ] || { echo "❌ 缺用例 $CASE"; exit 1; }

summary() {  # stdin → "pass=N fail=M"
    grep -m1 "^pass=" | sed 's/^pass=//; s/ fail=/ /'
}

echo "── [1/2] 解释器轨：$PX_I $CASE"
iout=$("$PX_I" "$CASE" 2>&1)
echo "$iout" | grep -E "^FAIL" | sed 's/^/     /'
echo "$iout" | grep -m1 "^pass=" | sed 's/^/     /'
iset=$(echo "$iout" | summary)
ip=${iset%% *}; ifail=${iset##* }

echo "── [2/2] 编译轨：$PX_C build + 运行"
( cd "$(dirname "$CASE")" && ../../tools/pxc build --no-quic "$(basename "$CASE")" >/dev/null 2>&1 ) || {
    echo "❌ 编译失败"; exit 1; }
BIN="$(dirname "$CASE")/build/$(basename "$CASE" .px)"
[ -x "$BIN" ] || { echo "❌ 编译产物缺失 $BIN"; exit 1; }
cout=$("$BIN" 2>&1)
echo "$cout" | grep -E "^FAIL" | sed 's/^/     /'
echo "$cout" | grep -m1 "^pass=" | sed 's/^/     /'
cset=$(echo "$cout" | summary)
cp=${cset%% *}; cfail=${cset##* }

bad=0
echo "── 判据 ①：两轨均无 FAIL"
[ "$ifail" = "0" ] && echo "    ✅ 解释器轨 fail=0" || { echo "    ❌ 解释器轨 fail=$ifail"; bad=1; }
[ "$cfail" = "0" ] && echo "    ✅ 编译轨   fail=0" || { echo "    ❌ 编译轨   fail=$cfail"; bad=1; }

echo "── 判据 ②：两轨 pass 数一致（同用例同断言集）"
if [ "$ip" = "$cp" ]; then
    echo "    ✅ 均为 pass=$ip"
else
    echo "    ❌ 分叉：解释器 pass=$ip / 编译 pass=$cp"; bad=1
fi

echo "── 判据 ③：用例内含绝对值断言（模式位 / 组杀）"
for k in "write_file mode 0600" "append_file mode 0600" "os_spawn 组杀成功"; do
    if echo "$iout" | grep -q "^PASS $k"; then
        echo "    ✅ 解释器轨断言通过：$k"
    else
        echo "    ❌ 解释器轨断言未通过：$k"; bad=1
    fi
done

echo "───"
if [ "$bad" = 0 ]; then
    echo "✅ 双轨内置行为一致性门：通过"
    exit 0
fi
echo "❌ 双轨内置行为一致性门：不通过"
exit 1
