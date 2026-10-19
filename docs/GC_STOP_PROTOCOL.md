# GC 暂停信号与互斥锁的协作协议（M312 建立）

> 一句话：**临界区里要屏蔽 `SIG_GC_STOP`；解除屏蔽必须在放锁之后。**
> 反序（`gc_unblock_stop()` → `pthread_mutex_unlock()`）会留下一个「仍持锁却已恢复信号」的窗口，
> 暂停信号恰在此刻递达 ⇒ 线程**持着锁**进暂停 ⇒ GC execp 等这把锁 ⇒ **整条服务被按住到秒级**。
> 生产实测：Mahesvara 8 并发 keep-alive 偶发 **p95 12822ms / max 12933ms**（晨曦 2026-10-18 报障）；
> 本机最小复现 **12146ms**，修后 **34ms**（358×）。

---

## 1 为什么临界区要屏蔽 `SIG_GC_STOP`

并发 GC 用 **stop-the-world**：executor 向每个已注册线程发 `SIG_GC_STOP`，线程在信号处理器里
保存 `ucontext` 并阻塞等待本轮结束（`gc_stop_handler`）。

如果线程**正持有一把 GC 标记期也需要获取的锁**时被暂停：

```
GC execp:  ...需要 g_xxx_mu...
线程 T  :  持有 g_xxx_mu —— 但已被暂停，永远不会释放
⇒ GC execp 等这把锁（无超时）
```

⇒ 因此这些临界区在**进入后**立即 `gc_block_stop(&old)`（屏蔽暂停信号），
使暂停延迟到临界区退出后递达。

**代价**：屏蔽期间 executor 的握手会看到该线程「未暂停」，因此保留 **5 秒兜底**（超时降级跳过本轮）。
即：屏蔽窗口本身也要短（临界区内不做重活、不阻塞 IO）。

---

## 2 反序陷阱（缺陷 535，本轮根因）

错误写法（修前 25 处）：

```c
pthread_mutex_lock(&g_hpend_mu);
gc_block_stop(&old);
... 临界区 ...
gc_unblock_stop(&old);          // ← 恢复信号：pending 的 SIG_GC_STOP 立刻递达
pthread_mutex_unlock(&g_hpend_mu);   // ← 但此刻仍持锁！
```

正确写法：

```c
pthread_mutex_lock(&g_hpend_mu);
gc_block_stop(&old);
... 临界区 ...
pthread_mutex_unlock(&g_hpend_mu);   // ← 先放锁
gc_unblock_stop(&old);               // ← 再恢复信号（此时被暂停也安全：不持锁）
```

### 实测证据（M312 · 本机 · 8 并发 keep-alive · `PX_GC_THRESHOLD=5000`）

修前（任意一次 GC 撞上窗口即整条卡住）：

```
[px-gc-stats] slow #1 total=12146.5ms lock=0.0 stop=0.7 mark=12143.4 sweep=1.9 resume=0.6
[px-gc-stats]   corosub_ms coro=0.0 http_pend=12142.5 pxserve_pend=0.0 coros=3
[px-gc-stats]   hpend wait_lock=12214.0ms scan=0.0ms cap=256 active=5
M312-LOAD conc=8 secs=12 req=128 rps=10.3 p95=12136.85ms max=12169.72ms
```

修后（同一装置，4 轮）：

```
M312-LOAD conc=8 secs=6 req=3670 rps=611.0 p50=12.35ms p95=23.58ms max=60.69ms
M312-LOAD conc=8 secs=6 req=4343 rps=723.1 p50=10.85ms p95=17.29ms max=35.29ms
M312-LOAD conc=8 secs=6 req=4531 rps=754.3 p50=10.26ms p95=17.13ms max=41.40ms
M312-LOAD conc=8 secs=6 req=4538 rps=755.2 p50=10.29ms p95=17.42ms max=37.14ms
（无任何 >100ms 的 GC 轮次）
```

### 为什么恢复时间是「秒级」而不是永久死锁

