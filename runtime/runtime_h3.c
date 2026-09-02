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
#include <ctype.h>
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
    int64_t sid;              // 已发动态字段段(ric>0)所在流
    uint64_t ric;             // 该字段段 Required Insert Count（待对端 Section Ack）
    int used;
} h3_outack;

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
    // M52：解码器流 ack 闭环（RFC 9204 §4.4）
    h3_outack out_ack[H3_STREAM_SLOTS]; // 本端已发、待对端 ack 的动态字段段（每流最多一条在途）
    int64_t dec_sent;      // 本端解码器流累计发送 ack 指令字节
    int64_t dec_sects;     // 本端已发 Section Ack 的字段段数
    int64_t enc_acks;      // 收对端解码器流指令数（Section Ack + Insert Count Increment）
} h3conn_state;
static h3conn_state g_h3st[H3_MAX_CONN];
static h3conn_state* h3_st(int64_t conn) {
    return (conn > 0 && conn <= H3_MAX_CONN) ? &g_h3st[conn - 1] : NULL;
}

// QUIC 流 id 次低位（0x2）：0=双向 / 1=单向（RFC 9000 §2.1；最低位 0x1 是发起者标志）
static int h3_sid_is_uni(int64_t sid) { return (int)(sid & 2) ? 1 : 0; }

// ==================== M52：解码器流 ack 闭环辅助 ====================
// QPACK prefixed integer 解码（RFC 7541 §5.1，prefix=N bits）→ 值 + 返回使用字节数 | 0
static int h3_qp_pref_dec(const uint8_t* p, int maxlen, int prefix_bits, uint64_t* out) {
    if (maxlen < 1) return 0;
    int mask = (1 << prefix_bits) - 1;
    uint64_t v = p[0] & (uint64_t)mask;
    if (v < (uint64_t)mask) { *out = v; return 1; }
    if (maxlen < 2) return 0;
    uint64_t m = 0; int off = 1;
    for (;;) {
        if (off >= maxlen) return 0;
        v += (uint64_t)(p[off] & 0x7f) * (1ULL << m);
        if ((p[off] & 0x80) == 0) break;
        m += 7;
        if (m > 62) return 0;
        off++;
    }
    *out = v;
    return off + 1;
}

// 登记本端已发字段段（ric>0）到 outstanding（同流覆盖——MVP 每流同步单条在途）
static void h3_outack_push(h3conn_state* st, int64_t sid, uint64_t ric) {
    if (!st || sid <= 0 || ric == 0) return;
    for (int i = 0; i < H3_STREAM_SLOTS; i++)
        if (st->out_ack[i].used && st->out_ack[i].sid == sid) {
            st->out_ack[i].ric = ric; return;
        }
    for (int i = 0; i < H3_STREAM_SLOTS; i++)
        if (!st->out_ack[i].used) {
            st->out_ack[i].used = 1; st->out_ack[i].sid = sid; st->out_ack[i].ric = ric;
            return;
        }
}
// 取走（并清除）某流的 outstanding RIC（收到 Section Ack/Stream Cancel 时）→ 0 无
static uint64_t h3_outack_take(h3conn_state* st, int64_t sid) {
    if (!st || sid <= 0) return 0;
    for (int i = 0; i < H3_STREAM_SLOTS; i++)
        if (st->out_ack[i].used && st->out_ack[i].sid == sid) {
            uint64_t ric = st->out_ack[i].ric;
            st->out_ack[i].used = 0; st->out_ack[i].sid = 0; st->out_ack[i].ric = 0;
            return ric;
        }
    return 0;
}

// 本端解码完对端字段段（ric>0）→ 在解码器流发 Section Ack（RFC 9204 §4.4.1）
static void h3_send_dec_ack(int64_t conn, h3conn_state* st, int64_t sid, int64_t ric) {
    if (!st || !st->used || st->qd <= 0 || st->dec_sid <= 0 || ric <= 0) return;
    uint8_t buf[16];
    int n = px_qd_dec_inst_section_ack(buf, (int)sizeof(buf), sid);
    if (n <= 0) return;
    if (px_quic_raw_send_on(conn, st->dec_sid, buf, n, 0) >= 0) {
        st->dec_sent += n;
        st->dec_sects++;
    }
}

