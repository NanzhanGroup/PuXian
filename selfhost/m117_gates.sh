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
step "M117 专项门"
run m117 bash examples/m117_realworld_defects/verify.sh
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
step "CI 同款 fmt / lint 门（CI 有、本地全门原先没有 —— 第 18 轮补进来）"
# 现场（第 18 轮，提交前预检）：`git push` 前按 CI 的 toolchain 步逐条预跑，发现
#   **15 个文件 fmt --check 不符**（`selfhost/codegen.px` `selfhost/pxlexer.px` +
#   `stdlib/` 13 个），而本地 m116/m117 **全绿** —— 又一条「CI 有、本地没有」的门：
#   本地门再全，只要 CI 独有的那几步不在本地，推上去就是红的。
#   （不符内容均为空行/续行缩进的规范化，语义零变化；但 CI 会判红。）
# 口径与 ci.yml 的 toolchain 步**逐字一致**：fmt 覆盖 selfhost+tools+stdlib，
#   lint 覆盖 compiler 主入口 + 16 个 tools 文件。
run fmt_check bash -c 'for f in selfhost/*.px tools/*.px stdlib/*.px; do ./bootstrap/pxfmt --check "$f" >/dev/null || { echo "❌ 格式不符: $f"; exit 1; }; done; echo "fmt --check 全绿（selfhost+tools+stdlib）"'
run lint_gate bash -c './bootstrap/pxlint selfhost/compiler.px >/dev/null || { echo "❌ compiler.px 项目级 lint 失败"; exit 1; }; for f in tools/fmt_core.px tools/fmtlexer.px tools/jsonrpc_core.px tools/lint_core.px tools/lsp_core.px tools/pxbench.px tools/pxcheck.px tools/pxdoc.px tools/pxfmt.px tools/pxlint.px tools/pxlsp.px tools/pxmcp.px tools/pxpkg.px tools/pxslice.px tools/pxtest.px tools/routegen.px; do ./bootstrap/pxlint "$f" >/dev/null || { echo "❌ lint 失败: $f"; exit 1; }; done; echo "lint 全绿（compiler 入口 0 错 + tools 16 文件 0/0）"'
step "生态索引防漂移（CI 有、本地全门原先没有 —— 第 17 轮补进来）"
# 现场（第 17 轮实测）：`docs/native_index.json` 停在 **313** 个 native，而 runtime 实际
#   **330** —— 自第 9 轮起新增的 17 个（chmod/chr/file_stat/flock/go_errno_string/ord/
#   json_parse_opt/regex_valid/sse_connect_ex/rpc… 等）**一个都没进索引**；
#   `docs/ecosystem_index.json` 同样停在 yaml 拆分前的形态。而这条门只写在 ci.yml 的
#   toolchain 步里，本地 m116/m117 都没有 ⇒ "本地全绿、CI 红"能潜伏好几轮。
# ⚠️ 本地不能照抄 CI 的 `git diff --exit-code`：本地工作区通常**未提交**（改动一堆），
#   那样必然假红。改为"**重生成 → 与 docs/ 现值比内容**"，与是否提交无关、语义等价。
run eco_index bash -c 'cp docs/ecosystem_index.json /tmp/eco_before_a.json && cp docs/native_index.json /tmp/eco_before_b.json && ./bootstrap/pxi tools/gen_ecosystem.px >/dev/null && bash tools/gen_native_table.sh >/dev/null && diff -q /tmp/eco_before_a.json docs/ecosystem_index.json && diff -q /tmp/eco_before_b.json docs/native_index.json && echo "索引与 stdlib/runtime 一致"'
step "语言侧常驻门（M130–M137：由 qg-issue 87 各轮真实缺陷攒下的回归门）"
# 说明（第 17 轮补）：这批门此前**只在本地跑**，CI 与 m116/m117 都不含 ——
#   等于"修好的缺陷没有防线"。此处显式纳入（顺序与台账一致）。
# ⚠️ m136 需要 Go（用真 encoding/json 产出真值），CI 无 Go 步骤，故只在本地全门里跑。
run m130_flock_chmod bash examples/m130_flock_chmod/verify.sh
run m131_stream_post bash examples/m131_http_stream_post/verify.sh
run m132_sse_chunked bash examples/m132_sse_chunked/verify.sh
run m133_unix_timeout bash examples/m133_http_unix_timeout/verify.sh
run m134_gc_obj_roots bash examples/m134_gc_obj_roots/verify.sh
run m136_go_json_fidelity bash examples/m136_go_json_fidelity/verify.sh
run m137_sse_connect_ex bash examples/m137_sse_connect_ex/verify.sh
# M139（第 19 轮）：裸标识符语句 = 编译错误（缺陷 101）。
# ⚠️ m138（正则的 Go 保真门）需要 Go 产真值，口径同 m136 ⇒ 只在本地全门跑，
#     CI 侧由 ci.yml 的 toolchain 步骤显式跑（该 runner 自带 Go）。
run m139_bare_ident_stmt bash examples/m139_bare_ident_stmt/verify.sh
# m140（连接失败成因分类 + 双栈）：**不依赖任何外部服务**，CI 也跑
run m140_http_conn_errno bash examples/m140_http_conn_errno/verify.sh
# m141（墙钟纳秒 now_ns + unix 服务端 remote = "@"，缺陷 109/110）：**不依赖任何外部服务**
#   （门内自带 unix + TCP 本地服务端），CI 也跑。
run m141_now_ns bash examples/m141_now_ns/verify.sh
# m142（Go encoding/json 的 Indent/Compact/HTMLEscape/quoteChar 逐字节复刻 + 缺陷 111/112）：
#   **需要 Go 产真值**（encoding/json 本尊），口径同 m136/m138 —— 本机全门跑，
#   CI runner 自带 Go ⇒ ci.yml 里也跑。
run m142_go_json_indent bash examples/m142_go_json_indent/verify.sh
# m143（float32 值族 / 位模式 / Go encoding/json 的 **32 位**浮点文本 + append_file_opt，
#   缺陷 115/117/118/119）：**需要 Go 产真值**，口径同 m136/m138/m142。
run m143_float32 bash examples/m143_float32/verify.sh
# m144（HTTP 大请求体 + chunked 请求/响应 + 连接级余留缓冲，缺陷 120–124）：进程内
#   spawn 服务端 + 客户端自调用，不依赖外部服务。
run m144_http_bigbody bash examples/m144_http_bigbody/verify.sh
# m145（循环引用值的 **比较 / 渲染 / JSON** 不得段错误，缺陷 125）：三轨
#   （VM / C / 解释轨 pxi）断言集相同 + 输出逐字节一致 + 环上 JSON 受控报错 + 2 道负控。
#   **不依赖任何外部服务** ⇒ 本机全门与 CI 都跑。
run m145_cycle_safe bash examples/m145_cycle_safe/verify.sh
run m146_float64_bits bash examples/m146_float64_bits/verify.sh
run m147_fmt_dec bash examples/m147_fmt_dec/verify.sh
run m148_ieee_div bash examples/m148_ieee_div/verify.sh
step "CI 其余独占门（m118/m119/m120/m122 + 发布侧守卫自测 —— 第 18 轮补进来）"
# 现场（第 18 轮提交前预检）：ci.yml 里还有这几步**本地门从来没有** ——
#   而其中两条**实际已经是红的**（m119 的一句负控、m122 的一个正控），只因它们
#   只在 CI 跑、而这段时间只有本地跑 ⇒ 无人看见。这是「本地全绿、CI 红」的第三次现身。
#   m67_multiarch（x86_64 档）与 bootstrap_prove(_bc) 亦属 CI 独占，但耗时长
#   （前者含 GC 压力；后者 ≈7min + ≈20min），保留在 CI 与手工预检，不进门。
run m118 bash examples/m118_realworld_defects/verify.sh
run m119 bash examples/m119_multiline_expr/verify.sh
run m120 bash examples/m120_dict_strict/verify.sh
run m122 bash examples/m122_builtin_args/verify.sh
run pkg_guard_monotonic bash packaging/selftest_rpm_monotonic_guard.sh
run pkg_make_release bash packaging/selftest_make_release.sh
run pkg_tag_guard bash packaging/selftest_tag_guard.sh
step "示例编译"
run ex_fib ./tools/pxc build examples/fib.px
run ex_match ./tools/pxc build examples/match.px
run ex_struct ./tools/pxc build examples/struct.px

echo ""
echo "══ 汇总：失败 $FAIL 项 ══"
exit $((FAIL > 0))
