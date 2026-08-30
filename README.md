# 普贤 PuXian（PX）

> **Python 的脸 + Rust 的类型 + Go 的并发 + C 的出身**
> 面向 AI 高效编程的自有语言，双模式运行于 Linux。

普贤（PuXian，`.px`）是一门从零实现的自有编程语言，核心定位是**让 AI（大模型）高效地编写可靠程序**：

- **语法 = Python 子集**：AI 在海量 Python 语料上训练，语法越像 Python，AI 生成的代码准确率越高；
- **类型 = Rust 风格**：渐进类型（不写类型直接跑，写类型拿性能）、枚举、模式匹配、Option/Result；
- **并发 = Go 风格**：`spawn` + `channel` + `select`，协程真并发；
- **出身 = C 编译器**：编译后端生成 C 源码，经 gcc 静态编译为**零依赖单二进制**（无 Go/Google、无 Rust/Mozilla 色彩，信创/政府场景友好）；
- **AI 协议层内置**：fmt / lint / test / bench / doc / LSP / MCP 全套工具链，AI 与 IDE 零配置接入。

---

## 特性一览

| 维度 | 能力 |
|------|------|
| 🏃 双模式 | 脚本模式（解释执行，秒起） / 编译模式（生成 C → gcc 静态二进制，接近 C 性能） |
| 🔀 并发 | `spawn` 真并发、`channel` 阻塞通信、`select` 随机就绪，线程安全运行时 |
| 🧹 内存 | 编译模式 C 运行时内置保守标记-清除 GC（循环引用可回收，自动触发） |
| 🧩 模块化 | `import std.*` / `import foo.bar` / `from foo import x` / 相对路径导入；`px pkg` 包管理（init/add/install/list/remove） |
| 🔒 网络 | `http_get` / `http_post` 支持 **HTTPS（TLS 1.2/1.3）** + 自动重定向；TCP 全功能 |
| 🔧 工具链 | `px fmt` / `px lint` / `px test` / `px bench` / `px doc` / `px ast` 全内置 |
| 🤖 AI 接入 | `px lsp`（语言服务器）、`px mcp`（MCP 服务器），AI 客户端可直接驱动 |
| 📚 标准库 | io / fs / json / time / string / math / collections / os / net（含 .px 自举库） |

---

## 快速开始

### 构建编译器（Rust 工具链）

```bash
cd compiler
cargo build --release
# 产物：target/release/px
```

> M10 起解释器依赖 rustls + webpki-roots（纯 Rust TLS），**首次构建需联网拉取依赖**（crates.io 或镜像）；
> 编译模式内嵌 mbedtls 静态库（`compiler/runtime/mbedtls/`，随仓库提交），克隆即能 `px build` HTTPS 程序。

### Hello World

```python
# hello.px
def main():
    let msg = "hello, 普贤\n"
    print(msg |> to_upper())
```

```bash
px run hello.px              # 脚本模式：解释执行，秒起
px build hello.px -o hello   # 编译模式：生成 C → gcc 静态二进制
./hello                      # 直接运行，零依赖
```

### CLI 一览

| 命令 | 说明 |
|------|------|
| `px run <file.px>` | 脚本模式执行 |
| `px build <file.px> -o out` | 编译为静态二进制 |
| `px repl` | 交互式 REPL |
| `px fmt <file.px>` | 代码格式化（`-w` 写回 / `--check` 检查） |
| `px lint <file.px>` | 静态检查（`--strict` 时 Warning 也失败） |
| `px test <file.px>` | 测试运行器（`def test_*`） |
| `px bench <file.px> <func>` | 基准测试（`--count` / `--repeat`） |
| `px doc <file.px>` | 文档生成（`--output out.md`） |
| `px ast <file.px>` | 树形查看 AST |
| `px lsp` | 语言服务器（LSP 协议） |
| `px mcp` | MCP 服务器（AI 工具调用） |
| `px pkg ...` | 包管理：`init` / `add` / `install` / `list` / `remove` |

---

## 文档

| 文档 | 说明 |
|------|------|
| [docs/requirements.md](docs/requirements.md) | 需求与设计讨论（动机、取舍、双模式架构） |
| [docs/plan.md](docs/plan.md) | 开发方案（命名、里程碑规划、语言要点） |
| [docs/spec.md](docs/spec.md) | 语言规格说明书（词法 / 语法 / 语义 / 标准库） |
| [docs/PROGRESS.md](docs/PROGRESS.md) | 开发进度（M0–M10 产出与验证记录） |

## 里程碑进度

| 阶段 | 内容 | 状态 |
|------|------|------|
| M0 | 需求、方案、规格文档 | ✅ |
| M1 | 词法分析 + 语法分析 | ✅ |
| M2 | 脚本模式解释器 | ✅ |
| M3 | 并发运行时（spawn/channel/select） | ✅ |
| M4 | C 代码生成（编译模式） | ✅ |
| M5 | 标准库（io/fs/json/time/string/math/collections/os/net） | ✅ |
| M6 | AI 工具链（fmt/lint/test/bench/doc/ast） | ✅ |
| M7 | LSP / MCP（AI 协议接入） | ✅ |
| M8 | GC 值对象自动释放（编译模式 C 运行时） | ✅ |
| M9 | 包管理器 / 模块化（px pkg + import 多文件） | ✅ |
| M10 | HTTPS（std.net 加密网络：TLS 1.2/1.3 + 重定向 + POST） | ✅ |

---

## 示例

`examples/` 目录：

- `hello.px` —— Hello World（管道操作符）
- `fib.px` —— 斐波那契
- `struct.px` —— 结构体
- `match.px` —— 模式匹配
- `concurrent.px` / `concurrent_m3.px` —— 并发（spawn / channel / select）
- `gc_demo.px` —— GC 演示（内存对照）
- `net_demo.px` —— HTTP 客户端
- `std_demo.px` —— 标准库
- `toolchain_demo.px` —— 工具链演示

```bash
px run examples/fib.px
px build examples/fib.px -o /tmp/fib && /tmp/fib
```

## 目录结构

```
├── compiler/          # 编译器本体（Rust）
│   ├── src/           # lexer / parser / ast / interp / codegen / fmt / lint / test / bench / doc / lsp / mcp / pkg / module ...
│   ├── runtime/       # C 运行时（runtime.c / runtime.h，含 GC）
│   └── tests/         # 测试（含 GC 单元测试）
├── docs/              # 需求 / 方案 / 规格 / 进度
├── examples/          # 示例程序（.px）
└── stdlib/            # 标准库自举（.px）
```

## 架构

```
源码 (Python-like .px)
   │
   ├─→ 解释器（AST 直译）→ 脚本模式：秒起、无编译
   │
   └─→ C 代码生成 → gcc/clang → 静态二进制（编译模式）
        │
        └─ runtime.c：对象模型 + 协程调度 + 保守标记-清除 GC
```

- 前端（词法 / 语法 / AST）同一套，双模式共享语义，`run` 与 `build` 行为一致；
- 编译器本体用 Rust 实现（内存安全，AI 生成质量高）；编译后端输出 C，最终二进制链 gcc，信创场景干净。

## 备注

- 目标平台：仅 Linux（x86_64 / aarch64）；
- 编译模式产物为静态链接二进制（`ldd` 显示 `statically linked`）；
- C 运行时内部符号 `lx_` / `LX_` 前缀为历史遗留（原工作名"灵犀"），仅内部实现细节，对外不可见。
