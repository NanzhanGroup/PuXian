# HTTP/2 与 HTTP/3 的口径（M180 · 回应晨曦 QA 清单 P0-3 / P1-4）

- 发布方：东月（dongyue）· 2026-09-22
- 触发：晨曦在 `puxian-0.2.0-1.m166.el9` 上的实测（节点总线 2026-09-21）
- 结论一句话：**H2 不做（长期）；H3 是唯一的"现代协议"路线，且在 x86_64 默认构建里已可用。**
- 状态：**本文件即口径**，Ma 侧文档可据此定稿。相关代码行为已按本口径改造并有门
  （`examples/m180_h2_h3_stance/`）守着。

---

## 一、HTTP/2：**长期不做**（明确口径）

### 1.1 事实（三轨一致，都在本仓可复现）

| 事实 | 证据 |
|---|---|
| 生产 Web 服务的 **ALPN 只声明 `http/1.1`** | `runtime/runtime.c` 的 TLS 配置处（`alpn_list = { "http/1.1", NULL }`），注释自陈「生产 Web 服务（多站点/deny/SPA/反代）必须走 handler」 |
| `runtime_h2.c` 的帧循环**不接 vhost/handler 管道** | 收到 HEADERS 后直接回一个**固定演示页**（`<h1>PuXian HTTP/2</h1>`），无限流 / 无日志 / 无静态 / 无视 docroot |
| h2 代码在多数产物里**被链接器裁掉** | 程序不引用 ⇒ `runtime_h2.o` 不进产物（`strings` 0 命中），与"不可用"观感一致 |

⇒ 对 Ma 侧的影响：**h2 不是配置问题，是能力不做**。现代浏览器与 CDN 回源拿不到 h2。

### 1.2 为什么不做（不是"来不及"，是取舍）

1. **要做就得重写，而不是补丁**：h2 需要 ALPN 协商 + 帧层 + HPACK（已有）+ **流控/多路复用**
   + **与 vhost/handler 管道集成**（这是架构级：`px_http_dispatch` 的输出模型是"整响应"，
   而 h2 是"分帧 + 可交错"）。
2. **收益被 H3 吃掉大半**：浏览器走 H3（QUIC）时延迟/丢包表现优于 h2；我们已经在
   `Alt-Svc` 自动通告 h3（见 §二），客户端会自动升级。
3. **保守性优先**：演示页的 200 响应是**静默错内容**（最坏一类），本项目的判据是
   「响亮优于静默」「不留静默的角落」——与其留一个"看起来能用、内容不对"的 h2，
   不如**明确不做**，并在协议层给出可解释的拒绝。

### 1.3 本口径下的行为（改动后，有门守着）

| 客户端行为 | 服务端行为 |
|---|---|
| TLS 上声明 `ALPN h2,http/1.1` | 协商为 **`http/1.1`**（与现状一致）；不声明 h2 ⇒ 不会误升级 |
| 明文发 `Upgrade: h2c` | **忽略 Upgrade，按 HTTP/1.1 正常服务**（= RFC 7230 允许的"忽略升级"，客户端拿到的是**正确的 docroot/handler 内容**，不再是演示页）+ stderr 一行说明 |
| 明文发 h2 **prior-knowledge 前导**（`PRI * HTTP/2.0`） | **`505 HTTP Version Not Supported`** + 纯文本说明（无法当 h1.1 解释，明确拒绝） |
| 应用代码里任何 `Upgrade: h2c` 的期望 | **不要依赖**：我们只保证"请求仍被正确服务" |

> ⚠️ 对 Ma 侧的迁移动作：**无需任何配置改动**。若 Ma 的站点配置里显式开了 h2（或反代声明
> `Upgrade: h2c`），会从"拿到演示页"变成"拿到真实内容（h1.1）"——这正是期望行为。

### 1.4 如果将来要做（触发条件）

出现下列之一时再评估：① H3 在某些关键链路不可达（UDP 被封）+ 该链路又不能降级到
h1.1 连接复用；② 上游/CDN 明确要求源站 h2；③ 需要 h2 的**服务端推送**语义（我们无此需求）。
届时的工作量按"重写帧循环 + 流控 + 管道集成"估，**不是小补丁**。

---

## 二、HTTP/3：**已可用（x86_64）**，作为唯一现代协议路线

### 2.1 事实

- H3 的**管道是完整的**：`runtime/runtime_h3.c`（1451 行）+ `px_http_dispatch_h3` 走
  与 h1.1 **同一张 handler 表**（vhost / 限流 / 访问日志 / 静态 / `.px`）。
- 开启方式：`px_serve(port, docroot, timeout_ms, opts)` 里 `opts{"http3": true}`（自签证书）
  或 `opts{"http3": {"port":…, "cert":…, "key":…}}`；开启后会自动下发
  **`Alt-Svc: h3=":port"`** ⇒ 浏览器/客户端据此自动升级到 H3（**同理适用于 `http_serve`**）。
- 依赖：QUIC 需要 **ngtcp2 + openssl(quictls)** 静态库。仓库内预置**仅 x86_64**：
  `runtime/third_party/ngtcp2/lib/*.a`、`runtime/third_party/openssl/lib/*.a`。
- ⇒ **非 x86_64 宿主（aarch64/armv7/riscv64）默认 `--no-quic`**（`tools/px` 自动加并打印提示；
  自备库可设 `PX_HOST_QUIC=1` 保留）。

### 2.2 为什么晨曦那边看到"ngtcp2 0 命中"（**根因已定位并修掉**）

M180 复盘三个原因，**前两条是真问题**：

