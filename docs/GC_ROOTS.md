# GC 根面纪律（PuXian runtime · M170 → M206 累积）

> 本文是**native 桥**（`bi_*` / `px_*_raw_*` / runtime 内部 C 函数）里持有普贤对象的
> **硬性纪律**，以及**判据与工具**。凡改动 `runtime/*.c` 里任何构造/返回容器的函数，先读本文。

## 1 为什么需要它

VM 轨产物默认 **precise GC**（发射时插 `px_gc_set_precise(1)`）：

```
根面 = 全局槽（g_globals） + VM 帧槽（PXM 帧数组） + TLS 登记根栈（px_root_push/px_root_keep）
                   ↑ **不扫 C 栈**（precise 模式下 gc_scan_stack / 寄存器扫描被跳过）
```

而 C 轨逃生舱产物（`PX_BUILD_ENGINE=c`）是 **conservative**（整栈 + 寄存器扫描）——
**同一个函数在两条轨上安全性不同**，所以「C 轨跑得通」不能作为「VM 轨安全」的证据。

`px build` 在 x86_64 上 **默认 engine=vm**（`tools/px`：`engine="${PX_BUILD_ENGINE:-vm}"`）
⇒ 下面这类缺陷**在默认用户路径上是活的**，不是只在调试开关下才现形。

## 2 两条硬约束（判据）

> **① 容器/对象创建后必须登记；**
> **② 登记必须紧跟创建、先于下一次分配**（下一次分配就可能触发 GC）。

形式化：**若「未登记的活值集合」非空时又发生一次分配 ⇒ 该值可能被回收**。

```c
/* ✗ 反面：hargs[0] 只活在 C 数组里，hargs[2] 那次分配就可能回收它 */
LXValue hargs[3];
hargs[0] = px_str(ip);
hargs[1] = px_int(port);
hargs[2] = px_str_len(buf, n);      /* ← 分配 ⇒ GC ⇒ hargs[0] 可能已被回收 */
LXValue r = px_call(handler, hargs, 3);

/* ✓ 正面：push 作用域 + 创建即 KEEP */
LXValue hargs[3];
px_root_push();
hargs[0] = px_str(ip);            PX_KEEP(hargs[0]);
hargs[1] = px_int(port);
hargs[2] = px_str_len(buf, n);    PX_KEEP(hargs[2]);
LXValue r = px_call(handler, hargs, 3);  PX_KEEP(r);
px_root_pop();
```

⚠️ **`PX_KEEP(x)` 写在 `x` 的创建之前等于没登记**（它保护的是当时的 `null`）——M170 踩过。

## 3 两条例外（**不是**缺陷）

1. **受害值作为触发调用的实参**，且该调用是**会持有实参的构造器**：
   `px_ok(v)` / `px_err(v)` / `px_some(v)` / `px_tuple(items,n)` / `px_list_n(items,n)` /
   `px_struct(...)` / `px_enum(...)` / `px_cell(v)` / `px_func_env(...,env)`。
   理由：`gc_register` 把新对象放进 `g_tmp_root` 后才触发 GC ⇒ 标记从新对象出发 ⇒ 老值可达。
   ⚠️ **`px_call` / `px_method` 不在例外里** —— 被调方不保证持有实参
   （M206 缺陷 254：`px_call(json_parse, &v, 1)` 期间 `v` 被回收 ⇒ `json: 对象解析失败`）。
2. **受害值可由另一个已登记对象到达**，或**在触发点已经死了**
   （分支末尾 `return`/`continue`；互斥的 `if`/`switch` 分支；循环体跨轮同名变量）。
   这一类的判定结果写进基线表（见 §5），**不**靠扫描器猜。

## 4 工具与检测器

