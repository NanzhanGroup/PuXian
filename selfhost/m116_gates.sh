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
# ── M162（第 48 轮）：sorted 三轨一致 —— 缺陷 166（解释轨比较依据）+ 167（编译轨稳定性）──
#   背景：解释轨 `i_builtin_sorted` 用 `i_to_str` **渲染串**比较（`sorted([10,9,2])` = [10,2,9]），
#   编译轨用 `compare_values` 值比较（对）；且编译轨算法是**选择式**（两两交换）⇒ 在
#   「比较器判相等、值可渲染区分」的元素上**不稳定**（`sorted([1.0,1,0.5])` = [0.5,1,1.0]）。
#   修法：三轨同一条真相 = **值比较 + 稳定排序** —— 解释轨新增 `i_cmp_values`（对齐
#   `compare_values_raw` 三态 + `i_eq` 同款环保护）；编译轨 `bi_sorted` 改**相邻冒泡**。
#   判据：三轨 stdout 逐字节一致（26 + 12 + 4 断言）+ 交叉判据（第三方 PX-DEF-011 复现器）
#   + 3 道负控（值比较退回渲染串 ⇒ 解释轨必红 / 选择式回归 ⇒ 编译轨稳定性必红 /
#   list 逐元素退化 ⇒ PX-DEF-011 必红）。
run m162_sorted bash examples/m162_sorted_order/verify.sh
step "M163 字典键类型严格化（第 49 轮 · 缺陷 168/169/171 —— 构造位置非字符串键）"
# 背景：字面量 `{1: 11, 2: 22}` 与推导式 `{x: x for x in [1]}` 的非字符串键在 **VM 轨与 C 轨
#   被静默丢弃**（`len=0`；混合键只丢那一项）= 与 R1008「静默 null」同族的**静默数据丢失**，
#   且与解释轨（R1002）分叉；推导式上 VM 轨还只是借 `set` 方法词条报同码（同码不同文）；
#   索引赋值 `d[k]=v` 的解释轨词条与文档/编译轨不一致（缺陷 171）。
# 修法：三轨同一条真相 = **构造位置非字符串键 ⇒ R1002 `字典键必须是字符串，实际是 <t>`**
#   （新增 runtime `px_dict_set_checked` · VM 新指令 DICTSET · VM NEWDICT 内联检查 ·
#    C 轨字面量/推导式发射改走 checked · 解释轨两条路径词条对齐）。
# 判据：三轨 stdout 逐字节一致（16 + 6 断言）+ **严格性层 8 用例 × 三轨（rc≠0 + R1002 + 统一词条）**
#   + 3 道负控（VM NEWDICT 恢复跳过 / C checked 恢复静默 / 解释轨推导词条退回旧文案）。
run m163_dict_key bash examples/m163_dict_key_strict/verify.sh
step "M164 迭代位置语义与用户索引分离（第 50 轮 · 缺陷 170/174/175 —— d[int] / 惰性 GenExp seq / GenExp 形参捕获）"
# 背景：三处同病根的缺口 ——
#   ① 缺陷 170：`d[int]`（读取）解释轨报 R1002，而 VM/C 轨返回**第 i 个键**（M37 为 `for k in d`
#      迭代内部把「位置语义」塞进了用户可见的 `px_index`）⇒ 三轨分叉；
#   ② 缺陷 174：惰性生成器的 seq **只认 list/gen** ⇒ `(c for c in "abc")` / `(e for e in (1,2,3))`
#      / `(k for k in d)` 在 VM/C 轨**静默产出空生成器**（解释轨正常；range 看着正常只因 C 端
#      range 早已物化成 list）；
#   ③ 缺陷 175：`bc_genexp_caps` 把**已 unescape 的名字**再塞回 Param 节点，而 `cg_closure_caps`
#      又调 `rust_unescape(p[1])`（该函数假定带引号、**无条件剥首尾各一字符**）⇒ "c"→""，被
#      `cg_name_add` 的空串判据静默丢弃 ⇒ 形参被当自由变量；一旦本帧有同名局部（`var c = "Z"`）
#      就被装箱捕获 ⇒ **VM 轨**读到外层值（解释/C 轨正确）。
# 修法：迭代协议与用户索引**彻底分离** —— runtime 新增 `px_iter_at`（迭代专用：dict → 第 i 个键）
#   并**删除** `px_index` 的 dict-int 分支；VM 新指令 **ITERAT**；C 轨 cg_stmt/cg_expr 的迭代步
#   改走 `px_iter_at`；`px_lazy_seq_get` 补 dict/str/tuple（走同一协议）；`bc_genexp_caps` 改
#   「体内引用名 − 形参，再 ∩ 本帧局部」（与 C 轨 cg_gen_lambda 同一条真相，且不再二次 unescape）。
# 判据：三轨 stdout 逐字节一致（22 + 10 + 4 断言）+ **严格性层 5 用例 × 三轨（rc≠0 + R1002 + 统一词条）**
#   + 4 道负控（px_index 恢复 dict-int / 两轨发射器退回 INDEX / 惰性 seq 退回只认 list/gen /
#   bc_genexp_caps 退回旧捕获算法 —— 每道必须判红 + 逐字节还原）。
run m164_iter bash examples/m164_iter_index_split/verify.sh
step "M165 求值顺序 = 词法左→右（第 51 轮 · 缺陷 178/179 —— C 轨实参顺序 / 解释轨赋值顺序）"
# 背景：C 标准**未定义**实参求值顺序（gcc 实测右→左），而解释轨（AST 序）/ VM 轨（字节码序）
#   都是左→右 ⇒ 一个 C 表达式里 ≥2 个带副作用的操作数就三轨分叉。实测修前：
#   `f() + g()` → C 轨 `G,F`；`"${f()}${g()}"` 同；
#   `print("A=" + str(l.pop()) + " len=" + str(len(l)))` → C 轨 `len=3`（解释/VM 轨 2）。
#   赋值侧：`o[k] = v` 解释轨 `V,O,K` 而 VM 轨 `O,K,V`；`o.f = v` 解释轨 `V,O` 而 VM 轨 `O,V`。
# 修法：统一为**词法左→右**（对齐 Go 规范）—— C 轨 ≥2 个含副作用的操作数按序提升为临时变量
#   （语句表达式；接线点 17 处，短路运算不接线）；解释轨 `i_assign` 收右值 AST、按
#   「目标对象 → 键 → 右值」求值。VM 轨无需改动（字节码本身即左→右）。
# 判据：三轨 stdout 逐字节一致（24 断言，副作用日志直接观测顺序）+ 4 道负控
#   （关闭全部序列化 ⇒ 11 项红 / 仅 Binary 站点 ⇒ 6 项红 / 仅方法站点 ⇒ 1 项红 /
#    仅索引赋值站点 ⇒ 2 项红 —— 每道必须判红 + 逐字节还原）。
run m165_eval_order bash examples/m165_eval_order/verify.sh
# M166（第 52 轮 · 缺陷 176）：**迭代期间修改被迭代容器 ⇒ R1003**（三轨同码同文）。
# 背景（修前**四类**分歧，同一份源码）：`for x in l { l.append(9) }` 解释/C 轨按活长度多迭代
#   （泄漏新元素）vs VM 轨按快照；`for x in l { l.pop() }` VM 轨越界 rc=1；`for k in d { d.set }`
#   解释/VM 轨键快照 vs C 轨活长度；`for k in d { d.remove }` VM 轨越界 rc=1。
# 语义裁定：for-in / 推导式**进入循环时快照长度**，迭代期间长度变化（增或减）⇒
#   `R1003 迭代期间被迭代容器长度变化: n0 → n1`（与 Python dict 的 RuntimeError 同向，
#   只是把 list 也纳入 —— 响亮优于静默，同 R1008 / M163 键严格化）。
# 判据：① 合法侧 18 断言三轨逐字节一致；② 6 类修改用例 × 三轨（rc≠0 + R1003 + 统一词条）；
#   ③ 负控 3 道（VM 去掉 ITERLEN / C 去掉 px_iter_ck / 解释轨 i_iter_ck 放行 —— 各自独立判红）。
run m166_iter_mutate bash examples/m166_iter_mutate/verify.sh
# M167（第 53 轮 · 缺陷 180/182）：**解包（destructuring）统一**。
#   ① 新语法：语句形式 `for a, b[, c…] in xs:`（与推导式**同语义**：逐元素解包）——
#      此前语句形式 parser 直接 E2001（只有推导式支持解包），而 Go 的 `for k, v := range m`
#      是最常用迭代形式 ⇒ 语句/推导式不对等。AST 里单变量仍是**字符串**（既有 golden 不变）。
#   ② 三轨解包诊断统一（修前**三轨四种行为**）：元素非 list/tuple ⇒ 解释轨
#      `R1002 推导式解包需要…` vs VM/C 轨 `R1003 字符串索引越界`；长度不足 ⇒ 解释轨**静默给 null**
#      （错值）/ VM「列表索引越界」（无 R1003 前缀）/ C「R1003: 列表索引越界」；长度超出 ⇒ 三轨
#      都静默忽略。现统一为：长度必须**恰等于**变量个数，否则 `R1002 解包需要 list/tuple，实际是 <t>`
#      / `R1002 解包需要 N 个元素，实际是 M`（响亮优于静默，同 R1008 / M163 / M166）。
#   ③ `dict.items()`（`[[k, v], …]` 插入序快照）⇒ `for k, v in d.items()` = Go `for k, v := range m`；
#      dict 直接迭代只产键 ⇒ `for k, v in d` 响亮报 R1002（不静默）指引用 `.items()`。
#   ④ 顺带收口**缺陷 182**（VM 轨 · 循环变量被闭包捕获）：装箱变量必须走 CELLSET 而非直写槽
#      （直写冲掉 cell ⇒ `R1002 此类型不支持索引: int`；值为字符串时恰好返回自身 ⇒ **静默碰对**）。
# 判据：① 合法侧 16 断言三轨逐字节一致；② 8 类严格性用例 × 三轨（rc≠0 + R1002 + 统一词条）；
#   ③ 负控 3 道（VM 去 UNPACKCK / C 去 px_unpack_ck 发射 / 解释轨类型检查放行 —— 各自独立判红）。
run m167_unpack bash examples/m167_unpack/verify.sh
# M169（第 54 轮 · 缺陷 181 + 同族 183/184）：**模块体 / 闭包帧的「绑定归属」统一**。
#   一条规则：① 模块体（顶层**含嵌套块**）里的所有绑定 = **模块级全局**（模块体没有块作用域）；
#   ② 帧（函数/闭包体）捕获它们 ⇒ **按引用**；③ **闭包体也是帧**（体内 hoist + 装箱预扫描 ——
#   VM 轨 M160 已有、C 轨此前两者都缺）；④ 帧内**声明式**绑定 = 帧局部（遮蔽模块全局），
#   **赋值式**绑定命中模块绑定则写模块全局、否则也是帧局部。
#   修前实测（同一份源码**三轨四种行为**，含编译失败与静默错值）：
#     `for p in [1,2,3]: 0` 后 `print(p)` → 解释/VM 3 · **C 编译失败**（`'_v3' undeclared`）；
#     `if true: let x = 42` 后 `print(x)` → 解释/VM 42 · **C 编译失败**；
#     模块体 for 后由**函数**读循环变量 → 解释 8 · **VM/C 未定义变量**；
#     顶层闭包捕获顶层循环变量 → 解释**引用** 20/20 · **VM R1002 索引错** · **C 按值 10/20**；
#     lambda 体 for 后引用 → 解释/VM 9 · **C 编译失败**；
#     闭包内闭包捕获外层闭包局部 → 解释/VM `[1,2,2]` · **C `[1,2,0]`**（按值 ⇒ 写丢失）；
#     帧内赋值未声明名 → 解释「未定义变量」· **VM/C 全局写**（惰性 `bc_is_global` 顺序敏感）。
# 判据：① 15 断言三轨 stdout 逐字节一致；② 3 类严格性用例 × 三轨（rc≠0 + 「未定义变量」+
#   程序自身输出逐字节一致）；③ 负控 4 道（VM for 退回帧槽 / C for 退回就地声明 /
#   C 闭包体退回不 hoist 不装箱 / C 帧内 hoist 判据退回旧口径 —— 各自独立判红）。
run m169_toplevel_scope bash examples/m169_toplevel_scope/verify.sh
# M170（第 55 轮 · 用户报障【清歌】）：**native 桥的 precise GC 根面收口**（缺陷 187 + 同族 188/189/190）。
# 现场：`sqlite_query` 返回的行 dict 在 GC 第一次回收后被写坏 —— 默认阈值 n=16388 崩（3/3 一致）、
#   PX_GC_THRESHOLD=1000 ⇒ n≈23；GC 关 ⇒ 15 万轮零破坏 ⇒ 病灶在运行期 GC（precise 根面漏登记）。
# 同族四处：① sqlite/xml/onnx/rsa/h3 桥的容器未登记；② longjmp 落点**野根**（隔离点不归还登记
#   深度）⇒ 新增 px_root_depth/px_root_restore，5 个 setjmp 落点全接；③ native 建表期回收
#   （g_gc_frozen）；④ QPACK 解码 `val` 未登记 ⇒ 值退化为名字（只在 stress 下必现）。
# 新检测器 PX_GC_STRESS=1（每次分配即 GC）—— 把「靠阈值凑巧发作」变成「必然发作」。
# 判据：6 层正判据 + 4 道负控（A 去 sqlite PX_KEEP / B xml 登记滞后 / C restore 换 if(0) /
#   D 去 QPACK val 登记 —— 各自独立判红 + sha256 逐字节还原）。191 刻意不配负控：实测其症状
#   **时序相关**（去 KEEP 后 5 连跑：1 丢头 / 3 绿 / 1 SIGSEGV）⇒ 按纪律**不设假负控**，
#   改由 ⑦ 层正判据锁症状（修前 stress 下 core dump，正向复现 2/2）。
run m170_gc_bridge bash examples/m170_gc_bridge_root/verify.sh
# M171（第 56 轮 · 台账缺陷 116 + 晨曦 QA 清单）：**lint 作用域模型对齐 spec §17**。
# 现场：`px lint` 对**合法**程序报大量假 L002 —— 闭包/嵌套 def 捕获外层帧局部、
#   赋值式绑定（无 var/let）、推导式变量、模块体的嵌套块绑定、嵌套 def 互递归、
#   上一行/下一行的模块级绑定、`.px_modules` 包布局导入，**六类全误报**
#   （全仓实测：151 个文件 3651 条 L002 → 21 个文件 801 条；`stdlib/collections.px`
#   46 → 0、`examples/webapp/*` 全清）。根因 = lint 的作用域模型与 M169 定下的
#   「帧归属规则」不一致（帧 = 函数/闭包体；帧内声明式绑定帧顶 hoist；赋值式绑定
#   就地声明；内层帧可见外层帧）。
# 判据：① 合法侧 5 例（lint 0/0 **且** 三轨 stdout 逐字节一致 —— 只改绿 = 漏判，
#   只三轨一致 = lint 仍假红，两件事必须同时成立）；② 真阳性侧 2 例（读未绑定名仍报
#   L002 / 声明未读仍报 L001 —— 防「一律不报也是 0 错」）；③ 仓内 8 个真实文件 0 错误；
#   ④ 负控 3 道（顶层帧顶 hoist / 推导式变量声明 / 外层帧可见链 —— 各自独立判红）。
run m171_lint_scope bash examples/m171_lint_scope/verify.sh
# M172（第 56 轮 · 台账缺陷 186）：**运行期诊断的通道与措辞统一**。
# 修前实测（三轨四种行为）：`def f(): return zzz + 1` 解释轨把诊断写 **stdout**
#   （`运行时错误: 错误 [R1001] 2:12: 未定义变量: 'zzz'`）、编译轨写 stderr
#   （`运行时错误 [f 行2]: 未定义变量: zzz`）；`def main(): return Err("boom")` 解释轨
#   同样写 stdout。⇒ ① 诊断混进**产物通道**（与 Issue 45/51 及 engine_parity 判据 B
#   「失败时 stdout 必须为空」同轴相悖）；② 三轨措辞不同 ⇒ 自动化无法按错误码判。
# 修法：解释轨三处诊断出口改 `print_err`（stderr）· 编译轨 `未定义变量` 两处（runtime.c
#   px_get_global / vm.c GETG）补 `R1001:` 前缀与引号 · `print_err` 先 `fflush(stdout)`
#   （否则诊断在 `2>&1` 合并流里会跑到程序输出**前面**）。
# 判据：① R1001 三轨（rc≠0 · stdout 恰为程序输出 · stderr 含 R1001 与 `未定义变量: 'zzz'`）；
#   ② main→Err 三轨（rc≠0 · stdout 空 · stderr 含「错误: boom」）；③ 正常程序 stderr 0 字节；
#   ④ 真·合并流顺序（首行必须 `out-1`）；⑤ 负控 2 道（通道 / 措辞，各自独立判红 + 还原复绿）。
# 诚实边界：**位置前缀**保留各轨最优信息（解释轨行列 / 编译轨函数+行），判据只断言
#   「同通道 + 同错误码 + 同消息体 + rc」，不按整行对拍。
run m172_diag_channel bash examples/m172_diag_channel/verify.sh
# M173（第 57 轮 · 晨曦 QA 清单 P1-2 + P1-1）：**HTTP 反代/静态两件**。
# P1-2：`vhost(host, handler)` 分支**没有 gzip 判定** —— 压缩只在 ① px_serve 原生静态分支、
#   ② `.px` 脚本响应分支存在 ⇒ 所有 vhost 站点（Mahesvara 全部站点）文本响应明文下发
#   （晨曦实测 3408B 页面无 Content-Encoding / 无 Vary，白耗 3~5× 带宽，CDN 回源同步放大）。
#   修法：在 px_vhost_respond 复用同一套 px_resp_gzipable + px_gzip_compress；
#   两个不压条件（handler 已自带 Content-Encoding ⇒ 防双重压缩 / 204·304 无体）。
# P1-1：池连接**复用时不重设** SO_RCVTIMEO/SO_SNDTIMEO（只在新建连接时设）⇒ 先大超时建池、
#   之后小超时失效（反代无法「按路径收紧超时」）。修法：HPoolSlot 记 to_ms，复用前比对，
#   不一致才重设（TLS 走 mbedtls_ssl_conf_read_timeout）。
# 判据（VM+C 双轨；HTTP 服务端示例含 spawn，解释轨不支持 ⇒ 与 m23c/m31 同口径）：
#   ① 用例 A 双轨（**看线上字节**：裸 TCP + hex 断言「头结束符后紧跟 gzip 魔数 1f8b」，
#      因为 http_request 客户端会自动 gunzip，只看头验不出真压没压）· 三轨共 12 断言；
#   ② 用例 B 双轨（大超时建池 → 300ms 复用必须报错且耗时 < 1.5s；再反向放大超时恢复成功）；
#   ③ 负控 2 道（关 vhost gzip 块 / 关复用重设块 —— 各自独立判红 + sha256 逐字节还原复绿）。
run m173_http_proxy bash examples/m173_http_proxy/verify.sh
# M174（第 57 轮 · 晨曦 QA 清单 P2-7 + 新登记缺陷 194）：**`d.get(k[, default])` 的语义统一**。
# 定调（文档 + 解释轨既有）：默认值**只覆盖「键不存在」**；键存在但值为 null ⇒ 返回 null
#   （= Go 两值语义 / Python dict.get）。出处 docs/DICT_STRICT_MIGRATION.md + 速查表。
# 病灶（缺陷 194）：runtime 侧用 `px_is_null(取出的值) && nargs >= 2` 判存在性 ⇒
#   「键存在但值为 null + 给了默认值」时 VM/C 轨返回默认值、解释轨返回 null ⇒ 三轨分叉，
#   且与文档相悖。修法：改用 `px_dict_has` 判存在（一个 if 的事，但是三轨真相的开关）。
# 判据：① 语义矩阵 12 条三轨逐字节一致（含「键存在 + null + 默认值」定点）；
#   ② 缺键下标读三轨 rc≠0 + R1008 + 统一词条；③ `get` 非字符串键三轨 rc≠0 + R1002 + 统一词条；
#   ④ 负控（runtime 改回 null 判定 ⇒ VM 轨必须与解释轨不一致；sha256 逐字节还原复绿）。
run m174_dict_get bash examples/m174_dict_get/verify.sh
# M175（第 57 轮 · 台账小项收口）：**`len(bytes)`（缺陷 153）+ `os_popen` 的 stderr 去向（缺陷 14）**。
# 153：`len(bytes)` 落进 `px_len` 的 default ⇒ `len 不支持类型 bytes`（只能用 `bytes_len`）；
#   而解释轨的 `i_builtin_len` **本来就写了 bytes 分支**（`len(args[0])`）—— 那一步调到的正是
#   同一个 runtime 函数 ⇒「同一份意图、两处实现」的又一例。定调：**每类型按自然单位**
#   （str = rune 数；bytes = 字节数）。
# 14：`os_popen` 子进程只 dup2 了 0/1、**fd 2 继承宿主** ⇒ 子进程 stderr 漏进宿主（同族的
#   os_capture/os_spawn_capture 一直分离捕获）。修法：第 3 参 `opts{"stderr":"inherit"（默认）/pipe/null"}`
#   —— 纯增量：不传 opts 时逐字节保持旧行为（门用「宿主 stderr 里**应当**出现 CHILD-ERR」反证）；
#   `pipe` ⇒ 返回值多一个 `stderr_fd`；`null` ⇒ /dev/null。解释轨的 .px 桥同步透传第 3 参
#   （取值校验只在 runtime 一处，不由桥复制）。
# 判据：① `len(bytes)` 语义矩阵 12 条 × 三轨逐字节一致；② B1/B2/B3 三轨 + **宿主 stderr 侧**
#   反向判据；③ 非法 opts 取值三轨 rc≠0 + R1002 + 枚举文案；④ 负控 2 道（去 px_len 分支 /
#   去 stderr 开关 —— 各自独立判红 + sha256 逐字节还原复绿）。
run m175_bytes_stderr bash examples/m175_bytes_stderr/verify.sh
# M176（第 57 轮 · 晨曦 QA 清单 P1-5）：**零停机换二进制**（SO_REUSEPORT）。
# 病灶：监听套接字只设 SO_REUSEADDR（TIME_WAIT 可复用），两个进程**无法同时监听同一端口**
#   ⇒ 每次换二进制必有空窗（晨曦三节点实测 352ms / 75ms / 249ms）。
# 修法：`opts{"reuse_port": true}`（px_serve / http_serve 有 opts）或 `PX_REUSE_PORT=1`
#   （全局开关，覆盖 sse_serve / tcp_listen 这类无 opts 的原语）；失败**响亮报错**
#   （内核不支持就不能假装设上了）。bind 失败文案补 strerror（换二进制最常见的失败是
#   EADDRINUSE =「对端没开 SO_REUSEPORT」，只说「绑定失败」会让人查错方向）。
# 升级链：启动新进程（同端口）→ 健康门 → SIGTERM 旧进程（M27 起「停 accept + 等在途请求」）
#   = 真零停机。⚠️ 内核要求**双方都 opt-in** ⇒ 旧版也必须带此开关启动（门把这条**测出来**）。
# 判据：① A(reuse=1) 与 B(reuse=1) **并存**；② SIGTERM A 后 B 仍服务；
#   ③ **700 次高频探测 refused=0 bad=0**（这才是"零停机"的定义）；
#   ④ 反向语义：C(reuse=0) 在 B 监听期间 **bind 必须失败**（把内核契约变成判据）；
#   ⑤ 负控（px_sock_set_reuseport 变 no-op ⇒ B 起不来）+ sha256 逐字节还原复绿。
run m176_reuseport bash examples/m176_reuseport/verify.sh
# M177（第 57 轮 · 缺陷 165 的审计发现）：**内置面统一**。把「GenExp 物化路径」按矩阵展开
#   （内置 × 实参形态 × 三轨）后跳出来的是一族结构性缺口：
#   ① **解释轨有、编译轨没有**的内置 `dict()` / `unique()` / `flatten()` ⇒ 照解释轨写、
#      一上编译轨 `R1001 未定义变量`；而 lint 名册取**并集**、门 ③④ 只看「⊆ 名册」⇒ 全绿到真编译。
#   ② `min(x)`/`max(x)` 单参数：编译轨**静默返回实参本身**（`min(gen)` 得到生成器对象）；解释轨报「需要两个参数」。
#   ③ `sum`：编译轨只收 list、解释轨收 list/tuple。
#   修法：runtime 补三个内置 · `min/max` 单参数可迭代取元素最值 · `sum` 收 list/tuple/生成器；
#   返回 TRUE 的公共助手 `px_as_list`（= `px_len`+`px_iter_at`，与 for-in 同源）；解释轨对称补齐
#   （min/max 变 variadic、新增 `i_seq_of` 规范化生成器 —— 解释轨的生成器是 dict{"__gen__"}，
#    直接交给 runtime `list()` 会迭代出**键名**，实测 `sum(gen)` 报 `+ 不支持: int + string`）。
#   名册门新增判据 ⑦「解释轨注册名 ⊆ runtime 可达名」—— 把这类差从**无人可测**变成**门能红**。
# 判据：① 20 条内置面矩阵三轨 stdout 逐字节一致；② 真不支持的类型/空可迭代 ⇒ 三轨同码同文；
#   ③ 名册门 ⑦ 全绿；④ 负控（min/max 单参数改回直接返回 ⇒ 两轨必须不一致；sha256 还原复绿）。
run m177_builtin_parity bash examples/m177_builtin_parity/verify.sh
# M178（第 57 轮）：**可迭代实参统一** —— join / sorted / reversed / contains 的实参面与文案。
#   修前同一份源码三轨四种行为：
#   ① `join("-", gen)`：解释轨报**与 join 无关**的「字典索引键必须是字符串」（直接 args[1][i]，
#      而解释轨的生成器是 dict{"__gen__"}）vs 编译轨 `1-2-3`；
#   ② `sorted((3,1,2))`/`reversed((1,2,3))`/`contains((1,2,3),2)`：编译轨只收 list，解释轨文案各异；
#   ③ `reversed("中文")`：解释轨 `[文, 中]`（list of rune）vs 编译轨**按字节反转** ⇒
#      **非法 UTF-8**（渲染乱码）—— 不只是分叉，是**坏值**；
#   ④ `join("-","abc")`/`sorted("cab")`：解释轨 reject vs 编译轨 accept（str 经 px_as_list 混进来）。
#   一条真相：可迭代实参 = list/tuple/生成器/**字符串**（str 按 rune）；`reversed(str)` → str
#   （rune 级反转）；拒绝文案统一 `<名> 参数需要 list/tuple/生成器/字符串，实际是 <t>`。
# 判据：① 31 行矩阵（4 形态 × 4 入口）三轨 stdout 逐字节一致；② 12 行 str 面（含 emoji/NUL）一致；
#   ③ 4 个真不支持类型 ⇒ 三轨 rc≠0 + stdout 恰为 before + R1002 + 同文案；
#   ④ 登记项（不计失败）缺陷 195；⑤ 负控 3 道（runtime 字节反转 / 解释轨收回 list / join 拒生成器）。
run m178_iterable_args bash examples/m178_iterable_args/verify.sh
# M179（第 57 轮 · 缺陷 195/196）：**运算族收口** —— 码 + 措辞三轨统一 + 静默坏值。
#   病灶：runtime 数值分支直接用 `num_val()`（对非数值读 union 的 `as.f` = 指针位模式，**UB**）
#   ⇒ `1.0 * "x"` 两轨两个不同垃圾值（6.905e-310 / 6.952e-310）、`1 / "x"` ⇒ inf、
#   `2 ** "x"` ⇒ 1.0、`1.0 + "x"` ⇒ 1.0；`1 < "x"` 编译轨静默 true（按类型名字典序）；
#   位运算/索引位置对 float 静默截断；整数除零无码；索引越界三轨三种文（VM 轨连码都没有）。
#   一条真相：算术要求数值（`str*int` 例外 = 重复）、次序比较要求「双数值或同类型」、
#   位运算与索引位置要求 int、整数除零 `R1006`；`sorted`/`min`/`max` 用**全序内部比较器**。
# 判据：① 28 行合法侧矩阵三轨逐字节一致；② 20 个错误用例三轨同码同消息体 + stdout 恰为 before；
#   ③ 负控 3 道（num_val UB 恢复 / 解释轨文案退回 / 跨型比较放行）。
run m179_arith_diag bash examples/m179_arith_diag/verify.sh
# M180（第 58 轮 · 晨曦 QA 清单 P0-3/P1-4）：**HTTP/2 口径落地 + HTTP/3 能力自证**。
#   口径一句话：**h2 不做（长期）；h3 是唯一现代路线且在 x86_64 默认构建里已可用**
#   （全文 docs/HTTP2_DECISION.md）。修前的真实行为比 QA 报的更糟一档：
#     · `Upgrade: h2c` ⇒ 进演示帧层 ⇒ **200 + 固定演示页**（无视 docroot/handler）= 静默错内容；
#     · `PRI * HTTP/2.0` 前导 ⇒ 同上；`--no-quic` + opts.http3 ⇒ **静默忽略**（H3 没开也不报错）。
#   落地：h2c **忽略升级按 h1.1 服务**（RFC 7230 允许）· 前导 ⇒ **505 + 说明** ·
#   删掉不可达的 ALPN-h2 死代码（ALPN 全线只声明 http/1.1）· h2 演示帧层收进
#   `opts{"h2_demo": true}`（默认关）· `--no-quic` 下要求 H3 ⇒ **响亮报错** ·
#   `px build` 收尾打**能力行**（quic=on/off，走 stderr，stdout 逐字节不变）。
# 判据：① h2c 升级拿到真实内容且无演示页；② 前导 ⇒ 505；③ --no-quic+http3 ⇒ rc≠0 同文案；
#   ④ 能力自证（默认构建含 ngtcp2/Alt-Svc，--no-quic 不含 + 能力行 quic=off）；⑤ 负控 1 道。
run m180_h2_h3_stance bash examples/m180_h2_h3_stance/verify.sh
# M181（第 59 轮 · 缺陷 193）：**帧内绑定「未初始化即读」三轨统一**。
#   病灶：帧局部槽初值原是 `px_null()`/calloc 零值，与「已声明且值为 null」**不可区分**
#   ⇒ `def f(): print(x); let x = 42` 解释轨报 `R1001 未定义变量: 'x'`，VM/C 轨**静默给 null**。
#   同族：条件分支未走、空循环的循环变量、while 体 0 次迭代、`let x = x + 1`（修前编译轨报
#   与根因无关的 `R1002 无法相加: null + int`）、以及**帧内声明遮蔽同名模块/外层绑定**
#   （解释轨穿链读到外层值 —— M181 同批把解释轨也收到同一条真相：帧入口登记「已声明未执行」）。
#   一条真相：stderr · `R1001` · `未定义变量: '<名>'` · rc≠0；**初始化单调** ⇒ 声明执行过之后
#   的任何读都合法（闭包先创建、后声明、再调用 = 合法程序，必须继续给 1）。
# 判据：① 合法侧 7 例三轨逐字节一致；② 严格性 8 例 × 三轨同码同消息体（诊断走 stderr）；
#   ③ 负控 3 道**各自独立判红**（VM 去 UNINIT / C 轨 hoist 退 px_null / 解释轨不登记）。
run m181_uninit_read bash examples/m181_uninit_read/verify.sh
step "M182 门（第 60 轮 · native 桥「登记窗口」· 缺陷 192 + 同族）"
# 主题：M170 立了「容器创建后必须登记」，**没管住「登记之前的那段窗口」** —— 尤以
#   **返回值 / out-param** 形态为最：值的生命从**被调用方**的登记帧里出来，调用方接手的
#   那一瞬间是**裸的**（`h_exchange` 返回前已 `px_root_pop()` 自己的帧）。
#   一条真相（第一条硬约束的补句）：**登记作用域必须建立在该值的第一个「跨分配窗口」之前**。
# 机制（为什么长期只在调试开关组合下发作）：`px_alloc` 里
#   `deferrable = (g_active_threads > 0) && !g_gc_force_inline` —— 多线程服务模式默认把 GC
#   延迟到安全点，期间窗口被「稍后的登记」补上 ⇒ **靠运气遮住**；`PX_GC_INLINE=1` 强制内联
#   ⇒ 窗口必现。实测对照：基线绿 · 单开 STRESS 绿 · 单开 INLINE 绿 · **双开 3/3 红**。
# 判据：① m23c 压力档整门通过（修前 R1008 丢 X-Test 3/3）；② 探针（json_path_set 三路 +
#   本仓**第一个 http_unix 成功路径**用例）基线 vs 压力档逐字节一致；③ 负控 A/B/C 各自独立判红。
run m182_hdr_root bash examples/m182_hdr_root/verify.sh
# M183（第 61 轮）：**根栈「交棒窗口」**+ native 桥漏登记（缺陷 197/198/199）。
#   197 = px_root_pop 的出口协作式安全点恰落在「返回值既不在本帧也不在调用方帧」的缝里
#         （`bi_http_request` 的 headers 被回收 ⇒ d["headers"]=d 自引用环）；修法 = 延迟收缩。
#   198/199 = `bi_s3_list` / `bi_px_exec` 的容器局部从未登记（可被新检测器 PX_GC_UAFDET 直指）。
# 判据：① handover 在 STRESS 下连跑 10 次全绿；② s3flow(STRESS+INLINE+UAFDET) 全绿；
#   ③b m32_hot_reload 压力档无 UAFDET；④ PX_GC_TRACE 硬不变量无命中；⑤ 根栈峰值有界；
#   ⑥ 负控 A/B/C/D 各自独立判红。
run m183_gc_root_handover bash examples/m183_gc_root_handover/verify.sh
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
# M168（用户报障）：安装脚本的发行版矩阵（openEuler 认脸 + 字面目录 + 不支持组合给替代路线）
run pkg_install_rpm bash packaging/selftest_install_rpm.sh
# M168：**二进制可移植性门** —— 入库件必须全静态（动态件在老 glibc 上"装得上跑不起来"）
run portability_selftest bash selfhost/check_bin_portability.sh --self-test
run portability_shipped bash selfhost/check_bin_portability.sh --arch x86_64 --require-static bootstrap/*
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
