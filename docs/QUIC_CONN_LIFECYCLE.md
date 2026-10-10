# QUIC listener / 连接 的生命周期口径（M296 建立）

> 本文回答三个**用户会直接踩到**的问题，每个都对应一个已修缺陷或一条硬口径：
> ① 一条 `quic_listen` 的 listener **能不能**连续接多条连接？（M296 缺陷 511/512：修前不能）
> ② 谁负责关 **UDP socket**？（fd 归属判据：只有独占者才能关）
> ③ `quic_accept` 收到「不是给新连接的包」时该怎么办？（丢弃 + 继续等，**不是**判死）

---

## §1 fd 归属：连接与 listener 可能**共享**同一个 UDP socket

`quic_listen(port)` 建一个 UDP socket；此后**手写服务端**形态下每条被接受的连接
**不新开 socket**，而是复用 listener 的：

```c
qc->fd = ql->fd;        /* runtime_quic.c：bi_quic_accept / quic_srv_new_conn 两处 */
```

⇒ **判据（唯一）**：`close(fd)` 之前先问「这个 fd 是不是某个**活跃 listener** 的」：

| 形态 | `qc->fd` 来源 | 谁负责关 |
|---|---|---|
| `quic_listen` + `quic_accept`（手写服务端） | **共享** `ql->fd` | **listener**（`quic_close_listener`） |
| `px_serve` / `quic_h3_listen`（托管） | **共享** `ql->fd` | **listener** |
| `quic_connect` / `quic_connect_resume`（客户端） | `socket()` **独占** | 连接自己（`quic_close`） |
| `quic_migrate` 之后 | 新 `nfd` **独占**（旧 fd 在切换时关掉） | 连接自己 |

实现 = `quic_fd_shared_with_listener(fd)`（扫 `g_qlis` 的 `used && fd`）。

**为什么这条必须以「归属」判、不能以「是不是服务端连接」判**：
`owner_listener` 只在**托管**路径被设置，手写服务端的连接它是 `0`
⇒ 用 `owner_listener` 判会漏掉手写服务端（缺陷 511 的第一版就是这样想的，且正是被漏掉的那半）。

---

## §2 `quic_accept`：收到的包**不一定**是给新连接的

UDP 是无连接的，listener 的 socket 上会混进：

* **上一条连接的残包** —— 最常见的是 **PMTUD probe**（远大于 Initial）、ACK、`CONNECTION_CLOSE`；
* 扫描 / 探测流量、损坏包、其他协议的包。

**口径（M296 缺陷 512 修后）**：`accept` 的语义是「**在 deadline 之前等到一条有效的新连接**」，
所以：

```
poll → recvfrom → 解析 QUIC 长头（失败 ⇒ 丢，继续）
     → 建临时 conn → 喂首包
         · 成功 → 泵握手 → 返回 conn id
         · 失败（DROP_CONN / 解密失败 / 握手泵不起来）⇒ **丢弃这个包、清理、继续等**
deadline 用尽 ⇒ 返回 -1
```

* **不许**「首包无效就整次返回」—— 那等于让**任何一个噪声包**把 listener 的下一条连接挡在门外
  （实测：一条 `quic_close` 过的连接留下的 PMTUD probe 就能做到）。
* **不许静默**：丢弃要计数 + 限流报告（首次 · 每 1000 次），带 `stage / rv / len`。
* deadline 保护**不变**（默认由调用方给 `timeout_ms`）。

---

## §3 手写服务端的正确写法（`quic_listen` + 循环 `quic_accept`）

```px
var lst = quic_listen(19931)
while true:
    var c = quic_accept(lst, 12000)      # 超时 ⇒ -1；噪声包会被自动丢弃，不影响后续
    if c <= 0: continue
    var d = quic_recv(c, 2048)
    quic_send(c, "echo:" + d)
    quic_close(c)                        # ⚠️ 不会再关掉 listener 的 socket（M296 缺陷 511 已修）
quic_close_listener(lst)                 # 由它负责关 UDP socket
```

**修前**这段循环**跑不到第二轮**：第一次 `quic_close(c)` 就把 listener 关掉了
（且 `accept` 一旦撞上残包就判死）。两个病灶**各自独立**，见 `CHANGELOG.md` M296。

---

## §4 诊断开关

| 开关 | 作用 |
|---|---|
| `PX_QUIC_VERBOSE=1` | 把 **ngtcp2 内部日志**打到 stderr（本轮用它把 `DROP_CONN` 从「一个错误码」变成「`pkt read packet 1406 left 0` 之后就没了」） |
| `quic_pool_stats()` | listener 槽 / 连接槽 / cid 表使用量与失败计数（M295 新增 · 13 键） |
| `quic_accept` 的丢弃报告 | `[quic] accept: 丢弃无效包（<stage> rv=<rv> len=<len>）—— 累计 N 次；继续等待下一个包` |

---

## §5 门与覆盖边界

* 门：`examples/m296_quic_accept_reuse/`（缺陷 511/512 常设化 · 负控 A/B/C）。
* 同族门：`m293_h3_robustness`（托管 listener 健壮性）· `m294_h3_qpack_share`（并发连接 QPACK 会话）
  · `m295_quic_slot`（连接槽位耗尽）。
* **未覆盖**（如实）：噪声包洪泛下的 accept 时延 · 托管路径的端到端观测 ·
  `quic_migrate` 的同族 fd 处理（缺陷 504 待修）。
