# M110 计划书 · Issue 38（serve 内分配 13× 放大）+ Issue 34 附页2（交替必备字面核心预筛）

> 立项：2026-09-12 · 现场：`/data/qg-issue/38-puxian-serve-worker-regex-amplification/ISSUE.md`（清歌）
> · `34-puxian-regex-throughput/34-ADDENDUM-2-20260912-清歌.md`
> 根因定位（东月，前置已完成）：`38-根因定位-东月-20260912.md`
> 基线 HEAD：`99d3517`（m111）

## §0 一句话

服务进程内**每一对 `xmalloc`/`xfree` 付 2 次 `rt_sigprocmask` 系统调用**（`gc_block_stop` 的软屏蔽快路径被
`g_active_threads == 0` 门控，而 `px serve` 池 worker 常驻注册 ⇒ 门控恒不成立）⇒ 单次 64KB 正则扫描
**1,053,631 次**系统调用、`sys` 占墙钟 56%。同一份代码在 CLI 单线程进程里 83ms，在 serve 进程里 **1117ms（13.5×）**。

两个互相独立的修法（**都要做**，各自只解决一半）：

* **S1 = 少做无用功**：交替「必备字面核心」整串预筛 —— 现网 `secret_patterns[1]` 是**无公共前缀的大交替**，
  M107-S1b 的前缀预筛对它取不到前缀 ⇒ 回落「逐起点 rmatch」，1MB 干净文本 1.54s。
* **S2 = 不再付系统调用**：软屏蔽常开 + **协作式安全点**（临界区出口主动上报暂停），
  使软屏蔽与「真屏蔽」**语义等价**（真屏蔽下信号也恰在解除屏蔽处投递）。

## §1 现状与量化（S0）

### 1.1 复现（本机 HEAD runtime 现编，`taskset -c 3`，64KB 干净文本，现网 p1 原文）

| 测量点 | regex_find | alloc_loop（2 万次拼接） |
|---|---|---|
| CLI 主线程 | **83 ms** | 15 ms |
| serve 池 worker | **1117 ms（13.5×）** | 53 ms |
| serve 进程**主线程** | **1092 ms（13.2×）** | 50 ms |

⇒ 与「哪个线程」无关，与「进程是否多线程」有关（修正 Issue 38 原文表述）。

### 1.2 资源账

`strace -c`（仅主线程）：单次 64KB 扫描 = **1,053,631 次 `rt_sigprocmask`**（≈52.7 万对分配/释放）；CLI 同一工作 **0 次**。

### 1.3 排除项（前置报告 §4，本轮沿用）

`PX_SERVE_WORKERS=2`（nlwp 266→12）不变 · 不绑核不变 · `PX_GC_INLINE`/`PX_GC_THRESHOLD` 三项不变 · 线程数扫描 K=0→200 不变。

## §2 S1：交替「必备字面核心」整串预筛

### 2.1 病灶

`runtime.c:5354 rn_lit_prefix` 对 `RN_ALT` 取**所有分支的公共前缀**；`(api|apikey|access|…|auth)` 无公共前缀
⇒ 返回 0 ⇒ `rsearch_from` 退回 `for (s=start; s<=len; s++) rmatch(root, s)`（1MB = 100 万次 rmatch，
每次约 8 次分配 ⇒ 52.7 万对系统调用）。

### 2.2 修法（保守扩张，逐类可证）

新增 `rn_necess_core(n, out)`：算出一组**可证明必出现于匹配文本中**的字面串（并集语义）。

| 节点 | 核心 |
|---|---|
| `RN_CHAR` | {该字符} |
| `RN_GROUP` | child |
| `RN_REP` | `min ≥ 1` → child；`min = 0` → **无**（可不消费） |
| `RN_SEQ` | **首个能给出核心的 kid**（该 kid 必匹配 ⇒ 其核心必出现） |
| `RN_ALT` | **每个分支都能给出核心**时取**并集**；任一分支给不出 ⇒ 整条放弃预筛 |
| `RN_ANY` / `RN_CLASS` / `RN_START` / `RN_END` | 无 |

优先复用 `rn_lit_prefix`（最强且长度自然截断到 `RN_CORE_LEN`；**截断仍保持必要性**）。
含 NUL、超长、超过 `RN_CORE_MAX` 项一律**弃用**（保守）。

`rsearch_from`：前缀不可用时，先做一次 `rn_find_any_lit`（首字节位图 + 逐串 memcmp）扫描；
**一个都不出现 ⇒ 直接返回「无匹配」**。只跳过、不改任何匹配结果。

### 2.3 红线

