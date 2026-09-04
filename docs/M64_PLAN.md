# M64_PLAN —— 工具链自举恢复（fmt/lint/test/bench/doc/lsp/mcp）

> 状态：M64-S1 已完成（keep-lexer 底座入库）· S2 已完成（pxfmt 格式化器自举）
> 进度：M64a fmt 主逻辑完成（S2）· S3 待开工（lint）
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

## 9. M64-S1 侦查记录与决策（已执行，commit 待入库）

### 9.1 行结构 token 现状（实测）
- `selfhost/pxlexer.px`（760 行，编译/解析用）：token = `[kind串, val, line, col]`；kind 含中文字符串 `换行/缩进/去缩进/EOF` 结构 token，标识符=`标识符`、关键字/运算符=`原文`、字符串=`字符串`。
- 注释完全丢弃：`next_token` 遇 `#` → `skip_comment()` 吞掉不产 token；`handle_line_start` 中注释行/空行 continue 吞掉（不产 Newline/Indent）。
- `pxlexer.px` 被 parser/codegen/compiler/interp import（bootstrap_prove.sh 守护链）→ **改它需自举重建 pxc/pxi + golden 全量对拍**。

### 9.2 Rust 版 fmt 依赖（fmt.rs / lexer.rs 实证）
- `Lexer::new_with_comments` = 同一 Lexer + `preserve_comments=true`（开关式）。
- preserve 语义：① 行首注释 → `Comment(text)` + 若后随 `\n` 补 `Newline`，**注释行不产 Indent/Dedent**（continue 直到真代码行）；② 行中注释 → `Comment(text)`（行尾换行由主循环产）；③ 空行仍吞掉。
- fmt 主循环用 `Newline/Indent/Dedent/Comment` 重建源码：level 由 Indent/Dedent 驱动；行首注释按原列 `col-1` 空格对齐；行内注释前补 2 空格；连续空行压到 1；末尾单换行。

### 9.3 import 语义实验（PuXian 实证，决定底座文件组织）
- import 只注册被导入文件的 **def 函数**；**顶层 var/let 均不注册**（d.px 主文件不声明全局 → 被导入函数引用全局报 `R1001 未定义变量`）。
- 被导入函数引用顶层全局时在**主文件作用域**解析：主文件声明同名全局则函数绑定主文件全局（b.px：get_gx() 返回主文件 gx=99）。
- 推论：跨文件共享词法全局 = 主文件重复声明全套全局（lexer.px/parser.px/compiler.px 先例即此）。

### 9.4 keep-lexer 底座决策（定案）
- **不修改 pxlexer.px**（避免动自举链）；新建独立保留模式词法器 **`tools/fmtlexer.px`**（788 行，由 pxlexer.px 复制派生，头注释声明同步维护约定）。
- 与 pxlexer 唯一行为差异 = 新增 `g_keep` 开关 + 入口 `lex_tokens_keep(src)`：`g_keep=false`（默认）行为与 pxlexer.lex_tokens 完全一致；`true` 时注释以 `kind="注释"` token 输出，纯注释行产 `[注释, 换行]`（对齐 Rust new_with_comments ①②③）。
- 后续 doc/lint 共用同一 `lex_tokens_keep`；调用方（pxfmt.px 等）须按 §9.3 惯例重复声明全局。

### 9.5 S1 验证结果（实测）
- `tools/t_fmtlex_sample.px`（行首/行内/块注释跨行/空行/缩进样本）：keep 模式 43 token，注释+行结构序列符合 §9.2 语义（行首注释后补换行、注释行不产缩进 token、代码行才产缩进/去缩进）。
- 默认模式对拍：`fmtlexer.lex_tokens` 输出 vs `pxc lex`（pxl）**逐字节一致**（复制保真）。
- 性能：pxi 解释器跑含 `CTRL_ALL`（61 个 `\u{..}` 转义）长串 >20s（解释器 FFI 开销假象）；**编译版 0.048s**（`tools/pxc build` 后）→ fmt 工具以编译版二进制交付（同 pxl/pxpar 模式），性能无虞；`selfhost/lexer.px` 全量 keep 词法化编译版 0.177s / 387 token / 9 注释。
- 边界记录：跨行块注释若 `|#` 后同行残留代码属非法源码（编译链同样处理），keep 模式报缩进错一致。

