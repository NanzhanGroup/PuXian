// 普贤 (PuXian) C 运行时 — WebSocket（RFC 6455）模块（M22）
// 语言层 API（与解释器 ws.rs 双模式一致）：
//   ws_serve(port, handler)  —— 服务端：accept 循环，每连接 px_spawn 处理线程
//                               （握手 → 注册 conn id → 调 handler(conn) → 保持到关闭）
//   ws_connect(host, port, path) → int conn | null（客户端握手）
//   ws_send(conn, data) → bool（文本帧；客户端连接自动掩码）
//   ws_recv(conn) → str|null（阻塞读一条完整消息；自动重组分片、回复 ping、响应 close）
//   ws_close(conn) → bool（发 close 帧 + shutdown 唤醒阻塞 recv）
// 并发安全：注册表 g_ws_mu 持锁期间屏蔽 SIG_GC_STOP（gc_block_stop，同 M22 slab 模式）
//          防"持锁线程被 GC 暂停 → GC 等锁死锁"；send/recv 不持注册表锁（复制 fd 后操作）。
// 连接线程为 px_spawn 注册进 GC 槽位（与 M11 并发 GC 兼容）。
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include "mbedtls/sha1.h"
#include "locktrack.h"   // M127（qg-issue 84）：回卷锁审计 —— 必须放在**最后一个 include**

// M32：wss 客户端复用 runtime.c 的 HTTPS 会话（HttpsSession 为内部类型，void* 包装）
extern void* px_https_connect_ex(const char* host, int port);
extern int px_https_fd_ex(void* hs);
extern void px_https_close_ex(void* hs);
extern PxConn* px_pxconn_from_https(void* hs);

// ==================== 连接注册表 ====================
#define MAX_WS_CONNS 256
static pthread_mutex_t g_ws_mu = PTHREAD_MUTEX_INITIALIZER;
static struct {
    int fd;            // socket
    int64_t id;        // conn id
    int active;        // 槽位占用
    int client;        // 1 = 客户端连接（发送需掩码）；0 = 服务端连接
    int closed;        // 已发送/收到 close（停止使用）
    long long last_activity; // 最近读到任何帧的时间（毫秒，0=未初始化；ws_heartbeat 超时检测）
    int hb_active;           // 心跳线程已启动（防重复）
    PxConn* conn;            // M27：连接对象（明文/TLS 统一读写；TLS 时共享指针）
    void* hs;                // M32：wss 客户端 HttpsSession*（关闭时释放）
    // ── M235：握手元信息（晨曦 ws-edge 需求）──
    //   修前 `ws_server_handshake` 读完请求头即 `free(head)` ⇒ path 永久丢失，
    //   语言层也没有任何 API 能拿到 path/对端地址 ⇒ 多节点路由、单 IP 限流、审计
    //   全部做不了（ws-edge §七.3/§七.4 记载的两处缺口）。此处把它们随连接留存。
    char path[512];          // 握手请求行里的 path（如 "/agent/cx-node-7"）
    char* hdrs;              // 握手请求头原文（malloc，为 NULL 表示无；槽复用时释放）
    char peer[72];           // 对端 "ip:port"（getpeername，AF_INET/AF_INET6）
} g_ws_conns[MAX_WS_CONNS];
static int64_t g_ws_next_id = 1;
// M38：客户端自动重连配置（conn id → url/重连间隔 ms；明文 ws:// 重连）
static char g_ws_reconnect_url[MAX_WS_CONNS][512];
static long long g_ws_reconnect_ms[MAX_WS_CONNS];
static int g_ws_reconnect_set[MAX_WS_CONNS];

// 当前毫秒时间戳
static long long ws_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// 标记最近活动时间（读到任何帧时调用；持锁）
static void ws_mark_activity(int idx) {
    if (idx >= 0 && idx < MAX_WS_CONNS) {
        g_ws_conns[idx].last_activity = ws_now_ms();
    }
}

static int ws_find(int64_t id) {
    for (int i = 0; i < MAX_WS_CONNS; i++) {
        if (g_ws_conns[i].active && g_ws_conns[i].id == id) return i;
    }
    return -1;
}

static int ws_alloc_slot(void) {
    for (int i = 0; i < MAX_WS_CONNS; i++) {
        if (!g_ws_conns[i].active) {
            // M235：槽是按需复用（而非每连接释放）⇒ 握手头原文在此处归还，
            //   保证泄漏有界（≤ MAX_WS_CONNS 份）。
            if (g_ws_conns[i].hdrs) { free(g_ws_conns[i].hdrs); g_ws_conns[i].hdrs = NULL; }
            g_ws_conns[i].path[0] = 0;
            g_ws_conns[i].peer[0] = 0;
            return i;
        }
    }
    return -1;
}

// 释放 wss 客户端 TLS 会话（持锁调用；hs 为 HttpsSession*）
static void ws_free_hs_locked(int idx) {
    if (idx >= 0 && idx < MAX_WS_CONNS && g_ws_conns[idx].hs) {
        void* hs = g_ws_conns[idx].hs;
        g_ws_conns[idx].hs = NULL;
        px_https_close_ex(hs);
    }
}

// 复制 fd（不持锁做 IO；返回 -1 表示不存在/已关闭）
static int ws_get_fd(int64_t id, int* is_client) {
    int fd = -1;
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(id);
    if (idx >= 0 && !g_ws_conns[idx].closed) {
        fd = g_ws_conns[idx].fd;
        if (is_client) *is_client = g_ws_conns[idx].client;
    }
    pthread_mutex_unlock(&g_ws_mu);
    return fd;
}

// M240（晨曦缺陷 B 之三 = 晨曦建议 ①）：**释放连接对象前同步失效 WS 注册表**。
//   为什么必须有：语言层只拿得到整数 conn id，`ws_get_conn(id)` 从注册表槽里取裸指针。
//   若对象被 `px_conn_owner_free` xfree 而槽仍指着它，**下一次 `ws_recv(id)` 就会把这个
//   悬垂指针取出来**（acquire 也要先摸 `c->mu` ⇒ 直接 SIGSEGV）。实测 core：
//   `px_conn_free_res ← px_conn_release ← bi_ws_recv ← offload 线程`。
//   ⇒ 「谁真正释放对象，谁先清槽」是这一对结构的**闭合条件**（与 M235 的引用计数正交：
//   计数保证「正在用的不被释放」，清槽保证「释放后不会被取到」）。
// 锁序：调用方必须**不在** `g_ws_mu` 内（本文件各 API 与 runtime.c 的释放点都满足）。
// M240（修复 g）：PxConn 的**客户端/旁路**初始化 —— 与 `px_conn_init` 共用「字段归零 +
//   mutex 初始化」，但**不做**服务端 TLS 状态分配（`owned=0`：TLS 由外部 HttpsSession 管理）。
//   ⚠️ 原先 4 处旁路只 `memset` + 手赋 fd ⇒ **漏了 `pthread_mutex_init(&c->mu)`**，
//   而 M235 的引用计数（acquire/release/close）全以 `c->mu` 为同步原语 ⇒ 这些对象一直在
//   未初始化的 mutex 上加解锁（实测崩溃对象来自这条旁路）。
static void px_conn_init_client(PxConn* c, int fd) {
    memset(c, 0, sizeof(*c));
    pthread_mutex_init(&c->mu, NULL);
    c->fd = fd;
    c->is_tls = 0;
    c->owned = 0;          // 客户端：TLS 状态不归本对象（由 px_https_close_ex 释放）
    c->rlen = 0;
    c->roff = 0;
    c->refs = 0;
    c->pending_free = 0;
    c->freed = 0;
    c->obj_free_pending = 0;
    c->obj_freed = 0;
    c->res_done = 0;
}

void px_ws_detach_conn(PxConn* c) {
    if (!c) return;
    pthread_mutex_lock(&g_ws_mu);
    for (int i = 0; i < MAX_WS_CONNS; i++) {
        if (g_ws_conns[i].active && g_ws_conns[i].conn == c) {
            g_ws_conns[i].active = 0;
            g_ws_conns[i].fd = -1;
            g_ws_conns[i].conn = NULL;
            g_ws_conns[i].closed = 1;
            g_ws_conns[i].hb_active = 0;
            ws_free_hs_locked(i);
        }
    }
    pthread_mutex_unlock(&g_ws_mu);
}

// M27：取连接对象（TLS/明文统一；不持锁做 IO）。返回 NULL 表示不存在/已关闭。
// M240（晨曦缺陷 B 之二）：**返回前在锁内 acquire 一次引用** —— 原实现把裸指针交出去，
//   调用方在锁外使用，而「HTTP 侧释放对象」与「槽被注销」是两步 ⇒ 取指针后、使用前
//   对象可能已被 `px_conn_owner_free`（xfree）⇒ UAF（实测 core：`px_conn_free_res`
//   ← `bi_ws_recv` ← offload 线程）。持引用后，M235 的延迟释放会兜住最后一次使用。
//   **调用方必须配对 `px_conn_release(c)`**（本文件 4 个语言层 API 已逐出口配对）。
//   锁序：`g_ws_mu` → `c->mu`（`px_conn_*` 内部不碰 `g_ws_mu` ⇒ 无反向嵌套）。
static PxConn* ws_get_conn(int64_t id, int* is_client) {
    PxConn* c = NULL;
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(id);
    if (idx >= 0 && !g_ws_conns[idx].closed) {
        c = g_ws_conns[idx].conn;
        if (is_client) *is_client = g_ws_conns[idx].client;
        if (c && !px_conn_acquire(c)) c = NULL;   // M240：对象已关闭/已释放 ⇒ 当作不存在
    }
    pthread_mutex_unlock(&g_ws_mu);
    return c;
}

