#!/usr/bin/env bash
# M116 全门本地复跑（= CI regression + toolchain 关键步）
set -uo pipefail
cd /data/code/puxian
export LC_ALL=C LANG=C
FAIL=0

# ── 缺陷 139（第 33 轮）：负控门被打断会把 runtime/*.c 留在「篡改态」（语法合法、语义反向）——
#   全门开跑前先查残留标记，避免整轮白跑 / 篡改态被误提交。
if grep -l 'NEGCTL' runtime/*.c >/dev/null 2>&1; then
    echo "❌ 负控残留：$(grep -l 'NEGCTL' runtime/*.c | tr '\n' ' ') 仍含 NEGCTL 标记（上一轮门被中断？先还原再跑）"
    exit 1
fi
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
#   缺陷 115/117/118/119）：**需要 Go 产真值**（math.Float32bits / Float32frombits /
#   encoding/json 本尊），口径同 m136/m138/m142 —— 本机全门跑，CI 里也跑。
run m143_float32 bash examples/m143_float32/verify.sh
# m144（HTTP 客户端大请求体 / opts.chunked / 服务端 chunked 请求体 + 连接级余留缓冲，
#   缺陷 120/121/122/123/124）：进程内 spawn 服务端 + 客户端自调用，**不依赖外部服务**
#   ⇒ 本机全门与 CI 都跑。
run m144_http_bigbody bash examples/m144_http_bigbody/verify.sh
# m145（循环引用值的 **比较 / 渲染 / JSON** 不得段错误，缺陷 125）：三轨
#   （VM / C / 解释轨 pxi）断言集相同 + 输出逐字节一致 + 环上 JSON 受控报错 + 2 道负控。
#   **不依赖任何外部服务** ⇒ 本机全门与 CI 都跑。
run m145_cycle_safe bash examples/m145_cycle_safe/verify.sh
run m146_float64_bits bash examples/m146_float64_bits/verify.sh
run m147_fmt_dec bash examples/m147_fmt_dec/verify.sh
run m148_ieee_div bash examples/m148_ieee_div/verify.sh
# m149（TCP「带超时 + 可辨别失败」族 —— Redis/PG 客户端移植的前提；
#   连接超时 / 读超时 / EOF 可辨别 / TCP_NODELAY 默认对齐 Go；3 道负控）：VM+C+解释轨
run m149_tcp_deadline bash examples/m149_tcp_deadline/verify.sh
# M150（第 32 轮）：摘要/密钥派生族（md5/md5_bytes/pbkdf2_sha256）+ TLS 客户端族
#   （tls_connect/tls_send/tls_recv/tls_close）；Go 本尊对拍 + 受控 TLS 服务端
run m150_tls_crypto bash examples/m150_tls_crypto/verify.sh
# M151（第 33 轮）：① `tcp_send*` 认 **bytes**（缺陷 137：修前发的是占位符 `"<bytes N>"`）
#   ② `tls_upgrade(fd, opts)` —— 在**已连接 fd** 上升级 TLS（PG 式 SSLRequest 协商必需，
#      `tls_connect` 自建 socket 表达不出）③ 解释轨「模块缺失」诊断不再打挂进程（缺陷 138）。
#   受控服务端（回显 / PG 式协商）+ **Go crypto/tls 同一服务端对拍** + 3 道负控 ⇒ 不依赖外网。
run m151_pg_tls_bytes bash examples/m151_pg_tls_bytes/verify.sh
# M153（第 35 轮）：常量池 / 短串池化（缺陷 145 第一刀）+ 分配统计可观测 + 缺陷 147
#   （runtime 编译失败 ⇒ 重烘门静默跳过 ⇒ **退化放行暗门**）。
#   含：三轨语义（37 断言）· 分配预算硬阈值（计数可复现）· 输出冻结 · 3 道负控。
run m153_alloc bash examples/m153_alloc/verify.sh
# M154（第 36 轮）：分配率第二刀 —— 小整数 `str()` 池 + 多字节 rune 池 + join 字节精确
#   （缺陷 149）+ 常量池按指针哈希 + 比较器同一性快路径 + 钉住表去重集合。
#   含：三轨语义（57 断言，含内嵌 NUL 的 join/`+` 逐字节一致）· 分配预算硬阈值 · 输出冻结
#   · 3 道负控（关 rune 池 / 关整数文本池 ⇒ 预算红；join 退回 strlen ⇒ 语义红）。
run m154_alloc2 bash examples/m154_alloc2/verify.sh
# M155（第 37 轮）：含内嵌 NUL 的字符串 —— 字面量 → 常量池 → 渲染 → 输出（缺陷 148/150/151/152）。
#   含：三轨语义（41 断言 × 3 轨，VM/C/解释轨逐字节一致）· stdout/落盘**原始字节**逐字节
#   · 发射形状（PX_STR_LIT/PXK_STR_LIT 恒 3 位八进制）· 词法/AST 面 · 自举安全不变式
#   · 4 道负控（长度退回 strlen / print 退回 printf / 长度多算 1 / VM 忽略 K 长度）。
run m155_nul_bytes bash examples/m155_nul_bytes/verify.sh
# M156（第 38 轮）：零依赖 ONNX 解析面（token-cache embedding 引擎的 Go 侧是
#   cgo + onnxruntime .so；本实现自解 protobuf，零外部依赖）。
#   含：**构造性真值 ↔ 独立参考解码器 ↔ C 侧 native** 三重对拍（12 个正例）
#   · 负控 4 个模型必须报错（无 graph / 截断 / field 0 / 9 维超界）
#   · 权重 head16 逐字节（含窄类型低字节截断）· VM 轨与解释轨逐字节一致
#   · 真实模型（90MB MiniLM）可选，SKIP 显式不计入 PASS。
run m156_onnx bash examples/m156_onnx/verify.sh
# ── M157（第 39 轮）：ONNX **执行面**（张量 + 64 算子 + 拓扑执行器）──
#   三方对拍（C ↔ 独立参考 ↔ 手算真值）· 算子覆盖率（注册表 64 个必须全被执行过）
#   · 执行器行为（乱序可跑 / 未知算子·环·广播·缺 feed 必须报错）· erf 1001 点精度
#   · 真实模型端到端（可选）· C 侧负控 2 道（含"编译失败也算没能证明"）
run m157_onnx_exec bash examples/m157_onnx_exec/verify.sh
# ── M158（第 40 轮）：解释轨函数值 → runtime native 桥（缺陷 114 根治）+ type 口径（缺陷 161）──
#   背景：解释轨把用户函数包装成 dict，而 runtime native 按 PX_FUNC 校验 ⇒ 解释轨
#   `set_interval(fn (): …)` 报 R1002（**不可捕获**）⇒ 函数值传给 native 整族不可用。
#   判据：三轨（解释轨 / VM / C）stdout 逐字节一致 + type(函数值)=="function" 口径守卫
#   + 3 道负控（关自动桥接 / 不装调度器 / 调度器忽略函数值 —— 每道必须判红 + 逐字节还原）。
#   另含已知缺口报告（缺陷 159 VM 轨闭包 upvalue / 160 C 轨嵌套闭包捕获，SKIP 不计失败）。
run m158_interp_fn bash examples/m158_interp_fn/verify.sh
# ── M159（第 45 轮）：宿主机架构自适应（第三方仓库照出的缺口 G2）──────────────
#   背景：aarch64 **原生**上裸 `px build` 此前必失败 —— mbedtls 库路径写死仓库 x86_64
#   布局（lib），QUIC 静态库（ngtcp2/openssl-quictls）只有 x86_64 预置 ⇒ 用户被迫手写
#   --cc/--mbedtls-lib/--sqlite-obj/--zlib-lib/--no-quic 一长串（第三方 px-openEuler-bootstrap
#   正因此自己写了一套自举脚本）。
#   判据：宿主架构决策可断言（--print-plan + PX_HOST_ARCH 覆盖）——
#   x86_64 计划零变化 / aarch64·armv7·riscv64 自动换 lib-<arch> 布局并明确提示裁 QUIC /
#   逃生舱 PX_HOST_QUIC=1 / 显式 flag 优先 / 负控：宿主库缺失必须明确报错且**不得静默退化** /
#   真机回归：x86_64 仍能真编译真运行。
run m159_hostarch bash examples/m159_hostarch/verify.sh
# ── M160（第 46 轮）：词法闭包 / 函数体内 `def` —— 缺陷 159（VM 轨）+ 160（C 轨）收口 ──
#   背景：VM 轨（**用户面默认轨**）此前无捕获机制（闭包体把外层局部当全局名）+ 不支持
#   函数体内 `FuncDef`；C 轨的 `cg_ast_bound` 把 Assign 目标当"绑定" ⇒ 闭包自由变量漏算。
#   修法：两轨共用同一条自由变量分析 + **按引用捕获（cell）**（与解释轨 env 链同义）。
#   判据：三轨 stdout 逐字节一致（13 + 5 + 5 断言）+ 交叉判据（M158 缺口复现器升格为硬判据）
#   + 3 道负控（恢复 Assign 绑定口径 ⇒ C 轨必红 / 关本帧装箱 ⇒ VM 必红 / MKCLO 槽基址 +1 ⇒ VM 必红）。
run m160_closure bash examples/m160_closure/verify.sh
# ── M161（第 47 轮）：生成器捕获（GenExp）—— 缺陷 163（VM 轨）+ 164（C 轨）收口 ──
#   背景：生成器 `transform`/`filter` 合成的 lambda **没有捕获表** ⇒ VM 轨（用户面默认轨）
#   把外层局部按全局名解析（M160 门第 ⑥ 层登记为缺陷 163）；而**同帧里闭包已把该局部装箱
#   为 cell** 时，C 轨的生成器直接**共享该 cell** ⇒ 退化成引用语义（本轮加强面抓到，登记 164）。
#   修法：三轨同一条真相 = **按值快照**（= 解释轨 GenExp 创建时求值）——VM 轨先取捕获值
#   （CELLGET/MOV）到连续临时槽、再逐个 CELLNEW 造新 cell；C 轨 `px_cell(px_cell_get(cv))`。
#   **闭包**捕获仍按引用（不动）。
#   判据：三轨 stdout 逐字节一致（13 + 6 + 5 断言）+ 交叉判据（缺陷 163 最小复现器由报告
#   升格为硬判据）+ 3 道负控（捕获表置空 ⇒ VM 必红 / 快照退化为普通值槽 ⇒ VM 必红 /
#   filter 捕获表丢弃 ⇒ VM 必红）。
run m161_genexp bash examples/m161_genexp_capture/verify.sh
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
step "发射冻结门（M153 建立）：253+ 个用例的 --emit-c 输出必须逐字节不变"
# 为什么：`--check`/`prove` 只看 **compiler.px 自己**的产物；"改了 runtime/发射路径却顺手
#   动摇了别的程序的发射结果"这类回归此前没有任何门看得见（第 34 轮起靠临时脚本手查）。
run emitc_freeze bash selfhost/emitc_freeze.sh --check
step "示例编译"
run ex_fib ./tools/pxc build examples/fib.px
run ex_match ./tools/pxc build examples/match.px
run ex_struct ./tools/pxc build examples/struct.px

echo ""
echo "══ 汇总：失败 $FAIL 项 ══"
exit $((FAIL > 0))
