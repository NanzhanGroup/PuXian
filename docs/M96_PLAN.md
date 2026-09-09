# M96_PLAN · 客户端网络 IO 协程化（D8-① 阻塞桥不卡 worker）

> 状态：✅ **M96 全段完成（2026-09-10）**：S1 立项定稿 → S2 offload 执行器核心
> （84bff6d+8a8add4）→ S3 名单全集 + 错误回传重构（d3802de）→ S4 收口（本 commit，
> tag v0.2.0-m96）。S4 全量回归绿 + 双自举证明 + bootstrap/pxi_vm 重链吸收 M96
> runtime；顺带修复 M93 协程化潜伏缺陷（并发 println 行交错 → g_print_mu 整行
> 原子锁）。验证矩阵 §六逐项实跑全绿（见 CHANGELOG M96 段）。
> 上游：M93_PLAN §D8 ①（http_request/tcp/udp/ws/s3/dns/fd_wait 同步阻塞桥 → 协程
>   上下文感知 → 不阻塞 worker）+ M94_PLAN §五 编排（M96 = D8-① 客户端网络 IO
>   协程化；前置 M94 抢占 + M95 服务端 handler 协程化已齐）+ M95-S2/S4（handler
>   已是帧协程，handler 内调 http_get/s3 等慢上游会卡 worker —— 本里程碑的真实
>   消费场景）+ M93-S3（阻塞原语让出协议：等待表/唤醒/让出原语 + px_native_
>   blocking_kind 拦截点）+ M88-B/M95（全局根表 + 事件内核先例）。
> 基线：v0.2.0-m95（a6d6056 + 2cc4365 + 872041f，M95 收官于 S4）。性质：**L0 runtime**
>   （执行模型 + 线程层）。
> 关联：M95-S5（px_serve handler 协程化）已从 M95 剥离 **升格 M97**（5e171ac 立项，
>   暂缓，见 M95_S5_PLAN.md）——M96 先行，两者独立。

## 〇、一句话

把「C 层阻塞型客户端网络 native」（http_get/https/s3/ws/tcp/udp/dns/http_unix）
在协程上下文里的执行从「worker 线程内同步阻塞（卡死整 worker，抢占救不了）」
→「**阻塞 native 外包执行器**：投递到专用执行线程池异步跑完，协程登记等待让出，
完成唤醒取结果直行」——worker 永不因阻塞调用卡死，M:N 对 C 阻塞边界成立。

## 一、D0 侦察量化（2026-09-10，dongyue，基线 872041f）

- **客户端网络 native 全部是 C 层阻塞全协议**（证据，runtime.c）：
  - `bi_http_get/post`(9732/9746) → `px_http_request`(9643) → `px_http_once`(9530)
    = getaddrinfo + 阻塞 `connect` + `recv` 循环(9602) + header/chunked/gzip/重定向
    解析全在 C；`px_https_request`(9450) = `mbedtls_net_connect` + `mbedtls_ssl_read`
    阻塞 IO。
  - `s3_*`(8976-9014) → `px_s3_exec` = SigV4 签名 + 连接池 `hconnect/https_connect`
    + `h_exchange`（M24 https 连接池）阻塞。
  - `ws_*` 客户端在 runtime_ws.c（`ws_connect/ws_send/ws_recv`），同构 C 阻塞。
  - `tcp_connect/send/recv`(7795/7819/7832)、`udp_send/recv`(7919/7955)：阻塞 syscall。
  - `dns_lookup`(4334)：`getaddrinfo` 阻塞（远程 DNS 可秒级）。
- **M93-M95 让出覆盖的真实死角**：M93-S3 让出只覆盖 VM 层可控等待（sleep/chan/
  mutex —— `px_native_blocking_kind`(runtime.c 3983) 按函数指针识别）；M94 抢占只
  在 VM 指令边界（绝不在 native C 内部）；M95 handler 协程化后，handler 内调
  http_get/s3 等**慢上游** → C native 内阻塞 → 整 worker 冻结（该 worker 上其它
  协程全停）→ 慢上游并发数 ≥ worker 数即饿死全系统。这是 M:N 化引入的真实缺口。
