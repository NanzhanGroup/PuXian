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
| WebServer | ✅ HTTP/1.1/2/3、HTTPS、WebSocket、SSE、路由/中间件/限流/日志/vhost/SNI/S3 |
| HTTP/3 | ✅ QUIC 传输 → HTTP/3 语义 → QPACK（Huffman/静态表/动态表/SETTINGS/多路复用/解码器流 ack）→ **px_serve 三栈合一（M53）+ aioquic 外部互操作** → **生产化（M54：1-RTT resumption / 0-RTT early data / 连接迁移 / BLOCKED_STREAMS）** |
| 生态 | 仓库外私有生产应用（dogfood）、80+ examples、registry 版本化（semver + lockfile） |

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
| M53 | HTTP/3 三栈合一 WebServer：`px_serve` opts.http3（HTTP/1.1+HTTP/2+HTTP/3 共用公共 HTTP 管道）+ Alt-Svc 通告 + **aioquic 外部互操作打通** + pxi 重建解释同能力 |
| M54 | **HTTP/3 生产化**：TLS 1.3 会话恢复（1-RTT resumption）+ **0-RTT early data**（含收包路由 DCID 修复、H3 静态表子集）+ 连接迁移（client 换源 + server path 跟随 + PATH_CHALLENGE）+ BLOCKED_STREAMS 流上限协商（-206 阻塞 / MAX_STREAMS 放行）；语言 API 12 项；S5 全量回归（pxi 重建 + capability 双模式 + diffcheck + 自举 + 14 项端到端） |

> **主线外已占用编号**（非 ROADMAP 功能里程碑，已记录于 CHANGELOG，勿复用）：
> **M55** = issue #2 并发安全 hotfix（修复 · M55）；**M56** = 外部生产应用配套 runtime
> `http_unix` 内建（新增 · M56）。主线后续功能里程碑从 **M57** 起排。

## 三、远期方向

### M57 · 边缘设备层支持（Linux 用户态）

> 与清歌嵌入式讨论的落地结论：PuXian（runtime 含 GC/线程/动态值，编译模式=生成 C→gcc）
> 在嵌入式方向只能到 **Linux 边缘设备层**（树莓派 / 网关 / 盒子）；裸机 MCU（STM32/ESP32）
> 无 OS + 架构不符，明确不做。相比给暂无真实用户的 H3 栈做打磨（1→1.01），设备层是
> **0→1 开新使用域**（物理世界接口：点灯 / 读传感器 / 控 I2C/SPI/GPIO/tty/网卡/帧缓冲），
> 首刀成本低（ioctl 胶水约 100–200 行 C，走 M42 FFI 桥），复用 fd / bytes(ptr,len) /
> 生成 C→gcc 交叉等既有资产；静态二进制免环境是边缘差异化卖点。
>
> 落地顺序（对齐清歌建议：先通用设备绑定，通用动态 FFI 等真需求再上）：

- S1 ioctl 胶水内建：`ioctl(fd, request, arg)`（arg 支持 int / bytes → 通吃
  i2c-dev / spi-dev / gpio(老 ioctl) / tty / 网卡；open/read/write 已被文件 IO 覆盖）
- S2 mmap/munmap 语言内建：映射 fd（帧缓冲 / 共享内存 / DMA）→ bytes 视图
- S3 GPIO / I2C 设备示例 + x86 ioctl mock 验证（无真板子，先用 mock 验胶水语义）
- S4 交叉编译工具链：本地 aarch64 交叉编译 → qemu-aarch64 跑静态产物验证
  + runtime 裁剪开关（--no-ssl 等，解开 mbedtls/sqlite3/openssl 平台依赖）
- S5 pxi 重建 + capability / diffcheck / 自举全绿
- S6 文档（spec §8.x + ROADMAP + CHANGELOG）
- 明确不做：裸机 MCU（架构不符）；通用动态 FFI / dlsym（费劲，等「任意 C 库即插即用」
  需求再上）

### 搁置 · HTTP/3 / QUIC 健壮性加固（可靠性生产化）

> 曾列为 M57 候选主线；评估后降级：H3 目前 **无真实用户**（自签证书下 Chrome/Firefox 直接
> 退回 HTTP/1.1，不走本栈），给无人使用的栈做 fuzz / 并发审计价值前提不成立（1→1.01 而非
> 0→1）。待 H3 出现真实用户（如生产应用配真证书公网/局域网浏览器实测）再捞回。

- 协议 fuzz：QUIC / H3 / QPACK 解析器模糊测试（畸形包 / 超大帧 / 恶意 SETTINGS），零崩溃
- 并发 / 内存安全审计：race + ASAN 全量回归；fd / 缓冲 / 会话生命周期核查
  （M55 issue#2 一类问题的系统性修复）
- 资源边界：慢客户端 / 半开连接 / 流与连接上限的拒绝与回收；OOM 与超长响应边界
- 互操作健壮性扩展（可选并入验证）：aioquic 畸形 / 边界场景回归

### 搁置 · HTTP/3 深度生产化剩余项（RFC 完备性洁癖，价值低）

> 评估：三项均无真实用户场景，不建议主线排期；待出现明确需求再捞回。

- 0-RTT + QPACK 动态表前缀（RFC 9204 深度语义）—— Chrome 2021 起禁用 QPACK 动态表
  （容量=0），业界普遍跟随；收益被队头阻塞与复杂度抵消，前缀语义 ≈ 纸面
- 服务端主动迁移 / immediate migration —— 现实服务端换址直接断连重开，无迁移等待场景；
  immediate migration 连 ngtcp2 上游也未实现（带断言），属「等别人」
- 0-RTT / 迁移 / 流控的深度第三方互操作扩展 —— 边际收益递减（M53/M54 已各验一次真实互操作）

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
