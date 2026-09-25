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

⚠️ **M209 精确化（缺陷 279 · 见 §9.1）**：这里的「分配」严格指**触发点** = 调用链上会
`gc_register` 的入口（构造函数集合）。**`px_dict_set` / `px_list_push` 不是触发点** ——
它们只做 slabs/mmap 裸分配，不判阈值、不置 `g_gc_pending`；且它们被调用时受害者恰是
「本线程最近登记的对象」⇒ 受 `g_tmp_root` 保护。

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
   M206/M207 时这一类的判定靠**人工读代码**并写进基线表（§5）；
   **M209 起已下沉为判据**（覆盖 / 作用域 / deref 交棒三条，见 §9.2）——
   判据能判的，不再由人判。

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

### 8.5 ⚠️ 缺陷 271 族（`--grow` 新照出的 18 处）→ **M209 已全部收口（见 §9）**

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

**判据边界（如实）**：M208 门只对 `runtime/vm.c`（`--grow`）断言 **0**，
对全仓 `--grow` 候选**只记录不断言**；`m206` 门继续走默认（legacy）规则集。
**⇒ M209（§9）把全部 19 条逐条定证**：11 条由「触发点仅构造器」消除（缺陷 279，前提错位），
8 条由「覆盖 / 作用域 / deref 交棒」三条排除规则消除（缺陷 280–282）⇒ **全仓候选 0**。

### 8.6 门

`examples/m208_vm_c_local_roots/`（6 层）：① 审计器自证 **16/16**（M209 起）+ `--grow`（旧规则对照）下
`vm.c` 候选 0 · ② S13 不变量（相邻两段式 = 0 · 原语在位 · 自检在位）· ③ S14 守卫（两条回收路径都不清
`g_tmp_root`）· ④ `probe_lit`（单线程）/`probe_coro`（8 协程）两档 · ⑤ `m93_s3/coro_gc_block`
压力档回归（修前必红）· ⑥ 负控 3 道（恢复 `g_tmp_root = NULL;` / 撤 NEWDICT 登记 /
撤 `px_list_n` 登记 ⇒ 各自独立判红 + 源逐字节还原）。

## 9 M209：「触发点」的更正 + 三条排除规则（缺陷 279–282）

> 一句话：M208 给审计器加的「隐式分配」规则**前提错位**（把安全点当触发点），
> 那些候选的**受害者归因也是错的**。本轮读源码定证 + 动态取证 + 判据回放三层做完，
> 把 M206 的**人工判定**下沉成**判据**，并给判据本身加了「不许放水」的反向证据。

### 9.1 缺陷 279 —— M208 的「隐式分配」前提被证伪

| 事实 | 位置（`runtime/runtime.c`） |
|---|---|
| **full GC 的唯一触发点 = `gc_register`** | `:3048` `g_alloc_bytes += est;` → `:3051` 判阈值 ⇒ 内联 `px_gc_collect()` 或 `g_gc_pending = 1`；`:3049` 置 `g_tmp_root = o` |
| `px_dict_set` 只走**裸分配**（`m128_strdup` / `m128_alloc` → `xmalloc`/`xrealloc`） | `:5105` |
| `px_list_push` 同 | `:5020` |
| `g_tmp_root` 是 `__thread`，且回收出口**不清**它（M208 缺陷 269） | `:1457` · `:2696` |

⇒ 这两个调用**不会触发 GC**。它们内部确有 M110-S2 安全点（`gc_unblock_stop` → `gc_pause_if_requested`），
但那只在**另一个线程的 GC 已在跑**时暂停本线程；那一刻被保护的面是本线程 `ti→tmp_root`
（`:1973` 快照自 `g_tmp_root`；`:2635`/`:2742` 逐线程标记）= **本线程最近登记的对象**。
而审计器报告的每个候选，受害者**恰是「创建后的第一个触发点」**上的对象 ⇒ 触发点是 push/dict_set 时，
受害者**就是**最近登记对象 ⇒ 被护住 ⇒ **安全**。**19 条里 11 条由此消失。**

