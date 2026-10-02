# WebSocket 连接的生命周期与并发契约（M235 建立）

> 起因：晨曦 ws-edge（PuXian 版）在 **m231** 上「负载下一次真实中转即崩」，
> 内核记录 `segfault at 7f...b002c ip ...498601`（`mbedtls_debug_print_msg` 读 `ssl->conf`）。
> M235 钉死根因 → 修复 → 并把「什么能并发、什么不能」写成契约，避免同类再撞。

## 一 根因（缺陷 353）

`px_conn_close()` 会做 `mbedtls_ssl_free()` + `free(c->ssl)`，而 `px_conn_read/write`
**只在入口**查 `c->closed`。两条线程可以这样交错：

```
T1: 检查 c->closed 通过  →（还没进入 mbedtls_ssl_read）
T2: px_conn_close(c)     → free(c->ssl)；c->ssl = NULL
T1: mbedtls_ssl_read((mbedtls_ssl_context*)c->ssl, …)   ← 用已释放的上下文 ⇒ UAF
```

崩溃点落在 `mbedtls_debug_print_msg`（读 `ssl->conf->f_dbg` @0x28）——
因为 `shutdown(fd)` 唤醒阻塞读之后，mbedTLS 走**错误分支**并尝试打印调试信息，
而那时 `ssl` 已被释放。**该崩点与「传给 debug 的字符串」无关**，是控制块本身失效。

本机最小复现器（`examples/m235_ws_conn_uaf/ws_uaf_repro.px`）：
主线 `ws_recv(cli)` 阻塞 + spawn 出的线程 150ms 后 `ws_close(cli)`
⇒ 修前 **9/10 崩溃（rc=139）**，核心栈
`mbedtls_ssl_read ← px_conn_read ← ws_read_frame ← bi_ws_recv ← ws_conn_worker`。

## 二 修法：引用计数 + 延迟释放

```
px_conn_acquire(c)    // 1 = 拿到引用（此后可安全访问 c->ssl）；0 = 已关闭/已释放
px_conn_release(c)    // 释放引用；归零时执行挂起的资源释放
px_conn_close(c)      // 标记 closed + shutdown(fd, SHUT_RD)；若无人使用则就地释放
px_conn_owner_free(c) // 仅对象创建者：close + 释放对象（有使用者在读则延后）
```

- `refs` 是**使用者**计数（不含创建者）⇒ 无人并发读写时，`close` 的释放时机与修前一致
  （不引入新的延迟与泄漏）；只有「确有人在阻塞读」时才把释放推后到它退出。
- `shutdown()` 移进 `close` **无条件**执行（只关读方向）——
  ① 唤醒阻塞在 `mbedtls_ssl_read` 的线程，否则 `refs` 永不归零、资源永不释放；
  ② 只动读方向 ⇒ **不打断可能正在进行的写**。
- `px_conn_read/write` 内部包 acquire/release ⇒ 所有调用点自动受益，无需逐个改。

## 三 什么可以并发、什么不可以（**必读**）

⚠️ 本仓的 mbedTLS 3.6.2 预编译库**未编线程支持**（`MBEDTLS_THREADING_C` 关，
config 2100/2111/3630 全注释）⇒ **库内无互斥**。这是 M101 就记下的前提，
M101 处理的是**握手期**的共享对象（per-连接 RSA 私钥 clone + session cache 加锁），
M235 处理的是**连接建立后**的并发使用。

| 场景 | 是否安全 | 说明 |
|---|---|---|
| 一条线程读、另一条线程**关闭**同一连接 | ✅ **安全**（M235 起） | 释放被推后到读者退出 |
| 一条线程读、另一条线程**写**同一连接（都不关闭） | ⚠️ **仍是 data race** | mbedTLS 上下文非线程安全；实践中通常能跑，但**不是保证** |
| 两条线程同时读同一连接 | ❌ 不安全 | 同上 |
| 两条线程操作**不同**连接 | ✅ 安全 | 各自独立的 ssl 上下文 |

⇒ **「双向中转」若用 `spawn` 两条泵（一读一写）**，属于上表第二行。
`ws-edge` 的 `relay.px` 正是这个形态 —— 本机实测该形态在修复后 6/6 稳定，
但**这是实测结论，不是语言保证**。

**当前推荐的稳妥写法**（单执行流 + 超时轮询，完全避开并发）：

```px
def pump(cli, up):
    while true:
        m = ws_recv(cli, 50)        // 带超时：无数据返回 null，连接仍可用
        if m != null:
            if ws_send(up, m) == false: break
        n = ws_recv(up, 50)
        if n != null:
            if ws_send(cli, n) == false: break
```

