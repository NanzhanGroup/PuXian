// 普贤 (PuXian) C 运行时 — QPACK 动态表 + SETTINGS（M49）
// ------------------------------------------------------------
// 在 M48 无动态表 QPACK codec 之上，实现 RFC 9204 连接级 QPACK 会话：
//   - 动态表（容量上限 SETTINGS_QPACK_MAX_TABLE_CAPACITY、当前容量、插入/驱逐/重复、
//     绝对/相对/Post-Base 索引）
//   - 编码器流指令：Set Dynamic Table Capacity / Insert with Name Reference /
//     Insert with Literal Name / Duplicate
//   - 解码器流指令：Section Acknowledgment / Stream Cancellation /
//     Insert Count Increment（字节级编解码辅助，供上层/测试使用）
//   - 字段段：RIC（wrap 编码）+ Base（S+Delta）+ 静态/动态/Post-Base 字段行
//   - HTTP/3 SETTINGS 帧（RFC 9114 §7.2.8，QPACK 参数 0x01=MAX_TABLE_CAPACITY）
// 语言层 API（extern def，双模式一致，注册进 FFI 表 + 全局）：
//   h3_qs_open(max_capacity:int) -> int       # 打开 QPACK 会话 → 句柄 | -1
//   h3_qs_close(sess:int) -> bool
//   h3_qs_enc(sess, headers:list) -> bytes    # 编码字段段（自动插入动态表，返回字段段）
//   h3_qs_take_enc(sess) -> bytes             # 取走编码器流待发指令（取后清空）
//   h3_qs_dec_ingest(sess, enc_stream:bytes)  # 解码端处理编码器流指令
//   h3_qs_dec(sess, section:bytes) -> list|null
//   h3_settings_enc(pairs:list) -> bytes      # 构造 SETTINGS 帧
//   h3_settings_dec(frame:bytes) -> list|null
// 说明：自包含实现（不依赖 M48 runtime_h3_qpack.c 的内部 static），
//   Huffman/前缀整数/静态表原语在此重实现（含 tbl.h 数据）。
#define _GNU_SOURCE
#include "runtime.h"
#include "runtime_h3_qpack_tbl.h"
#include "runtime_h3_qpack_dyn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QD_MAX_SESS 64
#define QD_ENT_MAX 4096        // 动态表最大条目槽位（环形，绝对索引单调）
#define QD_CAP_MAX 262144      // 动态表最大容量字节上限（SETTINGS 可设 0..此值）
#define QD_BUF_MAX (1 << 20)   // 单字段段/指令缓冲上限 1MB

// ==================== 小原语 ====================
static int qd_pref_enc(uint8_t* out, int prefix_bits, uint64_t value) {
    uint64_t maxv = (1ULL << prefix_bits) - 1;
    if (value < maxv) { out[0] = (uint8_t)value; return 1; }
    out[0] = (uint8_t)maxv;
    uint64_t rest = value - maxv;
    int n = 1;
    while (rest >= 128) { out[n++] = (uint8_t)((rest & 0x7f) | 0x80); rest >>= 7; }
    out[n++] = (uint8_t)rest;
    return n;
}
static uint64_t qd_pref_dec(const uint8_t* p, int maxlen, int prefix_bits, int* used) {
    if (maxlen < 1) { *used = 0; return 0; }
    uint64_t maxv = (1ULL << prefix_bits) - 1;
    uint64_t v = p[0] & maxv;
    if (v < maxv) { *used = 1; return v; }
    int i = 1, shift = 0;
    while (i < maxlen) {
        uint8_t b = p[i++];
        v += (uint64_t)(b & 0x7f) << shift;
        if ((b & 0x80) == 0) { *used = i; return v; }
        shift += 7;
    }
    *used = 0; return 0;
}

// ---------- Huffman（RFC 7541 App B / RFC 9204 §4.1.2）----------
static int qd_huff_enc(uint8_t* out, int cap, const char* s, int slen) {
    uint64_t acc = 0; int nbits = 0, o = 0;
    for (int i = 0; i < slen; i++) {
        uint8_t c = (uint8_t)s[i];
        acc = (acc << qp_huff_len[c]) | qp_huff_code[c];
        nbits += qp_huff_len[c];
        while (nbits >= 8) { if (o >= cap) return -1; out[o++] = (uint8_t)(acc >> (nbits - 8)); nbits -= 8; }
        acc &= (1ULL << nbits) - 1;
    }
    if (nbits > 0) {
        if (o >= cap) return -1;
        uint8_t pad = (uint8_t)(acc << (8 - nbits)) | (uint8_t)((1 << (8 - nbits)) - 1);
        out[o++] = pad;
    }
    return o;
}
static int qd_huff_enc_len(const char* s, int slen) {
    uint64_t bits = 0;
    for (int i = 0; i < slen; i++) bits += qp_huff_len[(uint8_t)s[i]];
    return (int)((bits + 7) >> 3);
}
static uint64_t qd_bits_peek(const uint8_t* p, int bitpos, int n) {
    uint64_t v = 0;
    for (int i = 0; i < n; i++) {
        int bp = bitpos + i;
        v = (v << 1) | (uint64_t)((p[bp >> 3] >> (7 - (bp & 7))) & 1);
    }
    return v;
}
static int qd_huff_dec(const uint8_t* in, int ilen, uint8_t* out, int cap) {
    int bitpos = 0, olen = 0, total = ilen * 8;
    while (total - bitpos >= 5) {
        int avail = total - bitpos;
        uint64_t w = qd_bits_peek(in, bitpos, avail >= 30 ? 30 : avail);
        int found = -1, flen = 0;
        for (int i = 0; i < 256; i++) {
            int L = qp_huff_len[i];
            if (L > avail) continue;
            if ((w >> (avail >= 30 ? (30 - L) : (avail - L))) == qp_huff_code[i]) { found = i; flen = L; break; }
        }
        if (found < 0) {
            if (avail <= 7) {
                uint64_t pad = w & ((1ULL << avail) - 1);
                if (pad == ((1ULL << avail) - 1)) break;
            }
            return -1;
        }
        if (olen >= cap) return -1;
        out[olen++] = (uint8_t)found;
        bitpos += flen;
    }
    int rem = total - bitpos;
    if (rem > 0 && qd_bits_peek(in, bitpos, rem) != ((1ULL << rem) - 1)) return -1;
    return olen;
}

// ---------- 静态表 ----------
static int qd_static_full(const char* n, int nl, const char* v, int vl) {
    for (int i = 0; i < 99; i++)
        if ((int)strlen(qp_static_table[i].name) == nl && memcmp(qp_static_table[i].name, n, (size_t)nl) == 0 &&
            (int)strlen(qp_static_table[i].value) == vl && memcmp(qp_static_table[i].value, v, (size_t)vl) == 0) return i;
    return -1;
}
static int qd_static_name(const char* n, int nl) {
    for (int i = 0; i < 99; i++)
        if ((int)strlen(qp_static_table[i].name) == nl && memcmp(qp_static_table[i].name, n, (size_t)nl) == 0) return i;
    return -1;
}

// ==================== 会话 + 动态表 ====================
typedef struct { char* name; int nlen; char* val; int vlen; } qd_entry;

typedef struct {
    int used;
    int64_t max_cap;          // 解码端允许的最大容量（SETTINGS）
    int64_t enc_cap;          // 当前动态表容量（encoder 本地 + decoder 镜像用同值）
    // encoder 侧动态表（环形数组，绝对索引单调）
    qd_entry en[QD_ENT_MAX]; int en_head, en_len;  // 有效槽 [head, head+len) mod ENT_MAX
    uint64_t en_next_abs;     // 下一条目绝对索引
    uint8_t* eout; int eout_len, eout_cap;         // 编码器流待发指令缓冲
    // decoder 侧动态表（对端镜像，用于本端解码请求头）
    qd_entry de[QD_ENT_MAX]; int de_head, de_len;
    uint64_t de_next_abs;
    uint64_t de_ins;          // decoder 已处理插入计数（RIC wrap 还原用）
    int dec_blocked;          // M51：最近一次 qd_dec_section 是否因缺 encoder 指令而阻塞（de_ins<RIC）
    uint64_t en_krc;          // M52：本端编码器 Known Received Count（对端解码器流 ack 推进，RFC 9204 §2.1.4）
    uint64_t en_last_ric;     // M52：最近一次编码字段段的 Required Insert Count（上层登记 outstanding 用）
    uint64_t dec_ric;         // M52：最近一次成功解码字段段的 Required Insert Count（>0 需发 Section Ack）
} qd_sess;

