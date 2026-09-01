/* 普贤 (PuXian) C 运行时 — HTTP/2 最小服务端（M35）
 * h2c Upgrade + prior knowledge + HPACK（静态表 + 字面量 + Huffman）+ 帧层
 * 与解释器 h2.rs 双模式一致：SETTINGS/PING/HEADERS/DATA/WINDOW_UPDATE/RST_STREAM/GOAWAY
 * 响应：单流 GET → HEADERS(:status 200 + content-type + content-length) + DATA
 */
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

/* RFC 7541 附录 B Huffman 表 (code, len) */
static const unsigned int HPACK_HUFFMAN[257][2] = {
    {0x1ff8u, 13u}, /* 0 */
    {0x7fffd8u, 23u}, /* 1 */
    {0xfffffe2u, 28u}, /* 2 */
    {0xfffffe3u, 28u}, /* 3 */
    {0xfffffe4u, 28u}, /* 4 */
    {0xfffffe5u, 28u}, /* 5 */
    {0xfffffe6u, 28u}, /* 6 */
    {0xfffffe7u, 28u}, /* 7 */
    {0xfffffe8u, 28u}, /* 8 */
    {0xffffeau, 24u}, /* 9 */
    {0x3ffffffcu, 30u}, /* 10 */
    {0xfffffe9u, 28u}, /* 11 */
    {0xfffffeau, 28u}, /* 12 */
    {0x3ffffffdu, 30u}, /* 13 */
    {0xfffffebu, 28u}, /* 14 */
    {0xfffffecu, 28u}, /* 15 */
    {0xfffffedu, 28u}, /* 16 */
    {0xfffffeeu, 28u}, /* 17 */
    {0xfffffefu, 28u}, /* 18 */
    {0xffffff0u, 28u}, /* 19 */
    {0xffffff1u, 28u}, /* 20 */
    {0xffffff2u, 28u}, /* 21 */
    {0x3ffffffeu, 30u}, /* 22 */
    {0xffffff3u, 28u}, /* 23 */
    {0xffffff4u, 28u}, /* 24 */
    {0xffffff5u, 28u}, /* 25 */
    {0xffffff6u, 28u}, /* 26 */
    {0xffffff7u, 28u}, /* 27 */
    {0xffffff8u, 28u}, /* 28 */
    {0xffffff9u, 28u}, /* 29 */
    {0xffffffau, 28u}, /* 30 */
    {0xffffffbu, 28u}, /* 31 */
    {0x14u, 6u}, /* 32 */
    {0x3f8u, 10u}, /* 33 */
    {0x3f9u, 10u}, /* 34 */
    {0xffau, 12u}, /* 35 */
    {0x1ff9u, 13u}, /* 36 */
    {0x15u, 6u}, /* 37 */
    {0xf8u, 8u}, /* 38 */
    {0x7fau, 11u}, /* 39 */
    {0x3fau, 10u}, /* 40 */
    {0x3fbu, 10u}, /* 41 */
    {0xf9u, 8u}, /* 42 */
    {0x7fbu, 11u}, /* 43 */
    {0xfau, 8u}, /* 44 */
    {0x16u, 6u}, /* 45 */
    {0x17u, 6u}, /* 46 */
    {0x18u, 6u}, /* 47 */
    {0x0u, 5u}, /* 48 */
    {0x1u, 5u}, /* 49 */
    {0x2u, 5u}, /* 50 */
    {0x19u, 6u}, /* 51 */
    {0x1au, 6u}, /* 52 */
    {0x1bu, 6u}, /* 53 */
    {0x1cu, 6u}, /* 54 */
    {0x1du, 6u}, /* 55 */
    {0x1eu, 6u}, /* 56 */
    {0x1fu, 6u}, /* 57 */
    {0x5cu, 7u}, /* 58 */
    {0xfbu, 8u}, /* 59 */
    {0x7ffcu, 15u}, /* 60 */
    {0x20u, 6u}, /* 61 */
    {0xffbu, 12u}, /* 62 */
    {0x3fcu, 10u}, /* 63 */
    {0x1ffau, 13u}, /* 64 */
    {0x21u, 6u}, /* 65 */
    {0x5du, 7u}, /* 66 */
    {0x5eu, 7u}, /* 67 */
    {0x5fu, 7u}, /* 68 */
    {0x60u, 7u}, /* 69 */
    {0x61u, 7u}, /* 70 */
    {0x62u, 7u}, /* 71 */
    {0x63u, 7u}, /* 72 */
    {0x64u, 7u}, /* 73 */
    {0x65u, 7u}, /* 74 */
    {0x66u, 7u}, /* 75 */
    {0x67u, 7u}, /* 76 */
    {0x68u, 7u}, /* 77 */
    {0x69u, 7u}, /* 78 */
    {0x6au, 7u}, /* 79 */
    {0x6bu, 7u}, /* 80 */
    {0x6cu, 7u}, /* 81 */
    {0x6du, 7u}, /* 82 */
    {0x6eu, 7u}, /* 83 */
    {0x6fu, 7u}, /* 84 */
    {0x70u, 7u}, /* 85 */
    {0x71u, 7u}, /* 86 */
    {0x72u, 7u}, /* 87 */
    {0xfcu, 8u}, /* 88 */
    {0x73u, 7u}, /* 89 */
    {0xfdu, 8u}, /* 90 */
    {0x1ffbu, 13u}, /* 91 */
    {0x7fff0u, 19u}, /* 92 */
    {0x1ffcu, 13u}, /* 93 */
    {0x3ffcu, 14u}, /* 94 */
    {0x22u, 6u}, /* 95 */
    {0x7ffdu, 15u}, /* 96 */
    {0x3u, 5u}, /* 97 */
    {0x23u, 6u}, /* 98 */
    {0x4u, 5u}, /* 99 */
    {0x24u, 6u}, /* 100 */
    {0x5u, 5u}, /* 101 */
    {0x25u, 6u}, /* 102 */
    {0x26u, 6u}, /* 103 */
    {0x27u, 6u}, /* 104 */
    {0x6u, 5u}, /* 105 */
    {0x74u, 7u}, /* 106 */
    {0x75u, 7u}, /* 107 */
    {0x28u, 6u}, /* 108 */
    {0x29u, 6u}, /* 109 */
    {0x2au, 6u}, /* 110 */
    {0x7u, 5u}, /* 111 */
    {0x2bu, 6u}, /* 112 */
    {0x76u, 7u}, /* 113 */
    {0x2cu, 6u}, /* 114 */
    {0x8u, 5u}, /* 115 */
    {0x9u, 5u}, /* 116 */
    {0x2du, 6u}, /* 117 */
    {0x77u, 7u}, /* 118 */
    {0x78u, 7u}, /* 119 */
    {0x79u, 7u}, /* 120 */
    {0x7au, 7u}, /* 121 */
    {0x7bu, 7u}, /* 122 */
    {0x7ffeu, 15u}, /* 123 */
    {0x7fcu, 11u}, /* 124 */
    {0x3ffdu, 14u}, /* 125 */
    {0x1ffdu, 13u}, /* 126 */
    {0xffffffcu, 28u}, /* 127 */
    {0xfffe6u, 20u}, /* 128 */
    {0x3fffd2u, 22u}, /* 129 */
    {0xfffe7u, 20u}, /* 130 */
    {0xfffe8u, 20u}, /* 131 */
    {0x3fffd3u, 22u}, /* 132 */
    {0x3fffd4u, 22u}, /* 133 */
    {0x3fffd5u, 22u}, /* 134 */
    {0x7fffd9u, 23u}, /* 135 */
    {0x3fffd6u, 22u}, /* 136 */
    {0x7fffdau, 23u}, /* 137 */
    {0x7fffdbu, 23u}, /* 138 */
    {0x7fffdcu, 23u}, /* 139 */
    {0x7fffddu, 23u}, /* 140 */
    {0x7fffdeu, 23u}, /* 141 */
    {0xffffebu, 24u}, /* 142 */
    {0x7fffdfu, 23u}, /* 143 */
    {0xffffecu, 24u}, /* 144 */
    {0xffffedu, 24u}, /* 145 */
    {0x3fffd7u, 22u}, /* 146 */
    {0x7fffe0u, 23u}, /* 147 */
    {0xffffeeu, 24u}, /* 148 */
    {0x7fffe1u, 23u}, /* 149 */
    {0x7fffe2u, 23u}, /* 150 */
    {0x7fffe3u, 23u}, /* 151 */
    {0x7fffe4u, 23u}, /* 152 */
    {0x1fffdcu, 21u}, /* 153 */
    {0x3fffd8u, 22u}, /* 154 */
    {0x7fffe5u, 23u}, /* 155 */
    {0x3fffd9u, 22u}, /* 156 */
    {0x7fffe6u, 23u}, /* 157 */
    {0x7fffe7u, 23u}, /* 158 */
    {0xffffefu, 24u}, /* 159 */
    {0x3fffdau, 22u}, /* 160 */
    {0x1fffddu, 21u}, /* 161 */
    {0xfffe9u, 20u}, /* 162 */
    {0x3fffdbu, 22u}, /* 163 */
    {0x3fffdcu, 22u}, /* 164 */
    {0x7fffe8u, 23u}, /* 165 */
    {0x7fffe9u, 23u}, /* 166 */
    {0x1fffdeu, 21u}, /* 167 */
    {0x7fffeau, 23u}, /* 168 */
    {0x3fffddu, 22u}, /* 169 */
    {0x3fffdeu, 22u}, /* 170 */
    {0xfffff0u, 24u}, /* 171 */
    {0x1fffdfu, 21u}, /* 172 */
    {0x3fffdfu, 22u}, /* 173 */
    {0x7fffebu, 23u}, /* 174 */
    {0x7fffecu, 23u}, /* 175 */
    {0x1fffe0u, 21u}, /* 176 */
    {0x1fffe1u, 21u}, /* 177 */
    {0x3fffe0u, 22u}, /* 178 */
    {0x1fffe2u, 21u}, /* 179 */
    {0x7fffedu, 23u}, /* 180 */
    {0x3fffe1u, 22u}, /* 181 */
    {0x7fffeeu, 23u}, /* 182 */
    {0x7fffefu, 23u}, /* 183 */
    {0xfffeau, 20u}, /* 184 */
    {0x3fffe2u, 22u}, /* 185 */
    {0x3fffe3u, 22u}, /* 186 */
    {0x3fffe4u, 22u}, /* 187 */
    {0x7ffff0u, 23u}, /* 188 */
    {0x3fffe5u, 22u}, /* 189 */
    {0x3fffe6u, 22u}, /* 190 */
    {0x7ffff1u, 23u}, /* 191 */
    {0x3ffffe0u, 26u}, /* 192 */
    {0x3ffffe1u, 26u}, /* 193 */
    {0xfffebu, 20u}, /* 194 */
    {0x7fff1u, 19u}, /* 195 */
    {0x3fffe7u, 22u}, /* 196 */
    {0x7ffff2u, 23u}, /* 197 */
    {0x3fffe8u, 22u}, /* 198 */
    {0x1ffffecu, 25u}, /* 199 */
    {0x3ffffe2u, 26u}, /* 200 */
    {0x3ffffe3u, 26u}, /* 201 */
    {0x3ffffe4u, 26u}, /* 202 */
    {0x7ffffdeu, 27u}, /* 203 */
    {0x7ffffdfu, 27u}, /* 204 */
    {0x3ffffe5u, 26u}, /* 205 */
    {0xfffff1u, 24u}, /* 206 */
    {0x1ffffedu, 25u}, /* 207 */
    {0x7fff2u, 19u}, /* 208 */
    {0x1fffe3u, 21u}, /* 209 */
    {0x3ffffe6u, 26u}, /* 210 */
    {0x7ffffe0u, 27u}, /* 211 */
    {0x7ffffe1u, 27u}, /* 212 */
    {0x3ffffe7u, 26u}, /* 213 */
    {0x7ffffe2u, 27u}, /* 214 */
    {0xfffff2u, 24u}, /* 215 */
    {0x1fffe4u, 21u}, /* 216 */
    {0x1fffe5u, 21u}, /* 217 */
    {0x3ffffe8u, 26u}, /* 218 */
    {0x3ffffe9u, 26u}, /* 219 */
    {0xffffffdu, 28u}, /* 220 */
    {0x7ffffe3u, 27u}, /* 221 */
    {0x7ffffe4u, 27u}, /* 222 */
    {0x7ffffe5u, 27u}, /* 223 */
    {0xfffecu, 20u}, /* 224 */
    {0xfffff3u, 24u}, /* 225 */
    {0xfffedu, 20u}, /* 226 */
    {0x1fffe6u, 21u}, /* 227 */
    {0x3fffe9u, 22u}, /* 228 */
    {0x1fffe7u, 21u}, /* 229 */
    {0x1fffe8u, 21u}, /* 230 */
    {0x7ffff3u, 23u}, /* 231 */
    {0x3fffeau, 22u}, /* 232 */
    {0x3fffebu, 22u}, /* 233 */
    {0x1ffffeeu, 25u}, /* 234 */
    {0x1ffffefu, 25u}, /* 235 */
    {0xfffff4u, 24u}, /* 236 */
    {0xfffff5u, 24u}, /* 237 */
    {0x3ffffeau, 26u}, /* 238 */
    {0x7ffff4u, 23u}, /* 239 */
    {0x3ffffebu, 26u}, /* 240 */
    {0x7ffffe6u, 27u}, /* 241 */
    {0x3ffffecu, 26u}, /* 242 */
    {0x3ffffedu, 26u}, /* 243 */
    {0x7ffffe7u, 27u}, /* 244 */
    {0x7ffffe8u, 27u}, /* 245 */
    {0x7ffffe9u, 27u}, /* 246 */
    {0x7ffffeau, 27u}, /* 247 */
    {0x7ffffebu, 27u}, /* 248 */
    {0xffffffeu, 28u}, /* 249 */
    {0x7ffffecu, 27u}, /* 250 */
    {0x7ffffedu, 27u}, /* 251 */
    {0x7ffffeeu, 27u}, /* 252 */
    {0x7ffffefu, 27u}, /* 253 */
    {0x7fffff0u, 27u}, /* 254 */
    {0x3ffffeeu, 26u}, /* 255 */
    {0x3fffffffu, 30u}, /* 256 */
};