- **复用点**：vm.c CALL native 预检（473-510：`yield_ok && px_coro_active()` →
  `px_native_blocking_kind` → 让出）即 offload 接线点；M93 让出/等待/唤醒原语
  （`PX_CORO_WAIT_BLOCKED`、等待表、wake_pending）可扩展新等待类别；`px_native`
  对象带名字（runtime.h 68 `as.native.name`）→ 跨文件名单可达（ws 在 runtime_ws.c）；
  `px_error` 用**线程局部**隔离点 longjmp（1908-1913 `g_err_jmp_set/g_err_jmp`）→
  外包线程可装自己的 setjmp 捕获错误消息回传；GC 全局根表先例（M95 http_pend_
  gc_mark/g_hpend_mu + M92 根栈 + M31.4b fserve worker GC 槽注册）。

## 二、路线决策（关键分岔，请你拍板）

| 路线 | 做法 | 量级 | 评价 |
|---|---|---|---|
| **α** | socket 非阻塞化 + epoll 事件登记让出 + **协议状态机化重写**（http 解析/chunked/gzip/重定向 + mbedtls TLS 非阻塞 WANT_READ/WRITE + s3 SigV4 + ws 帧全部分步） | **数个里程碑** | 真协程无线程（Go netpoller 纯正）；但 px 网络栈在 C 层，状态机化 = 重写全部协议 + TLS 非阻塞化，风险/工期不成比例，M96 不可达 |
| **β** ⭐ | **阻塞 native 外包执行线程池**（Go cgo/LockOSThread 范式）：名单命中 → 投递线程池异步执行 → 协程登记等待让出 → 完成唤醒取结果 | **一个里程碑** | C 代码零改动、语义零变化、一刀切覆盖全部阻塞 native；worker 永不卡死；代价 = 阻塞期短暂 OS 线程（池化复用 + 上限 + 空闲回收）。px 网络栈等价 Go 的 cgo 边界 → β 是正确工程选择 |
| 混合 | 单系统调用原语（tcp/udp）α + 全协议 β | 两套机制 | 复杂度翻倍、收益重叠，不取 |

**推荐 β**。D8-① 原文（epoll 登记让出）是 Go 纯网络栈视角的理想形态；px 的现实
是 C 全协议阻塞客户端（cgo 等价物），β 达到同一价值目标（慢上游不卡 worker、并发
wall≈max、协程数与线程解耦），风险低一个量级。α 记入二期候选（协议整体 VM 化后
自然可得，届时才是纯正 epoll 形态）。

## 三、设计定稿（D1-D6，β 路线）

### D1 · offload 语义与让出协议
- vm.c CALL native 预检扩展：`yield_ok && px_coro_active()` 且
  `px_native_offload_kind(fnv)` 命中名单 → 打包 args（abuf 已在堆）→ 投递任务队列
  → 协程登记 offload 等待（新等待类别，复用 M93 等待表/唤醒）→ 让出（return 1）。
- 外包线程执行 `fn(args)` → 结果/错误写任务槽 → 唤醒协程 → `px_vm_resume` 恢复：
  结果写 dst + pc 前进（**不回退重试**——native 已在别的线程执行完，语义 = M93
  sleep 预写 dst 让出模式的推广）。主线程/嵌套 native 回调/逃生舱（yield_ok=0）→
  直调 px_call，语义零变化。

### D2 · 名单（名字匹配 `as.native.name`，跨文件安全）
- http_get / http_post / http_get_stream；s3_put / s3_get / s3_delete / s3_list；
  ws_connect / ws_connect_auto / ws_send / ws_recv（runtime_ws.c）；tcp_connect /
  tcp_send / tcp_recv；udp_send / udp_recv；dns_lookup；http_unix 客户端类。
- 排除：sleep/chan/mutex/rwlock（已让出）；fd_wait（用户显式短等，自控超时）。
- 二期候选：文件 IO / popen 子进程等待等其余阻塞 native。