static qd_sess g_qds[QD_MAX_SESS];
static qd_sess* qd_get(int64_t id) { return (id > 0 && id <= QD_MAX_SESS && g_qds[id - 1].used) ? &g_qds[id - 1] : NULL; }

static int qd_entry_size(const qd_entry* e) { return e->nlen + e->vlen + 32; }
static int qd_entry_size2(const char* n, int nl, const char* v, int vl) { return nl + vl + 32; }

// 环形数组：按绝对索引单调存放，驱逐最老（绝对索引最小）。
static void qd_tab_append(qd_entry* t, int* head, int* len, const char* n, int nl, const char* v, int vl) {
    int pos = (*head + *len) % QD_ENT_MAX;
    char* nn = (char*)malloc((size_t)nl + 1);
    char* vv = (char*)malloc((size_t)vl + 1);
    if (!nn || !vv) { free(nn); free(vv); return; }
    if (nl > 0) memcpy(nn, n, (size_t)nl);
    nn[nl] = 0;
    if (vl > 0) memcpy(vv, v, (size_t)vl);
    vv[vl] = 0;
    t[pos].name = nn; t[pos].nlen = nl; t[pos].val = vv; t[pos].vlen = vl;
    (*len)++;
}
static void qd_tab_drop(qd_entry* t, int* head, int* len) {
    if (*len <= 0) return;
    free(t[*head].name); free(t[*head].val);
    *head = (*head + 1) % QD_ENT_MAX;
    (*len)--;
}
static void qd_tab_free(qd_entry* t, int* head, int* len) {
    while (*len > 0) { free(t[*head].name); free(t[*head].val); *head = (*head + 1) % QD_ENT_MAX; (*len)--; }
}

// 动态表当前占用字节
static int64_t qd_tab_bytes(qd_entry* t, int head, int len) {
    int64_t b = 0;
    for (int i = 0; i < len; i++) b += qd_entry_size(&t[(head + i) % QD_ENT_MAX]);
    return b;
}

// ==================== 缓冲辅助 ====================
static int qd_buf_append(uint8_t** buf, int* len, int* cap, const uint8_t* d, int n) {
    if (n <= 0) return 0;
    if (*len + n > QD_BUF_MAX) return -1;
    if (*len + n > *cap) {
        int nc = *cap > 0 ? *cap : 512;
        while (nc < *len + n) nc *= 2;
        if (nc > QD_BUF_MAX) nc = QD_BUF_MAX;
        uint8_t* nd = (uint8_t*)realloc(*buf, (size_t)nc);
        if (!nd) return -1;
        *buf = nd; *cap = nc;
    }
    memcpy(*buf + *len, d, (size_t)n); *len += n;
    return 0;
}

// 8-bit prefix string literal（Huffman 若更短）→ buf
static int qd_str_out(uint8_t** buf, int* len, int* cap, const char* s, int sl) {
    int hl = qd_huff_enc_len(s, sl);
    int use_h = hl > 0 && hl < sl;
    int dl = use_h ? hl : sl;
    uint8_t tmp[16];
    int n = qd_pref_enc(tmp, 7, (uint64_t)dl);
    uint8_t h[16]; int ho = 0;
    h[ho++] = (uint8_t)((use_h ? 0x80 : 0) | tmp[0]);
    for (int i = 1; i < n; i++) h[ho++] = tmp[i];
    if (qd_buf_append(buf, len, cap, h, ho) != 0) return -1;
    if (use_h) {
        uint8_t* t = (uint8_t*)malloc((size_t)hl + 8);
        if (!t) return -1;
        qd_huff_enc(t, hl + 8, s, sl);
        int r = qd_buf_append(buf, len, cap, t, hl);
        free(t);
        return r == 0 ? 0 : -1;
    }
    return qd_buf_append(buf, len, cap, (const uint8_t*)s, sl);
}

// ==================== 语言绑定 helper ====================
// header list -> 临时字段数组（name/val 指针）。返回字段数或 -1。
typedef struct { const char* n; int nl; const char* v; int vl; } qd_field;
static int qd_collect_fields(LXValue headers, qd_field* f, int maxf) {
    if (headers.type != PX_LIST && headers.type != PX_TUPLE) return -1;
    LXObject* hd = headers.as.obj;
    int n = hd->as.list.len;
    if (n > maxf) n = maxf;
    for (int i = 0; i < n; i++) {
        LXValue it = hd->as.list.items[i];
        if (it.type != PX_LIST && it.type != PX_TUPLE) return -1;
        LXObject* p = it.as.obj;
        if (p->as.list.len < 2) return -1;
        LXValue* kv = p->as.list.items;
        if (kv[0].type != PX_STR || kv[1].type != PX_STR) return -1;
        f[i].n = kv[0].as.obj->as.str.data; f[i].nl = kv[0].as.obj->as.str.len;
        f[i].v = kv[1].as.obj->as.str.data; f[i].vl = kv[1].as.obj->as.str.len;
    }
    return n;
}

// ==================== h3_qs_open / close ====================
static LXValue bi_qs_open(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) px_error("h3_qs_open 需要 (max_capacity: int)");
    int64_t cap = args[0].as.i;
    if (cap < 0) cap = 0;
    if (cap > QD_CAP_MAX) cap = QD_CAP_MAX;
    for (int i = 0; i < QD_MAX_SESS; i++) {
        if (!g_qds[i].used) {
            qd_sess* s = &g_qds[i];
            memset(s, 0, sizeof(*s));
            s->used = 1;
            s->max_cap = cap;
            s->enc_cap = cap;      // 会话创建即允许该容量（SETTINGS 已收）
            return px_int(i + 1);
        }
    }
    return px_int(-1);
}
static LXValue bi_qs_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_bool(false);
    qd_sess* s = qd_get(args[0].as.i);
    if (!s) return px_bool(false);
    qd_tab_free(s->en, &s->en_head, &s->en_len);
    qd_tab_free(s->de, &s->de_head, &s->de_len);
    free(s->eout);
    memset(s, 0, sizeof(*s));
    return px_bool(true);
}

// ==================== QPACK 编码（RFC 9204 §4.3 + §4.5）====================
// 编码端策略（确定性、RFC 正确的最小实现）：
//   1) 对每个字段：若静态表有全匹配 → Indexed(static)；否则若动态表有全匹配 → Indexed(dyn)；
//      否则 Literal（名可引用静态/动态表名；值照发）。额外：把"值不可压缩但名命中"
//      的字段尝试插入动态表（Insert with Name Ref，静态或动态名），
//      让后续相同值可命中动态表全匹配。
//   2) 本 MVP 不做容量逐出决策（保持简单），每次编码前按序处理插入；命中动态表条件：
//      动态表中已存在该 name+value。
//   Base 恒取当前动态表大小（next_abs = 已插条数），所以所有动态引用都 < Base，用相对索引。
//   字段段前缀：RIC = max(被引用动态 abs)+1（若有动态引用，否则 0）；
//   Base = RIC（S=0, Delta=0 单字节）。
//   注意：为了 decoder 能还原，编码时必须保证 decoder 已先收到对应 encoder 指令
//   （h3_qs_take_enc 取出的字节须先喂给对端 h3_qs_dec_ingest，再喂字段段）。

// 动态表查找（含最新优先）返回绝对索引（若在表中）
static int qd_dyn_find_full(qd_entry* t, int head, int len, uint64_t next_abs,
                            const char* n, int nl, const char* v, int vl) {
    for (int i = len - 1; i >= 0; i--) {
        qd_entry* e = &t[(head + i) % QD_ENT_MAX];
        if (e->nlen == nl && e->vlen == vl && memcmp(e->name, n, (size_t)nl) == 0 &&
            memcmp(e->val, v, (size_t)vl) == 0)
            return (int)(next_abs - (uint64_t)len + (uint64_t)i);
    }
    return -1;
}
static int qd_dyn_find_name(qd_entry* t, int head, int len, uint64_t next_abs,
                            const char* n, int nl) {
    for (int i = len - 1; i >= 0; i--) {
        qd_entry* e = &t[(head + i) % QD_ENT_MAX];
        if (e->nlen == nl && memcmp(e->name, n, (size_t)nl) == 0)
            return (int)(next_abs - (uint64_t)len + (uint64_t)i);
    }
    return -1;
}

