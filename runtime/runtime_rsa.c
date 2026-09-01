// 普贤 (PuXian) M23d RSA 内置函数（编译模式，mbedtls 实现，与解释器 Rust 输出逐字节一致）
// - rsa_gen_key(bits) → dict {n,e,d,p,q}（hex；bits 512..4096）
// - rsa_encrypt(data, n, e) → hex 密文 | null（PKCS#1 v1.5 type 2；数据 ≤ 模长-11 字节）
// - rsa_decrypt(ct_hex, n, d) → 明文 str | null（type 2 解码，含 padding 校验）
// - rsa_sign(data, n, d) → hex 签名 | null（type 1，直接签数据不包 DigestInfo）
// - rsa_verify(data, sig_hex, n, e) → bool
#include "runtime.h"
#include "mbedtls/rsa.h"
#include "mbedtls/bignum.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// ---- RNG（/dev/urandom；密钥生成 / 加密 padding / 私钥盲化需要） ----
static int px_rng(void* p, unsigned char* out, size_t len) {
    (void)p;
    static int fd = -2;
    if (fd == -2) fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t got = 0;
    while (got < len) {
        ssize_t r = read(fd, out + got, len - got);
        if (r <= 0) return -1;
        got += (size_t)r;
    }
    return 0;
}

// 取字符串字节与长度（static 独立于 runtime_aes.c 的同名辅助）
static const char* rsa_str(LXValue v) {
    if (v.type != PX_STR) px_error("期望字符串，实际是 %s", px_type_name(v));
    return v.as.obj->as.str.data;
}
static int rsa_strlen(LXValue v) {
    if (v.type != PX_STR) px_error("期望字符串，实际是 %s", px_type_name(v));
    return v.as.obj->as.str.len;
}

// 字节 → 小写 hex（malloc；调用方 free）
static char* rsa_hex(const unsigned char* in, int len) {
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

// hex → 字节（malloc + 长度；非法返回 NULL）
static unsigned char* rsa_unhex(const char* in, int len, int* outlen) {
    if (len % 2 != 0) return NULL;
    unsigned char* out = (unsigned char*)malloc((size_t)len / 2);
    if (!out) return NULL;
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
        if (hi < 0 || lo < 0) { free(out); return NULL; }
        out[i / 2] = (unsigned char)((hi << 4) | lo);
    }
    *outlen = len / 2;
    return out;
}

// hex 字符串 → mbedtls_mpi（支持 0x 前缀；失败返回非 0）
static int rsa_mpi_from_hex(mbedtls_mpi* m, const char* hex) {
    const char* s = hex;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
    mbedtls_mpi_init(m);
    return mbedtls_mpi_read_string(m, 16, s);
}

// mbedtls_mpi → hex 字符串（malloc；调用方 free；失败 NULL）
static char* rsa_mpi_to_hex(const mbedtls_mpi* m) {
    char buf[16384];
    size_t olen = 0;
    if (mbedtls_mpi_write_string(m, 16, buf, sizeof(buf), &olen) != 0) return NULL;
    char* s = (char*)malloc(olen + 1);
    if (!s) return NULL;
    memcpy(s, buf, olen + 1);
    return s;
}

static void rsa_free_mpis(mbedtls_mpi* N, mbedtls_mpi* P, mbedtls_mpi* Q, mbedtls_mpi* D, mbedtls_mpi* E) {
    if (N) mbedtls_mpi_free(N);
    if (P) mbedtls_mpi_free(P);
    if (Q) mbedtls_mpi_free(Q);
    if (D) mbedtls_mpi_free(D);
    if (E) mbedtls_mpi_free(E);
}

