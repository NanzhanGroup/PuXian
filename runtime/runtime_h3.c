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
#include "runtime_h3_qpack.h"
#include "runtime_h3_qpack_dyn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define H3_MAX_CONN 64
#define H3_FRAME_HEADERS 0x01
#define H3_FRAME_DATA    0x00
#define H3_FRAME_SETTINGS 0x04   // M51：控制流 SETTINGS 帧（RFC 9114 §7.2.8）
#define H3_BUF_MAX (1 << 20)   // 单帧/消息上限 1MB（MVP）
#define H3_METHOD ":method"
#define H3_SCHEME ":scheme"
#define H3_AUTH   ":authority"
#define H3_PATH   ":path"
#define H3_STATUS ":status"
// M51：SETTINGS 键（RFC 9114 §7.2.8）
#define H3_SET_QPACK_MAX_TABLE_CAPACITY 0x01
#define H3_SET_QPACK_BLOCKED_STREAMS    0x07
// M51：单向流类型（RFC 9114 §6.2.1，流首字节 varint）
#define H3_UT_CONTROL  0x00
#define H3_UT_ENCODER  0x02
#define H3_UT_DECODER  0x03

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


// ==================== per-stream 接收缓冲（M50：多路复用）====================
// 每条活跃双向流一个 H3 帧缓冲槽（按 sid 精确匹配线性槽，上限 H3_STREAM_SLOTS）。
#define H3_STREAM_SLOTS 24   // M51：含对端控制流等 uni 槽（对齐 QUIC_STREAM_MAX）

typedef struct {
    int64_t sid;           // 绑定流 id
    int used;              // 槽占用
    uint8_t* data;
    int len, cap;
} h3connbuf;

static h3connbuf g_h3buf[H3_MAX_CONN][H3_STREAM_SLOTS];
static int64_t g_last_sid[H3_MAX_CONN];   // 旧 API 兼容：最近读请求/响应的流

// ==================== M51：连接级 HTTP/3 会话（QPACK 动态表接入线上）====================
// 已 setup 的连接：本端开 3 条单向流（RFC 9114 §6.2.1：控制流必须是本端第一条 uni 流），
//   控制流首字节 0x00 + SETTINGS 帧（qcap / blocked）；编码器流 0x02 走 QPACK
//   Insert/SetCapacity 指令；解码器流 0x03 预留（MVP 不在线自动发 ack）。
// QPACK 会话（px_qd_*，g_qds 句柄）承载本端编码表 + 对端镜像表。
typedef struct {
    int used;              // 会话已 setup
    int64_t qd;            // 本端 QPACK 会话 id | 0
    int64_t ctrl_sid;      // 本端控制流 sid
    int64_t enc_sid;       // 本端编码器流 sid
    int64_t dec_sid;       // 本端解码器流 sid
    int64_t peer_ctrl;     // 对端控制流 sid（已分类）| 0 未知
    int64_t peer_enc;      // 对端编码器流 sid | 0 未知
    int64_t peer_dec;      // 对端解码器流 sid | 0 未知
    int64_t peer_qcap;     // 对端 SETTINGS QPACK_MAX_TABLE_CAPACITY | -1 未收
    int64_t peer_blocked;  // 对端 SETTINGS QPACK_BLOCKED_STREAMS | -1 未收
    int64_t peer_enc_bytes;// 对端编码器流已 ingest 字节
    int64_t enc_sent;      // 本端编码器流累计发送字节
    int64_t enc_sects;     // 动态表编码的字段段计数
    int64_t blocked_cnt;   // 解码阻塞恢复次数
} h3conn_state;
static h3conn_state g_h3st[H3_MAX_CONN];
static h3conn_state* h3_st(int64_t conn) {
    return (conn > 0 && conn <= H3_MAX_CONN) ? &g_h3st[conn - 1] : NULL;
}

// QUIC 流 id 次低位（0x2）：0=双向 / 1=单向（RFC 9000 §2.1；最低位 0x1 是发起者标志）
static int h3_sid_is_uni(int64_t sid) { return (int)(sid & 2) ? 1 : 0; }

// 构造 SETTINGS 帧（完整帧，type=0x04）：payload = varint 键值对
static int h3_build_settings_frame(uint8_t* out, int qcap, int blocked) {
    uint8_t pl[64]; int po = 0;
    uint8_t t[8]; int tn;
    tn = h3_varint_enc(t, H3_SET_QPACK_MAX_TABLE_CAPACITY); memcpy(pl + po, t, (size_t)tn); po += tn;
    tn = h3_varint_enc(t, (uint64_t)(qcap < 0 ? 0 : qcap)); memcpy(pl + po, t, (size_t)tn); po += tn;
    tn = h3_varint_enc(t, H3_SET_QPACK_BLOCKED_STREAMS); memcpy(pl + po, t, (size_t)tn); po += tn;
    tn = h3_varint_enc(t, (uint64_t)(blocked < 0 ? 0 : blocked)); memcpy(pl + po, t, (size_t)tn); po += tn;
    int off = 0;
    tn = h3_varint_enc(t, H3_FRAME_SETTINGS); memcpy(out + off, t, (size_t)tn); off += tn;
    tn = h3_varint_enc(t, (uint64_t)po); memcpy(out + off, t, (size_t)tn); off += tn;
    memcpy(out + off, pl, (size_t)po); off += po;
    return off;
}

