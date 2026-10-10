// 普贤 (PuXian) C 运行时 — QUIC 传输级绑定（M46 HTTP/3 应用验证）
// ------------------------------------------------------------
// 语言层 API（双模式一致，对齐 udp_* 心智）：
//   quic_listen(port) -> int            服务端监听（UDP bind + 自签证书就绪）→ listener id | -1
//   quic_accept(listener, timeout_ms) -> int  阻塞等 QUIC 连接（握手完成）→ conn id | -1 超时/失败
//   quic_connect(ip, port, alpn) -> int 客户端连接（握手完成）→ conn id | -1 失败
//   quic_connect_resume(ip, port, alpn, session) -> int  M54-S1：恢复 TLS 会话（1-RTT）
//   quic_session_save(conn) -> str    M54-S1：导出 TLS session（hex，含 NewSessionTicket）
//   quic_conn_resumed(conn) -> bool   M54-S1：本连接是否会话恢复（resumption）握手
//   quic_send(conn, data) -> int        发送（当前双向流）→ 本次写入流字节数 | -1
//   quic_recv(conn, maxlen) -> str      阻塞接收应用数据（截断到 maxlen）→ str（"" = 超时/对端关闭）
//   quic_close(conn) -> bool
//   quic_close_listener(listener) -> bool
// 机制：注册进 FFI 表（px_ffi_register），双模式统一走 bi_ffi_call（M42）。
// 实现：ngtcp2 1.26（libngtcp2 + libngtcp2_crypto_ossl）+ OpenSSL 3.5 QUIC API，纯阻塞事件循环。
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <ngtcp2/ngtcp2.h>
#include <ngtcp2/ngtcp2_crypto.h>
#include <ngtcp2/ngtcp2_crypto_quictls.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/provider.h>
#include <openssl/objects.h>
#include "locktrack.h"   // M127（qg-issue 84）：回卷锁审计 —— 必须放在**最后一个 include**

// M295（缺陷 508）：拆两个上限 —— 原来 listener 与 connection **共用** QUIC_MAX=64，
//   而真实面是「连接槽位耗尽」：第 65 条连接的 Initial 被**静默丢弃**（客户端只见
//   connect-fail，服务端日志一行都没有），且槽位要等连接线程空闲退出才回收（实测
//   默认约 10s）⇒ 64 并发在真实负载下太小。
//   listener 槽保持 64（每槽 ~12KB，行为零变更）；连接槽提到 256（每槽 ~1.4KB ⇒ ~360KB）。
#define QUIC_MAX_LISTENERS 64
#define QUIC_CONN_MAX      PX_QUIC_CONN_MAX   // M295：单一真相见 runtime.h
// M295（缺陷 508）·第二面：**cid 路由表**容量。
//   修前是硬编码 256，而每连接至少 2 条 cid（本端 SCID + 客户端 Initial DCID）+ ngtcp2 的
//   CID 轮换 ⇒ 实测**64 条连接就把 256 条 cid 用满**，满了以后 `quic_cid_add` **静默丢弃**
//   ⇒ 新连接的短头包路由不到 ⇒ 客户端只看到 TIMEOUT（conn 槽位却只用掉 64/256）。
//   容量按连接上限成比例（×8 留轮换余量）。
#define QUIC_CIDTAB_MAX    (QUIC_CONN_MAX * 8)
#define QUIC_PKT_BUF 65536
#define QUIC_SCIDLEN 8

// ---------- 数据结构 ----------
// M53：cid 路由表（服务端收包路由：按包 DCID 定位连接）
typedef struct {
    uint8_t cid[NGTCP2_MAX_CIDLEN];
    size_t  cidlen;
    int64_t conn;                  // g_qconns 槽（1 起）
} quic_cid_entry;

// M53：每连接处理回调（握手完成后由连接处理线程调用；ud 透传）
typedef void (*quic_conn_cb)(int64_t conn, void* ud);

typedef struct {
    int            fd;            // UDP socket
    struct sockaddr_in local;
    SSL_CTX*       ssl_ctx;       // 服务器 TLS 上下文（含自签证书）
    // M53：h3 server 托管
    quic_cid_entry cidtab[QUIC_CIDTAB_MAX];   // 本端签发 cid → conn（收包路由用）
    int            cidtab_n;
    pthread_t      router_thr;    // 收包路由线程
    int            router_started;
    int            router_stop;
    quic_conn_cb   conn_cb;       // 每连接处理回调（NULL → 默认 echo）
    void*          conn_ud;
    // M54-S4：流上限协商 —— 本 listener 每连接允许 client 开的 bidi 流数（0=默认 16）
    int            max_client_bidi;
    int            used;
} quic_listener;

// M50：多路复用——每连接多条双向流，各流独立 FIFO 接收缓冲
// M51：含单向流槽（HTTP/3 控制/编码器/解码器流）→ 上限提到 24
#define QUIC_STREAM_MAX 24   // 每连接流槽（bidi 16 配额 + uni 若干）

typedef struct {
    int64_t  sid;            // QUIC 流 id（client bidi 0,4,8…；server bidi 1,5,9…）
    uint8_t* buf;            // 该流接收缓冲（动态增长）
    size_t   len, cap;
    int      fin;            // 对端 FIN 已到
    int      peer;           // 对端发起（本地被动建槽）
    int      used;
} quic_stream;

typedef struct {
    int            fd;            // UDP socket
    ngtcp2_conn*   conn;
    ngtcp2_crypto_conn_ref conn_ref; // SSL app data（ossl_crypto_send 取 conn）
    SSL_CTX*       ssl_ctx;       // 客户端上下文（NULL 服务端）
    SSL*           ssl;
    ngtcp2_path    path;
    struct sockaddr_storage local_sa, remote_sa;
    quic_stream    streams[QUIC_STREAM_MAX]; // M50：per-stream 接收缓冲
    int            handshake_done;
    int            peer_closed;
    int            used;
    // M54-S2：0-RTT early data
    int            early_sent;      // quic_connect_0rtt 成功路径（握手未完成即返回）
    int            early_rejected;  // 服务端拒绝 early data（握手完成回调检测）
    // M54-S3：连接迁移
    int            hs_confirmed;    // 握手已确认（客户端收到 HANDSHAKE_DONE）——迁移前提
    // M53：server 多连接托管（收包路由线程 → 本连接处理线程）
    // 并发：队列 push/pop/free 统一持全局 g_quic_srv_mu；qcv 每连接独立（避免惊群）
    int              owner_listener;         // >0 = 由该 listener 托管
    pthread_cond_t   qcv;
    uint8_t**        qbufs;                  // 环形队列（每包 malloc）
    int*             qlens;
    struct sockaddr_storage* qfroms;
    size_t           qcap, qhead, qtail, qn;
    int              qclosed;
    pthread_t        thr;                    // 连接处理线程
    int              thr_started;
    int64_t          epoch;                  // M225：连接代次（槽位复用后必然不同）
} quic_conn;

static ngtcp2_conn* quic_get_conn_from_ref(ngtcp2_crypto_conn_ref* ref) {
    quic_conn* qc = (quic_conn*)ref->user_data;
    return qc ? qc->conn : NULL;
}

static quic_listener g_qlis[QUIC_MAX_LISTENERS];
static quic_conn     g_qconns[QUIC_CONN_MAX];
static int           g_quic_init = 0;
// M225：连接代次（每次槽位分配 +1；同一 conn 号在不同代次代表不同连接）
static int64_t       g_quic_epoch = 0;
// M295（缺陷 508）：连接池可见性 —— 修改前「槽位耗尽」在服务端**完全静默**
//   （第 65 条连接被丢弃、日志无痕、客户端只见 connect-fail）。
//   现在每次失败都记账；**首次**（以及每满 1000 次）打一行 stderr 报告；
//   语言侧可用 `quic_pool_stats()` 随时查询（运维自检/监控）。
static int64_t       g_quic_alloc_ok   = 0;   // 累计成功分配
static int64_t       g_quic_alloc_fail = 0;   // 累计失败（= 满）
static int64_t       g_quic_live_peak  = 0;   // 并发占用峰值
static int64_t       g_quic_full_first = 0;   // 首次失败的时刻（monotonic ms，0=从未）
static int64_t       g_quic_peer_closed = 0;  // 对端正常关闭（收到 CONNECTION_CLOSE）计数
static int64_t       g_quic_cid_drop = 0;     // cid 路由表满导致的丢弃次数（M295，修前静默）
// M297（缺陷 504）：**路径切换**可观测性。
//   修前「对端换源」这件事**完全不可观测**，而且发送地址被收包路径**无条件覆盖**
//   （= 谁最后到谁说了算）：队列里任何一个旧源的迟到包都能把发送地址切回一个**已关闭**的
//   socket —— 托管 listener 下表现为「回包发往死地址、客户端只见超时」
//   （M293/M297 现场 strace：send→新源 与 send→旧源 **交替**）。现在把它变成三个计数：
//     path_migrated 路径验证**成功**次数（= 真的换过源并被 ngtcp2 确认，RFC 9000 §9.3）
//     path_missing  写包时 ngtcp2 未给出路径（理论不可达 ⇒ 已兜底发送并**响亮记账**）
//     send_err      px_io_sendto 失败次数（EINTR 已在内部重试；只有其它错误才计）
static int64_t       g_quic_path_migrated = 0;
static int64_t       g_quic_path_missing  = 0;
static int64_t       g_quic_send_err      = 0;
// M225：连接回收钩子（runtime_h3.c 注册；槽位释放前清理该 conn 的 h3 会话状态）
static px_quic_conn_recycle_cb g_quic_recycle = NULL;

// M53：server 托管全局锁（cid 路由表 + 收包队列 push/pop + conn 清理互斥）
static pthread_mutex_t g_quic_srv_mu = PTHREAD_MUTEX_INITIALIZER;

static uint64_t quic_now(void) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (uint64_t)tp.tv_sec * NGTCP2_SECONDS + (uint64_t)tp.tv_nsec;
}

// M295（缺陷 508）：槽位分配失败时的统一记账 + **限流**报告（首次 + 每 1000 次）。
//   修前这条路径**完全静默**：服务端丢弃 Initial、日志无痕，客户端只看到 connect-fail
//   ⇒「服务端永久不再接受新连接」而无人知晓（M294 的 [6b] 只能靠人工探测才发现）。
// ⚠️ 刻意不做「每次打印」—— M293 的教训：刷屏会挤掉真信号（ERR_DRAINING 曾经刷满日志）。
static void quic_alloc_fail_note(void) {
    g_quic_alloc_fail++;
    if (g_quic_full_first == 0) g_quic_full_first = (int64_t)(quic_now() / NGTCP2_MILLISECONDS);
    if (g_quic_alloc_fail == 1 || (g_quic_alloc_fail % 1000) == 0) {
        fprintf(stderr,
                "[quic] 连接槽位已满（QUIC_CONN_MAX=%d）：新连接被拒绝 —— "
                "累计失败 %lld 次（首次失败于 %lld ms）。"
                "可用 quic_pool_stats() 查询占用；上限见 runtime/runtime_quic.c 的 QUIC_CONN_MAX。\n",
                QUIC_CONN_MAX, (long long)g_quic_alloc_fail, (long long)g_quic_full_first);
    }
}

// ---------- callbacks ----------
static void quic_rand_cb(uint8_t* dest, size_t destlen,
                         const ngtcp2_rand_ctx* rctx) {
    (void)rctx;
    if (RAND_bytes(dest, (int)destlen) != 1) abort();
}

// M53：cid 路由表登记/查找/移除（listener 级；供收包路由线程按 DCID 定位 conn）
// 并发：读写均持 g_quic_srv_mu（get_new_cid_cb 在处理线程触发，router 在收包线程触发）
static void quic_cid_add(quic_listener* ql, const uint8_t* cid, size_t len, int64_t conn) {
    if (!ql || len == 0 || len > NGTCP2_MAX_CIDLEN) return;
    pthread_mutex_lock(&g_quic_srv_mu);
    for (int i = 0; i < ql->cidtab_n; i++) {
        if (ql->cidtab[i].cidlen == len && memcmp(ql->cidtab[i].cid, cid, len) == 0) {
            ql->cidtab[i].conn = conn;   // 更新归属（cid 轮换后同值重绑）
            pthread_mutex_unlock(&g_quic_srv_mu);
            return;
        }
    }
    if (ql->cidtab_n < QUIC_CIDTAB_MAX) {
        memcpy(ql->cidtab[ql->cidtab_n].cid, cid, len);
        ql->cidtab[ql->cidtab_n].cidlen = len;
        ql->cidtab[ql->cidtab_n].conn = conn;
        ql->cidtab_n++;
    } else {
        // M295（缺陷 508）·第二面：**修前这里是静默丢弃** —— 新连接的短头包将路由不到，
        //   客户端只看到 TIMEOUT（与「服务端没起来」不可区分），而 conn 槽位可能才用了一半。
        g_quic_cid_drop++;
        if (g_quic_cid_drop == 1 || (g_quic_cid_drop % 1000) == 0) {
            fprintf(stderr,
                    "[quic] cid 路由表已满（QUIC_CIDTAB_MAX=%d, listener=%lld）：新 cid 无法登记 —— "
                    "该连接的后续短头包会被丢弃（客户端表现为 TIMEOUT）。累计 %lld 次。\n",
                    QUIC_CIDTAB_MAX, (long long)(ql - g_qlis + 1), (long long)g_quic_cid_drop);
        }
    }
    pthread_mutex_unlock(&g_quic_srv_mu);
}

static int64_t quic_cid_find(quic_listener* ql, const uint8_t* cid, size_t len) {
    if (!ql) return -1;
    pthread_mutex_lock(&g_quic_srv_mu);
    int64_t r = -1;
    for (int i = 0; i < ql->cidtab_n; i++) {
        if (ql->cidtab[i].cidlen == len && memcmp(ql->cidtab[i].cid, cid, len) == 0) {
            r = ql->cidtab[i].conn;
            break;
        }
    }
    pthread_mutex_unlock(&g_quic_srv_mu);
    return r;
}

static int quic_get_new_cid_cb(ngtcp2_conn* conn, ngtcp2_cid* cid,
                               uint8_t* token, size_t cidlen, void* user_data) {
    (void)conn;
    quic_conn* qc = (quic_conn*)user_data;
    if (cidlen > NGTCP2_MAX_CIDLEN) cidlen = NGTCP2_MAX_CIDLEN;
    if (RAND_bytes(cid->data, (int)cidlen) != 1) return NGTCP2_ERR_CALLBACK_FAILURE;
    cid->datalen = cidlen;
    if (RAND_bytes(token, NGTCP2_STATELESS_RESET_TOKENLEN) != 1)
        return NGTCP2_ERR_CALLBACK_FAILURE;
    // M53：新 cid 登记进归属 listener 路由表（服务端收包路由需要）
    if (qc && qc->owner_listener > 0 && qc->owner_listener <= QUIC_MAX_LISTENERS) {
        quic_listener* ql = &g_qlis[qc->owner_listener - 1];
        if (ql->used) quic_cid_add(ql, cid->data, cidlen, qc - g_qconns + 1);
    }
    return 0;
}

static int quic_handshake_completed_cb(ngtcp2_conn* conn, void* user_data) {
    (void)conn;
    quic_conn* qc = (quic_conn*)user_data;
    qc->handshake_done = 1;
    // M54-S2：0-RTT early data 状态（OpenSSL 在握手完成后才报告 accepted/rejected）。
    // 若服务端拒绝了 early data：必须调 ngtcp2_conn_tls_early_data_rejected 通知 ngtcp2
    // 释放 0-RTT 发送状态 → 后续 write 把 0-RTT 流数据按 1-RTT 重发 —— 否则客户端误以
    // 为 early data 已送达（实际被服务端丢弃），服务端永远收不到请求且重传状态错乱。
    if (qc->ssl) {
        int st = SSL_get_early_data_status(qc->ssl);
        if (st == SSL_EARLY_DATA_REJECTED) {
            qc->early_rejected = 1;
            if (qc->conn) ngtcp2_conn_tls_early_data_rejected(qc->conn);
        }
    }
    return 0;
}

// M54-S3：握手确认（HANDSHAKE_DONE 到达/发出）——客户端发起连接迁移的前提（RFC 9000 §9）
static int quic_hs_confirmed_cb(ngtcp2_conn* conn, void* user_data) {
    (void)conn;
    quic_conn* qc = (quic_conn*)user_data;
    if (qc) qc->hs_confirmed = 1;
    return 0;
}

// ---------- per-stream 槽管理（M50）----------
static quic_stream* quic_stream_slot(quic_conn* qc, int64_t sid) {
    for (int i = 0; i < QUIC_STREAM_MAX; i++)
        if (qc->streams[i].used && qc->streams[i].sid == sid) return &qc->streams[i];
    return NULL;
}

static quic_stream* quic_stream_new(quic_conn* qc, int64_t sid, int peer) {
    quic_stream* s = quic_stream_slot(qc, sid);
    if (s) return s;
    for (int i = 0; i < QUIC_STREAM_MAX; i++) {
        if (!qc->streams[i].used) {
            qc->streams[i].used = 1;
            qc->streams[i].sid = sid;
            qc->streams[i].buf = NULL;
            qc->streams[i].len = qc->streams[i].cap = 0;
            qc->streams[i].fin = 0;
            qc->streams[i].peer = peer;
            return &qc->streams[i];
        }
    }
    return NULL;  // 槽满
}

static void quic_stream_free_all(quic_conn* qc) {
    for (int i = 0; i < QUIC_STREAM_MAX; i++) {
        if (qc->streams[i].buf) { free(qc->streams[i].buf); qc->streams[i].buf = NULL; }
        qc->streams[i].used = 0;
    }
}

// 任一流有数据/FIN → 返回 sid 最小者（确定性优先序）；无则 NULL
static quic_stream* quic_any_data(quic_conn* qc) {
    quic_stream* best = NULL;
    for (int i = 0; i < QUIC_STREAM_MAX; i++) {
        quic_stream* s = &qc->streams[i];
        if (!s->used) continue;
        // M53-S4：仅"有未读数据"的流视为活跃。fin-only 流（对端 FIN 已到、数据已读空）
        // 此前也算活跃 → px_quic_raw_poll 恒返回已结束流 → H3 server 连接线程忙循环烧 CPU、
        // close_listener join 永不返回（px_serve SIGTERM 优雅关闭挂死）。数据都在 len 中，
        // fin 只是结束标记：len>0 已涵盖一切可读数据。
        if (s->len > 0 && (!best || s->sid < best->sid)) best = s;
    }
    return best;
}

// 默认流 = 已存在流中 sid 最小者（单流 M46/M47 兼容语义）；无流返回 -1
static int64_t quic_default_stream(quic_conn* qc) {
    int64_t best = -1;
    for (int i = 0; i < QUIC_STREAM_MAX; i++) {
        quic_stream* s = &qc->streams[i];
        if (!s->used) continue;
        if (best < 0 || s->sid < best) best = s->sid;
    }
    return best;
}

