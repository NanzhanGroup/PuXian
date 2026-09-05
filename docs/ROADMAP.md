# PuXian 路线图（ROADMAP）

> 定位：PuXian 主线公开路线图。展示已实现能力与规划方向，保持单一事实源。
> 原则：每个里程碑完成后，更新本文档 + `CHANGELOG.md`，一次 commit 推送。

## 一、能力基线

| 能力 | 状态 |
|---|---|
| 语言自举 | ✅ 编译器由 PuXian 自身编写（三步自举证明：A.c == B.c == B2.c 逐字节一致） |
| 工具链 | ✅ `tools/pxc`（build/run/lex/parse/fmt/lint/doc/test/bench/lsp/mcp）+ `bootstrap/` 自举二进制，零 Rust 依赖 |
| 双后端 | ✅ tree-walking 解释器 + C 转译编译，双模式行为一致（diffcheck 逐字节对拍） |
| 类型系统 | ✅ Result/Option（`?`/`!`）、不可变（E3002）、空安全（E3003）、定义级泛型 |
| 回归体系 | ✅ diffcheck（lexer/parser/errors/codegen/value/interp）+ capability + 自举证明 |
| WebServer | ✅ HTTP/1.1/2/3、HTTPS、WebSocket、SSE、路由/中间件/限流/日志/vhost/SNI/S3 |
| HTTP/3 | ✅ QUIC 传输 → HTTP/3 语义 → QPACK（Huffman/静态表/动态表/SETTINGS/多路复用/解码器流 ack）→ **px_serve 三栈合一（M53）+ aioquic 外部互操作** → **生产化（M54：1-RTT resumption / 0-RTT early data / 连接迁移 / BLOCKED_STREAMS）** |
| 边缘设备层 | ✅ M57：fd 原语（`open`/`close`/`ioctl`/`os_errno`，ioctl arg 三形态 int/bytes 就地 buffer）+ `read`/`write` 数据通道 + **mmap/munmap 活映射**（MAP_SHARED 帧缓冲/共享内存直访，GC 自动 munmap）+ GPIO/I2C 示例 + **aarch64 交叉编译**（`--no-quic` 裁剪 + qemu 验证）——Linux 边缘设备层（树莓派/网关/盒子）单静态二进制 |
| 生态 | 仓库外私有生产应用（dogfood）+ **119 examples 能力导航**（docs/ECOSYSTEM.md）+ **AI 速查包**（PUXIAN_CHEATSHEET，native 281 单一事实源）+ **registry 拉取闭环**（9 官库随库入库，pxpkg fetch→import 双模式） |

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
| M57 | **边缘设备层支持（Linux 用户态）**：fd 原语 `open`/`close`/`ioctl`/`os_errno`（ioctl arg 三形态：int 直传 / bytes·str 就地 in/out buffer，`_IOR` 内核直接填充）+ fd 数据通道 `read`/`write` + **mmap 活映射** `mmap`/`munmap`/`mem_write`（MAP_SHARED，GC 自动 munmap）+ GPIO/I2C 示例 + 真内核替身验证（lo ifreq/PTY）+ **aarch64 交叉编译 + qemu 验证 + `--no-quic` 裁剪** + pxi 重建解释同能力（capability 双模式 253 PASS + diffcheck + 自举证明）；裸机 MCU 明确不做，通用动态 FFI（dlsym）等真需求再上 |

| M58 | **首个 dogfood 真实应用：pxhwmond 硬件健康守护 daemon**（examples/m58_hwmond）：多文件 import 工程（每文件 <500 行）+ M57 fd 通道采 /proc（CPU 差值/内存/负载/uptime/net + 温度条件降级）+ **mmap MAP_SHARED 活映射 IPC**（快照区 + 控制区，外部 dump 活读 / 命令通道双向可见）+ 手写最小 HTTP 状态页（/healthz JSON + / HTML + 404，显式响应头自验 M57-S7）+ run.sh 崩溃自愈 + 阈值告警/webhook dry-run + aarch64 交叉 qemu 验证；dogfood 暴露语言欠账 8 项（http_post 失败即 panic 最优先，MINI_SUBSET §十三） |