// 解析 SETTINGS 帧 payload（键值 varint 循环）→ 存 peer_qcap/peer_blocked
static void h3_parse_settings_payload(const uint8_t* p, int plen, h3conn_state* st) {
    int off = 0;
    while (off < plen) {
        uint64_t k = 0, v = 0;
        int n1 = h3_varint_dec(p + off, plen - off, &k);
        if (n1 <= 0) return;
        off += n1;
        if (off >= plen) return;
        int n2 = h3_varint_dec(p + off, plen - off, &v);
        if (n2 <= 0) return;
        off += n2;
        if (k == H3_SET_QPACK_MAX_TABLE_CAPACITY) st->peer_qcap = (int64_t)v;
        else if (k == H3_SET_QPACK_BLOCKED_STREAMS) st->peer_blocked = (int64_t)v;
        // 其他键忽略（RFC 9114 §7.2.8：未知 SETTINGS 必须忽略）
    }
}

static h3connbuf* h3_buf_for(int64_t conn, int64_t sid) {
    if (conn <= 0 || conn > H3_MAX_CONN) return NULL;
    h3connbuf (*slots)[H3_STREAM_SLOTS] = &g_h3buf[conn - 1];
    for (int i = 0; i < H3_STREAM_SLOTS; i++)
        if ((*slots)[i].used && (*slots)[i].sid == sid) return &(*slots)[i];
    for (int i = 0; i < H3_STREAM_SLOTS; i++) {
        if (!(*slots)[i].used) {
            (*slots)[i].used = 1;
            (*slots)[i].sid = sid;
            (*slots)[i].len = (*slots)[i].cap = 0;
            (*slots)[i].data = NULL;
            return &(*slots)[i];
        }
    }
    return NULL;  // 槽满
}

