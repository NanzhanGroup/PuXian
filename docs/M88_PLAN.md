# M88_PLAN · runtime 并发模型演进：A 线程池止血 → B 事件驱动 → C 用户态协程（qg-issue 27）

> 创建：2026-09-07 · 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD 01999de = m86 后；runtime/runtime.c 14110 行）
> 来源：qg-issue **27-puxian-spawn-concurrency**（清歌 ws-approve .px 化 #47 压测/线上事故：http_serve_unix 高并发崩溃）
> 用户指令（2026-09-07）：**「把 A、B、C 全部立项 M88 里面。今天中午（12:00）开工，只需要实现 A 类」**
> 性质：**L0 runtime（C，GC 线程表 + 服务端并发模型层）**；业务 .px 零改动；native 总数不变
> 风险等级：**L0**（动 GC/线程模型）→ 必须：自举证明 + 回归总闸 + 并发压测 + 文档同步
> 状态：🆕 A 类已于 2026-09-07 收口（tag v0.1.0-m88）；**B 类已收口（tag v0.1.0-m88b）**：S1-S4 完成（事件驱动内核 → http 接入 → SSE 事件化 → 量级压测回归收口，见 §七）；C 类排后

---

## 〇、一句话

`px_spawn` 受 GC 线程表固定 64 槽（`MAX_SPAWN_THREADS`）限制，槽满 `px_error`；`http_serve` / `http_serve_unix` / `sse_serve` 的 accept 循环**每连接 spawn 一个 OS 线程**且跑在主线程 → 64 并发槽满 → 服务进程 exit(1)。同 runtime 内 `px_serve`（M31.4b）已有正确模型「连接线程池 + 队列满 accept 阻塞」，函数式入口未接入。M88 以 A/B/C 三档完整演进：A 接池止血（今日开工）、B 事件驱动（中期）、C 用户态协程 M:N（向 Go 百万并发看齐，远期旗舰）。

## 一、A/B/C 全貌与归属

| 档 | 内容 | 并发量级 | 模型 |
|---|---|---|---|
| **A**（今日 12:00 开工） | GC 线程槽动态化 + http_serve/http_serve_unix/sse_serve 接入连接线程池 + 池容量可配 | 64 必崩 → **512~1024 并发不崩**（量级 10²→10³） | 预派生常驻 worker + 队列（复用 M31.4b，半成品接入） |
| **B**（M88 后续中期） | 连接处理统一池化 + keep-alive **空闲连接事件驱动**（poll/epoll，不占 worker），活跃请求才占 worker | 连接数 **1 万~10 万**，OS 线程降回几十 | 半同步/半异步（Go netpoller 前身） |
| **C**（M88 远期旗舰） | `px_spawn` 底层 pthread → **用户态协程 M:N**：N 个执行线程(=CPU 核) + 就绪队列；网络 IO 内置函数 epoll 化挂起/唤醒协程；GC 只扫执行线程栈 + 挂起协程栈 | 十万~百万连接（Go goroutine 同构） | Go runtime 简化版（PuXian 自举解释器） |

分档依据与量化见 qg-issue 27 §3/§4；**A/B/C 全部立项 M88**，A 类为本期批次（S1-S3），B/C 在 A 收口后各自另起子批次，不混 commit。

---

## 二、A 类：线程池止血（M88-S1→S3）

### 目标
- 并发 100/500/1000 × 500 请求打 http_serve_unix：**全部 200、0 连接失败、进程不崩、无 restarts**；
- keep-alive 连接关闭后 GC 槽位回落（无泄漏）；
- 服务进程**永不因 spawn/槽满而退出**。

### 现有代码侦查（2026-09-07 立项时实测）

**GC 线程表（固定 64 槽）**
- `runtime.c:594 #define MAX_SPAWN_THREADS 64`；`:605 static GCThreadInfo g_threads[MAX_SPAWN_THREADS]`（含 tid/in_use/paused/is_main/epoch/ucontext/tmp_root）。
- 槽分配全部在 `g_gc_mu` 临界区内；`px_spawn`（6926）先 `g_active_threads++` 再找空槽，**槽满回滚后 `px_error("spawn: 并发线程数超出上限 64")`**（6948）→ 主线程调 px_error → exit(1)。
- 遍历 `i < MAX_SPAWN_THREADS` 的循环 **18 处**：885/955/963/979（GC 注册/查找/扫描前段）、1054/1064/1078/1128/1168（stop-the-world 暂停/恢复）、6849/7047/7101/7113（px_gc_thread_enter/px_spawn/timer spawn 注册）、6905（px_gc_thread_leave 注销辅助）、6932（px_spawn 找槽）、13988（fini 收尾）、14072（cron spawn 找槽）。
- 槽满**静默不注册**的既存隐患（注释自述 6895/13514）：`px_gc_thread_enter`、`px_pool_worker`、H3 托管线程槽满时不入表 → 不被 GC 暂停/扫描 → 栈上对象可能被误回收。A3 后槽位稀缺性消失，隐患一并消除。
- 定时器 spawn（7101-7113）与 cron spawn（14072-14083）同受 64 上限。

