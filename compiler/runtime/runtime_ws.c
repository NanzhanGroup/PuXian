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

// ==================== 连接注册表 ====================
#define MAX_WS_CONNS 256
static pthread_mutex_t g_ws_mu = PTHREAD_MUTEX_INITIALIZER;
static struct {
    int fd;            // socket
    int64_t id;        // conn id
    int active;        // 槽位占用
    int client;        // 1 = 客户端连接（发送需掩码）；0 = 服务端连接
    int closed;        // 已发送/收到 close（停止使用）
} g_ws_conns[MAX_WS_CONNS];
static int64_t g_ws_next_id = 1;

static int ws_find(int64_t id) {
    for (int i = 0; i < MAX_WS_CONNS; i++) {
        if (g_ws_conns[i].active && g_ws_conns[i].id == id) return i;
    }
    return -1;
}

static int ws_alloc_slot(void) {
    for (int i = 0; i < MAX_WS_CONNS; i++) {
        if (!g_ws_conns[i].active) return i;
    }
    return -1;
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
static int ws_read_exact(int fd, unsigned char* buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t r = recv(fd, buf + off, n - off, 0);
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
static int ws_read_frame(int fd, int* fin, unsigned char** payload, size_t* plen) {
    unsigned char h[2];
    if (ws_read_exact(fd, h, 2) < 0) return -1;
    *fin = (h[0] & 0x80) ? 1 : 0;
    int opcode = h[0] & 0x0F;
    int masked = (h[1] & 0x80) ? 1 : 0;
    uint64_t len = h[1] & 0x7F;
    if (len == 126) {
        unsigned char ext[2];
        if (ws_read_exact(fd, ext, 2) < 0) return -1;
        len = ((uint64_t)ext[0] << 8) | ext[1];
    } else if (len == 127) {
        unsigned char ext[8];
        if (ws_read_exact(fd, ext, 8) < 0) return -1;
        len = 0;
        for (int i = 0; i < 8; i++) len = (len << 8) | ext[i];
    }
    if (len > 64ULL * 1024 * 1024) return -1;
    unsigned char mask[4] = {0, 0, 0, 0};
    if (masked && ws_read_exact(fd, mask, 4) < 0) return -1;
    unsigned char* p = (unsigned char*)malloc((size_t)len + 1);
    if (len > 0) {
        if (ws_read_exact(fd, p, (size_t)len) < 0) { free(p); return -1; }
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
static int ws_send_frame(int fd, int opcode, const unsigned char* data, size_t len, int mask_out) {
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
        if (send(fd, hdr, (int)hl, MSG_NOSIGNAL) < 0) { free(tmp); return -1; }
        if (len > 0 && send(fd, tmp, (int)len, MSG_NOSIGNAL) < 0) { free(tmp); return -1; }
        free(tmp);
        return 0;
    }
    if (send(fd, hdr, (int)hl, MSG_NOSIGNAL) < 0) return -1;
    if (len > 0 && send(fd, data, (int)len, MSG_NOSIGNAL) < 0) return -1;
    return 0;
}

// ==================== 握手（RFC 6455 §4.2） ====================
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

// 读 HTTP 头直到 \r\n\r\n（上限 64KB）。返回 0 成功；buf/len 出参（malloc）。
static int ws_read_http_header(int fd, char** out, int* out_len) {
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
static int ws_server_handshake(int fd) {
    char* head = NULL;
    int hlen = 0;
    if (ws_read_http_header(fd, &head, &hlen) < 0) return -1;
    if (strncmp(head, "GET ", 4) != 0) { free(head); return -1; }
    char* key = ws_header_value(head, "Sec-WebSocket-Key");
    char* upgrade = ws_header_value(head, "Upgrade");
    free(head);
    if (!key || !upgrade || strcasecmp(upgrade, "websocket") != 0) {
        if (key) free(key);
        if (upgrade) free(upgrade);
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
    if (send(fd, resp, rl, MSG_NOSIGNAL) < 0) return -1;
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
    if (ws_read_http_header(fd, &head, &hlen) < 0) return -1;
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
LXValue bi_ws_serve(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT) px_error("ws_serve 需要 (port, handler) 参数");
    LXValue handler = args[1];
    if (handler.type != PX_FUNC && handler.type != PX_NATIVE) px_error("ws_serve 的 handler 必须是函数");
    px_set_global("__ws_handler", handler);
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
        int cfd = accept(sfd, NULL, NULL);
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
    // 1. 握手
    if (ws_server_handshake(fd) < 0) { close(fd); return px_null(); }
    // 2. 注册
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
    g_ws_conns[slot].client = 0;
    g_ws_conns[slot].closed = 0;
    pthread_mutex_unlock(&g_ws_mu);
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
        int opcode = ws_read_frame(fd, &fin, &payload, &plen);
        if (opcode < 0) break;  // EOF / 错误 / shutdown
        if (opcode == WS_OP_PING) {
            ws_send_frame(fd, WS_OP_PONG, payload, plen, 0);
        } else if (opcode == WS_OP_CLOSE) {
            ws_send_frame(fd, WS_OP_CLOSE, NULL, 0, 0);
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
    if (closed) close(fd);
    return px_null();
}

// ws_connect(host, port, path) → int conn | null（客户端握手）
LXValue bi_ws_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3 || args[0].type != PX_STR || args[1].type != PX_INT || args[2].type != PX_STR)
        px_error("ws_connect 需要 (host, port, path) 参数");
    const char* host = args[0].as.obj->as.str.data;
    int port = (int)args[1].as.i;
    const char* path = args[2].as.obj->as.str.data;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return px_null();
    struct hostent* he = gethostbyname(host);
    if (!he) { close(fd); return px_null(); }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    memcpy(&addr.sin_addr, he->h_addr, (size_t)he->h_length);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return px_null(); }
    if (ws_client_handshake(fd, host, port, path) < 0) { close(fd); return px_null(); }
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
    pthread_mutex_unlock(&g_ws_mu);
    return px_int(conn);
}

// ws_send(conn, data) → bool
LXValue bi_ws_send(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2 || args[0].type != PX_INT) px_error("ws_send 需要 (conn, data) 参数");
    int64_t conn = args[0].as.i;
    int is_client = 0;
    int fd = ws_get_fd(conn, &is_client);
    if (fd < 0) return px_bool(false);
    const char* data = px_val_cstr(args[1]);
    int ok = (ws_send_frame(fd, WS_OP_TEXT, (const unsigned char*)data, strlen(data), is_client) == 0);
    if (!ok) {
        // 写失败：标记关闭 + 清理
        pthread_mutex_lock(&g_ws_mu);
        int idx = ws_find(conn);
        if (idx >= 0) {
            g_ws_conns[idx].active = 0;
            g_ws_conns[idx].fd = -1;
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
        pthread_mutex_unlock(&g_ws_mu);
    }
    return px_bool(ok);
}

// ws_recv(conn) → str | null（阻塞读一条完整消息；自动重组分片、回复 ping）
LXValue bi_ws_recv(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || nargs > 2 || args[0].type != PX_INT)
        px_error("ws_recv 需要 (conn) 或 (conn, timeout_ms) 参数");
    int timeout_ms = -1;
    if (nargs == 2) {
        if (args[1].type != PX_INT) px_error("ws_recv 的 timeout_ms 必须是整数");
        timeout_ms = (int)args[1].as.i;
    }
    int64_t conn = args[0].as.i;
    int is_client = 0;
    int fd = ws_get_fd(conn, &is_client);
    if (fd < 0) return px_null();
    unsigned char* msg = NULL;
    size_t mlen = 0, mcap = 0;
    for (;;) {
        // M23：可选超时 —— 每帧等待前 poll（控制帧/分片间同样生效）。
        // 超时返回 null（连接状态完好可继续使用）；对端关闭检测（HUP/ERR 且无数据）。
        if (timeout_ms >= 0) {
            struct pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLIN;
            int pr = poll(&pfd, 1, timeout_ms);
            if (pr == 0) {
                // 超时：不标记关闭，连接完好
                if (msg) free(msg);
                return px_null();
            }
            if (pr < 0) {
                if (msg) free(msg);
                return px_null();
            }
            if ((pfd.revents & (POLLHUP | POLLERR)) && !(pfd.revents & POLLIN)) {
                if (msg) free(msg);
                pthread_mutex_lock(&g_ws_mu);
                int idx = ws_find(conn);
                if (idx >= 0) { g_ws_conns[idx].active = 0; g_ws_conns[idx].fd = -1; }
                pthread_mutex_unlock(&g_ws_mu);
                close(fd);
                return px_null();
            }
        }
        int fin = 0;
        unsigned char* payload = NULL;
        size_t plen = 0;
        int opcode = ws_read_frame(fd, &fin, &payload, &plen);
        if (opcode < 0) {
            if (payload) free(payload);
            if (msg) free(msg);
            // 连接断开：清理注册
            pthread_mutex_lock(&g_ws_mu);
            int idx = ws_find(conn);
            if (idx >= 0) { g_ws_conns[idx].active = 0; g_ws_conns[idx].fd = -1; close(fd); }
            pthread_mutex_unlock(&g_ws_mu);
            return px_null();
        }
        if (opcode == WS_OP_PING) {
            ws_send_frame(fd, WS_OP_PONG, payload, plen, is_client);
            free(payload);
            continue;
        }
        if (opcode == WS_OP_CLOSE) {
            ws_send_frame(fd, WS_OP_CLOSE, NULL, 0, is_client);
            free(payload);
            if (msg) free(msg);
            pthread_mutex_lock(&g_ws_mu);
            int idx = ws_find(conn);
            if (idx >= 0) { g_ws_conns[idx].active = 0; g_ws_conns[idx].fd = -1; close(fd); }
            pthread_mutex_unlock(&g_ws_mu);
            return px_null();
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
                return r;
            }
        } else {
            free(payload);  // 忽略未知 opcode（含 PONG）
        }
    }
}

// ws_close(conn) → bool（发 close 帧 + shutdown 唤醒阻塞 recv）
LXValue bi_ws_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("ws_close 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    int is_client = 0;
    int fd = ws_get_fd(conn, &is_client);
    if (fd < 0) return px_bool(false);
    ws_send_frame(fd, WS_OP_CLOSE, (const unsigned char*)"\x03\xe8", 2, is_client);
    shutdown(fd, SHUT_RDWR);
    pthread_mutex_lock(&g_ws_mu);
    int idx = ws_find(conn);
    if (idx >= 0) {
        g_ws_conns[idx].active = 0;
        g_ws_conns[idx].fd = -1;
    }
    pthread_mutex_unlock(&g_ws_mu);
    close(fd);
    return px_bool(true);
}

// ws_ping(conn) → bool（发送 ping 帧；心跳保活，对端应回 pong）
LXValue bi_ws_ping(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("ws_ping 需要 (conn) 参数");
    int64_t conn = args[0].as.i;
    int is_client = 0;
    int fd = ws_get_fd(conn, &is_client);
    if (fd < 0) return px_bool(false);
    if (ws_send_frame(fd, WS_OP_PING, NULL, 0, is_client) < 0) {
        shutdown(fd, SHUT_RDWR);
        close(fd);
        pthread_mutex_lock(&g_ws_mu);
        int idx = ws_find(conn);
        if (idx >= 0) { g_ws_conns[idx].active = 0; g_ws_conns[idx].fd = -1; }
        pthread_mutex_unlock(&g_ws_mu);
        return px_bool(false);
    }
    return px_bool(true);
}
