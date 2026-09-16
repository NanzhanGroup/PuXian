# M126 · 信号处理器隔离点 + 纳入并发 GC（qg-issue 83）· 验收材料

改动：`runtime/runtime.c`（+124 / −4）
例子：`examples/m126_signal_isolate/{sig_isolate.px, verify.sh}`（本门 **42/42 全绿**）

## 1 病灶（同一线程上的两个缺陷）

`runtime/runtime.c` 的 `sig_dispatch_thread`（self-pipe + 专用分发线程）此前：

```c
if (h.type == PX_FUNC || h.type == PX_NATIVE) {
    LXValue arg = px_int(sig);
    px_call(h, &arg, 1);          // ← ⓐ 无隔离点   ⓑ 线程未注册进 g_threads
}
```

| | 缺陷 | 后果 |
|---|---|---|
| ⓐ | 处理器内任何运行时错误 / 分配失败走「无隔离点 → 保留致命语义」= `exit(1)` | **一个信号带走整个进程**。生产实证：mahesvara 的 SIGHUP reload 处理器（`main.px:318`）一错，8 站点同进程一起死 —— journal `00:40:53 / 00:42:24 / 00:42:43 / 00:43:35 / 00:43:46` 三分钟内 5 次重启 |
| ⓑ | 该线程**从未注册进 `g_threads`**（`px_gc_thread_enter` 此前只有 spawn / 协程 worker / H3 连接线程在用） | 并发 GC 既不暂停它、也不标它的 `ti->vm_state`（GC 根4 只遍历已注册线程）⇒ **处理器持有的普贤对象被回收** = UAF |

ⓑ 的实测崩点（gdb，`FX=storm` + `PX_GC_THRESHOLD=100`，修复前 4/4 复现）：

```
#0 px_add   #1 vm_run_loop   #2 px_vm_entry   #3 sig_dispatch_thread
```

## 2 修法

```c
px_gc_thread_enter();                       // ⓑ 纳入 GC：注册窗口只覆盖处理器执行期
(void)px_sig_isolate_begin();               // ⓐ 隔离点（kind = 2）
if (!px_sig_isolate_errored()) px_call(h, &arg, 1);
int errored = px_sig_isolate_errored();
px_sig_isolate_end();
px_gc_thread_leave();
if (errored) px_sig_err_note(sig);          // [px-signal] 行
```

- **隔离点种类**（新增 `t_isolate_kind`，只用于**失败文案精确化**，生死判定仍只看 `g_err_jmp_set`）：
  `1` = 协程/spawn（请求级 → 5xx）· `2` = 信号处理器（本次处理器调用级）· `0` = 无隔离点（启动期/GC → 保留致命语义）。
- **退出码有意不同**（与 spawn 隔离**刻意区分**）：信号处理器失败是「服务侧已处理的**生命周期事件**」（对齐 nginx：reload 失败保留旧配置、服务不受影响），**不计入退出码**；否则长跑服务只要历史上有过一次 reload 失败，systemd 就会在**计划内**重启时判 `failed`（与 qg-issue 72 重启风暴同源）。
  证据主渠道 = stderr `[px-signal]` 行 ＋ 进程退出时的「收尾提示」行（可被监控 grep）。
- **逃生舱**：`PX_SPAWN_ISOLATE=0` → 回退 `exit(1)` 致命语义（与其它隔离点一致）。

## 3 门（`verify.sh`，42/42）

```sh
bash examples/m126_signal_isolate/verify.sh
```

| 用例 | 注入 | 判据 |
|---|---|---|
| A | `FX=ok` | 正控：处理器确被调用并跑到结尾（否则后面全是假绿） |
| B | `FX=abort`（处理器内 assert 失败） | 进程存活 + `/health` 200 + 只终止本次调用 + 退出码 0 |
| C | `FX=huge`（处理器内 `read_file` 100 GB 稀疏文件） | 同上，且文案带分类真因 + `errno` + 「处理器调用级」 |
| D | `FX=inject`（`PX_ALLOC_FAIL_MIN=8388610`） | 同上；且**只打中处理器**、未波及运行时内部 |
| E | `FX=storm`（处理器内 5000 次分配并持有 + 后台风暴 + `PX_GC_THRESHOLD=100`） | 无 SIGSEGV（ⓑ 修复） |
| F | `PX_SPAWN_ISOLATE=0` 负控 | **必须仍 `exit` 非 0**（证明隔离不是「永远吞掉」） |
| G | 连发 3 个信号 | 计数递增到「第 3 次」、进程持续可服务、退出码 0 |
| H | 无信号自然退出 | 退出码 0 且**无**隔离痕迹（断言非恒真） |

### 阈值标定（D 用例的 `8388610` 不是随手取的）

实测扫过 5 个 `PX_ALLOC_FAIL_MIN` 阈值（handler 载荷 = 8388609 字节，`read_file` 申请 8388610）：

| 阈值 | handler 隔离 | 运行时内部 | `/health` | 进程 |
|---|---|---|---|---|
| 未设 / 16777216 | — | — | 200 | 自然退出 rc=0 |
| 6291456（6 MB） | ✅ 成功隔离 | ❌ **内部 8 MB 分配被打掉** | **不通** | **假死**（120 s 不退出） |
| 8388608（8 MB） | ✅ | ❌ **启动期 8 MB 分配失败 → 绑定端口失败** | 不通 | 假死 |
| **8388610** | ✅ | ✅ 无波及 | **200** | **自然退出 rc=0** |

⇒ 运行时**服务路径内部存在 8 MB 级分配**（协程栈）。阈值必须精确落在两者之间。

## 4 ⚠️ 附带发现（**未闭环，建议立 M127**；不影响本门结论）

阈值取 6 MB 时复现出一条**比 `exit(1)` 更糟**的路径：运行时内部 8 MB 分配失败 → 协程被隔离终止 → **进程既不服务也不退出（永久假死）**。

gdb 两处现场（同一进程，`HOLD_MS=120000` 到期仍未退出）：

```
主线程 #0 __lll_lock_wait  #1 pthread_mutex_lock  #2 gc_register  #3 px_str_len  #4 vm_run_loop   ← 等锁
另一轮 #0 syscall  #1 gc_stop_handler  ← STW 暂停路径上卡住
```

这与 M125/M126 注释里写明的边界一致：**`longjmp` 不展开 pthread 锁** —— 失败若发生在持锁临界区，锁永久遗留。差别在于：
**旧行为 = 快速 `exit(1)`（systemd `Restart=always` 拉起，中断 ~3 s）；新行为 = 假死（进程还在，systemd 不管，服务永久不可用）**。

**诚实边界**：① 触发源是**测试钩子**（生产未设 `PX_ALLOC_FAIL_MIN`）；② 这是 **M125 引入的语义换挡**（`exit(1)` → 隔离），非 M126 特有；③ **未证明**具体是哪把锁被遗留 —— gdb 读不到 mutex 类型（二进制该 TU 无调试符号），`gc_register` 等锁 / `gc_stop_handler` 卡住是**两组现场观察**，不是同一次的证据链。

**建议（M127）**：隔离点捕获后检查运行时锁状态；若无法安全回滚，**宁可退回 `exit(1)`** —— 「让 systemd 拉起」优于「假死无人知」。

## 5 复跑

```sh
bash examples/m126_signal_isolate/verify.sh          # 42/42
bash examples/m125_alloc_fail/verify.sh              # 11/11（回归：M125 语义零漂）
./selfhost/rebake_bin.sh --check-all                 # 入库二进制指纹门（改 runtime ⇒ 必重烘）
```