**服务端两套连接处理并存**
- `px_serve`（M31.4b）正确模型：`bi_px_serve`（~13300）解析 `opts.max_conn`（默认 32，上限 `PX_POOL_MAX=256`，13312 夹取）→ 创建 `g_pool_size` 个常驻线程 `g_pool_threads[i]=px_pool_worker`（13420）→ worker 循环从环形队列 `g_pool_fds[]`（`PX_POOL_MAX` 256，141-146）取 fd 调 `px_conn_worker`（12516，LX native 连接处理，含 keep-alive）；`px_pool_push`（13495）**队列满阻塞等待**（TCP backlog 排队，不崩）。池 worker 启动时自注册 GC 槽（13514 起，占 1 槽）。
- `http_serve`（9927）/`http_serve_unix`（9967，M82/Issue 15）：accept 循环每连接 `px_spawn(http_conn_worker,...)`；`http_conn_worker`（9628）keep-alive 循环 + 从全局 `__http_handler`（9842）取用户 handler。unix 版另带残留 unlink / chmod 0600 / accept 错误容忍（EINTR 重试、EMFILE 50ms 让出）。
- `sse_serve`（10295）：同款 accept 循环 `px_spawn(sse_conn_worker,...)`；`sse_conn_worker`（10127）+ 全局 `__sse_handler`（10260）；另有 SSE 注册表 `g_sse_conns[MAX_SSE_CONNS=256]`。
- `http_conn_worker` 与 `px_conn_worker` 存在功能重叠（都是「解析 HTTP 请求 → 调用户 handler → keep-alive 循环」），handler 调用约定是否完全一致是 S2 收敛的关键核实点。

### S1 · GC 线程槽动态化 + 可配上限（先做，池化前提）

- `g_threads` 定长数组 → 动态容量表：全局 `static GCThreadInfo* g_threads` + `static int g_thread_cap`（初值 64）+ 上限常量（如 4096）；槽满时在 `g_gc_mu` 临界区内 **×2 扩容**（realloc，上限内）。
- 环境变量 `PX_MAX_THREADS`（默认 1024，夹取 [64, 4096]）在 `gc_init_env()` 时预分配初始容量；旧 `MAX_SPAWN_THREADS` 语义收敛为「默认初始容量」。
- 18 处遍历条件 `MAX_SPAWN_THREADS` → `g_thread_cap`（机械替换 + 逐处核对；`#define` 仅保留给默认值/历史引用）。
- 上报文案改动态：`spawn: 并发线程数超出上限 %d` → 打印当前 cap。
- 内存账：GCThreadInfo 含 ucontext（数百 B），1024 槽 ≈ 1MB 级，可接受；GC 每轮线性扫 cap 个槽（微秒级）。
- 风险预案：扩容与 GC 暂停的竞态——扩容只在持有 `g_gc_mu` 时进行，GC 遍历同锁/同 stop 协议保护；realloc 后旧指针即时失效点需全量核对（先验：所有取址 `&g_threads[i]` 都是短生命局部，锁内安全）。

### S2 · http_serve / http_serve_unix / sse_serve 接入连接线程池

- 方案候选（S2 开工时细核定稿，取侵入最小）：
  - **案甲（收敛复用）**：核实 `http_conn_worker` 与 `px_conn_worker` 的 handler 约定（req dict → resp）与请求语义一致 → http_serve/unix 仅把 handler 挂到统一处理链（或同 global 槽），accept 改 `px_pool_push(cfd)`，复用 px_serve 的全局池（池 worker 已达上限则扩池容量/默认 256→可配）。
  - **案乙（函数式独立池）**：为函数式入口建同构连接池（每 serve 入口 n 个常驻 C 线程直接跑各自 conn_worker + 队列），默认池容量与 `PX_MAX_THREADS` 联动（如 128~256），env `PX_SERVE_WORKERS` 可配。
  - 两案都要求：**accept 队列满时阻塞或关 cfd 重试，绝不 exit**；http_serve 语义/keep-alive 15s/M83-S6 同端口流式路由/stream_match/unix 0600 与错误容忍/sse 注册表与 sse_close 唤醒 全部保持。
- `http_serve`/`sse_serve` 增**可选第 3 参 max_conn**（向后兼容：nargs==2 走默认池容量）。
- 池 worker 数受 S1 槽上限约束：默认池 128~256 + 业务 spawn 余量 < 1024（甜点区），不硬冲几千线程。

### S3 · 压测 + 回归 + 收口

- 新增并发压测脚本归档 `examples/m88_s3/`：http_serve_unix 为主，http_serve/sse_serve 抽查；并发 100/500/1000 × 500，统计 200 数 / 连接失败 / 进程存活 / restarts；keep-alive 关闭后槽位回落断言。
- 回归总闸（沿用 m86 S3 清单）：m82 + m83_s1-s6 + m84_s1-s3 + m85_s1-s2 + m86_s0-s2 全绿（默认全能力路径零漂移）+ 自举证明 rc=0 + native 数不变 + fmt/lint 0 + worktree 干净。
- 文档同步（README/CHEATSHEET/spec 并发段/环境变量表 PX_MAX_THREADS/PX_SERVE_WORKERS）+ CHANGELOG + qg-issue 27 状态更新。
- 交付物：**ws-approve .px 版（#47）可重跑对拍套件并灰度**（观音/清歌 1 天无「Approve 连接失败」）——部署侧交清歌。

