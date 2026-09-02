// 普贤 (PuXian) C 运行时 — HTTP/3 语义层（M47）
// ------------------------------------------------------------
// 目标：在 M46 QUIC 传输 API（单条双向流）之上实现 HTTP/3 语义 MVP：
//   QPACK 头压缩（RFC 9204 无动态表子集）+ HTTP/3 帧（HEADERS/DATA）
//   + 请求/响应对拍（不依赖 FIN，MVP 约定单 DATA 帧界定消息）。
// 语言层 API（extern def，双模式一致，注册进 FFI 表 + 全局）：
//   h3_qenc(headers:list) -> bytes                 # QPACK 编码字段段（纯 codec，测试用）
//   h3_qdec(data:bytes) -> list                    # QPACK 解码字段段
//   h3_frame(type:int, payload) -> bytes           # 构造一个 H3 帧
//   h3_serve_read_request(conn, timeout_ms) -> dict|null
//   h3_serve_send_response(conn, status, headers, body) -> bool
//   h3_client_connect(ip, port, alpn) -> int
//   h3_client_send_request(conn, method, scheme, authority, path, headers, body) -> bool
//   h3_client_read_response(conn, timeout_ms) -> dict|null
// 说明：底层收发复用 runtime_quic.c 导出的 px_quic_raw_*（同一连接/流）。
// 连接级接收缓冲按 conn id 维护（h3_take_frame 消费式读取，支持分片到达）。
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define H3_MAX_CONN 64
#define H3_FRAME_HEADERS 0x01
#define H3_FRAME_DATA    0x00
#define H3_BUF_MAX (1 << 20)   // 单帧/消息上限 1MB（MVP）
#define H3_METHOD ":method"
#define H3_SCHEME ":scheme"
#define H3_AUTH   ":authority"
#define H3_PATH   ":path"
#define H3_STATUS ":status"

// ==================== QUIC varint（RFC 9000 §16）====================
static int h3_varint_enc(uint8_t* out, uint64_t v) {
    if (v < (1ULL << 6)) { out[0] = (uint8_t)v; return 1; }
    if (v < (1ULL << 14)) {
        out[0] = (uint8_t)(0x40 | (uint8_t)(v >> 8)); out[1] = (uint8_t)v; return 2;
    }
    if (v < (1ULL << 30)) {
        out[0] = (uint8_t)(0x80 | (uint8_t)(v >> 24));
        out[1] = (uint8_t)((v >> 16) & 0xff);
        out[2] = (uint8_t)((v >> 8) & 0xff);
        out[3] = (uint8_t)(v & 0xff);
        return 4;
    }
    out[0] = (uint8_t)(0xc0 | (uint8_t)(v >> 56));
    out[1] = (uint8_t)((v >> 48) & 0xff);
    out[2] = (uint8_t)((v >> 40) & 0xff);
    out[3] = (uint8_t)((v >> 32) & 0xff);
    out[4] = (uint8_t)((v >> 24) & 0xff);
    out[5] = (uint8_t)((v >> 16) & 0xff);
    out[6] = (uint8_t)((v >> 8) & 0xff);
    out[7] = (uint8_t)(v & 0xff);
    return 8;
}

// 解码 varint：成功返回消耗字节数；数据不足返回 0；非法返回 -1
static int h3_varint_dec(const uint8_t* p, int maxlen, uint64_t* out) {
    if (maxlen < 1) return 0;
    uint8_t first = p[0];
    if ((first & 0xc0) == 0x00) { *out = first; return 1; }
    if ((first & 0xc0) == 0x40) {
        if (maxlen < 2) return 0;
        *out = ((uint64_t)(first & 0x3f) << 8) | p[1];
        return 2;
    }
    if ((first & 0xc0) == 0x80) {
        if (maxlen < 4) return 0;
        *out = ((uint64_t)(first & 0x3f) << 24) | ((uint64_t)p[1] << 16) |
               ((uint64_t)p[2] << 8) | p[3];
        return 4;
    }
    if (maxlen < 8) return 0;
    *out = ((uint64_t)(first & 0x3f) << 56) | ((uint64_t)p[1] << 48) |
           ((uint64_t)p[2] << 40) | ((uint64_t)p[3] << 32) | ((uint64_t)p[4] << 24) |
           ((uint64_t)p[5] << 16) | ((uint64_t)p[6] << 8) | p[7];
    return 8;
}

