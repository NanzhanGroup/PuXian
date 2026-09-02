#!/usr/bin/env bash
# ============================================================
# M49 QPACK 动态表 + SETTINGS 会话验证
#   编译模式（pxc build）+ 解释模式（pxi）双跑，
#   输出必须逐字节一致（.px 内 assert 覆盖字节/语义精确）。
# ============================================================
set -u
cd "$(dirname "$0")/.."
B=examples/build

run_mode() {
    local mode=$1 cmd=$2
    echo "--- [$mode] ---"
    local out ec
    out=$(bash -c "$cmd" 2>&1)
    ec=$?
    if [ "$ec" != "0" ]; then
        echo "[$mode] 运行失败 exit=$ec"
        echo "$out"
        return 1
    fi
    echo "$out" | grep -q "M49 QPACK 动态表 + SETTINGS verify PASS" || { echo "[$mode] 汇总行缺失"; echo "$out"; return 1; }
    echo "$out" | grep -q "dynamic_reuse=ok" || { echo "[$mode] dynamic_reuse 行缺失"; echo "$out"; return 1; }
    echo "$out" | grep -q "compression=ok" || { echo "[$mode] compression 行缺失"; echo "$out"; return 1; }
    echo "$out" | grep -q "settings_roundtrip=ok" || { echo "[$mode] settings 行缺失"; echo "$out"; return 1; }
    echo "[$mode] QPACK 动态表会话验证 PASS"
    echo "$out" > /tmp/m49_qpack_dyn_$mode.out
    return 0
}

echo "=== M49 QPACK 动态表 + SETTINGS 会话验证 ==="
ok=1
./tools/pxc build examples/m49_qpack_dyn_verify.px >/dev/null 2>&1 || { echo "FAIL: 编译失败"; ok=0; }
if [ "$ok" = "1" ]; then
    run_mode comp "$B/m49_qpack_dyn_verify" || ok=0
fi
if [ "$ok" = "1" ]; then
    run_mode interp "./bootstrap/pxi examples/m49_qpack_dyn_verify.px" || ok=0
fi
if [ "$ok" = "1" ]; then
    if cmp -s /tmp/m49_qpack_dyn_comp.out /tmp/m49_qpack_dyn_interp.out; then
        echo "编译/解释双模式输出逐字节一致 ✅"
    else
        echo "FAIL: 双模式输出不一致"; ok=0
    fi
fi
if [ "$ok" = "1" ]; then
    echo "M49 QPACK 动态表 + SETTINGS 验证 PASS（编译+解释）"
    exit 0
else
    echo "M49 验证 FAIL"
    exit 1
fi