// 在 dyn 表按绝对索引取条目（0=最新-1...），不存在返回 NULL
static qd_entry* qd_dyn_at_abs(qd_entry* t, int head, int len, uint64_t next_abs, uint64_t abs) {
    if (abs >= next_abs || next_abs - abs > (uint64_t)len) return NULL;
    int off = (int)(next_abs - abs - 1);      // 0 = 最新
    int pos = (head + (len - 1 - off)) % QD_ENT_MAX;
    return &t[pos];
}

// h3_qs_enc(sess, headers) -> bytes
static LXValue bi_qs_enc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT) px_error("h3_qs_enc 需要 (sess, headers: list)");
    qd_sess* s = qd_get(args[0].as.i);
    if (!s) return px_null();
    qd_field f[512];
    int nf = qd_collect_fields(args[1], f, 512);
    if (nf < 0) return px_null();

    // 输出缓冲
    uint8_t* out = NULL; int olen = 0, ocap = 0;

    // 编码策略：对每个字段，若静态表有全匹配 → 不插入（Indexed 静态即可）；
    // 否则若动态表已含全匹配 → 不插入；否则尝试插入动态表（生成编码器流 Insert 指令 +
    // 本地表更新），让后续字段段可复用。字段段本身优先 Indexed（静态→动态），
    // 其次 Literal Name Reference（静态名→动态名），最次 Literal Name。
    for (int i = 0; i < nf; i++) {
        // 已存在完整动态条目则不插
        if (qd_dyn_find_full(s->en, s->en_head, s->en_len, s->en_next_abs, f[i].n, f[i].nl, f[i].v, f[i].vl) >= 0) continue;
        int esz = qd_entry_size2(f[i].n, f[i].nl, f[i].v, f[i].vl);
        if (esz > s->enc_cap) continue;
        if (s->en_len >= QD_ENT_MAX) continue;
        // 容量驱逐（M52：仅驱逐对端已 ack 的条目 abs < en_krc；未确认引用的条目不可驱逐，
        // RFC 9204 §2.1.1/§2.2.2——驱逐必须先于 Insert 指令发出，保证 decoder 永不引用已驱逐条目）
        while (qd_tab_bytes(s->en, s->en_head, s->en_len) + esz > s->enc_cap && s->en_len > 0 &&
               s->en_next_abs - (uint64_t)s->en_len < s->en_krc)
            qd_tab_drop(s->en, &s->en_head, &s->en_len);
        if (qd_tab_bytes(s->en, s->en_head, s->en_len) + esz > s->enc_cap) continue;
        // 生成 Insert 指令（编码器流）
        int sidx = qd_static_name(f[i].n, f[i].nl);
        if (sidx >= 0) {
            // Insert with Name Reference, T=1
            uint8_t tmp[16]; int n = qd_pref_enc(tmp, 6, (uint64_t)sidx);
            uint8_t h[16]; int ho = 0;
            h[ho++] = (uint8_t)(0xC0 | (tmp[0] & 0x3f));
            for (int j = 1; j < n; j++) h[ho++] = tmp[j];
            if (qd_buf_append(&s->eout, &s->eout_len, &s->eout_cap, h, ho) != 0) continue;
            qd_str_out(&s->eout, &s->eout_len, &s->eout_cap, f[i].v, f[i].vl);
        } else {
            int didx = qd_dyn_find_name(s->en, s->en_head, s->en_len, s->en_next_abs, f[i].n, f[i].nl);
            if (didx >= 0) {
                // Insert with Name Reference, T=0（相对索引 = next_abs-1-abs）
                uint64_t rel = s->en_next_abs - 1 - (uint64_t)didx;
                uint8_t tmp[16]; int n = qd_pref_enc(tmp, 6, rel);
                uint8_t h[16]; int ho = 0;
                h[ho++] = (uint8_t)(0x80 | (tmp[0] & 0x3f));
                for (int j = 1; j < n; j++) h[ho++] = tmp[j];
                if (qd_buf_append(&s->eout, &s->eout_len, &s->eout_cap, h, ho) != 0) continue;
                qd_str_out(&s->eout, &s->eout_len, &s->eout_cap, f[i].v, f[i].vl);
            } else {
                // Insert with Literal Name：01 + name(6bit prefix str) + value
                // 6-bit prefix string literal：H 在 bit5，长度 5-bit
                int nhl = qd_huff_enc_len(f[i].n, f[i].nl);
                int use_h = nhl > 0 && nhl < f[i].nl;
                int dl = use_h ? nhl : f[i].nl;
                uint8_t tmp[16]; int n = qd_pref_enc(tmp, 5, (uint64_t)dl);
                uint8_t h[32]; int ho = 0;
                h[ho++] = (uint8_t)(0x40 | (use_h ? 0x20 : 0) | (tmp[0] & 0x1f));
                for (int j = 1; j < n; j++) h[ho++] = tmp[j];
                if (qd_buf_append(&s->eout, &s->eout_len, &s->eout_cap, h, ho) != 0) continue;
                if (use_h) {
                    uint8_t* t = (uint8_t*)malloc((size_t)nhl + 8);
                    if (!t) continue;
                    qd_huff_enc(t, nhl + 8, f[i].n, f[i].nl);
                    qd_buf_append(&s->eout, &s->eout_len, &s->eout_cap, t, nhl);
                    free(t);
                } else {
                    qd_buf_append(&s->eout, &s->eout_len, &s->eout_cap, (const uint8_t*)f[i].n, f[i].nl);
                }
                qd_str_out(&s->eout, &s->eout_len, &s->eout_cap, f[i].v, f[i].vl);
            }
        }
        // 本地动态表插入（驱逐同上：仅已确认 abs < en_krc 可驱逐；插不下则放弃本条插入——
        // 此时 eout 未写入该条指令，字段段已按字面量编码，两端状态一致）
        if (s->en_len >= QD_ENT_MAX) continue;
        int esz2 = qd_entry_size2(f[i].n, f[i].nl, f[i].v, f[i].vl);
        while (qd_tab_bytes(s->en, s->en_head, s->en_len) + esz2 > s->enc_cap && s->en_len > 0 &&
               s->en_next_abs - (uint64_t)s->en_len < s->en_krc)
            qd_tab_drop(s->en, &s->en_head, &s->en_len);
        if (qd_tab_bytes(s->en, s->en_head, s->en_len) + esz2 > s->enc_cap) continue;
        qd_tab_append(s->en, &s->en_head, &s->en_len, f[i].n, f[i].nl, f[i].v, f[i].vl);
        s->en_next_abs++;
    }

    // ---- 编码字段段 ----
    // 先决定每行将引用的动态绝对索引，求 RIC = max(被引用动态 abs)+1
    // （与下方字段行编码决策严格一致：static-full / dyn-full / static-name / dyn-name / literal）
    int ric = 0;
    for (int i = 0; i < nf; i++) {
        if (qd_static_full(f[i].n, f[i].nl, f[i].v, f[i].vl) >= 0) continue;
        int da = qd_dyn_find_full(s->en, s->en_head, s->en_len, s->en_next_abs, f[i].n, f[i].nl, f[i].v, f[i].vl);
        if (da >= 0) { if (da + 1 > ric) ric = da + 1; continue; }
        if (qd_static_name(f[i].n, f[i].nl) >= 0) continue;   // name-ref T=1 不引用动态
        int didx = qd_dyn_find_name(s->en, s->en_head, s->en_len, s->en_next_abs, f[i].n, f[i].nl);
        if (didx >= 0) { if (didx + 1 > ric) ric = didx + 1; }
    }
    // M52：记录本字段段 RIC（上层发 Section Ack 时按流登记 outstanding）
    s->en_last_ric = (uint64_t)ric;
    // RIC 编码：EncInsertCount = (RIC==0?0:(RIC mod (2*MaxEntries))+1)，MaxEntries=floor(max_cap/32)
    uint64_t enc_ric = 0;
    if (ric > 0) {
        int64_t me = s->max_cap / 32;
        if (me < 1) me = 1;
        enc_ric = ((uint64_t)ric % (uint64_t)(2 * me)) + 1;
    }
    uint8_t tmp[16]; int n;
    n = qd_pref_enc(tmp, 8, enc_ric);
    if (qd_buf_append(&out, &olen, &ocap, tmp, n) != 0) { free(out); return px_null(); }
    // Base 编码：取 Base = RIC（S=0, DeltaBase=0 单字节）。被引用条目 abs < RIC = Base，
    // 均可用相对索引（rel = Base-1-abs ≥ 0）；无动态引用时 RIC=0/Base=0（同 M48 前缀）。
    uint64_t base = (uint64_t)ric;
    uint8_t b0 = 0x00;
    if (qd_buf_append(&out, &olen, &ocap, &b0, 1) != 0) { free(out); return px_null(); }
    // 字段行
    for (int i = 0; i < nf; i++) {
        const char* nn = f[i].n; int nl = f[i].nl; const char* vv = f[i].v; int vl = f[i].vl;
        int full = qd_static_full(nn, nl, vv, vl);
        if (full >= 0) {
            // Indexed Field Line T=1（static）
            uint8_t t[16]; int tn = qd_pref_enc(t, 6, (uint64_t)full);
            uint8_t b = (uint8_t)(0xC0 | (t[0] & 0x3f));
            if (qd_buf_append(&out, &olen, &ocap, &b, 1) != 0) { free(out); return px_null(); }
            for (int j = 1; j < tn; j++) if (qd_buf_append(&out, &olen, &ocap, &t[j], 1) != 0) { free(out); return px_null(); }
            continue;
        }
        int da = qd_dyn_find_full(s->en, s->en_head, s->en_len, s->en_next_abs, nn, nl, vv, vl);
        if (da >= 0) {
            // Indexed Field Line T=0（dynamic relative to Base）：rel = base-1-abs
            uint64_t rel = base - 1 - (uint64_t)da;
            uint8_t t[16]; int tn = qd_pref_enc(t, 6, rel);
            uint8_t b = (uint8_t)(0x80 | (t[0] & 0x3f));
            if (qd_buf_append(&out, &olen, &ocap, &b, 1) != 0) { free(out); return px_null(); }
            for (int j = 1; j < tn; j++) if (qd_buf_append(&out, &olen, &ocap, &t[j], 1) != 0) { free(out); return px_null(); }
            continue;
        }
        // Literal：name 引用静态/动态名（有则省名），否则字面名
        int sidx = qd_static_name(nn, nl);
        if (sidx >= 0) {
            // Literal Field Line with Name Reference：01 N=0 T=1 NameIdx(4+) + value
            uint8_t t[16]; int tn = qd_pref_enc(t, 4, (uint64_t)sidx);
            uint8_t b = (uint8_t)(0x50 | (t[0] & 0x0f));
            if (qd_buf_append(&out, &olen, &ocap, &b, 1) != 0) { free(out); return px_null(); }
            for (int j = 1; j < tn; j++) if (qd_buf_append(&out, &olen, &ocap, &t[j], 1) != 0) { free(out); return px_null(); }
            if (qd_str_out(&out, &olen, &ocap, vv, vl) != 0) { free(out); return px_null(); }
            continue;
        }
        int didx = qd_dyn_find_name(s->en, s->en_head, s->en_len, s->en_next_abs, nn, nl);
        if (didx >= 0) {
            // Literal Field Line with Name Reference：01 N=0 T=0 NameIdx(4+，相对 Base)
            uint64_t rel = base - 1 - (uint64_t)didx;
            uint8_t t[16]; int tn = qd_pref_enc(t, 4, rel);
            uint8_t b = (uint8_t)(0x40 | (t[0] & 0x0f));
            if (qd_buf_append(&out, &olen, &ocap, &b, 1) != 0) { free(out); return px_null(); }
            for (int j = 1; j < tn; j++) if (qd_buf_append(&out, &olen, &ocap, &t[j], 1) != 0) { free(out); return px_null(); }
            if (qd_str_out(&out, &olen, &ocap, vv, vl) != 0) { free(out); return px_null(); }
            continue;
        }
        // Literal Field Line with Literal Name：001 N=0 + name(3+ prefix str) + value
        {
            int nhl = qd_huff_enc_len(nn, nl);
            int use_h = nhl > 0 && nhl < nl;
            int dl = use_h ? nhl : nl;
            uint8_t t[16]; int tn = qd_pref_enc(t, 3, (uint64_t)dl);
            uint8_t b = (uint8_t)(0x20 | (use_h ? 0x08 : 0) | (t[0] & 0x07));
            if (qd_buf_append(&out, &olen, &ocap, &b, 1) != 0) { free(out); return px_null(); }
            for (int j = 1; j < tn; j++) if (qd_buf_append(&out, &olen, &ocap, &t[j], 1) != 0) { free(out); return px_null(); }
            if (use_h) {
                uint8_t* tt = (uint8_t*)malloc((size_t)nhl + 8);
                if (!tt) { free(out); return px_null(); }
                qd_huff_enc(tt, nhl + 8, nn, nl);
                qd_buf_append(&out, &olen, &ocap, tt, nhl);
                free(tt);
            } else {
                if (qd_buf_append(&out, &olen, &ocap, (const uint8_t*)nn, nl) != 0) { free(out); return px_null(); }
            }
            if (qd_str_out(&out, &olen, &ocap, vv, vl) != 0) { free(out); return px_null(); }
        }
    }
    LXValue r = px_bytes_len(out, olen);
    free(out);
    return r;
}

