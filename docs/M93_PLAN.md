# M93_PLAN · 帧协程 M:N（M88 C 类旗舰落地：px_spawn 用户态协程化）

> 状态：🚧 **S3 阻塞原语让出完成（路线 B，本 commit）**。S4 GC 压力验证中。
> 上游：M88_PLAN §一 C 档（用户态协程 M:N，Go runtime 简化版）+ M89_vm_design D3
> （显式帧 = 槽数组，挂起 = 帧拷贝存档，"本设计为其铺路"）+ M89_vm_prestudy §2.4
> （C 类协程在 VM 上自然可得；C 递归模型完全不可行）+ M91 默认轨切 VM + M92 精确 GC
> （退役整栈保守扫描 → 挂起协程 = 纯帧槽根，STW 与协程数解耦）。
> 基线：v0.2.0-m92（6b536dc）。性质：**L0 runtime**（GC 根面 + 执行模型层）。

## 〇、一句话

`px_spawn` 底层从 **pthread（每 spawn 一个 OS 线程）** 换成 **用户态协程 M:N**
（M 个 worker 线程 = CPU 核数 + 全局就绪队列），协程 = 一份独立 VM 显式帧状态
（PxVmState，全堆上）——挂起 = 帧存档入等待队列，恢复 = 换回继续解释循环；
阻塞原语（chan/mutex/rwlock/sleep）在 VM 层让出而非阻塞 worker 线程；
精确 GC 把全部存活协程帧槽纳入根面，STW 与协程数/栈深解耦。C 轨逃生舱产物
（fn_* C 递归）保持 pthread spawn（帧协程在其上不可行，逃生舱语义零变化）。

## 一、前置核对（全满足）

| 前置 | 状态 |
|---|---|
| M88 A 线程池止血 + B 事件驱动（服务端连接不占线程） | ✅ 已收口（v0.1.0-m88 / m88b），C 档 = 本里程碑 |
| M89 显式帧 VM（px→px 调用全在显式帧，不回 C 递归） | ✅ 默认轨；PxFrame.slots 全堆上 |
| M91 默认轨切 VM（px build 产物 = BCModule 字节码） | ✅ 主执行轨 = VM，spawn 目标 = PX_FUNC(fn=px_vm_entry) |
| M92 精确 GC（precise 根 = 全局槽 + VM 帧槽 + 登记根） | ✅ 挂起协程无 C 栈 → 帧槽即可精确标记，无需保守扫栈 |
| 帧协程 = 帧拷贝存档（M89 D3 已坐实可行性） | ✅ PxVmState 整体堆上，挂起/恢复 = 指针交接 |

## 二、D0 侦察量化（2026-09-10，dongyue，基线 6b536dc）

- **spawn 现状 = pthread**（runtime/runtime.c §spawn）：`px_spawn_name → px_spawn_ctx`
  → pthread_create（detach），每线程注册 GC 槽（g_threads 动态表，默认上限 1024/硬限
  4096）。`spawn_thread` 里 setjmp/longjmp 错误隔离（M72-S3）。函数目标：VM 产物 =
  PX_FUNC(fn=px_vm_entry, ctx=PxVMFunc*)（px_get_global 按名取）→ 可判 `fn==px_vm_entry`
  即协程化对象；C 轨逃生舱 fn_*（ctx=NULL）→ 保持 pthread。
- **VM 执行模型**（runtime/vm.c + vm.h）：`PxVmState { PxFrame* frames; nframes; cap }`
  `__thread` 线程局部（px_vm_state 懒建）；`PxFrame { PxVMFunc* f; LXValue* slots;
  pc; line; ret_dst; nargs }` 槽数组 calloc 堆上。解释循环 `switch(PXOP_*)`；px→px
  CALL 在循环内压帧（不回 C）；px→native→回调 px 经 `px_call → px_vm_entry` C 递归
  一层（很浅）。→ **协程 = 独立 PxVmState；切换只换指针，无 ucontext/C 栈切换**。
