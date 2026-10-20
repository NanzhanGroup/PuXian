# HTTP 客户端的「自管请求头」口径（M316 · 缺陷 550）

## 一 一句话

运行时自己会补的请求头，**调用方给了就不补**；给了但值是空的，**当没给**。

## 二 谁算「自管头」

| 头 | 运行时什么时候补 | 调用方能否覆盖 |
|---|---|---|
| `Host` | 总是（`http_request` = `host:port`；`http_unix` = `localhost`） | ✅ 能 |
| `User-Agent` | 总是（`PuXian/0.1`） | ✅ 能 |
| `Content-Type` | 有 body 且调用方未给时（`application/x-www-form-urlencoded`） | ✅ 能（M84-S1） |
| `Content-Length` / `Transfer-Encoding` | 有 body 时按长度感知补 | ✅ 能（M84-S1 / M144） |
| `Connection` | 总是（`http_request` = `keep-alive`；其余 = `close`） | ❌ **不能**（见 §五） |

## 三 取值判定（`px_hdr_val` 三态）

| 返回 | 场景 | 行为 |
|---|---|---|
| `1` | `Host: example.com` | 用调用方的，**不补**内部头 |
| `2` | `Host: ` / `Host: \t ` | 视作**未提供** ⇒ 用内部值，且**丢掉这一行** |
| `0` | 没有这一行 | 用内部值 |

为什么 `2` 要单独处理：HTTP/1.1 要求 `Host` **非空**。发一个空 Host 比发两个 Host
更容易被直接 400 —— 「调用方写了个空值」几乎总是模板拼接的意外，不是意图。

判定细节（与 `strncasecmp` 一致）：
- **头名大小写不敏感**（`host:` 等价于 `Host:`）
- **必须顶着冒号**：`X-Content-Type` 不会被当成 `Content-Type`
- 值是**去首尾空白**后的内容（`Host:   a.com  ` ⇒ `a.com`）

## 四 写调用方代码时的三条

```px
// ① 虚拟主机 / 反代：直接给 Host，不要自己拼「空 Host」
http_request("http://127.0.0.1:8080/api", "GET", null, {"Host": "api.example.com"})

// ② 想用默认值：不传这一项即可（不要传空串）
http_request(url, "GET")                       // Host = host:port，UA = PuXian/0.1

// ③ 想「删掉」UA：现在做不到 —— 传空值等于没传（运行时仍会补 PuXian/0.1）
```

⚠️ 第 ③ 条是**已知限制**：HTTP/1.1 不要求 `User-Agent`，但本运行时恒补一条。
若将来需要「显式不发」，要加一个 `opts.omit_default_headers`（登记为候选）。

## 五 修前是什么样（缺陷 550）

```
GET /api/reg/ HTTP/1.1
Host: 127.0.0.1:8080        ← 运行时补的
User-Agent: PuXian/0.1      ← 运行时补的
Connection: keep-alive
Host: api.example.com       ← 调用方给的（被原样照抄）
User-Agent: Ma/1.0
```

受害面：
- nginx 报 `400 Bad Request`（`Host` 重复是**硬错误**，不是忽略其一）
- Go `net/http` 反代把首条当权威 ⇒ 路由到错的后端
- 云 WAF 判协议违规
- 400 之后连接不可复用 ⇒ **连接池复用率归零**（晨曦的生产观测）

## 六 与 Go `net/http` 的语义对照

| | Go | PuXian（M316 起） |
|---|---|---|
| `req.Header.Set("Host", …)` | **无效**（Host 由 `req.Host` 字段决定） | **有效**（覆盖内部 Host） |
| 其它头同名多次 | `Header.Set` 覆盖 / `Add` 追加 | 调用方给的那一行**原样发**（运行时不再补） |
| 空值 | 发一个空值头 | **丢掉该行**（视作未提供） |

⇒ 我们的口径比 Go 更**宽容**（允许覆盖 Host），但**不会**产生重复头。

## 七 覆盖边界

- `px_s3_exec` / `bi_http_get_stream` / `px_http_once` **不适用**：内部构造，无 `headers` 注入面
  （报障曾把它们列为「同族」，已核查排除）。
- `Connection` 仍由运行时决定（见 §二）—— 改它要同步连接池回收语义，属独立改动。
- 代理（`opts.proxy`）与 TLS（https）走**同一段构造代码**，未单独门覆盖。
- sse 客户端（`sse_connect(..., {headers})`）走同一套判定，但本仓只有**静态**判据
  （动态抓首包需要事件循环装置），配 `m137_sse_connect_ex` 门回归。

## 八 门

`examples/m316_http_dup_host/` —— 22 通过 / 0 失败（`--neg-skip` 档 15/0）：

- TCP 6 case + Unix 4 case，判据是**每个头名恰好 1 条**（看线上字节，不看解析结果）
- 判据自证（合成 hex：单 Host 必绿 / 双 Host 必红）
- 负控：忠实撤回 ⇒ 编译器与语料**都重建**（前提自证 sha 不同）⇒ 缺陷复现 ⇒ 还原回绿
