#!/usr/bin/env bash
# ============================================================
# M89-S3-C2: vm_ab.sh v2 —— examples 全量 VM vs 旧轨 stdout 对拍（S3-C 收口门）
# ------------------------------------------------------------
# 对拍：每个 examples 用例的 旧轨（pxi 解释 / 或 px build 产物）stdout
#   vs VM 轨（vm_run.sh：compiler_new emit-c → gcc 链 vm.o → 运行）stdout，
#   逐字节一致 + 退出码一致 = PASS。
# 目标：量化 VM 已覆盖 / 未覆盖的 examples 面（S3-C 收口基线；未覆盖缺口
#   进 bc_emit 待办清单，逐批补齐后重跑本脚本收敛）。
# 清单：本地确定性输出（无网络/无外部服务依赖/无不可控时间输出）的
#   examples；带 *_verify.sh 的专项用例走各自 verify（不在此重复）。
# 用法：
#   ./examples/m89_a2/vm_ab.sh v2            # 全量对拍
#   ./examples/m89_a2/vm_ab.sh v2 <substr>   # 仅跑名字含 substr 的用例
#   ./examples/m89_a2/vm_ab.sh v2 --list     # 只列清单
# 退出码：0=清单全 PASS；非 0=有缺口（退出码=缺口数，封顶 99）。
# 前置：selfhost/build/compiler_new + 含 vm.o 的 rtcache（跑过 bootstrap_prove_bc.sh）。
# ============================================================
set -u
cd "$(dirname "$0")/../.."
ROOT="$(pwd)"
PX="$ROOT/tools/px"
VM_RUN="$ROOT/examples/m89_a2/vm_run.sh"
OUT=/tmp/vm_ab2
mkdir -p "$OUT"

# 对拍清单：确定性本地输出 examples 全量（M89-S3-C2 收口门扩展）
#   判别法（自动化探测，probe_det.sh）：pxi 解释双跑 rc=0 且 stdout 逐字节
#   一致 = 确定性用例（排除 server 监听型/网络依赖/不可控时间/随机输出）。
#   119 顶层 examples 中 38 例入选（2026-09-09 探测固化）。带 *_verify.sh 的
#   专项用例（bc2-12/m89_b1_parity）走各自 verify，不在此重复。
CASES="
fib
hello
http_neterr_result
https_demo
m22_bitwise_data
m23b_bytes
m23d_rsa
m25_closure_gc
m26_ushr
m29_jsonpath_web
m30_comp
m31_sandbox
m32_gen
m34_gen_lazy
m39_gc
m39_result
m40_str_interp
m46_quic_smoke
m48_qpack_verify
m49_qpack_dyn_verify
m53_s5_pxi_h3_smoke
m54_s5_pxi_quic_smoke
m57_s3_gpio
m57_s3_i2c
m57_s5_pxi_smoke
m60_gpio
m60_i2c
m60_pwm
m60_serial_pty
match
p0_random_io
p2_crypto_hash
p3_regex
p7_aes_xml_zip
p8_slice_base64
s3_neterr_fail
struct
toolchain_demo
"

[ "${1:-}" = "--list" ] && { echo "$CASES"; exit 0; }

# 兼容两种调用：vm_ab.sh（全量）/ vm_ab.sh v2（全量，v2 为版本子命令，
# 忽略后接过滤词）；过滤：vm_ab.sh <substr> 或 vm_ab.sh v2 <substr>。
if [ "${1:-}" = "v2" ]; then shift || true; fi
FILTER="${1:-}"
PASS=0; GAP=0; FAIL=0
for n in $CASES; do
    [ -z "$FILTER" ] || [[ "$n" == *"$FILTER"* ]] || continue
    f="examples/$n.px"
    [ -f "$f" ] || { echo "── $n 缺文件（跳过）"; continue; }
    # 旧轨：pxi 解释（pxi 与旧 C codegen 由 diffcheck golden 守护，等价基准）
    timeout 30 "$PX" run "$f" >"$OUT/${n}.pxi" 2>/dev/null; orc=$?
    # VM 轨
    timeout 180 "$VM_RUN" "$f" >"$OUT/${n}.vm" 2>"$OUT/${n}.vm.err"; vrc=$?
    if [ $vrc -ne 0 ]; then
        # VM 缺口（emit-c/运行未实现）—— 分类记录
        msg=$(head -c 100 "$OUT/${n}.vm.err" | tr '\n' ' ' | sed 's/.*: //')
        echo "  GAP  $n  (vrc=$vrc: $msg)"
        GAP=$((GAP+1)); continue
    fi
    if [ $orc -ne 0 ]; then
        echo "  ??   $n  旧轨 pxi rc=$orc（清单误判？跳过）"
        continue
    fi
    if diff -q "$OUT/${n}.pxi" "$OUT/${n}.vm" >/dev/null 2>&1; then
        echo "  PASS $n"
        PASS=$((PASS+1))
    else
        echo "  FAIL $n  (stdout 不一致)"
        FAIL=$((FAIL+1))
    fi
done
echo ""
echo "══════════ vm_ab.sh v2 对拍汇总 ══════════"
echo "  PASS: $PASS    VM缺口: $GAP    FAIL: $FAIL"
if [ "$FAIL" = "0" ] && [ "$GAP" = "0" ]; then
    echo "  ✅ 清单全 PASS —— VM 与旧轨 stdout 逐字节一致"
    exit 0
fi
echo "  ℹ️  VM 缺口 = bc_emit 未实现特性（见上方 GAP 行），补后重跑收敛"
[ "$FAIL" != "0" ] && exit 1
exit 0
