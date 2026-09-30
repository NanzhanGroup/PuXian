# WebSocket 升级接管（`ws_stream`）

> M236 · 2026-10-01 · 需求来源：晨曦（wsa-chenxi）《px_serve / vhost 支持 WebSocket 升级接管》
> 判据载体：`examples/m236_ws_stream/`（35 断言 + 负控 3 道）

---

## 一 要解决的问题

一个已经占用 **443** 的 `.px` 服务（我们的 Ma）需要在**同一监听、同一进程**上接受 WebSocket 升级：

```
手机 4G/5G ──wss://<节点>/agent/<节点名>──► 各节点 9519
```

修前有三种做法，都不理想：

| 做法 | 问题 |
|---|---|
| 另起进程占 443 | 全站 HTTPS 前面多一个组件（运维面 +1，须自带监控/自愈） |
| Ma 让出 443、改听本地端口 | 架构改动大，SNI/Host 分发链断掉 |
| **`ws_serve`**（既有 WS 服务端） | 自行 `socket()+bind()+listen()`（只设 `SO_REUSEADDR`）⇒ 443 已被占用时**无法共存** |

而运行时里其实**已经有**「同端口接管」的机制：`http_stream`（SSE）—— 但它只挂在
`http_serve` 轨，`px_serve` 的 worker 没有这个钩子，WS 升级更是完全没有。

⇒ 本轮的缺口是**一条能力**，不是缺陷：**让请求管道把「HTTP 请求」升格为「WS 连接」**。

---

## 二 API

### `ws_stream(path, on_connect[, opts]) → bool`

与 `http_stream` **同构**（同一张路由表，用 `is_ws` 区分两类接管）。

```px
def agent(conn, req):
    # 连接元信息（M235 交付，零改动复用）
    p = ws_conn_path(conn)             # "/agent/cx-node-7"
    peer = ws_conn_peer(conn)          # "203.0.113.9:51234"
    tok = ws_conn_header(conn, "X-Token")
    q = req["query"]                   # 鉴权/路由信息也在这里

    ws_send(conn, "welcome")
    while true:
        m = ws_recv(conn, 2000)
        if m == null: break
        ws_send(conn, "echo:" + m)

ws_stream("/agent/cx-node-7", agent, {})
px_serve(443, ".", 30000, {})          # Ma 保持唯一入口
```

| opts | 语义 |
|---|---|
| `methods` | 默认 `["GET"]`；**只允许 GET**（RFC 6455 握手方法固定）——给别的 ⇒ `R1002` |
| `manual` | `true` ⇒ 运行时**不写 101**，由语言层调 `ws_reply_101(conn)` 完成握手（见 §三） |
| `headers` | **未实现** ⇒ 给了会 `R1002`（不静默忽略） |

回调签名 `fn(conn, req)`：`conn` 可直接用于 `ws_send / ws_recv / ws_close / ws_ping /
ws_heartbeat / ws_conn_path / ws_conn_header / ws_conn_peer`。

### `ws_reply_101(conn) → bool`

只有 `opts.manual: true` 的端点需要它：语言层做完**准入决策**后调用它完成握手
（`Sec-WebSocket-Accept` 仍由运行时计算 —— 语言层不必自己 SHA1+base64）。

---

## 三 语义

### 3.1 谁写 101

| 模式 | 101 由谁写 | 适用 |
|---|---|---|
| 默认（`manual` 缺省/false） | **运行时**，在调回调**之前** | 绝大多数场景（与 `ws_serve` 同序） |
| `manual: true` | **语言层**，调 `ws_reply_101(conn)` | 需要「先决策、后握手」：鉴权、子协议协商 |

`manual` 模式下的拒绝路径：**不调** `ws_reply_101` 即可（客户端拿不到 101 ⇒ 握手失败）。
当前建议配合 `ws_close(conn)` 让客户端立刻结束等待。

### 3.2 回调返回之后

回调返回后，运行时**保持连接**并跑一个泵循环（回 `ping`、响应 `close`），直到对端关闭 ——
与 `ws_serve` 的 `ws_conn_worker` 第 4 步**逐句同构**。

⇒ 语言层两种写法都成立：
- 在回调里自己 `while true { ws_recv(...) }` 收发（本门用法）；
- 回调只做握手与注册，之后由泵循环兜底（帧会被丢弃）。

