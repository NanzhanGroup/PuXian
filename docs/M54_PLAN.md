# M54 · HTTP/3 生产化（0-RTT / 连接迁移 / BLOCKED_STREAMS）

> 目标：把 M53 已接入 px_serve 的 HTTP/3 栈从"功能可用"推向"生产级传输语义"：
> **TLS 1.3 会话恢复（1-RTT resumption）+ 0-RTT early data、连接迁移（path
> validation）、流上限协商（STREAMS_BLOCKED / MAX_STREAMS）** 三项 RFC 9000/9114
> 生产化特性，全部以 PuXian 语言 API 落地 + 自研双端/第三方互操作验证。
>
> 范围：仅 PuXian 语言/runtime 迭代（ws-web 归清歌，不在本里程碑）。
> 验证基调：与 M46–M53 一致——PuXian 自研 client ↔ 自研 server 回环 + 可外部
> 互操作项用 aioquic（第三方独立实现）对拍。

## 一、现状（调研结论）

| 层 | 现状 | 缺口（M54 待补） |
|---|---|---|
| QUIC/TLS 版本 | vendored ngtcp2 **1.25.90** + OpenSSL **3.0.9+quic**(quictls)，均含完整生产 API | 未启用任何会话恢复 / early data / 迁移 / 流控协商 |
| TLS server ctx | `quic_make_server_ctx_cert()`：自签或 PEM 证书，无 session ticket 配置 | TLS1.3 session ticket（stateless）未显式开启；`max_early_data` 未设 |
| TLS client ctx | `quic_make_client_ctx()`：每次 `SSL_new` 全新握手，**不保存 session** | 无 `SSL_SESSION` 导出/导入/恢复；无 0-RTT 触发 |
| QUIC 连接 | `quic_conn` 单 fd + 固定 `local_sa/remote_sa`；ngtcp2 path 未随包更新 | 连接迁移需支持**换源地址续传**；服务端需 path validation |
| 流上限 | ngtcp2 默认 max_streams（较大），无语言层控制 | 无设置/放行流上限 API；STREAMS_BLOCKED 无显式互操作用例 |
| 0-RTT 语义 | M51+ QPACK 动态表已闭环；0-RTT 未做 | 0-RTT 请求只能用静态表/字面量（RFC 9204 §3.3.3），需显式约束与验证 |
| 互操作 | M53-S4 已打通 aioquic 常规 200 | 0-RTT / 迁移 / 流控上限尚无第三方对拍 |

参考实现（ngtcp2 官方，/tmp/ngtcp2/examples）：
`tls_client_session_quictls.cc` / `tls_server_session_quictls.cc` / `client.cc`
（--session-file、0-RTT 触发与 early_data_rejected）/ `server.cc`。

## 二、架构决策

- **D1 · 会话恢复（client 侧，最小侵入）**
  沿用现有 `quic_connect` 心智，新增：
  - `quic_session_save(conn) -> str`：握手完成后导出 TLS session（`SSL_get1_session`
    → `i2d_SSL_SESSION` → hex/base64）。
  - `quic_connect_resume(ip, port, alpn, session) -> int`：`SSL_set_session` 后走
    正常握手；新增 `quic_conn_resumed(conn) -> bool`（`SSL_session_reused`）。
  服务端 ctx 显式开 TLS1.3 stateless ticket（`SSL_CTX_set_session_cache_mode`
  SERVER + `SSL_CTX_set_session_ticket_cb` 默认键 或 quictls 默认），保证书不变
  → ticket 可复用。
  验证：`.px` client 首次握手后 save → 二次 `connect_resume` → resumed=true；
  对比首/二次握手 RTT（完成回调时间戳差）。

- **D2 · 0-RTT early data（传输层为主，H3 受限子集）**
  - server ctx：`SSL_CTX_set_max_early_data(ctx, 1<<20)`；accept 侧在
    `read_pkt` 路径由 ngtcp2_crypto_quictls 自动解密 0-RTT 包 → 0-RTT 应用数据
    进 `recv_stream_data_cb`（现槽位机制天然可收，握手未完成也已缓冲）。
  - client：`quic_connect_0rtt(ip, port, alpn, session) -> int`：
    `ngtcp2_conn_decode_and_set_0rtt_transport_params`（session 内保存 0rtt tp）
    + ngtcp2_crypto 装 early key → **握手完成前即可 `quic_send`**；服务端拒绝
    early data（`ngtcp2_conn_tls_early_data_rejected` 回调）时语言层标记
    `quic_0rtt_rejected(conn)`，已发数据按对端重发语义由上层处理。
  - H3 0-RTT 子集：请求头编码强制静态表/字面量（0-RTT 期间不引用动态表，
    RFC 9204 §3.3.3）；0-RTT 只发幂等 GET（RFC 9114 §3.5）。
  验证：client 连接后立即 `quic_send`（不等握手）→ server 握手完成回调里已见
  该流数据（证明 0-RTT 提前到达）；H3 子集：0-RTT GET → 200。

