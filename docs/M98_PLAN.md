# M98_PLAN · px_serve（route/vhost handler）协程化 —— D8-② 二期收官

> 状态：🟡 **S1 立项 + D0 复核（进行中）**。基线 v0.2.0-m97（f4b805f，M97 收口后）。
> 承接：**M95_S5_PLAN.md**（2026-09-09 5e171ac 立项，原 M95-S5）—— M97 期间
> qg-issue 31/32 插入使编号顺延，本文件为 M98 正式计划（M95_S5_PLAN.md 保持有效
> 作历史快照，本文档以其为基础更新基线/行号/实现决策）。
> 上游：M95_PLAN §五 S5 编排；M95-S2（http_serve handler 协程化，http_pend 蓝本）；
> M95-S4（sse_serve handler 协程化，PxConn 堆化注册蓝本）；M88-B（fserve 事件化）；
> M93/M94（帧协程内核 + 抢占）；M96（offload 让出）；M97（px_send_all 全量写）。
> 性质：**L0 runtime**（px_serve 连接并发模型层）。

## 〇、一句话

px_serve（M31.4b g_pool + px_conn_worker 整连接 keep-alive 循环）的 **route/vhost
VM handler（含 runtime_route 的 middleware 链后置 handler）** 执行从「每连接一个
g_pool 常驻线程内同步 px_call（长业务占线程）」→「handler 帧协程（让出占协程不占
线程）」。handler 完成回调写 pending + 连接投回 g_pool，续处理 worker 段2 走
normalize + respond + 日志 + keep-alive 下一请求。

## 一、D0 复核（2026-09-10，dongyue，基线 f4b805f / runtime.c 15853 行）

- **px_serve 并发模型**（与 http_serve 本质不同，复核确认）：`bi_px_serve`（runtime.c
  ~15012）accept → `px_pool_push`（~15236）→ 常驻 `px_pool_worker`（~15247，PX_POOL_MAX
  =256，默认 max_conn=32）→ **每连接一个线程**跑 `px_conn_worker`（~14246）阻塞
  keep-alive 循环直到连接关闭（SO_RCVTIMEO 15s；无事件化 IDLE 交还）。
- **px_conn_worker**（~14246-14580，约 330 行）：整连接循环。迭代：读请求头 → 解析
  method/path/query/headers/body（chunked/落盘）→ req dict（M92 precise 登记）→ h2c
  升级/prior knowledge（px_h2_handle 整连接接管）→ **px_http_dispatch（~13783，公共
  管道）** → 清理 → 下一请求。`PxConn conn` 为**栈对象**（TLS 会话），`PxHttpOut out`
  绑定 &conn，`g_cur_conn=&conn`。
- **handler 调用点（全部在 px_http_dispatch 管道内）**：
  1. **vhost handler**（~13810）：vhost() 注册 VM/原生 handler → `px_call(vhandler,&req,1)`
     → 返回非 null → px_vhost_normalize + respond（**无访问日志，历史语义**）；返回 null
     → 落入 route/静态（docroot 回退，__thread g_vroot_tls 已 store）。
  2. **route handler**（runtime_route.c px_route_try_dispatch ~302，runtime.c ~13890 调用）：
     route_match → per-route 限流 → **middleware 链**（px_call 短路）→ 命中 handler
     `px_call(handler,[req,params],2)` → route_normalize + route_send + 访问日志。
  3. .px 子进程池（px_pool_run，C 阻塞等子进程，不占协程让出面 → 不在本里程碑）。
  4. CORS/限流/静态/日志：纯 C，无 VM handler。
- **与 http_serve/sse_serve 的差异**：http/sse 走 fserve job 池（M88-B 事件化），
  handler 调用点单一、http_pend/sse 注册表拆段已落地；px_serve 是 g_pool 整连接
  阻塞循环 + 多 handler 调用点深埋管道 + PxConn 栈对象 → **不可直接平移 http_pend**：
  拆段期间连接对象必须跨 worker 存活 → PxConn 堆化 + 连接注册表（sse 蓝本）。
- **GC 面**：px_conn_worker 请求迭代已有 M92-S2c precise 登记（px_root_push/PX_KEEP
  req/headers/form）；拆段后 pending 表持 req/resp/params 跨线程 → **须补 GC 根
  （http_pend_gc_mark 同款独立锁 + SIG_GC_STOP 屏蔽）**。
- **逃生舱**：PX_NATIVE/原生 handler → 同步直调零变化；--c 轨（无 coro.o）→ weak
  px_coro_spawn_ex 空转 → 退回同步（理论不达，pxi_vm 双轨均为 VM 产物）。
- **h2c/h2（PX_NO_H2 裁剪面）**：h2 整连接 px_h2_handle 内自处理请求循环，**不经
  px_conn_worker 拆段点** → 本里程碑 h2 连接 handler 仍同步（h2 请求循环另一实现，
  另立二期评估；px_serve HTTP/1.1 + H3 管道共用 px_http_dispatch，H3 走
  px_http_dispatch_h3 → 同步拆段点需保持 H3 零变化）。

## 二、范围决策（重要）