被暂停线程的 handler 有 **兜底**（`300 × 20ms ≈ 6s` 后强制恢复）；负载下实测单次 **12s**
（两轮超时叠加 / futex 唤醒延迟）。⇒ 表现是「偶发多秒停顿」而不是挂死，**这正是它长期没被定位的原因**：
从线程栈上看只有「很多线程在等锁、一个在 GC」，看起来像「GC 太慢」或「握手卡住」。

⚠️ **缩短这个兜底不是解法** —— 它会让「真正的暂停」提前失效（宁可漏回收也不能丢一致性）。
正解是**不让窗口存在**。

---

## 3 GC 标记期会获取的锁（必须遵守本协议）

| 锁 | 谁在标记期拿 | 修前顺序 | 现状 |
|---|---|---|---|
| `g_hpend_mu` | `http_pend_gc_mark()` | ❌ 反序 ×5 | ✅ 已改正序 |
| `g_pxpend_mu` | `px_pxserve_pend_gc_mark()` | ❌ 反序 ×13 | ✅ 已改正序 |
| `g_gc_mu` | `px_gc_collect()` 本体 | ❌ 反序 ×12 | ✅ 已改正序 |
| `g_slab_mu` | `slab_*`（sweep 期） | ✅ 正序 ×3 | 本来就对 |
| `g_coro_mu` | `px_coro_gc_mark_roots()` | ✅ block→lock / unlock→unblock | 本来就对 |
| `g_pinned_mu` | `gc_mark_pinned()` | ⚠️ **无屏蔽**（见 §5） | 未纳入 |
| `g_globals_mu` | 根1 扫描 | 在 STW 之前获取 | 未纳入 |

**判据**：任何「在 `px_gc_collect` 的 mark/sweep 期间被 executor 获取」的锁，
其**所有**临界区都必须 `lock → block ... unlock → unblock`。

---

## 4 写新代码的检查清单

1. 这把锁会不会在 GC 标记期被 executor 拿？→ 查 §3 表 / 看是否在 `*_gc_mark*` 里出现。
2. 若是 ⇒ 临界区必须 `gc_block_stop()`。
3. **解除顺序**：先 `pthread_mutex_unlock()`，再 `gc_unblock_stop()`。
4. 临界区内**不做**可能阻塞的事（IO、等别的锁、分配大内存）——
   屏蔽期间 executor 的握手会等它（最多 5s 兜底）。
5. 持锁时**不要**调用可能分配大量内存或触发 GC 的函数（锁序问题，M261/M262 已有专门守卫）。
6. 新增/修改后跑 `selfhost/check_gc_lock_order.sh`（静态守卫）+ `examples/m312_gc_lock_order/`。

---

## 5 覆盖边界（如实登记）

- **`g_pinned_mu`**（`gc_mark_pinned`）：`px_pin_obj` 侧**没有** `gc_block_stop`；若在临界区内被暂停，
  executor 会等它。当前 `px_pin_obj` 临界区极短（一次数组写入），**未观测到实际停顿**；
  但这是**同类风险**，登记为下一轮候选。
- **`g_globals_mu`**：在 STW **之前**获取（`pthread_rwlock_wrlock`），语义不同 —— 它靠「先拿锁再暂停」
  避免死锁；本轮未改。
- **C 轨产物（`--c`）与单线程路径**：`g_active_threads == 0` 时无 STW，本协议不适用
  （但仍建议保持同样的书写顺序，以免路径切换时踩雷）。
- **非本仓的锁**（应用层自建、`pthread_mutex` 在 `.px` 里拿不到）不在覆盖范围内。
- 观测开关 `PX_GC_STATS` 的诊断开销（每轮 ~10 次 `clock_gettime`）**只在开启时**发生；
  默认关闭时只多一次可预测分支。

---

## 6 相关

- 观测项：`PX_GC_STATS=1` · `PX_GC_SLOW_MS=<ms>`（默认 100）· `PX_GC_STATS_INTERVAL=<s>`
  输出前缀 `[px-gc-stats]`（见 `gcs_dump` / `gcs_report_run`）。
- 门：`examples/m312_gc_lock_order/`（静态 + 动态复现 + 负控）
- 历史：M124（futex 等待）· M211（信号处理器保 errno）· M258（根栈延迟收缩）· M310（两值原子入根）
