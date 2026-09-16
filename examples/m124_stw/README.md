# M124 · qg-issue 80 · STW 屏障自旋 → futex 阻塞等待（验收材料）

分支：`fix/m124-gc-stw-futex`（提交 `59ba749`，基于 `4127306` = 远端 `main` tip）

> **提交号/基线勘误**：早期材料写 `f0ce2bc`（amend 前的旧提交，`git reflog` 可查 `f0ce2bc → ba92b07 → 8de0a78 → 59ba749`）与基线 `93f114d`。
> `93f114d` 是更早的 M122 提交，其 `runtime.c` 与 `4127306` 差 74 行、四处 `sched_yield` 行号整体错位 5 行。
> 本补丁的上下文/行号以 **4127306** 为准（复现与 patch 应用都在该提交上做过）。
改动：`runtime/runtime.c`（+78 / −12，四处等待原语）；`m124_stw_futex.patch` 为同内容补丁。

## 1 复现：STW 自旋放大

`stw_stress.px`：N 个帧协程各自密集分配（字符串拼接 + `str(i)`），触发高频并发 GC；
主线程等 8 个协程返回后打印总迭代数与 iters/s（吞吐判据）。

```sh
./tools/px build examples/m124_stw/stw_stress.px
bash examples/m124_stw/probe.sh <label> ./build/stw_stress 8 60    # CPU + strace + perf
```

实测（8 worker，40s，同机同负载，未挂 strace）：

| 指标 | 4127306（base） | M124（本分支） | 变化 |
|---|---|---|---|
| 有用吞吐 iters/s | 120,392 | 150,305 | **+24.9%** |
| 总 CPU（bash time）| user 51.4s + sys 133.6s = 185.0s | user 52.1s + sys 86.7s = 138.8s | **−24.9%**（sys −35%）|
| CPU 折核（/40s）| 4.60 核 | 3.45 核 | −1.15 核 |

`strace` 从进程启动即挂（避免 attach 抖动）：
`strace -f -c -e trace=sched_yield -o out.txt <bin> 8 15`
→ base **302,384 次**；M124 **输出文件 0 字节（0 次）**。清歌判据（<500/s）达成。

GC 行为（`PX_GC_DEBUG=1`，8 worker × 12s）：collect 轮次 base 20 / M124 18，
存活对象 266 / 266，`跳过` 0 / 0，单轮耗时 96ms / 81–116ms ⇒ 无回归。
`tools/gen_native_map.sh` 重生成 `native_mod_map.txt` 无差异（无新增/变更内建）。

放大模型（与生产实测吻合）：CPU 税 = 参与暂停线程数 × GC 时长 × GC 频率
（生产 3.7 次/s × ~96ms 级 × ~10 线程 ≈ 2~3 核，与 issue 80 §2.5 的 ~198% 同档）。

正确性门：
- `examples/m93_s2/verify.sh`：**6/6 PASS**（含 500 协程 × 高频分配 × PX_GC_THRESHOLD=4000）。
- `examples/m89_s3d/verify.sh`：步骤 1–4 **PASS**（含 3 轮并发 GC 压测 + 堆回落）；
  步骤 5 失败为**既有**编译错误（`vm_conc_gen_stress.px`：E3002 对不可变变量 'v' 赋值），
  与 runtime 无关（本次未改编译器与该用例）。
- `examples/m110_s2/run.sh`：serve/cli 放大倍率 —— M124 单发 1.50× / 1.62×，**base 4127306 对拍 1.31× / 1.31×**。
  单发噪声大（`alloc_loop` 仅 ~15ms 量级，±3ms 量化 ⇒ 比值 ±20%）；各 5 发采样：
  base `alloc` 0.88–1.38× / `scan_core` 1.03–1.26×，M124 `alloc` 0.89–1.38× / `scan_core` 0.85–1.22×
  ⇒ 两树同档，**无回归**成立（此前只写 1.57×/1.21×，是单发高值，无基线不可判）。

## 2 附带修复：暂停等待的「5 秒兜底」实为 ~1000 秒

`gc_stop_world` 暂停等待原为 `spins > 5,000,000` 且每轮 `nanosleep(200us)` ⇒ 实际 ≈1000s
（注释写 5 秒）。现改为 `gc_mono_ms()` 时钟预算，行为与文档一致（GC 卡住不再拖到 16 分钟）。

## 3 P2 排查中发现：既有 SIGSEGV 最小复现（与本次改动无关）

`shortid_probe.px` / `srvloop2.px`：serve handler 内做 short_id 形态字符串循环，
主线程用 `http_unix` 自环请求。**base 与 M124 均 3~5s 内 SIGSEGV**（`rc=139`）。

```sh
./tools/px build examples/m124_stw/srvloop2.px && ./build/srvloop2 300
# → 崩点 __vfprintf_internal / __strlen_avx2；core：coredumpctl info
```

窄化矩阵：

| 变体 | 结果 |
|---|---|
| handler 只返回 "ok"，3000 次请求（`srvloop.px`） | 不崩 |
| handler 内 5000 次 short_id 循环，300 次请求（`srvloop2.px`） | **崩**（base ~200 轮 / M124 ~25 轮）|
| `PX_GC_THRESHOLD=999999999`（等价关 GC） | 不崩（300 轮跑完）|
| `PX_GC_THRESHOLD=100`（近乎每次分配都 GC） | 不崩（300 轮跑完）|
| 分配风暴协程 churn=0（`shortid_probe.px 8 0`） | **仍崩** |

