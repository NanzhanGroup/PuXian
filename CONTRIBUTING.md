# Contributing to PuXian

感谢你愿意参与 PuXian（普贤）的建设 🙏 本指南覆盖：提 Issue、提 PR、本地构建与测试、代码风格约定。

> 无论你是人类开发者还是 AI 开发者，规则一视同仁——本项目本身就是「由 AI 参与编写、面向 AI 高效编程」的 dogfooding 工程。

## 快速上手

```bash
# 1. 克隆
git clone git@github.com:NanzhanGroup/PuXian.git
cd PuXian

# 2. 唯一外部依赖：gcc（仓库自带自举工具链 bootstrap/pxc，克隆即用，零 Rust 依赖）
./tools/px --version         # 应输出 px 0.1.0（源码仓为语义版；发布包内为 px 0.1.0-mXX）

# 3. 试跑
./tools/pxc run examples/hello.px
./tools/pxc build examples/fib.px && ./examples/build/fib
```

## 目录速览

| 路径 | 内容 |
|---|---|
| `bootstrap/pxc` `bootstrap/pxi` | 自举引导二进制（编译器/解释器，静态 ELF，仓库自带） |
| `tools/px` | 用户入口（官方命令名，M86 起）：build / run / lex / parse / --version；`tools/pxc` = symlink → px 兼容别名 |
| `selfhost/` | **核心**：PuXian 写的编译器（compiler.px + codegen/interp/lexer/parser 模块） |
| `selfhost/cases*` `golden/` | 对拍用例与基准产物 |
| `runtime/` | C 运行时（runtime.c + 网络/加密/数据库等扩展 + third_party） |
| `stdlib/` | 标准库（.px） |
| `examples/` | 示例程序与里程碑验证脚本 |
| `docs/` | 语言规格（spec.md）、Mini 子集规范、路线图（ROADMAP.md） |

## 提 Issue

- **Bug**：请附**最小复现用例**（单个 `.px` 文件）+ 期望输出 / 实际输出 + 你用的命令（build 还是 run）。
  模板见 [.github/ISSUE_TEMPLATE/bug_report.md](.github/ISSUE_TEMPLATE/bug_report.md)。
- **特性请求**：请说明使用场景与预期行为，模板见 [.github/ISSUE_TEMPLATE/feature_request.md](.github/ISSUE_TEMPLATE/feature_request.md)。
- 语言/编译器类问题可用 label：`compiler` / `interp` / `runtime`。

## 本地验证（提交 PR 前必须全绿）

PuXian 是自举语言，回归门槛是**双模式（编译 + 解释）行为一致** + **自举不回归**：

```bash
# 1. 对拍回归（lexer / parser / errors / codegen / value / interp 各阶段）
./selfhost/diffcheck.sh --all
./selfhost/diffcheck.sh --errors

# 2. 能力自检（双模式一致）
./bootstrap/pxi selfhost/capability.px            # 解释模式
./tools/pxc build selfhost/capability.px && ./selfhost/build/capability   # 编译模式

# 3. 自举证明（PuXian 编译器编译自己与基准逐字节一致——改编译器后必须跑）
cd selfhost && ./bootstrap_prove.sh          # C 轨：golden/compiler.c
cd selfhost && ./bootstrap_prove_bc.sh       # BC 轨：golden/compiler.bc.dump（字节码权威基准）

# 4. 引擎一致性门（用户面默认轨 tools/px 必须与基准轨说同一句话，Issue 51）
./selfhost/engine_parity.sh

# 5. 改过编译器源码 / 基准后：重烘入库二进制（否则用户拿到的仍是旧引擎）
#    M113-S0（Issue 58）：此前只写了 `cp compiler_vm bootstrap/pxc_vm`，
#    **C 轨 `bootstrap/pxc` 无人重烘**（自 M112 起落后源码，CI 结构性发现不了）。
./selfhost/rebake_bin.sh           # 重烘 bootstrap/pxc（静态 + 全 runtime）+ bootstrap/pxc_vm
./selfhost/rebake_bin.sh --check   # 断言入库件与「当前源码现编产物」行为一致（能红）

# 6. 新增/改动功能时补对应用例（selfhost/cases/ 或 examples/）并更新 golden
#    有意改动基准时重定基（两条基准须与源码同批提交）：
#    cd selfhost && ./bootstrap_prove.sh --update-golden && ./bootstrap_prove_bc.sh --update-golden
```

> ⚠️ 自举证明 C 轨 `--fresh` 实测 ≈6.5-7 分钟（405s，8 核 16G 单线程）与约 1.6GB 内存；BC 轨全链 `--fresh` 约 12-15 分钟（见 MINI_SUBSET §十二.2）。若你的改动未触及 `selfhost/compiler.px` 及其模块，可跳过第 3 步并在 PR 描述中注明。

## 语言约束（写代码前必读）

编译器自身运行在 **Mini 子集**锁定面上（语法基线 M40），写 `.px` 时必须规避以下坑，详见 [docs/MINI_SUBSET.md](docs/MINI_SUBSET.md)：

- ❌ 匿名 fn 捕获外部变量（编译模式仅无捕获闭包）
- ❌ 函数内 `def` 嵌套函数、`match` 语句形式（case 体多语句/return）
- ❌ `try/throw`（Result + `?` 是唯一错误通道）、字符串乘法、命名参数、`*args`
- ❌ 多行 list/dict 字面量、多行 fn `fn(x):`、单行 `if x: y`、顶层 `?`
- ⚠️ 编译模式 `str(float)` 用 `%g`（6 位有效数字），避免高精度/大指数浮点字面量
- ⚠️ 空 dict 不能写 `{}`（是空块=null），需 `{"_": 0}` 后 remove

## PR 流程

1. fork 或开分支 → 改动 → 本地跑通上述验证
2. 提交信息建议风格：`M## 标题（一句话）` + 变更要点列表（参考仓库历史）
3. 打开 PR，勾选 template 中的验证清单
4. 维护者 review：一般 1-2 轮，关注点 = 双模式一致、不回归、文档同步

## 代码风格约定

- 语言代码：缩进 4 空格；标识符蛇形命名（`read_file`）；类型标注可省略（渐进类型）
- C 运行时：`runtime_*.c` 单文件自包含，函数 `px_*`/`bi_*` 前缀，符号静态
- 文档：改动语言/运行时能力时同步更新 `docs/spec.md`（规格）与 `docs/MINI_SUBSET.md`（子集/限制）

## 行为准则

保持友善、聚焦技术。任何形式的骚扰/贬低不受欢迎。详见项目开源精神（Apache-2.0 LICENSE + AUTHORS.md）。

---

有问题？开 Issue 即可，我们会尽快回复。
