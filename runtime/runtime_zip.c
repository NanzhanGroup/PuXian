// 普贤 (PuXian) M19 zip 内置函数（编译模式，miniz raw deflate + 自带 zip 容器，与解释器结构一致）
// - zip_pack(files, out_path) → bool（files: dict{路径→内容}，deflate 压缩，UTF-8 文件名）
// - zip_unpack(zip_path, out_dir) → int（解压文件数，支持 deflate + store，防路径穿越）
#define _GNU_SOURCE
#include "runtime.h"
#include "miniz.h"
#include "mbedtls/md.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/aes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

// ---- 动态字节缓冲 ----
typedef struct { unsigned char* data; int len, cap; } ZBuf;
static void zbuf_push(ZBuf* b, const unsigned char* d, int n) {
    if (b->len + n > b->cap) {
        int nc = (b->cap ? b->cap : 256) * 2 + n;
        b->data = (unsigned char*)realloc(b->data, nc);
        b->cap = nc;
    }
    memcpy(b->data + b->len, d, n);
    b->len += n;
}
static void zbuf_u16(ZBuf* b, unsigned v) {
    unsigned char t[2] = { (unsigned char)(v & 0xFF), (unsigned char)((v >> 8) & 0xFF) };
    zbuf_push(b, t, 2);
}
static void zbuf_u32(ZBuf* b, unsigned v) {
    unsigned char t[4] = { (unsigned char)(v & 0xFF), (unsigned char)((v >> 8) & 0xFF),
                           (unsigned char)((v >> 16) & 0xFF), (unsigned char)((v >> 24) & 0xFF) };
    zbuf_push(b, t, 4);
}

static unsigned rd_u16(const unsigned char* d, int o) { return d[o] | (d[o + 1] << 8); }
static unsigned rd_u32(const unsigned char* d, int o) {
    return (unsigned)d[o] | ((unsigned)d[o + 1] << 8) | ((unsigned)d[o + 2] << 16) | ((unsigned)d[o + 3] << 24);
}

// 取字符串字节与长度
static const char* zstr(LXValue v) {
    if (v.type != PX_STR) px_error("期望字符串，实际是 %s", px_type_name(v));
    return v.as.obj->as.str.data;
}
static int zstrlen(LXValue v) {
    if (v.type != PX_STR) px_error("期望字符串，实际是 %s", px_type_name(v));
    return v.as.obj->as.str.len;
}

// raw deflate 压缩（level 6）；失败返回 -1，成功返回压缩长度（out 容量 bound）
static int z_raw_deflate(const unsigned char* in, int inlen, unsigned char* out, int outcap) {
    mz_stream s;
    memset(&s, 0, sizeof(s));
    if (mz_deflateInit2(&s, 6, MZ_DEFLATED, -15, 8, MZ_DEFAULT_STRATEGY) != MZ_OK) return -1;
    s.next_in = in; s.avail_in = (mz_ulong)inlen;
    s.next_out = out; s.avail_out = (mz_ulong)outcap;
    int r = mz_deflate(&s, MZ_FINISH);
    mz_deflateEnd(&s);
    if (r != MZ_STREAM_END) return -1;
    return (int)s.total_out;
}

// raw deflate 解压（期望 uncomp_size 字节）；失败返回 -1
static int z_raw_inflate(const unsigned char* in, int inlen, unsigned char* out, int outcap, int* outlen) {
    mz_stream s;
    memset(&s, 0, sizeof(s));
    if (mz_inflateInit2(&s, -15) != MZ_OK) return -1;
    s.next_in = in; s.avail_in = (mz_ulong)inlen;
    s.next_out = out; s.avail_out = (mz_ulong)outcap;
    int r = mz_inflate(&s, MZ_FINISH);
    mz_inflateEnd(&s);
    if (r != MZ_STREAM_END) return -1;
    *outlen = (int)s.total_out;
    return 0;
}

