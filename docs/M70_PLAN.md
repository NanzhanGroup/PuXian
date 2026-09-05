# M70_PLAN · 语言缺口修复（表达式跨行 + 模块顶层状态）

> 创建：2026-09-05 · 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD main @ 0d6e32b，M69 闭环 · v0.1.0-m69 已发布）
> 来源：M69-S4 `docs/ECOSYSTEM_GAPS.md §4`（写库体验评估拆出的语言缺口候选，记录在案：**M70 候选 A · 表达式跨行**（G2）+ **M70 候选 B · 模块顶层状态**（G1））
> 用户指令（2026-09-05）：见 M69 收尾后提问「M70 立项预备是啥」→ 东月说明候选 A/B → 用户：「开」（不排 ws-todo，直接开工，按推荐 A→B）
> 目标：修复写库/写码两大语言缺口——**①表达式可跨行（多行 list/dict/调用参数/括号）②模块顶层 let/var 真正全局化（跨函数访问 + import 可导出）**；M70 是语言面修复里程碑（改 lexer/parser/codegen/语义），回归须带全套（参照 M62/M63/M64 先例）。

## 〇、现状侦查（2026-09-05 源码实录，非记忆推断）

**换行/缩进的 lexer 模型（pxlexer.px）**
- 物理换行 → emit token「换行」（pxlexer.px:666）；行首缩进与 `g_indent_stack` 比较 → emit「缩进/去缩进」（pxlexer.px:111-120）；**无括号深度感知**——任何物理行首都做缩进栈检查，缩进不一致即 E2002。
- 语句边界：parser 以「换行/去缩进/EOF」结束语句（parser.px:232）。

**表达式跨行现状（parser.px）**
| 结构 | 函数 | 跨行现状 |
|---|---|---|
| `{...}` dict/块 | parse_brace (:1055) | ✅ **已支持**：adv `{` 后 `skip_newlines()` + `skip_brace_indents()`（:1056-1058），entries 循环与闭合 `}` 前同样跳过（:1065-1078） |
| `{...}` 判定 | brace_looks_like_dict (:1037) | 扫 token 遇「换行」即判非 dict（:1051）→ **dict 若跨行会被误判为块**？需实证（现有跨行 dict 用例若不存在则此路径未覆盖） |
| `[...]` list/推导 | parse_list_or_comp (:964) | ❌ 不支持：`parse_expr()` 后遇「换行」即断，逗号后无跳过 |
| `(...)` 括号/元组 | parse_paren_or_tuple (:1015) | ❌ 不支持（同理） |
| `f(a, b)` 调用参数 | parse_call_args (:887) | ❌ 不支持：`expect("(")` 后无 skip，逗号后无跳过 |
| 索引/切片 `a[...]` | parse_postfix (:828) | parse_expr 后 expect("]")，跨行 ❌ |

**模块顶层状态现状**
- parser：顶层 VarDecl 三态（Let/Var/Const）照常解析（parse_var_decl :256）。
- codegen.px：顶层 VarDecl 名收进 `cg_globals`（:475，Let/Const 进 cg_immutables）；生成在 `main()` 内由 `cg_gen_stmt` 输出 → **局部 C 变量**，跨函数访问报未定义；函数体内变量提升排除 cg_globals（:419）→ 函数内引用顶层 var 名会走全局名路径（当前对非函数全局名无正确生成）。
- import 合并 cg_module.px：`cg_is_definition` 只导出 Const VarDecl（:112），**顶层 let/var 不导出**（:186 过滤）；模块顶层非定义语句（含赋值/初始化）不执行（import 无副作用原则，:186-188）。
- runtime.c 已有 `px_set_global`/`px_get_global`（:2545/2592，M55/P0 引入，注册全局函数用）→ 可承载**运行时全局变量槽**，无需新 C 设施。
- 解释器侧：istmt/iexpr 顶层 var 语义待实证（G1 记载「同一文件流程函数内可读写」→ 预期解释器已通、编译模式不通，修复目标 = 编译模式对齐解释器）。

## 一、范围与边界

