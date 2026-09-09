# M102_PLAN · 二期候选侦察 + .px 子进程池协程化（D8-② 收官）

> 状态：🔄 **进行中（S1 立项 + D0 侦察定稿）**。
> 基线 v0.2.0-m101（31ff74a，px_serve 并发 TLS 握手修复收口后）。
> 上游：M96（offload 外包执行器 β 路线）；M98/M100（px_serve 管道拆段 defer +
> PxPend 注册表 + 段2 续处理骨架）；M99（连接级事件化 IDLE）。M95_PLAN §五 S5
> 完成后的剩余"同步阻塞点"清点。性质：**L0 runtime**（px_serve 管道同步阻塞收尾）。

## 〇、M102 范围（用户拍板口径）

M102 = 二期候选 **B（.px 子进程池协程化）+ C（h2 handler 协程化）**：先各做 D0
侦察，**成立则实施，不成立就地关闭并记录**（侦察驱动，不为不存在的东西写代码）。

## 一、D0 侦察结论（2026-09-10，dongyue，基线 31ff74a）

### 候选 C · h2 handler 协程化 → ❌ 关闭（记录，不实施）

- **h2 现状**：runtime_h2.c（M35，640 行）是最小 h2c 实现：h2c Upgrade + prior
  knowledge + HPACK（静态表+字面量+Huffman）+ 帧层。响应 = **固定回显**
  `PuXian HTTP/2 method=… path=…`（单流 GET → HEADERS+DATA），**不调用 VM handler、
  不经 px_http_dispatch 公共管道**。
- **TLS ALPN 已固定 http/1.1**（runtime.c 13241-13245 注释链）：M-B9b 决策 ——
  vhost handler 仅在 http/1.1 路径生效；h2 帧循环（M37）绕过 vhost，生产 Web 服务
  （多站点/deny/SPA/反代）必须走 handler → ALPN 只声明 `{"http/1.1", NULL}`。
  → TLS 端口实际**不再协商 h2**；仅明文 h2c Upgrade / prior knowledge 进入帧循环。
- **"h2 handler 协程化"前提不存在**：当前 h2 帧循环没有 VM handler 概念（固定响应、
  不经管道），无从"协程化 handler"。要让 h2 具备 handler = 把 h2 请求接入公共管道
  （完整 h2 服务端：HPACK 动态表 / 多路复用流调度 / 流控 / 接入 px_http_dispatch），
  是独立大工程且与 M-B9b 的 ALPN 决策冲突（需先重启 ALPN-h2 的产品决策）。
- **记录**：h2 生产化（含 handler 管道接入）另立大里程碑评估；本里程碑关闭，零代码。

### 候选 B · .px 子进程池协程化 → ✅ 成立（本里程碑实施）

- **执行模型（M17/M25）**：`px_pool` = 预派生 N 个常驻 `px --worker` 解释器进程
  （PHP-FPM 风格，默认 4，PX_POOL_WORKERS 可配 ≤16）；父进程把任务帧
  （path\0env_json\0dump\0timeout）写入空闲 worker stdin → worker 执行 → 结果帧写回
  stdout。池满/崩溃 → 兜底每请求 fork+exec `px run`。**父进程在 px_pool_recv_result
  用 poll+read 同步阻塞等子进程结果**（timeout_ms 内；超时 SIGKILL）。
- **阻塞点（2 个）**：
  1. **px_serve URL 直达 .px**（px_http_dispatch 静态/.px 分支 14157-14260）：
     g_pool 连接 worker 线程内**同步 C 阻塞**等子进程 —— 慢 .px 占死 worker（与
     M98/M100 解决的 handler 慢同型，但 .px 分支是纯 C 非 VM native 调用点）。
  2. **语言层 px_exec**（bi_px_exec 16068，VM native）：协程 ctx 内调 px_exec →
     内部 px_pool_run C 阻塞 —— M96 offload 名单未含 px_exec（D2 二期候选明确
     "popen/子进程等待类"排期外）→ 直调阻塞 worker。
- **价值**：px_serve 是"应用平台"（docroot 下放 .px 当脚本页，M17/M25/M43 webapp
  均用）；慢 .px 请求饿死 g_pool worker 的路径与 route/vhost handler 协程化同构，
  是 M98/M99/M100 协程化后 px_serve 管道**最后的同步阻塞收尾**。
- **H3 面**：px_http_dispatch 由 px_serve(HTTP/1.1, async_ok=1) 与 H3
  (px_http_dispatch_h3, async_ok=0) 共享 —— .px 分支拆段必须 async_ok=1 时才 defer，
  async_ok=0 原同步路径逐字节零变化（M98 route defer 同款约定）。

## 二、范围决策（S 拆分）