// ==================== QPACK prefix integer（RFC 7541 §5.1 / RFC 9204 §4.1.1）====================
// 编码：把 value 写入首字节（高 8-prefix_bits 位为 first_hi）+ 续字节。
// first_hi 是调用方已构造的高位（如 0x20 表示 001 模式 + N/H 位）。
// 返回写入总字节数。
static int qp_enc_prefint(uint8_t* out, int prefix_bits, uint64_t value) {
    uint64_t maxv = (1ULL << prefix_bits) - 1;
    if (value < maxv) { out[0] = (uint8_t)value; return 1; }
    out[0] = (uint8_t)maxv;
    uint64_t rest = value - maxv;
    int n = 1;
    while (rest >= 128) {
        out[n++] = (uint8_t)((rest & 0x7f) | 0x80);
        rest >>= 7;
    }
    out[n++] = (uint8_t)rest;
    return n;
}

// 解码 prefix integer：p 指向首字节（其高 8-prefix_bits 位已在调用处校验/忽略）。
// 返回 [值, 消耗字节数]；数据不足消耗=0。
static uint64_t qp_dec_prefint(const uint8_t* p, int maxlen, int prefix_bits, int* used) {
    if (maxlen < 1) { *used = 0; return 0; }
    uint64_t maxv = (1ULL << prefix_bits) - 1;
    uint64_t v = p[0] & maxv;
    if (v < maxv) { *used = 1; return v; }
    int i = 1;
    int shift = 0;
    while (i < maxlen) {
        uint8_t b = p[i++];
        v += (uint64_t)(b & 0x7f) << shift;
        if ((b & 0x80) == 0) { *used = i; return v; }
        shift += 7;
    }
    *used = 0;
    return 0;
}

// ==================== QPACK 字段段（MVP：Literal Field Line with Literal Name，无 Huffman/动态表）====================
// 字段行：001 N H | NameLen(3+)  name...  H | ValueLen(7+)  value...
//   N=0（不敏感），H=0（不用 Huffman）。前缀：Required Insert Count=0 + Base=0 → 0x00 0x00。
// 返回写入字节数。
static int qp_enc_field(uint8_t* out, const char* name, int nlen, const char* val, int vlen) {
    int off = 0;
    uint8_t tmp[16];
    int n = qp_enc_prefint(tmp, 3, (uint64_t)nlen);
    out[off++] = (uint8_t)(0x20 | tmp[0]);   // 001 N=0 H=0 | len 低 3 位
    for (int i = 1; i < n; i++) out[off++] = tmp[i];
    memcpy(out + off, name, (size_t)nlen); off += nlen;
    n = qp_enc_prefint(tmp, 7, (uint64_t)vlen);
    out[off++] = tmp[0];                     // H=0 | len 低 7 位
    for (int i = 1; i < n; i++) out[off++] = tmp[i];
    memcpy(out + off, val, (size_t)vlen); off += vlen;
    return off;
}

// 编码字段段（2 字节前缀 + 字段行）。fields: list of [name,value]（PX_LIST）
static int qp_enc_section(uint8_t* out, LXValue* fields, int nf) {
    int off = 0;
    out[off++] = 0x00;  // Required Insert Count = 0
    out[off++] = 0x00;  // Base = 0（S=0, Delta=0）
    for (int i = 0; i < nf; i++) {
        if (fields[i].type != PX_LIST && fields[i].type != PX_TUPLE) return -1;
        LXObject* f = fields[i].as.obj;
        if (f->as.list.len < 2) return -1;
        LXValue* kv = f->as.list.items;
        if (kv[0].type != PX_STR || kv[1].type != PX_STR) return -1;
        const char* nm = kv[0].as.obj->as.str.data; int nml = kv[0].as.obj->as.str.len;
        const char* vl = kv[1].as.obj->as.str.data; int vll = kv[1].as.obj->as.str.len;
        if (off + nml + vll + 8 > H3_BUF_MAX) return -1;
        off += qp_enc_field(out + off, nm, nml, vl, vll);
    }
    return off;
}