**做（S1–S6）**：
- **S1 · 表达式跨行（候选 A）**：parser 层跨行容忍——parse_call_args / parse_list_or_comp / parse_paren_or_tuple / parse_postfix 索引切片 / brace_looks_like_dict 跨行 dict 判定，在元素前、逗号后、闭合括号前跳过「换行/缩进/去缩进」（对齐 parse_brace 已有模式，**不动 lexer token 流** → 现有 golden tokens 零漂移）。约束：续行缩进须与缩进栈相容（E2002 仍是硬错误），由 fmt + 写库规范保证。
- **S2 · fmt 同步（候选 A 收口）**：`pxc fmt` 支持多行 list/dict/call——侦查 fmt_core.px 架构（token 重排 or AST 打印）后定实现；目标：fmt 多行结构输出规范缩进、不压平不报错；fmt 输出必须能通过自身 parser 往返。
- **S3 · 模块顶层状态（候选 B）**：codegen 顶层 let/var 全局化（main 内 `px_set_global` 注册槽 + 函数内读写路由 `px_get/set_global`）；cg_module `cg_is_definition` 允许导出非 Const 顶层 VarDecl（模块级状态槽，初始 null，import 不执行顶层语句——语义定案见下）；interp 对齐实证。
- **S4 · 回归总闸**：golden 全量重生成 + diffcheck --all/--errors 全绿 + capability 双模式 + pxi/pxc 重建 + 自举证明（compiler.px 改动后 B.c 与 golden 逐字节一致）+ stdlib 9 库 + 历史语言面回归（m62_langfix/m63_langfix/m64_*）。
- **S5 · 文档收口**：spec（语言参考：表达式跨行规则 + 模块顶层状态语义）/ MINI_SUBSET（§四/§八/§九 限制更新）/ PUXIAN_CHEATSHEET / ECOSYSTEM_GAPS（G1/G2 勾除或降级）/ ROADMAP M70 行 / CHANGELOG / README(.en) 语言特性行。
- **S6 · 发布**：tag `v0.1.0-m70` 自动发布 + GitHub 产物二次冒烟 + 本机留档 + 发布指引更新。

**不做**：
- ❌ G3 `let` 不可变（有意设计，保持现状）；G4 分号（不做，表达式跨行并入 S1 已覆盖续行场景）。
- ❌ 新语言特性（模式匹配增强/宏/类型推断深化等）——不在缺口表不立项。
- ❌ 动 runtime C 语义/白名单/FFI 表；不开 PR（维持既定边界，直接 commit + push origin main）。

## 二、分步计划

### S1 · 表达式跨行（parser，候选 A）
- 实证基线：写跨行探针 `.px`（多行 list / 多行 dict / 多行调用 / 多行括号元组 / 多行索引）跑 bootstrap/pxpar + pxi 记录当前报错（E2001/E2002），作为修复前后对照。
- 修 parser.px：
  - 新辅助 `skip_expr_ws()`（跳过「换行/缩进/去缩进」，对齐 skip_brace_indents 语义，注意与 skip_newlines 组合使用顺序）。
  - parse_call_args：`(` 后、每实参 parse_expr 前、`,` 后、`)` 前跳过；空参 `()` 不受影响。
  - parse_list_or_comp：`[` 后、元素间、`,` 后、推导 for 后、`]` 前跳过；空 list、listcomp 完整支持。
  - parse_paren_or_tuple：`(` 后、元素间、`)` 前跳过；区分「括号内跨行 = 元组/分组表达式」与语句边界。
  - parse_postfix 索引 `[...]` / 切片：界内跳过。
  - brace_looks_like_dict：跨行 dict 判定修正（depth==0 遇「换行」不应直接判非 dict——需看换行后是否仍在 `{` 内；改为遇 `}` 才收敛，换行仅当同时 depth==0 且… 实证后定）。
  - 注意：语句起始表达式（parse_assign_or_expr :275）遇跨行起始（如 `[\n...]` 作为语句首）时 parse_expr 前需先跳过换行——只限「表达式自身括号内」，不能吞掉真正的语句边界（下一语句起始换行不可跳）。逐点实证。
- 用例 + golden：新增 cases（如 s15_multiline.px：多行 list/dict/call/tuple/索引 + 语义断言；v 系列补跨行 + 模块顶层状态联动）→ lex/parse/codegen/run 四类 golden 生成并对拍；补 cases_bad（括号内不闭合跨行 → 仍报错）。
- 验证：diffcheck 相关单测 + 全量无回归 + pxi/pxc 双模式跨行用例一致。

