// 普贤 (PuXian) M19 AES 内置函数（编译模式，mbedtls 实现，与解释器 Rust 输出逐字节一致）
// - aes_encrypt(data, key, iv) → hex（AES-CBC-PKCS7；key 16/24/32 字节 → 128/192/256 位）
// - aes_decrypt(hex, key, iv) → str 或 null（padding 非法 / 非 UTF-8 → null）
// - aes_gcm_encrypt(data, key, iv) → hex（密文 + 16 字节 tag）
// - aes_gcm_decrypt(hex, key, iv) → str 或 null（tag 校验失败 → null）
#include "runtime.h"
#include "mbedtls/aes.h"
#include "mbedtls/gcm.h"
#include <string.h>
#include <stdlib.h>

// 取字符串字节与长度
static const char* vstr(LXValue v) {
    if (v.type != PX_STR) px_error("期望字符串，实际是 %s", px_type_name(v));
    return v.as.obj->as.str.data;
}
static int vstrlen(LXValue v) {
    if (v.type != PX_STR) px_error("期望字符串，实际是 %s", px_type_name(v));
    return v.as.obj->as.str.len;
}

// 字节 → 小写 hex（out 需 >= len*2+1）
static void aes_hex(const unsigned char* in, int len, char* out) {
    static const char HEX[] = "0123456789abcdef";
    for (int i = 0; i < len; i++) {
        out[i * 2] = HEX[in[i] >> 4];
        out[i * 2 + 1] = HEX[in[i] & 0x0F];
    }
    out[len * 2] = '\0';
}

// hex → 字节（out 需 >= len/2；非法返回 -1）
static int aes_unhex(const char* in, int len, unsigned char* out) {
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

// 严格 UTF-8 校验（对齐 Rust String::from_utf8：拒绝 overlong / surrogate / 超范围）
static int aes_is_utf8(const unsigned char* s, int len) {
    int i = 0;
    while (i < len) {
        unsigned char c = s[i];
        if (c < 0x80) { i++; }
        else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= len || (s[i + 1] & 0xC0) != 0x80 || c < 0xC2) return 0;
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= len || (s[i + 1] & 0xC0) != 0x80 || (s[i + 2] & 0xC0) != 0x80) return 0;
            unsigned int cp = ((c & 0x0F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F);
            if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) return 0;
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= len || (s[i + 1] & 0xC0) != 0x80 || (s[i + 2] & 0xC0) != 0x80 || (s[i + 3] & 0xC0) != 0x80) return 0;
            unsigned int cp = ((c & 0x07) << 18) | ((s[i + 1] & 0x3F) << 12) | ((s[i + 2] & 0x3F) << 6) | (s[i + 3] & 0x3F);
            if (cp < 0x10000 || cp > 0x10FFFF) return 0;
            i += 4;
        } else {
            return 0;
        }
    }
    return 1;
}

// aes_encrypt(data, key, iv) → hex（AES-CBC-PKCS7）
LXValue bi_aes_encrypt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("aes_encrypt 需要 3 个参数: (data, key, iv)");
    const char* data = vstr(args[0]); int dlen = vstrlen(args[0]);
    const char* key = vstr(args[1]); int klen = vstrlen(args[1]);
    const char* iv = vstr(args[2]); int ivlen = vstrlen(args[2]);
    if (klen != 16 && klen != 24 && klen != 32) px_error("AES 密钥长度须为 16/24/32 字节（128/192/256 位），实际 %d", klen);
    if (ivlen != 16) px_error("CBC 模式 IV 必须 16 字节，实际 %d", ivlen);
    int pad = 16 - (dlen % 16);
    int buflen = dlen + pad;
    unsigned char* buf = (unsigned char*)malloc(buflen);
    memcpy(buf, data, dlen);
    memset(buf + dlen, pad, pad);
    unsigned char* out = (unsigned char*)malloc(buflen);
    unsigned char ivcopy[16];
    memcpy(ivcopy, iv, 16);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_enc(&aes, (const unsigned char*)key, klen * 8) != 0) {
        mbedtls_aes_free(&aes); free(buf); free(out);
        px_error("AES 密钥设置失败");
    }
    int rc = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, buflen, ivcopy, buf, out);
    mbedtls_aes_free(&aes);
    free(buf);
    if (rc != 0) { free(out); px_error("AES 加密失败"); }
    char* hex = (char*)malloc(buflen * 2 + 1);
    aes_hex(out, buflen, hex);
    LXValue r = px_str(hex);
    free(out); free(hex);
    return r;
}

