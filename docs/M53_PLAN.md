# M53 · HTTP/3 接入 px_serve（三栈合一 WebServer）

> 目标：把 M46–M52 的 QUIC/HTTP/3/QPACK 能力从"语言层 demo 驱动"升级为
> **px_serve 内置服务管道的一员**——HTTP/1.1 + HTTP/2(prior knowledge/ALPN) + HTTP/3
> 共用同一套 vhost / 路由 / 限流 / 访问日志 / 静态文件 / .px 脚本管道。
> 验证：真实第三方 HTTP/3 客户端（aioquic 独立实现）与 PuXian 自研 h3 client 双端互操作。

## 一、现状（调研结论）

| 层 | 现状 | 缺口 |
|---|---|---|
| QUIC 传输（runtime_quic.c） | quic_listen/accept 阻塞单连接握手；**listener fd 被每连接共享、各自 poll+recvfrom**，多连接会互相抢包 | 服务端需要**单 fd 收包路由**（按 DCID 分发到对应 conn），多连接并发 |
| QUIC TLS | quic_make_server_ctx() **运行时自签临时证书** | 需支持加载 PEM 证书/私钥（生产应用证书）；SNI 多证书后续 |
| HTTP/3 语义（runtime_h3.c） | h3_conn_setup/poll/read/send 语言 API 齐全（QPACK 动态表、SETTINGS、多流、ack 闭环） | 全部是**阻塞式、单连接循环驱动**；无多连接托管、无监听服务形态 |
| HTTP 管道（runtime.c px_serve） | px_conn_worker：HTTP/1.1 请求 → req dict → CORS/限流/vhost/路由/静态/.px/日志，输出与 **fd/TLS 强耦合**（px_px_send_ex/send/流式文件） | "请求处理"逻辑未与传输层解耦，H3 无法复用 |
| 互操作验证 | m46–m52 全部为 PuXian 自 client ↔ 自 server 回环 | 无第三方独立实现打通过（外部互操作空白） |

## 二、架构决策

- **D1 · 收包路由（runtime_quic.c 新增，不动 demo 路径）**
  新增 h3 专用 listener + 每 listener 单收包线程：poll(fd) → recvfrom → 包头 DCID 路由
  到对应 conn 的入包队列；每连接由其 H3 处理线程消费队列 → ngtcp2 read_pkt →
  write_pkt 即时 sendto。demo 级 quic_accept/quic_pump 原样保留（m46–m52 不回归）。
- **D2 · 证书加载**
  quic_make_server_ctx 扩展：`quic_h3_listen(port, cert, key)`，cert/key 非空走 PEM 加载
  （SSL_CTX_use_certificate_chain_file / use_PrivateKey_file）；为空退回自签（demo/测试）。
- **D3 · 公共请求管道抽取（runtime.c）**
  把 px_conn_worker 中"req dict 就绪 → 响应发出"的逻辑抽为
  `static void px_http_dispatch(PxHttpOut* out, LXValue req, ...)`，其中
  `PxHttpOut` 是输出抽象（send status/body、流式 sendfile、HEAD/keep-alive 语义）：
  - HTTP/1.1 out：包 fd/TLS，行为与现状逐字节一致（gzip/ETag/Range/304/日志全保留）
  - HTTP/3 out：包 conn+sid，HEADERS/DATA 走 QPACK（复用 h3_send_fields）
- **D4 · px_serve opts 增 http3**
  `px_serve(port, docroot, timeout, {http3: {cert,key[,port]}, alt_svc,...})`：
  启动 TCP 服务的同时，若 http3 配置存在 → 同端口（或指定 UDP 端口）起 H3 listener；
  连接握手完成后按连接托管，每连接一个处理线程跑 H3 循环；每条请求流解码为
  与 HTTP/1.1 等价的 req dict（version="HTTP/3"）→ 走 D3 公共管道 → H3 响应。
  同时注入 `Alt-Svc: h3=":port"`（M33.4 已有 alt_svc 通告钩子）。
- **D5 · 互操作验证**
  - 第三方：`pip install aioquic`（Python 独立 QUIC+H3 实现，仅依赖 pyopenssl/cryptography）
    → aioquic 客户端请求 PuXian H3 服务，断言 200 + body 与 HTTPS 一致。
  - 自研：m52 类 client.px 多连接并发连 H3 server（多连接路由验证）。
  - 管道一致性：同 URL 分别走 HTTP/1.1 与 HTTP/3，响应头/体逐字节一致。

## 三、子步划分（每步可独立回归）

