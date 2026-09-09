# M95_PLAN · 服务端 handler 协程化（D8-② http_serve 系）

> 状态：✅ **M95-S1 立项定稿（2026-09-10）**。✅ **M95-S2 完成（2026-09-10）**：
> http_serve/http_serve_unix handler 协程化闭环 —— VM handler 以帧协程执行
> （完成回调投回续处理），fserve worker 释放；m95_s2 套件 12 PASS + 协程内核回归
> 全绿（m93_s2/s3 + m94_s2/s3 20 PASS）。**S2 排障记录**：① verify 门5 假 FAIL
> 根因 = verify.sh 统计 bug（echo 未重定向致响应粘连无换行、grep 行锚定失效），
> 修正 printf '\n' >>out + 逐文件 grep -l；② 真实死锁修复 = http_pend_gc_mark
> 在 GC 标记期持 g_conn_mu，与不屏蔽 SIG_GC_STOP 的既有 g_conn_mu 临界区形成
> STW 死锁面（GC 等锁 vs 持锁线程被暂停）→ pending 表改独立锁 g_hpend_mu +
> 全部临界区屏蔽 SIG_GC_STOP（对齐 M92 根栈原子性模式）。S3（sse_serve）/
> S4（px_serve）二期编排见 §五。
> 上游：M93_PLAN §D8 ②（http_serve/sse handler 协程化）+ M94_PLAN §五 编排
>   （M95 = 服务端活跃请求处理从 fserve pthread 池 → 协程）+ M94 抢占（防单
>   handler 饿死池）+ M93 阻塞原语让出（chan/sleep 占协程不占线程）+ M88-B 事件化
>   （空闲连接不占 worker）+ M92-S2c（http_conn_worker/sse_conn_worker 已做
>   precise 登记 PX_KEEP——协程化前置已就绪）。
> 基线：v0.2.0-m94（6724835）。性质：**L0 runtime**（服务端并发模型层）。

## 〇、一句话

http_serve/http_serve_unix 的活跃请求 handler 执行从「fserve pthread worker
线程内同步 px_call（handler 长业务占线程）」→「handler 帧协程（M93 协程，阻塞
让出占协程不占线程）」，fserve worker 在 handler 挂起后释放去取下一 job；handler
完成后由协程 worker 完成回调把连接续处理（响应写 + keep-alive）投回 fserve 队列。
突发长业务请求占协程不占 fserve worker；空闲连接仍 M88-B 事件循环照看。

## 一、D0 侦察量化（2026-09-10，dongyue，基线 6724835）

- **现状**：http_serve(port,handler)/http_serve_unix → fserve_ensure（默认 256
  pthread，PX_SERVE_WORKERS 夹取 [8,4095]）→ accept 循环 fserve_push(fd, HTTP)
  → fserve_worker 取 job 调 http_conn_worker(fd)。http_conn_worker 大循环一次迭代：
  读请求头(px_recv_wait)→解析→req dict→（GET+http_stream 命中→stream_takeover
  接管）→ `resp = px_call(handler, &req, 1)`（同步嵌套）→ 写响应 → keep-alive 判定
  （close / 交还 IDLE 事件循环 / 继续读）。
- **让出边界（M93 D1）坐实**：px_call = px_vm_run_func（yield_ok=0，不可让出）；
  协程让出只发生在 px_vm_run_coro/px_vm_resume（yield_ok=1，coro.c worker 顶层）。
  → handler 要让出（chan/sleep 不占线程），必须以**顶层帧协程**执行；C 壳
  （http_conn_worker）不能驻留 C 栈同步等 handler（让出回卷 C 栈会丢 C 壳状态）。
  → 唯一正道：**handler 调用点异步化（C 壳拆段）**：读+解析（段1）→ spawn handler
  帧协程 → worker 释放 → handler 完成回调投回续处理 job → 段2（响应写+keep-alive）。
- **handler 形态**：默认轨（VM）px 函数 = PX_FUNC(fn=px_vm_entry, ctx=PxVMFunc*)
  → 可 px_coro_spawn_ex 协程化；PX_NATIVE（C 注册）handler = 逃生舱 → 保留同步
  直调（行为零变化）。判定 = handler.fn==px_vm_entry。
