# ECOSYSTEM_GAPS · 写库规范与语言缺口评估（M69-S4）

> 用途：① 给**生态库/复用模块作者**一份可照做的写库 checklist；② 把写库时踩到的**语言缺口**逐一评估入档——每条给影响场景/绕行规范/修复成本收益/结论；**语义修复不在 M69 执行**（M69 铁律：不改 compiler/parser/语言语义），拆 **M70 候选**记录在案。
> 建立：M69-S4（2026-09-05）· 依据：M69 全程实证 + `docs/MINI_SUBSET.md` §四/§八/§九/§十三 + 历史里程碑记录。
> 配套：写码速查见 `docs/PUXIAN_CHEATSHEET.md §1`（11 条易错事实）。

---

## 1. 写库规范 checklist（发布库/复用模块前逐项过）

- [ ] **纯函数优先**：库顶层全 `def`，无 IO/无状态 → 编译（pxc build）与解释（pxi run）天然一致（生态库底线，9 个 stdlib 全部如此）。
- [ ] **顶层只导出 def/struct/enum/trait/impl/const**；需要导出的常量用 `const`；**不写顶层 `let/var` 状态**（模块顶层 var 不跨函数、不导出，见 G1）。
- [ ] **空 dict**：`json_parse("{}")`（`{}` 字面量 = null，见 F1）；dict 写 `.set(k,v)`、查 `.has(k)`、读 `d[k]`。
- [ ] **集合字面量与函数调用单行**（多行 → E2001，见 G2）；长表用循环 `.append`/`.set` 或 `"""` 多行字符串。
- [ ] 参数名**不用 `fn`**（`fn` 是匿名函数关键字）；用 `f`/`g` 等。
- [ ] 错误走 **Result**（`Ok(x)`/`Err(e)` + `?`/`!`/`is_err`），致命才 `panic`。
- [ ] 库文件**不定义 `main`**（模块不导出 main，编译模式避免 fn_main 重定义）。
- [ ] 依赖外部库用 `import std.*`（内置）或相对路径 `import "path.px"`（同仓复用）；不产生 import 副作用。
- [ ] 每文件 **<500 行**（大模型友好）；过线拆多文件 import。
- [ ] 通过 `pxc fmt` + `pxc lint`（0 错）+ `pxc doc`（## 注释齐全）；`tools/pxc run` 与 `tools/pxc build` 双模式输出一致。
- [ ] 需要版本化分发 → 发布到 registry：`registry/<name>/<version>/<name>.px`（见 `registry/README.md`）。
- [ ] 若含并发/常驻服务（spawn/chan/http_serve 回调）→ 注释标注**需编译模式**（pxi Mini 子集无 spawn，见 F3）。

## 2. 语言缺口评估（M69 只评估入档，修复拆 M70 候选）

### G1 · 模块顶层 `let/var` 不可跨函数访问 / 不导出

- **现象**（MINI_SUBSET §八.1/§九.7 记录）：顶层 `let` 变量生成局部 C 变量，跨函数访问报「未定义变量」；模块顶层 `let` **不导出**（import 只合并 def/struct/enum/trait/impl/const）；`const` 可导出。顶层 `let/var` 即全局变量（函数内可读写）仅限主程序同一文件流程。
- **影响场景**：想写「带模块级状态/配置中心」的库（计数器、缓存、单例、读一次配置全局用）时被迫显式传状态。
- **绕行规范（当前）**：纯函数显式传状态；模块常量用 `const`；配置由调用方传入。
- **修复成本/收益**：需 codegen 把顶层 var/let 真正全局化（`px_set_global`）+ import 合并非 Const 的顶层 VarDecl；触及 parser/codegen/双模式对拍，回归面大。收益：生态库可写有状态模块。
- **结论**：**建议修复（M70 候选）**，与 M68/M69 范围分离（改语言语义须独立排期）。

### G2 · 多行 list/dict 字面量与多行函数调用不支持

