#!/bin/bash
# 普贤全量回归（M0–M10）
# 用法：bash tests/run_all.sh
# 覆盖：
#   1) Rust 单元测试（cargo test）
#   2) C 级 GC 测试（tests/gc/run.sh）
#   3) 所有示例双模式验证（px run == px build 输出一致）
#   4) 编译产物静态链接检查
# 注意：https_demo.px 需要外网；断网时跳过（不判定失败）
set -u
cd "$(dirname "$0")/.."   # compiler/
PX=./target/debug/px
FAIL=0

echo "========== [1/4] Rust 单元测试 =========="
if ! cargo test 2>&1 | tail -3 | grep -q "0 failed"; then
    echo "❌ cargo test 失败"; FAIL=1
else
    echo "✅ cargo test 通过"
fi

echo "========== [2/4] C 级 GC 测试 =========="
if ! bash tests/gc/run.sh >/tmp/gc_out.txt 2>&1; then
    echo "❌ GC 测试失败"; tail -20 /tmp/gc_out.txt; FAIL=1
elif ! grep -q "ALL GC TESTS PASSED" /tmp/gc_out.txt; then
    echo "❌ GC 测试未全部通过"; tail -20 /tmp/gc_out.txt; FAIL=1
else
    echo "✅ GC 测试通过（ALL GC TESTS PASSED）"
fi

echo "========== [3/4] 示例双模式回归 =========="
EXAMPLES="hello fib struct match concurrent concurrent_m3 std_demo net_demo toolchain_demo p0_random_io p1_mutex_rwlock p2_crypto_hash p3_regex p4_http_server p5_px_serve p6_timer p7_aes_xml_zip"
for ex in $EXAMPLES; do
    f="../examples/$ex.px"
    if [ ! -f "$f" ]; then echo "⚠ 跳过（不存在）: $f"; continue; fi
    r1=$("$PX" run "$f" 2>/dev/null)
    if [ $? -ne 0 ]; then echo "❌ $ex run 失败"; FAIL=1; continue; fi
    if ! "$PX" build "$f" >/dev/null 2>&1; then echo "❌ $ex build 失败"; FAIL=1; continue; fi
    out="../examples/build/$ex"
    r2=$("$out" 2>/dev/null)
    # 宽松比较：过滤已知波动行（计时、JSON 键序、时区）
    r1n=$(echo "$r1" | grep -v -E "当前时间|耗时|elapsed|JSON 回写")
    r2n=$(echo "$r2" | grep -v -E "当前时间|耗时|elapsed|JSON 回写")
    # concurrent.px 的 first/second 顺序与配对取决于调度（并发非确定），
    # 只验证两端输出含相同数字集合（{2,4}）
    if [ "$ex" == "concurrent" ]; then
        r1n=$(echo "$r1n" | grep -oE '[0-9]+' | sort)
        r2n=$(echo "$r2n" | grep -oE '[0-9]+' | sort)
    fi
    if [ "$r1n" == "$r2n" ]; then
        echo "✅ $ex run==build"
    else
        echo "❌ $ex 输出不一致"
        echo "--- run ---"; echo "$r1n" | head -5
        echo "--- build ---"; echo "$r2n" | head -5
        FAIL=1
    fi
done

# HTTPS 示例（需要外网，失败不判定 FAIL）
if [ -f ../examples/https_demo.px ]; then
    echo "--- https_demo（需外网） ---"
    r1=$("$PX" run ../examples/https_demo.px 2>/dev/null)
    if [ $? -eq 0 ]; then
        if "$PX" build ../examples/https_demo.px >/dev/null 2>&1; then
            r2=$(../examples/build/https_demo 2>/dev/null)
            if [ "$r1" == "$r2" ]; then echo "✅ https_demo run==build（外网）"; else echo "⚠ https_demo 输出不一致（不判失败）"; fi
        else echo "⚠ https_demo build 失败（不判失败）"; fi
    else
        echo "⚠ https_demo 需要外网，跳过（不判失败）"
    fi
fi

echo "========== [4/4] 静态链接检查 =========="
for ex in hello net_demo https_demo; do
    b="../examples/build/$ex"
    if [ -f "$b" ]; then
        if file "$b" | grep -q "statically linked"; then
            echo "✅ $ex 静态链接"
        else
            echo "❌ $ex 非静态链接"; FAIL=1
        fi
    fi
done

echo "========== 结果 =========="
if [ $FAIL -eq 0 ]; then
    echo "✅ 全部通过"
else
    echo "❌ 存在失败项"
fi
exit $FAIL
