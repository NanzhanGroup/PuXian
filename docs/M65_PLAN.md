# M65_PLAN —— LSP / MCP 自举（spec §12 工具链最后两项 + AI agent 协议）

> 状态：执行中 —— M65-S1/S2 完成（2026-09-04），S3 待开工
> 关联：docs/spec.md §12/§12.1、docs/M64_PLAN.md（M64d 按需立项前置侦查）、
> archive/rust-compiler/src/lsp.rs (36618B)、mcp.rs (13409B)（只读语义参考，已退役）
> 立项依据：M64 收尾确认「runtime fd stdin/stdout 原语已具备 → lsp/mcp 按需立项新里程碑」

## 1. 背景与现状（侦查 2025-09-05，全部实测）

spec §12 定义 8 项工具 + 1 协议节：`fmt`/`lint`/`test`/`doc`/`bench`/`ast`/`pkg`/`lsp` + §12.1 MCP。
前 7 项已由 PuXian 自举（M45 pkg、M64a–c fmt/lint/test/doc/bench、astdump.px）；**lsp / mcp 为最后两项**。

Rust 版语义参考（archive，只读）：
- **lsp.rs**（36618B）：JSON-RPC 2.0 over stdio（自研 Content-Length 帧解析 + json.rs）。
  能力：生命周期 initialize/initialized/shutdown/exit；文档同步 didOpen/didChange/didSave/didClose；
  诊断 publishDiagnostics（词法错误 + 语法错误 + lint L001–L008）；completion（关键字 + 内置函数 +
  std 函数 + 用户符号 + 局部变量）；definition（def/struct/enum/trait/impl/顶层 var）；hover（签名/类型/文档注释）。
  Pos 1-based → LSP Position 0-based（line/character 各 -1）。文档缓存 uri → (Program, 诊断)，didChange 失效。
- **mcp.rs**（13409B）：MCP 2024-11-05，stdio transport（LSP 同款 Content-Length 帧）。
  tools/list 暴露 8 工具：run/fmt/lint/test/bench/doc/ast/version（file|code 二参）；tools/call 执行。
  **子进程调用自身 CLI 执行工具，避免污染协议 stdout 通道**（自举版需等效方案，见 §4）。

### 自举侧能力侦查结论（本里程碑可行性基座）
| # | 能力 | 现状（实测） | 对 LSP/MCP 的意义 |
|---|---|---|---|
| 1 | JSON 编解码 | ✅ runtime 内建 `json_parse`/`json_stringify`/`json_path`/`json_path_set`（dict/list/str/int/float/bool/null 全类型） | JSON-RPC 消息编解码零依赖 |
| 2 | stdio 双工 | ✅ `read(fd, maxlen) → bytes`（read(2) 直通阻塞；EOF=空 bytes；失败 int -1）；`write(fd, data) → int`；fd 0/1/2 = stdin/stdout/stderr | Content-Length 帧收发可行（服务器长驻阻塞等消息即 LSP 模型） |
| 3 | stdin 行读 | `input([prompt])` 存在但为 fgets 4KB 上限 | 仅作辅助，帧解析用 read(0,…) 自建缓冲 |
| 4 | os_spawn | `os_spawn(cmd,args)→pid`、`os_wait(pid)→rc`，**fork+execvp 无管道/重定向，子进程 stdout/stderr 继承终端**（pxtest 已注） | ⚠️ MCP run/test/bench 需捕获子进程 stdout 时**缺原语** → 见 §4 runtime 补丁点 |
| 5 | 词法/语法诊断 | ✅ 自举 parser.px 可 import 复用（pxlint 已如此）；错误带文件:行:列 | analyze 复用，诊断直接可得 |
| 6 | lint 诊断 | ✅ lint_core.px 纯 defs（L001–L008）+ pxlint `--json`（机器可读） | publishDiagnostics 数据源 |
| 7 | 符号/行定位 | ✅ pxslice.pp_collect 顶层块文本级扫描（def/struct/enum/trait/impl/import）；pxdoc 已实现「`##` 注释紧跟 def → 定义行定位」 | definition/hover 的符号 → 位置兜底（文本级行扫） |
| 8 | 关键字/内建清单 | pxlint KEYWORDS/BUILTINS 集合已维护（M64 收尾补 32 QUIC/H3） | completion 静态候选源 |
| 9 | 子进程 stdin 注入 | os_spawn 无 stdin 管道参数 | MCP run(code) 无法直接喂 stdin → 临时文件路径方案 |