### S2 · fmt 同步（候选 A 收口）
- 侦查 tools/fmt_core.px + fmtlexer.px 架构（token 流重排 or AST 打印）→ 定多行支持实现点。
- 目标行为：`pxc fmt` 对多行 list/dict/call 输入 → 规范缩进输出（对齐缩进栈，保证 E2002 不触发）；对合法单行保持原样（现有 golden 零漂移）；fmt 输出经 parser 往返成功。
- 验证：多行样本 fmt → 再 fmt 幂等；fmt 后 diffcheck/pxi 双模式运行一致；fmt golden（若有）更新。

### S3 · 模块顶层状态（候选 B）
- 语义定案（写入 spec）：
  1. 主程序顶层 `let/var x = 初值` → x 为**进程级全局状态槽**：同文件任何函数内可读（let/var）/可写（var），赋值 `x = v` 即改全局；let 不可重新赋值（沿用 E3002）。
  2. import 的模块顶层非 Const `let/var` → **可导出的模块级状态槽**：import 侧可用 `模块名.变量`？——**先实证解释器 import 语义再定命名**（普贤 import 是合并 AST，非命名空间对象，可能直接以变量名全局可见）；冲突处理沿用「用户定义优先」。
  3. import **不执行模块顶层赋值/初始化语句**（无副作用原则保持）→ 模块级状态槽初始为 null（若带初值，由模块内首个使用方惰性初始化或文档约束「模块状态须由显式 init 函数初始化」——写库规范同步）。
- codegen.px：顶层 VarDecl（Let/Var）→ 生成 `px_set_global("x", 值)` 于 main 注册段（照顶层函数注册模式 :530-537）；函数体/表达式内变量解析：cg_globals 中非函数名 → 读 `px_get_global("x")`、写 `px_set_global("x", v)`（cg_expr/cg_stmt 赋值与引用两处路由）；Const 仍编译期常量（现状不动）。
- cg_module.px：`cg_is_definition` 对 VarDecl Let/Var 返回 true（导出）；`cg_def_name` 覆盖 Let/Var。
- interp.px 对齐实证：解释器顶层 var 跨函数访问（预期已通）+ import 模块顶层 var 可见性（若解释器已按 AST 合并则天然一致）；不一致处对齐。
- 用例 + golden：v 系列补「顶层 var 跨函数读写」+「import 模块状态（init 函数模式）」+「主程序与模块同名冲突」；编译/解释双模式对拍；自举证明（compiler.px 自身若有顶层 var 依赖——lexer.px 顶部 g_src 等即顶层 var 且被 import 使用——**重点回归：lexer/parser/codegen 自举**）。

### S4 · 回归总闸（全链复跑）
- `selfhost/diffcheck.sh --all`（lex+parse+codegen+run 全 golden 逐字节）+ `--errors`（cases_bad）。
- golden 重生成核对：仅因新用例/新语义预期变化者更新，diff 人工核对无意外漂移。
- capability 双模式（capability.px 编译 vs 解释输出逐字节一致）。
- stdlib 9 库 + tools（pxc/pxfmt/pxlint/pxdoc/pxcheck/pxpkg/pxlsp/pxmcp）自举重建 + 自举证明（compiler.c golden 与重建后编译产物一致）。
- 历史语言面回归：examples m62_langfix / m63_langfix / m64_fmt / m64_lint + stdlib dogfood。
- fmt/lint 0 错；ci.yml YAML 语法；make_release.sh bash -n。

### S5 · 文档收口
- spec.md：新增/修订「表达式跨行」规则段 + 「模块顶层状态」语义段（含 import 导出、副作用边界、E2002 续行缩进约束）。
- MINI_SUBSET.md：§四/§八/§九/§十三 相关限制项更新（G1/G2 从「限制」改「已支持+规则」）；PUXIAN_CHEATSHEET §1 易错事实同步；ECOSYSTEM_GAPS G1/G2 标记 M70 已修（保留历史评估）。
- ROADMAP M70 行；CHANGELOG M70 条目（含 S1-S6 提交汇总）；README(.en) 语言特性行。

### S6 · 发布
- 全链复跑绿 → commit 收口 → tag `v0.1.0-m70` → push tag → CI 自动发布。
- GitHub 产物二次冒烟（下载 tarball + sha256 三方核对 + 解释/编译双模式冒烟用例含跨行语法 + 模块状态语义）。
- 本机留档（/data/release）+ 发布指引（RELEASE_PROCESS.md / 00-README）更新至 m70。

