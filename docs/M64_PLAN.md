# M64_PLAN —— 工具链自举恢复（fmt/lint/test/bench/doc/lsp/mcp）

> 状态：已立项 · 待 S1 开工
> 关联：docs/spec.md §12（AI 工具链接口规格）、docs/GAP_ANALYSIS.md、docs/MINI_SUBSET.md

## 1. 背景与现状（全部实测 2025-09-04）

spec §12 定义 8 项工具：`fmt`/`lint`/`test`/`doc`/`bench`/`ast`/`lsp`/`mcp`。
Rust 版全套归档于 `archive/rust-compiler/src/`（只读参考），已于 M-B9a 退役。

| 工具 | Rust 版参考 | 自举现状 | 结论 |
|---|---|---|---|
| pkg | pkg.rs | ✅ 已恢复（tools/pxpkg.px，M45） | 完成 |
| test | test.rs (5778B) | ⚠️ 半自举：断言在语言内（capability.px），编排仍是 shell | M64c |
| fmt | fmt.rs (19552B) | ❌ 无 .px 实现 | M64a |
| lint | lint.rs (32729B) | ❌ 无 .px 实现 | M64b |
| bench | bench.rs (4314B) | ❌ 无 .px 实现 | M64c |
| doc | doc.rs (6761B) | ❌ 无 .px 实现 | M64c |
| ast | ast_view.rs / astdump.px | ✅ astdump.px 已有（JSON/Debug AST） | 复用 |
| lsp | lsp.rs (36618B) | ❌ 无 .px 实现 | M64d（按需） |
| mcp | mcp.rs (13409B) | ❌ 无 .px 实现 | M64d（按需） |

**关键架构事实（实测）：**
- 当前 pxc 子命令仅 build/run/lex/parse/version/help；新工具全部走「.px 源 → bootstrap/pxc 编译 → bootstrap/pxNN 二进制 → tools/pxc 薄壳子命令」模式（与 pxpkg/pxl/pxpar 同构）。
- `pxlexer.px` 现为**编译/解析用 lexer**：注释跳过（skip_comment）、注释行/空行不产 token；而 Rust 版 fmt.rs/doc.rs 依赖 `Lexer::new_with_comments`（**保留 Comment + Newline/Indent/Dedent 行结构 token**）。→ **fmt/doc 需要一个"保留模式" lexer 底座**（M64a S0），以独立函数/文件隔离，不污染共享 lex_tokens。
- Rust test.rs 实际约定：**顶层 `def test_xxx()` 无参函数 = 测试用例**（spec §12 写"@test 标注"，实现取命名约定；可把 `# @test` 注释作等价标注接受）。
- doc.rs 约定：`##` 开头的文档注释紧跟顶层 def/struct/enum/trait 定义。
- lint.rs 规则编号 L001–L008（已确认：L005 重复顶层定义、L006 命名规范、L007 行过长>100、L008 行尾空白；其余实现时逐条核对 archive）。
- 编译/运行基底：bootstrap/pxc 可把任意 .px 编译为静态二进制；runtime 已含 now_us 等计时原语（bench 用）。
- dogfood 语料规模：selfhost/*.px 8403 行 + stdlib + tools/*.px + examples/**/*.px + capability.px（897 行断言框架本身）。

## 2. 目标

以 PuXian 自身实现 spec §12 全套工具（pkg/ast 已恢复），消灭 Rust 依赖，达到：
- 每个工具 = .px 源码（可被自举编译、可被 fmt/lint dogfood）+ bootstrap 二进制 + pxc 子命令；
- 全仓无格式/风格漂移（fmt --check、lint 可进 CI 回归）；
- 测试/文档/基准从 shell 半自举提升为语言内工具；
- 为 AI agent 协议（lsp/mcp，§12.1）预留底座。

## 3. 架构统一决策

1. **保留模式 lexer 底座（先做）**：在 pxlexer.px 新增独立 `lex_tokens_keep(src)`（或独立 pxfmtlexer.px 文件——以不污染共享 lex 对拍为硬约束，实现时侦查行结构 token 现状后定），输出含 `Newline/Indent/Dedent/Comment` 的 token 流，供 fmt/doc 共用。**验收：原 lex_tokens 输出不变，diffcheck 全量回归通过。**
2. **工具布局**：
   - 源码：`tools/pxfmt.px`、`tools/pxlint.px`、`tools/pxdoc.px`、`tools/pxtest.px`、`tools/pxbench.px`
   - 产物：`bootstrap/pxfmt`、`pxlint`、`pxdoc`、`pxtest`、`pxbench`
   - 入口：tools/pxc 增加 `fmt`/`lint`/`doc`/`test`/`bench` 子命令（shell 薄壳，行为对齐 Rust 版）
3. **版本/帮助**：每工具支持 `--version`（对齐 GO_BUILD/P0：所有程序支持 --version）+ `-h`。
4. **对拍基准策略**（Rust 版已退役、无执行参照）：
   - fmt：幂等性 `fmt(fmt(x))==fmt(x)` + 构造覆盖样本（注释/字符串/一元二元/链式/dict/list/多行与插值字符串/空行压缩）+ 全仓 --check 收敛
   - lint：自测语料（cases_bad 期望告警 + 干净样本零告警）+ dogfood 收敛
   - test/bench/doc：行为语义对齐 Rust 版描述（test_ 约定 / bench 单函数计时 / ## 注释生成 md）