- **阻塞 native 清单**（协程让出目标，runtime.c）：
  - sleep/sleep_us：`nanosleep` 阻塞线程（3112/3938）
  - fd_wait：`poll(tmo)`（4019）
  - chan send/recv：`pthread_cond_wait`（7146/7161/7192）—— PX_CHAN 对象含
    mu/cv_send/cv_recv + 环形缓冲（容量可配）
  - mutex lock / rwlock rlock/wlock：`pthread_cond_wait`（6998/7044/7076）
  - tcp_accept/recv、udp_recv、http_request/https 同步调用、unix_connect：
    socket 阻塞（7633-7800/8095+）—— **二期**（网络 IO 协程化，服务端已事件化 M88-B）
  - select：`px_select_signal` + g_sel_cv 线程级唤醒（6967）—— chan 让出模型需对齐
- **已有 try 变体**（让出重试机制可复用）：chan_try_recv ✓（7199）、mutex try_lock ✓
  （7004）、rwlock try_rlock/try_wlock ✓（2718/2719）；**缺 chan try_send**（需补）。
- **GC 现状（M92 precise）**：precise 根 = 全局槽 + VM 帧槽（ti->vm_state →
  px_vm_gc_mark_state 跨线程）+ TLS 登记根栈 + 暂存根；保守扫栈已退役（默认轨）。
  线程暂停 = SIG_GC_STOP 信号协议 + g_threads 表。→ 协程不是线程：需**新增协程表**
  为全局根（就绪/阻塞/运行中协程的 PxVmState 帧槽全部精确标记）；worker 线程照旧
  注册 g_threads（数量 = 协程 worker ≤ CPU 核，远小于现 spawn 线程上限）。
- **服务端现状（M88-B）**：http_serve/sse 已事件化（epoll 照看空闲连接 + fserve 池
  处理活跃突发）→ 服务端不因连接数占线程。协程化收益主战场 = **spawn 并发计算 +
  原生线程模型收敛**；http handler 协程化列二期。

## 三、设计定稿（D1–D8）

### D1 · 协程 = 独立 PxVmState（帧协程，不做 C 栈切换）
- `PxCoro { int id; int state; PxVmState vm; PxVMFunc* f; LXValue* args; int nargs;
  ... 等待登记（chan/mutex/定时器链表节点） }`。state = CORO_READY/RUNNING/BLOCKED/DONE。
- 理由：px→px 全在显式帧 → 协程全部执行状态 = PxVmState.frames[]（堆上槽数组）；
  挂起/恢复 = PxVmState 指针交接，**零 ucontext/零汇编**，跨架构天然可移植
  （aarch64/armv7/riscv64 一等目标，M67）。
- 边界：协程切换只发生在解释循环的指令边界（native 桥 C 栈空或仅浅一层且无
  跨协程持有）——见 D3 让出机制保证。

### D2 · 调度器 M:N（worker 池 + 就绪队列）
- 全局：就绪队列（环形/链表 + mutex + cond，多 worker 互斥取）+ worker 线程池。
- worker 数 = `PX_CORO_WORKERS`（env，默认 min(在线 CPU, 8)，范围 [1,64]）；
  主线程在无其它活时也参与取协程（避免纯主线程场景空转）。
- worker 循环：取就绪协程 → 用其 PxVmState 进解释循环 → 协程 BLOCKED/结束 → 回取。
- spawn 分派：目标 PX_FUNC.fn == px_vm_entry → **建协程入就绪队列**（不建线程）；
  fn_* C 函数 / native → 原 pthread 路径（逃生舱，语义零变化）。同一 VM 产物内
  spawn VM 函数 100% 协程化 → 线程数从「spawn 数」收敛到「worker 数」。

### D3 · 阻塞点让出（VM 层，不阻塞 worker）—— 关键机制
- 原则：阻塞 native 在协程模式**绝不真的阻塞线程**。做法 = 解释循环级让出：
  阻塞原语调用编译为 **try 优先**序列，失败即登记等待 + 让出当前协程。
- 机制（一期，发射器 + VM 微调）：
  1. bc_emit 对「阻塞调用点」（chan send/recv、mutex lock、rwlock r/w lock、
     sleep、fd_wait 等白名单）编译为：`CALLTRY 目标(白名单) → 返回值槽 t` +
     `JMPT t 成功` + `BLOCK t（登记等待源，协程 BLOCKED → 让出）`；唤醒后从
     `CALLTRY` 重试（pc 回退），直到成功。
  2. native 层提供非阻塞变体：chan_try_recv ✓ / **chan_try_send（补）** /
     mutex try_lock ✓ / rwlock try_* ✓ / sleep→定时器登记 / fd_wait(0) 先行。
  3. 失败登记：chan 无缓冲/满 → 协程挂 chan 等待者链表；锁被占 → 挂锁等待队列；
     sleep → 全局定时器最小堆；fd 不可用 → epoll 登记（二期网络统一）。
