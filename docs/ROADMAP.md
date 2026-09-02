# PuXian 路线图（ROADMAP）

> 定位：PuXian 主线公开路线图。展示已实现能力与规划方向，保持单一事实源。
> 原则：每个里程碑完成后，更新本文档 + `CHANGELOG.md`，一次 commit 推送。

## 一、能力基线

| 能力 | 状态 |
|---|---|
| 语言自举 | ✅ 编译器由 PuXian 自身编写（三步自举证明：A.c == B.c == B2.c 逐字节一致） |
| 工具链 | ✅ `tools/pxc`（build/run/lex/parse）+ `bootstrap/` 自举二进制，零 Rust 依赖 |
| 双后端 | ✅ tree-walking 解释器 + C 转译编译，双模式行为一致（diffcheck 逐字节对拍） |
| 类型系统 | ✅ Result/Option（`?`/`!`）、不可变（E3002）、空安全（E3003）、定义级泛型 |
| 回归体系 | ✅ diffcheck（lexer/parser/errors/codegen/value/interp）+ capability + 自举证明 |
| WebServer | ✅ HTTP/1.1/2、HTTPS、WebSocket、SSE、路由/中间件/限流/日志/vhost/SNI/S3 |
| HTTP/3 | ✅ QUIC 传输 → HTTP/3 语义 → QPACK（Huffman/静态表/动态表/SETTINGS/多路复用/解码器流 ack） |
| 生态 | ws-web（dogfood 生产应用）、80+ examples、registry 版本化（semver + lockfile） |

## 二、已完成主线（里程碑记录，详见 CHANGELOG.md）

| 里程碑 | 主题 |
|---|---|
| M0–M40 | 语言内核 / 标准库 / AI 工具链 / LSP/MCP / GC / 加密 / 网络 / WebServer 生产化 / HTTP2 / Result 错误处理 / 字符串插值（Rust 时代，已退役） |
| M-B1→M-B9 | 自举：能力门禁 + Mini 子集 + 对拍框架 → lexer/parser/value/codegen/interp 逐组件 PuXian 重写 → 自举证明 → Rust 退役 → CI |
| M41 | 类型系统欠账清零：edition / 不可变 / 空安全 / 泛型 |
| M42 | 显式 C 库 import（FFI 平台杠杆）：`import "c/xxx"` + `extern def`，双模式统一走 C 桥 |
| M43 | 文件即路由（PHP 式框架形态）：routegen 构建期生成路由注册 |
| M44 | 语言糖：简化枚举（`type X const (...)`）+ 列表追加简写（`<-`） |
| M45 | registry 版本化：semver 库 + `tools/pxpkg` + px.pkg.lock 可复现构建 |
| M46 | HTTP/3 应用验证：QUIC 传输级 API（ngtcp2 + quictls 静态编译，回环端到端） |
| M47 | HTTP/3 语义层：QPACK（无动态表 codec）+ HEADERS/DATA 帧 + 请求/响应对拍 |
| M48 | QPACK 完整 codec：RFC Huffman + 静态表 99 项索引压缩 |
| M49 | QPACK 动态表 + SETTINGS 控制帧（RFC 9204/9114 连接级完整化） |
| M50 | HTTP/3 多路复用：QUIC 多双向流并发请求/响应（per-stream 缓冲） |
| M51 | QPACK 会话接入线上：控制/编码器/解码器三单向流 + SETTINGS 协商 |
| M52 | QPACK 解码器流 ack 线上化（RFC 9204 §4.4 闭环 + 编码表驱逐安全化） |

## 三、远期方向

### M53+ · HTTP/3 完整化

- 0-RTT / 连接迁移（QUIC 生产化）
- BLOCKED_STREAMS 互操作
- 接入现有 HTTP 路由 / 中间件 / 日志管道

### 平台 / 生态（按需排期）

- WASM 多后端：解释器 + C 转译双后端已就绪，加 WASM 后端让 .px 跑浏览器——暂不排期，等生态需求
- 更多 C 库绑定（`import "c/xxx"` 生态扩展，SDL2/raylib 游戏线等）
- 明确不做：3D（OpenGL/Vulkan，绑定工程量大）；异常 try/throw（Result + `?` 是唯一错误通道）

## 四、语言面已知欠账（写编译器/生态代码需规避）

见 `docs/MINI_SUBSET.md` §三「明确排除」与 §八~§十二「已知限制」（自举编译器语言面锁定，持续更新）。

## 五、验证体系（每个里程碑通用）

| 验证 | 命令/方式 | 门槛 |
|---|---|---|
| 双模式一致 | `./selfhost/diffcheck.sh --all` | 全绿（含新增用例） |
| 错误场景 | `./selfhost/diffcheck.sh --errors` | 全绿 |
| 能力自检 | `pxi selfhost/capability.px` + 编译模式 | 双模式 PASS 一致 |
| 自举不回归 | bootstrap/pxc 编译 compiler.px → B.c == golden/compiler.c | 逐字节一致 |
| 端到端示例 | examples/ 新增用例编译运行 | 输出符合预期 |

## 六、执行约定

- 多步骤里程碑用 task_checkpoint 记录子步进度（网关重启后可续）。
- 完成一个里程碑：更新本文档状态 + CHANGELOG.md + README.md（示例/能力表）+ spec.md（如需）+ 一次 commit 推送。