/* RFC 7541 附录 A 静态表 (name, value) */
static const char* HPACK_STATIC[61][2] = {
    {":authority", ""},
    {":method", "GET"},
    {":method", "POST"},
    {":path", "/"},
    {":path", "/index.html"},
    {":scheme", "http"},
    {":scheme", "https"},
    {":status", "200"},
    {":status", "204"},
    {":status", "206"},
    {":status", "304"},
    {":status", "400"},
    {":status", "404"},
    {":status", "500"},
    {"accept-charset", ""},
    {"accept-encoding", "gzip, deflate"},
    {"accept-language", ""},
    {"accept-ranges", ""},
    {"accept", ""},
    {"access-control-allow-origin", ""},
    {"age", ""},
    {"allow", ""},
    {"authorization", ""},
    {"cache-control", ""},
    {"content-disposition", ""},
    {"content-encoding", ""},
    {"content-language", ""},
    {"content-length", ""},
    {"content-location", ""},
    {"content-range", ""},
    {"content-type", ""},
    {"cookie", ""},
    {"date", ""},
    {"etag", ""},
    {"expect", ""},
    {"expires", ""},
    {"from", ""},
    {"host", ""},
    {"if-match", ""},
    {"if-modified-since", ""},
    {"if-none-match", ""},
    {"if-range", ""},
    {"if-unmodified-since", ""},
    {"last-modified", ""},
    {"link", ""},
    {"location", ""},
    {"max-forwards", ""},
    {"proxy-authenticate", ""},
    {"proxy-authorization", ""},
    {"range", ""},
    {"referer", ""},
    {"refresh", ""},
    {"retry-after", ""},
    {"server", ""},
    {"set-cookie", ""},
    {"strict-transport-security", ""},
    {"transfer-encoding", ""},
    {"user-agent", ""},
    {"vary", ""},
    {"via", ""},
    {"www-authenticate", ""},
};