// 追加读取：从 QUIC 指定流收数据进缓冲（至多等 timeout_ms）。
static int h3_buf_fill(int64_t conn, int64_t sid, h3connbuf* b, int64_t timeout_ms) {
    uint8_t tmp[4096];
    int64_t got = px_quic_raw_recv_on(conn, sid, tmp, (int)sizeof(tmp), (int)timeout_ms);
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

// 取一帧（指定流）：阻塞等到该流缓冲内有完整帧（timeout_ms 上限）。消费式移除。
static int h3_take_frame(int64_t conn, int64_t sid, uint8_t** payload, int* plen, int64_t timeout_ms) {
    h3connbuf* b = h3_buf_for(conn, sid);
    if (!b) return -1;
    for (;;) {
        int po = 0, pl = 0;
        int t = h3_buf_parse(b, &po, &pl);
        if (t == -2) {
            if (b->len >= H3_BUF_MAX) return -2;
            if (h3_buf_fill(conn, sid, b, timeout_ms) != 0) return -1;
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
// ==================== M51：QPACK/控制流线上接线（真实单向流）====================

// 对端控制流数据缓存进该流帧缓冲并解析完整帧（SETTINGS 等）
static void h3_ctrl_ingest(int64_t conn, int64_t sid, h3conn_state* st,
                           const uint8_t* data, int len) {
    h3connbuf* b = h3_buf_for(conn, sid);
    if (!b) return;
    if (b->len + len > b->cap) {
        int nc = b->cap > 0 ? b->cap * 2 : 4096;
        while (nc < b->len + len) nc *= 2;
        uint8_t* nd = (uint8_t*)realloc(b->data, (size_t)nc);
        if (!nd) return;
        b->data = nd; b->cap = nc;
    }
    if (len > 0 && data) { memcpy(b->data + b->len, data, (size_t)len); b->len += len; }
    for (;;) {
        int po = 0, pl = 0;
        int t = h3_buf_parse(b, &po, &pl);
        if (t == -2) break;                 // 不完整：等更多数据
        if (t == -1) { b->len = 0; break; } // 非法帧：丢弃该流缓冲
        if (t == H3_FRAME_SETTINGS) h3_parse_settings_payload(b->data + po, pl, st);
        int consumed = po + pl;
        if (consumed < b->len) memmove(b->data, b->data + consumed, (size_t)(b->len - consumed));
        b->len -= consumed;
        if (b->len <= 0) break;
    }
}

// 处理一条对端单向流（数据已 recv 到 buf）：首见分类（0x00 控制 / 0x02 编码器 /
// 0x03 解码器），随后按类型消费（SETTINGS 解析 / QPACK ingest / 丢弃）。
static int h3_consume_peer_uni(int64_t conn, int64_t sid, h3conn_state* st,
                               const uint8_t* buf, int n) {
    if (!st || !st->used || n <= 0) return -1;
    int off = 0;
    int known = (sid == st->peer_ctrl) || (sid == st->peer_enc) || (sid == st->peer_dec);
    if (!known) {
        int type = buf[0] & 0x3f;           // 流类型 varint（<64 单字节）
        if (type == H3_UT_CONTROL) st->peer_ctrl = sid;
        else if (type == H3_UT_ENCODER) st->peer_enc = sid;
        else if (type == H3_UT_DECODER) st->peer_dec = sid;
        else return 0;                       // 未知/保留类型：丢弃该流
        off = 1;                             // 已消费流类型字节
    }
    const uint8_t* d = buf + off; int dl = n - off;
    if (sid == st->peer_enc) {
        if (st->qd > 0 && dl > 0) {
            if (px_qd_ingest(st->qd, d, dl) != 0) return -1;   // 非法编码器指令
            st->peer_enc_bytes += dl;
        }
    } else if (sid == st->peer_ctrl) {
        if (dl > 0) h3_ctrl_ingest(conn, sid, st, d, dl);
    }
    // peer_dec：解码器流指令 MVP 不依赖 → 丢弃
    return 0;
}

// 泵对端编码器流并 ingest（解码字段段遇阻塞 -2 时调用；编码器流与请求流是不同
// QUIC 流、无跨流顺序保证，RFC 9204 以阻塞等待兜底）。对端编码器流未知时按
// HTTP/3 uni 布局探测分类（对端首条 uni = 2/3，步进 4；MVP 双端各 3 条 uni）。
// 注意：不能用 quic_poll 定位对端 uni（bidi 数据占住最小 sid 时 poll 不会换流），
// 故直接按流 id 探测 recv（不存在/空的流立即返回 0）。返回 0 有进展 / -1 无进展。
static int h3_ingest_peer_enc(int64_t conn, h3conn_state* st, int64_t timeout_ms) {
    uint8_t buf[8192];
    if (!st) return -1;
    int64_t first = (st->ctrl_sid & 1) ? 2 : 3;   // 对端首条 uni（bit0=0 → 对端 server：3…）
    for (int attempt = 0; attempt < 8; attempt++) {
        if (st->peer_enc == 0) {
            // 探测对端 uni 流（前 4 条足够 MVP）
            for (int64_t s = first; s < first + 16; s += 4) {
                int64_t n = px_quic_raw_recv_on(conn, s, buf, (int)sizeof(buf), 0);
                if (n <= 0) continue;
                int64_t before = st->peer_enc_bytes;
                if (h3_consume_peer_uni(conn, s, st, buf, (int)n) != 0) return -1;
                if (st->peer_enc_bytes != before) return 0;   // 已 ingest 到编码器指令（进展）
            }
        }
        if (st->peer_enc > 0) {
            int64_t n = px_quic_raw_recv_on(conn, st->peer_enc, buf, (int)sizeof(buf), 0);
            if (n > 0) {
                if (st->qd > 0 && px_qd_ingest(st->qd, buf, (int)n) != 0) return -1;
                st->peer_enc_bytes += n;
                return 0;
            }
            if (attempt >= 3) return -1;                 // enc 已空且等过
            px_quic_raw_poll(conn, (int)(timeout_ms > 0 && timeout_ms < 2000 ? timeout_ms : 800));
            continue;
        }
        // enc 流尚未出现：等待对端数据
        if (attempt >= 6) return -1;
        px_quic_raw_poll(conn, (int)(timeout_ms > 0 && timeout_ms < 2000 ? timeout_ms : 800));
    }
    return -1;
}

// 等任一对端双向流（请求流）有数据 → sid | -1 超时/出错。
// 对端单向流自动分类消费（SETTINGS 解析 / QPACK ingest / 解码器丢弃）。
static int64_t h3_poll_requests(int64_t conn, int64_t timeout_ms) {
    h3conn_state* st = h3_st(conn);
    uint8_t buf[8192];
    int spins = 0;
    for (;;) {
        int64_t sid = px_quic_raw_poll(conn, (int)(timeout_ms > 0 ? timeout_ms : 500));
        if (sid < 0) return sid;
        if (!(sid & 2)) return sid;                     // bidi → 请求/响应流
        if (!st || !st->used) {                          // 未 setup：防御性丢弃
            while (px_quic_raw_recv_on(conn, sid, buf, (int)sizeof(buf), 0) > 0) {}
            if (++spins > 20) return -1;
            continue;
        }
        int64_t n = px_quic_raw_recv_on(conn, sid, buf, (int)sizeof(buf), 500);
        if (n <= 0) { if (++spins > 20) return -1; continue; }
        if (h3_consume_peer_uni(conn, sid, st, buf, (int)n) != 0) return -1;
        timeout_ms = 2000;                              // 后续轮询短超时
        if (++spins > 200) return -1;
    }
}

// 字段列表 LXValue → C 字符串数组（strndup 堆拷贝；用 h3_free_c_fields 释放）
static int h3_fields_to_c(LXValue fields, char** names, char** vals,
                          int* nls, int* vls, int maxf) {
    if (fields.type != PX_LIST) return -1;
    LXObject* fo = fields.as.obj;
    int nf = 0;
    for (int i = 0; i < fo->as.list.len && nf < maxf; i++) {
        LXValue it = fo->as.list.items[i];
        if (it.type != PX_LIST) continue;
        LXObject* p = it.as.obj;
        if (p->as.list.len < 2) continue;
        LXValue* kv = p->as.list.items;
        if ((kv[0].type != PX_STR && kv[0].type != PX_BYTES)) continue;
        if ((kv[1].type != PX_STR && kv[1].type != PX_BYTES)) continue;
        int l0 = (int)kv[0].as.obj->as.str.len;
        int l1 = (int)kv[1].as.obj->as.str.len;
        names[nf] = strndup(kv[0].as.obj->as.str.data, (size_t)l0);
        vals[nf] = strndup(kv[1].as.obj->as.str.data, (size_t)l1);
        if (!names[nf] || !vals[nf]) { free(names[nf]); free(vals[nf]); return nf > 0 ? nf : -1; }
        nls[nf] = l0; vls[nf] = l1;
        nf++;
    }
    return nf;
}
static void h3_free_c_fields(char** names, char** vals, int nf) {
    for (int i = 0; i < nf; i++) { free(names[i]); free(vals[i]); }
}
// C 字段数组 → LXValue 字段列表
static LXValue h3_c_fields_to_lx(char** names, char** vals, int* nls, int* vls, int nf) {
    LXValue fields = px_list(8);
    for (int i = 0; i < nf; i++) {
        LXValue pair = px_list(2);
        px_list_push(pair, px_str_len(names[i] ? names[i] : "", nls[i]));
        px_list_push(pair, px_str_len(vals[i] ? vals[i] : "", vls[i]));
        px_list_push(fields, pair);
    }
    return fields;
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
    int n = px_h3_qenc(out, lst->as.list.items, lst->as.list.len);
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
    return px_h3_qdec((const uint8_t*)o->as.str.data, o->as.str.len);
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


// ---- 请求/响应高层（M50：per-stream 多路复用；旧 API 兼容默认流）----

// 追加 headers（list of [name,value]）到 fields 列表
static void h3_append_headers(LXValue fields, LXValue headers_val) {
    if (headers_val.type != PX_LIST) return;
    LXObject* hd = headers_val.as.obj;
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

// 编码 fields 并以 HEADERS+DATA 两帧写到指定流
// M51：连接已 setup → QPACK 动态表编码（encoder 指令 flush 到本端编码器单向流）；
//      未 setup → 无状态 codec（M47–M50 行为不变）。
static bool h3_send_fields(int64_t conn, int64_t sid, LXValue fields, LXValue body_val) {
    const uint8_t* bd = NULL; int blen = 0;
    if (body_val.type == PX_STR || body_val.type == PX_BYTES) {
        bd = (const uint8_t*)body_val.as.obj->as.str.data;
        blen = body_val.as.obj->as.str.len;
    } else if (body_val.type != PX_NULL) return false;
    uint8_t* q = (uint8_t*)malloc(H3_BUF_MAX);
    uint8_t* f = (uint8_t*)malloc(H3_BUF_MAX + 16);
    if (!q || !f) { free(q); free(f); return false; }
    LXObject* fo = fields.as.obj;
    int qn;
    h3conn_state* st = h3_st(conn);
    if (st && st->used && st->qd > 0 && st->enc_sid > 0) {
        char* names[512]; char* vals[512]; int nls[512], vls[512];
        int nf = h3_fields_to_c(fields, names, vals, nls, vls, 512);
        if (nf < 0) { free(q); free(f); return false; }
        qn = px_qd_enc(st->qd, names, vals, nls, vls, nf, q, H3_BUF_MAX);
        h3_free_c_fields(names, vals, nf);
        if (qn >= 0) {
            uint8_t eout[8192];
            int en = px_qd_take_enc(st->qd, eout, (int)sizeof(eout));
            if (en > 0) {
                px_quic_raw_send_on(conn, st->enc_sid, eout, en, 0);
                st->enc_sent += en;
            }
            st->enc_sects++;
        }
    } else {
        qn = px_h3_qenc(q, fo->as.list.items, fo->as.list.len);
    }
    if (qn < 0) qn = 0;   // 原行为：编码失败发空字段段（理论不发生）
    int fn1 = h3_build_frame(f, H3_FRAME_HEADERS, q, qn);
    int fn2 = h3_build_frame(f + fn1, H3_FRAME_DATA, bd, blen);
    int64_t sent = px_quic_raw_send_on(conn, sid, f, fn1 + fn2, 0);
    free(q); free(f);
    return sent >= 0;
}

// 从指定流读一个完整消息（HEADERS 帧 + DATA 帧）→ 返回字段 list + body（heap）
// M51：连接已 setup → QPACK 动态表解码（遇阻塞 -2 泵对端编码器流 ingest 后重试）；
//      未 setup → 无状态 codec。返回帧级状态：0 成功；-1 超时/关闭；-2 非法。
static int h3_read_section(int64_t conn, int64_t sid, int64_t timeout_ms,
                           LXValue* pfields, uint8_t** pbody, int* pblen) {
    uint8_t* hd = NULL; int hlen = 0;
    int t = h3_take_frame(conn, sid, &hd, &hlen, timeout_ms);
    if (t != H3_FRAME_HEADERS) { free(hd); return -1; }
    LXValue fields;
    h3conn_state* st = h3_st(conn);
    if (st && st->used && st->qd > 0) {
        char** names = NULL; char** vals = NULL; int* nls = NULL; int* vls = NULL;
        int nf = -1, tries = 0;
        for (;;) {
            nf = px_qd_dec(st->qd, hd, hlen, &names, &vals, &nls, &vls);
            if (nf >= 0) break;
            if (nf != -2) { free(hd); return -2; }       // 非法字段段
            if (++tries > 300) { free(hd); return -1; }  // 阻塞超时（防御）
            if (h3_ingest_peer_enc(conn, st, 1500) != 0) { free(hd); return -1; }
            st->blocked_cnt++;                           // 阻塞恢复
        }
        fields = h3_c_fields_to_lx(names, vals, nls, vls, nf);
        h3_free_c_fields(names, vals, nf);
        free(names); free(vals); free(nls); free(vls);
    } else {
        fields = px_h3_qdec(hd, hlen);
    }
    free(hd);
    if (fields.type != PX_LIST) return -2;
    uint8_t* bd = NULL; int blen = 0;
    t = h3_take_frame(conn, sid, &bd, &blen, timeout_ms);
    if (t != H3_FRAME_DATA) { free(bd); return -1; }
    *pfields = fields;
    *pbody = bd;
    *pblen = blen;
    return 0;
}

// 请求字段 → dict（含 sid）
static LXValue h3_fields_to_request(LXValue fields, int64_t sid,
                                    const uint8_t* bd, int blen) {
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
    px_dict_set(d, "sid", px_int(sid));
    px_dict_set(d, "method", px_str(method));
    px_dict_set(d, "scheme", px_str(scheme));
    px_dict_set(d, "authority", px_str(auth));
    px_dict_set(d, "path", px_str(path));
    px_dict_set(d, "headers", hdr);
    px_dict_set(d, "body", px_str_len((const char*)bd, blen));
    return d;
}

// 响应字段 → dict（含 sid）
static LXValue h3_fields_to_response(LXValue fields, int64_t sid,
                                     const uint8_t* bd, int blen) {
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
    px_dict_set(d, "sid", px_int(sid));
    px_dict_set(d, "status", px_str(status));
    px_dict_set(d, "headers", hdr);
    px_dict_set(d, "body", px_str_len((const char*)bd, blen));
    return d;
}

// 响应字段组装：[:status, n] + headers
static LXValue h3_make_response_fields(int status, LXValue headers_val) {
    LXValue fields = px_list(8);
    char sb[16]; snprintf(sb, sizeof(sb), "%d", status);
    LXValue pair0 = px_list(2);
    px_list_push(pair0, px_str(H3_STATUS));
    px_list_push(pair0, px_str(sb));
    px_list_push(fields, pair0);
    h3_append_headers(fields, headers_val);
    return fields;
}

// 请求字段组装：伪头 + headers
static LXValue h3_make_request_fields(const char* method, const char* scheme,
                                      const char* auth, const char* path,
                                      LXValue headers_val) {
    LXValue fields = px_list(8);
    LXValue p0 = px_list(2); px_list_push(p0, px_str(H3_METHOD)); px_list_push(p0, px_str(method)); px_list_push(fields, p0);
    LXValue p1 = px_list(2); px_list_push(p1, px_str(H3_SCHEME)); px_list_push(p1, px_str(scheme)); px_list_push(fields, p1);
    LXValue p2 = px_list(2); px_list_push(p2, px_str(H3_AUTH)); px_list_push(p2, px_str(auth)); px_list_push(fields, p2);
    LXValue p3 = px_list(2); px_list_push(p3, px_str(H3_PATH)); px_list_push(p3, px_str(path)); px_list_push(fields, p3);
    h3_append_headers(fields, headers_val);
    return fields;
}

// ==================== M51：连接级会话管理（QPACK 动态表接入线上）====================

// h3_conn_setup(conn, qpack_cap:int) -> bool
// 为连接建立 HTTP/3 会话：开 3 条单向流（控制流=首条 uni + SETTINGS、QPACK 编码器流、
// 解码器流），建 QPACK 动态表会话。幂等。之后该连接请求/响应自动走动态表编解码。
static LXValue bi_h3_conn_setup(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("h3_conn_setup 需要 (conn: int, qpack_cap: int)");
    int64_t conn = args[0].as.i;
    int64_t cap = args[1].as.i;
    h3conn_state* st = h3_st(conn);
    if (!st) return px_bool(false);
    if (st->used) return px_bool(true);               // 幂等
    int64_t ctrl = px_quic_raw_open_uni_stream(conn); // 首条 uni = 控制流（RFC 9114 §6.2.1）
    if (ctrl < 0) return px_bool(false);
    int64_t enc = px_quic_raw_open_uni_stream(conn);
    if (enc < 0) return px_bool(false);
    int64_t dec = px_quic_raw_open_uni_stream(conn);
    if (dec < 0) return px_bool(false);
    int64_t qd = px_qd_open(cap < 0 ? 0 : cap);
    if (qd <= 0) return px_bool(false);
    uint8_t buf[512];
    buf[0] = H3_UT_CONTROL;
    int n = h3_build_settings_frame(buf + 1, (int)(cap < 0 ? 0 : cap), 100);
    px_quic_raw_send_on(conn, ctrl, buf, n + 1, 0);
    uint8_t t1 = H3_UT_ENCODER;
    px_quic_raw_send_on(conn, enc, &t1, 1, 0);
    uint8_t t2 = H3_UT_DECODER;
    px_quic_raw_send_on(conn, dec, &t2, 1, 0);
    st->used = 1; st->qd = qd;
    st->ctrl_sid = ctrl; st->enc_sid = enc; st->dec_sid = dec;
    st->peer_qcap = -1; st->peer_blocked = -1;
    return px_bool(true);
}

// h3_conn_close(conn) -> bool：关闭连接级会话（释放 QPACK 会话），连接本身不关
static LXValue bi_h3_conn_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_bool(false);
    h3conn_state* st = h3_st(args[0].as.i);
    if (!st || !st->used) return px_bool(false);
    if (st->qd > 0) px_qd_close(st->qd);
    memset(st, 0, sizeof(*st));
    return px_bool(true);
}

// h3_conn_peer(conn) -> dict|null：对端 HTTP/3 会话状态（uni 流分类 / SETTINGS / ingest 统计）
static LXValue bi_h3_conn_peer(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_null();
    h3conn_state* st = h3_st(args[0].as.i);
    if (!st || !st->used) return px_null();
    LXValue d = px_dict();
    px_dict_set(d, "peer_ctrl", px_int(st->peer_ctrl));
    px_dict_set(d, "peer_enc", px_int(st->peer_enc));
    px_dict_set(d, "peer_dec", px_int(st->peer_dec));
    px_dict_set(d, "peer_qcap", px_int(st->peer_qcap));
    px_dict_set(d, "peer_blocked", px_int(st->peer_blocked));
    px_dict_set(d, "peer_enc_bytes", px_int(st->peer_enc_bytes));
    return d;
}

// h3_conn_stats(conn) -> dict|null：本端会话统计（QPACK 表大小 / 编码器流发送 / 阻塞恢复）
static LXValue bi_h3_conn_stats(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_null();
    h3conn_state* st = h3_st(args[0].as.i);
    if (!st || !st->used) return px_null();
    LXValue d = px_dict();
    px_dict_set(d, "qd", px_int(st->qd));
    px_dict_set(d, "ctrl_sid", px_int(st->ctrl_sid));
    px_dict_set(d, "enc_sid", px_int(st->enc_sid));
    px_dict_set(d, "dec_sid", px_int(st->dec_sid));
    px_dict_set(d, "enc_sent", px_int(st->enc_sent));   // 本端编码器流累计发送字节
    px_dict_set(d, "enc_sects", px_int(st->enc_sects)); // 动态表编码字段段数
    px_dict_set(d, "blocked_cnt", px_int(st->blocked_cnt));
    px_dict_set(d, "en_len", px_int(px_qd_en_len(st->qd)));    // 本端编码表条目
    px_dict_set(d, "de_len", px_int(px_qd_de_len(st->qd)));    // 对端镜像表条目
    px_dict_set(d, "ins", px_int(px_qd_ins(st->qd)));          // 已 ingest 插入数
    return d;
}

// ==================== 服务端多流 API（M50 新）====================

// h3_serve_poll_stream(conn, timeout_ms) -> int：等任一对端双向流（请求流）有数据 → sid | -1。
// M51 增强：对端单向流（控制/编码器/解码器）自动分类消费（SETTINGS 解析 / QPACK ingest）。
static LXValue bi_h3_serve_poll_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("h3_serve_poll_stream 需要 (conn, timeout_ms)");
    int64_t conn = args[0].as.i;
    int64_t timeout = args[1].as.i;
    int64_t sid = h3_poll_requests(conn, timeout);
    return px_int(sid);
}

// h3_serve_read_request_stream(conn, sid, timeout_ms) -> dict|null
static LXValue bi_h3_serve_read_request_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 3 || args[0].type != PX_INT || args[1].type != PX_INT || args[2].type != PX_INT)
        px_error("h3_serve_read_request_stream 需要 (conn, sid, timeout_ms)");
    int64_t conn = args[0].as.i;
    int64_t sid = args[1].as.i;
    int64_t timeout = args[2].as.i;
    LXValue fields; uint8_t* bd = NULL; int blen = 0;
    int st = h3_read_section(conn, sid, timeout, &fields, &bd, &blen);
    if (st != 0) return px_null();
    LXValue d = h3_fields_to_request(fields, sid, bd, blen);
    free(bd);
    if (conn > 0 && conn <= H3_MAX_CONN) g_last_sid[conn - 1] = sid;
    return d;
}

// h3_serve_send_response_stream(conn, sid, status:int, headers:list, body) -> bool
static LXValue bi_h3_serve_send_response_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 5 || args[0].type != PX_INT || args[1].type != PX_INT || args[2].type != PX_INT)
        px_error("h3_serve_send_response_stream 需要 (conn, sid, status:int, headers:list, body)");
    int64_t conn = args[0].as.i;
    int64_t sid = args[1].as.i;
    int status = (int)args[2].as.i;
    LXValue fields = h3_make_response_fields(status, args[3]);
    bool ok = h3_send_fields(conn, sid, fields, args[4]);
    return px_bool(ok);
}

