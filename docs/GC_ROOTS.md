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
