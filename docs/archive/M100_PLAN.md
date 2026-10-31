# M100_PLAN · middleware 链协程化 —— 二期候选第 2 项（D8-② 补缺）

> 状态：✅ **M100 完成（S2 链状态机 defer 实现 + S3 收口，tag v0.2.0-m100）**。
> 基线 v0.2.0-m99（d0d1f60）。S2 验证：examples/m100 verify.sh 11P/0F（并发 20×/slow-mw
> middleware sleep600 wall=0.60s（同步占 2 worker 需 ~6s+，核心铁证）/ fast 30 不饿死
> 0.01s / 短路 401/403 / 多段链 null 推进 / keep-alive 5 请求 / 线程峰值 12≤20 /
> (middleware) 短路日志 / 优雅关闭干净退出）。S3 收口：px_serve 管道直接面回归
> （m28_route/m29_webprod/m31_vhost/m33_route_rate_limit/m57_s7_vhost_headers 5P +
> m43_webapp 10P/0F STDOUT==GOLDEN + m98_s2 7P + m99_s2 8P）+ 协程内核 suites
> 13 套件（m82/m83_s6/m89_s3d/m93_s2/s3/m94_s2/s3/m95_s2/s4/m96_s2/s3/m97_s2/s3）
> 全绿 + diffcheck --all rc=0 + vm_ab 38P/0GAP/0F + 双自举证明（C 轨 rc=0 + BC 轨
> 30582 行 dump 逐字节一致）+ pxi/pxi_vm 重链吸收 M100 runtime + 文档。
> 上游：M98（px_serve route/vhost handler 拆段协程化 —— px_pxserve_defer + PxPend
> 注册表 + 段2 续处理，本里程碑直接复用其骨架）；M99（px_serve 连接级事件化 IDLE）；
> M93/M94（帧协程内核 + 抢占）；M96（offload 让出）。M98_PLAN §二 ⚠️ 二期清单第 4 项
> "middleware 链协程化" 正式立项（原 M100 位次；二期排序第 2 项）。
> 性质：**L0 runtime**（px_serve route 管道内 middleware 链并发模型层）。
> 规划前置建议（M98 收口时提出）：middleware 多为纯计算/短路（记录/校验/限流计数/CORS）
> → 让出场景少 → 若 D0 证实，可降级"保持同步短路 + 仅让出点化"轻量版 —— 本计划按
> D0 结论落定（见 §二 范围决策）。

## 〇、一句话

px_serve（async_ok=1）route 命中后，**middleware 链上 VM middleware 的执行**从「在
g_pool worker 线程内同步 px_call（链上任一 middleware 内 chan/sleep 长业务 → 占死
worker）」→「链状态机 defer：逐段帧协程 spawn 执行（占协程不占 worker）+ done 回调
推进链（null → 下一 middleware / 全 null → 进入 handler 段 / 非 null → 短路段2）」。
middleware 链获得与 route/vhost handler（M98）同级的让出能力，**同步路径零变化**。

## 一、D0 侦察（2026-09-11，dongyue，基线 d0d1f60 / runtime.c 16317 行）

### 1. middleware 链现状（runtime_route.c px_route_try_dispatch @325）

- 语言层 API：`middleware(fn)`（bi_middleware @~155）注册全局链 `g_middlewares[32]`
  （static LXValue 数组，注册顺序 = 执行顺序；**只收 PX_FUNC**，不收 PX_NATIVE）。
- 执行点：px_route_try_dispatch @325：route_match（同步纯 C）→ per-route 限流（同步
  C）→ **middleware 链 for 循环 @~367-391**（持 g_route_mu 快照 mws[] → 逐个
  `px_call(mws[i], &req, 1)`：**null → 继续下一段；非 null → 短路** = route_normalize
  + route_send + 访问日志（文案 `(middleware)`）→ return 1）→ handler 段 @392-405
  （M98：async_ok && VM → px_pxserve_defer 拆段 return 2；否则同步 px_call → 公共
  px_route_respond 段2 @304 → return 1）。
- **让出缺口（M98 明确遗留）**：middleware 链仍在 g_pool worker 线程内同步 px_call。
  链上 VM middleware 内 chan/sleep/长业务（与 handler 同构）→ 阻塞 worker；多慢
  middleware 请求把 max_conn 池占满 → 快路径饿死。M98 只拆了「链之后的 handler」，
  链本身未协程化。