// ==================== 客户端多流 API（M50 新）====================

// h3_client_open_stream(conn) -> int：open 一条新请求双向流
static LXValue bi_h3_client_open_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) px_error("h3_client_open_stream 需要 (conn)");
    int64_t conn = args[0].as.i;
    int64_t sid = px_quic_raw_open_stream(conn);
    return px_int(sid);
}

// h3_client_send_request_stream(conn, sid, method, scheme, authority, path, headers, body) -> bool
static LXValue bi_h3_client_send_request_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 8 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("h3_client_send_request_stream 需要 (conn, sid, method, scheme, authority, path, headers, body)");
    int64_t conn = args[0].as.i;
    int64_t sid = args[1].as.i;
    if (args[2].type != PX_STR || args[3].type != PX_STR || args[4].type != PX_STR || args[5].type != PX_STR)
        return px_bool(false);
    LXValue fields = h3_make_request_fields(args[2].as.obj->as.str.data,
                                            args[3].as.obj->as.str.data,
                                            args[4].as.obj->as.str.data,
                                            args[5].as.obj->as.str.data, args[6]);
    bool ok = h3_send_fields(conn, sid, fields, args[7]);
    if (ok && conn > 0 && conn <= H3_MAX_CONN) g_last_sid[conn - 1] = sid;
    return px_bool(ok);
}

