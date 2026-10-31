# M97_PLAN · 连接复用生命周期缺陷修复（qg-issue 31 客户端 + 32 服务端）

> 状态：✅ **M97 完成（2026-09-10）**：S2（F31 客户端：h_exchange 协议感知 keep_alive
>   + 池死连接自动新建重发）与 S3（F32 服务端：px_send_all 全量写 + 响应入内核才交还
>   IDLE + 裸 send 全替换）一次落地；examples/m97_s2（4 PASS，旧 runtime pxi 对拍
>   ok=15/30 奇偶失败精确复现）+ m97_s3（6 PASS，Go keep-alive 单连接复用 100 请求
>   含 2MB 大响应 100/100 零悬挂零丢 + audit==100 对拍）新增套件全绿；全量回归
>   （m89_s3d 9 + m93_s2/s3 12 + m94_s2/s3 8 + m95_s2 14 + m95_s4 10 + m96_s2/s3 17 +
>   m82/m83_s6 + vm_ab 38P/0GAP/0F + diffcheck --all）+ 双自举证明 + pxi/pxi_vm 重链
>   吸收 M97 runtime + tag v0.2.0-m97。
> 来源：qingge（清歌）——Issue 31（Mahesvara proxy_forward 反代实测：http_request 客户端
>   keep-alive 误判 HTTP/1.0 死连接回池 → 复用失败奇偶失败）+ Issue 32（晨曦 token-cache
>   网关实测：http_serve_unix 服务端 keep-alive 长连响应悬挂/偶发丢失 → 1s fail-closed
>   误报「Approve 权限审批系统连接失败」）。清歌补发 Issue 32 后一并立项本里程碑；
>   原 M97（px_serve route/vhost handler 协程化，M95_S5_PLAN.md）**顺延为 M98**。
> 基线：v0.2.0-m96（1138f5f）。性质：**L0 runtime**（http_request 客户端连接池 +
>   http_serve/http_serve_unix 服务端 keep-alive 连接生命周期）。
> 分层：两 issue 均 runtime 网络层，业务 .px 代码零改动（Mahesvara 侧 0.9.4 已自行止血
>   崩溃层；token-cache 为 Go 客户端无法业务层绕过 → 须 runtime serve 侧根治）。

## 〇、一句话

清歌两个新 issue 同族（连接复用生命周期，方向相反）：
- **Issue 31（客户端）**：`h_exchange` 判定连接可复用只看响应头有无 `Connection: close`，
  不判 HTTP 协议版本 → HTTP/1.0 上游（无 Connection 头 + 有 Content-Length，发完即关）
  的死连接被误判可复用回池 → 第二次请求取死连接失败；且默认 retries 不覆盖「已取池
  连接」失败路径 → 无自动新建重连 → **奇偶失败**（1/3/5 成功、2/4/6 失败）。
- **Issue 32（服务端）**：http_serve_unix 被 Go 长连客户端（http.Transport keep-alive 复用）
  高频调用下偶发「请求已收到并处理（audit 全录）但响应未在 1s 内完整回到客户端」→
  token-cache fail-closed 误报。疑点集中在服务端 keep-alive 循环响应写完整性 /
  连接生命周期（非阻塞 send 不完整 / 事件循环不等可写 / 挂起期不照看）。

## 一、D0 侦察量化（2026-09-10，dongyue，基线 1138f5f）

### Issue 31（客户端 http_request 连接池）证据链

- **keep_alive 判定只看 Connection 头、不判协议版本**（runtime.c h_exchange 8614）：
  ```c
  if (strcasecmp(k, "Connection") == 0 && strcasecmp(v, "close") == 0) keep_alive = 0;
  ```
  响应行版本被 `sscanf(buf, "HTTP/%*s %d", &status)` **直接跳过**（8569）→ 协议版本从未
  解析。HTTP/1.0 响应带 Content-Length、无 Connection 头 → keep_alive 保持初值 1 →
  bi_http_request(8799-8800) `hpool_put` 回池。HTTP/1.0 发完即关 → 池中死连接。
- **池连接复用失败无自动重连（默认配置）**（bi_http_request 8745-8814）：
  ```c
  int retries = 1;                       // opts 缺省 → max_attempts = 1
  ...
  int max_attempts = retries;
  for (attempt...) {
      if (hpool_take(key, &slot) != 0) { ...新建连接... }
      if (h_exchange(...) == 0) { ...hpool_put/关闭... return d; }   // 成功
      ...close...
      if (attempt < max_attempts - 1) continue;  // 默认 max_attempts=1 → 永不重试
      return px_net_err("net: http_request 失败: 连接关闭");
  }
  ```
  第二次取到池中死连接 → h_exchange 写请求（内核缓冲可能成功）→ 读响应 EOF → -1 →
  默认 retries=1 → 直接返回 Err。奇偶失败 = 池中死连接命中即失败、下次新建又成功。
  即便用户配 retries>1，重试也是「整次请求重来」（新建连接），本可自愈——但默认值 1
  让最常见调用（反代连续回源）恰好无重试兜底。