- **与 handler defer 的结构差异**：handler defer 是单点（跑完 → resp → 段2 respond）；
  middleware 是**多段串联**（每段独立 px_call + 返回值决定链去向：短路 / 继续 / 进
  handler）→ 不能平移单点 defer，需「链状态机」：逐段 spawn + done 回调推进。

### 2. 可复用基建（M98/M99 已落地）

- **PxPend 连接挂起注册表**（runtime.c @14349）：fd→堆 PxConn + stage（0 无挂起 /
  1 协程运行 / 2 待段2）+ kind（0 route / 1 vhost）+ req/resp GC 根 + 独立锁
  g_pxpend_mu（屏蔽 SIG_GC_STOP）+ px_pxserve_pend_gc_mark 补标（@14522 + GC 两路径
  @1362/@1441 调用）。
- **px_pxserve_defer**（@14575）：stage 0→1 + kind/vroot/method_head/client_close/
  req_id 登记 + req 入根 → px_coro_spawn_ex(handler, hargs, done=px_serve_route_done,
  ud=fd)。仅 PX_FUNC 且 fn==px_vm_entry（VM handler）+ 有协程内核才 defer。
- **px_serve_route_done**（@14543）：coro worker 线程；stage 1→2 写 resp + tmp 移交
  + px_pool_push(fd) 投回（g_px_stop → 直接 px_pxpend_close）。
- **px_conn_worker 段2 续处理**（@14644+）：重入 worker 见 stage==2 → take → skind
  分派（1 vhost respond / null 回退续管道 skip_pre；0 route → px_route_respond）→
  keep-alive → px_pxserve_idle_after_resp（M99 交 IDLE）或 break。
- **GC 根面**：保守（默认 g_gc_precise=0 扫线程栈）+ precise（px_root_push/PX_KEEP
  登记）。g_middlewares 的 func 保活依赖 VM 全局/模块表持有 def 函数（既有语义，
  同步链已如此）；defer 引用沿用一致语义。

### 3. middleware 真实形态（让出价值评估）

- 库内 middleware 用例极少：`examples/m43_webapp/middleware.px`（1 文件，纯计算：
  print 记录 + starts_with /admin → 401 短路 + null 放行）；stdlib/tools 无
  middleware 注册。无「middleware 内 chan/sleep 长业务」真实用例（印证 M98 二期注）。
- **结论**：middleware 让出是**能力补齐**（与 handler 对齐），非既有痛点修复——价值
  在消除「未来 middleware 写长业务会占死 worker」的架构缺口 + 语义一致性。

### 4. 范围决策定稿（轻量版但完整闭环）

- **链 defer 触发条件**（全 VM 链）：`async_ok && px_coro_spawn_ex && px_vm_entry &&
  链非空 && 链上每个 middleware 都是 VM 函数（PX_FUNC 且 fn==px_vm_entry）`。
  非 VM PX_FUNC（C 闭包）middleware 无法 spawn → 含 C 闭包的链**整链退回同步**
  （行为与 M98 完全一致，C middleware 罕见且本不长业务）。
- 触发时：**middleware 链 + 通过后的 handler 整体走链状态机**（链段 spawn → null 推进 /
  短路段2 / 全 null 进 handler 段）。handler 段完成 = 现有 kind=0 route 段2 语义。
- 未触发（async_ok=0 / 无 VM 链 / 无协程内核 / http_serve / H3）：**原同步路径
  零变化**（含短路 + handler defer 尝试）。
- **PxPend 扩展**：kind 值域扩为 0=route handler 完成段2 / 1=vhost 段2 / 2=middleware
  链 defer 运行中（状态机）/ 3=middleware 短路段2。新增字段（GC 根面）：
  `int mw_i; int mw_n; LXValue mw_chain[32]; LXValue mw_handler; LXValue mw_params;`
  （mw_chain 快照 defer 登记时持 g_route_mu 拷入 → spawn 下一段免锁直取 + 函数值
  显式入根跨协程保活，不依赖 g_middlewares 运行期一致性）。
- **公共短路 respond**：middleware 短路段2 抽 `px_route_mw_short_respond`（normalize +
  send + `(middleware)` 访问日志）——同步短路（px_route_try_dispatch 内）与异步段2
  （kind=3）共用（px_route_respond 同款模式）。
