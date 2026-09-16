# M127 · 隔离点回卷的锁安全审计（qg-issue 84）· 验收材料

改动：`runtime/locktrack.h`（新增）· `runtime/runtime.c`（+约 90 行，含审计与两个钩子）·
`runtime/{coro,runtime_ws,runtime_route,runtime_sqlite,runtime_quic}.c`（各 +1 行 include）·
`tools/px`（把 `locktrack.h` 纳入 rtcache 源清单与缓存键）
例子：`examples/m127_unwind_lock/{unwind_lock.px, verify.sh}`（本门 **33/33 全绿**）

## 1 病灶

M125/M126 把「运行时错误 / 分配失败」从**进程级**收紧到**请求级**（`longjmp` 回隔离点，HTTP 5xx）。
但 **`longjmp` 不展开 pthread 锁**：失败点若落在临界区内，回卷会把该锁**永久**留在「已锁」状态
⇒ 其他线程随后阻塞在 `pthread_mutex_lock` 上 ⇒ **进程既不服务也不退出**。

**假死比旧的 `exit(1)` 更坏**：进程还在 ⇒ systemd 认为健康（不会拉起）· 监控看不到 failed ·
只有外部探测才能发现「服务不再应答」。

**实测现场**（gdb 全线程栈，生产 mahesvara 崩溃窗口）：

```
#0 __lll_lock_wait  #1 pthread_mutex_lock  #2 px_dict_set  #3 vm_run_loop  #4 coro_worker
                                                    ^^^^^^^^^^ 持 g_gc_mu 分配
```

**具体机制（本次坐实，含调用链符号化）**：`px_dict_set` / `px_list_push` 在**持 `g_gc_mu`
期间**做可失败分配：

```c
pthread_mutex_lock(&g_gc_mu);
...
    o->as.dict.keys = xrealloc(o->as.dict.keys, sizeof(char*) * o->as.dict.cap);   // 可失败
    o->as.dict.vals = xrealloc(o->as.dict.vals, sizeof(LXValue) * o->as.dict.cap); // 可失败
    o->as.dict.keys[len] = xstrdup(key);                                          // 可失败（每次新键）
pthread_mutex_unlock(&g_gc_mu);
```

而**几乎每个 HTTP 响应**都要构造字典（响应头 / JSON body）⇒ 这条路径上任何一次分配失败
（真 OOM，或「尺寸算错」）都会**不是 5xx、也不是 exit(1)，而是整进程假死**。
`px_list_push`（`xrealloc`）、`gc_register`（对象表扩容 `xrealloc`）同型。

## 2 修法（两层）

**① 审计闸（本里程碑核心）**：所有运行时 pthread 锁的加/解锁**同时**记进「本线程持锁栈」
（TLS，O(1)、无分配、可在任意上下文调用）；隔离点在 `longjmp` 之前审计：

| 审计结果 | 处置 | 与旧行为的关系 |
|---|---|---|
| 无锁在身 | 照旧 `longjmp` → 请求级 5xx | 与 M125/M126 **完全一致**（绝大多数情况） |
| 有锁在身 | **不回卷**：打印持锁清单 + `_exit(1)`（systemd 3s 内拉起） | 消灭「假死」；比留锁更可观测 |

- 实现在 `runtime/locktrack.h`：`#define pthread_mutex_lock(m) px_lt_lock_((m), #m)` 等
  **宏接管**（因此无需改 216 处调用点；包装函数定义在 `#define` **之前**，故不自递归）。
  头文件必须作为该 TU 的**最后一个 include**。
- 审计函数 `px_unwind_lock_guard()` 在 `px_error` / `px_alloc_fail` 的 `longjmp` 之前调用。
- 逃生舱 `PX_UNWIND_LOCK_GUARD=0` → 回退「无条件 `longjmp`」（**门负控专用，生产勿设**）。

**② 确定性复现钩子**（默认关）：`PX_ALLOC_FAIL_IN_LOCK=<锁名子串>` —— 仅当**本线程此刻持有**
名字含该子串的运行时锁、**且处在隔离点内**时，才让这次分配失败。否则只能靠真 OOM 撞，
概率极低（且撞不到就没法验收）。

## 3 门 33/33（`examples/m127_unwind_lock/verify.sh`）