// M27：连接对象是否有 TLS 读缓冲（ws_recv 超时 poll 前检查：缓冲有数据则无需 poll）
static int ws_conn_has_buffered(PxConn* c) {
    return c && c->is_tls && c->roff < c->rlen;
}

// ==================== base64（握手 Accept 计算用，本地副本） ====================
static const char WS_B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void ws_b64_encode(const unsigned char* in, size_t len, char* out) {
    size_t i = 0, oi = 0;
    while (i + 3 <= len) {
        unsigned n = ((unsigned)in[i] << 16) | ((unsigned)in[i + 1] << 8) | in[i + 2];
        out[oi++] = WS_B64[(n >> 18) & 63];
        out[oi++] = WS_B64[(n >> 12) & 63];
        out[oi++] = WS_B64[(n >> 6) & 63];
        out[oi++] = WS_B64[n & 63];
        i += 3;
    }
    size_t rem = len - i;
    if (rem == 1) {
        unsigned n = (unsigned)in[i] << 16;
        out[oi++] = WS_B64[(n >> 18) & 63];
        out[oi++] = WS_B64[(n >> 12) & 63];
        out[oi++] = '='; out[oi++] = '=';
    } else if (rem == 2) {
        unsigned n = ((unsigned)in[i] << 16) | ((unsigned)in[i + 1] << 8);
        out[oi++] = WS_B64[(n >> 18) & 63];
        out[oi++] = WS_B64[(n >> 12) & 63];
        out[oi++] = WS_B64[(n >> 6) & 63];
        out[oi++] = '=';
    }
    out[oi] = 0;
}

// ==================== 帧协议（RFC 6455 §5） ====================
#define WS_OP_CONT 0x0
#define WS_OP_TEXT 0x1
#define WS_OP_BINARY 0x2
#define WS_OP_CLOSE 0x8
#define WS_OP_PING 0x9
#define WS_OP_PONG 0xA

// 精确读 n 字节（阻塞；EOF/错误返回 -1，EINTR 重试）
static int ws_read_exact(PxConn* c, unsigned char* buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t r = px_conn_read(c, buf + off, n - off);
        if (r == 0) return -1;
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)r;
    }
    return 0;
}

// 读一帧：返回 opcode（<0 错误）；fin/payload 由出参给出。payload malloc，调用者 free。
static int ws_read_frame(PxConn* c, int* fin, unsigned char** payload, size_t* plen) {
    unsigned char h[2];
    if (ws_read_exact(c, h, 2) < 0) return -1;
    *fin = (h[0] & 0x80) ? 1 : 0;
    int opcode = h[0] & 0x0F;
    int masked = (h[1] & 0x80) ? 1 : 0;
    uint64_t len = h[1] & 0x7F;
    if (len == 126) {
        unsigned char ext[2];
        if (ws_read_exact(c, ext, 2) < 0) return -1;
        len = ((uint64_t)ext[0] << 8) | ext[1];
    } else if (len == 127) {
        unsigned char ext[8];
        if (ws_read_exact(c, ext, 8) < 0) return -1;
        len = 0;
        for (int i = 0; i < 8; i++) len = (len << 8) | ext[i];
    }
    if (len > 64ULL * 1024 * 1024) return -1;
    unsigned char mask[4] = {0, 0, 0, 0};
    if (masked && ws_read_exact(c, mask, 4) < 0) return -1;
    unsigned char* p = (unsigned char*)malloc((size_t)len + 1);
    if (len > 0) {
        if (ws_read_exact(c, p, (size_t)len) < 0) { free(p); return -1; }
        if (masked) {
            for (size_t i = 0; i < len; i++) p[i] ^= mask[i & 3];
        }
    }
    p[len] = 0;
    *payload = p;
    *plen = (size_t)len;
    return opcode;
}

// 编码并发送一帧（mask_out=1 时客户端掩码）。返回 0 成功。
static int ws_send_frame(PxConn* c, int opcode, const unsigned char* data, size_t len, int mask_out) {
    unsigned char hdr[14];
    size_t hl = 2;
    hdr[0] = (unsigned char)(0x80 | opcode);  // FIN=1
    uint64_t l = len;
    if (l < 126) {
        hdr[1] = (unsigned char)l;
    } else if (l <= 0xFFFF) {
        hdr[1] = 126;
        hdr[2] = (unsigned char)((l >> 8) & 0xFF);
        hdr[3] = (unsigned char)(l & 0xFF);
        hl = 4;
    } else {
        hdr[1] = 127;
        for (int i = 0; i < 8; i++) hdr[2 + i] = (unsigned char)((l >> (56 - i * 8)) & 0xFF);
        hl = 10;
    }
    if (mask_out) {
        hdr[1] |= 0x80;
        // 生成 4 字节掩码（时间 + 计数器派生）
        static unsigned long long ws_seq = 0;
        unsigned long long t = (unsigned long long)time(NULL) * 2654435761u + (ws_seq++);
        unsigned char mask[4] = {
            (unsigned char)(t & 0xFF), (unsigned char)((t >> 8) & 0xFF),
            (unsigned char)((t >> 16) & 0xFF), (unsigned char)((t >> 24) & 0xFF)
        };
        memcpy(hdr + hl, mask, 4);
        hl += 4;
        // 掩码后的载荷
        unsigned char* tmp = (unsigned char*)malloc(len ? len : 1);
        for (size_t i = 0; i < len; i++) tmp[i] = data[i] ^ mask[i & 3];
        if (px_conn_write(c, hdr, hl) < 0) { free(tmp); return -1; }
        if (len > 0 && px_conn_write(c, tmp, len) < 0) { free(tmp); return -1; }
        free(tmp);
        return 0;
    }
    if (px_conn_write(c, hdr, hl) < 0) return -1;
    if (len > 0 && px_conn_write(c, data, len) < 0) return -1;
    return 0;
}

// ==================== 握手（RFC 6455 §4.2） ====================
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

// 读 HTTP 头直到 \r\n\r\n（上限 64KB）。返回 0 成功；buf/len 出参（malloc）。
// 明文 fd 版本（客户端握手 ws_client_handshake 用）
static int ws_read_http_header_fd(int fd, char** out, int* out_len) {
    char* buf = (char*)malloc(65536);
    int len = 0;
    int header_end = -1;
    while (len < 65535) {
        ssize_t n = recv(fd, buf + len, (size_t)(65535 - len), 0);
        if (n <= 0) break;
        len += (int)n;
        buf[len] = 0;
        char* sep = strstr(buf, "\r\n\r\n");
        if (sep) { header_end = (int)(sep - buf); break; }
    }
    if (header_end < 0) { free(buf); return -1; }
    buf[header_end] = 0;
    *out = buf;
    *out_len = header_end;
    return 0;
}

// PxConn 版本（服务端/客户端统一；TLS 支持）
static int ws_read_http_header(PxConn* c, char** out, int* out_len) {
    char* buf = (char*)malloc(65536);
    int len = 0;
    int header_end = -1;
    while (len < 65535) {
        ssize_t n = px_conn_read(c, buf + len, (size_t)(65535 - len));
        if (n <= 0) break;
        len += (int)n;
        buf[len] = 0;
        char* sep = strstr(buf, "\r\n\r\n");
        if (sep) { header_end = (int)(sep - buf); break; }
    }
    if (header_end < 0) { free(buf); return -1; }
    buf[header_end] = 0;
    *out = buf;
    *out_len = header_end;
    return 0;
}

// 从头中提取指定字段值（大小写不敏感；返回 malloc 或 NULL）
static char* ws_header_value(const char* head, const char* name) {
    const char* p = head;
    while (p && *p) {
        const char* eol = strstr(p, "\r\n");
        size_t ll = eol ? (size_t)(eol - p) : strlen(p);
        char line[4096];
        size_t cl = ll < 4095 ? ll : 4095;
        memcpy(line, p, cl);
        line[cl] = 0;
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = 0;
            if (strcasecmp(line, name) == 0) {
                char* v = colon + 1;
                while (*v == ' ') v++;
                char* ve = v + strlen(v);
                while (ve > v && (ve[-1] == ' ' || ve[-1] == '\r')) ve--;
                *ve = 0;
                return strdup(v);
            }
        }
        if (!eol) break;
        p = eol + 2;
    }
    return NULL;
}

// 计算 Sec-WebSocket-Accept = base64(SHA1(key + GUID))
static void ws_accept_key(const char* key, char* out) {
    char input[256];
    snprintf(input, sizeof(input), "%s%s", key, WS_GUID);
    unsigned char digest[20];
    mbedtls_sha1((const unsigned char*)input, strlen(input), digest);
    ws_b64_encode(digest, 20, out);
}