- **GC mark 扩展**：kind==2 时补标 mw_chain[0..mw_n) + mw_handler + mw_params。
- **body tmp**：链 defer 与 handler defer 同路径（px_conn_worker dret==1 分支
  px_pxpend_set_tmp 移交，done 清理）——复用，零新增。

## 二、S 拆分与验证

| S | 内容 | 验证 |
|---|---|---|
| S1 | 立项 + D0 + 设计 | 本文档 |
| S2 | 实现：PxPend kind=2/3 + mw 字段 + px_pxserve_mw_defer（链 defer 登记 + spawn 首段）+ px_serve_mw_done（链推进状态机）+ px_route_try_dispatch 分流（全 VM 链 → defer return 2）+ px_route_mw_short_respond 公共化（同步短路改造复用）+ px_conn_worker 段2 skind==3 分支 + px_pxserve_pend_gc_mark 扩展 + runtime.h 导出 | examples/m100 套件：门1 慢 middleware（sleep 500ms）并发 20×/max_conn=2 wall 数秒 + fast 不饿死；门2 短路（401/dict）与同步路径逐字节一致；门3 多段链 null 推进 / 中途短路 / 全 null → handler 正确；门4 短路日志 `(middleware)` 文案与同步一致；门5 keep-alive 续处理 + 访问日志不丢；门6 线程峰值 ≤ 上限（worker 释放）；门7 全 VM 链 defer / 含 C 闭包链同步 / async_ok=0 同步 三分支行为一致 |
| S3 | 收口：px_serve 全量回归（m28_route/m29_webprod/m31_vhost/m33_route_rate_limit/m43_webapp/m57_s7_vhost_headers + m98_s2 + m99_s2）+ 里程碑 suites + vm_ab + diffcheck --all + 双自举证明 + pxi/pxi_vm 重链 + CHANGELOG/ROADMAP 文档 + tag v0.2.0-m100 | 全绿 |

## 三、风险与预案

| 风险 | 预案 |
|---|---|
| 链状态机推进与同步路径语义漂移（短路响应/日志/顺序） | 短路段2 抽公共 px_route_mw_short_respond（同步 + 异步 kind=3 共用，文案逐字一致）；handler 段完成 = kind=0 复用既有 px_route_respond 段2；链 defer 仅全 VM 链触发，其余零变化 |
| PxPend 字段膨胀（mw_chain[32] 快照 512B/槽） | g_pxpend 仅 px_serve 连接使用（M98 已如此，256 连接 ≈ 128KB，可接受）；初始化/close/take 置 PX_NULL 防误标 |
| 链 defer 跨段 GC 根泄漏/漏标 | kind==2 期间 mw_chain/mw_handler/mw_params 入 px_pxserve_pend_gc_mark 补标；stage 归还（take/close）时清 PX_NULL |
| done 回调推进期 g_pxpend 竞争/连接关闭 | 沿用 px_serve_route_done 锁协议（g_pxpend_mu + SIG_GC_STOP 屏蔽 + active/stage 校验）；g_px_stop → 直接收尾防 push 无消费者 |
| PX_NO_ROUTE 裁剪（runtime_route.c 独立单元） | kind=2/3 仅 route 链 defer 产生；PX_NO_ROUTE 下 route 表空 → 无链 defer 登记（理论不达，防御同 skind==0） |
| 帧协程失败/px_error 逃逸（middleware 内报错） | 沿用 M96/M98 错误语义：协程内 px_error → 协程异常终结 → done 回调 ret 为错误值 → 段2 归一化响应（与同步 px_call px_error 语义对齐面另验证） |

## 四、文件改动面（预估）

- runtime/runtime.c：PxPend struct（+mw 字段）、px_pxpend 初始化/close/take 清理、
  px_pxserve_pend_gc_mark 扩展、新增 px_pxserve_mw_defer + px_serve_mw_done、
  px_conn_worker 段2 加 skind==3 分支、px_http_dispatch 注释。
- runtime/runtime_route.c：px_route_try_dispatch middleware 链分流（全 VM 链 →
  px_pxserve_mw_defer return 2）、抽 px_route_mw_short_respond（同步短路改调）、
  新增链 VM 判定辅助。
- runtime/runtime.h：px_pxserve_mw_defer / px_route_mw_short_respond 导出声明。
- examples/m100/：验证套件。