**动态取证**（不靠"读代码猜"）：`examples/m209_gcroot_rules/probe_rules.px` 把可复现的五族
（`bi_map` / `bi_filter` / `px_gen_*` / `px_cell` / `bi_os_popen`）跑进
`PX_GC_STRESS=1 PX_GC_INLINE=1 PX_GC_LIVECHK=1 PX_GC_UAFDET=1` ⇒ 与正常档**逐字节一致**、无检测器标记。
且**负控 A**（撤 `bi_map` 的 `px_root_push_keep(r)`）在同一档下 **rc=134 + LIVECHK 响亮** ⇒ **探针有牙**。

⚠️ **探针必须让回调真的分配**：首版 `inc(x) = x + 1`（`px_int` 是立即数、不 `gc_register`）⇒ 循环里
**根本没有 GC**，负控 A 照样绿（**假绿**）。回调里一律构造字符串 —— 这条写进了 probe 头注释。

### 9.2 缺陷 280 / 281 / 282 —— 三条排除规则（人工判定 → 判据）

| 规则 | 判据（可证） | 消灭的候选 |
|---|---|---|
| **触发点集合** | 只有会 `gc_register` 的入口算触发点 | 11 条（§9.1）|
| **覆盖排除** | 触发点语句**正在写同名直接局部**，且右值**未读**旧值 ⇒ 旧值此刻已死 | `runtime_h3_qpack.c:350 name` · `runtime_sqlite.c:209 v` |
| **作用域排除** | 受害者的**声明块**在触发点之前已闭合 ⇒ 读不到 ⇒ 不是受害者 | `keep`（`px_gen_materialize`/`px_gen_next`）· 宏体 `rv`（3 条）|
| **deref 交棒** | `*outv = px_str_len(…)` 把值交给调用方（与 `return` 同口径）⇒ 不计本帧 C 局部活值 | `runtime_h3_qpack.c:262 outv` |

⚠️ **作用域判据必须按「块身份」而不是「括号深度」**（第一版用深度 ⇒ 自证 `miss7` 判红）：宏体
（`#define STREAM_FLUSH()` 的 `do{…}`）在**源码里**位于较浅的深度，而其**调用点**在更深的分支里
⇒「深度变小」永不成立 ⇒ 宏体局部 `rv` 被误当活值。块身份（每语句至多一个 `{`/`}`，入/出栈）则精确。

⚠️ **作用域要绑「声明」而不是「赋值」**：`v = px_str_len(…)` 写在 `switch` 的 case 块里，而
`LXValue v;` 在外层 —— 用**赋值处**的块会**误剪**（把仍在作用域的值当死值）⇒ **漏报**，
违反审计器本分。修法：`DECL_RX` 记每个名字的**声明块 id**，剪枝按它。

### 9.3 判据不放水（本轮最重要的一层）

改判最容易把工具改瞎，所以给了两层反向证据：

1. **反向判据**（`--self-test` 的 16 锚点里 3 条）：`hit6b`（push 之后**插一次真构造器**仍必中）·
   `hit7`（受害者**仍在作用域**时必中）· `hit8`（右值**读了**旧值时，覆盖排除**不得**生效）。
2. **判据回放**（`examples/m209_gcroot_rules/verify.sh` 层 ⑤）：把 2 处**已修**站点的修复**原样退回**，
   审计器必须**命中**：
   - 撤 `px_session_read` 的 `PX_KEEP(v)` ⇒ 在后续 `px_call` 处报出 `v`（缺陷 254 形状）；
   - 撤 `bi_map` 的 `px_root_push_keep(r)` ⇒ 报出**容器 `r`** —— 顺带证明 M208 的**受害者归因是错的**：
     真实风险在**容器**（下一轮 `px_call` 的分配），而不是 push 的实参 `res`。

### 9.4 收官与边界