// 服务端握手：读请求 → 校验 → 发 101。返回 0 成功。
// M235：out_path/out_head 为输出参数（可为 NULL）。成功时二者所有权移交调用方
//   （head 可能为 NULL —— 分配失败时退化为「无头信息」，不影响握手本身）。
static int ws_server_handshake(PxConn* c, char** out_path, char** out_head) {
    char* head = NULL;
    int hlen = 0;
    if (ws_read_http_header(c, &head, &hlen) < 0) return -1;
    if (strncmp(head, "GET ", 4) != 0) { free(head); return -1; }
    char* key = ws_header_value(head, "Sec-WebSocket-Key");
    char* upgrade = ws_header_value(head, "Upgrade");
    // M235：请求行 = "GET <path> HTTP/1.1" ⇒ 取第一、二空格之间的片段。
    char* path = (char*)malloc(512);
    if (path) {
        path[0] = 0;
        const char* sp = strchr(head + 4, ' ');
        if (sp) {
            size_t pl = (size_t)(sp - (head + 4));
            if (pl > 0 && pl < 511) { memcpy(path, head + 4, pl); path[pl] = 0; }
        }
    }
    if (!key || !upgrade || strcasecmp(upgrade, "websocket") != 0) {
        if (key) free(key);
        if (upgrade) free(upgrade);
        if (path) free(path);
        free(head);
        return -1;
    }
    char accept[64];
    ws_accept_key(key, accept);
    free(key);
    free(upgrade);
    char resp[256];
    int rl = snprintf(resp, sizeof(resp),
        "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n",
        accept);
    if (px_conn_write(c, resp, (size_t)rl) < 0) {
        if (path) free(path);
        free(head);
        return -1;
    }
    if (out_path) *out_path = path; else if (path) free(path);
    if (out_head) *out_head = head; else free(head);
    return 0;
}

// 客户端握手：发 Upgrade 请求 → 校验 101 + Accept。返回 0 成功。
static int ws_client_handshake(int fd, const char* host, int port, const char* path) {
    // 16 字节 key（时间 + 计数器派生）
    static unsigned long long ws_key_seq = 0;
    unsigned long long t = (unsigned long long)time(NULL) * 2654435761u + (ws_key_seq++);
    unsigned char kb[16];
    for (int i = 0; i < 16; i++) {
        t = t * 6364136223846793005ULL + 1442695040888963407ULL;
        kb[i] = (unsigned char)((t >> 33) & 0xFF);
    }
    char key[32];
    ws_b64_encode(kb, 16, key);
    char req[1024];
    int rl = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Version: 13\r\n\r\n",
        path, host, port, key);
    if (send(fd, req, rl, MSG_NOSIGNAL) < 0) return -1;
    char* head = NULL;
    int hlen = 0;
    if (ws_read_http_header_fd(fd, &head, &hlen) < 0) return -1;
    if (strstr(head, " 101 ") == NULL) { free(head); return -1; }
    char* got = ws_header_value(head, "Sec-WebSocket-Accept");
    free(head);
    if (!got) return -1;
    char expect[64];
    ws_accept_key(key, expect);
    int ok = (strcmp(got, expect) == 0) ? 0 : -1;
    free(got);
    return ok;
}

// 客户端握手（PxConn 版本：明文/TLS 统一；M32 wss 用）
static int ws_client_handshake_px(PxConn* c, const char* host, int port, const char* path) {
    static unsigned long long ws_key_seq = 0;
    unsigned long long t = (unsigned long long)time(NULL) * 2654435761u + (ws_key_seq++);
    unsigned char kb[16];
    for (int i = 0; i < 16; i++) {
        t = t * 6364136223846793005ULL + 1442695040888963407ULL;
        kb[i] = (unsigned char)((t >> 33) & 0xFF);
    }
    char key[32];
    ws_b64_encode(kb, 16, key);
    char req[1024];
    int rl = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\nHost: %s:%d\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: %s\r\nSec-WebSocket-Version: 13\r\n\r\n",
        path, host, port, key);
    if (px_conn_write(c, req, (size_t)rl) < 0) return -1;
    char* head = NULL;
    int hlen = 0;
    if (ws_read_http_header(c, &head, &hlen) < 0) return -1;
    if (strstr(head, " 101 ") == NULL) { free(head); return -1; }
    char* got = ws_header_value(head, "Sec-WebSocket-Accept");
    free(head);
    if (!got) return -1;
    char expect[64];
    ws_accept_key(key, expect);
    int ok = (strcmp(got, expect) == 0) ? 0 : -1;
    free(got);
    return ok;
}

// ==================== 语言层 API ====================

// ws_serve(port, handler)：阻塞 accept 循环（Go 风格）
// M36：服务端自动心跳配置（ws_serve opts{heartbeat:{interval_ms,timeout_ms}}）
static long long g_ws_hb_interval = 0;
static long long g_ws_hb_timeout = 60000;

LXValue bi_ws_serve(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    // M36：ws_serve(port, handler[, opts{heartbeat:{interval_ms,timeout_ms}}])
    if (nargs < 2 || nargs > 3 || args[0].type != PX_INT) px_error("R1002: ws_serve 需要 (port, handler[, opts]) 参数");
    LXValue handler = args[1];
    if (handler.type != PX_FUNC && handler.type != PX_NATIVE) px_error("R1002: ws_serve 的 handler 必须是函数");
    px_set_global("__ws_handler", handler);
    // M36：心跳配置
    g_ws_hb_interval = 0;
    g_ws_hb_timeout = 60000;
    // M194：可选实参存在即校验（`null` = 未提供）
    if (nargs >= 3 && args[2].type != PX_NULL && args[2].type != PX_DICT)
        px_error("R1002: ws_serve 的 opts 需要字典，实际是 %s", px_type_name(args[2]));
    if (nargs >= 3 && args[2].type == PX_DICT) {
        LXValue hb = px_dict_get(args[2], "heartbeat");
        if (hb.type == PX_DICT) {
            int hb_i_has = 0, hb_t_has = 0;
            int64_t hb_i = px_opt_dur_ms(hb, "interval_ms", "ws_serve(heartbeat)", &hb_i_has);
            if (hb_i_has && hb_i > 0) g_ws_hb_interval = hb_i;
            int64_t hb_t = px_opt_dur_ms(hb, "timeout_ms", "ws_serve(heartbeat)", &hb_t_has);
            if (hb_t_has && hb_t > 0) g_ws_hb_timeout = hb_t;
        }
    }
    int port = (int)args[0].as.i;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) px_error("ws_serve: socket 创建失败");
    int one = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sfd);
        px_error("ws_serve: 绑定端口 %d 失败", port);
    }
    if (listen(sfd, 128) < 0) {
        close(sfd);
        px_error("ws_serve: listen 失败");
    }
    for (;;) {
        int cfd = px_io_accept(sfd, NULL, NULL);   // M256（缺陷 456）
        if (cfd < 0) continue;
        LXValue arg = px_int(cfd);
        px_spawn(ws_conn_worker, &arg, 1);
    }
    return px_null(); // 不可达
}

// ws 连接线程（px_spawn）：args[0] = fd。握手 → 注册 → handler(conn) → 保持到关闭。
LXValue ws_conn_worker(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) return px_null();
    int fd = (int)args[0].as.i;
    // M27：TLS 握手（若 tls_server 注册）→ PxConn 统一读写（堆分配共享给 ws_send 等）
    PxConn* c = (PxConn*)xmalloc(sizeof(PxConn));
    if (px_conn_init(c, fd) != 0) { xfree(c); return px_null(); }   // M240(d)：分配器配对
    g_cur_conn = c;
    __sync_fetch_and_add(&g_px_inflight, 1);
    // 1. 握手（M235：同时取回 path 与请求头原文）
    char* hs_path = NULL;
    char* hs_head = NULL;
    if (ws_server_handshake(c, &hs_path, &hs_head) < 0) {
        px_conn_close(c);  // 对象保留（closed 标记）
        __sync_fetch_and_sub(&g_px_inflight, 1);
        g_cur_conn = NULL;
        return px_null();
    }
    // 2. 注册
    pthread_mutex_lock(&g_ws_mu);
    int slot = ws_alloc_slot();
    if (slot < 0) {
        pthread_mutex_unlock(&g_ws_mu);
        px_conn_close(c);  // 对象保留
        __sync_fetch_and_sub(&g_px_inflight, 1);
        g_cur_conn = NULL;
        return px_null();
    }
    int64_t conn = g_ws_next_id++;
    g_ws_conns[slot].fd = fd;
    g_ws_conns[slot].id = conn;
    g_ws_conns[slot].active = 1;
    g_ws_conns[slot].client = 0;
    g_ws_conns[slot].closed = 0;
    g_ws_conns[slot].last_activity = 0;
    g_ws_conns[slot].hb_active = 0;
    g_ws_conns[slot].conn = c;
    // M235：随连接留存握手元信息（path / 请求头 / 对端地址）
    g_ws_conns[slot].path[0] = 0;
    if (hs_path) {
        snprintf(g_ws_conns[slot].path, sizeof(g_ws_conns[slot].path), "%s", hs_path);
        free(hs_path);
    }
    g_ws_conns[slot].hdrs = hs_head;   // 所有权移交（ws_alloc_slot 复用时归还）
    g_ws_conns[slot].peer[0] = 0;
    {
        struct sockaddr_storage ss;
        socklen_t sl = sizeof(ss);
        if (getpeername(fd, (struct sockaddr*)&ss, &sl) == 0) {
            char ip[64];
            ip[0] = 0;
            int prt = 0;
            if (ss.ss_family == AF_INET) {
                struct sockaddr_in* s4 = (struct sockaddr_in*)&ss;
                inet_ntop(AF_INET, &s4->sin_addr, ip, sizeof(ip));
                prt = ntohs(s4->sin_port);
            } else if (ss.ss_family == AF_INET6) {
                struct sockaddr_in6* s6 = (struct sockaddr_in6*)&ss;
                inet_ntop(AF_INET6, &s6->sin6_addr, ip, sizeof(ip));
                prt = ntohs(s6->sin6_port);
            }
            if (ip[0]) snprintf(g_ws_conns[slot].peer, sizeof(g_ws_conns[slot].peer), "%s:%d", ip, prt);
        }
    }
    pthread_mutex_unlock(&g_ws_mu);
    // M36：服务端自动心跳（ws_serve opts{heartbeat:{interval_ms,timeout_ms}}）
    if (g_ws_hb_interval > 0) {
        LXValue hb_args[3];
        hb_args[0] = px_int(conn);
        hb_args[1] = px_int(g_ws_hb_interval);
        hb_args[2] = px_int(g_ws_hb_timeout);
        (void)bi_ws_heartbeat(hb_args, 3, NULL);
    }
    // 3. 调 handler(conn)
    LXValue handler = px_get_global("__ws_handler");
    if (handler.type == PX_FUNC || handler.type == PX_NATIVE) {
        LXValue arg = px_int(conn);
        px_call(handler, &arg, 1);
    }
    // 4. 保持连接：读帧直到关闭（回复 ping / 响应 close），清理注册
    for (;;) {
        int fin = 0;
        unsigned char* payload = NULL;
        size_t plen = 0;
        int opcode = ws_read_frame(c, &fin, &payload, &plen);
        // 心跳：读到任何帧（含 pong）即更新最近活动时间
        {
            pthread_mutex_lock(&g_ws_mu);
            int a_idx = ws_find(conn);
            if (a_idx >= 0) g_ws_conns[a_idx].last_activity = ws_now_ms();
            pthread_mutex_unlock(&g_ws_mu);
        }
        if (opcode < 0) break;  // EOF / 错误 / shutdown
        if (opcode == WS_OP_PING) {
            ws_send_frame(c, WS_OP_PONG, payload, plen, 0);
        } else if (opcode == WS_OP_CLOSE) {
            ws_send_frame(c, WS_OP_CLOSE, NULL, 0, 0);
            break;
        }
        // 文本/二进制/继续帧：handler 已返回，丢弃
        free(payload);
        (void)fin;
    }
    // 5. 清理
    int closed = 0;
    pthread_mutex_lock(&g_ws_mu);
    for (int i = 0; i < MAX_WS_CONNS; i++) {
        if (g_ws_conns[i].active && g_ws_conns[i].fd == fd) {
            g_ws_conns[i].active = 0;
            g_ws_conns[i].fd = -1;
            closed = 1;
        }
    }
    pthread_mutex_unlock(&g_ws_mu);
    if (closed) {
        pthread_mutex_lock(&g_ws_mu);
        for (int j = 0; j < MAX_WS_CONNS; j++) {
            if (g_ws_conns[j].conn == c) {
                g_ws_conns[j].conn = NULL;
                ws_free_hs_locked(j);
            }
        }
        pthread_mutex_unlock(&g_ws_mu);
        px_conn_close(c);  // 对象保留（closed 标记）
        __sync_fetch_and_sub(&g_px_inflight, 1);
        g_cur_conn = NULL;
    }
    return px_null();
}