// h3_qs_take_enc(sess) -> bytes（取走并清空）
static LXValue bi_qs_take_enc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_null();
    qd_sess* s = qd_get(args[0].as.i);
    if (!s) return px_null();
    if (s->eout_len == 0) return px_bytes_len("", 0);
    LXValue r = px_bytes_len(s->eout, s->eout_len);
    s->eout_len = 0;
    return r;
}

// ==================== QPACK 解码 ====================
// 处理编码器流指令（更新 decoder 镜像表）
static int qd_dec_ingest(qd_sess* s, const uint8_t* p, int len) {
    int off = 0;
    while (off < len) {
        uint8_t b0 = p[off];
        int used = 0;
        if ((b0 & 0xE0) == 0x20) {
            // Set Dynamic Table Capacity
            uint64_t cap = qd_pref_dec(p + off, len - off, 5, &used);
            if (!used) return -1;
            off += used;
            if (cap > (uint64_t)s->max_cap) return -1;
            s->enc_cap = (int64_t)cap;
            while (qd_tab_bytes(s->de, s->de_head, s->de_len) > s->enc_cap && s->de_len > 0)
                qd_tab_drop(s->de, &s->de_head, &s->de_len);
        } else if ((b0 & 0x80) == 0x80) {
            // Insert with Name Reference：1 T NameIdx(6+) + value
            int T = (b0 >> 6) & 1;
            uint64_t idx = qd_pref_dec(p + off, len - off, 6, &used);
            if (!used) return -1;
            off += used;
            const char* nm = NULL; int nl = 0;
            if (T) {
                if (idx >= 99) return -1;
                nm = qp_static_table[idx].name; nl = (int)strlen(nm);
            } else {
                // 相对索引（0=最新）：abs = de_next_abs-1-idx
                if (idx >= s->de_next_abs || s->de_next_abs - idx > (uint64_t)s->de_len) return -1;
                qd_entry* e = qd_dyn_at_abs(s->de, s->de_head, s->de_len, s->de_next_abs, s->de_next_abs - 1 - idx);
                if (!e) return -1;
                nm = e->name; nl = e->nlen;
            }
            if (off >= len) return -1;
            int H = (p[off] >> 7) & 1;
            uint64_t vlen = qd_pref_dec(p + off, len - off, 7, &used);
            if (!used) return -1;
            off += used;
            if (off + (int)vlen > len) return -1;
            char* vv = NULL; int vl = 0;
            if (H) {
                uint8_t* t = (uint8_t*)malloc((size_t)(vlen * 2 + 16));
                if (!t) return -1;
                int r = qd_huff_dec(p + off, (int)vlen, t, (int)(vlen * 2 + 16));
                if (r < 0) { free(t); return -1; }
                vv = (char*)malloc((size_t)r + 1);
                if (!vv) { free(t); return -1; }
                memcpy(vv, t, (size_t)r); vv[r] = 0; vl = r;
                free(t);
            } else {
                vv = (char*)malloc((size_t)vlen + 1);
                if (!vv) return -1;
                memcpy(vv, p + off, (size_t)vlen); vv[vlen] = 0; vl = (int)vlen;
            }
            off += (int)vlen;
            int esz = qd_entry_size2(nm, nl, vv, vl);
            if (esz > s->enc_cap) { free(vv); return -1; }
            while (qd_tab_bytes(s->de, s->de_head, s->de_len) + esz > s->enc_cap && s->de_len > 0)
                qd_tab_drop(s->de, &s->de_head, &s->de_len);
            if (qd_tab_bytes(s->de, s->de_head, s->de_len) + esz > s->enc_cap) { free(vv); return -1; }
            qd_tab_append(s->de, &s->de_head, &s->de_len, nm, nl, vv, vl);
            free(vv);
            s->de_next_abs++;
            s->de_ins++;
        } else if ((b0 & 0xC0) == 0x40) {
            // Insert with Literal Name：01 + name(6 prefix str) + value(8 prefix str)
            int Hn = (b0 >> 5) & 1;
            uint64_t nlen = qd_pref_dec(p + off, len - off, 5, &used);
            if (!used) return -1;
            off += used;
            if (off + (int)nlen > len) return -1;
            char* nm = NULL; int nl = 0;
            if (Hn) {
                uint8_t* t = (uint8_t*)malloc((size_t)(nlen * 2 + 16));
                if (!t) return -1;
                int r = qd_huff_dec(p + off, (int)nlen, t, (int)(nlen * 2 + 16));
                if (r < 0) { free(t); return -1; }
                nm = (char*)malloc((size_t)r + 1);
                if (!nm) { free(t); return -1; }
                memcpy(nm, t, (size_t)r); nm[r] = 0; nl = r;
                free(t);
            } else {
                nm = (char*)malloc((size_t)nlen + 1);
                if (!nm) return -1;
                memcpy(nm, p + off, (size_t)nlen); nm[nlen] = 0; nl = (int)nlen;
            }
            off += (int)nlen;
            if (off >= len) { free(nm); return -1; }
            int Hv = (p[off] >> 7) & 1;
            uint64_t vlen = qd_pref_dec(p + off, len - off, 7, &used);
            if (!used) { free(nm); return -1; }
            off += used;
            if (off + (int)vlen > len) { free(nm); return -1; }
            char* vv = NULL; int vl = 0;
            if (Hv) {
                uint8_t* t = (uint8_t*)malloc((size_t)(vlen * 2 + 16));
                if (!t) { free(nm); return -1; }
                int r = qd_huff_dec(p + off, (int)vlen, t, (int)(vlen * 2 + 16));
                if (r < 0) { free(t); free(nm); return -1; }
                vv = (char*)malloc((size_t)r + 1);
                if (!vv) { free(t); free(nm); return -1; }
                memcpy(vv, t, (size_t)r); vv[r] = 0; vl = r;
                free(t);
            } else {
                vv = (char*)malloc((size_t)vlen + 1);
                if (!vv) { free(nm); return -1; }
                memcpy(vv, p + off, (size_t)vlen); vv[vlen] = 0; vl = (int)vlen;
            }
            off += (int)vlen;
            int esz = qd_entry_size2(nm, nl, vv, vl);
            if (esz > s->enc_cap) { free(nm); free(vv); return -1; }
            while (qd_tab_bytes(s->de, s->de_head, s->de_len) + esz > s->enc_cap && s->de_len > 0)
                qd_tab_drop(s->de, &s->de_head, &s->de_len);
            if (qd_tab_bytes(s->de, s->de_head, s->de_len) + esz > s->enc_cap) { free(nm); free(vv); return -1; }
            qd_tab_append(s->de, &s->de_head, &s->de_len, nm, nl, vv, vl);
            free(nm); free(vv);
            s->de_next_abs++;
            s->de_ins++;
        } else if ((b0 & 0xE0) == 0x00) {
            // Duplicate：000 + rel(5+)
            uint64_t rel = qd_pref_dec(p + off, len - off, 5, &used);
            if (!used) return -1;
            off += used;
            if (s->de_len <= 0 || rel >= s->de_next_abs || s->de_next_abs - rel > (uint64_t)s->de_len) return -1;
            qd_entry* e = qd_dyn_at_abs(s->de, s->de_head, s->de_len, s->de_next_abs, s->de_next_abs - 1 - rel);
            if (!e) return -1;
            int esz = qd_entry_size(e);
            if (esz > s->enc_cap) return -1;
            while (qd_tab_bytes(s->de, s->de_head, s->de_len) + esz > s->enc_cap && s->de_len > 0)
                qd_tab_drop(s->de, &s->de_head, &s->de_len);
            if (qd_tab_bytes(s->de, s->de_head, s->de_len) + esz > s->enc_cap) return -1;
            qd_tab_append(s->de, &s->de_head, &s->de_len, e->name, e->nlen, e->val, e->vlen);
            s->de_next_abs++;
            s->de_ins++;
        } else {
            return -1;
        }
    }
    return 0;
}