- **SSE / px_serve（二期边界，见 §五）**：sse_conn_worker 的 handler 执行依赖
  `__thread g_cur_conn`（TLS 连接上下文，协程跨 worker 线程恢复会丢）→ 需先做
  「协程局部连接上下文」机制；px_conn_worker（M31.4b 独立池 + route/vhost 管道）
  handler 调用点深埋 runtime_route，改造路径独立。二者列二期（下轮）。
- **GC 根面**：req 挂起期间保活 = handler 协程 args（px_coro_gc_mark_roots 已标）
  + 新增 http pending 表（fd 索引，持 req/resp LXValue → 挂 gc 标记）。响应段
  （finish）在 fserve worker 线程执行（同现 http_conn_worker 收尾），precise 登记
  已就绪（M92-S2c PX_KEEP 作用域平移）。
- **收益场景**：N 个并发请求 handler 内 sleep/chan 长阻塞 → 旧模型占 N 个 fserve
  线程（>256 排队）；新模型占 N 个协程（PX_MAX_COROS 默认 65536），fserve worker
  少量即可。handler 纯计算短请求 → M94 抢占轮转，无饿死。

## 二、设计定稿

### D1 · 协程完成回调（coro.c 扩展，通用机制）
- PxCoro 增 `void (*done_cb)(void* ud, LXValue ret)` + `void* done_ud`。
- 新增 `px_coro_spawn_ex(ctx, args, nargs, done_cb, done_ud)`（px_coro_spawn 保持
  原签名 = ex 传 NULL 壳）。
- vm.h PxVmState 增 `LXValue ret_val`：px_vm_run_coro/px_vm_resume 跑完（return 0）
  时把顶层返回值存入 st->ret_val（供完成回调取）。
- coro.c worker 完成路径（yield_rc==0）：摘 g_all 后、free 前调 done_cb（worker
  仍注册 GC；回调开头 PX_KEEP(ret) 防 precise 窗口 GC）。隔离异常（longjmp 跳过）
  路径：协程异常终止 = 无返回值 → 完成回调不触发，由 pending 连接 15s/keep-alive
  语义兜底收尾（异常即连接处理中断，语义对齐原 px_error 中断）。
- px_coro_gc_mark_roots 增标 `c->vm.ret_val`（完成窗口兜底）。

### D2 · http pending 表（fd 索引 + GC 根）
```
typedef struct HttpPending { int fd; int active; int stage; // 1=handler 运行中
  2=handler 完成待 finish；LXValue req, resp; int method_head, client_close; } HP;
static HP* g_hpend;  // fd 索引表（懒分配 cap=PX_MAX_CONNS，同 ConnCtx 语义）
```
- 锁 = g_conn_mu（与 ConnCtx 同锁；表容量独立分配，fd 超上限 → 不登记 → 退回
  同步路径）。
- GC：mark 期（px_coro_gc_mark_roots 旁）weak/直接 hook 遍历 active 项标记
  req/resp（持 g_conn_mu）。
- 生命周期：段1 读+解析完成 → put(fd, req, flags)（stage=1）→ spawn handler 协程
  → handler_done（coro worker 线程）→ 写 resp（stage=2）→ fserve_push(fd, HTTP)
  唤醒续处理；段2（fserve worker）http_conn_worker 重入 → 循环顶见 stage=2 →
  take(fd) → 响应写 + keep-alive 判定 → 清理项（fd close / 交还 IDLE 时项随连接
  收尾；fd 复用由 px_evc_close 统一清 pending 项防串扰）。

### D3 · http_conn_worker 拆段（响应段抽公共函数）
- 原第 7/8 步（file 流式、HEAD、px_http_build_response、keep-alive/交还）抽为
  `http_send_resp(fd, req, resp, method_head, client_close)` 返回动作码
  （0=close 已 px_evc_close / 1=交还 IDLE / 2=继续读下一请求）——同步路径与
  finish 路径共用，逐字节语义与现版一致。
- handler 调用点分派：
  - handler 非 VM（PX_NATIVE）→ 原同步 px_call + http_send_resp 路径（逃生舱零变化）
  - handler VM（fn==px_vm_entry）→ put pending + px_coro_spawn_ex(handler.ctx,
    [req], http_handler_done, fd) → px_root_pop → return（worker 释放）
  - handler_done(ud=fd, ret)：PX_KEEP(ret) → g_conn_mu 内查 pending[fd]（stage 1→2
    resp=ret）→ fserve_push(fd, HTTP)（若表项已清/连接已关 → 忽略 ret，防 fd 复用）
