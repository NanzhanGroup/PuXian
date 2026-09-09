# M101_PLAN · px_serve 并发 TLS 握手缺陷修复（qg 二期候选 A）

> 状态：✅ **M101 完成（S2 修复实现 + S3 收口，tag v0.2.0-m101）**。
> 基线 v0.2.0-m100（1df2cd6）。S2 修复 + 验证：examples/m101_s2 verify.sh 三轮并发
> 握手全 0 失败（RSA-TLS1.3/1.2、EC-TLS1.3 各 144/144；修复前 RSA-TLS1.3 ok=2 fail=142
> 'invalid signature...crypto/rsa'、TLS1.2 144 全 EOF）+ 独立 864 连发 0 失败 0 崩溃 +
> SNI 并发 30/30 + m99_s2 verify_tls 8P/0F。S3 收口：回归 suites（m82/m83_s6/m89_s3d/
> m93_s2/s3/m94_s2/s3/m95_s2/s4/m96_s2/s3/m97_s2/s3/m98_s2/m99_s2/m100）全绿 + vm_ab
> 38P/0GAP/0F + diffcheck --all rc=0 + 双自举证明（C 轨 + BC 轨）+ pxi/pxi_vm 重链
> 吸收 M101 runtime + CHANGELOG/ROADMAP 文档 + tag v0.2.0-m101。
> 缺陷来源：M99-S3 记档（先于 M99、M98 runtime 复现，qg 二期候选）——
> **px_serve 并发 TLS 握手：TLS1.3 CertificateVerify 签名错 + TLS1.2 大并发 EOF**。
> 性质：**L0 runtime bugfix**（服务端 TLS 多 worker 并发握手正确性）。
> 上游：M31.4b 连接线程池（g_pool 多 worker 并发处理连接）→ M10/M27/M30/M33
> 服务端 TLS（mbedtls 握手 + 全局 session cache + SNI 多证书）→ M99 事件化
> IDLE（并发握手线程环境放大，缺陷暴露）。
> 二期候选排序第 A 项：已复现真实缺陷、影响真实并发 TLS 场景 → 先清账。

## 〇、一句话

px_serve TLS 服务端用 **g_pool 多 worker 线程并发执行 mbedtls 握手**，而 mbedtls
预编译库（3.6.2）**未编入线程支持（MBEDTLS_THREADING_C 关）**，握手中多线程
**共享可变对象**（全局 RSA/EC 私钥 ctx g_srv_key / SNI key / 全局 session cache
g_srv_tls_cache）被并发读写 → data race → 偶发签名错（TLS1.3 CertificateVerify）
与握手中断（TLS1.2 EOF）。修复 = **消除握手中的跨线程共享可变写**（per-连接私钥
副本 + session cache 加锁包装），使并发握手正确并行（不做全局握手串行化新瓶颈）。

## 一、D0 侦察（2026-09-11，dongyue，基线 1df2cd6 / runtime.c 16481 行）

### 1. 线程模型与握手路径

- px_serve accept 循环（~15793）accept → px_pool_push(fd) → **g_pool 预派生 worker
  （max_conn 个，px_pool_worker @15876）并发**取 fd → px_conn_worker 入口（14408，
  M98 拆段）：新连接（!active）→ 堆 PxConn + `px_conn_init`（@13165）→
  `px_conn_tls_handshake`（@13125，mbedtls 服务端握手）。→ **N 个 worker 可同时
  处在握手期**（并发新连接数 ≤ max_conn）。
- 每连接独立 malloc：ssl / conf / ctr_drbg / entropy（px_conn_init）→ 读缓冲 rbuf
  亦 per-conn。**每个连接 mbedtls_ctr_drbg_seed 自己的 drbg**（pers="px_server"）。

### 2. 跨线程共享可变对象清单（竞态面）

| 共享对象 | 类型 | 握手期访问 | 风险 |
|---|---|---|---|
| `g_srv_key` | mbedtls_pk_context（RSA/EC 私钥，全局仅 1 份） | conf_own_cert + 握手中服务端签名（TLS1.2 ServerKeyExchange / TLS1.3 CertificateVerify） | **多 worker 并发私钥签名 → RSA CRT 推导/窗口缓存写竞争 → 签名错/段错误** |
| `g_srv_cert` | mbedtls_x509_crt | 只读 | 低（纯读） |
| `g_srv_tls_cache` | mbedtls_ssl_cache_context（全局 session cache，128 项链表） | 握手恢复/存储会话（get/set） | **无锁链表并发 get/set → 链表损坏/会话数据错 → EOF/握手失败** |
| `g_sni_certs[i].cert/key` | 每 SNI 一份 | px_sni_cb set_hs_own_cert → 签名 | 命中 SNI 时同 g_srv_key 风险 |

### 3. mbedtls 3.6.2 线程支持证据（runtime/mbedtls）

- include/mbedtls/mbedtls_config.h：`//#define MBEDTLS_THREADING_C`（3630 注释）、
  `//#define MBEDTLS_THREADING_PTHREAD`（2111 注释）、`//#define MBEDTLS_THREADING_ALT`
  （2100 注释）→ **库内无任何互斥**；官方文档明确 ssl_cache / pk 多线程共享须调用方
  提供外部同步或开 threading。
- `//#define MBEDTLS_USE_PSA_CRYPTO`（2141 注释）→ **legacy PK 后端启用**：
  mbedtls_pk_rsa / mbedtls_pk_ec 宏可用，pk_ctx 为 mbedtls_rsa_context* /
  mbedtls_ecp_keypair*。