// 消费对端解码器流指令（数据不含流类型字节）：Section Ack / Stream Cancellation /
// Insert Count Increment（RFC 9204 §4.4）。返回 0 正常 | -1 非法（连接级错误）。
static int h3_decoder_ingest(int64_t conn, h3conn_state* st, const uint8_t* p, int n) {
    (void)conn;
    int off = 0;
    while (off < n) {
        uint8_t b0 = p[off];
        int used = 0;
        uint64_t v = 0;
        if (b0 & 0x80) {
            // Section Acknowledgment：1 + Stream ID (7+)。ack 提升 KRC 到该流最早
            // 未确认字段段的 RIC（本端发送时按流登记过，RFC 9204 §4.4.1/§2.1.4）
            int r = h3_qp_pref_dec(p + off, n - off, 7, &v);
            if (r <= 0) return -1;
            off += r;
            uint64_t ric = h3_outack_take(st, (int64_t)v);
            if (ric > 0) {
                if (px_qd_ack_sec(st->qd, (int64_t)ric) != 0) return -1;
                st->enc_acks++;
            }
            continue;   // 无 outstanding（重复/未知流 ack）→ 宽松忽略
        } else if ((b0 & 0xC0) == 0x40) {
            // Stream Cancellation：01 + Stream ID (6+)
            int r = h3_qp_pref_dec(p + off, n - off, 6, &v);
            if (r <= 0) return -1;
            off += r;
            (void)h3_outack_take(st, (int64_t)v);   // 该流引用全部取消（不推进 KRC）
            continue;
        } else if ((b0 & 0xC0) == 0x00) {
            // Insert Count Increment：00 + Increment (6+)
            int r = h3_qp_pref_dec(p + off, n - off, 6, &v);
            if (r <= 0) return -1;
            off += r;
            if (px_qd_ack_inc(st->qd, (int64_t)v) != 0) return -1;
            st->enc_acks++;
            continue;
        } else {
            return -1;
        }
    }
    return 0;
}


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
// 0x03 解码器），随后按类型消费（SETTINGS 解析 / QPACK ingest / 解码器指令解析）。
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
    } else if (sid == st->peer_dec) {
        // M52：解码器流指令（Section Ack / Stream Cancel / Insert Count Increment）→ 推进 KRC
        if (st->qd > 0 && dl > 0 && h3_decoder_ingest(conn, st, d, dl) != 0) return -1;
    }
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
        // M53-S4 互操作修复：HTTP/3 要求头字段名小写（RFC 9114 §4.2；QPACK 静态/动态表
        // 字段名亦小写）。管道 handler 返回的 "Content-Type"/"X-Request-Id" 大小写任意
        // （HTTP/1.1 允许），编码为 H3 前统一 tolower —— 否则 aioquic 等严格实现报
        // H3_MESSAGE_ERROR（Header contains invalid characters）拒收。
        const char* _n = kv[0].as.obj->as.str.data;
        int _nl = kv[0].as.obj->as.str.len;
        char* lname = (char*)malloc((size_t)_nl + 1);
        if (!lname) continue;
        for (int _i = 0; _i < _nl; _i++) lname[_i] = (char)tolower((unsigned char)_n[_i]);
        lname[_nl] = 0;
        LXValue pair = px_list(2);
        px_list_push(pair, px_str(lname));
        px_list_push(pair, kv[1]);
        px_list_push(fields, pair);
        free(lname);
    }
}