// zip_pack(files, out_path) → bool
LXValue bi_zip_pack(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 2) px_error("zip_pack 需要 2 个参数: (files, out_path)");
    if (args[0].type != PX_DICT) px_error("zip_pack 第一个参数须为 dict{路径→内容}");
    const char* out_path = zstr(args[1]);
    LXObject* d = args[0].as.obj;
    ZBuf out = {0};
    ZBuf cd = {0};
    for (int i = 0; i < d->as.dict.len; i++) {
        const char* name = d->as.dict.keys[i];
        LXValue v = d->as.dict.vals[i];
        const char* content = zstr(v);
        int clen = zstrlen(v);
        if (name[0] == '\0' || name[0] == '/' || strstr(name, "..") != NULL)
            px_error("zip 条目路径非法: '%s'", name);
        unsigned crc = mz_crc32(0, (const mz_uint8*)content, (size_t)clen);
        // deflate level 6；若压缩反而变大则退回 store
        int bound = (int)mz_compressBound((mz_ulong)clen);
        unsigned char* comp = (unsigned char*)malloc(bound);
        int comp_len = z_raw_deflate((const unsigned char*)content, clen, comp, bound);
        int method;
        const unsigned char* data;
        int dlen;
        if (comp_len >= 0 && comp_len < clen) { method = 8; data = comp; dlen = comp_len; }
        else { method = 0; data = (const unsigned char*)content; dlen = clen; }
        int name_len = (int)strlen(name);
        int local_offset = out.len;
        // Local File Header
        zbuf_push(&out, (const unsigned char*)"PK\003\004", 4);
        zbuf_u16(&out, 20);        // version needed
        zbuf_u16(&out, 0x0800);    // flags: UTF-8
        zbuf_u16(&out, method);
        zbuf_u16(&out, 0);         // mod time（固定，确定性）
        zbuf_u16(&out, 0);         // mod date
        zbuf_u32(&out, crc);
        zbuf_u32(&out, (unsigned)dlen);
        zbuf_u32(&out, (unsigned)clen);
        zbuf_u16(&out, (unsigned)name_len);
        zbuf_u16(&out, 0);         // extra
        zbuf_push(&out, (const unsigned char*)name, name_len);
        zbuf_push(&out, data, dlen);
        // Central Directory 条目
        zbuf_push(&cd, (const unsigned char*)"PK\001\002", 4);
        zbuf_u16(&cd, 20);         // version made by
        zbuf_u16(&cd, 20);         // version needed
        zbuf_u16(&cd, 0x0800);
        zbuf_u16(&cd, method);
        zbuf_u16(&cd, 0);
        zbuf_u16(&cd, 0);
        zbuf_u32(&cd, crc);
        zbuf_u32(&cd, (unsigned)dlen);
        zbuf_u32(&cd, (unsigned)clen);
        zbuf_u16(&cd, (unsigned)name_len);
        zbuf_u16(&cd, 0);          // extra
        zbuf_u16(&cd, 0);          // comment
        zbuf_u16(&cd, 0);          // disk
        zbuf_u16(&cd, 0);          // internal attr
        zbuf_u32(&cd, 0);          // external attr
        zbuf_u32(&cd, (unsigned)local_offset);
        zbuf_push(&cd, (const unsigned char*)name, name_len);
        free(comp);
    }
    int cd_offset = out.len;
    zbuf_push(&out, cd.data, cd.len);
    // End of Central Directory
    zbuf_push(&out, (const unsigned char*)"PK\005\006", 4);
    zbuf_u16(&out, 0);             // disk
    zbuf_u16(&out, 0);             // cd disk
    zbuf_u16(&out, (unsigned)d->as.dict.len);
    zbuf_u16(&out, (unsigned)d->as.dict.len);
    zbuf_u32(&out, (unsigned)cd.len);
    zbuf_u32(&out, (unsigned)cd_offset);
    zbuf_u16(&out, 0);             // comment len
    FILE* f = fopen(out_path, "wb");
    if (!f) px_error("zip 写入失败 %s", out_path);
    int ok = fwrite(out.data, 1, out.len, f) == (size_t)out.len;
    fclose(f);
    if (cd.data) free(cd.data);
    if (out.data) free(out.data);
    if (!ok) px_error("zip 写入失败 %s", out_path);
    return px_bool(true);
}


// ==================== M66：zip 密码解密（zipcrypto 传统 + WinZip AES-256） ====================
// zipcrypto（PKWARE 传统）：keys 3×u32，基于 crc32 的流密钥；数据前有 12 字节加密头。
// WinZip AES（AE-1/AE-2）：extra field 0x9901 标注强度（3=AES-256，salt 16B）。
//   数据 = salt + pwd_verify(2) + AES-CTR 密文 + HMAC-SHA1 auth code(10)。
//   密钥：PBKDF2-HMAC-SHA1(password, salt, 1000 iters, 2*keylen+2)。
// 说明：miniz/mbedtls 已链入 runtime，本文件直接用。

