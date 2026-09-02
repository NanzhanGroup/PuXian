# 普贤 PuXian（PX）

> **Python 的脸 + Rust 的类型 + Go 的并发 + C 的出身**
> 面向 AI 高效编程的自有语言，双模式运行于 Linux。

普贤（PuXian，`.px`）是一门从零实现的自有编程语言，核心定位是**让 AI（大模型）高效地编写可靠程序**：

- **语法 = Python 子集**：AI 在海量 Python 语料上训练，语法越像 Python，AI 生成的代码准确率越高；
- **类型 = Rust 风格**：渐进类型（不写类型直接跑，写类型拿性能）、枚举、模式匹配、Option/Result；
- **并发 = Go 风格**：`spawn` + `channel` + `select`，协程真并发；
- **出身 = C 编译器**：编译后端生成 C 源码，经 gcc 静态编译为**零依赖单二进制**（无 Go/Google、无 Rust/Mozilla 色彩，信创/政府场景友好）。

---

## 🎉 当前状态：编译器已自举

| 状态 | 说明 |
|---|---|
| ✅ **自举完成（M-B8）** | **PuXian 编译器由 PuXian 自己写成**：`lexer / parser / codegen / interp / 值系统` 五大核心全部用 `.px` 重写，自举证明 A.c == B.c == B2.c 逐字节一致 |
| ✅ **Rust 版已退役（M-B9a）** | Rust 源码归档至 `archive/rust-compiler/`（只读），**新工具链 `tools/pxc` 完全无需 Rust**，基于自举二进制运行 |
| ✅ **CI 已接入** | GitHub Actions：每次提交自动跑回归 + 自举证明 + 示例编译 + ws-web 冒烟 |
| 🔄 **ws-web 开发中（M-B9b）** | 用 PuXian 写第一个生产应用（HTTP + SQLite 服务），见 `ws-web/` |

> 现在克隆仓库即可用 `tools/pxc` 编译/运行 PuXian 程序，**不需要安装 Rust**。

---

## 快速开始

### 准备（唯一依赖：gcc + make）

```bash
# Linux（x86_64 / aarch64），确认 gcc 可用
which gcc
# 仓库自带自举工具链（bootstrap/pxc 编译器 + runtime C 运行时），克隆即用
```

### Hello World

```python
# hello.px
def main():
    let msg = "hello, 普贤\n"
    print(msg |> to_upper())
```

```bash
./tools/pxc run hello.px              # 脚本模式：解释执行，秒起
./tools/pxc build hello.px            # 编译模式：生成 C → gcc 静态二进制
./hello/build/hello                   # 直接运行，零依赖（产物在 <目录>/build/）
```

### CLI 一览（`tools/pxc`）

| 命令 | 说明 |
|---|---|
| `pxc build <file.px>` | 编译为静态二进制（输出 `<目录>/build/<name>`） |
| `pxc run <file.px> [args...]` | 脚本模式执行 |
| `pxc lex <file.px>` | 打印 Token 流（调试，走 PuXian lexer） |
| `pxc parse <file.px>` | 打印 AST（调试，走 PuXian parser） |
| `pxc --version` / `-v` | 输出版本号 |
| `pxc help` | 帮助 |

> **工具链现状说明**：自举版 `pxc` 当前提供上表核心命令；Rust 版曾有的 `fmt / lint / test / bench / doc / lsp / mcp / pkg` 等完整工具链，其源码保留在 `archive/rust-compiler/`（只读归档，可作参考或按需用 PuXian 重写恢复）。

---

## 特性一览