### D3 · 执行线程池
- 按需创建（首个 offload 即起），空闲回收（闲置超时退），上限 PX_OFFLOAD_MAX
  默认 max(8, CPU×2)。阻塞并发数不受 worker 数限制（语义对齐 pthread 时代每
  spawn 一线程的阻塞自由度），线程数上界 = worker + 上限（对比 pthread 时代 =
  spawn 数），空闲回落。

### D4 · GC 根面
- offload 任务（args + 结果槽 + fn）注册全局根表（复用 M95 http_pend 模式：
  独立锁 + GC 标记期补标）；执行线程执行期注册 GC 槽（fserve worker 先例）。
- S2 查证 M11 全线程栈保守扫描是否已覆盖外包线程栈上 args 引用，决定是否需要
  显式登记。

### D5 · 错误回传
- 外包线程装线程局部 setjmp 隔离点（对齐 spawn_thread/coro worker M93 隔离语义）
  → px_error longjmp 捕获 → 记错误消息 + 置错误态 → 唤醒协程 → 协程恢复后
  px_error(msg) 重抛（保 spawn/coro 隔离：宿主继续、主线程 exit，语义与直调一致）。

### D6 · 边界（语义零变化声明）
- 逃生舱 --c（fn_*）/主线程/嵌套回调：直调不 offload。
- fd_wait/sleep/chan/mutex：已让出，不入名单。
- px_serve 内部线程阻塞（accept/事件循环）不在 VM native 名单，不涉及。

## 四、S 级拆分（每 S = commit + 编译 + 验证绿，不混）

- **S1** 立项（本文档）+ 路线拍板 + M95 收官标注。→ 本文档 commit
- **S2** offload 执行器核心：任务结构 + 队列 + 线程池（创建/回收/上限）+
  协程等待登记与唤醒恢复（vm.c 预检接线，先挂 http_get + tcp_recv 两试点）+
  GC 根 + 错误回传。
  **验证**：协程并发 N×慢 http_get（本地慢服务）→ worker(=2) 不卡、其它计算协程
  存活、wall≈max；线程数收敛（空闲回收）；逃生舱 --c 直调语义不变；offload 与
  直调对拍逐字节一致。
- **S3** 名单全集 + 压力 + 对拍：http_post/get_stream/s3_*/ws_*/tcp/udp/dns/
  http_unix 全纳入；并发慢上游压力；与直调对拍一致；万级协程低阈值 GC（offload
  根面实证）。
- **S4** 收口：全量回归（vm_ab + m89_s3d + m93_s2/s3 + m94_s2/s3 + m95_s2/s4 +
  m82 + m83_s6 + diffcheck + 双自举）+ pxi_vm 重链 + 文档 + tag v0.2.0-m96。

## 五、风险与预案

- 外包线程 px_error/全局并发写 → setjmp 隔离点 + 名单约束（纯网络 native，不回调
  VM，无嵌套 px_call）。
- GC 根遗漏（任务/结果被回收）→ 全局根表 + 执行期线程 GC 槽 + ASAN + 低阈值 GC
  压力。
- 线程数膨胀 → 池上限 + 空闲回收 + 收敛验证。
- offload 结果 LXValue 含对象 → 结果槽根登记，唤醒后协程消费前不回收。
- native 非线程安全假设（若有）→ S2 试点用 ASAN + 并发对拍暴露。

## 六、验证矩阵（预填，S2-S4 逐项实跑）

| 套件 | 断言 |
|---|---|
| m96_s2 | 并发慢 http_get 不卡 worker；计算协程存活；wall≈max；逃生舱直调不变；对拍一致 |
| m96_s3 | 名单全集对拍；并发压力；万级协程 + 低阈值 GC 零崩；线程收敛 |
| 回归 | vm_ab 38P/0GAP/0F + m89_s3d 9 + m93_s2/s3 12 + m94_s2/s3 8 + m95_s2/s4 + m82/m83_s6 + diffcheck + 双自举 |