static int quic_recv_stream_data_cb(ngtcp2_conn* conn, uint32_t flags,
                                    int64_t stream_id, uint64_t offset,
                                    const uint8_t* data, size_t datalen,
                                    void* user_data, void* stream_user_data) {
    (void)conn; (void)offset; (void)stream_user_data;
    quic_conn* qc = (quic_conn*)user_data;
    quic_stream* s = quic_stream_slot(qc, stream_id);
    if (!s) s = quic_stream_new(qc, stream_id, 1);   // 对端首见建槽
    if (!s) return 0;
    if (datalen > 0) {
        if (s->len + datalen > s->cap) {
            size_t ncap = s->cap ? s->cap : 4096;
            while (ncap < s->len + datalen) ncap *= 2;
            if (ncap > (1u << 22)) return 0;         // 单流缓冲上限 4MB
            uint8_t* nb = (uint8_t*)realloc(s->buf, ncap);
            if (!nb) return 0;
            s->buf = nb;
            s->cap = ncap;
        }
        memcpy(s->buf + s->len, data, datalen);
        s->len += datalen;
    }
    if (flags & NGTCP2_STREAM_DATA_FLAG_FIN) s->fin = 1;
    return 0;
}

// M296 诊断（临时）：PX_QUIC_VERBOSE=1 时把 ngtcp2 内部日志打到 stderr。
//   用途：DROP_CONN 这类「ngtcp2 只给一个错误码、不说为什么」的现场（本轮 511 定位用）。
static void quic_log_cb(void* user_data, const char* fmt, ...) {
    (void)user_data;
    static int on = -1;
    if (on < 0) { const char* e = getenv("PX_QUIC_VERBOSE"); on = (e && e[0] && e[0] != '0') ? 1 : 0; }
    if (!on) return;
    va_list ap; va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
}

// M297（缺陷 504）：诊断开关读取（与 quic_log_cb 同口径；放全局便于两个回调共用）
static int quic_verbose_on(void) {
    static int on = -1;
    if (on < 0) {
        const char* e = getenv("PX_QUIC_VERBOSE");
        on = (e && e[0] && e[0] != '0') ? 1 : 0;
    }
    return on;
}

// M297 诊断：单 addr -> "ip:port"
static void quic_dbg_addr1(const ngtcp2_addr* a, char* out, size_t n) {
    out[0] = 0;
    if (!a || !a->addr) { snprintf(out, n, "?"); return; }
    char ip[INET6_ADDRSTRLEN];
    int port = 0;
    if (a->addr->sa_family == AF_INET) {
        const struct sockaddr_in* s4 = (const struct sockaddr_in*)a->addr;
        if (inet_ntop(AF_INET, &s4->sin_addr, ip, sizeof(ip))) port = ntohs(s4->sin_port);
        else { snprintf(out, n, "?"); return; }
    } else if (a->addr->sa_family == AF_INET6) {
        const struct sockaddr_in6* s6 = (const struct sockaddr_in6*)a->addr;
        if (inet_ntop(AF_INET6, &s6->sin6_addr, ip, sizeof(ip))) port = ntohs(s6->sin6_port);
        else { snprintf(out, n, "?"); return; }
    } else { snprintf(out, n, "?"); return; }
    snprintf(out, n, "%s:%d", ip, port);
}

// M297（缺陷 504）：**路径验证开始**回调 —— 修前这条链路**完全不可观测**
//   （「对端换源」只能靠 strace 看 sendto 目标交替）。PX_QUIC_VERBOSE=1 时打印。
static int quic_begin_path_validation_cb(ngtcp2_conn* conn, uint32_t flags,
                                         const ngtcp2_path* path, const ngtcp2_path* fallback_path,
                                         void* user_data) {
    (void)conn; (void)fallback_path; (void)user_data;
    if (quic_verbose_on()) {
        char np[128] = "?", fp[128] = "?";
        if (path) quic_dbg_addr1(&path->remote, np, sizeof(np));
        if (fallback_path) quic_dbg_addr1(&fallback_path->remote, fp, sizeof(fp));
        fprintf(stderr, "[quic] path-validation BEGIN flags=%u new=%s fallback=%s\n",
                (unsigned)flags, np, fp);
    }
    return 0;
}

// M297（缺陷 504）：**路径验证结果**回调。
//   RFC 9000 §9.3：只有在「从新地址收到**非探测包**」并完成路径验证之后，才把该地址
//   作为当前路径；探测包（PATH_CHALLENGE / PATH_RESPONSE / NEW_CONNECTION_ID / PADDING）
//   **不**触发迁移。ngtcp2 自己维护当前路径（`ngtcp2_conn_write_*` 的输出 path 会跟随它），
//   本回调只**同步我们自己的元信息**（`quic_conn_path` / `quic_conn_local` 读的就是这两个
//   字段），并记账以便观测。修前这里是「无条件跟随来源地址」—— 那正是缺陷 504 的根因。
//   ⚠️ 只在 SUCCESS 时更新（与 ngtcp2 官方 server 示例 `update_path` 同口径）；
//      FAILURE/ABORTED 时当前路径仍是原路径，无需动作（fallback_path 由 ngtcp2 自己用）。
static int quic_path_validation_cb(ngtcp2_conn* conn, uint32_t flags,
                                   const ngtcp2_path* path, const ngtcp2_path* fallback_path,
                                   ngtcp2_path_validation_result res, void* user_data) {
    (void)conn; (void)flags; (void)fallback_path;
    quic_conn* qc = (quic_conn*)user_data;
    if (!qc) return 0;
    if (res == NGTCP2_PATH_VALIDATION_RESULT_SUCCESS && path &&
        path->remote.addr && path->remote.addrlen > 0 &&
        (size_t)path->remote.addrlen <= sizeof(qc->remote_sa)) {
        memcpy(&qc->remote_sa, path->remote.addr, (size_t)path->remote.addrlen);
        qc->path.remote.addr = (struct sockaddr*)&qc->remote_sa;
        qc->path.remote.addrlen = (socklen_t)path->remote.addrlen;
        g_quic_path_migrated++;
    }
    if (quic_verbose_on()) {
        char np[128] = "?";
        if (path) quic_dbg_addr1(&path->remote, np, sizeof(np));
        fprintf(stderr, "[quic] path-validation END res=%d remote=%s（累计成功 %lld）\n",
                (int)res, np, (long long)g_quic_path_migrated);
    }
    return 0;
}

// ALPN 选择回调：选客户端提供的第一个协议（对齐 ngtcp2 官方 ossl 例子）
static int quic_alpn_select_cb(SSL* ssl, const unsigned char** out,
                               unsigned char* outlen, const unsigned char* in,
                               unsigned int inlen, void* arg) {
    (void)ssl; (void)arg;
    const unsigned char* p = in;
    const unsigned char* end = in + inlen;
    while (p + 1 <= end) {
        unsigned char l = *p++;
        if (p + l <= end) {
            *out = p;
            *outlen = l;
            return SSL_TLSEXT_ERR_OK;
        }
        p += l;
    }
    return SSL_TLSEXT_ERR_NOACK;
}

// ---------- 证书：内存生成 ECDSA P-256 自签（老 API + 显式 provider，静态链接兼容）----------
static EVP_PKEY* quic_gen_ec_key(void) {
    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    if (!pctx) return NULL;
    if (EVP_PKEY_keygen_init(pctx) <= 0 ||
        EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1) <= 0 ||
        EVP_PKEY_keygen(pctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        return NULL;
    }
    EVP_PKEY_CTX_free(pctx);
    return pkey;
}

static SSL_CTX* quic_make_server_ctx(void) {
    EVP_PKEY* pkey = quic_gen_ec_key();
    if (!pkey) return NULL;
    X509* x509 = X509_new();
    if (!x509) { EVP_PKEY_free(pkey); return NULL; }
    X509_set_version(x509, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 0x20260902L);
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 365 * 24 * 3600L);
    X509_set_pubkey(x509, pkey);
    X509_NAME* name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                               (const unsigned char*)"puxian-quic", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    if (X509_sign(x509, pkey, EVP_sha256()) <= 0) {
        X509_free(x509); EVP_PKEY_free(pkey); return NULL;
    }
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) { X509_free(x509); EVP_PKEY_free(pkey); return NULL; }
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_ECDH_USE | SSL_OP_CIPHER_SERVER_PREFERENCE |
                              SSL_OP_NO_ANTI_REPLAY);
    SSL_CTX_set_mode(ctx, SSL_MODE_RELEASE_BUFFERS);
    SSL_CTX_set_ciphersuites(ctx, "TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256");
    SSL_CTX_set1_groups_list(ctx, "X25519:P-256");
    SSL_CTX_set_alpn_select_cb(ctx, quic_alpn_select_cb, NULL);
    // M54-S1: TLS1.3 stateless session ticket（会话恢复前提）
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);
    SSL_CTX_set_session_id_context(ctx, (const unsigned char*)"puxian-quic", 11);
    // M54-S2: 允许 0-RTT early data（签发带 max_early_data 的 ticket，1MB 上限）
    SSL_CTX_set_max_early_data(ctx, 1u << 20);
    if (SSL_CTX_use_certificate(ctx, x509) != 1 ||
        SSL_CTX_use_PrivateKey(ctx, pkey) != 1) {
        fprintf(stderr, "[quic] server ctx use_cert/key FAIL\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx); X509_free(x509); EVP_PKEY_free(pkey);
        return NULL;
    }
    if (ngtcp2_crypto_quictls_configure_server_context(ctx) != 0) {
        fprintf(stderr, "[quic] server ctx quictls configure FAIL\n");
        SSL_CTX_free(ctx); X509_free(x509); EVP_PKEY_free(pkey);
        return NULL;
    }
    X509_free(x509); EVP_PKEY_free(pkey);
    return ctx;
}

static SSL_CTX* quic_make_client_ctx(void) {
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) return NULL;
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);  // MVP 不校验自签证书
    if (ngtcp2_crypto_quictls_configure_client_context(ctx) != 0) {
        SSL_CTX_free(ctx);
        return NULL;
    }
    return ctx;
}

// M297（缺陷 504）：sockaddr_storage → 实际长度（收包路径需要给 ngtcp2 一个准确的 addrlen）
static socklen_t quic_sa_len(const struct sockaddr_storage* sa) {
    if (sa->ss_family == AF_INET6) return (socklen_t)sizeof(struct sockaddr_in6);
    if (sa->ss_family == AF_INET)  return (socklen_t)sizeof(struct sockaddr_in);
    return 0;
}

// M297（缺陷 504）：把 ngtcp2 写出的包**按它给出的路径**发出。
//   修前：5 处发送点一律 `sendto(..., &qc->remote_sa)`，而 remote_sa 被**收包路径无条件覆盖**
//   ⇒ 谁最后到谁说了算（旧源迟到包 ⇒ 回包发往已关闭的 socket）。托管 listener 下必现。
//   修后：发送目标只由 ngtcp2 的**输出 path** 决定 —— 它按 RFC 9000 §9.3 维护当前路径
//   （路径验证成功才切到新地址），我们不再自己记「对端在哪」。
//   ⚠️ 顺带收口 M295s1（缺陷 510）同族：一律走 px_io_sendto（裸 sendto 不重试 EINTR，
//      并发 GC 的 STW 信号会打断阻塞中的系统调用 ⇒ 静默丢包）。
//   ⚠️ 兜底：ngtcp2 未填 path（理论不可达）时退回已知地址并**记账**（不许静默）。
static void quic_send_to_path(quic_conn* qc, const uint8_t* buf, size_t len,
                              const ngtcp2_path* op) {
    const struct sockaddr* dst;
    socklen_t dlen;
    if (op && op->remote.addr && op->remote.addrlen > 0) {
        dst = (const struct sockaddr*)op->remote.addr;
        dlen = (socklen_t)op->remote.addrlen;
    } else {
        g_quic_path_missing++;
        dst = (const struct sockaddr*)&qc->remote_sa;
        dlen = (socklen_t)sizeof(qc->remote_sa);
        if (g_quic_path_missing == 1 || (g_quic_path_missing % 1000) == 0) {
            fprintf(stderr, "[quic] write 未给出路径（累计 %lld 次）—— 已退回已知对端地址\n",
                    (long long)g_quic_path_missing);
        }
    }
    if (px_io_sendto(qc->fd, buf, len, 0, dst, dlen) < 0) g_quic_send_err++;
}

// ---------- 事件泵：收包→喂 conn→发包，直到 handshake 或数据就绪或超时 ----------
// mode: 0=握手模式（直到 handshake_done）; 1=数据模式（直到 rlen>0 或对端关闭）
// 返回: 0 成功 / -1 超时 / -2 错误
// data_mode=1 时：want_sid>=0 → 泵到指定流有数据/FIN；want_sid<0 → 泵到任一流有数据
// M53：收包队列 pop 前向声明（托管 conn 泵用；定义见 M53 块）
static int quic_q_pop(quic_conn* qc, uint8_t** pkt, int* plen,
                      struct sockaddr_storage* from, int timeout_ms);

static int quic_pump(quic_conn* qc, int64_t timeout_ms, int data_mode, int64_t want_sid) {
    struct pollfd pfd = { .fd = qc->fd, .events = POLLIN };
    int64_t deadline = quic_now() + (uint64_t)timeout_ms * NGTCP2_MILLISECONDS;
    // M53：托管 conn（owner_listener>0）由收包路由线程投包 → 走队列取包；
    // 普通 conn（demo accept/connect）保持原 poll+recvfrom 单 fd 语义。
    int qmode = (qc->owner_listener > 0);
    for (;;) {
        /* M50: data_mode 目标 = 活跃流有数据/FIN；M51: 读指定流需避免其他流数据导致空转 */
        if (data_mode) {
            if (want_sid >= 0) {
                quic_stream* t = quic_stream_slot(qc, want_sid);
                if (t && (t->len > 0 || t->fin)) return 0;
                if (qc->peer_closed) return 0;
            } else {
                if (quic_any_data(qc) != NULL) return 0;
                if (qc->peer_closed) return 0;
            }
        } else {
            if (qc->handshake_done) return 0;
        }
        // 发送待发数据（握手/ACK/流控帧）
        uint8_t out[QUIC_PKT_BUF];
        // M297（缺陷 504）：path 用**局部输出缓冲**接收 —— 发送目标由 ngtcp2 决定
        ngtcp2_path_storage ps;
        ngtcp2_path_storage_zero(&ps);
        ngtcp2_ssize n = ngtcp2_conn_write_pkt(qc->conn, &ps.path, NULL,
                                               out, sizeof(out), quic_now());
        if (n > 0) {
            quic_send_to_path(qc, out, (size_t)n, &ps.path);
        } else if (n < 0 && n != NGTCP2_ERR_WRITE_MORE) {
            fprintf(stderr, "[quic] pump write_pkt rv=%zd (%s)\n", n, ngtcp2_strerror((int)n));
            return -2;
        }
        if (qc->peer_closed) return data_mode ? 0 : -1;
        // 等待收包
        int64_t remain = deadline - quic_now();
        if (remain <= 0) return -1;
        int ms = (int)((remain + NGTCP2_MILLISECONDS - 1) / NGTCP2_MILLISECONDS);
        if (ms > 500) ms = 500;   // 分段等，避免长眠漏状态（qmode 需周期性 flush）
        uint8_t pkt[QUIC_PKT_BUF];
        int plen = 0;
        struct sockaddr_storage from;
        memset(&from, 0, sizeof(from));
        int got_pkt = 0;
        if (qmode) {
            uint8_t* qpkt = NULL; int qlen = 0;
            int dv = quic_q_pop(qc, &qpkt, &qlen, &from, ms);
            if (dv == 1) {
                plen = qlen;
                if (plen > (int)sizeof(pkt)) plen = (int)sizeof(pkt);
                memcpy(pkt, qpkt, (size_t)plen);
                free(qpkt);
                got_pkt = 1;
            } else if (dv == -1) {
                return data_mode ? 0 : -1;   // qclosed
            }
            // dv==0 超时 → 继续（deadline 判超时）
        } else {
            int pr = poll(&pfd, 1, ms);
            if (pr < 0) { if (errno == EINTR) continue; fprintf(stderr, "[quic] pump poll err=%s\n", strerror(errno)); return -2; }
            if (pr == 0) return -1;  // 超时
            socklen_t fromlen = sizeof(from);
            ssize_t rl = recvfrom(qc->fd, pkt, sizeof(pkt), 0,
                                  (struct sockaddr*)&from, &fromlen);
            if (rl <= 0) { if (errno == EINTR || errno == EAGAIN) continue; fprintf(stderr, "[quic] pump recvfrom err=%s\n", strerror(errno)); return -2; }
            plen = (int)rl;
            got_pkt = 1;
        }
        if (!got_pkt) continue;
        // M297（缺陷 504）：把「这个包**实际来自**的路径」交给 ngtcp2 —— 路径的比较与验证
        //   由它按 RFC 9000 §9.3 处理（非探测包才可能触发迁移；验证成功由
        //   quic_path_validation_cb 通知我们）。**不再**无条件改写 qc->path / qc->remote_sa：
        //   修前那样做等于「谁最后到、谁说了算」—— 队列里旧源的迟到包会把发送地址切回
        //   一个已关闭的 socket（托管 listener 下回包发往死地址、客户端只见超时）。
        socklen_t flen = quic_sa_len(&from);
        // M297（缺陷 504）：这条包该配到哪条路径上？
        //   · **local 用 ngtcp2 自己认为的当前本地地址**（current path.local），**不是**
        //     qc->local_sa —— ngtcp2 里存的是**地址副本**，不跟随我们的字段。
        //     客户端 quic_migrate 换了本地地址后，qc->local_sa 已是新地址而 ngtcp2 的
        //     current path 仍是旧地址；若我们把新地址当 path 喂进去，路径就对不上
        //     ⇒ 连对端发来的 PATH_CHALLENGE 都不回应 ⇒ 服务端路径验证永不完成
        //     ⇒ 当前路径不切换 ⇒ 回包继续发往旧（已关闭的）socket ⇒ 迁移后请求超时。
        //     （实测留证 /tmp/m297/diag3run.log：`migrate: qc->local_sa=…:50156
        //      ngtcp2 current=(…:59047 -> …)`）
        //   · **remote 用这个包的真实来源** —— 只有这样才能发现「对端换源」
        //     （RFC 9000 §9.3；服务端靠它触发路径验证，验证成功由回调通知我们切地址）。
        const ngtcp2_path* cur = ngtcp2_conn_get_path(qc->conn);
        const ngtcp2_sockaddr* lsa_p = (cur && cur->local.addr)
                                       ? (const ngtcp2_sockaddr*)cur->local.addr
                                       : (const ngtcp2_sockaddr*)&qc->local_sa;
        ngtcp2_socklen lsa_len = (cur && cur->local.addr)
                                 ? cur->local.addrlen : quic_sa_len(&qc->local_sa);
        ngtcp2_path_storage sps;
        if (flen > 0) {
            ngtcp2_path_storage_init(&sps, lsa_p, lsa_len, (const ngtcp2_sockaddr*)&from,
                                     (ngtcp2_socklen)flen, NULL);
        } else {
            ngtcp2_path_storage_init(&sps, lsa_p, lsa_len,
                                     (const ngtcp2_sockaddr*)&qc->remote_sa,
                                     quic_sa_len(&qc->remote_sa), NULL);
        }
        // 过期定时器
        ngtcp2_conn_handle_expiry(qc->conn, quic_now());
        // 喂包
        int rv = ngtcp2_conn_read_pkt(qc->conn, &sps.path, NULL, pkt, (size_t)plen,
                                      quic_now());
        if (rv != 0) {
            if (rv == NGTCP2_ERR_DRAINING || rv == NGTCP2_ERR_DROP_CONN) {
                // M295（缺陷 508）：这是**对端正常关闭**（CONNECTION_CLOSE 到达）—— 协议流程，
                //   不是错误。修前之所以不打它是「碰巧」：客户端从来不发 CONNECTION_CLOSE
                //   （见 bi_quic_close 的 M295 注释）。本轮的 quic_close 修复激活了这条路径，
                //   若继续打印，**每关一条连接就刷一行**（实测 300 次建连环 ⇒ 300 行）。
                //   M293 的教训：刷屏会挤掉真信号 ⇒ 改为**计数**（quic_pool_stats().peer_closed_cnt），
                //   真错误（其它 rv）仍然打印。
                g_quic_peer_closed++;
                qc->peer_closed = 1;
                return data_mode ? 0 : -1;
            }
            fprintf(stderr, "[quic] read_pkt rv=%d (%s)\n", rv, ngtcp2_strerror(rv));
            return -2;
        }
    }
}