// 编码 fields 并以 HEADERS+DATA 两帧写到指定流
// M51：连接已 setup → QPACK 动态表编码（encoder 指令 flush 到本端编码器单向流）；
//      未 setup → 无状态 codec（M47–M50 行为不变）。
// fin=1：末字节后关闭本端写侧（响应流 FIN，RFC 9114）；请求侧暂传 0（帧边界界定消息）。
static bool h3_send_fields(int64_t conn, int64_t sid, LXValue fields, LXValue body_val, int fin) {
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
            // M52：登记 outstanding（ric>0 → 对端须 Section Ack，RFC 9204 §2.2.2.1）
            int ric = px_qd_enc_last_ric(st->qd);
            if (ric > 0) h3_outack_push(st, sid, (uint64_t)ric);
        }
    } else {
        qn = px_h3_qenc(q, fo->as.list.items, fo->as.list.len);
    }
    if (qn < 0) qn = 0;   // 原行为：编码失败发空字段段（理论不发生）
    int fn1 = h3_build_frame(f, H3_FRAME_HEADERS, q, qn);
    int fn2 = h3_build_frame(f + fn1, H3_FRAME_DATA, bd, blen);
    int64_t sent = px_quic_raw_send_on(conn, sid, f, fn1 + fn2, fin);
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
        // M52：解码完成 → ric>0 则向对端发 Section Ack（RFC 9204 §2.2.2.1：含动态引用必须 ack）
        int dec_ric = px_qd_dec_last_ric(st->qd);
        if (dec_ric > 0) h3_send_dec_ack(conn, st, sid, dec_ric);
    } else {
        fields = px_h3_qdec(hd, hlen);
    }
    free(hd);
    if (fields.type != PX_LIST) return -2;
    uint8_t* bd = NULL; int blen = 0;
    t = h3_take_frame(conn, sid, &bd, &blen, timeout_ms);
    if (t != H3_FRAME_DATA) {
        // M53-S4 互操作修复：GET 等无 body 请求 = HEADERS 帧后流即 FIN（无 DATA 帧）。
        // 对端流已结束（fin 已到、数据已读空 → recv 立即返回 0）→ body 视为空，合法；
        // 否则（超时/其他帧/协议错误）→ 失败。自研 client 总发 DATA 帧（含空 body），
        // aioquic 等标准实现按 RFC 9114 不发空 DATA —— 此前强制要 DATA 帧导致标准
        // 客户端请求被拒（read_section -1），是外部互操作失败根因之一。
        if (t == -1 && px_quic_raw_stream_fin(conn, sid) == 1) {
            free(bd); bd = NULL; blen = 0;   // 无 body：HEADERS + FIN
        } else {
            free(bd); return -1;
        }
    }
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
// M53-S3：C 侧实现（托管 H3 server 连接回调复用，不经语言层）。
static bool h3_conn_setup_c(int64_t conn, int64_t cap) {
    h3conn_state* st = h3_st(conn);
    if (!st) return false;
    if (st->used) return true;               // 幂等
    int64_t ctrl = px_quic_raw_open_uni_stream(conn); // 首条 uni = 控制流（RFC 9114 §6.2.1）
    if (ctrl < 0) return false;
    int64_t enc = px_quic_raw_open_uni_stream(conn);
    if (enc < 0) return false;
    int64_t dec = px_quic_raw_open_uni_stream(conn);
    if (dec < 0) return false;
    int64_t qd = px_qd_open(cap < 0 ? 0 : cap);
    if (qd <= 0) return false;
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
    return true;
}

static LXValue bi_h3_conn_setup(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT || args[1].type != PX_INT)
        px_error("h3_conn_setup 需要 (conn: int, qpack_cap: int)");
    int64_t conn = args[0].as.i;
    int64_t cap = args[1].as.i;
    return px_bool(h3_conn_setup_c(conn, cap));
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
    px_dict_set(d, "krc", px_int(px_qd_krc(st->qd)));          // M52：本端编码器 Known Received Count
    px_dict_set(d, "dec_sent", px_int(st->dec_sent));          // M52：本端解码器流已发 ack 字节
    px_dict_set(d, "dec_sects", px_int(st->dec_sects));        // M52：本端已发 Section Ack 的字段段数
    px_dict_set(d, "enc_acks", px_int(st->enc_acks));          // M52：收对端解码器流指令数
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
    bool ok = h3_send_fields(conn, sid, fields, args[4], 1);
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
    bool ok = h3_send_fields(conn, sid, fields, args[7], 0);
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
    bool ok = h3_send_fields(conn, sid, fields, args[3], 1);
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
    bool ok = h3_send_fields(conn, sid, fields, args[6], 0);
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