// ws_connect(host, port, path) → int conn | null（客户端握手）
// ws_connect(host, port, path) → int conn | null（客户端握手）
// M240（晨曦缺陷 A）：`gethostbyname()` 返回**进程级静态** `struct hostent`，不可重入。
//   `px_serve` 是 worker 池（本机 max_conn=32~64），而 `/agent/*` 每会话建链都要调
//   `ws_connect` ⇒ 多线程同进时静态对象被撕裂（实测 `h_length=4` 而 `h_addr_list[0]=NULL`）
//   ⇒ 紧接着的 `memcpy` 越界。`runtime.c` 自身早已是 `getaddrinfo` 口径（12 处），本文件
//   此前漏改 —— 本函数是这 4 处的**唯一**解析入口。
//   `host` 允许是 IPv4 字面量或域名（`AI_NUMERICHOST` 不开，两者都能过）。
// 返回 0 = 成功并写入 `out4`；非 0 = 解析失败（调用方按「连不上」返回 null）。
static int px_ws_resolve_v4(const char* host, struct in_addr* out4) {
    if (!host || !host[0] || !out4) return -1;
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          // 与既有 `sockaddr_in` 口径一致（本文件只支持 v4）
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, "0", &hints, &res) != 0 || !res) return -1;
    *out4 = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
    freeaddrinfo(res);
    return 0;
}

// ws_connect(host, port, path) → int conn | null（客户端握手）
// M32：支持一行连接 ws_connect("ws://host:port/path") / "wss://host:port/path"（wss 走 TLS）
LXValue bi_ws_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    const char* host = NULL;
    int port = 0;
    const char* path = NULL;
    int is_tls = 0;
    if (nargs == 1 && args[0].type == PX_STR) {
        // URL 形式：ws:// 或 wss://
        const char* url = args[0].as.obj->as.str.data;
        const char* rest;
        if (strncmp(url, "wss://", 6) == 0) { is_tls = 1; rest = url + 6; }
        else if (strncmp(url, "ws://", 5) == 0) { rest = url + 5; }
        else return px_null();
        const char* slash = strchr(rest, '/');
        int hl;
        if (slash) { hl = (int)(slash - rest); path = slash; }
        else { hl = (int)strlen(rest); path = "/"; }
        if (hl <= 0) return px_null();
        char hbuf[512];
        if (hl >= (int)sizeof(hbuf)) return px_null();
        memcpy(hbuf, rest, (size_t)hl);
        hbuf[hl] = 0;
        char* colon = strchr(hbuf, ':');
        port = is_tls ? 443 : 80;
        if (colon) {
            *colon = 0;
            port = atoi(colon + 1);
            if (port <= 0) return px_null();
        }
        host = hbuf;
        if (!*host) return px_null();
        // 注意：host 指向栈缓冲区，连接建立后不再使用 → 安全
        if (is_tls) {
            void* hs = px_https_connect_ex(host, port);
            if (!hs) return px_null();
            PxConn* cc = px_pxconn_from_https(hs);
            if (!cc) { px_https_close_ex(hs); return px_null(); }
            if (ws_client_handshake_px(cc, host, port, path) < 0) {
                px_https_close_ex(hs);
                return px_null();
            }
            int fd = px_https_fd_ex(hs);
            pthread_mutex_lock(&g_ws_mu);
            int slot = ws_alloc_slot();
            if (slot < 0) {
                pthread_mutex_unlock(&g_ws_mu);
                px_https_close_ex(hs);
                return px_null();
            }
            int64_t conn = g_ws_next_id++;
            g_ws_conns[slot].fd = fd;
            g_ws_conns[slot].id = conn;
            g_ws_conns[slot].active = 1;
            g_ws_conns[slot].client = 1;
            g_ws_conns[slot].closed = 0;
            g_ws_conns[slot].last_activity = 0;
            g_ws_conns[slot].hb_active = 0;
            g_ws_conns[slot].conn = cc;
            g_ws_conns[slot].hs = hs;
            pthread_mutex_unlock(&g_ws_mu);
            return px_int(conn);
        }
        // 明文 URL：连接后走下方共用注册
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return px_null();
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((uint16_t)port);
        if (px_ws_resolve_v4(host, &addr.sin_addr) != 0) { close(fd); return px_null(); }  // M240：无静态状态
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return px_null(); }
        if (ws_client_handshake(fd, host, port, path) < 0) { close(fd); return px_null(); }
        PxConn* cc = (PxConn*)xmalloc(sizeof(PxConn));
        px_conn_init_client(cc, fd);   // M240(g)：统一初始化（含 mutex）
        pthread_mutex_lock(&g_ws_mu);
        int slot = ws_alloc_slot();
        if (slot < 0) {
            pthread_mutex_unlock(&g_ws_mu);
            close(fd);
            return px_null();
        }
        int64_t conn = g_ws_next_id++;
        g_ws_conns[slot].fd = fd;
        g_ws_conns[slot].id = conn;
        g_ws_conns[slot].active = 1;
        g_ws_conns[slot].client = 1;
        g_ws_conns[slot].closed = 0;
        g_ws_conns[slot].last_activity = 0;
        g_ws_conns[slot].hb_active = 0;
        g_ws_conns[slot].conn = cc;
        g_ws_conns[slot].hs = NULL;
        pthread_mutex_unlock(&g_ws_mu);
        return px_int(conn);
    }
    if (nargs != 3 || args[0].type != PX_STR || args[1].type != PX_INT || args[2].type != PX_STR)
        px_error("R1002: ws_connect 需要 (url) 或 (host, port, path) 参数");
    host = args[0].as.obj->as.str.data;
    port = (int)args[1].as.i;
    path = args[2].as.obj->as.str.data;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return px_null();
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (px_ws_resolve_v4(host, &addr.sin_addr) != 0) { close(fd); return px_null(); }  // M240：无静态状态
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return px_null(); }
    if (ws_client_handshake(fd, host, port, path) < 0) { close(fd); return px_null(); }
    pthread_mutex_lock(&g_ws_mu);
    int slot = ws_alloc_slot();
    if (slot < 0) {
        pthread_mutex_unlock(&g_ws_mu);
        close(fd);
        return px_null();
    }
    PxConn* cc = (PxConn*)xmalloc(sizeof(PxConn));
    px_conn_init_client(cc, fd);   // M240(g)：统一初始化（含 mutex）
    int64_t conn = g_ws_next_id++;
    g_ws_conns[slot].fd = fd;
    g_ws_conns[slot].id = conn;
    g_ws_conns[slot].active = 1;
    g_ws_conns[slot].client = 1;
    g_ws_conns[slot].closed = 0;
    g_ws_conns[slot].last_activity = 0;
    g_ws_conns[slot].hb_active = 0;
    g_ws_conns[slot].conn = cc;
    g_ws_conns[slot].hs = NULL;
    pthread_mutex_unlock(&g_ws_mu);
    return px_int(conn);
}