px_serve 连接若要 handler 让出后 **g_pool 线程释放去处理其它连接**，连接处理必须
从「每连接独占线程到关闭」升级为「连接挂起 + handler 协程 + 完成投回续处理」。
与 SSE 同构：连接对象堆化并注册，worker 拆段返回，done 回调投回 g_pool，续处理
worker 从注册表取回连接继续。

- **S2（本里程碑核心，分两步落地）**：
  - **S2a（连接对象堆化 + 拆段内核 + route handler）**：px_serve 连接注册表
    （fd→PxConn*，含 TLS 会话存活 + PxHttpOut 重建），px_conn_worker 改为从注册表
    取/建连接；拆段点 = px_http_dispatch 返回 int（0=同步完成 / 1=已拆段待续），
    route VM handler 命中 → px_call 前登记 pending(stage1) + px_coro_spawn_ex(handler,
    [req,params], done, fd) → worker 返回（线程释放）；done（coro worker）→ pending
    stage2 + px_pool_push(fd)；续处理 worker 重入 px_conn_worker → pending take
    (stage2) → route normalize + respond + 访问日志（段2）→ keep-alive 下一请求。
  - **S2b（vhost handler 拆段 + 静态/.px 尾段续处理）**：vhost VM handler 命中 → 拆段；
    段2 遇返回 null（docroot 回退）→ 调用管道尾段（route+静态+.px，抽公共
    px_http_dispatch_tail）完成 —— 不重复 CORS/限流（已在段1 执行）。
  - 未命中 route/vhost VM（静态/.px/CORS/限流/原生/非 VM）→ **原同步路径零变化**；
    H3（px_http_dispatch_h3）传 async_ok=0 → 同步路径零变化。
- **S3（收口）**：m98_s1 验证套件（route handler /sleep /chan 长业务并发 + g_pool 少量
  + 线程收敛 + vhost handler + precise GC 压力 + keep-alive/HEAD/日志回归）+ px_serve
  全量回归（m28_route/m29_webprod/m31_vhost/m33_route_rate_limit/m53_s4_pxserve_h3/
  m57_s7_vhost_headers/m27a/m27b 等）+ 双轨冒烟 + pxi/pxi_vm 重链 + 文档 + commit。
- ⚠️ 二期（另立）：px_serve 连接级事件化 IDLE（keep-alive 空闲不占 g_pool 线程）、
  h2 连接 handler 协程化、.px 子进程池协程化、middleware 链协程化。

## 三、风险与预案

| 风险 | 预案 |
|---|---|
| px_serve 核心路径回归（route/vhost/静态/.px/H3/TLS 共用管道） | 未命中 VM handler / H3 / 原生 → 同步路径零变化（async_ok=0）；命中才拆段；全量回归 + 逐字节对拍 |
| PxConn 栈对象跨 worker 释放丢 TLS 会话 | S2a 先行：连接堆化注册表（sse_conn_worker 同款 xmalloc + fd 表）；续处理从表取回，不重新握手 |
| req/params/resp 跨线程（handler 协程期 + done 投回） | px_pend 表持 req/resp/params 为 GC 根 + 独立锁（g_pxpend_mu）+ 临界区屏蔽 SIG_GC_STOP（http_pend fix 同款）；gc 标记期补标 |
| handler 挂起期连接 fd 无人读/超时 | 与原线程模型 handler 占用期一致（挂起=连接挂起）；done 投回续处理；fd 复用由注册表 fd 唯一占位防串扰 |
| vhost null 回退 / route normalize / 日志顺序语义漂移 | 段2 复用既有 normalize/respond/日志代码（抽公共函数），仅调用点异步化；S2b 尾段复用管道本体（不重复 CORS/限流）；m31_vhost/m33 对拍 |
| runtime_route.c 独立单元 + PX_NO_ROUTE 裁剪 | route 拆段辅助封装 runtime.c（weak 探测 px_route_has + async_ok），runtime_route.c 仅加"handler 执行拆出"导出函数（弱化裁剪边界，语义 R1001） |
| 优雅关闭（g_px_stop）与拆段在途 | 连接注册表随 px_serve 收尾清理（同 sse_server_close_fd 思路）；g_px_inflight 拆段期仍计数 |

## 四、S 拆分与验证
| S | 内容 | 验证 |
|---|---|---|
| S1 | 立项 + D0 复核 + 本文档 | 本文档 |
| S2a | 连接堆化注册表 + px_http_dispatch async 拆段内核 + route VM handler 拆段（pending + spawn + done 投回 + 段2 normalize/respond/日志） | m98_s1 套件：route handler /sleep /chan 长业务并发 + g_pool=2 + 线程收敛 + precise GC + keep-alive/HEAD/日志回归 + route 对拍 |
| S2b | vhost VM handler 拆段 + null 回退尾段（route+静态+.px 公共 tail）+ H3 async_ok=0 确认 | vhost handler /sleep + null 回退 + TLS 冒烟 + H3 回归零变化 |
| S3 | px_serve 全量回归 + 双轨冒烟 + 重链 + 文档 + tag | 全绿 |
