// 普贤 (PuXian) C 运行时 — QPACK 完整 codec（M48）
// ------------------------------------------------------------
// 在 M47 QPACK MVP（仅 Literal Field Line with Literal Name、无 Huffman/静态表）
// 基础上升级为 RFC 9204 无动态表子集的完整编码/解码：
//   - 字段行表示：Indexed Field Line（T=1 静态）、
//     Literal Field Line with Name Reference（T=1 静态）、
//     Literal Field Line with Literal Name（001 模式，name/value 可 Huffman）
//   - Huffman 编解码（RFC 7541 Appendix B，QPACK 复用同表）
//   - 静态表索引压缩（RFC 9204 Appendix A，99 项）
//   - 仍无动态表（MaxTableCapacity=0 语义：前缀恒 RIC=0/Base=0；
//     解码遇动态表引用 → null）
// 语言层接口不变（h3_qenc/h3_qdec），内部实现升级。
#define _GNU_SOURCE
#include "runtime.h"
#include "runtime_h3_qpack_tbl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QP_OUT_MAX (1 << 20)   // 单字段段上限 1MB（对齐 H3_BUF_MAX）

// ==================== prefix integer（RFC 7541 §5.1 / RFC 9204 §4.1.1）====================
// 编码 value → tmp 字节流（tmp[0] 无高位 pattern，调用方 OR 高位）；返回字节数。
static int qp_pref_enc(uint8_t* out, int prefix_bits, uint64_t value) {
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

// 解码：p 指向含 prefix 的字节；prefix_bits 决定低 n 位承载。返回 [值, 消耗字节]。
static uint64_t qp_pref_dec(const uint8_t* p, int maxlen, int prefix_bits, int* used) {
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

// ==================== Huffman（RFC 7541 Appendix B）====================
// 编码 s[0..slen) → out；返回字节数（-1 容量不足）。末尾补 1（EOS 前缀）。
static int qp_huff_enc(uint8_t* out, int cap, const char* s, int slen) {
    uint64_t acc = 0;
    int nbits = 0;
    int o = 0;
    for (int i = 0; i < slen; i++) {
        uint8_t c = (uint8_t)s[i];
        uint32_t code = qp_huff_code[c];
        int L = qp_huff_len[c];
        acc = (acc << L) | code;
        nbits += L;
        while (nbits >= 8) {
            if (o >= cap) return -1;
            out[o++] = (uint8_t)(acc >> (nbits - 8));
            nbits -= 8;
        }
        acc &= (1ULL << nbits) - 1; // 保留低位未输出位
    }
    if (nbits > 0) {  // padding：补 1 至字节边界（EOS=30 个 1 的前缀）
        if (o >= cap) return -1;
        uint8_t pad = (uint8_t)(acc << (8 - nbits));
        pad |= (uint8_t)((1 << (8 - nbits)) - 1);
        out[o++] = pad;
    }
    return o;
}

// 编码长度预判（仅统计，不写）
static int qp_huff_enc_len(const char* s, int slen) {
    uint64_t bits = 0;
    for (int i = 0; i < slen; i++) bits += qp_huff_len[(uint8_t)s[i]];
    return (int)((bits + 7) >> 3);
}

// 从 p 的第 bitpos 位起读 n 位（MSB 序，n<=30）
static uint64_t qp_bits_peek(const uint8_t* p, int bitpos, int n) {
    uint64_t v = 0;
    for (int i = 0; i < n; i++) {
        int bp = bitpos + i;
        int b = (p[bp >> 3] >> (7 - (bp & 7))) & 1;
        v = (v << 1) | (uint64_t)b;
    }
    return v;
}

// 解码 Huffman 字节流 in[0..ilen) → out；返回解码后字节数（-1 非法）。
// 线性扫 257 项匹配（前缀码完备 → 每窗口唯一匹配）；结尾 1..7 位全 1 padding 合法。
static int qp_huff_dec(const uint8_t* in, int ilen, uint8_t* out, int cap) {
    int bitpos = 0;
    int olen = 0;
    int total = ilen * 8;
    while (total - bitpos >= 5) {   // 最短码长 5
        int avail = total - bitpos;
        uint64_t w = qp_bits_peek(in, bitpos, avail >= 30 ? 30 : avail);
        int found = -1, flen = 0;
        for (int i = 0; i < 256; i++) {   // 0..255（EOS=256 不输出）
            int L = qp_huff_len[i];
            if (L > avail) continue;
            uint64_t code = qp_huff_code[i];
            // 窗口高 L 位 == code（code 数值 < 2^L，左对齐比较）
            if ((w >> (avail >= 30 ? (30 - L) : (avail - L))) == code &&
                (avail >= 30 || (avail - L) >= 0)) {
                found = i; flen = L; break;
            }
        }
        if (found < 0) {
            // 无匹配：若剩余 <=7 位且全 1 → padding 结束
            if (avail <= 7) {
                uint64_t pad = w & ((1ULL << avail) - 1);
                uint64_t all1 = (avail == 64) ? ~0ULL : ((1ULL << avail) - 1);
                if (pad == all1) break;
            }
            return -1;
        }
        if (olen >= cap) return -1;
        out[olen++] = (uint8_t)found;
        bitpos += flen;
    }
    // 剩余 <5 位：必须是 1..4 个全 1 padding
    int rem = total - bitpos;
    if (rem > 0) {
        uint64_t pad = qp_bits_peek(in, bitpos, rem);
        uint64_t all1 = (1ULL << rem) - 1;
        if (pad != all1) return -1;
    }
    return olen;
}

// ==================== 静态表查找 ====================
// name/value 完全匹配 → 返回 index；否则 -1
static int qp_static_full(const char* name, int nlen, const char* val, int vlen) {
    for (int i = 0; i < 99; i++) {
        if ((int)strlen(qp_static_table[i].name) == nlen &&
            memcmp(qp_static_table[i].name, name, (size_t)nlen) == 0 &&
            (int)strlen(qp_static_table[i].value) == vlen &&
            memcmp(qp_static_table[i].value, val, (size_t)vlen) == 0)
            return i;
    }
    return -1;
}

// name 匹配 → 返回第一个同名 index；否则 -1
static int qp_static_name(const char* name, int nlen) {
    for (int i = 0; i < 99; i++) {
        if ((int)strlen(qp_static_table[i].name) == nlen &&
            memcmp(qp_static_table[i].name, name, (size_t)nlen) == 0)
            return i;
    }
    return -1;
}

// ==================== 编码端 ====================
// 写入 8-bit prefix string literal（value）：H 位 + 7-bit len + data（Huffman 若更短）。
// 返回新增字节数或 -1。
static int qp_enc_value_string(uint8_t* out, int cap, int off, const char* val, int vlen) {
    int hl = qp_huff_enc_len(val, vlen);
    int use_h = hl > 0 && hl < vlen;
    int data_len = use_h ? hl : vlen;
    uint8_t tmp[16];
    int n = qp_pref_enc(tmp, 7, (uint64_t)data_len);
    if (off + n + data_len > cap) return -1;
    out[off++] = (uint8_t)((use_h ? 0x80 : 0) | tmp[0]);
    for (int i = 1; i < n; i++) out[off++] = tmp[i];
    if (use_h) {
        int r = qp_huff_enc(out + off, cap - off, val, vlen);
        if (r < 0) return -1;
        off += r;
    } else {
        memcpy(out + off, val, (size_t)vlen);
        off += vlen;
    }
    return off;
}

// 编码字段段（2 字节前缀 00 00 + 字段行）。fields: list of [name,value]（PX_LIST）
// 返回总字节数或 -1。
int px_h3_qenc(uint8_t* out, LXValue* fields, int nf) {
    int off = 0;
    out[off++] = 0x00;   // Required Insert Count = 0
    out[off++] = 0x00;   // Base = 0（S=0, Delta=0）
    for (int i = 0; i < nf; i++) {
        if (fields[i].type != PX_LIST && fields[i].type != PX_TUPLE) return -1;
        LXObject* f = fields[i].as.obj;
        if (f->as.list.len < 2) return -1;
        LXValue* kv = f->as.list.items;
        if (kv[0].type != PX_STR || kv[1].type != PX_STR) return -1;
        const char* nm = kv[0].as.obj->as.str.data; int nml = kv[0].as.obj->as.str.len;
        const char* vl = kv[1].as.obj->as.str.data; int vll = kv[1].as.obj->as.str.len;
        if (off + nml + vll + 16 > QP_OUT_MAX) return -1;

        int full = qp_static_full(nm, nml, vl, vll);
        if (full >= 0) {
            // Indexed Field Line：1 T=1 | Index(6+)
            uint8_t tmp[16];
            int n = qp_pref_enc(tmp, 6, (uint64_t)full);
            out[off++] = (uint8_t)(0xC0 | (tmp[0] & 0x3f));
            for (int j = 1; j < n; j++) out[off++] = tmp[j];
            continue;
        }
        int nidx = qp_static_name(nm, nml);
        if (nidx >= 0) {
            // Literal Field Line with Name Reference：01 N=0 T=1 | NameIndex(4+) + value string
            uint8_t tmp[16];
            int n = qp_pref_enc(tmp, 4, (uint64_t)nidx);
            out[off++] = (uint8_t)(0x50 | (tmp[0] & 0x0f));
            for (int j = 1; j < n; j++) out[off++] = tmp[j];
            int no = qp_enc_value_string(out, QP_OUT_MAX, off, vl, vll);
            if (no < 0) return -1;
            off = no;
            continue;
        }
        // Literal Field Line with Literal Name：001 N=0 H | NameLen(3+) name + value string
        int nhlen = qp_huff_enc_len(nm, nml);
        int name_h = nhlen > 0 && nhlen < nml;
        int name_len = name_h ? nhlen : nml;
        uint8_t tmp[16];
        int n = qp_pref_enc(tmp, 3, (uint64_t)name_len);
        out[off++] = (uint8_t)(0x20 | (name_h ? 0x08 : 0) | (tmp[0] & 0x07));
        for (int j = 1; j < n; j++) out[off++] = tmp[j];
        if (name_h) {
            int r = qp_huff_enc(out + off, QP_OUT_MAX - off, nm, nml);
            if (r < 0) return -1;
            off += r;
        } else {
            memcpy(out + off, nm, (size_t)nml);
            off += nml;
        }
        int no = qp_enc_value_string(out, QP_OUT_MAX, off, vl, vll);
        if (no < 0) return -1;
        off = no;
    }
    return off;
}

// ==================== 解码端 ====================
// 解码一段 8-bit prefix string literal（value 形式，H 在 bit7）；返回 [ok, off, str LXValue]
static int qp_dec_value_string(const uint8_t* p, int len, int off, LXValue* outv) {
    if (off >= len) return -1;
    int H = (p[off] >> 7) & 1;
    int used = 0;
    uint64_t slen = qp_pref_dec(p + off, len - off, 7, &used);
    if (used == 0) return -1;
    off += used;
    if (off + (int)slen > len) return -1;
    if (!H) {
        *outv = px_str_len((const char*)(p + off), (int)slen);
        return off + (int)slen;
    }
    // Huffman 解码：上限按 slen*2+16 保守分配（解码输出≤输入字节×2 的常见情况），
    // 超限时按 1MB 上限拒绝（MVP 防爆）
    size_t tcap = (size_t)(slen * 2 + 16 < (1 << 20) ? slen * 2 + 16 : (1 << 20));
    uint8_t* tmp = (uint8_t*)malloc(tcap);
    if (!tmp) return -1;
    int r = qp_huff_dec(p + off, (int)slen, tmp, (int)tcap);
    if (r < 0) { free(tmp); return -1; }
    *outv = px_str_len((const char*)tmp, r);
    free(tmp);
    return off + (int)slen;
}

// 解码字段段 → list of [name,value] | null
LXValue px_h3_qdec(const uint8_t* p, int len) {
    int off = 0;
    int used = 0;
    if (len < 2) return px_null();
    uint64_t ric = qp_pref_dec(p, len, 8, &used);      // Required Insert Count（8-bit）
    if (used == 0) return px_null();
    off += used;
    if (off >= len) return px_null();
    uint64_t base = qp_pref_dec(p + off, len - off, 7, &used);  // Base（S+7，S 忽略）
    if (used == 0) return px_null();
    off += used;
    if (ric != 0) return px_null();    // 无动态表：RIC 必须 0（引用动态表不支持）
    LXValue fields = px_list(8);
    while (off < len) {
        uint8_t b0 = p[off];
        if (b0 & 0x80) {
            // Indexed Field Line：1 T | Index(6+)。T=1 静态；T=0 动态 → null
            if (!(b0 & 0x40)) return px_null();
            uint64_t idx = qp_pref_dec(p + off, len - off, 6, &used);
            if (used == 0) return px_null();
            off += used;
            if (idx >= 99) return px_null();
            LXValue pair = px_list(2);
            px_list_push(pair, px_str(qp_static_table[idx].name));
            px_list_push(pair, px_str(qp_static_table[idx].value));
            px_list_push(fields, pair);
        } else if (b0 & 0x40) {
            // Literal Field Line with Name Reference：01 N T | NameIndex(4+) + value
            if (!(b0 & 0x10)) return px_null();   // T=0 动态 → null
            uint64_t idx = qp_pref_dec(p + off, len - off, 4, &used);
            if (used == 0) return px_null();
            off += used;
            if (idx >= 99) return px_null();
            LXValue val = px_null();
            int no = qp_dec_value_string(p, len, off, &val);
            if (no < 0) return px_null();
            off = no;
            LXValue pair = px_list(2);
            px_list_push(pair, px_str(qp_static_table[idx].name));
            px_list_push(pair, val);
            px_list_push(fields, pair);
        } else if (b0 & 0x20) {
            // Literal Field Line with Literal Name：001 N H | NameLen(3+) name + value
            int name_h = (b0 >> 3) & 1;
            uint64_t nlen = qp_pref_dec(p + off, len - off, 3, &used);
            if (used == 0) return px_null();
            off += used;
            if (off + (int)nlen > len) return px_null();
            LXValue name = px_null();
            if (!name_h) {
                name = px_str_len((const char*)(p + off), (int)nlen);
                off += (int)nlen;
            } else {
                size_t tcap = (size_t)(nlen * 2 + 16 < (1 << 20) ? nlen * 2 + 16 : (1 << 20));
                uint8_t* tmp = (uint8_t*)malloc(tcap);
                if (!tmp) return px_null();
                int r = qp_huff_dec(p + off, (int)nlen, tmp, (int)tcap);
                if (r < 0) { free(tmp); return px_null(); }
                name = px_str_len((const char*)tmp, r);
                free(tmp);
                off += (int)nlen;
            }
            LXValue val = px_null();
            int no = qp_dec_value_string(p, len, off, &val);
            if (no < 0) return px_null();
            off = no;
            LXValue pair = px_list(2);
            px_list_push(pair, name);
            px_list_push(pair, val);
            px_list_push(fields, pair);
        } else {
            return px_null();   // 0000/0001 post-base / 其它 → 无动态表不支持
        }
    }
    return fields;
}

// ==================== 语言层绑定：Huffman 纯 codec（capability/互操作验证用）====================
// h3_huff(s:str|bytes) -> bytes：Huffman 编码（RFC 7541）
LXValue bi_h3_huff(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || (args[0].type != PX_STR && args[0].type != PX_BYTES))
        px_error("h3_huff 需要 (s: str|bytes)");
    const char* s = args[0].as.obj->as.str.data;
    int slen = args[0].as.obj->as.str.len;
    int cap = slen * 4 + 16;
    if (cap < 64) cap = 64;
    if (cap > (1 << 20)) cap = 1 << 20;
    uint8_t* out = (uint8_t*)malloc((size_t)cap);
    if (!out) return px_null();
    int n = qp_huff_enc(out, cap, s, slen);
    if (n < 0) { free(out); return px_null(); }
    LXValue r = px_bytes_len(out, n);
    free(out);
    return r;
}

// h3_unhuff(data:bytes) -> str|null：Huffman 解码
LXValue bi_h3_unhuff(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 1 || (args[0].type != PX_STR && args[0].type != PX_BYTES))
        px_error("h3_unhuff 需要 (data: bytes)");
    const uint8_t* p = (const uint8_t*)args[0].as.obj->as.str.data;
    int plen = args[0].as.obj->as.str.len;
    int cap = plen * 2 + 16;
    if (cap < 64) cap = 64;
    if (cap > (1 << 20)) cap = 1 << 20;
    uint8_t* tmp = (uint8_t*)malloc((size_t)cap);
    if (!tmp) return px_null();
    int r = qp_huff_dec(p, plen, tmp, cap);
    if (r < 0) { free(tmp); return px_null(); }
    LXValue v = px_str_len((const char*)tmp, r);
    free(tmp);
    return v;
}
