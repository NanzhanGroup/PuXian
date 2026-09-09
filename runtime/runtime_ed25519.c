// 普贤 (PuXian) M83-S3 ed25519 签名/验签（qg-issue 17 GAP-ED25519-1）
// 实现：RFC 8032 Ed25519 —— tweetnacl-20140427（public domain 参考实现）+ 32B seed 扩展。
// 与 Go crypto/ed25519（RFC 8032，确定性签名）互通：同 seed + 同 msg → 逐字节同签名。
//
// 第三方引入记录（规范：来源/版本/许可/sha256 归档）：
//   源码  https://tweetnacl.cr.yp.to/20140427/tweetnacl.c   （public domain）
//   头    https://tweetnacl.cr.yp.to/20140427/tweetnacl.h
//   上游 sha256（本地仅一处扩展，余下逐字节相同；见 runtime/tweetnacl.c 内注释）：
//     tweetnacl.c pristine 02e65bc3013ff2168983365e55906bc783c4c7e0a60d8100f17bb303a17175c4
//     tweetnacl.h        43f29ad721d9927b747b0100ab4160c119e7bb180c7c98a66e4bf79d31244287
//   本地扩展：tweetnacl.c 中 crypto_sign_keypair 之后新增 crypto_sign_seed_keypair
//     （RFC8032 seed→sk64=seed||pub，对齐官方 NaCl 同名 API 语义；Go PKCS8 导出即 32B seed）。
//
// - ed25519_sign(priv, msg) → sig_hex(128 字符) | null
//     priv：hex 32B seed（64 hex）| hex 64B sk（128 hex，seed||pub）| PEM PKCS8 PRIVATE KEY
//     msg ：str|bytes（二进制安全，含 NUL 不截断）
// - ed25519_verify(pub, msg, sig) → bool
//     pub ：hex 32B（64 hex）| PEM PUBLIC KEY（SPKI）
//     sig ：hex 64B（128 hex）
// - ed25519_keygen() → dict {pk_hex, sk_hex, pk_pem, sk_pem}（M103-S2b，Issue 29
//   GAP-ED25519-2）：tweetnacl randombytes seed32 → crypto_sign_seed_keypair 展开
//   sk64=seed||pub；PEM = PKCS8/SPKI（RFC 8410，与 Go x509 逐字节互通）
#include "runtime.h"
#include "tweetnacl.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// M83-S3 扩展 crypto_sign_seed_keypair 的显式声明（tweetnacl.h 保持上游逐字节不变，不注入；
// C99+ 禁止隐式函数声明 —— gcc 仅警告故 x86_64 主链通过，clang/交叉（zig cc -target *-musl）按
// 错误处理导致 CI 多架构预编译失败；定义见 tweetnacl.c 内「M83-S3 扩展」注释，签名与官方
// crypto_sign_keypair 一致：pk[32] + sk[64]=seed(32)||pub(32)）
extern int crypto_sign_seed_keypair(unsigned char *pk, unsigned char *sk);

// ---- randombytes：tweetnacl 引用（crypto_sign_keypair/crypto_box_keypair 需要，
//      M83-S3 只暴露显式 seed 入口不调用，但整 .o 链接必须有定义）----
void randombytes(unsigned char* x, unsigned long long xlen) {
    static int fd = -2;
    if (fd == -2) fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) { memset(x, 0, (size_t)xlen); return; }
    size_t got = 0;
    while (got < xlen) {
        ssize_t r = read(fd, x + got, (size_t)xlen - got);
        if (r <= 0) { memset(x + got, 0, (size_t)xlen - got); return; }
        got += (size_t)r;
    }
}

// ---- 取 str|bytes 指针与长度（二进制安全；类型错 px_error）----
static const char* e_bytes(LXValue v, int* len) {
    if (v.type == PX_STR || v.type == PX_BYTES) {
        *len = v.as.obj->as.str.len;
        return v.as.obj->as.str.data;
    }
    px_error("期望字符串或 bytes，实际是 %s", px_type_name(v));
    return NULL;
}

// ---- 字节 → 小写 hex（malloc；调用方 free；失败 NULL）----
static char* e_hex(const unsigned char* in, int len) {
    static const char HEX[] = "0123456789abcdef";
    char* out = (char*)malloc((size_t)len * 2 + 1);
    if (!out) return NULL;
    for (int i = 0; i < len; i++) {
        out[i * 2] = HEX[in[i] >> 4];
        out[i * 2 + 1] = HEX[in[i] & 0x0F];
    }
    out[len * 2] = '\0';
    return out;
}