// 解码字段段 → list of [name,value]。失败返回 null。
static LXValue qp_dec_section(const uint8_t* p, int len) {
    int off = 0;
    int used = 0;
    if (len < 2) return px_null();
    uint64_t ric = qp_dec_prefint(p, len, 8, &used);   // Required Insert Count（8-bit prefix）
    if (used == 0) return px_null();
    off += used;
    if (off >= len) return px_null();
    uint64_t base = qp_dec_prefint(p + off, len - off, 7, &used);  // Base（S+7）
    if (used == 0) return px_null();
    off += used;
    if (ric != 0) return px_null();   // MVP：动态表未启用
    LXValue fields = px_list(8);
    while (off < len) {
        uint8_t b0 = p[off];
        if ((b0 & 0xe0) != 0x20) return px_null();   // 001 模式（MVP 仅 Literal Name）
        // b0 bit4=N, bit3=H
        int H = (b0 >> 3) & 1;
        if (H != 0) return px_null();                // 不支持 Huffman
        uint64_t nlen = qp_dec_prefint(p + off, len - off, 3, &used);
        if (used == 0) return px_null();
        off += used;
        if (off + (int)nlen > len) return px_null();
        const char* nm = (const char*)(p + off);
        off += (int)nlen;
        if (off >= len) return px_null();
        uint8_t v0 = p[off];
        if ((v0 & 0x80) != 0) return px_null();      // H=0
        uint64_t vlen = qp_dec_prefint(p + off, len - off, 7, &used);
        if (used == 0) return px_null();
        off += used;
        if (off + (int)vlen > len) return px_null();
        LXValue pair = px_list(2);
        px_list_push(pair, px_str_len(nm, (int)nlen));
        px_list_push(pair, px_str_len((const char*)(p + off), (int)vlen));
        px_list_push(fields, pair);
        off += (int)vlen;
    }
    return fields;
}

// ==================== HTTP/3 帧 ====================
static int h3_build_frame(uint8_t* out, int type, const uint8_t* payload, int plen) {
    int off = 0;
    uint8_t t[8], l[8];
    int tn = h3_varint_enc(t, (uint64_t)type);
    int ln = h3_varint_enc(l, (uint64_t)plen);
    memcpy(out + off, t, (size_t)tn); off += tn;
    memcpy(out + off, l, (size_t)ln); off += ln;
    if (plen > 0) { memcpy(out + off, payload, (size_t)plen); off += plen; }
    return off;
}

// ==================== 连接级接收缓冲 ====================
typedef struct {
    uint8_t* data;
    int len, cap;
} h3connbuf;
static h3connbuf g_h3buf[H3_MAX_CONN];

static h3connbuf* h3_buf_for(int64_t conn) {
    if (conn <= 0 || conn > H3_MAX_CONN) return NULL;
    return &g_h3buf[conn - 1];
}

// 追加读取：把 conn 上可用数据收进缓冲（非阻塞轮询至多 timeout_ms）。
static int h3_buf_fill(int64_t conn, h3connbuf* b, int64_t timeout_ms) {
    uint8_t tmp[4096];
    int64_t got = px_quic_raw_recv(conn, tmp, (int)sizeof(tmp), timeout_ms);
    if (got <= 0) return -1;   // 超时/关闭
    if (b->len + (int)got > b->cap) {
        int ncap = b->cap > 0 ? b->cap : 4096;
        while (ncap < b->len + (int)got) ncap *= 2;
        if (ncap > H3_BUF_MAX) ncap = H3_BUF_MAX;
        if (ncap < b->len + (int)got) return -2;
        uint8_t* nd = (uint8_t*)realloc(b->data, (size_t)ncap);
        if (!nd) return -2;
        b->data = nd;
        b->cap = ncap;
    }
    memcpy(b->data + b->len, tmp, (size_t)got);
    b->len += (int)got;
    return 0;
}

