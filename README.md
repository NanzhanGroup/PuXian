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
| 🏃 双模式 | 脚本模式（解释执行，秒起） / 编译模式（生成 C → gcc 静态二进制，接近 C 性能），`run` 与 `build` 逐字节一致 |
| 🔀 并发 | `spawn` 真并发、`channel` 阻塞通信、`select` 随机就绪 + **并发 GC**（stop-the-world 全量回收，线程安全） |
| ⏱ 定时器 | `set_timeout` / `set_interval` / `clear_timer`（一次性/周期回调，可变参数透传，回调内并发原语安全） |
| 🧹 内存 | 编译模式 C 运行时内置保守标记-清除 GC（循环引用可回收，自动触发）+ **slab 分配器**（21 档 size-class 槽位复用）；解释器侧**追踪式 GC** 回收循环引用（list/dict/chan/**闭包 Func↔Env 循环**）+ `gc()` 强制回收 |
| 🧩 模块化 | `import std.*` / `import foo.bar` / `from foo import x` / 相对路径导入；`px pkg` 包管理（init/add/install/list/remove） |
| 🌐 网络 | HTTP 客户端（**HTTPS TLS 1.2/1.3** + gzip/chunked 自动解码 + **http/https 连接池复用** + **TLS 会话票据恢复**（同 host 断连重连缩短握手）+ **流式 gzip 边下边解**）+ **HTTP 服务端**（`http_serve`，含 gzip/chunked 响应、keep-alive、file 流式）+ **WebSocket**（RFC 6455，心跳/超时）+ **SSE** 服务端/客户端（LLM 流式推送基石）+ TCP 全功能 |
| 🛡 加密/文档 | **AES-CBC-PKCS7 / AES-GCM**、**RSA**（PKCS#1 v1.5）、**XML** 解析/转义/**生成**（xml_build）、**zip** 打包/解压、**base64**、sha256 / xxhash |
| 🔢 语言能力 | 切片语法 `a[i:j]` / `a[i:j:k]`（步长/反转，str 按 UTF-8 字符）、位运算 + 二进制数据视图（int_to_hex / bytes_to_hex / bit_count / bit_length）、正则表达式、锁原语（mutex / rwlock）、文件随机读写 + fsync、进程/信号（os_spawn / os_wait / signal） |
| 🚀 应用平台 | **.px 脚本执行机制**（`px_serve` PHP/OpenResty 式应用服务器、`px_exec` 语言层嵌入 API）+ **.px 进程池**（编译模式预派生 `px --worker` 解释器常驻复用，PHP-FPM 风格，超时 kill/崩溃补位） |
| 🔧 工具链 | `px fmt`（`-w` 写回 / `--check` 检查 / `--diff` 打印 unified diff）/ `px lint` / `px test` / `px bench` / `px doc` / `px ast` 全内置 |
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
| `px fmt <file.px>` | 代码格式化（`-w` 写回 / `--check` 检查 / `--diff` 打印 unified diff） |
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
| [docs/plan.md](docs/plan.md) | 开发方案（语言命名、里程碑规划、语言要点） |
| [docs/spec.md](docs/spec.md) | 语言规格说明书（词法 / 语法 / 语义 / 标准库） |
| [docs/PROGRESS.md](docs/PROGRESS.md) | 开发进度（M0–M26 产出与验证记录，含 M27 候选） |

---

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
| M11 | 并发 GC（spawn 活跃时 stop-the-world 全量回收） | ✅ |
| M12 | 文件随机读写 + fsync（存储基石） | ✅ |
| M13 | 锁原语：mutex / rwlock | ✅ |
| M14 | crypto 哈希：sha256 / xxhash | ✅ |
| M15 | 正则表达式（回溯引擎） | ✅ |
| M16 | HTTP 服务端框架（http_serve） | ✅ |
| M17 | .px 脚本执行机制（px_serve / px_exec，PHP 式应用平台） | ✅ |
| M18 | 后台定时任务/定时器（set_timeout / set_interval / clear_timer） | ✅ |
| M19 | AES 加密 + XML 解析 + zip 打包/解压 | ✅ |
| M20 | C 运行时内部符号统一（lx_/LX_ → px_/PX_） | ✅ |
| M21 | HTTP chunked + gzip + 切片语法 + base64 + SSE | ✅ |
| M22 | slab 分配器 + 解释器追踪式 GC + WebSocket + 位运算/二进制数据视图 | ✅ |
| M23 | 网络/存储/安全收尾：SSE 客户端 + WS 心跳/超时 + 二进制安全 bytes + HTTP keep-alive/连接池/流式 + 进程/信号 + RSA | ✅ |
| M24 | XML 生成 + 切片步长 + https 连接池复用 + HTTP 流式 gzip 解压 | ✅ |
| M25 | 闭包循环回收 + .px 进程池化 + TLS 会话票据恢复 + fmt --diff | ✅ |
| M26 | 无符号右移 `>>>` + WebSocket 内置心跳 + SSE 客户端 https + 远程包 registry | ✅ |

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
- `https_demo.px` —— HTTPS（TLS）
- `std_demo.px` —— 标准库
- `toolchain_demo.px` —— 工具链演示
- `p0_random_io.px` —— 文件随机读写 + fsync
- `p1_mutex_rwlock.px` —— 锁原语
- `p2_crypto_hash.px` —— sha256 / xxhash
- `p3_regex.px` —— 正则表达式
- `p4_http_server.px` —— HTTP 服务端
- `p5_px_serve.px` —— .px 脚本应用服务器
- `p6_timer.px` —— 定时器
- `p7_aes_xml_zip.px` —— AES + XML + zip
- `p8_slice_base64.px` —— 切片 + base64
- `p9_http_adv.px` —— gzip/chunked/SSE
- `m22_bitwise_data.px` —— 位运算/二进制数据视图
- `m22_websocket.px` —— WebSocket 客户端
- `m22_tracing_gc.px` —— 解释器循环引用回收
- `m23a_sse_ws.px` —— SSE 客户端 + WebSocket 心跳/超时
- `m23b_bytes.px` —— 二进制安全字节串
- `m23c_http_adv.px` —— HTTP keep-alive/连接池/流式
- `m23d_proc_signal.px` / `m23d_rsa.px` —— 进程/信号 / RSA
- `m24_slice_xml.px` —— 切片步长 + XML 生成
- `m24_http_adv.px` —— https 连接池 + HTTP 流式 gzip
- `m25_closure_gc.px` —— 闭包循环回收
- `m25_tls_resume.px` —— TLS 会话票据恢复
- `m26_ushr.px` —— 无符号右移 `>>>`
- `m26_ws_heartbeat.px` —— WebSocket 内置心跳
- `m26_sse_https.px` —— SSE 客户端 https（需本地 https SSE 端点 + PX_TLS_CA_FILE）

```bash
px run examples/fib.px
px build examples/fib.px -o /tmp/fib && /tmp/fib
```

---

## 目录结构

```
├── compiler/          # 编译器本体（Rust）
│   ├── src/           # lexer / parser / ast / interp / codegen / fmt / lint / test / bench / doc / lsp / mcp / pkg / module ...
│   ├── runtime/       # C 运行时（runtime.c / runtime.h / runtime_aes.c / runtime_xml.c / runtime_zip.c / runtime_ws.c / runtime_rsa.c，含 GC + slab 分配器 + HTTP 连接池；mbedtls / third_party/miniz 第三方库）
│   └── tests/         # 测试（含 GC 单元测试）
├── docs/              # 需求 / 方案 / 规格 / 进度
├── examples/          # 示例程序（.px）
└── stdlib/            # 标准库自举（.px）
```

---

## 架构

```
源码 (Python-like .px)
   │
   ├─→ 解释器（AST 直译）→ 脚本模式：秒起、无编译
   │
   └─→ C 代码生成 → gcc/clang → 静态二进制（编译模式）
        │
        └─ runtime.c：对象模型 + 协程调度 + 保守标记-清除 GC + slab 分配器
```

- 前端（词法 / 语法 / AST）同一套，双模式共享语义，`run` 与 `build` 行为一致；
- 编译器本体用 Rust 实现（内存安全，AI 生成质量高）；编译后端输出 C，最终二进制链 gcc，信创场景干净。

---

## 备注

- 目标平台：仅 Linux（x86_64 / aarch64）；
- 编译模式产物为静态链接二进制（`ldd` 显示 `statically linked`）；
- C 运行时内部符号统一使用 `px_` / `PX_` 前缀（M20 全量迁移完成），第三方库（mbedtls / miniz）保留各自前缀；
- 回归基线：`cargo test` 157/157、并发 GC 60/60 稳定、示例双模式逐字节一致。