// ============================================================
// M53：HTTP/3 server 多连接托管（收包路由 + 自动 accept + 每连接处理线程）
// ------------------------------------------------------------
// 背景：demo 级 quic_accept 每连接共享 listener fd 自行 recvfrom，多连接互相抢包。
// M53 引入"单 fd 收包路由"：router 线程统一 recvfrom → 按包头 DCID 路由到对应
// conn 的入包队列；每连接处理线程（quic_srv_conn_thr）消费队列泵 ngtcp2，
// 握手完成后执行连接回调（默认 QUIC echo；HTTP/3 接入时由 runtime_h3.c 覆盖）。
// 兼容性：旧 quic_accept / quic_pump 路径保持不变（m46–m52 不回归）。
// ============================================================
// 前向声明（定义于本段之后的 accept/listener 区）
static int64_t quic_alloc_listener(quic_listener* ql);
static quic_listener* quic_get_listener(int64_t id);
static int64_t quic_alloc_conn(quic_conn* qc);
static quic_conn* quic_get_conn(int64_t id);

#define QUIC_QCAP 64

// 全局连接回调（runtime_h3.c 在注册时设置；NULL → 默认 echo）
static quic_conn_cb g_quic_conn_cb = NULL;
static void*        g_quic_conn_ud = NULL;
void px_quic_raw_h3_set_conn_cb(void (*cb)(int64_t conn, void* ud), void* ud) {
    g_quic_conn_cb = cb;
    g_quic_conn_ud = ud;
}

// 证书加载：cert/key 非空 → PEM 链/私钥；空 → 自签兜底（demo/测试）
static SSL_CTX* quic_make_server_ctx_cert(const char* certfile, const char* keyfile) {
    if (!certfile || !*certfile || !keyfile || !*keyfile)
        return quic_make_server_ctx();
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) return NULL;
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_ECDH_USE | SSL_OP_CIPHER_SERVER_PREFERENCE |
                              SSL_OP_NO_ANTI_REPLAY);
    SSL_CTX_set_mode(ctx, SSL_MODE_RELEASE_BUFFERS);
    SSL_CTX_set_ciphersuites(ctx, "TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256");
    SSL_CTX_set1_groups_list(ctx, "X25519:P-256");
    SSL_CTX_set_alpn_select_cb(ctx, quic_alpn_select_cb, NULL);
    // M54-S1: TLS1.3 stateless session ticket（会话恢复前提）
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);
    SSL_CTX_set_session_id_context(ctx, (const unsigned char*)"puxian-quic", 11);
    // M54-S2: 允许 0-RTT early data（签发带 max_early_data 的 ticket，1MB 上限）
    SSL_CTX_set_max_early_data(ctx, 1u << 20);
    if (SSL_CTX_use_certificate_chain_file(ctx, certfile) != 1 ||
        SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM) != 1) {
        fprintf(stderr, "[quic] h3 server ctx 加载证书失败 cert=%s key=%s\n", certfile, keyfile);
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    if (ngtcp2_crypto_quictls_configure_server_context(ctx) != 0) {
        fprintf(stderr, "[quic] h3 server ctx quictls configure FAIL\n");
        SSL_CTX_free(ctx);
        return NULL;
    }
    return ctx;
}

// 收包队列（push 由 router 线程、pop 由连接处理线程；统一持 g_quic_srv_mu）
static int quic_q_init(quic_conn* qc) {
    qc->qcap = QUIC_QCAP; qc->qhead = qc->qtail = qc->qn = 0; qc->qclosed = 0;
    qc->qbufs = (uint8_t**)calloc(qc->qcap, sizeof(uint8_t*));
    qc->qlens = (int*)calloc(qc->qcap, sizeof(int));
    qc->qfroms = (struct sockaddr_storage*)calloc(qc->qcap, sizeof(struct sockaddr_storage));
    if (!qc->qbufs || !qc->qlens || !qc->qfroms) {
        if (qc->qbufs) free(qc->qbufs);
        if (qc->qlens) free(qc->qlens);
        if (qc->qfroms) free(qc->qfroms);
        qc->qbufs = NULL; qc->qlens = NULL; qc->qfroms = NULL;
        return -1;
    }
    pthread_cond_init(&qc->qcv, NULL);
    return 0;
}

static void quic_q_push(quic_conn* qc, const uint8_t* pkt, int len,
                        const struct sockaddr_storage* from) {
    pthread_mutex_lock(&g_quic_srv_mu);
    if (!qc || !qc->qbufs || qc->qclosed) { pthread_mutex_unlock(&g_quic_srv_mu); return; }
    if (qc->qn >= qc->qcap) {   // 满：丢最旧（QUIC 会重传，MVP 可接受）
        uint8_t* old = qc->qbufs[qc->qhead];
        if (old) free(old);
        qc->qbufs[qc->qhead] = NULL;
        qc->qhead = (qc->qhead + 1) % qc->qcap;
        qc->qn--;
    }
    size_t tail = (qc->qhead + qc->qn) % qc->qcap;
    uint8_t* cp = (uint8_t*)malloc((size_t)len);
    if (!cp) { pthread_mutex_unlock(&g_quic_srv_mu); return; }
    memcpy(cp, pkt, (size_t)len);
    qc->qbufs[tail] = cp;
    qc->qlens[tail] = len;
    if (from) qc->qfroms[tail] = *from;
    qc->qn++;
    pthread_cond_signal(&qc->qcv);
    pthread_mutex_unlock(&g_quic_srv_mu);
}

// 返回 1 有包 / 0 超时 / -1 队列关闭
static int quic_q_pop(quic_conn* qc, uint8_t** pkt, int* plen,
                      struct sockaddr_storage* from, int timeout_ms) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
    pthread_mutex_lock(&g_quic_srv_mu);
    while (qc->qn == 0 && !qc->qclosed) {
        if (pthread_cond_timedwait(&qc->qcv, &g_quic_srv_mu, &ts) != 0)
            break;
    }
    if (qc->qn == 0) {
        int r = qc->qclosed ? -1 : 0;
        pthread_mutex_unlock(&g_quic_srv_mu);
        return r;
    }
    *pkt = qc->qbufs[qc->qhead];
    *plen = qc->qlens[qc->qhead];
    if (from) *from = qc->qfroms[qc->qhead];
    qc->qbufs[qc->qhead] = NULL;
    qc->qhead = (qc->qhead + 1) % qc->qcap;
    qc->qn--;
    pthread_mutex_unlock(&g_quic_srv_mu);
    return 1;
}

// 由收到的长头 Initial 包创建服务端连接（握手后续由处理线程队列泵完成）。
// 复制自 bi_quic_accept 创建段，另登记初始 scid 到 listener cid 路由表。
static int quic_srv_new_conn(quic_listener* ql, const uint8_t* pkt, size_t rl,
                             const struct sockaddr_storage* from, socklen_t fromlen,
                             int64_t* out_cid) {
    ngtcp2_version_cid vc;
    if (ngtcp2_pkt_decode_version_cid(&vc, pkt, rl, 0) != 0) return -1;
    quic_conn qc0; memset(&qc0, 0, sizeof(qc0));
    int64_t cid = quic_alloc_conn(&qc0);
    if (cid < 0) return -1;
    quic_conn* qc = &g_qconns[cid - 1];
    qc->fd = ql->fd;
    qc->ssl_ctx = NULL;
    qc->owner_listener = (int)((ql - g_qlis) + 1);
    if (quic_q_init(qc) != 0) { g_qconns[cid - 1].used = 0; return -1; }
    memcpy(&qc->remote_sa, from, fromlen);
    memcpy(&qc->local_sa, &ql->local, sizeof(ql->local));
    qc->path.local.addr = (struct sockaddr*)&qc->local_sa;
    qc->path.local.addrlen = sizeof(struct sockaddr_in);
    qc->path.remote.addr = (struct sockaddr*)&qc->remote_sa;
    qc->path.remote.addrlen = fromlen;

    ngtcp2_cid cdcid;  // 对端连接 ID = 客户端 Initial 的 SCID
    cdcid.datalen = vc.scidlen;
    memcpy(cdcid.data, vc.scid, vc.scidlen);
    ngtcp2_cid scid;
    scid.datalen = QUIC_SCIDLEN;
    if (RAND_bytes(scid.data, QUIC_SCIDLEN) != 1) {
        if (qc->qbufs) { for (size_t qi = 0; qi < qc->qcap; qi++) if (qc->qbufs[qi]) free(qc->qbufs[qi]); free(qc->qbufs); }
        if (qc->qlens) free(qc->qlens);
        if (qc->qfroms) free(qc->qfroms);
        pthread_cond_destroy(&qc->qcv);
        g_qconns[cid - 1].used = 0;
        return -1;
    }
    ngtcp2_settings settings;
    ngtcp2_settings_default(&settings);
    settings.initial_ts = quic_now();
    settings.log_printf = quic_log_cb;
    ngtcp2_transport_params params;
    ngtcp2_transport_params_default(&params);
    // M54-S4：listener 可配 client bidi 流上限（quic_set_max_client_streams）——经 transport
    // params 下发给对端（0=默认 16）
    params.initial_max_streams_bidi = (ql->max_client_bidi > 0) ? ql->max_client_bidi : 16;
    params.initial_max_streams_uni = 3;
    params.initial_max_stream_data_bidi_local = 128 * 1024;
    params.initial_max_stream_data_bidi_remote = 128 * 1024;
    params.initial_max_stream_data_uni = 128 * 1024;
    params.initial_max_data = 1024 * 1024;
    params.original_dcid.datalen = vc.dcidlen;
    memcpy(params.original_dcid.data, vc.dcid, vc.dcidlen);
    params.original_dcid_present = 1;

    ngtcp2_callbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.recv_client_initial = ngtcp2_crypto_recv_client_initial_cb;
    cb.recv_crypto_data = ngtcp2_crypto_recv_crypto_data_cb;
    cb.handshake_completed = quic_handshake_completed_cb;
    cb.handshake_confirmed = quic_hs_confirmed_cb;
    cb.encrypt = ngtcp2_crypto_encrypt_cb;
    cb.decrypt = ngtcp2_crypto_decrypt_cb;
    cb.hp_mask = ngtcp2_crypto_hp_mask_cb;
    cb.recv_stream_data = quic_recv_stream_data_cb;
    cb.rand = quic_rand_cb;
    cb.get_new_connection_id = quic_get_new_cid_cb;
    cb.remove_connection_id = NULL;
    cb.update_key = ngtcp2_crypto_update_key_cb;
    cb.delete_crypto_aead_ctx = ngtcp2_crypto_delete_crypto_aead_ctx_cb;
    cb.delete_crypto_cipher_ctx = ngtcp2_crypto_delete_crypto_cipher_ctx_cb;
    cb.get_path_challenge_data = ngtcp2_crypto_get_path_challenge_data_cb;
    cb.version_negotiation = ngtcp2_crypto_version_negotiation_cb;
    cb.path_validation = quic_path_validation_cb;   // M297（缺陷 504）
    cb.begin_path_validation = quic_begin_path_validation_cb; // M297 诊断

    int rv = ngtcp2_conn_server_new(&qc->conn, &cdcid, &scid, &qc->path,
                                    vc.version, &cb, &settings, &params, NULL, qc);
    if (rv != 0) {
        fprintf(stderr, "[quic] srv_new server_new rv=%d (%s)\n", rv, ngtcp2_strerror(rv));
        g_qconns[cid - 1].used = 0;
        return -1;
    }
    SSL* ssl = SSL_new(ql->ssl_ctx);
    if (!ssl) { ngtcp2_conn_del(qc->conn); qc->conn = NULL; g_qconns[cid - 1].used = 0; return -1; }
    qc->ssl = ssl;
    ngtcp2_conn_set_tls_native_handle(qc->conn, ssl);
    qc->conn_ref.get_conn = quic_get_conn_from_ref;
    qc->conn_ref.user_data = qc;
    SSL_set_app_data(ssl, &qc->conn_ref);
    SSL_set_accept_state(ssl);
    SSL_set_quic_early_data_enabled(ssl, 1);   // M54-S2：接受 0-RTT early data（官方 quictls 顺序：accept_state 后启用）
    // 登记初始 scid（客户端 1-RTT 短头包 DCID = 本端 scid）
    quic_cid_add(ql, scid.data, scid.datalen, cid);
    // M54-S2：登记客户端 Initial 的 DCID（包头 DCID = vc.dcid）→ 同连接。0-RTT early data
    // 长头包与 Initial 使用同一 DCID（ngtcp2 client 在收到 ServerHello/本端 SCID 前，以
    // Initial 的 DCID 充当服务端 CID），必须可路由到本 conn —— 若误登记 vc.scid（对端 SCID，
    // 仅用于回包 DCID）则 0-RTT 包 cid_find 未命中、被 router 丢弃（early data 静默丢失、
    // 客户端等响应超时/重传状态错乱）。
    {
        ngtcp2_cid init_dcid;
        init_dcid.datalen = vc.dcidlen;
        memcpy(init_dcid.data, vc.dcid, vc.dcidlen);
        quic_cid_add(ql, init_dcid.data, init_dcid.datalen, cid);
    }
    *out_cid = cid;
    return 0;
}

// 默认连接回调：QUIC 流 echo（S1 多连接路由验证用）
static void quic_srv_echo_cb(int64_t conn, void* ud);

// M295（缺陷 508）：槽位清理 —— 从连接线程退出路径里**抽出**，供两处共用：
//   ① 连接线程正常退出；② `pthread_create` **失败**时立即释放（修前那条路径**漏了释放**，
//      会永久泄漏一个 conn 槽 + 它的 cid 路由项 ⇒ 表被"僵尸槽"占满）。
// ⚠️ 调用者必须**已持有** g_quic_srv_mu（与 router 的 cid 路由 / 队列 pop 互斥）。
// ⚠️ 语义与原内联版本逐行一致（含 cidtab 移除与 M225 的 h3 会话回收钩子）。
static void quic_srv_conn_cleanup_locked(quic_conn* qc, int64_t cid) {
    int lid = qc->owner_listener;
    quic_listener* ql = (lid > 0 && lid <= QUIC_MAX_LISTENERS) ? &g_qlis[lid - 1] : NULL;
    if (qc->ssl) { SSL_free(qc->ssl); qc->ssl = NULL; }
    if (qc->conn) { ngtcp2_conn_del(qc->conn); qc->conn = NULL; }
    quic_stream_free_all(qc);
    if (qc->qbufs) {
        for (size_t i = 0; i < qc->qcap; i++) if (qc->qbufs[i]) free(qc->qbufs[i]);
        free(qc->qbufs);
    }
    if (qc->qlens) free(qc->qlens);
    if (qc->qfroms) free(qc->qfroms);
    qc->qbufs = NULL; qc->qlens = NULL; qc->qfroms = NULL;
    qc->qclosed = 1;
    pthread_cond_destroy(&qc->qcv);
    // 从 cid 路由表移除本连接所有项
    if (ql) {
        for (int i = 0; i < ql->cidtab_n; i++) {
            if (ql->cidtab[i].conn == cid) {
                memmove(&ql->cidtab[i], &ql->cidtab[i + 1],
                        (size_t)(ql->cidtab_n - i - 1) * sizeof(quic_cid_entry));
                ql->cidtab_n--;
                i--;
            }
        }
    }
    // M225（缺陷 327）：槽位即将复用 ⇒ 先清该 conn 的 h3 会话状态（QPACK/流缓冲/last_sid），
    // 否则新连接拿到同一 conn 号时会继承上一个连接的 QPACK 上下文。
    if (g_quic_recycle) g_quic_recycle(cid);
    memset(qc, 0, sizeof(*qc));   // used=0 → 槽位可复用
}

// 连接处理线程：泵握手 → 连接回调（HTTP/3 或 echo）→ 清理槽位
static void* quic_srv_conn_thr(void* arg) {
    int64_t cid = (int64_t)(intptr_t)arg;
    quic_conn* qc = quic_get_conn(cid);
    if (!qc) return NULL;
    // M53-S3：注册并发 GC（连接回调构造普贤对象：请求 dict / 响应字段 / handler 调用）。
    // 必须在本线程触碰任何普贤对象之前注册；leave 在清理完成、不再持有对象后调用。
    px_gc_thread_enter();
    int lid = qc->owner_listener;
    quic_listener* ql = (lid > 0 && lid <= QUIC_MAX_LISTENERS) ? &g_qlis[lid - 1] : NULL;
    int pr = quic_pump(qc, 10000, 0, -1);   // 泵到握手完成（最多 10s）
    if (pr == 0 && qc->handshake_done && ql && ql->used) {
        if (ql->conn_cb) ql->conn_cb(cid, ql->conn_ud);
        else quic_srv_echo_cb(cid, NULL);
    }
    // 清理本连接（持 srv_mu 与 router/其他线程互斥）
    pthread_mutex_lock(&g_quic_srv_mu);
    quic_srv_conn_cleanup_locked(qc, cid);
    pthread_mutex_unlock(&g_quic_srv_mu);
    px_gc_thread_leave();
    return NULL;
}

