# M88_PLAN · runtime 并发模型演进：A 线程池止血 → B 事件驱动 → C 用户态协程（qg-issue 27）

> 创建：2026-09-07 · 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD 01999de = m86 后；runtime/runtime.c 14110 行）
> 来源：qg-issue **27-puxian-spawn-concurrency**（清歌 ws-approve .px 化 #47 压测/线上事故：http_serve_unix 高并发崩溃）
> 用户指令（2026-09-07）：**「把 A、B、C 全部立项 M88 里面。今天中午（12:00）开工，只需要实现 A 类」**
> 性质：**L0 runtime（C，GC 线程表 + 服务端并发模型层）**；业务 .px 零改动；native 总数不变
> 风险等级：**L0**（动 GC/线程模型）→ 必须：自举证明 + 回归总闸 + 并发压测 + 文档同步
> 状态：🆕 立项（A/B/C 全部归属 M88；A 类今日 12:00 开工，见 §七 执行状态）

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

## 三、B 类（M88 后续批次，事件驱动半同步/半异步）

- 目标：连接数上万~10 万，keep-alive **空闲连接不占 OS 线程**。
- 方案：连接处理统一收敛到线程池；空闲连接由 poll/epoll 事件循环照看，**有完整请求才派给池 worker**（半同步/半异步，即 issue 建议 B + 那篇 C 语言 epoll 文章的架构内核）；`SO_RCVTIMEO` 轮询式空闲判定改为事件驱动。
- 依赖：A 类 S1（槽动态化）已完成，本档收益最大化。参考：cnblogs.com/zhengpan0526/p/18860258（epoll 演进 + Linux 内核参数调优表：fs.file-max/nofile/nf_conntrack/tcp_mem…对 Go/PuXian 通用）。

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

## 六、风险与预案
| 风险 | 预案 |
|---|---|
| 槽动态扩容引入 GC 竞态/use-after-free（realloc 后旧址） | 扩容仅在 g_gc_mu 临界区；取址 `&g_threads[i]` 全量核对为锁内短生命；扩容先于并发压测的冒烟验证 |
| 池化改变 http_serve/sse_serve 既有语义（handler 约定/keep-alive/流式路由/sse 注册表） | S2 先核实两 conn_worker 与 handler 调用约定，案甲/案乙取侵入最小；行为回归用既有 m82/m83_s6/m84 样例 + 新增对拍 |
| 高线程数下 GC stop-the-world 停顿变长 | 默认槽上限 1024（甜点区）；文档明示；B 类事件驱动从根上降线程数 |
| px_serve 既有池被复用后容量/行为回归 | 案甲若复用需跑 px_serve 既有 verify（p5_px_serve/m34_pool_cfg/m36_pool_grace 等） |
| spawn/池 worker 同时抢槽扩容 | 槽分配本就全在 g_gc_mu，扩容并入同临界区；verify 并发 spawn + serve 混跑 |

---

## 附：执行状态记录（2026-09-07）

| 项 | 状态 | 说明 |
|---|---|---|
| 立项 | ✅ done | docs/M88_PLAN.md 落盘（本文件）；qg-issue 27 状态更新；ws-todo **#14**（2026-09-07 12:00）已入，中午触发开工 |
| A 类 S1 | ⏳ 今日 12:00 开工 | GC 线程槽动态化 + PX_MAX_THREADS 可配（18 处遍历 + 扩容 + env） |
| A 类 S2 | ⏳ | http_serve/http_serve_unix/sse_serve 接入连接线程池（案甲/案乙定稿后实施） |
| A 类 S3 | ⏳ | 压测（examples/m88_s3）+ 回归总闸 + 文档 + 收口 |
| 远景路线裁定 | ✅ done | §四·A 落盘（VM 化=总钥匙；native 后端排最后；Windows 排 native 后；单线程子集可应急） |

> 立项 commit：M88-PLAN 落盘（qg-issue 27 ISSUE.md 状态 + docs/M88_PLAN.md）