## 4. 里程碑与验收

### M64a：fmt（确定性格式化器）
- S0 keep-lexer 底座（含行结构 token 现状侦查与加装）
- S1 needs_space/render 规则移植（对齐 fmt.rs 173–360：一元±、f( 无空格、not 空格、fn( 无空格、值+( 无空格…）
- S2 缩进/空行规则 + 行首注释按原列对齐 + 行内注释前补两空格 + 空行最多 1 个
- S3 `--check`/`--diff`(unified diff LCS)/写回 + 幂等验证
- S4 全仓 dogfood：selfhost/stdlib/tools/examples 跑 `pxc fmt --check`，修复仓库自身不一致后收敛全绿
- 验收：幂等 + --check 全绿 + 样本集覆盖

### M64b：lint（静态检查）
- S1 L001–L008 规则全移植（行过长/行尾空白/重复顶层定义/命名规范/未用变量/未定义变量/不可达/空 body/重复局部定义…以 archive lint.rs 为准逐条对齐编号与级别）
- S2 builtin/std 名集合（collect_std_names 语义：扫描 import std.* 读 stdlib 模块顶层函数）
- S3 输出格式对齐（代码/位置/消息）+ `--json`（§12.1 机器可读）+ 退出码
- S4 自测语料 + dogfood selfhost 收敛
- 验收：cases_bad 全命中、干净样本零告警、selfhost 全绿（或白名单留档）

### M64c：doc + test + bench
- doc：复用 keep-lexer；`##` 注释 → Markdown（对齐 doc.rs 12–115：签名提取 def/struct/enum/trait + `## 函数`/`## 类型` 分节 + 无文档注释给出"（无文档）"）
- test：扫描顶层 `def test_xxx()`（无参）→ 逐用例独立运行（构造仅该测试程序，对齐 test.rs run_one_test）→ 汇总 pass/fail + filter 参数
- bench：单函数计时（now_us）+ 对比输出（对齐 bench.rs opts）
- 验收：pxc test 对 capability.px 或新样例等价 diffcheck 结论；pxc doc 生成可读 md；pxc bench 输出数值

### M64d（三档·按需）：lsp → mcp
- 前置侦查：runtime fd stdin/stdout 原语现状（M60 有 fd read/write？）——缺则先补
- lsp：诊断（复用 lint）+ 补全 + 跳转 + 悬停（AST 驱动）；stdio JSON-RPC
- mcp：复用 stdio/JSON-RPC 底座暴露 px 工具（build/lint/fmt/test/doc）
- 验收：与标准 LSP/MCP client 最小握手（initialize → 诊断请求）

## 5. CI 集成（发布/回归自动化）

- ci.yml 回归 job 增加：`pxc fmt --check` + `pxc lint`（dogfood 全仓）+ 工具自测；
- make_release.sh 冒烟增加：pxfmt --version、pxlint --version 等新工具存在性；
- CHANGELOG / README §工具链 / spec §12 勾选状态同步更新。

## 6. 风险与缓解

| 风险 | 缓解 |
|---|---|
| keep-lexer 污染共享 lex 影响 compiler 对拍 | 独立函数/文件 + 每次改动跑 diffcheck --all + capability 双模式回归 |
| fmt 大改仓库既有格式 | 先 --check 只报不改，diff 人工审后一次性收敛；缩进/引号选项保留（Rust FormatOptions 语义） |
| lint 误报引发 dogfood 大改 | 规则先与 Rust 版对齐（warn 为主），严重项才 error；白名单/`# noqa` 语义留档 |
| lsp/mcp 重量级（stdio 双工/增量/长驻） | 列为 M64d 按需；先确认 runtime stdin 原语，缺则补最小 read_line |
| .px 单文件超 500 行 | 工具 >500 行时按模块拆分 import（pxlexer/parser 同构惯例） |

## 7. 本里程碑不做（边界）

- Rust 版工具源码不删除（archive 只读留档），但不再编译/引用；
- pkg/ast 已有自举版，不在本次范围（仅回归确认）；
- lsp/mcp 仅在 M64d 按需启动，不在 M64a–c 承诺内；
- 不新增语言特性，纯工具链自举恢复。

## 8. 交付物清单

- docs/M64_PLAN.md（本文件）
- tools/pxfmt.px、pxlint.px、pxdoc.px、pxtest.px、pxbench.px + bootstrap/px* + tools/pxc 子命令
- selfhost/ 或 tools/ 新增 keep-lexer 底座
- 全仓 fmt --check / lint 收敛改动（selfhost/stdlib/tools/examples）
- ci.yml + make_release.sh 更新、CHANGELOG、README、spec §12 勾选
- 每里程碑 verify 脚本（examples/m64*_verify.sh）与 docs/M64*_PLAN.md 进展记录