* 只做「不成立即整段跳过」，**不改变任何匹配结果**（对 `find`/`match`/`search`/`find_all`/`replace`/`split` 一致生效）；
* 组捕获、零宽匹配、`^`/`$` 语义不变（核心只用于「整串不出现 ⇒ 必不匹配」这一必要条件）；
* 不引入新 API、不改 `regex_find*` 返回类型。

## §3 S2：软屏蔽常开 + 协作式安全点

### 3.1 病灶

```c
// runtime.c:1355
if (g_active_threads == 0) { g_gcs_skip = 1; g_gc_crit = 1; return; }   // 免系统调用
g_gcs_skip = 0; pthread_sigmask(SIG_BLOCK, &set, old);                  // 每次分配 2 次系统调用
```

门控存在的原因不是正确性，而是**停顿及时性**：软屏蔽下信号处理器只做「延迟暂停」，
对「连续海量临界区」的线程可能一直抢不到 `g_gc_crit == 0` 的窗口 ⇒ 拖到 5s 兜底
（`g_gc_skips++`，**跳过本轮 = 漏扫**）。

### 3.2 修法：协作式安全点

`gc_unblock_stop` 在 `g_gc_crit` 清零后立即检查「本轮是否在等本线程暂停」，若是则**就地暂停**
（复用信号处理器的暂停体：保存 `vm_state`/`roots`/`tmp_root`/`uc`、`ti->epoch = g_gc_epoch`、
`paused = 1`、自旋至本轮回落）。

**等价性论证（关键）**：真屏蔽下，`pthread_sigmask(SIG_SETMASK, old)` 返回的**同一位置**内核投递挂起信号，
处理器在**解除屏蔽点**执行暂停体。协作式安全点在**同一个解除屏蔽点**执行同一个暂停体
⇒ **暂停时机、暂停点、根面快照完全一致**。故「软屏蔽 + 协作安全点」≡「真屏蔽」。

**不会自锁**：`gc_unblock_stop` 位于 `pthread_mutex_unlock(&g_slab_mu)` 之后 ⇒ 暂停时不持 slab 锁；
`g_globals_mu` 亦不可持有（executor 在 stop 前先取写锁，持读锁的线程必已跑完）。executor 自身由
`g_gc_executor` 自检排除。

### 3.3 红线

* `g_gc_crit` 的 block/unblock **严格 LIFO 配对**不变（内层直接返回）；
* 逃逸阀 `PX_GS_HARD=1` 强制旧「真屏蔽」路径（A/B 与回滚）；
* 现有 `atfork` 复位、嵌套告警、`g_gc_deferred` 观测保持；
* **GC 语义零变化**：mark/sweep/根面/兜底全不动，只把「等信号」换成「临界区出口自查」。

## §4 切片

| 片 | 内容 | 风险 |
|---|---|---|
| S0 | 量化定标 + 复现台（本文件 §1） | 低 |
| S1 | `rn_necess_core` + `rn_find_any_lit` + `rsearch_from` 预筛 | 中（正则语义） |
| S2 | 软屏蔽常开 + `gc_pause_if_requested` 协作安全点 + `PX_GS_HARD` 逃逸阀 | 中高（GC 暂停协议） |
| S3 | 收口：闸门 + tag + 交付 | 低 |

## §5 验收

| 判据 | 目标 |
|---|---|
| V1 | `examples/m110_s1`：p1 1MB 干净文本 `regex_find` **< 50ms**（改前 1541ms） |
| V2 | V1 语义：命中/未命中、组捕获、`find_all`/`replace`/`split` 结果与改前**逐字节一致** |
| V3 | `examples/m110_s2`：serve 内 64KB `regex_find` 与 CLI 基线**同档**（改前 13.5×） |
| V4 | GC 语义：`m89_s3d` / `m93_s3` / `m96_s2` / `m103_s2d` / GC 压力套件 **0 失败** |
| V5 | 回归：`diffcheck --all` rc=0 · `vm_ab` 0 GAP · 生态索引无漂移 · p3_regex 双模式 |
| V6 | 双自举逐字节（C 轨 `B.c == golden` / BC 轨 `dump == golden`） |
| V7 | 可观测：`g_gc_skips` 不增长；`PX_GS_HARD=1` 与默认路径结果一致 |

## §6 不做（本轮）

* 分配器「线程局部自由链」（38-A 的另一形态）——S2 已把临界区成本降到 0，收益重叠；
  若 S2 后 `g_slab_mu` 竞争成为新瓶颈再单独立项（需多核压测背书）。
* 换开 `MBEDTLS_THREADING_C` 去握手锁串行化（M108 二期，与本题正交）。
