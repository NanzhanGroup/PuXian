# M105_PLAN —— 运行时热点路径重构（全局表名解析 O(1) 化 + GC 同步瘦身）

> 上游依据：`docs/M104_PLAN.md` §7.3 归因 ——「M104 证伪了『优化解释循环分派可拿到 1.2~1.5x』的假设，
> 真瓶颈在 runtime 侧的名解析与锁路径」。M105 承接该结论，**先量化、再动手**。
>
> 修订纪律：S1 未出量化表前不动实现；量化表否定的子项立即降级（不许按立项时的想象推进）。

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
