# M105_PLAN —— 运行时热点路径重构（全局表名解析 O(1) 化 + GC 同步瘦身）

> 上游依据：`docs/M104_PLAN.md` §7.3 归因 ——「M104 证伪了『优化解释循环分派可拿到 1.2~1.5x』的假设，
> 真瓶颈在 runtime 侧的名解析与锁路径」。M105 承接该结论，**先量化、再动手**。
>
> 修订纪律：S1 未出量化表前不动实现；量化表否定的子项立即降级（不许按立项时的想象推进）。
>
> **进度**：S1 量化 ✅ · S2 全局表 O(1) ✅（`a4debda`，实测 1.41x）· **S3 GC 同步瘦身 ✅（§七，实测再 1.30~1.33x，累计 ~1.9x）** · **S5 收口 ✅（§八：双自举 + pxi/pxi_vm 重链 + 回归 + tag `v0.2.0-m105`）**。

---

## 一、立项口径

| 项 | 内容 |
|---|---|
| **目标** | 削掉 M104 归因中指认的 runtime 侧真瓶颈：**全局表线性名解析** + **GC 同步开销** |
| **靶负载** | 真实负载 `compiler_vm bc stdlib/yaml.px`（592 行；VM 轨自举编译器；M104 基线 1.04x 的那个负载）为主判据；辅以 4 个微基准 |
| **红线** | `vm_ab` 38P/0GAP/0F · `diffcheck --all` rc=0 · 双自举逐字节 · m89~m104 suites 零回归 · GC/协程并发面（m93~m102）零回归 |
| **不做** | 去箱化/类型反馈 JIT（研究级，D0 已判 6 个月+）；native 后端；百万并发压测（等云机器） |

---

## 二、S1 量化（2026-09-11，本机 8 核 16G）

### 2.1 方法（零侵入 + 可复现）

| 手段 | 说明 |
|---|---|
| **指令直方图** | `/tmp/m105inst/vm.c` 副本插桩（`g_op[op]++` 于解释循环取指后），重链 `compiler_vm` 得到插桩二进制；**产物 dump 与基线逐字节一致**（证明插桩行为中性） |
| **调用计数** | `LD_PRELOAD` shim 拦截 `strcmp/pthread_sigmask/pthread_rwlock_*lock/pthread_mutex_lock`，按调用者返回地址归档（`addr2line` 归因） |
| **单元成本** | `/tmp/m105inst/unitbench.c` —— 链接 rtcache 的 `runtime.o`，复刻 `g_len=537`（255 内建 + 282 模块全局）实测各原语 ns |
| **A/B 反事实** | 在 rtcache 口径下重编 `runtime.c`（cuts 旗标推定正确 → dump 逐字节复现），替换为哈希索引原型，5 轮 `taskset -c 3` 取 min CPU 时间 |

> 插桩件全部位于 `/tmp/m105inst/`，**不入库**；`selfhost/build/compiler_vm`（基线）与 `compiler_vm_v0`（同码重编）互为对照。

### 2.2 指令构成（`compiler_vm bc stdlib/yaml.px`，单进程）

| 项 | 值 |
|---|---|
| 动态指令总数 | **5,302,670** |
| `GETG`（全局读） | **723,980（13.7%）** |
| `SETG`（全局写） | 51,557 |
| `SRCLINE` | 869,191（16.4%） |
| `CALL` | 312,181 |
| `INDEX` | 289,942 |
| `GETF`/`SETF` | **0 / 0**（该负载不用结构体） |

### 2.3 名解析：**每次 GETG 平均探测 331.6 项**

| 指标 | 实测 |
|---|---|
| `strcmp` 调用总数 | **262,956,746** |
| 其中来自 `px_get_global` 循环 | **240,083,442（91.3%）** |
| 其中来自 `px_set_global` 循环 | 22,601,530（8.6%） |
| **全局表名解析合计** | **249,747,716 / 262,956,746 = 95.0%** |
| 平均每次 GETG 的 strcmp 数 | 240,083,442 / 723,980 = **331.6** |
| 全局表规模 | `g_len ≈ 537`（255 内建 + 282 模块全局）→ **平均扫过全表 62%** |

