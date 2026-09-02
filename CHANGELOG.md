# Changelog

本项目的所有重要变更都会记录在此文件。
格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，
版本语义遵循 [Semantic Versioning](https://semver.org/lang/zh-CN/)。

## [Unreleased]

## [0.1.0] - 2026

首个可公开版本：语言已自举（PuXian 编译器由 PuXian 自身编写），
工具链零 Rust 依赖（`tools/pxc` + 仓库自带自举二进制），静态二进制分发。

### 语言与工具链（自举核心）

- 语言：缩进语法（Python 脸）、渐进类型、`struct/enum/trait/impl`、
  `Option`/`Result` + `?`/`!`、`match`、推导式、生成器、闭包（无捕获面）、
  字符串插值 `${expr}`、管道 `|>`、可选链 `?.`、空合并 `??`
- 双后端：tree-walking 解释器（秒起）+ C 转译编译（`gcc` → 静态二进制），
  双模式行为一致（`selfhost/diffcheck.sh` 逐字节对拍）
- 自举闭环：`compiler.px` 编译自己 → `B.c == golden/compiler.c` 逐字节一致（三步证明）
- 工具：`tools/pxc`（build/run/lex/parse）、`tools/pxpkg`（包管理 + semver + lockfile）、
  `tools/routegen`（文件即路由生成器）
- Mini 子集规范（`docs/MINI_SUBSET.md`）：自举期间语言面锁定 + 已知限制清单

### 运行时与网络

- 内置：AES/XML/zip/gzip/正则/base64/JSON（含 JSONPath）/SQLite/协程/事件总线/沙箱
- WebServer 生产化：HTTP/1.1 服务端 + 路由/中间件/限流/访问日志/虚拟主机（SNI）/
  静态文件/Range/ETag/Session/Cookie/优雅关闭/进程池热更新
- HTTP/2：h2c + HPACK/Huffman + 多流 + over-TLS（ALPN h2）
- WebSocket / SSE（含自动重连、心跳、广播）、UDP、S3/MinIO（SigV4）
- HTTP/3 / QUIC（RFC 9114/9000/9204 预研落地）：
  - M46 QUIC 传输层（ngtcp2 + quictls 静态编译，握手/双向流/单向流）
  - M47 HTTP/3 语义层（QPACK 无动态表 codec + HEADERS/DATA 帧 + 请求/响应对拍）
  - M48 QPACK 完整 codec（RFC Huffman + 静态表 99 项字节精确）
  - M49 QPACK 动态表 + SETTINGS 控制帧（RFC 9204 全量会话）
  - M50 多路复用（单连接多双向流并发请求/响应，per-stream 缓冲）
  - M51 QPACK 会话上真实 QUIC 单向流（控制/编码器/解码器三流 + SETTINGS 协商）
  - M52 QPACK 解码器流 ack 闭环（Section Ack / KRC 推进 / 编码表驱逐安全化）

### 生态

- ws-web：用 PuXian 写的第一个生产应用（HTTP + SQLite），dogfooding 验证
- 80+ 示例程序 + 里程碑验证脚本（`examples/`）

[Unreleased]: https://github.com/NanzhanGroup/PuXian
[0.1.0]: https://github.com/NanzhanGroup/PuXian