// 从 p[off] 读 8-bit string literal（Huffman），拷贝返回 malloc 串；返回新 off 或 -1
static int qd_str_in(const uint8_t* p, int len, int off, char** out, int* outl) {
    if (off >= len) return -1;
    int H = (p[off] >> 7) & 1;
    int used = 0;
    uint64_t sl = qd_pref_dec(p + off, len - off, 7, &used);
    if (!used) return -1;
    off += used;
    if (off + (int)sl > len) return -1;
    if (H) {
        uint8_t* t = (uint8_t*)malloc((size_t)(sl * 2 + 16));
        if (!t) return -1;
        int r = qd_huff_dec(p + off, (int)sl, t, (int)(sl * 2 + 16));
        if (r < 0) { free(t); return -1; }
        char* s2 = (char*)malloc((size_t)r + 1);
        if (!s2) { free(t); return -1; }
        memcpy(s2, t, (size_t)r); s2[r] = 0; *out = s2; *outl = r;
        free(t);
    } else {
        char* s2 = (char*)malloc((size_t)sl + 1);
        if (!s2) return -1;
        memcpy(s2, p + off, (size_t)sl); s2[sl] = 0; *out = s2; *outl = (int)sl;
    }
    return off + (int)sl;
}

// 解码字段段 → list of [name,value] | null
static LXValue qd_dec_section(qd_sess* s, const uint8_t* p, int len) {
    int off = 0, used = 0;
    s->dec_blocked = 0;
    if (len < 2) return px_null();
    // RIC
    uint64_t encric = qd_pref_dec(p, len, 8, &used);
    if (!used) return px_null();
    off += used;
    if (off >= len) return px_null();
    // Base（S + 7+）
    int sign = (p[off] >> 7) & 1;
    uint64_t deltab = qd_pref_dec(p + off, len - off, 7, &used);
    if (!used) return px_null();
    off += used;
    // 还原 RIC（RFC 9204 §4.5.1.1）
    int64_t ric = 0;
    if (encric != 0) {
        int64_t me = s->max_cap / 32; if (me < 1) me = 1;
        int64_t full = 2 * me;
        if (encric > (uint64_t)full) return px_null();
        int64_t maxval = (int64_t)s->de_ins + me;
        int64_t maxwrap = (maxval / full) * full;
        ric = maxwrap + (int64_t)encric - 1;
        if (ric > maxval) { if (ric <= full) return px_null(); ric -= full; }
        if (ric == 0) return px_null();
        if ((int64_t)s->de_ins < ric) { s->dec_blocked = 1; return px_null(); }  // 阻塞：指令未到
    }
    // M52：记录本字段段 RIC（>0 时上层须发 Section Ack，RFC 9204 §2.2.2.1）
    s->dec_ric = (uint64_t)ric;
    int64_t base;
    if (sign) { base = ric - (int64_t)deltab - 1; if (base < 0) return px_null(); }
    else base = ric + (int64_t)deltab;
    LXValue fields = px_list(8);
    while (off < len) {
        uint8_t b0 = p[off];
        int used2 = 0;
        if (b0 & 0x80) {
            // Indexed Field Line：1 T Index(6+)
            int T = (b0 >> 6) & 1;
            uint64_t idx = qd_pref_dec(p + off, len - off, 6, &used2);
            if (!used2) return px_null();
            off += used2;
            LXValue pair = px_list(2);
            if (T) {
                if (idx >= 99) return px_null();
                px_list_push(pair, px_str(qp_static_table[idx].name));
                px_list_push(pair, px_str(qp_static_table[idx].value));
            } else {
                // 相对 Base：abs = base-1-idx
                if (idx > (uint64_t)base || (int64_t)idx >= base) return px_null();
                int64_t abs = base - 1 - (int64_t)idx;
                qd_entry* e = qd_dyn_at_abs(s->de, s->de_head, s->de_len, s->de_next_abs, (uint64_t)abs);
                if (!e) return px_null();
                px_list_push(pair, px_str_len(e->name, e->nlen));
                px_list_push(pair, px_str_len(e->val, e->vlen));
            }
            px_list_push(fields, pair);
        } else if ((b0 & 0xF0) == 0x10) {
            // Indexed Field Line with Post-Base Index：0001 Index(4+) → abs = base + idx
            uint64_t idx = qd_pref_dec(p + off, len - off, 4, &used2);
            if (!used2) return px_null();
            off += used2;
            int64_t abs = base + (int64_t)idx;
            qd_entry* e = qd_dyn_at_abs(s->de, s->de_head, s->de_len, s->de_next_abs, (uint64_t)abs);
            if (!e) return px_null();
            LXValue pair = px_list(2);
            px_list_push(pair, px_str_len(e->name, e->nlen));
            px_list_push(pair, px_str_len(e->val, e->vlen));
            px_list_push(fields, pair);
        } else if (b0 & 0x40) {
            // Literal Field Line with Name Reference：01 N T NameIdx(4+) + value
            int T = (b0 >> 4) & 1;
            uint64_t idx = qd_pref_dec(p + off, len - off, 4, &used2);
            if (!used2) return px_null();
            off += used2;
            char* nm = NULL; int nl = 0;
            if (T) {
                if (idx >= 99) return px_null();
                nm = (char*)qp_static_table[idx].name; nl = (int)strlen(nm);
            } else {
                if ((int64_t)idx >= base) return px_null();
                int64_t abs = base - 1 - (int64_t)idx;
                qd_entry* e = qd_dyn_at_abs(s->de, s->de_head, s->de_len, s->de_next_abs, (uint64_t)abs);
                if (!e) return px_null();
                nm = e->name; nl = e->nlen;
            }
            char* vv = NULL; int vl = 0;
            int no = qd_str_in(p, len, off, &vv, &vl);
            if (no < 0) return px_null();
            off = no;
            LXValue pair = px_list(2);
            px_list_push(pair, px_str_len(nm, nl));
            px_list_push(pair, px_str_len(vv, vl));
            free(vv);
            px_list_push(fields, pair);
        } else if ((b0 & 0xF8) == 0x00 || (b0 & 0x0F) == 0x00) {
            // Literal Field Line with Post-Base Name Reference：0000 N NameIdx(3+) → abs=base+idx
            // 高 4 位为 0000（0x00-0x0F）
            if ((b0 & 0xF0) != 0x00) return px_null();
            uint64_t idx = qd_pref_dec(p + off, len - off, 3, &used2);
            if (!used2) return px_null();
            off += used2;
            int64_t abs = base + (int64_t)idx;
            qd_entry* e = qd_dyn_at_abs(s->de, s->de_head, s->de_len, s->de_next_abs, (uint64_t)abs);
            if (!e) return px_null();
            char* vv = NULL; int vl = 0;
            int no = qd_str_in(p, len, off, &vv, &vl);
            if (no < 0) return px_null();
            off = no;
            LXValue pair = px_list(2);
            px_list_push(pair, px_str_len(e->name, e->nlen));
            px_list_push(pair, px_str_len(vv, vl));
            free(vv);
            px_list_push(fields, pair);
        } else if (b0 & 0x20) {
            // Literal Field Line with Literal Name：001 N H NameLen(3+) + name + value
            int Hn = (b0 >> 3) & 1;
            uint64_t nlen = qd_pref_dec(p + off, len - off, 3, &used2);
            if (!used2) return px_null();
            off += used2;
            if (off + (int)nlen > len) return px_null();
            char* nm = NULL; int nl = 0;
            if (Hn) {
                uint8_t* t = (uint8_t*)malloc((size_t)(nlen * 2 + 16));
                if (!t) return px_null();
                int r = qd_huff_dec(p + off, (int)nlen, t, (int)(nlen * 2 + 16));
                if (r < 0) { free(t); return px_null(); }
                nm = (char*)malloc((size_t)r + 1);
                if (!nm) { free(t); return px_null(); }
                memcpy(nm, t, (size_t)r); nm[r] = 0; nl = r;
                free(t);
            } else {
                nm = (char*)malloc((size_t)nlen + 1);
                if (!nm) return px_null();
                memcpy(nm, p + off, (size_t)nlen); nm[nlen] = 0; nl = (int)nlen;
            }
            off += (int)nlen;
            char* vv = NULL; int vl = 0;
            int no = qd_str_in(p, len, off, &vv, &vl);
            if (no < 0) { free(nm); return px_null(); }
            off = no;
            LXValue pair = px_list(2);
            px_list_push(pair, px_str_len(nm, nl));
            px_list_push(pair, px_str_len(vv, vl));
            free(nm); free(vv);
            px_list_push(fields, pair);
        } else {
            return px_null();
        }
    }
    return fields;
}