- **现象**（MINI_SUBSET §八.1/#5 + M69-S2 亲历）：`[a,\n b]` / `{\n "k": v\n}` / 跨行调用参数 → **E2001 意外的 token: 换行**。语句以换行为边界（G4）。
- **影响场景**：大配置表、长参数列表、字典批量构造——只能单行写完或循环构建；AI 生成代码时最常见语法错之一。
- **绕行规范（当前）**：集合字面量单行；长数据用循环 `.append`/`.set`；结构文本用 `"""` 多行字符串 + `yaml_parse`/`pxml_parse` 载入。
- **修复成本/收益**：parser 允许表达式内换行（需区分语句边界与表达式内部换行的 token 语义）+ fmt 规则同步 + diffcheck 全量对拍；收益 = 写库体验最大痛点，直接影响 AI 生成正确率。
- **结论**：**建议修复（M70 候选，优先级最高）**。

### G3 · `let` 不可变（赋值报 E3002）

- **现象**：`let` 默认不可变，对其赋值 = 编译错误 E3002（M41.2 实现）；要可变用 `let mut` 或 `var`。
- **评估**：**有意设计**（默认不可变，意图清晰），非缺口。写库约定：常量/不变量用 `let`，可变用 `var`。
- **结论**：**保持现状**，不做修复；写库规范已列。

### G4 · 无分号、语句以换行分隔（缩进块语言）

- **现象**：语句以换行结束，无分号（`;`）；`if/for/while/fn` 体用缩进；if 表达式 `let x = if c: a else: b` 为单行形式。
- **影响**：与 G2 互为因果（表达式不能跨行）；AI 需遵守「行 = 语句」心智。
- **评估**：加「表达式续行」的收益主要在 G2；引入分号语义 = lexer/parser 大改且与缩进风格冲突，**无收益**。
- **结论**：**不做分号**；表达式跨行并入 G2 一起考虑。

## 3. M69 实证新发现（写库/用库必读，均已绕行或修复）

| # | 事实 | 实证 | 处理 |
|---|---|---|---|
| F1 | **`{}` 字面量 = `null`**（不是空 dict） | M69-S1：`stdlib/collections.px group_by` 用 `result = {}` 致返回 null / 双模式不可用（M5 历史 bug，从未被调用测试暴露） | 已修复（`json_parse("{}")`，M69-S1）；ECOSYSTEM §6 + CHEATSHEET §1 警示 |
| F2 | **无 `d[k] = v` dict 赋值** | M69-S1 实证 R1002「索引赋值目标不支持」 | 写库规范用 `.set(k,v)`；与 F1 同源（`{}`=null 时对 null 赋值） |
| F3 | **pxi Mini 子集无 `spawn`/`chan`** | M69-S2 AI 自测：http_serve 服务端 pxi 报「interp 不支持 spawn」 | 服务端/并发程序标注编译模式；CHEATSHEET §1.11 |
| F4 | **纯普贤逐行扫大文件慢**（解释器字符串密集循环） | M69-S2：gen_native_table 普贤版扫 runtime.c 5000+ 行超时 → 改 bash+grep（281 项秒级） | 工具选型：重文本/大文件用 C 或 shell；普贤适合业务/IO 编排/小文件 |
| F5 | **import 双路径并存**：`import std.x`（stdlib 内置）与裸名 `import x`（`.px_modules` registry 包）同源不同路径 | M69-S3 verify | registry/README + spec §8.6.3 文档化 |

## 4. 拆 M70 候选（记录在案，M69 不执行）

- **M70 候选 A · 表达式跨行**（G2）：parser 支持多行 list/dict 字面量与多行调用参数（含 fmt/diffcheck 回归）——写库体验第一痛点。
- **M70 候选 B · 模块顶层状态**（G1）：顶层 var/let 全局化 + import 导出非 Const 顶层变量（codegen + cg_module 语义）。
- 两者都碰 compiler/parser/语言语义 → 必须独立里程碑排期（参照 M62/M63 语言面修复先例：diffcheck --all + capability 双模式 + pxi 重建 + 自举证明全套回归）。

## 5. 与既有文档对齐

- `docs/MINI_SUBSET.md`：§四 已知限制、§八 M-B2 限制、§九 M-B5 限制、§十三 语言欠账——本文档为「写库视角」重述与评估，不新增语义改动。
- `docs/PUXIAN_CHEATSHEET.md §1`：11 条易错事实（含 F1/F2/F3 速查）。
- `docs/ECOSYSTEM.md §6`：生态健康速查（F1/F2 已在）。
- qg-issue 00-README 已知缺口段：本文件可作为仓库内权威缺口评估副本；仓库外 qg-issue 侧引用本文档。
