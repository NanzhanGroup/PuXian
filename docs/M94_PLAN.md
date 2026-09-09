# M94_PLAN · D8 二期-A：协程调度完备性（抢占 + 定时器并入调度循环 + 逃生舱边界决策）

> 状态：🚧 S1 立项（本 commit）。S2 抢占 / S3 定时器合并 / S4 收口。
> 上游：M93_PLAN §D8 二期边界（四条预埋：①网络 IO 协程化 ②http_serve/sse handler
>   协程化 ③抢占式调度与 work-stealing；定时器堆合并调度器循环 ④C 轨逃生舱精确化 +
>   逃生舱内 spawn 协程化）。本里程碑 = D8 第 **③④** 条；①② 编排 M95/M96（见 §五）。
> 基线：v0.2.0-m93（92149c9）。性质：**L0 runtime**（调度器 + 让出协议层）。

## 〇、一句话

M93 协程内核无抢占（纯计算协程独占 worker 至阻塞/结束）→ M94 补**抢占式时间片**
（指令边界让出、worker 直接回队尾），把 sleep **定时器摘取并入 worker 调度循环**
（退役独立 timer 线程），并对 **D8-④ 逃生舱协程化做边界决策收口**（保持 pthread +
逃生舱内 spawn VM 函数已协程化实证）。为 M95 服务端 handler 协程化铺好公平调度地基。

## 一、D0 侦察量化（2026-09-10，dongyue，基线 92149c9）

- **抢占缺口（M93 遗留，真实缺陷）**：worker 循环（coro.c coro_worker）取队首 →
  `px_vm_run_coro/px_vm_resume` 跑到「阻塞让出（返回 1）或完成（返回 0）」。
  纯计算不阻塞协程（如 `while true: i+=1`）**永不返回** → 独占一个 worker 直至
  进程退出。M 个死循环协程可饿死整个 worker 池（其它 spawn 永不执行）。m93 各套件
  全是有限任务，未暴露。
- **让出协议现状**：`vm_run_loop(st, base, yield_ok, out)` 返回 0=完成 / 1=阻塞让出
  （协程已登记 chan/mutex/rwlock/sleep 等待；worker 置 BLOCKED，唤醒方 take+wake
  入队；让出窗口竞态由 wake_pending 状态机收敛）。`st->suspended` 仅诊断。
  让出点全在解释循环指令边界（CALL 预检 / CALLM vm_coro_method 内 try+wait）。
- **抢占安全点分析**：抢占只能在解释循环指令边界检查（不能在 native C 内部 / 让出
  登记窗口内）。`vm_run_loop` 主循环顶部 = 天然安全点（每指令边界）。帧栈完整保留、
  pc 已指向下一条 → 抢占让出无需回退 pc，resume 直接续跑。与阻塞让出（已登记等待）
  的关键差异：抢占让出**未登记任何等待** → worker 可直接把协程放回就绪队尾，无
  lost-wakeup / 双执行竞态（无人会 wake 未登记协程）。
- **timer 线程现状**：独立 `coro_timer_thread`（g_timer_mu/g_timer_cond + g_sleepers
  升序链表）到点摘 sleeper → px_coro_wake 入就绪队列。锁序 g_timer_mu→g_coro_mu
  （wake 时先放 timer 锁再拿 coro 锁）。**合并障碍 = 双锁序**：worker 持 g_coro_mu
  cond_wait 若再拿 g_timer_mu 读最近到期 → 反向锁序死锁。
- **逃生舱现状**：--c 产物 = fn_* C 函数（ctx=NULL，无 VM 状态），px_spawn_name 判
  目标 PX_FUNC.fn==px_vm_entry → px_coro_spawn（M93-S2 weak 分派）。逃生舱产物内
  全部函数为 fn_* → spawn 目标非 px_vm_entry → pthread 路径。**逃生舱内 spawn 的
  「VM 函数目标」早已协程化**（分派按目标函数类型，与调用者线程无关）。

## 二、设计定稿

### D1 · 抢占式调度（时间片，worker 回队尾）
- PxCoro 增 `long long run_begin_us`：worker resume 协程前记 `coro_now_us()`。
- 解释循环预算检查：`vm_run_loop` 主循环顶部插指令计数（每 4096 条）→ weak 调
  `px_coro_preempt_check()`（coro.c 实现；无 coro.o 链时 no-op——逃生舱产物无 vm.o/
  coro.o 不受影响）。耗时超 `PX_CORO_QUANTUM_US`（env，默认 5000us=5ms，夹取
  [200, 1000000]）→ 置 st->suspended=1 并 **return 2**（新返回码：抢占让出）。
- worker 处理返回码 2：持 g_coro_mu 把协程 state=READY 放回就绪**队尾**（FIFO 公平
  轮转）+ cond_signal；不置 BLOCKED、不触碰等待表、无 wake_pending 竞态（未登记即
  无唤醒源）。恢复走 px_vm_resume（pc 已在下一条，续跑）。
- 抢占点不变量：仅指令边界（循环顶部 tick），绝不在 CALL→native 内部 / 让出登记
  临界区。解释循环纯指令段无跨协程持锁 → 抢占安全。
- 默认开启（时间片是公平基座；M95 handler 协程化依赖它防单 handler 饿死池）。
  `PX_CORO_QUANTUM_US=0` 关闭（回归 M93 无抢占语义，逃生阀）。

