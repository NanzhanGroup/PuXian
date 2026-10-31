# M99_PLAN · px_serve 连接级事件化 IDLE —— keep-alive 空闲连接不占 g_pool worker

> 状态：✅ **M99 完成（S2 实现 + S3 验证 + S4 收口，tag v0.2.0-m99）**。
> 基线 v0.2.0-m98（e76df86，M98 收口后，runtime.c 16217 行）。S2 验证：examples/m99_s2 verify.sh
> 8P/0F + verify_tls.sh 8P/0F。S3 回归：m95_s2/s4、m97_s2/s3、m82、m83_s6 全 RC=0 + M98_s2 回归 7P/0F。
> S4 收口：diffcheck --all rc=0（重链前后两轮）+ vm_ab 37P/1GAP(环境)/0F + 双自举证明
> （C 轨 rc=0 + BC 轨 dump 与 golden 逐字节一致）+ pxi/pxi_vm 重链（pxi 9,513,256B / pxi_vm
> 9,343,016B，双轨 hello stdout 一致）+ CHANGELOG/ROADMAP 更新。
> 上游：M95（http_serve handler 协程化 + px_evc 事件循环内核）、M95-S4（sse_serve 事件化、
> PxConn 堆化注册蓝本）、M88-B（fserve 事件化：FREE→ACTIVE→IDLE→派发）、M97（px_send_all
> 全量写）、M98（px_serve route/vhost handler 协程化：PxPend 连接堆化注册表）。
> 性质：**L0 runtime**（px_serve 连接并发模型收尾）。

## 〇、一句话

px_serve（M31.4b g_pool + M98 PxPend 注册表）的连接处理：handler 拆段后（M98）长业务已
不占 worker，但 **keep-alive 空闲连接仍每连接一个 g_pool 线程阻塞在下一请求的 recv 直到
连接关闭**（SO_RCVTIMEO 15s）。M99 把「请求间空闲」接入既有全局事件循环（px_evc/px_ev_loop）
：响应写完 + keep-alive + 无下一请求在途 → 连接交 IDLE（事件循环照看，worker 释放取下一
job）→ 事件循环 detect 可读 → 投回 g_pool 续读。**px_serve 并发模型推到与 http_serve 同级
终点：空闲连接 0 占用 worker，线程真正按需。**

## 一、D0 侦察（2026-09-10，dongyue，基线 e76df86 / runtime.c 16217 行）

- **px_serve 连接处理现状**（M98 后）：`bi_px_serve` accept → `px_pool_push(cfd)` →
  常驻 `px_pool_worker`（取 job 循环）→ `px_conn_worker(fd)`（~14535）。每 job：PxPend 槽
  取/建（px_pxpend_enter：新连接 px_conn_init 含 TLS 握手 + inflight++）→ stage==2 续处理
  （route/vhost 段2）→ `for(;;)` keep-alive 循环：**阻塞读请求头（SO_RCVTIMEO 15s）** →
  解析/body → px_http_dispatch（同步完成 或 dret==1 拆段 worker 释放）→ req_done 清理 →
  `if (!client_keep_alive) break` → 下一轮阻塞读。连接结束 px_pxpend_close（close+TLS
  free+inflight--）。**空闲连接 = 阻塞在 recv 的 g_pool 线程**。
- **已有事件循环内核**（http/sse 用，可复用）：`px_ev_loop`（~11262）epoll 照看 IDLE 连接；
  PxConnCtx（FREE/ACTIVE/IDLE，fd 索引表 g_conns）；px_evc_acquire（~11162，FREE→ACTIVE，
  非 FREE 强制清旧）；px_evc_close（~11183）；px_evc_detach（~11200，摘事件上下文不 close fd）；
  px_evc_idle_put（~11218，ACTIVE→IDLE + epoll ADD EPOLLIN|EPOLLRDHUP|EPOLLET + **强制
  px_fd_nonblock**）；px_evc_idle_pop；FSERVE_KIND_HTTP=0 / SSE=1。事件循环 detect：HTTP
  IDLE 可读 → fserve_push 投回；SSE → 断开检测。tick 每 ~1s 扫超时：仅 HTTP IDLE 受 15s
  约束，close 前 poll(0) 二次确认（漏报救回 → fserve_push 重投）。