// ==================== HPACK 解码 ====================

// Huffman 解码（RFC 7541 附录 B）：bit 流 → 字节；EOS(256) 提前结束
static int h2_huffman_decode(const unsigned char* bits, int blen, unsigned char* out, int outcap, int* outlen) {
    unsigned int code = 0;
    int n = 0;
    int o = 0;
    for (int bi = 0; bi < blen; bi++) {
        for (int i = 7; i >= 0; i--) {
            code = (code << 1) | ((bits[bi] >> i) & 1);
            n++;
            // 线性查表
            int found = -1;
            for (int sym = 0; sym < 257; sym++) {
                if ((int)HPACK_HUFFMAN[sym][1] == n && HPACK_HUFFMAN[sym][0] == code) {
                    found = sym;
                    break;
                }
            }
            if (found >= 0) {
                if (found == 256) { *outlen = o; return 0; } // EOS
                if (o >= outcap) return -1;
                out[o++] = (unsigned char)found;
                code = 0;
                n = 0;
            }
        }
    }
    *outlen = o;
    return 0;
}

// 读 HPACK 字符串（首字节高 1 位 H=Huffman，低 7 位长度；全 1 扩展）→ 写入 out（动态分配）
static int h2_hpack_str(const unsigned char* data, int dlen, int* pos, char** out) {
    if (*pos >= dlen) return -1;
    unsigned char b = data[(*pos)++];
    int huff = (b & 0x80) != 0;
    int len = b & 0x7f;
    if (len == 127) {
        for (;;) {
            if (*pos >= dlen) return -1;
            unsigned char c = data[(*pos)++];
            len = (len << 7) | (c & 0x7f);
            if ((c & 0x80) == 0) break;
        }
    }
    if (*pos + len > dlen) return -1;
    if (huff) {
        unsigned char* dec = malloc((size_t)len * 2 + 1);
        int olen = 0;
        if (!dec) return -1;
        if (h2_huffman_decode(data + *pos, len, dec, len * 2 + 1, &olen) != 0) { free(dec); return -1; }
        dec[olen] = 0;
        *out = (char*)dec;
        *pos += len;
        return olen;
    }
    char* s = malloc((size_t)len + 1);
    if (!s) return -1;
    memcpy(s, data + *pos, (size_t)len);
    s[len] = 0;
    *out = s;
    *pos += len;
    return len;
}