其余 strcmp 来源：`px_method` 54,843 · `px_dict_get` 165,391 · 其它 <1e4 —— **可忽略**。

### 2.4 单元成本（`taskset -c 3`，7 轮）

| 原语 | 实测 |
|---|---|
| `strcmp`（热、首字符不等 / 全等） | 0.7 ~ 1.5 ns |
| `pthread_rwlock_rdlock + unlock`（无争用） | **12 ns** |
| `pthread_sigmask` 屏蔽 + 还原（= 一次 `gc_block_stop`/`unblock` 对） | **343 ~ 372 ns**（≈ 2 次 `rt_sigprocmask` 系统调用） |
| `px_get_global`（首位命中 / 末位命中 / 全表均值） | 1111 / 2316 / **2081 ns** |
| `px_dict_get`（64 项末位） | 266 ~ 325 ns |
| 合成对照：`rdlock + sigmask对 + 1×strcmp` | **367 ns** |

> 即 `px_get_global` 的成本 ≈ `sigmask 对（343ns） + 探测 N 项 × 约 5ns`。
> **锁不是问题（12ns），`sigmask` 是固定大头，线性探测是随规模增长的大头。**

### 2.5 GC 同步开销（跨负载，`sys` 时间即信号系统调用）

| 负载 | `sigmask` 调用 | `mutex_lock` | `rwlock_rdlock` | user / **sys** |
|---|---|---|---|---|
| `while_sum`（纯算术） | 1,670 | 834 | 3 | 0.31 / **0.00** |
| `fib26`（递归） | 3,930,062 | 851 | 1,964,181 | 4.24 / **0.55** |
| `loop_sum`（`range(3M)` 物化） | 6,001,778 | 3,000,884 | 5 | 0.56 / **0.87** |
| `field_while`（结构体读写） | 8,001,682 | 4,000,841 | 3 | 1.08 / **1.11** |

**结论（数据否决立项想象）**：
- `px_field`/`px_field_set` 的**名字解析**（线性 strcmp）成本很小（`field_while` 全程仅 18M strcmp ≈ 0.1s）→
  **原计划 S3「字段/字典名解析缓存」应降级**；
- 但 `px_field_set` / `px_list_push` / `px_dict_set` 的 **`pthread_mutex_lock(g_gc_mu)` + `sigprocmask` 对**才是容器/字段路径的支配项
  —— `field_while` 与 `loop_sum` 的 **sys 时间 ≈ user 时间**，几乎全部是信号系统调用；
- `while_sum`（M104 优化的那种负载）**根本不碰 runtime** → 这解释了 M104 为何封顶 1.10x。

---

## 三、S1 反事实原型（**验证收益，不交付**）

在 rtcache 口径下重编 `runtime.c`，把全局表线性扫描换成 **64 位名哈希开放寻址索引**（`GHASH_CAP = 2×GLOBAL_CAP`，容量 8192、负载 ≤0.5；
插入点唯一：`px_set_global` 的 `g_keys[g_len]=…`；**锁语义、GC 根扫描线性遍历均不变**，仅命中时做 1 次 `strcmp` 兜底）。

| 变体 | CPU 时间（min/5 轮） | vs v0 | 产物 dump |
|---|---|---|---|
| baseline（M104 入库二进制） | 5.160 s | — | 基线 |
| **v0（同码重编，对照）** | 4.810 ~ 4.770 s | 1.00x | 逐字节一致 ✅ |
| **h1 = 全局表哈希索引** | **3.550 ~ 3.630 s** | **1.33x** | 逐字节一致 ✅ |
| h2 = h1 + 跳过 `sigprocmask`（**不安全，仅测上限**） | **2.570 s** | **1.87x** | 逐字节一致 ✅ |