- 新规则集下**全仓候选 0**（25 文件 · 函数 1593 · 登记站点 211）；`--grow` 保留为**旧规则对照**
  （11 条，**全部**带「隐式分配·旧规则」标记 —— 门逐条断言，防对照被悄悄清空）。
- `examples/m206_gcroot/BASELINE.tsv` **收敛为 0 条**：M206 的 8 条人工判定转入 §② 档案区，
  逐条注明「现由哪条规则排除」⇒ 本表自此是**安全网**（任何**新增候选**都必须进表）。
- **覆盖边界（如实）**：动态只覆盖五族；`bi_session_set/del` · `px_http_dispatch_h3` ·
  `bi_h3_conn_peer/stats` · `bi_img_decode` 四族**仅静态判据**覆盖 —— 形状与已覆盖族相同
  （「容器随后写入」/「构造器结果随后入列」），但缺单机可复现的触发路径（需 serve / H3 / 图片）。
- **仍未做**：逐桥**人工**审计（本系列覆盖的是「能扫出来的形状 + 动态筛得到的」）；
  `PX_GC_STRESS` 铺到全部 `examples/*/verify.sh`（按语料分批）。

---

## 10 M212：触发点集合**必须源码派生**（缺陷 288 / 289 / 290 / 291）

### 10.1 缺陷 288 —— 「手抄集合」把 300+ 个分配入口漏在判定之外

M209 已把判据定成「**触发点 = 调用链上会 `gc_register` 的入口**」，但审计器里那份名单
（`_ALLOC_NAMES`）是 M206 **手抄**的 27 个构造器，**从没跟着判据改**：

| | 数量 |
|---|---|
| 手抄 `_ALLOC_NAMES` | **32**（含 5 个非构造器语义项） |
| 源码派生 **TRIGGER**（`gc_register` 的传递闭包） | **518** |
| 其中 **PRODUCER**（返回类型是 `LXValue`） | **316** |

⇒ 手抄集合漏掉 `px_call` / `px_session_read` / `h3_send_fields` / `px_s3_exec` / `bi_*` 整族 …
（实测：把集合改成派生后，全仓候选 **0 → 3**，其中 1 条**直接照出一处真漏登记**）。

**新工具**：`selfhost/gcroot_derive.py`（`--list/--json/--self-test`；10 锚点）。
审计器默认 `--trigger-set derived`，`legacy` 仅作 A/B 与留证；派生失败
**拒绝静默回退**（悄悄退回手抄集合 = 把漏报伪装成全绿）。

### 10.2 缺陷 289 —— 静态调用图**看不见间接调用**

`px_call` 的体是 `fn.as.obj->as.func.fn(args, nargs, …)` —— **被调方是形参**（函数指针）
⇒ 朴素的「谁调了 `gc_register`」闭包**不含它**，实测派生集合会从 518 掉到 329、
候选从 3 掉到 2（**把真漏报藏起来**）。

⇒ 判据补一条：**被调方是形参**，或出现 `(*p)(…)` ⇒ 该函数视为触发点。
（真实树里 `间接` 入口 **102** 个。）

### 10.3 缺陷 290 —— 「登记动作」本身就是根

形态：`o->as.gen.list = px_list(0);` … `gc_register(o, sizeof(LXObject));`
—— 注册点那一刻 `o` 进 `g_tmp_root` 且进 `g_objs` ⇒ 标记阶段扫 `o` 的全部字段
⇒ `o->…` 里的子对象**可达** ⇒ 安全。
**这正是 M207 缺陷 264 的修复形态**（字段填齐之后再注册），却被判成候选。
⇒ 判据：触发点是 `gc_register(X,…)` / `PX_KEEP(X)` / `px_root_push_keep(X)`，
且受害者的**基名** == `X` ⇒ 豁免（负控 C：撤掉后候选 3 → 4）。

### 10.4 缺陷 291 —— **M170 同族的漏网实参**（真漏登记，已修）