| 工具 | 作用 | 用法 |
|---|---|---|
| `selfhost/gcroot_audit.py` | **静态审计**：全仓找「创建 → 未登记 → 再分配」形状 | `python3 selfhost/gcroot_audit.py [--self-test] [--json] [--show-victims]` |
| `PX_GC_STRESS=1` | **每次分配即 GC**（把「偶发」变「必现」；代价 O(n²)，只用于小语料/门） | 见下 |
| `PX_GC_INLINE=1` | 强制内联回收（服务模式默认会把 GC 延迟到安全点 ⇒ 压力档必须加它才生效） | 见下 |
| `PX_GC_LIVECHK=1` | 读/写**已回收对象**当场响亮（接在 `px_dict_get/set`、`px_list_push`、`px_index`、`px_len`） | 见下 |
| `PX_GC_UAFDET=1` | 空闲链表 UAF（写坏 slab）——带槽地址与归属类型的响亮报错 | 见下 |
| `PX_GC_TRACE=1` | 硬不变量：回收**仍登记在根栈里**的对象 ⇒ 响亮 | 见下 |
| `PX_GC_DEBUG=1` | 打印根登记栈峰值 / 每轮 GC 统计（退出时 `px_root_peak_dump`） | 见下 |

**筛这类缺陷的标准配方**：

```bash
PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1 <二进制>   # 症状 = 响亮 + 核心转储
PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_UAFDET=1 <二进制>   # 症状 = 写坏空闲链表
```

## 5 判据化（门）

`examples/m206_gcroot/`（M206 建）：

- **层 ①** `gcroot_audit.py --self-test`：**8 锚点**（5 必中 / 3 必不中；M207 起多了
  「跨行签名」「`px_func_env` 不担豁免」「`px_list_n` 不担豁免」「跨行签名 + 已登记」四条 ——
  前三条正是 M207 三个判据缺口的**独立**自证）。
- **层 ②** 全仓扫描 **候选 ⇄ `BASELINE.tsv`** 逐条对齐（多重集比较）+ 规模下限
  （函数 ≥ **1500**、登记站点 ≥ **200**；M207 起上调 —— 扫描面随判据修复变大）。基线里出现「真」判定 ⇒ **门必红**（= 有未修的漏登记）。
  `BASELINE.tsv` 每行 = `file / func / trigger / victim / 判定 / 理由`；**只登记假阳**。
- **层 ③④** 动态：HTTP 面（20 urlencoded + 20 multipart POST，含 session 族）与 UDP 面
  （20 次往返）在**基线档与压力档都必须全绿**，且压力档**不得**出现 `PX_GC_LIVECHK`。
- **层 ⑤** 负控 3 道（各自独立判红 + 源逐字节还原）：撤 `bi_udp_recv` 的 KEEP ⇒ ④ 必红；
  撤 `px_session_read` 的 KEEP ⇒ ③ 必红；撤 `h3_extra_to_headers` 的 KEEP ⇒ ② 静态必红。
  ⚠️ **覆盖边界（如实登记）**：`px_parse_urlenc` / `px_parse_cookie` / `px_quic_raw_*` 三族
  由**静态判据**覆盖 —— `http_serve` 走的是**内联** urlencoded 解析，探针不经过 `px_parse_urlenc`。

## 6 已收口清单（截至 M206）

| 里程碑 | 编号 | 站点 |
|---|---|---|
| M170 | 187–191 | `bi_sqlite_query` / `bi_xml_parse` / onnx / rsa / h3 桥容器 · 隔离点野根 · `px_register_builtins` 建表期 · QPACK · `h_exchange` |
| M182 | 192 + 同族 | `h_exchange` 的「登记迟到」 |
| M183 | 197–199 | 根栈**交棒窗口**（`px_root_pop` 出口是暂停点）· `bi_s3_list` · `bi_px_exec` |
| **M206** | **248–258** | **20 处**：udp_serve / multipart ×4 / urlenc · cookie / session_open · set / quic raw ×2 / h3-extra / session_read / bus_publish / basic_auth / http_get_stream ×4 / udp_recv |
| **M207** | **259–264** | **10 处**：`bi_os_capture`（容器）· `h_exchange`「同名头首次升级为 list」· `px_http_dispatch` ×4（.px 脚本分派）· `vm_run_loop` MKCLO 的 `env` · `px_as_list`（tuple/str 支）· `px_gen_from_list` / `px_gen_lazy`（**未 `gc_register`**）|

### 6.1 M207：判据自身的三个缺口（都由「实跑」照出来）