// ---- hex → 字节（out 需 >= len/2；非法返回 -1）----
static int e_unhex(const char* in, int len, unsigned char* out) {
    if (len % 2 != 0) return -1;
    for (int i = 0; i < len; i += 2) {
        int hi = -1, lo = -1;
        char c = in[i];
        if (c >= '0' && c <= '9') hi = c - '0';
        else if (c >= 'a' && c <= 'f') hi = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') hi = c - 'A' + 10;
        c = in[i + 1];
        if (c >= '0' && c <= '9') lo = c - '0';
        else if (c >= 'a' && c <= 'f') lo = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') lo = c - 'A' + 10;
        if (hi < 0 || lo < 0) return -1;
        out[i / 2] = (unsigned char)((hi << 4) | lo);
    }
    return len / 2;
}

// ---- base64 解码（标准表，容忍换行/空白与尾部 '='；len 语义）----
static int e_b64dec(const char* in, int inlen, unsigned char* out, int outcap) {
    int acc = 0, nbits = 0, n = 0;
    for (int i = 0; i < inlen; i++) {
        unsigned char c = (unsigned char)in[i];
        int v = -1;
        if (c >= 'A' && c <= 'Z') v = c - 'A';
        else if (c >= 'a' && c <= 'z') v = c - 'a' + 26;
        else if (c >= '0' && c <= '9') v = c - '0' + 52;
        else if (c == '+') v = 62;
        else if (c == '/') v = 63;
        else if (c == '=' || c == '\n' || c == '\r' || c == ' ' || c == '\t') continue;
        if (v < 0) return -1;
        acc = (acc << 6) | v;
        nbits += 6;
        if (nbits >= 8) {
            nbits -= 8;
            if (n >= outcap) return -1;
            out[n++] = (unsigned char)((acc >> nbits) & 0xFF);
        }
    }
    return n;
}

// ---- 小型 DER TLV 走查（ed25519 PKCS8/SPKI 结构固定，短/长格式长度均可）----
typedef struct { unsigned char tag; const unsigned char* p; const unsigned char* end; } DerT;
// 解析 d 处一个 TLV（out 内容指针/end）；返回整 TLV 头长度（tag+len），失败 -1
static int der_tlv(const unsigned char* d, const unsigned char* end, DerT* out) {
    if (d + 2 > end) return -1;
    out->tag = d[0];
    const unsigned char* q = d + 1;
    size_t len = 0;
    if (*q < 0x80) { len = *q++; }
    else {
        int nb = *q & 0x7F; q++;
        if (nb == 0 || nb > 4 || q + (size_t)nb > end) return -1;
        for (int i = 0; i < nb; i++) len = (len << 8) | *q++;
    }
    if ((size_t)(end - q) < len) return -1;
    out->p = q;
    out->end = q + len;
    return (int)(q - d);
}
// 跳过 d 处一个 TLV，返回其内容之后位置（即下一 TLV 起点；失败 NULL）
static const unsigned char* der_skip(const unsigned char* d, const unsigned char* end) {
    DerT t;
    if (der_tlv(d, end, &t) < 0) return NULL;
    return t.end;
}
// OID 是否为 1.3.101.112（ed25519）：DER 内容 = 2b 65 70
static int der_oid_is_ed25519(const DerT* oid) {
    return oid->tag == 0x06 && oid->end - oid->p == 3 &&
           oid->p[0] == 0x2B && oid->p[1] == 0x65 && oid->p[2] == 0x70;
}