// ---- rsa_gen_key(bits) → dict{n,e,d,p,q} ----
LXValue bi_rsa_gen_key(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("rsa_gen_key 需要 (bits) 参数");
    int bits = (int)args[0].as.i;
    if (bits < 512 || bits > 4096) px_error("rsa_gen_key 的 bits 必须在 512..4096");
    mbedtls_rsa_context rsa;
    mbedtls_rsa_init(&rsa);
    if (mbedtls_rsa_gen_key(&rsa, px_rng, NULL, (unsigned int)bits, 65537) != 0) {
        mbedtls_rsa_free(&rsa);
        return px_null();
    }
    mbedtls_mpi N, P, Q, D, E;
    mbedtls_mpi_init(&N); mbedtls_mpi_init(&P); mbedtls_mpi_init(&Q);
    mbedtls_mpi_init(&D); mbedtls_mpi_init(&E);
    int rc = mbedtls_rsa_export(&rsa, &N, &P, &Q, &D, &E);
    mbedtls_rsa_free(&rsa);
    if (rc != 0) {
        rsa_free_mpis(&N, &P, &Q, &D, &E);
        return px_null();
    }
    char* hn = rsa_mpi_to_hex(&N);
    char* he = rsa_mpi_to_hex(&E);
    char* hd = rsa_mpi_to_hex(&D);
    char* hp = rsa_mpi_to_hex(&P);
    char* hq = rsa_mpi_to_hex(&Q);
    rsa_free_mpis(&N, &P, &Q, &D, &E);
    LXValue d = px_dict();
    px_dict_set(d, "n", px_str(hn ? hn : "0"));
    px_dict_set(d, "e", px_str(he ? he : "0"));
    px_dict_set(d, "d", px_str(hd ? hd : "0"));
    px_dict_set(d, "p", px_str(hp ? hp : "0"));
    px_dict_set(d, "q", px_str(hq ? hq : "0"));
    free(hn); free(he); free(hd); free(hp); free(hq);
    return d;
}

// 构建公钥上下文（N,E）→ 0 成功；调用方负责 free
static int rsa_pub_ctx(mbedtls_rsa_context* rsa, const char* n_hex, const char* e_hex) {
    mbedtls_rsa_init(rsa);
    mbedtls_mpi N, E;
    mbedtls_mpi_init(&N); mbedtls_mpi_init(&E);
    if (rsa_mpi_from_hex(&N, n_hex) != 0 || rsa_mpi_from_hex(&E, e_hex) != 0) {
        rsa_free_mpis(&N, NULL, NULL, NULL, &E);
        return -1;
    }
    int rc = mbedtls_rsa_import(rsa, &N, NULL, NULL, NULL, &E);
    rsa_free_mpis(&N, NULL, NULL, NULL, &E);
    if (rc != 0) return -1;
    if (mbedtls_rsa_complete(rsa) != 0) return -1;
    if (mbedtls_rsa_check_pubkey(rsa) != 0) return -1;
    return 0;
}

// 构建私钥上下文（N,P,Q,D,E）→ 0 成功；调用方负责 free
static int rsa_priv_ctx(mbedtls_rsa_context* rsa, const char* n_hex, const char* d_hex) {
    mbedtls_rsa_init(rsa);
    mbedtls_mpi N, P, Q, D, E;
    mbedtls_mpi_init(&N); mbedtls_mpi_init(&P); mbedtls_mpi_init(&Q);
    mbedtls_mpi_init(&D); mbedtls_mpi_init(&E);
    if (rsa_mpi_from_hex(&N, n_hex) != 0 || rsa_mpi_from_hex(&D, d_hex) != 0 ||
        rsa_mpi_from_hex(&E, "10001") != 0) {
        rsa_free_mpis(&N, &P, &Q, &D, &E);
        return -1;
    }
    int rc = mbedtls_rsa_import(rsa, &N, &P, &Q, &D, &E);
    rsa_free_mpis(&N, &P, &Q, &D, &E);
    if (rc != 0) return -1;
    if (mbedtls_rsa_complete(rsa) != 0) return -1;
    if (mbedtls_rsa_check_privkey(rsa) != 0) return -1;
    return 0;
}

// ---- rsa_encrypt(data, n, e) → hex 密文 | null ----
LXValue bi_rsa_encrypt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("rsa_encrypt 需要 (data, n_hex, e_hex) 参数");
    const char* data = rsa_str(args[0]);
    int dlen = rsa_strlen(args[0]);
    const char* n_hex = rsa_str(args[1]);
    const char* e_hex = rsa_str(args[2]);
    mbedtls_rsa_context rsa;
    if (rsa_pub_ctx(&rsa, n_hex, e_hex) != 0) return px_null();
    size_t k = mbedtls_rsa_get_len(&rsa);
    if ((size_t)dlen > k - 11) { mbedtls_rsa_free(&rsa); return px_null(); }
    unsigned char* out = (unsigned char*)malloc(k);
    if (!out) { mbedtls_rsa_free(&rsa); return px_null(); }
    int rc = mbedtls_rsa_pkcs1_encrypt(&rsa, px_rng, NULL, (size_t)dlen,
                                       (const unsigned char*)data, out);
    mbedtls_rsa_free(&rsa);
    if (rc != 0) { free(out); return px_null(); }
    char* hex = rsa_hex(out, (int)k);
    free(out);
    LXValue v = hex ? px_str(hex) : px_null();
    free(hex);
    return v;
}