### 关键架构事实（复用 M64 约定）
- 工具模式：`.px 源 → bootstrap/pxc 编译 → bootstrap/pxNN 二进制 → tools/pxc 薄壳子命令`。
- **pxc 薄壳直通**：bash `exec` 替换进程保留 fd 0/1 → `pxc lsp` / `pxc mcp` 可直接作 LSP/MCP 服务器入口
  （编辑器/agent client 以子进程拉起，stdio 即管道）。
- 工具 = 纯 defs 核心文件（fmt_core/lint_core/pxslice 惯例，import 合并，可被另一 .px 复用）+ CLI 薄壳。
- 双模式：pxi（解释）+ pxc（编译 C 静态）行为一致；动 runtime C 须 pxi 白名单同步 + capability 双模式回归。
- .px 单文件 >500 行拆分（M64 惯例）。Rust lsp.rs 36KB（≈1000+ 行）→ **必须拆多文件**。

## 2. 目标

以 PuXian 自身实现 spec §12 最后两项（lsp/mcp），完成工具链全自举 + AI agent 协议底座：
- **LSP 服务器**（pxc lsp）：与标准 LSP client 握手 → 诊断实时推送 → 补全/跳转/悬停；
- **MCP 服务器**（pxc mcp）：AI agent 经 MCP 直接调用 px 工具链（run/fmt/lint/test/bench/doc/ast/version）；
- 语言分析能力（parser/lint/符号表）以模块复用方式内嵌，不依赖子进程（除必须执行的 run/test/bench）；
- 消灭对 Rust lsp.rs/mcp.rs 的依赖（archive 只读留档）。

## 3. 交付物布局

| 文件 | 角色 | 说明 |
|---|---|---|
| `tools/jsonrpc_core.px` | **共享底座（纯 defs，先做）** | Content-Length 帧 read_message/write_message（read(0) 累积缓冲按 header/body 精确取帧）+ JSON-RPC 骨架：request→result / notification→无回 / error(-32700 解析/-32601 方法/…)、id 提取、method 分发表 | 
| `tools/pxlsp.px` | LSP 服务器（纯 defs 分析核心 + main 循环薄壳） | 能力对齐 Rust lsp.rs（生命周期/同步/诊断/补全/跳转/悬停），>500 行再拆 lsp_core.px |
| `tools/pxmcp.px` | MCP 服务器 | 对齐 mcp.rs（tools/list 8 工具 + tools/call） |
| `bootstrap/pxlsp`、`bootstrap/pxmcp` | 编译产物（静态） | pxc build tools/pxlsp.px（--version/-h） |
| `tools/pxc` | 增 `lsp`/`mcp` 子命令 | exec 直通，fd 保持（服务器入口） |
| runtime | **条件补丁：`os_spawn_capture`**（S1 决策，见 §4） | 仅 MCP run/test/bench 捕获子进程输出需要；lsp 不需要 |
| `examples/m65_lsp/`、`examples/m65_mcp/` | 握手 + 能力 verify | 模拟标准 client（见 §5 验收） |
| docs | M65_PLAN + 收口同步 | spec §12 勾选全绿、ROADMAP 工具链行、CHANGELOG、README |

## 4. 里程碑与验收（S 步，平铺编号）

### M65-S1：侦查 + JSON-RPC 共享底座（jsonrpc_core.px）
- 侦查确认：parser.px AST 是否带 Pos / 诊断位置来源（lint_core、parser 错误结构）；read(0) 字节级行为；
  bytes→str 转换原语（M23b bytes 模块现状）；pxc build 长模块编译时长；client 模拟工具可用性（python3 / bash coproc）。
- 决策：**runtime 是否补 `os_spawn_capture(cmd, args, stdin_text?) → [rc, stdout, stderr]`**
  （pipe+fork+dup2+waitpid 收集，≤100 行 C + pxi 白名单 + capability 双模式回归）；
  缺省此步一并实现，避免 M65-S4 再动 runtime。