// ---- PEM 提取 + base64 解码成 DER（label 含 PRIVATE 判私钥 / PUBLIC 判公钥）----
// 返回 malloc DER；*is_priv=1/0；失败 NULL
static unsigned char* e_pem_der(const char* pem, int plen, int* derlen, int* is_priv) {
    const char* b = pem;
    const char* end = pem + plen;
    while (b + 11 <= end && memcmp(b, "-----BEGIN ", 11) != 0) b++;
    if (b + 11 > end) return NULL;
    const char* eol = b;
    while (eol < end && *eol != '\n' && *eol != '\r') eol++;
    const char* label = b + 11;
    int llen = (int)(eol - label);
    int priv = 0, pub = 0;
    for (int i = 0; i + 6 <= llen; i++) if (memcmp(label + i, "PRIVATE", 7) == 0) priv = 1;
    for (int i = 0; i + 5 <= llen; i++) if (memcmp(label + i, "PUBLIC", 6) == 0) pub = 1;
    if (!priv && !pub) return NULL;
    const char* body = eol;
    if (body < end && *body == '\r') body++;
    if (body < end && *body == '\n') body++;
    const char* en = body;
    while (en + 8 <= end && memcmp(en, "-----END", 8) != 0) en++;
    if (en + 8 > end) return NULL;
    int cap = (int)((size_t)(en - body) * 3 / 4 + 4);
    unsigned char* der = (unsigned char*)malloc((size_t)cap);
    if (!der) return NULL;
    int dl = e_b64dec(body, (int)(en - body), der, cap);
    if (dl <= 0) { free(der); return NULL; }
    *derlen = dl;
    *is_priv = priv;
    return der;
}

// ---- 从 DER 取密钥材料：priv → sk64（seed 自动展开，附 pk）；pub → pk32 ----
// is_priv=1：out_sk 填 64B、out_pk 填 32B；is_priv=0：out_pk 填 32B。成功 0，失败 -1
static int e_der_key(const unsigned char* der, int dlen, int is_priv,
                     unsigned char* out_sk, unsigned char* out_pk) {
    const unsigned char* end = der + dlen;
    DerT outer, alg, oid, key;
    const unsigned char* c;
    if (der_tlv(der, end, &outer) < 0 || outer.tag != 0x30) return -1;
    c = outer.p;
    if (is_priv) {
        c = der_skip(c, outer.end);                       // 跳过 version INTEGER
        if (!c || der_tlv(c, outer.end, &alg) < 0 || alg.tag != 0x30) return -1;
        if (der_tlv(alg.p, alg.end, &oid) < 0 || !der_oid_is_ed25519(&oid)) return -1;
        const unsigned char* k = der_skip(c, outer.end);  // 跳过整个 algid → OCTET STRING
        if (!k || der_tlv(k, outer.end, &key) < 0 || key.tag != 0x04) return -1;
        const unsigned char* kp = key.p;
        size_t kl = (size_t)(key.end - kp);
        if (kl == 34 && kp[0] == 0x04 && kp[1] == 0x20) { kp += 2; kl -= 2; }  // RFC8410 嵌套
        if (kl == 32) {                                  // seed → 展开 sk64
            unsigned char sk[64];
            memcpy(sk, kp, 32);
            if (crypto_sign_seed_keypair(out_pk, sk) != 0) return -1;
            memcpy(out_sk, sk, 64);
            return 0;
        }
        if (kl == 64) { memcpy(out_sk, kp, 64); memcpy(out_pk, kp + 32, 32); return 0; }
        return -1;
    }
    if (der_tlv(c, outer.end, &alg) < 0 || alg.tag != 0x30) return -1;
    if (der_tlv(alg.p, alg.end, &oid) < 0 || !der_oid_is_ed25519(&oid)) return -1;
    const unsigned char* bs = der_skip(c, outer.end);     // 跳过整个 algid → BIT STRING
    if (!bs || der_tlv(bs, outer.end, &key) < 0 || key.tag != 0x03) return -1;
    if (key.end - key.p == 33 && key.p[0] == 0x00) { memcpy(out_pk, key.p + 1, 32); return 0; }
    if (key.end - key.p == 32) { memcpy(out_pk, key.p, 32); return 0; }
    return -1;
}

// ---- 解析私钥（hex seed32 / hex sk64 / PEM PKCS8）→ 64B sk + 32B pk；成功 0 ----
static int e_parse_priv(const char* s, int slen, unsigned char* sk64, unsigned char* pk) {
    unsigned char buf[128];
    if (slen == 64 || slen == 128) {
        int bl = e_unhex(s, slen, buf);
        if (bl == 32) { memcpy(sk64, buf, 32); return crypto_sign_seed_keypair(pk, sk64); }
        if (bl == 64) { memcpy(sk64, buf, 64); memcpy(pk, buf + 32, 32); return 0; }
    }
    int dlen = 0, is_priv = 0;
    unsigned char* der = e_pem_der(s, slen, &dlen, &is_priv);
    if (!der || !is_priv) { free(der); return -1; }
    int rc = e_der_key(der, dlen, 1, sk64, pk);
    free(der);
    return rc;
}