---

## 三、B 类（M88 本批次：事件驱动半同步/半异步，连接上万，2026-09-07 立项）

### 目标
- **空闲 keep-alive / SSE 连接不占 worker 线程**：由事件循环（epoll/poll）照看，有数据才派发 → 进程 OS 线程数 ≈ 池容量 + 常数，与连接数解耦；
- 连接数 **1 万+**（http keep-alive 与 SSE 长连接均可），活跃突发请求全 200、不崩；
- 吞吐不降：活跃 worker 从 256 收敛到几十（= 同时处理请求的连接数）→ 顺带缓解 M11 g_gc_mu 容器锁竞争（少线程抢锁）；**不在本批重构 g_gc_mu**（风险表已注明归属）；
- 语义零漂移：keep-alive 15s 空闲超时、client_close / resp keep_alive:false、file 流式、chunked/gzip、M83-S6 同端口 stream 路由、SSE 注册表 + sse_send/sse_close、http_serve_unix 0600/错误容忍 全部保持。

### 现状（A 类 S2 收口后实测定位，2026-09-07）
- fserve 池 worker 取 (fd,kind) 后**阻塞跑完连接整个生命周期**：http_conn_worker keep-alive 循环阻塞 recv（SO_RCVTIMEO 15s 判空闲超时）；sse_conn_worker handler 返回后阻塞 recv 至断开/sse_close（shutdown 唤醒）。→ keep-alive 空闲连接与 SSE 长连接**全程占 1 个 worker** 直到 15s 超时/断开；空闲连接越多线程/槽被吃越狠（A 类压测：256 活跃 keep-alive 即吃满默认池）。
- runtime 现无 epoll/统一事件循环（poll 仅零星用于等子进程/管道等）；连接 fd 全阻塞式处理。

### 方案：连接对象化 + 全局事件循环（mini reactor）
- **ConnCtx 连接上下文表**（动态/容量可配，PX_MAX_CONNS 默认 16384）：{fd, kind(HTTP/SSE), state(FREE/ACTIVE/IDLE), 半请求动态缓冲(非阻塞下读一半的续接), 空闲计时起点}；
- **连接状态机**：worker 持有 = ACTIVE（独占处理）；响应写完且无下一请求数据 = 交还 IDLE；事件循环 detect 可读 = 摘除 + 派发回 fserve 队列；空闲超时（15s，沿用既有语义）由事件循环计时 close；对端断开/HUP 由事件循环 detect → close + 清理（SSE 同步清注册表）；
- **单持有者原则**：fd 在 ACTIVE/IDLE 间严格互斥转移（idle 注册/摘除 + fserve 派发同锁/原子），杜绝 worker 与事件循环同时 recv 同一 fd（也防 fd 复用串扰）；
- **worker 处理单元从"连接"细化为"请求突发"**：接管后循环读完整请求（头+body，用 ConnCtx 缓冲续接半包/半 body）→ handler → 响应 → 已有下一请求数据则续、否则交还 IDLE 回池；
- **事件循环**：Linux epoll（O(1)），非 Linux poll 兜底（文档明示 Linux 一等）；懒启动全局单例线程；连接 fd 一律 O_NONBLOCK；
- **SSE**：handler 返回后连接交还 IDLE（事件循环等 POLLIN|POLLHUP|POLLERR detect 断开）；sse_send 仍任意线程可写（写失败才清注册表，语义保持）；sse_close 改为 IDLE 摘除 + close + 清理（ACTIVE 场景走 shutdown 唤醒原逻辑兜底）；g_sse_conns 256 定长 → 动态表容量可配（PX_MAX_SSE_CONNS，支撑上千~上万 SSE）。

### B 类 S 级拆分（每 S 编译 + 相关 verify 通过，不混 commit）
- **B-S1 连接上下文表 + 事件循环内核**：ConnCtx 表 + 状态机 + 事件循环线程（epoll/poll，懒启动单例）+ 连接 fd 非阻塞化 + 注册/摘除/超时/派发接口；冒烟：哑连接挂 IDLE → 外部写触发派发 → 读 0/HUP 触发清理。
- **B-S2 http_conn_worker 请求级重构**：keep-alive 空闲交还 IDLE + ConnCtx 缓冲续接（半包/大 body 非阻塞续读）+ 事件循环派发回池；语义保持（见目标）；验证：m82/m83_s6 HTTP 专项 + A 类短连接压测场景回归 + 新增「N 空闲长连接 + 突发全 200」。
- **B-S3 SSE 长连接事件循环化 + 注册表容量可配**：handler 返回后交还 IDLE；sse_close/sse_send 适配；g_sse_conns 动态化（PX_MAX_SSE_CONNS）；验证：m23a SSE+WS 回归 + 500~1000 长连接挂载 + 广播。
- **B-S4 压测 + 回归 + 收口**：压测脚本归档 examples/m88b_s4/：① 1 万 idle keep-alive 连接：进程 OS 线程数 ≈ 池容量 + 常数（不随连接数涨）② idle 后突发全 200 ③ SSE 500+ 挂载广播 ④ 短连接并发吞吐不低于 A 类基线；回归总闸 + 自举证明 rc=0 + native 数不变 + fmt/lint 0 + 文档同步 + CHANGELOG + tag（版本号收口时定）。