| # | 原因 | 说明 | 处置 |
|---|---|---|---|
| ① | **按引用集自动裁剪把 `quic` 裁掉了**（真因） | `tools/px` 会按程序**引用的 native 名**裁剪模块（`runtime/native_mod_map.txt`）。而该表**没有把 `px_serve` 映到 `quic`** ⇒ 「只用 `opts{"http3": true}` 开 H3」的程序**不会被识别为需要 QUIC** ⇒ 自动加 `-DPX_NO_QUIC` ⇒ 连 `px_serve` 里的 H3 代码块都被编译掉 ⇒ 运行期 `opts.http3` **静默无效** | **修**：`native_mod_map.txt` 增 `px_serve=quic`（服务端程序默认带 H3；要裁用 `--no-quic`），并新增**响亮报错**（要求 H3 而产物无 QUIC ⇒ 直接 `R1002`） |
| ② | 链接器裁剪 | 不引用 H3 的程序不会把 `runtime_h3.o`/ngtcp2 拉进产物（`strings` 自然 0 命中）——这是**正常**的 | 判据改成"**要求 H3 的产物**必须含 ngtcp2"（门 D 层） |
| ③ | 架构 | 非 x86_64 宿主默认 `--no-quic`（预置静态库只有 x86_64）——这是**有意**的 | 文档写清 + `px build` 打**能力行**（`quic=on|off`） |

**M180 实测（本机 x86_64）**：默认构建要求 H3 的程序 ⇒ 产物 `strings` 含 `ngtcp2`、运行起来
`[px-serve] HTTP/3 listening udp/<port> … alt_svc=h3=":<port>"`；
`--no-quic` 构建同一程序 ⇒ `strings` 不含 `ngtcp2`，且**要求 H3 时报 `R1002`**（不再静默）。
⇒ 也就是说：**H3 现在是"默认可用 + 能力可自证 + 裁掉时响亮"**。

### 2.3 本口径下的改动（让"能力"可自证，不再靠 strings 猜）

| 改动 | 目的 |
|---|---|
| `PX_BUILD_FEATURES=1 px build …` 打 **能力行**（`quic=on/off h2=off 宿主=…`，走 stderr） | 一眼可判，不用 strings；**默认静默**（成功路径不污染 stderr —— engine_parity 的判据要求） |
| 以 `--no-quic` 构建的产物**若**在 `opts.http3` 上被要求 H3 ⇒ **响亮报错**（`R1002`，说明本二进制无 QUIC） | 修掉"静默不服务"（最坏一类） |
| 门 `examples/m180_h2_h3_stance/`：默认 x86_64 构建的产物必须 `strings` 命中 `ngtcp2`/`Alt-Svc` + m53_s4 端到端真跑 | 能力成为**判据**而非说法 |
| `packaging/README` + `RELEASE_PROCESS` + README 能力表写明"QUIC 仅 x86_64" | 发布侧不再含糊 |

### 2.4 【重要】带 H3 的构建代价（实测，便于权衡）

本机（x86_64）实测同一个"要 H3 的服务端程序"：

| 构建 | 产物大小 | 说明 |
|---|---|---|
| 默认（含 H3） | **8,120,136 B（≈8.1 MB）** | `px_serve` 被引用 ⇒ 自动保留 `quic` 模块 ⇒ 链 ngtcp2 + openssl(quictls) |
| `--no-quic` | **3,031,000 B（≈3.0 MB）** | 无 H3；此时若代码里要 `opts.http3` ⇒ **运行期响亮报错**（不会静默） |

⇒ 带 H3 的代价 ≈ **+5.1 MB/二进制**。若某个服务确实不需要 H3（内网、或 UDP 被封），
用 `px build --no-quic` 换体积，代价是那份源码里**不要**开 `opts.http3`（否则它会明确报错）。

### 2.5 Ma 侧上 H3 的步骤（三步）

```
1. 站点入口传 opts{"http3": true}（或 {"http3": {"cert":…, "key":…}} 用正式证书）
2. 放行 UDP 同端口（QUIC 走 UDP；TCP 仍照常监听，双栈并存）
3. 客户端首次经 h1.1 访问 → 拿到 `Alt-Svc: h3=":port"` → 自动升级（浏览器无需配置）
```
⚠️ 若 Ma 的链路（CDN/防火墙）封 UDP，则 H3 不可达而 h1.1 仍然工作 —— 这是有意的降级路径。

---

## 三、与 h2/h3 无关但同批处理的两条（QA 清单其余项）

| 项 | 处置 |
|---|---|
| P1-5 `SO_REUSEPORT`（零停机换二进制） | **已在 M176 收口**（`opts{"reuse_port": true}` / `PX_REUSE_PORT=1`；门实测 700 次探测 `refused=0`） |
| P2-6 `unix_connect/read/write` 不在 offload 白名单 | **有意保留**（阻塞原语，见速查表事实 201：改它需把 unix I/O 全量协程化，收益低、风险高） |
| P2-7 字典缺键 `R1008` | **已在 M174 收口**（给了三条正路表：`d.get` / `d.has` / `d[k]`） |

---

## 四、一句话给 Ma 文档

> **PuXian 不做 HTTP/2。** 现代协议走 **HTTP/3**（`opts{"http3": true}`，自动 `Alt-Svc` 通告，
> 与 h1.1 同一条 handler 管道）；其余场合用 **HTTP/1.1**（keep-alive + gzip 已就绪）。
> h2c 升级请求会被**忽略并按 h1.1 正确服务**；h2 prior-knowledge 前导会被**明确拒绝（505）**。
> 零停机换二进制用 `SO_REUSEPORT`。