| # | 缺口 | 后果 | 修法 |
|---|---|---|---|
| ① | `split_functions` 用**单行**正则匹配 `name(...) {` | **跨行签名**的函数**整体不在审计面上** —— 实测漏掉 `h_exchange`（缺陷 260 所在） | 累积签名行直到「括号闭合后的 `{`」；遇顶层 `;` 判为原型并丢弃（函数数 1456 → **1587**）|
| ② | `CONSUME_RX` 把 `px_dict_set`/`px_list_push` 当「消费」 | 「**先建容器、再往里放东西**」这一族**整族漏报**（漏掉 `bi_os_capture`） | 从「消费」里摘出（它们持有**值**，但**不登记接收者**）|
| ③ | `px_func_env` 被当作**安全持有者**（担豁免） | MKCLO 的 `env` 被漏报（缺陷 262） | 从 `HOLD_RX` 摘出 —— 它**先 `gc_register` 再填 env 字段**，注册点那次 GC 看不到实参 |
| ④ | `_ALLOC_NAMES` 缺 `px_iter_at` | **字符串取值会新建串对象**，是一条分配路径（缺陷 263） | 补进分配名单 |
| ⑤ | `px_list_n` 曾被当作**安全持有者**（担豁免）| `spawn_capture` 的输出串被回收（缺陷 266）| 从 `HOLD_RX` 摘出 —— 它内部先 `px_list(n)`（**注册 + 可能触发 GC**）再逐项入列 ⇒ 注册点看不到 items |

⇒ **判据定稿**：`HOLD_RX` 里只保留**核对过源码顺序**为「先填字段、后 `gc_register`」的构造器
（`px_ok`/`px_err`/`px_some`/`px_tuple`/`px_struct`/`px_cell`）与「写进**已注册**容器/入根」的调用
（`px_dict_set`/`px_list_push`/`px_chan_send`/`px_global_set`/`px_set_global`）。

### 6.2 M207 的「家族」观察

- **「构造器 vs 持有者」**：`px_dict_set`/`px_list_push` **不是**登记点；
  `px_func_env` 的「先注册后填字段」使**豁免规则失效** ⇒ 判据必须按**逐个构造器**成立，
  不能按「看起来会持有」推断。
- **「未注册的 GC 对象」**：`px_gen_lazy`/`px_gen_from_list` 用裸 `xmalloc` 建 `LXObject`
  却**不 `gc_register`** ⇒ 标记阶段工作表（= `g_objs` 中 `gc_mark` 的项）**永远不处理它**
  ⇒ 其全部子对象（物化 list / seq / transform / filter 闭包）**不被标记** ⇒ 被 sweep 回收。
  这是**与「漏登记 C 局部」不同的一类**：不是「根没指到」，而是「对象不在表里」。

**仍未做**：逐桥**人工**审计（M206/M207 做的是「静态形状扫描 + 动态压力取证」，覆盖的是
**能扫出来的形状**）；`PX_GC_STRESS` 尚未铺到全部 `examples/*/verify.sh`（本轮把「跑法」做成
常设工具 `gcstress_sweep.sh`，但**未逐个改动 143 个 verify.sh** —— 见 §7）。

## 7 常设压力筛（M207）

`selfhost/gcstress_sweep.sh` —— 把「一次性人工筛」变成 **可重复 · 可分批 · 可复现** 的能力。

### 7.1 判据（两档差分 · 三跑）

- **正常档跑两次**，两遍 stdout 必须逐字节一致；不一致 ⇒ `NONDET`（程序自身不确定，**如实登记**）；
- **压力档**（`PX_GC_STRESS=1 PX_GC_INLINE=1 [+ PX_GC_LIVECHK=1]`）与正常档比：
  stdout 逐字节一致 · rc 一致 · stderr 无检测器标记 · 未被信号杀死；
- **确认步（M207 新增）**：`FAIL_OUT` 时压力档**再跑一遍**，两遍自身不一致 ⇒ 降级 `NONDET`
  —— 挡掉「正常档两遍侥幸一致、压力档把计时放大」的程序（实测 `examples/m117_realworld_defects`
  打印「用时 701ms（阈值 700ms）」⇒ 假红）。

⚠️ `PX_GC_INLINE=1` **不可省**：服务/`spawn` 模式下 GC 被延迟到安全点（实测 collect 次数差一个
量级：32 vs 288）⇒ 缺陷不显形。`--no-inline` 只用于**判据自证**。

### 7.2 分类（`--out` 的 TSV，第二列 = tag）

