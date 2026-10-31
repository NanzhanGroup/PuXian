# M96_PLAN · px_serve（route/vhost handler）协程化 —— D8-② 二期收官

> 状态：🟡 **S1 立项+D0 侦察（进行中）**。基线 v0.2.0-m95 + M95-S4（872041f）。
> 上游：M95_PLAN §五 S5 编排（px_serve handler 协程化）；M95-S2（http_serve handler
> 协程化，同构蓝本）；M95-S4（sse_serve handler 协程化，同构蓝本）；M88-B（fserve/
> 事件化连接模型）；M93/M94（帧协程内核 + 抢占）。
> 性质：**L0 runtime**（px_serve 连接并发模型层）。

## 〇、一句话

px_serve（M31.4b g_pool + px_conn_worker 整连接处理）的 **route/vhost handler
（VM px 函数，含 runtime_route 的 middleware + handler）** 执行从「每连接一个
g_pool 常驻线程内同步 px_call（长业务占线程）」→「handler 帧协程（让出占协程不占
线程）」。handler 完成后响应写经 PxHttpOut 续处理，连接 keep-alive 继续。

## 一、D0 侦察量化（dongyue，基线 872041f）

- **px_serve 并发模型（与 http_serve 本质不同）**：http_serve/sse_serve 走 fserve
  job 池 + M88-B 事件化（IDLE 不占线程）；**px_serve 独立 g_pool（M31.4b，
  PX_POOL_MAX=256，默认 max_conn=32）**：`bi_px_serve` accept 循环 → `px_pool_push(fd)`
  → 常驻 `px_pool_worker`（每连接**独占一个线程**取 fd → `px_conn_worker` 阻塞
  keep-alive 循环直到连接关闭/超时；SO_RCVTIMEO 15s 阻塞读、**无 M88-B 事件化
  IDLE 交还**）。→ 现状：空闲 keep-alive 与长业务 handler **都占 g_pool 线程**。
- **px_conn_worker**（runtime.c ~14117-14520，约 400 行）：整连接 keep-alive 循环。
  每次迭代：读请求头（阻塞 px_conn_read SO_RCVTIMEO 15s）→ 解析 method/path/query/
  headers/body（含 chunked/落盘）→ 构造 req dict → h2c 升级/prior knowledge 检查
  （px_h2_handle 整连接接管）→ **px_http_dispatch（公共管道）** → 清理 → 下一请求。
  `PxConn conn` 为栈对象（TLS 会话），`PxHttpOut out` 绑定 &conn。
- **handler 调用点（全部在 px_http_dispatch 管道内，与 http_serve 的单一 __http_handler
  不同）**：
  1. **vhost handler**（runtime.c ~13732）：`px_call(vhandler, &req, 1)` —— vhost()
     注册的 VM/原生 handler，返回 dict → px_vhost_normalize + pout->respond + 日志。
  2. **route handler + middleware 链**（runtime_route.c px_route_try_dispatch，
     runtime.c ~13765 调用）：middleware `px_call(mws[i],&req,1)`（非 null 短路响应）；
     命中 handler `px_call(handler, [req,params], 2)` → route_normalize + route_send +
     访问日志。**本调用点在 runtime_route.c 独立编译单元（PX_NO_ROUTE 裁剪）。**
  3. .px 文件执行（px_pool_run = fork `px --worker` 子进程池，**C 阻塞等待子进程**，
     不占 VM 协程可让出范围 → 不在本里程碑 handler 协程化面）。
  4. 静态文件/CORS/限流/日志：无 VM handler，纯 C。
- **与 http_conn_worker（M95-S2 已协程化）的差异**：http_serve 每连接由 fserve job
  池 + IDLE 事件化承载，handler 调用点单一、`http_pend` 拆段已落地；px_serve 是
  **整连接阻塞循环 + 多 handler 调用点深埋管道 + PxHttpOut 输出抽象 + 独立 g_pool
  线程模型** → 不能直接平移 http_pend，需先给 px_serve 补齐「连接级拆段 + handler
  调用点异步化 + 连接续处理投回 g_pool」。
- **收益场景**：route/vhost handler 内 chan/sleep 长业务（如内部接口聚合、等待下游
  chan 超时重试）——现状占 g_pool 常驻线程（32 上限即并发连接上限，长业务连接把
  池占满 → 后续连接 accept 排队）；改造后 handler 让出占协程，g_pool 线程释放去
  accept 新连接；handler 纯计算短请求走 M94 抢占，无饿死。
