# HTTP 响应头的行级防护（M435）

> 缺陷 902 / 903 · 门 `m435_resp_header_guard` · 2026-11-22

## 1 一句话

请求头在 M434 已经「非 tchar / 值含 CTL ⇒ 400」了；**响应头**这边却没有同一份判定 ——
`px_serve` 主路有 `px_hdr_append`（M109-S1/S4），但 `px_http_build_response` 自己手搓
`"%s: %s\r\n"`。同一件事两份实现，防护只落在其中一份上。

## 2 修前实测（http_serve，`/tmp/m435probe`）

| handler headers | 修前响应 | 性质 |
|---|---|---|
| `{"X-A": "a\r\nInjected: yes"}` | 多出 `Injected: yes` 一行 | 响应头注入（响应拆分） |
| `{"X-A\r\nInjected4": "1"}` | `X-A` + `Injected4: 1` | 名字方向同样可注入 |
| `{"X-A": "a\rInjected2: yes"}` / 裸 LF | 同样注入 | 裸 CR / 裸 LF 都放行 |
| `{"X Bad": "1"}` / `{"": "1"}` | 原样写出 | 名非 tchar / 空名 |
| `{"Content-Length": "0"}`（体 `ok`） | `Content-Length: 2` + `Content-Length: 0` | 重复 CL ⇒ 不同中间件取不同值 |
| `{"Transfer-Encoding": "chunked"}`（体未分块） | `TE: chunked` + `CL: 2` | RFC 9112 §6.1 明禁；响应走私面 |
| `{"Connection": "keep-alive"}` | 两条 Connection | 重复 |
| `{"Date": "x"}` | `Date: x` | 覆盖服务器该自己生成的字段 |
| `{"status": -5}` | `HTTP/1.1 -5 status code -5` | 非法状态行 |
| `{"status": 999}` / `{"status": 100}` | 原样 / 100 当最终响应（体仍带 CL） | 状态码域 + 1xx 语义 |

## 3 修法

```
px_http_build_response
    headers 一律走 px_hdr_append(h, extra_headers, 0, sizeof, 0, &dropped)
    ⇒ CRLF/非法名丢弃 + 拒绝名单（CL/TE/Connection/Keep-Alive/Date/Server…）+ 预算 + 多值头

sse_start / http_stream（手写拼头路径）
    if (!px_hdr_line_ok(name, value)) { 计数; continue; }      // 未消费额外头
    if (!px_hdr_line_ok(dname[i], v)) { 计数; v = dval[i]; }   // 覆写值非法 ⇒ 回落默认

状态码（px_http_build_response）
    status < 200 || status > 599 ⇒ 500（1xx 是中间响应，本构造器发的是最终响应）
```

计数 `g_hdr_drop_syntax` + 限频 stderr —— 与 px_serve 的 `g_hdr_drop_crlf` 口径一致，**不静默**。

## 4 门判据（27 条 · 自测 7/7）

- 注入面：R1/R2/R3（值 CRLF/CR/LF）、R6（名 CRLF）、R16（多值项 CRLF）⇒ 响应里不得出现注入标记
- 非法名：R5（名含空格）、R7（空名）⇒ 不得写出
- 重复/越权头：R8（CL 计数=1）、R9（TE 不出现且 CL=1）、R10（Connection=1）、R11（Date 不出现）
- 防过度修复：R12（HTAB 保留）、R13（空值保留）、R14（多值两行都在）
- 状态码域：R17/R18/R19（-5/999/100 ⇒ 500）、R20（204 合法）
- 两路同口径：R21 在 `px_serve` 路径跑同一组非法头
- 负控 N1：把手搓拼装注回 + 摘状态码域 ⇒ 注入复现 · CL=2 · -5 不再 500
  （**前提自证**：负控服务必须存活且 R0 仍 200 —— 缺陷 898 的教训：NA 不等于「放行」）
- G1 环境守卫 · G2 `runtime.c` 原样回滚（sha + git status）· G3 不留监听

## 5 边界（明确未做）

1. `http_request` / `http_unix` 的**请求**头仍手搓 —— 客户端方向，攻击面要应用自己把关（下一轮候选）
2. WS 握手重建头来自**已过 M434 校验**的 `req.headers` ⇒ 本轮不加二次校验（纵深防御候选）
3. `{"status": 204}` 仍会写 `Content-Length: 0`（RFC 9110 §15.3.5 语义上不宜，下一轮）
4. `px_vhost_normalize` / `route_normalize` 的 headers 路径是否与本轮两条同口径 —— 本轮只证了
   `px_serve` 与 `http_serve` 两路；其余路径未在本门覆盖面内

## 6 为什么这不是「产品退化」

`px_serve` 主路的防护（M109-S1/S4）一直都在，本门 R21 对拍即证明它修前就绿。
本轮暴露的是**覆盖面的不一致**：同一份契约被写成两份实现，只有一份长着判据。
门的意义就是把这个不一致钉住 —— 任何新写的拼头路径都必须过 `px_hdr_line_ok` / `px_hdr_append`。