- **HTTP/1.0 语义（RFC 7230 §6.3）**：HTTP/1.1 默认 keep-alive、显式 close 才关；
  HTTP/1.0 默认短连接、须显式 `Connection: keep-alive` 才可复用。h_exchange 未按此判定。
- 影响面：http_request/http_get/http_post 客户端全部经 h_exchange；s3_* 亦复用
  （9032）；http_unix 不池化每次新建（8863 注释）不受回池影响但共享 h_exchange 读逻辑。

### Issue 32（服务端 http_serve_unix keep-alive 长连）证据链

- **服务端连接 fd 一律非阻塞**（px_fd_nonblock，M88-B-S2 http_conn_worker 10307；
  px_evc_idle_put 11170 亦重申）。请求读已事件化兜底（px_recv_wait poll+recv）。
- **响应写是「单次裸 send 且不检查返回值」**（http_send_resp 10379-10387）：
  ```c
  char* out = px_http_build_response(resp, &out_len, &resp_keep_alive);
  if (out) {
      if (method_head) {...}
      if (out_len > 0) send(fd, out, out_len, 0);   // 非阻塞 fd：EAGAIN / 部分写不处理！
      xfree(out);
  }
  ```
  http_conn_worker 内另有 5 处裸 send（1109 注释自述；413/文件流/404 等路径同型）。
  非阻塞 fd 上大响应/瞬时高水位 → send 返回 -1(EAGAIN) 或部分字节 → **响应截断**，
  代码不重试直接进 keep-alive 判定。
- **事件循环只等可读、不等可写**（px_evc_idle_put 11174-11175）：
  ```c
  ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;   // 无 EPOLLOUT
  ```
  若 send 部分写/EAGAIN 后连接因无下一请求在途被交还 IDLE → 剩余响应数据**永不写出**
  （事件循环不关心 POLLOUT）→ 悬挂至 15s IDLE 超时 close → 客户端早 1s fail-closed。
  与清歌「已处理但响应未完整回客户端 + 坏连接池内逐步累积、重启顶 10 分钟」吻合。
- **M95-S2 handler 协程化挂起期连接不交还也不照看**：VM handler 帧协程执行期 worker
  已释放但连接仍 ACTIVE（http_pend_put stage=1 → return）——该期间若有慢 handler /
  fserve 投回排队延迟，客户端 1s 超时先到；Go http.Client 超时会 close 该连接 →
  服务端完成写响应到已关连接（send EPIPE 被屏蔽 1109）→ 读下一请求 recv EOF → close。
  短连接（每次新建）零失败与此不矛盾：慢路径只在复用长连上暴露。
- **服务端响应行固定 HTTP/1.1 + Connection: keep-alive**（px_http_build_response 10171/
  10230）——对 HTTP/1.0 客户端（无 keep-alive 头）会等连接关闭才判 body 完 → 悬挂。
  请求行版本已解析（10509 version[]）但响应未按请求版本回写。（低优先记录）

### 修复语义边界（不破坏既有正确行为）

- Issue 31 只改 keep_alive 判定与重试路径：HTTP/1.1 既有行为（默认 keep-alive、显式
  close 不回池）零变化；HTTP/1.0 仅显式 keep-alive 才回池；池连接失败默认也自动新建
  重试一次（对已取池连接：请求可能未发出 → 安全；POST 在连接建立成功后才写 → 重试
  新建连接重发幂等，与既有 retries 语义一致）。
- Issue 32 只改响应写完整性与连接生命周期：send 全量循环写（EAGAIN → poll POLLOUT）；
  事件循环 IDLE 登记补 EPOLLOUT 不必要——改为「写不完整不交还 IDLE」+ 同步阻塞式
  写完成（或写侧 poll 等待可写），保证交还 IDLE 前响应字节已全部入内核。HTTP/1.0
  请求按版本回 HTTP/1.0 响应（记录项，S3 视回归面定）。

## 二、设计（修复方案定稿）

### F31（Issue 31 客户端，runtime.c）

1. **h_exchange 解析协议版本 + 协议感知 keep_alive**：
   - 读响应行首行取版本（HTTP/1.0 / HTTP/1.1 / HTTP/2 等），不再 `%*s` 跳过；
   - `keep_alive` 初值 = (版本 == HTTP/1.1)（HTTP/1.0 默认 0）；
   - Connection 头：`close` → 0；`keep-alive` → 1（HTTP/1.0 显式复用）；
   - 无长度（读到 EOF）分支已置 0 不变（该路径连接必关）；
   - 顺带：Connection 头值比较已 strcasecmp 大小写不敏感 ✓（保持）。