// ws_send(conn, data) → bool
LXValue bi_ws_send(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("R1002: ws_send 需要 (conn, data) 参数");
    if (args[0].type != PX_INT) px_error("R1002: ws_send 的 conn 需要整数，实际是 %s", px_type_name(args[0]));
    // M193（缺陷 221 · 守卫不完整）：data 此前**完全不检查** ⇒ `ws_send(c, 2)` 会把 int 经
    //   val_cstr 串化成 "2" 静默发出（静默错值）。文本帧语义 = 字符串，故显式收口。
    if (args[1].type != PX_STR) px_error("R1002: ws_send 的 data 需要字符串，实际是 %s", px_type_name(args[1]));
    int64_t conn = args[0].as.i;
    int is_client = 0;
    PxConn* c = ws_get_conn(conn, &is_client);
    if (!c) { LXValue _lr = (px_bool(false)); px_conn_release(c); return _lr; }
    const char* data = px_val_cstr(args[1]);
    int ok = (ws_send_frame(c, WS_OP_TEXT, (const unsigned char*)data, strlen(data), is_client) == 0);
    if (!ok) {        // 写失败：标记关闭 + 清理
        pthread_mutex_lock(&g_ws_mu);
        int idx = ws_find(conn);
        if (idx >= 0) {
            g_ws_conns[idx].active = 0;
            g_ws_conns[idx].fd = -1;
            g_ws_conns[idx].conn = NULL;
            ws_free_hs_locked(idx);
            shutdown(c->fd, SHUT_RDWR);
            px_conn_close(c);  // 对象保留
        }
        pthread_mutex_unlock(&g_ws_mu);
    }
    { LXValue _lr = (px_bool(ok)); px_conn_release(c); return _lr; }
}

// M38：ws_connect_auto(url, reconnect_ms) → conn（断线自动重连；明文 ws:// 重连）
LXValue bi_ws_connect_auto(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_STR || args[1].type != PX_INT)
        px_error("R1002: ws_connect_auto 需要 (url, reconnect_ms) 参数");
    const char* url = args[0].as.obj->as.str.data;
    long long ms = args[1].as.i;
    if (ms <= 0) px_error("R1002: ws_connect_auto 的 reconnect_ms 需要正整数");
    if (strncmp(url, "wss://", 6) == 0) px_error("R1002: ws_connect_auto 暂支持 ws://（明文重连）");
    if (strncmp(url, "ws://", 5) != 0) return px_null();
    const char* rest = url + 5;
    const char* slash = strchr(rest, '/');
    char hbuf[512];
    int hl;
    const char* path = "/";
    if (slash) { hl = (int)(slash - rest); path = slash; }
    else hl = (int)strlen(rest);
    if (hl <= 0 || hl >= (int)sizeof(hbuf)) return px_null();
    memcpy(hbuf, rest, (size_t)hl); hbuf[hl] = 0;
    int port = 80;
    char* colon = strchr(hbuf, ':');
    if (colon) { *colon = 0; port = atoi(colon + 1); if (port <= 0) return px_null(); }
    if (!hbuf[0]) return px_null();
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return px_null();
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (px_ws_resolve_v4(hbuf, &addr.sin_addr) != 0) { close(fd); return px_null(); }  // M240：无静态状态
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return px_null(); }
    if (ws_client_handshake(fd, hbuf, port, path) < 0) { close(fd); return px_null(); }
    PxConn* cc = (PxConn*)xmalloc(sizeof(PxConn));
    px_conn_init_client(cc, fd);   // M240(g)：统一初始化（含 mutex）
    pthread_mutex_lock(&g_ws_mu);
    int slot = ws_alloc_slot();
    if (slot < 0) { pthread_mutex_unlock(&g_ws_mu); close(fd); xfree(cc); return px_null(); }   // M240(d)
    int64_t conn = g_ws_next_id++;
    g_ws_conns[slot].fd = fd;
    g_ws_conns[slot].id = conn;
    g_ws_conns[slot].active = 1;
    g_ws_conns[slot].client = 1;
    g_ws_conns[slot].closed = 0;
    g_ws_conns[slot].last_activity = 0;
    g_ws_conns[slot].hb_active = 0;
    g_ws_conns[slot].conn = cc;
    g_ws_conns[slot].hs = NULL;
    snprintf(g_ws_reconnect_url[slot], sizeof(g_ws_reconnect_url[0]), "%s", url);
    g_ws_reconnect_ms[slot] = ms;
    g_ws_reconnect_set[slot] = 1;
    pthread_mutex_unlock(&g_ws_mu);
    return px_int(conn);
}

// M34：ws_broadcast(data) → int —— 向 ws_serve 全部活跃**服务端**连接群发文本帧，返回成功数
// 只广播服务端连接（client=0）：避免回环（客户端连接也广播会把帧发回自身对端）
LXValue bi_ws_broadcast(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("R1002: ws_broadcast 需要 (data) 参数");
    // M195（缺陷 225）：与 M193 修过的 `ws_send` 的 data 同口径（同为**发送体**语义）
    const char* data = px_arg_str(args[0], "ws_broadcast", "data");
    size_t dlen = strlen(data);
    int ok = 0;
    pthread_mutex_lock(&g_ws_mu);
    for (int i = 0; i < MAX_WS_CONNS; i++) {
        if (!g_ws_conns[i].active || g_ws_conns[i].closed || g_ws_conns[i].fd < 0) continue;
        if (g_ws_conns[i].client) continue;  // M34：只广播服务端连接
        PxConn* c = g_ws_conns[i].conn;
        if (c && ws_send_frame(c, WS_OP_TEXT, (const unsigned char*)data, dlen, 0) == 0) ok++;
    }
    pthread_mutex_unlock(&g_ws_mu);
    return px_int(ok);
}

// ws_recv(conn) → str | null（阻塞读一条完整消息；自动重组分片、回复 ping）

// M38：客户端自动重连（ws_connect_auto 配置；明文 ws://）——重连并替换 slot 句柄
// 成功返回 1 且 *cpp 指向新连接；失败返回 0
static int ws_auto_reconnect(int64_t conn, PxConn** cpp) {
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(conn);
    long long rms = (idx >= 0) ? g_ws_reconnect_ms[idx] : 0;
    if (idx < 0 || !g_ws_reconnect_set[idx] || !g_ws_reconnect_url[idx][0]) {
        pthread_mutex_unlock(&g_ws_mu);
        return 0;
    }
    // 关闭旧连接
    if (g_ws_conns[idx].fd >= 0) { close(g_ws_conns[idx].fd); g_ws_conns[idx].fd = -1; }
    if (g_ws_conns[idx].conn) px_conn_close(g_ws_conns[idx].conn);
    g_ws_conns[idx].active = 0;
    pthread_mutex_unlock(&g_ws_mu);
    struct timespec ts = { rms / 1000, (rms % 1000) * 1000000L };
    px_io_sleep_ms(rms);   // M256（缺陷 456）
    pthread_mutex_lock(&g_ws_mu);
    // 重连窗口期 slot 保持 id（active 已清），直接用原 slot
    int s2 = idx;
    if (s2 < 0 || !g_ws_reconnect_url[s2][0]) { pthread_mutex_unlock(&g_ws_mu); return 0; }
    const char* ru = g_ws_reconnect_url[s2] + 5; // ws://
    const char* rslash = strchr(ru, '/');
    char rh[256]; int rhl;
    const char* rpath = "/";
    if (rslash) { rhl = (int)(rslash - ru); rpath = rslash; }
    else rhl = (int)strlen(ru);
    int ok = 0;
    if (rhl < 255) {
        memcpy(rh, ru, (size_t)rhl); rh[rhl] = 0;
        int rp = 80;
        char* rc = strchr(rh, ':');
        if (rc) { *rc = 0; rp = atoi(rc + 1); if (rp <= 0) rp = 80; }
        int nfd = socket(AF_INET, SOCK_STREAM, 0);
        if (nfd >= 0) {
            struct sockaddr_in ra;
            memset(&ra, 0, sizeof(ra));
            ra.sin_family = AF_INET;
            ra.sin_port = htons((uint16_t)rp);
            // M240（缺陷 A）：原实现 `memcpy(&ra.sin_addr, rh2 ? rh2->h_addr : NULL,
            //   rh2 ? rh2->h_length : 0)` —— 在 `gethostbyname` 被撕裂时 `h_length` 可能是
            //   垃圾值 ⇒ 越界写；解析失败时又是 NULL 源。统一走 helper（失败即不连）。
            int ra_ok = (px_ws_resolve_v4(rh, &ra.sin_addr) == 0);
            if (ra_ok && connect(nfd, (struct sockaddr*)&ra, sizeof(ra)) == 0 &&
                ws_client_handshake(nfd, rh, rp, rpath) == 0) {
                PxConn* nc = (PxConn*)xmalloc(sizeof(PxConn));
                px_conn_init_client(nc, nfd);   // M240(g)：统一初始化（含 mutex）
                g_ws_conns[s2].fd = nfd;
                g_ws_conns[s2].conn = nc;
                g_ws_conns[s2].active = 1;
                g_ws_conns[s2].closed = 0;
                pthread_mutex_unlock(&g_ws_mu);
                px_conn_close(*cpp);
                // M240（缺陷 B 之二配套）：调用方**原本持有的那个引用**要**接续到新对象**上
                //   （`ws_get_conn` 现在返回已 acquire 的指针 ⇒ 出口统一 release 的是它拿到的
                //   那个变量）。不 acquire 的话，出口的 release 会**透支**新对象的计数。
                px_conn_acquire(nc);
                *cpp = nc;
                ok = 1;
            } else {
                close(nfd);
                pthread_mutex_unlock(&g_ws_mu);
            }
        } else {
            pthread_mutex_unlock(&g_ws_mu);
        }
    } else {
        pthread_mutex_unlock(&g_ws_mu);
    }
    return ok;
}