### 3.3 路由优先级

接管点位于 **`Upgrade: h2c` 处理之后、公共管道（CORS/限流/vhost/路由/静态/.px）之前**：

- WS 路由**优先于** vhost/handler/静态 —— 正是「同一监听、同一进程」所需；
- `Upgrade: h2c` 的既有口径（**忽略升级、按 HTTP/1.1 服务**）一字未动；
- **未注册的路径**（哪怕带 `Upgrade: websocket`）⇒ 落回普通 HTTP 管道（本门 [5] 层判据）；
- **普通请求**（无 `Upgrade`）⇒ 完全不受影响（本门 [6] 层判据）。

### 3.4 与 SSE 路由互不串扰

两类接管共用 `g_stream_routes`，但：

- `stream_match`（SSE）**排除** `is_ws` 路由；
- `stream_match_ws` 只认 `is_ws` 路由；
- 同一路径**不允许**既是 SSE 又是 WS ⇒ 第二个注册者 `R1002`（静默覆盖会让先注册者永久失效）。

---

## 四 取舍（务必知悉）

### 4.1 WS 会话期间占用一个 worker 线程

接管是**同步**的（与 `http_stream` 的 SSE 接管同构）：从握手到泵循环结束都在调用方线程里。
`px_serve` 的 worker 数 = `opts.max_conn`（默认 **32**，上限 256）。

⇒ **N 条并发 WS 长连接会占掉 N 个 worker**，剩余 worker 服务 HTTP。

- 多节点路由（每节点 1–2 条）通常远小于 32，**够用**；
- 若 WS 连接数会接近 `max_conn`，**请调大 `max_conn`**。

> 这一条是本轮**有意**的选择：与既有 SSE 接管保持同一模型、不引入新的连接所有权语义
> （那会牵动 `PxPend` 槽、事件循环 IDLE 机制与 M235 的引用计数）。
> 「把 WS 会话挪进帧协程以释放 worker」登记为后续候选。

### 4.2 轨间覆盖

| 轨 | WS 接管 | 说明 |
|---|---|---|
| `px_serve` | ✅（含 TLS） | 复用该轨**已完成握手**的现成 `PxConn` ⇒ SNI 多证书照常 |
| `http_serve` / `http_serve_unix` | ✅（**仅明文**） | 该轨是 fd 基、无现成 `PxConn` ⇒ 按 `stream_takeover_conn` 同法即时新建；与 `http_stream` 的既有边界一致 |

需要 **TLS + WS** 请用 `px_serve` 轨（Ma 的场景即此）。

---

## 五 与既有能力的关系

| 能力 | 监听 | 升级接管 | path/对端可见 |
|---|---|---|---|
| `ws_serve(port, handler)` | 自己 bind（`SO_REUSEADDR`） | 自己读请求头 | M235 起 ✅ |
| `http_stream(path, fn)`（SSE） | 复用 `http_serve` | ✅（SSE） | — |
| **`ws_stream(path, fn)`**（本轮） | **复用 `px_serve` / `http_serve`** | ✅（WS） | ✅（复用 M235） |

`ws_serve` 的独立端口场景**行为完全不变**。

---

## 六 覆盖边界（如实）

- `http_serve_unix`（Unix socket）轨未单独覆盖（走同一 `http_conn_worker` 分支）；
- TLS + WS 在本门用明文；`px_serve` 轨的 TLS 路径复用 M235 的 `PxConn`，未单独覆盖；
- `opts.headers` 未实现（101 的 `Upgrade`/`Connection`/`Sec-WebSocket-Accept` 由 RFC 6455 规定）；
- `manual` 模式的**拒绝**路径目前只有「不写 101 + `ws_close`」两种手段；专用的
  「回普通 HTTP 响应」API 待后续；
- 语言层在回调里自行 `spawn` 读循环的用法未覆盖（本门为同步回调 + 运行时泵循环）；
- 泵循环丢弃**未被回调消费**的文本/二进制帧（与 `ws_serve` 同语义）。

---

## 七 相关

- 连接生命周期与引用计数：`docs/WS_CONN_LIFECYCLE.md`（M235）
- 判据：`examples/m236_ws_stream/verify.sh`（35 断言 + 负控 3 道）
- 需求原文：晨曦《ws-upgrade-feature-request-chenxi.md》（2026-09-30）