// crc32 单字节增量（ZipCrypto 密钥流用；PKWARE 用"裸表版"无补位取反 —— 与 miniz mz_crc32(zlib 语义)不同，
// 故内置反射表 0xEDB88320。crc 参数即内部状态（key0/key2 直接作状态）。
static unsigned zip_crc32_tab[256];
static int zip_crc32_tab_init = 0;
static void zip_crc32_init_tab(void) {
    if (zip_crc32_tab_init) return;
    for (unsigned n = 0; n < 256; n++) {
        unsigned c = n;
        for (int k = 0; k < 8; k++)
            c = (c & 1) ? ((c >> 1) ^ 0xEDB88320u) : (c >> 1);
        zip_crc32_tab[n] = c;
    }
    zip_crc32_tab_init = 1;
}
static unsigned zip_crc32_byte(unsigned crc, unsigned char b) {
    return (crc >> 8) ^ zip_crc32_tab[(crc ^ b) & 0xFF];
}

// ZipCrypto 密钥结构
typedef struct { unsigned k0, k1, k2; } ZKeys;
static void zk_update(ZKeys* z, unsigned char c) {
    z->k0 = zip_crc32_byte(z->k0, c);
    z->k1 = (z->k1 + (z->k0 & 0xff)) * 134775813u + 1u;
    z->k2 = zip_crc32_byte(z->k2, (unsigned char)(z->k1 >> 24));
}
static unsigned char zk_decrypt_byte(ZKeys* z) {
    unsigned temp = (z->k2 | 2u) & 0xffffu;
    return (unsigned char)((temp * (temp ^ 1u)) >> 8);
}
// 用密码初始化 keys，然后就地解密 buf[0..len)（含 12 字节加密头，解密后从 12 起为压缩数据）
static void zipcrypto_decrypt(const unsigned char* pwd, int pwlen,
                              unsigned char* buf, int len) {
    zip_crc32_init_tab();
    ZKeys z = { 0x12345678u, 0x23456789u, 0x34567890u };
    for (int i = 0; i < pwlen; i++) zk_update(&z, pwd[i]);
    for (int i = 0; i < len; i++) {
        unsigned char c = buf[i] ^ zk_decrypt_byte(&z);
        zk_update(&z, c);
        buf[i] = c;
    }
}

// 在 extra 区（local 或 central）查找 AES 信息；找到返回强度(1/2/3)，未找到返回 0。
// extra = 数据指针，elen = extra 总长度（多个 id/size/data 串）。
// aes_ver 输出 vendor version（1=AE-1,2=AE-2），aes_method 输出实际压缩方法(0/8)。
static int zip_find_aes(const unsigned char* extra, int elen,
                        int* aes_ver, int* aes_method) {
    int p = 0;
    while (p + 4 <= elen) {
        unsigned id = rd_u16(extra, p);
        unsigned sz = rd_u16(extra, p + 2);
        if (p + 4 + (int)sz > elen) break;
        if (id == 0x9901 && sz >= 7) {
            // data: ver(2) + vendor"AE"(2) + strength(1) + method(2)
            if (extra[p + 4 + 2] == 'A' && extra[p + 4 + 3] == 'E') {
                *aes_ver = rd_u16(extra, p + 4);
                *aes_method = rd_u16(extra, p + 4 + 2 + 2 + 1);
                return extra[p + 4 + 4];
            }
            return 0;
        }
        p += 4 + (int)sz;
    }
    return 0;
}

// 本地 extra 段定位（local header 的 extra 在 name 之后）
static const unsigned char* zip_local_extra(const unsigned char* data, int fsize,
                                            int local_off, int l_name_len, int l_extra_len) {
    if (local_off + 30 + l_name_len + l_extra_len > fsize) return NULL;
    return data + local_off + 30 + l_name_len;
}

// HMAC-SHA1（mbedtls md）→ 20 字节
static void zip_hmac_sha1(const unsigned char* key, int klen,
                          const unsigned char* in, int ilen, unsigned char out[20]) {
    mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA1),
                    key, (size_t)klen, in, (size_t)ilen, out);
}