static void quic_srv_start_conn_thr(int64_t cid) {
    quic_conn* qc = quic_get_conn(cid);
    if (!qc) return;
    int thr_create_failed = 0;
    pthread_mutex_lock(&g_quic_srv_mu);
    if (!qc->thr_started) {
        qc->thr_started = 1;
        pthread_t t;
        if (pthread_create(&t, NULL, quic_srv_conn_thr, (void*)(intptr_t)cid) == 0)
            qc->thr = t;
        else {
            qc->thr_started = 0;
            thr_create_failed = 1;   // M295（缺陷 508）：见下
        }
    }
    pthread_mutex_unlock(&g_quic_srv_mu);
    // M295（缺陷 508）：**修前这里只是把 thr_started 置回 0 就返回** ——
    //   而槽位（与 cid 路由项）的释放**只发生在连接线程退出路径**里 ⇒ 没有线程 ⇒ **永久泄漏**。
    //   线程创建失败（资源上限 / 内存压力）下，64 个槽会被"僵尸"逐个占满，且**全程静默**。
    //   ⇒ 必须在此**同步释放**（锁外调用，避免与 cleanup 内的加锁互死）。
    if (thr_create_failed) {
        quic_conn* q2 = quic_get_conn(cid);
        if (q2) {
            pthread_mutex_lock(&g_quic_srv_mu);
            quic_srv_conn_cleanup_locked(q2, cid);
            pthread_mutex_unlock(&g_quic_srv_mu);
        }
        fprintf(stderr, "[quic] pthread_create 失败：连接槽 %lld 已立即释放（不泄漏）\n",
                (long long)cid);
    }
}

// 收包路由线程：poll fd → recvfrom → 按 DCID 路由 / 新 Initial 自动建连接
static void* quic_srv_router(void* arg) {
    int64_t lid = (int64_t)(intptr_t)arg;
    quic_listener* ql = quic_get_listener(lid);
    if (!ql) return NULL;
    struct pollfd pfd = { .fd = ql->fd, .events = POLLIN };
    while (!ql->router_stop) {
        int pr = px_io_poll(&pfd, 1, 200);   // M256（缺陷 456）
        if (pr <= 0) continue;
        uint8_t pkt[QUIC_PKT_BUF];
        struct sockaddr_storage from;
        socklen_t fromlen = sizeof(from);
        ssize_t rl = px_io_recvfrom(ql->fd, pkt, sizeof(pkt), 0,   // M256（缺陷 456）
                                    (struct sockaddr*)&from, &fromlen);
        if (rl <= 0) continue;
        int is_long = (pkt[0] & 0x80) != 0;
        if (is_long) {
            ngtcp2_version_cid vc;
            if (ngtcp2_pkt_decode_version_cid(&vc, pkt, (size_t)rl, 0) != 0) continue;
            int64_t cid = quic_cid_find(ql, vc.dcid, vc.dcidlen);
            if (cid < 0) {
                int ptype = (pkt[0] >> 4) & 0x3;   // Initial=0 / 0-RTT=1 / Handshake=2 / Retry=3
                if (ptype == 0 && vc.version != 0) {
                    int64_t nc = -1;
                    if (quic_srv_new_conn(ql, pkt, (size_t)rl, &from, fromlen, &nc) == 0) {
                        quic_q_push(&g_qconns[nc - 1], pkt, (int)rl, &from);  // 首包入队
                        quic_srv_start_conn_thr(nc);
                    }
                }
                continue;
            }
            quic_conn* qc = quic_get_conn(cid);
            if (qc) quic_q_push(qc, pkt, (int)rl, &from);
        } else {
            if ((size_t)rl < 1 + QUIC_SCIDLEN) continue;
            int64_t cid = quic_cid_find(ql, pkt + 1, QUIC_SCIDLEN);  // 短头 DCID = 本端 scid
            if (cid < 0) continue;
            quic_conn* qc = quic_get_conn(cid);
            if (qc) quic_q_push(qc, pkt, (int)rl, &from);
        }
    }
    return NULL;
}

// M53：关闭托管 listener（停 router → 关托管 conn → 释放）
static void quic_srv_close_listener(int64_t lid) {
    quic_listener* ql = quic_get_listener(lid);
    if (!ql) return;
    ql->router_stop = 1;
    if (ql->router_started) {
        if (ql->fd >= 0) { shutdown(ql->fd, SHUT_RDWR); }
        pthread_join(ql->router_thr, NULL);
        ql->router_started = 0;
    }
    for (int i = 0; i < QUIC_CONN_MAX; i++) {
        if (g_qconns[i].used && g_qconns[i].owner_listener == lid) {
            quic_conn* qc = &g_qconns[i];
            pthread_mutex_lock(&g_quic_srv_mu);
            qc->qclosed = 1;
            pthread_cond_signal(&qc->qcv);
            pthread_mutex_unlock(&g_quic_srv_mu);
            if (qc->thr_started) pthread_join(qc->thr, NULL);
            // 连接线程退出时已清理槽位；若未启动线程则直接清
            if (g_qconns[i].used) quic_srv_conn_thr((void*)(intptr_t)(i + 1));
        }
    }
    if (ql->ssl_ctx) { SSL_CTX_free(ql->ssl_ctx); ql->ssl_ctx = NULL; }
    if (ql->fd >= 0) { close(ql->fd); }
    memset(ql, 0, sizeof(*ql));
}

// 语言 API：quic_h3_listen(port:int, cert:str, key:str) → listener id
static LXValue bi_quic_h3_listen(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    // M195（缺陷 223）：修前是 `nargs < 1`（**无上界**）——
    //   `quic_h3_listen(9000, "c", "k", 多余的)` 被静默接受；`(9000, "c")`（缺 key）也会静默
    //   把 cert 丢掉。M190 的普查只覆盖「三轨 rc **分叉**」，这一类是三轨一致的宽容。
    if (nargs != 1 && nargs != 3)
        px_error("R1002: quic_h3_listen 需要 (port) 或 (port, cert, key) 参数");
    if (args[0].type != PX_INT)
        px_error("R1002: quic_h3_listen 的 port 需要整数，实际是 %s", px_type_name(args[0]));
    int port = (int)args[0].as.i;
    const char* cert = "";
    const char* key = "";
    // M194：可选的 cert/key **存在即校验**（`null` = 未提供）
    if (nargs >= 2 && args[1].type != PX_NULL && args[1].type != PX_STR)
        px_error("R1002: quic_h3_listen 的 cert 需要字符串，实际是 %s", px_type_name(args[1]));
    if (nargs >= 3 && args[2].type != PX_NULL && args[2].type != PX_STR)
        px_error("R1002: quic_h3_listen 的 key 需要字符串，实际是 %s", px_type_name(args[2]));
    if (nargs >= 3 && args[1].type == PX_STR && args[2].type == PX_STR) {
        cert = args[1].as.obj->as.str.data;
        key = args[2].as.obj->as.str.data;
    }
    if (!g_quic_init) {
        ngtcp2_crypto_quictls_init();
        OPENSSL_init_ssl(0, NULL);
        OSSL_PROVIDER_load(NULL, "default");
        g_quic_init = 1;
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return px_int(-1);
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        return px_int(-1);
    }
    SSL_CTX* sctx = quic_make_server_ctx_cert(cert, key);
    if (!sctx) { close(fd); return px_int(-1); }
    quic_listener ql0; memset(&ql0, 0, sizeof(ql0));
    int64_t id = quic_alloc_listener(&ql0);
    if (id < 0) { SSL_CTX_free(sctx); close(fd); return px_int(-1); }
    quic_listener* ql = &g_qlis[id - 1];
    ql->fd = fd;
    ql->local = addr;
    ql->ssl_ctx = sctx;
    ql->used = 1;
    ql->conn_cb = g_quic_conn_cb;
    ql->conn_ud = g_quic_conn_ud;
    ql->router_stop = 0;
    ql->router_started = 1;
    if (pthread_create(&ql->router_thr, NULL, quic_srv_router, (void*)(intptr_t)id) != 0) {
        ql->router_started = 0;
        SSL_CTX_free(sctx);
        close(fd);
        memset(ql, 0, sizeof(*ql));
        return px_int(-1);
    }
    return px_int(id);
}

// ============================================================
// ============================================================
// raw：启动 h3 server listener（语言 bi_quic_h3_listen 的 raw 封装）
int64_t px_quic_raw_h3_listen(int port, const char* cert, const char* key) {
    // M206（缺陷 252）：`a[2] = px_str(key)` 那次分配会回收刚建的 `a[1]`（裸 C 数组槽）。
    px_root_push();
    LXValue a[3];
    a[0] = px_int(port);
    a[1] = px_str(cert ? cert : ""); PX_KEEP(a[1]);
    a[2] = px_str(key ? key : "");   PX_KEEP(a[2]);
    LXValue r = bi_quic_h3_listen(a, 3, NULL);
    px_root_pop();
    return r.type == PX_INT ? r.as.i : -1;
}

// ==================== M53-S3：H3 listener（显式连接回调） ====================
// 与 bi_quic_h3_listen 同构，但连接回调显式传入（runtime_h3.c 管道托管用它；
// 避免依赖全局 g_quic_conn_cb 的调用顺序 —— echo/demo 与管道托管可并存多 listener）。
// 注：与 bi_quic_h3_listen 主体保持同步（fd/证书/router 线程创建一致）。
static int64_t quic_h3_listen_cb_impl(int port, const char* cert, const char* key,
                                      quic_conn_cb cb, void* ud) {
    if (!g_quic_init) {
        ngtcp2_crypto_quictls_init();
        OPENSSL_init_ssl(0, NULL);
        OSSL_PROVIDER_load(NULL, "default");
        g_quic_init = 1;
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { close(fd); return -1; }
    SSL_CTX* sctx = quic_make_server_ctx_cert(cert, key);
    if (!sctx) { close(fd); return -1; }
    quic_listener ql0; memset(&ql0, 0, sizeof(ql0));
    int64_t id = quic_alloc_listener(&ql0);
    if (id < 0) { SSL_CTX_free(sctx); close(fd); return -1; }
    quic_listener* ql = &g_qlis[id - 1];
    ql->fd = fd;
    ql->local = addr;
    ql->ssl_ctx = sctx;
    ql->used = 1;
    ql->conn_cb = cb;
    ql->conn_ud = ud;
    ql->router_stop = 0;
    ql->router_started = 1;
    if (pthread_create(&ql->router_thr, NULL, quic_srv_router, (void*)(intptr_t)id) != 0) {
        ql->router_started = 0;
        SSL_CTX_free(sctx);
        close(fd);
        memset(ql, 0, sizeof(*ql));
        return -1;
    }
    return id;
}

// raw：以显式连接回调启动 h3 server listener → listener id | -1
int64_t px_quic_raw_h3_listen_cb(int port, const char* cert, const char* key,
                                 px_quic_conn_cb cb, void* ud) {
    return quic_h3_listen_cb_impl(port, cert, key, cb, ud);
}

// M225：连接代次查询（0 = 无此连接）。同一 conn 号被复用后返回值必不同。
int64_t px_quic_raw_conn_epoch(int64_t conn) {
    quic_conn* qc = quic_get_conn(conn);
    return qc ? qc->epoch : 0;
}

// M225：注册连接回收钩子（runtime_h3.c 用；槽位释放前调用一次）
void px_quic_set_conn_recycle_cb(px_quic_conn_recycle_cb cb) {
    g_quic_recycle = cb;
}

// raw：连接对端地址 → "ip:port"（请求 remote 字段；连接已清理/无效 → 空串）
void px_quic_raw_peer_addr(int64_t conn, char* out, size_t n) {
    if (!out || n == 0) return;
    out[0] = 0;
    quic_conn* qc = quic_get_conn(conn);
    if (!qc) return;
    char ip[INET6_ADDRSTRLEN];
    int port = 0;
    if (qc->remote_sa.ss_family == AF_INET) {
        struct sockaddr_in* sin = (struct sockaddr_in*)&qc->remote_sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip))) port = ntohs(sin->sin_port);
        else return;
    } else if (qc->remote_sa.ss_family == AF_INET6) {
        struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&qc->remote_sa;
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof(ip))) port = ntohs(sin6->sin6_port);
        else return;
    } else {
        return;
    }
    snprintf(out, n, "%s:%d", ip, port);
}

// M53-S4：指定流对端 FIN 是否已到（1=是/0=否或流不存在）。H3 server 判请求无 body：
// HEADERS 帧后流即结束（无 DATA 帧）→ body 空合法。
int px_quic_raw_stream_fin(int64_t conn, int64_t sid) {
    quic_conn* qc = quic_get_conn(conn);
    if (!qc) return 0;
    quic_stream* s = quic_stream_slot(qc, sid);
    if (!s) return 0;
    return s->fin ? 1 : 0;
}


// 默认连接回调：QUIC 流 echo（S1 多连接路由验证用）
static void quic_srv_echo_cb(int64_t conn, void* ud) {
    (void)ud;
    int idle = 0;
    while (idle < 3) {
        quic_conn* qc = quic_get_conn(conn);
        if (!qc || qc->peer_closed) break;
        int64_t sid = px_quic_raw_poll(conn, 2000);
        if (sid < 0) { idle++; continue; }
        idle = 0;
        uint8_t buf[65536];
        for (;;) {
            int64_t got = px_quic_raw_recv_on(conn, sid, buf, (int)sizeof(buf), 100);
            if (got <= 0) break;
            // M54-S3：迁移后新路径需 path validation —— writev_stream 在验证完成前可能
            // 未发出（返回 <=0），pump 推进 PATH_CHALLENGE/RESPONSE 后重试补发。
            int64_t sent = -1;
            for (int t = 0; t < 10; t++) {
                sent = px_quic_raw_send_on(conn, sid, buf, (int)got, 0);
                if (sent > 0) break;
                px_quic_raw_poll(conn, 400);   // 驱动验证/流控推进
            }
        }
    }
}

// ---------- 服务端：quic_listen ----------
static int64_t quic_alloc_listener(quic_listener* ql) {
    for (int i = 0; i < QUIC_MAX_LISTENERS; i++) {
        if (!g_qlis[i].used) { g_qlis[i].used = 1; *ql = g_qlis[i]; return i + 1; }
    }
    return -1;
}

static quic_listener* quic_get_listener(int64_t id) {
    if (id <= 0 || id > QUIC_MAX_LISTENERS) return NULL;
    quic_listener* ql = &g_qlis[id - 1];
    return ql->used ? ql : NULL;
}

static LXValue bi_quic_listen(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_listen 需要 (port: int)");
    int port = (int)args[0].as.i;
    if (!g_quic_init) {
        ngtcp2_crypto_quictls_init();
        OPENSSL_init_ssl(0, NULL);
        OSSL_PROVIDER_load(NULL, "default");  // 静态链接需显式加载 provider
        g_quic_init = 1;
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return px_int(-1);
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        return px_int(-1);
    }
    SSL_CTX* sctx = quic_make_server_ctx();
    if (!sctx) { close(fd); return px_int(-1); }
    int64_t id = -1;
    for (int i = 0; i < QUIC_MAX_LISTENERS; i++) {
        if (!g_qlis[i].used) {
            g_qlis[i].used = 1;
            g_qlis[i].fd = fd;
            g_qlis[i].local = addr;
            g_qlis[i].ssl_ctx = sctx;
            id = i + 1;
            break;
        }
    }
    if (id < 0) { SSL_CTX_free(sctx); close(fd); }
    return px_int(id);
}

// ---------- 服务端：quic_accept ----------
static int64_t quic_alloc_conn(quic_conn* qc) {
    // M225：与「连接回收钩子」互斥 —— 否则"清理 h3 会话"可能落在新连接已占槽之后
    pthread_mutex_lock(&g_quic_srv_mu);
    for (int i = 0; i < QUIC_CONN_MAX; i++) {
        if (!g_qconns[i].used) {
            g_qconns[i].used = 1;
            g_qconns[i].epoch = ++g_quic_epoch;   // M225：新代次（不复用）
            int64_t live = 0;
            for (int k = 0; k < QUIC_CONN_MAX; k++) if (g_qconns[k].used) live++;
            if (live > g_quic_live_peak) g_quic_live_peak = live;
            g_quic_alloc_ok++;
            *qc = g_qconns[i];
            pthread_mutex_unlock(&g_quic_srv_mu);
            return i + 1;
        }
    }
    pthread_mutex_unlock(&g_quic_srv_mu);
    // M295（缺陷 508）：**修前此处静默返回 -1** —— 服务端丢弃 Initial 无任何痕迹。
    quic_alloc_fail_note();
    return -1;
}

static quic_conn* quic_get_conn(int64_t id) {
    if (id <= 0 || id > QUIC_CONN_MAX) return NULL;
    quic_conn* qc = &g_qconns[id - 1];
    return qc->used ? qc : NULL;
}

// M296（缺陷 512）：listener socket 上**不属于新连接**的包（前一条连接的 PMTUD probe /
//   ACK / CONNECTION_CLOSE、扫描流量、损坏包）必须**丢弃后继续等**，而不是让 accept 整体失败。
//   修前：accept#2 收到 A 的 PMTUD probe（1406B，抓包实证）⇒ read_pkt 返回 DROP_CONN ⇒
//   `return px_int(-1)` ⇒ 早已躺在同一 socket 缓冲区里的 B 的 Initial（1200B）**永远读不到**
//   ⇒ 一条 listener 实际只能接一条连接。
//   计数器 + 限流报告：这种丢弃**不许静默**（同 M295 的 quic_alloc_fail_note 口径）。
static uint64_t g_quic_accept_drop = 0;
static void quic_accept_drop_note(const char* stage, int rv, ssize_t rl) {
    g_quic_accept_drop++;
    static int verbose = -1;
    if (verbose < 0) { const char* e = getenv("PX_QUIC_VERBOSE"); verbose = (e && e[0] && e[0] != '0') ? 1 : 0; }
    if (verbose || g_quic_accept_drop == 1 || (g_quic_accept_drop % 1000) == 0) {
        fprintf(stderr, "[quic] accept: 丢弃无效包（%s rv=%d len=%zd）—— 累计 %llu 次；"
                        "继续等待下一个包\n",
                stage, rv, rl, (unsigned long long)g_quic_accept_drop);
    }
}

