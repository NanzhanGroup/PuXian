# BUG REPORT：vhost handler dict headers 仅透传 Content-Type（Location/Cache-Control 等响应头丢失）

- 报告人：清歌（ws-web 维护 / 文殊系统）
- 日期：2026-09-03
- 影响模块：PuXian runtime（runtime/runtime.c `px_vhost_normalize`）—— 非 ws-web 应用层
- 严重度：P1（功能缺陷：破坏标准 HTTP 语义，301 无 Location、缓存头丢失；不崩溃）

## 一、现象

`vhost(host, handler)` 的 handler 返回完整响应 `{"status":…, "headers": {…}, "body":…}` 时，
**headers 中除 `Content-Type` 外的所有自定义响应头都不会到达客户端**：

1. `ws-web/http.px:195` http→https 跳转返回 `{"status":301, "headers":{"Location": "https://…"}, "body":""}`，
   实测 301 状态码正常但 **Location 头丢失 → 浏览器不跳转**；
2. ws-web v0.6.0 静态缓存优化为 css/js/图片返回 `Cache-Control: max-age=3600`，
   实测该头无法到达浏览器 → 缓存策略静默失效；
3. 同路径下 `Set-Cookie` / `Content-Disposition` / `Access-Control-*`（CORS）等亦全部丢失。

## 二、最小复现用例（单文件，编译模式实测）

见同目录 `tests/bug_vhost_extra_headers.px`：

```px
def hh(req):
    return {"status": 301, "headers": {"Location": "https://new.example/go", "Cache-Control": "no-store"}, "body": ""}
vhost("h.test", hh)
def serve():
    px_serve(PORT, D, 10000)
spawn serve()
sleep(300)
r = http_request("http://127.0.0.1:" + str(PORT) + "/go", "GET", "", {"Host": "h.test"})
```

### 期望 / 实际

- 期望输出：
  ```
  status: 301
  has Location: true
  has Cache-Control: true
  ```
- 实际输出（`./tools/pxc build ws-web/tests/bug_vhost_extra_headers.px` 后运行）：
  ```
  status: 301
  has Location: false
  has Cache-Control: false
  ```
- 命令：`./tools/pxc build`（编译模式，解释模式同路径）

## 三、根因定位

`runtime/runtime.c` `px_vhost_normalize`（11707-11735），headers 分支只取 Content-Type 一项，其余全部忽略：

```c
LXValue h = px_dict_get(v, "headers");
if (h.type == PX_DICT) {
    LXValue ctv = px_dict_get_ci(h, "Content-Type");
    if (ctv.type == PX_STR) *ct = ctv.as.obj->as.str.data;
}
```

调用点（10802）的响应缓冲 `extra` 仅预填 `X-Request-Id`，未承接任何自定义头；
`pout->respond` 最终只输出 `Content-Type` + `extra` 中的头。

对照：普通 route handler 路径（非 vhost normalize）响应头可全量透传（examples/m23c_http_adv.px 中
`X-Test` 头客户端可收到）——缺陷专属于 vhost handler 归一化路径，即 ws-web 主路径。

## 四、修复建议（供 runtime 维护者东月参考）

1. `px_vhost_normalize` 增加 `char* extra, int extra_sz` 出参缓冲，遍历 headers dict 时把白名单头
   写入 `extra`（`Content-Type` 维持现状走 `*ct`，避免重复输出）；
2. 白名单建议：`Location` / `Cache-Control` / `Content-Disposition` / `Set-Cookie` /
   `Content-Language` / `X-Robots-Tag` / `Access-Control-Allow-Origin` / `-Methods` / `-Headers` /
   `Access-Control-Max-Age`；
3. **CRLF 注入防护**：头值写入前必须 `memchr` 检查 `\r`/`\n`，含则丢弃该头；`extra` 缓冲需从
   256 扩到 1024 并做 `snprintf` 边界截断（防多个长头溢出）；
4. 注：此前曾有一次实现该逻辑的 runtime 改动（commit a18ba04，vhost handler 响应头白名单透传），
   因未走正常流程被回滚（revert 0b64215）——该实现可作参考蓝本，但需按本仓库流程重新落地；
5. 修复后回归：`tests/bug_vhost_extra_headers.px` 应输出 `has Location: true` /
   `has Cache-Control: true`；并跑 ws-web http→https 301 实站验证。

## 五、复现物证

- 最小复现：`ws-web/tests/bug_vhost_extra_headers.px`（本提交一并入库）
- 受影响现网代码：`ws-web/http.px:195`（http→https 301 依赖 Location 头）
- 环境：pxc 0.1.0（普贤 PuXian · selfhosted M-B9a），Linux，编译模式