## 三、回归总闸（S4/S6 判据）
1. diffcheck --all + --errors 全绿（golden 逐字节）。
2. capability 双模式一致（逐字节）。
3. pxi/pxc 重建成功（>10 分钟编译预留）+ 自举证明通过（compiler.c golden 一致）。
4. stdlib 9 库 + m62/m63/m64 语言面历史回归全过。
5. fmt/lint 0 错；ci.yml / make_release.sh 静态校验过。
6. 跨行 + 模块状态新用例：编译/解释双模式输出一致。
7. GitHub 产物二次冒烟通过（sha256 三方一致 + 双模式跑通）。

## 四、风险与预案
| 风险 | 预案 |
|---|---|
| lexer 无括号深度感知：括号内跨行缩进不规则 → E2002 | 写库规范 + fmt 强制规范缩进；S1 文档明示「续行须与缩进栈相容」；极端场景用 `"""`/循环构建绕行（已文档化） |
| parse_brace/brace_looks_like_dict 跨行 dict 判定改动引发 block 误判 | 加 cases_bad（跨行 dict 未闭合/跨行块）；golden diff 人工核对 |
| 顶层状态全局化牵动 compiler.px 自举（lexer/parser/codegen 自身顶层 var 被 import 使用） | S3 先跑自举证明；改动分 commit，每步 diffcheck 护航；compiler.c golden 逐字节核对 |
| import 模块状态槽多文件共享语义不清 | 语义定案先写 spec + 双模式实证；初始 null + init 函数模式文档化；冲突「用户优先」沿用 |
| fmt 多行支持改动大 | 若 fmt_core 为 token 流架构，多行仅需「跨行结构不压平 + 缩进规范化」，控制改动面 |
| golden 重生成面大 | 每 S 步增量更新 + diff 人工核对 + diffcheck --all 兜底 |

## 五、验收清单（里程碑完成判据）
- [x] S1：跨行探针（多行 list/dict/call/tuple/索引）pxi/pxc 双模式跑通，语义与单行等价；单行行为零回归（golden 无意外漂移）
- [x] S2：fmt 多行样本规范化输出 + 幂等 + 往返成功；单行格式零回归
- [x] S3：顶层 var 跨函数读写双模式一致；import 模块状态槽可用且无副作用原则成立；自举证明通过
- [x] S4：回归总闸 1-6 全绿
- [x] S5：spec/MINI_SUBSET/CHEATSHEET/ECOSYSTEM_GAPS/ROADMAP/CHANGELOG/README 全部收口
- [ ] S6：tag v0.1.0-m70 发布 + GitHub 产物二次冒烟通过 + 本机留档 + 发布指引更新

## 六、执行进度（实时记录 · 东月）
（每完成一步在此追加：日期/commit/验证结果）