| 维度 | 能力 |
|------|------|
| 🏃 双模式 | 脚本模式（解释执行，秒起）/ 编译模式（生成 C → gcc 静态二进制，接近 C 性能），`run` 与 `build` 产物逐字节一致 |
| 🔀 并发 | `spawn` 真并发、`channel` 阻塞通信、`select` 随机就绪 + **并发 GC**（stop-the-world 全量回收，线程安全） |
| ⏱ 定时器 | `set_timeout` / `set_interval` / `clear_timer`（一次性/周期回调，可变参数透传，回调内并发原语安全） |
| 🧹 内存 | 编译模式 C 运行时内置保守标记-清除 GC（循环引用可回收，自动触发）+ **slab 分配器**（21 档 size-class 槽位复用）；解释器侧**追踪式 GC** 回收循环引用（list/dict/chan/**闭包 Func↔Env 循环**）+ `gc()` 强制回收 |
| 🧩 模块化 | `import std.*` / `import foo.bar` / `from foo import x` / 相对路径导入 |
| 🌐 网络 | HTTP 客户端（**HTTPS TLS 1.2/1.3** + gzip/chunked 自动解码 + **http/https 连接池复用** + **TLS 会话票据恢复** + **流式 gzip 边下边解**）+ **HTTP 服务端**（`http_serve` gzip/chunked/keep-alive/流式 + **`px_serve` 服务端 TLS**：`tls_server(cert,key[,hostname])` 注册后 HTTPS/WSS/SSE-over-TLS + **TLS SNI 多证书按域名选择** + 请求体大小可配 + 413 + 大 body 落盘 + **优雅关闭** + **per-route 限流**（路由粒度 429）+ **访问日志落盘轮转** + **Alt-Svc 通告**）+ **WebSocket**（RFC 6455，心跳/超时，**`ws://`/`wss://` 一行连接**）+ **SSE** 服务端/客户端（**断线自动重连**，带 Last-Event-ID）+ **UDP**（udp_open/send/recv/close，QUIC 预研地基）+ TCP 全功能 |
| 🛡 加密/文档 | **AES-CBC-PKCS7 / AES-GCM**、**RSA**（PKCS#1 v1.5）、**XML** 解析/转义/**生成**（xml_build）、**zip** 打包/解压、**base64**、sha256 / xxhash、**SQLite**（open/exec/query/close，参数绑定+结果集） |
| 🔢 语言能力 | 切片语法 `a[i:j]` / `a[i:j:k]`（步长/反转，str 按 UTF-8 字符）、**生成器表达式** `(x for x in xs)`（**惰性**：单层 for 延迟求值 / `gen_next` 逐项 / for-in / `list()` 转换）、位运算 + 二进制数据视图（int_to_hex / bytes_to_hex / bit_count / bit_length）、正则表达式、锁原语（mutex / rwlock）、文件随机读写 + fsync、进程/信号（os_spawn / os_wait / signal）、**Result/Option 错误处理**（`Ok(x)`/`Err(e)`/`Some(x)` 构造，`?` 错误传播——Err/None 立即返回、`!` 强制解包、is_ok/is_err/unwrap 方法，spec 唯一错误通道）、字符串插值 `${expr}`、推导式、可选链 `?.`、空合并 `??`、管道 `\|>` |
| 🚀 应用平台 | **.px 脚本执行机制**（`px_serve` PHP/OpenResty 式应用服务器：Cookie/Session/基础认证 + 服务端 TLS + 优雅关闭、`px_exec` 语言层嵌入 API）+ **.px 进程池**（编译模式预派生 worker 解释器常驻复用，PHP-FPM 风格，**脚本/二进制变更自动滚动重启热更新**）+ 路由表+中间件（method+path 模式 / `:id` 参数 / `*` 通配 / 中间件链）+ cron 调度（6 字段）+ JSON 路径（json_path/json_path_set） |
| 📚 标准库 | `stdlib/collections.px`（sorted/reversed/map/filter/reduce/unique/group_by）+ 内置注册表白名单（见 MINI_SUBSET §2.5） |

---

## 自举（Bootstrapping）

PuXian 最与众不同的地方：**它的编译器是它自己写的**。2026 年完成了一轮完整自举：

### 自举链路（Bootstrap Chain）

```
selfhost/*.px（PuXian 源码）───编译───► bootstrap/pxc（编译器二进制，入库）
                                          │ 编译任何 .px
                                          ▼
                                     C 源码 + runtime/ ──gcc──► 静态二进制
```

| 组件 | 说明 |
|---|---|
| `bootstrap/pxc` | PuXian 版编译器（静态二进制，由 `selfhost/compiler.px` 编译而来，随仓库提交） |
| `bootstrap/pxi` | PuXian 版解释器（由 `selfhost/interp.px` 编译而来） |
| `bootstrap/pxl` / `pxpar` | PuXian 版 lexer / parser（调试用） |
| `selfhost/compiler.px` | **编译器源码（PuXian 自己）**：import codegen.px（pxlexer→parser→cg_module→codegen）全链 |
| `selfhost/golden/compiler.c` | 自举基准（6003 行 C）：引导编译器编译自身的一次性产物 |
| `selfhost/bootstrap_prove.sh` | 自举证明：`bootstrap/pxc` 编译 `compiler.px` 与基准逐字节 diff |

### 自举证明（经典三步）

1. 编译器 A（`bootstrap/pxc`）运行 `build compiler.px` → 生成 B.c；
2. `B.c` 与基准 `golden/compiler.c` **逐字节一致（6002 行 0 差异）** → 自举成立；
3. 强化闭环：B.c 经 gcc 编成 B 二进制 → B 再编译 compiler.px → B2.c，**A.c == B.c == B2.c 三者完全一致**。

CI 每次提交自动跑此证明（`.github/workflows/ci.yml`）。

### Mini 子集（语言面锁定）

自举期间语言被锁定为 **Mini 子集**（`docs/MINI_SUBSET.md`，语法基线 M40，图灵完备最小面）：只准修 bug 不准加特性。PuXian 版编译器只需正确编译该子集（自身源码即在子集内）。

> **已知限制**：编译模式 `str(float)` 大浮点 %g 精度（>6 位有效数字截断）；编译版无法解析含 NUL 的源码字符串；编译自己需 ~3.5min/1.6GB（C 运行时解释执行 PuXian 编译器逻辑）；编译版解释器仅 Mini 子集内置白名单（`sqlite_*/http_*` 等生产内置只在编译模式可用）。详见 MINI_SUBSET §八~§十二。

---

## 目录结构

```
├── bootstrap/              # 自举引导二进制（pxc 编译器 / pxi 解释器 / pxl lexer / pxpar parser，静态 ELF）
├── tools/pxc               # 用户入口：build / run / lex / parse / --version（bash 包装，零 Rust 依赖）
├── selfhost/               # 自举工程（核心！）
│   ├── compiler.px         #   PuXian 版完整编译器 CLI（import codegen.px 全链）
│   ├── codegen.px + cg_*.px #   codegen 模块（AST → C）
│   ├── interp.px + i*.px   #   解释器模块（tree-walking）
│   ├── lexer.px pxlexer.px #   词法分析器
│   ├── parser.px           #   语法分析器
│   ├── value.px env.px module.px  # 值系统 / 作用域 / 模块加载
│   ├── capability.px       #   能力自检（110/110）
│   ├── cases/ + golden/    #   对拍用例（s01-s09 + v01-v03）与基准产物
│   ├── cases_bad/          #   错误场景（lex 14 + parse 9）
│   └── diffcheck.sh / bootstrap_prove.sh  # 对拍框架 / 自举证明
├── runtime/                # C 运行时（runtime.c/h + aes/xml/zip/ws/rsa/sqlite/route/h2 + mbedtls + third_party）
├── stdlib/                 # 标准库（collections.px）
├── ws-web/                 # 第一个生产应用（M-B9b，清歌负责）：HTTP + SQLite 服务骨架
├── examples/               # 80+ 个示例（hello / fib / match / 并发 / 网络 / TLS / SQLite / 推导式 ...）
├── archive/rust-compiler/  # Rust 版编译器源码归档（只读，自举前的实现，git 历史保留）
├── docs/                   # 文档（规格 / Mini 子集 / 自举计划 / 进度）
└── .github/workflows/ci.yml # CI：回归 + 自举证明 + 示例编译 + ws-web 冒烟
```

---

## 文档

| 文档 | 说明 |
|---|---|
| [docs/spec.md](docs/spec.md) | 语言规格说明书（词法 / 语法 / 语义 / 标准库） |
| [docs/MINI_SUBSET.md](docs/MINI_SUBSET.md) | **Mini 子集规范**（自举编译器语言面锁定：支持特性 / 明确排除 / 已知限制） |
| [docs/BOOTSTRAP_PLAN.md](docs/BOOTSTRAP_PLAN.md) | 自举计划（M-B1 → M-B9 里程碑 + 进度表） |
| [docs/PROGRESS.md](docs/PROGRESS.md) | 功能开发进度（M0–M40） |
| [docs/DEV_HISTORY.md](docs/DEV_HISTORY.md) | 开发历史 |
| [docs/requirements.md](docs/requirements.md) | 需求与设计讨论（动机、取舍、双模式架构） |
| [ws-web/README.md](ws-web/README.md) | 第一个生产应用：快速开始 + 10 条避坑清单 + 协作协议 |

---

## 里程碑进度

### 功能开发（M0–M40，Rust 时代，全部完成 ✅）

| 阶段 | 内容 |
|---|---|
| M0–M9 | 需求/方案/规格 → 词法+语法 → 解释器 → 并发运行时 → C 代码生成 → 标准库 → AI 工具链（fmt/lint/test/bench/doc/ast）→ LSP/MCP → GC 值对象 → 包管理/模块化 |
| M10–M19 | HTTPS（TLS 1.2/1.3）→ 并发 GC → 文件随机读写+fsync → 锁原语 → sha256/xxhash → 正则 → HTTP 服务端框架 → .px 脚本执行机制 → 定时器 → AES/XML/zip |
| M20–M29 | C 运行时符号统一 → chunked/gzip/切片/base64/SSE → slab 分配器+追踪式 GC+WebSocket+位运算 → 网络/存储/安全收尾 → XML 生成+切片步长+连接池 → 闭包循环回收+进程池+TLS 票据恢复 → `>>>`+WS 心跳+远程 registry → 服务端 TLS+请求体限制+Session → 路由/时间时区/cron/SQLite → JSON 路径+静态缓存头+Range+访问日志+请求 ID |
| M30–M40 | 服务端 https 连接池+字节序整数↔bytes+推导式补全+fmt 配置化 → 沙箱+虚拟主机+限流+HTTP/2 预检+连接线程池 → ws/wss 一行连接+SSE 重连+进程池热更新+生成器 → per-route 限流+TLS SNI+访问日志轮转+QUIC 预研 → 惰性生成器+进程池配置化+WS 广播+事件总线 → gzip 解压+推导式 range+HTTP/2 最小服务端+多维限流 → 日志增强+请求上下文+WS 心跳配置+优雅关闭 → HTTP/2 over TLS+响应压缩+S3 → WS 自动重连+HTTP/2 多流+UDP echo → **Result/Option 唯一错误通道** → **字符串插值 `${expr}`** |

### 自举（M-B1 → M-B9，全部 ✅，M-B9b 进行中）

| 里程碑 | 内容 | 结果 |
|---|---|---|
| M-B1 | 能力门禁 + Mini 子集 + 对拍框架 | 能力自检 110/110 |
| M-B2 | lexer 用 PuXian 重写 | token 流对拍 9/9 |
| M-B3 | parser 用 PuXian 重写 | AST 对拍 8/8（双模式） |
| M-B4 | parser 错误处理/恢复 | 错误场景 23/23（双模式） |
| M-B5 | value/env/module 值系统 | 对拍 v01-v03 双模式全过 |
| M-B6 | codegen 用 PuXian 重写 | C 源码对拍 12/12（双模式） |
| M-B7 | interp 用 PuXian 重写 | stdout 8/8 + v01-v03 全 PASS |
| M-B8 | **自举证明** | **A.c == B.c == B2.c 逐字节一致** 🎉 |
| M-B9a | 退役 Rust 版 + 接入 CI + 引导链 | `tools/pxc` 全链路可用，CI 四 job |
| M-B9b | ws-web（第一个生产应用） | 🔄 进行中（清歌负责） |

---

## 示例

`examples/` 目录（80+ 个），快速上手：

```bash
# 解释运行
./tools/pxc run examples/fib.px
./tools/pxc run examples/match.px
./tools/pxc run examples/m39_result.px
./tools/pxc run examples/m40_str_interp.px

