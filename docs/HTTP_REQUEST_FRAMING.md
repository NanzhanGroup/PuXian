# HTTP/1.1 请求体「长度声明」契约（M432 · 缺陷 891 / 892）

> 本文件是「服务端如何决定一个请求体到哪里结束」的**唯一口径**。
> 门：`selfhost/check_request_framing.sh`（registry 名 `m432_request_framing`）。

## 1 为什么有这份口径（修前实测，px_serve，2026-11-22 12:4x）

```
请求                                                    修前        说明
POST / CL: 3000（max_body_size=1024）                    413  ✅     本来就对
POST / Transfer-Encoding: chunked（体 3000）              413  ✅     本来就对
POST / CL: 5 + Transfer-Encoding: chunked                 200  ❌    静默按 chunked，CL 被丢弃
POST / CL: 5 + CL: 10                                     200  ❌    最后一个赢（len=10）
POST / CL: abc                                            200  ❌    atoi=0 ⇒ 体当空，后续字节按下一请求解析
POST / CL: -1                                             200  ❌    负值当「无体」
POST / Transfer-Encoding: gzip                            200  ❌    不支持的传输编码被静默忽略
```

四处 `content_length = atoi(v)`：`http_conn_worker`（http_serve / http_serve_unix）、
`px_conn_worker`（px_serve），以及两处客户端读响应的路径（本轮**未**改，见 §5）。

**为什么这是安全面**：CL 与 TE 并存、重复 CL 冲突、非数字 CL 这三种「边界含糊」，
恰好是反向代理与源站各取不同读法时的**请求走私**（RFC 9112 §6.2 / §6.3）入口：
代理按第一个 CL 切、源站按最后一个 CL 切 ⇒ 攻击者的第二个请求被夹带进下一个连接。

## 2 口径（服务端最小集合，宁拒不含糊）

| 情形 | 处置 |
|---|---|
| `Content-Length` 非纯十进制（`abc` / `-1` / `+5` / 内嵌空格 / 超 18 位） | **400** |
| 多个 `Content-Length`，数值**相同** | 接受（RFC 允许；值为该数） |
| 多个 `Content-Length`，数值**冲突** | **400** |
| `Content-Length` 与 `Transfer-Encoding` **同时**出现 | **400**（不猜、不「TE 优先」） |
| `Transfer-Encoding` 不是**唯一** `chunked`（如 `gzip` / `chunked, gzip`） | **501** |
| `Content-Length` > `INT_MAX` | **413** |
| `Content-Length` > `opts.max_body_size` | **413**（既有行为，不退化） |
| 首尾 OWS（`"  5  "`） | 接受（合法容忍） |

实现：`px_cl_strict` / `px_te_strict` / `px_framing_feed` / `px_framing_resolve`
（`runtime/runtime.c`，M432 收敛为**一份**判定，两个服务端 worker 共用）。

任何拒绝：`Connection: close`，且**先 `px_reject_drain` 再关**。

## 3 缺陷 892：拒绝响应会被 RST 吞掉（判据自身的坑）

拒绝时客户端往往**已经**把体发出来了。直接 `close()` 会让内核在接收队列非空时发 **RST**，
客户端可能**收不到**这条 400 —— 实测同一请求，客户端看到的是空响应。

修法：`px_reject_drain(fd)` —— 先把「已经在线上的」未读字节吸干（≤64KB，单次 20ms 超时），
再关连接。门里 P14 专门守这一点：只看状态码不看「响应真读得到」会假绿。

## 4 覆盖边界（如实登记）

- ✅ 覆盖：`px_serve`（`px_conn_worker`）与 `http_serve` / `http_serve_unix`（`http_conn_worker`）的**请求**侧。
- ⚠️ **未**覆盖：客户端读**响应**时的 CL 解析（`px_http_request` / `http_stream`）——另轮；
  服务端读响应侧的畸形 CL 只会影响「本进程怎么读上游」，不构成对外走私面。
- ⚠️ 未做：`Expect: 100-continue` 的早拒（拒绝时仍会先把体吸干再关）。
- ⚠️ HTTP/2 / HTTP/3：帧层自带长度语义，不经本口径（H2/H3 各自的 framing 由协议栈负责）。

## 5 相关里程碑

- **M432**：本口径 + 门 `m432_request_framing`（306 门）+ 缺陷 891（四种静默形态）/ 892（RST 吞响应）
