# TLS 证书「热加载（重注册）」的生命周期契约

> M245（第 122 轮）· 缺陷 414 · 来自晨曦报障（Mahesvara WS 统一入口 P0 崩溃）

## 1 一句话

**`tls_server(cert, key[, hostname])` 可以在服务运行期间被再次调用（= 热加载 / 重注册）。
它现在与「任何一步 TLS 握手」互斥** —— 修前不是，于是「释放旧证书/私钥 → 重新 parse」
这个窗口可以被并发握手读到，造成 UAF（TLS1.3 服务端签名崩溃）。

## 2 为什么这曾经是缺陷

### 2.1 对象是共享的，而且**地址稳定、内容不稳定**

```c
static mbedtls_x509_crt g_srv_cert;          // 默认证书
static mbedtls_pk_context g_srv_key;         // 默认私钥
typedef struct { char hostname[256];
                 mbedtls_x509_crt cert; mbedtls_pk_context key;
                 int active; } PxSniCert;
static PxSniCert g_sni_certs[PX_MAX_SNI_CERTS];   // SNI 多证书表（固定大小，**不搬迁**）
```

`px_sni_cb` / 配置阶段把 `&g_sni_certs[slot].cert`、`&g_srv_key` 这样的**指针**交给 SSL 上下文，
握手**后续若干步**还会用它（TLS1.3 在 `ssl_tls13_write_certificate_verify_body` 用私钥做 ECDSA 签名）。

重注册的动作序列是：

```
mbedtls_x509_crt_free(&cert);     ← 结构体里的内部指针（raw.p / pk.pk_ctx）从此悬垂
mbedtls_pk_free(&key);
memset(&slot, 0, sizeof(PxSniCert));
mbedtls_x509_crt_init(...); mbedtls_pk_init(...);
mbedtls_x509_crt_parse_file(...);  ← 到这里才重新有效
mbedtls_pk_parse_keyfile(...);
```

⚠️ **关键区别（与 M242 的表指针外泄不同）**：这里数组**不会搬家**，所以元素的**地址**始终有效；
失效的是它**内部的指针**。⇒ 这类「跨锁外泄」有两个形态：

| 形态 | 机制 | 先例 |
|---|---|---|
| **地址失效** | `xrealloc` 让整块表搬家（旧块被 munmap）| M242（缺陷 403–408）· M240（缺陷 399）|
| **内容失效** | 对象地址不变，但内部指针被 `free` 后重填 | **M245（缺陷 414）** |

### 2.2 两把锁互不排斥

| 锁 | 保护什么 | 谁持 |
|---|---|---|
| `g_srv_hs_mu` | **全局握手串行锁**：任一时刻只有一个线程进入 mbedtls | 配置阶段 + **握手的每一步** |
| `g_srv_tls_mu` | 证书表 / 私钥的**写入** | `px_sni_cb`（读）+ `bi_tls_server`（写）|

修前 `bi_tls_server` **只取 `g_srv_tls_mu`**，而握手的每一步只取 `g_srv_hs_mu`
⇒ **两把锁互不排斥** ⇒ 上面那个「free → parse」窗口必然可以与握手步重叠。

### 2.3 为什么 EC 私钥比 RSA 危险

M101 做了两件事：

* ① **RSA 私钥 per-连接 clone**（`px_pk_clone_rsa` → `c->own_pk` / `c->own_pk_sni`）
  ⇒ RSA 连接用的是**独立副本**，注册释放全局对象不影响它；
* ② EC 私钥走「**共享只读**」—— 连接的 SSL 上下文直接指向 `g_sni_certs[slot].key`。

而 ② 成立的**前提**（M245 才写清）是两件事同时成立：
1. 每次进入 mbedtls 都持 `g_srv_hs_mu` ⇒ 无并发签名；
2. **注册也持同一把锁** ⇒ 共享对象不会被 free。

修前只有 ①。⇒ **EC 密钥是真正的受害面**；也正因如此，
本仓门 `examples/m245_tls_cert_reload` **必须用 EC(P-256)** —— 用 RSA 会被 clone 掩盖。

## 3 现在的契约

```
① `tls_server(...)` 取锁顺序：g_srv_hs_mu → g_srv_tls_mu
   （与握手路径同序；全仓 tls_mu 的三个取锁点**都在 hs_mu 之下**，无反向序 ⇒ 无死锁）
② 注册期间（含证书文件的磁盘 I/O 与 parse）**握手排队**。这是**有意**的：
   对象地址稳定 ⇒ 只需排除「free 与 parse 之间」被读，不必把慢路径搬出锁；
   而正确调用方只在证书**内容真变**时注册（续期 / 装新证），启动期一次性登记，不是热路径。
③ 4 个出口（1 正常 + 3 报错）逐个解锁；`px_error` 在 spawn 隔离点 / json 捕获点会 `longjmp`
   ⇒ **绝不可**持锁调用（判据由门按**形状**核验）。
④ 语义：在途握手会**排队**，然后拿到**新**证书。若证书内容真的变了，那次握手可能需重来
   —— 但**不会崩**。
```

## 4 测试钩子 `PX_TLS_RELOAD_GAP_MS`

在「free 旧对象 → 重新 parse」之间插入 `nanosleep`（毫秒，上限 5s；**默认不设环境变量 = 完全无感**）。

为什么必须有：这是一个**竞态**。「修好了」与「这次没复现」用一次运行无法区分
⇒ 把窗口放大成**必然**，负控才判得出来。与 `PX_GC_STRESS` / `PX_GC_INLINE`（M170/M207）同族。

## 5 判据取「客户端成功率」而不是「崩不崩」

竞态的直接后果是**握手读到被 free / 清零的 cert/key ⇒ 握手失败**（坏证书 / 空私钥），
而「崩溃」只是其中一种可能（取决于 mbedtls 内部有没有把结构体清零）。
⇒ 门取**可观测的行为**：风暴期间客户端成功率必须 100% 且服务端存活。

## 6 覆盖边界（如实）

* 只动态覆盖 **EC(P-256)** 路径；**RSA 是否有窗口未判定**（被 per-连接 clone 掩盖）。
* 只覆盖**文件路径**来源（`parse_file`）；PEM 内容直传走同一段窗口代码，未单独跑动态档。
* 不是 ASan/valgrind：某次竞态若恰好没造成握手失败，门看不见 —— 所以必须靠钩子放大。
* **不覆盖**「真续期」的业务语义（那里内容会变）。
* 仓内写 `g_sni_certs` / `g_srv_cert` 的地方**只有 `bi_tls_server` 一处**（已 grep 核验）。

## 7 未做（候选）

**「注册不阻塞握手」**需要**换代 + 引用计数**（旧代延迟释放：注册时把新对象放进新代，
旧代在所有引用它的握手结束后再 free），而不是把锁拆掉。当前实现选了「持锁做完全部工作」
这条更简单、更容易证明正确的路。