- **px_serve 不可直接照抄的差异点（关键）**：
  1. **读侧阻塞 vs 非阻塞**：http_serve 连接 fd 一律非阻塞 + px_recv_wait(poll+recv)；px_serve
     连接 fd 保持阻塞 + SO_RCVTIMEO 15s recv（px_conn_read：明文 recv / TLS mbedtls_ssl_read）。
     `px_evc_idle_put` 强制 px_fd_nonblock → 若 px_serve 交 IDLE 用原函数，fd 变非阻塞后
     worker 续读 recv 立即 EAGAIN → 读循环误判断开。→ **px_serve 需不加 nonblock 的交 IDLE
     变体**（epoll 对阻塞 fd 照常报可读；投回时数据在途 → 阻塞 recv 立即返回）。
  2. **TLS 缓冲残留不可见 epoll**：PxConn 有 rbuf/rlen/roff（mbedtls_ssl_read 一次读整 record
     缓冲）；读完当前请求后缓冲可能残留下一请求字节 → 交 IDLE 后 epoll 永不报（数据在 mbedtls
     缓冲）→ 悬挂。→ 在途判定必须含 `conn->roff < conn->rlen`（TLS 缓冲非空即算在途）。
  3. **tick 超时 close 语义**：http tick close = 裸 close(fd)（无 PxConn 对象）；px_serve 连接
     有 PxPend 表项 + TLS 会话 + inflight 计数 → 超时关闭必须走 **px_pxpend_close(fd)**（清
     表 + TLS free + inflight-- + close），不可裸 close。
  4. **优雅关闭**：M98 前 keep-alive 空闲连接阻塞在 worker recv，join 等超时；M99 后空闲连接
     交 IDLE 不占 worker → accept 退出 + join 后仍有 IDLE 连接在事件循环（inflight 不归零）→
     须补「关全部 PXSERVE 登记连接」步骤，优雅关闭才能干净退出。
  5. 写侧：px_serve 响应经 PxHttpOut→px_conn_write（明文循环 send MSG_NOSIGNAL / TLS
     mbedtls_ssl_write 循环），fd 保持阻塞 → 无 EAGAIN/WANT 问题（不切非阻塞则零改动零回归；
     M97 px_send_all 修的是 http_serve 非阻塞 fd，本里程碑不触碰 px_serve 写路径）。
- **在途判定蓝本**：http_send_resp（~10455）：`!px_fd_readable_now(fd)` → px_ev_ensure +
  px_evc_idle_put → worker return。px_serve 同构但探测函数升级（含 TLS 缓冲）。

## 二、范围决策

M99 = **px_serve 连接「请求间空闲」事件化**（明文 + TLS 全支持，因 TLS 缓冲探测可精确判定
在途；fd 保持阻塞 → 读写层零改动零 EAGAIN 回归）。**不做**：accept 后首字节等待事件化
（与 http_serve 现状一致：accept 投池 worker 等首请求/ClientHello——TLS 握手需在 worker 内
阻塞做；可二期）；h2 连接（PX_NO_H2 裁剪面 + M-B9b ALPN 固定 http/1.1）；H3（独立管道）。
优雅关闭补 IDLE 清理（必要条件）。

- **S2（实现）**：
  1. `FSERVE_KIND_PXSERVE 2`（两处宏定义区同步）。
  2. px_serve 版交 IDLE：抽公共 epoll 注册，新增 `px_evc_idle_put_fd(fd,kind,nonblock)`；
     px_evc_idle_put 保持 nonblock=1；px_serve 用 nonblock=0（fd 保持阻塞 + SO_RCVTIMEO 语义）。
  3. `px_pxserve_inflight_data(conn,fd)`：TLS 缓冲非空（roff<rlen）→ 1；否则 px_fd_readable_now。
  4. px_conn_worker：每 job 开头 px_evc_acquire(fd,PXSERVE)（登记 ACTIVE，幂等）；循环尾
     req_done 收尾后：`client_keep_alive` 且 `!inflight_data` → px_ev_ensure + idle_put_fd
     (nonblock=0) 成功 → return（worker 释放）；失败 → 落 for(;;) 顶部原阻塞续读（功能不降，
     非 Linux 亦然）。
  5. 事件循环 px_ev_loop：detect 可读 kind==PXSERVE → px_pool_push(fd)（非 fserve_push）；
     tick 超时扫描放行 PXSERVE（同 15s）：close 走 px_pxpend_close(fd)；漏报救回 → 重新
     acquire + px_pool_push(fd)。
  6. px_pxpend_close 出锁后前置 px_evc_detach(fd)（防事件循环照看已关 fd / fd 复用串扰；
     detach 不 close，px_conn_close 负责）。
  7. bi_px_serve 优雅关闭（join 池 worker 后、等待 inflight 前）：新增
     `px_pxserve_ev_close_all()` 遍历 g_conns kind==PXSERVE → 摘除 + px_pxpend_close。
