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
# M17：编译模式 .px 脚本执行依赖 px 解释器（PX_BIN 或 PATH）→ 指向当前构建
export PX_BIN="$(pwd)/target/debug/px"
export PATH="$(pwd)/target/debug:$PATH"
# cargo 可能不在默认 PATH（setsid/CI 环境）→ 自动探测
if [ -x /root/.cargo/bin/cargo ]; then
    CARGO=/root/.cargo/bin/cargo
else
    CARGO=cargo
fi
FAIL=0

echo "========== [1/4] Rust 单元测试 =========="
# --test-threads=1：解释器/GC 测试共享全局状态（gc collect / 路由表 / SQLite 句柄），
# 并行跑会互相干扰（历史 flaky，M23 部分加锁未覆盖全部）→ 单线程串行稳定；
# --release：release 二进制已预编译（无 target 锁竞争 / 编译慢问题）
if ! timeout 600 $CARGO test --release -- --test-threads=1 >/tmp/cargo_test.log 2>&1; then
    echo "❌ cargo test 失败（退出码 $?，日志 /tmp/cargo_test.log）"; FAIL=1
elif ! grep -q "0 failed" /tmp/cargo_test.log; then
    echo "❌ cargo test 失败（test result 非 0 failed）"; tail -5 /tmp/cargo_test.log; FAIL=1
else
    echo "✅ cargo test 通过（$(grep "test result" /tmp/cargo_test.log | tail -1 | grep -oE "[0-9]+ passed")）"
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
EXAMPLES="hello fib struct match concurrent concurrent_m3 std_demo net_demo toolchain_demo p0_random_io p1_mutex_rwlock p2_crypto_hash p3_regex p4_http_server p5_px_serve p6_timer p7_aes_xml_zip m22_bitwise_data m23b_bytes m24_slice_xml m25_closure_gc m26_ushr m28_time_sqlite m28_route m29_jsonpath_web m29_webprod m30_int_bytes m30_comp m30_tls_pool m31_sandbox m31_vhost m32_gen m32_sse_reconnect m32_hot_reload m33_route_rate_limit m33_sni m33_access_log m33_udp_alt_svc m34_gen_lazy m34_pool_cfg m34_bus_ws m35_h2 m35_gzip_rl m36_log_ctx m36_ws_hb m36_pool_grace m37_s3 m37_h2_tls m38_udp_serve m38_chunked m38_ws_reconnect m38_h2_multi m39_result m39_gc"
for ex in $EXAMPLES; do
    f="../examples/$ex.px"
    if [ ! -f "$f" ]; then echo "⚠ 跳过（不存在）: $f"; continue; fi
    r1=$("$PX" run "$f" 2>/dev/null)
    if [ $? -ne 0 ]; then echo "❌ $ex run 失败"; FAIL=1; continue; fi
    if ! "$PX" build "$f" >/dev/null 2>&1; then echo "❌ $ex build 失败"; FAIL=1; continue; fi
    out="../examples/build/$ex"
    r2=$("$out" 2>/dev/null)
    # 宽松比较：过滤已知波动行（计时、JSON 键序、时区、access log 时间戳、udp/s3_list 顺序）
    r1n=$(echo "$r1" | grep -v -E "当前时间|耗时|elapsed|JSON 回写|\[px-access\]|req=px-[0-9]|udp recv:.*from 127\.0\.0\.1:[0-9]+|s3_list:")
    r2n=$(echo "$r2" | grep -v -E "当前时间|耗时|elapsed|JSON 回写|\[px-access\]|req=px-[0-9]|udp recv:.*from 127\.0\.0\.1:[0-9]+|s3_list:")
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

# M32 wss 一行连接（本地 wss：预生成自签 CA 供双模式信任；失败判定 FAIL）
echo "--- m32_ws_url（本地 wss） ---"
openssl req -x509 -newkey rsa:2048 -keyout /tmp/px_m32_key.pem -out /tmp/px_m32_cert.pem -days 1 -nodes -subj /CN=localhost -addext basicConstraints=critical,CA:FALSE -addext subjectAltName=DNS:localhost >/dev/null 2>&1
export PX_TLS_CA_FILE=/tmp/px_m32_cert.pem
r1=$(timeout 60 "$PX" run ../examples/m32_ws_url.px 2>/dev/null)
if [ $? -ne 0 ] || ! echo "$r1" | grep -q M32_WS_URL_OK; then echo "❌ m32_ws_url run 失败"; FAIL=1; else
    if "$PX" build ../examples/m32_ws_url.px >/dev/null 2>&1; then
        r2=$(timeout 60 ../examples/build/m32_ws_url 2>/dev/null)
        if echo "$r2" | grep -q M32_WS_URL_OK; then echo "✅ m32_ws_url run==build"; else echo "❌ m32_ws_url build 输出不一致"; FAIL=1; fi
    else echo "❌ m32_ws_url build 失败"; FAIL=1; fi
fi

# M39 Err 传播到 main 顶层：预期打印"错误: something bad"且退出码 1（双模式）
echo "--- m39_err_main（Err 传播退出码） ---"
err1=$("$PX" run ../examples/m39_err_main.px 2>&1); c1=$?
err2=""; c2=0
if "$PX" build ../examples/m39_err_main.px >/dev/null 2>&1; then
    err2=$(../examples/build/m39_err_main 2>&1); c2=$?
fi
if [ "$c1" -eq 1 ] && echo "$err1" | grep -q "错误: something bad" && \
   [ "$c2" -eq 1 ] && echo "$err2" | grep -q "错误: something bad"; then
    echo "✅ m39_err_main 双模式 Err 传播 + 退出码 1"
else
    echo "❌ m39_err_main 失败（run 退出 $c1 / build 退出 $c2）"; FAIL=1
fi

echo "========== [4/4] 静态链接检查 =========="
for ex in hello net_demo https_demo m29_jsonpath_web m39_result; do
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