| M59 | **数学与随机补齐（游戏/边缘两条线公共地基）**：C libm 内置 14 函数 + 2 常量——`sin`/`cos`/`tan`/`atan2(y,x)`（弧度）+ `floor`/`ceil`/`round`（返回 float，与 sqrt 一致）+ `log`/`log10`/`exp` + `random`/`random_int`/`random_seed`（splitmix64 确定性、跨平台可复现）+ `pi`/`e` 全精度常量；域错误 NaN/inf 透传不终止、参数错误终止（编程契约）；pxi 解释器白名单 +15 双模式同步（含补平 sqrt 不对称）+ `bootstrap/pxi` 重建；编译/解释/qemu-aarch64 三态断言 + splitmix 序列 x86==aarch64（examples/m59_math + verify_s1~s4） |
| M60 | **边缘设备深化（树莓派线，收敛 GAP §三 #1–#5）**：5 个 C 小内置——`sleep_us`/`now_us`（CLOCK_MONOTONIC 微秒）/`fcntl`/`tty_config`（串口 termios raw+波特率）/`fd_wait`（内部 poll 暴露，GPIO 边沿/多 fd 等待）+ 第 4 个 stdlib **`std.edge`**（GPIO V2 line 请求/读写/边沿事件、I2C 寄存器、serial_open、PWM sysfs，纯语言零新 C）+ 示例（m60_serial_pty **x86 实跑 PTY 真内核串口 loopback** / m60_gpio·i2c·pwm 真板段 SKIP 通道）+ GPIO V2 592B 布局 C offsetof 单测；pxi 白名单 +5 双模式同步 + bootstrap/pxi 重建 + aarch64 交叉 qemu 三态一致（examples/m60_dev + examples/m60_*.px） |
| M61 | **外部库 FFI proof（zlib）+ 纯语言 2D 游戏内圈（无真板期，游戏线 0→1 地基）**：A = 外部系统库绑定全链路——zlib 1.3.1 源码自编两版静态 .a 入库（runtime/third_party/zlib/{lib,lib-aarch64}，tools/build_zlib.sh）+ pxc `--zlib-lib` 自动架构探测 + 薄胶水 runtime_zlib.c（`zlib_crc32`/`zlib_compress`[uLongf* 长度指针]/`zlib_uncompress`[inflate 渐进扩容]，纯语言 CRC32 查表互证 + nm 实证符号 + aarch64 qemu 输出 diff 一致）；B = 第 5/6 个 stdlib **`std.gfx`**（Bresenham/中点圆/blit/5x7 字形 text，画布 list[int] 0xRRGGBB）**`std.png`**（纯语言 PNG stored 编码器：CRC32+ADLER+zlib stored block，零 FFI）+ demo（**Mandelbrot 640x480** / 合成场景 / **raw 终端可玩贪吃蛇** dogfood M60 设备组）+ **FFI 压缩 PNG 联动**（zlib_compress 出 IDAT，python 独立解码全验）+ pxi 重建（zlib extern 双模式一致）+ gfx 整数路径 PNG 跨架构 sha256 一致；图片落盘（QQ 富媒体通道暂拒）；SDL2/raylib 真窗口结论留档（无屏不实装） |
| M62 | **语言面欠账修复 L1–L7**（MINI_SUBSET §十三，docs/M62_PLAN.md）：L1 浮点打印 `.0` 对齐 + L5 codegen 块作用域 hoist（if/for/while 内变量提升，B.c 全量同步）+ L6 split 保留空段回归 + L7 pxi bytes 族白名单补齐（+14 双模式同步）+ L2/L3/L4 不改语义留档 |
| M63 | **语言面欠账修复 L8–L11**（docs/M63_PLAN.md）：L8 pxi 网络真实 API 白名单（http/s3 + Result 透传，双模式一致）+ L9 float→str 最短 roundtrip 全精度（fmt_num 定点/科学舒适区）+ L10 编译期浮点字面量全精度（自举重建内嵌）+ L11 bootstrap/pxc --version |
| M64 | **工具链自举恢复**（fmt/lint/test/bench/doc，docs/M64_PLAN.md）：keep-lexer 底座（fmtlexer.px 保留行结构，不碰 pxlexer）→ `pxc fmt`（确定性格式化/幂等）→ `pxc lint`（L001–L008 AST 驱动）→ fmt 全仓收敛（selfhost+tools+stdlib 净 -318 行，B.c==golden 逐字节 + capability 253 PASS）→ `pxc doc/test/bench`（## 注释→Markdown / 顶层 test_xxx 子进程编排 / N×R 计时）+ 收尾欠债清理（CI 质量门、stdlib 收敛、QUIC/H3 内建白名单、TypeConst 收集、L007 noqa） |
| M65 | **LSP / MCP 自举**（spec §12 收官 + §12.1 AI agent 协议，docs/M65_PLAN.md）：jsonrpc_core.px JSON-RPC/Content-Length 底座（粘包/半包自测）+ runtime `os_spawn_capture`（唯一补丁，LSP 诊断与 MCP 工具子进程化共用）→ `pxc lsp`（生命周期/文档同步/publishDiagnostics 子进程 pxcheck 深度诊断 + completion/definition/hover，lsp_core 符号层）→ `pxc mcp`（tools/list 8 工具 + tools/call 崩溃隔离）→ spec §12 8 工具全自举勾选全绿 |
| M66 | **自举 wsAgent runtime 原语补全 + stdlib 收编**（qg-issue 01–06 全量合入，docs/M66_PLAN.md）：L0 11 项 native —— unix_connect（AF_UNIX 裸连接）+ os 五件套（os_exec 进程替换 / os_rename 原子覆盖 / os_remove_all 递归删防删根 / os_random_hex / os_file_sha256）+ os_capture（双管道分离捕获，G6 which 用法覆盖）+ os_popen（双向管道）+ os_kill group 组杀 + write_file/append_file mode + zip_unpack 密码（zipcrypto + WinZip AES-256），pxi 白名单三处同步重建 → L1 收编第 7/8/9 个标准库：std.yaml（YAML 子集）/ std.pxml（PXML 规范进 docs/PXML.md，dogfood 闭环）/ std.lunar（1900-2100 公农历互转，农历生日卖点，ws-todo lunar:M-D 落点）→ spec §8.20 + ROADMAP/README/CI 生态收口 |
| M67 | **多架构一等支持：aarch64 交叉 + GC 架构抽象 + armv7/riscv64**（qg-issue 07 两阶段，docs/M67_PLAN.md）：阶段一 aarch64 一等（README 中英「交叉编译」章节 + CI aarch64 job + examples/m67_aarch64 hello/http/sqlite 三用例 qemu 全绿）→ 阶段二 runtime GC 架构抽象（3 处架构 #if 迁出为 `runtime/arch.h` 统一接口 arch_read_sp/arch_scan_registers/arch_uc_sp + 分架构头 x86_64/aarch64 原样迁出 + 新增 **armv7(armhf)/riscv64** mcontext，GC 主逻辑不再见 #if）+ `pxc` riscv64 自动 -no-pie + zlib 探测三架构 + `cross_multiarch.sh`（--arch 三件套现编，cross_aarch64 薄包装）→ CI **四档矩阵**（x86_64 native 含 gc_stress 并发 GC 压力 + aarch64/armv7/riscv64 qemu 各 hello/http/sqlite）→ 真机侧 qemu-user 并发 GC 限制留档 |
| M68 | **pxi 一致性收官：解释器 native 可达性根治**（docs/M68_PLAN.md）：根因 = 编译产物默认可达 runtime `px_set_global` 全局 native 281，pxi 只认 interp.px 白名单 129 → 差集 155（sqlite/aes/rsa/xml/zip/tcp/udp/ws/sse/cron/session/bus/http_serve/os_pid/now_ms…）pxi 裸脚本 R1001 → 根治 = C 侧 `ffi_call` **双表兜底**（ffi 注册表 → 新增 `px_global_native()` 全局 PX_NATIVE 单源）+ pxi `i_eval_call` env 未命中自动回退 + 未注册返回可辨 Err（typo 仍 R1001）→ **零 extern def 裸脚本 pxi 与编译产物一致**（t_native 19/19、capability 253 双模式逐字节一致、diffcheck --all 全绿、m66 stdlib verify 双模式 PASS）→ spec §9.3 / MINI_SUBSET §十三.0 / README 已知限制收敛 |
| M69 | **生态启动：资产化 + AI 速查 + registry 拉取闭环**（docs/M69_PLAN.md）：S1 生态资产化 —— docs/ECOSYSTEM.md（9 库一览/119 dogfood 能力导航/消费路径）+ tools/gen_ecosystem.px 机器索引 + CI 防漂移；修复 stdlib collections.group_by 历史 bug（{} 字面量 = null + 无 d[k]=v）；S2 AI 速查包 docs/PUXIAN_CHEATSHEET.md + tools/gen_native_table.sh（runtime 注册表 281 全量单一事实源）+ AI 自测 3/3；S3 registry 拉取闭环 —— registry/ 9 官库 0.1.0 随库入库 + pxpkg fetch→import 双模式端到端（examples/m69_registry verify 11 断言）+ spec §8.6.3；S4 写库评估 docs/ECOSYSTEM_GAPS.md（缺口 G1-G4 入档，修复拆 M70 候选 A/B）；S5 全链回归（capability 253 双模式逐字节一致 + diffcheck --all 全绿）+ tag v0.1.0-m69 自动发布 |
| M70 | **语言缺口修复：表达式跨行 + 模块顶层状态**（docs/M70_PLAN.md）：S1 parser 括号上下文换行容忍（新增 skip_expr_ws —— 多行 list/dict/调用参数（含尾部逗号）/元组/索引切片/推导全支持，brace_looks_like_dict 跨行 dict 判定修正；**不动 lexer token 流 → 现有 golden 零漂移**；s15_multiline + cases_bad b11/b12）→ S2 fmt 多行验证收口（fmt_core token 流架构**零代码改动**天然支持 + verify_fmt_multiline.sh 全过）→ S3 模块顶层状态（cg_module 导出非 Const 顶层 VarDecl —— import 合并模块级状态槽 + 主程序顶层 VarDecl 初始化注册 px_set_global，双模式共用 cg_resolve_modules 一处修复；v04_module_state 9 断言双模式 + 自举证明 B.c==A.c）→ S4 全能力重建 bootstrap 链（pxi/pxc/pxpar 带 quic，与 M68/M69 发布物对齐）+ 全链回归（capability 253 双模式逐字节一致 + diffcheck --all/--errors 全绿 + stdlib/m62-m64/m69_registry/m70 verify + fmt/lint 0 错）→ S5 文档收口（spec §4.1 表达式跨行规则 / §5.1 顶层 let·var 全局状态槽语义 / §8.4 import 导出边界 + MINI_SUBSET §四·八·九·十三 + ECOSYSTEM_GAPS G1/G2 标记已修 + CHEATSHEET + README(.en) 里程碑行）→ S6 tag v0.1.0-m70 自动发布 + 产物二次冒烟 + 本机留档 + 发布指引更新 |
> **主线外已占用编号**（非 ROADMAP 功能里程碑，已记录于 CHANGELOG，勿复用）：
> **M55** = issue #2 并发安全 hotfix（修复 · M55）；**M56** = 外部生产应用配套 runtime
> `http_unix` 内建（新增 · M56）。