static LXValue bi_quic_accept(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("R1002: quic_accept 需要 (listener: int, timeout_ms: int)");
    quic_listener* ql = quic_get_listener(args[0].as.i);
    if (!ql) return px_int(-1);
    int timeout_ms = (int)args[1].as.i;

    // 等第一个 QUIC Initial 包
    struct pollfd pfd = { .fd = ql->fd, .events = POLLIN };
    int64_t deadline = quic_now() + (uint64_t)timeout_ms * NGTCP2_MILLISECONDS;
    for (;;) {
        int64_t remain = deadline - quic_now();
        if (remain <= 0) return px_int(-1);
        int ms = (int)((remain + NGTCP2_MILLISECONDS - 1) / NGTCP2_MILLISECONDS);
        int pr = px_io_poll(&pfd, 1, ms);   // M256（缺陷 456）：EINTR 曾判成「等不到 Initial 包」
        if (pr <= 0) return px_int(-1);
        uint8_t pkt[QUIC_PKT_BUF];
        struct sockaddr_storage from;
        socklen_t fromlen = sizeof(from);
        ssize_t rl = px_io_recvfrom(ql->fd, pkt, sizeof(pkt), 0,   // M256（缺陷 456）
                                    (struct sockaddr*)&from, &fromlen);
        if (rl <= 0) continue;
        // 解析 QUIC 长头包
        ngtcp2_version_cid vc;
        int rv = ngtcp2_pkt_decode_version_cid(&vc, pkt, (size_t)rl, 0);
        if (rv != 0) continue;  // 非 QUIC 包，忽略
        // 创建连接
        quic_conn qc0; memset(&qc0, 0, sizeof(qc0));
        int64_t cid = quic_alloc_conn(&qc0);
        if (cid < 0) return px_int(-1);
        quic_conn* qc = &g_qconns[cid - 1];
        qc->fd = ql->fd;
        qc->ssl_ctx = NULL;
        memcpy(&qc->remote_sa, &from, fromlen);
        memcpy(&qc->local_sa, &ql->local, sizeof(ql->local));
        qc->path.local.addr = (struct sockaddr*)&qc->local_sa;
        qc->path.local.addrlen = sizeof(struct sockaddr_in);
        qc->path.remote.addr = (struct sockaddr*)&qc->remote_sa;
        qc->path.remote.addrlen = fromlen;

        ngtcp2_cid cdcid;  // 对端连接 ID = 客户端 Initial 的 SCID
        cdcid.datalen = vc.scidlen;
        memcpy(cdcid.data, vc.scid, vc.scidlen);
        ngtcp2_cid scid;
        scid.datalen = QUIC_SCIDLEN;
        if (RAND_bytes(scid.data, QUIC_SCIDLEN) != 1) {
            qc->used = 0; return px_int(-1);
        }
        ngtcp2_settings settings;
        ngtcp2_settings_default(&settings);
        settings.initial_ts = quic_now();
        settings.log_printf = quic_log_cb;
        ngtcp2_transport_params params;
        ngtcp2_transport_params_default(&params);
        // M54-S4：listener 可配 client bidi 流上限（quic_set_max_client_streams）
        params.initial_max_streams_bidi = (ql->max_client_bidi > 0) ? ql->max_client_bidi : 16;
        params.initial_max_streams_uni = 3;
        params.initial_max_stream_data_bidi_local = 128 * 1024;
        params.initial_max_stream_data_bidi_remote = 128 * 1024;
        params.initial_max_stream_data_uni = 128 * 1024;
        params.initial_max_data = 1024 * 1024;
        // 服务端必须设置 original_dcid（= 客户端首包 DCID）
        params.original_dcid.datalen = vc.dcidlen;
        memcpy(params.original_dcid.data, vc.dcid, vc.dcidlen);
        params.original_dcid_present = 1;

        ngtcp2_callbacks cb;
        memset(&cb, 0, sizeof(cb));
        cb.recv_client_initial = ngtcp2_crypto_recv_client_initial_cb;
        cb.recv_crypto_data = ngtcp2_crypto_recv_crypto_data_cb;
        cb.handshake_completed = quic_handshake_completed_cb;
    cb.handshake_confirmed = quic_hs_confirmed_cb;
        cb.encrypt = ngtcp2_crypto_encrypt_cb;
        cb.decrypt = ngtcp2_crypto_decrypt_cb;
        cb.hp_mask = ngtcp2_crypto_hp_mask_cb;
        cb.recv_stream_data = quic_recv_stream_data_cb;
        cb.rand = quic_rand_cb;
        cb.get_new_connection_id = quic_get_new_cid_cb;
        cb.remove_connection_id = NULL;
        cb.update_key = ngtcp2_crypto_update_key_cb;
        cb.delete_crypto_aead_ctx = ngtcp2_crypto_delete_crypto_aead_ctx_cb;
        cb.delete_crypto_cipher_ctx = ngtcp2_crypto_delete_crypto_cipher_ctx_cb;
        cb.get_path_challenge_data = ngtcp2_crypto_get_path_challenge_data_cb;
        cb.version_negotiation = ngtcp2_crypto_version_negotiation_cb;
        cb.path_validation = quic_path_validation_cb;   // M297（缺陷 504）
        cb.begin_path_validation = quic_begin_path_validation_cb; // M297 诊断

        rv = ngtcp2_conn_server_new(&qc->conn, &cdcid, &scid, &qc->path,
                                    vc.version, &cb, &settings, &params, NULL, qc);
        if (rv != 0) {
            fprintf(stderr, "[quic] server_new rv=%d (%s)\n", rv, ngtcp2_strerror(rv));
            qc->used = 0; return px_int(-1);
        }
        // TLS（quictls：native handle 就是 SSL*；SSL_CTX 已在 listen 时 configure）
        SSL* ssl = SSL_new(ql->ssl_ctx);
        if (!ssl) { ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1); }
        qc->ssl = ssl;
        ngtcp2_conn_set_tls_native_handle(qc->conn, ssl);
        qc->conn_ref.get_conn = quic_get_conn_from_ref;
        qc->conn_ref.user_data = qc;
        SSL_set_app_data(ssl, &qc->conn_ref);
        SSL_set_accept_state(ssl);
        SSL_set_quic_early_data_enabled(ssl, 1);   // M54-S2：接受 0-RTT early data（官方 quictls 顺序：accept_state 后启用）
        // 喂第一个包（M297（缺陷 504）：用**实际来源**构造路径 —— 与 quic_pump 同口径）
        ngtcp2_conn_handle_expiry(qc->conn, quic_now());
        ngtcp2_path_storage sps0;
        {   // M297（缺陷 504）：与 quic_pump 同口径 —— local 取 ngtcp2 当前路径，remote 取真实来源
            const ngtcp2_path* cur0 = ngtcp2_conn_get_path(qc->conn);
            const ngtcp2_sockaddr* l0 = (cur0 && cur0->local.addr)
                                        ? (const ngtcp2_sockaddr*)cur0->local.addr
                                        : (const ngtcp2_sockaddr*)&qc->local_sa;
            ngtcp2_socklen l0len = (cur0 && cur0->local.addr)
                                   ? cur0->local.addrlen : quic_sa_len(&qc->local_sa);
            ngtcp2_path_storage_init(&sps0, l0, l0len, (const ngtcp2_sockaddr*)&from,
                                     (ngtcp2_socklen)quic_sa_len(&from), NULL);
        }
        rv = ngtcp2_conn_read_pkt(qc->conn, &sps0.path, NULL, pkt, (size_t)rl, quic_now());
        if (rv != 0) {
            // M296（缺陷 512）：这个包不是本连接的有效 Initial ⇒ **丢弃它、继续等下一个**
            //   （修前是 return -1 ⇒ 一个残包就把整次 accept 打掉）。
            quic_accept_drop_note("first read_pkt", rv, rl);
            SSL_free(ssl); ngtcp2_conn_del(qc->conn); qc->used = 0;
            continue;
        }
        // 泵到握手完成
        int pr2 = quic_pump(qc, timeout_ms, 0, -1);
        if (pr2 != 0) {
            // M296（缺陷 512）：握手泵不起来（对端没再发、或包不可用）⇒ 同样丢弃并继续等，
            //   而不是把整次 accept 判死。
            quic_accept_drop_note("handshake pump", pr2, rl);
            SSL_free(ssl); ngtcp2_conn_del(qc->conn); qc->used = 0;
            continue;
        }
        return px_int(cid);
    }
}

// M54-S1：hex 编解码（session 序列化传输用）前向声明
static int quic_unhex(const char* in, size_t len, unsigned char* out);

// ---------- 客户端：quic_connect / quic_connect_resume ----------
// M54-S1：TLS1.3 会话恢复 —— impl 共用主体；sess_hex 非空 → SSL_set_session 恢复
static LXValue quic_conn_connect_impl(LXValue* args, int nargs, const char* sess_hex) {
    if (nargs < 3 || args[0].type != PX_STR || args[1].type != PX_INT || args[2].type != PX_STR)
        px_error("R1002: quic_connect 需要 (ip: str, port: int, alpn: str)");
    const char* ip = args[0].as.obj->as.str.data;
    int port = (int)args[1].as.i;
    const char* alpn = args[2].as.obj->as.str.data;
    if (!g_quic_init) {
        ngtcp2_crypto_quictls_init();
        OPENSSL_init_ssl(0, NULL);
        OSSL_PROVIDER_load(NULL, "default");  // 静态链接需显式加载 provider
        g_quic_init = 1;
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { fprintf(stderr, "[quic] connect: socket fail\n"); return px_int(-1); }
    struct sockaddr_in raddr;
    memset(&raddr, 0, sizeof(raddr));
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip, &raddr.sin_addr) != 1) { fprintf(stderr, "[quic] connect: inet_pton fail\n"); close(fd); return px_int(-1); }
    if (connect(fd, (struct sockaddr*)&raddr, sizeof(raddr)) != 0) { fprintf(stderr, "[quic] connect: connect fail\n"); close(fd); return px_int(-1); }

    quic_conn qc0; memset(&qc0, 0, sizeof(qc0));
    int64_t cid = quic_alloc_conn(&qc0);
    if (cid < 0) { fprintf(stderr, "[quic] connect: conn table full\n"); close(fd); return px_int(-1); }
    quic_conn* qc = &g_qconns[cid - 1];
    qc->fd = fd;
    memset(&qc->local_sa, 0, sizeof(qc->local_sa));
    socklen_t ll = sizeof(qc->local_sa);
    getsockname(fd, (struct sockaddr*)&qc->local_sa, &ll);
    memcpy(&qc->remote_sa, &raddr, sizeof(raddr));
    qc->path.local.addr = (struct sockaddr*)&qc->local_sa;
    qc->path.local.addrlen = ll;
    qc->path.remote.addr = (struct sockaddr*)&qc->remote_sa;
    qc->path.remote.addrlen = sizeof(raddr);

    ngtcp2_cid dcid, scid;
    dcid.datalen = NGTCP2_MIN_INITIAL_DCIDLEN;
    if (RAND_bytes(dcid.data, dcid.datalen) != 1) { fprintf(stderr, "[quic] connect: RAND dcid fail\n"); close(fd); qc->used = 0; return px_int(-1); }
    scid.datalen = QUIC_SCIDLEN;
    if (RAND_bytes(scid.data, QUIC_SCIDLEN) != 1) { fprintf(stderr, "[quic] connect: RAND scid fail\n"); close(fd); qc->used = 0; return px_int(-1); }

    ngtcp2_settings settings;
    ngtcp2_settings_default(&settings);
    settings.initial_ts = quic_now();
    settings.log_printf = quic_log_cb;
    ngtcp2_transport_params params;
    ngtcp2_transport_params_default(&params);
    params.initial_max_streams_bidi = 16;
    params.initial_max_streams_uni = 3;
    params.initial_max_stream_data_bidi_local = 128 * 1024;
    params.initial_max_stream_data_bidi_remote = 128 * 1024;
    params.initial_max_stream_data_uni = 128 * 1024;
    params.initial_max_data = 1024 * 1024;

    ngtcp2_callbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.client_initial = ngtcp2_crypto_client_initial_cb;
    cb.recv_crypto_data = ngtcp2_crypto_recv_crypto_data_cb;
    cb.handshake_completed = quic_handshake_completed_cb;
    cb.handshake_confirmed = quic_hs_confirmed_cb;
    cb.encrypt = ngtcp2_crypto_encrypt_cb;
    cb.decrypt = ngtcp2_crypto_decrypt_cb;
    cb.hp_mask = ngtcp2_crypto_hp_mask_cb;
    cb.recv_retry = ngtcp2_crypto_recv_retry_cb;
    cb.recv_stream_data = quic_recv_stream_data_cb;
    cb.rand = quic_rand_cb;
    cb.get_new_connection_id = quic_get_new_cid_cb;
    cb.remove_connection_id = NULL;
    cb.update_key = ngtcp2_crypto_update_key_cb;
    cb.delete_crypto_aead_ctx = ngtcp2_crypto_delete_crypto_aead_ctx_cb;
    cb.delete_crypto_cipher_ctx = ngtcp2_crypto_delete_crypto_cipher_ctx_cb;
    cb.get_path_challenge_data = ngtcp2_crypto_get_path_challenge_data_cb;
    cb.version_negotiation = ngtcp2_crypto_version_negotiation_cb;
    cb.path_validation = quic_path_validation_cb;   // M297（缺陷 504）
    cb.begin_path_validation = quic_begin_path_validation_cb; // M297 诊断

    int rv = ngtcp2_conn_client_new(&qc->conn, &dcid, &scid, &qc->path,
                                    NGTCP2_PROTO_VER_V1, &cb, &settings, &params,
                                    NULL, qc);
    if (rv != 0) {
        fprintf(stderr, "[quic] client_new rv=%d (%s)\n", rv, ngtcp2_strerror(rv));
        close(fd); qc->used = 0; return px_int(-1);
    }
    SSL_CTX* cctx = quic_make_client_ctx();
    if (!cctx) { fprintf(stderr, "[quic] connect: client ctx fail\n"); close(fd); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1); }
    qc->ssl_ctx = cctx;
    SSL* ssl = SSL_new(cctx);
    if (!ssl) { fprintf(stderr, "[quic] connect: SSL_new fail\n"); SSL_CTX_free(cctx); close(fd); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1); }
    qc->ssl = ssl;
    // ALPN
    size_t alpnlen = strlen(alpn);
    if (alpnlen > 0 && alpnlen <= 255) {
        unsigned char proto[256];
        proto[0] = (unsigned char)alpnlen;
        memcpy(proto + 1, alpn, alpnlen);
        SSL_set_alpn_protos(ssl, proto, (unsigned)alpnlen + 1);
    }
    // M54-S1：TLS 会话恢复 —— 握手开始前恢复保存的 session（1-RTT resumption）
    if (sess_hex && *sess_hex) {
        size_t sl = strlen(sess_hex);
        if (sl % 2 == 0 && sl > 0 && sl <= 262144) {
            unsigned char* der = (unsigned char*)malloc(sl / 2);
            int dlen = der ? quic_unhex(sess_hex, sl, der) : -1;
            if (dlen > 0) {
                const unsigned char* p = der;
                SSL_SESSION* sess = d2i_SSL_SESSION(NULL, &p, (long)dlen);
                if (sess) {
                    if (SSL_set_session(ssl, sess) != 1)
                        fprintf(stderr, "[quic] resume: SSL_set_session FAIL\n");
                    SSL_SESSION_free(sess);
                } else {
                    fprintf(stderr, "[quic] resume: d2i_SSL_SESSION FAIL\n");
                }
            }
            if (der) free(der);
        }
    }
    // quictls：SSL_CTX 已在 quic_make_client_ctx 时 configure，native handle = SSL*
    ngtcp2_conn_set_tls_native_handle(qc->conn, ssl);
    qc->conn_ref.get_conn = quic_get_conn_from_ref;
    qc->conn_ref.user_data = qc;
    SSL_set_app_data(ssl, &qc->conn_ref);
    SSL_set_connect_state(ssl);
    // 泵到握手完成（open_bidi_stream 需在对端参数已知后，即握手完成后）
    int pr = quic_pump(qc, 10000, 0, -1);
    if (pr != 0) {
        fprintf(stderr, "[quic] connect pump rv=%d\n", pr);
        SSL_free(ssl); SSL_CTX_free(cctx); close(fd); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1);
    }
    /* M50：打开第一条双向流（client bidi id 从 0 开始，后续 quic_open_stream 每次 +4）*/
    int64_t sid0 = -1;
    rv = ngtcp2_conn_open_bidi_stream(qc->conn, &sid0, NULL);
    if (rv != 0 || sid0 < 0) {
        fprintf(stderr, "[quic] connect: open_bidi_stream rv=%d (%s)\n", rv, ngtcp2_strerror(rv));
        SSL_free(ssl); SSL_CTX_free(cctx); close(fd); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1);
    }
    if (!quic_stream_new(qc, sid0, 0)) { /* 槽满，理论上不可能（首条）*/ }
    return px_int(cid);
}
// ---------- M54-S1：TLS 会话保存/恢复 语言层 API ----------
static int quic_hexv(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
static void quic_hex(const unsigned char* in, size_t len, char* out) {
    static const char* d = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out[i * 2] = d[in[i] >> 4];
        out[i * 2 + 1] = d[in[i] & 0xf];
    }
    out[len * 2] = 0;
}
static int quic_unhex(const char* in, size_t len, unsigned char* out) {
    if (len % 2) return -1;
    for (size_t i = 0; i < len; i += 2) {
        int hi = quic_hexv(in[i]);
        int lo = quic_hexv(in[i + 1]);
        if (hi < 0 || lo < 0) return -1;
        out[i / 2] = (unsigned char)((hi << 4) | lo);
    }
    return (int)(len / 2);
}

static LXValue bi_quic_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    // M198（缺陷 234）：quic_connect 是 3 或 4 参（第 4 参 = 可选 session hex）——
    //   修前 `nargs >= 4 ? 4 : 3` 会把**第 5 个起**的实参静默丢掉。
    if (nargs != 3 && nargs != 4)
        px_error("R1002: quic_connect 需要 (ip: str, port: int, alpn: str[, session: str]) 参数");
    // M199（缺陷 237）：非 str 的 session 此前静默当 NULL（= 静默放弃会话恢复）。
    //   null 仍表示「不恢复」（可选实参约定），只有**别**的类型才响亮。
    if (nargs >= 4 && args[3].type != PX_STR && args[3].type != PX_NULL)
        px_error("R1002: quic_connect 的 session 需要字符串，实际是 %s", px_type_name(args[3]));
    return quic_conn_connect_impl(args, nargs >= 4 ? 4 : 3, nargs >= 4 && args[3].type == PX_STR
                                  ? args[3].as.obj->as.str.data : NULL);
}

static LXValue bi_quic_connect_resume(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 4 || args[0].type != PX_STR || args[1].type != PX_INT ||
        args[2].type != PX_STR || args[3].type != PX_STR)
        px_error("R1002: quic_connect_resume 需要 (ip: str, port: int, alpn: str, session: str)");
    return quic_conn_connect_impl(args, 4, args[3].as.obj->as.str.data);
}