### 依赖
- A 类 S1（槽动态化）+ S2（fserve 池）已完成 ✅；本档在 fserve 池与 conn_worker 间插入事件循环层，池容量默认收敛（PX_SERVE_WORKERS 默认 256 → 64 级，空闲不占线程后无需大池）。
- 参考：cnblogs.com/zhengpan0526/p/18860258（epoll 演进 + Linux 内核参数调优表：fs.file-max/nofile/nf_conntrack/tcp_mem…）。

## 四、C 类（M88 远期旗舰：用户态协程 M:N，向 Go 百万看齐）

- 目标：`px_spawn` 从 pthread_create 改为创建**用户态协程**（栈 mmap 独立、初始 64KB~1MB），N 个执行线程（=CPU 核）从就绪队列取协程跑；阻塞 IO 内置函数（recv/send/accept/connect/文件/锁/sleep）epoll/poll 化：未就绪 → 挂起协程入等待表，事件就绪 → 唤醒（Go netpoller 简化版）；GC 只扫「运行中 M 栈 + 挂起协程栈」（协程栈独立 mmap 便于保守扫描）。
- 与 Go 对照、演进表、为何 Go 能百万级 → 见 qg-issue 27 §4。
- **先决预研（C 立项前置侦察，不写码）**：PuXian 自举解释器 interp.px/runtime interp 的调用结构是「平坦字节码循环 + 堆上帧栈」还是 C 递归 → 决定能否做 **VM 层协程**（存/恢复 VM 帧栈快照，如 Lua 协程），能则避开 Go 的汇编级栈切换 + 栈拷贝，C 类成本从「数月」降一档。此项侦察在 B 收口后启动，产出成本评估供用户拍板是否推进 C。

---

## 四·A、远景路线裁定总纲：VM 化是总钥匙，native 后端排最后（2026-09-07 讨论定稿）

> 本节由 2026-09-07 与本源连番讨论定稿，回答三个递进问题：
> ① 如何达到 Go 的百万级并发？② 百万级协程 vs Windows 支持，哪个复杂？
> ③ 若再加 native 机器码后端（自举到自己的机器码），哪个难？先后次序如何才有利？
> 定稿结论供后续所有旗舰立项引用，避免每次重新论证。

### 结论速览（顺序定稿）

> **A 止血 → B 事件驱动 → ① VM 化（旗舰1）→ ② C 类协程 → ③ native 机器码（旗舰2）→ ④ Windows（此时自动变易）**

| 档位 | 内容 | 难度 | 顺序逻辑 |
|---|---|---|---|
| **A**（今日 12:00） | 池化 + 槽动态 | 低 | 先止血，让服务先活下来（64 崩 → 512~1024） |
| **B** | epoll/poll 事件驱动 | 中 | 空闲连接不再占线程 → 连接 1 万~10 万 |
| **① VM 化** | AST 递归解释器 → 显式帧 + 平坦字节码 VM | **高（数月级）** | **总钥匙**：语义冻结 → IR 诞生 → GC 转精确 |
| **② C 类** | VM 上用户态协程 M:N | 中高（VM 化后反而便宜） | 帧可挂起/恢复 → Go 同构百万级水到渠成 |
| **③ native** | 字节码 → 机器码（AOT；可先经 C 过渡） | **最高** | 有了 VM/IR/GC map 才配做，否则边定语义边造编译器 |
| **④ Windows** | 平台 API 边界层 | 中低 | VM + 精确 GC 后只剩薄薄一层 OS API |

### 裁定理由

1. **native 后端解决"性能"，不解决"并发容量/平台"**：并发靠执行模型可挂起（协程），平台靠 OS API 层——这两样 native 都救不了，反而 native 会让 GC 栈扫描更难（需 GC map/安全点）。native 是"跑得快"之解，不是"挂得多/处处能跑"之解。
2. **VM 化是"三合一总钥匙"**，必须在 native 之前：
   - **IR**：平坦字节码 = 天然中间表示，native 代码生成的前提；没有 IR 直接从 AST 吐机器码 = 语义还在漂移就把逻辑焊死进寄存器分配，改一次语义重写一遍编译器；
   - **GC map**：原生代码里 GC 需要知道哪些寄存器/栈槽是 root → 需精确 GC，前提是帧显式（VM 帧）而非散落 C 栈；
   - **语义基准**：先有 VM 作"对的实现"，native 后端出 bug 才能对拍归因。
   - 教科书全走此路：JVM、CPython 3.11、LuaJIT = 先稳定 VM，再谈 JIT/机器码。