// ---- 解析公钥（hex 32B / PEM SPKI）→ 32B pk；成功 0 ----
static int e_parse_pub(const char* s, int slen, unsigned char* pk) {
    unsigned char buf[64];
    if (slen == 64) {
        int bl = e_unhex(s, slen, buf);
        if (bl == 32) { memcpy(pk, buf, 32); return 0; }
    }
    int dlen = 0, is_priv = 0;
    unsigned char* der = e_pem_der(s, slen, &dlen, &is_priv);
    if (!der || is_priv) { free(der); return -1; }
    unsigned char sk[64];
    int rc = e_der_key(der, dlen, 0, sk, pk);
    free(der);
    return rc;
}

// ed25519_sign(priv, msg) → sig_hex(128) | null（RFC8032 确定性签名）
LXValue bi_ed25519_sign(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("ed25519_sign 需要 2 个参数: (priv, msg)");
    int plen = 0, mlen = 0;
    const char* priv = e_bytes(args[0], &plen);
    const char* msg = e_bytes(args[1], &mlen);
    unsigned char sk[64], pk[32];
    if (e_parse_priv(priv, plen, sk, pk) != 0) return px_null();  // 内容非法 → null
    unsigned char* sm = (unsigned char*)malloc((size_t)mlen + 64);
    if (!sm) return px_null();
    unsigned long long smlen = 0;
    int rc = crypto_sign(sm, &smlen, (const unsigned char*)msg, (unsigned long long)mlen, sk);
    if (rc != 0 || smlen != (unsigned long long)mlen + 64) { free(sm); return px_null(); }
    char* hex = e_hex(sm, 64);
    free(sm);
    LXValue v = hex ? px_str(hex) : px_null();
    free(hex);
    return v;
}

// ed25519_verify(pub, msg, sig) → bool
LXValue bi_ed25519_verify(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("ed25519_verify 需要 3 个参数: (pub, msg, sig)");
    int plen = 0, mlen = 0, slen = 0;
    const char* pub = e_bytes(args[0], &plen);
    const char* msg = e_bytes(args[1], &mlen);
    const char* sig = e_bytes(args[2], &slen);
    unsigned char pk[32];
    if (e_parse_pub(pub, plen, pk) != 0) return px_bool(false);
    unsigned char sb[64];
    if (slen != 128 || e_unhex(sig, slen, sb) != 64) return px_bool(false);
    // sm = sig(64) || msg；crypto_sign_open 需 m 缓冲 >= n 字节作 scratch
    unsigned long long n = (unsigned long long)mlen + 64;
    unsigned char* sm = (unsigned char*)malloc((size_t)n);
    unsigned char* m = (unsigned char*)malloc((size_t)n);
    if (!sm || !m) { free(sm); free(m); return px_bool(false); }
    memcpy(sm, sb, 64);
    memcpy(sm + 64, msg, (size_t)mlen);
    unsigned long long mlen_out = 0;
    int ok = crypto_sign_open(m, &mlen_out, sm, n, pk) == 0 &&
             mlen_out == (unsigned long long)mlen;
    free(sm);
    free(m);
    return px_bool(ok);
}

// ---- M103-S2b（Issue 29 GAP-ED25519-2）：ed25519_keygen 密钥对生成 ----
// ws-ddns local 端 -genkey 无 native（Issue 17 仅 sign/verify 的延伸）。tweetnacl
// randombytes + crypto_sign_seed_keypair 现成：seed32 随机 → pk32 + sk64=seed||pk。
// 输出与 Go crypto/ed25519 + x509（RFC 8410）逐字节互通：
//   PKCS8 DER = 30 2E 02 01 00 30 05 06 03 2B 65 70 04 22 04 20 <seed32>（嵌套 OCTET，
//   Go x509.MarshalPKCS8PrivateKey 同构；现有 e_der_key 剥 kl==34 嵌套路径可回读）
//   SPKI  DER = 30 2A 30 05 06 03 2B 65 70 03 21 00 <pk32>（RFC 8410 / Go PKIX 同构，
//   总长 44B = 2B 头(30 2A) + 内容 42B）
// ed25519_keygen() → dict {pk_hex(64), sk_hex(128=seed||pub), pk_pem(SPKI), sk_pem(PKCS8)}