// quic_session_save(conn) -> str：握手完成后导出 TLS session（含 NewSessionTicket）
// 的 hex 序列化（DER）。内部先泵至多 ~3s 等 server 的 NewSessionTicket 到达，
// 保证返回的 session 可用于后续恢复。
static LXValue bi_quic_session_save(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_session_save 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->ssl) return px_str("");
    int waited = 0;
    for (;;) {
        SSL_SESSION* s0 = SSL_get0_session(qc->ssl);
        if (s0 && SSL_SESSION_has_ticket(s0)) break;
        if (qc->peer_closed || waited >= 3000) break;
        int pr = quic_pump(qc, 500, 1, -1);
        waited += 500;
        if (pr == -2) break;   // 错误；超时(-1)/无数据(0) 继续等 ticket
    }
    SSL_SESSION* s = SSL_get1_session(qc->ssl);
    if (!s) return px_str("");
    unsigned char* der = NULL;
    int dlen = i2d_SSL_SESSION(s, &der);
    SSL_SESSION_free(s);
    if (dlen <= 0 || !der) { if (der) OPENSSL_free(der); return px_str(""); }
    char* hex = (char*)malloc((size_t)dlen * 2 + 1);
    quic_hex(der, (size_t)dlen, hex);
    OPENSSL_free(der);
    LXValue r = px_str(hex);
    free(hex);
    return r;
}

// quic_conn_resumed(conn) -> bool：本连接是否为会话恢复（1-RTT）握手
static LXValue bi_quic_conn_resumed(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_conn_resumed 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->ssl) return px_bool(false);
    return px_bool(SSL_session_reused(qc->ssl) == 1);
}

// ==================== M54-S2：0-RTT early data ====================
// 语言 API：
//   quic_0rtt_save(conn) -> str          "TLS-session-hex|0rtt-tp-hex"（session 段 = M54-S1
//                                        quic_session_save 的 DER hex；tp 段 = 0-RTT transport
//                                        params 编码，RFC 9000 要求随 ticket 一并记忆）。0-RTT
//                                        tp 不可编码时退化为纯 session（1-RTT 可用）。
//   quic_connect_0rtt(ip,port,alpn,s) -> conn   会话恢复 + 0-RTT early data：握手完成前即可
//                                        quic_send（数据随 0-RTT 包提前到达 server）。tp 缺失/
//                                        解码失败 → 自动降级普通 resume（等握手完成）。
//   quic_0rtt_rejected(conn) -> bool     early data 被服务端拒绝（需重发）。
//   quic_conn_handshake_done(conn)->bool 握手是否已完成（断言 0-RTT：connect_0rtt 返回时未完成）。
// 0-RTT 流程（对齐 ngtcp2 官方 client.cc / tls_client_session_quictls.cc）：
//   1) session 段 d2i → SSL_set_session（恢复）；SSL_SESSION_get_max_early_data>0 才可 0-RTT
//   2) SSL_set_quic_early_data_enabled(ssl,1) + tp 段 decode_and_set_0rtt_transport_params
//   3) 首次 ngtcp2_conn_write_pkt 发出 Initial/ClientHello（含 early_data 扩展，装 0-RTT key）
//   4) open_bidi_stream 后立即返回 —— 不等握手完成；应用随即 quic_send → 0-RTT 包提前发出
// 服务端：SSL_CTX_set_max_early_data（ctx）+ SSL_set_quic_early_data_enabled（连接）→
// ticket 带 max_early_data；0-RTT 包由 quictls 自动解密 → recv_stream_data_cb 提前缓冲。
static LXValue quic_conn_connect_0rtt_impl(LXValue* args, int nargs) {
    if (nargs < 4 || args[0].type != PX_STR || args[1].type != PX_INT ||
        args[2].type != PX_STR || args[3].type != PX_STR)
        px_error("R1002: quic_connect_0rtt 需要 (ip: str, port: int, alpn: str, session0rtt: str)");
    const char* ip = args[0].as.obj->as.str.data;
    int port = (int)args[1].as.i;
    const char* alpn = args[2].as.obj->as.str.data;
    const char* s0 = args[3].as.obj->as.str.data;
    // 拆分 "session|tp"
    const char* sep = strchr(s0, '|');
    const char* sess_hex = s0;
    const char* tp_hex = NULL;
    size_t sess_len = strlen(s0);
    if (sep) { sess_len = (size_t)(sep - s0); tp_hex = sep + 1; }
    if (!g_quic_init) {
        ngtcp2_crypto_quictls_init();
        OPENSSL_init_ssl(0, NULL);
        OSSL_PROVIDER_load(NULL, "default");
        g_quic_init = 1;
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { fprintf(stderr, "[quic] 0rtt: socket fail\n"); return px_int(-1); }
    struct sockaddr_in raddr;
    memset(&raddr, 0, sizeof(raddr));
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip, &raddr.sin_addr) != 1) { close(fd); return px_int(-1); }
    if (connect(fd, (struct sockaddr*)&raddr, sizeof(raddr)) != 0) { close(fd); return px_int(-1); }
    quic_conn qc0; memset(&qc0, 0, sizeof(qc0));
    int64_t cid = quic_alloc_conn(&qc0);
    if (cid < 0) { close(fd); return px_int(-1); }
    quic_conn* qc = &g_qconns[cid - 1];
    qc->fd = fd;
    memset(&qc->local_sa, 0, sizeof(qc->local_sa));
    socklen_t ll = sizeof(qc->local_sa);
    getsockname(fd, (struct sockaddr*)&qc->local_sa, &ll);
    memcpy(&qc->remote_sa, &raddr, sizeof(raddr));
    qc->path.local.addr = (struct sockaddr*)&qc->local_sa;
    qc->path.local.addrlen = ll;
    qc->path.remote.addr = (struct sockaddr*)&qc->remote_sa;
    qc->path.remote.addrlen = sizeof(raddr);
    ngtcp2_cid dcid, scid;
    dcid.datalen = NGTCP2_MIN_INITIAL_DCIDLEN;
    if (RAND_bytes(dcid.data, dcid.datalen) != 1) { close(fd); qc->used = 0; return px_int(-1); }
    scid.datalen = QUIC_SCIDLEN;
    if (RAND_bytes(scid.data, QUIC_SCIDLEN) != 1) { close(fd); qc->used = 0; return px_int(-1); }
    ngtcp2_settings settings;
    ngtcp2_settings_default(&settings);
    settings.initial_ts = quic_now();
    settings.log_printf = quic_log_cb;
    ngtcp2_transport_params params;
    ngtcp2_transport_params_default(&params);
    params.initial_max_streams_bidi = 16;
    params.initial_max_streams_uni = 3;
    params.initial_max_stream_data_bidi_local = 128 * 1024;
    params.initial_max_stream_data_bidi_remote = 128 * 1024;
    params.initial_max_stream_data_uni = 128 * 1024;
    params.initial_max_data = 1024 * 1024;
    ngtcp2_callbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.client_initial = ngtcp2_crypto_client_initial_cb;
    cb.recv_crypto_data = ngtcp2_crypto_recv_crypto_data_cb;
    cb.handshake_completed = quic_handshake_completed_cb;
    cb.handshake_confirmed = quic_hs_confirmed_cb;
    cb.encrypt = ngtcp2_crypto_encrypt_cb;
    cb.decrypt = ngtcp2_crypto_decrypt_cb;
    cb.hp_mask = ngtcp2_crypto_hp_mask_cb;
    cb.recv_retry = ngtcp2_crypto_recv_retry_cb;
    cb.recv_stream_data = quic_recv_stream_data_cb;
    cb.rand = quic_rand_cb;
    cb.get_new_connection_id = quic_get_new_cid_cb;
    cb.remove_connection_id = NULL;
    cb.update_key = ngtcp2_crypto_update_key_cb;
    cb.delete_crypto_aead_ctx = ngtcp2_crypto_delete_crypto_aead_ctx_cb;
    cb.delete_crypto_cipher_ctx = ngtcp2_crypto_delete_crypto_cipher_ctx_cb;
    cb.get_path_challenge_data = ngtcp2_crypto_get_path_challenge_data_cb;
    cb.version_negotiation = ngtcp2_crypto_version_negotiation_cb;
    cb.path_validation = quic_path_validation_cb;   // M297（缺陷 504）
    cb.begin_path_validation = quic_begin_path_validation_cb; // M297 诊断
    int rv = ngtcp2_conn_client_new(&qc->conn, &dcid, &scid, &qc->path,
                                    NGTCP2_PROTO_VER_V1, &cb, &settings, &params,
                                    NULL, qc);
    if (rv != 0) { close(fd); qc->used = 0; return px_int(-1); }
    SSL_CTX* cctx = quic_make_client_ctx();
    if (!cctx) { close(fd); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1); }
    qc->ssl_ctx = cctx;
    SSL* ssl = SSL_new(cctx);
    if (!ssl) { SSL_CTX_free(cctx); close(fd); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1); }
    qc->ssl = ssl;
    size_t alpnlen = strlen(alpn);
    if (alpnlen > 0 && alpnlen <= 255) {
        unsigned char proto[256];
        proto[0] = (unsigned char)alpnlen;
        memcpy(proto + 1, alpn, alpnlen);
        SSL_set_alpn_protos(ssl, proto, (unsigned)alpnlen + 1);
    }
    // session 段恢复（同 quic_connect_resume）
    int have_sess = 0;
    if (sess_len > 0 && sess_len % 2 == 0 && sess_len <= 262144) {
        unsigned char* der = (unsigned char*)malloc(sess_len / 2);
        int dlen = der ? quic_unhex(s0, sess_len, der) : -1;
        if (dlen > 0) {
            const unsigned char* p = der;
            SSL_SESSION* sess = d2i_SSL_SESSION(NULL, &p, (long)dlen);
            if (sess) {
                if (SSL_set_session(ssl, sess) == 1) have_sess = 1;
                else fprintf(stderr, "[quic] 0rtt: SSL_set_session FAIL\n");
                SSL_SESSION_free(sess);
            }
        }
        if (der) free(der);
    }
    // tp 段 → 0-RTT transport params（decode 必须在首次 write_pkt 前，供 0-RTT 流控/开流）
    int early_ok = 0;
    if (have_sess && tp_hex && *tp_hex) {
        size_t tl = strlen(tp_hex);
        if (tl % 2 == 0 && tl > 0 && tl <= 4096) {
            unsigned char* tpb = (unsigned char*)malloc(tl / 2);
            int tlen = tpb ? quic_unhex(tp_hex, tl, tpb) : -1;
            if (tlen > 0) {
                int drv = ngtcp2_conn_decode_and_set_0rtt_transport_params(
                              qc->conn, tpb, (size_t)tlen);
                if (drv == 0) {
                    SSL_set_quic_early_data_enabled(ssl, 1);   // 0-RTT 触发开关
                    early_ok = 1;
                } else {
                    fprintf(stderr, "[quic] 0rtt: decode tp rv=%d (%s) → 降级 1-RTT\n",
                            drv, ngtcp2_strerror(drv));
                }
            }
            if (tpb) free(tpb);
        }
    }
    ngtcp2_conn_set_tls_native_handle(qc->conn, ssl);
    qc->conn_ref.get_conn = quic_get_conn_from_ref;
    qc->conn_ref.user_data = qc;
    SSL_set_app_data(ssl, &qc->conn_ref);
    SSL_set_connect_state(ssl);
    if (early_ok) {
        // 首次 write_pkt：触发 client_initial_cb → ClientHello（early_data 扩展）+ 装 0-RTT key
        uint8_t out[QUIC_PKT_BUF];
        ngtcp2_path_storage ps;                 // M297（缺陷 504）：发送目标由 ngtcp2 给出
        ngtcp2_path_storage_zero(&ps);
        ngtcp2_ssize n = ngtcp2_conn_write_pkt(qc->conn, &ps.path, NULL,
                                               out, sizeof(out), quic_now());
        if (n > 0) {
            quic_send_to_path(qc, out, (size_t)n, &ps.path);
        } else if (n < 0 && n != NGTCP2_ERR_WRITE_MORE) {
            fprintf(stderr, "[quic] 0rtt: first write_pkt rv=%zd (%s)\n", n, ngtcp2_strerror((int)n));
            SSL_free(ssl); SSL_CTX_free(cctx); close(fd);
            ngtcp2_conn_del(qc->conn); qc->used = 0;
            return px_int(-1);
        }
        // 0-RTT 打开首条双向流（decode tp 后即可，无需握手完成）
        int64_t sid0 = -1;
        rv = ngtcp2_conn_open_bidi_stream(qc->conn, &sid0, NULL);
        if (rv != 0 || sid0 < 0) {
            fprintf(stderr, "[quic] 0rtt: open_bidi_stream rv=%d (%s)\n", rv, ngtcp2_strerror(rv));
            SSL_free(ssl); SSL_CTX_free(cctx); close(fd);
            ngtcp2_conn_del(qc->conn); qc->used = 0;
            return px_int(-1);
        }
        quic_stream_new(qc, sid0, 0);
        qc->early_sent = 1;   // 语言层可断言 quic_conn_handshake_done==false 且能立即 quic_send
        return px_int(cid);
    }
    // 降级：0-RTT 不可用 → 普通 resume（泵到握手完成再开流）
    int pr = quic_pump(qc, 10000, 0, -1);
    if (pr != 0) {
        SSL_free(ssl); SSL_CTX_free(cctx); close(fd);
        ngtcp2_conn_del(qc->conn); qc->used = 0;
        return px_int(-1);
    }
    int64_t sid0 = -1;
    rv = ngtcp2_conn_open_bidi_stream(qc->conn, &sid0, NULL);
    if (rv != 0 || sid0 < 0) {
        SSL_free(ssl); SSL_CTX_free(cctx); close(fd);
        ngtcp2_conn_del(qc->conn); qc->used = 0;
        return px_int(-1);
    }
    quic_stream_new(qc, sid0, 0);
    return px_int(cid);
}

// quic_0rtt_save(conn) -> str：同 quic_session_save 等 NewSessionTicket，另拼 0-RTT
// transport params（握手完成且 0-RTT 可用时 encode；失败仅输出 session 段）。
static LXValue bi_quic_0rtt_save(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_0rtt_save 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->ssl) return px_str("");
    int waited = 0;
    for (;;) {
        SSL_SESSION* s0 = SSL_get0_session(qc->ssl);
        if (s0 && SSL_SESSION_has_ticket(s0)) break;
        if (qc->peer_closed || waited >= 3000) break;
        int pr = quic_pump(qc, 500, 1, -1);
        waited += 500;
        if (pr == -2) break;
    }
    SSL_SESSION* s = SSL_get1_session(qc->ssl);
    if (!s) return px_str("");
    unsigned char* der = NULL;
    int dlen = i2d_SSL_SESSION(s, &der);
    SSL_SESSION_free(s);
    if (dlen <= 0 || !der) { if (der) OPENSSL_free(der); return px_str(""); }
    char* hex = (char*)malloc((size_t)dlen * 2 + 1);
    quic_hex(der, (size_t)dlen, hex);
    OPENSSL_free(der);
    // 0-RTT transport params（encode 本端上一连接协商结果；须在握手完成后）
    uint8_t tpb[512];
    ngtcp2_ssize tplen = ngtcp2_conn_encode_0rtt_transport_params2(qc->conn, tpb, sizeof(tpb));
    char* joined = NULL;
    if (tplen > 0) {
        char* tphex = (char*)malloc((size_t)tplen * 2 + 1);
        quic_hex(tpb, (size_t)tplen, tphex);
        size_t hl = strlen(hex);
        joined = (char*)malloc(hl + (size_t)tplen * 2 + 2);
        memcpy(joined, hex, hl);
        joined[hl] = '|';
        memcpy(joined + hl + 1, tphex, (size_t)tplen * 2 + 1);
        free(tphex);
    }
    LXValue r = px_str(joined ? joined : hex);
    if (joined) free(joined);
    free(hex);
    return r;
}

// quic_connect_0rtt(ip, port, alpn, session0rtt) -> int
static LXValue bi_quic_connect_0rtt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    // M198（缺陷 234）：固定 4 参（修前 `nargs < 4` 无上界 ⇒ 多余实参静默丢掉）。
    if (nargs != 4)
        px_error("R1002: quic_connect_0rtt 需要 (ip: str, port: int, alpn: str, session0rtt: str) 参数");
    return quic_conn_connect_0rtt_impl(args, nargs);
}

// quic_0rtt_rejected(conn) -> bool：本连接 early data 被服务端拒绝
static LXValue bi_quic_0rtt_rejected(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_0rtt_rejected 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->ssl) return px_bool(false);
    return px_bool(qc->early_rejected ? true : false);
}

// quic_conn_handshake_done(conn) -> bool：握手是否已完成
static LXValue bi_quic_conn_handshake_done(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_conn_handshake_done 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_bool(false);
    return px_bool(qc->handshake_done ? true : false);
}