**旁证**：h1 的 `strcmp` 调用数 **262,956,746 → 1,047,279（-99.6%）**；`sigmask` 调用数不变（4,384,444）→ 正是 h2 再吃掉的 1.06s。

> **S1 结论**：M105 方向的收益上界远高于 M104 —— 单「全局表 O(1)」即 **1.33x**（真实负载，实测），
> 叠加「GC 同步瘦身」达 **1.87x**（上限，需安全实现）。

---

## 四、S2~S5 重新排序（数据驱动，替代立项时的顺序）

| 序 | 子步 | 依据 | 预期（实测/推算） | 风险 |
|---|---|---|---|---|
| **S2** | **全局表 O(1) 名解析**（哈希索引，锁/GC 语义不变） | §2.3 95% strcmp、§三 实测 | **1.33x**（已实测） | 低（原型已 dump 逐字节一致） |
| **S3** | **GC 同步瘦身**：`gc_block_stop/unblock` 快速旁路（原子标志；无 GC 线程/未启用时免系统调用），保持 stop-the-world 协议 | §2.5 sys≈user | up to **+1.41x**（h2 实测增量） | **中高**（协议正确性） |
| S4 | 容器/字段写入路径的 GC 锁粒度（`px_list_push`/`px_dict_set`/`px_field_set` 每操作一次 mutex+sigmask） | §2.5（6M~8M 次/负载） | 与 S3 部分重叠，S3 后重测再定 | 中 |
| ~~S5~~ | ~~字段/字典名解析缓存~~ | §2.5 **数据否决**（仅 18M strcmp ≈ 0.1s） | 降级为「暂不做」 | — |

---

## 五、回归红线（每步必过）

`vm_ab` 38P/0GAP/0F · `diffcheck --all` rc=0 · 双自举逐字节（C 轨 B.c + BC 轨重放）· m89_s3d/m93_s3/m96_s2/m99_s2/m103_s2d 零回归 · GC/协程并发面（m93~m102）零回归 · 真实负载 dump 逐字节一致。
**收益不达标即逐项记录回退**（M104 纪律）。

---

## 六、S2 实施记录（入库：`runtime/runtime.c`，+67/-30）

### 6.1 改动

| 位置 | 改动 |
|---|---|
| 全局表定义后 | 新增 **64 位名哈希开放寻址索引**：`g_hidx[GHASH_CAP]`（槽位号+1，0=空）+ `g_hval[GHASH_CAP]`（探测位哈希）+ `g_name_hash`（FNV-1a 64）+ `g_hash_find` + `g_hash_put`；`GHASH_CAP = 2×GLOBAL_CAP = 8192`（负载因子 ≤0.5） |
| `px_get_global` | 线性 `for + strcmp` → `g_hash_find`（命中时 1 次 strcmp 兜底） |
| `px_global_native` | 同上（ffi 兜底路径，语义按「名存在则按类型返回」保留） |
| `px_set_global` | 查找改哈希；**新增槽位处 `g_hash_put` 落位**（写入点唯一，索引与 `g_keys/g_vals` 恒同步） |
| `px_method`（struct 方法查找） | 线性扫描 → 哈希（CALLM 热路径，名 =「类型.方法」） |

**不变项（红线）**：锁语义（读锁只读索引 / 写锁只写索引，与 `g_keys/g_vals` 同锁）、
GC 根面（根扫描仍线性遍历 `g_vals`，索引不参与 GC）、插入点唯一性、`GLOBAL_CAP` 溢出语义、错误文案。

### 6.2 实测（`taskset -c 3`，5 轮取 min CPU 时间；真实负载 `compiler_vm bc stdlib/yaml.px`）

| 变体 | CPU 时间 | vs 基线 | 产物 dump |
|---|---|---|---|
| baseline（M104 入库二进制） | 5.080 s | 1.00x | 基线 |
| v0（同码重编对照） | 5.080 s | 1.00x | 逐字节一致 ✅ |
| **S2（全局表哈希，入库版）** | **3.600 s** | **1.41x** | **逐字节一致 ✅** |