### 9.6 S2 开工点（fmt 主逻辑 tools/pxfmt.px）
- 结构：import "fmtlexer.px" + 重复声明全局（§9.3）；主循环对齐 fmt.rs（level/at_line_start/prev/prev2 + needs_space/is_unary_context/render/escape_str）；`--check/--diff/写回` + 空行压缩 + 行首注释按原列对齐 + 行内注释补 2 空格；编译 `tools/pxc build` → `bootstrap/pxfmt` → `pxc fmt` 子命令。


## 10. M64-S2 记录（pxfmt 格式化器自举，已完成）

### 10.1 交付物
- `tools/fmt_core.px`（格式化核心，自包含零全局）：tok_text/render（整数/浮点补 .0/字符串 rust_str_debug 值）、needs_space、is_unary_ctx、行结构重建（level/at_line_start/prev/prev2）、行首注释按原列对齐、行内注释补 2 空格、空行压缩、尾部单换行、unified_diff（行级 LCS + @@ hunk + 上下文 3 行）。
- `tools/pxfmt.px`（CLI 薄壳）：import fmtlexer.px + fmt_core.px + §9.3 重复声明全局；`<file>` 默认 stdout、`-w` 写回、`--check` 仅检查（不一致 rc=1）、`--diff` 打印 unified diff；`--version`。
- `bootstrap/pxfmt`（自举二进制，9.0MB 静态，入库同 pxl 模式）+ `tools/pxc fmt` 子命令。
- `examples/m64_fmt/`：`m64_fmt_in.px`（乱格式样本）+ `m64_fmt_gold.px`（golden）+ `verify.sh`（26 项断言）。
- `.gitignore` 增 `tools/build/`（pxc build 产物 26MB 不入库）。

### 10.2 行为对齐与差异（实测）
- 与 Rust fmt.rs 对齐：运算符/逗号/冒号空格、`f(` 无空格、关键字 `if (` 有空格、注释保留、空行压缩、尾单换行、浮点补 `.0`（fmtlexer 去 .0 → render 补回闭环）、`--check/--diff/-w` 语义、unified diff 格式。
- **修正 Rust 一元负号缺陷**：Rust needs_space "cur 是 +/- 且 prev 一元上下文 → 前不空格" 会把 `x = -1` 压成 `x =-1`、`1 + -2` 压成 `1 +-2`；pxfmt 去掉该条、仅保留 "-1 内部紧贴" 条 → `let neg = -3`、`let pos = 1 + -2`（与全仓惯例及 `<-` 列表追加歧义规避一致）。已留注释于 fmt_core.px。
- 插值 `${x}` 规范化为等价拼接 `"val=" + str(x) + "end"`（spec/MINI_SUBSET 明示该语义保持行为）。
- 切片冒号后空格 `arr[0: 2]`（与 Rust Colon 规则一致）。
- 空行被词法器吞（Rust lexer 同）→ def 间空行压缩为 0，属对拍一致行为；自举工具自身已按此收敛。
- `fmtlexer.px` 豁免 fmt --check（由 pxlexer 派生的底座，保持与源文件逐行可 diff）。

### 10.3 验证（真实执行）
- examples/m64_fmt/verify.sh 26 项全 ✅：golden 逐字节、幂等、语义等价（重 lex token 序列）、--check/--diff/-w、自举 dogfood（fmt_core/pxfmt 自身格式正确）。
- 大文件：tools/pxpkg.px（372 行）格式化 → 352 行（空行压缩），2332 语义 token 前后**完全一致**，幂等 OK，2s。
- 自举闭环：fmt_core.px / pxfmt.px 已用 pxfmt -w 收敛（dogfood），重新编译后功能回归全过。
- 编译：tools/pxc build tools/pxfmt.px ~2.5 分钟（runtime 全量静态）。

### 10.4 遗留（S3+ / M64a 收尾）
- `--indent N|tab / --quote single / --config .pxfmt.toml`（Rust M30 配置化）未做，S2 固定 4 空格 + 双引号（M64_PLAN §4 已列 M64d 按需）。
- selfhost/stdlib 全仓 fmt 收敛：待 lint 落地后与 --check 一起统一收尾（先报 diff 人工审再一次性收敛，§6 风险缓解）。

## 11. M64-S3 记录（lint 静态检查器自举，已完成）