`h3_send_fields(conn, sid, fields, body_val, fin)`：
`M170` 为 `fields` 加了 `PX_KEEP(fields)`（理由：`px_qd_enc`/`px_h3_qenc` 会分配），
但**同一个窗口里的第二个实参 `body_val` 没有登记** —— 而它的数据指针 `bd`
正是 DATA 帧（`h3_build_frame(f, H3_FRAME_DATA, bd, blen)`）的依据
⇒ 压力档下 `body_val` 被回收后 `bd` **悬垂**。
**修法**：入口 `PX_KEEP(body_val)`（复用已有 `px_root_depth`/`px_root_restore` 出口归一；
`PX_KEEP` 对非对象是 no-op）。
⚠️ 这一处**只有**源码派生的触发点集合能看见（手抄集合下它不可见）。

### 10.5 3 条候选的判定（逐条，全部「假阳」）

| # | 位置 | 判定依据 |
|---|---|---|
| 1 | `bi_session_del ← px_session_write(` 受害者 `nd` | **动态取证**：`probe_rt.px` 补 `session_del` 后，压力档 **8 轮 × 40 请求全绿**。机制：`nd` 经 `sess` 可达，而 `sess` 由 `px_session_read` 的 TLS 条目持有（M183 **延迟收缩**：到下一次 KEEP 才物理回收，`bi_session_del` 全程无 KEEP）。⚠️ 属**隐性耦合**，与 `bi_session_set` 的显式 `PX_KEEP` 不对称 ⇒ 下一轮宜对称化 |
| 2 | `h3_out_send ← h3_send_fields(` 受害者 `bv` | 报告点是**调用点**，真漏洞点在**被调方内部** —— 已由缺陷 291 修复（入口即登记）⇒ 调用点的 `bv` 安全 |
| 3 | `xml_build_node ← bi_xml_escape(` 受害者 `sv` | `bi_xml_escape` 体内**唯一**分配点是 `px_str(out.data)`，晚于对 `args[0]` 的读取与**全部消费**（循环里 `xbuf_push` 走裸 realloc）⇒ 到达触发点时 `sv` 已死（**后置存活**） |

### 10.6 已知判据缺口（下一轮候选）

1. **跨函数**的「被调方登记了实参」看不见 ⇒ 被修好的站点仍会在**调用点**被报（#2 的形状）。
2. 「**后置存活**」（受害者读点是否在触发点之后）尚未判据化 ⇒ #3 目前靠人工判定。
3. 动态覆盖：H3 族仍缺单机 QUIC listener 夹具。

### 10.7 压力筛的「慢语料」出口（与 KNOWN 分工）

`examples/m207_gcstress/SLOW.tsv` + `gcstress_sweep.sh --slow FILE`：
**「输出确定且正确、只是慢」** ⇒ 给足时间**继续判**（覆盖面不丢），与
`--known`（**输出本身不确定** ⇒ 排除在判定之外）**分工明确**。
首条：`m88_s3/s1b_gc_stress`（200 线程 × 300 分配）在 `STRESS+INLINE` 下每次分配都触发
全 STW GC ⇒ **实测 274.5s · rc=0 · 输出与正常档逐字节一致 · RSS 平稳 ~7MB**
⇒ 结论「**O(n²) 正常**」，默认 60s 上限必记 STIMEOUT。

---

## 11 · M213（第 92 轮）：审计器的「判据诚实度 + 覆盖面」（缺陷 293–298）

M209 把触发点定义为「可达 `gc_register`」，M212 把集合改为**源码派生**并照出候选 3。
本轮继续追同一件事的**两个面** —— 结果证明：**「候选 0」与「候选 N」都可能是判据自身的产物**。

### 11.1 覆盖面（漏报）· 缺陷 294 / 296