LXValue bi_ws_recv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 2 || args[0].type != PX_INT)
        px_error("R1002: ws_recv 需要 (conn) 或 (conn, timeout_ms) 参数");
    int timeout_ms = -1;
    if (nargs == 2) {
        if (args[1].type != PX_INT) px_error("R1002: ws_recv 的 timeout_ms 必须是整数");
        timeout_ms = (int)args[1].as.i;
    }
    int64_t conn = args[0].as.i;
    int is_client = 0;
    PxConn* c = ws_get_conn(conn, &is_client);
    if (!c) { LXValue _lr = (px_null()); px_conn_release(c); return _lr; }
    unsigned char* msg = NULL;
    size_t mlen = 0, mcap = 0;
    for (;;) {
        // M23：可选超时 —— 每帧等待前 poll（控制帧/分片间同样生效）。
        // 超时返回 null（连接状态完好可继续使用）；对端关闭检测（HUP/ERR 且无数据）。
        // M27：TLS 读缓冲有数据则跳过 poll（缓冲已就绪，无需等底层可读）。
        if (timeout_ms >= 0 && !ws_conn_has_buffered(c)) {
            struct pollfd pfd;
            pfd.fd = c->fd;
            pfd.events = POLLIN;
            int pr = px_io_poll(&pfd, 1, timeout_ms);   // M256（缺陷 456）：EINTR 曾静默返回 null（丢帧）
            if (pr == 0) {
                // 超时：不标记关闭，连接完好
                if (msg) free(msg);
                { LXValue _lr = (px_null()); px_conn_release(c); return _lr; }
            }
            if (pr < 0) {
                if (msg) free(msg);
                { LXValue _lr = (px_null()); px_conn_release(c); return _lr; }
            }
            if ((pfd.revents & (POLLHUP | POLLERR)) && !(pfd.revents & POLLIN)) {
                if (msg) free(msg);
                // M38：客户端自动重连
                if (ws_auto_reconnect(conn, &c)) {
                    continue;
                }
                pthread_mutex_lock(&g_ws_mu);
                int idx2 = ws_find(conn);
                if (idx2 >= 0) { g_ws_conns[idx2].active = 0; g_ws_conns[idx2].fd = -1; g_ws_conns[idx2].conn = NULL; ws_free_hs_locked(idx2); }
                pthread_mutex_unlock(&g_ws_mu);
                px_conn_close(c);  // 对象保留
                { LXValue _lr = (px_null()); px_conn_release(c); return _lr; }
            }
        }
        int fin = 0;
        unsigned char* payload = NULL;
        size_t plen = 0;
        int opcode = ws_read_frame(c, &fin, &payload, &plen);
        // 心跳：读到任何帧（含 pong）即更新最近活动时间
        {
            pthread_mutex_lock(&g_ws_mu);
            int a_idx = ws_find(conn);
            if (a_idx >= 0) g_ws_conns[a_idx].last_activity = ws_now_ms();
            pthread_mutex_unlock(&g_ws_mu);
        }
        if (opcode < 0) {
            if (payload) free(payload);
            if (msg) free(msg);
            // 连接断开：清理注册
            pthread_mutex_lock(&g_ws_mu);
            int idx = ws_find(conn);
            if (idx >= 0) { g_ws_conns[idx].active = 0; g_ws_conns[idx].fd = -1; g_ws_conns[idx].conn = NULL;
            ws_free_hs_locked(idx); }
            pthread_mutex_unlock(&g_ws_mu);
            px_conn_close(c);  // 对象保留
            { LXValue _lr = (px_null()); px_conn_release(c); return _lr; }
        }
        if (opcode == WS_OP_PING) {
            ws_send_frame(c, WS_OP_PONG, payload, plen, is_client);
            free(payload);
            continue;
        }
        if (opcode == WS_OP_CLOSE) {
            ws_send_frame(c, WS_OP_CLOSE, NULL, 0, is_client);
            // M38：客户端自动重连（服务器主动断开场景）
            if (ws_auto_reconnect(conn, &c)) {
                free(payload);
                continue;
            }
            free(payload);
            if (msg) free(msg);
            pthread_mutex_lock(&g_ws_mu);
            int idx = ws_find(conn);
            if (idx >= 0) { g_ws_conns[idx].active = 0; g_ws_conns[idx].fd = -1; g_ws_conns[idx].conn = NULL;
            ws_free_hs_locked(idx); }
            pthread_mutex_unlock(&g_ws_mu);
            px_conn_close(c);  // 对象保留
            { LXValue _lr = (px_null()); px_conn_release(c); return _lr; }
        }
        if (opcode == WS_OP_TEXT || opcode == WS_OP_BINARY || opcode == WS_OP_CONT) {
            if (mlen + plen > mcap) {
                size_t ncap = mcap ? mcap * 2 : (plen + 64);
                if (ncap < mlen + plen) ncap = mlen + plen + 64;
                msg = (unsigned char*)realloc(msg, ncap);
                mcap = ncap;
            }
            memcpy(msg + mlen, payload, plen);
            mlen += plen;
            free(payload);
            if (fin) {
                LXValue r = px_str_len((const char*)msg, (int)mlen);
                free(msg);
                { LXValue _lr = (r); px_conn_release(c); return _lr; }
            }
        } else {
            free(payload);  // 忽略未知 opcode（含 PONG）
        }
    }
}

// ws_close(conn) → bool（发 close 帧 + shutdown 唤醒阻塞 recv）
LXValue bi_ws_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: ws_close 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    int is_client = 0;
    PxConn* c = ws_get_conn(conn, &is_client);
    if (!c) { LXValue _lr = (px_bool(false)); px_conn_release(c); return _lr; }
    ws_send_frame(c, WS_OP_CLOSE, (const unsigned char*)"\x03\xe8", 2, is_client);
    shutdown(c->fd, SHUT_RDWR);
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(conn);
    if (idx >= 0) {
        g_ws_conns[idx].active = 0;
        g_ws_conns[idx].fd = -1;
        g_ws_conns[idx].conn = NULL;
            ws_free_hs_locked(idx);
    }
    pthread_mutex_unlock(&g_ws_mu);
    px_conn_close(c);  // 对象保留
    { LXValue _lr = (px_bool(true)); px_conn_release(c); return _lr; }
}

// ==================== M235：ws 连接元信息 API ====================
// 这三个函数解决的是「握手信息读完即丢」这一个根因的两处表现（晨曦 ws-edge）：
//   · 多节点路由 `/agent/<节点>` 需要 path；
//   · 单 IP 限流 / 审计 / 白名单需要对端地址。
// 语义：连接不存在或信息缺失 ⇒ 返回空串（path/peer）/ null（header 未命中），
//   不抛错 —— 与 ws_send/ws_recv 的「连接不在 ⇒ 安静失败」口径一致。

// ws_conn_path(conn) → str（握手请求的 path；无则 ""）
LXValue bi_ws_conn_path(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: ws_conn_path 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    char buf[512];
    buf[0] = 0;
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(conn);
    if (idx >= 0) snprintf(buf, sizeof(buf), "%s", g_ws_conns[idx].path);
    pthread_mutex_unlock(&g_ws_mu);
    return px_str(buf);   // 分配在锁外（g_ws_mu 只保护注册表，不覆盖分配/GC）
}

// ws_conn_peer(conn) → str（对端 "ip:port"；无则 ""）
LXValue bi_ws_conn_peer(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: ws_conn_peer 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    char buf[72];
    buf[0] = 0;
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(conn);
    if (idx >= 0) snprintf(buf, sizeof(buf), "%s", g_ws_conns[idx].peer);
    pthread_mutex_unlock(&g_ws_mu);
    return px_str(buf);
}

// ws_conn_header(conn, name) → str | null（握手请求头，大小写不敏感；未命中 ⇒ null）
// 注意：返回的是**握手那一刻**的请求头（连接建立后客户端再发的帧不在此列）。
LXValue bi_ws_conn_header(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT || args[1].type != PX_STR)
        px_error("R1002: ws_conn_header 需要 (conn, name) 参数");
    int64_t conn = args[0].as.i;
    const char* nm = args[1].as.obj->as.str.data;
    char* found = NULL;
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(conn);
    if (idx >= 0 && g_ws_conns[idx].hdrs) found = ws_header_value(g_ws_conns[idx].hdrs, nm);
    pthread_mutex_unlock(&g_ws_mu);
    if (!found) return px_null();
    LXValue r = px_str(found);   // 同样在锁外构造
    free(found);
    return r;
}

