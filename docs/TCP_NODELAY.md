# 服务端 socket 口径：accept 必须显式设 TCP_NODELAY

> 适用：所有基于 `px_serve` / `ws_serve` / `sse_serve` / `tcp_serve` / `http_serve_unix` 的服务端。
> 本文是 M313（第 184 轮 · 缺陷 539）的产物：晨曦生产实测后要求的口径收口。

## 一 一句话

**`tcp_connect_ex`（出站）从 M149 就给连接设了 `TCP_NODELAY`，而 `accept`（入站）这一侧一直漏着**
—— 同一条语义只做了一半。修法：在 `px_io_accept()` 里统一设，**所有服务端共用一个入口**。

## 二 为什么会多出 40ms

`px_serve` 的响应是**两次 `write`**（先响应头、再响应体）。没有 `TCP_NODELAY` 时：

```
服务端 write(头)  ──▶ 对端
服务端 write(体)  ──▶ Nagle：体很小、且上一个包还没被 ACK ⇒ **压住不发**
对端延迟 ACK（典型 40ms）
服务端收到 ACK   ──▶ 才发出体
```

⇒ **每条 keep-alive 响应多出约 40ms**。晨曦生产实测：

| | hdr→body gap | 单连接吞吐 |
|---|---|---|
| 修前 | **40.6ms** | 23.5 rps |
| 修后 | **0.02ms** | 1042–1622 rps |

（他们的做法是在生产补 3 行；每次 `dnf upgrade puxian` 都要手工重打，**已经漏过一次** ⇒ 要求并入上游。）

## 三 判据为什么是「状态位」而不是「40ms」

**本机回环上 ACK 是即时的**：我方实测「有 NODELAY / 无 NODELAY」回环 gap **恒 0ms**。
⇒ 用量墙钟做判据会买到**假绿**（CI 与开发机都测不出来）。

所以门 `examples/m313_accept_nodelay/` 的主判据是**可判定的状态**：

```
accept 之后  tcp_opt(cfd, {})  →  nodelay == true
```

`tcp_opt` 改完会 `getsockopt` **回读生效值**（M149 设计），所以这不是「相信我们设了」，而是「确认内核里是开着的」。
时间间隔只在门里作**信息项**打印，不作硬判据。

## 四 覆盖与边界

- 覆盖：`px_io_accept` 这**一个**入口 —— 门 [1.3]/[1.4] 守「调用点 ≥5」且「**无裸 `accept` 旁路**」。
  （本轮据此发现并收口了 `http_serve_unix` 里唯一一处裸 `accept()`：AF_UNIX 上设 NODELAY 会失败但被忽略，
  改走同一入口是为了「日后有人把 unix 监听换成 TCP 时不悄悄漏掉这个选项」。）
- 不受影响：QUIC（UDP，无 Nagle）· 已建立连接的 TLS（同一 fd，已覆盖）。
- 与 Go 对齐：`net.TCPConn` 默认 `NoDelay = true`。

## 五 写新服务端时的检查清单

1. accept 一律走 `px_io_accept`（不要自己写 `accept()` + EINTR 循环）。
2. 出站连接用 `tcp_connect_ex`（它默认 NODELAY）或 `tcp_opt(fd, {"nodelay": true})`。
3. 响应若分多次 `write`（头 / 体 / chunk），**必须先确认 NODELAY 已开** —— 否则长尾就是这个。