- 交付 jsonrpc_core.px：帧读写（read_message 支持累积缓冲、半包/粘包、缺 Content-Length 报错 EOF 处理）+ JSON-RPC 骨架 +
  样例级自测（写入→读回回环；模拟两帧粘包）。
- 验收：jsonrpc 回环自测 PASS；`--check`/lint 新文件全绿；若补 runtime → diffcheck + capability 双模式 253 PASS 无回归。

### M65-S2：LSP 核心（生命周期 + 文档同步 + 诊断）
- analyze(uri, src)：词法/语法（import parser.px 复用）+ lint（import lint_core.px）→ 诊断列表
  （Pos→0-based；错误级别 error/warning 映射；去重）。
- 生命周期 initialize（serverInfo/capabilities：textDocumentSync=Full、completionProvider、
  definitionProvider、hoverProvider）/initialized/shutdown/exit；未知名方法 → MethodNotFound。
- 文档同步 didOpen/didChange(全量 text)/didSave/didClose + 文档缓存（uri→src + 分析结果），
  didChange 后失效重析 → publishDiagnostics 通知（含清空）。
- 交付 pxlsp.px（此步以 stdio 真实跑通：模拟 client 发 initialize + didOpen → 收到 publishDiagnostics）。
- 验收：握手 + 诊断推送端到端 PASS（**规范关键**：诊断内容 = 语法错误样例 + lint 样例逐一命中）。

### M65-S3：LSP 增强（completion / definition / hover）
- 符号表收集：顶层 def/struct/enum/trait/impl/顶层 var（定义行定位复用 pxslice 文本级扫描或 AST pos）+
  文档内局部 let/参数（文本/词法级扫描兜底）；detail 签名串（对齐 lsp.rs collect_symbols）。
- completion：关键字 + BUILTINS + 用户符号 + 局部变量；word_at 光标词；触发后排序去重。
- definition：光标词 → 符号定义行 → Location（0-based Range）。
- hover：符号 → 签名/类型；def 上方 `##` 文档注释并入。
- 验收：对 selfhost 真实文件（compiler.px 等）逐一验证补全候选命中/跳转行正确/hover 文本；错误输入不崩。

### M65-S4：MCP 服务器（pxmcp.px）
- 对齐 mcp.rs：initialize（protocolVersion 2024-11-05）+ tools/list（8 工具 + description + 参数 schema）+
  tools/call（按工具分派）+ 通知忽略 + error 处理。
- 工具执行策略（**不污染协议 stdout**）：
  - fmt/lint/doc/ast：code 或 file 输入 → **语言内直接调用核心**（import fmt_core/lint_core/pxdoc 相关 defs），
    返回文本/JSON 结果；无子进程。
  - version：返回版本串（env/inline）。
  - run/test/bench：file 或 code（写临时文件）→ 子进程 `os_spawn_capture(PX_PXI|PX_PXC, …)` 捕获输出回传
    （S1 补丁原语；无则文件重定向不可行——故 S1 决策必须前置）。
- 验收：模拟 MCP client initialize → tools/list（8 项）→ tools/call 每工具各一次成功回包 + 错误参数回 error。

### M65-S5：收口（全仓回归 + 文档 + 握手实跑）
- 全仓 dogfood：新 .px 文件 pxfmt --check / pxlint 全绿；pxc 冒烟（pxlsp/pxmcp --version）。
- **与模拟标准 client 最小握手实跑**（examples/m65_lsp/verify.sh、m65_mcp/verify.sh）：
  python3 subprocess（pipe 双向）首选 / bash coproc 备选，实测：lsp initialize→didOpen→publishDiagnostics、
  completion/definition/hover 请求各一；mcp initialize→tools/list→tools/call lint(fmt/run)。
- 收口文档：spec §12 实现状态表 + §12.1 勾选（lsp/mcp 完成）、ROADMAP 能力/工具链行、CHANGELOG、
  README 工具链、M64_PLAN M64d 状态行更新（已由 M65 承接完成）。
- 验收：verify.sh 全绿 + capability 双模式无回归 + 仓库收敛。