- 语义等价性：阻塞 = 等待条件满足；try+重试在 VM 层模拟等价（登记到条件满足前
  不重试，唤醒即条件满足概率高，重试仅竞态兜底）。结果值与原 pthread 阻塞版一致。

### D4 · chan/mutex/rwlock 双轨等待队列（协程 + pthread 共存）
- PX_CHAN/PX_MUTEX/PX_RWLOCK 对象增**协程等待者链表**（与 cond_wait 并存）：
  - 协程让出等待：挂链表 + BLOCKED（不占线程、不持 pthread 锁等待）
  - pthread 等待：原 cond_wait 路径不变（逃生舱/外部线程/服务 worker 内用）
- 唤醒方（chan send/close、unlock、signal）：**同时** 唤醒一个协程（入就绪队列）
  和/或 pthread cond_signal —— 两种消费者可混用同一对象，无死锁（等待者互斥
  转移：任一唤醒路径先移出等待者再入队/解锁）。
- select_try/chan_try_recv 语义保留（VM B5 已有非阻塞轮询，协程内可空转轮询
  后置为 epoll 唤醒）。

### D5 · GC 集成（协程 = 全局精确根，无 C 栈）
- 新增全局协程表 g_coros（协程对象池，容量 = 现线程上限语义 PX_MAX_COROS 可配，
  默认 65536，env 夹取 [64, 1M]）：READY/RUNNING/BLOCKED 的协程都登记在表 →
  precise GC 根面 = 全局槽 + **全部存活协程 PxVmState 帧槽** + worker TLS 登记根
  + 暂存根。挂起协程无 C 栈 → 无需保守扫栈（M92 精确根面的直接受益）。
- worker 线程注册 g_threads（被 SIG 暂停协议覆盖，同现并发 GC）；DONE 协程
  帧槽释放（slots free）后出表。
- 协程间传参：args 拷贝入协程（同现 spawn job args 语义）；登记前 PX_KEEP 保护。

### D6 · 错误隔离（对齐 M72-S3）
- 协程内 px_error：worker 内 setjmp/longjmp 捕获（现 spawn_thread 同构）→ 打印
  现场 + 协程标 DONE + 记错误计数，worker 继续取下一协程。PX_SPAWN_ISOLATE=0
  关（exit 语义向后兼容）。

### D7 · 程序收尾 / 主线程语义
- spawn 现为 fire-and-forget（detach）。协程同语义：main 返回 → 进程退出 →
  未完成协程自然终止（与现 pthread detach 下 main exit 一致，无行为差异）。
- worker 为守护线程：主线程阻塞等 chan 时须能取协程 —— 主线程也进调度循环
  （D2：主线程参与 worker），chan 收尾（现用 sleep 轮询/select 的测试写法保持可用）。

### D8 · 二期边界（本里程碑不做，文档预埋）
- 网络 IO 协程化：http_request/tcp/udp/ws/s3/dns 同步阻塞桥 → epoll 挂起/唤醒
  （M88 B 事件内核复用）+ fd_wait 全协程化。
- http_serve/sse handler 协程化（请求突发占协程不占 fserve worker）。
- 抢占式调度（时间片）与 work-stealing（负载均衡）后置；定时器堆合并调度器循环。
- C 轨逃生舱产物精确化 + 逃生舱内 spawn 协程化（codegen 改造，golden 大迁移范畴）。

## 四、范围

- **本里程碑（M93）**：D1-D7（协程内核 + spawn VM 函数协程化 + 阻塞原语首批让出
  + GC 根集成 + 错误隔离 + 回归/压测 + 文档 + tag）。
- **不在本里程碑（D8 二期）**：网络 IO epoll 协程化、服务端 handler 协程化、抢占、
  work-stealing、逃生舱协程化。

## 五、S 级拆分（每 S = commit + 编译 + 验证绿，不混）