## 三、远期方向

### 候选主线排期（由 docs/GAP_ANALYSIS.md 驱动，按建议顺序）

> 定位：未来主线的候选池。立项流程：出 `docs/M*_PLAN.md` 规划供审 → 审批后按子步落地
> （verify 回归 + 文档收口，见「六、执行约定」）。当前（M68 已闭环，见上表 + CHANGELOG）
> 候选池仅剩 FFI 外部库绑定验证（游戏窗口线 0→1 前提）与真板物理回归（需硬件）。

| 里程碑 | 主题 | 规模 / 前置 |
|---|---|---|
| 候选 | 真板物理回归（树莓派 + LED / 按键 / 温湿度） | 需硬件；一切边缘能力的最终裁决 |

> FFI 外部库绑定验证候选已由 **M61 闭环**（zlib proof 打通「外部 .a 入库 → C 胶水注册 →
> pxc 链接 → 语言调用 → 跨架构」全链路，立任意外部库模板；见上表 M61 + spec §8.19）。
> 无真板前提下候选池即空 → 下一主线候选：Mahesvara 侧（独立仓）或语言面欠账（MINI_SUBSET
> §十三）按需立项。

> 差距细节见 docs/GAP_ANALYSIS.md；语言面欠账另见 MINI_SUBSET §十三（M58 dogfood 暴露，
> 其中 §十三.1/.2 HTTP/S3 网络失败→Err 已修复）。

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