// h3_qs_dec_ingest(sess, bytes) -> bool
static LXValue bi_qs_dec_ingest(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT) return px_bool(false);
    qd_sess* s = qd_get(args[0].as.i);
    if (!s) return px_bool(false);
    if (args[1].type != PX_STR && args[1].type != PX_BYTES) return px_bool(false);
    const uint8_t* p = (const uint8_t*)args[1].as.obj->as.str.data;
    int plen = args[1].as.obj->as.str.len;
    return px_bool(qd_dec_ingest(s, p, plen) == 0);
}

// h3_qs_dec(sess, section) -> list|null
static LXValue bi_qs_dec(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT) return px_null();
    qd_sess* s = qd_get(args[0].as.i);
    if (!s) return px_null();
    if (args[1].type != PX_STR && args[1].type != PX_BYTES) return px_null();
    const uint8_t* p = (const uint8_t*)args[1].as.obj->as.str.data;
    int plen = args[1].as.obj->as.str.len;
    return qd_dec_section(s, p, plen);
}

// ==================== 解码器流指令编解码（测试/上层用）====================
int px_qd_dec_inst_section_ack(uint8_t* out, int cap, int64_t stream_id) {
    uint8_t tmp[16]; int n = qd_pref_enc(tmp, 7, (uint64_t)stream_id);
    if (n > cap) return -1;
    out[0] = (uint8_t)(0x80 | (tmp[0] & 0x7f));
    for (int i = 1; i < n; i++) out[i] = tmp[i];
    return n;
}
int px_qd_dec_inst_stream_cancel(uint8_t* out, int cap, int64_t stream_id) {
    uint8_t tmp[16]; int n = qd_pref_enc(tmp, 6, (uint64_t)stream_id);
    if (n > cap) return -1;
    out[0] = (uint8_t)(0x40 | (tmp[0] & 0x3f));
    for (int i = 1; i < n; i++) out[i] = tmp[i];
    return n;
}
int px_qd_dec_inst_inc(uint8_t* out, int cap, int64_t inc) {
    uint8_t tmp[16]; int n = qd_pref_enc(tmp, 6, (uint64_t)inc);
    if (n > cap) return -1;
    out[0] = (uint8_t)(tmp[0] & 0x3f);
    for (int i = 1; i < n; i++) out[i] = tmp[i];
    return n;
}

// ==================== SETTINGS 帧（RFC 9114 §7.2.8）====================
// QUIC varint（RFC 9000 §16）
static int qd_varint_enc(uint8_t* out, uint64_t v) {
    if (v < (1ULL << 6)) { out[0] = (uint8_t)v; return 1; }
    if (v < (1ULL << 14)) { out[0] = (uint8_t)(0x40 | (v >> 8)); out[1] = (uint8_t)v; return 2; }
    if (v < (1ULL << 30)) {
        out[0] = (uint8_t)(0x80 | (v >> 24)); out[1] = (uint8_t)(v >> 16);
        out[2] = (uint8_t)(v >> 8); out[3] = (uint8_t)v; return 4;
    }
    out[0] = (uint8_t)(0xc0 | (v >> 56)); out[1] = (uint8_t)(v >> 48);
    out[2] = (uint8_t)(v >> 40); out[3] = (uint8_t)(v >> 32);
    out[4] = (uint8_t)(v >> 24); out[5] = (uint8_t)(v >> 16);
    out[6] = (uint8_t)(v >> 8); out[7] = (uint8_t)v; return 8;
}
static int qd_varint_dec(const uint8_t* p, int maxlen, uint64_t* out) {
    if (maxlen < 1) return 0;
    uint8_t f = p[0];
    if ((f & 0xc0) == 0x00) { *out = f; return 1; }
    if ((f & 0xc0) == 0x40) { if (maxlen < 2) return 0; *out = ((uint64_t)(f & 0x3f) << 8) | p[1]; return 2; }
    if ((f & 0xc0) == 0x80) { if (maxlen < 4) return 0; *out = ((uint64_t)(f & 0x3f) << 24) | ((uint64_t)p[1] << 16) | ((uint64_t)p[2] << 8) | p[3]; return 4; }
    if (maxlen < 8) return 0;
    *out = ((uint64_t)(f & 0x3f) << 56) | ((uint64_t)p[1] << 48) | ((uint64_t)p[2] << 40) | ((uint64_t)p[3] << 32) |
           ((uint64_t)p[4] << 24) | ((uint64_t)p[5] << 16) | ((uint64_t)p[6] << 8) | p[7];
    return 8;
}