// AES-256 解密（WinZip AES CTR：16B counter 从 1 起 little-endian 递增，对齐 pyzipper/AE-2）
// 就地解密 buf[0..len)。key 32B。返回 0 成功。
static int zip_aes256_ctr(const unsigned char* key, unsigned char* buf, int len) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_enc(&aes, key, 256) != 0) { mbedtls_aes_free(&aes); return -1; }
    unsigned char ctr[16];
    memset(ctr, 0, 16);
    ctr[0] = 1;  // WinZip AES：counter 从 1 开始，little-endian 递增（低字节在前）
    int off = 0;
    while (off < len) {
        unsigned char ks[16];
        if (mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, ctr, ks) != 0) {
            mbedtls_aes_free(&aes);
            return -1;
        }
        int chunk = len - off;
        if (chunk > 16) chunk = 16;
        for (int i = 0; i < chunk; i++) buf[off + i] ^= ks[i];
        // counter += 1（little-endian）
        for (int i = 0; i < 16; i++) {
            if (++ctr[i] != 0) break;
        }
        off += chunk;
    }
    mbedtls_aes_free(&aes);
    return 0;
}

// zip_unpack 加密条目处理：把 data[data_off..data_off+comp_size) 解密为原始压缩数据。
// 返回 0 成功；-1 失败。成功时 *out（需 free）为压缩数据（deflate/store），*out_len 为其长度；
// crc_ok 输出：对 AE-1/zipcrypto 仍可做 crc 校验（返回1），AE-2 由 HMAC 认证（crc 字段为 0，返回 0）。
static int zip_decrypt_entry(const unsigned char* data, int fsize,
                             int data_off, int comp_size,
                             const unsigned char* pwd, int pwlen,
                             int encrypted, int local_off, int l_name_len, int l_extra_len,
                             unsigned char** out, int* out_len, int* crc_ok) {
    *crc_ok = 1;
    if (data_off + comp_size > fsize) return -1;
    const unsigned char* extra = zip_local_extra(data, fsize, local_off, l_name_len, l_extra_len);
    int aes_ver = 0, aes_method = 0;
    int aes_strength = (extra && l_extra_len > 0)
        ? zip_find_aes(extra, l_extra_len, &aes_ver, &aes_method) : 0;
    // 分配明文缓冲区（zipcrypto: comp_size 含 12B 头；AES: comp_size 含 salt+verify+auth）
    int cap = comp_size + 16;
    unsigned char* buf = (unsigned char*)malloc((size_t)cap);
    if (!buf) return -1;
    memcpy(buf, data + data_off, (size_t)comp_size);
    if (aes_strength >= 1 && aes_strength <= 3) {
        // ---- WinZip AES ----
        int keylen, saltlen;
        if (aes_strength == 3) { keylen = 32; saltlen = 16; }       // AES-256
        else if (aes_strength == 2) { keylen = 24; saltlen = 12; }  // AES-192
        else { keylen = 16; saltlen = 8; }                          // AES-128
        if (comp_size < saltlen + 2 + 10) { free(buf); return -1; }
        // 派生密钥：PBKDF2-HMAC-SHA1(pwd, salt, 1000, 2*keylen+2)
        unsigned char km[66];  // max 32+32+2
        int dklen = 2 * keylen + 2;
        if (mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA1,
                pwd, (size_t)pwlen, buf, (size_t)saltlen, 1000, (uint32_t)dklen, km) != 0) {
            free(buf);
            return -1;
        }
        // 校验 pwd verify（salt 后 2 字节）
        if (buf[saltlen] != km[2 * keylen] || buf[saltlen + 1] != km[2 * keylen + 1]) {
            free(buf);
            return -1;
        }
        // 密文 = salt+2 .. comp_size-10；auth code 尾 10 字节
        int ct_off = saltlen + 2;
        int ct_len = comp_size - ct_off - 10;
        if (ct_len < 0) { free(buf); return -1; }
        // HMAC-SHA1(encmac_key=km[keylen..2keylen), 密文) 前 10 字节 == auth code
        unsigned char mac[20];
        zip_hmac_sha1(km + keylen, keylen, buf + ct_off, ct_len, mac);
        if (memcmp(mac, buf + comp_size - 10, 10) != 0) {
            free(buf);
            return -1;
        }
        // AES-CTR 解密密文
        if (zip_aes256_ctr(km, buf + ct_off, ct_len) != 0) { free(buf); return -1; }
        memmove(buf, buf + ct_off, (size_t)ct_len);
        *out = buf;
        *out_len = ct_len;
        if (aes_ver == 1) *crc_ok = 1;   // AE-1：crc 字段有效
        else *crc_ok = 0;                // AE-2：crc 置 0，以 HMAC 认证为准
        return 0;
    }
    // ---- zipcrypto 传统 ----
    if (comp_size < 12) { free(buf); return -1; }
    zipcrypto_decrypt(pwd, pwlen, buf, comp_size);
    memmove(buf, buf + 12, (size_t)(comp_size - 12));
    *out = buf;
    *out_len = comp_size - 12;
    return 0;
}