// ws_ping(conn) → bool（发送 ping 帧；心跳保活，对端应回 pong）
LXValue bi_ws_ping(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: ws_ping 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    int is_client = 0;
    PxConn* c = ws_get_conn(conn, &is_client);
    if (!c) { LXValue _lr = (px_bool(false)); px_conn_release(c); return _lr; }
    if (ws_send_frame(c, WS_OP_PING, NULL, 0, is_client) < 0) {
        shutdown(c->fd, SHUT_RDWR);
        pthread_mutex_lock(&g_ws_mu);
        int idx = ws_find(conn);
        if (idx >= 0) { g_ws_conns[idx].active = 0; g_ws_conns[idx].fd = -1; g_ws_conns[idx].conn = NULL;
            ws_free_hs_locked(idx); }
        pthread_mutex_unlock(&g_ws_mu);
        px_conn_close(c);  // 对象保留
        { LXValue _lr = (px_bool(false)); px_conn_release(c); return _lr; }
    }
    { LXValue _lr = (px_bool(true)); px_conn_release(c); return _lr; }
}

// ==================== M26 ws_heartbeat：内置自动心跳 ====================
// ws_heartbeat(conn, interval_ms, timeout_ms) → bool
// - 每 interval_ms 发送 ping 帧保活（对端应回 pong；ws_recv 读到帧即刷新活动时间）；
// - 超过 timeout_ms 未读到任何帧（含 pong）→ 判定死链：shutdown+close 并清理注册
//   （阻塞中的 ws_recv 返回 null，应用层可感知断线）。
// 连接不存在/已关闭 → false；同连接重复调用 → true（已启动，不重复起线程）。
// 与解释器 ws.rs::ws_heartbeat 语义一致（双模式）。

struct ws_hb_arg {
    int64_t conn;
    long long interval;
    long long timeout;
};

static void* ws_heartbeat_thread(void* arg) {
    struct ws_hb_arg* a = (struct ws_hb_arg*)arg;
    int64_t conn = a->conn;
    long long interval = a->interval;
    long long timeout = a->timeout;
    free(a);
    for (;;) {
        struct timespec ts;
        ts.tv_sec = interval / 1000;
        ts.tv_nsec = (long)(interval % 1000) * 1000000L;
        px_io_sleep_ms(interval);   // M256（缺陷 456）
        pthread_mutex_lock(&g_ws_mu);
        int idx = ws_find(conn);
        if (idx < 0 || g_ws_conns[idx].closed) {
            if (idx >= 0) g_ws_conns[idx].hb_active = 0;
            pthread_mutex_unlock(&g_ws_mu);
            return NULL; // 连接已移除/关闭
        }
        int fd = g_ws_conns[idx].fd;
        PxConn* c = g_ws_conns[idx].conn;
        // M240（修复 f · 由 ASAN 直接指出）：**锁内 acquire**。
        //   本线程拿完指针就出锁做 IO，而 `px_conn_owner_free` / `px_conn_close` 可能
        //   在别的线程把对象释放 ⇒ 锁外那次 `ws_send_frame(c,…)` / `px_conn_close(c)`
        //   摸到已回收对象。ASAN 实测栈：
        //     `px_conn_free_res` ← `ws_heartbeat_thread`（SEGV on unknown address）
        //   口径与 `ws_get_conn` 一致：每次醒来 acquire 一次，本轮用完 release。
        if (c && !px_conn_acquire(c)) c = NULL;
        int is_client = g_ws_conns[idx].client;
        long long last = g_ws_conns[idx].last_activity;
        long long now = ws_now_ms();
        pthread_mutex_unlock(&g_ws_mu);
        // 1) 发 ping（不持锁做 IO）
        int pr = (c && !g_ws_conns[idx].closed) ? ws_send_frame(c, WS_OP_PING, NULL, 0, is_client) : -1;
        // 2) 超时检测：写失败 或 超时未活动 → 死链
        int dead = 0;
        if (pr < 0) {
            dead = 1;
        } else if (last > 0 && now - last > timeout) {
            dead = 1;
        }
        if (dead) {
            shutdown(fd, SHUT_RDWR);
            pthread_mutex_lock(&g_ws_mu);
            idx = ws_find(conn);
            if (idx >= 0) {
                g_ws_conns[idx].active = 0;
                g_ws_conns[idx].fd = -1;
                g_ws_conns[idx].conn = NULL;
            ws_free_hs_locked(idx);
                g_ws_conns[idx].hb_active = 0;
            }
            pthread_mutex_unlock(&g_ws_mu);
            if (c) px_conn_close(c);    // 对象保留（引用计数会推迟真正的释放）
            if (c) px_conn_release(c);  // M240（修复 f）：归还本次醒来的引用
            return NULL;
        }
        if (c) px_conn_release(c);      // M240（修复 f）：非 dead 路径同样要归还，否则对象永不释放
    }
}

LXValue bi_ws_heartbeat(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_INT)
        px_error("R1002: ws_heartbeat 需要 (conn, interval_ms, timeout_ms) 参数");
    int64_t conn = args[0].as.i;
    // M199（缺陷 237）：非 int 的间隔/超时此前**静默取默认值**（10000 / 60000）——
    //   用户以为设了心跳参数，实际拿到的是默认值（与 M198 的 opts.timeout_ms 静默忽略同族）。
    //   注意：**0 / 负数仍走下面的默认值分支**（那是文档化的「用默认」写法），只有类型错才响亮。
    if (args[1].type != PX_INT)
        px_error("R1002: ws_heartbeat 的 interval_ms 需要整数，实际是 %s", px_type_name(args[1]));
    if (args[2].type != PX_INT)
        px_error("R1002: ws_heartbeat 的 timeout_ms 需要整数，实际是 %s", px_type_name(args[2]));
    long long interval = args[1].as.i;
    long long timeout = args[2].as.i;
    if (interval <= 0) interval = 10000;
    if (timeout <= 0) timeout = 60000;
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(conn);
    if (idx < 0 || g_ws_conns[idx].closed) {
        pthread_mutex_unlock(&g_ws_mu);
        return px_bool(false);
    }
    if (g_ws_conns[idx].hb_active) {
        pthread_mutex_unlock(&g_ws_mu);
        return px_bool(true); // 已启动
    }
    g_ws_conns[idx].hb_active = 1;
    pthread_mutex_unlock(&g_ws_mu);
    struct ws_hb_arg* a = (struct ws_hb_arg*)malloc(sizeof(struct ws_hb_arg));
    a->conn = conn;
    a->interval = interval;
    a->timeout = timeout;
    pthread_t th;
    if (pthread_create(&th, NULL, ws_heartbeat_thread, a) != 0) {
        free(a);
        pthread_mutex_lock(&g_ws_mu);
        idx = ws_find(conn);
        if (idx >= 0) g_ws_conns[idx].hb_active = 0;
        pthread_mutex_unlock(&g_ws_mu);
        return px_bool(false);
    }
    pthread_detach(th);
    return px_bool(true);
}


// ==================== M236：HTTP 请求 → WebSocket 升级接管（晨曦特性请求）====================
// 需求与取舍见 docs/WS_UPGRADE.md。三句话：
//   ① 与既有 http_stream（SSE 同端口接管）**同构** —— 注册一个 path，命中即把连接交语言层；
//   ② 唯一不同的是「握手信息来源」：ws_serve 自己读请求头，而本路径的请求头**已被 HTTP 层
//      读掉**（px_conn_worker / http_conn_worker 里解析进 req）⇒ 从已解析的 req 取；
//   ③ 复用 M235 的连接元信息槽（path/hdrs/peer）+ 既有 ws_send/ws_recv/ws_close/ws_ping
//      ⇒ 语言层除「新增一行 ws_stream 注册」外零改动。
// 动机（晨曦 ws-edge · 2026-09-30 特性请求）：Ma（.px 服务）独占 443 且在同进程按 SNI/Host
//   分发多域名，另起进程占 443 或让 Ma 让出 443 都不理想 ⇒ 让运行时自己具备升级接管能力，
//   使 `wss://<节点>/agent/<节点名>` 能在**同一监听、同一进程**上完成（多节点路由）。

// 从 req.headers（dict）重建握手头原文（"Name: Value\r\n"…）。
// ws_conn_header 按该格式解析（ws_header_value）⇒ 重建后 M235 的三个元信息 API
// （ws_conn_path/ws_conn_header/ws_conn_peer）在 ws_stream 连接上零改动可用；
// bi_ws_reply_101 也靠它取回 Sec-WebSocket-Key。返回 malloc 串（可能为 ""）；失败 NULL。
static char* ws_rebuild_head_from_req(LXValue req) {
    LXValue hv = px_dict_get(req, "headers");
    if (hv.type != PX_DICT) return NULL;
    LXObject* ho = hv.as.obj;
    size_t cap = 128, n = 0;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    out[0] = 0;
    for (int i = 0; i < ho->as.dict.len; i++) {
        LXValue vv = ho->as.dict.vals[i];
        if (vv.type != PX_STR) continue;
        const char* k = ho->as.dict.keys[i];
        if (!k || !k[0]) continue;
        const char* val = vv.as.obj->as.str.data;
        size_t need = n + strlen(k) + strlen(val) + 4;
        if (need + 1 > cap) {
            size_t ncap = cap;
            while (ncap < need + 1) ncap *= 2;
            char* np = (char*)realloc(out, ncap);
            if (!np) { free(out); return NULL; }
            out = np; cap = ncap;
        }
        n += (size_t)snprintf(out + n, cap - n, "%s: %s\r\n", k, val);
    }
    return out;
}

