#!/usr/bin/env bash
# ============================================================
# M48 QPACK 完整 codec 验证（Huffman + 静态表压缩）
#   编译模式（pxc build）+ 解释模式（pxi run）双跑，
#   输出必须逐字节一致（RFC 官方向量字节精确断言在 .px 内）。
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
    # 关键输出断言（对拍文件内 assert 已覆盖字节精确性，这里抽查关键行）
    echo "$out" | grep -q "huff_www=f1e3c2e5f23a6ba0ab90f4ff" || { echo "[$mode] huff_www 行缺失/不符"; echo "$out"; return 1; }
    echo "$out" | grep -q "huff_nocache=a8eb10649cbf" || { echo "[$mode] huff_nocache 行缺失/不符"; echo "$out"; return 1; }
    echo "$out" | grep -q "qenc_static=0000d1" || { echo "[$mode] qenc_static 行缺失/不符"; echo "$out"; return 1; }
    echo "$out" | grep -q "M48 QPACK codec verify PASS" || { echo "[$mode] 汇总行缺失"; echo "$out"; return 1; }
    echo "[$mode] QPACK codec 字节精确断言 PASS"
    echo "$out" > /tmp/m48_qpack_$mode.out
    return 0
}

echo "=== M48 QPACK 完整 codec 验证 ==="
ok=1
./tools/pxc build examples/m48_qpack_verify.px >/dev/null 2>&1 || { echo "FAIL: 编译失败"; ok=0; }
if [ "$ok" = "1" ]; then
    run_mode comp "$B/m48_qpack_verify" || ok=0
fi
if [ "$ok" = "1" ]; then
    run_mode interp "./bootstrap/pxi examples/m48_qpack_verify.px" || ok=0
fi
# 双模式输出逐字节一致
if [ "$ok" = "1" ]; then
    if cmp -s /tmp/m48_qpack_comp.out /tmp/m48_qpack_interp.out; then
        echo "编译/解释双模式输出逐字节一致 ✅"
    else
        echo "FAIL: 双模式输出不一致"; ok=0
    fi
fi
if [ "$ok" = "1" ]; then
    echo "M48 QPACK codec 验证 PASS（编译+解释）"
    exit 0
else
    echo "M48 QPACK codec 验证 FAIL"
    exit 1
fi