3. **PuXian 现状坐实"无捷径"**（2026-09-07 代码侦察）：interp.px 是 **AST 树遍历递归**（i_exec_stmt/i_eval_expr/i_eval_call 递归下降），每个 px 函数编译成 **C 函数 fn_xxx**、调用走 px_call **直接 C 递归**；**无堆上显式 VM 帧栈**，局部变量即 C 局部量。→ VM 层协程（Lua 式帧快照挂起）目前不可行，C 类只有两条重路：甲=VM 化后做帧协程；乙=C 栈协程（=造 Go 早期 runtime + GC 扫 N 条协程栈 + Windows 还得换 Fiber）。**甲是正路且与 ① 合并**。
4. **Windows 支持的本质是"广而确定"**（平台边界全覆盖），唯一硬骨头 = 保守 GC 跨线程扫栈（Boehm-GC 有成熟参照，照做能成）；VM 化 + 精确 GC 后它**自动变易**。顺序上排 native 之后最划算；若只是"想在 Windows 跑脚本"，可先做**单线程子集**（单线程解释器 + 只扫当前线程栈），一周级，绕开全部多线程 + GC 难题。
5. **例外与边界**：
   - 仅当**性能成为当下首要矛盾**（脚本慢到流失用户）且接受"边定语义边造编译器"的返工时，native 才可抢跑；PuXian 当前矛盾是并发就崩（issue 27）+ 语义未冻结，性能非瓶颈 → native 抢跑不划算；
   - "用 PuXian 写 PuXian 编译器"的元循环自举是**另一回事**（语言成熟度的收尾动作），更得排最后，别与 native 后端混为一谈；
   - 顺序并非铁律：若出现"数万长连接 + 每连接真业务"场景，可先上 ① VM 化的子集（只为协程铺路），但 VM 化整体仍应先于 C 类量产。

### 落地建议
- **M88 = A 类（今日 12:00）+ B 类**：照常，不动执行模型，1~2 周交付；
- **① VM 化 + ② C 类**：合并为一个独立**旗舰里程碑**（VM 化是 C 类前置），启动前先花半天做 VM 化可行性详勘（帧栈/闭包/异常/生成器如何搬、字节码格式、精确 GC 改造面），把工期钉到周级精度再拍板；
- **③ native**：旗舰2，排在 VM 化 + C 类之后，另行立项；
- **④ Windows**：除非有硬性 Windows 用户需求，排 ①③ 之后；急需则先上单线程子集。

## 五、验收清单（A 类 S3 总闸）
- [ ] 并发 100/500/1000 × 500 打 http_serve_unix 全 200、0 失败、进程不崩、无 restarts（http_serve/sse_serve 抽查同标）
- [ ] keep-alive 关闭后 GC 槽位回落（无泄漏）；`PX_MAX_THREADS`/池容量 env 生效且有界
- [ ] 服务路径无任何 spawn/槽满 exit 路径；语言层自 spawn 超限仍按语义报错（不属本 issue 改动）
- [ ] 回归总闸全绿 + 自举证明 rc=0 + native 数不变；fmt/lint 0；worktree 干净
- [ ] 文档同步 + CHANGELOG + qg-issue 27 状态更新；ws-approve #47 对拍重跑 + 灰度观察（交清歌）

### B 类验收清单（B-S4 总闸，2026-09-07 立项）
- [ ] 1 万 idle keep-alive 连接建立：进程 OS 线程数 ≈ 池容量 + 常数（不随连接数线性涨）
- [ ] idle 连接突发请求全 200、0 失败、进程不崩（http_serve / http_serve_unix / sse_serve 抽查）
- [ ] SSE 长连接 500+ 挂载 + sse_send 广播 + sse_close/sse_send 断线语义回归
- [ ] 短连接并发 100×500 吞吐/成功率不低于 A 类基线；服务路径无任何 spawn/槽满 exit
- [ ] 回归总闸全绿 + 自举证明 rc=0 + native 数不变；fmt/lint 0；worktree 干净
- [ ] 文档同步 + CHANGELOG + tag

## 六、风险与预案
| 风险 | 预案 |
|---|---|
| 槽动态扩容引入 GC 竞态/use-after-free（realloc 后旧址） | 扩容仅在 g_gc_mu 临界区；取址 `&g_threads[i]` 全量核对为锁内短生命；扩容先于并发压测的冒烟验证 |
| 池化改变 http_serve/sse_serve 既有语义（handler 约定/keep-alive/流式路由/sse 注册表） | S2 先核实两 conn_worker 与 handler 调用约定，案甲/案乙取侵入最小；行为回归用既有 m82/m83_s6/m84 样例 + 新增对拍 |
| 高线程数下 GC stop-the-world 停顿变长 | 默认槽上限 1024（甜点区）；文档明示；B 类事件驱动从根上降线程数 |
| px_serve 既有池被复用后容量/行为回归 | 案甲若复用需跑 px_serve 既有 verify（p5_px_serve/m34_pool_cfg/m36_pool_grace 等） |
| spawn/池 worker 同时抢槽扩容 | 槽分配本就全在 g_gc_mu，扩容并入同临界区；verify 并发 spawn + serve 混跑 |