// 解码一个 header block → (name, value) 数组；返回 header 数（nmax 上限）
typedef struct {
    char name[128];
    char value[512];
} H2Header;
static int h2_hpack_decode(const unsigned char* data, int dlen, H2Header* hdrs, int nmax) {
    int nh = 0;
    int pos = 0;
    while (pos < dlen) {
        unsigned char b = data[pos];
        if (b & 0x80) {
            // 索引引用（静态表）
            int idx = b & 0x7f;
            pos++;
            if (idx == 0) {
                for (;;) {
                    if (pos >= dlen) return -1;
                    unsigned char c = data[pos++];
                    idx = (idx << 7) | (c & 0x7f);
                    if ((c & 0x80) == 0) break;
                }
            }
            if (idx >= 1 && idx <= 61) {
                if (nh >= nmax) return -1;
                snprintf(hdrs[nh].name, sizeof(hdrs[nh].name), "%s", HPACK_STATIC[idx - 1][0]);
                snprintf(hdrs[nh].value, sizeof(hdrs[nh].value), "%s", HPACK_STATIC[idx - 1][1]);
                nh++;
            } else return -1;
        } else if (b & 0x40) {
            // 字面量增量索引：name 索引或字面量 + value 字面量
            int nidx = b & 0x3f;
            pos++;
            if (nh >= nmax) return -1;
            if (nidx == 0) {
                char* name = NULL;
                if (h2_hpack_str(data, dlen, &pos, &name) < 0) return -1;
                snprintf(hdrs[nh].name, sizeof(hdrs[nh].name), "%s", name);
                free(name);
            } else if (nidx >= 1 && nidx <= 61) {
                snprintf(hdrs[nh].name, sizeof(hdrs[nh].name), "%s", HPACK_STATIC[nidx - 1][0]);
            } else return -1;
            char* val = NULL;
            if (h2_hpack_str(data, dlen, &pos, &val) < 0) return -1;
            snprintf(hdrs[nh].value, sizeof(hdrs[nh].value), "%s", val);
            free(val);
            nh++;
        } else if (b & 0x20) {
            // 动态表大小更新（忽略）
            pos++;
        } else {
            // 字面量无索引 / 从不索引
            int nidx = b & 0x1f;
            pos++;
            if (nh >= nmax) return -1;
            if (nidx == 0) {
                char* name = NULL;
                if (h2_hpack_str(data, dlen, &pos, &name) < 0) return -1;
                snprintf(hdrs[nh].name, sizeof(hdrs[nh].name), "%s", name);
                free(name);
            } else if (nidx >= 1 && nidx <= 61) {
                snprintf(hdrs[nh].name, sizeof(hdrs[nh].name), "%s", HPACK_STATIC[nidx - 1][0]);
            } else return -1;
            char* val = NULL;
            if (h2_hpack_str(data, dlen, &pos, &val) < 0) return -1;
            snprintf(hdrs[nh].value, sizeof(hdrs[nh].value), "%s", val);
            free(val);
            nh++;
        }
    }
    return nh;
}