⇒ 与「分配风暴」无关，与 GC **开启且按默认阈值**有关；崩点为读已回收/已 munmap 的
字符串对象（`__vfprintf_internal`、`__strlen_avx2`），指向并发 GC 根集漏标窗口，而非
「同一协程被 ≥2 worker 并发执行」。

## 3.2 崩点取证（qg-issue 81 一手证据）：**栈边界越界**，不是堆上读已回收对象

对 core 做「落点 `si_addr` × 该线程栈映射区间」比对 —— 栈区间取 `readelf -lW <core> | awk '$1=="LOAD"'`
（core 被 systemd 截到 1GB 时**栈段不在文件里，但程序头里的地址区间仍在**，这是 `eu-stack` 拿不到的），
落点取 `gdb -ex 'p/x $_siginfo._sifields._sigfault.si_addr'`：

| 版本 | 崩点 | si_addr | rsp | 该线程栈映射下界 | 相对位置 | si_code |
|---|---|---|---|---|---|---|
| M124 | 主线程 `__vfprintf_internal+27`（`mov %rsi,(%rsp)`）| 0x7ffe2c1f1f30 | 同 | 0x7ffe2c1f2000（映射 8MB）| **下界之下 0xD0** | 1 = SEGV_MAPERR |
| base 4127306 | 主线程 `__vfprintf_internal+27` | 0x7ffe20057ff0 | 同 | 0x7ffe20058000（8MB）| 下界之下 0x10 | 1 |
| M124，`ulimit -s 65536` | 主线程 `__vfprintf_internal+27` | 0x7ffc85076fa0 | 同 | 0x7ffc85077000（**已长满 64MB**）| 下界之下 0x60 | 1 |

- 12 个 core 的落点分布：主线程 `vfprintf+27` 栈越界 8 个（base/M124 都有）；其余 4 个在非主线程
  （`xmalloc` / `_IO_default_xsputn` / `__libc_recv`）。
- 稳态下主线程栈映射**只有 132KB**（10ms 采样 `/proc/PID/maps [stack]` 全程不变），崩前才长到 RLIMIT 满值
  ⇒ **瞬时跑满，不是慢慢耗尽**；崩点固定落在 printf 家族（`vfprintf` 一次 push ~0x550B 大帧，是栈触界的第一处写）。
- 把 `str(r)`（响应 dict 的**递归**渲染）换成浅打印（`srvloop3.px`）：崩率 4/6 → 2/6（另一个 rc=134 SIGABRT）
  ⇒ 该递归渲染是**主触发**，但不是唯一路径。
- **判据修正**：若真是「读已回收堆对象」，落点应是**堆地址**且 PC 在 `__strlen_avx2` 读侧；实测三次全是
  **栈下界之上（更低位）的写**、`si_code=SEGV_MAPERR`。这与「GC 开才崩、关 GC（阈值 999999999）干净」并不矛盾：
  漏根使对象图出现悬垂/自环，递归渲染/遍历一路走到底直到爆栈（`gc_mark_obj` 是显式栈 DFS，不背这锅，
  锅在**渲染/遍历侧的递归**）。崩率为 4/6，不是每次必崩 —— 与 gdb 下时序被抹平、跑不崩可互相印证。

## 3.1 P2 探针实测（两个方向都测了）

**① 帧槽写回丢失 / 循环不收敛 —— 未复现。** `p2_probe.px`：8 协程各把 n=961 的补零循环
跑 60,000 次（叠加分配压力抬高 GC 频率），断言每次循环次数 g==1、结果 s=="0961"：

```
coro0..coro7  iters=60000 gmax=1 first= bad=0
=== p2_probe done: coros=8 total_bad=0 verdict=true ===
```

⇒ 在本机 M:N 调度（含 M94-S2 抢占让出 + 跨 worker 迁移）下，「让出/抢占后帧槽写回丢失」
不可复现。此结论与 §3 的崩溃复现方向（根集漏标，而非帧槽丢写）一致。

**② 暂停线程落在哪里（清歌判据②）** —— `where.sh` / `stw_where.sh`（eu-stack 采样）：

| 版本 | 被暂停线程栈顶 | 解释 |
|---|---|---|
| base | `#0 __sched_yield` / `#1 gc_stop_handler` | 信号处理器内**内核态自旋**（与 issue 80 §2.4 一致）|
| M124 | `#0 syscall`（futex）/ `#1 gc_stop_handler` | 处理器内**阻塞在内核**，不再烧 CPU |

## 4 文件

| 文件 | 用途 |
|---|---|
| `stw_stress.px` | 自旋放大复现 + 吞吐判据（N 协程 × 秒数）|
| `measure.sh` / `probe.sh` / `probe2.sh` / `single.sh` | CPU/strace/perf 测量脚本 |
| `shortid_probe.px` | P2 卡死探针（3s 未退出即报 HANG）+ 崩溃复现 |
| `srvloop2.px` / `srvloop.px` / `srvloop3.px` | 崩溃最小复现 / 对照组（只打 int，不崩）/ 浅打印变体（崩率 4/6→2/6）|
| `m124_stw_futex.patch` | 本次改动补丁 |