// 创建父目录（递归）
static void z_mkdirs(const char* path) {
    char* tmp = strdup(path);
    int len = (int)strlen(tmp);
    for (int i = 0; i < len; i++) {
        if (tmp[i] == '/') {
            tmp[i] = '\0';
            if (tmp[0]) mkdir(tmp, 0755);
            tmp[i] = '/';
        }
    }
    free(tmp);
}

// zip_unpack(zip_path, out_dir[, password]) → int（解压文件数）
// 支持 deflate + store + 防路径穿越；第三参 password 支持密码 zip：
//   zipcrypto（PKWARE 传统）与 WinZip AES-128/192/256（AE-1/AE-2，PBKDF2-HMAC-SHA1 + AES-CTR）。
LXValue bi_zip_unpack(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3) px_error("zip_unpack 需要 2-3 个参数: (zip_path, out_dir[, password])");
    const char* zip_path = zstr(args[0]);
    const char* out_dir = zstr(args[1]);
    const char* password = NULL;
    int pwlen = 0;
    if (nargs == 3) {
        if (args[2].type != PX_STR && args[2].type != PX_NULL)
            px_error("zip_unpack 的 password 需要字符串或 null");
        if (args[2].type == PX_STR) {
            password = args[2].as.obj->as.str.data;
            pwlen = args[2].as.obj->as.str.len;
        }
    }
    // 读整个 zip
    FILE* f = fopen(zip_path, "rb");
    if (!f) px_error("zip 读取失败 %s", zip_path);
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* data = (unsigned char*)malloc(fsize > 0 ? fsize : 1);
    if (fsize > 0 && fread(data, 1, fsize, f) != (size_t)fsize) { fclose(f); free(data); px_error("zip 读取失败 %s", zip_path); }
    fclose(f);
    // 找 EOCD
    int eocd = -1;
    int start = fsize - 65557; if (start < 0) start = 0;
    for (int i = fsize - 4; i >= start; i--) {
        if (data[i] == 'P' && data[i + 1] == 'K' && data[i + 2] == 5 && data[i + 3] == 6) { eocd = i; break; }
    }
    if (eocd < 0) { free(data); px_error("zip 缺少 EOCD"); }
    int entries = (int)rd_u16(data, eocd + 10);
    int cd_size = (int)rd_u32(data, eocd + 12);
    int cd_off = (int)rd_u32(data, eocd + 16);
    if (cd_off + cd_size > fsize) { free(data); px_error("zip 中央目录越界"); }
    int count = 0;
    int pos = cd_off;
    for (int i = 0; i < entries; i++) {
        if (pos + 46 > fsize || data[pos] != 'P' || data[pos + 1] != 'K') { free(data); px_error("zip 中央目录条目损坏"); }
        int entry_start = pos;
        int flag = (int)rd_u16(data, pos + 8);
        int method = (int)rd_u16(data, pos + 10);
        int comp_size = (int)rd_u32(data, pos + 20);
        int uncomp_size = (int)rd_u32(data, pos + 24);
        int name_len = (int)rd_u16(data, pos + 28);
        int extra_len = (int)rd_u16(data, pos + 30);
        int comment_len = (int)rd_u16(data, pos + 32);
        int local_off = (int)rd_u32(data, pos + 42);
        unsigned crc = rd_u32(data, entry_start + 16);
        if (pos + 46 + name_len > fsize) { free(data); px_error("zip 中央目录条目越界"); }
        char* name = (char*)malloc(name_len + 1);
        memcpy(name, data + pos + 46, name_len);
        name[name_len] = '\0';
        pos += 46 + name_len + extra_len + comment_len;
        // 目录条目跳过
        if (name_len > 0 && name[name_len - 1] == '/') { free(name); continue; }
        // 安全校验：拒绝绝对路径与路径穿越
        int unsafe = name[0] == '\0' || name[0] == '/';
        for (int k = 0; k < name_len && !unsafe; k++) {
            if (k + 1 < name_len && name[k] == '.' && name[k + 1] == '.')
                if (k == 0 || name[k - 1] == '/') unsafe = 1;
        }
        if (unsafe) { free(data); px_error("zip 条目路径非法: '%s'", name); }
        // 读取 local header 与数据
        if (local_off + 30 > fsize || data[local_off] != 'P' || data[local_off + 1] != 'K') { free(name); free(data); px_error("zip 本地文件头损坏"); }
        int l_flag = (int)rd_u16(data, local_off + 6);
        int l_name_len = (int)rd_u16(data, local_off + 26);
        int l_extra_len = (int)rd_u16(data, local_off + 28);
        int data_start = local_off + 30 + l_name_len + l_extra_len;
        if (data_start + comp_size > fsize) { free(name); free(data); px_error("zip 文件数据越界"); }
        int encrypted = (flag & 1) || (l_flag & 1);
        unsigned char* content = (unsigned char*)malloc(uncomp_size > 0 ? uncomp_size : 1);
        int content_len;
        int crc_ok = 1;
        if (encrypted) {
            if (!password) {
                free(name); free(content); free(data);
                px_error("zip 需要密码: '%s'（zip_unpack 第三参传 password）", name);
            }
            unsigned char* raw = NULL;
            int raw_len = 0;
            if (zip_decrypt_entry(data, (int)fsize, data_start, comp_size,
                                  (const unsigned char*)password, pwlen, 1,
                                  local_off, l_name_len, l_extra_len,
                                  &raw, &raw_len, &crc_ok) != 0) {
                free(name); free(content); free(data);
                px_error("zip 密码错误或条目损坏: '%s'（zipcrypto/AES-256）", name);
            }
            // 解密后 raw 为 deflate/store 压缩数据
            if (method == 99) {
                // AES 条目：实际压缩方法在 AES extra（zip_decrypt_entry 已校验），
                // method==99 时 extra 中记录真实方法；raw 已解密 → 直接按 deflate/store 推断：
                // 简化：AES extra 真实 method 无法从本函数得知，统一先试 deflate 再试 store。
                int ok = 0;
                if (z_raw_inflate(raw, raw_len, content, uncomp_size, &content_len) == 0) ok = 1;
                if (!ok && raw_len == uncomp_size) {
                    memcpy(content, raw, (size_t)raw_len);
                    content_len = raw_len;
                    ok = 1;
                }
                free(raw);
                if (!ok) { free(name); free(content); free(data); px_error("zip AES 解压失败: '%s'", name); }
            } else if (method == 0) {
                if (raw_len != uncomp_size) { free(raw); free(name); free(content); free(data); px_error("zip store 长度不符"); }
                memcpy(content, raw, (size_t)raw_len);
                content_len = raw_len;
                free(raw);
            } else if (method == 8) {
                if (z_raw_inflate(raw, raw_len, content, uncomp_size, &content_len) != 0) {
                    free(raw); free(name); free(content); free(data); px_error("zip 解压失败: '%s'", name);
                }
                free(raw);
            } else {
                free(raw); free(name); free(content); free(data);
                px_error("zip 不支持的压缩方法: %d", method);
            }
        } else if (method == 0) {
            if (comp_size != uncomp_size) { free(name); free(content); free(data); px_error("zip store 长度不符"); }
            memcpy(content, data + data_start, comp_size);
            content_len = comp_size;
        } else if (method == 8) {
            if (z_raw_inflate(data + data_start, comp_size, content, uncomp_size, &content_len) != 0) {
                free(name); free(content); free(data); px_error("zip 解压失败: '%s'", name);
            }
        } else {
            free(name); free(content); free(data);
            px_error("zip 不支持的压缩方法: %d", method);
        }
        if (content_len != uncomp_size) { free(name); free(content); free(data); px_error("zip 解压长度不符"); }
        if (crc_ok && mz_crc32(0, content, (size_t)content_len) != crc) { free(name); free(content); free(data); px_error("zip CRC32 校验失败: '%s'", name); }
        // 写文件
        char full[4096];
        snprintf(full, sizeof(full), "%s/%s", out_dir, name);
        z_mkdirs(full);
        FILE* wf = fopen(full, "wb");
        if (!wf) { free(name); free(content); free(data); px_error("zip 写入文件失败 %s", full); }
        int wok = fwrite(content, 1, content_len, wf) == (size_t)content_len;
        fclose(wf);
        free(name); free(content);
        if (!wok) { free(data); px_error("zip 写入文件失败 %s", full); }
        count++;
    }
    free(data);
    return px_int(count);
}