（这条写法不需要 `spawn`，也就没有并发访问同一个 ssl 上下文的问题。）

## 四 连接元信息（M235 新增）

修前 `ws_server_handshake` 读完请求头即 `free(head)` ⇒ **path 永久丢失**，
且没有任何 API 能拿到 path / 请求头 / 对端地址 ⇒ 多节点路由与单 IP 限流都做不了
（ws-edge §七.3 / §七.4 记载的两处缺口）。

| API | 返回 | 语义 |
|---|---|---|
| `ws_conn_path(conn)` | `str` | 握手请求行里的 path（如 `/agent/cx-node-7`）；无则 `""` |
| `ws_conn_peer(conn)` | `str` | 对端 `"ip:port"`（AF_INET / AF_INET6）；无则 `""` |
| `ws_conn_header(conn, name)` | `str \| null` | **握手那一刻**的请求头，大小写不敏感；未命中 `null` |

- 连接不存在 / 信息缺失 ⇒ 空串或 `null`，**不抛错**（与 `ws_send`/`ws_recv` 的
  「连接不在 ⇒ 安静失败」口径一致）。
- 握手头原文随连接留存（`g_ws_conns[].hdrs`），在**槽复用时**归还 ⇒ 泄漏有界
  （≤ `MAX_WS_CONNS` = 256 份）。
- 绑定的是**握手时**的信息；连接建立后客户端再发的帧不在此列。

## 五 门与覆盖边界

门：`examples/m235_ws_conn_uaf/verify.sh`（8 层）
- [1] 静态：引用计数 API / read-write 包裹 / `SHUT_RD` / `owner_free` / 旧写法已清除
- [2] 形态一（阻塞读 + 并发 close）**12 次 · 崩溃 0**
- [3] 形态二（ws-edge 双向中转真形）**6 次 · 崩溃 0**
- [4] 元信息：path / peer / header（含未命中 ⇒ `null`）
- [5] 负控 A（判据灵敏度 · **必崩侧**）：注入必然 SIGSEGV 的替身 ⇒ 检测**必须**报「已崩」
- [6] 负控 B（判据灵敏度 · **必活侧**）：注入必然存活（打 `M235-SURVIVED`）的替身
      ⇒ 检测**必须**报「存活」。两侧合起来**同时排除「恒绿」「恒红」**
- [7] 覆盖边界登记
- [8] 分配器配对：`PxConn` 由 `xmalloc` 创建 ⇒ 释放必须 `xfree`（缺陷 354）

⚠️ **负控形态沿革（M242 缺陷 406 / 408 —— 全量门逼出来的）**

原形态是「忠实撤回 M235 的引用计数 ⇒ 期望必崩」。随修复叠加，该竞态**被多层防护逐步收窄**：

| 撤回层数 | 实测崩溃 |
|---|---|
| 撤一层（引用计数） | M235 时 9/10 ⇒ M240 后 **0/6**（认领闸门兜住） |
| 撤两层（+ 认领闸门） | **0/6** |
| 撤三层（+ `close` 里的 `shutdown`） | **1/6**（rc=139 core dumped） |

⇒ **「撤回修复」不可靠复现、不可作判据**（假绿风险 > 判据价值，违反«门不许假红假绿»）。
⇒ 改用**判据灵敏度 · 双向**：直接验证「检测机制有鉴别力」。
⇒ 「修复是否必要」由 **[1] 静态层** + **[8] 分配器配对** + 本页登记覆盖。

⭐ 通用教训：**多层防护叠加后，「撤回一层」不再是有效负控** ——
   负控必须在**当前代码**上仍然**可判定地**判红，否则应当**换判据**，
   而不是一味「换更强的撤回」（撤回越多、越贴近修前，越可能变成另一次「碰巧绿」）。

**覆盖边界（如实）**：
- 未覆盖「两个执行流同时 read + write（都不关闭）」—— 见 §三，那一行仍是 data race。
- 未覆盖 ws 路径 `PxConn` 对象本身的释放：仍是既有取舍「**对象永不 free**」
  （避免悬垂指针）⇒ 每连接泄漏 `sizeof(PxConn)` ≈ 16.4KB。本轮**未动**该契约。
- 未覆盖 wss **客户端**连接的单独用例（共用 `px_conn_read/write`，理论上同修）。

## 六 相关

- M101：并发 TLS **握手**的共享对象修复（per-连接 RSA clone / session cache 加锁）
- M211（缺陷 265）：`px_conn_read` 明文分支不重试 EINTR ⇒ STW 被误判成对端关闭
- M182/M183（缺陷 197/198/199）：native 桥 GC 根面 + 根栈「交棒窗口」
- 本页：连接**生命周期**面的并发契约