### 执行记录
- **S0 立项侦查**（2026-09-05, commit b927602）：M70_PLAN 起草入库。源码实录确认：parse_brace 已支持跨行 dict/block；parse_call_args/parse_list_or_comp/parse_paren_or_tuple/索引切片不支持（实测 E2001）；顶层 VarDecl codegen 为 main 内局部变量；runtime 已有 px_set_global/get_global 可承载全局槽。
- **S1 表达式跨行**（2026-09-05）：parser.px 新增 `skip_expr_ws()`（跳过 换行/缩进/去缩进），在 parse_call_args（含尾部逗号）、parse_list_or_comp（含 listcomp for/if 分行）、parse_paren_or_tuple、parse_postfix 索引/切片四处括号上下文加入跨行容忍（对齐 parse_brace 既有模式，不动 lexer → 现有 golden 零漂移）。重建 pxpar/pxi/pxc（--no-quic）→ 探针 C1-C9 全过、双模式输出逐字节一致；新增 s15_multiline.px 用例 + 四类 golden、cases_bad parse_b11/b12（未闭合仍报错）+ errors golden；diffcheck --all 与 --errors 全绿（现有 golden 零漂移）。**commit c4571b7（20:00）**。
- **S3 模块顶层状态**（2026-09-05）：实证澄清——主程序同文件顶层 let/var 跨函数早已支持（codegen cg_globals → px_get/set_global，探针 g1a/g1b 双模式一致）；G1 真缺口 = **import 不导出非 Const 顶层 VarDecl**（实测 import mystate → 双模式「未定义变量 g_state」）。修复 cg_module.px：cg_is_definition/cg_def_name 的 VarDecl 分支由「仅 Const」放宽为全部 VarDecl（let/var/const）→ import 合并模块级状态槽，主程序顶层执行 VarDecl 初始化（px_set_global 注册）。双模式共用 cg_resolve_modules → 一处修复双模式生效。验证：mystate 探针 import set/get 双模式一致（42）；新增 v04_module_state.px（9 断言：模块函数读写/主程序直名读写/let 只读/无副作用）pxi+pxc 双模式 9 PASS；副本声明（compiler 链各文件同名声明显式）导出后同值覆盖无害——自举证明通过（新 golden/compiler.c 6992 行，B.c==A.c 逐字节）。diffcheck --all/--errors 全绿。注意：模块顶层 var 初始化表达式随合并于主程序启动执行一次（import 副作用边界文档化，S5 写库规范更新）。**commit 66af554（20:26）**。
- **S2 fmt 多行收口**（2026-09-05）：fmt_core 为 token 流重排架构 → **零代码改动**；新增 examples/m70_langfix/verify_fmt_multiline.sh（① 多行输入→golden 结构保留+规范化 ② 幂等 ③ 重 lex 语义等价 ④ fmt 输出 pxi 往返一致 ⑤ --check）全过。**commit 3cf0b6d（20:01）**。
- **S4 回归总闸（2026-09-05，20:40-20:59）**：侦查发现 S1/S3 期间重建的 bootstrap/pxi pxc pxpar 为 **--no-quic 裁剪态**（pxi 4245944B，vs M68/M69 发布物全能力 pxi 9326544B）→ capability.px quic 段（M46-M54）FAIL（pxi 178/180）、双模式不一致 → **判定：M70 发布物须全能力重建**（与 M68/M69 对齐，quic/h3 解释能力保持，M68「差集 155 含 quic/h3」成果不丢）。`tools/pxc build`（不带 --no-quic）三连：interp.px → pxi **9355256B**（real 9m10s）/ compiler.px → pxc **9275232B**（real 5m0s）/ parser.px → pxpar **9142352B**（real 1m49s）。最终回归（全能力产物下）**全绿**：① capability 双模式 **253 PASS 逐字节一致**（CAPABILITY_DIFF_IDENTICAL，capability.px 全能力重编 9098040B）；② diffcheck --all rc=0；③ diffcheck --errors rc=0；④ m62/m64_fmt/m64_lint/m66_yaml(35)/pxml/lunar/m69_registry(11 断言)/m70_fmt verify 全过 + m63_langfix（mock :18080 起后）ALL OK；⑤ fmt --check（selfhost+tools+stdlib）0 错、lint compiler.px + tools 0 错；⑥ ci.yml/release.yml YAML OK、make_release.sh bash -n OK。**噪音排除记录**：m63 verify 初跑 rc=1 = mock server 未起（环境依赖，起后 ALL OK）；regress 初跑 capability DIFFERS = --no-quic 裁剪态（全能力重建后 IDENTICAL）。**commit 见收口**。
- **S5 文档收口（2026-09-05）**：spec §4.1（表达式跨行规则：括号内换行/语句边界/缩进栈约束/dict 判定）+ §5.1（顶层 let·var = 全局状态槽 px_set_global + import 导出模块顶层 let/var/const + 同名冲突用户优先 + let 仍 E3002）+ §8.4（import 无副作用边界：仅顶层 var/let/const 声明随合并导出并初始化一次）修订；MINI_SUBSET §四 #1/#5、§九 #7、§十三 #5、§十四 L4 更新（G1/G2 从「限制」改「已支持+规则」）；ECOSYSTEM_GAPS G1/G2 标记 M70 已修（保留历史评估供追溯）；CHEATSHEET §0.3（import 导出）、§1 类型表（list 可跨行）、§1 坑 3/6/7（表达式跨行/顶层全局状态槽/import 副作用边界）同步 M70 语义；ROADMAP M70 行；CHANGELOG M70 条目；README(.en) 原生开发表补 M68/M69/M70 三行。**commit 见收口**。


（每完成一步在此追加：日期/commit/验证结果）