## 5. CI 集成
- ci.yml 冒烟/回归增加：pxlsp --version、pxmcp --version 存在性；m65_lsp/m65_mcp verify.sh（python3 可用时）。
- make_release.sh 冒烟同步。

## 6. 风险与缓解

| 风险 | 缓解 |
|---|---|
| read(0) 阻塞无超时、半包/粘包 | LSP 长驻模型本就阻塞等消息；jsonrpc_core 累积缓冲精确按 Content-Length 取帧（S1 回环含粘包样例） |
| print 污染协议 stdout | 服务器代码**禁用 print**；调试走 write(2)/stderr 或日志文件（lint 规则外人工审 + 头注释声明） |
| parser AST 无 Pos → 跳转/悬停定位难 | 兜底：pxslice 文本级符号行扫描 + fmtlexer keep token 带 line/col（pxdoc 已有先例）；S1 侦查定主方案 |
| os_spawn 无捕获 → MCP run/test/bench 拿不到输出 | **S1 前置决策补 os_spawn_capture**（唯一 runtime 补丁，双模式同步 + capability 回归）；不拖到 S4 |
| 编译/运行耗时长（模块大） | 拆多文件（jsonrpc_core/lsp_core/mcp_core）；wst-exec 后台编译；verify 用小样本 |
| 双模式不一致（补 runtime 后） | pxi 白名单 + pxc 同注册；capability 双模式 + diffcheck 回归 |
| 与真实 client 兼容性（vscode/claude） | 验收以协议规范帧为准（模拟 client 逐帧比对），真实 client 列后续可选外测 |
| .px >500 行 | jsonrpc_core.px / lsp_core.px / mcp_core.px 拆分（import 同构惯例） |

## 7. 本里程碑不做（边界）
- 不新增语言特性；不改 fmt/lint/doc/test/bench 已有行为（仅可能加 runtime 1 原语 os_spawn_capture）；
- 不做增量文本同步（didChange 全量 text，对齐 Rust 版实现）；不做 workspace/semanticTokens/格式化 on-type；
- 不做 LSP over TCP/WebSocket（仅 stdio）；MCP 仅 stdio transport（不实现 HTTP/SSE transport）；
- Rust lsp.rs/mcp.rs 不删除（archive 只读）；真实编辑器/agent 客户端集成留作后续外测项；
- benchmark 不在本里程碑（LSP/MCP 为交互式长驻，性能验收=握手/请求响应正确性）。

## 8. 交付物清单
- docs/M65_PLAN.md（本文件）+ 收口更新 spec.md §12/§12.1、ROADMAP.md、CHANGELOG.md、README、M64_PLAN M64d 行
- tools/jsonrpc_core.px、pxlsp.px、pxmcp.px（+ 超行拆分 lsp_core.px/mcp_core.px）+ bootstrap/pxlsp、pxmcp + tools/pxc lsp/mcp 子命令
- runtime：os_spawn_capture（S1 决策若补）
- examples/m65_lsp/（verify.sh + 样本）、examples/m65_mcp/（verify.sh + 样本）
- ci.yml / make_release.sh 更新

## 9. 执行约定
- S 步每完成调 task_checkpoint 记进度；verify 全绿才 commit；commit 信息 `M65-Sn: …`；
- 所有程序 --version；新 .px 通过 pxfmt --check + pxlint（0 错 0 警）才入库；
- 动 runtime 的步骤先行 diffcheck --all + capability 双模式回归再继续。

## 10. S1 执行记录（M65-S1 running，2026-09-04）