- **D3 · 连接迁移（client 主动换源地址）**
  - `quic_conn` 增 `migrate_fd/local_sa` 支持：`quic_migrate(conn, local_ip, local_port)`
    → 新建 UDP fd（新源端口）→ 后续收发包走新 fd；ngtcp2 path 由每次
    `read_pkt(path=新local+remote)` 推进（M53 托管 server 的 qfroms 已逐包带
    from，天然支持"对端换源端口后仍按 DCID 路由到同 conn"）。
  - server 侧：对端换源后 ngtcp2 自动发 PATH_CHALLENGE 验证（pump 内 write_pkt
    带出）；验证成功 `path_validation_result` 回调记录 → 后续 sendto 走新地址。
  - 语言层：`quic_conn_path(conn) -> str`（当前对端地址，验证迁移生效）。
  验证：client 发 msg1 → migrate 换源端口 → 发 msg2 → server 回 msg2' 到**新地址**
  且同一 conn（无重新握手）；server `quic_conn_path` 变更为新源。

- **D4 · 流上限协商（BLOCKED_STREAMS）**
  - server listener/conn 增：`quic_set_max_client_streams(conn, n_bidi)`（初始上限，
    收包建 conn 时应用到 ngtcp2：本地可开上限 = 对端允许值，用
    `ngtcp2_conn_set_max_local_streams_bidi` 在 server 侧即"允许 client 开几条"）。
  - 放行：`quic_extend_max_streams(conn, add_bidi)` →
    `ngtcp2_conn_extend_max_local_streams_bidi` → 发 MAX_STREAMS。
  - client 被阻塞：`quic_open_bidi_stream` 达上限时返回 `NGTCP2_ERR_STREAM_ID_BLOCKED`
    （语言层返回 -206 语义/等待）；`quic_streams_blocked(conn) -> bool` 供断言。
  - 互操作：自研双端为主（aioquic 流上限控制面弱，保留常规 200 回归）。
  验证：server 上限 2 → client 开 0/4 成功、开 8 阻塞 → server extend +4 →
  client 开 8 成功；帧级可选抓 STREAMS_BLOCKED/MAX_STREAMS 字节。

- **D5 · 回归收口**：pxi 重建（新内置进解释器）+ capability 双模式 +
  diffcheck + 自举证明 B.c==golden + m4x（m46–m52）/m53_s1–s5 全量复验。

## 三、子步划分（每步可独立回归，逐步 commit）

| 子步 | 内容 | 验证 |
|---|---|---|
| S1 ✅ | D1 TLS1.3 会话恢复（resumption） | commit `dfba8f2`；client 二次连接 resumed=true + echo 正常；m53_s1 回归 8/8 |
| S2 ✅ | D2 0-RTT early data（传输层 + H3 静态表子集） | commit `7d004f3`；传输层 echo + H3 0-RTT GET 200 双 PASS |
| S3 ✅ | D3 连接迁移（client 换源 + server path 跟随） | commit `4a463a1`；msg 跨迁移续传 PASS（同 conn 无重握手） |
| S4 ✅ | D4 BLOCKED_STREAMS 流控协商 | commit `0bfd5a5`；上限 2 阻塞(-206)、extend +4 放行 PASS |
| S5 ✅ | D5 pxi 重建 + capability/diffcheck/自举/全量回归 | 本 commit：**bootstrap/pxi 重建**（链接 M54 runtime，9.04MB；examples/m54_s5_pxi_quic_smoke.px 解释模式自检 PASS，证明 pxi 解释器同能力支持 M54 新内置）；**capability 双模式**（解释 253 PASS + 编译 253 PASS，输出逐字节一致）；**diffcheck --all rc=0 / --errors rc=0**；**自举证明 B.c==golden（6381 行 C 逐字节一致）**；**m4x + m53 + m54 全量回归全 PASS**（m46/m47/m48/m49/m50/m51/m52/m53_s1/m53_s3/m53_s4/m54_s1/m54_s2/m54_s3/m54_s4 共 14 项；其中 m51/m53_s4/m54_s4 初跑因验证脚本自身问题——verify.sh 无执行位 / 残留进程占端口 / 断言文本不匹配——修复后重跑 PASS） |
| S6 | 文档收尾（spec §8.15、ROADMAP、CHANGELOG、README） | 里程碑闭合 |

## 四、风险与边界

- 0-RTT 跨层细节多（0rtt tp 序列化、early key、拒绝语义），S2 先行传输层
  echo 验证，H3 0-RTT 限定幂等 GET + 静态表；QPACK 动态表 0-RTT 前缀属
  RFC 9204 深度语义，若互操作不稳则记入边界（M55+ 方向），不以自研回环
  掩盖标准缺口。
- 迁移依赖服务端"按 DCID 路由 + 逐包 from"（M53 已具备）；仅测 client 主动
  迁移（NAT 换源场景），服务端主动迁移不在范围。
- 流上限语言层 API 为新内置，pxi 解释器需 S5 重建后支持（S1–S4 验证跑编译
  模式，同 M53-S3/S4 策略）。
- 全部新 API 保持"旧行为零变化"：未用新 API 的 m46–m53 脚本不受影响。