// ==================== M54-S3：连接迁移（client 换源 + server path 跟随） ====================
// 语言 API：
//   quic_migrate(conn, local_ip:str, local_port:int) -> bool
//       连接迁移（client 换源）：新建 UDP fd 绑定新源端口 → 后续收发走新 fd（源端口
//       变化 = NAT rebinding 语义）。服务器端收包路由按 DCID 定位同 conn（M53 单 fd 收包
//       router + 逐包 from）→ ngtcp2 server 对"新源地址的包"自动做 path validation
//       （PATH_CHALLENGE→PATH_RESPONSE），pump 更新 remote_sa → 回包发到新源 —— 同一
//       QUIC 连接续传，无重新握手。
//       注：不调 ngtcp2_conn_initiate_immediate_migration —— ngtcp2 1.25.90 该路径触发
//       conn_reset_congestion_state→cc.reset→init_pacing_rate 断言 cstat->cwnd 崩溃（迁移
//       新路径 pmtud 未初始化导致 cwnd=0）。MVP 以被动换源（客户端直接换 fd，ngtcp2
//       视角路径不变、不做客户端主动迁移前置校验）完成迁移：客户端换源后照常发包，
//       服务器完成新路径验证并跟随 —— 行为与 NAT rebinding 一致，RFC 9000 §9 允许。
//       local_ip "" = INADDR_ANY；local_port 0 = 内核自动选。
//   quic_conn_path(conn) -> str   当前对端地址 "ip:port"（迁移后服务器视角应变为新源）
//   quic_conn_local(conn) -> str  当前本地地址 "ip:port"（迁移后源端口应变化，断言用）
static LXValue bi_quic_migrate(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_INT || args[1].type != PX_STR || args[2].type != PX_INT)
        px_error("R1002: quic_migrate 需要 (conn: int, local_ip: str, local_port: int)");
    int64_t conn = args[0].as.i;
    const char* lip = args[1].as.obj->as.str.data;
    int lport = (int)args[2].as.i;
    quic_conn* qc = quic_get_conn(conn);
    if (!qc || !qc->conn || qc->owner_listener > 0) return px_bool(false);  // 仅 client 侧可迁移
    // M54-S3：切换前先泵 ~300ms 冲刷旧路径在途包（ACK/控制帧），否则 close 旧 fd 后
    // 迟到的旧源包会把服务器 remote_sa 回切到已关闭的旧地址（echo 回包发往死地址）。
    quic_pump(qc, 300, 1, -1);
    // 新建 UDP fd：bind 新源地址 → connect 原对端
    int nfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (nfd < 0) return px_bool(false);
    int one = 1;
    setsockopt(nfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in laddr;
    memset(&laddr, 0, sizeof(laddr));
    laddr.sin_family = AF_INET;
    laddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (lip && *lip) inet_pton(AF_INET, lip, &laddr.sin_addr);
    laddr.sin_port = htons((uint16_t)lport);
    if (bind(nfd, (struct sockaddr*)&laddr, sizeof(laddr)) != 0) {
        fprintf(stderr, "[quic] migrate: bind %s:%d fail\n", lip ? lip : "", lport);
        close(nfd);
        return px_bool(false);
    }
    if (px_io_connect(nfd, (struct sockaddr*)&qc->remote_sa, sizeof(qc->remote_sa)) != 0) {   // M256（缺陷 456）
        fprintf(stderr, "[quic] migrate: connect fail\n");
        close(nfd);
        return px_bool(false);
    }
    // 切换 fd / 本地地址 / 路径（后续收发走新 fd；server 按 DCID 续传 + 新源验证跟随）
    memset(&qc->local_sa, 0, sizeof(qc->local_sa));
    socklen_t ll = sizeof(qc->local_sa);
    getsockname(nfd, (struct sockaddr*)&qc->local_sa, &ll);
    qc->path.local.addr = (struct sockaddr*)&qc->local_sa;
    qc->path.local.addrlen = ll;
    close(qc->fd);
    qc->fd = nfd;
    if (quic_verbose_on()) {
        const ngtcp2_path* cp = ngtcp2_conn_get_path(qc->conn);
        char cl[128], cr[128], ql[128];
        if (cp) { quic_dbg_addr1(&cp->local, cl, sizeof(cl)); quic_dbg_addr1(&cp->remote, cr, sizeof(cr)); }
        else { snprintf(cl, sizeof(cl), "(null)"); snprintf(cr, sizeof(cr), "(null)"); }
        { ngtcp2_addr la; la.addr = (struct sockaddr*)&qc->local_sa; la.addrlen = quic_sa_len(&qc->local_sa);
          quic_dbg_addr1(&la, ql, sizeof(ql)); }
        fprintf(stderr, "[quic] migrate: qc->local_sa=%s ngtcp2 current=(%s -> %s)\n", ql, cl, cr);
    }
    return px_bool(true);
}

// 地址 → "ip:port"（复用 px_quic_raw_peer_addr 逻辑，支持任意 sockaddr）
static void quic_addr_to_str(const struct sockaddr_storage* sa, char* out, size_t n) {
    out[0] = 0;
    char ip[INET6_ADDRSTRLEN];
    int port = 0;
    if (sa->ss_family == AF_INET) {
        struct sockaddr_in* sin = (struct sockaddr_in*)sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip))) port = ntohs(sin->sin_port);
        else return;
    } else if (sa->ss_family == AF_INET6) {
        struct sockaddr_in6* sin6 = (struct sockaddr_in6*)sa;
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof(ip))) port = ntohs(sin6->sin6_port);
        else return;
    } else {
        return;
    }
    snprintf(out, n, "%s:%d", ip, port);
}

// quic_conn_path(conn) -> str：当前对端地址
static LXValue bi_quic_conn_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_conn_path 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc) return px_str("");
    char buf[128];
    quic_addr_to_str(&qc->remote_sa, buf, sizeof(buf));
    return px_str(buf);
}

// quic_conn_local(conn) -> str：当前本地地址（client 迁移后源端口变化）
static LXValue bi_quic_conn_local(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_conn_local 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc) return px_str("");
    char buf[128];
    quic_addr_to_str(&qc->local_sa, buf, sizeof(buf));
    return px_str(buf);
}

// ==================== M54-S4：BLOCKED_STREAMS 流控协商 ====================
// 语言 API：
//   quic_set_max_client_streams(listener, n_bidi) -> bool
//       设置 listener 每个新连接「允许对端(client)开的 bidi 流数」（0 = 默认 16）。
//       经 transport params 下发（ngtcp2 server_new 时应用）。
//   quic_extend_max_streams(listener, add_bidi) -> bool
//       放行：对 listener 当前所有活跃连接调 ngtcp2_conn_extend_max_streams_bidi
//       → pump write_pkt 带出 MAX_STREAMS 帧 → 对端阻塞的 quic_open_stream 解禁。
//   quic_streams_left(conn) -> int   本端还可开的 bidi 流数（对端当前允许配额剩余）
//   quic_open_stream 达对端上限返回 -206（NGTCP2_ERR_STREAM_ID_BLOCKED，RFC 9000 §19.11
//   语义：客户端发 STREAMS_BLOCKED 告知对端被阻塞，对端可 MAX_STREAMS 放行）。
static LXValue bi_quic_set_max_client_streams(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("R1002: quic_set_max_client_streams 需要 (listener: int, n_bidi: int)");
    quic_listener* ql = quic_get_listener(args[0].as.i);
    if (!ql) return px_bool(false);
    ql->max_client_bidi = (int)args[1].as.i;
    return px_bool(true);
}

static LXValue bi_quic_extend_max_streams(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("R1002: quic_extend_max_streams 需要 (listener: int, add_bidi: int)");
    int64_t lid = args[0].as.i;
    int add = (int)args[1].as.i;
    quic_listener* ql = quic_get_listener(lid);
    if (!ql || add <= 0) return px_bool(false);
    int done = 0;
    for (int i = 0; i < QUIC_CONN_MAX; i++) {
        quic_conn* qc = &g_qconns[i];
        // 匹配本 listener 的连接：托管 conn（owner_listener）或 demo accept conn
        // （与 listener 共享 fd —— qc->fd == ql->fd）
        if (qc->used && qc->conn && (qc->owner_listener == lid || qc->fd == ql->fd)) {
            ngtcp2_conn_extend_max_streams_bidi(qc->conn, (uint64_t)add);
            done++;
        }
    }
    return px_bool(done > 0);
}

static LXValue bi_quic_streams_left(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_streams_left 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    return px_int((int)ngtcp2_conn_get_streams_bidi_left2(qc->conn));
}

// ============================================================
// 发送/接收（M50：per-stream；旧 quic_send/quic_recv = 默认流兼容）
// ============================================================

// 写指定流：尽量写完，内部处理流控。返回写入字节数 / -1。
static int64_t quic_write_stream_bytes(quic_conn* qc, int64_t sid,
                                       const uint8_t* data, size_t len) {
    if (!qc || !qc->conn) return -1;
    if (!quic_stream_slot(qc, sid)) return -1;      // 流不存在（未 open/未见对端）
    size_t woff = 0;
    int64_t total = 0;
    int spins = 0;
    while (woff < len && spins < 16) {
        uint8_t out[QUIC_PKT_BUF];
        ngtcp2_vec v = { (uint8_t*)data + woff, len - woff };
        ngtcp2_ssize ndone = 0;
        ngtcp2_path_storage ps;                 // M297（缺陷 504）：发送目标由 ngtcp2 给出
        ngtcp2_path_storage_zero(&ps);
        ngtcp2_ssize n = ngtcp2_conn_writev_stream(qc->conn, &ps.path, NULL,
                                                   out, sizeof(out), &ndone,
                                                   NGTCP2_WRITE_STREAM_FLAG_NONE,
                                                   sid, &v, 1, quic_now());
        if (n > 0) {
            quic_send_to_path(qc, out, (size_t)n, &ps.path);
        } else if (n < 0 && n != NGTCP2_ERR_WRITE_MORE) {
            return -1;
        }
        if (ndone > 0) { woff += (size_t)ndone; total += (int64_t)ndone; }
        if (ndone == 0 && n == 0) {
            // 流控阻塞：泵一次等 ACK 释放窗口
            int pr = quic_pump(qc, 1000, 0, -1);
            if (pr != 0) break;
        }
        spins++;
    }
    return total;
}

// 从指定流读取：泵到该流有数据/FIN/超时。消费式拷出。返回字节数 / 0。
static int64_t quic_read_stream_bytes(quic_conn* qc, int64_t sid,
                                      uint8_t* out, int maxlen, int timeout_ms) {
    if (!qc || !qc->conn) return 0;
    quic_stream* s = quic_stream_slot(qc, sid);
    if (!s) return 0;                                // 流不存在
    if (s->len == 0 && !s->fin && !qc->peer_closed) {
        // 需泵数据：可能命中其他流，循环泵直到本流有数据/FIN/超时
        int64_t deadline = quic_now() + (uint64_t)timeout_ms * NGTCP2_MILLISECONDS;
        for (;;) {
            int64_t remain = deadline - quic_now();
            if (remain <= 0) return 0;
            int ms = (int)((remain + NGTCP2_MILLISECONDS - 1) / NGTCP2_MILLISECONDS);
            int pr = quic_pump(qc, ms, 1, sid);
            // M297（缺陷 504）：**别把「分段超时」当成「整体超时」**。
            //   quic_pump 内部把等待切成 ≤500ms 一段（`if (ms > 500) ms = 500;`），
            //   一段等不到就返回 -1 ⇒ 修前这里直接 return 0 意味着：无论调用方给
            //   timeout_ms 多少，实际等待上限都只有 ~500ms（超时参数不生效）。
            //   迁移场景下这尤其致命：对端换源后响应要走新路径重传，
            //   需要的时间 > 500ms ⇒ 调用方被「提前判超时」。
            //   ⇒ 只有**到 deadline** 才算超时；-2（真错误）仍然立即返回。
            if (pr == -2) return 0;
            if (pr != 0) {
                if (quic_now() >= deadline) return 0;
                continue;
            }
            if (s->len > 0 || s->fin || qc->peer_closed) break;
        }
    }
    if (s->len == 0) return 0;
    size_t take = s->len;
    if (maxlen > 0 && take > (size_t)maxlen) take = (size_t)maxlen;
    if (take > 0 && out) memcpy(out, s->buf, take);
    if (take < s->len) {
        memmove(s->buf, s->buf + take, s->len - take);
        s->len -= take;
    } else {
        s->len = 0;
    }
    return (int64_t)take;
}

// ---------- quic_open_stream（新 M50：本地 open 一条新双向流）----------
static LXValue bi_quic_open_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_open_stream 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    int64_t sid = -1;
    int rv = ngtcp2_conn_open_bidi_stream(qc->conn, &sid, NULL);
    if (rv != 0 || sid < 0) return px_int(rv != 0 ? rv : -1);   // M54-S4：透出 -206 STREAM_ID_BLOCKED
    if (!quic_stream_new(qc, sid, 0)) return px_int(-1);
    return px_int(sid);
}

// ---------- quic_open_uni_stream（新 M51：本地 open 一条单向流）----------
// HTTP/3 用：控制流/编码器流/解码器流各占一条 uni（client 2,6,10… / server 3,7,11…，
// 由 ngtcp2 按序分配，首条 uni 即控制流，符合 RFC 9114 §6.2.1）。
static LXValue bi_quic_open_uni_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_open_uni_stream 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    int64_t sid = -1;
    int rv = ngtcp2_conn_open_uni_stream(qc->conn, &sid, NULL);
    if (rv != 0 || sid < 0) return px_int(-1);
    // 本地开 uni 只发不收：建本地槽以便 quic_write_stream_bytes 可写
    if (!quic_stream_new(qc, sid, 0)) return px_int(-1);
    return px_int(sid);
}

// ---------- quic_send（旧：写默认流，单流兼容）----------
static LXValue bi_quic_send(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT) px_error("R1002: quic_send 需要 (conn: int, data: str)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    int64_t sid = quic_default_stream(qc);
    if (sid < 0) return px_int(-1);
    const uint8_t* data;
    size_t len;
    if (args[1].type == PX_STR || args[1].type == PX_BYTES) {
        data = (const uint8_t*)args[1].as.obj->as.str.data;
        len = (size_t)args[1].as.obj->as.str.len;
    } else { px_error("R1002: quic_send 的 data 需要 str"); return px_int(-1); }
    return px_int(quic_write_stream_bytes(qc, sid, data, len));
}

// ---------- quic_send_stream（新 M50：写指定流）----------
static LXValue bi_quic_send_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("R1002: quic_send_stream 需要 (conn: int, sid: int, data: str)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    int64_t sid = args[1].as.i;
    const uint8_t* data;
    size_t len;
    if (args[2].type == PX_STR || args[2].type == PX_BYTES) {
        data = (const uint8_t*)args[2].as.obj->as.str.data;
        len = (size_t)args[2].as.obj->as.str.len;
    } else { px_error("R1002: quic_send_stream 的 data 需要 str"); return px_int(-1); }
    return px_int(quic_write_stream_bytes(qc, sid, data, len));
}

// ---------- quic_poll（新 M50：等任一活跃流有数据/FIN → sid）----------
static LXValue bi_quic_poll(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("R1002: quic_poll 需要 (conn: int, timeout_ms: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    int timeout_ms = (int)args[1].as.i;
    if (quic_any_data(qc) == NULL && !qc->peer_closed) {
        int pr = quic_pump(qc, timeout_ms, 1, -1);
        if (pr != 0 && quic_any_data(qc) == NULL) return px_int(-1);
    }
    quic_stream* s = quic_any_data(qc);
    return px_int(s ? s->sid : -1);
}

// ---------- quic_recv（旧：读默认流，单流兼容）----------
static LXValue bi_quic_recv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("R1002: quic_recv 需要 (conn: int, maxlen: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_str("");
    int maxlen = (int)args[1].as.i;
    if (quic_any_data(qc) == NULL && !qc->peer_closed) {
        int pr = quic_pump(qc, 5000, 1, -1);
        if (pr != 0 && quic_any_data(qc) == NULL) return px_str("");
    }
    quic_stream* s = quic_any_data(qc);
    if (!s) return px_str("");
    size_t take = s->len;
    if (maxlen > 0 && take > (size_t)maxlen) take = (size_t)maxlen;
    LXValue r = px_str_len((const char*)s->buf, (int)take);
    if (take < s->len) {
        memmove(s->buf, s->buf + take, s->len - take);
        s->len -= take;
    } else {
        s->len = 0;
    }
    return r;
}

// ---------- quic_recv_stream（新 M50：读指定流）----------
static LXValue bi_quic_recv_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 4 || args[0].type != PX_INT || args[1].type != PX_INT ||
        args[2].type != PX_INT || args[3].type != PX_INT)
        px_error("R1002: quic_recv_stream 需要 (conn: int, sid: int, maxlen: int, timeout_ms: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_str("");
    int64_t sid = args[1].as.i;
    int maxlen = (int)args[2].as.i;
    int timeout_ms = (int)args[3].as.i;
    uint8_t tmp[4096];
    int cap = (maxlen > 0 && maxlen < (int)sizeof(tmp)) ? maxlen : (int)sizeof(tmp);
    int64_t got = quic_read_stream_bytes(qc, sid, tmp, cap, timeout_ms);
    if (got <= 0) return px_str("");
    return px_str_len((const char*)tmp, (int)got);
}

// ---------- quic_close ----------
// quic_pool_stats() -> dict —— M295（缺陷 508）：**连接池可见性**。
//   修前「槽位耗尽」在服务端**完全不可观测**：第 65 条连接的 Initial 被静默丢弃、
//   日志无痕，客户端只看到 connect-fail（与「服务端没起来」不可区分）。
//   本入口让语言层 / 监控**随时自检**：
//     conns_used 逼近 conns_max ⇒ 即将开始拒绝新连接；
//     alloc_fail > 0          ⇒ 已经发生过拒绝（full_first_ms = 首次发生时刻）。
static LXValue bi_quic_pool_stats(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 0) px_error("R1002: quic_pool_stats 不接受参数，实际给了 %d 个", nargs);
    int64_t lused = 0, cused = 0;
    pthread_mutex_lock(&g_quic_srv_mu);
    for (int i = 0; i < QUIC_MAX_LISTENERS; i++) if (g_qlis[i].used) lused++;
    for (int i = 0; i < QUIC_CONN_MAX; i++) if (g_qconns[i].used) cused++;
    int64_t aok = g_quic_alloc_ok, afail = g_quic_alloc_fail;
    int64_t peak = g_quic_live_peak, ffirst = g_quic_full_first;
    int64_t pclosed = g_quic_peer_closed;
    int64_t ciddrop = g_quic_cid_drop, cidused = 0;
    int64_t pmig = g_quic_path_migrated, pmiss = g_quic_path_missing, serr = g_quic_send_err;
    for (int i = 0; i < QUIC_MAX_LISTENERS; i++) if (g_qlis[i].used) cidused += g_qlis[i].cidtab_n;
    pthread_mutex_unlock(&g_quic_srv_mu);
    const char* e = getenv("PX_H3_IDLE_MS");   // 与 runtime_h3.c 的 h3_idle_ms() 同口径
    int idle = (!e || !*e) ? 8000 : atoi(e);
    if (idle < 50) idle = 50;
    if (idle > 60000) idle = 60000;
    LXValue d = px_dict();
    px_root_push_keep(d);   // M206：容器创建后**立刻**登记（下面 px_int/px_str 可能触发 GC）
    px_dict_set(d, "listeners_max", px_int(QUIC_MAX_LISTENERS));
    px_dict_set(d, "listeners_used", px_int(lused));
    px_dict_set(d, "conns_max", px_int(QUIC_CONN_MAX));
    px_dict_set(d, "conns_used", px_int(cused));
    px_dict_set(d, "conns_peak", px_int(peak));
    px_dict_set(d, "alloc_ok", px_int(aok));
    px_dict_set(d, "alloc_fail", px_int(afail));
    px_dict_set(d, "full_first_ms", px_int(ffirst));
    px_dict_set(d, "peer_closed_cnt", px_int(pclosed));   // M295：对端正常关闭次数（不再刷日志）
    px_dict_set(d, "cidtab_max", px_int(QUIC_CIDTAB_MAX)); // M295·第二面：单 listener cid 容量
    px_dict_set(d, "cidtab_used", px_int(cidused));        // 活跃 listener 的 cid 条目合计
    px_dict_set(d, "cid_drop", px_int(ciddrop));           // cid 表满导致的丢弃次数（修前静默）
    // M297（缺陷 504）：路径切换可观测性（修前「换源」完全不可观测，只能靠 strace）
    px_dict_set(d, "path_migrated", px_int(pmig));         // 路径验证成功次数（= 真的换过源）
    px_dict_set(d, "path_missing", px_int(pmiss));         // 写包时 ngtcp2 未给路径（兜底已发）
    px_dict_set(d, "send_err", px_int(serr));              // px_io_sendto 失败次数（非 EINTR）
    px_dict_set(d, "h3_idle_ms", px_int(idle));
    px_root_pop();
    return d;
}