// h3_settings_enc(pairs:list of [k:int,v:int]) -> bytes（完整 SETTINGS 帧 type=0x04）
static LXValue bi_settings_enc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || (args[0].type != PX_LIST && args[0].type != PX_TUPLE)) return px_null();
    LXObject* lst = args[0].as.obj;
    uint8_t payload[1024]; int plen = 0;
    for (int i = 0; i < lst->as.list.len; i++) {
        LXValue it = lst->as.list.items[i];
        if (it.type != PX_LIST && it.type != PX_TUPLE) return px_null();
        LXObject* pr = it.as.obj;
        if (pr->as.list.len < 2) return px_null();
        LXValue* kv = pr->as.list.items;
        if (kv[0].type != PX_INT || kv[1].type != PX_INT) return px_null();
        uint8_t t[8];
        int n1 = qd_varint_enc(t, (uint64_t)kv[0].as.i);
        if (plen + n1 > 1024) return px_null();
        memcpy(payload + plen, t, (size_t)n1); plen += n1;
        int n2 = qd_varint_enc(t, (uint64_t)kv[1].as.i);
        if (plen + n2 > 1024) return px_null();
        memcpy(payload + plen, t, (size_t)n2); plen += n2;
    }
    uint8_t head[16];
    int hn = qd_varint_enc(head, 0x04);                       // frame type SETTINGS
    int ln = qd_varint_enc(head + hn, (uint64_t)plen);        // frame length
    uint8_t* out = (uint8_t*)malloc((size_t)(hn + ln + plen));
    if (!out) return px_null();
    memcpy(out, head, (size_t)hn);
    memcpy(out + hn, head + hn, (size_t)ln);
    if (plen) memcpy(out + hn + ln, payload, (size_t)plen);
    LXValue r = px_bytes_len(out, hn + ln + plen);
    free(out);
    return r;
}

// h3_settings_dec(frame) -> list of [k,v] | null
static LXValue bi_settings_dec(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || (args[0].type != PX_STR && args[0].type != PX_BYTES)) return px_null();
    const uint8_t* p = (const uint8_t*)args[0].as.obj->as.str.data;
    int plen = args[0].as.obj->as.str.len;
    int off = 0;
    uint64_t type = 0;
    int n = qd_varint_dec(p + off, plen - off, &type);
    if (n <= 0) return px_null();
    off += n;
    if (type != 0x04) return px_null();
    uint64_t flen = 0;
    n = qd_varint_dec(p + off, plen - off, &flen);
    if (n <= 0) return px_null();
    off += n;
    if (off + (int)flen > plen) return px_null();
    int end = off + (int)flen;
    LXValue pairs = px_list(4);
    while (off < end) {
        uint64_t k = 0, v = 0;
        int n1 = qd_varint_dec(p + off, end - off, &k);
        if (n1 <= 0) return px_null();
        off += n1;
        int n2 = qd_varint_dec(p + off, end - off, &v);
        if (n2 <= 0) return px_null();
        off += n2;
        LXValue pair = px_list(2);
        px_list_push(pair, px_int((int64_t)k));
        px_list_push(pair, px_int((int64_t)v));
        px_list_push(pairs, pair);
    }
    return pairs;
}

// ==================== M52 语言层诊断（capability 断言用）====================
// h3_qs_krc(sess) -> int：本端编码器 Known Received Count
static LXValue bi_qs_krc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_int(-1);
    qd_sess* s = qd_get(args[0].as.i);
    return s ? px_int((int64_t)s->en_krc) : px_int(-1);
}
// h3_qs_enc_ric(sess) -> int：最近一次编码字段段 RIC
static LXValue bi_qs_enc_ric(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_int(-1);
    qd_sess* s = qd_get(args[0].as.i);
    return s ? px_int((int64_t)s->en_last_ric) : px_int(-1);
}
// h3_qs_dec_ric(sess) -> int：最近一次成功解码字段段 RIC
static LXValue bi_qs_dec_ric(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_int(-1);
    qd_sess* s = qd_get(args[0].as.i);
    return s ? px_int((int64_t)s->dec_ric) : px_int(-1);
}
// h3_qs_ack_sec(sess, ric) -> bool：处理 Section Ack（推进 KRC，RFC 9204 §4.4.1）
static LXValue bi_qs_ack_sec(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT || args[1].type != PX_INT) return px_bool(false);
    qd_sess* s = qd_get(args[0].as.i);
    if (!s || args[1].as.i < 0) return px_bool(false);
    if ((uint64_t)args[1].as.i > s->en_krc) s->en_krc = (uint64_t)args[1].as.i;
    return px_bool(true);
}
// h3_qs_ack_inc(sess, inc) -> bool：处理 Insert Count Increment（KRC += inc，RFC 9204 §4.4.3）
static LXValue bi_qs_ack_inc(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || args[0].type != PX_INT || args[1].type != PX_INT) return px_bool(false);
    qd_sess* s = qd_get(args[0].as.i);
    if (!s || args[1].as.i < 0) return px_bool(false);
    s->en_krc += (uint64_t)args[1].as.i;
    return px_bool(true);
}

// h3_qs_en_len(sess) -> int：本端编码表条目
static LXValue bi_qs_en_len(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_int(-1);
    qd_sess* s = qd_get(args[0].as.i);
    return s ? px_int(s->en_len) : px_int(-1);
}
// h3_qs_de_len(sess) -> int：对端镜像表条目
static LXValue bi_qs_de_len(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_int(-1);
    qd_sess* s = qd_get(args[0].as.i);
    return s ? px_int(s->de_len) : px_int(-1);
}
// h3_qs_ins(sess) -> int：已 ingest 插入计数
static LXValue bi_qs_ins(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || args[0].type != PX_INT) return px_int(-1);
    qd_sess* s = qd_get(args[0].as.i);
    return s ? px_int((int64_t)s->de_ins) : px_int(-1);
}

// ==================== 注册 ====================
void px_register_h3_qpack_dyn(void) {
    px_set_global("h3_qs_open", px_native("h3_qs_open", bi_qs_open));
    px_set_global("h3_qs_close", px_native("h3_qs_close", bi_qs_close));
    px_set_global("h3_qs_enc", px_native("h3_qs_enc", bi_qs_enc));
    px_set_global("h3_qs_take_enc", px_native("h3_qs_take_enc", bi_qs_take_enc));
    px_set_global("h3_qs_dec_ingest", px_native("h3_qs_dec_ingest", bi_qs_dec_ingest));
    px_set_global("h3_qs_dec", px_native("h3_qs_dec", bi_qs_dec));
    px_set_global("h3_settings_enc", px_native("h3_settings_enc", bi_settings_enc));
    px_set_global("h3_settings_dec", px_native("h3_settings_dec", bi_settings_dec));
    px_set_global("h3_qs_krc", px_native("h3_qs_krc", bi_qs_krc));
    px_set_global("h3_qs_enc_ric", px_native("h3_qs_enc_ric", bi_qs_enc_ric));
    px_set_global("h3_qs_dec_ric", px_native("h3_qs_dec_ric", bi_qs_dec_ric));
    px_set_global("h3_qs_ack_sec", px_native("h3_qs_ack_sec", bi_qs_ack_sec));
    px_set_global("h3_qs_ack_inc", px_native("h3_qs_ack_inc", bi_qs_ack_inc));
    px_set_global("h3_qs_en_len", px_native("h3_qs_en_len", bi_qs_en_len));
    px_set_global("h3_qs_de_len", px_native("h3_qs_de_len", bi_qs_de_len));
    px_set_global("h3_qs_ins", px_native("h3_qs_ins", bi_qs_ins));
    px_ffi_register("h3_qs_open", bi_qs_open);
    px_ffi_register("h3_qs_close", bi_qs_close);
    px_ffi_register("h3_qs_enc", bi_qs_enc);
    px_ffi_register("h3_qs_take_enc", bi_qs_take_enc);
    px_ffi_register("h3_qs_dec_ingest", bi_qs_dec_ingest);
    px_ffi_register("h3_qs_dec", bi_qs_dec);
    px_ffi_register("h3_settings_enc", bi_settings_enc);
    px_ffi_register("h3_settings_dec", bi_settings_dec);
    px_ffi_register("h3_qs_krc", bi_qs_krc);
    px_ffi_register("h3_qs_enc_ric", bi_qs_enc_ric);
    px_ffi_register("h3_qs_dec_ric", bi_qs_dec_ric);
    px_ffi_register("h3_qs_ack_sec", bi_qs_ack_sec);
    px_ffi_register("h3_qs_ack_inc", bi_qs_ack_inc);
    px_ffi_register("h3_qs_en_len", bi_qs_en_len);
    px_ffi_register("h3_qs_de_len", bi_qs_de_len);
    px_ffi_register("h3_qs_ins", bi_qs_ins);
}