// h3_client_read_response_stream(conn, sid, timeout_ms) -> dict|null
static LXValue bi_h3_client_read_response_stream(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 3 || args[0].type != PX_INT || args[1].type != PX_INT || args[2].type != PX_INT)
        px_error("h3_client_read_response_stream 需要 (conn, sid, timeout_ms)");
    int64_t conn = args[0].as.i;
    int64_t sid = args[1].as.i;
    int64_t timeout = args[2].as.i;
    LXValue fields; uint8_t* bd = NULL; int blen = 0;
    int st = h3_read_section(conn, sid, timeout, &fields, &bd, &blen);
    if (st != 0) return px_null();
    LXValue d = h3_fields_to_response(fields, sid, bd, blen);
    free(bd);
    return d;
}

// ==================== 旧 API 兼容（M47 单流语义 = 默认流）====================

// h3_serve_read_request(conn, timeout_ms) -> dict|null
static LXValue bi_h3_serve_read_request(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT) px_error("h3_serve_read_request 需要 (conn, timeout_ms)");
    int64_t conn = args[0].as.i;
    int64_t timeout = args[1].as.i;
    // 默认流：先等一条 peer 流出现（poll），再从该流读完整请求
    int64_t sid = px_quic_raw_first_stream(conn);
    if (sid < 0) sid = px_quic_raw_poll(conn, (int)timeout);
    if (sid < 0) return px_null();
    LXValue fields; uint8_t* bd = NULL; int blen = 0;
    int st = h3_read_section(conn, sid, timeout, &fields, &bd, &blen);
    if (st != 0) return px_null();
    LXValue d = h3_fields_to_request(fields, sid, bd, blen);
    free(bd);
    if (conn > 0 && conn <= H3_MAX_CONN) g_last_sid[conn - 1] = sid;
    return d;
}