- http_conn_worker 循环顶：若 pending[fd].stage==2 → take → http_send_resp →
  按动作码 break（close/交还）或 continue（读下一请求）。
- 请求头/body 读（段1）原逻辑不动（px_recv_wait 事件化兜底：job 派发时数据在途，
  快速）。M83-S6 http_stream 分支在 handler 前，原样优先。

### D4 · 并发/生命周期安全
- handler 协程运行期连接 fd 无人读（客户端 pipelining 数据堆积 socket 缓冲）→
  与原线程模型 handler 占用期一致（handler 挂起 = 连接挂起）；handler 完成后
  finish 的 px_fd_readable_now 检测到在途数据 → 继续读下一请求（keep-alive 语义
  保持）。15s 空闲超时语义：交还 IDLE 后由事件循环 tick 关（原样）。
- handler 协程异常/死循环/永久等待：M94 抢占轮转（不饿死池）；协程永久 BLOCKED
  = 连接永久挂起（原模型 fserve 线程永久阻塞同语义）；异常隔离 longjmp → 协程
  终止 → 连接 pending 不 finish → fd 由客户端断开/事件循环 15s 超时收尾（px_evc_
  close 清 pending）。可接受（原模型 px_error 中断连接同效果）。
- fd 复用防串扰：px_evc_close 路径清 pending[fd] 项（active=0）。handler_done 查
  表项不存在（连接已关）→ 丢弃 ret。
- fserve_push 在 coro worker 线程调用 → g_fserve_mu 线程安全 ✓。

## 三、S 拆分

| S | 内容 | 验证 |
|---|---|---|
| **S1** | 立项 + D0 + 设计定稿 | 本文档 |
| **S2** | coro 完成回调 + ret_val + pending 表 + http_conn_worker 拆段 | m95_s2 套件：长业务 handler（sleep/chan/spawn）并发请求功能正确 + 线程数收敛（fserve 1-2 + coro worker 少量）+ handler 结果/keep-alive/流式/HEAD/close 逐字节对拍 + precise 低阈值压力 |
| **S3** | 全量回归（m82/m83_s6/http_stream/m89_s3d/vm_ab/diffcheck）+ 文档 + commit | 全绿 |

## 四、范围与边界
- **本里程碑做**：http_serve / http_serve_unix（共享 http_conn_worker）handler 协程化。
- **二期（M95_S4/S5 或另立，见 §五）**：sse_serve（g_cur_conn TLS → 协程局部连接
  上下文机制前置）、px_serve（route/vhost 管道）、fserve 默认 worker 数随协程化
  下调策略。
- 逃生舱（--c 产物、PX_NATIVE handler）行为零变化。

## 五、后续编排（sse/px handler 协程化）
- **M95-S4（sse_serve）**：前置 = 「协程局部连接上下文」（PxCoro 关联 PxConn*
  g_cur_conn 替代 __thread；sse handler 内 sse_send/sse_close 按 conn id 查表本不
  依赖 TLS，需核查 ws 升级等 g_cur_conn 消费点）→ sse_conn_worker step7 handler
  协程化 + 完成回调做 step8 收尾（交 IDLE / TLS 保持读）。
- **M95-S5（px_serve）**：M31.4b g_pool 连接处理 = px_conn_worker（route/vhost/
  静态/.px 管道）handler 调用点定位（runtime_route action）→ 同构协程化；g_pool
  线程收敛到 coro worker。
- 完成后评估 fserve/pool 默认 worker 数下调（协程承载长业务后 256 → CPU 级）。

## 六、风险与预案
| 风险 | 预案 |
|---|---|
| http 核心路径回归（m82/m83 大量 http_serve 用例） | 同步逃生舱路径保留（PX_NATIVE handler / 非 VM）；diffcheck + m82/m83_s6/vm_ab 全量把关；语义逐字节对拍 |
| 完成回调/precise GC 窗口（ret_val、resp 转移） | PX_KEEP 登记 + pending 表全局根 + mark 补标 ret_val；precise 低阈值压力套件 |
| handler 挂起期 fd 无人读/超时语义偏差 | 与原线程模型占用期语义对齐（挂起 = 连接挂起）；15s 由事件循环收尾；专项用例 |
| fserve_push 跨线程（coro worker → fserve） | g_fserve_mu 线程安全；专项并发用例 |
| 协程耗尽/死锁（handler 永久等待） | PX_MAX_COROS 上限 + M94 抢占；语义与原线程占死对等，不新增死锁面 |
