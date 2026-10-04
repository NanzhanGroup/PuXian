# PuXian IO 原语与 EINTR（M256 · 第 120 轮）

> **一句话**：并发 GC 的 stop-the-world 会给**每个活跃线程**发 `SIG_GC_STOP`；线程若正阻塞在
> 系统调用上，调用被**打断**并返回 `-1/EINTR` —— 而「被打断 ≠ 失败」，**重试即可**。

## 一 为什么 `SA_RESTART` 救不了（本文件存在的理由）

`gc_stop_handler` 装处理器时用了 `SA_RESTART`，注释写「被信号打断的系统调用自动重启」。
但 **`signal(7)` 明确列了一张「永不重启」清单**，其中两类正是本运行时的热路径：

| 类别 | 接口 | 本运行时的命中点 |
|---|---|---|
| 设了 `SO_RCVTIMEO`/`SO_SNDTIMEO` 的 socket | `read` `recv` `recvfrom` `accept` `connect` `send` `sendto` `write` | 用户面 fd **都设了**（`tcp_connect_ex(…,{timeout_ms})` / `tcp_opt(…,{read_timeout_ms})` / `px_serve` 的连接） |
| 永远不重启 | `poll` `select` `epoll_wait` `nanosleep` `usleep` `sigsuspend` | 事件循环 · TLS 握手等待 · QUIC · 各处退避 |

**实测证据**（`/tmp/m256/eintr_probe.c`，与 `gc_stop_handler` 同款 `sigaction`）：
`socketpair` + `SO_RCVTIMEO=5s` + 自送信号 ⇒ **`recv` 返回 `-1`，`errno=4`（EINTR）**。

## 二 唯一语义（`runtime/runtime.h` 的 `px_io_*` 族）

| 函数 | 语义 |
|---|---|
| `px_io_read/write/recv/send/recvfrom/sendto/accept` | 重试到**成功**或**真错误** |
| `px_io_connect` | 被打断时**不裸重试**（Linux 语义：连接仍在**异步**进行，再调 `connect` 返回 `EALREADY`）⇒ 改用 `poll(POLLOUT)` 等完成 + `getsockopt(SO_ERROR)` 取结果 |
| `px_io_poll` / `px_io_epoll_wait` | 正超时**扣掉已过去的时间**（否则每次被打断都重置整个超时 ⇒ 超时语义被无限拉长）；预算耗尽 ⇒ 按**超时**返回（`0`） |
| `px_io_sleep_ns/ms/us` | `nanosleep(&ts,&ts)` **续睡剩余**（POSIX：内核对第二参数写剩余时间） |

### 已知取舍（如实登记）

* **`SO_RCVTIMEO` 的计时会被顺延**：内核在**每次进入** `read` 时启动计时器，而 `EINTR` + 重试
  ⇒ 新一次 `read` ⇒ 新计时器。在**持续** GC 压力下，带超时的读可能比名义超时更晚返回。
  判据：**「宁可等，不可误判」** —— 与 M211 收口 `px_conn_read` 时的决策一致。
  正常负载下 EINTR 稀疏 ⇒ 实际影响可忽略；服务端连接的**空闲回收**另有事件循环 tick 兜底。
* **非阻塞 fd 不受影响**：`EAGAIN` 不是 `EINTR`，重试循环不进入。

## 三 静态守卫（`selfhost/check_eintr.py`）

不让「某处又忘了」再发生：

* **A 段（无例外族）**：`poll` / `select` / `epoll_wait` / `nanosleep` / `usleep` / `sigsuspend`
  —— 这些接口**永不**被 `SA_RESTART` 重启，**没有合法例外**。
* **B 段（用户面 fd 原语）**：所有 `bi_*` native 体内的 `read/write/recv/send/recvfrom/sendto/accept/connect`。

每个站点必须满足其一：① 已是 `px_io_*` 包装调用；② 调用点 ±8 行窗口内有 `EINTR` 判据；
③ 该行带 `PX_IO_EINTR_OK` **豁免标记（必须附理由）**。另有两条规模锚点（防「扫不到文件」
与「包装调用全没了 ⇒ 空集 ⊇ 任意集」）与 9 条判据自证。

**当前状态**：A 段 0 · B 段 0（唯一豁免：`px_futex_wait` 的 `nanosleep` —— futex 语义下
「被打断 = 提前返回，由调用方循环复检条件」，**不能续睡**）。

## 四 动态门（`examples/m256_eintr/`）

不加用户级重试的一次 `read(fd, 64)` + 一次 `udp_recv`，在 `PX_GC_STRESS=1 PX_GC_INLINE=1`
下必须与正常档**逐字节一致**：

| 档 | T1（read 5 字节） | U2（udp_recv 4 字节） |
|---|---|---|
| 正常 | `5` | `4` |
| 压力 | `5` | `4` |
| **修前压力档** | **`-1`**（误报读失败） | **`-1`**（null = 「没有包」，**静默丢包**） |

修前指纹（`/tmp/m256/exp4.px`）：`errno=4` · 用户级重试 **3881** 次 · 2560ms 后才读到数据
⇒ **瞬时打断**（重试能救回），**不是永久错误**。

**触发条件 = 两个开关同开**（实测矩阵 `/tmp/m256/matrix.log`）：

| 开关 | T1 | U2 |
|---|---|---|
| 无 | 5 | 4 |
| `PX_GC_STRESS=1` 单开 | 5 | 4 |
| `PX_GC_INLINE=1` 单开 | 5 | 4 |
| **两者同开** | **-1** | **-1**（3/3 确定）|

## 五 两个必须知道的工程事实

1. **门的时间预算要容纳「压力档的减速」**：首版把 `SO_RCVTIMEO` 设成 3s/6s，
   而压力档下 `sleep(1500)` 的唤醒会被 GC 拖长 ⇒ 数据真的比超时晚到 ⇒ 偶发
   `T1=-1`/`U2=-1` —— **那是 timeout，不是 EINTR**。修前 EINTR 是**立即**返回
   （与超时无关）⇒ 放宽到 20s **不影响复现力**，只去掉假红。
2. **`PX_GC_INLINE=1` 会撞上「缺陷 267 家族」**（并发/挂起执行流在该开关下容器被误回收；
   M207 登记、M208 定位为「构造 → 登记」窗口，**仍未修**）：本机实测 ~1/35 概率 SIGSEGV，
   core 栈 `xmalloc ← px_dict ← vm_run_loop`（主线程）+ 另一线程在 `px_gc_collect`。
   门的处置 = **被信号杀死时记数并重试 + 响亮打印命中次数**（不隐藏），判据仍以
   T1/U2/DONE 为准。**这不是把崩溃当绿** —— 267 有自己的账（`m207_gcstress` /
   `m208_gcroot` 两道门仍在看守）。