// M296（缺陷 511）：fd 是否被某个**活跃 listener** 持有（= 连接与 listener 共享 UDP socket）。
//   这种 fd **不能**在连接关闭时 close —— 关掉的是 listener 的 socket（端口随即不再监听）。
//   共享形态出现在两处：quic_srv_new_conn（px_serve 托管）与 bi_quic_accept（手写服务端：
//   quic_listen + quic_accept，qc->fd = ql->fd）。
static int quic_fd_shared_with_listener(int fd) {
    if (fd < 0) return 0;
    for (int i = 0; i < QUIC_MAX_LISTENERS; i++)
        if (g_qlis[i].used && g_qlis[i].fd == fd) return 1;
    return 0;
}

static LXValue bi_quic_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_close 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc) return px_bool(false);
    // M296（缺陷 511）·第二面：**托管连接**（quic_listen + 路由线程形态，owner_listener > 0）
    //   由连接处理线程拥有（qc->thr 正在读写 qc 的队列与条件变量）；语言层对它 close 会
    //   `memset(qc, 0, …)` ⇒ 清掉线程正在用的 pthread_cond_t / 队列指针 = 跨线程内存破坏。
    //   实测这类 conn id **不会**交到语言层（px_serve 内部持有），故本守卫在当前版本**不可达**
    //   —— 它的价值是「结构上消除」未来某条路径把它暴露出去时的破坏面（宁可响亮拒绝）。
    if (qc->owner_listener > 0) {
        fprintf(stderr, "[quic] quic_close: 连接 %lld 由 listener %d 托管（连接线程拥有），"
                        "语言层不得关闭；请用 quic_close_listener\n",
                (long long)args[0].as.i, qc->owner_listener);
        return px_bool(false);
    }
    // M295（缺陷 508）：**修前**这里直接 ngtcp2_conn_del + close(fd)，**不发 CONNECTION_CLOSE**
    //   ⇒ 对端无从得知本端已关闭，只能等自己的空闲超时才回收槽位（服务端托管形态实测约 10s）；
    //   更糟的是「优雅关闭」与「进程被杀」在对端看来**完全一样** —— M295 实测：用 quic_close 的
    //   grace 档与不关闭的 abrupt 档，失败区间（第 64 起）与恢复点**逐项相同**。
    //   ⇒ 先写一个终止包再释放，让对端立刻进入 draining（服务端的 h3 连接循环随即退出）。
    // ⚠️ 只对客户端形态（owner_listener <= 0）发：服务端托管 conn 由连接线程拥有，
    //    语言层调用 quic_close 本就是跨线程操作 —— 不在此处扩展语义。
    if (qc->conn && qc->owner_listener <= 0) {
        uint8_t cc_out[QUIC_PKT_BUF];
        // ⚠️ M295 现场教训（必读）：`CCERR` 形参是 **`ngtcp2_ccerr*`（结构体指针）**，不是错误码整数。
        //   第一版写成 `(uint64_t)NGTCP2_NO_ERROR`（= 0）⇒ C 的隐式转换把它变成 **NULL 指针**，
        //   编译**不报错**（无 -Werror），运行到这一步直接 **SIGSEGV 在 ngtcp2 内部**
        //   （gdb 栈：`ngtcp2_conn_write_connection_close_versioned ← bi_quic_close`）。
        //   抓它的唯一办法是**让这条路径真的被执行**（本轮的装置 C「优雅关闭」档一次就抓到了；
        //   只跑「不关闭客户端」的验证会把它放过去）。
        ngtcp2_ccerr ccerr;
        ngtcp2_ccerr_set_transport_error(&ccerr, (uint64_t)NGTCP2_NO_ERROR, NULL, 0);
        ngtcp2_path_storage ps;                 // M297（缺陷 504）：终止包同样按 ngtcp2 的路径发
        ngtcp2_path_storage_zero(&ps);
        ngtcp2_ssize cc_n = ngtcp2_conn_write_connection_close(
            qc->conn, &ps.path, NULL, cc_out, sizeof(cc_out),
            &ccerr, quic_now());
        if (cc_n > 0) {
            // M295s1（缺陷 510）：必须走 px_io_sendto —— 裸 sendto 不重试 EINTR，
            //   而并发 GC 的 STW 信号（SIG_GC_STOP）会打断阻塞中的系统调用，
            //   且「设了 SO_*TIMEO 的 socket 系统调用」在 signal(7) 的「永不重启」清单里
            //   ⇒ 终止包会被静默丢掉 ⇒ 服务端又回到「分不清优雅关闭 vs 进程被杀」，
            //   即本修复的目的在并发下失效。同族：M211 缺陷 265 / M152 缺陷 146 / M256 缺陷 456。
            //   （抓到它的是 eintr_guard 与 m256_eintr —— M295 自己的门结构上看不见这里。）
            quic_send_to_path(qc, cc_out, (size_t)cc_n, &ps.path);
        }
    }
    if (qc->ssl) { SSL_free(qc->ssl); qc->ssl = NULL; }
    if (qc->ssl_ctx) { SSL_CTX_free(qc->ssl_ctx); qc->ssl_ctx = NULL; }
    if (qc->conn) { ngtcp2_conn_del(qc->conn); qc->conn = NULL; }
    quic_stream_free_all(qc);
    // M296（缺陷 511）：**fd 归属**判据 —— 只有独占 fd 才能关。
    //   `quic_listen` + `quic_accept` 形态下 qc->fd == ql->fd（与 listener **共享**同一个
    //   UDP socket，见 quic_srv_new_conn / bi_quic_accept）。修前这里无条件 close ⇒
    //   关一条连接就把 listener 的 socket 一起关掉：端口不再监听、后续 accept 全部失败
    //   （实测：close#1 之后 `ss -lun` 看不到该端口、accept#2 = -1、第二条客户端 CONNECTFAIL）。
    //   判据 = 「该 fd 是否被某个活跃 listener 持有」；持有 ⇒ 跳过（由 quic_close_listener 负责）。
    //   顺带消除既有示例 `quic_close(c); quic_close_listener(lst)` 的 **double close**。
    if (!quic_fd_shared_with_listener(qc->fd)) close(qc->fd);
    // M225：客户端侧（h3_client_*）同样按 conn 号持有 h3 会话状态 —— 一并回收
    if (g_quic_recycle) g_quic_recycle(args[0].as.i);
    memset(qc, 0, sizeof(*qc));
    return px_bool(true);
}

static LXValue bi_quic_close_listener(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: quic_close_listener 需要 (listener: int)");
    int64_t lid = args[0].as.i;
    quic_listener* ql = quic_get_listener(lid);
    if (!ql) return px_bool(false);
    if (ql->router_started) {          // M53：托管 listener → 停路由 + 关 conn
        quic_srv_close_listener(lid);
        return px_bool(true);
    }
    if (ql->ssl_ctx) { SSL_CTX_free(ql->ssl_ctx); ql->ssl_ctx = NULL; }
    close(ql->fd);
    memset(ql, 0, sizeof(*ql));
    return px_bool(true);
}

// ============================================================
// raw 接口（M47+：供 runtime_h3.c HTTP/3 语义层复用，绕开 LXValue）
// 命名 px_quic_raw_*：直接操作 conn/listener id 与字节缓冲。
// 复用上方 static 的 bi_* 注册函数与内部 quic_conn/listener 表。
// ============================================================

int64_t px_quic_raw_listen(int port) {
    LXValue a[1]; a[0] = px_int(port);
    LXValue r = bi_quic_listen(a, 1, NULL);
    return r.type == PX_INT ? r.as.i : -1;
}

int64_t px_quic_raw_accept(int64_t listener, int timeout_ms) {
    LXValue a[2]; a[0] = px_int(listener); a[1] = px_int(timeout_ms);
    LXValue r = bi_quic_accept(a, 2, NULL);
    return r.type == PX_INT ? r.as.i : -1;
}

int64_t px_quic_raw_connect(const char* ip, int port, const char* alpn) {
    // M206（缺陷 252 同族）：a[0] 跨 `a[2] = px_str(alpn)` 的分配。
    px_root_push();
    LXValue a[3];
    a[0] = px_str(ip); PX_KEEP(a[0]);
    a[1] = px_int(port);
    a[2] = px_str(alpn); PX_KEEP(a[2]);
    LXValue r = bi_quic_connect(a, 3, NULL);
    px_root_pop();
    return r.type == PX_INT ? r.as.i : -1;
}

// M50：打开一条新双向流（本地发起）。返回 sid / -1。
int64_t px_quic_raw_open_stream(int64_t conn) {
    LXValue a[1]; a[0] = px_int(conn);
    LXValue r = bi_quic_open_stream(a, 1, NULL);
    return r.type == PX_INT ? r.as.i : -1;
}

// M51：打开一条新单向流（本地发起）。返回 sid / -1。
int64_t px_quic_raw_open_uni_stream(int64_t conn) {
    LXValue a[1]; a[0] = px_int(conn);
    LXValue r = bi_quic_open_uni_stream(a, 1, NULL);
    return r.type == PX_INT ? r.as.i : -1;
}

// 写指定流（fin 预留；M50 MVP 以单 DATA 帧长度界定消息，不依赖 FIN，恒传 0）
int64_t px_quic_raw_send_on(int64_t conn, int64_t sid, const uint8_t* data, int len, int fin) {
    quic_conn* qc = quic_get_conn(conn);
    if (!qc || !qc->conn) return -1;
    int64_t w = quic_write_stream_bytes(qc, sid, data, (size_t)len);
    // M53-S4：HTTP/3 响应流结束需发 FIN（RFC 9114：响应以流结束结束）。数据全部写入后，
    // 以 writev_stream(空数据 + NGTCP2_WRITE_STREAM_FLAG_FIN) 发出流结束 —— aioquic 等
    // 标准客户端依赖 FIN 判定流结束；此前恒忽略 fin，标准客户端读响应永远等不到流结束而超时。
    // 注：不用 ngtcp2_conn_shutdown_stream_write（那是 abrupt 中止，会丢弃未确认数据 = RST 语义）。
    if (fin && w >= 0 && (w == len || len == 0)) {
        uint8_t out[QUIC_PKT_BUF];
        ngtcp2_ssize ndone = 0;
        ngtcp2_vec v0 = { NULL, 0 };
        ngtcp2_path_storage ps;                 // M297（缺陷 504）：发送目标由 ngtcp2 给出
        ngtcp2_path_storage_zero(&ps);
        ngtcp2_ssize n = ngtcp2_conn_writev_stream(qc->conn, &ps.path, NULL, out,
                                                   sizeof(out), &ndone,
                                                   NGTCP2_WRITE_STREAM_FLAG_FIN,
                                                   sid, &v0, 0, quic_now());
        if (n > 0) {
            quic_send_to_path(qc, out, (size_t)n, &ps.path);
        }
    }
    return w;
}

// 读指定流：消费式。返回实际字节数 / 0（超时/FIN/无数据）。
int64_t px_quic_raw_recv_on(int64_t conn, int64_t sid, uint8_t* out, int maxlen, int timeout_ms) {
    quic_conn* qc = quic_get_conn(conn);
    if (!qc || !qc->conn) return 0;
    return quic_read_stream_bytes(qc, sid, out, maxlen, timeout_ms);
}

// 等到任一活跃流有数据/FIN/新对端流 → 返回该 sid；超时 -1；错误 -2
int64_t px_quic_raw_poll(int64_t conn, int timeout_ms) {
    quic_conn* qc = quic_get_conn(conn);
    if (!qc || !qc->conn) return -2;
    if (quic_any_data(qc) == NULL && !qc->peer_closed) {
        int pr = quic_pump(qc, timeout_ms, 1, -1);
        if (pr != 0 && quic_any_data(qc) == NULL) return -1;
    }
    quic_stream* s = quic_any_data(qc);
    return s ? s->sid : -1;
}

// 默认流 = 最小活跃 sid（供 M47 旧 API 单流兼容定位）
int64_t px_quic_raw_first_stream(int64_t conn) {
    quic_conn* qc = quic_get_conn(conn);
    if (!qc) return -1;
    return quic_default_stream(qc);
}

// 发送字节（默认流：一次性尽量写完，内部处理流控）。返回写入字节数 / -1。
int64_t px_quic_raw_send(int64_t conn, const uint8_t* data, int len) {
    quic_conn* qc = quic_get_conn(conn);
    if (!qc) return -1;
    int64_t sid = quic_default_stream(qc);
    if (sid < 0) return -1;
    return quic_write_stream_bytes(qc, sid, data, (size_t)len);
}

// 阻塞接收至多 maxlen 字节（默认流；最多等 timeout_ms；0 = 超时/对端关闭）。
// 从内部该流缓冲取走并移除。返回实际字节数 / 0。
int64_t px_quic_raw_recv(int64_t conn, uint8_t* out, int maxlen, int timeout_ms) {
    quic_conn* qc = quic_get_conn(conn);
    if (!qc || !qc->conn) return 0;
    if (quic_any_data(qc) == NULL && !qc->peer_closed) {
        int pr = quic_pump(qc, timeout_ms, 1, -1);
        if (pr != 0 && quic_any_data(qc) == NULL) return 0;
    }
    quic_stream* s = quic_any_data(qc);
    if (!s) return 0;
    size_t take = s->len;
    if (maxlen > 0 && take > (size_t)maxlen) take = (size_t)maxlen;
    if (take > 0 && out) memcpy(out, s->buf, take);
    if (take < s->len) {
        memmove(s->buf, s->buf + take, s->len - take);
        s->len -= take;
    } else {
        s->len = 0;
    }
    return (int64_t)take;
}

bool px_quic_raw_close(int64_t conn) {
    LXValue a[1]; a[0] = px_int(conn);
    LXValue r = bi_quic_close(a, 1, NULL);
    return r.type == PX_BOOL && r.as.b;
}

bool px_quic_raw_close_listener(int64_t listener) {
    LXValue a[1]; a[0] = px_int(listener);
    LXValue r = bi_quic_close_listener(a, 1, NULL);
    return r.type == PX_BOOL && r.as.b;
}

// ---------- 注册（runtime.c px_register_builtins 调用）----------
void px_register_quic(void) {
    px_set_global("quic_listen", px_native("quic_listen", bi_quic_listen));
    px_set_global("quic_accept", px_native("quic_accept", bi_quic_accept));
    px_set_global("quic_connect", px_native("quic_connect", bi_quic_connect));
    px_set_global("quic_open_stream", px_native("quic_open_stream", bi_quic_open_stream));
    px_set_global("quic_open_uni_stream", px_native("quic_open_uni_stream", bi_quic_open_uni_stream));
    px_set_global("quic_send", px_native("quic_send", bi_quic_send));
    px_set_global("quic_send_stream", px_native("quic_send_stream", bi_quic_send_stream));
    px_set_global("quic_recv", px_native("quic_recv", bi_quic_recv));
    px_set_global("quic_recv_stream", px_native("quic_recv_stream", bi_quic_recv_stream));
    px_set_global("quic_poll", px_native("quic_poll", bi_quic_poll));
    px_set_global("quic_pool_stats", px_native("quic_pool_stats", bi_quic_pool_stats));
    px_set_global("quic_close", px_native("quic_close", bi_quic_close));
    px_set_global("quic_close_listener", px_native("quic_close_listener", bi_quic_close_listener));
    px_set_global("quic_h3_listen", px_native("quic_h3_listen", bi_quic_h3_listen));
    px_set_global("quic_connect_resume", px_native("quic_connect_resume", bi_quic_connect_resume));
    px_set_global("quic_session_save", px_native("quic_session_save", bi_quic_session_save));
    px_set_global("quic_conn_resumed", px_native("quic_conn_resumed", bi_quic_conn_resumed));
    px_set_global("quic_0rtt_save", px_native("quic_0rtt_save", bi_quic_0rtt_save));
    px_set_global("quic_connect_0rtt", px_native("quic_connect_0rtt", bi_quic_connect_0rtt));
    px_set_global("quic_0rtt_rejected", px_native("quic_0rtt_rejected", bi_quic_0rtt_rejected));
    px_set_global("quic_conn_handshake_done", px_native("quic_conn_handshake_done", bi_quic_conn_handshake_done));
    px_set_global("quic_migrate", px_native("quic_migrate", bi_quic_migrate));
    px_set_global("quic_conn_path", px_native("quic_conn_path", bi_quic_conn_path));
    px_set_global("quic_conn_local", px_native("quic_conn_local", bi_quic_conn_local));
    px_set_global("quic_set_max_client_streams", px_native("quic_set_max_client_streams", bi_quic_set_max_client_streams));
    px_set_global("quic_extend_max_streams", px_native("quic_extend_max_streams", bi_quic_extend_max_streams));
    px_set_global("quic_streams_left", px_native("quic_streams_left", bi_quic_streams_left));
    px_ffi_register("quic_listen", bi_quic_listen);
    px_ffi_register("quic_accept", bi_quic_accept);
    px_ffi_register("quic_connect", bi_quic_connect);
    px_ffi_register("quic_open_stream", bi_quic_open_stream);
    px_ffi_register("quic_open_uni_stream", bi_quic_open_uni_stream);
    px_ffi_register("quic_send", bi_quic_send);
    px_ffi_register("quic_send_stream", bi_quic_send_stream);
    px_ffi_register("quic_recv", bi_quic_recv);
    px_ffi_register("quic_recv_stream", bi_quic_recv_stream);
    px_ffi_register("quic_poll", bi_quic_poll);
    px_ffi_register("quic_pool_stats", bi_quic_pool_stats);
    px_ffi_register("quic_close", bi_quic_close);
    px_ffi_register("quic_close_listener", bi_quic_close_listener);
    px_ffi_register("quic_h3_listen", bi_quic_h3_listen);
    px_ffi_register("quic_connect_resume", bi_quic_connect_resume);
    px_ffi_register("quic_session_save", bi_quic_session_save);
    px_ffi_register("quic_conn_resumed", bi_quic_conn_resumed);
    px_ffi_register("quic_0rtt_save", bi_quic_0rtt_save);
    px_ffi_register("quic_connect_0rtt", bi_quic_connect_0rtt);
    px_ffi_register("quic_0rtt_rejected", bi_quic_0rtt_rejected);
    px_ffi_register("quic_conn_handshake_done", bi_quic_conn_handshake_done);
    px_ffi_register("quic_migrate", bi_quic_migrate);
    px_ffi_register("quic_conn_path", bi_quic_conn_path);
    px_ffi_register("quic_conn_local", bi_quic_conn_local);
    px_ffi_register("quic_set_max_client_streams", bi_quic_set_max_client_streams);
    px_ffi_register("quic_extend_max_streams", bi_quic_extend_max_streams);
    px_ffi_register("quic_streams_left", bi_quic_streams_left);
}