// h3_serve_send_response(conn, status:int, headers:list, body:str) -> bool
static LXValue bi_h3_serve_send_response(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 4 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("h3_serve_send_response 需要 (conn, status:int, headers:list, body)");
    int64_t conn = args[0].as.i;
    int status = (int)args[1].as.i;
    int64_t sid = (conn > 0 && conn <= H3_MAX_CONN) ? g_last_sid[conn - 1] : -1;
    if (sid < 0) sid = px_quic_raw_first_stream(conn);
    if (sid < 0) return px_bool(false);
    LXValue fields = h3_make_response_fields(status, args[2]);
    bool ok = h3_send_fields(conn, sid, fields, args[3]);
    return px_bool(ok);
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
    if (c > 0 && c <= H3_MAX_CONN) g_last_sid[c - 1] = -1;
    return px_int(c);
}

// h3_client_send_request(conn, method, scheme, authority, path, headers, body) -> bool
static LXValue bi_h3_client_send_request(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 7 || args[0].type != PX_INT) px_error("h3_client_send_request 需要 (conn, method, scheme, authority, path, headers, body)");
    int64_t conn = args[0].as.i;
    if (args[1].type != PX_STR || args[2].type != PX_STR || args[3].type != PX_STR || args[4].type != PX_STR)
        return px_bool(false);
    int64_t sid = px_quic_raw_first_stream(conn);   // 默认 = 首条流（M47 connect 已 open）
    if (sid < 0) return px_bool(false);
    LXValue fields = h3_make_request_fields(args[1].as.obj->as.str.data,
                                            args[2].as.obj->as.str.data,
                                            args[3].as.obj->as.str.data,
                                            args[4].as.obj->as.str.data, args[5]);
    bool ok = h3_send_fields(conn, sid, fields, args[6]);
    if (ok && conn > 0 && conn <= H3_MAX_CONN) g_last_sid[conn - 1] = sid;
    return px_bool(ok);
}