// ---- rsa_decrypt(ct_hex, n, d) → 明文 | null ----
LXValue bi_rsa_decrypt(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("rsa_decrypt 需要 (ct_hex, n_hex, d_hex) 参数");
    const char* ct_hex = rsa_str(args[0]);
    int ctlen = rsa_strlen(args[0]);
    const char* n_hex = rsa_str(args[1]);
    const char* d_hex = rsa_str(args[2]);
    int cblen = 0;
    unsigned char* cb = rsa_unhex(ct_hex, ctlen, &cblen);
    if (!cb) return px_null();
    mbedtls_rsa_context rsa;
    if (rsa_priv_ctx(&rsa, n_hex, d_hex) != 0) { free(cb); return px_null(); }
    size_t k = mbedtls_rsa_get_len(&rsa);
    if ((size_t)cblen != k) { mbedtls_rsa_free(&rsa); free(cb); return px_null(); }
    unsigned char* out = (unsigned char*)malloc(k);
    if (!out) { mbedtls_rsa_free(&rsa); free(cb); return px_null(); }
    size_t olen = k;
    int rc = mbedtls_rsa_pkcs1_decrypt(&rsa, px_rng, NULL, &olen,
                                       cb, out, k);
    mbedtls_rsa_free(&rsa);
    free(cb);
    if (rc != 0) { free(out); return px_null(); }
    LXValue v = px_str_len((const char*)out, (int)olen);
    free(out);
    return v;
}

// ---- rsa_sign(data, n, d) → hex 签名 | null ----
LXValue bi_rsa_sign(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 3) px_error("rsa_sign 需要 (data, n_hex, d_hex) 参数");
    const char* data = rsa_str(args[0]);
    int dlen = rsa_strlen(args[0]);
    const char* n_hex = rsa_str(args[1]);
    const char* d_hex = rsa_str(args[2]);
    mbedtls_rsa_context rsa;
    if (rsa_priv_ctx(&rsa, n_hex, d_hex) != 0) return px_null();
    size_t k = mbedtls_rsa_get_len(&rsa);
    if ((size_t)dlen > k - 11) { mbedtls_rsa_free(&rsa); return px_null(); }
    unsigned char* out = (unsigned char*)malloc(k);
    if (!out) { mbedtls_rsa_free(&rsa); return px_null(); }
    int rc = mbedtls_rsa_pkcs1_sign(&rsa, px_rng, NULL, MBEDTLS_MD_NONE,
                                    (unsigned int)dlen, (const unsigned char*)data, out);
    mbedtls_rsa_free(&rsa);
    if (rc != 0) { free(out); return px_null(); }
    char* hex = rsa_hex(out, (int)k);
    free(out);
    LXValue v = hex ? px_str(hex) : px_null();
    free(hex);
    return v;
}

// ---- rsa_verify(data, sig_hex, n, e) → bool ----
LXValue bi_rsa_verify(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 4) px_error("rsa_verify 需要 (data, sig_hex, n_hex, e_hex) 参数");
    const char* data = rsa_str(args[0]);
    int dlen = rsa_strlen(args[0]);
    const char* sig_hex = rsa_str(args[1]);
    int siglen = rsa_strlen(args[1]);
    const char* n_hex = rsa_str(args[2]);
    const char* e_hex = rsa_str(args[3]);
    int sblen = 0;
    unsigned char* sb = rsa_unhex(sig_hex, siglen, &sblen);
    if (!sb) return px_bool(false);
    mbedtls_rsa_context rsa;
    if (rsa_pub_ctx(&rsa, n_hex, e_hex) != 0) { free(sb); return px_bool(false); }
    size_t k = mbedtls_rsa_get_len(&rsa);
    bool ok = false;
    if ((size_t)sblen == k) {
        ok = mbedtls_rsa_pkcs1_verify(&rsa, MBEDTLS_MD_NONE,
                                      (unsigned int)dlen, (const unsigned char*)data, sb) == 0;
    }
    mbedtls_rsa_free(&rsa);
    free(sb);
    return px_bool(ok);
}
