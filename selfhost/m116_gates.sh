#!/usr/bin/env bash
# M116 全门本地复跑（= CI regression + toolchain 关键步）
set -uo pipefail
cd /data/code/puxian
export LC_ALL=C LANG=C
FAIL=0
step() { echo ""; echo "══ $* ══"; }
run() {  # $1=名 $2..=命令
    local name="$1"; shift
    if "$@" > "/tmp/gate_$name.log" 2>&1; then
        echo "✅ $name"
    else
        echo "❌ $name（rc=$?）"; tail -12 "/tmp/gate_$name.log" | sed 's/^/     /'; FAIL=$((FAIL+1))
    fi
}

step "全件源码链门"
run check_all ./selfhost/rebake_bin.sh --check-all
step "C 轨重烘门"
run check_c ./selfhost/rebake_bin.sh --check
step "VM 轨重烘门"
run check_vm ./selfhost/rebake_bin.sh --check-vm
step "对拍回归"
run diff_lexer ./selfhost/diffcheck.sh --lexer
run diff_parser ./selfhost/diffcheck.sh --parser
run diff_errors ./selfhost/diffcheck.sh --errors
run diff_codegen ./selfhost/diffcheck.sh --codegen
run diff_value ./selfhost/diffcheck.sh --value
run diff_interp ./selfhost/diffcheck.sh --interp
step "引擎一致性 / 内置双轨 / 僵尸回收"
run engine_parity ./selfhost/engine_parity.sh
run interp_builtin_parity ./selfhost/interp_builtin_parity.sh
run zombie_reap ./selfhost/zombie_reap_check.sh
step "M116 专项门"
run m116 bash examples/m116_builtin_semantics/verify.sh
step "工具自测（CI toolchain 步列出的那批）"
run m64_fmt bash examples/m64_fmt/verify.sh
run m64_lint bash examples/m64_lint/verify.sh
run m65_lsp bash examples/m65_lsp/verify.sh
run m65_mcp bash examples/m65_mcp/verify.sh
run m66_yaml bash examples/m66_yaml/verify.sh
run m66_pxml bash examples/m66_pxml/verify.sh
run m66_lunar bash examples/m66_lunar/verify.sh
run m71_mcp_build bash examples/m71_mcp_build/verify.sh
run m82_http_serve_unix bash examples/m82_http_serve_unix/verify.sh
run interp_builtin_list bash selfhost/builtin_list_check.sh
step "示例编译"
run ex_fib ./tools/pxc build examples/fib.px
run ex_match ./tools/pxc build examples/match.px
run ex_struct ./tools/pxc build examples/struct.px

echo ""
echo "══ 汇总：失败 $FAIL 项 ══"
exit $((FAIL > 0))