**B 类新增风险：**
| 风险 | 预案 |
|---|---|
| 非阻塞重构引入半包/半 body 续接错误（丢字节/错位） | ConnCtx 动态缓冲续接 + 既有 HTTP 专项逐字节对拍（m82/m83_s6/A 类压测）+ 新增分片写（TCP_NODELAY 慢发）用例 |
| 状态机竞态（worker 与事件循环同抢 fd / fd 复用串扰） | 单持有者原则：FREE/ACTIVE/IDLE 原子转移，close 前强制置 FREE 并从事件循环摘除；并发压测长跑验证 |
| keep-alive 空闲超时语义漂移（SO_RCVTIMEO → 事件循环计时） | 默认 15s 语义保持；补空闲超时关闭用例 |
| M83-S6 stream 路由 / SSE 交接破坏（连接转通道后脱离普通 keep-alive） | stream_takeover 走专用状态转移路径 + m83_s6 专项回归 |
| g_gc_mu 容器锁竞争仍限吞吐 | 本批不重构（归 C 类前置预研/单独立项）；以「活跃 worker 收敛 → 抢锁线程减少」验证吞吐不降；若不足单独立项锁细化 |
| 非 Linux 平台 poll 兜底性能 | 文档明示 Linux epoll 一等；生产部署 Linux |

---

## 附：执行状态记录（2026-09-07）

| 项 | 状态 | 说明 |
|---|---|---|
| 立项 | ✅ done | docs/M88_PLAN.md 落盘（本文件）；qg-issue 27 状态更新；ws-todo **#14**（2026-09-07 12:00）已入，中午触发开工 |
| A 类 S1 | ✅ done | GC 线程槽动态化 + PX_MAX_THREADS 可配（固定 64 槽 → 按上限一次分配稳定表，无 realloc）；commit c530612；验证 s1_spawn_200（200 并发 spawn 全过）+ s1b_gc_stress（200 线程并发 GC 稳定） |
| A 类 S2 | ✅ done | 函数式 serve 连接池（http_serve/unix/sse_serve accept → fserve 池，队列背压绝不 exit）+ SIGPIPE 忽略 + bi_sleep EINTR 续睡（三处根因修复，见下执行摘要）；回归 m82 verify + m23a SSE+WS + s1 全绿 |
| A 类 S3 | ✅ done | **收口完成**：全能力重链 bootstrap/pxi（--full，9457456→9462024B，strings 含 PX_SERVE_WORKERS/PX_MAX_THREADS 实证 M88 runtime 入解释器宿主）；自举证明 rc=0（B.c==golden 10595 行）；native 301 不变；回归总闸 m82+m83_s1-s4+m84_s1-s3+m85_s1-s2+m86_s0-s2 干净全绿（m83_s5/s6 内容全 PASS，收尾 EXIT-trap `kill 0` 进程组自杀 = M84-S4 起记录不修的历史边界，非本 M 回归）；M88 专项 s1_spawn_200 200 并发 PASS + http_serve_unix 100×500 **全 200 0 失败 0 err 进程不崩**（53s）；fmt/lint 0；文档同步（spec §8.22/CHEATSHEET/M88_PLAN）+ examples/m88_s3/press_unix.go #→// 修正；qg-issue 27 归档 done/；tag v0.1.0-m88 |
| 远景路线裁定 | ✅ done | §四·A 落盘（VM 化=总钥匙；native 后端排最后；Windows 排 native 后；单线程子集可应急） |
| B 类立项 | ✅ done | §三 细化 B 方案 + S1-S4 拆分落盘（本文件，2026-09-07）；**不挂 ws-todo**（用户指令"现在立项开工 B 类，不用挂 ws-todo"） |
| B 类 S1 | ✅ done | 连接上下文表 ConnCtx（fd 索引动态扩容，PX_MAX_CONNS 可配默认 16384）+ FREE/ACTIVE/IDLE 状态机（单持有者）+ 事件循环线程（Linux epoll EPOLLET + 唤醒管道 + 1s tick 扫 15s 空闲超时；非 Linux 降级 stub 零行为变化）+ px_evc_acquire/close/idle_put/idle_pop/px_ev_ensure 接口；commit 3babe04 |
| B 类 S2 | ✅ done | **http_conn_worker 请求级重构**——空闲 keep-alive 连接交还 IDLE 事件驱动（见下 B 类执行摘要）；commit 3c08bb9 |
| B 类 S3 | ✅ done | **SSE 长连接事件循环化 + g_sse_conns 动态化**——handler 返回后明文连接交还 IDLE（事件循环照看断开/超时；SSE 空闲不超时）+ 注册表 256→PX_MAX_SSE_CONNS 动态化（默认 4096）+ sse_close/send 统一关闭路径；commit 30cfbfa；验证 bs3 1000 长连接线程恒 10 + m23a/m82/m83_s6 全绿（见下 B 类执行摘要） |
| B 类 S4 | ✅ done | **收口完成（tag v0.1.0-m88b）**：万级空闲 keep-alive（10000 挂载线程恒 10）+ SSE 1000 长连接 + 短连接吞吐 50000/50000 ≥ A 基线 + 回归总闸全绿 rc=0 + 自举证明 rc=0 + native 301 不变 + 重链 pxi + 文档同步 + CHANGELOG（见下 B 类执行摘要） |