| S | 内容 | 交付 |
|---|---|---|
| **S1**（本 commit） | 立项 + D0 侦察定稿（h2 关闭记录 / .px 成立）+ 本文档 | docs/M102_PLAN.md |
| **S2** | .px 协程化实现 + examples/m102 验证套件 | 见下 D1-D4 |
| **S3** | 收口：全量回归 + 双自举 + pxi/pxi_vm 重链 + 文档 + tag v0.2.0-m102 | CHANGELOG/ROADMAP |

## 三、设计定稿（S2，D1-D4）

### D1 · 语言层 px_exec 纳入 offload 名单（最小改动）
- `px_native_offload_kind`（runtime.c 4032）加 `px_exec` → 协程 ctx 内调 px_exec
  自动外包执行线程池（M96 β），worker 不卡。验证 bi_px_exec 内部仅调 native
  （json_stringify 为 native）不回调用户 VM → 满足 D5 约束；px_pool 全局池有
  g_px_pool_mu 保护，外包线程并发安全（与多连接线程直调并发语义一致）。

### D2 · px_serve URL 直达 .px 请求异步化（拆段 defer + 外包执行器）
- 复用 M98/M100 骨架：px_http_dispatch .px 分支在 `async_ok && 未关闭` 时：
  - **段1**（连接 worker 内）：解析 docroot/fpath/is_px（现有）→ 构造
    env_json（json_stringify，现有）→ **登记 PxPend kind=4**（存 fd/req GC 根 +
    fpath/env_json/timeout_ms 快照）→ 提交"阻塞任务"到外包执行器 →
    **return 1**（调用方释放 worker，同 route defer）。
  - **外包执行器**：跑 `px_pool_run(fpath, env_json, 1, timeout_ms, …)`
    （阻塞等子进程，与同步路径逐字同参）→ 完成：结果写 PxPend（持
    g_pxpend_mu + 屏蔽 SIG_GC_STOP）→ 出锁 `px_pool_push(fd)` 投回 g_pool。
  - **段2**（续处理 worker 重入 px_conn_worker stage==2 && kind==4）：从 pend 取
    out/out_len/exit_code/rc → **px_px_script_respond**（抽公共函数：status 解析
    __PX_RESPONSE__ / gzip / pout->respond / 访问日志）→ keep-alive 下一请求。
  - async_ok=0（H3/vhost 回退）→ 原同步路径零变化。
- **px_px_script_respond 抽取**：把现 is_px 分支响应组装段（14183-14260）抽公共
  函数，同步/异步共用，文案逐字一致（M98 px_route_respond 抽取先例）。
- 优雅关闭：g_px_stop 时挂起 .px 任务外包完成后投回 → 段2 直接收尾（同 M98 done
  路径，PxPend kind=4 关闭期防御）。

### D3 · 挂起表/GC 根
- PxPend kind 扩展注释：4 = .px 脚本 defer（段2 px_px_script_respond）。
- GC 根：req 已在 pend GC 根（stage>=1 补标现成）；外包执行期 req 存活由 pend 根
  保护；env_json/fpath/timeout 为 C 拷贝（strdup/值）非普贤对象 → 无 GC 面。
- 完成回调写 pend 出锁后 px_pool_push（锁序同 M98：g_pxpend_mu 不嵌套）。

### D4 · 验证（examples/m102）
- 语言层：协程 ctx 并发 px_exec 慢 .px（sleep 600ms×N）→ wall≈单次（外包并行）+
  worker 不饿死（对照同步串行）+ 语义对拍（stdout 捕获一致）。
- URL .px：并发 20×/slow.px（max_conn=2）wall≈1.2s（同步占 2 worker 需 ~6s+）
  + fast 30 不饿死 + keep-alive 续处理 dials=1 + 504 超时 + 优雅关闭。
- 回归：px_serve 管道直接面（m34_pool_cfg / p5_px_serve 语义 .px / m43_webapp /
  m28_route / m31_vhost / m57_s7）+ 里程碑 suites + diffcheck + vm_ab + 双自举 +
  pxi/pxi_vm 重链。

## 四、风险与预案
| 风险 | 预案 |
|---|---|
| .px 分支同步路径重构回归（m34/p5/m43/H3 共享） | 抽公共函数时同步/异步共用同一 px_px_script_respond；diffcheck + 管道回归全量把关 |
| 外包执行器与 g_pool 并发 px_pool 池状态 | g_px_pool_mu 现成保护；外包线程 = 等子进程的替代者，与多连接线程直调并发语义一致 |
| 优雅关闭期挂起 .px | kind=4 done 出锁后 g_px_stop → 段2 收尾（M98 同款防御） |
| 外包执行器线程数失控 | 上限 = 池 worker 数（默认 4，px_pool 自然限并发）+ 兜底 fork 场景设 PX_PXRUN_MAX 上限 |
| px_exec offload 与 px_pool 惰性 spawn（fork） | 外包线程 fork+exec 安全（execlp 替换）；M34 已证 spawn 可在非主线程 |
