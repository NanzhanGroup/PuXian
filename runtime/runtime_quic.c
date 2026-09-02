// 普贤 (PuXian) C 运行时 — QUIC 传输级绑定（M46 HTTP/3 应用验证）
// ------------------------------------------------------------
// 语言层 API（双模式一致，对齐 udp_* 心智）：
//   quic_listen(port) -> int            服务端监听（UDP bind + 自签证书就绪）→ listener id | -1
//   quic_accept(listener, timeout_ms) -> int  阻塞等 QUIC 连接（握手完成）→ conn id | -1 超时/失败
//   quic_connect(ip, port, alpn) -> int 客户端连接（握手完成）→ conn id | -1 失败
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

#define QUIC_MAX 64
#define QUIC_PKT_BUF 65536
#define QUIC_SCIDLEN 8

// ---------- 数据结构 ----------
typedef struct {
    int            fd;            // UDP socket
    struct sockaddr_in local;
    SSL_CTX*       ssl_ctx;       // 服务器 TLS 上下文（含自签证书）
    int            used;
} quic_listener;

// M50：多路复用——每连接多条双向流，各流独立 FIFO 接收缓冲
#define QUIC_STREAM_MAX 16   // 每连接并发双向流槽（对齐 initial_max_streams_bidi=16）

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
} quic_conn;

static ngtcp2_conn* quic_get_conn_from_ref(ngtcp2_crypto_conn_ref* ref) {
    quic_conn* qc = (quic_conn*)ref->user_data;
    return qc ? qc->conn : NULL;
}

static quic_listener g_qlis[QUIC_MAX];
static quic_conn     g_qconns[QUIC_MAX];
static int           g_quic_init = 0;

static uint64_t quic_now(void) {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (uint64_t)tp.tv_sec * NGTCP2_SECONDS + (uint64_t)tp.tv_nsec;
}

// ---------- callbacks ----------
static void quic_rand_cb(uint8_t* dest, size_t destlen,
                         const ngtcp2_rand_ctx* rctx) {
    (void)rctx;
    if (RAND_bytes(dest, (int)destlen) != 1) abort();
}

static int quic_get_new_cid_cb(ngtcp2_conn* conn, ngtcp2_cid* cid,
                               uint8_t* token, size_t cidlen, void* user_data) {
    (void)conn; (void)user_data;
    if (RAND_bytes(cid->data, (int)cidlen) != 1) return NGTCP2_ERR_CALLBACK_FAILURE;
    cid->datalen = cidlen;
    if (RAND_bytes(token, NGTCP2_STATELESS_RESET_TOKENLEN) != 1)
        return NGTCP2_ERR_CALLBACK_FAILURE;
    return 0;
}