| 子步 | 内容 | 验证 |
|---|---|---|
| S1 ✅ | runtime_quic.c：h3 listener（外部证书）+ 单 fd 收包路由队列 + raw API | commit `d328041`；8 进程并发 QUIC echo 全 PASS + close 优雅退出 + m46 回环回归 |
| S2 ✅ | runtime.c 管道抽取 PxHttpOut（纯重构，行为零变化） | 本 commit：m43_webapp(10 PASS)/m33_sni/m33_route_rate_limit/m35_gzip_rl/capability(253 PASS) + 原始响应字节 old==new 逐字节一致（静态/gzip/304/Range/HEAD/.px/404/403/路由/429/CORS/keep-alive） |
| S3 ✅ | runtime_h3.c：H3 PxHttpOut 输出抽象（复用 h3_send_fields）+ 托管连接回调（请求流 → req dict → runtime.c 接入桥 px_http_dispatch_h3 补全 query/cookie/form/version → 公共管道 → HEADERS/DATA 响应）；runtime_quic.c 显式回调 listener（px_quic_raw_h3_listen_cb）与对端地址（px_quic_raw_peer_addr）；H3 连接线程纳入并发 GC（px_gc_thread_enter/leave） | 本 commit：4 条 QUIC 连接并发 ×5 请求（路由 handler / :id+query / 静态 / 404 / 403）= 20 全 PASS 且与 HTTP/1.1 curl 同一管道输出一致（examples/m53_s3_pipe_verify.sh）；m52 QPACK ack 双端 PASS；m53_s1 echo 8 并发 PASS；m43_webapp 10 PASS == golden |
| S4 ✅ | px_serve opts http3 + Alt-Svc 注入 + 生产应用支持 | px_serve(...,{http3:{port,cert,key}}) 单调用三栈合一（TCP+UDP 同端口）；Alt-Svc 自动通告 h3=":port"；生产应用 config http3 支持；**aioquic（第三方 HTTP/3 实现）互操作 200 + 自研 client 双端一致** + SIGTERM 优雅关闭（本 commit，验证 examples/m53_s4_pxserve_h3_verify.sh） |
| S5 ✅ | 全量回归：pxi 重建、capability、diffcheck --all/--errors、自举证明 B.c==golden、m4x 回归 | 本 commit：**bootstrap/pxi 重建**（interp.px 编译 + 当前 runtime 静态链接，9.03MB；解释器新增内置 h3_server_listen 等 M53 能力——examples/m53_s5_pxi_h3_smoke.px 解释模式自检 id>0 PASS）；**capability 解释 + 编译双模式各 253 PASS/0 FAIL 且输出逐字节一致**；**diffcheck --all rc=0 / --errors rc=0**（全量对拍全绿）；**自举证明 B.c==golden/compiler.c 逐字节一致**；**m4x 回归全 PASS**（m46/m47/m48/m49/m50/m51/m52，编译+解释双模式）+ m53_s1/s3/s4 端到端复验全 PASS（含 aioquic 第三方互操作） |
| S6 ✅ | 文档：spec §8.14、ROADMAP、README、CHANGELOG；一次 commit push | 本 commit：spec.md §8.14（HTTP/3 三栈合一规范）+ §8.8 过时陈述更新 + §8.13 代码围栏补齐；ROADMAP M53 移入已完成主线、能力基线/远期 M54+ 同步；README.md / README.en.md 网络行 + 里程碑 M41–M53 + 示例 m53；CHANGELOG [Unreleased] 记 M53（新增）+ M55（issue #2 修复）；**M53 里程碑闭合** |

## 四、风险与规避

- 管道抽取动 HTTP/1.1 生产核心 → S2 纯重构先行，靠 m27/m33/m43/p5 等端到端 + capability
  双模式锁定行为；抽取过程中不合并任何行为改动。
- H3 多线程与 GC：px_call（语言 handler）在线程内执行；全局表/GC 锁已由 M55 保障；
  H3 conn 状态（QPACK 表）**每连接单线程串行**访问，不做跨连接共享。
- H3 托管连接线程（裸 pthread，不经 px_spawn）构造普贤对象 → S3 已纳入并发 GC：
  quic_srv_conn_thr 开头 px_gc_thread_enter / 清理后 px_gc_thread_leave（g_threads 槽位
  上限 64 与 spawn/连接池共享；槽满不暂停的风险与连接池 worker 一致，S4 生产评估）。
- 大响应（>700KB body）：H3 out 走 HEADERS + 多 DATA 分帧（无状态 QPACK 编码，
  标准多帧）；自研 MVP client 只取首 DATA —— S4 aioquic 走标准路径验证完整接收。
- curl 7.76 无真 HTTP/3（无 ngtcp2/quiche）→ 外部互操作用 aioquic（pip，已确认网络可用）。
- 静态文件 H3 大文件：H3 响应体经 QUIC 流分帧发送，不整读 1MB 上限（S3 处理分帧）。