### 11.1 交付物
- `tools/lint_core.px`（纯 defs 零顶层数据；规则 L001-L008 对齐 Rust lint.rs）
- `tools/pxlint.px`（CLI：`<file> [--json] [--strict] [--version]`；import ../selfhost/parser.px 复用自举 lexer+parser 产 AST）
- `bootstrap/pxlint`（编译版 9.1MB）+ `pxc lint` 子命令
- `examples/m64_lint/`（bad.px/longline.px/clean.px + verify.sh 18 断言）
- 退出码：Error → 1；仅 Warning → 0；--strict 时 Warning 也 → 1；--json 输出诊断数组

### 11.2 关键实现事实（全部实测）
- **AST=list 嵌套**（parser.px 产出），字符串字段 rust_str_debug 带引号 → lc_unq 还原（参考 it_util.rust_unescape）；pos=[line,col]。
- **空 dict 字面量 `{}` 求值为 null**（实测！）→ 空 dict 只能用「哨兵键 + remove」构造（lc_dnew）；早期 walker 因此 R1007 null.has 崩溃。
- **PuXian list/dict 字面量不能跨行**（括号续行不存在）→ 大常量（BUILTINS 178 名）用 `split("...", " ")` 单行字符串构造；KEYWORDS/CTRL_ALL/BUILTINS 等数据行必然超 100 字符 → `# noqa` 尾注豁免（lc_check_lines 支持，Rust lint 后自举增强，已文档化）。
- **`pass` 不存在**、list 无 `.sort()`（用内置 sorted）、let 不可变赋值编译期 E3002（dogfood 拆行时踩中 r1 += 需 var）。
- **模块 main 不进 import 合并**（cg_is_definition 排除）→ pxlint import parser.px 无 fn_main 冲突；pxlint 顶层 def main 自动调用（pxi/编译约定）。
- **L002 已知名集合** = builtins 全集（从 Rust lint.rs Builtin 枚举派生 snake 178 名）+ 顶层 def/全局 var + std.* 模块函数 + **递归 import 的文件顶层 FuncDef**（对齐 cg_resolve_modules 级联合并：pxlint 调 lex_tokens 因 parser→pxlexer 递归链，单层收集会误报）。宿主依赖模块（it_util 用 hex_to_char、astdump 用 LAYOUT——依赖被 import 方提供全局）单文件 lint 必报 L002 → 模块惯例固有边界，白名单留档（Rust lint.rs 同）。
- L004 空块在 PuXian **语法上不可表达**（parser 强制块非空，空 def/if/for 均 parse 报错）→ 实现保留防御（对齐 Rust），自测不覆盖。
- L007 行 >100 字符（unicode 码点，len 按码点实测）；`# noqa` 行豁免（KEYWORDS/CTRL_ALL/BUILTINS/LAYOUT 等语言强制单行数据）。
- pxi 解释器无 args()（pxlint 以编译版交付）；解释器跑 walker >120s，编译版 0.5s/文件。

### 11.3 dogfood 收敛（lint 自举工具链 7 文件 0 错误 0 警告）
- lint_core.px / pxlint.px / fmt_core.px / pxfmt.px / fmtlexer.px / pxpkg.px / routegen.px 全绿。
- 修的真实问题：lint_core 超长 elif 重构为 helper（lc_stmt_noop/lc_is_def_stmt）；pxlint json dict 字面量超长拆多语句；pxpkg/routegen print 长消息与长拼接拆多行（dogfood 期间 routegen 曾因 let r1 + r1+= 触发编译期 E3002 → 改 var，展示 E3002 属编译器检查非 lint 规则）。
- pxfmt -w 收敛 lint_core/pxlint 自身格式（fmt 与 lint 双 dogfood 闭环）。
- selfhost 源码（含编译器本体）未动：超长 KEYWORDS/CTRL_ALL 数据行与历史逻辑行 L007 属语言强制/历史留档；宿主依赖模块 L002 白名单留档；全仓 fmt 收敛归 M64a-S4 收尾（涉及自举对拍，须专项）。

### 11.4 验证（真实执行）
- examples/m64_lint/verify.sh 18/18 PASS（六规则命中 + L007/L008 + clean 零告警 + --json + --strict）
- 自举工具链 7 文件 lint 0/0；pxc lint 子命令可用；--version 输出
- pxpkg（改动后）init/list 实测通过；routegen（改动后）重编译 + 实际生成 6 路由成功