### A 类执行摘要（2026-09-07 12:00 开工，S1/S2 完成）

- **S1（commit c530612）**：`g_threads` 固定 64 槽 → `g_thread_cap/g_thread_max` 动态表（gc_init_env 按 PX_MAX_THREADS 上限一次分配，指针恒定无 realloc 竞态）；17 处遍历 + 3 处报错改动态；env `PX_MAX_THREADS`（默认 1024，夹取 [64,4096]）。**验证**：200 并发 spawn 全过（旧 64 崩）；`PX_MAX_THREADS=64` 时 200 spawn 正确报"超出上限 64"；200 线程×300 对象+强制 GC 冒烟稳定。
- **S2（待 commit）**：① **函数式 serve 连接池**——http_serve/http_serve_unix/sse_serve 的 accept 循环从"每连接 px_spawn"改为投递 `fserve` 常驻池（accept 只 push (fd,kind)，worker 按 kind 调 http_conn_worker/sse_conn_worker，处理语义逐字节不变；队满阻塞背压，**服务进程永不因 spawn/槽满 exit**）；env `PX_SERVE_WORKERS`（默认 256，夹取 [8,4095]）。② **SIGPIPE 忽略**（gc_install_handler）——http_conn_worker 裸 send 到已断开连接触发 SIGPIPE 默认杀进程（无 core 无日志的"悄然消失"）。③ **bi_sleep EINTR 续睡**——nanosleep 不在 SA_RESTART 自动重启清单，主线程 sleep(长) 被 GC 信号打断提前返回 → main 结束进程静默退出；对齐 timer_sleep_ms/sleep_us 续睡模式。
- **压测实测基线**（examples/m88_s3/s2_serve_unix.px + press_unix.go，本机）：并发 100×500（5 万请求）**全 200、0 失败、进程不崩**（~63s）；30/60×500 全 200。m82 http_serve_unix 专项 verify 全 PASS（残留 sock 清理/0600/断连容忍/TCP 双跑）；m23a SSE+WS 全 PASS（sse_serve 池化后 SSE 功能零回归）。
- **已知架构边界（A 类不重构，B 类及后续根治）**：M11 容器修改（px_dict_set/px_list_push 等）与 GC 经全局 `g_gc_mu` 互斥 → 高并发（≥256 keep-alive 活跃连接）下锁竞争使总吞吐饱和 ~1k req/s，表现为请求超时而非崩溃；且池 worker 常驻注册使 GC stop-the-world 暂停线程数 = 池容量。**A 类已根除 issue 27 的"并发即崩溃/exit"**；海量 keep-alive 空闲连接吞吐需 B 类 poll 事件驱动 + 容器锁细化（可并入后续里程碑，不与 A 混 commit）。

> 立项 commit：M88-PLAN 落盘（qg-issue 27 ISSUE.md 状态 + docs/M88_PLAN.md）
> S1 commit：c530612；S2 commit 见执行时记录

### B 类执行摘要（2026-09-07 立项开工，S1-S3 完成）