// 从缓冲解析一帧（需完整在缓冲内）。成功：*poff/*plen 指向帧内 payload，返回帧类型。
// 不足：返回 -2；非法：-1。
static int h3_buf_parse(h3connbuf* b, int* poff, int* plen) {
    if (b->len < 2) return -2;
    uint64_t type = 0, plen_v = 0;
    int u = h3_varint_dec(b->data, b->len, &type);
    if (u == 0) return -2;
    int u2 = h3_varint_dec(b->data + u, b->len - u, &plen_v);
    if (u2 == 0) return -2;
    if (plen_v > H3_BUF_MAX) return -1;
    if (b->len - u - u2 < (int)plen_v) return -2;
    *poff = u + u2;
    *plen = (int)plen_v;
    return (int)type;
}

// 取一帧：阻塞等到缓冲内有完整帧（timeout_ms 上限）。消费式：读完从缓冲移除。
static int h3_take_frame(int64_t conn, uint8_t** payload, int* plen, int64_t timeout_ms) {
    h3connbuf* b = h3_buf_for(conn);
    if (!b) return -1;
    for (;;) {
        int po = 0, pl = 0;
        int t = h3_buf_parse(b, &po, &pl);
        if (t == -2) {
            if (b->len >= H3_BUF_MAX) return -2;
            if (h3_buf_fill(conn, b, timeout_ms) != 0) return -1;
            continue;
        }
        if (t == -1) return -2;
        // 拷贝 payload
        uint8_t* p = (uint8_t*)malloc((size_t)(pl > 0 ? pl : 1));
        if (!p) return -2;
        if (pl > 0) memcpy(p, b->data + po, (size_t)pl);
        *payload = p;
        *plen = pl;
        // 消费：移除已解析帧
        int consumed = po + pl;
        if (consumed < b->len) memmove(b->data, b->data + consumed, (size_t)(b->len - consumed));
        b->len -= consumed;
        return t;
    }
}

// ==================== 语言层绑定 ====================

// ---- 纯 codec（测试用）----

// h3_qenc(headers: list of [name,value]) -> bytes
static LXValue bi_h3_qenc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_LIST) px_error("h3_qenc 需要 (headers: list)");
    LXObject* lst = args[0].as.obj;
    uint8_t* out = (uint8_t*)malloc(H3_BUF_MAX);
    if (!out) return px_null();
    int n = qp_enc_section(out, lst->as.list.items, lst->as.list.len);
    if (n < 0) { free(out); return px_null(); }
    LXValue r = px_bytes_len(out, n);
    free(out);
    return r;
}

// h3_qdec(bytes) -> list of [name,value] | null
static LXValue bi_h3_qdec(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || (args[0].type != PX_STR && args[0].type != PX_BYTES))
        px_error("h3_qdec 需要 (data: bytes)");
    LXObject* o = args[0].as.obj;
    return qp_dec_section((const uint8_t*)o->as.str.data, o->as.str.len);
}

// h3_frame(type:int, payload:bytes) -> bytes
static LXValue bi_h3_frame(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT) px_error("h3_frame 需要 (type:int, payload)");
    int type = (int)args[0].as.i;
    const uint8_t* pl = NULL; int plen = 0;
    if (args[1].type == PX_STR || args[1].type == PX_BYTES) {
        pl = (const uint8_t*)args[1].as.obj->as.str.data;
        plen = args[1].as.obj->as.str.len;
    } else if (args[1].type != PX_NULL) px_error("h3_frame 的 payload 需要 str/bytes");
    uint8_t* out = (uint8_t*)malloc(H3_BUF_MAX + 16);
    if (!out) return px_null();
    int n = h3_build_frame(out, type, pl, plen);
    LXValue r = px_bytes_len(out, n);
    free(out);
    return r;
}

// ---- 请求/响应高层 ----