// h3_client_read_response(conn, timeout_ms) -> dict|null
static LXValue bi_h3_client_read_response(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT) px_error("h3_client_read_response 需要 (conn, timeout_ms)");
    int64_t conn = args[0].as.i;
    int64_t timeout = args[1].as.i;
    int64_t sid = (conn > 0 && conn <= H3_MAX_CONN) ? g_last_sid[conn - 1] : -1;
    if (sid < 0) return px_null();
    LXValue fields; uint8_t* bd = NULL; int blen = 0;
    int st = h3_read_section(conn, sid, timeout, &fields, &bd, &blen);
    if (st != 0) return px_null();
    LXValue d = h3_fields_to_response(fields, sid, bd, blen);
    free(bd);
    return d;
}

// ==================== 注册（runtime.c px_register_builtins 调用）====================
void px_register_h3(void) {
    px_set_global("h3_qenc", px_native("h3_qenc", bi_h3_qenc));
    px_set_global("h3_qdec", px_native("h3_qdec", bi_h3_qdec));
    px_set_global("h3_huff", px_native("h3_huff", bi_h3_huff));
    px_set_global("h3_unhuff", px_native("h3_unhuff", bi_h3_unhuff));
    px_set_global("h3_frame", px_native("h3_frame", bi_h3_frame));
    px_set_global("h3_serve_read_request", px_native("h3_serve_read_request", bi_h3_serve_read_request));
    px_set_global("h3_serve_send_response", px_native("h3_serve_send_response", bi_h3_serve_send_response));
    px_set_global("h3_serve_poll_stream", px_native("h3_serve_poll_stream", bi_h3_serve_poll_stream));
    px_set_global("h3_serve_read_request_stream", px_native("h3_serve_read_request_stream", bi_h3_serve_read_request_stream));
    px_set_global("h3_serve_send_response_stream", px_native("h3_serve_send_response_stream", bi_h3_serve_send_response_stream));
    px_set_global("h3_client_connect", px_native("h3_client_connect", bi_h3_client_connect));
    px_set_global("h3_client_open_stream", px_native("h3_client_open_stream", bi_h3_client_open_stream));
    px_set_global("h3_client_send_request", px_native("h3_client_send_request", bi_h3_client_send_request));
    px_set_global("h3_client_send_request_stream", px_native("h3_client_send_request_stream", bi_h3_client_send_request_stream));
    px_set_global("h3_client_read_response", px_native("h3_client_read_response", bi_h3_client_read_response));
    px_set_global("h3_client_read_response_stream", px_native("h3_client_read_response_stream", bi_h3_client_read_response_stream));
    px_set_global("h3_conn_setup", px_native("h3_conn_setup", bi_h3_conn_setup));
    px_set_global("h3_conn_close", px_native("h3_conn_close", bi_h3_conn_close));
    px_set_global("h3_conn_peer", px_native("h3_conn_peer", bi_h3_conn_peer));
    px_set_global("h3_conn_stats", px_native("h3_conn_stats", bi_h3_conn_stats));
    px_ffi_register("h3_qenc", bi_h3_qenc);
    px_ffi_register("h3_qdec", bi_h3_qdec);
    px_ffi_register("h3_huff", bi_h3_huff);
    px_ffi_register("h3_unhuff", bi_h3_unhuff);
    px_ffi_register("h3_frame", bi_h3_frame);
    px_ffi_register("h3_serve_read_request", bi_h3_serve_read_request);
    px_ffi_register("h3_serve_send_response", bi_h3_serve_send_response);
    px_ffi_register("h3_serve_poll_stream", bi_h3_serve_poll_stream);
    px_ffi_register("h3_serve_read_request_stream", bi_h3_serve_read_request_stream);
    px_ffi_register("h3_serve_send_response_stream", bi_h3_serve_send_response_stream);
    px_ffi_register("h3_client_connect", bi_h3_client_connect);
    px_ffi_register("h3_client_open_stream", bi_h3_client_open_stream);
    px_ffi_register("h3_client_send_request", bi_h3_client_send_request);
    px_ffi_register("h3_client_send_request_stream", bi_h3_client_send_request_stream);
    px_ffi_register("h3_client_read_response", bi_h3_client_read_response);
    px_ffi_register("h3_client_read_response_stream", bi_h3_client_read_response_stream);
    px_ffi_register("h3_conn_setup", bi_h3_conn_setup);
    px_ffi_register("h3_conn_close", bi_h3_conn_close);
    px_ffi_register("h3_conn_peer", bi_h3_conn_peer);
    px_ffi_register("h3_conn_stats", bi_h3_conn_stats);
}