// ============================================================
// M51：纯 C 会话接口（供 runtime_h3.c 线上接线调用——QPACK 动态表
// 会话接入真实 QUIC 单向流）。薄封装：复用上方 bi_* 语言函数与 qd_*
// 核心，零逻辑重复（M49 纯函数语义不变）。
// ============================================================

// 打开 QPACK 会话（cap = 本端 SETTINGS 声明的 QPACK_MAX_TABLE_CAPACITY）。
// 返回会话 id（1..QD_MAX_SESS）| 0 失败。
int64_t px_qd_open(int64_t cap) {
    LXValue a[1]; a[0] = px_int(cap);
    LXValue r = bi_qs_open(a, 1, NULL);
    return (r.type == PX_INT) ? r.as.i : 0;
}

void px_qd_close(int64_t id) {
    LXValue a[1]; a[0] = px_int(id);
    (void)bi_qs_close(a, 1, NULL);
}

// 编码 nf 个字段（names/vals 各自独立字符串 + 长度）→ 字段段字节写 sect[scap]。
// 返回字段段长度 | -1。编码器流指令累积进会话 eout（随后 px_qd_take_enc 取走发编码器流）。
int px_qd_enc(int64_t id, char* const* names, char* const* vals,
              int* nls, int* vls, int nf, uint8_t* sect, int scap) {
    if (nf < 0 || nf > 512 || !sect || scap <= 0) return -1;
    LXValue lst = px_list(8);
    for (int i = 0; i < nf; i++) {
        LXValue pair = px_list(2);
        px_list_push(pair, px_str_len(names[i], nls[i]));
        px_list_push(pair, px_str_len(vals[i], vls[i]));
        px_list_push(lst, pair);
    }
    LXValue a[2]; a[0] = px_int(id); a[1] = lst;
    LXValue r = bi_qs_enc(a, 2, NULL);
    if (r.type != PX_BYTES && r.type != PX_STR) return -1;
    int n = (int)r.as.obj->as.str.len;
    if (n > scap) return -1;
    if (n > 0) memcpy(sect, r.as.obj->as.str.data, (size_t)n);
    return n;
}

// 取走会话 eout（编码器流待发指令），返回字节数 | -1（cap 不足/会话无效）。
int px_qd_take_enc(int64_t id, uint8_t* out, int cap) {
    qd_sess* s = qd_get(id);
    if (!s || !out || cap <= 0) return -1;
    int n = s->eout_len;
    if (n > cap) return -1;
    if (n > 0) memcpy(out, s->eout, (size_t)n);
    s->eout_len = 0;
    return n;
}

// 处理对端编码器流指令字节（更新 decoder 镜像表）。返回 0 成功 | -1 非法/会话无效。
int px_qd_ingest(int64_t id, const uint8_t* p, int len) {
    qd_sess* s = qd_get(id);
    if (!s || !p || len < 0) return -1;
    return qd_dec_ingest(s, p, len);
}

// 解码字段段 → names/vals/nls/vls（每项 malloc，调用方逐个 free 后 free 三个数组）。
// 返回字段数；-1 非法（数据损坏）；-2 阻塞（RIC 超出已 ingest 指令 → 需先泵对端
// 编码器流 ingest 更多字节后重试）。
int px_qd_dec(int64_t id, const uint8_t* p, int len,
              char*** names, char*** vals, int** nls, int** vls) {
    qd_sess* s = qd_get(id);
    if (!s || !p || !names || !vals || !nls || !vls) return -1;
    LXValue r = qd_dec_section(s, p, len);
    if (r.type == PX_NULL) return s->dec_blocked ? -2 : -1;
    if (r.type != PX_LIST) return -1;
    int nf = (int)r.as.obj->as.list.len;
    char** nn = (char**)calloc((size_t)(nf > 0 ? nf : 1), sizeof(char*));
    char** vv = (char**)calloc((size_t)(nf > 0 ? nf : 1), sizeof(char*));
    int* nl = (int*)calloc((size_t)(nf > 0 ? nf : 1), sizeof(int));
    int* vl = (int*)calloc((size_t)(nf > 0 ? nf : 1), sizeof(int));
    if (!nn || !vv || !nl || !vl) { free(nn); free(vv); free(nl); free(vl); return -1; }
    for (int i = 0; i < nf; i++) {
        LXValue it = r.as.obj->as.list.items[i];
        if (it.type != PX_LIST || it.as.obj->as.list.len < 2) continue;
        LXValue* kv = it.as.obj->as.list.items;
        if (kv[0].type == PX_STR || kv[0].type == PX_BYTES) {
            int l0 = (int)kv[0].as.obj->as.str.len;
            nn[i] = (char*)malloc((size_t)l0 + 1);
            if (nn[i]) { memcpy(nn[i], kv[0].as.obj->as.str.data, (size_t)l0); nn[i][l0] = 0; nl[i] = l0; }
        }
        if (kv[1].type == PX_STR || kv[1].type == PX_BYTES) {
            int l1 = (int)kv[1].as.obj->as.str.len;
            vv[i] = (char*)malloc((size_t)l1 + 1);
            if (vv[i]) { memcpy(vv[i], kv[1].as.obj->as.str.data, (size_t)l1); vv[i][l1] = 0; vl[i] = l1; }
        }
    }
    *names = nn; *vals = vv; *nls = nl; *vls = vl;
    return nf;
}

// 诊断/统计辅助：会话无效返回 -1。
int px_qd_en_len(int64_t id)   { qd_sess* s = qd_get(id); return s ? s->en_len : -1; }      // 本端编码表条目
int px_qd_de_len(int64_t id)   { qd_sess* s = qd_get(id); return s ? s->de_len : -1; }      // 对端镜像表条目
int px_qd_ins(int64_t id)      { qd_sess* s = qd_get(id); return s ? (int)s->de_ins : -1; } // 已 ingest 插入数
int px_qd_eout_len(int64_t id) { qd_sess* s = qd_get(id); return s ? s->eout_len : -1; }    // eout 待发字节

// ============ M52：解码器流指令线上处理（本端作为编码器，消费对端解码器流）============
// 注意：QPACK 解码器流指令（RFC 9204 §4.4）无 RIC 字段——Section Ack 只带 stream id；
// 本端编码器按流登记过 outstanding 字段段的 RIC，收到 ack 时用登记的 RIC 提升 KRC。
// 对端解码器流指令由 runtime_h3.c 解析后调用下列函数更新本端编码会话状态。

// Section Acknowledgment：KRC = max(KRC, ric)（RFC 9204 §2.1.4）。返回 0 | -1。
int px_qd_ack_sec(int64_t id, int64_t ric) {
    qd_sess* s = qd_get(id);
    if (!s || ric < 0) return -1;
    if ((uint64_t)ric > s->en_krc) s->en_krc = (uint64_t)ric;
    return 0;
}

// Insert Count Increment：KRC += increment（RFC 9204 §4.4.3）。返回 0 | -1。
int px_qd_ack_inc(int64_t id, int64_t inc) {
    qd_sess* s = qd_get(id);
    if (!s || inc < 0) return -1;
    s->en_krc += (uint64_t)inc;
    return 0;
}

// 本端编码器 Known Received Count | -1。
int64_t px_qd_krc(int64_t id) { qd_sess* s = qd_get(id); return s ? (int64_t)s->en_krc : -1; }

// 最近一次编码字段段的 RIC | -1（发送方登记 outstanding 用）。
int px_qd_enc_last_ric(int64_t id) { qd_sess* s = qd_get(id); return s ? (int)s->en_last_ric : -1; }

// 最近一次成功解码字段段的 RIC | -1（>0 → 上层须发 Section Ack）。
int px_qd_dec_last_ric(int64_t id) { qd_sess* s = qd_get(id); return s ? (int)s->dec_ric : -1; }