// 本请求能不能被当成 WS 升级（= 带 Sec-WebSocket-Key）？供调用方**在建连接对象之前**判定，
//   避免「建了 PxConn 才发现不是 WS」的浪费与回退复杂度。
int px_ws_can_takeover(LXValue req) {
    LXValue hd = px_dict_get(req, "headers");
    if (hd.type != PX_DICT) return 0;
    LXValue kv = px_dict_get_ci(hd, "Sec-WebSocket-Key");
    if (kv.type != PX_STR) return 0;
    return kv.as.obj->as.str.data[0] ? 1 : 0;
}

// 写 101 Switching Protocols（运行时算 Sec-WebSocket-Accept）—— 内部用，锁外调用。
// 返回 0 成功。key 取自连接留存的握手头原文（hdrs）。
static int ws_write_101(PxConn* c, const char* hdrs) {
    char* key = hdrs ? ws_header_value(hdrs, "Sec-WebSocket-Key") : NULL;
    if (!key) return -1;
    char accept[64];
    ws_accept_key(key, accept);
    free(key);
    char resp[256];
    int rl = snprintf(resp, sizeof(resp),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n\r\n", accept);
    return (px_conn_write(c, resp, (size_t)rl) < 0) ? -1 : 0;
}

// ws_reply_101(conn) → bool（M236）
//   仅 `ws_stream(..., opts{"manual": true})` 端点需要：语言层做完准入决策后调用它完成握手
//   （accept 仍由运行时算，语言层不必自己 SHA1+base64）。
//   连接不是待握手的 ws_stream 连接 / 已写过 / 写失败 ⇒ false（不响亮：连接可能已被对端关掉）。
LXValue bi_ws_reply_101(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("R1002: ws_reply_101 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    char* hdrs = NULL;
    PxConn* c = NULL;
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(conn);
    if (idx >= 0 && g_ws_conns[idx].active && !g_ws_conns[idx].closed) {
        c = g_ws_conns[idx].conn;
        if (g_ws_conns[idx].hdrs) hdrs = strdup(g_ws_conns[idx].hdrs);
    }
    pthread_mutex_unlock(&g_ws_mu);
    if (!c || !hdrs) { if (hdrs) free(hdrs); return px_bool(false); }
    int rc = ws_write_101(c, hdrs);
    free(hdrs);
    return px_bool(rc == 0);
}

// 把「已解析的 HTTP 请求连接」升级为 WS 连接并交给语言层回调。
//   · c    —— 已建好并完成 TLS 握手（若有）的连接对象（px_serve 轨用现成的；http_serve 轨
//             由调用方按 stream_takeover_conn 同法新建）
//   · req  —— 已解析的请求 dict（含 headers/path；GC 根由调用方保证）
//   · path —— 请求 path（用于 ws_conn_path）
//   · route_idx —— ws_stream 注册下标；回调经全局键 `__wsstream_fn_<idx>` 取回
//   · manual —— 1 = **不写 101**，由语言层调 ws_reply_101(conn) 完成握手（先决策后握手，
//               用于鉴权/子协议协商）；0 = 运行时立即写 101（与 ws_serve 同序）
// 返回 0 = 已接管并**已收尾**（调用方不得再 close/释放该连接，也不再落 HTTP 管道）；
//      -1 = 未接管（缺 Sec-WebSocket-Key / 无槽 / 写 101 失败 ⇒ 调用方走原路径）。
// ⚠️ 阻塞语义：本函数内含「注册 → 101 → 回调 → 泵循环 → 注销」全过程，与 http_stream 的
//    SSE 接管同构 ⇒ WS 会话期间占用调用方一个 worker 线程（量化见 docs/WS_UPGRADE.md §4）。
int px_ws_takeover_http_conn(PxConn* c, LXValue req, const char* path, int route_idx, int manual) {
    if (!c) return -1;
    if (!px_ws_can_takeover(req)) return -1;

    // 1. 注册（先注册、后写 101 —— 与 ws_conn_worker 同序；槽满时按未接管返回，调用方走原路径）
    char* hdr_raw = ws_rebuild_head_from_req(req);
    int fd = c->fd;
    pthread_mutex_lock(&g_ws_mu);
    int slot = ws_alloc_slot();
    if (slot < 0) {
        pthread_mutex_unlock(&g_ws_mu);
        if (hdr_raw) free(hdr_raw);
        return -1;
    }
    int64_t conn = g_ws_next_id++;
    g_ws_conns[slot].fd = fd;
    g_ws_conns[slot].id = conn;
    g_ws_conns[slot].active = 1;
    g_ws_conns[slot].client = 0;
    g_ws_conns[slot].closed = 0;
    g_ws_conns[slot].last_activity = ws_now_ms();
    g_ws_conns[slot].hb_active = 0;
    g_ws_conns[slot].conn = c;      // 共享 PxConn（明文/TLS 统一读写）
    g_ws_conns[slot].path[0] = 0;
    if (path) snprintf(g_ws_conns[slot].path, sizeof(g_ws_conns[slot].path), "%s", path);
    g_ws_conns[slot].hdrs = hdr_raw;   // 所有权移交（槽复用时由 ws_free_hs_locked 归还）
    g_ws_conns[slot].peer[0] = 0;
    {
        struct sockaddr_storage ss;
        socklen_t sl = sizeof(ss);
        if (getpeername(fd, (struct sockaddr*)&ss, &sl) == 0) {
            char ip[64];
            ip[0] = 0;
            int prt = 0;
            if (ss.ss_family == AF_INET) {
                struct sockaddr_in* s4 = (struct sockaddr_in*)&ss;
                inet_ntop(AF_INET, &s4->sin_addr, ip, sizeof(ip));
                prt = ntohs(s4->sin_port);
            } else if (ss.ss_family == AF_INET6) {
                struct sockaddr_in6* s6 = (struct sockaddr_in6*)&ss;
                inet_ntop(AF_INET6, &s6->sin6_addr, ip, sizeof(ip));
                prt = ntohs(s6->sin6_port);
            }
            if (ip[0])
                snprintf(g_ws_conns[slot].peer, sizeof(g_ws_conns[slot].peer), "%s:%d", ip, prt);
        }
    }
    pthread_mutex_unlock(&g_ws_mu);

    // 2. 非 manual ⇒ 立即写 101（锁外）。写不出 ⇒ 撤注册、按未接管返回（不留悬挂项）
    if (!manual) {
        if (ws_write_101(c, hdr_raw) != 0) {
            pthread_mutex_lock(&g_ws_mu);
            int ix = ws_find(conn);
            if (ix >= 0) {
                g_ws_conns[ix].active = 0; g_ws_conns[ix].fd = -1; g_ws_conns[ix].conn = NULL;
                ws_free_hs_locked(ix);
            }
            pthread_mutex_unlock(&g_ws_mu);
            return -1;
        }
    }

    // 3. 交给语言层：fn(conn, req) —— 两个实参（与晨曦建议的 fn(conn, req) 同形）
    char gk[64];
    snprintf(gk, sizeof(gk), "__wsstream_fn_%d", route_idx);
    LXValue fn = px_get_global(gk);
    if (fn.type == PX_FUNC || fn.type == PX_NATIVE) {
        LXValue cargs[2];
        cargs[0] = px_int(conn);
        cargs[1] = req;
        LXValue cap_ret = px_null();
        char emsg[256];
        emsg[0] = 0;
        // 错误边界（M92-S2c/M98 同口径）：语言层异常**不得** longjmp 越过本函数的清理路径
        //   —— px_native_call_capture 自带隔离点（M170 的 px_root_iso_mark/restore_iso）。
        if (px_native_call_capture(fn, cargs, 2, &cap_ret, emsg, (int)sizeof(emsg))) {
            fprintf(stderr, "[ws-stream] 回调出错（conn=%lld）：%s\n",
                    (long long)conn, emsg[0] ? emsg : "未知错误");
        }
    }

    // 4. 泵循环：回调返回后保持连接，直到对端关闭（回 ping / 响应 close）。
    //    与 ws_conn_worker 第 4 步**逐句同构**（语言层若已在回调里自行 ws_recv 循环，
    //    本步通常立即读到 close/EOF 而退出）。
    for (;;) {
        int fin = 0;
        unsigned char* payload = NULL;
        size_t plen = 0;
        int opcode = ws_read_frame(c, &fin, &payload, &plen);
        {
            pthread_mutex_lock(&g_ws_mu);
            int a_idx = ws_find(conn);
            if (a_idx >= 0) g_ws_conns[a_idx].last_activity = ws_now_ms();
            pthread_mutex_unlock(&g_ws_mu);
        }
        if (opcode < 0) break;   // EOF / 错误 / shutdown
        if (opcode == WS_OP_PING) {
            ws_send_frame(c, WS_OP_PONG, payload, plen, 0);
        } else if (opcode == WS_OP_CLOSE) {
            ws_send_frame(c, WS_OP_CLOSE, NULL, 0, 0);
            break;
        }
        free(payload);
        (void)fin;
    }

    // 5. 注销注册项（连接本身的关闭/释放由**调用方**按既有路径收尾：px_pxpend_close /
    //    px_conn_close，含 M235 的引用计数语义 —— 本函数不 close，避免双重释放）。
    pthread_mutex_lock(&g_ws_mu);
    {
        int ix = ws_find(conn);
        if (ix >= 0) {
            g_ws_conns[ix].active = 0;
            g_ws_conns[ix].fd = -1;
            g_ws_conns[ix].conn = NULL;
            ws_free_hs_locked(ix);
            g_ws_conns[ix].hb_active = 0;
        }
    }
    pthread_mutex_unlock(&g_ws_mu);
    return 0;
}