| # | 形状 | 为什么看不见 | 处置 |
|---|---|---|---|
| **294-a** | 出口参数式构造函数：`px_as_list(v, &out)` —— 返回 `int`、经 `LXValue* out` **输出新对象** | 审计器只认「赋值给 lvalue 的 PRODUCER」⇒ 这类**出口**不在视野 | 新 `derive_out_producers`（**按位置**把实参配对到形参，只收 `LXValue*` 且体内 `*p = …`）⇒ 12 条 |
| **294-b** | 成员取址别名：`LXObject* o = xs.as.obj;` | 被 ④b 当「**消费**」⇒ 把刚记进 live 的 `xs` **弹掉**（而它只读了指针，`xs` 必须继续活着） | 别名（纯成员链）**不消费** ⇒ 两者都盯着 |
| **296** | **显式 GC 入口**：`px_gc_collect()`（`gc()` 内建 / 池循环硬上限兜底）· `px_gc_poll()`（请求间安全点） | 它们**不经过** `gc_register` ⇒ 派生集合看不见 | 显式纳入触发点种子（`_GC_ENTRY_RE` + `_EXPLICIT_GC`） |

**294 的实证**：撤掉 `bi_join` 的 `PX_KEEP(xs)` 后，**修前报不出**（而 M207 缺陷 263 正在这条路上 —— 当年是**动态**压力筛抓的）。
**296 的实证**：把两处口子都堵上后，**tight 档**三个入口族**全部掉出** TRIGGER；而 **loose 档仍在**
—— 因为宽松兜底**偶然**收住了它们（`px_gc_collect` 体内有 `sizeof(*b)` 形状）。⇒ 这既是「缺陷 296 真实存在」的证据，
也是「**不能收紧间接调用判据**」的实证。

### 11.2 诚实度（假阳）· 缺陷 295

间接调用判据 M212 的**意图**写的是「被调方是形参 或 `(*p)(…)`」，**实现**却是
`re.search(r'\(\s*\*\s*[A-Za-z_]', body)` —— 它匹配的是「**括号后跟星号**」这一**语法形状**，
与「是否调用」无关，于是把

```
while (*pp) {              ← 条件里的指针解引用
memset(b, 0, sizeof(*b));  ← sizeof 的类型操作数
(*p).field                 ← 成员访问
```

全当成间接调用。实测：**假种子 99**（seed 119 → 精确 20）· 闭包 **518 → 349**（虚增 169 个函数）·
并**产出候选 #3**（`px_rate_limit_try`，体内**只有裸 `xmalloc`/`snprintf`**，无任何 `gc_register` 级调用）。

**处置：分层 + 分诊，默认留「过近似」档。**

| 档 | 规则 | 用途 |
|---|---|---|
| **precise** | ① 被调名 ∈ 形参 ② `(*name)(…)` ③ 成员调用 `x.name(…)`/`x->name(…)` | 真的「调用」 |
| **conservative** | 任何 `(*x` 形状（含上面的非调用语境） | **有意的过近似**（宁多收） |
| 默认 | `loose` = precise ∪ conservative | 安全侧（漏报比假阳危险） |

`--indirect tight` 仅用于**分诊**；候选 JSON 带 `trigger_kind`（`precise`/`conservative`）⇒
**过近似代价一眼可见**（本轮 4 条候选里恰 1 条是 `conservative`）。

### 11.3 新判据 **F1「被调方登记了实参」**（缺陷 297 · 收口 M212 §10.6 缺口①）

调用点的活值作为**实参**传进被调方，而被调方**入口**就 `PX_KEEP(形参)`（形参**按位置**配对），
且该登记**先于被调方第一个触发点** ⇒ 进入被调方后在任何分配之前即被登记 ⇒ 调用点**不构成风险**。

**必须有「先于第一个触发点」这条**：登记出现在首次分配**之后** = 「**登记迟到**」
（M182 缺陷 192 同族）⇒ 那时值**已被回收** ⇒ **不得豁免**。
（自证锚点 ⑰ 就是按这条判红的 —— **反向判据**，防「把真缺陷豁免掉」。）