// aes_decrypt(hex, key, iv) → str 或 null
LXValue bi_aes_decrypt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("aes_decrypt 需要 3 个参数: (hex, key, iv)");
    const char* hs = vstr(args[0]); int hlen = vstrlen(args[0]);
    const char* key = vstr(args[1]); int klen = vstrlen(args[1]);
    const char* iv = vstr(args[2]); int ivlen = vstrlen(args[2]);
    if (klen != 16 && klen != 24 && klen != 32) px_error("AES 密钥长度须为 16/24/32 字节（128/192/256 位），实际 %d", klen);
    if (ivlen != 16) px_error("CBC 模式 IV 必须 16 字节，实际 %d", ivlen);
    unsigned char* ct = (unsigned char*)malloc(hlen / 2 + 1);
    int ctlen = aes_unhex(hs, hlen, ct);
    if (ctlen < 0 || ctlen == 0 || ctlen % 16 != 0) { free(ct); return px_null(); }
    unsigned char* out = (unsigned char*)malloc(ctlen);
    unsigned char ivcopy[16];
    memcpy(ivcopy, iv, 16);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_dec(&aes, (const unsigned char*)key, klen * 8) != 0) {
        mbedtls_aes_free(&aes); free(ct); free(out);
        px_error("AES 密钥设置失败");
    }
    int rc = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ctlen, ivcopy, ct, out);
    mbedtls_aes_free(&aes);
    free(ct);
    if (rc != 0) { free(out); px_error("AES 解密失败"); }
    // PKCS7 校验
    int pad = out[ctlen - 1];
    if (pad == 0 || pad > 16) { free(out); return px_null(); }
    for (int i = 0; i < pad; i++) {
        if (out[ctlen - 1 - i] != pad) { free(out); return px_null(); }
    }
    int plen = ctlen - pad;
    if (!aes_is_utf8(out, plen)) { free(out); return px_null(); }
    LXValue r = px_str_len((const char*)out, plen);
    free(out);
    return r;
}

// aes_gcm_encrypt(data, key, iv) → hex（密文 + 16 字节 tag）
LXValue bi_aes_gcm_encrypt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("aes_gcm_encrypt 需要 3 个参数: (data, key, iv)");
    const char* data = vstr(args[0]); int dlen = vstrlen(args[0]);
    const char* key = vstr(args[1]); int klen = vstrlen(args[1]);
    const char* iv = vstr(args[2]); int ivlen = vstrlen(args[2]);
    if (klen != 16 && klen != 24 && klen != 32) px_error("AES 密钥长度须为 16/24/32 字节（128/192/256 位），实际 %d", klen);
    if (ivlen <= 0) px_error("GCM 模式 IV 不能为空");
    unsigned char* ct = (unsigned char*)malloc(dlen + 1);
    unsigned char tag[16];
    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, (const unsigned char*)key, klen * 8) != 0) {
        mbedtls_gcm_free(&gcm); free(ct);
        px_error("AES 密钥设置失败");
    }
    int rc = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, dlen,
                                       (const unsigned char*)iv, ivlen, NULL, 0,
                                       (const unsigned char*)data, ct, 16, tag);
    mbedtls_gcm_free(&gcm);
    if (rc != 0) { free(ct); px_error("AES GCM 加密失败"); }
    char* hex = (char*)malloc((dlen + 16) * 2 + 1);
    aes_hex(ct, dlen, hex);
    aes_hex(tag, 16, hex + dlen * 2);
    LXValue r = px_str(hex);
    free(ct); free(hex);
    return r;
}

// aes_gcm_decrypt(hex, key, iv) → str 或 null（tag 校验失败 → null）
LXValue bi_aes_gcm_decrypt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("aes_gcm_decrypt 需要 3 个参数: (hex, key, iv)");
    const char* hs = vstr(args[0]); int hlen = vstrlen(args[0]);
    const char* key = vstr(args[1]); int klen = vstrlen(args[1]);
    const char* iv = vstr(args[2]); int ivlen = vstrlen(args[2]);
    if (klen != 16 && klen != 24 && klen != 32) px_error("AES 密钥长度须为 16/24/32 字节（128/192/256 位），实际 %d", klen);
    if (ivlen <= 0) px_error("GCM 模式 IV 不能为空");
    unsigned char* all = (unsigned char*)malloc(hlen / 2 + 1);
    int alllen = aes_unhex(hs, hlen, all);
    if (alllen < 17) { free(all); return px_null(); }
    int ctlen = alllen - 16;
    unsigned char* out = (unsigned char*)malloc(ctlen + 1);
    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, (const unsigned char*)key, klen * 8) != 0) {
        mbedtls_gcm_free(&gcm); free(all); free(out);
        px_error("AES 密钥设置失败");
    }
    // mbedtls 3.x：auth_decrypt 签名为 (ctx, len, iv, iv_len, add, add_len, tag, tag_len, input, output)
    int rc = mbedtls_gcm_auth_decrypt(&gcm, ctlen,
                                      (const unsigned char*)iv, ivlen, NULL, 0,
                                      all + ctlen, 16, all, out);
    mbedtls_gcm_free(&gcm);
    free(all);
    if (rc != 0) { free(out); return px_null(); }  // tag 校验失败
    if (!aes_is_utf8(out, ctlen)) { free(out); return px_null(); }
    LXValue r = px_str_len((const char*)out, ctlen);
    free(out);
    return r;
}
