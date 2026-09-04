# 普贤 PuXian（PX）

> **Python 的脸 + Rust 的类型 + Go 的并发 + C 的出身**
> 面向 AI 高效编程的自有语言，双模式运行于 Linux。

普贤（PuXian，`.px`）是一门从零实现的自有编程语言，核心定位是**让 AI（大模型）高效地编写可靠程序**：

- **语法 = Python 子集**：AI 在海量 Python 语料上训练，语法越像 Python，AI 生成的代码准确率越高；
- **类型 = Rust 风格**：渐进类型（不写类型直接跑，写类型拿性能）、枚举、模式匹配、Option/Result；
- **并发 = Go 风格**：`spawn` + `channel` + `select`，协程真并发；
- **出身 = C 编译器**：编译后端生成 C 源码，经 gcc 静态编译为**零依赖单二进制**（供应链中立、部署极简）。

---

## ⚖️ License

![License](https://img.shields.io/badge/License-Apache--2.0-blue.svg)

PuXian 采用 **Apache License 2.0** 开源 —— 任何人可自由使用、修改、分发与商用（含闭源商用），无需授权费。完整条款见根目录 [`LICENSE`](LICENSE)。

对本项目使用者最重要的几点：

- **编译产物不受许可证约束**：你用 PuXian 写出的程序/服务，可任意闭源商用，不构成衍生作品；
- 修改并分发源码时，需保留版权声明与本 License 副本（Apache 宽松协议，不要求你的修改开源）；
- **贡献方式**：项目初期以提 issue 为主（bug 报告 / 功能建议 / 使用反馈），暂不开放代码 PR，路线由核心团队掌控；
- **Copyright © 2026 The PuXian Authors**

---

## ® Trademark

**达者同游** 为南瞻集团注册商标；**PuXian** 为达者同游团队项目名称（非注册商标）。
未经书面许可，不得将上述名称用于标识非达者同游团队官方发布的产品或服务。

---

## 🎉 当前状态：编译器已自举

| 状态 | 说明 |
|---|---|
| ✅ **自举完成（M-B8）** | **PuXian 编译器由 PuXian 自己写成**：`lexer / parser / codegen / interp / 值系统` 五大核心全部用 `.px` 重写，自举证明 A.c == B.c == B2.c 逐字节一致 |
| ✅ **Rust 版已退役（M-B9a）** | Rust 源码归档至 `archive/rust-compiler/`（只读），**新工具链 `tools/pxc` 完全无需 Rust**，基于自举二进制运行 |
| ✅ **CI 已接入** | GitHub Actions：每次提交自动跑回归 + 自举证明 + 示例编译 |
| ✅ **dogfood 完成（M-B9b）** | 用 PuXian 写出第一个生产应用（HTTP + SQLite 服务），代码已迁至独立私有仓库维护 |

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
| `pxc fmt <file.px> [-w] [--check] [--diff]` | 代码格式化（M64a 自举） |
| `pxc lint <file.px> [--json] [--strict]` | 静态检查 L001-L008（M64b 自举） |
| `pxc doc <file.px> [--output out.md]` | 从 `##` 注释生成 Markdown 文档（M64c 自举） |
| `pxc test <file.px> [filter] [--list]` | 运行顶层 `def test_xxx()` 测试（M64c 自举） |
| `pxc bench <file.px> <func> [--count N] [--repeat R]` | 基准测试（M64c 自举） |
| `pxc --version` / `-v` | 输出版本号 |
| `pxc help` | 帮助 |

> **工具链现状（M64 工具链自举恢复）**：`pkg / ast / fmt / lint / test / doc / bench` 七项
> 均已由 PuXian 自举实现（`.px` 源码 → bootstrap 二进制 → `pxc` 子命令），源码见
> `tools/pxpkg.px`、`tools/pxfmt.px`、`tools/pxlint.px`、`tools/pxdoc.px`、
> `tools/pxtest.px`、`tools/pxbench.px`（Rust 版全套留档 `archive/rust-compiler/` 只读）。
> `lsp / mcp` 两项为 M64d（按需，stdio/JSON-RPC 底座侦查后启动）。

---

## 特性一览

| 维度 | 能力 |
|------|------|
| 🏃 双模式 | 脚本模式（解释执行，秒起）/ 编译模式（生成 C → gcc 静态二进制，接近 C 性能），`run` 与 `build` 产物逐字节一致 |
| 🔀 并发 | `spawn` 真并发、`channel` 阻塞通信、`select` 随机就绪 + **并发 GC**（stop-the-world 全量回收，线程安全） |
| ⏱ 定时器 | `set_timeout` / `set_interval` / `clear_timer`（一次性/周期回调，可变参数透传，回调内并发原语安全） |
| 🧹 内存 | 编译模式 C 运行时内置保守标记-清除 GC（循环引用可回收，自动触发）+ **slab 分配器**（21 档 size-class 槽位复用）；解释器侧**追踪式 GC** 回收循环引用（list/dict/chan/**闭包 Func↔Env 循环**）+ `gc()` 强制回收 |
| 🧩 模块化 | `import std.*` / `import foo.bar` / `from foo import x` / 相对路径导入 |
| 🌐 网络 | HTTP 客户端（**HTTPS TLS 1.2/1.3** + gzip/chunked 自动解码 + **http/https 连接池复用** + **TLS 会话票据恢复** + **流式 gzip 边下边解** + **Unix socket HTTP 客户端**（`http_unix(sock,path,method,...)` 本地服务/LLM 网关调用，自动补 Content-Length））+ **HTTP 服务端**（`http_serve` gzip/chunked/keep-alive/流式 + **`px_serve` 服务端 TLS**：`tls_server(cert,key[,hostname])` 注册后 HTTPS/WSS/SSE-over-TLS + **TLS SNI 多证书按域名选择** + 请求体大小可配 + 413 + 大 body 落盘 + **优雅关闭** + **per-route 限流**（路由粒度 429）+ **访问日志落盘轮转** + **Alt-Svc 通告** + **HTTP/3 三栈合一**（`px_serve(...,{http3:true|{port?,cert?,key?}})` 同端口托管 H3/QUIC，HTTP/1.1+HTTP/2+HTTP/3 共用同一 vhost/路由/限流/访问日志/静态/.px 管道；`h3_server_listen` 独立 H3 listener；**aioquic 第三方互操作**；**HTTP/3 生产化**（TLS 1.3 会话恢复 1-RTT resumption、0-RTT early data 握手前可发、连接迁移换源续传、BLOCKED_STREAMS 流控协商 -206/MAX_STREAMS）））+ **WebSocket**（RFC 6455，心跳/超时，**`ws://`/`wss://` 一行连接**）+ **SSE** 服务端/客户端（**断线自动重连**，带 Last-Event-ID）+ **UDP**（udp_open/send/recv/close）+ TCP 全功能 |
| 🛡 加密/文档 | **AES-CBC-PKCS7 / AES-GCM**、**RSA**（PKCS#1 v1.5）、**XML** 解析/转义/**生成**（xml_build）、**zip** 打包/解压、**base64**、sha256 / xxhash、**SQLite**（open/exec/query/close，参数绑定+结果集） |
| 🔢 语言能力 | 切片语法 `a[i:j]` / `a[i:j:k]`（步长/反转，str 按 UTF-8 字符）、**生成器表达式** `(x for x in xs)`（**惰性**：单层 for 延迟求值 / `gen_next` 逐项 / for-in / `list()` 转换）、位运算 + 二进制数据视图（int_to_hex / bytes_to_hex / bit_count / bit_length）、正则表达式、锁原语（mutex / rwlock）、文件随机读写 + fsync、进程/信号（os_spawn / os_wait / signal）、**Result/Option 错误处理**（`Ok(x)`/`Err(e)`/`Some(x)` 构造，`?` 错误传播——Err/None 立即返回、`!` 强制解包、is_ok/is_err/unwrap 方法，spec 唯一错误通道）、字符串插值 `${expr}`、推导式、可选链 `?.`、空合并 `??`、管道 `\|>` |
| 🔌 边缘设备 | fd 原语 `open`/`close`/`ioctl`/`os_errno`（ioctl arg 三形态：int 直传 / bytes·str 就地 in/out buffer，`_IOR` 类内核直接填充同对象）+ fd 数据通道 `read`/`write`（read(2)/write(2) 直通）+ **mmap 活映射** `mmap`/`munmap`/`mem_write`（MAP_SHARED 帧缓冲/共享内存/DMA 直访，GC 自动 munmap，`mem_write` 就地写映射区）+ GPIO/I2C 设备示例 + **aarch64 交叉编译**（`pxc build --no-quic` 裁剪 + qemu-aarch64 验证与 x86 一致）——Linux 边缘设备层（树莓派/网关/盒子）单静态二进制免环境 |
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
├── runtime/                # C 运行时（runtime.c/h + aes/xml/zip/ws/rsa/sqlite/route/h2/h3/quic + mbedtls + third_party）
├── stdlib/                 # 标准库（collections.px）
├── examples/               # 80+ 个示例（hello / fib / match / 并发 / 网络 / TLS / SQLite / 推导式 ...）
├── archive/rust-compiler/  # Rust 版编译器源码归档（只读，自举前的实现，git 历史保留）
├── docs/                   # 文档（语言规格 spec / Mini 子集 / 路线图 ROADMAP）
└── .github/workflows/ci.yml # CI：回归 + 自举证明 + 示例编译
```

---

## 文档

| 文档 | 说明 |
|---|---|
| [docs/spec.md](docs/spec.md) | 语言规格说明书（词法 / 语法 / 语义 / 标准库） |
| [docs/MINI_SUBSET.md](docs/MINI_SUBSET.md) | **Mini 子集规范**（自举编译器语言面锁定：支持特性 / 明确排除 / 已知限制） |
| [docs/ROADMAP.md](docs/ROADMAP.md) | 路线图（已完成里程碑 + 远期方向） |
| [CHANGELOG.md](CHANGELOG.md) | 变更日志（按里程碑记录重要变更） |
| [CONTRIBUTING.md](CONTRIBUTING.md) | 贡献指南（构建 / 测试 / 提 PR 规范） |
| [SECURITY.md](SECURITY.md) | 安全漏洞报告策略 |

---

## 里程碑进度

### 功能开发（M0–M40，Rust 时代，全部完成 ✅）

| 阶段 | 内容 |
|---|---|
| M0–M9 | 需求/方案/规格 → 词法+语法 → 解释器 → 并发运行时 → C 代码生成 → 标准库 → AI 工具链（fmt/lint/test/bench/doc/ast）→ LSP/MCP → GC 值对象 → 包管理/模块化 |
| M10–M19 | HTTPS（TLS 1.2/1.3）→ 并发 GC → 文件随机读写+fsync → 锁原语 → sha256/xxhash → 正则 → HTTP 服务端框架 → .px 脚本执行机制 → 定时器 → AES/XML/zip |
| M20–M29 | C 运行时符号统一 → chunked/gzip/切片/base64/SSE → slab 分配器+追踪式 GC+WebSocket+位运算 → 网络/存储/安全收尾 → XML 生成+切片步长+连接池 → 闭包循环回收+进程池+TLS 票据恢复 → `>>>`+WS 心跳+远程 registry → 服务端 TLS+请求体限制+Session → 路由/时间时区/cron/SQLite → JSON 路径+静态缓存头+Range+访问日志+请求 ID |
| M30–M40 | 服务端 https 连接池+字节序整数↔bytes+推导式补全+fmt 配置化 → 沙箱+虚拟主机+限流+HTTP/2 预检+连接线程池 → ws/wss 一行连接+SSE 重连+进程池热更新+生成器 → per-route 限流+TLS SNI+访问日志轮转+QUIC 预研 → 惰性生成器+进程池配置化+WS 广播+事件总线 → gzip 解压+推导式 range+HTTP/2 最小服务端+多维限流 → 日志增强+请求上下文+WS 心跳配置+优雅关闭 → HTTP/2 over TLS+响应压缩+S3 → WS 自动重连+HTTP/2 多流+UDP echo → **Result/Option 唯一错误通道** → **字符串插值 `${expr}`** |

### 自举（M-B1 → M-B9b，全部 ✅）

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
| M-B9b | 第一个生产应用（dogfooding 验证） | ✅ 已迁独立私有仓库维护 |

### 原生开发（M41–M58，自举后 PuXian 自身开发，全部 ✅）

| 里程碑 | 内容 |
|---|---|
| M41 | 类型系统欠账清零：edition / 不可变 / 空安全 / 定义级泛型 |
| M42 | 显式 C 库 import（FFI 平台杠杆）：`import "c/xxx"` + `extern def` |
| M43 | 文件即路由（routegen 构建期生成路由注册，PHP 式框架形态） |
| M44 | 语言糖：简化枚举（type X const）+ 列表追加简写 `<-` |
| M45 | registry 版本化：semver 库 + pxpkg + px.pkg.lock 可复现构建 |
| M46–M52 | HTTP/3 / QUIC 全链路：QUIC 传输 → H3 语义层 → QPACK（Huffman/静态表/动态表/SETTINGS/多路复用/解码器流 ack） |
| M53 | **HTTP/3 三栈合一 WebServer**：px_serve http3（HTTP/1.1+HTTP/2+HTTP/3 共用公共管道）+ Alt-Svc + **aioquic 外部互操作** |
| M54 | **HTTP/3 生产化**：TLS 1.3 会话恢复（1-RTT resumption）+ **0-RTT early data**（握手前可发）+ 连接迁移（换源续传）+ BLOCKED_STREAMS 流控协商（-206/MAX_STREAMS） |
| M57 | **边缘设备层支持（Linux 用户态）**：fd 原语 `open`/`close`/`ioctl`/`os_errno`（ioctl arg 三形态：int 直传 / bytes·str 就地 buffer）+ `read`/`write` 数据通道 + **mmap/munmap 活映射**（MAP_SHARED，GC 自动 munmap）+ GPIO/I2C 示例 + **aarch64 交叉编译 + qemu 验证 + `--no-quic` 裁剪** |
| M58 | **首个 dogfood 真实应用「pxhwmond」**（硬件健康守护 daemon，examples/m58_hwmond）：多文件 import 工程 + M57 fd 通道 /proc 采集（温度条件降级）+ **mmap MAP_SHARED 快照 IPC**（外部 dump 活读 / 命令通道双向）+ 手写 HTTP 状态页（显式响应头）+ run.sh 崩溃自愈 + 阈值告警通知 + aarch64 交叉 qemu 验证 |

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
- `m50_h3_mux_verify.sh` —— HTTP/3 多路复用回环（同一连接 3 双向流并发请求/响应一一对应无串扰，编译/解释双模式）
- `m51_h3_qpack_wire_verify.sh` —— QPACK 会话接入线上回环（双端开控制/编码器/解码器 3 条 QUIC 单向流 + SETTINGS 协商，动态表指令经真实单向流传输：请求 1 插入、请求 2 复用零新增，编译/解释双模式）
- `m52_qpack_decack_verify.sh` —— QPACK 解码器流 ack 线上化回环（RFC 9204 §4.4：接收方解码动态字段段自动发 Section Ack、发送方消费对端解码器流推进 Known Received Count、编码表驱逐安全化，双向闭环编译/解释双模式）
- `m53_s3_pipe_verify.sh` —— HTTP/3 接入公共 HTTP 管道（同进程 px_serve + h3_server_listen 双栈，4 QUIC 连接 × 5 请求与 HTTP/1.1 同管道输出一致）
- `m53_s4_pxserve_h3_verify.sh` —— **HTTP/3 三栈合一端到端**（px_serve http3：HTTP/1.1 TCP + HTTP/3 UDP 同服务；自研 client + **aioquic 第三方互操作** 200；Alt-Svc 自动通告；SIGTERM 优雅关闭）
- `m53_s5_pxi_h3_smoke.px` —— pxi 重建后解释器 h3_server_listen 自检（id>0 PASS）
- `m54_s1_resume_verify.sh` —— TLS 1.3 会话恢复端到端（二次连接 resumed=true，确为 1-RTT）
- `m54_s2_0rtt_verify.sh` —— **0-RTT early data**（connect_0rtt 握手未完成即 send 成功 + H3 静态表 0-RTT GET → 200）
- `m54_s3_migrate_verify.sh` —— **连接迁移**（client 换源端口 → server path 跟随、同 conn 无重握手、echo 跨迁移续传）
- `m54_s4_streams_verify.sh` —— BLOCKED_STREAMS 流控协商（上限 2 → 开流 8 阻塞 -206 → extend +4 放行）
- `m54_s5_pxi_quic_smoke.px` —— pxi 重建后 M54 新内置 12 项自检（解释/编译双模式一致）
- `m57_s1_ioctl_verify.sh` —— 边缘设备层 fd 原语验证（open/close/ioctl/os_errno：TCP fd FIONREAD/FIONBIO + 真实设备条件探测，编译/解释双模式）
- `m57_s2_mmap_verify.sh` —— mmap 活映射验证（MAP_SHARED 双向可见 / offset 子视图 / munmap 解除语义 / GC 自动 munmap 300 轮，编译/解释双模式）
- `m57_s3_verify.sh` —— 设备示例 + 真内核替身（GPIO_GET_CHIPINFO / I2C_SLAVE 示例；loopback ifreq SIOCGIFADDR/FLAGS/HWADDR + PTY TIOCGPTN 真内核 ioctl 硬断言，编译/解释双模式）
- `m57_s4_cross_verify.sh` —— **aarch64 交叉编译 + qemu 验证**（arm64 静态产物 2.5MB 设备层 ioctl 与 x86 结果一致）
- `m57_s5_pxi_smoke.px` —— pxi 重建后 M57 新内置 10 项自检（open/read/ioctl 就地填充/write/mmap 活映射，解释/编译双模式输出一致）
- `m58_hwmond/` —— **M58 dogfood 真实应用：pxhwmond 硬件健康守护 daemon**（多文件 import 工程：main/collect/shm/serve/notify；verify_s1–s4.sh 各子步自检 + 使用/部署见 `m58_hwmond/README.md`）
- ... 完整列表见 `examples/`

---

## 生态与合作

- **仓库外私有生产应用**：PuXian 的第一个真实生产用户（HTTP + SQLite 服务，dogfooding 验证），代码维护于独立私有仓库。
- **问题反馈与贡献**：发现问题请附最小复现用例（单个 .px + 期望/实际输出）提交 issue（label：`M-B9b`）；欢迎提交 PR 参与改进。

---

## 💡 Credits

PuXian 由 **达者同游团队** 开发，并使用 **wsAgent（文殊智能体）** 辅助设计与实现：

| 中文名 | 英文名 | 类型 | 角色 |
|---|---|---|---|
| 本源 | Benyuan | 人类 Human | 创始人 / 总架构师（方向与架构决策、最终验收） |
| 东月 | Dongyue | 智能体 wsAgent | 开发工程师（编码实现） |
| 清歌 | Qingge | 智能体 wsAgent | 设计师 / 质量发现（设计辅助与问题发现） |

> 本语言由 wsAgent（文殊智能体）辅助开发 —— 面向 AI 高效编程的语言，由 AI 参与编写，dogfooding 自证。团队成员明细见 [`AUTHORS.md`](AUTHORS.md)。
