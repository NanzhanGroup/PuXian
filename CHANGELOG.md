# Changelog

本项目的所有重要变更都会记录在此文件。
格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，
版本语义遵循 [Semantic Versioning](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### 路线图 · M57 内容重定向（HTTP/3 深度生产化 → 健壮性加固）

- 原「M57+ · HTTP/3 深度生产化」三剩余项（QPACK 动态表前缀 / 服务端主动迁移·immediate
  migration / 深度互操作扩展）**降级为搁置**：Chrome 2021 起禁用 QPACK 动态表、immediate
  migration 连 ngtcp2 上游都未实现、深度互操作边际收益递减 —— 均无真实用户场景（详见
  `docs/ROADMAP.md` §三「搁置」评估注记）
- **M57 重定向为主线下一里程碑「HTTP/3 / QUIC 健壮性加固（可靠性生产化）」**：协议 fuzz
  （畸形包/超大帧/恶意 SETTINGS）+ 并发/内存安全审计（race+ASAN，收口 M55 issue#2 一类）
  + 资源边界（慢客户端/半开连接/流与连接上限/OOM）+ 互操作边界回归 —— 把 H3 栈从
  「能跑」推到「扛造」

### 新增 · M56 runtime http_unix 内建（ws-web 配套，非主线 HTTP/3 里程碑）

- `http_unix(sock_path, url_path, method[, body[, headers]]) -> dict{status, headers, body}`
  —— Unix domain socket HTTP 客户端内建（本地服务 / LLM 网关 / 容器 daemon 调用）：
  每次新建 AF_UNIX 连接、`Connection: close` 用完即关；收发超时 180s（本地长响应）；
  带 body 且未显式带 Content-Length 时自动补（Content-Type 可被 headers 覆盖）；
  响应解析复用 h_exchange（与 http_get/http_post 同解析器）
- 引入背景：ws-web LLM 接入 ws 词元缓存网关（unix socket 通道，key 零落盘）；
  由清歌提交（runtime/runtime.c +73；ws-web 侧 llm.px / translate.px / main.px），
  编号与主线 HTTP/3 里程碑并行插队 —— 本条目为 PuXian runtime 侧文档补齐

### 新增 · M54 HTTP/3 生产化（见 docs/M54_PLAN.md）

- **TLS 1.3 会话恢复（1-RTT resumption）**：server SSL_CTX 开启 stateless session ticket；
  语言 API `quic_connect_resume(ip,port,alpn,session)` / `quic_session_save(conn)` /
  `quic_conn_resumed(conn)` —— 二次连接确为 1-RTT 恢复（resumed=true）
- **0-RTT early data**：server `SSL_CTX_set_max_early_data` + 收包路由登记客户端 Initial
  DCID（修复 0-RTT 长头包被丢）；语言 API `quic_0rtt_save` / `quic_connect_0rtt`（握手完成
  前即可 `quic_send`，tp 缺失自动降级 1-RTT）/ `quic_0rtt_rejected` /
  `quic_conn_handshake_done`；H3 0-RTT 子集 `h3_server_listen_stateless`（静态表，幂等 GET）
- **连接迁移**：`quic_migrate(conn, ip, port)` 新建 UDP fd 换源（NAT rebinding 语义）；
  `quic_conn_path` / `quic_conn_local` 观测对端/本地地址；server 完成新路径
  PATH_CHALLENGE 验证并跟随（同 conn 无重握手，echo 跨迁移续传）
- **BLOCKED_STREAMS 流上限协商**：`quic_set_max_client_streams(listener, n)` 经 transport
  params 下发对端；`quic_extend_max_streams(listener, add)` 发 MAX_STREAMS 放行；
  `quic_streams_left(conn)` 剩余配额；`quic_open_stream` 达配额返回 -206（原吞为 -1）
- bootstrap/pxi 重建：解释器同能力支持 M54 全部新内置（S5 全量回归 14 项端到端全 PASS）

### 新增 · M53 HTTP/3 三栈合一 WebServer（见 docs/M53_PLAN.md）

- `px_serve(port, docroot, timeout, {http3: true | {port?, cert?, key?}})` 单调用同时托管
  HTTP/1.1（TCP）+ HTTP/3（QUIC/UDP，缺省同端口），HTTP/1.1+HTTP/2+HTTP/3 共用同一
  vhost/路由/限流/访问日志/静态/.px 管道；Alt-Svc 自动通告；SIGTERM 优雅关闭
- 语言 API：`h3_server_listen(port, cert, key)`（HTTP/3 管道托管 listener）、
  `quic_h3_listen(port, cert, key)`（QUIC raw listener，多连接托管地基）
- QUIC 服务端多连接托管：单 fd 收包路由（DCID → 连接入包队列）+ 每连接处理线程 + PEM 证书加载
- 请求管道与传输解耦：`PxHttpOut` 输出抽象（HTTP/1.1 与 HTTP/3 共用，纯重构响应字节零变化）
- **HTTP/3 外部互操作打通**：aioquic（第三方独立实现）请求 PuXian H3 服务 200；修复 4 个
  协议级缺陷（fin-only 流活性、无 body 请求、响应头字段名小写、响应流 FIN）
- bootstrap/pxi 重建：解释器同能力支持 h3_server_listen / px_serve http3（S5 全量回归）

### 修复 · M55 并发安全（issue #2）

- 全局符号表/GC root 扫描互斥：px_set_global / px_get_global / struct 方法查找全程持锁、
  g_len 原子化、stop-the-world 持锁扫 root —— 修复并发写全局变量 + GC 竞态崩溃

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
