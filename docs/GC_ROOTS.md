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

- **层 ①** `gcroot_audit.py --self-test`：4 锚点（2 必中 / 2 必不中）。
- **层 ②** 全仓扫描 **候选 ⇄ `BASELINE.tsv`** 逐条对齐（多重集比较）+ 规模下限
  （函数 ≥ 1400、登记站点 ≥ 170）。基线里出现「真」判定 ⇒ **门必红**（= 有未修的漏登记）。
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

**仍未做**：逐桥**人工**审计（M206 做的是「静态形状扫描 + 动态压力取证」，覆盖的是**能扫出来的形状**）；
`PX_GC_STRESS` 尚未铺到全部 `examples/*/verify.sh`（O(n²)，需按语料分批）。
