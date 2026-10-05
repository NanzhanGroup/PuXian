# 持锁临界区内的**可失败分配**（Lock-Held Allocation）

> 建立于 M264；口径来自 M125 → M127 → M128 → **M260（缺陷 439）** → **M261/M262（缺陷 464）**。
> 判据工具：`selfhost/check_lock_alloc.py` · 基线 `selfhost/lock_alloc_baseline.txt`。

## 一 为什么它是一类**独立**的故障

M125/M126 把「运行时错误 / 分配失败」从**进程级**收紧到**请求级**：失败时 `longjmp` 回隔离点。

**但 `longjmp` 不展开 `pthread` 锁**。失败点若落在临界区内：

```
线程 A：lock(mu) → 分配失败 → longjmp 回隔离点   ← mu 仍是「已锁」状态
线程 B：lock(mu) → 永久阻塞
⇒ 进程既不服务也不退出（**假死**）
```

M127 的处置是**审计 + 只能 `_exit(1)`**（服务中断 ~3s —— 好过永久假死，但仍是中断）。

## 二 修法：**两阶段**

> **锁外备货（可失败 ⇒ 请求级 5xx；无锁在身 ⇒ 回卷安全）→ 锁内只发布指针（不可失败）**

M128 定下这条口径并修了 3 处；M260 修了漏网的 `px_str_int_pool`（缺陷 439）；
**M261/M262 把守卫首扫的 18 处收口到 2 处**（只剩两条有意保留的兜底）。

| 轮次 | 收口 | 备注 |
|---|---|---|
| M128 | 3 处 | `px_list_push` list_grow · `px_dict_set` dict_keys/vals_grow · `gc_register` gc_objs_grow **主路径** |
| M260 | 3 处（缺陷 **439**） | `px_str_int_pool` · `px_empty_str_get` · `px_pin_obj` |
| **M261**（缺陷 **464**） | 7 处 | `fserve_ensure`×2 · `px_conn_pend_put` · `px_pin_obj`（**死代码**）· `px_const_put` · `px_rate_limit_try`×2 |
| **M262** | 4 处 | `route_match`（段快照到栈）· `bi_sse_read_line`（栈快路径 + 锁外备货） |

## 三 判据（`selfhost/check_lock_alloc.py`）

两条互补的扫描面：

1. **直接分配**：锁区间内出现 `xmalloc|xrealloc|xcalloc|xstrdup|m128_alloc|m128_strdup`。
2. **一层间接**：锁区间内出现**必然分配**的构造入口（`px_str_len`/`px_str`/`px_dict`/`px_list`/
   `px_bytes`/`px_to_string`/`px_pin_obj` …）。⚠️ 表**不追求完备** —— 完整调用闭包会把几乎所有
   函数算进来 ⇒ 判据失效。

### 基线机制（**只减不增**）

| 情况 | 处置 |
|---|---|
| 实测 − 基线 非空 | **判红**（新增，必须当场处理或论证） |
| 基线 − 实测 非空 | 提示**收口**（不判红；`--update` 后欠账数只减不增） |

### ⚠️ 三条**判据自身**的坑（都实测踩过）

1. **键里含行号** ⇒ 上游**任何一处编辑**都让基线整体失配 ⇒ 每次报一堆**假新增**
   （实测：只改 `runtime.c` 就让 3 条基线项变成「新增」）。
   ⇒ 比较键 = **(文件, 函数, 被调, 类别)** 的**多重集**（`Counter`），**不含行号**；
   行号照旧写进基线（人工定位用）但**不参与比较**。
2. **跨行签名** ⇒ 只匹配单行的函数正则把
   `static int route_match(const char* method, …\n …) {` 整个漏掉 ⇒ 命中被**归到上一个函数名下**
   （`px_route_has`）⇒ 键错。（M212 缺陷 289 同族。）
3. `--update` 只写**一次**每个键 ⇒ 「同函数同被调 N 处」下次扫描变成 N−1 处假新增
   ⇒ 必须**按出现次数逐行写出**。

### `ACCEPTED:` 分类

基线行可带第 5 字段 `ACCEPTED:<理由>` = **有意保留**（打印为 ℹ️）。
**理由为空 ⇒ `rc=3`**（判据失效）—— 「无理由豁免」= 把红当绿。
`--update` **不会**自动添加 `ACCEPTED`，但会**保留**已写下的理由。

当前 2 条（都是**兜底/对照路径**）：

| 站点 | 为什么保留 |
|---|---|
| `gc_register` 的 `xrealloc` | 对象表扩容的**兜底分支**（主路径已是两阶段：锁外备好 `spare`、锁内只换指针）。去掉它等于让对象注册在 OOM 时**静默失败**（丢 GC 根 ⇒ 更糟的静默错值）。 |
| `px_list_push_locked` | `PX_GROW_RETRY_MAX=0` 的**对照/退路**实现（m128 门 C8/C11 用它做 A/B 对照）；删掉会让「旧行为对照」失去基准。 |

## 四 门

| 门 | 内容 |
|---|---|
| `examples/m261_lock_alloc/` | 静态锚点 20 条（含**反向**：被收口的旧形态必须 0 次）+ 守卫自证 7/7 + 基线账 + 行为冒烟（fserve/rate_limit）+ **负控 A/B/C** |
| `examples/m262_lock_alloc2/` | 静态锚点 21 条（route_match / sse / 缺陷 465）+ 基线账（待收口 **0**）+ **路由默认档 CTR=0 / 诊断档 CTR>0** + 负控 A/B |
| `examples/m128_unlock_grow/` | **动态**证明：「锁内分配真的导致 `_exit(1)`」（`PX_ALLOC_FAIL_IN_LOCK` 注入 + 锁审计行） |

> **分工**：M261/M262 的门是**结构契约**（静态 + 冒烟）；「真的会假死/中断」的动态证明在 m128。

## 五 写新代码时的清单

1. 在临界区内**不做**任何可失败分配；把 `xmalloc`/`px_*` 构造移到 `pthread_mutex_lock` **之前**。
2. 需要「先探测再决定」时，用**探测（锁内只读）→ 备货（锁外）→ 复核 + 发布（锁内）**三段；
   两段之间状态可变 ⇒ **有界重来**（见 `px_conn_pend_put` 的 `attempt < 4`）。
3. 备货尺寸优先取**容量上界**（容量只增的量，如 `pend_cap`）⇒ 下一轮必然够用，避免重试放大。
4. 真到了「无法在锁外完成」的地步（如 `gc_register` 的兜底）：**保留 + 在基线里写 `ACCEPTED:` 理由**，
   不要为了「数字好看」把它删掉。
5. 改完跑：`python3 selfhost/check_lock_alloc.py --self-test && python3 selfhost/check_lock_alloc.py --root .`