# 编译为静态二进制
./tools/pxc build examples/fib.px && ./examples/build/fib
./tools/pxc build examples/m28_time_sqlite.px && ./examples/build/m28_time_sqlite
```

- `hello.px` —— Hello World（管道操作符）
- `fib.px` —— 斐波那契 / `match.px` —— 模式匹配
- `concurrent.px` —— 并发（spawn / channel / select）
- `m39_result.px` —— Result/Option 错误处理（`?` / `!`）
- `m40_str_interp.px` —— 字符串插值 `${expr}`
- `m28_time_sqlite.px` —— 时间时区 + SQLite（CRUD/参数绑定）
- `m28_route.px` —— 路由表 + 中间件链（:id 参数 / * 通配）
- `m29_webprod.px` —— WebServer 生产化（静态缓存头 / Range / 请求 ID）
- `m30_comp.px` —— 推导式语法补全（多 for / 多 if / DictComp）
- `m32_gen.px` —— 生成器表达式（惰性求值）
- `m37_s3.px` —— S3/MinIO 对象存储（AWS SigV4）
- `m38_h2_multi.px` —— HTTP/2 多流
- `m39_gc.px` —— GC 演示 / `m22_tracing_gc.px` —— 循环引用回收
- `m28_cron.px` —— cron 定时调度 / `m33_route_rate_limit.px` —— 限流
- `m46_quic_verify.sh` —— QUIC 传输级回环（握手 + hello-quic-42 → echo）
- `m47_h3_verify.sh` —— HTTP/3 语义层回环（QPACK 编解码 + HEADERS/DATA 帧 + 请求/响应，GET /hello → 200）
- `m48_qpack_verify.sh` —— QPACK 完整 codec 字节精确自检（RFC Huffman 官方向量 + 静态表索引 + 容错，编译/解释双模式）
- `m49_qpack_dyn_verify.sh` —— QPACK 动态表 + SETTINGS 会话字节精确自检（动态表复用/压缩收益/SETTINGS roundtrip，编译/解释双模式）
- ... 完整列表见 `examples/`

---

## 生态与合作

- **ws-web**（`ws-web/`）：第一个生产应用，HTTP + SQLite 服务，用于真实场景 dogfooding 验证 PuXian。
- **语言缺陷上报**：发现问题请写最小复现用例（单个 .px + 期望/实际输出），标注 `ws-web-blocker` 优先处理。
- **协作分工**：清歌负责用 PuXian 写应用（M-B9b），东月负责编译器维护 + 响应 blocker issue。