⚠️ **实现坑**：`body` **含签名**，而函数**自身**往往是触发点 ⇒ 从 body 开头搜「之前的触发点」
会**每次都在签名处自命中** ⇒ F1 整条**静默失效**（实测 F1 表只剩 3 条、`h3_send_fields` 被漏掉）。
⇒ 一律从**开括号之后**开始搜。

### 11.4 产品侧两处收口（缺陷 293 / 298）

| # | 位置 | 修前 | 修后 |
|---|---|---|---|
| **293** | `bi_session_del` | 全程**无登记**，靠 M183「根栈延迟收缩」的**副作用**兜住（`sess` 由 `px_session_read` 的 TLS 条目持有、条目要到**下一次 KEEP** 才物理回收）⇒ **隐性耦合** | 与 `bi_session_set` **对称化**：入口 `px_root_push()` + 逐个 `PX_KEEP` + 出口 `px_root_pop()`（push 之后**无提前 return**） |
| **298** | `px_route_try_dispatch` | `params` 的登记点在**限流块之后** ⇒ 中间隔着 429 短路分支里的 `route_send`/`px_access_log`（**都会分配**）；靠「该分支随即 `return`、不再读 `params`」这一**脆弱性质**兜住 | 登记**前移**到 `route_match` 之后 + 429 分支补 `px_root_pop()` 配对（**每个出口各一次**，静态门 ⑦ 守） |

> `handler`（同处另一受害者）**无需登记**：`route_match` 里 `*handler_out = g_routes[i].handler`
> ⇒ 经**全局表**可达 ⇒ 本就是 GC 根。同族：`vhandler` 经 `g_vhosts[].handler`。

### 11.5 本轮候选账（4 条 · 全判假阳）

见 `examples/m206_gcroot/BASELINE.tsv` §①（M213 已改写）。三条旧条目**移出**：
`bi_session_del`（293 已修）· `h3_out_send` / `h3_srv_{pipe,stateless}_cb`（297 判据化）；
一条**新增**：`px_route_try_dispatch ← px_rate_limit_try( · handler`（298 把 `params` 从受害面移除后只剩 `handler`）。

### 11.6 仍未判据化（下一轮候选）

1. **「全局表持有」**：受害者指向的对象由**文件级全局**数组/表持有（`g_routes[i].handler`、
   `g_vhosts[].handler`）⇒ **本就是 GC 根** ⇒ 候选 #2/#3 仍需人工判定。
2. **「后置存活」**：受害者的**最后一个读点**是否早于触发点（含**被调方体内**）⇒ 候选 #1/#4。
3. **提前 return 不可达**：#1 的形状（触发点在错误分支末、随后立即 `return`）。
4. 动态覆盖：H3 族仍缺单机 QUIC listener 夹具。
5. `bi_session_set` 的 `PX_KEEP(sess); PX_KEEP(data);` 与 F1 判据的口径可再核对
   （本轮只做了 `del` 的对称化）。

### 11.7 门与工具

| 门 | 内容 |
|---|---|
| `examples/m213_gcroot_precision/`（**新**） | 9 层：派生器 19 锚点 / 审计器 19 锚点 / **分诊** / **覆盖面**（loose+tight 两档）/ **F1 A/B 4⇄7** / 候选⇄基线 / **push/pop 平衡** / 负控 3 道 / 覆盖边界 |
| `examples/m206_gcroot/` | 自证锚点 16 → **19**；候选⇄基线 4 条 |
| `examples/m212_gcroot_derive/` | 锚点 10 → **19**；A/B 候选 3 → **4** |
| `examples/m209_gcroot_rules/` | 接受「候选 4」 |
| `selfhost/gcroot_derive.py` | 新增 `--indirect loose\|tight` · `--callee-keeps` · `derive_all` · `derive_callee_keeps` |
| `selfhost/gcroot_audit.py` | 新增 `--indirect` · `--no-callee-keep` · 候选 `trigger_kind` · F1 过滤 |