// ==================== M53-S3：HTTP/3 接入公共 HTTP 管道 ====================
// 目标：把 M46–M52 的 H3 语义层接进 px_serve 的 vhost/路由/限流/日志/静态/.px 管道。
//   每连接（托管 listener 自动 accept）一个处理线程跑本回调：
//   握手完成 → h3_conn_setup（QPACK 动态表会话）→ 循环 h3_poll_requests（自动消费
//   对端单向流：控制 SETTINGS / 编码器 ingest / 解码器 ack）→ h3_read_section 解码
//   请求（QPACK 阻塞自动泵对端编码器流；RIC>0 自动 Section Ack）→ 组 req dict
//   （method/path/headers/body/remote/version="HTTP/3"）→ px_http_dispatch_h3 补全
//   query/cookie/form 后送入公共管道 → 响应经 PxHttpOut H3 实现编码为 HEADERS/DATA。
// 输出约定（与 HTTP/1.1 对齐）：:status + content-type/content-length + 管道 extra 头
// （X-Request-Id/ETag/Last-Modified/Content-Encoding/Content-Range…；跳过 Connection/
// Alt-Svc/Transfer-Encoding 等 H3 无意义或由协商承担的传输头）。body ≤ 单消息上限走
// HEADERS+DATA 一次组装（自研 MVP client 兼容）；大 body 走 HEADERS + 多 DATA 分帧
// （RFC 9114 §6.2 标准，aioquic/S4 验证完整接收）。
#define H3_SEND_SINGLE_MAX (700 * 1024)     // 单消息（HEADERS+DATA 一次组装）body 上限
#define H3_DATA_CHUNK_MAX  (600 * 1024)     // 分帧模式每 DATA 帧体上限
#define H3_OUT_BODY_MAX    (16 * 1024 * 1024) // H3 响应体缓冲 MVP 上限（超限截断）

// extra_headers（"\r\n" 结尾 "K: V" 文本，同 HTTP/1.1 begin 入参）→ headers list
static void h3_extra_to_headers(LXValue hdrlist, const char* extra) {
    if (!extra || !*extra) return;
    const char* p = extra;
    while (*p) {
        const char* eol = strstr(p, "\r\n");
        int ln = eol ? (int)(eol - p) : (int)strlen(p);
        char line[1024];
        int cl = ln < (int)sizeof(line) - 1 ? ln : (int)sizeof(line) - 1;
        memcpy(line, p, (size_t)cl); line[cl] = 0;
        if (line[0] && line[0] != '\r') {
            char* colon = strchr(line, ':');
            if (colon) {
                *colon = 0;
                char* k = line;
                char* v = colon + 1;
                while (*v == ' ') v++;
                char* ve = v + strlen(v);
                while (ve > v && ve[-1] == ' ') ve--;
                *ve = 0;
                if (!(strcasecmp(k, "Connection") == 0 ||
                      strcasecmp(k, "Alt-Svc") == 0 ||
                      strcasecmp(k, "Transfer-Encoding") == 0)) {
                    LXValue pair = px_list(2);
                    px_list_push(pair, px_str(k));
                    px_list_push(pair, px_str(v));
                    px_list_push(hdrlist, pair);
                }
            }
        }
        if (!eol) break;
        p = eol + 2;
    }
}

// H3 输出上下文：缓冲整个响应，end() 时编码发送（单线程串行，无需跨响应并发）
typedef struct {
    int64_t conn, sid;
    int status;
    int head_only;
    int done;             // 响应已发送（防重复）
    LXValue fields;       // 响应字段（:status + headers list）
    uint8_t* body;        // body 缓冲（begin/write 累积）
    int blen, bcap;
} h3_out_ctx;

static LXValue h3_out_make_fields(int status, const char* ct, long long body_len,
                                  int head_only, const char* extra) {
    LXValue hdr = px_list(16);
    if (ct && *ct) {
        LXValue p = px_list(2); px_list_push(p, px_str("content-type")); px_list_push(p, px_str(ct)); px_list_push(hdr, p);
    }
    // HEAD 也带 Content-Length（同 HTTP/1.1 begin：头总是有 Content-Length）
    {
        char lb[32]; snprintf(lb, sizeof(lb), "%lld", body_len);
        LXValue p = px_list(2); px_list_push(p, px_str("content-length")); px_list_push(p, px_str(lb)); px_list_push(hdr, p);
    }
    h3_extra_to_headers(hdr, extra);
    (void)head_only;
    return h3_make_response_fields(status, hdr);
}