`strcmp` 调用数：**262,956,746 → 1,047,279（-99.6%，LD_PRELOAD 实测）**。

### 6.3 验证

| 门 | 结果 |
|---|---|
| `vm_ab.sh v2`（全量 VM vs 旧轨对拍） | **38 PASS / 0 GAP / 0 FAIL** ✅ |
| `selfhost/diffcheck.sh --all` | **rc=0（全量对拍全部通过）** ✅ |
| `examples/m89_s3d/verify.sh`（并发 GC / 生成器 / 堆回落） | **9 PASS / 0 FAIL** ✅ |
| `examples/m93_s3/verify.sh`（帧协程 chan/mutex/rwlock/sleep + 并发 GC） | **6 PASS / 0 FAIL** ✅ |
| `examples/m96_s2/verify.sh`（daemon / 线程池 / 逃生舱） | **8 PASS / 0 FAIL** ✅ |
| `examples/m103_s2d/verify.sh` | **rc=0** ✅ |
| C 轨 + VM 轨冒烟（`hello`/`fib`） | 双轨 stdout 一致 ✅ |
| 真实负载产物 dump | 与 M104 基线**逐字节一致** ✅ |

### 6.4 待办（S5 收口）

- 双自举证明（`bootstrap_prove.sh` + `bootstrap_prove_bc.sh --fresh`）——**runtime 变更需重链**；
- `bootstrap/pxi`（C 轨解释器）/ `bootstrap/pxi_vm`（VM 轨）重链吸收 M105 runtime；
- CI 全绿 + tag `v0.2.0-m105`。

### 6.5 S3 预备结论（数据已就位，未动手）

- `compiler_vm bc yaml.px` **单线程运行**（实测 `/proc/<pid>/status` Threads=1）；
- GC stop-the-world 以 `if (g_active_threads > 0)` 为闸门（`runtime.c` px_gc_collect）——
  即**无其他已注册活跃线程时，GC 根本不会发 `SIG_GC_STOP`**；
- 故 `gc_block_stop/unblock` 存在**可证明安全**的快速旁路（谓词：`g_active_threads == 0`），
  实现需处理「临界区内 spawn 线程 → 谓词翻转」的嵌套/初值问题（拟用 thread-local 层栈，
  保证 block/unblock 成对判定一致）。
- **收益上限已实测**：h2（不安全版，直接跳过 sigmask）= 2.570s，即 **1.87x vs 基线**；
  S2 之上再增 **1.41x**（3.600→2.570s）。

---

## 七、S3 实施记录（入库：`runtime/runtime.c`，+68/−4）

### 7.1 改动：真屏蔽 → 「软屏蔽 + 延迟暂停」

| 位置 | 改动 |
|---|---|
| `gc_block_stop` 前向声明后（文件头） | 新增 TLS 状态：`g_gcs_depth`（block/unblock 嵌套深度）· `g_gcs_skip`（最外层是否走软屏蔽）· `g_gc_crit`（软屏蔽临界区标志，处理器据此**延迟暂停**）· `g_gc_deferred`（观测计数） |
| `gc_stop_handler` **首行** | `if (g_gc_crit) { g_gc_deferred++; return; }` —— 临界区内收到暂停信号时**立刻返回**：不保存 ucontext、不上报 `paused`、不写线程表，交 executor 既有重发循环（200us 间隔 + 5s 兜底）稍后重试 |
| `gc_block_stop` / `gc_unblock_stop` | **仅最外层决策**：`g_active_threads == 0` ⇒ 免 `pthread_sigmask`（只置/清 `g_gc_crit` + `__atomic_signal_fence` 防编译器跨临界区重排）；否则走**原真屏蔽分支**。内层直接返回（LIFO 层栈 ⇒ 内外层判定不可能不一致）。含不配对防御（深度 >16 告警一次；负深度复位） |
| `gc_install_handler` | `pthread_atfork(NULL, NULL, gc_atfork_child)`（`#ifndef _WIN32`）：fork 子进程复位层栈——防「父进程恰在软屏蔽临界区 fork」致子进程永久 skip 态 |
| `px_gc_collect` 调试输出 | `[gc] me=… active=… deferred=…`（观测延迟暂停次数，S3 验证用） |