// h3_serve_send_response(conn, status:int, headers:list, body:str) -> bool
static LXValue bi_h3_serve_send_response(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 4 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("h3_serve_send_response 需要 (conn, status:int, headers:list, body)");
    int64_t conn = args[0].as.i;
    int status = (int)args[1].as.i;
    const uint8_t* bd = NULL; int blen = 0;
    if (args[3].type == PX_STR || args[3].type == PX_BYTES) {
        bd = (const uint8_t*)args[3].as.obj->as.str.data;
        blen = args[3].as.obj->as.str.len;
    } else if (args[3].type != PX_NULL) px_error("h3_serve_send_response 的 body 需要 str/bytes");
    // 组装字段：[":status", n] + headers
    LXValue fields = px_list(8);
    char sb[16]; snprintf(sb, sizeof(sb), "%d", status);
    LXValue pair0 = px_list(2);
    px_list_push(pair0, px_str(H3_STATUS));
    px_list_push(pair0, px_str(sb));
    px_list_push(fields, pair0);
    if (args[2].type == PX_LIST) {
        LXObject* hd = args[2].as.obj;
        for (int i = 0; i < hd->as.list.len; i++) {
            LXValue it = hd->as.list.items[i];
            if (it.type != PX_LIST) continue;
            LXObject* ito = it.as.obj;
            if (ito->as.list.len < 2) continue;
            LXValue* kv = ito->as.list.items;
            if (kv[0].type != PX_STR || kv[1].type != PX_STR) continue;
            LXValue pair = px_list(2);
            px_list_push(pair, kv[0]);
            px_list_push(pair, kv[1]);
            px_list_push(fields, pair);
        }
    }
    uint8_t* q = (uint8_t*)malloc(H3_BUF_MAX);
    uint8_t* f = (uint8_t*)malloc(H3_BUF_MAX + 16);
    if (!q || !f) { free(q); free(f); return px_bool(false); }
    LXObject* fo = fields.as.obj;
    int qn = qp_enc_section(q, fo->as.list.items, fo->as.list.len);
    int fn1 = h3_build_frame(f, H3_FRAME_HEADERS, q, qn > 0 ? qn : 0);
    int fn2 = h3_build_frame(f + fn1, H3_FRAME_DATA, bd, blen);
    int64_t sent = px_quic_raw_send(conn, f, fn1 + fn2);
    free(q); free(f);
    return px_bool(sent >= 0);
}

// h3_serve_read_request(conn, timeout_ms) -> dict|null
static LXValue bi_h3_serve_read_request(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT) px_error("h3_serve_read_request 需要 (conn, timeout_ms)");
    int64_t conn = args[0].as.i;
    int64_t timeout = args[1].as.i;
    // 收 HEADERS 帧
    uint8_t* hd = NULL; int hlen = 0;
    int t = h3_take_frame(conn, &hd, &hlen, timeout);
    if (t != H3_FRAME_HEADERS) { free(hd); return px_null(); }
    LXValue fields = qp_dec_section(hd, hlen);
    free(hd);
    if (fields.type != PX_LIST) return px_null();
    // 收 DATA 帧（body）
    uint8_t* bd = NULL; int blen = 0;
    t = h3_take_frame(conn, &bd, &blen, timeout);
    if (t != H3_FRAME_DATA) { free(bd); return px_null(); }
    // 组装 dict
    LXValue d = px_dict();
    LXValue hdr = px_dict();
    LXObject* fo = fields.as.obj;
    const char* method = ""; const char* scheme = ""; const char* auth = ""; const char* path = "";
    for (int i = 0; i < fo->as.list.len; i++) {
        LXValue it = fo->as.list.items[i];
        if (it.type != PX_LIST) continue;
        LXObject* p = it.as.obj;
        if (p->as.list.len < 2) continue;
        LXValue* kv = p->as.list.items;
        if (kv[0].type != PX_STR || kv[1].type != PX_STR) continue;
        const char* n = kv[0].as.obj->as.str.data;
        if (strcmp(n, H3_METHOD) == 0) method = kv[1].as.obj->as.str.data;
        else if (strcmp(n, H3_SCHEME) == 0) scheme = kv[1].as.obj->as.str.data;
        else if (strcmp(n, H3_AUTH) == 0) auth = kv[1].as.obj->as.str.data;
        else if (strcmp(n, H3_PATH) == 0) path = kv[1].as.obj->as.str.data;
        else {
            char* key = strndup(n, (size_t)kv[0].as.obj->as.str.len);
            px_dict_set(hdr, key, kv[1]);
            free(key);
        }
    }
    px_dict_set(d, "method", px_str(method));
    px_dict_set(d, "scheme", px_str(scheme));
    px_dict_set(d, "authority", px_str(auth));
    px_dict_set(d, "path", px_str(path));
    px_dict_set(d, "headers", hdr);
    px_dict_set(d, "body", px_str_len((const char*)bd, blen));
    free(bd);
    return d;
}