static int quic_handshake_completed_cb(ngtcp2_conn* conn, void* user_data) {
    (void)conn;
    quic_conn* qc = (quic_conn*)user_data;
    qc->handshake_done = 1;
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
        if ((s->len > 0 || s->fin) && (!best || s->sid < best->sid)) best = s;
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

static void quic_log_cb(void* user_data, const char* fmt, ...) {
    (void)user_data; (void)fmt;  // 静默
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

// ---------- 事件泵：收包→喂 conn→发包，直到 handshake 或数据就绪或超时 ----------
// mode: 0=握手模式（直到 handshake_done）; 1=数据模式（直到 rlen>0 或对端关闭）
// 返回: 0 成功 / -1 超时 / -2 错误
static int quic_pump(quic_conn* qc, int64_t timeout_ms, int data_mode) {
    struct pollfd pfd = { .fd = qc->fd, .events = POLLIN };
    int64_t deadline = quic_now() + (uint64_t)timeout_ms * NGTCP2_MILLISECONDS;
    for (;;) {
        /* M50: data_mode 目标 = 任一活跃流有数据/FIN */
        if (data_mode) {
            if (quic_any_data(qc) != NULL) return 0;
            if (qc->peer_closed) return 0;
        } else {
            if (qc->handshake_done) return 0;
        }
        // 发送待发数据（握手/ACK/流控帧）
        uint8_t out[QUIC_PKT_BUF];
        ngtcp2_ssize n = ngtcp2_conn_write_pkt(qc->conn, &qc->path, NULL,
                                               out, sizeof(out), quic_now());
        if (n > 0) {
            sendto(qc->fd, out, (size_t)n, 0,
                   (struct sockaddr*)&qc->remote_sa, sizeof(qc->remote_sa));
        } else if (n < 0 && n != NGTCP2_ERR_WRITE_MORE) {
            fprintf(stderr, "[quic] pump write_pkt rv=%zd (%s)\n", n, ngtcp2_strerror((int)n));
            return -2;
        }
        // 等待收包
        int64_t remain = deadline - quic_now();
        if (remain <= 0) return -1;
        int ms = (int)((remain + NGTCP2_MILLISECONDS - 1) / NGTCP2_MILLISECONDS);
        int pr = poll(&pfd, 1, ms);
        if (pr < 0) { if (errno == EINTR) continue; fprintf(stderr, "[quic] pump poll err=%s\n", strerror(errno)); return -2; }
        if (pr == 0) return -1;  // 超时
        // 收包
        uint8_t pkt[QUIC_PKT_BUF];
        struct sockaddr_storage from;
        socklen_t fromlen = sizeof(from);
        ssize_t rl = recvfrom(qc->fd, pkt, sizeof(pkt), 0,
                              (struct sockaddr*)&from, &fromlen);
        if (rl <= 0) { if (errno == EINTR || errno == EAGAIN) continue; fprintf(stderr, "[quic] pump recvfrom err=%s\n", strerror(errno)); return -2; }
        // 更新 remote（服务端 accept 后可能变化，通常一致）
        if (fromlen == sizeof(struct sockaddr_in)) {
            memcpy(&qc->remote_sa, &from, fromlen);
            qc->path.remote.addr = (struct sockaddr*)&qc->remote_sa;
            qc->path.remote.addrlen = fromlen;
        }
        // 过期定时器
        ngtcp2_conn_handle_expiry(qc->conn, quic_now());
        // 喂包
        int rv = ngtcp2_conn_read_pkt(qc->conn, &qc->path, NULL, pkt, (size_t)rl,
                                      quic_now());
        if (rv != 0) {
            fprintf(stderr, "[quic] read_pkt rv=%d (%s)\n", rv, ngtcp2_strerror(rv));
            if (rv == NGTCP2_ERR_DRAINING || rv == NGTCP2_ERR_DROP_CONN) {
                qc->peer_closed = 1;
                return data_mode ? 0 : -1;
            }
            return -2;
        }
    }
}

// ---------- 服务端：quic_listen ----------
static int64_t quic_alloc_listener(quic_listener* ql) {
    for (int i = 0; i < QUIC_MAX; i++) {
        if (!g_qlis[i].used) { g_qlis[i].used = 1; *ql = g_qlis[i]; return i + 1; }
    }
    return -1;
}

static quic_listener* quic_get_listener(int64_t id) {
    if (id <= 0 || id > QUIC_MAX) return NULL;
    quic_listener* ql = &g_qlis[id - 1];
    return ql->used ? ql : NULL;
}

static LXValue bi_quic_listen(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) px_error("quic_listen 需要 (port: int)");
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
    for (int i = 0; i < QUIC_MAX; i++) {
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
    for (int i = 0; i < QUIC_MAX; i++) {
        if (!g_qconns[i].used) { g_qconns[i].used = 1; *qc = g_qconns[i]; return i + 1; }
    }
    return -1;
}

static quic_conn* quic_get_conn(int64_t id) {
    if (id <= 0 || id > QUIC_MAX) return NULL;
    quic_conn* qc = &g_qconns[id - 1];
    return qc->used ? qc : NULL;
}

static LXValue bi_quic_accept(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("quic_accept 需要 (listener: int, timeout_ms: int)");
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
        int pr = poll(&pfd, 1, ms);
        if (pr <= 0) return px_int(-1);
        uint8_t pkt[QUIC_PKT_BUF];
        struct sockaddr_storage from;
        socklen_t fromlen = sizeof(from);
        ssize_t rl = recvfrom(ql->fd, pkt, sizeof(pkt), 0,
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
        params.initial_max_streams_bidi = 16;
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
        // 喂第一个包
        ngtcp2_conn_handle_expiry(qc->conn, quic_now());
        rv = ngtcp2_conn_read_pkt(qc->conn, &qc->path, NULL, pkt, (size_t)rl, quic_now());
        if (rv != 0) {
            fprintf(stderr, "[quic] accept first read_pkt rv=%d (%s)\n", rv, ngtcp2_strerror(rv));
            SSL_free(ssl); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1);
        }
        // 泵到握手完成
        int pr2 = quic_pump(qc, timeout_ms, 0);
        if (pr2 != 0) {
            fprintf(stderr, "[quic] accept pump rv=%d\n", pr2);
            SSL_free(ssl); ngtcp2_conn_del(qc->conn); qc->used = 0; return px_int(-1);
        }
        return px_int(cid);
    }
}

// ---------- 客户端：quic_connect ----------
static LXValue bi_quic_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 3 || args[0].type != PX_STR || args[1].type != PX_INT || args[2].type != PX_STR)
        px_error("quic_connect 需要 (ip: str, port: int, alpn: str)");
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
    // quictls：SSL_CTX 已在 quic_make_client_ctx 时 configure，native handle = SSL*
    ngtcp2_conn_set_tls_native_handle(qc->conn, ssl);
    qc->conn_ref.get_conn = quic_get_conn_from_ref;
    qc->conn_ref.user_data = qc;
    SSL_set_app_data(ssl, &qc->conn_ref);
    SSL_set_connect_state(ssl);
    // 泵到握手完成（open_bidi_stream 需在对端参数已知后，即握手完成后）
    int pr = quic_pump(qc, 10000, 0);
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
        ngtcp2_ssize n = ngtcp2_conn_writev_stream(qc->conn, &qc->path, NULL,
                                                   out, sizeof(out), &ndone,
                                                   NGTCP2_WRITE_STREAM_FLAG_NONE,
                                                   sid, &v, 1, quic_now());
        if (n > 0) {
            sendto(qc->fd, out, (size_t)n, 0,
                   (struct sockaddr*)&qc->remote_sa, sizeof(qc->remote_sa));
        } else if (n < 0 && n != NGTCP2_ERR_WRITE_MORE) {
            return -1;
        }
        if (ndone > 0) { woff += (size_t)ndone; total += (int64_t)ndone; }
        if (ndone == 0 && n == 0) {
            // 流控阻塞：泵一次等 ACK 释放窗口
            int pr = quic_pump(qc, 1000, 0);
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
            int pr = quic_pump(qc, ms, 1);
            if (pr != 0) return 0;
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
    if (nargs < 1 || args[0].type != PX_INT) px_error("quic_open_stream 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    int64_t sid = -1;
    int rv = ngtcp2_conn_open_bidi_stream(qc->conn, &sid, NULL);
    if (rv != 0 || sid < 0) return px_int(-1);
    if (!quic_stream_new(qc, sid, 0)) return px_int(-1);
    return px_int(sid);
}

// ---------- quic_send（旧：写默认流，单流兼容）----------
static LXValue bi_quic_send(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT) px_error("quic_send 需要 (conn: int, data: str)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    int64_t sid = quic_default_stream(qc);
    if (sid < 0) return px_int(-1);
    const uint8_t* data;
    size_t len;
    if (args[1].type == PX_STR || args[1].type == PX_BYTES) {
        data = (const uint8_t*)args[1].as.obj->as.str.data;
        len = (size_t)args[1].as.obj->as.str.len;
    } else { px_error("quic_send 的 data 需要 str"); return px_int(-1); }
    return px_int(quic_write_stream_bytes(qc, sid, data, len));
}

// ---------- quic_send_stream（新 M50：写指定流）----------
static LXValue bi_quic_send_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 3 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("quic_send_stream 需要 (conn: int, sid: int, data: str)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    int64_t sid = args[1].as.i;
    const uint8_t* data;
    size_t len;
    if (args[2].type == PX_STR || args[2].type == PX_BYTES) {
        data = (const uint8_t*)args[2].as.obj->as.str.data;
        len = (size_t)args[2].as.obj->as.str.len;
    } else { px_error("quic_send_stream 的 data 需要 str"); return px_int(-1); }
    return px_int(quic_write_stream_bytes(qc, sid, data, len));
}

// ---------- quic_poll（新 M50：等任一活跃流有数据/FIN → sid）----------
static LXValue bi_quic_poll(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("quic_poll 需要 (conn: int, timeout_ms: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_int(-1);
    int timeout_ms = (int)args[1].as.i;
    if (quic_any_data(qc) == NULL && !qc->peer_closed) {
        int pr = quic_pump(qc, timeout_ms, 1);
        if (pr != 0 && quic_any_data(qc) == NULL) return px_int(-1);
    }
    quic_stream* s = quic_any_data(qc);
    return px_int(s ? s->sid : -1);
}

// ---------- quic_recv（旧：读默认流，单流兼容）----------
static LXValue bi_quic_recv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("quic_recv 需要 (conn: int, maxlen: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc || !qc->conn) return px_str("");
    int maxlen = (int)args[1].as.i;
    if (quic_any_data(qc) == NULL && !qc->peer_closed) {
        int pr = quic_pump(qc, 5000, 1);
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
    if (nargs < 4 || args[0].type != PX_INT || args[1].type != PX_INT ||
        args[2].type != PX_INT || args[3].type != PX_INT)
        px_error("quic_recv_stream 需要 (conn: int, sid: int, maxlen: int, timeout_ms: int)");
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
static LXValue bi_quic_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) px_error("quic_close 需要 (conn: int)");
    quic_conn* qc = quic_get_conn(args[0].as.i);
    if (!qc) return px_bool(false);
    if (qc->ssl) { SSL_free(qc->ssl); qc->ssl = NULL; }
    if (qc->ssl_ctx) { SSL_CTX_free(qc->ssl_ctx); qc->ssl_ctx = NULL; }
    if (qc->conn) { ngtcp2_conn_del(qc->conn); qc->conn = NULL; }
    quic_stream_free_all(qc);
    close(qc->fd);
    memset(qc, 0, sizeof(*qc));
    return px_bool(true);
}

static LXValue bi_quic_close_listener(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) px_error("quic_close_listener 需要 (listener: int)");
    quic_listener* ql = quic_get_listener(args[0].as.i);
    if (!ql) return px_bool(false);
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
    LXValue a[3];
    a[0] = px_str(ip); a[1] = px_int(port); a[2] = px_str(alpn);
    LXValue r = bi_quic_connect(a, 3, NULL);
    return r.type == PX_INT ? r.as.i : -1;
}

// M50：打开一条新双向流（本地发起）。返回 sid / -1。
int64_t px_quic_raw_open_stream(int64_t conn) {
    LXValue a[1]; a[0] = px_int(conn);
    LXValue r = bi_quic_open_stream(a, 1, NULL);
    return r.type == PX_INT ? r.as.i : -1;
}

// 写指定流（fin 预留；M50 MVP 以单 DATA 帧长度界定消息，不依赖 FIN，恒传 0）
int64_t px_quic_raw_send_on(int64_t conn, int64_t sid, const uint8_t* data, int len, int fin) {
    (void)fin;
    quic_conn* qc = quic_get_conn(conn);
    if (!qc) return -1;
    return quic_write_stream_bytes(qc, sid, data, (size_t)len);
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
        int pr = quic_pump(qc, timeout_ms, 1);
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
        int pr = quic_pump(qc, timeout_ms, 1);
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
    px_set_global("quic_send", px_native("quic_send", bi_quic_send));
    px_set_global("quic_send_stream", px_native("quic_send_stream", bi_quic_send_stream));
    px_set_global("quic_recv", px_native("quic_recv", bi_quic_recv));
    px_set_global("quic_recv_stream", px_native("quic_recv_stream", bi_quic_recv_stream));
    px_set_global("quic_poll", px_native("quic_poll", bi_quic_poll));
    px_set_global("quic_close", px_native("quic_close", bi_quic_close));
    px_set_global("quic_close_listener", px_native("quic_close_listener", bi_quic_close_listener));
    px_ffi_register("quic_listen", bi_quic_listen);
    px_ffi_register("quic_accept", bi_quic_accept);
    px_ffi_register("quic_connect", bi_quic_connect);
    px_ffi_register("quic_open_stream", bi_quic_open_stream);
    px_ffi_register("quic_send", bi_quic_send);
    px_ffi_register("quic_send_stream", bi_quic_send_stream);
    px_ffi_register("quic_recv", bi_quic_recv);
    px_ffi_register("quic_recv_stream", bi_quic_recv_stream);
    px_ffi_register("quic_poll", bi_quic_poll);
    px_ffi_register("quic_close", bi_quic_close);
    px_ffi_register("quic_close_listener", bi_quic_close_listener);
}