**语义不变项（红线）**：真屏蔽分支**原样保留**；GC 根面 / 标记-清扫 / 暂停协议 / 锁语义 / 锁序**全部未动**；
公共 API `px_gc_block_stop_sig / px_gc_unblock_stop_sig` 签名不变（`coro.c` 20 处调用零改动）。

### 7.2 安全性论证（为什么「不屏蔽」≡「屏蔽」）

1. **关键不变量**：GC executor 只有在**所有已注册线程都上报本轮 `paused`** 之后才进入 mark（§-6.5 的等待循环）。
   软屏蔽临界区内的线程**永不上报 paused** ⇒ **标记期绝不与任何临界区并发** —— 与「真屏蔽」完全等价。
2. **最坏退化 = 改动前的最坏情况**：真屏蔽时「信号被阻塞 → 线程不上报 paused → executor 重发至 5s 兜底跳过」；
   延迟暂停时「处理器返回 → 线程不上报 paused → executor 重发至 5s 兜底跳过」—— **同一条退化路径**。
3. **常态代价**：真屏蔽是「挂起信号在解锁瞬间递达 → 立即暂停」；软屏蔽是「临界区退出后 ≤1 个重发周期（200us）暂停」。
4. **谓词陈旧不破坏安全**：`g_active_threads` 的无锁读若陈旧（临界区内恰好 spawn 线程 ⇒ 谓词翻转），
   信号确实可能到达 —— 但处理器只做**延迟暂停**，故**安全性不依赖该读值的时序**（仅影响性能）。
5. **层栈必要性**：若不按层栈而逐层独立决策，嵌套时会出现「外层 skip / 内层真屏蔽」的混合，
   使内层 unblock 误走 skip 分支（真掩码未还原 + `g_gc_crit` 下溢）—— 层栈保证成对判定唯一。

### 7.3 实测（`taskset -c 3`，5 轮取 min CPU 时间；真实负载 `compiler_vm bc stdlib/yaml.px`）

> 同会话三组交替（消除机器状态漂移；跨会话绝对值不可比，比值可比）：

| 变体 | CPU 时间（三轮取优） | 相对 S2 |
|---|---|---|
| baseline（M104 入库二进制） | 5.350 ~ 5.400 s | — |
| S2（全局表哈希名解析） | 3.690 / 3.940 / 3.770 s | 1.00x |
| h2（**裸跳过 sigmask，不安全**，仅测上限） | 2.900 / 2.980 / 2.780 s | 1.27~1.33x |
| **S3（软屏蔽，入库版）** | **2.950 / 2.990 / 2.840 s** | **1.30~1.33x** |

- **S3 vs S2 = 1.30~1.33x**；**S3 vs M104 基线 = 1.88~1.90x**（M105 累计）。
- **S3 贴着上限**：与 h2（无任何保护）差距 **2.2%** ⇒ 软屏蔽把可拿的收益**吃满**，保护成本≈0。

### 7.4 机制旁证（`LD_PRELOAD` 计数，S1 同一 shim）

| 指标 | S2 | S3 |
|---|---|---|
| `pthread_sigmask` 调用 | **4,384,444** | **0** |
| `strcmp` 调用 | 1,047,279 | 1,047,279（不变） |
| `rwlock_rdlock` / `mutex_lock` | 723,981 / 2,932,655 | 723,981 / 2,932,655（不变） |

→ 系统调用**全部消除**，且**其余行为计数逐项不变**（无副作用外溢）。
产物 dump：baseline / S2 / S3 三轮 **md5 相同**（`36a0f9d655d7d8122a961ca7f77d9436`，54380B）。

### 7.5 回归（全部在 S3 runtime 上重跑；缓存核对见下）