| 用例 | 判据 |
|---|---|
| A 正控（**无锁**在身失败） | 仍是请求级 5xx ＋ `/health` 200 ＋ **不**触发审计（无误伤）＋ 自然退出 rc=0 |
| B 关键（**持 `g_gc_mu`** 失败） | 进程 10s 内退出；文案含「无法安全回滚」＋持锁清单列出 `&g_gc_mu` ＋假死后果 ＋ `_exit(1)` ＋处理建议；**未走 longjmp** |
| C 负控（B ＋ `PX_UNWIND_LOCK_GUARD=0`） | **复现假死**：进程不退出（systemd 视为健康）＋ `/health` 无响应 ⇒ 证明「假死」真实存在、且正是审计拦住它 |
| D 无注入回归 | `/health`·`/plain`·`/grow` 全 200（`/grow` body = `objs=6000`）；无审计行、无运行时错误行；自然退出 rc=0 |
| E 启动期注入（无隔离点） | 文案区分「无隔离点 → 保留致命语义退出」＋ 未误报锁审计（M125 语义回归） |
| F 材料完整性 | `locktrack.h` 存在；记录逃生舱与边界；6 个持锁 TU 均已接入 |

诊断附注：B/C 命中的现场为 `px_dict_set` → `xstrlen`/`xrealloc`（`PX_ALLOC_FAIL_IN_LOCK`
的诊断行会打印**持锁栈深度与锁名**，避免「锁没记上」与「记上了但审计没跑」混淆）。

## 4 回归（同一提交实测）

| 套件 | 结果 |
|---|---|
| M126 门（信号处理器隔离） | **42/42** |
| M125 门（分配失败分类） | **11/11** |
| M120 门（dict 语义） | **31/31** |
| M123 门（断连提前收尾） | PASS（定量 0.14% ≤ 30%） |
| m93_s2（协程/线程/GC） | **6/6**（首次 5/6 系并跑负载下的 `pgrep` 竞争，单跑复现 6/6） |
| `selfhost/engine_parity.sh` | 通过（负例 30/30 · 正例 6/6） |
| `selfhost/rebake_bin.sh --check-all` | 改 runtime ⇒ 需重烘（本提交已重烘 14 件） |

性能：锁追踪为 TLS 计数（两次 TLS 读写/次加解锁），M123 定量门仍在红线内。

## 5 边界（如实，勿误读）

1. **审计是兜底，不是错误处理替代品**：有锁在身时进程仍会 `_exit(1)`（约 3s 中断）。
   「OOM → 请求级 5xx」在**持锁分配**的站点**尚未**成立 —— 见第 6 节（候选 M128）。
2. `pthread_cond_wait` 期间互斥量由 pthread 释放 ⇒ 追踪表在等待期间出栈（口径正确）；
   但「等待中被 `longjmp` 丢弃」会绕过 glibc 的 cond 组计数清理（罕见）。
3. 只覆盖 pthread 锁；自旋 / `_Atomic` 类自定义同步不在内（本仓 runtime 运行时锁均为 pthread 锁）。
4. 持锁栈深度 > `PX_LT_MAX`(24) 时只计数不记名（审计仍触发）。
5. 计数须**动态平衡**：既有「加锁后提前 return 不解锁」类缺陷会表现为审计假阳性
   （保守方向：宁可 `exit(1)`，也不留锁假死）。

## 6 后续（建议 M128，未做）

把**持锁分配**改成「先分配、后进临界区」或「先解锁、再报错」（对齐 M125 对 `g_slab_mu` 的处置）：
`px_dict_set`（`xrealloc`×2 ＋ `xstrdup`）· `px_list_push`（`xrealloc`）· `gc_register`（对象表扩容）。
做完后这些站点上的 OOM 才会真正落到**请求级 5xx**（而不是 `exit(1)`），也就是本源要的
「OOM 改请求级 500」。本里程碑的审计闸保证：**即使还没做完，也不会再出现永久假死**。

## 7 复跑

```bash
bash examples/m127_unwind_lock/verify.sh        # 33/33
# 只看关键两例（B 拦截 / C 复现假死）：
PX_ALLOC_FAIL_IN_LOCK=g_gc_mu ./examples/m127_unwind_lock/build/unwind_lock   # 触发后 _exit(1)
PX_ALLOC_FAIL_IN_LOCK=g_gc_mu PX_UNWIND_LOCK_GUARD=0 ./examples/m127_unwind_lock/build/unwind_lock  # 假死
```