| tag | 含义 |
|---|---|
| `PASS` | 两档一致 ✅ |
| `FAIL_OUT` / `FAIL_RC` / `FAIL_SIG` / `FAIL_DIAG` | **真信号**（stdout 差异 / rc 差异 / 被信号杀死 / 检测器响亮）|
| `STIMEOUT` | 正常档 OK、压力档超时（O(n²)，需**人工定性**）|
| `NONDET` | 正常档（或压力档确认步）自身不一致 ⇒ **不在本筛管辖区** |
| `SKIP_NORM` | 正常档就不通（常驻服务 / 需要参数 / 需要 fixture / 负样例），原因随行走 |
| `BUILDFAIL` | 构建失败 |

### 7.3 用法

```bash
selfhost/gcstress_sweep.sh --list                       # 候选清单（排序 ⇒ 可复现）
selfhost/gcstress_sweep.sh --batch 2/3 --out /tmp/s.tsv # 分批（各批并集 == 全量）
selfhost/gcstress_sweep.sh --only 'examples/m207*/*.px' --t-stress 60
selfhost/gcstress_sweep.sh --no-inline                  # 判据自证专用
```

退出码：出现 `FAIL_*` ⇒ 1；只有 `STIMEOUT` ⇒ 0；全绿 ⇒ 0。

### 7.4 门

`examples/m207_gcstress/`（6 层）：① 工具自证（`--help` 通道 · 未知选项 rc=2 · 候选规模下限 ·
**分批完备性**（3 批并集 == 全量，逐字节）· `--only` · 退出码语义）· ② 精选语料两档全绿 ·
③ **正判据**（`--env PX_M207_PERTURB=1` 故意造差异 ⇒ 必须判 `FAIL_OUT`）· ④⑤ 负控 A/B
（撤 `px_gen_lazy` 的 `gc_register` / 撤 `px_as_list` 的 `PX_KEEP` ⇒ 必红）·
⑥ 负控 C（判据自伤：比对改恒真 ⇒ ③ 不再红）· ⑦ 覆盖边界登记。

---

## 8 M208：VM 解释循环内的「构造 → 登记」窗口（缺陷 268 / 269 / 270）

### 8.1 背景

M207 的全量压力筛把 3 个语料判 `FAIL`（`m88_s3/s1b_gc_stress` · `m93_s3/coro_gc_block` ·
`m96_s3`），当时记账为「**缺陷 267 家族**（帧协程/挂起协程/多线程的帧槽根面）」。M208 用
**三件诊断**把它定位到真病灶（**不是帧槽根面**）：

| 诊断 | 作用 | 加在哪 |
|---|---|---|
| `PX_GC_LIVECHK` 命中时的**引用溯源** | 扫「执行者 TLS 登记根栈 + 全部存活协程帧槽 + 已暂停线程 vm_state 快照」，回答**谁还指着它** | `runtime.c: px_dbg_obj_refs` |
| **执行帧指令 dump** | 打印当前帧函数/pc/最近 8 条指令（`>>` 标当前） | `runtime.c/vm.c: px_vm_dbg_dump_exec` |
| `px_root_push_keep` **自检** | 登记那一刻对象**已经**是回收态 ⇒ 响亮「登记时该对象已被回收」 | `runtime.c` |

### 8.2 定论（实测链）

1. 溯源说：对象**无任何帧槽指向**、却在**执行者自己的 TLS 登记根栈里** ⇒ 槽/帧不是问题。
2. 执行帧 dump 说：正在执行 `NEWLIST` / `NEWDICT`（`allocer`/`worker`）⇒ **站点在解释循环里**。
3. 自检说：**登记那一刻它已经被回收** ⇒ 病灶在「**构造 → 登记**」这段窗口内。
4. `PX_CORO_WORKERS=1` + 单线程探针全绿、多线程/多协程必红 ⇒ 与**并发路径**强相关。
5. 机制闭合：`px_gc_collect` 出口把 `g_tmp_root = NULL`，而 `g_tmp_root` 正是这段窗口的
   **唯一**保护；这条路上又恰有一个 **M110-S2 协作式安全点**（`gc_unblock_stop →
   gc_pause_if_requested`）⇒ 本线程**主动**停在「对象已在堆上、却不在任何根面」，
   被并发的另一轮 GC 收走。**100% 复现**（`PX_GC_STRESS=1 PX_GC_INLINE=1`）。

**⇒ 这条前提被打破，正是 M170/182/183/206/207 所有 `PX_KEEP` 修复的共同依赖。**