| 门 | 结果 |
|---|---|
| `examples/m89_s3d/verify.sh`（并发 GC 压测 ×3 / spawn / 生成器 / 堆回落） | **9 PASS / 0 FAIL** ✅ |
| `examples/m93_s3/verify.sh`（帧协程 chan/mutex/rwlock/sleep + 并发 GC） | **6 PASS / 0 FAIL** ✅ |
| `examples/m96_s2/verify.sh`（daemon / 线程池 / 逃生舱） | **8 PASS / 0 FAIL** ✅ |
| `examples/m98_s2/verify.sh`（px_serve handler 协程化；含线程峰值 12≤18） | **7 PASS / 0 FAIL** ✅ |
| `examples/m99_s2/verify.sh`（连接级事件化 IDLE + TLS 空闲事件化） | **8 PASS / 0 FAIL** ✅ |
| `examples/m103_s2d/verify.sh`（img 链路） | **rc=0** ✅ |
| `examples/m89_a2/vm_ab.sh v2` | **38 PASS / 0 GAP / 0 FAIL** ✅ |
| `selfhost/diffcheck.sh --all` | **rc=0** ✅ |
| 冒烟（`hello`/`fib`，C 轨 + VM 轨） | 双轨一致 ✅ |

**缓存核对**（防「跑的是旧 runtime」）：本次回归窗口内新建的 3 个 rtcache（22:29:43 最小档 / 22:30:42 route 档 /
22:31:27 image 档）`runtime.c` 均含 S3 标记；`m96/m98/m99` 的 daemon 产物 22:30 重建且带 `deferred` 符号 ⇒
**并发/服务面套件确实跑在 S3 runtime 上**。
**并发面零行为变化实测**：正常谓词下并发 GC 9 轮 `deferred=0`（⇒ 并发模式全部走真屏蔽分支，与改动前逐字等价）。

### 7.6 对抗性验证（把「延迟暂停」路径逼出来）

延迟暂停是单线程负载**用不到**的兜底路径（单线程无人发信号）⇒ 用一个 **/tmp 专用变体**
（`runtime_force.c`：谓词强制恒真，并发下也走软屏蔽；**不入库**）链 `vm_conc_gc_stress` 跑并发 GC 压测：

| 轮 | rc | 结果 | 每轮 GC 的累计 `deferred` |
|---|---|---|---|
| 1 | 0 | **VM-CONCURRENT-GC-STRESS PASSED** | 0 → 2 → 24 → 26 |
| 2 | 0 | **PASSED** | 0 → 0 → 10 → 95 |
| 3 | 0 | **PASSED** | 0 → 2 → 2 → 12 |

⇒ **延迟暂停确实被触发**（deferred 计数增长）且**并发 GC 压测 3/3 全过**（零崩、零 UAF、token 和精确）
—— 兜底路径不是纸面论证，是被真实并发 GC 走过且安全的。

### 7.7 待办（S5 收口）→ **已于 §八全部完成**

---

## 八、S5 收口记录（2026-09-11 · 本机 8 核 16G）

### 8.1 交付清单

| 项 | 结果 |
|---|---|
| **C 轨自举证明** | `selfhost/bootstrap_prove.sh --fresh` → B.c == `golden/compiler.c`（**15060 行**）**逐字节一致** ✅ |
| **BC 轨自举证明** | `selfhost/bootstrap_prove_bc.sh --fresh` → `compiler_vm`（新链，runtime = M105 S2+S3）重放 dump == `golden/compiler.bc.dump`（**30581 行**）**逐字节一致** ✅ |
| **pxi 重链**（C 轨） | `tools/pxc build --c --full selfhost/interp.px` → **9,626,368 → 9,626,960 B**（+592）✅ |
| **pxi_vm 重链**（VM 轨） | `tools/pxc build --full selfhost/interp.px` → **9,460,200 → 9,460,784 B**（+584）✅ |
| tag | **`v0.2.0-m105`** |

**重链口径核实（先判轨、再重链，非盲改）**：