- **GC 面**：px_conn_worker 请求迭代已有 M92-S2c precise 登记（px_root_push/PX_KEEP
  req/resp/vhost handler 返回值）；handler 协程运行期 req/params 由协程 args 保活
  （px_coro_gc_mark_roots）→ pending 表若需存 req/params 须补 GC mark（对齐 http_pend）。
- **逃生舱**：PX_NATIVE/原生 handler → 同步直调（行为零变化）；.px 子进程池原样。
- **g_cur_conn**：px_conn_worker 设 g_cur_conn=&conn 仅赋值（D0：全库无读取消费者），
  handler 协程化无需 TLS 上下文迁移（同 M95-S4 结论）。

## 二、范围决策（重要）

px_serve 整连接 keep-alive 循环若要 handler 让出后**线程释放去 accept 新连接**，
必须把连接处理从「每连接独占线程」升级为「连接挂起 + handler 协程 + 完成后投回
续处理」。这与 http_serve 的 fserve job 化不同（px_serve 保留独立 g_pool），因此
**连接级续处理投回 g_pool 队列**（px_pool_push(fd)）即可，无需迁到 fserve。
范围分两条落地路径：

- **S2（本里程碑核心）**：px_conn_worker 循环内 px_http_dispatch 调用前**先 route/
  vhost 匹配**（预分派），命中 VM handler → 拆段：
  读+解析+req（段1，g_pool worker）→ px_pend_put(fd, req, 命中类型, ctx…) →
  px_coro_spawn_ex(handler.ctx, [req,params], done_cb, fd) → worker 返回（线程释放）；
  handler 完成回调（coro worker）→ resp 写 px_pend（stage=2）+ px_pool_push(fd) 投回；
  g_pool worker 重入 px_conn_worker 循环顶 px_pend_take(stage==2) → 用 resp 走
  **route/vhost normalize + pout->respond + 访问日志**（段2）→ keep-alive 下一请求。
  未命中 route/vhost（静态/.px/CORS/限流/日志）→ 原同步路径零变化。
- **S3（收口）**：m95_s6 验证套件 + px_serve 全量回归（m27a/m27b/m28_route/m29_webprod/
  m31_vhost/m33_route_rate_limit/m53_s4_pxserve_h3/m57_s7_vhost_headers 等）+ 文档 + commit。
- ⚠️ 二期（另立）：px_serve 连接级事件化 IDLE（keep-alive 空闲不占 g_pool 线程）、
  .px 子进程池协程化、fserve/pool 默认 worker 数随协程化下调策略。

## 三、风险与预案
| 风险 | 预案 |
|---|---|
| px_serve 核心路径回归（route/vhost/静态/.px/H3 共用 px_http_dispatch） | 未命中 handler 的路径零变化；命中 VM handler 才拆段；全量回归 m27/m28/m29/m31/m33/m53_s4/m57_s7；逃生舱原生 handler 同步 |
| runtime_route.c 独立单元 + PX_NO_ROUTE 裁剪 | handler 匹配/拆段封装在 runtime.c（weak px_route_has / 提供预分派辅助），runtime_route.c 只加"把已匹配 handler 执行拆出"的导出函数（弱化裁剪边界见 R1001 语义） |
| req/params/resp 跨线程（handler 协程期 + done 投回） | px_pend 表持 req/params/resp 为 GC 根 + gc 标记期补标（对齐 http_pend）；PX_KEEP 窗口保护 |
| handler 挂起期连接 fd 无人读/超时 | 与原线程模型 handler 占用期一致（挂起=连接挂起）；完成后投回续处理；fd 复用由 px_pool_push 投回前检查 pending 项（连接已关 → 丢弃） |
| 限流/访问日志/vhost normalize 顺序语义漂移 | 段2 复用既有 normalize/respond/日志代码（抽公共函数），仅调用点异步化；逐字节对拍 m31_vhost/m33 |

## 四、S 拆分
| S | 内容 | 验证 |
|---|---|---|
| S1 | 立项 + D0 + 设计定稿 | 本文档 |
| S2 | route/vhost VM handler 协程化（预分派 + px_pend + spawn + done 投回 + 段2 续处理） | m95_s6 套件：route handler /sleep /chan 长业务并发 + fserve/g_pool 少量 + 线程收敛 + vhost handler + precise GC + keep-alive/HEAD/日志回归 |
| S3 | px_serve 全量回归（route/vhost/静态/.px/H3）+ 文档 + commit | 全绿 |