// HPACK 编码字符串（字面量，无 Huffman）
static int h2_hpack_enc_str(unsigned char* out, const char* s) {
    int len = (int)strlen(s);
    out[0] = (unsigned char)len;
    memcpy(out + 1, s, (size_t)len);
    return len + 1;
}

// 响应 HEADERS 编码（:status 200 + content-type + content-length）→ 返回长度
static int h2_hpack_enc_response(unsigned char* out, int status, const char* ct, int body_len) {
    int o = 0;
    if (status == 200) {
        out[o++] = 0x88; // 索引 8 = :status 200
    } else {
        out[o++] = 0x40 | 8; // 增量索引 name :status（索引 8）
        o += h2_hpack_enc_str(out + o, "404");
    }
    out[o++] = 0x40 | 31; // content-type（索引 31）
    o += h2_hpack_enc_str(out + o, ct);
    out[o++] = 0x40 | 28; // content-length（索引 28）
    char lenstr[32];
    snprintf(lenstr, sizeof(lenstr), "%d", body_len);
    o += h2_hpack_enc_str(out + o, lenstr);
    return o;
}

// ==================== 帧层 ====================

// 读 n 字节：先消费 pending，不足再读连接（WouldBlock/TimedOut 短暂重试）
static int h2_read_exact(PxConn* c, const unsigned char** pending, int* plen, unsigned char* buf, int n) {
    int got = 0;
    while (got < n && *plen > 0) {
        int take = (n - got) < *plen ? (n - got) : *plen;
        memcpy(buf + got, *pending, (size_t)take);
        *pending += take;
        *plen -= take;
        got += take;
    }
    while (got < n) {
        ssize_t r = px_conn_read(c, buf + got, (size_t)(n - got));
        if (r > 0) { got += (int)r; continue; }
        if (r == 0) return -1; // EOF
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            struct timespec ts = { 0, 5000000 };
            nanosleep(&ts, NULL);
            continue;
        }
        return -1;
    }
    return 0;
}