| 判据 | `bootstrap/pxi` | `bootstrap/pxi_vm` |
|---|---|---|
| `fn_*` 符号（C 轨生成函数） | **235**（`t fn_advance` / `t fn_cg_load_module` …） | 6（runtime 侧残留） |
| `s_bc_*` 字节码表（VM 轨） | 0 | **230**（`r s_bc_100` …） |
| 模块符号集 | quic 728 / h3 59 / stbi 146 / sqlite3 277 / ws 30 | **完全一致** |
| ⇒ 判定 | **C 轨 · 全能力** | **VM 轨 · 全能力** |
| 产物 | statically linked x86-64 ELF，`--version` = `pxi 0.2.0` | 同左 |

### 8.2 回归（全部跑在 M105 runtime 上）

| 门 | 结果 |
|---|---|
| `examples/m89_a2/vm_ab.sh v2`（38 例 VM vs 旧轨对拍） | **38 PASS / 0 GAP / 0 FAIL** ✅ |
| `selfhost/diffcheck.sh --all` | **rc=0**（lex/parse/codegen/value/interp 全绿；**跑在重链后的 pxi 上**）✅ |
| `examples/m89_s3d/verify.sh`（并发 GC ×3 / 生成器 / VM 堆回落 / 混合压测） | **9 PASS / 0 FAIL** ✅ |
| `examples/m93_s3/verify.sh`（帧协程 chan/mutex/rwlock/sleep + 并发 GC + C 轨逃生舱） | **6 PASS / 0 FAIL** ✅ |
| `examples/m96_s2/verify.sh`（daemon / 线程池上限收敛+空闲回收 / 逃生舱 / VM 重建） | **8 PASS / 0 FAIL** ✅ |
| `examples/m103_s2d/verify.sh`（img 链路 + Go 对拍） | **rc=0**（512x384，px 13209B vs go_ref 13177B 同量级）✅ |
| 生态索引防漂移（CI 同款：`gen_ecosystem.px` + `gen_native_table.sh` → `git diff --exit-code`） | **无漂移** ✅ |
| 冒烟：C 轨 / VM 轨 `hello` 产物 + `pxi` / `pxi_vm` 解释 | **四方输出逐字节一致** ✅ |

### 8.3 S4 复评（保留为二期候选，本里程碑不承载）

S1 §2.5 指认的容器/字段写路径支配项 = `pthread_mutex_lock(g_gc_mu)` + `sigprocmask` 对；
**S3 已消掉其中 sigmask 一半**（实测 4,384,444 → 0），`mutex_lock` 仍在
（**2,932,655 次/负载**，S3 前后计数逐项不变）。
⇒ S4（`px_field_set` / `px_list_push` / `px_dict_set` 的 GC 锁粒度）**须在 S3 之上重测剩余占比再定**，
与 M104 §七二期候选（O5 内联缓存 / O6 计算跳转 / O8 超指令）合并排序。

### 8.4 里程碑结论

1. **达成且超额**：立项预期「1.4~2x、工期 1~1.5 周」；实测**真实负载 1.88~1.90x**，实际投入 ~1 天。
2. **「先量化、再动手」是决定性变量**：S1 量化表**否决**了立项时的第一直觉（字段/字典名解析缓存，
   实测全程仅 18M strcmp ≈ 0.1s），把火力对准真支配项（全局表线性扫描占 95% strcmp + 4.38M 次 sigmask 对）。
   若照立项想象推进，收益大概率 <1.1x（等于重演 M104）。
3. **成本结构刻画（两次实验合并）**：M104 证伪「分派层常数优化」→ M105 命中「名解析 O(n)→O(1) + 系统调用消除」
   ⇒ 本运行时的成本序：**动态值层（boxed `LXValue`）> 名解析/锁路径 > 分派层**。
4. **安全性纪律有效**：S3 引入协议级改动（GC 暂停协议）却零并发回归——真屏蔽分支原样保留（并发模式逐字等价）
   + 层栈保证嵌套判定唯一 + **对抗性验证**（/tmp 变体强制并发软屏蔽，逼出「延迟暂停」路径且 3/3 PASSED）。