| S | 内容 | 验收 |
|---|---|---|
| S1 | 立项 + 设计定稿（本文档 + D0 侦察量化） | 本文档；基线 v0.2.0-m92 工作区干净 |
| S2 | 协程内核：PxCoro + g_coros 表 + worker 池 + 就绪队列 + spawn 分派（px_vm_entry → 协程）+ 协程完成回收 + DONE 清理 | 最小用例：N 个纯计算 spawn 全完成（chan 收尾断言）；spawn 不再建 pthread（线程数 = worker 数）—— 进程 /proc/self/status Threads 计数验证；逃生舱 C 产物 spawn 仍 pthread（--c 构建旧用例回归） |
| S3 | 阻塞原语让出：bc_emit CALLTRY/BLOCK 编译 + chan_try_send 补 + chan/mutex/rwlock 协程等待链表 + sleep 定时器登记 | chan 乒乓（多生产者多消费者）协程版与 pthread 版结果一致；mutex 临界区互斥正确；sleep 协程并行（总时延 ≈ 最大单 sleep 而非累加）；conservative/逃生舱零回归 |
| S4 | GC 集成 + 压测：g_coros 精确根 + worker 并发 GC + 低阈值压力 | PX_GC_THRESHOLD 低阈值下协程密集（万级 spawn + 字符串构造）无 UAF/崩溃（对比 conservative）；并发 GC 下 worker 暂停协议正确；堆回落 |
| S5 | 收口：全量回归（vm_ab/diffcheck/m89_s3d/m82/m83 + 协程专项）+ 文档 + tag | 全绿 + tag v0.2.0-m93 |

## 六、风险与预案

| 风险 | 预案 |
|---|---|
| 阻塞点漏改 → worker 被阻塞（协程空转/并发塌陷） | 白名单发射器集中管理；压测断言「并发 sleep N 个总时延 ≈ max」暴露 |
| chan/mutex 双轨等待死锁（协程与 pthread 混等） | D4 等待者互斥转移协议；专项混用压测（协程消费者 + pthread 生产者） |
| 协程帧槽 GC 漏标（BLOCKED 存档不在 C 栈）→ UAF | g_coros 全局根在 precise 标记统一遍历；低阈值 + 万协程压力暴露 |
| 主线程参与调度与现有 sleep/轮询收尾测试冲突 | 主线程进 worker 循环仅在就绪队列非空时；空队列回 sleep 轮询（现语义） |
| C 轨逃生舱 spawn 误入协程（fn_* 无 VM 状态） | 分派判 fn==px_vm_entry 硬门；逃生舱旧用例回归覆盖 |
| 回归面大 | VM 轨 spawn 默认改协程后，vm_ab 38 例 + m89_s3d + m82/m83 全量回归把关；PX_CORO_WORKERS=0 或 env 强制全 pthread 逃生阀 |

## 七、验收（S5 汇总）

- 语义：spawn/chan/mutex/sleep 行为与 pthread 版对拍一致（专项脚本结果逐字节比对）。
- 并发：万级 spawn 稳定完成、进程线程数 = worker 数 + 常数（不与 spawn 数线性）；
  并发 sleep N 总时延 ≈ max(sleep)。
- GC：precise + 低阈值 + 万协程压力零 UAF/零崩溃、堆回落。
- 回归：vm_ab 38 PASS 0 GAP 0 FAIL · diffcheck --all ✅ · m89_s3d 9 PASS ·
  m82 8 PASS · m83_s6 全 PASS · 双自举证明 rc=0 · tag v0.2.0-m93。

## 八、落地记录

### S2 协程内核（本 commit）—— 完成
- **代码**：runtime/coro.c（新，协程内核）+ vm.c/vm.h（px_vm_state_init/px_vm_bind/
  px_vm_unbind）+ runtime.c（spawn 分派判 fn==px_vm_entry → px_coro_spawn weak；
  GC 标记期 px_coro_gc_mark_roots weak 补标协程表根面；导出 px_spawn_isolate_begin/
  end + px_gc_block_stop_sig/unblock_stop_sig 供 worker）+ tools/px（rt_src_files 增
  coro.c）+ runtime.h（signal.h include + 导出声明）。