// h3_client_connect(ip:str, port:int, alpn:str) -> int
static LXValue bi_h3_client_connect(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 3 || args[0].type != PX_STR || args[1].type != PX_INT || args[2].type != PX_STR)
        px_error("h3_client_connect 需要 (ip:str, port:int, alpn:str)");
    const char* ip = args[0].as.obj->as.str.data;
    int port = (int)args[1].as.i;
    const char* alpn = args[2].as.obj->as.str.data;
    int64_t c = px_quic_raw_connect(ip, port, alpn);
    return px_int(c);
}

// h3_client_send_request(conn, method:str, scheme:str, authority:str, path:str, headers:list, body:str) -> bool
static LXValue bi_h3_client_send_request(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 7 || args[0].type != PX_INT) px_error("h3_client_send_request 需要 (conn, method, scheme, authority, path, headers, body)");
    int64_t conn = args[0].as.i;
    if (args[1].type != PX_STR || args[2].type != PX_STR || args[3].type != PX_STR || args[4].type != PX_STR)
        return px_bool(false);
    const char* method = args[1].as.obj->as.str.data;
    const char* scheme = args[2].as.obj->as.str.data;
    const char* auth = args[3].as.obj->as.str.data;
    const char* path = args[4].as.obj->as.str.data;
    LXValue fields = px_list(8);
    LXValue p0 = px_list(2); px_list_push(p0, px_str(H3_METHOD)); px_list_push(p0, px_str(method)); px_list_push(fields, p0);
    LXValue p1 = px_list(2); px_list_push(p1, px_str(H3_SCHEME)); px_list_push(p1, px_str(scheme)); px_list_push(fields, p1);
    LXValue p2 = px_list(2); px_list_push(p2, px_str(H3_AUTH)); px_list_push(p2, px_str(auth)); px_list_push(fields, p2);
    LXValue p3 = px_list(2); px_list_push(p3, px_str(H3_PATH)); px_list_push(p3, px_str(path)); px_list_push(fields, p3);
    if (args[5].type == PX_LIST) {
        LXObject* hd = args[5].as.obj;
        for (int i = 0; i < hd->as.list.len; i++) {
            LXValue it = hd->as.list.items[i];
            if (it.type != PX_LIST) continue;
            LXObject* ito = it.as.obj;
            if (ito->as.list.len < 2) continue;
            LXValue* kv = ito->as.list.items;
            if (kv[0].type != PX_STR || kv[1].type != PX_STR) continue;
            LXValue pair = px_list(2);
            px_list_push(pair, kv[0]);
            px_list_push(pair, kv[1]);
            px_list_push(fields, pair);
        }
    }
    const uint8_t* bd = NULL; int blen = 0;
    if (args[6].type == PX_STR || args[6].type == PX_BYTES) {
        bd = (const uint8_t*)args[6].as.obj->as.str.data;
        blen = args[6].as.obj->as.str.len;
    } else if (args[6].type != PX_NULL) return px_bool(false);
    uint8_t* q = (uint8_t*)malloc(H3_BUF_MAX);
    uint8_t* f = (uint8_t*)malloc(H3_BUF_MAX + 16);
    if (!q || !f) { free(q); free(f); return px_bool(false); }
    LXObject* fo = fields.as.obj;
    int qn = qp_enc_section(q, fo->as.list.items, fo->as.list.len);
    int fn1 = h3_build_frame(f, H3_FRAME_HEADERS, q, qn > 0 ? qn : 0);
    int fn2 = h3_build_frame(f + fn1, H3_FRAME_DATA, bd, blen);
    int64_t sent = px_quic_raw_send(conn, f, fn1 + fn2);
    free(q); free(f);
    return px_bool(sent >= 0);
}