### 10.1 侦查结论（立项侦查表未覆盖的实测新增，直接影响 S2–S5 架构）
| # | 发现（实测） | 架构影响 |
|---|---|---|
| A | **selfhost parser.px/pxlexer.px 语法/词法错误 = `perr`/`l_err` → print + `panic` → C 层 px_error noreturn 杀进程**（不可捕获）；fmtlexer 同 | **LSP 服务器进程内 analyze 遇到语法错误会崩** → 深度诊断（parse+lint）必须子进程化（pxcheck 独立诊断器，S2 引入），pxlsp 用 os_spawn_capture 捕获其输出解析诊断；词法预检也走同路 |
| B | `json_parse` 遇非法 JSON **抛错杀进程**（"json: 无法解析"），非返回 null | jsonrpc_core.jr_pull 对 body 先验首字符 `{`（JSON-RPC 消息必为对象）再 parse，非对象直接 `__jr_parse__` 不触雷；`{` 开头畸形 JSON 仍违约终止（真实 client 不发，留档） |
| C | **`len(str)` 是码点数、str 内部存储为 UTF-8 字节**（`len("中a")==2` 而字节 4）；`bytes_len(bytes(s))` 才是字节数 | **Content-Length 定界必须 bytes 精确**：输入缓冲全 bytes（bytes_find/bytes_slice/bytes_concat），输出帧头字节数 = `bytes_len(bytes(json_stringify(body)))`；write(1, str) 按内部字节写全 ✓ |
| D | `read(fd,0)` read(2) 直通阻塞、EOF=空 bytes；`bytes_find` 找不到返回 null；`\r\n` 字面量转义为真实 CRLF；`bytes("")` 合法空 bytes；bytes==bytes 比较不可靠（用 bytes_len） | 帧解析/自测均按此语义实现 |
| E | PuXian `/` 是浮点除（`65/2=32.5`） | 整数除法须 `int(a/b)` 截断（自测 half 处踩坑） |
| F | codegen 对内置调用生成 `px_get_global("name")+px_call`（非静态链接） | runtime 加 native 只需 `px_set_global` 注册 → **任何 pxc build 产物自动可用**，无需重建 bootstrap/pxc |
| G | **bootstrap/pxc 编译 interp.px（pxi 全模块 import 合并）codegen 极慢**（实测 >10min、RSS 2.3GB、interp.c 0 字节滞留） | pxi 重建是 S1 关键路径长任务；runtime native 直调须重建 pxi（interp 二进制内嵌 runtime）；后续动 runtime 预留此耗时 |
| H | px 工具 import 惯例：纯 defs 核心（零顶层数据）+ 主文件重复声明顶层全局；`lc_dnew` 空 dict 哨兵（`{}`==null 陷阱） | jsonrpc_core.px 为纯 defs；dict 初始化用 dnew 哨兵 |
| I | pxlint BUILTINS 白名单缺裸 `read`/`write`（M57 fd 原语漏补） | 已补 read/write/os_spawn_capture（pxlint.px BUILTINS，需重建 pxlint 生效） |

### 10.2 S1 决策：runtime 补 `os_spawn_capture`（实施）
- 唯一 runtime 补丁（PLAN §4 前置决策确认必须）：**LSP 诊断子进程化（发现 A）与 MCP run/test/bench 都依赖它**，不只 MCP。
- 签名：`os_spawn_capture(cmd, args) → [rc:int, output:str] | null`
  - fork+execvp；子进程 stdout+stderr **合并同一管道**（2>&1 语义，单管道免 poll 死锁）；父进程读尽后 waitpid。
  - rc：正常=exit code；信号=128+sig；exec 失败=127；fork/pipe/wait 失败=null。
- 改动点：`runtime/runtime.c`（前向声明 + bi_os_spawn_capture 实现 + px_set_global 注册）；
  `selfhost/interp.px`（names 白名单加 os_spawn_capture）；`selfhost/ibuiltin.px`（dispatch 分支 Ok(os_spawn_capture(...))）。
- 编译模式冒烟已 PASS：echo rc=0 输出捕获 ✓、sh exit 3 透传 + stdout/stderr 合并 ✓、exec 失败 127 ✓。
- pxi 解释模式冒烟待 interp.px 重建（长任务，见 G）。

### 10.3 S1 交付物与验收状态
- `tools/jsonrpc_core.px`（纯 defs）：jr_pull（半包/粘包/坏头 `__jr_bad__`/坏 body `__jr_parse__`）、jr_encode/jr_write、
  jr_content_length、JSON-RPC 构造/分类/标准错误码。**双模式 33 PASS / 0 FAIL**（pxi + pxc 编译版实测）。
- `examples/m65_lsp/jsonrpc_selftest.px`、`spawncap_selftest.px`、`verify.sh`（双模式断言脚本）。
- 待收尾：pxi 重建完成 → pxi 跑 spawncap 冒烟 → capability 双模式回归 + diffcheck --all → pxlint 重建（BUILTINS 补漏）后新文件 lint 全绿 → S1 commit。