- **机制**：worker 池 = PX_CORO_WORKERS（默认 min(在线CPU,8)，范围 [1,64]）常驻；
  就绪队列 FIFO（mutex+cond）；协程 = 独立 PxVmState（全堆帧栈）。worker 空闲
  cond_wait 不注册 GC（STW 无空闲放大，对齐 M90 F3-fix 模式）；取到协程才
  px_gc_thread_enter → px_vm_bind(&coro.vm) → px_spawn_isolate_begin 隔离执行
  px_vm_entry → 解绑/注销 → 摘表回收。协程表 g_all 为 GC 根（args 副本 + 帧槽）。
- **并发安全**：g_coro_mu 临界区全部屏蔽 SIG_GC_STOP（持锁不被 STW 打断 → GC
  executor 标记拿锁不与其死锁）；precise/conservative 标记路径均补标协程表。
- **验证**（examples/m93_s2/verify.sh PASS=6 FAIL=0）：
  1) coro_print 64 纯算协程全部执行+回收（PX_CORO_DIAG done=64，结果行 R0..63）
  2) coro_many 1000 全部完成（done=1000）
  3) thr_count spawn 128 → 线程 9（8 worker+1 主；pthread 时代 = 129）线程数收敛
  4) coro_gc 500 × list 高频分配 + PX_GC_THRESHOLD=4000 多轮并发 GC STW → 500/500
     零崩溃零 UAF（协程表 GC 根面实证）
  5) C 轨逃生舱 --c（fn_*）spawn 128 → 线程 129（pthread 语义零变化）
- **回归门**：m89_s3d verify 9 PASS（含 vm_spawn_smoke 前置门——spawn 冒烟走新
  协程路径）+ vm_ab v2 PASS 31 / GAP 7（=已知 bc_emit native 缺口）/ FAIL 0。
- **工具链备注**：仓库 tools/px 与安装版 /usr/bin/px 并存——本里程碑起验证一律用
  仓库 `./tools/px`（安装版为旧打包，pxc_vm 行为/默认轨不一致，勿混用）。

### S3 阻塞原语让出（路线 B：解释循环 C 层自动包装）—— 完成（本 commit）
- **技术决策**：放弃 PLAN D3 原设计的发射器正规军（bc_emit CALLTRY/BLOCK 编译，
  需全链自举重建 + 双 golden 迁移），采用**路线 B —— 让出逻辑集中在 vm.c 解释
  循环 C 层**（M89「显式帧 VM = 单一执行引擎」哲学：阻塞点让出本是解释器内核职责，
  非发射器知识）。白名单由 runtime 导出识别，不动编译器/bc_emit，无自举重建。
- **代码**：
  - runtime.h：PX_CHAN/PX_MUTEX/PX_RWLOCK 对象增协程等待者链表（cw_send/cw_recv/
    cw/cw_w/cw_r，struct PxCoro* 前向）；px_chan_try_send 补；阻塞 native 识别
    px_native_blocking_kind（sleep/sleep_us 函数指针白名单）；协程登记/唤醒桥
    （px_coro_*_wait 返回 0/1/2 = 非协程 ctx 走原阻塞 / 已登记让出 / 条件满足重试）。
  - runtime.c：chan send/recv/try_recv/try_send/close 各成功/关闭点 cond_signal +
    take 摘链协程等待者（解锁后 wake）；mutex unlock、rwlock runlock/wunlock 同；
    px_chan_send 无缓冲接收者判据含 cw_recv。
  - vm.c/vm.h：解释循环抽 vm_run_loop(st, base, yield_ok, &ret)（返回 1=让出 0=完成）；
    px_vm_run_func = 不可让出（主线程/嵌套 native 回调 → 原 pthread 语义零变化）；
    px_vm_run_coro/px_vm_resume = 可让出（返回码 1/0，**不经协程对象字段传让出标志
    —— 见下 bug 修复**）。CALL 预检 sleep/sleep_us（定时器让出，不回退 pc 预写 null）；
    CALLM 预检 chan send/recv、mutex lock/with、rwlock rlock/wlock/with_read/with_write
    （vm_coro_method：try 变体失败 → 登记让出；成功 → 写 dst）。with 系列展开 = 压 fn
    帧 + PxFrame.unlock_kind/unlock_obj（帧弹公共路径自动解锁；fn 中途让出锁随帧
    保留，恢复后 fn RET 弹帧解锁；px_vm_gc_mark_state 补标 unlock_obj 防 fn 内 GC
    误回收锁对象；异常 longjmp 泄漏锁 = 与现 px_method with 同语义）。
  - coro.c：PxCoro 增 w_next（对象等待链）/s_next（sleep 定时链）/wake_pending/
    first；worker 循环改「跑到让出或完成」；px_coro_wake/px_coro_take_waiter；
    登记 wait API（chan/mutex/rwlock/sleep，对象锁内复查+挂链原子）；sleep 定时器
    （单后台 timer 线程 + g_sleepers 按到期升序链表 + cond_timedwait 绝对时间）；
    px_coro_gc_mark_roots 覆盖 BLOCKED 协程帧槽（精确根）。