- **S3（验证）**：examples/m99_s2 —— ① **线程占用实证**：max_conn=8，开 50 条 keep-alive
  空闲连接（发完请求保持连接不发新请求）→ 线程数 ≈ 8+少量（IDLE 不占 worker），对比事件化前
  ≈ 50+；② M98 功能回归不破（并发 slow/fast、keep-alive 续请求 dials=1、/big defer、
  vhost/vh-null）；③ 边界：单连接 pipelining 双请求同批（缓冲在途判定）、连接 15s 空闲超时
  关闭（fd 回收）、TLS（若 mbedtls 可用）keep-alive 空闲交 IDLE + 续请求、优雅关闭（IDLE
  连接随 g_px_stop 归零干净退出）；④ 压力：N 并发短请求 keep-alive 正确性（无丢无串无截断）。
- **S4（收口）**：全量回归 + diffcheck --all + 双自举证明 + pxi/pxi_vm 重链 + CHANGELOG/
  ROADMAP/PLAN 状态 + tag v0.2.0-m99。

## 三、S 拆分
- S1 立项（本文件，commit）
- S2 runtime.c 事件化实现
- S3 examples/m99_s2 verify.sh 验证门
- S4 收口（回归/自举/重链/文档/tag）

## 四、S2/S3 验证结果（2026-09-10）

- **examples/m99_s2 verify.sh（明文）8P/0F**：P1 并发 40×/fast max_conn=4 全成功 wall=0.01s
  （事件化前 4 worker 被 keep-alive 空闲占死 → 36 悬挂——核心铁证）；线程峰值 14≤20（40 空闲
  连接 0 占 worker）；P3 并发 40×/slow（sleep300 让出+IDLE）wall=0.30s；P6 空闲 16.5s 后 8/8
  连接被事件循环 15s tick 超时回收；优雅关闭 0.1s + 日志在途 0（px_pxserve_ev_close_all 生效）。
- **examples/m99_s2 verify_tls.sh（TLS）8P/0F**：顺序建 20 TLS keep-alive 全成功（0.97s，
  max_conn=2）；线程峰值 12≤18（20 TLS 空闲连接 0 占 worker）；P3 TLS 空闲后 20 conns 并发
  续请求 20/20 wall=0.00s（IDLE 唤醒 + mbedtls rbuf 缓冲探测续服务，不悬挂不丢）；优雅关闭干净。
- **回归面（M99 改动触及 px_evc 事件循环 + px_serve + 关闭路径）全绿**：m95_s2（http handler
  协程化）RC=0、m95_s4（sse handler 协程化 + px_evc SSE）RC=0、m97_s2/m97_s3（Issue31/32 连接
  复用）RC=0、m82_http_serve_unix RC=0、m83_s6 RC=0、M98_s2（route/vhost 协程化）7P/0F。
- **⚠️ 发现既有缺陷（先于 M99，非本里程碑引入，qg 二期候选）**：px_serve **并发 TLS 握手**
  缺陷——TLS1.3 下 CertificateVerify「crypto/rsa: verification error」（Go 客户端，InsecureSkipVerify
  不豁免 CertificateVerify 验签）+ TLS1.2 下大并发部分 EOF。用 M98 runtime（git show e76df86:
  runtime/runtime.c）复现同样失败（TLS1.3 20/20、TLS1.2 20/20 更严重）→ 确证与 M99 无关（M99
  不触握手路径：px_conn_init/px_conn_tls_handshake 未改）。M99 TLS 验证据此改为顺序建连（单连
  握手稳定）聚焦「已建 TLS 连接空闲事件化」；并发握手健壮性另立里程碑（疑似 mbedtls 全局
  session cache 并发竞态 / RSA-PSS CertificateVerify 边界，需 mbedtls debug 定位）。
- **注（px_serve 既有限制，非 M99 引入）**：HTTP/1.1 pipelining 同缓冲多请求——px_conn_worker
  读缓冲残余未 pbuf 续接（http_serve 有 PxConnCtx.pbuf 续接，px_serve 未实现）→ 流水线客户端
  残余请求丢失；keep-alive 顺序请求（标准用法）无此问题。二期候选（px_conn_worker pbuf 续接）。