2. **池连接复用失败自动新建重试**（bi_http_request 循环）：
   - 语义：本次 attempt 用的是**池中取出**的连接且 h_exchange 失败（死连接）→ 丢弃该
     连接（close）→ **不消耗 attempt**（或单独逻辑强制再试一次新建）→ 新建连接重发；
   - 实现建议：hpool_take 成功记 `from_pool=1`；失败路径若 from_pool && attempt 已是
     最后一轮 → 仍再试一轮（扩大 max_attempts 1 次或 goto 重来新建）；新连接再失败才
     按原 retries 语义返回 Err；
   - 安全性：池连接复用首次 IO 失败 = 连接已死（未发出或未收到任何业务响应字节）→
     重发安全（HTTP 请求-响应模型下该连接无半响应污染）。
3. http_unix 不池化不受 1 影响；s3 h_exchange 复用自动获得协议感知判定（其连接池
   同 hpool 机制），行为只会在 HTTP/1.0 上游从「错放死连接」变「正确关断」→ 净改善。

### F32（Issue 32 服务端）

1. **send 完整性（核心）**：新增 `px_send_all(fd, buf, len)`——非阻塞 fd 循环 send：
   EAGAIN/EWOULDBLOCK → poll(POLLOUT, 15s) → 续写；返回实际写完字节/失败。http_send_resp
   响应写、文件流写、413/404 等全部裸 send 替换。写失败（EPIPE/ERR）→ 连接置关闭
   （px_evc_close），不进 keep-alive 判定。
2. **交还 IDLE 前置条件补强**：响应字节全部入内核（px_send_all 成功）后才走
   `px_fd_readable_now / px_evc_idle_put`；写失败 → 直接 px_evc_close（不留半写连接）。
   事件循环 EPOLLIN/ET 不变（IDLE 期无写残留）。
3. **M95-S2 挂起期连接状态审查**：确认 handler 协程挂起期连接 ACTIVE 无 epoll 登记
   （不交 IDLE 正确——下一请求未读）；完成回调 → fserve_push → 重入 http_conn_worker
   循环顶 http_pend_take → http_send_resp（新 px_send_all）。核实无「完成但无人投回/
   投回丢失」路径（http_pend_take stage 判定 + fserve 队列背压均正确）。
4. **记录项**：HTTP/1.0 请求按版本回 HTTP/1.0 响应 + Connection 语义（S3 视复现与
   回归面决定是否纳入；Go 客户端为 1.1 非本 issue 触发面，低优先）。

## 三、S 拆分与验证门

| S | 内容 | 验证门 |
|---|---|---|
| **S1** | 立项 + D0 + 设计 + S 拆分（本 commit） | M97_PLAN.md 入库 |
| **S2** | **F31 客户端修复**：h_exchange 协议感知 keep_alive + 池连接失败自动新建重试 | examples/m97_s2：HTTP/1.0 模拟上游（Content-Length、无 Connection 头、发完即关）连续 N 次 http_request **零失败**（修复前 奇偶失败稳定复现）；HTTP/1.1 正常 keep-alive 复用回归（连续请求同连接）；HTTP/1.1 `Connection: close` 不回池；大小写混合 Connection 头 |
| **S3** | **F32 服务端修复**：px_send_all 全量写 + 交还 IDLE 前置 + 裸 send 全替换 | examples/m97_s3：Go/PuXian 长连客户端 keep-alive 复用同 unix 连接连续 100+ 请求零悬挂零丢；大响应（> socket buf，触发 EAGAIN/部分写）逐字节完整；服务端 audit 次数 = 客户端成功次数 |
| **S4** | 收口：全量回归 + 双自举 + pxi/pxi_vm 重链 + CHANGELOG/ROADMAP + tag v0.2.0-m97 | 全绿矩阵 |

## 四、回归面（S4 全量）

vm_ab（38P/0F）+ m89_s3d 9 + m93_s2/s3 12 + m94_s2/s3 8 + m95_s2 12 + m95_s4 10 +
m96_s2 8 + m96_s3 9 + m82/m83_s6 + diffcheck --all + 双自举证明 + pxi/pxi_vm 重链
吸收 M97 runtime + 本里程碑 m97_s2/s3 新增套件。

## 五、边界与后续

- M95-S5（px_serve route/vhost handler 协程化）**顺延 M98**（M95_S5_PLAN.md 保持有效，
  后续单独排期）；M98 之后评估 fserve/pool 默认 worker 收敛。
- 29/30（dns_txt/ed25519_keygen/图片/yaml 写等能力增强）未排期，业务侧无阻塞。