## 11. S2 执行记录（M65-S2 完成，2026-09-04）

### 11.1 交付物
| 文件 | 角色 | 验证 |
|---|---|---|
| `tools/pxcheck.px` → `bootstrap/pxcheck` | **独立诊断器**（LSP 子进程）：import parser+lint_core，lex+parse+lint → stdout 单行 JSON；parse/lex 错误 = parser 打印错误行后 panic（进程退出 1），pxlsp 按文本解析 | 干净 `[]`；L002 JSON（1-based 行/列）；语法/词法错误文本行 rc=1 |
| `tools/pxlsp.px` → `bootstrap/pxlsp` | **LSP 服务器**：生命周期 initialize/initialized/shutdown/exit + 文档同步 didOpen/didChange(Full)/didSave/didClose + publishDiagnostics（子进程 pxcheck 深度诊断，服务器不 import parser/lint） | python3 模拟 client 17 断言全绿 + verify.sh S1/S2 ALL PASS |
| `tools/pxc lsp` | pxc 薄壳直通（exec 保留 fd 0/1 + 注入 PX_PXCHECK） | `pxc lsp --version` + 文件喂帧端到端 rc=0 |
| `examples/m65_lsp/lsp_client_s2.py` + `verify.sh` | 模拟标准 LSP client（Content-Length 帧双向管道） | initialize→didOpen(L002@0-based 行3)→didChange 干净→didChange 语法错误→didClose→shutdown→exit 全 PASS |

### 11.2 S2 实测新增发现（影响 S3/S4 架构）
| # | 发现（实测） | 处置 |
|---|---|---|
| J | **PuXian if/elif 不建块作用域（函数级作用域）**：同一函数内多个顺序 if 链复用变量名（p/td/uri/text/last…）+ 后链对同名变量赋值 → 命中前链 `let` 的不可变绑定 → 编译错 E3002「对不可变变量赋值」。repro 证实（两个顺序 if 各 `let txt` + 第二个 `txt=` 报 E3002） | 文档同步每个 handler 拆**独立函数**（lsp_on_open/change/save/close），同函数内不跨 if 链复用变量名 |
| K | **`env()` 未设置时返回 null（非 ""）**：`if pxcheck == ""` 判不住 null → 后续 null+str 相加运行时错误「无法相加: null + string」杀服务器 | 默认值判断统一 `== null or == ""`；pxcheck PX_STDLIB 同修（原代码 `!= ""` 漏 null） |
| L | json_parse 产 dict **缺键访问 = R1008 运行时错误杀进程**（同 M65-S1 B 的不可捕获属性） | pxlsp 全部嵌套 dict 访问 .has() 守卫后才取（lsp_on_* 已按此写） |
| M | pxcheck 崩溃文本两种格式需分别解析：parser `L:C: 语法错误 CODE: msg` / lexer `错误: L:C: 词法错误 CODE: msg`（后者带「错误: 」前缀）；「运行时错误: …」行（px_error stderr）格式不同须跳过 | lsp_parse_err_line 双格式 + 首个匹配即返回（对齐 Rust 报首错即停） |
| N | LSP Position 0-based 与 px 1-based 的换算 + severity：Error=1/Warning=2、source="px"、range=点（start=end，对齐 Rust range(pos,pos)） | lsp_json_diags 统一换算 |
| O | pxcheck JSON 判定以 **stdout 是否以 `[` 开头**为准（pxcheck 有 lint Error 时 exit 1 但 JSON 仍输出）——不能以退出码判解析成败 | pxlsp 先 `starts_with(ot,"[")` → json_parse；否则走文本崩溃路径 |

### 11.3 S2 验收结论
- 能力位诚实协商：initialize 仅声明 `textDocumentSync {openClose, change:1(Full)}`；
  completion/definition/hover 能力位随 **S3** 落地后再开（不提前声明未实现能力，区别于 Rust lsp.rs 一次性声明）。
- lint/fmt：新文件 pxcheck.px / pxlsp.px pxlint 0 错 0 警 + pxfmt --check 全绿。
- 全仓回归：verify.sh（S1+S2）ALL PASS；未动 runtime/selfhost/pxlint（零回归面）。