// ---- base64 编码（标准表；malloc，调用方 free）----
static char* e_b64enc(const unsigned char* in, int len) {
    static const char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int cap = ((len + 2) / 3) * 4 + 1;
    char* out = (char*)malloc((size_t)cap);
    if (!out) return NULL;
    int o = 0;
    for (int i = 0; i < len; i += 3) {
        int n = len - i;
        int b0 = in[i], b1 = n > 1 ? in[i + 1] : 0, b2 = n > 2 ? in[i + 2] : 0;
        unsigned v = ((unsigned)b0 << 16) | ((unsigned)b1 << 8) | (unsigned)b2;
        out[o++] = B64[(v >> 18) & 63];
        out[o++] = B64[(v >> 12) & 63];
        out[o++] = n > 1 ? B64[(v >> 6) & 63] : '=';
        out[o++] = n > 2 ? B64[v & 63] : '=';
    }
    out[o] = '\0';
    return out;
}

// ---- DER → PEM（体每 64 字符换行；malloc，调用方 free）----
static char* e_pem(const char* label, const unsigned char* der, int len) {
    char head[96], tail[64];
    snprintf(head, sizeof(head), "-----BEGIN %s-----\n", label);
    snprintf(tail, sizeof(tail), "-----END %s-----\n", label);
    char* b64 = e_b64enc(der, len);
    if (!b64) return NULL;
    int blen = (int)strlen(b64);
    int nlines = (blen + 63) / 64;
    char* out = (char*)malloc(strlen(head) + (size_t)blen + (size_t)nlines + strlen(tail) + 1);
    if (!out) { free(b64); return NULL; }
    char* p = out;
    strcpy(p, head);
    p += strlen(head);
    for (int i = 0; i < blen; i += 64) {
        int c = blen - i;
        if (c > 64) c = 64;
        memcpy(p, b64 + i, (size_t)c);
        p += c;
        *p++ = '\n';
    }
    strcpy(p, tail);
    free(b64);
    return out;
}

// ed25519_keygen() → dict {pk_hex, sk_hex, pk_pem, sk_pem} | null（熵源失败）
LXValue bi_ed25519_keygen(LXValue* args, int nargs, void* ctx) {
    (void)args; (void)ctx;
    if (nargs != 0) px_error("ed25519_keygen 不需要参数");
    unsigned char pk[32], sk[64];
    randombytes(sk, 32);                              // 32B seed
    if (crypto_sign_seed_keypair(pk, sk) != 0) return px_null();
    // sk[0..31]=seed, sk[32..63]=pk
    unsigned char p8[48];                             // PKCS8（RFC8410 嵌套，对齐 Go x509）
    p8[0] = 0x30; p8[1] = 0x2E;
    p8[2] = 0x02; p8[3] = 0x01; p8[4] = 0x00;
    p8[5] = 0x30; p8[6] = 0x05; p8[7] = 0x06; p8[8] = 0x03; p8[9] = 0x2B; p8[10] = 0x65; p8[11] = 0x70;
    p8[12] = 0x04; p8[13] = 0x22; p8[14] = 0x04; p8[15] = 0x20;
    memcpy(p8 + 16, sk, 32);
    unsigned char spki[44];                           // SPKI（RFC 8410，对齐 Go PKIX；总 44B）
    spki[0] = 0x30; spki[1] = 0x2A;
    spki[2] = 0x30; spki[3] = 0x05; spki[4] = 0x06; spki[5] = 0x03; spki[6] = 0x2B; spki[7] = 0x65; spki[8] = 0x70;
    spki[9] = 0x03; spki[10] = 0x21; spki[11] = 0x00;
    memcpy(spki + 12, pk, 32);
    char* pk_hex = e_hex(pk, 32);
    char* sk_hex = e_hex(sk, 64);
    char* p8_pem = e_pem("PRIVATE KEY", p8, 48);
    char* spki_pem = e_pem("PUBLIC KEY", spki, 44);
    if (!pk_hex || !sk_hex || !p8_pem || !spki_pem) {
        free(pk_hex); free(sk_hex); free(p8_pem); free(spki_pem);
        return px_null();
    }
    LXValue d = px_dict();
    px_root_push();
    PX_KEEP(d);                                      // M92 precise：dict 跨 px_str 分配
    px_dict_set(d, "pk_hex", px_str(pk_hex));
    px_dict_set(d, "sk_hex", px_str(sk_hex));
    px_dict_set(d, "pk_pem", px_str(spki_pem));
    px_dict_set(d, "sk_pem", px_str(p8_pem));
    px_root_pop();
    free(pk_hex); free(sk_hex); free(p8_pem); free(spki_pem);
    return d;
}