### 8.3 三个编号与修法

| 编号 | 形状 | 修法 |
|---|---|---|
| **268** | `PXOP_NEWDICT` / `px_list_n` 的新建容器只在 **C 局部**，而 `px_dict_set`/`px_list_push` **本身会分配**（键副本 `m128_strdup`、条目 `xrealloc` 扩容） | 创建即登记（`px_root_push_keep`） |
| **269** | **回收出口清 `g_tmp_root`** ⇒ 「构造 → 调用方接住」窗口无保护（窗口内恰有安全点） | 出口**不清** `g_tmp_root`（语义 = 「本线程最近登记的对象 · 构造窗口未结束」；每轮必标记 ⇒ 不可能被回收；代价 = 每线程多保活 1 个对象） |
| **270** | 静态审计器**没把 `px_dict_set`/`px_list_push` 算作分配点** ⇒ 「先建容器、再往里放东西」一族**整族静默漏报** | 新增 `GROW_RX`（**隐式分配**）+ `--grow` 开关；`px_root_push_keep` 计入登记 |

### 8.4 判据（S13 / S14 · 新增）

- **S13**：**构造与登记之间不得存在安全点**。惯用法从两段式
  `px_root_push(); PX_KEEP(x);`（两次 `gc_unblock_stop` ⇒ 中间即窗口）
  改为**原子** `px_root_push_keep(x)`（一个临界区）。全仓已改写 **55 处**（runtime.c 53 / vm.c 2）。
- **S14**：**隐式分配**（`px_dict_set` / `px_list_push` / `*_locked`）计入审计器的分配点，
  且**不适用**「持有实参」豁免（容器本身不因此成为 GC 根）。

### 8.5 ⚠️ 未收口：缺陷 271 族（`--grow` 新照出的 18 处）

`--grow` 打开后全仓候选 **19**（默认 legacy 仍为 **8** —— 与 `m206/BASELINE.tsv` 一一对齐，
`runtime/vm.c` 为 **0**）。逐条判定后登记为**缺陷 271 族（下一轮收口）**：

| 文件 | 站点（行号·函数） | 受害者 |
|---|---|---|
| runtime.c | 3459/3460 `px_gen_materialize` · 3477 `px_gen_next` | `r = px_call(...)` 的返回值 |
| runtime.c | 11436 `bi_map` · 11452 `bi_filter` | 回调返回值 |
| runtime.c | 13204 `bi_os_popen` · 23493 `bi_session_set` · 23510 `bi_session_del` | 结果 dict |
| runtime.c | 24336 `px_http_dispatch_h3` | 结果 dict |
| runtime.c | 17587/17617/17638 `bi_http_get_stream` | 分段串 |
| runtime.h3.c | 899 `bi_h3_conn_peer` · 915 `bi_h3_conn_stats` | 结果 dict |
| runtime.h3_qpack.c | 272 `qp_dec_value_string` · 350 `px_h3_qdec` | 解码值 |
| runtime.image.c | 70 `bi_img_decode` | 结果 dict |
| runtime.sqlite.c | 211 `bi_sqlite_query` | 行数组 |
| runtime.c | 3241 `px_cell`（**假阳**：`px_list(1)` + 单次 push ⇒ 不可能扩容） | `l` |

**判据边界（如实）**：本轮 M208 门只对 `runtime/vm.c`（`--grow`）断言 **0**，
对全仓 `--grow` 候选**只记录不断言**；`m206` 门继续走默认（legacy）规则集，
故它的 `BASELINE.tsv` 无需改动。

### 8.6 门

`examples/m208_vm_c_local_roots/`（6 层）：① 审计器自证 10/10 + `--grow` 下 `vm.c` 候选 0 ·
② S13 不变量（相邻两段式 = 0 · 原语在位 · 自检在位）· ③ S14 守卫（两条回收路径都不清
`g_tmp_root`）· ④ `probe_lit`（单线程）/`probe_coro`（8 协程）两档 · ⑤ `m93_s3/coro_gc_block`
压力档回归（修前必红）· ⑥ 负控 3 道（恢复 `g_tmp_root = NULL;` / 撤 NEWDICT 登记 /
撤 `px_list_n` 登记 ⇒ 各自独立判红 + 源逐字节还原）。