// 读一帧（帧头 9 字节 + payload）
static int h2_read_frame(PxConn* c, const unsigned char** pending, int* plen,
                         unsigned char* hdr, unsigned char** payload, int* plen_out,
                         unsigned char* fty, unsigned char* fflags, unsigned int* fstream) {
    if (h2_read_exact(c, pending, plen, hdr, 9) != 0) return -1;
    int len = ((int)hdr[0] << 16) | ((int)hdr[1] << 8) | hdr[2];
    *fty = hdr[3];
    *fflags = hdr[4];
    *fstream = ((unsigned int)hdr[5] << 24) | ((unsigned int)hdr[6] << 16) | ((unsigned int)hdr[7] << 8) | hdr[8];
    if (len > (1 << 20)) return -1;
    *payload = NULL;
    *plen_out = 0;
    if (len > 0) {
        *payload = malloc((size_t)len);
        if (!*payload) return -1;
        if (h2_read_exact(c, pending, plen, *payload, len) != 0) { free(*payload); *payload = NULL; return -1; }
        *plen_out = len;
    }
    return 0;
}

// 写一帧
static int h2_write_frame(PxConn* c, unsigned char ty, unsigned char flags, unsigned int stream,
                          const unsigned char* payload, int plen) {
    unsigned char h[9];
    h[0] = (unsigned char)((plen >> 16) & 0xff);
    h[1] = (unsigned char)((plen >> 8) & 0xff);
    h[2] = (unsigned char)(plen & 0xff);
    h[3] = ty;
    h[4] = flags;
    h[5] = (unsigned char)((stream >> 24) & 0xff);
    h[6] = (unsigned char)((stream >> 16) & 0xff);
    h[7] = (unsigned char)((stream >> 8) & 0xff);
    h[8] = (unsigned char)(stream & 0xff);
    if (px_conn_write(c, h, 9) != 9) return -1;
    if (plen > 0 && px_conn_write(c, payload, (size_t)plen) != plen) return -1;
    return 0;
}