// h3_client_read_response(conn, timeout_ms) -> dict|null
static LXValue bi_h3_client_read_response(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT) px_error("h3_client_read_response 需要 (conn, timeout_ms)");
    int64_t conn = args[0].as.i;
    int64_t timeout = args[1].as.i;
    uint8_t* hd = NULL; int hlen = 0;
    int t = h3_take_frame(conn, &hd, &hlen, timeout);
    if (t != H3_FRAME_HEADERS) { free(hd); return px_null(); }
    LXValue fields = qp_dec_section(hd, hlen);
    free(hd);
    if (fields.type != PX_LIST) return px_null();
    uint8_t* bd = NULL; int blen = 0;
    t = h3_take_frame(conn, &bd, &blen, timeout);
    if (t != H3_FRAME_DATA) { free(bd); return px_null(); }
    LXValue d = px_dict();
    LXValue hdr = px_dict();
    LXObject* fo = fields.as.obj;
    const char* status = "";
    for (int i = 0; i < fo->as.list.len; i++) {
        LXValue it = fo->as.list.items[i];
        if (it.type != PX_LIST) continue;
        LXObject* p = it.as.obj;
        if (p->as.list.len < 2) continue;
        LXValue* kv = p->as.list.items;
        if (kv[0].type != PX_STR || kv[1].type != PX_STR) continue;
        const char* n = kv[0].as.obj->as.str.data;
        if (strcmp(n, H3_STATUS) == 0) status = kv[1].as.obj->as.str.data;
        else {
            char* key = strndup(n, (size_t)kv[0].as.obj->as.str.len);
            px_dict_set(hdr, key, kv[1]);
            free(key);
        }
    }
    px_dict_set(d, "status", px_str(status));
    px_dict_set(d, "headers", hdr);
    px_dict_set(d, "body", px_str_len((const char*)bd, blen));
    free(bd);
    return d;
}

// ==================== 注册（runtime.c px_register_builtins 调用）====================
void px_register_h3(void) {
    px_set_global("h3_qenc", px_native("h3_qenc", bi_h3_qenc));
    px_set_global("h3_qdec", px_native("h3_qdec", bi_h3_qdec));
    px_set_global("h3_frame", px_native("h3_frame", bi_h3_frame));
    px_set_global("h3_serve_send_response", px_native("h3_serve_send_response", bi_h3_serve_send_response));
    px_set_global("h3_serve_read_request", px_native("h3_serve_read_request", bi_h3_serve_read_request));
    px_set_global("h3_client_connect", px_native("h3_client_connect", bi_h3_client_connect));
    px_set_global("h3_client_send_request", px_native("h3_client_send_request", bi_h3_client_send_request));
    px_set_global("h3_client_read_response", px_native("h3_client_read_response", bi_h3_client_read_response));
    px_ffi_register("h3_qenc", bi_h3_qenc);
    px_ffi_register("h3_qdec", bi_h3_qdec);
    px_ffi_register("h3_frame", bi_h3_frame);
    px_ffi_register("h3_serve_send_response", bi_h3_serve_send_response);
    px_ffi_register("h3_serve_read_request", bi_h3_serve_read_request);
    px_ffi_register("h3_client_connect", bi_h3_client_connect);
    px_ffi_register("h3_client_send_request", bi_h3_client_send_request);
    px_ffi_register("h3_client_read_response", bi_h3_client_read_response);
}