static void h3_out_begin(PxHttpOut* o, int status, const char* ct, long long body_len,
                         int head_only, int keep_alive, const char* extra_headers) {
    (void)keep_alive;   // H3 无 Connection 语义：总是 keep-alive
    h3_out_ctx* c = (h3_out_ctx*)o->impl;
    c->status = status;
    c->head_only = head_only;
    c->done = 0;
    c->fields = h3_out_make_fields(status, ct, body_len, head_only, extra_headers);
    c->blen = 0;
    if (body_len > 0 && body_len < H3_OUT_BODY_MAX && !c->body) {
        c->body = (uint8_t*)malloc((size_t)body_len);
        if (c->body) c->bcap = (int)body_len;
    }
}

static int h3_out_write(PxHttpOut* o, const void* buf, size_t n) {
    h3_out_ctx* c = (h3_out_ctx*)o->impl;
    if (n == 0) return 0;
    if (c->blen + (int)n > c->bcap) {
        int nc = c->bcap > 0 ? c->bcap * 2 : 65536;
        while (nc < c->blen + (int)n && nc < H3_OUT_BODY_MAX) nc *= 2;
        if (nc >= H3_OUT_BODY_MAX) nc = H3_OUT_BODY_MAX;
        if (nc < c->blen + (int)n) return -1;   // 超 MVP 上限
        uint8_t* nd = (uint8_t*)realloc(c->body, (size_t)nc);
        if (!nd) return -1;
        c->body = nd;
        c->bcap = nc;
    }
    memcpy(c->body + c->blen, buf, n);
    c->blen += (int)n;
    return (int)n;
}

// 大响应（body > 单消息上限）：HEADERS 帧（无状态 QPACK 编码）+ 分块 DATA。
// 无状态字段段在已 setup 连接上同样合法（QPACK 允许无动态引用）；本路径不产生
// 动态表统计（enc_sects 等）——只影响超大响应，API 层统计由 m52 小响应路径覆盖。
static bool h3_send_response_big(int64_t conn, int64_t sid, LXValue fields,
                                 const uint8_t* bd, int blen) {
    LXObject* fo = fields.as.obj;
    uint8_t* q = (uint8_t*)malloc(H3_BUF_MAX);
    uint8_t* f = (uint8_t*)malloc(H3_BUF_MAX + 16);
    if (!q || !f) { free(q); free(f); return false; }
    int qn = px_h3_qenc(q, fo->as.list.items, fo->as.list.len);
    if (qn < 0) qn = 0;
    int fn = h3_build_frame(f, H3_FRAME_HEADERS, q, qn);
    int64_t s1 = px_quic_raw_send_on(conn, sid, f, fn, 0);
    bool ok = s1 >= 0;
    int off = 0;
    while (off < blen) {
        int chunk = blen - off;
        if (chunk > H3_DATA_CHUNK_MAX) chunk = H3_DATA_CHUNK_MAX;
        fn = h3_build_frame(f, H3_FRAME_DATA, bd + off, chunk);
        if (px_quic_raw_send_on(conn, sid, f, fn, 0) < 0) { ok = false; break; }
        off += chunk;
    }
    if (blen == 0) {   // 空 body 也发一个空 DATA（对齐单消息 HEADERS+DATA 语义）
        fn = h3_build_frame(f, H3_FRAME_DATA, NULL, 0);
        if (px_quic_raw_send_on(conn, sid, f, fn, 0) < 0) ok = false;
    }
    // M53-S4：大响应结束 → 流 FIN（RFC 9114 响应以流结束结束；aioquic 等标准客户端依赖）
    px_quic_raw_send_on(conn, sid, NULL, 0, 1);
    free(q); free(f);
    return ok;
}

static void h3_out_send(h3_out_ctx* c) {
    const uint8_t* bd = c->body ? c->body : (const uint8_t*)"";
    int blen = c->head_only ? 0 : c->blen;
    if (blen <= H3_SEND_SINGLE_MAX) {
        LXValue bv = px_bytes_len(bd, blen);
        h3_send_fields(c->conn, c->sid, c->fields, bv, 1);
    } else {
        h3_send_response_big(c->conn, c->sid, c->fields, bd, blen);
    }
}

static void h3_out_end(PxHttpOut* o) {
    h3_out_ctx* c = (h3_out_ctx*)o->impl;
    if (!c->done) { c->done = 1; h3_out_send(c); }
}