// ==================== 主处理 ====================

// 处理 HTTP/2 连接（upgrade=1 h2c 已读 HTTP/1.1 请求头；否则 prior knowledge）
// residual/pending：请求头后缓冲的残留字节（含 client preface）
void px_h2_handle(void* cp, int upgrade, const unsigned char* residual, int rlen) {
    PxConn* c = (PxConn*)cp;
    if (upgrade) {
        static const char* sw = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: h2c\r\n\r\n";
        px_conn_write(c, sw, (int)strlen(sw));
    }
    // 服务端 SETTINGS（空）
    h2_write_frame(c, 4, 0, 0, NULL, 0);
    // 读 client preface（24 字节）
    const unsigned char* pend = residual;
    int plen = rlen;
    unsigned char preface[24];
    if (h2_read_exact(c, &pend, &plen, preface, 24) != 0) return;
    static const unsigned char PREFACE[24] = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
    if (memcmp(preface, PREFACE, 24) != 0) return;
    // 帧循环
    for (;;) {
        unsigned char hdr[9];
        unsigned char* payload = NULL;
        int plen_out = 0;
        unsigned char fty = 0, fflags = 0;
        unsigned int fstream = 0;
        if (h2_read_frame(c, &pend, &plen, hdr, &payload, &plen_out, &fty, &fflags, &fstream) != 0) {
            if (payload) free(payload);
            break;
        }
        switch (fty) {
            case 4: // SETTINGS：无 ACK → 回 ACK
                if (!(fflags & 1)) h2_write_frame(c, 4, 1, 0, NULL, 0);
                break;
            case 6: // PING → ACK
                h2_write_frame(c, 6, 1, 0, payload, plen_out);
                break;
            case 1: { // HEADERS
                H2Header hdrs[32];
                int nh = h2_hpack_decode(payload, plen_out, hdrs, 32);
                if (nh > 0) {
                    const char* method = "GET";
                    const char* path = "/";
                    for (int i = 0; i < nh; i++) {
                        if (strcmp(hdrs[i].name, ":method") == 0) method = hdrs[i].value;
                        else if (strcmp(hdrs[i].name, ":path") == 0) path = hdrs[i].value;
                    }
                    char body[512];
                    int blen = snprintf(body, sizeof(body),
                                        "<h1>PuXian HTTP/2</h1><p>method=%s path=%s</p>\n",
                                        method, path);
                    unsigned char hb[512];
                    int hbl = h2_hpack_enc_response(hb, 200, "text/html; charset=utf-8", blen);
                    h2_write_frame(c, 1, 0x4, fstream, hb, hbl);  // HEADERS END_HEADERS
                    h2_write_frame(c, 0, 0x1, fstream, (const unsigned char*)body, blen); // DATA END_STREAM
                }
                break;
            }
            case 3: // RST_STREAM
            case 8: // WINDOW_UPDATE
                break;
            case 7: // GOAWAY
                if (payload) free(payload);
                return;
            default:
                break;
        }
        if (payload) free(payload);
    }
}