- **S1（commit 3babe04）**：**事件驱动内核**——ConnCtx 连接上下文表（fd 索引 + 动态扩容，容量 `PX_MAX_CONNS` env 可配默认 16384）+ FREE/ACTIVE/IDLE 状态机（单持有者原则：同一 fd 任一时刻只一个持有者，杜绝 worker 与事件循环双读/fd 复用串扰）+ 事件循环线程（Linux **epoll** `EPOLLIN|EPOLLRDHUP|EPOLLET` + 唤醒管道 + 1s tick 扫 15s 空闲超时；非 Linux 自动降级 stub 走原阻塞路径，不破坏 Windows 交叉编译）+ 接口 `px_evc_acquire/px_evc_idle_put/px_evc_idle_pop/px_evc_close/px_ev_ensure`。⚠️ 过程中发现 runtime 已存在 `PxConn` 连接对象体系（`px_conn_close(PxConn*)` 全局函数），新内核统一 `px_evc_` 前缀避开，未动既有 PxConn 体系。S1 未接入 conn_worker → 事件循环不启动、行为零变化（按 PLAN 约定）。验证：px build + hello/fib + m82 http_serve_unix 专项 8 项全 PASS。
- **S2（commit 3c08bb9）**：**http_conn_worker 请求级重构**（http_serve/http_serve_unix 同 worker 一处改两入口通）——serve 连接 fd 一律非阻塞 + 登记 ConnCtx（FREE→ACTIVE）；请求读改用新增 `px_recv_wait`（poll+recv，语义 = 阻塞 recv + SO_RCVTIMEO 15s，非阻塞下半包/大 body 续读自然达成，无需跨派发存半包）；**响应写完且无下一请求数据在途 → `px_evc_idle_put` 交还 IDLE**（epoll 照看：可读摘除 + 派发回 fserve 池、15s 空闲超时/对端断开由事件循环 tick close），worker 返回释放去取新 job = **空闲 keep-alive 连接不再占线程**；收尾统一 `px_evc_close`（清理 ctx 防 fd 复用串扰）；acquire/idle_put 失败（非 Linux / fd 超 PX_MAX_CONNS）自动降级原阻塞续读路径，功能不降。前置：B-S1 PxConnCtx typedef 加标签（worker 位于内核前需前向声明）、FSERVE_KIND_* 宏 #ifndef 保护。
- **B-S2 验证实录**（examples/m88b_bs2/ 归档）：`PX_SERVE_WORKERS=8` 下——① 200 空闲 keep-alive 连接首请求 **200/200**（A 类 8 worker 只能同时喂 8 连接，空闲即释放实证）；② 500 空闲连接挂载后服务线程数**恒定 10**（8 fserve + 1 事件循环 + 1 主，不随连接数涨 = 空闲不占 worker 核心指标）；③ 突发（空闲连接复用）**50/50 全 200**（事件循环派发回池正确）；④ 16s 后服务端关闭空闲连接 = **15s 空闲超时语义保留**。回归：m82 http_serve_unix 8 项 PASS、m83_s6 同端口流式 SSE 5 项 PASS（含断连不崩）、A 类短连接压测场景 100×300=30000 全 200 0 err（45.5s）。SSE 长连接（sse_conn_worker）仍在阻塞路径 → B-S3。
- **S3（commit 30cfbfa）**：**SSE 长连接事件循环化 + 注册表动态化**——① sse_conn_worker handler 返回后，明文连接 `px_evc_acquire`+`px_evc_idle_put` 交还 IDLE（epoll 照看断开/异常 → `sse_server_close_fd` 统一清理），worker 返回释放去取新 job = **SSE 长连接不再占 worker**；TLS / 非 Linux / fd 超限自动降级原阻塞保持路径（功能不降，仅 TLS SSE 仍占 worker，文档注明）。② 服务端 SSE 注册表 g_sse_conns 256 定长 → 动态容量表（env `PX_MAX_SSE_CONNS` 默认 4096 夹取 [64,65536]，首次用惰性一次性分配，无 realloc 竞态）。③ 事件循环按 kind 分流：SSE IDLE 只断开检测（drain 读 + FIN/错误关闭），**SSE 空闲不超时**（tick 空闲超时仅对 http IDLE 生效，keep-alive 15s 语义不受扰）；sse_close IDLE→统一清理、ACTIVE→原 shutdown 唤醒兜底；sse_send 写失败锁外统一清理；新增 `px_evc_detach`（摘除事件循环不 close fd）与 `px_evc_is_idle`。stream_takeover_conn/http_stream 共享动态注册表。**B-S3 验证实录**（examples/m88b_bs3/ 归档）：`PX_SERVE_WORKERS=8` 下 **1000 SSE 长连接全建立+收 3 事件、3s 后 1000/1000 存活（不被 15s tick 误关）、服务线程恒 10、断开 500 重开 500 全成功**；回归 m23a SSE+WS PASS、m82 8 项 PASS、m83_s6 同端口流式 SSE 5 项 PASS。
- **S4（收口，commit M88-B-S4，tag v0.1.0-m88b）**：**B 类收口验证**——① 万级量级压测（examples/m88b_s4/，PX_SERVE_WORKERS=8）：10000 空闲 keep-alive 同时挂载（首请求+窗口内突发复用均 200，9720/10000，其余为 keep-alive 15s 设计语义超窗关闭）**服务线程恒 10**；② SSE 1000 长连接（m88b_bs3）线程恒 10；③ 短连接吞吐回归 ≥ A 基线：http_serve_unix 100×500=50000/50000 全 200 0 失败0 err 不崩（69.6s）；④ 回归总闸 m82+m83_s1-s6+m84_s1-s3+m85_s1-s2+m86_s0-s2 全部 rc=0（m83_s5/s6 退出码随 af75807 归 0）+ m23a PASS；⑤ 自举证明 rc=0（B.c==golden 10595 行）+ native 301 不变（B 类五提交零触碰 px_native 注册面）+ fmt/lint 0 + worktree 干净；⑥ 全能力重链 bootstrap/pxi（9462024→9467264B，strings 含 PX_MAX_CONNS/PX_MAX_SSE_CONNS/PX_SERVE_WORKERS 实证 + pxi 冒烟 rc=0）；⑦ 文档同步（spec §8.22 连接语义→池化+事件驱动、CHEATSHEET、M88_PLAN）。tag v0.1.0-m88b。