### D2 · 定时器并入调度器循环（退役独立 timer 线程）
- **单锁收敛**：g_sleepers 升序链表移入 g_coro_mu 保护（登记/摘取/就绪队列同锁 →
  无锁序问题）；删除 g_timer_mu/g_timer_cond/coro_timer_thread/coro_ensure_timer。
- worker 取协程前置摘取：lock g_coro_mu → 摘 g_sleepers 到期者（due_us<=now）逐个
  入 g_rq 队尾 → 若 g_rq 空：有 sleeper → cond_timedwait(g_coro_cond, 最近 due)（
  到期自己醒 → 循环顶再摘）；无 sleeper → cond_wait。取队首 → RUNNING → unlock。
- px_coro_sleep_us：lock g_coro_mu → 升序插入 → unlock；若插入成为链表头（新的最
  近到期）→ cond_signal（唤醒一个 timedwait 中的 worker 重算 deadline——其它
  worker 仍等原 due，早醒者统一重摘，无漏醒）。
- 唤醒路径不变：px_coro_wake / 入队均 signal g_coro_cond。
- 收益：线程数 -1（收敛）；摘取由任一空闲 worker 承担（比专职线程更自然）；并发
  sleep 精度保持（cond_timedwait 绝对时间）。

### D3 · D8-④ 逃生舱边界决策（文档收口，不改 codegen）
- **决策**：逃生舱（--c，fn_* C 递归产物）**保持 pthread spawn，不做协程化**。
  理由：① fn_* 无 VM 显式帧状态，帧协程在其上不可行（M89 prestudy §2.4 已定论，
  M93 D1 边界）；② M91 起默认轨=VM，逃生舱仅编译自举/纯计算热点逃生用，非执行
  主战场；③ codegen 改造 + golden 大迁移成本极高，收益随默认轨 VM 化递减。
- **已覆盖项**：逃生舱产物内若持 VM 函数引用并 spawn（跨轨混合）→ M93-S2 分派按
  目标函数类型（fn==px_vm_entry→协程）已生效，与调用者轨无关。实证：m93_s2
  thr_count 逃生舱 --c 场景 spawn fn_* → 129 线程（pthread，预期），spawn VM 目标
  → 协程（9 线程）。
- 逃生舱**整体 VM 化/精确化**（fn_* 退役）→ 从 D8 待办降级为「codegen golden 大
  迁移立项候选」，不进本期排期。

## 三、范围

- **本里程碑（M94）**：D1 抢占 + D2 定时器合并 + D3 决策文档 + 回归收口 + tag。
- **不在本里程碑**：work-stealing（多队列负载均衡，后置）；M95 服务端 handler
  协程化（D8-②）；M96 客户端网络 IO 协程化（D8-①）；逃生舱整体 VM 化（D3 决策后
  移出排期）。

## 四、S 级拆分（每 S = commit + 编译 + 验证绿，不混）

| S | 内容 | 验收 |
|---|---|---|
| S1 | 立项 + 设计定稿（本文档） | 基线工作区干净；commit |
| S2 | D1 抢占：PxCoro.run_begin_us + vm_run_loop tick/return2 + coro_worker 分支 + PX_CORO_QUANTUM_US env | m94_s2/verify.sh：死循环协程 vs 有限协程公平性（有限者完成、进程退出）；coro_many 1000 全完成不回归；quantum=0 关闭逃生阀 |
| S3 | D2 定时器合并：g_sleepers 移 g_coro_mu + worker 置摘 + timedwait 协议 + 删 timer 线程 | m94_s3/verify.sh：coro_sleep_par 并发 sleep 总时延≈max（m93_s3 套件）；线程数收敛（无 timer 线程）；m94_s2 复绿 |
| S4 | 收口：全量回归（vm_ab / m89_s3d / m82 / m83 / m93_s2+s3 / diffcheck）+ 文档 + tag | 全绿 + tag v0.2.0-m94 |

## 五、后续编排（D8 二期 ①②，另立里程碑）

- **M95**（D8-② 服务端 handler 协程化）：http_serve/sse/px_serve 活跃请求处理从
  fserve pthread 池 → 协程（handler 占协程不占 fserve worker；突发请求占协程、空闲
  连接仍事件循环照看；handler 内 chan/网络阻塞让出不占线程）。前置：M94 抢占（防单
  handler 死循环饿死池）+ M93 阻塞原语让出。
- **M96**（D8-① 客户端网络 IO 协程化）：http_request/tcp/udp/ws/s3/dns/fd_wait 同步
  阻塞桥 → 协程上下文感知（px_coro_active → 非阻塞 + epoll/事件登记让出 + 唤醒重
  试）；复用 M88-B 事件内核 + M93 让出协议。前置：M94/M95 调度成熟。

## 六、风险与预案

| 风险 | 预案 |
|---|---|
| 抢占误伤确定性测试（协程执行顺序假设） | 协程并发本无总序；回归以结果断言为主；quantum=0 逃生阀兜底 |
| tick 检查开销（每 4096 条一次 weak 调用） | 单次 ~ns 级；quantum 默认 5ms 下每协程每时间片仅数十次检查 |
| 抢占与阻塞让出窗口竞态（漏登记/双入队） | 抢占让出未登记 = 无唤醒源 = 无竞态（设计不变量）；ASAN + 死循环压测佐证 |
| 定时器合并后锁序/漏醒 | 单锁收敛（g_coro_mu）消除锁序；cond_timedwait 绝对时间 + 醒来重摘；并发 sleep 专项 |
| 回归面大（L0 改动） | m93_s2/s3 + vm_ab + m89_s3d + m82/m83 + diffcheck 全量把关 |