static void h3_out_respond(PxHttpOut* o, int status, const char* ct, const char* body,
                           int body_len, int head_only, int keep_alive,
                           const char* extra_headers) {
    h3_out_begin(o, status, ct, (long long)body_len, head_only, keep_alive, extra_headers);
    if (!head_only && body_len > 0) h3_out_write(o, body, (size_t)body_len);
    h3_out_end(o);
}

// H3 输出实现初始化（impl=h3_out_ctx*；请求回调每请求建 ctx，用后 free(ctx->body)）
static void px_http_out_init_h3(PxHttpOut* o, h3_out_ctx* c, int64_t conn, int64_t sid) {
    memset(o, 0, sizeof(*o));
    memset(c, 0, sizeof(*c));
    c->conn = conn;
    c->sid = sid;
    c->done = 1;      // 未 begin（无响应）时 end 不发送
    o->impl = c;
    o->respond = h3_out_respond;
    o->begin = h3_out_begin;
    o->write = h3_out_write;
    o->end = h3_out_end;
}

// ==================== M53-S3：托管连接回调（请求 → 公共管道） ====================
// 由托管 listener 的连接处理线程调用（握手完成后，见 quic_srv_conn_thr）；
// 回调返回后连接槽位被清理，故本函数必须完整跑完该连接生命周期（空闲退出）。
static void h3_srv_pipe_cb(int64_t conn, void* ud) {
    (void)ud;
    if (!h3_conn_setup_c(conn, 4096)) return;
    char peer[96];
    px_quic_raw_peer_addr(conn, peer, sizeof(peer));
    int idle = 0;
    for (;;) {
        int64_t sid = h3_poll_requests(conn, 8000);   // 自动消费对端单向流（SETTINGS/QPACK）
        if (sid < 0) { if (++idle >= 2) break; continue; }  // 2×8s 空闲 → 关闭（同 keep-alive 超时）
        idle = 0;
        LXValue fields;
        uint8_t* bd = NULL;
        int blen = 0;
        if (h3_read_section(conn, sid, 8000, &fields, &bd, &blen) != 0) {
            if (bd) free(bd);
            continue;
        }
        LXValue req = h3_fields_to_request(fields, sid, bd, blen);
        free(bd);
        if (req.type != PX_DICT) continue;
        px_dict_set(req, "remote", px_str(peer));
        PxHttpOut out;
        h3_out_ctx ctx;
        px_http_out_init_h3(&out, &ctx, conn, sid);
        // 补全 req dict（query/version/cookie/form/request_id…）并送公共管道；响应经 out
        px_http_dispatch_h3(&out, req, 1);
        if (ctx.body) free(ctx.body);
    }
}

// M53-S4：px_serve opts.http3 内部入口 —— 公共 HTTP 管道托管 H3 listener
// （与 h3_server_listen 同语义；runtime.c px_serve 启动时调用，实现三栈合一 WebServer）。
int64_t px_h3_server_listen_pipe(int port, const char* cert, const char* key) {
    return px_quic_raw_h3_listen_cb(port, cert ? cert : "", key ? key : "", h3_srv_pipe_cb, NULL);
}

// h3_server_listen(port:int[, cert:str, key:str]) -> int —— M53-S3：
// 以公共 HTTP 管道托管 H3 server：每连接自动 accept + QPACK 会话 + 请求走
// vhost/路由/限流/静态/.px 管道（与 HTTP/1.1 px_serve 同一逻辑）。→ listener id | -1
static LXValue bi_h3_server_listen(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) px_error("h3_server_listen 需要 (port: int[, cert: str, key: str])");
    int port = (int)args[0].as.i;
    const char* cert = "";
    const char* key = "";
    if (nargs >= 3 && args[1].type == PX_STR && args[2].type == PX_STR) {
        cert = args[1].as.obj->as.str.data;
        key = args[2].as.obj->as.str.data;
    }
    int64_t id = px_quic_raw_h3_listen_cb(port, cert, key, h3_srv_pipe_cb, NULL);
    return px_int(id);
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
    px_set_global("h3_server_listen", px_native("h3_server_listen", bi_h3_server_listen));
    px_ffi_register("h3_server_listen", bi_h3_server_listen);
}