- 可复制 API 符号（nm lib/libmbedcrypto.a）：`mbedtls_rsa_copy`(0x38c0)、
  `mbedtls_ecp_copy`(0x16d0)、`mbedtls_ecp_group_load`(0x1680)、`mbedtls_pk_setup`(0x650)、
  `mbedtls_pk_get_type`(0x1210)。`mbedtls_pk_copy` **不存在**（3.6 PSA 化移除）→
  per-连接 key clone 需按类型手工组装（RSA：pk_setup(PK_RSA)+rsa_copy；EC：
  pk_setup(PK_ECKEY)+ecp_group_load+mpi_copy d+ecp_copy Q）。
- TLS1.3 启用（MBEDTLS_SSL_PROTO_TLS1_3 1812）、RSA-CRT 默认（NO_CRT 注释）、
  ECDSA/ECP/SECP256R1 启用。

### 4. 根因方向（待复现实验二分定夺）

候选① 私钥签名竞争（RSA CRT 写 ctx 缓存/推导；EC 亦共享 keypair）；候选② session
cache 无锁链表竞争。**二分实验**：RSA 证书 vs ECDSA 证书并发握手——若 RSA 失败而
ECDSA 全绿 → ①为主；若两者皆失败 → ②为主（或兼有）。据此定修复组合
（per-conn key clone 是否必须 / cache 锁是否必须）。

## 二、复现实验（examples/m101_s2/，S1）

- m101_s2_daemon.px：tls_server(cert,key) + route /fast + px_serve(max_conn=16)
  （并发握手数可达 16；M99 顺序建连规避了此缺陷，M101 用**高并发新建连接**暴露）。
- go_tls_client.go：barrier 同步 N=48 goroutine 并发 tls.Dial（每连接全新握手 +
  GET /fast + Connection: close 即关）+ 多轮；可选强制 TLS1.2 / TLS1.3。
- verify.sh：RSA-TLS1.3 / RSA-TLS1.2 / EC-TLS1.3 三轮，统计握手失败率。
- 预期（修复前 v0.2.0-m100 runtime）：RSA 并发握手 FAIL>0（复现 M99 记档）；
  EC 结果二分定位。修复后全 0。

## 三、修复方案（S2，复现定夺后落定）

原则：**不做全局握手串行化**（新瓶颈）；**消除握手中跨线程共享可变写** →
并发握手真正并行且正确。
1. PxConn 增加 per-连接私钥副本字段（owned=1 时 px_conn_close 释放）：
   - 默认证书：px_conn_tls_handshake 在 g_srv_tls_mu 锁内 clone g_srv_key →
     conf_own_cert(conf, &g_srv_cert, conn_key)（cert 只读共享可保留）。
   - SNI（px_sni_cb 命中 slot）：锁内 clone g_sni_certs[slot].key → 该连接私有
     set_hs_own_cert。
   - clone 按 pk 类型分支：RSA → mbedtls_pk_setup(PK_RSA) + mbedtls_rsa_copy；
     EC → mbedtls_pk_setup(PK_ECKEY) + ecp_group_load(grp.id) + mpi_copy(d) +
     ecp_copy(Q)。clone 失败 → 退保守：该连接走握手全局锁（安全降级）。
2. session cache 加锁包装：自定义 my_cache_get/my_cache_set（内部 g_cache_mu 锁）
   经 mbedtls_ssl_conf_session_cache 注册（不再直传库函数），根除 cache 链表竞争。
3. 若二分显示 EC 亦安全（仅 RSA CRT 写共享），仍统一 clone 两种类型以覆盖 SNI+
   未来证书轮换；实现不分支过度。

## 四、验证面（S3 收口）

- examples/m101_s2 verify.sh：RSA-TLS1.3 / RSA-TLS1.2 / EC-TLS1.3 并发握手全 0
  FAIL（修复前 >0 对照）+ 多轮稳定 + TLS session 恢复（短连接复连同 host 命中
  cache）不悬挂。
- 既有 TLS 门回归：m99_s2 verify_tls.sh 8P/0F（顺序建连 + 空闲续请求不受损）；
  sse/ws TLS（若 examples 有）抽 1；px_serve 明文管道（m28/m29/m31/m33/m57_s7）
  不受影响（明文路径零改动）。
- 里程碑 suites：m95/m96/m97/m98/m99/m100 相关门回归 RC=0；diffcheck --all rc=0；
  vm_ab 无新增差异；双自举证明（C/BC 轨）；pxi/pxi_vm 重链；文档 CHANGELOG/
  ROADMAP；tag v0.2.0-m101。

## 五、S 拆分（状态全完成）

- S1（✅）：立项 + D0 侦察（本文件）+ examples/m101_s2 复现套件 + 跑复现二分
  （RSA-TLS1.3 142/144 签名错 = 私钥竞争主因 / RSA-TLS1.2 144 EOF / EC-TLS1.3 140/144
  +4 EOF = cache 竞争次因）定根因。commit 1。
- S2（✅）：修复实现（per-conn RSA clone + session cache 加锁包装 → TLS1.2/EC 全绿、
  RSA-TLS1.3 残余 ~1% -110；加全局握手串行锁根治）+ m101_s2 三轮 144×3 全 0 失败 +
  864 连发 0 崩溃 + SNI 并发 30/30。commit 2。
- S3（✅）：收口（回归 suites 全绿 + vm_ab 38P + diffcheck --all rc=0 + 双自举证明 +
  pxi/pxi_vm 重链 + CHANGELOG/ROADMAP 文档 + tag v0.2.0-m101）。commit 3。