- **两处并发 bug 修复（压力暴露，非路线问题）**：
  1. **让出窗口双执行/UAF**：登记（对象锁内 append cw）到 worker 真正让出（运行函数
     返回）之间有窗口，唤醒方 take+wake 会把仍 RUNNING 的协程立即入队 → 第二个
     worker 并行 resume（帧撕裂）；且让出 worker 读 c->vm.suspended 时协程可能已被
     唤醒-恢复-回收（use-after-free，ASAN 实锤 coro.c:206 read-after-free）。修复 =
     让出标志改经运行函数**返回码**传递（不读协程字段）+ state 状态机收敛在
     g_coro_mu：登记不置 BLOCKED（保持 RUNNING）；wake 见 RUNNING → 置 wake_pending
     延迟调度；worker 让出收敛时置 BLOCKED + 查 pending（置则自行入队）。W=1/2 稳定、
     W=8 必崩 → 修复后 12+ 轮 ASAN 零报 + 8 轮 release 零崩。
  2. **timer 线程漏唤醒**：摘到到期协程后 g_sleepers 空 → 走 `else cond_wait` 永久
     阻塞（持 g_timer_mu，batch 永不 wake）。修复 = batch 非空时不进入任何 wait，
     直接 unlock + wake，循环顶重摘。
- **验证**（examples/m93_s3/verify.sh PASS=6 FAIL=0）：
  1) coro_mutex 4×1000 临界区计数 4000（lock 让出密集）
  2) coro_chan 8 对 × cap=1 乒乓 300 轮累计和 8758800（send 满/recv 空大量让出）
  3) coro_sleep_par 40 协程并发 sleep 40..120ms → wall≈121ms（≈max，串行 pthread 会 3.2s）
  4) coro_with mutex.with ×4×200 + rwlock with_write ×2×200 + with_read（帧弹解锁）
  5) coro_gc_block 60 协程乒乓 × 对象分配 × PX_GC_THRESHOLD=4000（BLOCKED 帧 = 精确根）
  6) C 轨逃生舱 --c coro_chan（pthread 语义对拍同 8758800）
- **回归门**：m93_s2 verify 6 PASS · m89_s3d 9 PASS · vm_ab 31 PASS/7 GAP(已知)/0 FAIL
  · diffcheck --all 全 ✅ · 万级 spawn（10000 协程纯算）3.26s 完成。
- **工具链**：调试用 `-g`/`-fsanitize=address` 为临时改动 tools/px（已还原 + 清
  rtcache 重编 release）；libasan 通过 yum 安装（RHEL9 gcc11 runtime 缺失）。

### S4 GC 集成 + 万级压力 —— 完成（与 S3 同 commit 验证）
> g_coros 精确根（S2）+ BLOCKED 让出协程帧槽标记（S3 px_coro_gc_mark_roots 遍历
> g_all 全部存活协程）在并发 GC（STW 暂停 worker）下实测：
> - coro_gc_block.px：60 协程 chan 乒乓 × list/str 高频分配 × PX_GC_THRESHOLD=4000
>   （多轮并发 GC STW × BLOCKED 帧精确根）→ 零崩/UAF，值正确。
> - coro_5k_gc.px（临时）：5000 协程 × 40 轮分配 + 每 11 轮 sleep_us(150) 让出 ×
>   PX_GC_THRESHOLD=8000 → 37.3s 全完成，total=2,400,000 精确（无丢失/重复）。
> - 万级纯算 spawn（10000 协程）3.26s 全完成。
> 验证了让出协程（无 C 栈）帧槽根面在并发 GC 下稳定（漏标 = UAF 会被低阈值压力
> 暴露；未发生）。

### S5 收口（全量回归 + 重链 + 文档 + tag）—— 进行中
